/*************************************************************************
 *
 *  $RCSfile: xmlerror.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: dvo $ $Date: 2001-09-18 16:28:34 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _XMLOFF_XMLERROR_HXX
#define _XMLOFF_XMLERROR_HXX



#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

// STL includes
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
// allways add to error code below the appropriate comment.

// I/O errors:

// format errors:
#define XMLERROR_SAX            ( XMLERROR_CLASS_FORMAT | 0x00000001 )

// API errors:

// other errors:
#define XMLERROR_CANCEL         ( XMLERROR_CLASS_OTHER  | 0x00000001 )



// 16bit error flag constants for use in the
// SvXMLExport/SvXMLImport error flags
#define ERROR_NO                0x0000
#define ERROR_DO_NOTHING        0x0001
#define ERROR_ERROR_OCCURED     0x0002
#define ERROR_WARNING_OCCURED   0x0004




// forward declarations
namespace rtl { class OUString; }
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
    AddRecord(
        sal_Int32 nId, /// error ID == error flags + error class + error number
        const ::com::sun::star::uno::Sequence<
                  ::rtl::OUString> & rParams,  /// parameters for error message
        const ::rtl::OUString& rExceptionMessage, /// original exception string
        sal_Int32 nRow,                     /// XLocator: file row number
        sal_Int32 nColumn,                  /// XLocator: file column number
        const ::rtl::OUString& rPublicId,   /// XLocator: file public ID
        const ::rtl::OUString& rSystemId ); /// XLocator: file system ID

    AddRecord(
        sal_Int32 nId, /// error ID == error flags + error class + error number
        const ::com::sun::star::uno::Sequence<
                  ::rtl::OUString> & rParams,  /// parameters for error message
        const ::rtl::OUString& rExceptionMessage, /// original exception string
        const ::com::sun::star::uno::Reference<
                 ::com::sun::star::xml::sax::XLocator> & rLocator); /// location

    AddRecord(
        sal_Int32 nId, /// error ID == error flags + error class + error number
        const ::com::sun::star::uno::Sequence<
                  ::rtl::OUString> & rParams,  /// parameters for error message
        const ::rtl::OUString& rExceptionMessage); /// original exception string

    AddRecord(
        sal_Int32 nId, /// error ID == error flags + error class + error number
        const ::com::sun::star::uno::Sequence<
                  ::rtl::OUString> & rParams); /// parameters for error message

};


#endif
