/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * rasqal.h - Rasqal RDF Query library interfaces and definition
 *
 * Copyright (C) 2003-2010, David Beckett http://www.dajobe.org/
 * Copyright (C) 2003-2005, University of Bristol, UK http://www.bristol.ac.uk/
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
 */



#ifndef RASQAL_H
#define RASQAL_H


#ifdef __cplusplus
extern "C" {
#endif


/**
 * RASQAL_VERSION:
 *
 * Rasqal library version number
 *
 * Format: major * 10000 + minor * 100 + release
 */
#define RASQAL_VERSION 930

/**
 * RASQAL_VERSION_STRING:
 *
 * Rasqal library version string
 */
#define RASQAL_VERSION_STRING "0.9.30"

/**
 * RASQAL_VERSION_MAJOR:
 *
 * Rasqal library major version
 */
#define RASQAL_VERSION_MAJOR 0

/**
 * RASQAL_VERSION_MINOR:
 *
 * Rasqal library minor version
 */
#define RASQAL_VERSION_MINOR 9

/**
 * RASQAL_VERSION_RELEASE:
 *
 * Rasqal library release
 */
#define RASQAL_VERSION_RELEASE 30


/**
 * RASQAL_API:
 *
 * Macro for wrapping API function call declarations.
 *
 */
#ifndef RASQAL_API
#  ifdef WIN32
#    ifdef __GNUC__
#      undef _declspec
#      define _declspec(x) __declspec(x)
#    endif
#    ifdef RASQAL_STATIC
#      define RASQAL_API
#    else
#      ifdef RASQAL_INTERNAL
#        define RASQAL_API _declspec(dllexport)
#      else
#        define RASQAL_API _declspec(dllimport)
#      endif
#    endif
#  else
#    define RASQAL_API
#  endif
#endif

/* Use gcc 3.1+ feature to allow marking of deprecated API calls.
 * This gives a warning during compiling.
 */
#if ( __GNUC__ == 3 && __GNUC_MINOR__ > 0 ) || __GNUC__ > 3
#define RASQAL_DEPRECATED __attribute__((deprecated))
#else
#define RASQAL_DEPRECATED
#endif


#ifndef LIBRDF_OBJC_FRAMEWORK
#include <raptor.h>
#else
#include <Redland/raptor.h>
#endif

#if 0
#include <sys/time.h>
#endif
#if 1
#include <time.h>
#endif

/* Public statics */

/**
 * rasqal_short_copyright_string:
 *
 * Short copyright string (one line).
 */
RASQAL_API
extern const char * const rasqal_short_copyright_string;

/**
 * rasqal_copyright_string:
 *
 * Copyright string (multiple lines).
 */
RASQAL_API
extern const char * const rasqal_copyright_string;

/**
 * rasqal_version_string:
 *
 * Rasqal version as a string.
 */
RASQAL_API
extern const char * const rasqal_version_string;

/**
 * rasqal_version_major:
 *
 * Rasqal major version number.
 */
RASQAL_API
extern const unsigned int rasqal_version_major;

/**
 * rasqal_version_minor:
 *
 * Rasqal minor version number.
 */
RASQAL_API
extern const unsigned int rasqal_version_minor;

/**
 * rasqal_version_release:
 *
 * Rasqal release version number.
 */
RASQAL_API
extern const unsigned int rasqal_version_release;

/**
 * rasqal_version_decimal:
 *
 * Rasqal version as a decimal number.
 *
 * Format: major * 10000 + minor * 100 + release
 */
RASQAL_API
extern const unsigned int rasqal_version_decimal;

/**
 * rasqal_license_string:
 *
 * Rasqal license string.
 */
RASQAL_API
extern const char * const rasqal_license_string;

/**
 * rasqal_home_url_string:
 *
 * Rasqal home page URL.
 */
RASQAL_API
extern const char * const rasqal_home_url_string;



/**
 * RASQAL_RAPTOR_VERSION:
 *
 * Version of Raptor that Rasqal was configured against.
 */
#define RASQAL_RAPTOR_VERSION 0


/* Public structures */

#ifndef RASQAL_WORLD_DECLARED
#define RASQAL_WORLD_DECLARED 1
/**
 * rasqal_world:
 *
 * Rasqal world class.
 */
typedef struct rasqal_world_s rasqal_world;
#endif

/**
 * rasqal_query:
 *
 * Rasqal query class.
 */
typedef struct rasqal_query_s rasqal_query;

/**
 * rasqal_query_results:
 *
 * Rasqal query results class.
 */
typedef struct rasqal_query_results_s rasqal_query_results;


#ifndef RASQAL_QUERY_RESULTS_FORMATTER_DECLARED
#define RASQAL_QUERY_RESULTS_FORMATTER_DECLARED 1
/**
 * rasqal_query_results_formatter:
 *
 * Rasqal query results formatter class.
 */
typedef struct rasqal_query_results_formatter_s rasqal_query_results_formatter;
#endif


typedef struct rasqal_literal_s rasqal_literal;

/**
 * rasqal_graph_pattern:
 *
 * Rasqal graph pattern class.
 */
typedef struct rasqal_graph_pattern_s rasqal_graph_pattern;


/**
 * rasqal_variables_table:
 *
 * Rasqal variables table class.
 */
typedef struct rasqal_variables_table_s rasqal_variables_table;


/**
 * rasqal_feature:
 * @RASQAL_FEATURE_NO_NET: Deny network requests.
 * @RASQAL_FEATURE_RAND_SEED: Set rand() / rand_r() seed
 * @RASQAL_FEATURE_LAST: Internal.
 *
 * Query features.
 *
 * None currently defined.
 */
typedef enum {
  RASQAL_FEATURE_NO_NET,
  RASQAL_FEATURE_RAND_SEED,
  RASQAL_FEATURE_LAST = RASQAL_FEATURE_RAND_SEED
} rasqal_feature;


/**
 * rasqal_prefix:
 * @world: rasqal_world object
 * @prefix: short prefix string
 * @uri: URI associated with the prefix.
 * @declared: Internal flag.
 * @depth: Internal flag.
 *
 * Namespace (prefix, uri) pair.
 *
 * Includes internal flags used for marking when prefixes are
 * declared and at what XML element depth when used in XML formats.
 */
typedef struct {
  rasqal_world* world;
  const unsigned char *prefix;
  raptor_uri* uri;
  int declared;
  int depth;
} rasqal_prefix;


/**
 * rasqal_variable_type:
 * @RASQAL_VARIABLE_TYPE_NORMAL: The regular variable type.
 * @RASQAL_VARIABLE_TYPE_ANONYMOUS: Anonymous variable type.
 * @RASQAL_VARIABLE_TYPE_UNKNOWN: Internal.
 *
 * Rasqal variable types.
 *
 * ANONYMOUS can be used in queries but cannot be returned in a
 * result.
 */
typedef enum {
  RASQAL_VARIABLE_TYPE_UNKNOWN   = 0,
  RASQAL_VARIABLE_TYPE_NORMAL    = 1,
  RASQAL_VARIABLE_TYPE_ANONYMOUS = 2
} rasqal_variable_type;


/* forward reference */
struct rasqal_expression_s;

/**
 * rasqal_variable:
 * @vars_table: variables table that owns this variable
 * @name: Variable name.
 * @value: Variable value or NULL if unbound.
 * @offset: Internal.
 * @type: Variable type.
 * @expression: Expression when the variable is a computed SELECT expression
 * @user_data: Pointer to user data associated with a variable.  This is not used by rasqal.
 * @usage: reference count
 *
 * Binding between a variable name and a value.
 *
 * Includes internal field @offset for recording the offset into the
 * (internal) rasqal_query variables array.
 */
typedef struct {
  rasqal_variables_table* vars_table;
  const unsigned char *name;
  rasqal_literal* value;
  int offset;
  rasqal_variable_type type;
  struct rasqal_expression_s* expression;
  void *user_data;
  int usage;
} rasqal_variable;


/**
 * rasqal_data_graph_flags:
 * @RASQAL_DATA_GRAPH_NONE: Internal.
 * @RASQAL_DATA_GRAPH_NAMED: Graphs with a source and name.
 * @RASQAL_DATA_GRAPH_BACKGROUND: Graphs with a source only.
 *
 * Flags for the type of #rasqal_data_graph.
 *
 * These are used by rasqal_new_data_graph_from_uri() and
 * rasqal_new_data_graph_from_iostream().  See #rasqal_data_graph.
 */
typedef enum {
  RASQAL_DATA_GRAPH_NONE  = 0,
  RASQAL_DATA_GRAPH_NAMED = 1,
  RASQAL_DATA_GRAPH_BACKGROUND = 2,
} rasqal_data_graph_flags;


/**
 * rasqal_data_graph:
 * @world: rasqal_world object
 * @uri: source URI
 * @name_uri: name of graph for %RASQAL_DATA_GRAPH_NAMED
 * @flags: %RASQAL_DATA_GRAPH_NAMED or %RASQAL_DATA_GRAPH_BACKGROUND
 * @format_type: MIME Type of data format at @uri (or NULL)
 * @format_name: Raptor parser Name of data format at @uri (or NULL)
 * @format_uri: URI of data format at @uri (or NULL)
 * @iostr: Raptor iostream for content, overriding @uri if present (or NULL)
 * @base_uri: base URI for reading from iostream
 * @usage: usage count of this object
 *
 * A source of RDF data for querying.
 *
 * If @iostr is present, the graph can be constructed by parsing the
 * iostream and using @base_uri as a base uri. Otherwise the graph
 * can be constructed from the graph at URI @uri.
 *
 * In either case the @name_uri is the graph name as long as @flags
 * is %RASQAL_DATA_GRAPH_NAMED
 */
typedef struct {
  rasqal_world* world;
  raptor_uri* uri;
  raptor_uri* name_uri;
  int flags;
  char* format_type;
  char* format_name;
  raptor_uri* format_uri;
  raptor_iostream* iostr;
  raptor_uri* base_uri;
  int usage;
} rasqal_data_graph;


/**
 * rasqal_literal_type:
 * @RASQAL_LITERAL_BLANK: RDF blank node literal (SPARQL r:bNode)
 * @RASQAL_LITERAL_URI: RDF URI Literal (SPARQL r:URI)
 * @RASQAL_LITERAL_STRING: RDF Plain Literal - no datatype (SPARQL r:Literal)
 * @RASQAL_LITERAL_XSD_STRING: String xsd:string
 * @RASQAL_LITERAL_BOOLEAN: Boolean literal xsd:boolean.
 * @RASQAL_LITERAL_INTEGER: Integer literal xsd:integer.
 * @RASQAL_LITERAL_DOUBLE: Double floating point literal xsd:double.
 * @RASQAL_LITERAL_FLOAT: Floating point literal xsd:float.
 * @RASQAL_LITERAL_DECIMAL: Decimal integer xsd:decimal.
 * @RASQAL_LITERAL_DATETIME: Date/Time literal xsd:dateTime.
 * @RASQAL_LITERAL_UDT: User defined typed literal with unknown datatype URI
 * @RASQAL_LITERAL_PATTERN: Pattern literal for a regex.
 * @RASQAL_LITERAL_QNAME: XML Qname literal.
 * @RASQAL_LITERAL_VARIABLE: Variable literal.
 * @RASQAL_LITERAL_DATE: Date literal xsd:date.
 * @RASQAL_LITERAL_INTEGER_SUBTYPE: Internal.
 * @RASQAL_LITERAL_UNKNOWN: Internal.
 * @RASQAL_LITERAL_FIRST_XSD: Internal.
 * @RASQAL_LITERAL_LAST_XSD: Internal.
 * @RASQAL_LITERAL_LAST: Internal.
 *
 * Types of literal.
 *
 * The order in the enumeration is significant as it encodes
 * the SPARQL term ordering conditions:
 *
 *   Blank Nodes << IRIs << RDF literals << typed literals
 *
 * which coresponds to in enum values
 *
 *   BLANK << URI << STRING <<
 *     (BOOLEAN | INTEGER | DOUBLE | FLOAT | DECIMAL | DATETIME | XSD_STRING)
 *
 *     (RASQAL_LITERAL_FIRST_XSD ... RASQAL_LITERAL_LAST_XSD)
 *
 * Not used (internal): PATTERN, QNAME, VARIABLE
 *
 * See rasqal_literal_compare() when used with flags
 * %RASQAL_COMPARE_XQUERY
 */
typedef enum {
  /* internal */
  RASQAL_LITERAL_UNKNOWN,
  RASQAL_LITERAL_BLANK,
  RASQAL_LITERAL_URI,
  RASQAL_LITERAL_STRING,
  RASQAL_LITERAL_XSD_STRING,
  RASQAL_LITERAL_BOOLEAN,
  RASQAL_LITERAL_INTEGER,
  RASQAL_LITERAL_FLOAT,
  RASQAL_LITERAL_DOUBLE,
  RASQAL_LITERAL_DECIMAL,
  RASQAL_LITERAL_DATETIME,
  /* internal */
  RASQAL_LITERAL_FIRST_XSD = RASQAL_LITERAL_XSD_STRING,
  /* internal */
  RASQAL_LITERAL_LAST_XSD = RASQAL_LITERAL_DATETIME,
  RASQAL_LITERAL_UDT,
  RASQAL_LITERAL_PATTERN,
  RASQAL_LITERAL_QNAME,
  RASQAL_LITERAL_VARIABLE,
  /* internal */
  RASQAL_LITERAL_INTEGER_SUBTYPE,
  RASQAL_LITERAL_DATE,
  /* internal */
  RASQAL_LITERAL_LAST = RASQAL_LITERAL_DATE
} rasqal_literal_type;

#define RASQAL_LITERAL_UDT_DEFINED 1


/**
 * rasqal_row:
 *
 * Rasqal Result Row class.
 */
typedef struct rasqal_row_s rasqal_row;


/**
 * rasqal_xsd_decimal:
 *
 * Rasqal XSD Decimal class.
 */
typedef struct rasqal_xsd_decimal_s rasqal_xsd_decimal;


/**
 * RASQAL_XSD_DATETIME_NO_TZ:
 *
 * Sentinel XSD Decimal timezone value indicating no timezone is present.
 */
#define RASQAL_XSD_DATETIME_NO_TZ (9999)

/**
 * rasqal_xsd_date:
 * @year: year
 * @month: month 1-12
 * @day: 1-31
 * @timezone_minutes: minutes +/- against UTC or RASQAL_XSD_DATETIME_NO_TZ if there is no timezone in the dateTime.
 * @time_on_timeline: time on timeline of first instant of date in timezone
 * @have_tz: timezone flag: 'Z' if Zulu, 'Y' if has other timezone offset in @timezone_minutes, 'N' if there is no timezone
 *
 * XML schema date datatype (xsd:date)
 *
 * Examples of timezone fields:
 *  "2010-01-02" : timezone_minutes RASQAL_XSD_DATETIME_NO_TZ, have_tz 'N'
 *  "2010-01-02Z" : timezone_minutes 0, have_tz 'Z'
 *  "2010-01-02+00:00" : timezone_minutes 0, have_tz 'Y'
 *  "2010-01-02-01:00" : timezone_minutes -60, have_tz 'Y'
 */
typedef struct {
  signed int year;
  /* the following fields are integer values not characters */
  unsigned char month;
  unsigned char day;
  signed short timezone_minutes;
  time_t time_on_timeline;
  char have_tz;
} rasqal_xsd_date;


/**
 * rasqal_xsd_datetime:
 * @year: year
 * @month: month 1-12
 * @day: 1-31
 * @hour: hour 0-23
 * @minute: minute 0-59
 * @second: second 0-60 (yes 60 is allowed for leap seconds)
 * @microseconds: microseconds
 * @timezone_minutes: minutes +/- against UTC or RASQAL_XSD_DATETIME_NO_TZ if there is no timezone in the dateTime.
 * @time_on_timeline: time on timeline
 * @have_tz: timezone flag: 'Z' if Zulu, 'Y' if has other timezone offset in @timezone_minutes, 'N' if there is no timezone
 *
 * XML Schema dateTime datatype (xsd:dateTime)
 *
 * Signed types are required for normalization process where a value
 * can be negative temporarily.
 *
 * Examples of timezone fields:
 *  "2010-01-02T01:02:03" : timezone_minutes RASQAL_XSD_DATETIME_NO_TZ, have_tz 'N'
 *  "2010-01-02T01:02:03Z" : timezone_minutes 0, have_tz 'Z'
 *  "2010-01-02T01:02:03+00:00" : timezone_minutes 0, have_tz 'Y'
 *  "2010-01-02T01:02:03-01:00" : timezone_minutes -60, have_tz 'Y'
 */
typedef struct {
  signed int year;
  unsigned char month;
  unsigned char day;
  /* the following fields are integer values not characters */
  signed char hour;
  signed char minute;
  signed char second;
  signed int microseconds;
  signed short timezone_minutes;
  time_t time_on_timeline;
  char have_tz;
} rasqal_xsd_datetime;


/**
 * rasqal_literal:
 * @world: world object pointer
 * @usage: Usage count.
 * @type: Type of literal.
 * @string: String form of literal for literal types UTF-8 string, pattern, qname, blank, double, float, decimal, datetime.
 * @string_len: Length of @string.
 * @value: Alternate value content.
 * @language: Language for string literal type.
 * @datatype: Datatype for string literal type.
 * @flags: Flags for literal types
 * @parent_type: parent XSD type if any or RASQAL_LITERAL_UNKNOWN
 * @valid: >0 if literal format is a valid lexical form for this datatype. 0 if not valid. <0 if this has not been checked yet
 *
 * Rasqal literal class.
 *
 */
struct rasqal_literal_s {
  rasqal_world *world;

  int usage;

  rasqal_literal_type type;

  /* UTF-8 string, pattern, qname, blank, double, float, decimal, datetime */
  const unsigned char *string;
  unsigned int string_len;

  union {
    /* integer and boolean types */
    int integer;
    /* double and float */
    double floating;
    /* uri (can be temporarily NULL if a qname, see flags below) */
    raptor_uri* uri;
    /* variable */
    rasqal_variable* variable;
    /* decimal */
    rasqal_xsd_decimal* decimal;
    /* datetime  */
    rasqal_xsd_datetime* datetime;
    /* date  */
    rasqal_xsd_date* date;
  } value;

  /* for string */
  const char *language;
  raptor_uri *datatype;

  /* various flags for literal types:
   *  pattern  regex flags
   *  string   datatype of qname
   *  uri      qname of URI not yet expanded (temporary)
   */
  const unsigned char *flags;

  rasqal_literal_type parent_type;

  int valid;
};


/**
 * rasqal_op:
 * @RASQAL_EXPR_AND: Expression for AND(A, B)
 * @RASQAL_EXPR_OR: Expression for OR(A, B)
 * @RASQAL_EXPR_EQ: Expression for A equals B
 * @RASQAL_EXPR_NEQ: Expression for A not equals B.
 * @RASQAL_EXPR_LT: Expression for A less than B.
 * @RASQAL_EXPR_GT: Expression for A greather than B.
 * @RASQAL_EXPR_LE: Expression for A less than or equal to B.
 * @RASQAL_EXPR_GE: Expression for A greater than or equal to B.
 * @RASQAL_EXPR_UMINUS: Expression for -A.
 * @RASQAL_EXPR_PLUS: Expression for +A.
 * @RASQAL_EXPR_MINUS: Expression for A-B.
 * @RASQAL_EXPR_STAR: Expression for A*B.
 * @RASQAL_EXPR_SLASH: Expression for A/B.
 * @RASQAL_EXPR_REM: Expression for A/B remainder.
 * @RASQAL_EXPR_STR_EQ: Expression for A string equals B.
 * @RASQAL_EXPR_STR_NEQ: Expression for A string not-equals B.
 * @RASQAL_EXPR_STR_MATCH: Expression for string A matches literal regex B with flags.
 * @RASQAL_EXPR_STR_NMATCH: Expression for string A not-matches literal regex B with flags.
 * @RASQAL_EXPR_REGEX: Expression for string A matches expression regex B with flags.
 * @RASQAL_EXPR_TILDE: Expression for binary not A.
 * @RASQAL_EXPR_BANG: Expression for logical not A.
 * @RASQAL_EXPR_LITERAL: Expression for a #rasqal_literal.
 * @RASQAL_EXPR_FUNCTION: Expression for a function A with arguments (B...).
 * @RASQAL_EXPR_BOUND: Expression for SPARQL ISBOUND(A).
 * @RASQAL_EXPR_STR: Expression for SPARQL STR(A).
 * @RASQAL_EXPR_LANG: Expression for SPARQL LANG(A).
 * @RASQAL_EXPR_LANGMATCHES: Expression for SPARQL LANGMATCHES(A, B).
 * @RASQAL_EXPR_DATATYPE: Expression for SPARQL DATATYPE(A).
 * @RASQAL_EXPR_ISURI: Expression for SPARQL ISURI(A).
 * @RASQAL_EXPR_ISBLANK: Expression for SPARQL ISBLANK(A).
 * @RASQAL_EXPR_ISLITERAL: Expression for SPARQL ISLITERAL(A).
 * @RASQAL_EXPR_CAST: Expression for cast literal A to type B.
 * @RASQAL_EXPR_ORDER_COND_ASC: Expression for SPARQL order condition ascending.
 * @RASQAL_EXPR_ORDER_COND_DESC: Expression for SPARQL order condition descending.
 * @RASQAL_EXPR_GROUP_COND_ASC: Obsolete - not used
 * @RASQAL_EXPR_GROUP_COND_DESC: Obsolete - not used
 * @RASQAL_EXPR_COUNT: Expression for LAQRS select COUNT() aggregate function
 * @RASQAL_EXPR_VARSTAR: Expression for LAQRS select Variable *
 * @RASQAL_EXPR_SAMETERM: Expression for SPARQL sameTerm
 * @RASQAL_EXPR_SUM: Expression for LAQRS select SUM() aggregate function
 * @RASQAL_EXPR_AVG: Expression for LAQRS select AVG() aggregate function
 * @RASQAL_EXPR_MIN: Expression for LAQRS select MIN() aggregate function
 * @RASQAL_EXPR_MAX: Expression for LAQRS select MAX() aggregate function
 * @RASQAL_EXPR_COALESCE: Expression for LAQRS COALESCE(Expr+)
 * @RASQAL_EXPR_IF: Expression for LAQRS IF(expr, expr, expr)
 * @RASQAL_EXPR_URI: Expression for LAQRS URI(expr)
 * @RASQAL_EXPR_IRI: Expression for LAQRS IRI(expr)
 * @RASQAL_EXPR_STRLANG: Expression for LAQRS STRLANG(expr, expr)
 * @RASQAL_EXPR_STRDT: Expression for LAQRS STRDT(expr, expr)
 * @RASQAL_EXPR_BNODE: Expression for LAQRS BNODE() and BNODE(expr)
 * @RASQAL_EXPR_GROUP_CONCAT: Expression for LAQRS GROUP_CONCAT(arglist) aggregate function
 * @RASQAL_EXPR_SAMPLE: Expression for LAQRS SAMPLE(expr) aggregate function
 * @RASQAL_EXPR_IN: Expression for LAQRS expr IN ( list of expr )
 * @RASQAL_EXPR_NOT_IN: Expression for LAQRS expr NOT IN ( list of expr )
 * @RASQAL_EXPR_ISNUMERIC: Expression for SPARQL 1.1 isNUMERIC(expr)
 * @RASQAL_EXPR_YEAR: Expression for SPARQL 1.1 YEAR(datetime)
 * @RASQAL_EXPR_MONTH: Expression for SPARQL 1.1 MONTH(datetime)
 * @RASQAL_EXPR_DAY: Expression for SPARQL 1.1 DAY(datetime)
 * @RASQAL_EXPR_HOURS: Expression for SPARQL 1.1 HOURS(datetime)
 * @RASQAL_EXPR_MINUTES: Expression for SPARQL 1.1 MINUTES(datetime)
 * @RASQAL_EXPR_SECONDS: Expression for SPARQL 1.1 SECONDS(datetime)
 * @RASQAL_EXPR_TIMEZONE: Expression for SPARQL 1.1 TIMEZONE(datetime)
 * @RASQAL_EXPR_CURRENT_DATETIME: Expression for LAQRS CURRENT_DATETIME( void )
 * @RASQAL_EXPR_NOW: Expression for LAQRS NOW( void )
 * @RASQAL_EXPR_FROM_UNIXTIME: Expression for LAQRS FROM_UNIXTIME(int)
 * @RASQAL_EXPR_TO_UNIXTIME: Expression for LAQRS TO_UNIXTIME(datetime)
 * @RASQAL_EXPR_CONCAT: Expression for SPARQL 1.1 CONCAT(strings)
 * @RASQAL_EXPR_STRLEN: Expression for SPARQL 1.1 STRLEN(str)
 * @RASQAL_EXPR_SUBSTR: Expression for SPARQL 1.1 SUBSTR(str, start[,offset])
 * @RASQAL_EXPR_UCASE: Expression for SPARQL 1.1 UCASE(str)
 * @RASQAL_EXPR_LCASE: Expression for SPARQL 1.1 LCASE(str)
 * @RASQAL_EXPR_STRSTARTS: Expression for SPARQL 1.1 STRSTARTS(str, str)
 * @RASQAL_EXPR_STRENDS: Expression for SPARQL 1.1 STRENDS(str, str)
 * @RASQAL_EXPR_CONTAINS: Expression for SPARQL 1.1 CONTAINS(str, str)
 * @RASQAL_EXPR_ENCODE_FOR_URI: Expression for SPARQL 1.1 ENCODE_FOR_URI(str)
 * @RASQAL_EXPR_TZ: Expression for SPARQL 1.1 TZ()
 * @RASQAL_EXPR_RAND: Expression for SPARQL 1.1 RAND()
 * @RASQAL_EXPR_ABS: Expression for SPARQL 1.1 ABS()
 * @RASQAL_EXPR_ROUND: Expression for SPARQL 1.1 ROUND()
 * @RASQAL_EXPR_CEIL: Expression for SPARQL 1.1 CEIL()
 * @RASQAL_EXPR_FLOOR: Expression for SPARQL 1.1 FLOOR()
 * @RASQAL_EXPR_MD5: Expression for SPARQL 1.1 MD5()
 * @RASQAL_EXPR_SHA1: Expression for SPARQL 1.1 SHA1()
 * @RASQAL_EXPR_SHA224: Expression for SPARQL 1.1 SHA224()
 * @RASQAL_EXPR_SHA256: Expression for SPARQL 1.1 SHA256()
 * @RASQAL_EXPR_SHA384: Expression for SPARQL 1.1 SHA384()
 * @RASQAL_EXPR_SHA512: Expression for SPARQL 1.1 SHA512()
 * @RASQAL_EXPR_STRBEFORE: Expression for SPARQL 1.1 STRBEFORE()
 * @RASQAL_EXPR_STRAFTER: Expression for SPARQL 1.1 STRAFTER()
 * @RASQAL_EXPR_REPLACE: Expression for SPARQL 1.1 REPLACE()
 * @RASQAL_EXPR_UUID: Expression for SPARQL 1.1 UUID()
 * @RASQAL_EXPR_STRUUID: Expression for SPARQL 1.1 STRUUID()
 * @RASQAL_EXPR_UNKNOWN: Internal
 * @RASQAL_EXPR_LAST: Internal
 *
 * Rasqal expression operators.  A mixture of unary, binary and
 * tertiary operators (string matches).  Also includes casting and
 * two ordering operators from ORDER BY in SPARQL.
 */
typedef enum {
  /* internal */
  RASQAL_EXPR_UNKNOWN,
  RASQAL_EXPR_AND,
  RASQAL_EXPR_OR,
  RASQAL_EXPR_EQ,
  RASQAL_EXPR_NEQ,
  RASQAL_EXPR_LT,
  RASQAL_EXPR_GT,
  RASQAL_EXPR_LE,
  RASQAL_EXPR_GE,
  RASQAL_EXPR_UMINUS,
  RASQAL_EXPR_PLUS,
  RASQAL_EXPR_MINUS,
  RASQAL_EXPR_STAR,
  RASQAL_EXPR_SLASH,
  RASQAL_EXPR_REM,
  RASQAL_EXPR_STR_EQ,
  RASQAL_EXPR_STR_NEQ,
  RASQAL_EXPR_STR_MATCH,
  RASQAL_EXPR_STR_NMATCH,
  RASQAL_EXPR_TILDE,
  RASQAL_EXPR_BANG,
  RASQAL_EXPR_LITERAL,
  RASQAL_EXPR_FUNCTION,
  RASQAL_EXPR_BOUND,
  RASQAL_EXPR_STR,
  RASQAL_EXPR_LANG,
  RASQAL_EXPR_DATATYPE,
  RASQAL_EXPR_ISURI,
  RASQAL_EXPR_ISBLANK,
  RASQAL_EXPR_ISLITERAL,
  RASQAL_EXPR_CAST,
  RASQAL_EXPR_ORDER_COND_ASC,
  RASQAL_EXPR_ORDER_COND_DESC,
  RASQAL_EXPR_LANGMATCHES,
  RASQAL_EXPR_REGEX,
  RASQAL_EXPR_GROUP_COND_ASC,
  RASQAL_EXPR_GROUP_COND_DESC,
  RASQAL_EXPR_COUNT,
  RASQAL_EXPR_VARSTAR,
  RASQAL_EXPR_SAMETERM,
  RASQAL_EXPR_SUM,
  RASQAL_EXPR_AVG,
  RASQAL_EXPR_MIN,
  RASQAL_EXPR_MAX,
  RASQAL_EXPR_COALESCE,
  RASQAL_EXPR_IF,
  RASQAL_EXPR_URI,
  RASQAL_EXPR_IRI,
  RASQAL_EXPR_STRLANG,
  RASQAL_EXPR_STRDT,
  RASQAL_EXPR_BNODE,
  RASQAL_EXPR_GROUP_CONCAT,
  RASQAL_EXPR_SAMPLE,
  RASQAL_EXPR_IN,
  RASQAL_EXPR_NOT_IN,
  RASQAL_EXPR_ISNUMERIC,
  RASQAL_EXPR_YEAR,
  RASQAL_EXPR_MONTH,
  RASQAL_EXPR_DAY,
  RASQAL_EXPR_HOURS,
  RASQAL_EXPR_MINUTES,
  RASQAL_EXPR_SECONDS,
  RASQAL_EXPR_TIMEZONE,
  RASQAL_EXPR_CURRENT_DATETIME,
  RASQAL_EXPR_NOW,
  RASQAL_EXPR_FROM_UNIXTIME,
  RASQAL_EXPR_TO_UNIXTIME,
  RASQAL_EXPR_CONCAT,
  RASQAL_EXPR_STRLEN,
  RASQAL_EXPR_SUBSTR,
  RASQAL_EXPR_UCASE,
  RASQAL_EXPR_LCASE,
  RASQAL_EXPR_STRSTARTS,
  RASQAL_EXPR_STRENDS,
  RASQAL_EXPR_CONTAINS,
  RASQAL_EXPR_ENCODE_FOR_URI,
  RASQAL_EXPR_TZ,
  RASQAL_EXPR_RAND,
  RASQAL_EXPR_ABS,
  RASQAL_EXPR_ROUND,
  RASQAL_EXPR_CEIL,
  RASQAL_EXPR_FLOOR,
  RASQAL_EXPR_MD5,
  RASQAL_EXPR_SHA1,
  RASQAL_EXPR_SHA224,
  RASQAL_EXPR_SHA256,
  RASQAL_EXPR_SHA384,
  RASQAL_EXPR_SHA512,
  RASQAL_EXPR_STRBEFORE,
  RASQAL_EXPR_STRAFTER,
  RASQAL_EXPR_REPLACE,
  RASQAL_EXPR_UUID,
  RASQAL_EXPR_STRUUID,
  /* internal */
  RASQAL_EXPR_LAST = RASQAL_EXPR_STRUUID
} rasqal_op;


/**
 * rasqal_expression_flags:
 * @RASQAL_EXPR_FLAG_DISTINCT: Distinct
 * @RASQAL_EXPR_FLAG_AGGREGATE: Aggregate function expression
 *
 * Flags for expressions.
 */
typedef enum {
  RASQAL_EXPR_FLAG_DISTINCT = 1,
  RASQAL_EXPR_FLAG_AGGREGATE = 2
} rasqal_expression_flags;


/**
 * rasqal_expression:
 * @world: rasqal_world object
 * @usage: reference count - 1 for itself
 * @op: expression operation
 * @arg1: first argument
 * @arg2: second argument
 * @arg3: third argument (for #RASQAL_EXPR_REGEX )
 * @literal: literal argument
 * @value: UTF-8 value
 * @name: name for extension function qname(args...) and cast-to-uri
 * @args: args for extension function qname(args...), cast-to-uri and COALESCE
 * @params: args for extension function parameters (SPARQL 1.1) (Rasqal 0.9.20+)
 * @flags: bitflags from #rasqal_expression_flags for expressions (Rasqal 0.9.20+)
 *
 * Expression with arguments
 *
 */
struct rasqal_expression_s {
  rasqal_world* world;

  int usage;

  rasqal_op op;

  struct rasqal_expression_s* arg1;
  struct rasqal_expression_s* arg2;
  struct rasqal_expression_s* arg3;
  rasqal_literal* literal;
  unsigned char *value;

  raptor_uri* name;
  raptor_sequence* args;

  raptor_sequence* params;
  unsigned int flags;
  struct rasqal_expression_s* arg4;
};
typedef struct rasqal_expression_s rasqal_expression;


/**
 * rasqal_triple:
 * @subject: Triple subject.
 * @predicate: Triple predicate.
 * @object: Triple object.
 * @origin: Triple origin.
 * @flags: Or of enum #rasqal_triple_flags bits.
 *
 * A triple pattern or RDF triple.
 *
 * This is used as a triple pattern in queries and
 * an RDF triple when generating RDF triples such as with SPARQL CONSTRUCT.
 */
typedef struct {
  rasqal_literal* subject;
  rasqal_literal* predicate;
  rasqal_literal* object;
  rasqal_literal* origin;
  unsigned int flags;
} rasqal_triple;


/**
 * rasqal_pattern_flags:
 * @RASQAL_PATTERN_FLAGS_OPTIONAL: True when the graph pattern is an optional match.
 * @RASQAL_PATTERN_FLAGS_LAST: Internal
 *
 * Flags for #rasqal_graph_pattern.
 */
typedef enum {
  RASQAL_PATTERN_FLAGS_OPTIONAL = 1,

  RASQAL_PATTERN_FLAGS_LAST     = RASQAL_PATTERN_FLAGS_OPTIONAL
} rasqal_pattern_flags;


/**
 * rasqal_generate_bnodeid_handler:
 * @world: world arg
 * @user_data: user data given to
 * @user_bnodeid: user blank node ID string passed in
 *
 * User handler used with rasqal_world_set_generate_bnodeid_handler() to set method for generating a blank node ID.
 *
 * Return value: blank node ID string or NULL on failure.
 */
typedef unsigned char* (*rasqal_generate_bnodeid_handler)(rasqal_world* world, void *user_data, unsigned char *user_bnodeid);


/**
 * rasqal_query_verb:
 * @RASQAL_QUERY_VERB_SELECT: SPARQL query select verb.
 * @RASQAL_QUERY_VERB_CONSTRUCT: SPARQL query construct verb.
 * @RASQAL_QUERY_VERB_DESCRIBE: SPARQL query describe verb.
 * @RASQAL_QUERY_VERB_ASK: SPARQL query ask verb.
 * @RASQAL_QUERY_VERB_DELETE: LAQRS query delete verb.
 * @RASQAL_QUERY_VERB_INSERT: LAQRS query insert verb.
 * @RASQAL_QUERY_VERB_UPDATE: SPARQL 1.1 (draft) update operation
 * @RASQAL_QUERY_VERB_UNKNOWN: Internal
 * @RASQAL_QUERY_VERB_LAST: Internal
 *
 * Query main operation verbs describing the major type of query
 * being performed.
 */
typedef enum {
  /* internal */
  RASQAL_QUERY_VERB_UNKNOWN   = 0,
  RASQAL_QUERY_VERB_SELECT    = 1,
  RASQAL_QUERY_VERB_CONSTRUCT = 2,
  RASQAL_QUERY_VERB_DESCRIBE  = 3,
  RASQAL_QUERY_VERB_ASK       = 4,
  RASQAL_QUERY_VERB_DELETE    = 5,
  RASQAL_QUERY_VERB_INSERT    = 6,
  RASQAL_QUERY_VERB_UPDATE    = 7,

  /* internal */
  RASQAL_QUERY_VERB_LAST = RASQAL_QUERY_VERB_UPDATE
} rasqal_query_verb;


/**
 * rasqal_query_results_type:
 * @RASQAL_QUERY_RESULTS_BINDINGS: variable binding
 * @RASQAL_QUERY_RESULTS_BOOLEAN: a single boolean
 * @RASQAL_QUERY_RESULTS_GRAPH: an RDF graph
 * @RASQAL_QUERY_RESULTS_SYNTAX: a syntax
 * @RASQAL_QUERY_RESULTS_UNKNOWN: unknown type
 *
 * Query result type.
 */

typedef enum {
  RASQAL_QUERY_RESULTS_BINDINGS,
  RASQAL_QUERY_RESULTS_BOOLEAN,
  RASQAL_QUERY_RESULTS_GRAPH,
  RASQAL_QUERY_RESULTS_SYNTAX,
  RASQAL_QUERY_RESULTS_UNKNOWN
} rasqal_query_results_type;


/**
 * rasqal_update_type:
 * @RASQAL_UPDATE_TYPE_CLEAR: Clear graph.
 * @RASQAL_UPDATE_TYPE_CREATE: Create graph.
 * @RASQAL_UPDATE_TYPE_DROP: Drop graph.
 * @RASQAL_UPDATE_TYPE_LOAD: Load graph.
 * @RASQAL_UPDATE_TYPE_UPDATE: Insert or Delete graph or triples.
 * @RASQAL_UPDATE_TYPE_ADD: Add graph to another graph.
 * @RASQAL_UPDATE_TYPE_MOVE: Move graph to another graph.
 * @RASQAL_UPDATE_TYPE_COPY: Copy graph to another graph.
 * @RASQAL_UPDATE_TYPE_UNKNOWN: Internal
 * @RASQAL_UPDATE_TYPE_LAST: Internal
 *
 * Update type being performed.
 *
 */
typedef enum {
  /* internal */
  RASQAL_UPDATE_TYPE_UNKNOWN   = 0,
  RASQAL_UPDATE_TYPE_CLEAR     = 1,
  RASQAL_UPDATE_TYPE_CREATE    = 2,
  RASQAL_UPDATE_TYPE_DROP      = 3,
  RASQAL_UPDATE_TYPE_LOAD      = 4,
  RASQAL_UPDATE_TYPE_UPDATE    = 5,
  RASQAL_UPDATE_TYPE_ADD       = 6,
  RASQAL_UPDATE_TYPE_MOVE      = 7,
  RASQAL_UPDATE_TYPE_COPY      = 8,

  /* internal */
  RASQAL_UPDATE_TYPE_LAST = RASQAL_UPDATE_TYPE_COPY
} rasqal_update_type;


/**
 * rasqal_update_flags:
 * @RASQAL_UPDATE_FLAGS_SILENT: the update operation should be silent
 * @RASQAL_UPDATE_FLAGS_DATA: the update operation is triple data not templates
 *
 * Bitflags for graph update operations
 */
typedef enum {
  RASQAL_UPDATE_FLAGS_SILENT = 1,
  RASQAL_UPDATE_FLAGS_DATA = 2
} rasqal_update_flags;


/**
 * rasqal_update_graph_applies:
 * @RASQAL_UPDATE_GRAPH_ONE: the update operation applies to 1 graph
 * @RASQAL_UPDATE_GRAPH_DEFAULT: the update operation applies to the default graph
 * @RASQAL_UPDATE_GRAPH_NAMED: the update operation applies to all named graphs
 * @RASQAL_UPDATE_GRAPH_ALL: the update operation applies ALL graphs
 *
 * The graph(s) that the update operation applies to.
 */
typedef enum {
  RASQAL_UPDATE_GRAPH_ONE = 0,
  RASQAL_UPDATE_GRAPH_DEFAULT = 1,
  RASQAL_UPDATE_GRAPH_NAMED = 2,
  RASQAL_UPDATE_GRAPH_ALL = 3
} rasqal_update_graph_applies;


/**
 * rasqal_update_operation:
 * @type: type of update
 * @graph_uri: optional graph URI (clear, drop, load, with ... delete, insert); source graph (add, move, copy)
 * @document_uri: optional document URI (load); destination graph (add, move, copy)
 * @insert_templates: optional sequence of #rasqal_triple to insert. Data triples if @flags is #RASQAL_UPDATE_FLAGS_DATA set, templates otherwise.
 * @delete_templates: optional sequence of #rasqal_triple templates to delete
 * @where: optional where template (insert/delete)
 * @flags: update flags - bit-or of flags defined in #rasqal_update_flags
 * @applies: the graph(s) that the update operation applies to, or @graph_uri if #RASQAL_UPDATE_GRAPH_ONE
 *
 * Update operation - changing the dataset
 *
 * For LOAD and CLEAR if @applies is set (not 0) then the operation
 * applies to just those graph(), otherwise it applies to the @graph_uri.
 *
 * For ADD, MOVE and COPY the source graph is stored in @graph_uri
 * field and the destination graph in the @document_uri field.  The
 * field names have no meaning in this case since both values are
 * always present, always graphs and a NULL value signifies the
 * default graph.
 *
 */
typedef struct {
  rasqal_update_type type;

  raptor_uri* graph_uri;

  raptor_uri* document_uri;

  raptor_sequence* insert_templates;

  raptor_sequence* delete_templates;

  rasqal_graph_pattern* where;

  int flags;

  rasqal_update_graph_applies applies;
} rasqal_update_operation;


/**
 * rasqal_graph_pattern_operator:
 * @RASQAL_GRAPH_PATTERN_OPERATOR_BASIC: Just triple patterns and constraints.
 * @RASQAL_GRAPH_PATTERN_OPERATOR_OPTIONAL: Set of graph patterns (ANDed) and constraints.
 * @RASQAL_GRAPH_PATTERN_OPERATOR_UNION: Set of graph patterns (UNIONed) and constraints.
 * @RASQAL_GRAPH_PATTERN_OPERATOR_GROUP: Set of graph patterns (ANDed) and constraints.
 * @RASQAL_GRAPH_PATTERN_OPERATOR_GRAPH: A graph term + a graph pattern and constraints.
 * @RASQAL_GRAPH_PATTERN_OPERATOR_FILTER: A filter graph pattern with an expression
 * @RASQAL_GRAPH_PATTERN_OPERATOR_LET: LET ?var := Expression (LAQRS)
 * @RASQAL_GRAPH_PATTERN_OPERATOR_SELECT: SELECT graph pattern
 * @RASQAL_GRAPH_PATTERN_OPERATOR_SERVICE: SERVICE graph pattern
 * @RASQAL_GRAPH_PATTERN_OPERATOR_MINUS: MINUS graph pattern
 * @RASQAL_GRAPH_PATTERN_OPERATOR_UNKNOWN: Internal.
 * @RASQAL_GRAPH_PATTERN_OPERATOR_LAST: Internal.
 *
 * Graph pattern operators
 */
typedef enum {
  RASQAL_GRAPH_PATTERN_OPERATOR_UNKNOWN   = 0,
  RASQAL_GRAPH_PATTERN_OPERATOR_BASIC     = 1,
  RASQAL_GRAPH_PATTERN_OPERATOR_OPTIONAL  = 2,
  RASQAL_GRAPH_PATTERN_OPERATOR_UNION     = 3,
  RASQAL_GRAPH_PATTERN_OPERATOR_GROUP     = 4,
  RASQAL_GRAPH_PATTERN_OPERATOR_GRAPH     = 5,
  RASQAL_GRAPH_PATTERN_OPERATOR_FILTER    = 6,
  RASQAL_GRAPH_PATTERN_OPERATOR_LET       = 7,
  RASQAL_GRAPH_PATTERN_OPERATOR_SELECT    = 8,
  RASQAL_GRAPH_PATTERN_OPERATOR_SERVICE   = 9,
  RASQAL_GRAPH_PATTERN_OPERATOR_MINUS     = 10,

  RASQAL_GRAPH_PATTERN_OPERATOR_LAST = RASQAL_GRAPH_PATTERN_OPERATOR_MINUS
} rasqal_graph_pattern_operator;


/**
 * rasqal_graph_pattern_visit_fn:
 * @query: #rasqal_query containing the graph pattern
 * @gp: current graph_pattern
 * @user_data: user data passed in
 *
 * User function to visit an graph_pattern and operate on it with
 * rasqal_graph_pattern_visit() or rasqal_query_graph_pattern_visit()
 *
 * Return value: non-0 to truncate the visit
 */
typedef int (*rasqal_graph_pattern_visit_fn)(rasqal_query* query, rasqal_graph_pattern* gp, void *user_data);


/* RASQAL API */

/* Public functions */

RASQAL_API
rasqal_world *rasqal_new_world(void);
RASQAL_API
int rasqal_world_open(rasqal_world* world);
RASQAL_API
void rasqal_free_world(rasqal_world* world);

RASQAL_API
void rasqal_world_set_raptor(rasqal_world* world, raptor_world* raptor_world_ptr);
RASQAL_API
raptor_world *rasqal_world_get_raptor(rasqal_world* world);

RASQAL_API
void rasqal_world_set_log_handler(rasqal_world* world, void *user_data, raptor_log_handler handler);

RASQAL_API
int rasqal_world_set_default_generate_bnodeid_parameters(rasqal_world* world, char *prefix, int base);
RASQAL_API
int rasqal_world_set_generate_bnodeid_handler(rasqal_world* world, void *user_data, rasqal_generate_bnodeid_handler handler);

RASQAL_API
int rasqal_world_set_warning_level(rasqal_world* world, unsigned int warning_level);

RASQAL_API
const raptor_syntax_description* rasqal_world_get_query_results_format_description(rasqal_world* world, unsigned int counter);

RASQAL_API
const char* rasqal_world_guess_query_results_format_name(rasqal_world* world, raptor_uri *uri, const char *mime_type, const unsigned char *buffer, size_t len, const unsigned char *identifier);

/* Features */
RASQAL_API
int rasqal_features_enumerate(rasqal_world* world, const rasqal_feature feature, const char **name, raptor_uri **uri, const char **label);
RASQAL_API
unsigned int rasqal_get_feature_count(void);
RASQAL_API
rasqal_feature rasqal_feature_from_uri(rasqal_world* world, raptor_uri *uri);
RASQAL_API
int rasqal_feature_value_type(const rasqal_feature feature);


RASQAL_API
const raptor_syntax_description* rasqal_world_get_query_language_description(rasqal_world* world, unsigned int counter);

RASQAL_API RASQAL_DEPRECATED
int rasqal_languages_enumerate(rasqal_world* world, unsigned int counter, const char **name, const char **label, const unsigned char **uri_string);

RASQAL_API
int rasqal_language_name_check(rasqal_world* world, const char *name);


/* Query class */

/* Create */
RASQAL_API
rasqal_query* rasqal_new_query(rasqal_world* world, const char *name, const unsigned char *uri);

/* Destroy */
RASQAL_API
void rasqal_free_query(rasqal_query* query);

/* Methods */
RASQAL_API
const char* rasqal_query_get_name(rasqal_query* query);
RASQAL_API
const char* rasqal_query_get_label(rasqal_query* query);


RASQAL_API
int rasqal_query_set_feature(rasqal_query* query, rasqal_feature feature, int value);
RASQAL_API
int rasqal_query_set_feature_string(rasqal_query *query, rasqal_feature feature, const unsigned char *value);
RASQAL_API
int rasqal_query_get_feature(rasqal_query *query, rasqal_feature feature);
RASQAL_API
const unsigned char* rasqal_query_get_feature_string(rasqal_query *query, rasqal_feature feature);

RASQAL_API
rasqal_query_verb rasqal_query_get_verb(rasqal_query* query);
RASQAL_API
int rasqal_query_get_wildcard(rasqal_query* query);
RASQAL_API
void rasqal_query_set_wildcard(rasqal_query* query, int wildcard);
RASQAL_API
int rasqal_query_get_distinct(rasqal_query* query);
RASQAL_API
void rasqal_query_set_distinct(rasqal_query* query, int distinct_mode);
RASQAL_API
int rasqal_query_get_explain(rasqal_query* query);
RASQAL_API
void rasqal_query_set_explain(rasqal_query* query, int is_explain);
RASQAL_API
int rasqal_query_get_limit(rasqal_query* query);
RASQAL_API
void rasqal_query_set_limit(rasqal_query* query, int limit);
RASQAL_API
int rasqal_query_get_offset(rasqal_query* query);
RASQAL_API
void rasqal_query_set_offset(rasqal_query* query, int offset);

RASQAL_API
int rasqal_query_add_data_graph(rasqal_query* query, rasqal_data_graph* data_graph);
RASQAL_API
int rasqal_query_add_data_graphs(rasqal_query* query, raptor_sequence* data_graphs);

RASQAL_API
raptor_sequence* rasqal_query_get_data_graph_sequence(rasqal_query* query);
RASQAL_API
rasqal_data_graph* rasqal_query_get_data_graph(rasqal_query* query, int idx);
RASQAL_API
int rasqal_query_dataset_contains_named_graph(rasqal_query* query, raptor_uri *graph_uri);

RASQAL_API
int rasqal_query_add_variable(rasqal_query* query, rasqal_variable* var);
RASQAL_API
raptor_sequence* rasqal_query_get_bound_variable_sequence(rasqal_query* query);
RASQAL_API
raptor_sequence* rasqal_query_get_describe_sequence(rasqal_query* query);
RASQAL_API
raptor_sequence* rasqal_query_get_anonymous_variable_sequence(rasqal_query* query);
RASQAL_API
raptor_sequence* rasqal_query_get_all_variable_sequence(rasqal_query* query);
RASQAL_API
rasqal_variable* rasqal_query_get_variable(rasqal_query* query, int idx);
RASQAL_API
int rasqal_query_has_variable2(rasqal_query* query, rasqal_variable_type type, const unsigned char *name);
RASQAL_API RASQAL_DEPRECATED
int rasqal_query_has_variable(rasqal_query* query, const unsigned char *name);
RASQAL_API
int rasqal_query_set_variable2(rasqal_query* query, rasqal_variable_type type, const unsigned char *name, rasqal_literal* value);
RASQAL_API RASQAL_DEPRECATED
int rasqal_query_set_variable(rasqal_query* query, const unsigned char *name, rasqal_literal* value);
RASQAL_API
raptor_sequence* rasqal_query_get_triple_sequence(rasqal_query* query);
RASQAL_API
rasqal_triple* rasqal_query_get_triple(rasqal_query* query, int idx);
RASQAL_API
int rasqal_query_add_prefix(rasqal_query* query, rasqal_prefix* prefix);
RASQAL_API
raptor_sequence* rasqal_query_get_prefix_sequence(rasqal_query* query);
RASQAL_API
rasqal_prefix* rasqal_query_get_prefix(rasqal_query* query, int idx);
RASQAL_API
raptor_sequence* rasqal_query_get_order_conditions_sequence(rasqal_query* query);
RASQAL_API
rasqal_expression* rasqal_query_get_order_condition(rasqal_query* query, int idx);
RASQAL_API
raptor_sequence* rasqal_query_get_group_conditions_sequence(rasqal_query* query);
RASQAL_API
rasqal_expression* rasqal_query_get_group_condition(rasqal_query* query, int idx);
RASQAL_API
raptor_sequence* rasqal_query_get_having_conditions_sequence(rasqal_query* query);
RASQAL_API
rasqal_expression* rasqal_query_get_having_condition(rasqal_query* query, int idx);
RASQAL_API
raptor_sequence* rasqal_query_get_construct_triples_sequence(rasqal_query* query);
RASQAL_API
rasqal_triple* rasqal_query_get_construct_triple(rasqal_query* query, int idx);
RASQAL_API RASQAL_DEPRECATED
void rasqal_query_graph_pattern_visit(rasqal_query* query, rasqal_graph_pattern_visit_fn visit_fn, void* data);
RASQAL_API
int rasqal_query_graph_pattern_visit2(rasqal_query* query, rasqal_graph_pattern_visit_fn visit_fn, void* data);
RASQAL_API
int rasqal_query_write(raptor_iostream* iostr, rasqal_query* query, raptor_uri* format_uri, raptor_uri* base_uri);

/* update */
RASQAL_API
raptor_sequence* rasqal_query_get_update_operations_sequence(rasqal_query* query);
RASQAL_API
rasqal_update_operation* rasqal_query_get_update_operation(rasqal_query* query, int idx);

/* results */
int rasqal_query_set_store_results(rasqal_query* query, int store_results);

/* graph patterns */
RASQAL_API
rasqal_graph_pattern* rasqal_query_get_query_graph_pattern(rasqal_query* query);
RASQAL_API
raptor_sequence* rasqal_query_get_graph_pattern_sequence(rasqal_query* query);
RASQAL_API
rasqal_graph_pattern* rasqal_query_get_graph_pattern(rasqal_query* query, int idx);
RASQAL_API
int rasqal_graph_pattern_add_sub_graph_pattern(rasqal_graph_pattern* graph_pattern, rasqal_graph_pattern* sub_graph_pattern);
RASQAL_API
rasqal_triple* rasqal_graph_pattern_get_triple(rasqal_graph_pattern* graph_pattern, int idx);
RASQAL_API
raptor_sequence* rasqal_graph_pattern_get_sub_graph_pattern_sequence(rasqal_graph_pattern* graph_pattern);
RASQAL_API
rasqal_graph_pattern* rasqal_graph_pattern_get_sub_graph_pattern(rasqal_graph_pattern* graph_pattern, int idx);
RASQAL_API
rasqal_graph_pattern_operator rasqal_graph_pattern_get_operator(rasqal_graph_pattern* graph_pattern);
RASQAL_API
const char* rasqal_graph_pattern_operator_as_string(rasqal_graph_pattern_operator op);
RASQAL_API
int rasqal_graph_pattern_print(rasqal_graph_pattern* gp, FILE* fh);
RASQAL_API
int rasqal_graph_pattern_set_filter_expression(rasqal_graph_pattern* gp, rasqal_expression* expr);
RASQAL_API
rasqal_expression* rasqal_graph_pattern_get_filter_expression(rasqal_graph_pattern* gp);
RASQAL_API
int rasqal_graph_pattern_visit(rasqal_query* query, rasqal_graph_pattern *gp, rasqal_graph_pattern_visit_fn fn, void* user_data);
RASQAL_API
int rasqal_graph_pattern_get_index(rasqal_graph_pattern* gp);
RASQAL_API
int rasqal_graph_pattern_variable_bound_in(rasqal_graph_pattern *gp, rasqal_variable *v);
RASQAL_API
rasqal_literal* rasqal_graph_pattern_get_origin(rasqal_graph_pattern* graph_pattern);
RASQAL_API
rasqal_variable* rasqal_graph_pattern_get_variable(rasqal_graph_pattern* graph_pattern);
RASQAL_API
rasqal_literal* rasqal_graph_pattern_get_service(rasqal_graph_pattern* graph_pattern);
RASQAL_API
raptor_sequence* rasqal_graph_pattern_get_flattened_triples(rasqal_query* query, rasqal_graph_pattern* graph_pattern);

/* Utility methods */
RASQAL_API
const char* rasqal_query_verb_as_string(rasqal_query_verb verb);
RASQAL_API
int rasqal_query_print(rasqal_query* query, FILE* fh);

/* Query */
RASQAL_API
int rasqal_query_prepare(rasqal_query* query, const unsigned char *query_string, raptor_uri *base_uri);
RASQAL_API
rasqal_query_results* rasqal_query_execute(rasqal_query* query);

RASQAL_API
void* rasqal_query_get_user_data(rasqal_query* query);
RASQAL_API
void rasqal_query_set_user_data(rasqal_query* query, void *user_data);

RASQAL_API
raptor_sequence* rasqal_query_get_bindings_variables_sequence(rasqal_query* query);
RASQAL_API
rasqal_variable* rasqal_query_get_bindings_variable(rasqal_query* query, int idx);
RASQAL_API
raptor_sequence* rasqal_query_get_bindings_rows_sequence(rasqal_query* query);
RASQAL_API
rasqal_row* rasqal_query_get_bindings_row(rasqal_query* query, int idx);
RASQAL_API
rasqal_query_results_type rasqal_query_get_result_type(rasqal_query* query);

/* query results */
RASQAL_API
rasqal_query_results* rasqal_new_query_results(rasqal_world* world, rasqal_query* query, rasqal_query_results_type type, rasqal_variables_table* vars_table);
RASQAL_API
void rasqal_free_query_results(rasqal_query_results *query_results);

RASQAL_API
rasqal_query* rasqal_query_results_get_query(rasqal_query_results* query_results);

/* Bindings result format */
RASQAL_API
rasqal_query_results_type rasqal_query_results_get_type(rasqal_query_results* query_results);
RASQAL_API
int rasqal_query_results_is_bindings(rasqal_query_results *query_results);
RASQAL_API
int rasqal_query_results_get_count(rasqal_query_results *query_results);
RASQAL_API
int rasqal_query_results_next(rasqal_query_results *query_results);
RASQAL_API
int rasqal_query_results_finished(rasqal_query_results *query_results);
RASQAL_API
int rasqal_query_results_get_bindings(rasqal_query_results *query_results, const unsigned char ***names, rasqal_literal ***values);
RASQAL_API
rasqal_literal* rasqal_query_results_get_binding_value(rasqal_query_results *query_results, int offset);
RASQAL_API
const unsigned char* rasqal_query_results_get_binding_name(rasqal_query_results *query_results, int offset);
RASQAL_API
rasqal_literal* rasqal_query_results_get_binding_value_by_name(rasqal_query_results *query_results, const unsigned char *name);
RASQAL_API
int rasqal_query_results_get_bindings_count(rasqal_query_results *query_results);
RASQAL_API
int rasqal_query_results_add_row(rasqal_query_results* query_results, rasqal_row* row);
RASQAL_API
rasqal_row* rasqal_query_results_get_row_by_offset(rasqal_query_results* query_results, int result_offset);

/* Boolean result format */
RASQAL_API
int rasqal_query_results_is_boolean(rasqal_query_results *query_results);
RASQAL_API
int rasqal_query_results_get_boolean(rasqal_query_results *query_results);

/* Graph result format */
RASQAL_API
int rasqal_query_results_is_graph(rasqal_query_results *query_results);
RASQAL_API
raptor_statement* rasqal_query_results_get_triple(rasqal_query_results *query_results);
RASQAL_API
int rasqal_query_results_next_triple(rasqal_query_results *query_results);

/* Syntax result format */
RASQAL_API
int rasqal_query_results_is_syntax(rasqal_query_results* query_results);

RASQAL_API
int rasqal_query_results_write(raptor_iostream *iostr, rasqal_query_results *results, const char* name, const char* mime_type, raptor_uri *format_uri, raptor_uri *base_uri);
RASQAL_API
int rasqal_query_results_read(raptor_iostream *iostr, rasqal_query_results *results, const char* name, const char* mime_type, raptor_uri *format_uri, raptor_uri *base_uri);

/* One more time */
RASQAL_API
int rasqal_query_results_rewind(rasqal_query_results* query_results);


/**
 * rasqal_query_results_format_flags:
 * @RASQAL_QUERY_RESULTS_FORMAT_FLAG_READER: format can be read.
 * @RASQAL_QUERY_RESULTS_FORMAT_FLAG_WRITER: format can be written.
 *
 * Bitflags for rasqal_query_results_formats_check() to find formats with features.
 */
typedef enum {
  RASQAL_QUERY_RESULTS_FORMAT_FLAG_READER = 1,
  RASQAL_QUERY_RESULTS_FORMAT_FLAG_WRITER = 2
} rasqal_query_results_format_flags;


RASQAL_API
int rasqal_query_results_formats_check(rasqal_world* world, const char *name, raptor_uri* uri, const char *mime_type, int flags);
RASQAL_API
rasqal_query_results_formatter* rasqal_new_query_results_formatter(rasqal_world* world, const char *name, const char *mime_type, raptor_uri* format_uri);
RASQAL_API
rasqal_query_results_formatter* rasqal_new_query_results_formatter_for_content(rasqal_world* world, raptor_uri *uri, const char *mime_type, const unsigned char *buffer, size_t len, const unsigned char *identifier);
RASQAL_API
void rasqal_free_query_results_formatter(rasqal_query_results_formatter* formatter);
RASQAL_API
int rasqal_query_results_formatter_write(raptor_iostream *iostr, rasqal_query_results_formatter* formatter, rasqal_query_results* results, raptor_uri *base_uri);
RASQAL_API
int rasqal_query_results_formatter_read(rasqal_world* world, raptor_iostream *iostr, rasqal_query_results_formatter* formatter, rasqal_query_results* results, raptor_uri *base_uri);

RASQAL_API
int rasqal_query_iostream_write_escaped_counted_string(rasqal_query* query, raptor_iostream* iostr, const unsigned char* string, size_t len);
RASQAL_API
unsigned char* rasqal_query_escape_counted_string(rasqal_query* query, const unsigned char *string, size_t len, size_t* output_len_p);


/* Data graph class */
RASQAL_API
rasqal_data_graph* rasqal_new_data_graph_from_uri(rasqal_world* world, raptor_uri* uri, raptor_uri* name_uri, int flags, const char* format_type, const char* format_name, raptor_uri* format_uri);
RASQAL_API
rasqal_data_graph* rasqal_new_data_graph_from_iostream(rasqal_world* world, raptor_iostream* iostr, raptor_uri* base_uri, raptor_uri* name_uri, unsigned int flags, const char* format_type, const char* format_name, raptor_uri* format_uri);
RASQAL_API
rasqal_data_graph* rasqal_new_data_graph_from_data_graph(rasqal_data_graph* dg);
RASQAL_API
void rasqal_free_data_graph(rasqal_data_graph* dg);
RASQAL_API
int rasqal_data_graph_print(rasqal_data_graph* dg, FILE* fh);


/**
 * rasqal_compare_flags:
 * @RASQAL_COMPARE_NOCASE: String comparisons are case independent.
 * @RASQAL_COMPARE_XQUERY: XQuery comparison rules apply.
 * @RASQAL_COMPARE_RDF:    RDF Term comparison rules apply.
 * @RASQAL_COMPARE_URI:    Allow comparison of URIs and allow strings to have a boolean value (unused; was for RDQL)
 * @RASQAL_COMPARE_SAMETERM: SPARQL sameTerm() builtin rules apply.
 *
 * Flags for rasqal_expression_evaluate(), rasqal_literal_compare() or
 * rasqal_literal_as_string_flags()
 */
typedef enum {
  RASQAL_COMPARE_NOCASE = 1,
  RASQAL_COMPARE_XQUERY = 2,
  RASQAL_COMPARE_RDF    = 4,
  RASQAL_COMPARE_URI    = 8,
  RASQAL_COMPARE_SAMETERM = 16
} rasqal_compare_flags;


/**
 * rasqal_random:
 *
 * Internal
 */
typedef struct rasqal_random_s rasqal_random;


/**
 * rasqal_evaluation_context:
 * @world: rasqal world
 * @base_uri: base URI of expression context (or NULL)
 * @locator: locator or NULL
 * @flags: expression comparison flags
 * @seed: random seed
 * @random: random number generator object
 *
 * A context for evaluating an expression such as with
 * rasqal_expression_evaluate2()
 */
typedef struct {
  rasqal_world *world;
  raptor_uri* base_uri;
  raptor_locator *locator;
  int flags;
  unsigned int seed;
  rasqal_random* random;
} rasqal_evaluation_context;


/* Expression class */
RASQAL_API
rasqal_expression* rasqal_new_0op_expression(rasqal_world* world, rasqal_op op);
RASQAL_API
rasqal_expression* rasqal_new_1op_expression(rasqal_world* world, rasqal_op op, rasqal_expression* arg);
RASQAL_API
rasqal_expression* rasqal_new_2op_expression(rasqal_world* world, rasqal_op op, rasqal_expression* arg1, rasqal_expression* arg2);
RASQAL_API
rasqal_expression* rasqal_new_3op_expression(rasqal_world* world, rasqal_op op, rasqal_expression* arg1,  rasqal_expression* arg2, rasqal_expression* arg3);
RASQAL_API
rasqal_expression* rasqal_new_4op_expression(rasqal_world* world, rasqal_op op, rasqal_expression* arg1, rasqal_expression* arg2, rasqal_expression* arg3, rasqal_expression* arg4);
RASQAL_API
rasqal_expression* rasqal_new_string_op_expression(rasqal_world* world, rasqal_op op, rasqal_expression* arg1, rasqal_literal* literal);
RASQAL_API
rasqal_expression* rasqal_new_literal_expression(rasqal_world* world, rasqal_literal* literal);
RASQAL_API
rasqal_expression* rasqal_new_function_expression(rasqal_world* world, raptor_uri* name, raptor_sequence* args, raptor_sequence* params, unsigned int flags);
RASQAL_API
rasqal_expression* rasqal_new_aggregate_function_expression(rasqal_world* world, rasqal_op op, rasqal_expression* arg1, raptor_sequence* params, unsigned int flags);
RASQAL_API
rasqal_expression* rasqal_new_cast_expression(rasqal_world* world, raptor_uri* name, rasqal_expression *value);
RASQAL_API
rasqal_expression* rasqal_new_expr_seq_expression(rasqal_world* world, rasqal_op op, raptor_sequence* args);
RASQAL_API
rasqal_expression* rasqal_new_set_expression(rasqal_world* world, rasqal_op op, rasqal_expression* arg1, raptor_sequence* args);
RASQAL_API
rasqal_expression* rasqal_new_group_concat_expression(rasqal_world* world,  int flags, raptor_sequence* args, rasqal_literal* separator);
RASQAL_API
rasqal_expression* rasqal_new_expression_from_expression(rasqal_expression* e);

RASQAL_API
void rasqal_free_expression(rasqal_expression* e);
RASQAL_API
void rasqal_expression_print_op(rasqal_expression* e, FILE* fh);
RASQAL_API
int rasqal_expression_print(rasqal_expression* e, FILE* fh);
RASQAL_API RASQAL_DEPRECATED
rasqal_literal* rasqal_expression_evaluate(rasqal_world *world, raptor_locator *locator, rasqal_expression* e, int flags);
RASQAL_API
rasqal_literal* rasqal_expression_evaluate2(rasqal_expression *e, rasqal_evaluation_context* eval_context, int *error_p);
RASQAL_API
const char* rasqal_expression_op_label(rasqal_op op);
RASQAL_API
int rasqal_expression_compare(rasqal_expression* e1, rasqal_expression* e2, int flags, int* error_p);

/**
 * rasqal_expression_visit_fn:
 * @user_data: user data passed in with rasqal_expression_visit()
 * @e: current expression
 *
 * User function to visit an expression and operate on it with
 * rasqal_expression_visit()
 *
 * Return value: non-0 to truncate the visit
 */
typedef int (*rasqal_expression_visit_fn)(void *user_data, rasqal_expression *e);
RASQAL_API
int rasqal_expression_visit(rasqal_expression* e, rasqal_expression_visit_fn fn, void *user_data);

RASQAL_API
rasqal_evaluation_context* rasqal_new_evaluation_context(rasqal_world* world, raptor_locator* locator, int flags);
RASQAL_API
void rasqal_free_evaluation_context(rasqal_evaluation_context* eval_context);
RASQAL_API
int rasqal_evaluation_context_set_base_uri(rasqal_evaluation_context* eval_context, raptor_uri *base_uri);
RASQAL_API
int rasqal_evaluation_context_set_rand_seed(rasqal_evaluation_context* eval_context, unsigned int seed);


/* Literal class */
RASQAL_API
rasqal_literal* rasqal_new_integer_literal(rasqal_world* world, rasqal_literal_type type, int integer);
RASQAL_API
rasqal_literal* rasqal_new_numeric_literal_from_long(rasqal_world* world, rasqal_literal_type type, long value);
RASQAL_API
rasqal_literal* rasqal_new_typed_literal(rasqal_world* world, rasqal_literal_type type, const unsigned char* string);
RASQAL_API
rasqal_literal* rasqal_new_double_literal(rasqal_world* world, double d);
RASQAL_API
rasqal_literal* rasqal_new_floating_literal(rasqal_world *world, rasqal_literal_type type, double d);
RASQAL_API RASQAL_DEPRECATED
rasqal_literal* rasqal_new_float_literal(rasqal_world* world, float f);
RASQAL_API
rasqal_literal* rasqal_new_uri_literal(rasqal_world* world, raptor_uri* uri);
RASQAL_API
rasqal_literal* rasqal_new_pattern_literal(rasqal_world* world, const unsigned char *pattern, const char *flags);
RASQAL_API
rasqal_literal* rasqal_new_string_literal(rasqal_world* world, const unsigned char *string, const char *language, raptor_uri *datatype, const unsigned char *datatype_qname);
RASQAL_API
rasqal_literal* rasqal_new_simple_literal(rasqal_world* world, rasqal_literal_type type, const unsigned char *string);
RASQAL_API
rasqal_literal* rasqal_new_boolean_literal(rasqal_world* world, int value);
RASQAL_API
rasqal_literal* rasqal_new_variable_literal(rasqal_world* world, rasqal_variable *variable);
RASQAL_API
rasqal_literal* rasqal_new_decimal_literal(rasqal_world* world, const unsigned char *string);
RASQAL_API
rasqal_literal* rasqal_new_decimal_literal_from_decimal(rasqal_world* world, const unsigned char *string, rasqal_xsd_decimal* decimal);
RASQAL_API
rasqal_literal* rasqal_new_datetime_literal_from_datetime(rasqal_world* world, rasqal_xsd_datetime* dt);


RASQAL_API
rasqal_literal* rasqal_new_literal_from_literal(rasqal_literal* l);
RASQAL_API
void rasqal_free_literal(rasqal_literal* l);
RASQAL_API
int rasqal_literal_print(rasqal_literal* l, FILE* fh);
RASQAL_API
const char* rasqal_literal_type_label(rasqal_literal_type type);
RASQAL_API
void rasqal_literal_print_type(rasqal_literal* l, FILE* fh);
RASQAL_API
rasqal_variable* rasqal_literal_as_variable(rasqal_literal* l);
RASQAL_API
const unsigned char* rasqal_literal_as_counted_string(rasqal_literal* l, size_t *len_p, int flags, int *error_p);
RASQAL_API
const unsigned char* rasqal_literal_as_string(rasqal_literal* l);
RASQAL_API
const unsigned char* rasqal_literal_as_string_flags(rasqal_literal* l, int flags, int *error_p);
RASQAL_API
rasqal_literal* rasqal_literal_as_node(rasqal_literal* l);
RASQAL_API
raptor_uri* rasqal_literal_datatype(rasqal_literal* l);
RASQAL_API
rasqal_literal* rasqal_literal_value(rasqal_literal* l);

RASQAL_API
int rasqal_literal_compare(rasqal_literal* l1, rasqal_literal* l2, int flags, int *error_p);
RASQAL_API
int rasqal_literal_equals(rasqal_literal* l1, rasqal_literal* l2);
RASQAL_API
int rasqal_literal_same_term(rasqal_literal* l1, rasqal_literal* l2);
RASQAL_API
rasqal_literal_type rasqal_literal_get_rdf_term_type(rasqal_literal* l);
RASQAL_API
int rasqal_literal_is_rdf_literal(rasqal_literal* l);


RASQAL_API
rasqal_prefix* rasqal_new_prefix(rasqal_world* world, const unsigned char* prefix, raptor_uri* uri);
RASQAL_API
void rasqal_free_prefix(rasqal_prefix* p);
RASQAL_API
int rasqal_prefix_print(rasqal_prefix* p, FILE* fh);


/* Row class */
RASQAL_API
rasqal_row* rasqal_new_row_for_size(rasqal_world* world, int size);
RASQAL_API
void rasqal_free_row(rasqal_row* row);
RASQAL_API
int rasqal_row_set_value_at(rasqal_row* row, int offset, rasqal_literal* value);


/* Triple class */
RASQAL_API
rasqal_triple* rasqal_new_triple(rasqal_literal* subject, rasqal_literal* predicate, rasqal_literal* object);
RASQAL_API
rasqal_triple* rasqal_new_triple_from_triple(rasqal_triple* t);
RASQAL_API
void rasqal_free_triple(rasqal_triple* t);
RASQAL_API
int rasqal_triple_print(rasqal_triple* t, FILE* fh);
RASQAL_API
void rasqal_triple_set_origin(rasqal_triple* t, rasqal_literal *l);
RASQAL_API
rasqal_literal* rasqal_triple_get_origin(rasqal_triple* t);

/* Variable class */
RASQAL_API
rasqal_variable* rasqal_new_variable_from_variable(rasqal_variable* v);
RASQAL_API
void rasqal_free_variable(rasqal_variable* v);
RASQAL_API
int rasqal_variable_print(rasqal_variable* v, FILE* fh);
RASQAL_API
void rasqal_variable_set_value(rasqal_variable* v, rasqal_literal* l);


/* Variables Table */
RASQAL_API
rasqal_variables_table* rasqal_new_variables_table(rasqal_world* world);
RASQAL_API
void rasqal_free_variables_table(rasqal_variables_table* vt);
RASQAL_API
rasqal_variable* rasqal_variables_table_add(rasqal_variables_table* vt, rasqal_variable_type type, const unsigned char *name, rasqal_literal *value);
RASQAL_API
int rasqal_variables_table_add_variable(rasqal_variables_table* vt, rasqal_variable* variable);
RASQAL_API
rasqal_variable* rasqal_variables_table_get_by_name(rasqal_variables_table* vt, rasqal_variable_type type, const unsigned char *name);
RASQAL_API
int rasqal_variables_table_contains(rasqal_variables_table* vt, rasqal_variable_type type, const unsigned char *name);

/* memory functions */
RASQAL_API
void rasqal_free_memory(void *ptr);
RASQAL_API
void* rasqal_alloc_memory(size_t size);
RASQAL_API
void* rasqal_calloc_memory(size_t nmemb, size_t size);


/* decimal functions */
RASQAL_API
rasqal_xsd_decimal* rasqal_new_xsd_decimal(rasqal_world* world);
RASQAL_API
void rasqal_free_xsd_decimal(rasqal_xsd_decimal* dec);
RASQAL_API
int rasqal_xsd_decimal_set_string(rasqal_xsd_decimal* dec, const char* string);
RASQAL_API
double rasqal_xsd_decimal_get_double(rasqal_xsd_decimal* dec);
RASQAL_API
long rasqal_xsd_decimal_get_long(rasqal_xsd_decimal* dec, int* error_p);
RASQAL_API
char* rasqal_xsd_decimal_as_string(rasqal_xsd_decimal* dec);
RASQAL_API
char* rasqal_xsd_decimal_as_counted_string(rasqal_xsd_decimal* dec, size_t* len_p);
RASQAL_API
int rasqal_xsd_decimal_set_long(rasqal_xsd_decimal* dec, long l);
RASQAL_API
int rasqal_xsd_decimal_set_double(rasqal_xsd_decimal* dec, double d);
RASQAL_API
int rasqal_xsd_decimal_print(rasqal_xsd_decimal* dec, FILE* stream);
RASQAL_API
int rasqal_xsd_decimal_add(rasqal_xsd_decimal* result, rasqal_xsd_decimal* a, rasqal_xsd_decimal* b);
RASQAL_API
int rasqal_xsd_decimal_subtract(rasqal_xsd_decimal* result, rasqal_xsd_decimal* a, rasqal_xsd_decimal* b);
RASQAL_API
int rasqal_xsd_decimal_multiply(rasqal_xsd_decimal* result, rasqal_xsd_decimal* a, rasqal_xsd_decimal* b);
RASQAL_API
int rasqal_xsd_decimal_divide(rasqal_xsd_decimal* result, rasqal_xsd_decimal* a, rasqal_xsd_decimal* b);
RASQAL_API
int rasqal_xsd_decimal_negate(rasqal_xsd_decimal* result, rasqal_xsd_decimal* a);
RASQAL_API
int rasqal_xsd_decimal_compare(rasqal_xsd_decimal* a, rasqal_xsd_decimal* b);
RASQAL_API
int rasqal_xsd_decimal_equals(rasqal_xsd_decimal* a, rasqal_xsd_decimal* b);
RASQAL_API
int rasqal_xsd_decimal_is_zero(rasqal_xsd_decimal* d);
RASQAL_API
int rasqal_xsd_decimal_abs(rasqal_xsd_decimal* result, rasqal_xsd_decimal* a);
RASQAL_API
int rasqal_xsd_decimal_round(rasqal_xsd_decimal* result, rasqal_xsd_decimal* a);
RASQAL_API
int rasqal_xsd_decimal_ceil(rasqal_xsd_decimal* result, rasqal_xsd_decimal* a);
RASQAL_API
int rasqal_xsd_decimal_floor(rasqal_xsd_decimal* result, rasqal_xsd_decimal* a);


/* date functions */
RASQAL_API
rasqal_xsd_date* rasqal_new_xsd_date(rasqal_world* world, const char *date_string);
RASQAL_API
void rasqal_free_xsd_date(rasqal_xsd_date* d);
RASQAL_API
char* rasqal_xsd_date_to_counted_string(const rasqal_xsd_date *date, size_t *len_p);
RASQAL_API
char* rasqal_xsd_date_to_string(const rasqal_xsd_date *d);
RASQAL_API
int rasqal_xsd_date_equals(const rasqal_xsd_date *d1, const rasqal_xsd_date *d2, int *incomparible_p);
RASQAL_API
int rasqal_xsd_date_compare(const rasqal_xsd_date *d1, const rasqal_xsd_date *d2, int *incomparible_p);

/* datetime functions */
RASQAL_API
rasqal_xsd_datetime* rasqal_new_xsd_datetime(rasqal_world* world, const char *datetime_string);
RASQAL_API
rasqal_xsd_datetime* rasqal_new_xsd_datetime_from_unixtime(rasqal_world* world, time_t secs);
RASQAL_API
rasqal_xsd_datetime* rasqal_new_xsd_datetime_from_timeval(rasqal_world* world, struct timeval *tv);
RASQAL_API
rasqal_xsd_datetime* rasqal_new_xsd_datetime_from_xsd_date(rasqal_world* world, rasqal_xsd_date *date);
RASQAL_API
void rasqal_free_xsd_datetime(rasqal_xsd_datetime* dt);
RASQAL_API
char* rasqal_xsd_datetime_to_counted_string(const rasqal_xsd_datetime *dt, size_t *len_p);
RASQAL_API
char* rasqal_xsd_datetime_to_string(const rasqal_xsd_datetime *dt);
RASQAL_API
int rasqal_xsd_datetime_equals2(const rasqal_xsd_datetime *dt1, const rasqal_xsd_datetime *dt2, int *incomparible_p);
RASQAL_API RASQAL_DEPRECATED
int rasqal_xsd_datetime_equals(const rasqal_xsd_datetime *dt1, const rasqal_xsd_datetime *dt2);
RASQAL_API
int rasqal_xsd_datetime_compare2(const rasqal_xsd_datetime *dt1, const rasqal_xsd_datetime *dt2, int *incomparible_p);
RASQAL_API RASQAL_DEPRECATED
int rasqal_xsd_datetime_compare(const rasqal_xsd_datetime *dt1, const rasqal_xsd_datetime *dt2);
RASQAL_API
rasqal_xsd_decimal* rasqal_xsd_datetime_get_seconds_as_decimal(rasqal_world* world, rasqal_xsd_datetime* dt);
RASQAL_API
int rasqal_xsd_datetime_set_from_timeval(rasqal_xsd_datetime *dt, struct timeval *tv);
RASQAL_API
int rasqal_xsd_datetime_set_from_unixtime(rasqal_xsd_datetime* dt, time_t clock);
RASQAL_API
time_t rasqal_xsd_datetime_get_as_unixtime(rasqal_xsd_datetime* dt);
RASQAL_API
struct timeval* rasqal_xsd_datetime_get_as_timeval(rasqal_xsd_datetime *dt);
RASQAL_API
char* rasqal_xsd_datetime_get_timezone_as_counted_string(rasqal_xsd_datetime *dt, size_t *len_p);
RASQAL_API
char* rasqal_xsd_datetime_get_tz_as_counted_string(rasqal_xsd_datetime* dt, size_t *len_p);


/* regex utilities */
RASQAL_API
char* rasqal_regex_replace(rasqal_world* world, raptor_locator* locator, const char* pattern, const char* regex_flags, const char* subject, size_t subject_len, const char* replace, size_t replace_len, size_t* result_len_p);


/**
 * rasqal_service:
 *
 * Rasqal SPARQL Protocol Service
 */
typedef struct rasqal_service_s rasqal_service;

RASQAL_API
rasqal_service* rasqal_new_service(rasqal_world* world, raptor_uri* service_uri, const unsigned char* query_string, raptor_sequence* data_graphs);
RASQAL_API
void rasqal_free_service(rasqal_service* svc);
RASQAL_API
rasqal_query_results* rasqal_service_execute(rasqal_service* svc);
RASQAL_API
int rasqal_service_set_www(rasqal_service* svc, raptor_www* www);
RASQAL_API
int rasqal_service_set_format(rasqal_service* svc, const char *format);



/**
 * rasqal_triple_parts:
 * @RASQAL_TRIPLE_NONE: no parts
 * @RASQAL_TRIPLE_SUBJECT: Subject present in a triple.
 * @RASQAL_TRIPLE_PREDICATE: Predicate present in a triple.
 * @RASQAL_TRIPLE_OBJECT: Object present in a triple.
 * @RASQAL_TRIPLE_ORIGIN: Origin/graph present in a triple.
 * @RASQAL_TRIPLE_GRAPH:  Alias for RASQAL_TRIPLE_ORIGIN
 * @RASQAL_TRIPLE_SPO: Subject, Predicate and Object present in a triple.
 * @RASQAL_TRIPLE_SPOG: Subject, Predicate, Object, Graph present in a triple.
 *
 * Flags for parts of a triple.
 */
typedef enum {
  RASQAL_TRIPLE_NONE     = 0,
  RASQAL_TRIPLE_SUBJECT  = 1,
  RASQAL_TRIPLE_PREDICATE= 2,
  RASQAL_TRIPLE_OBJECT   = 4,
  RASQAL_TRIPLE_ORIGIN   = 8,
  RASQAL_TRIPLE_GRAPH    = RASQAL_TRIPLE_ORIGIN,
  RASQAL_TRIPLE_SPO      = RASQAL_TRIPLE_SUBJECT | RASQAL_TRIPLE_PREDICATE | RASQAL_TRIPLE_OBJECT,
  RASQAL_TRIPLE_SPOG     = RASQAL_TRIPLE_SPO | RASQAL_TRIPLE_GRAPH
} rasqal_triple_parts;



/**
 * rasqal_triples_match:
 * @world: rasqal_world object
 * @user_data: User data pointer for factory methods.
 * @bind_match: The [4]array (s,p,o,origin) bindings against the current triple match only touching triple parts given. Returns parts that were bound or 0 on failure.
 * @next_match: Move to next match.
 * @is_end: Check for end of triple match - return non-0 if is end.
 * @finish: Finish triples match and destroy any allocated memory.
 * @is_exact: non-0 if triple to match is all literal constants
 * @finished: >0 if the match has finished
 *
 * Triples match structure as initialised by #rasqal_triples_source
 * method init_triples_match.
 */
struct rasqal_triples_match_s {
  rasqal_world *world;

  void *user_data;

  rasqal_triple_parts (*bind_match)(struct rasqal_triples_match_s* rtm, void *user_data, rasqal_variable *bindings[4], rasqal_triple_parts parts);

  void (*next_match)(struct rasqal_triples_match_s* rtm, void *user_data);

  int (*is_end)(struct rasqal_triples_match_s* rtm, void *user_data);

  void (*finish)(struct rasqal_triples_match_s* rtm, void *user_data);

  int is_exact;

  int finished;
};
typedef struct rasqal_triples_match_s rasqal_triples_match;


/**
 * rasqal_triple_meta:
 * @bindings: Variable bindings for this triple+origin to set.
 * @triples_match: The matcher that is setting these bindings.
 * @context: Context data used by the matcher.
 * @parts: Bitmask of #rasqal_triple_parts flags describing the parts of the triple pattern that will bind to variables.  There may also be variables mentioned that are bound in other triple patterns even if @parts is 0.
 * @is_exact: unused
 * @executed: unused
 *
 * Metadata for triple pattern matching for one triple pattern.
 */
typedef struct {
  /* triple (subject, predicate, object) and origin */
  rasqal_variable* bindings[4];

  rasqal_triples_match *triples_match;

  void *context;

  rasqal_triple_parts parts;

  int is_exact;

  int executed;
} rasqal_triple_meta;


/**
 * RASQAL_TRIPLES_SOURCE_MIN_VERSION:
 *
 * Lowest accepted @rasqal_triples_source API version
 */
#define RASQAL_TRIPLES_SOURCE_MIN_VERSION 1

/**
 * RASQAL_TRIPLES_SOURCE_MAX_VERSION:
 *
 * Highest accepted @rasqal_triples_source API version
 */
#define RASQAL_TRIPLES_SOURCE_MAX_VERSION 2


/**
 * rasqal_triples_source_feature:
 * @RASQAL_TRIPLES_SOURCE_FEATURE_NONE: No feature
 * @RASQAL_TRIPLES_SOURCE_FEATURE_IOSTREAM_DATA_GRAPH: Support raptor_iostream data graphs
 *
 * Optional features that may be supported by a triple source factory
 */
typedef enum {
  RASQAL_TRIPLES_SOURCE_FEATURE_NONE,
  RASQAL_TRIPLES_SOURCE_FEATURE_IOSTREAM_DATA_GRAPH
} rasqal_triples_source_feature;


/**
 * rasqal_triples_source:
 * @version: API version - only V1 is defined for now
 * @query: Source for this query.
 * @user_data: Context user data passed into the factory methods.
 * @init_triples_match: Factory method to initialise a new #rasqal_triples_match.
 * @triple_present: Factory method to return presence or absence of a complete triple.
 * @free_triples_source: Factory method to deallocate resources.
 * @support_feature: Factory method to test support for a feature, returning non-0 if supported
 *
 * Triples source as initialised by a #rasqal_triples_source_factory.
 */
struct rasqal_triples_source_s {
  int version;

  rasqal_query* query;

  void *user_data;

  /* API v1 */
  int (*init_triples_match)(rasqal_triples_match* rtm, struct rasqal_triples_source_s* rts, void *user_data, rasqal_triple_meta *m, rasqal_triple *t);

  int (*triple_present)(struct rasqal_triples_source_s* rts, void *user_data, rasqal_triple *t);

  void (*free_triples_source)(void *user_data);

  /* API v2 onwards */
  int (*support_feature)(void *user_data, rasqal_triples_source_feature feature);
};
typedef struct rasqal_triples_source_s rasqal_triples_source;


/**
 * RASQAL_TRIPLES_SOURCE_FACTORY_MIN_VERSION:
 *
 * Lowest accepted @rasqal_triples_source_factory API version
 */
#define RASQAL_TRIPLES_SOURCE_FACTORY_MIN_VERSION 1

/**
 * RASQAL_TRIPLES_SOURCE_FACTORY_MAX_VERSION:
 *
 * Highest accepted @rasqal_triples_source_factory API version
 */
#define RASQAL_TRIPLES_SOURCE_FACTORY_MAX_VERSION 2


/**
 * rasqal_triples_error_handler:
 * @query: query object
 * @locator: error locator (or NULL)
 * @message: error message
 *
 * Triples source factory error handler callback.
 */
typedef void (*rasqal_triples_error_handler)(rasqal_query* query, raptor_locator* locator, const char* message);


/**
 * rasqal_triples_source_factory:
 * @version: API factory version from 1 to 2
 * @user_data: User data for triples_source_factory.
 * @user_data_size: Size of @user_data for new_triples_source.
 * @new_triples_source: Create a new triples source - returns non-zero on failure &lt; 0 is a 'no rdf data error', &gt; 0 is an unspecified error. Error messages are generated by rasqal internally. (V1)
 * @init_triples_source: Initialise a new triples source V2 for a particular source URI/base URI and syntax. Returns non-zero on failure with errors reported via the handler callback by the implementation. (V2)
 *
 * A factory that initialises #rasqal_triples_source structures to
 * returning matches to a triple pattern across the dataset formed
 * from the data graphs recorded in the @query object.
 */
typedef struct {
  int version;

  void *user_data;
  size_t user_data_size;

  /* API v1 */
  int (*new_triples_source)(rasqal_query* query, void *factory_user_data, void *user_data, rasqal_triples_source* rts);
  /* API v2 onwards */
  int (*init_triples_source)(rasqal_query* query, void *factory_user_data, void *user_data, rasqal_triples_source* rts, rasqal_triples_error_handler handler);
} rasqal_triples_source_factory;


/**
 * rasqal_triples_source_factory_register_fn:
 * @factory: factory to register
 *
 * Register a factory for generating triples sources #rasqal_triples_source
 *
 * Return value: non-0 on failure
 */
typedef int (*rasqal_triples_source_factory_register_fn)(rasqal_triples_source_factory *factory);


/* set the triples_source_factory */
RASQAL_API
int rasqal_set_triples_source_factory(rasqal_world* world, rasqal_triples_source_factory_register_fn register_fn, void* user_data);



/* The info below is solely for gtk-doc - ignore it */

/**
 * raptor_world:
 *
 * Internal
 */

/**
 * RASQAL_QUERY_RESULTS_FORMATTER_DECLARED:
 *
 * Internal
 */

/**
 * RASQAL_WORLD_DECLARED:
 *
 * Internal
 */


/**
 * RASQAL_LITERAL_UDT_DEFINED
 *
 * Internal
 */

/**
 * rasqal_expression_s:
 * @usage: Internal
 * @op: Internal
 * @arg1: Internal
 * @arg2: Internal
 * @arg3: Internal
 * @literal: Internal
 * @value: Internal
 * @name: Internal
 * @args: Internal
 * @params: Internal
 * @flags: Internal
 *
 * Internal - see #rasqal_expression.
 *
 */

/**
 * bind_match:
 * @rtm: triples match context
 * @user_data: user data
 * @bindings: variable binding for parts of triple (s, p, o, g)
 * @parts: parts of triple to match
 *
 * Internal - see #rasqal_triples_match
 *
 * Return value: match parts
*/

/**
 * next_match:
 * @rtm: triples match context
 * @user_data: user data
 *
 * Internal - see #rasqal_triples_match
 */

/**
 * is_end:
 * @rtm: triples match context
 * @user_data: user data
 *
 * Internal - see #rasqal_triples_match
 *
 * Return value: non-0 if end of match
 */

/**
 * finish:
 * @rtm: triples match context
 * @user_data: user data
 *
 * Internal - see #rasqal_triples_match
 */

/**
 * init_triples_match:
 * @rtm: triples match context
 * @rts: triples match source
 * @user_data: user data
 * @m: triple meta
 * @t: triple
 *
 * Internal - see #rasqal_triples_source
 *
 * Return value: non-0 on failure
 */

/**
 * triple_present:
 * @rts: triples match source
 * @user_data: user data
 * @t: triple to test for presence
 *
 * Internal - see #rasqal_triples_source
 *
 * Return value: non-0 on failure
 */

/**
 * free_triples_source:
 * @user_data: user data
 *
 * Internal - see #rasqal_triples_source
 */

/**
 * support_feature:
 * @user_data: user data
 * @feature: feature to test
 *
 * Internal - see #rasqal_triples_source
 *
 * Return value: non-0 if supported
 */

/**
 * rasqal_variables_table:
 *
 * Internal - for now
 */


#ifdef __cplusplus
}
#endif

#endif
