# [Unfinished] cjson-light

A lightweight, modular, fast, and easy-to-use JSON library in c.

## Key features
[spotlights](#spotlights) | [easy-to-use](#usage) | [fast](#benchmark) | [lightweight](#lightweight)

## Getting started

__Supported Platforms__
I only tested my code with gnu-c99/c11 on Ubuntu-20.04.

__Building__

So far only xmake is supported to build the project. 
Run `xmake` to build 

__Including json-c__

If you make successfully installed it, you may include it like:
```
#include <json.h>
```

## Data structures

### __*str_view_t*__
`str_view_t` is provided to facilitate string usage and avoid frequent malloc/free, and it works quite like the `std::string_view` in c++11. The major difference is that our str_view is *NOT* read-only, and you may modify the char* value inside.
the struct looks like:
  ```
  
struct str_view {
  char *str;
  size_t len;
};
  ...
  
typedef struct str_view str_view_t;
```

We also have our own custom printf specifier and one can use `printf` to print a `str_view_t` just like other variables:
*Warning: only gnu-c supports this feature*
```
str_view_t sv;
str_view_init(&sv,"foo", 3);
printf("print str_view_t:%v\n", sv);

>>> print str_view_t:foo
```

### __*struct json_obj*__
`json_obj` is a struct contraining key and value infos. It is defined as follows:

```
  struct json_obj {
    str_view_t key; // should not be used for array
  
    union {
      struct json *array;
      struct json *object; // to be introduced in the next data structure : )
      str_view_t str;
      double number;
      int boolean;
  
      char lz_str[__JSON_OBJ_LZ_STR_LEN]; // lazy string to avoid malloc
    } value;

    // FOR INTERNAL USE.
    char *__source;
    size_t __source_len;
    int type;
  };
```

### __*struct json*__
`struct json` is the structure that stores an array/collection of `struct json_obj`. It is designed to hide implementation details of how the json object array/collection is stored and managed.
the definition is as follows:
```
struct json {
  void *storage;
  struct _json_op *_op;
};
```
There are some basic operations exposed to users:
- init: initiate the json. you need to use different initiation function for different storage type (e.g. `json_list_storage_init`). (So far only linked-list storage is supported.)
- `json_destroy`: destroy everything within json.
- `json_add`: add an json_obj into json. memory copy involved.
- `_json_add_empty`: add an empty json_obj into json, returns the pointer to the obj. Used in case you wanna avoid memory copy.
- `json_remove_by_key`: remove a json_obj by key (char*).
- `json_remove_by_key_view`: remove a json_obj by key (str_view_t).
- `json_remove_by_index`: remove a json_obj by index.
- `json_get_by_key`: get a json_obj by key (char*).
- `json_get_by_key_view`: get a json_obj by key (str_view_t).
- `json_get_by_index`: get a json_obj by index.
- `json_get_size`: return number of elements within json (non-recursive. i.e. nested json counts as one obj.)
- `json_begin`, `json_next`, `json_end`: a iterator for user's convenience. To use it:
  ```
    for (
      struct json_obj *cur = json_begin (&json_parsed);
      cur != json_end (&json_parsed);
      cur = json_next (&json_parsed, cur)
    ) {
      ...
    }
    ```


## Usage

### __*Parsing*__
```
  struct json json_parsed;
  json_list_storage_init (&json_parsed);

  char *json_str = "{\"foo\": \"bar\", \"num\": 123.456, \"bool\": true, \"arr\": [1,2,3,4,5]}";
  json_parse (&json_parsed, json_str, strlen (json_str));

  struct json_obj *obj = json_get_by_key (&json_parsed, "foo");
  str_view_t sv = json_obj_get_value_str (obj);
  printf ("sv.str=%v\n", sv);

  obj = json_get_by_key (&json_parsed, "num");
  double num = json_obj_get_value_number (obj);
  printf ("num=%f\n", num);

  obj = json_get_by_key (&json_parsed, "bool");
  int boolean = json_obj_get_value_boolean (obj);
  printf ("bool=%d\n", boolean);

  obj = json_get_by_key (&json_parsed, "arr");
  struct json *arr = json_obj_get_value_array (obj);
  printf ("arr.size=%d\n", json_get_size (arr));

  json_destroy (&json_parsed);
```

### __*Iteration*__
```
  struct json json_parsed;
  json_list_storage_init (&json_parsed);

  char *json_str = "{\"foo\": \"bar\", \"num\": 123.456, \"bool\": true, \"arr\": [1,2,3,4,5]}";
  json_parse (&json_parsed, json_str, strlen (json_str));

  for (
    struct json_obj *cur = json_begin (&json_parsed);
    cur != json_end (&json_parsed);
    cur = json_next (&json_parsed, cur)
  ) {
    str_view_t sv = json_obj_get_key (cur);
    printf ("key=%v\n", sv);
  }

  json_destroy (&json_parsed);
```

### __*Printing*__
```
  str_view_t sv;
  str_view_init (&sv, "foo", 3);
  // just like normal printf!
  printf ("sv=%v\n", sv);
```


## Spotlights

### __*Data structures*__

```
// I don't know what storage type it is, but it doesn't matter!
struct json *json1 = json_new(cstr); 
json_delete(json1);

struct json json2; // you may put it on stack

// in case you want control over the storage type
json_list_storage_init (&json2, cstr); // make it list storage
json_destroy (&json2);

json_vector_storage_init (&json2, cstr); // you can also make it vector storage
json_destroy (&json2); // just destroy, you  don't need to care about the storage type!
```

As you may notice, `cJSON` uses linked list to store json objects for simplicity. However, it is not efficient in terms of memory usage and performance. 

In this library, I would provide a more efficient way without losing simplicity: I would use a virtual-function like method to abstract basic apis for user, and provide different implementations for different storage types. The operations for each storage type is `struct _json_op`. In this way, we can easily switch between different storage types without changing the user code. Moreover, one can easily implement his/her own storage type and use it in this library simply by providing the corresponding definition of storage, `struct _json_op` implementation and initinizer of storage. This makes the library modularized and extensible.

### __*Better memory management*__

**String view**

`cJSON` uses `char *` for storing key and string values, this makes allocations of memory required during parsing. However, most of time we just want a read-only reference to key/value. So we provide `str_view_t` to avoid frequent memory allocations. During parsing, we only store the view of the string to reference the key/value in the original json string. This makes the parsing process faster and more memory efficient.

**Lazy string**
When building a json object, sometimes we need to convert a number or boolean to string. In this case memory allocation seems to be inevitable. However, in this case, the value string is always small enough to be stored in within 128 bytes. So we provide a `lz_str` field in `struct json_obj` to store the string value to again avoid memory allocation. This makes it possible to avoid frequent memory allocation in most cases.

### __*Easy printing*__

```
printf("just like normal printf! %v\n", json_obj_get_key(obj));

>>> just like normal printf! foo
```

Yes, you could use `printf` on `str_view_t` directly. This is because we provide a custom printf specifier `%v` for `str_view_t`. This makes printing a `str_view_t` as easy as printing a normal string. In the future, I would make it possible to print a json object directly using `printf`.

### __*Easy iteration*__

```
for (
  struct json_obj *cur = json_begin (&json_parsed);
  cur != json_end (&json_parsed);
  cur = json_next (&json_parsed, cur)
) {
  str_view_t sv = json_obj_get_key (cur);
  printf ("key=%v\n", sv);
}
```

We provide a iterator for user's convenience. To use it, just use `json_begin`, `json_next`, `json_end` to iterate through the json object array/collection. This makes it possible to iterate through the json object array/collection without knowing the underlying storage type.

## Modular

All storage type implementations are within a single file, and you could just have your own implementation by starting another file. Don't forget to have your definition of storage in `json_store.h`


## Lightweight

`cJSON` was the most ultra-lightweight json library written in c (there were approximately 500 lines of code, if I remember correctly). However, it now has about 5000 lines of code as of 2021-1-20. Moreover, it is not modularized and you could see thousands of lines of code in a single file. I think it is a bit too much for a lightweight library, so I believe it is necessary to write a new lightweight json library in c.

### __*Code size*__
| Library | lines of code (*.h) | lines of code (*.c) | total lines of code | maximum lines of code in a single file |
| --- | --- | --- | --- | --- |
| cjson-light | 1492  | 1589 | **3081** | 552 |
| cJSON |  388 |  4610 | 4998 | 3129 |

*We only count \*.c and \*.h files that is actually used to build the released library. That is, code for testing and benchmark are not included.*

## Benchmark

Because of the reduced memory allocation, the parsing process is faster than `cJSON`.

No rigorous benchmark is available yet, but I have done some simple tests on my laptop (AMD Ryzen 7 4800H, Ubuntu 20.04, gcc 9.4.0). Parsing is 2-3 times faster when using the same storage type (linked list) as `cJSON`, and 1-2 times faster when using vector storage (slower parsing for faster indexing).