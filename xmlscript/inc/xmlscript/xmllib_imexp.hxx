/*************************************************************************
 *
 *  $RCSfile: xmllib_imexp.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: dbo $ $Date: 2001-04-04 14:35:07 $
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
#ifndef _XMLSCRIPT_XMLLIB_IMEXP_HXX_
#define _XMLSCRIPT_XMLLIB_IMEXP_HXX_

#ifndef _COM_SUN_STAR_XML_SAX_XEXTENDEDDOCUMENTHANDLER_HPP_
#include <com/sun/star/xml/sax/XExtendedDocumentHandler.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif


namespace xmlscript
{

#define XMLNS_LIBRARY_URI "http://openoffice.org/2000/library"
#define XMLNS_LIBRARY_UID 1
#define XMLNS_LIBRARY_PREFIX "library"

#define XMLNS_XLINK_URI "http://www.w3.org/1999/xlink"
#define XMLNS_XLINK_UID 2
#define XMLNS_XLINK_PREFIX "xlink"

//==================================================================================================
// Library container export

// HACK C++ struct to transport info. Later the container
// itself should do the export/import and use exportet XML
// functionality from xmlscript
struct LibDescriptor
{
    ::rtl::OUString aName;
    ::rtl::OUString aStorageURL;
    sal_Bool bLink;
    sal_Bool bReadOnly;
    ::com::sun::star::uno::Sequence< ::rtl::OUString > aElementNames;
};

SAL_DLLEXPORT void
SAL_CALL exportLibraryContainer(
    ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XExtendedDocumentHandler > const & xOut,
    LibDescriptor* pLibs, sal_Int32 nLibCount )
        SAL_THROW( (::com::sun::star::uno::Exception) );

SAL_DLLEXPORT ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler >
SAL_CALL importLibraryContainer(
    LibDescriptor*& rpLibs, sal_Int32& rnLibCount )
        SAL_THROW( (::com::sun::star::uno::Exception) );

//==================================================================================================

}

#endif
