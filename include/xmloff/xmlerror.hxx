/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_XMLOFF_XMLERROR_HXX
#define INCLUDED_XMLOFF_XMLERROR_HXX

#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <o3tl/typed_flags_set.hxx>

#include <vector>

// masks for the error ID fields
#define XMLERROR_MASK_FLAG      0xF0000000
#define XMLERROR_MASK_CLASS     0x00FF0000
#define XMLERROR_MASK_NUMBER    0x0000FFFF

// error flags:
#define XMLERROR_FLAG_WARNING   0x10000000
#define XMLERROR_FLAG_ERROR     0x20000000
#define XMLERROR_FLAG_SEVERE    0x40000000

// error classes: Error ID
#define XMLERROR_CLASS_IO       0x00010000
#define XMLERROR_CLASS_FORMAT   0x00020000
#define XMLERROR_CLASS_API      0x00040000
#define XMLERROR_CLASS_OTHER    0x00080000

// error numbers, listed by error class
// Within each class, errors should be numbered consecutively. Please
// always add to error code below the appropriate comment.

// I/O errors:

// format errors:
#define XMLERROR_SAX                ( XMLERROR_CLASS_FORMAT | 0x00000001 )
#define XMLERROR_STYLE_ATTR_VALUE   ( XMLERROR_CLASS_FORMAT | 0x00000002 )
#define XMLERROR_NO_INDEX_ALLOWED_HERE ( XMLERROR_CLASS_FORMAT | 0x00000003 )
#define XMLERROR_PARENT_STYLE_NOT_ALLOWED ( XMLERROR_CLASS_FORMAT | 0x00000004 )
#define XMLERROR_ILLEGAL_EVENT (XMLERROR_CLASS_FORMAT | 0x00000005 )
#define XMLERROR_NAMESPACE_TROUBLE (XMLERROR_CLASS_FORMAT | 0x00000006 )

#define XMLERROR_XFORMS_NO_SCHEMA_SUPPORT ( XMLERROR_CLASS_FORMAT | XMLERROR_FLAG_WARNING | 0x00000007 )
#define XMLERROR_XFORMS_UNKNOWN ( XMLERROR_CLASS_FORMAT | XMLERROR_FLAG_WARNING | 0x00000008 )
#define XMLERROR_XFORMS_ONLY_ONE_INSTANCE_ELEMENT ( XMLERROR_CLASS_FORMAT | XMLERROR_FLAG_WARNING | 0x00000009 )
#define XMLERROR_UNKNOWN_ATTRIBUTE ( XMLERROR_CLASS_FORMAT | XMLERROR_FLAG_WARNING | 0x0000000a )
#define XMLERROR_UNKNOWN_ELEMENT ( XMLERROR_CLASS_FORMAT | XMLERROR_FLAG_WARNING | 0x0000000b )
#define XMLERROR_UNKNOWN_CHARACTERS ( XMLERROR_CLASS_FORMAT | XMLERROR_FLAG_WARNING | 0x0000000c )
#define XMLERROR_UNKNOWN_ROOT (XMLERROR_CLASS_FORMAT | 0x0000000d )

// API errors:
#define XMLERROR_STYLE_PROP_VALUE   ( XMLERROR_CLASS_API    | 0x00000001 )
#define XMLERROR_STYLE_PROP_UNKNOWN ( XMLERROR_CLASS_API    | 0x00000002 )
#define XMLERROR_STYLE_PROP_OTHER   ( XMLERROR_CLASS_API    | 0x00000003 )
#define XMLERROR_API                ( XMLERROR_CLASS_API    | 0x00000004 )

// other errors:
#define XMLERROR_CANCEL         ( XMLERROR_CLASS_OTHER  | 0x00000001 )

// 16bit error flag constants for use in the
// SvXMLExport/SvXMLImport error flags
enum class SvXMLErrorFlags {
    NO                = 0x0000,
    DO_NOTHING        = 0x0001,
    ERROR_OCCURRED    = 0x0002,
    WARNING_OCCURRED  = 0x0004,
};

namespace o3tl
{
    template<> struct typed_flags<SvXMLErrorFlags> : is_typed_flags<SvXMLErrorFlags, 0x7> {};
}

namespace com { namespace sun { namespace star {
    namespace uno { template<class X> class Sequence; }
    namespace uno { template<class X> class Reference; }
    namespace xml { namespace sax { class XLocator; } }
} } }

class ErrorRecord;

/**
 * The XMLErrors is used to collect all errors and warnings that occur
 * for appropriate processing.
 */
class XMLErrors
{
    /// definition of type for error list
    typedef ::std::vector<ErrorRecord> ErrorList;

    ErrorList aErrors;  /// list of error records

public:

    XMLErrors();
    ~XMLErrors();

    /// add a new entry to the list of error messages
    void AddRecord(
        sal_Int32 nId, /// error ID == error flags + error class + error number
        const css::uno::Sequence< OUString> & rParams,  /// parameters for error message
        const OUString& rExceptionMessage, /// original exception string
        sal_Int32 nRow,                     /// XLocator: file row number
        sal_Int32 nColumn,                  /// XLocator: file column number
        const OUString& rPublicId,   /// XLocator: file public ID
        const OUString& rSystemId ); /// XLocator: file system ID

    void AddRecord(
        sal_Int32 nId, /// error ID == error flags + error class + error number
        const css::uno::Sequence<OUString> & rParams,  /// parameters for error message
        const OUString& rExceptionMessage, /// original exception string
        const css::uno::Reference<css::xml::sax::XLocator> & rLocator); /// location

    /**
     * throw a SAXParseException that describes the first error that matches
     * the given mask
     */
    /// @throws css::xml::sax::SAXParseException
    void ThrowErrorAsSAXException( sal_Int32 nIdMask );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
