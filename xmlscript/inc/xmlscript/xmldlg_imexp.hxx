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
#ifndef _XMLSCRIPT_XMLDLG_IMEXP_HXX_
#define _XMLSCRIPT_XMLDLG_IMEXP_HXX_

#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HXX_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XINPUTSTREAMPROVIDER_HXX_
#include <com/sun/star/io/XInputStreamProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_XEXTENDEDDOCUMENTHANDLER_HXX_
#include <com/sun/star/xml/sax/XExtendedDocumentHandler.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HXX_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif

#include "xmlscript/xmlns.h"
#include "xmlscript/xcrdllapi.h"

namespace xmlscript
{

//==============================================================================
XCR_DLLPUBLIC void SAL_CALL exportDialogModel(
    ::com::sun::star::uno::Reference<
    ::com::sun::star::xml::sax::XExtendedDocumentHandler > const & xOut,
    ::com::sun::star::uno::Reference<
    ::com::sun::star::container::XNameContainer > const & xDialogModel )
    SAL_THROW( (::com::sun::star::uno::Exception) );

//==============================================================================
XCR_DLLPUBLIC ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler >
SAL_CALL importDialogModel(
    ::com::sun::star::uno::Reference<
    ::com::sun::star::container::XNameContainer > const & xDialogModel,
    ::com::sun::star::uno::Reference<
    ::com::sun::star::uno::XComponentContext > const & xContext )
    SAL_THROW( (::com::sun::star::uno::Exception) );

// additional functions for convenience

//==============================================================================
XCR_DLLPUBLIC ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStreamProvider >
SAL_CALL exportDialogModel(
    ::com::sun::star::uno::Reference<
    ::com::sun::star::container::XNameContainer > const & xDialogModel,
    ::com::sun::star::uno::Reference<
    ::com::sun::star::uno::XComponentContext > const & xContext )
    SAL_THROW( (::com::sun::star::uno::Exception) );

//==============================================================================
XCR_DLLPUBLIC void SAL_CALL importDialogModel(
    ::com::sun::star::uno::Reference<
    ::com::sun::star::io::XInputStream > xInput,
    ::com::sun::star::uno::Reference<
    ::com::sun::star::container::XNameContainer > const & xDialogModel,
    ::com::sun::star::uno::Reference<
    ::com::sun::star::uno::XComponentContext > const & xContext )
    SAL_THROW( (::com::sun::star::uno::Exception) );

}

#endif
