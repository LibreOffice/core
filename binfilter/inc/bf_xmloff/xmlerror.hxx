/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _XMLOFF_XMLERROR_HXX
#define _XMLOFF_XMLERROR_HXX

#include <com/sun/star/xml/sax/SAXParseException.hpp>

#include <sal/types.h>

// STL includes
#include <vector>
// forward declarations
namespace rtl { class OUString; }
namespace com { namespace sun { namespace star {
    namespace uno { template<class X> class Sequence; }
    namespace uno { template<class X> class Reference; }
    namespace xml { namespace sax { class XLocator; } }
} } }
namespace binfilter {

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
// allways add to error code below the appropriate comment.

// I/O errors:

// format errors:
#define XMLERROR_SAX                ( XMLERROR_CLASS_FORMAT | 0x00000001 )
#define XMLERROR_STYLE_ATTR_VALUE   ( XMLERROR_CLASS_FORMAT | 0x00000002 )
#define XMLERROR_NO_INDEX_ALLOWED_HERE ( XMLERROR_CLASS_FORMAT | 0x00000003 )
#define XMLERROR_PARENT_STYLE_NOT_ALLOWED ( XMLERROR_CLASS_FORMAT | 0x00000004 )
#define XMLERROR_ILLEGAL_EVENT (XMLERROR_CLASS_FORMAT | 0x00000005 )

// API errors:
#define XMLERROR_STYLE_PROP_VALUE   ( XMLERROR_CLASS_API    | 0x00000001 )
#define XMLERROR_STYLE_PROP_UNKNOWN ( XMLERROR_CLASS_API    | 0x00000002 )
#define XMLERROR_STYLE_PROP_OTHER   ( XMLERROR_CLASS_API    | 0x00000003 )
#define XMLERROR_API                ( XMLERROR_CLASS_API    | 0x00000004 )

// other errors:
#define XMLERROR_CANCEL         ( XMLERROR_CLASS_OTHER  | 0x00000001 )

// 16bit error flag constants for use in the 
// SvXMLExport/SvXMLImport error flags
#define ERROR_NO                0x0000
#define ERROR_DO_NOTHING        0x0001
#define ERROR_ERROR_OCCURED     0x0002
#define ERROR_WARNING_OCCURED   0x0004


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
        const ::com::sun::star::uno::Sequence< 
                  ::rtl::OUString> & rParams,  /// parameters for error message
        const ::rtl::OUString& rExceptionMessage, /// original exception string
        sal_Int32 nRow,                     /// XLocator: file row number
        sal_Int32 nColumn,                  /// XLocator: file column number
        const ::rtl::OUString& rPublicId,   /// XLocator: file public ID
        const ::rtl::OUString& rSystemId ); /// XLocator: file system ID

    void AddRecord( 
        sal_Int32 nId, /// error ID == error flags + error class + error number
        const ::com::sun::star::uno::Sequence< 
                  ::rtl::OUString> & rParams,  /// parameters for error message
        const ::rtl::OUString& rExceptionMessage, /// original exception string
        const ::com::sun::star::uno::Reference< 
                 ::com::sun::star::xml::sax::XLocator> & rLocator); /// location

    /**
     * throw a SAXParseException that describes the first error that matches
     * the given mask
     */
    void ThrowErrorAsSAXException( sal_Int32 nIdMask )
        throw( ::com::sun::star::xml::sax::SAXParseException );
};

}//end of namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
