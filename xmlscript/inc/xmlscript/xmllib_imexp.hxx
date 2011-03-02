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
#ifndef _XMLSCRIPT_XMLLIB_IMEXP_HXX_
#define _XMLSCRIPT_XMLLIB_IMEXP_HXX_

#include <com/sun/star/xml/sax/XExtendedDocumentHandler.hpp>
#include <com/sun/star/uno/Sequence.hxx>

#include "xmlscript/xmlns.h"
#include "xmlscript/xcrdllapi.h"

namespace xmlscript
{

//==============================================================================
// Library container export
// HACK C++ struct to transport info. Later the container
// itself should do the export/import and use exportet XML
// functionality from xmlscript
struct XCR_DLLPUBLIC LibDescriptor
{
    ::rtl::OUString aName;
    ::rtl::OUString aStorageURL;
    sal_Bool bLink;
    sal_Bool bReadOnly;
    sal_Bool bPasswordProtected;
    ::com::sun::star::uno::Sequence< ::rtl::OUString > aElementNames;
    sal_Bool bPreload;
};

struct XCR_DLLPUBLIC LibDescriptorArray
{
    LibDescriptor* mpLibs;
    sal_Int32 mnLibCount;

    LibDescriptorArray( void ) { mpLibs = NULL; mnLibCount = 0; }
    LibDescriptorArray( sal_Int32 nLibCount );

    ~LibDescriptorArray();

};

XCR_DLLPUBLIC void
SAL_CALL exportLibraryContainer(
    ::com::sun::star::uno::Reference<
    ::com::sun::star::xml::sax::XExtendedDocumentHandler > const & xOut,
    const LibDescriptorArray* pLibArray )
        SAL_THROW( (::com::sun::star::uno::Exception) );

XCR_DLLPUBLIC ::com::sun::star::uno::Reference<
    ::com::sun::star::xml::sax::XDocumentHandler >
SAL_CALL importLibraryContainer( LibDescriptorArray* pLibArray )
        SAL_THROW( (::com::sun::star::uno::Exception) );


XCR_DLLPUBLIC void
SAL_CALL exportLibrary(
    ::com::sun::star::uno::Reference<
    ::com::sun::star::xml::sax::XExtendedDocumentHandler > const & xOut,
    const LibDescriptor& rLib )
        SAL_THROW( (::com::sun::star::uno::Exception) );

XCR_DLLPUBLIC ::com::sun::star::uno::Reference<
    ::com::sun::star::xml::sax::XDocumentHandler >
SAL_CALL importLibrary( LibDescriptor& rLib )
        SAL_THROW( (::com::sun::star::uno::Exception) );

}

#endif
