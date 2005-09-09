/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmllib_imexp.hxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 16:09:20 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

#include "xmlscript/xmlns.h"


namespace xmlscript
{

//==============================================================================
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
    sal_Bool bPasswordProtected;
    ::com::sun::star::uno::Sequence< ::rtl::OUString > aElementNames;
    sal_Bool bPreload;
};

struct LibDescriptorArray
{
    LibDescriptor* mpLibs;
    sal_Int32 mnLibCount;

    LibDescriptorArray( void ) { mpLibs = NULL; mnLibCount = 0; }
    LibDescriptorArray( sal_Int32 nLibCount );

    ~LibDescriptorArray();

};

void
SAL_CALL exportLibraryContainer(
    ::com::sun::star::uno::Reference<
    ::com::sun::star::xml::sax::XExtendedDocumentHandler > const & xOut,
    const LibDescriptorArray* pLibArray )
        SAL_THROW( (::com::sun::star::uno::Exception) );

::com::sun::star::uno::Reference<
    ::com::sun::star::xml::sax::XDocumentHandler >
SAL_CALL importLibraryContainer( LibDescriptorArray* pLibArray )
        SAL_THROW( (::com::sun::star::uno::Exception) );


void
SAL_CALL exportLibrary(
    ::com::sun::star::uno::Reference<
    ::com::sun::star::xml::sax::XExtendedDocumentHandler > const & xOut,
    const LibDescriptor& rLib )
        SAL_THROW( (::com::sun::star::uno::Exception) );

::com::sun::star::uno::Reference<
    ::com::sun::star::xml::sax::XDocumentHandler >
SAL_CALL importLibrary( LibDescriptor& rLib )
        SAL_THROW( (::com::sun::star::uno::Exception) );

}

#endif
