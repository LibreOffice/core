/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor.h - Redland Parser Toolkit for RDF (Raptor) - public API
 *
 * Copyright (C) 2000-2013, David Beckett http://www.dajobe.org/
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



#ifndef RAPTOR_H
#define RAPTOR_H


#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

/* Required for va_list in raptor_vsnprintf */
#include <stdarg.h>


/**
 * RAPTOR_V2_AVAILABLE
 *
 * Flag for marking raptor2 API availability.
 */
#define RAPTOR_V2_AVAILABLE 1


/**
 * RAPTOR_VERSION:
 *
 * Raptor library version number
 *
 * Format: major * 10000 + minor * 100 + release
 */
#define RAPTOR_VERSION 20015

/**
 * RAPTOR_VERSION_STRING:
 *
 * Raptor library version string
 */
#define RAPTOR_VERSION_STRING "2.0.15"

/**
 * RAPTOR_VERSION_MAJOR:
 *
 * Raptor library major version
 */
#define RAPTOR_VERSION_MAJOR 2

/**
 * RAPTOR_VERSION_MINOR:
 *
 * Raptor library minor version
 */
#define RAPTOR_VERSION_MINOR 0

/**
 * RAPTOR_VERSION_RELEASE:
 *
 * Raptor library release
 */
#define RAPTOR_VERSION_RELEASE 15

/**
 * RAPTOR_API:
 *
 * Macro for wrapping API function call declarations.
 *
 */
#ifndef RAPTOR_API
#  ifdef _WIN32
#    ifdef __GNUC__
#      undef _declspec
#      define _declspec(x) __declspec(x)
#    endif
#    ifdef RAPTOR_STATIC
#      define RAPTOR_API
#    else
#      ifdef RAPTOR_INTERNAL
#        define RAPTOR_API _declspec(dllexport)
#      else
#        define RAPTOR_API _declspec(dllimport)
#      endif
#    endif
#  else
#    define RAPTOR_API
#  endif
#endif

/* Use gcc 3.1+ feature to allow marking of deprecated API calls.
 * This gives a warning during compiling.
 */
#if defined(__GNUC__) && (__GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 1))
#define RAPTOR_DEPRECATED __attribute__((deprecated))
#define RAPTOR_NORETURN __attribute__((__noreturn__))
#else
#define RAPTOR_DEPRECATED
#define RAPTOR_NORETURN
#endif

/**
 * RAPTOR_PRINTF_FORMAT:
 * @string_index: ignore me
 * @first_to_check_index: ignore me
 *
 * Internal macro
 */
#if defined(__GNUC__) && (__GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 5))
#define RAPTOR_PRINTF_FORMAT(string_index, first_to_check_index) \
  __attribute__((__format__(__printf__, string_index, first_to_check_index)))
#else
#define RAPTOR_PRINTF_FORMAT(string_index, first_to_check_index)
#endif

/**
 * raptor_uri:
 *
 * Raptor URI Class.
 */
typedef struct raptor_uri_s raptor_uri;


/* Public statics */

/**
 * raptor_short_copyright_string:
 *
 * Short copyright string (one line).
 */
RAPTOR_API
extern const char * const raptor_short_copyright_string;

/**
 * raptor_copyright_string:
 *
 * Copyright string (multiple lines).
 */
RAPTOR_API
extern const char * const raptor_copyright_string;

/**
 * raptor_version_string:
 *
 * Raptor version as a string.
 */
RAPTOR_API
extern const char * const raptor_version_string;

/**
 * raptor_version_major:
 *
 * Raptor major version number.
 */
RAPTOR_API
extern const unsigned int raptor_version_major;

/**
 * raptor_version_minor:
 *
 * Raptor minor version number.
 */
RAPTOR_API
extern const unsigned int raptor_version_minor;

/**
 * raptor_version_release:
 *
 * Raptor release version number.
 */
RAPTOR_API
extern const unsigned int raptor_version_release;

/**
 * raptor_version_decimal:
 *
 * Raptor version as a decimal number.
 *
 * Format: major * 10000 + minor * 100 + release
 */
RAPTOR_API
extern const unsigned int raptor_version_decimal;

/**
 * raptor_license_string:
 *
 * Raptor license string.
 */
RAPTOR_API
extern const char * const raptor_license_string;

/**
 * raptor_home_url_string:
 *
 * Raptor home page URL.
 */
RAPTOR_API
extern const char * const raptor_home_url_string;

/**
 * raptor_xml_namespace_uri:
 *
 * XML Namespace (xml:) URI string.
 */
RAPTOR_API
extern const unsigned char * const raptor_xml_namespace_uri;


/**
 * raptor_rdf_namespace_uri:
 *
 * RDF Namespace (rdf:) URI string.
 */
RAPTOR_API
extern const unsigned char * const raptor_rdf_namespace_uri;

/**
 * raptor_rdf_namespace_uri_len:
 *
 * Length of #raptor_rdf_namespace_uri string
 */
RAPTOR_API
extern const unsigned int raptor_rdf_namespace_uri_len;

/**
 * raptor_rdf_schema_namespace_uri:
 *
 * RDF Schema (rdfs:) Namespace URI string.
 */
RAPTOR_API
extern const unsigned char * const raptor_rdf_schema_namespace_uri;

/**
 * raptor_xmlschema_datatypes_namespace_uri:
 *
 * XML Schema datatypes (xsd:) namespace URI string.
 */
RAPTOR_API
extern const unsigned char * const raptor_xmlschema_datatypes_namespace_uri;

/**
 * raptor_owl_namespace_uri:
 *
 * OWL (owl:) Namespace URI string.
 */
RAPTOR_API
extern const unsigned char * const raptor_owl_namespace_uri;

/**
 * raptor_xml_literal_datatype_uri_string:
 *
 * XML Literal datatype (rdf:XMLLiteral) URI string.
 */
RAPTOR_API
extern const unsigned char * const raptor_xml_literal_datatype_uri_string;

/**
 * raptor_xml_literal_datatype_uri_string_len:
 *
 * Length of #raptor_xml_literal_datatype_uri_string
 */
RAPTOR_API
extern const unsigned int raptor_xml_literal_datatype_uri_string_len;


/* Public structure */
/**
 * raptor_world:
 *
 * Raptor world class.
 */
typedef struct raptor_world_s raptor_world;
/**
 * raptor_parser:
 *
 * Raptor Parser class
 */
typedef struct raptor_parser_s raptor_parser;
/**
 * raptor_serializer:
 *
 * Raptor Serializer class
 */
typedef struct raptor_serializer_s raptor_serializer;

/**
 * raptor_www:
 *
 * Raptor WWW class
 */
typedef struct raptor_www_s raptor_www;
/**
 * raptor_iostream:
 *
 * Raptor I/O Stream class
 */
typedef struct raptor_iostream_s raptor_iostream;
/**
 * raptor_xml_element:
 *
 * Raptor XML Element class
 */
typedef struct raptor_xml_element_s raptor_xml_element;
/**
 * raptor_xml_writer:
 *
 * Raptor XML Writer class
 */
typedef struct raptor_xml_writer_s raptor_xml_writer;
/**
 * raptor_qname:
 *
 * Raptor XML qname class
 */
typedef struct raptor_qname_s raptor_qname;
/**
 * raptor_namespace:
 *
 * Raptor XML Namespace class
 */
typedef struct raptor_namespace_s raptor_namespace;
/**
 * raptor_namespace_stack:
 *
 * Raptor XML Namespace Stack class
 */
typedef struct raptor_namespace_stack_s raptor_namespace_stack;

/**
 * raptor_sax2:
 *
 * Raptor SAX2 class
 */
typedef struct raptor_sax2_s raptor_sax2;


/**
 * raptor_type_q:
 * @mime_type: MIME type string
 * @mime_type_len: length of @mime_type
 * @q: Q value 0-10 standing for decimal 0.0-1.0
 *
 * (MIME Type, Q) pair
 */
typedef struct {
  const char* mime_type;
  size_t mime_type_len;
  unsigned char q;
} raptor_type_q;


/**
 * raptor_syntax_bitflags:
 * @RAPTOR_SYNTAX_NEED_BASE_URI: the syntax requires a base URI
 *
 * Bit flags for #raptor_syntax_description flags field
 */
typedef enum {
  RAPTOR_SYNTAX_NEED_BASE_URI = 1
} raptor_syntax_bitflags;


/**
 * raptor_syntax_description:
 * @names: array of syntax names - the first one (required) is the public name, the rest are aliases.  The array is NULL terminated.
 * @names_count: size of @names array
 * @label: long descriptive label for syntax
 * @mime_types: Array of (MIME type, Q) values associated with the syntax (or NULL).  If present the array is NULL terminated.
 * @mime_types_count: size of @mime_types array
 * @uri_strings: array of URIs identifying the syntax (or NULL). The first one if present is the main URI, the rest are aliases.  The array is NULL terminated.
 * @uri_strings_count: size of @uri_strings array
 * @flags: See #raptor_syntax_bitflags for the bits
 *
 * Description of a syntax or file format.
 *
 */
typedef struct {
  const char* const* names;
  unsigned int names_count;

  const char* label;

  const raptor_type_q* mime_types;
  unsigned int mime_types_count;

  const char* const* uri_strings;
  unsigned int uri_strings_count;

  unsigned int flags;
} raptor_syntax_description;


/**
 * raptor_term_type:
 * @RAPTOR_TERM_TYPE_URI: RDF URI
 * @RAPTOR_TERM_TYPE_LITERAL: RDF literal
 * @RAPTOR_TERM_TYPE_BLANK: RDF blank node
 * @RAPTOR_TERM_TYPE_UNKNOWN: Internal
 *
 * Type of term in a #raptor_statement
 *
 * Node type 3 is unused but exists to preserve numeric compatibility
 * with librdf_node_type values.
 */
typedef enum {
  RAPTOR_TERM_TYPE_UNKNOWN = 0,
  RAPTOR_TERM_TYPE_URI     = 1,
  RAPTOR_TERM_TYPE_LITERAL = 2,
  /* unused type 3 */
  RAPTOR_TERM_TYPE_BLANK   = 4
} raptor_term_type;


/**
 * raptor_locator:
 * @uri: URI of location (or NULL)
 * @file: Filename of location (or NULL)
 * @line: Line number of location (or <0 for no line)
 * @column: Column number of location (or <0 for no column)
 * @byte: Byte number of location (or <0 for no byte)
 *
 * Location information for an error, warning or information message.
 */
typedef struct {
  raptor_uri *uri;
  const char *file;
  int line;
  int column;
  int byte;
} raptor_locator;

/**
 * raptor_option:
 * @RAPTOR_OPTION_SCANNING: If true (default false), the RDF/XML
 *   parser will look for embedded rdf:RDF elements inside the XML
 *   content, and not require that the XML start with an rdf:RDF root
 *   element.
 * @RAPTOR_OPTION_ALLOW_NON_NS_ATTRIBUTES: If true (default true)
 *   then the RDF/XML parser will allow non-XML namespaced attributes
 *   to be accepted as well as rdf: namespaced ones.  For example,
 *   'about' and 'ID' will be interpreted as if they were rdf:about
 *   and rdf:ID respectively.
 * @RAPTOR_OPTION_ALLOW_OTHER_PARSETYPES: If true (default true)
 *   then the RDF/XML parser will allow unknown parsetypes to be
 *   present and will pass them on to the user.  Unimplemented at
 *   present.
 * @RAPTOR_OPTION_ALLOW_BAGID: If true (default true) then the
 *   RDF/XML parser will support the rdf:bagID attribute that was
 *   removed from the RDF/XML language when it was revised.  This
 *   support may be removed in future.
 * @RAPTOR_OPTION_ALLOW_RDF_TYPE_RDF_LIST: If true (default false)
 *   then the RDF/XML parser will generate the idList rdf:type
 *   rdf:List triple in the handling of rdf:parseType="Collection".
 *   This triple was removed during the revising of RDF/XML after
 *   collections were initially added.
 * @RAPTOR_OPTION_NORMALIZE_LANGUAGE: If true (default true) then
 *   XML language values such as from xml:lang will be normalized to
 *   lowercase.
 * @RAPTOR_OPTION_NON_NFC_FATAL: If true (default false) then
 *  illegal Unicode Normal Form C in literals will give a fatal
 *  error, otherwise just a warning.
 * @RAPTOR_OPTION_WARN_OTHER_PARSETYPES: If true (default true) then
 *   the RDF/XML parser will warn about unknown rdf:parseType values.
 * @RAPTOR_OPTION_CHECK_RDF_ID: If true (default true) then the
 *   RDF/XML will check rdf:ID attribute values for duplicates and
 *   cause an error if any are found.
 * @RAPTOR_OPTION_RELATIVE_URIS: If true (default true) then
 *   relative URIs will be used wherever possible when serializing.
 * @RAPTOR_OPTION_WRITER_AUTO_INDENT: Automatically indent elements when
 *   seriailizing.
 * @RAPTOR_OPTION_WRITER_AUTO_EMPTY: Automatically detect and
 *   abbreviate empty elements when serializing.
 * @RAPTOR_OPTION_WRITER_INDENT_WIDTH: Integer number of spaces to use
 *   for each indent level when serializing with auto indent.
 * @RAPTOR_OPTION_WRITER_XML_VERSION: Integer XML version XML 1.0 (10) or XML 1.1 (11)
 * @RAPTOR_OPTION_WRITER_XML_DECLARATION: Write XML 1.0 or 1.1 declaration.
 * @RAPTOR_OPTION_NO_NET: Deny network requests inside other requests.
 * @RAPTOR_OPTION_RESOURCE_BORDER: Border color of resource
 *   nodes for GraphViz DOT serializer.
 * @RAPTOR_OPTION_LITERAL_BORDER: Border color of literal nodes
 *   for GraphViz DOT serializer.
 * @RAPTOR_OPTION_BNODE_BORDER: Border color of blank nodes for
 *   GraphViz DOT serializer.
 * @RAPTOR_OPTION_RESOURCE_FILL: Fill color of resource nodes
 *   for GraphViz DOT serializer.
 * @RAPTOR_OPTION_LITERAL_FILL: Fill color of literal nodes for
 *   GraphViz DOT serializer.
 * @RAPTOR_OPTION_BNODE_FILL: Fill color of blank nodes for
 *   GraphViz DOT serializer.
 * @RAPTOR_OPTION_HTML_TAG_SOUP: Use a lax HTML parser if an XML parser
 *   fails when read HTML for GRDDL parser.
 * @RAPTOR_OPTION_MICROFORMATS: Look for microformats for GRDDL parser.
 * @RAPTOR_OPTION_HTML_LINK: Look for head &lt;link&gt; to type rdf/xml
 *   for GRDDL parser.
 * @RAPTOR_OPTION_WWW_TIMEOUT: Set timeout for internal WWW URI requests
 *   for GRDDL parser.
 * @RAPTOR_OPTION_WRITE_BASE_URI: Write @base directive for Turtle/N3.
 * @RAPTOR_OPTION_WWW_HTTP_CACHE_CONTROL: HTTP Cache-Control: header
 * @RAPTOR_OPTION_WWW_HTTP_USER_AGENT: HTTP User-Agent: header
 * @RAPTOR_OPTION_JSON_CALLBACK: JSON serializer callback function.
 * @RAPTOR_OPTION_JSON_EXTRA_DATA: JSON serializer extra top-level data
 * @RAPTOR_OPTION_RSS_TRIPLES: Atom/RSS serializer writes extra RDF triples it finds (none, rdf-xml, atom-triples)
 * @RAPTOR_OPTION_ATOM_ENTRY_URI: Atom entry URI.  If given, generate an Atom Entry Document with the item having the given URI, otherwise generate an Atom Feed Document with any items found.
 * @RAPTOR_OPTION_PREFIX_ELEMENTS: Integer. If set, generate Atom/RSS1.0 documents with prefixed elements, otherwise unprefixed.
 * @RAPTOR_OPTION_STRICT: Boolean. If set, operate in strict conformance mode.
 * @RAPTOR_OPTION_WWW_CERT_FILENAME: String. SSL client certificate filename
 * @RAPTOR_OPTION_WWW_CERT_TYPE: String. SSL client certificate type
 * @RAPTOR_OPTION_WWW_CERT_PASSPHRASE: String. SSL client certificate passphrase
 * @RAPTOR_OPTION_WWW_SSL_VERIFY_PEER:  Integer. SSL verify peer - non-0 to verify peer SSL certificate (default)
 * @RAPTOR_OPTION_WWW_SSL_VERIFY_HOST: Integer. SSL verify host - 0 none, 1 CN match, 2 host match (default). Other values are ignored.
 * @RAPTOR_OPTION_NO_FILE: Deny file reading requests inside other requests.
 * @RAPTOR_OPTION_LOAD_EXTERNAL_ENTITIES: When reading XML, load external entities.
 * @RAPTOR_OPTION_LAST: Internal
 *
 * Raptor parser, serializer or XML writer options.
 */
typedef enum {
  RAPTOR_OPTION_SCANNING,
  RAPTOR_OPTION_ALLOW_NON_NS_ATTRIBUTES,
  RAPTOR_OPTION_ALLOW_OTHER_PARSETYPES,
  RAPTOR_OPTION_ALLOW_BAGID,
  RAPTOR_OPTION_ALLOW_RDF_TYPE_RDF_LIST,
  RAPTOR_OPTION_NORMALIZE_LANGUAGE,
  RAPTOR_OPTION_NON_NFC_FATAL,
  RAPTOR_OPTION_WARN_OTHER_PARSETYPES,
  RAPTOR_OPTION_CHECK_RDF_ID,
  RAPTOR_OPTION_RELATIVE_URIS,
  RAPTOR_OPTION_WRITER_AUTO_INDENT,
  RAPTOR_OPTION_WRITER_AUTO_EMPTY,
  RAPTOR_OPTION_WRITER_INDENT_WIDTH,
  RAPTOR_OPTION_WRITER_XML_VERSION,
  RAPTOR_OPTION_WRITER_XML_DECLARATION,
  RAPTOR_OPTION_NO_NET,
  RAPTOR_OPTION_RESOURCE_BORDER,
  RAPTOR_OPTION_LITERAL_BORDER,
  RAPTOR_OPTION_BNODE_BORDER,
  RAPTOR_OPTION_RESOURCE_FILL,
  RAPTOR_OPTION_LITERAL_FILL,
  RAPTOR_OPTION_BNODE_FILL,
  RAPTOR_OPTION_HTML_TAG_SOUP,
  RAPTOR_OPTION_MICROFORMATS,
  RAPTOR_OPTION_HTML_LINK,
  RAPTOR_OPTION_WWW_TIMEOUT,
  RAPTOR_OPTION_WRITE_BASE_URI,
  RAPTOR_OPTION_WWW_HTTP_CACHE_CONTROL,
  RAPTOR_OPTION_WWW_HTTP_USER_AGENT,
  RAPTOR_OPTION_JSON_CALLBACK,
  RAPTOR_OPTION_JSON_EXTRA_DATA,
  RAPTOR_OPTION_RSS_TRIPLES,
  RAPTOR_OPTION_ATOM_ENTRY_URI,
  RAPTOR_OPTION_PREFIX_ELEMENTS,
  RAPTOR_OPTION_STRICT,
  RAPTOR_OPTION_WWW_CERT_FILENAME,
  RAPTOR_OPTION_WWW_CERT_TYPE,
  RAPTOR_OPTION_WWW_CERT_PASSPHRASE,
  RAPTOR_OPTION_NO_FILE,
  RAPTOR_OPTION_WWW_SSL_VERIFY_PEER,
  RAPTOR_OPTION_WWW_SSL_VERIFY_HOST,
  RAPTOR_OPTION_LOAD_EXTERNAL_ENTITIES,
  RAPTOR_OPTION_LAST = RAPTOR_OPTION_LOAD_EXTERNAL_ENTITIES
} raptor_option;


/**
 * raptor_term_literal_value:
 * @string: literal string
 * @string_len: length of string
 * @datatype: datatype URI (or NULL)
 * @language: literal language (or NULL)
 * @language_len: length of language
 *
 * Literal term value - this typedef exists solely for use in #raptor_term
 *
 * Either @datatype or @language may be non-NULL but not both.
 */
typedef struct {
  unsigned char *string;
  unsigned int string_len;

  raptor_uri *datatype;

  unsigned char *language;
  unsigned char language_len;
} raptor_term_literal_value;


/**
 * raptor_term_blank_value:
 * @string: literal string
 * @string_len: length of string
 *
 * Blank term value - this typedef exists solely for use in #raptor_term
 *
 */
typedef struct {
  unsigned char *string;
  unsigned int string_len;
} raptor_term_blank_value;


/**
 * raptor_term_value:
 * @uri: uri value when term type is #RAPTOR_TERM_TYPE_URI
 * @literal: literal value when term type is #RAPTOR_TERM_TYPE_LITERAL
 * @blank: blank value when term type is #RAPTOR_TERM_TYPE_BLANK
 *
 * Term value - this typedef exists solely for use in #raptor_term
 *
 **/
typedef union {
  raptor_uri *uri;

  raptor_term_literal_value literal;

  raptor_term_blank_value blank;
} raptor_term_value;


/**
 * raptor_term:
 * @world: world
 * @usage: usage reference count (if >0)
 * @type: term type
 * @value: term values per type
 *
 * An RDF statement term
 *
 */
typedef struct {
  raptor_world* world;

  int usage;

  raptor_term_type type;

  raptor_term_value value;

} raptor_term;


/**
 * raptor_statement:
 * @world: world pointer
 * @usage: usage count
 * @subject: statement subject
 * @predicate: statement predicate
 * @object: statement object
 * @graph: statement graph name (or NULL if not present)
 *
 * An RDF triple with optional graph name (quad)
 *
 * See #raptor_term for a description of how the fields may be used.
 * As returned by a parser statement_handler.
 */
typedef struct {
  raptor_world* world;
  int usage;
  raptor_term* subject;
  raptor_term* predicate;
  raptor_term* object;
  raptor_term* graph;
} raptor_statement;


/**
 * raptor_log_level:
 * @RAPTOR_LOG_LEVEL_NONE: Internal
 * @RAPTOR_LOG_LEVEL_TRACE: very fine-grained tracing messages information
 * @RAPTOR_LOG_LEVEL_DEBUG: fine-grained tracing messages suitable for debugging
 * @RAPTOR_LOG_LEVEL_INFO: coarse-grained information messages
 * @RAPTOR_LOG_LEVEL_WARN: warning messages of potentially harmful problems
 * @RAPTOR_LOG_LEVEL_ERROR: error messages where the application can continue
 * @RAPTOR_LOG_LEVEL_FATAL: fatal error message where the application will likely abort
 * @RAPTOR_LOG_LEVEL_LAST: Internal
 *
 * Log levels
 */
typedef enum {
  RAPTOR_LOG_LEVEL_NONE,
  RAPTOR_LOG_LEVEL_TRACE,
  RAPTOR_LOG_LEVEL_DEBUG,
  RAPTOR_LOG_LEVEL_INFO,
  RAPTOR_LOG_LEVEL_WARN,
  RAPTOR_LOG_LEVEL_ERROR,
  RAPTOR_LOG_LEVEL_FATAL,
  RAPTOR_LOG_LEVEL_LAST = RAPTOR_LOG_LEVEL_FATAL
} raptor_log_level;


/**
 * raptor_domain:
 * @RAPTOR_DOMAIN_IOSTREAM: I/O stream
 * @RAPTOR_DOMAIN_NAMESPACE: XML Namespace / namespace stack
 * @RAPTOR_DOMAIN_PARSER: RDF Parser
 * @RAPTOR_DOMAIN_QNAME: XML QName
 * @RAPTOR_DOMAIN_SAX2:  XML SAX2
 * @RAPTOR_DOMAIN_SERIALIZER: RDF Serializer
 * @RAPTOR_DOMAIN_TERM: RDF Term
 * @RAPTOR_DOMAIN_TURTLE_WRITER: Turtle Writer
 * @RAPTOR_DOMAIN_URI: RDF Uri
 * @RAPTOR_DOMAIN_WORLD: RDF world
 * @RAPTOR_DOMAIN_WWW: WWW
 * @RAPTOR_DOMAIN_XML_WRITER: XML Writer
 * @RAPTOR_DOMAIN_NONE: Internal
 * @RAPTOR_DOMAIN_LAST: Internal
 *
 * Log domain
 */
typedef enum {
  RAPTOR_DOMAIN_NONE,
  RAPTOR_DOMAIN_IOSTREAM,
  RAPTOR_DOMAIN_NAMESPACE,
  RAPTOR_DOMAIN_PARSER,
  RAPTOR_DOMAIN_QNAME,
  RAPTOR_DOMAIN_SAX2,
  RAPTOR_DOMAIN_SERIALIZER,
  RAPTOR_DOMAIN_TERM,
  RAPTOR_DOMAIN_TURTLE_WRITER,
  RAPTOR_DOMAIN_URI,
  RAPTOR_DOMAIN_WORLD,
  RAPTOR_DOMAIN_WWW,
  RAPTOR_DOMAIN_XML_WRITER,
  RAPTOR_DOMAIN_LAST = RAPTOR_DOMAIN_XML_WRITER
} raptor_domain;


/**
 * raptor_log_message:
 * @code: error code or < 0 if not used or known
 * @domain: message domain or #RAPTOR_DOMAIN_NONE if not used or known
 * @level: log message level
 * @locator: location associated with message or NULL if not known
 * @text: message string
 *
 * Log message.
 */
typedef struct {
  int code;
  raptor_domain domain;
  raptor_log_level level;
  raptor_locator *locator;
  const char *text;
} raptor_log_message;


/**
 * raptor_log_handler:
 * @user_data: user data
 * @message: log message
 *
 * Handler function for log messages with location
 *
 * Used during parsing and serializing for errors and warnings that
 * may include location information. Handlers may be set
 * by raptor_world_set_log_handler().
 *
 */
typedef void (*raptor_log_handler)(void *user_data, raptor_log_message *message);


/**
 * raptor_statement_handler:
 * @user_data: user data
 * @statement: statement to report
 *
 * Statement (triple) reporting handler function.
 *
 * This handler function set with
 * raptor_parser_set_statement_handler() on a parser receives
 * statements as the parsing proceeds. The @statement argument to the
 * handler is shared and must be copied by the caller with
 * raptor_statement_copy().
 */
typedef void (*raptor_statement_handler)(void *user_data, raptor_statement *statement);

/**
 * raptor_graph_mark_flags:
 * @RAPTOR_GRAPH_MARK_START: mark is start of graph (otherwise is end)
 * @RAPTOR_GRAPH_MARK_DECLARED: mark was declared in syntax rather than implict
 *
 * Graph mark handler bitmask flags
 */
typedef enum {
  RAPTOR_GRAPH_MARK_START = 1,
  RAPTOR_GRAPH_MARK_DECLARED = 2
} raptor_graph_mark_flags;


/**
 * raptor_graph_mark_handler:
 * @user_data: user data
 * @graph: graph to report, NULL for the default graph
 * @flags: bitmask of #raptor_graph_mark_flags flags
 *
 * Graph start/end mark handler function.
 *
 * Records start and end of graphs happening in a stream of generated
 * #raptor_statement via the statement handler.  The callback starts a
 * graph when @flags has #RAPTOR_GRAPH_MARK_START bit set.
 *
 * The start and ends may be either declared in the syntax via some
 * keyword or mechanism such as TRiG {} syntax when @flags has bit
 * #RAPTOR_GRAPH_MARK_DECLARED set, or be implied by the start/end of
 * the data in other syntaxes, and the bit will be unset.
 */
typedef void (*raptor_graph_mark_handler)(void *user_data, raptor_uri *graph, int flags);

/**
 * raptor_generate_bnodeid_handler:
 * @user_data: user data
 * @user_bnodeid: a user-specified ID or NULL if none available.
 *
 * Generate a blank node identifier handler function.
 *
 * Return value: new blank node ID to use
 */
typedef unsigned char* (*raptor_generate_bnodeid_handler)(void *user_data, unsigned char* user_bnodeid);

/**
 * raptor_namespace_handler:
 * @user_data: user data
 * @nspace: #raptor_namespace declared
 *
 * XML Namespace declaration reporting handler set by
 * raptor_parser_set_namespace_handler().
 */
typedef void (*raptor_namespace_handler)(void* user_data, raptor_namespace *nspace);


/**
 * raptor_www_write_bytes_handler:
 * @www: WWW object
 * @userdata: user data
 * @ptr: data pointer
 * @size: size of individual item
 * @nmemb: number of items
 *
 * Receiving bytes of data from WWW retrieval handler.
 *
 * Set by raptor_www_set_write_bytes_handler().
 */
typedef void (*raptor_www_write_bytes_handler)(raptor_www* www, void *userdata, const void *ptr, size_t size, size_t nmemb);

/**
 * raptor_www_content_type_handler:
 * @www: WWW object
 * @userdata: user data
 * @content_type: content type seen
 *
 * Receiving Content-Type: header from WWW retrieval handler.
 *
 * Set by raptor_www_set_content_type_handler().
 */
typedef void (*raptor_www_content_type_handler)(raptor_www* www, void *userdata, const char *content_type);

/**
 * raptor_www_final_uri_handler:
 * @www: WWW object
 * @userdata: user data
 * @final_uri: final URI seen
 *
 * Receiving the final resolved URI from a WWW retrieval
 *
 * Set by raptor_www_set_final_uri_handler().
 */
typedef void (*raptor_www_final_uri_handler)(raptor_www* www, void *userdata, raptor_uri *final_uri);

/**
 * raptor_uri_filter_func:
 * @user_data: user data
 * @uri: #raptor_uri URI to check
 *
 * Callback function for #raptor_www_set_uri_filter
 *
 * Return value: non-0 to filter the URI
 */
typedef int (*raptor_uri_filter_func)(void *user_data, raptor_uri* uri);


/**
 * raptor_world_flag:
 * @RAPTOR_WORLD_FLAG_LIBXML_GENERIC_ERROR_SAVE: if set (non-0 value) - save/restore the libxml generic error handler when raptor library initializes (default set)
 * @RAPTOR_WORLD_FLAG_LIBXML_STRUCTURED_ERROR_SAVE: if set (non-0 value) - save/restore the libxml structured error handler when raptor library terminates (default set)
 * @RAPTOR_WORLD_FLAG_URI_INTERNING: if set (non-0 value) - each URI is saved interned in-memory and reused (default set)
 * @RAPTOR_WORLD_FLAG_WWW_SKIP_INIT_FINISH: if set (non-0 value) the raptor will neither initialise or terminate the lower level WWW library.  Usually in raptor initialising either curl_global_init (for libcurl) are called and in raptor cleanup, curl_global_cleanup is called.   This flag allows the application finer control over these libraries such as setting other global options or potentially calling and terminating raptor several times.  It does mean that applications which use this call must do their own extra work in order to allocate and free all resources to the system.
 *
 * Raptor world flags
 *
 * These are used by raptor_world_set_flags() to control raptor-wide
 * options across classes.  These must be set before
 * raptor_world_open() is called explicitly or implicitly (by
 * creating a raptor object).  There is no enumeration function for
 * these flags because they are not user options and must be set
 * before the library is initialised.  For similar reasons, there is
 * no get function.
 *
 * If any libxml handler saving/restoring is enabled, any existing
 * handler and context is saved before parsing and restored
 * afterwards.  Otherwise, no saving/restoring is performed.
 *
 */
typedef enum {
  RAPTOR_WORLD_FLAG_LIBXML_GENERIC_ERROR_SAVE = 1,
  RAPTOR_WORLD_FLAG_LIBXML_STRUCTURED_ERROR_SAVE = 2,
  RAPTOR_WORLD_FLAG_URI_INTERNING = 3,
  RAPTOR_WORLD_FLAG_WWW_SKIP_INIT_FINISH = 4
} raptor_world_flag;


/**
 * raptor_data_compare_arg_handler:
 * @data1: first object
 * @data2: second object
 * @user_data: user data argument
 *
 * Function to compare two data objects with a user data argument
 *
 * Designed to be used with raptor_sort_r() and compatible functions
 * such as raptor_sequence_sort_r() which uses it.
 *
 * Return value: compare value <0 if @data1 is before @data2, =0 if equal, >0 if @data1 is after @data2
 */
typedef int (*raptor_data_compare_arg_handler)(const void *data1, const void *data2, void *user_data);


/**
 * raptor_data_compare_handler:
 * @data1: first data object
 * @data2: second data object
 *
 * Function to compare two data objects - signature like strcmp() and function pssed to qsort()
 *
 * Designed to be passed into generic data structure constructors
 * like raptor_new_avltree().
 *
 * Return value: compare value <0 if @data1 is before @data2, =0 if equal, >0 if @data1 is after @data2
 */
typedef int (*raptor_data_compare_handler)(const void* data1, const void* data2);


/**
 * raptor_data_malloc_handler:
 * @size: data size
 *
 * Typedef for a function to allocate memory - signature like malloc()
 *
 * Designed to be passed into constructors
 * like raptor_www_fetch_to_string
 *
 * Return value: pointer to newly allocated memory or NULL on failure
 */
typedef void* (*raptor_data_malloc_handler)(size_t size);


/**
 * raptor_data_free_handler:
 * @data: data object or NULL
 *
 * Typedef for function to free a data object - signature like free()
 *
 * Designed to be passed into generic data structure constructors
 * like raptor_new_avltree().  If @data is NULL, nothing should be done.
 */
typedef void (*raptor_data_free_handler)(void* data);


/**
 * raptor_data_context_free_handler:
 * @context: context data for the free function
 * @object: object to free
 *
 * Handler function for freeing a sequence item with a contextual pointer.
 *
 * Set by raptor_new_sequence_with_context().
*/
typedef void (*raptor_data_context_free_handler)(void* context, void* object);

/**
 * raptor_data_print_handler:
 * @object: object to print
 * @fh: FILE* to print to
 *
 * Handler function for printing an object to a stream.
 *
 * Set by raptor_new_sequence()
 *
 * Return value: non-0 on failure
 */
typedef int (*raptor_data_print_handler)(void *object, FILE *fh);

/**
 * raptor_data_context_print_handler:
 * @context: context data for the print function
 * @object: object to print
 * @fh: FILE* to print to
 *
 * Function function for printing an object with data context to a stream.
 *
 * Set by raptor_new_sequence_with_context()
 *
 * Return value: non-0 on failure
 */
typedef int (*raptor_data_context_print_handler)(void *context, void *object, FILE *fh);

/**
 * raptor_stringbuffer:
 *
 * Raptor string buffer class
 */
typedef struct raptor_stringbuffer_s raptor_stringbuffer;


/* Public functions */

#define raptor_new_world() raptor_new_world_internal(RAPTOR_VERSION)
/* The real target of the raptor_new_world() macro */
RAPTOR_API
raptor_world *raptor_new_world_internal(unsigned int version_decimal);
RAPTOR_API
int raptor_world_open(raptor_world* world);
RAPTOR_API
void raptor_free_world(raptor_world* world);
RAPTOR_API
int raptor_world_set_libxslt_security_preferences(raptor_world *world, void *security_preferences);
RAPTOR_API
int raptor_world_set_flag(raptor_world *world, raptor_world_flag flag, int value);
RAPTOR_API
int raptor_world_set_log_handler(raptor_world *world, void *user_data, raptor_log_handler handler);
RAPTOR_API
void raptor_world_set_generate_bnodeid_handler(raptor_world* world, void *user_data, raptor_generate_bnodeid_handler handler);
RAPTOR_API
unsigned char* raptor_world_generate_bnodeid(raptor_world *world);
RAPTOR_API
void raptor_world_set_generate_bnodeid_parameters(raptor_world* world, char *prefix, int base);
RAPTOR_API
const char* raptor_log_level_get_label(raptor_log_level level);
RAPTOR_API
const char* raptor_domain_get_label(raptor_domain domain);

/* Names */
RAPTOR_API
int raptor_world_is_parser_name(raptor_world* world, const char *name);
RAPTOR_API
const char* raptor_world_guess_parser_name(raptor_world* world, raptor_uri *uri, const char *mime_type, const unsigned char *buffer, size_t len, const unsigned char *identifier);
RAPTOR_API
int raptor_world_is_serializer_name(raptor_world* world, const char *name);

/* Syntax descriptions */
RAPTOR_API
int raptor_world_get_parsers_count(raptor_world* world);
RAPTOR_API
int raptor_world_get_serializers_count(raptor_world* world);
RAPTOR_API
const raptor_syntax_description* raptor_world_get_parser_description(raptor_world* world, unsigned int counter);
RAPTOR_API
const raptor_syntax_description* raptor_world_get_serializer_description(raptor_world* world, unsigned int counter);
RAPTOR_API
int raptor_syntax_description_validate(raptor_syntax_description* desc);

RAPTOR_API
raptor_option raptor_world_get_option_from_uri(raptor_world* world, raptor_uri *uri);


/* Term Class */
RAPTOR_API
raptor_term* raptor_new_term_from_uri(raptor_world* world, raptor_uri* uri);
RAPTOR_API
raptor_term* raptor_new_term_from_counted_uri_string(raptor_world* world,  const unsigned char *uri_string, size_t length);
RAPTOR_API
raptor_term* raptor_new_term_from_uri_string(raptor_world* world, const unsigned char *uri_string);
RAPTOR_API
raptor_term* raptor_new_term_from_literal(raptor_world* world, const unsigned char* literal, raptor_uri* datatype, const unsigned char* language);
RAPTOR_API
raptor_term* raptor_new_term_from_counted_literal(raptor_world* world, const unsigned char* literal, size_t literal_len, raptor_uri* datatype, const unsigned char* language, unsigned char language_len);
RAPTOR_API
raptor_term* raptor_new_term_from_blank(raptor_world* world, const unsigned char* blank);
RAPTOR_API
raptor_term* raptor_new_term_from_counted_blank(raptor_world* world, const unsigned char* blank, size_t length);
RAPTOR_API
raptor_term* raptor_new_term_from_counted_string(raptor_world* world, unsigned char* string, size_t length);
RAPTOR_API
raptor_term* raptor_term_copy(raptor_term* term);
RAPTOR_API
int raptor_term_compare(const raptor_term *t1, const raptor_term *t2);
RAPTOR_API
int raptor_term_equals(raptor_term* t1, raptor_term* t2);
RAPTOR_API
void raptor_free_term(raptor_term *term);

RAPTOR_API
unsigned char* raptor_term_to_counted_string(raptor_term *term, size_t* len_p);
RAPTOR_API
unsigned char* raptor_term_to_string(raptor_term *term);
RAPTOR_API
int raptor_term_escaped_write(const raptor_term *term, unsigned int flags, raptor_iostream* iostr);
RAPTOR_API RAPTOR_DEPRECATED
int raptor_term_ntriples_write(const raptor_term *term, raptor_iostream* iostr);
RAPTOR_API
int raptor_uri_turtle_write(raptor_world *world, raptor_iostream* iostr, raptor_uri* uri, raptor_namespace_stack *nstack, raptor_uri *base_uri);
RAPTOR_API
int raptor_term_turtle_write(raptor_iostream* iostr, raptor_term* term, raptor_namespace_stack *nstack, raptor_uri *base_uri);
RAPTOR_API
unsigned char* raptor_uri_to_turtle_counted_string(raptor_world *world, raptor_uri* uri, raptor_namespace_stack *nstack, raptor_uri *base_uri, size_t *len_p);
RAPTOR_API
unsigned char* raptor_uri_to_turtle_string(raptor_world *world, raptor_uri* uri, raptor_namespace_stack *nstack, raptor_uri *base_uri);
RAPTOR_API
unsigned char* raptor_term_to_turtle_counted_string(raptor_term* term, raptor_namespace_stack *nstack, raptor_uri *base_uri, size_t *len_p);
RAPTOR_API
unsigned char* raptor_term_to_turtle_string(raptor_term* term, raptor_namespace_stack *nstack, raptor_uri *base_uri);


/* Statement Class */
RAPTOR_API
void raptor_statement_init(raptor_statement *statement, raptor_world *world);
RAPTOR_API
void raptor_statement_clear(raptor_statement *statement);
RAPTOR_API
raptor_statement* raptor_new_statement(raptor_world *world);
RAPTOR_API
raptor_statement* raptor_new_statement_from_nodes(raptor_world* world, raptor_term *subject, raptor_term *predicate, raptor_term *object, raptor_term *graph);
RAPTOR_API
raptor_statement* raptor_statement_copy(raptor_statement *statement);
RAPTOR_API
void raptor_free_statement(raptor_statement *statement);

RAPTOR_API
int raptor_statement_print(const raptor_statement * statement, FILE *stream);
RAPTOR_API
int raptor_statement_print_as_ntriples(const raptor_statement * statement, FILE *stream);
RAPTOR_API
int raptor_statement_compare(const raptor_statement *s1, const raptor_statement *s2);
RAPTOR_API
int raptor_statement_equals(const raptor_statement* s1, const raptor_statement* s2);


/* Parser Class */
RAPTOR_API
raptor_parser* raptor_new_parser(raptor_world* world, const char *name);
RAPTOR_API
raptor_parser* raptor_new_parser_for_content(raptor_world* world, raptor_uri *uri, const char *mime_type, const unsigned char *buffer, size_t len, const unsigned char *identifier);
RAPTOR_API
void raptor_free_parser(raptor_parser* parser);

/* methods */

/* Handlers */
RAPTOR_API
void raptor_parser_set_statement_handler(raptor_parser* parser, void *user_data, raptor_statement_handler handler);
RAPTOR_API
void raptor_parser_set_graph_mark_handler(raptor_parser* parser, void *user_data, raptor_graph_mark_handler handler);
RAPTOR_API
void raptor_parser_set_namespace_handler(raptor_parser* parser, void *user_data, raptor_namespace_handler handler);
RAPTOR_API
void raptor_parser_set_uri_filter(raptor_parser* parser, raptor_uri_filter_func filter, void* user_data);
RAPTOR_API
raptor_locator* raptor_parser_get_locator(raptor_parser* rdf_parser);


/* Parsing functions */
RAPTOR_API
int raptor_parser_parse_start(raptor_parser *rdf_parser, raptor_uri *uri);
RAPTOR_API
int raptor_parser_parse_chunk(raptor_parser* rdf_parser, const unsigned char *buffer, size_t len, int is_end);
RAPTOR_API
int raptor_parser_parse_file_stream(raptor_parser* rdf_parser, FILE *stream, const char *filename, raptor_uri *base_uri);
RAPTOR_API
int raptor_parser_parse_file(raptor_parser* rdf_parser, raptor_uri *uri, raptor_uri *base_uri);
RAPTOR_API
int raptor_parser_parse_uri(raptor_parser* rdf_parser, raptor_uri *uri, raptor_uri *base_uri);
RAPTOR_API
int raptor_parser_parse_uri_with_connection(raptor_parser* rdf_parser, raptor_uri *uri, raptor_uri *base_uri, void *connection);
RAPTOR_API
int raptor_parser_parse_iostream(raptor_parser* rdf_parser, raptor_iostream *iostr, raptor_uri *base_uri);
RAPTOR_API
void raptor_parser_parse_abort(raptor_parser* rdf_parser);
RAPTOR_API
const char* raptor_parser_get_name(raptor_parser *rdf_parser);
RAPTOR_API
const raptor_syntax_description* raptor_parser_get_description(raptor_parser *rdf_parser);

/* parser option methods */
RAPTOR_API
int raptor_parser_set_option(raptor_parser *parser, raptor_option option, const char* string, int integer);
RAPTOR_API
int raptor_parser_get_option(raptor_parser *parser, raptor_option option, char** string_p, int* integer_p);

/* parser utility methods */
RAPTOR_API
const char* raptor_parser_get_accept_header(raptor_parser* rdf_parser);
RAPTOR_API
raptor_world* raptor_parser_get_world(raptor_parser* rdf_parser);
RAPTOR_API
raptor_uri* raptor_parser_get_graph(raptor_parser* rdf_parser);


/* Locator Class */
/* methods */
RAPTOR_API
int raptor_locator_print(raptor_locator* locator, FILE *stream);
RAPTOR_API
int raptor_locator_format(char *buffer, size_t length, raptor_locator* locator);
RAPTOR_API
int raptor_locator_line(raptor_locator *locator);
RAPTOR_API
int raptor_locator_column(raptor_locator *locator);
RAPTOR_API
int raptor_locator_byte(raptor_locator *locator);
RAPTOR_API
const char* raptor_locator_file(raptor_locator *locator);
RAPTOR_API
const char* raptor_locator_uri(raptor_locator *locator);


/* Serializer Class */
RAPTOR_API
raptor_serializer* raptor_new_serializer(raptor_world* world, const char *name);
RAPTOR_API
void raptor_free_serializer(raptor_serializer* rdf_serializer);

/* methods */
RAPTOR_API
int raptor_serializer_start_to_iostream(raptor_serializer *rdf_serializer, raptor_uri *uri, raptor_iostream *iostream);
RAPTOR_API
int raptor_serializer_start_to_filename(raptor_serializer *rdf_serializer, const char *filename);
RAPTOR_API
int raptor_serializer_start_to_string(raptor_serializer *rdf_serializer, raptor_uri *uri, void **string_p, size_t *length_p);
RAPTOR_API
int raptor_serializer_start_to_file_handle(raptor_serializer *rdf_serializer, raptor_uri *uri, FILE *fh);
RAPTOR_API
int raptor_serializer_set_namespace(raptor_serializer* rdf_serializer, raptor_uri *uri, const unsigned char *prefix);
RAPTOR_API
int raptor_serializer_set_namespace_from_namespace(raptor_serializer* rdf_serializer, raptor_namespace *nspace);
RAPTOR_API
int raptor_serializer_serialize_statement(raptor_serializer* rdf_serializer, raptor_statement *statement);
RAPTOR_API
int raptor_serializer_serialize_end(raptor_serializer *rdf_serializer);
RAPTOR_API
raptor_iostream* raptor_serializer_get_iostream(raptor_serializer *serializer);
RAPTOR_API
raptor_locator* raptor_serializer_get_locator(raptor_serializer *rdf_serializer);
RAPTOR_API
int raptor_serializer_flush(raptor_serializer *rdf_serializer);
RAPTOR_API
const raptor_syntax_description* raptor_serializer_get_description(raptor_serializer *rdf_serializer);

/* serializer option methods */
RAPTOR_API
int raptor_serializer_set_option(raptor_serializer *serializer, raptor_option option, const char* string, int integer);
RAPTOR_API
int raptor_serializer_get_option(raptor_serializer *serializer, raptor_option option, char** string_p, int* integer_p);

/* utility methods */
RAPTOR_API
raptor_world* raptor_serializer_get_world(raptor_serializer* rdf_serializer);


/* memory functions */
RAPTOR_API
void raptor_free_memory(void *ptr);
RAPTOR_API
void* raptor_alloc_memory(size_t size);
RAPTOR_API
void* raptor_calloc_memory(size_t nmemb, size_t size);


/* URI Class */
RAPTOR_API
raptor_uri* raptor_new_uri_from_counted_string(raptor_world* world, const unsigned char *uri_string, size_t length);
RAPTOR_API
raptor_uri* raptor_new_uri(raptor_world* world, const unsigned char *uri_string);
RAPTOR_API
raptor_uri* raptor_new_uri_from_uri_local_name(raptor_world* world, raptor_uri *uri, const unsigned char *local_name);
RAPTOR_API
raptor_uri* raptor_new_uri_relative_to_base(raptor_world* world, raptor_uri *base_uri, const unsigned char *uri_string);
RAPTOR_API
raptor_uri* raptor_new_uri_relative_to_base_counted(raptor_world* world, raptor_uri *base_uri, const unsigned char *uri_string, size_t uri_len);
RAPTOR_API
raptor_uri* raptor_new_uri_from_id(raptor_world* world, raptor_uri *base_uri, const unsigned char *id);
RAPTOR_API
raptor_uri* raptor_new_uri_from_uri_or_file_string(raptor_world* world, raptor_uri* base_uri, const unsigned char* uri_or_file_string);
RAPTOR_API
raptor_uri* raptor_new_uri_for_rdf_concept(raptor_world* world, const unsigned char *name);
RAPTOR_API
raptor_uri* raptor_new_uri_for_xmlbase(raptor_uri* old_uri);
RAPTOR_API
raptor_uri* raptor_new_uri_for_retrieval(raptor_uri* old_uri);
RAPTOR_API
void raptor_free_uri(raptor_uri *uri);

/* methods */
RAPTOR_API
int raptor_uri_equals(raptor_uri* uri1, raptor_uri* uri2);
RAPTOR_API
int raptor_uri_compare(raptor_uri* uri1, raptor_uri* uri2);
RAPTOR_API
raptor_uri* raptor_uri_copy(raptor_uri *uri);
RAPTOR_API
unsigned char* raptor_uri_as_string(raptor_uri *uri);
RAPTOR_API
unsigned char* raptor_uri_as_counted_string(raptor_uri *uri, size_t* len_p);
RAPTOR_API
unsigned char* raptor_uri_to_relative_counted_uri_string(raptor_uri *base_uri, raptor_uri *reference_uri, size_t *length_p);
RAPTOR_API
unsigned char* raptor_uri_to_relative_uri_string(raptor_uri *base_uri,  raptor_uri *reference_uri);
RAPTOR_API
int raptor_uri_print(const raptor_uri* uri, FILE *stream);
RAPTOR_API
unsigned char* raptor_uri_to_counted_string(raptor_uri *uri, size_t *len_p);
RAPTOR_API
unsigned char* raptor_uri_to_string(raptor_uri *uri);
RAPTOR_API
raptor_world* raptor_uri_get_world(raptor_uri *uri);
RAPTOR_API
int raptor_uri_file_exists(raptor_uri* uri);
RAPTOR_API
int raptor_uri_escaped_write(raptor_uri* uri, raptor_uri* base_uri, unsigned int flags, raptor_iostream *iostr);

/* XML utility functions */
RAPTOR_API
int raptor_xml_escape_string_any(raptor_world* world, const unsigned char *string, size_t len, unsigned char *buffer, size_t length, char quote, int xml_version);
RAPTOR_API
int raptor_xml_escape_string_any_write(const unsigned char *string, size_t len, char quote, int xml_version, raptor_iostream* iostr);
RAPTOR_API
int raptor_xml_escape_string(raptor_world *world, const unsigned char *string, size_t len, unsigned char *buffer, size_t length, char quote);
RAPTOR_API
int raptor_xml_escape_string_write(const unsigned char *string, size_t len, char quote, raptor_iostream* iostr);
RAPTOR_API
int raptor_xml_name_check(const unsigned char *string, size_t length, int xml_version);


/* portable vsnprintf utility function */
RAPTOR_API RAPTOR_DEPRECATED
char* raptor_vsnprintf(const char *format, va_list arguments) RAPTOR_PRINTF_FORMAT(1, 0);
RAPTOR_API
int raptor_vsnprintf2(char *buffer, size_t size, const char *format, va_list arguments) RAPTOR_PRINTF_FORMAT(3, 0);
RAPTOR_API
int raptor_snprintf(char *buffer, size_t size, const char *format, ...) RAPTOR_PRINTF_FORMAT(3, 4);
RAPTOR_API
int raptor_vasprintf(char **ret, const char *format, va_list arguments) RAPTOR_PRINTF_FORMAT(2, 0);

/* RFC2396 URI resolving functions */
RAPTOR_API
size_t raptor_uri_resolve_uri_reference(const unsigned char *base_uri, const unsigned char *reference_uri, unsigned char* buffer, size_t length);

/* URI String utility functions */
RAPTOR_API
unsigned char* raptor_uri_counted_filename_to_uri_string(const char *filename, size_t filename_len);
RAPTOR_API
unsigned char* raptor_uri_filename_to_uri_string(const char *filename);
RAPTOR_API
int raptor_uri_filename_exists(const unsigned char* path);
RAPTOR_API
char* raptor_uri_uri_string_to_filename(const unsigned char *uri_string);
RAPTOR_API
char* raptor_uri_uri_string_to_filename_fragment(const unsigned char *uri_string, unsigned char **fragment_p);
RAPTOR_API
int raptor_uri_uri_string_is_file_uri(const unsigned char* uri_string);
RAPTOR_API
int raptor_stringbuffer_append_uri_escaped_counted_string(raptor_stringbuffer* sb, const char* string, size_t length, int space_is_plus);
RAPTOR_API
char* raptor_uri_uri_string_to_counted_filename_fragment(const unsigned char *uri_string, size_t* len_p, unsigned char **fragment_p, size_t* fragment_len_p);
RAPTOR_API
int raptor_uri_uri_string_is_absolute(const unsigned char* uri_string);


/**
 * RAPTOR_RDF_MS_URI:
 *
 * RDF Namespace URI (rdf:).
 *
 * Copy with raptor_uri_copy() to use.
 */
#define RAPTOR_RDF_MS_URI raptor_rdf_namespace_uri

/**
 * RAPTOR_RDF_SCHEMA_URI:
 *
 * RDF Schema Namespace URI (rdfs:).
 *
 * Copy with raptor_uri_copy() to use.
 */
#define RAPTOR_RDF_SCHEMA_URI raptor_rdf_schema_namespace_uri

/**
 * RAPTOR_XMLSCHEMA_DATATYPES_URI:
 *
 * XML Schema Datatypes URI (xsd:).
 *
 * Copy with raptor_uri_copy() to use.
 */
#define RAPTOR_XMLSCHEMA_DATATYPES_URI raptor_xmlschema_datatypes_namespace_uri

/**
 * RAPTOR_OWL_URI:
 *
 * OWL Namespace URI (owl:).
 *
 * Copy with raptor_uri_copy() to use.
 */
#define RAPTOR_OWL_URI raptor_owl_namespace_uri


/* raptor_www */
RAPTOR_API
raptor_www* raptor_new_www(raptor_world* world);
RAPTOR_API
raptor_www* raptor_new_www_with_connection(raptor_world* world, void* connection);
RAPTOR_API
void raptor_free_www(raptor_www *www);
RAPTOR_API
int raptor_www_set_ssl_cert_options(raptor_www* www, const char* cert_filename, const char* cert_type, const char* cert_passphrase);
RAPTOR_API
int raptor_www_set_ssl_verify_options(raptor_www* www, int verify_peer, int verify_host);
RAPTOR_API
void raptor_www_set_user_agent(raptor_www *www, const char *user_agent);
RAPTOR_API
void raptor_www_set_proxy(raptor_www *www, const char *proxy);
RAPTOR_API
void raptor_www_set_http_accept(raptor_www *www, const char *value);
RAPTOR_API
void raptor_www_set_write_bytes_handler(raptor_www *www, raptor_www_write_bytes_handler handler, void *user_data);
RAPTOR_API
void raptor_www_set_content_type_handler(raptor_www *www, raptor_www_content_type_handler handler, void *user_data);
RAPTOR_API
void raptor_www_set_final_uri_handler(raptor_www* www, raptor_www_final_uri_handler handler, void *user_data);
RAPTOR_API
void raptor_www_set_uri_filter(raptor_www* www, raptor_uri_filter_func filter, void* user_data);
RAPTOR_API
void raptor_www_set_connection_timeout(raptor_www* www, int timeout);
RAPTOR_API
int raptor_www_set_http_cache_control(raptor_www* www, const char* cache_control);
RAPTOR_API
int raptor_www_fetch(raptor_www *www, raptor_uri *uri);
RAPTOR_API
int raptor_www_fetch_to_string(raptor_www *www, raptor_uri *uri, void **string_p, size_t *length_p, raptor_data_malloc_handler const malloc_handler);
RAPTOR_API
void* raptor_www_get_connection(raptor_www *www);
RAPTOR_API
void raptor_www_abort(raptor_www *www, const char *reason);
RAPTOR_API
raptor_uri* raptor_www_get_final_uri(raptor_www* www);


/* XML QNames Class */
RAPTOR_API
raptor_qname* raptor_new_qname(raptor_namespace_stack *nstack, const unsigned char *name, const unsigned char *value);
RAPTOR_API
raptor_qname* raptor_new_qname_from_namespace_local_name(raptor_world* world, raptor_namespace *ns, const unsigned char *local_name, const unsigned char *value);

/* methods */
RAPTOR_API
raptor_qname* raptor_qname_copy(raptor_qname *qname);
RAPTOR_API
void raptor_free_qname(raptor_qname* name);
RAPTOR_API
int raptor_qname_equal(raptor_qname *name1, raptor_qname *name2);
RAPTOR_API
unsigned char* raptor_qname_to_counted_name(raptor_qname *qname, size_t* length_p);
RAPTOR_API
const raptor_namespace* raptor_qname_get_namespace(raptor_qname* name);
RAPTOR_API
const unsigned char* raptor_qname_get_local_name(raptor_qname* name);
RAPTOR_API
const unsigned char* raptor_qname_get_value(raptor_qname* name);
RAPTOR_API
const unsigned char* raptor_qname_get_counted_value(raptor_qname* name, size_t* length_p);
RAPTOR_API
int raptor_qname_write(raptor_qname *qname, raptor_iostream* iostr);

/* QName String utility functions */
RAPTOR_API
raptor_uri* raptor_qname_string_to_uri(raptor_namespace_stack *nstack,  const unsigned char *name, size_t name_len);
RAPTOR_API
unsigned char* raptor_qname_format_as_xml(const raptor_qname *qname, size_t *length_p);

/* XML Namespaces Stack class */
RAPTOR_API
raptor_namespace* raptor_new_namespace_from_uri(raptor_namespace_stack *nstack, const unsigned char *prefix,  raptor_uri* ns_uri, int depth);
RAPTOR_API
raptor_namespace_stack* raptor_new_namespaces(raptor_world* world, int defaults);
RAPTOR_API
int raptor_namespaces_init(raptor_world* world, raptor_namespace_stack *nstack, int defaults);
RAPTOR_API
void raptor_namespaces_clear(raptor_namespace_stack *nstack);
RAPTOR_API
void raptor_free_namespaces(raptor_namespace_stack *nstack);

/* methods */
RAPTOR_API
void raptor_namespaces_start_namespace(raptor_namespace_stack *nstack, raptor_namespace *nspace);
RAPTOR_API
int raptor_namespaces_start_namespace_full(raptor_namespace_stack *nstack, const unsigned char *prefix, const unsigned char *ns_uri_string, int depth);
RAPTOR_API
void raptor_namespaces_end_for_depth(raptor_namespace_stack *nstack, int depth);
RAPTOR_API
raptor_namespace* raptor_namespaces_get_default_namespace(raptor_namespace_stack *nstack);
RAPTOR_API
raptor_namespace* raptor_namespaces_find_namespace(raptor_namespace_stack *nstack, const unsigned char *prefix, int prefix_length);
RAPTOR_API
raptor_namespace* raptor_namespaces_find_namespace_by_uri(raptor_namespace_stack *nstack, raptor_uri *ns_uri);
RAPTOR_API
int raptor_namespaces_namespace_in_scope(raptor_namespace_stack *nstack, const raptor_namespace *nspace);
RAPTOR_API
raptor_qname* raptor_new_qname_from_namespace_uri(raptor_namespace_stack *nstack,  raptor_uri *uri, int xml_version);


/* XML Namespace Class */
RAPTOR_API
raptor_namespace* raptor_new_namespace(raptor_namespace_stack *nstack, const unsigned char *prefix, const unsigned char *ns_uri_string, int depth);
RAPTOR_API
void raptor_free_namespace(raptor_namespace *ns);
RAPTOR_API
int raptor_namespace_stack_start_namespace(raptor_namespace_stack *nstack, raptor_namespace *ns, int new_depth);
RAPTOR_API
raptor_uri* raptor_namespace_get_uri(const raptor_namespace *ns);
RAPTOR_API
const unsigned char* raptor_namespace_get_prefix(const raptor_namespace *ns);
RAPTOR_API
const unsigned char* raptor_namespace_get_counted_prefix(const raptor_namespace *ns, size_t *length_p);
RAPTOR_API
unsigned char* raptor_namespace_format_as_xml(const raptor_namespace *ns, size_t *length_p);
RAPTOR_API
int raptor_namespace_write(raptor_namespace *ns, raptor_iostream* iostr);

/* namespace string utility function */
RAPTOR_API
int raptor_xml_namespace_string_parse(const unsigned char *string, unsigned char **prefix, unsigned char **uri_string);

/* Sequence class */
/**
 * raptor_sequence:
 *
 * Raptor sequence class
 */
typedef struct raptor_sequence_s raptor_sequence;

/* Sequence Class */
RAPTOR_API
raptor_sequence* raptor_new_sequence(raptor_data_free_handler free_handler, raptor_data_print_handler print_handler);
RAPTOR_API
raptor_sequence* raptor_new_sequence_with_context(raptor_data_context_free_handler free_handler, raptor_data_context_print_handler print_handler, void* handler_context);
RAPTOR_API
void raptor_free_sequence(raptor_sequence* seq);

/* methods */
RAPTOR_API
int raptor_sequence_size(raptor_sequence* seq);
RAPTOR_API
int raptor_sequence_set_at(raptor_sequence* seq, int idx, void *data);
RAPTOR_API
int raptor_sequence_push(raptor_sequence* seq, void *data);
RAPTOR_API
int raptor_sequence_shift(raptor_sequence* seq, void *data);
RAPTOR_API
void* raptor_sequence_get_at(raptor_sequence* seq, int idx);
RAPTOR_API
void* raptor_sequence_pop(raptor_sequence* seq);
RAPTOR_API
void* raptor_sequence_unshift(raptor_sequence* seq);
RAPTOR_API
void* raptor_sequence_delete_at(raptor_sequence* seq, int idx);

RAPTOR_API
void raptor_sequence_sort(raptor_sequence* seq, raptor_data_compare_handler compare);
RAPTOR_API
void raptor_sequence_sort_r(raptor_sequence* seq, raptor_data_compare_arg_handler compare, void* user_data);
RAPTOR_API
int raptor_sequence_swap(raptor_sequence* seq, int i, int j);
RAPTOR_API
int raptor_sequence_reverse(raptor_sequence* seq, int start_index, int length);
RAPTOR_API
int raptor_sequence_next_permutation(raptor_sequence *seq, raptor_data_compare_handler compare);

/* helper for printing sequences of strings */
RAPTOR_API
int raptor_sequence_print(raptor_sequence* seq, FILE* fh);
RAPTOR_API
int raptor_sequence_join(raptor_sequence* dest, raptor_sequence *src);


/* Unicode and UTF8 */

/**
 * raptor_unichar:
 *
 * raptor Unicode codepoint
 */
typedef unsigned long raptor_unichar;
RAPTOR_API
int raptor_unicode_utf8_string_put_char(raptor_unichar c, unsigned char *output, size_t length);
RAPTOR_API
int raptor_unicode_utf8_string_get_char(const unsigned char *input, size_t length, raptor_unichar *output);
RAPTOR_API
int raptor_unicode_is_xml11_namestartchar(raptor_unichar c);
RAPTOR_API
int raptor_unicode_is_xml10_namestartchar(raptor_unichar c);
RAPTOR_API
int raptor_unicode_is_xml11_namechar(raptor_unichar c);
RAPTOR_API
int raptor_unicode_is_xml10_namechar(raptor_unichar c);
RAPTOR_API
int raptor_unicode_check_utf8_string(const unsigned char *string, size_t length);
RAPTOR_API
int raptor_unicode_utf8_strlen(const unsigned char *string, size_t length);
RAPTOR_API
size_t raptor_unicode_utf8_substr(unsigned char* dest, size_t* dest_length_p, const unsigned char* src, size_t src_length, int startingLoc, int length);

/* Stringbuffer Class */
RAPTOR_API
raptor_stringbuffer* raptor_new_stringbuffer(void);
RAPTOR_API
void raptor_free_stringbuffer(raptor_stringbuffer *stringbuffer);

/* methods */
RAPTOR_API
int raptor_stringbuffer_append_counted_string(raptor_stringbuffer* stringbuffer, const unsigned char *string, size_t length, int do_copy);
RAPTOR_API
int raptor_stringbuffer_append_string(raptor_stringbuffer* stringbuffer, const unsigned char *string, int do_copy);
RAPTOR_API
int raptor_stringbuffer_append_decimal(raptor_stringbuffer* stringbuffer, int integer);
RAPTOR_API
int raptor_stringbuffer_append_hexadecimal(raptor_stringbuffer* stringbuffer, int hex);
RAPTOR_API
int raptor_stringbuffer_append_stringbuffer(raptor_stringbuffer* stringbuffer, raptor_stringbuffer* append);
RAPTOR_API
int raptor_stringbuffer_prepend_counted_string(raptor_stringbuffer* stringbuffer, const unsigned char *string, size_t length, int do_copy);
RAPTOR_API
int raptor_stringbuffer_prepend_string(raptor_stringbuffer* stringbuffer, const unsigned char *string, int do_copy);
RAPTOR_API
unsigned char* raptor_stringbuffer_as_string(raptor_stringbuffer* stringbuffer);
RAPTOR_API
size_t raptor_stringbuffer_length(raptor_stringbuffer* stringbuffer);
RAPTOR_API
int raptor_stringbuffer_copy_to_string(raptor_stringbuffer* stringbuffer, unsigned char *string, size_t length);

/**
 * raptor_iostream_init_func:
 * @context: stream context data
 *
 * Handler function for #raptor_iostream initialising.
 *
 * Return value: non-0 on failure.
 */
typedef int (*raptor_iostream_init_func) (void *context);

/**
 * raptor_iostream_finish_func:
 * @context: stream context data
 *
 * Handler function for #raptor_iostream terminating.
 *
 */
typedef void (*raptor_iostream_finish_func) (void *context);

/**
 * raptor_iostream_write_byte_func
 * @context: stream context data
 * @byte: byte to write
 *
 * Handler function for implementing raptor_iostream_write_byte().
 *
 * Return value: non-0 on failure.
 */
typedef int (*raptor_iostream_write_byte_func) (void *context, const int byte);

/**
 * raptor_iostream_write_bytes_func:
 * @context: stream context data
 * @ptr: pointer to bytes to write
 * @size: size of item
 * @nmemb: number of items
 *
 * Handler function for implementing raptor_iostream_write_bytes().
 *
 * Return value: non-0 on failure.
 */
typedef int (*raptor_iostream_write_bytes_func) (void *context, const void *ptr, size_t size, size_t nmemb);

/**
 * raptor_iostream_write_end_func:
 * @context: stream context data
 *
 * Handler function for implementing raptor_iostream_write_end().
 *
 * Return value: non-0 on failure.
 */
typedef int (*raptor_iostream_write_end_func) (void *context);

/**
 * raptor_iostream_read_bytes_func:
 * @context: stream context data
 * @ptr: pointer to buffer to read into
 * @size: size of buffer
 * @nmemb: number of items
 *
 * Handler function for implementing raptor_iostream_read_bytes().
 *
 * Return value: number of items read, 0 or < @size on EOF, <0 on failure
 */
typedef int (*raptor_iostream_read_bytes_func) (void *context, void *ptr, size_t size, size_t nmemb);

/**
 * raptor_iostream_read_eof_func:
 * @context: stream context data
 *
 * Handler function for implementing raptor_iostream_read_eof().
 *
 * Return value: non-0 if EOF
 */
typedef int (*raptor_iostream_read_eof_func) (void *context);

/**
 * raptor_iostream_handler:
 * @version: interface version.  Presently 1 or 2.
 * @init:  initialisation handler - optional, called at most once (V1)
 * @finish: finishing handler -  optional, called at most once (V1)
 * @write_byte: write byte handler - required (for writing) (V1)
 * @write_bytes: write bytes handler - required (for writing) (V1)
 * @write_end: write end handler - optional (for writing), called at most once (V1)
 * @read_bytes: read bytes handler - required (for reading) (V2)
 * @read_eof: read EOF handler - required (for reading) (V2)
 *
 * I/O stream implementation handler structure.
 *
 */
typedef struct {
  int version;

  /* V1 functions */
  raptor_iostream_init_func         init;
  raptor_iostream_finish_func       finish;
  raptor_iostream_write_byte_func   write_byte;
  raptor_iostream_write_bytes_func  write_bytes;
  raptor_iostream_write_end_func    write_end;

  /* V2 functions */
  raptor_iostream_read_bytes_func   read_bytes;
  raptor_iostream_read_eof_func     read_eof;
} raptor_iostream_handler;


/* I/O Stream Class */
RAPTOR_API
raptor_iostream* raptor_new_iostream_from_handler(raptor_world* world, void *user_data, const raptor_iostream_handler* const handler);
RAPTOR_API
raptor_iostream* raptor_new_iostream_to_sink(raptor_world* world);
RAPTOR_API
raptor_iostream* raptor_new_iostream_to_filename(raptor_world* world, const char *filename);
RAPTOR_API
raptor_iostream* raptor_new_iostream_to_file_handle(raptor_world* world, FILE *handle);
RAPTOR_API
raptor_iostream* raptor_new_iostream_to_string(raptor_world* world, void **string_p, size_t *length_p, raptor_data_malloc_handler const malloc_handler);
RAPTOR_API
raptor_iostream* raptor_new_iostream_from_sink(raptor_world* world);
RAPTOR_API
raptor_iostream* raptor_new_iostream_from_filename(raptor_world* world, const char *filename);
RAPTOR_API
raptor_iostream* raptor_new_iostream_from_file_handle(raptor_world* world, FILE *handle);
RAPTOR_API
raptor_iostream* raptor_new_iostream_from_string(raptor_world* world, void *string, size_t length);
RAPTOR_API
void raptor_free_iostream(raptor_iostream *iostr);

RAPTOR_API
int raptor_iostream_write_bytes(const void *ptr, size_t size, size_t nmemb, raptor_iostream *iostr);
RAPTOR_API
int raptor_iostream_write_byte(const int byte, raptor_iostream *iostr);
RAPTOR_API
int raptor_iostream_write_end(raptor_iostream *iostr);
RAPTOR_API
int raptor_iostream_string_write(const void *string, raptor_iostream *iostr);
RAPTOR_API
int raptor_iostream_counted_string_write(const void *string, size_t len, raptor_iostream *iostr);
RAPTOR_API
unsigned long raptor_iostream_tell(raptor_iostream *iostr);
RAPTOR_API
int raptor_iostream_decimal_write(int integer, raptor_iostream* iostr);
RAPTOR_API
int raptor_iostream_hexadecimal_write(unsigned int integer, int width, raptor_iostream* iostr);
RAPTOR_API
int raptor_stringbuffer_write(raptor_stringbuffer *sb, raptor_iostream* iostr);
RAPTOR_API
int raptor_uri_write(raptor_uri *uri, raptor_iostream *iostr);
RAPTOR_API
int raptor_iostream_read_bytes(void *ptr, size_t size, size_t nmemb, raptor_iostream* iostr);
RAPTOR_API
int raptor_iostream_read_eof(raptor_iostream *iostr);

/* I/O Stream utility functions */

/**
 * raptor_escaped_write_bitflags:
 * @RAPTOR_ESCAPED_WRITE_BITFLAG_BS_ESCAPES_BF   : Allow \b \f,
 * @RAPTOR_ESCAPED_WRITE_BITFLAG_BS_ESCAPES_TNRU : ALlow \t \n \r \u
 * @RAPTOR_ESCAPED_WRITE_BITFLAG_UTF8            : Allow UTF-8 for printable U *
 * @RAPTOR_ESCAPED_WRITE_BITFLAG_SPARQL_URI_ESCAPES: Must escape #x00-#x20<>\"{}|^` in URIs
 * @RAPTOR_ESCAPED_WRITE_NTRIPLES_LITERAL: N-Triples literal
 * @RAPTOR_ESCAPED_WRITE_NTRIPLES_URI: N-Triples URI
 * @RAPTOR_ESCAPED_WRITE_SPARQL_LITERAL: SPARQL literal: allows raw UTF8 for printable literals
 * @RAPTOR_ESCAPED_WRITE_SPARQL_LONG_LITERAL: SPARQL long literal: no BS-escapes allowed
 * @RAPTOR_ESCAPED_WRITE_SPARQL_URI: SPARQL uri: have to escape certain characters
 * @RAPTOR_ESCAPED_WRITE_TURTLE_URI: Turtle 2013 URIs (like SPARQL)
 * @RAPTOR_ESCAPED_WRITE_TURTLE_LITERAL: Turtle 2013 literals (like SPARQL)
 * @RAPTOR_ESCAPED_WRITE_TURTLE_LONG_LITERAL: Turtle 2013 long literals (like SPARQL)
 * @RAPTOR_ESCAPED_WRITE_JSON_LITERAL: JSON literals: \b \f \t \r \n and \u \U
 *
 * Bit flags for raptor_string_escaped_write() and friends.
 */
typedef enum {
  RAPTOR_ESCAPED_WRITE_BITFLAG_BS_ESCAPES_BF      = 1,
  RAPTOR_ESCAPED_WRITE_BITFLAG_BS_ESCAPES_TNRU    = 2,
  RAPTOR_ESCAPED_WRITE_BITFLAG_UTF8               = 4,
  RAPTOR_ESCAPED_WRITE_BITFLAG_SPARQL_URI_ESCAPES = 8,

  /* N-Triples - favour writing \u, \U over UTF8 */
  RAPTOR_ESCAPED_WRITE_NTRIPLES_LITERAL = RAPTOR_ESCAPED_WRITE_BITFLAG_BS_ESCAPES_TNRU | RAPTOR_ESCAPED_WRITE_BITFLAG_BS_ESCAPES_BF,
  RAPTOR_ESCAPED_WRITE_NTRIPLES_URI     = RAPTOR_ESCAPED_WRITE_BITFLAG_SPARQL_URI_ESCAPES,

  /* SPARQL literal: allows raw UTF8 for printable literals */
  RAPTOR_ESCAPED_WRITE_SPARQL_LITERAL = RAPTOR_ESCAPED_WRITE_BITFLAG_UTF8,

  /* SPARQL long literal: no BS-escapes allowed */
  RAPTOR_ESCAPED_WRITE_SPARQL_LONG_LITERAL = RAPTOR_ESCAPED_WRITE_BITFLAG_UTF8,

  /* SPARQL uri: have to escape certain characters */
  RAPTOR_ESCAPED_WRITE_SPARQL_URI     = RAPTOR_ESCAPED_WRITE_BITFLAG_UTF8 | RAPTOR_ESCAPED_WRITE_BITFLAG_SPARQL_URI_ESCAPES,

  /* Turtle (2013) escapes are like SPARQL */
  RAPTOR_ESCAPED_WRITE_TURTLE_URI     = RAPTOR_ESCAPED_WRITE_SPARQL_URI,
  RAPTOR_ESCAPED_WRITE_TURTLE_LITERAL = RAPTOR_ESCAPED_WRITE_SPARQL_LITERAL,
  RAPTOR_ESCAPED_WRITE_TURTLE_LONG_LITERAL = RAPTOR_ESCAPED_WRITE_SPARQL_LONG_LITERAL,

  /* JSON literals: \b \f \t \r \n and \u \U */
  RAPTOR_ESCAPED_WRITE_JSON_LITERAL = RAPTOR_ESCAPED_WRITE_BITFLAG_BS_ESCAPES_TNRU | RAPTOR_ESCAPED_WRITE_BITFLAG_BS_ESCAPES_BF
} raptor_escaped_write_bitflags;


RAPTOR_API
int raptor_string_ntriples_write(const unsigned char *string, size_t len, const char delim, raptor_iostream *iostr);
RAPTOR_API
int raptor_bnodeid_ntriples_write(const unsigned char *bnodeid, size_t len, raptor_iostream *iostr);
RAPTOR_API RAPTOR_DEPRECATED
int raptor_string_python_write(const unsigned char *string, size_t len, const char delim, unsigned int mode, raptor_iostream *iostr);
RAPTOR_API
int raptor_statement_ntriples_write(const raptor_statement *statement, raptor_iostream* iostr, int write_graph_term);
RAPTOR_API
int raptor_string_escaped_write(const unsigned char *string, size_t len, const char delim, unsigned int flags, raptor_iostream *iostr);


/* Parser and Serializer options */

/**
 * raptor_option_value_type:
 * @RAPTOR_OPTION_VALUE_TYPE_BOOL: Boolean integer value. Non-0 is true
 * @RAPTOR_OPTION_VALUE_TYPE_INT: Decimal integer value
 * @RAPTOR_OPTION_VALUE_TYPE_STRING: String value
 * @RAPTOR_OPTION_VALUE_TYPE_URI: URI String value.
 * @RAPTOR_OPTION_VALUE_TYPE_LAST: internal
 *
 * Option value types.
 */
typedef enum {
  RAPTOR_OPTION_VALUE_TYPE_BOOL,
  RAPTOR_OPTION_VALUE_TYPE_INT,
  RAPTOR_OPTION_VALUE_TYPE_STRING,
  RAPTOR_OPTION_VALUE_TYPE_URI,
  RAPTOR_OPTION_VALUE_TYPE_LAST = RAPTOR_OPTION_VALUE_TYPE_URI
} raptor_option_value_type;


/**
 * raptor_option_description:
 * @domain: domain ID
 * @option: option ID
 * @value_type: data type of option value
 * @name: short name for option
 * @name_len: length of @name
 * @label: description of option
 * @uri: URI identifying option
 *
 * Description of an option for a domain.
 */
typedef struct {
  raptor_domain domain;
  raptor_option option;
  raptor_option_value_type value_type;
  const char* name;
  size_t name_len;
  const char* label;
  raptor_uri* uri;
} raptor_option_description;


RAPTOR_API
unsigned int raptor_option_get_count(void);
RAPTOR_API
const char* raptor_option_get_value_type_label(const raptor_option_value_type type);
RAPTOR_API
void raptor_free_option_description(raptor_option_description* option_description);
RAPTOR_API
raptor_option_description* raptor_world_get_option_description(raptor_world* world, const raptor_domain domain, const raptor_option option);


/* SAX2 element Class (raptor_xml_element) */
RAPTOR_API
raptor_xml_element* raptor_new_xml_element(raptor_qname* name, const unsigned char* xml_language, raptor_uri* xml_base);
RAPTOR_API
raptor_xml_element* raptor_new_xml_element_from_namespace_local_name(raptor_namespace *ns, const unsigned char *name, const unsigned char *xml_language, raptor_uri *xml_base);
RAPTOR_API
void raptor_free_xml_element(raptor_xml_element *element);

/* methods */
RAPTOR_API
raptor_qname* raptor_xml_element_get_name(raptor_xml_element *xml_element);
RAPTOR_API
void raptor_xml_element_set_attributes(raptor_xml_element* xml_element, raptor_qname **attributes, int count);
RAPTOR_API
raptor_qname** raptor_xml_element_get_attributes(raptor_xml_element* xml_element);
RAPTOR_API
int raptor_xml_element_get_attributes_count(raptor_xml_element* xml_element);
RAPTOR_API
int raptor_xml_element_declare_namespace(raptor_xml_element* xml_element, raptor_namespace *nspace);
RAPTOR_API
int raptor_xml_element_write(raptor_xml_element *element, raptor_namespace_stack *nstack, int is_empty, int is_end, int depth, raptor_iostream *iostr);
RAPTOR_API
int raptor_xml_element_is_empty(raptor_xml_element* xml_element);
RAPTOR_API
const unsigned char* raptor_xml_element_get_language(raptor_xml_element* xml_element);


/* XML Writer Class (raptor_xml_writer) */
RAPTOR_API
raptor_xml_writer* raptor_new_xml_writer(raptor_world* world, raptor_namespace_stack *nstack, raptor_iostream* iostr);
RAPTOR_API
void raptor_free_xml_writer(raptor_xml_writer* xml_writer);

/* methods */
RAPTOR_API
void raptor_xml_writer_empty_element(raptor_xml_writer* xml_writer, raptor_xml_element *element);
RAPTOR_API
void raptor_xml_writer_start_element(raptor_xml_writer* xml_writer, raptor_xml_element *element);
RAPTOR_API
void raptor_xml_writer_end_element(raptor_xml_writer* xml_writer, raptor_xml_element *element);
RAPTOR_API
void raptor_xml_writer_newline(raptor_xml_writer* xml_writer);
RAPTOR_API
void raptor_xml_writer_cdata(raptor_xml_writer* xml_writer, const unsigned char *s);
RAPTOR_API
void raptor_xml_writer_cdata_counted(raptor_xml_writer* xml_writer, const unsigned char *s, unsigned int len);
RAPTOR_API
void raptor_xml_writer_raw(raptor_xml_writer* xml_writer, const unsigned char *s);
RAPTOR_API
void raptor_xml_writer_raw_counted(raptor_xml_writer* xml_writer, const unsigned char *s, unsigned int len);
RAPTOR_API
void raptor_xml_writer_comment(raptor_xml_writer* xml_writer, const unsigned char *s);
RAPTOR_API
void raptor_xml_writer_comment_counted(raptor_xml_writer* xml_writer, const unsigned char *s, unsigned int len);
RAPTOR_API
void raptor_xml_writer_flush(raptor_xml_writer* xml_writer);
RAPTOR_API
int raptor_xml_writer_set_option(raptor_xml_writer *xml_writer, raptor_option option, char* string, int integer);
RAPTOR_API
int raptor_xml_writer_get_option(raptor_xml_writer *xml_writer, raptor_option option, char** string_p, int* integer_p);
RAPTOR_API
int raptor_xml_writer_get_depth(raptor_xml_writer *xml_writer);

/**
 * raptor_sax2_start_element_handler:
 * @user_data: user data
 * @xml_element: XML element
 *
 * SAX2 start element handler
 */
typedef void (*raptor_sax2_start_element_handler)(void *user_data, raptor_xml_element *xml_element);

/**
 * raptor_sax2_end_element_handler:
 * @user_data: user data
 * @xml_element: XML element
 *
 * SAX2 end element handler
 */
typedef void (*raptor_sax2_end_element_handler)(void *user_data, raptor_xml_element* xml_element);

/**
 * raptor_sax2_characters_handler:
 * @user_data: user data
 * @xml_element: XML element
 * @s: string
 * @len: string len
 *
 * SAX2 characters handler
 */
typedef void (*raptor_sax2_characters_handler)(void *user_data, raptor_xml_element* xml_element, const unsigned char *s, int len);

/**
 * raptor_sax2_cdata_handler:
 * @user_data: user data
 * @xml_element: XML element
 * @s: string
 * @len: string len

 * SAX2 CDATA section handler
 */
typedef void (*raptor_sax2_cdata_handler)(void *user_data, raptor_xml_element* xml_element, const unsigned char *s, int len);

/**
 * raptor_sax2_comment_handler:
 * @user_data: user data
 * @xml_element: XML element
 * @s: string
 *
 * SAX2 XML comment handler
 */
typedef void (*raptor_sax2_comment_handler)(void *user_data, raptor_xml_element* xml_element, const unsigned char *s);

/**
 * raptor_sax2_unparsed_entity_decl_handler:
 * @user_data: user data
 * @entityName: entity name
 * @base: base URI
 * @systemId: system ID
 * @publicId: public ID
 * @notationName: notation name
 *
 * SAX2 unparsed entity (NDATA) handler
 */
typedef void (*raptor_sax2_unparsed_entity_decl_handler)(void *user_data, const unsigned char* entityName, const unsigned char* base, const unsigned char* systemId, const unsigned char* publicId, const unsigned char* notationName);

/**
 * raptor_sax2_external_entity_ref_handler:
 * @user_data: user data
 * @context: context
 * @base: base URI
 * @systemId: system ID
 * @publicId: public ID
 *
 * SAX2 external entity reference handler
 *
 * Return value: 0 if processing should not continue because of a
 * fatal error in the handling of the external entity.
 */
typedef int (*raptor_sax2_external_entity_ref_handler)(void *user_data, const unsigned char* context, const unsigned char* base, const unsigned char* systemId, const unsigned char* publicId);


/* SAX2 API */
RAPTOR_API
raptor_sax2* raptor_new_sax2(raptor_world *world, raptor_locator *locator, void* user_data);
RAPTOR_API
void raptor_free_sax2(raptor_sax2 *sax2);

/* methods */
RAPTOR_API
void raptor_sax2_set_start_element_handler(raptor_sax2* sax2, raptor_sax2_start_element_handler handler);
RAPTOR_API
void raptor_sax2_set_end_element_handler(raptor_sax2* sax2, raptor_sax2_end_element_handler handler);
RAPTOR_API
void raptor_sax2_set_characters_handler(raptor_sax2* sax2, raptor_sax2_characters_handler handler);
RAPTOR_API
void raptor_sax2_set_cdata_handler(raptor_sax2* sax2, raptor_sax2_cdata_handler handler);
RAPTOR_API
void raptor_sax2_set_comment_handler(raptor_sax2* sax2, raptor_sax2_comment_handler handler);
RAPTOR_API
void raptor_sax2_set_unparsed_entity_decl_handler(raptor_sax2* sax2, raptor_sax2_unparsed_entity_decl_handler handler);
RAPTOR_API
void raptor_sax2_set_external_entity_ref_handler(raptor_sax2* sax2, raptor_sax2_external_entity_ref_handler handler);
RAPTOR_API
void raptor_sax2_set_namespace_handler(raptor_sax2* sax2, raptor_namespace_handler handler);
RAPTOR_API
void raptor_sax2_set_uri_filter(raptor_sax2* sax2, raptor_uri_filter_func filter, void *user_data);
RAPTOR_API
void raptor_sax2_parse_start(raptor_sax2 *sax2, raptor_uri *base_uri);
RAPTOR_API
int raptor_sax2_parse_chunk(raptor_sax2* sax2, const unsigned char *buffer, size_t len, int is_end);
RAPTOR_API
const unsigned char* raptor_sax2_inscope_xml_language(raptor_sax2* sax2);
RAPTOR_API
raptor_uri* raptor_sax2_inscope_base_uri(raptor_sax2* sax2);



/* AVL Trees */

/**
 * raptor_avltree:
 *
 * AVL Tree
 */
typedef struct raptor_avltree_s raptor_avltree;

/**
 * raptor_avltree_iterator:
 *
 * AVL Tree Iterator as created by raptor_new_avltree_iterator()
 */
typedef struct raptor_avltree_iterator_s raptor_avltree_iterator;

/**
 * raptor_avltree_visit_handler:
 * @depth: depth of object in tree
 * @data: data object being visited
 * @user_data: user data arg to raptor_avltree_visit()
 *
 * AVL Tree visitor function as given to raptor_avltree_visit()
 *
 * Return value: non-0 to terminate visit early.
 */
typedef int (*raptor_avltree_visit_handler)(int depth, void* data, void *user_data);


/**
 * raptor_avltree_bitflags:
 * @RAPTOR_AVLTREE_FLAG_REPLACE_DUPLICATES: If set raptor_avltree_add() will replace any duplicate items. If not set, raptor_avltree_add() will not replace them and will return status >0 when adding a duplicate. (Default is not set)
 *
 * Bit flags for AVL Tree class constructor raptor_new_avltree()
 **/
typedef enum {
 RAPTOR_AVLTREE_FLAG_REPLACE_DUPLICATES = 1
} raptor_avltree_bitflags;


RAPTOR_API
raptor_avltree* raptor_new_avltree(raptor_data_compare_handler compare_handler, raptor_data_free_handler free_handler, unsigned int flags);
RAPTOR_API
void raptor_free_avltree(raptor_avltree* tree);

/* methods */
RAPTOR_API
int raptor_avltree_add(raptor_avltree* tree, void* p_data);
RAPTOR_API
void* raptor_avltree_remove(raptor_avltree* tree, void* p_data);
RAPTOR_API
int raptor_avltree_delete(raptor_avltree* tree, void* p_data);
RAPTOR_API
void* raptor_avltree_search(raptor_avltree* tree, const void* p_data);
RAPTOR_API
int raptor_avltree_visit(raptor_avltree* tree, raptor_avltree_visit_handler visit_handler, void* user_data);
RAPTOR_API
int raptor_avltree_size(raptor_avltree* tree);
RAPTOR_API
void raptor_avltree_set_print_handler(raptor_avltree* tree, raptor_data_print_handler print_handler);
RAPTOR_API
int raptor_avltree_print(raptor_avltree* tree, FILE* stream);

RAPTOR_API
raptor_avltree_iterator* raptor_new_avltree_iterator(raptor_avltree* tree, void* range,  raptor_data_free_handler range_free_handler, int direction);
RAPTOR_API
void raptor_free_avltree_iterator(raptor_avltree_iterator* iterator);

RAPTOR_API
int raptor_avltree_iterator_is_end(raptor_avltree_iterator* iterator);
RAPTOR_API
int raptor_avltree_iterator_next(raptor_avltree_iterator* iterator);
RAPTOR_API
void* raptor_avltree_iterator_get(raptor_avltree_iterator* iterator);

/* utility methods */
RAPTOR_API
void raptor_sort_r(void *base, size_t nel, size_t width, raptor_data_compare_arg_handler compar, void *user_data);


#ifdef __cplusplus
}
#endif

#endif
