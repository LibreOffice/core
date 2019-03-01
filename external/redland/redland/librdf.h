/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * redland.h - Redland RDF Application Framework public API
 *
 * Copyright (C) 2000-2011, David Beckett http://www.dajobe.org/
 * Copyright (C) 2000-2005, University of Bristol, UK http://www.bristol.ac.uk/
 *
 * This package is Free Software and part of Redland http://librdf.org/
 *
 * It is licensed under the following three licenses as alternatives:
 *   1. GNU Lesser General Public License (LGPL) V2.1 or any newer version
 *   2. GNU General Public License (GPL) V2 or any newer version
 *   3. Apache License, V2.0 or any newer version
 *
 * You may not use this file except in compliance with at least one of
 * the above three licenses.
 *
 * See LICENSE.html or LICENSE.txt at the top of this package for the
 * complete terms and further detail along with the license texts for
 * the licenses in COPYING.LIB, COPYING and LICENSE-2.0.txt respectively.
 *
 *
 */


#ifndef LIBRDF_H
#define LIBRDF_H

#ifndef LIBRDF_OBJC_FRAMEWORK
/* raptor */
#include <raptor2.h>
/* rasqal: uses raptor */
#include <rasqal.h>
/* librdf: uses rasqal and raptor */
#else
#include <Redland/raptor2.h>
#include <Redland/rasqal.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

#ifndef REDLAND_API
#  ifdef _WIN32
#    ifdef __GNUC__
#      undef _declspec
#      define _declspec(x) __declspec(x)
#    endif
#    ifdef REDLAND_STATIC
#      define REDLAND_API
#    else
#      ifdef LIBRDF_INTERNAL
#        define REDLAND_API _declspec(dllexport)
#      else
#        define REDLAND_API _declspec(dllimport)
#      endif
#    endif
#  else
#    define REDLAND_API
#  endif
#endif

#ifndef REDLAND_CALLBACK_STDCALL
#  if defined(_WIN32) && defined(USE_STDCALL_CALLBACKS)
#    define REDLAND_CALLBACK_STDCALL _stdcall
#  else
#    define REDLAND_CALLBACK_STDCALL
#  endif
#endif

/* Use gcc 3.1+ feature to allow marking of deprecated API calls.
 * This gives a warning during compiling.
 */
#if ( __GNUC__ == 3 && __GNUC_MINOR__ > 0 ) || __GNUC__ > 3
#define REDLAND_DEPRECATED __attribute__((deprecated))
#define REDLAND_NORETURN __attribute__((__noreturn__))
#else
#define REDLAND_DEPRECATED
#define REDLAND_NORETURN
#endif


#if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ > 4)
#define REDLAND_PRINTF_FORMAT(string_index, first_to_check_index) \
  __attribute__((__format__(__printf__, string_index, first_to_check_index)))
#else
#define REDLAND_PRINTF_FORMAT(string_index, first_to_check_index)
#endif


/* Public defines */

/**
 * LIBRDF_VERSION:
 *
 * Redland librdf library version number
 *
 * Format: major * 10000 + minor * 100 + release
 */
#define LIBRDF_VERSION 10017

/**
 * LIBRDF_VERSION_STRING:
 *
 * Redland librdf library version string
 */
#define LIBRDF_VERSION_STRING "1.0.17"

/**
 * LIBRDF_VERSION_MAJOR:
 *
 * Redland librdf library major version
 */
#define LIBRDF_VERSION_MAJOR 1

/**
 * LIBRDF_VERSION_MINOR:
 *
 * Redland librdf library minor version
 */
#define LIBRDF_VERSION_MINOR 0

/**
 * LIBRDF_VERSION_RELEASE:
 *
 * Redland librdf library release
 */
#define LIBRDF_VERSION_RELEASE 17



/* Public typedefs (references to private structures) */

/**
 * librdf_world:
 *
 * Redland world class.
 */
typedef struct librdf_world_s librdf_world;

/**
 * librdf_hash:
 *
 * Redland hash class.
 */
typedef struct librdf_hash_s librdf_hash;

/**
 * librdf_hash_cursor:
 *
 * Redland hash cursor class.
 */
typedef struct librdf_hash_cursor_s librdf_hash_cursor;

/**
 * librdf_digest:
 *
 * Redland content digest class.
 */
typedef struct librdf_digest_s librdf_digest;

/**
 * librdf_digest_factory:
 *
 * Redland digest factory class.
 */
typedef struct librdf_digest_factory_s librdf_digest_factory;

/**
 * librdf_uri:
 *
 * Redland URI class.
 */
typedef struct raptor_uri_s librdf_uri;

/**
 * librdf_list:
 *
 * Redland list class.
 */
typedef struct librdf_list_s librdf_list;

/**
 * librdf_iterator:
 *
 * Redland iterator class.
 */
typedef struct librdf_iterator_s librdf_iterator;

/**
 * librdf_node:
 *
 * Redland node class.
 */
typedef raptor_term librdf_node;

/**
 * librdf_statement:
 *
 * Redland statement class.
 */
typedef raptor_statement librdf_statement;

/**
 * librdf_model:
 *
 * Redland model class.
 */
typedef struct librdf_model_s librdf_model;

/**
 * librdf_model_factory:
 *
 * Redland model factory class.
 */
typedef struct librdf_model_factory_s librdf_model_factory;

/**
 * librdf_storage:
 *
 * Redland storage class.
 */
typedef struct librdf_storage_s librdf_storage;

/**
 * librdf_storage_factory:
 *
 * Redland storage factory class.
 */
typedef struct librdf_storage_factory_s librdf_storage_factory;

/**
 * librdf_stream:
 *
 * Redland stream class.
 */
typedef struct librdf_stream_s librdf_stream;

/**
 * librdf_parser:
 *
 * Redland parser class.
 */
typedef struct librdf_parser_s librdf_parser;

/**
 * librdf_parser_factory:
 *
 * Redland parser factory class.
 */
typedef struct librdf_parser_factory_s librdf_parser_factory;

/**
 * librdf_query:
 *
 * Redland query class.
 */
typedef struct librdf_query_s librdf_query;

/**
 * librdf_query_factory:
 *
 * Redland query factory class.
 */
typedef struct librdf_query_factory_s librdf_query_factory;

/**
 * librdf_query_results:
 *
 * Redland query results class.
 */
typedef struct librdf_query_results_s librdf_query_results;

/**
 * librdf_query_results_formatter:
 *
 * Redland query results formatter class.
 */
typedef struct librdf_query_results_formatter_s librdf_query_results_formatter;

/**
 * librdf_serializer:
 *
 * Redland serializer class.
 */
typedef struct librdf_serializer_s librdf_serializer;

/**
 * librdf_serializer_factory:
 *
 * Redland serializer factory class.
 */
typedef struct librdf_serializer_factory_s librdf_serializer_factory;


/* Public statics */

/**
 * librdf_short_copyright_string:
 *
 * Short copyright string (one line).
 */
REDLAND_API
extern const char * const librdf_short_copyright_string;

/**
 * librdf_copyright_string:
 *
 * Copyright string (multiple lines).
 */
REDLAND_API
extern const char * const librdf_copyright_string;

/**
 * librdf_version_string:
 *
 * Redland librdf version as a string.
 */
REDLAND_API
extern const char * const librdf_version_string;

/**
 * librdf_version_major:
 *
 * Redland librdf major version number.
 */
REDLAND_API
extern const unsigned int librdf_version_major;

/**
 * librdf_version_minor:
 *
 * Redland librdf minor version number.
 */
REDLAND_API
extern const unsigned int librdf_version_minor;

/**
 * librdf_version_release:
 *
 * Redland librdf release version number.
 */
REDLAND_API
extern const unsigned int librdf_version_release;

/**
 * librdf_version_decimal:
 *
 * Redland librdf version as a decimal number.
 *
 * Format: major * 10000 + minor * 100 + release
 */
REDLAND_API
extern const unsigned int librdf_version_decimal;

/**
 * librdf_license_string:
 *
 * Redland librdf license string.
 */
REDLAND_API
extern const char * const librdf_license_string;

/**
 * librdf_home_url_string:
 *
 * Redland librdf home page URL.
 */
REDLAND_API
extern const char * const librdf_home_url_string;

/* Required for va_list in error handler function registrations
 * which are in the public API
 */
#include <stdarg.h>


/* internal interfaces  */
#ifdef LIBRDF_INTERNAL
#include <rdf_internal.h>
#endif

/* public interfaces */

/* FIXME: Should be replaced with automatically pulled
 * definitions from the listed rdf_*.h header files.
 */

#ifndef LIBRDF_OBJC_FRAMEWORK
#include <rdf_log.h>
#include <rdf_digest.h>
#include <rdf_hash.h>
#include <rdf_init.h>
#include <rdf_iterator.h>
#include <rdf_uri.h>
#include <rdf_node.h>
#include <rdf_concepts.h>
#include <rdf_statement.h>
#include <rdf_model.h>
#include <rdf_storage.h>
#include <rdf_parser.h>
#include <rdf_raptor.h>
#include <rdf_serializer.h>
#include <rdf_stream.h>
#include <rdf_query.h>
#include <rdf_utf8.h>
#else
#include <Redland/rdf_log.h>
#include <Redland/rdf_digest.h>
#include <Redland/rdf_hash.h>
#include <Redland/rdf_init.h>
#include <Redland/rdf_iterator.h>
#include <Redland/rdf_uri.h>
#include <Redland/rdf_node.h>
#include <Redland/rdf_concepts.h>
#include <Redland/rdf_statement.h>
#include <Redland/rdf_model.h>
#include <Redland/rdf_storage.h>
#include <Redland/rdf_parser.h>
#include <Redland/rdf_raptor.h>
#include <Redland/rdf_serializer.h>
#include <Redland/rdf_stream.h>
#include <Redland/rdf_query.h>
#include <Redland/rdf_utf8.h>
#endif

#ifdef __cplusplus
}
#endif

#endif
