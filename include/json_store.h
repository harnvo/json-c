#pragma once

#include "shared.h"
#include "str_view.h"
#include "json_obj.h"

#include <stdio.h>
#include <stdlib.h>

// define storage place for json_obj

#ifdef __cplusplus
extern "C" {
#endif

struct _json_op {
  // int (*add) (struct json *json, struct json_obj *obj);
  struct json_obj *(*_add_empty) (struct json *json);
  // int (*remove) (struct json *json, struct json_obj *obj);
  int (*remove_by_key) (struct json *json, str_view_t *key);
  int (*remove_by_index) (struct json *json, size_t index);

  struct json_obj *(*get_by_key) (struct json *json, str_view_t *key);
  struct json_obj *(*get_by_index) (struct json *json, size_t index);

  void (*destroy) (struct json *json);

  /* size */
  size_t (*size) (const struct json *json);

  /* a iterator for convenience. */
  struct json_obj *(*begin) (const struct json *json);
  struct json_obj *(*next) (const struct json *json, struct json_obj *obj);
  struct json_obj *(*end) (const struct json *json);

  /* extra op for internal use. 
   * If you want to use it, you should know what you are doing.
   * currently unused.
   */
  void (*__remove_by_obj) (struct json *json, struct json_obj *obj);
};

struct json {
  void *storage;
  // size_t size;

  struct _json_op *_op;
};

__EXPOSED __HEADER_ONLY int
json_add (struct json *json, struct json_obj *obj) {
  struct json_obj *new_obj = json->_op->_add_empty (json);
  if (!new_obj) {
    return -1;
  }

  memcpy (new_obj, obj, sizeof (struct json_obj));
  return 0;
}

__EXPOSED __HEADER_ONLY struct json_obj *
_json_add_empty (struct json *json) {
  return json->_op->_add_empty (json);
}

__EXPOSED __HEADER_ONLY int
json_remove_by_key (struct json *json, const char *key) {
  // make a str_view_t
  str_view_t _view = { (char*) key, strlen (key) };
  return json->_op->remove_by_key (json, &_view);
}

__EXPOSED __HEADER_ONLY int
json_remove_by_key_view (struct json *json, str_view_t *key) {
  return json->_op->remove_by_key (json, key);
}

__EXPOSED __HEADER_ONLY int
json_remove_by_index (struct json *json, size_t index) {
  return json->_op->remove_by_index (json, index);
}

__EXPOSED __HEADER_ONLY struct json_obj *
json_get_by_key (struct json *json, const char *key) {
  // make a str_view_t
  str_view_t _view = { (char*) key, strlen (key) };
  return json->_op->get_by_key (json, &_view);
}

__EXPOSED __HEADER_ONLY struct json_obj *
json_get_by_key_view (struct json *json, str_view_t *key) {
  return json->_op->get_by_key (json, key);
}

__EXPOSED __HEADER_ONLY struct json_obj *
json_get_by_index (struct json *json, size_t index) {
  return json->_op->get_by_index (json, index);
}

__EXPOSED __HEADER_ONLY void
json_destroy (struct json *json) {
  json->_op->destroy (json);
}

__EXPOSED __HEADER_ONLY size_t
json_get_size (const struct json *json) {
  return json->_op->size (json);
}

__EXPOSED __HEADER_ONLY int
json_is_empty (const struct json *json) {
  return json_get_size (json) == 0;
}

__EXPOSED __HEADER_ONLY struct json_obj *
json_begin (const struct json *json) {
  return json->_op->begin (json);
}

__EXPOSED __HEADER_ONLY struct json_obj *
json_next (const struct json *json, struct json_obj *obj) {
  return json->_op->next (json, obj);
}

__EXPOSED __HEADER_ONLY struct json_obj *
json_end (const struct json *json) {
  return json->_op->end (json);
}

// iterative print
__HIDDEN __HEADER_ONLY void
__json_print (const struct json *json, int flags, int __cur_depth) {
  struct json_obj *obj = json_begin (json);
  while (obj != json_end (json)) {
    __json_obj_print (obj, flags, __cur_depth);
    obj = json_next (json, obj);
  }
}

__EXPOSED __HEADER_ONLY void
json_print (const struct json *json, int flags) {
  __json_print (json, flags, 0);
}

/* --- list storage --- */
typedef struct json_list_storage json_list_storage_t;

int json_list_storage_init (struct json *json,
                            json_list_storage_t *storage);
                            
void json_list_storage_destroy (struct json *json);
