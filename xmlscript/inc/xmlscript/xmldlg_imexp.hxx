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
#ifndef _XMLSCRIPT_XMLDLG_IMEXP_HXX_
#define _XMLSCRIPT_XMLDLG_IMEXP_HXX_

#include <com/sun/star/frame/XModel.hpp>

#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/io/XInputStreamProvider.hpp>
#include <com/sun/star/xml/sax/XExtendedDocumentHandler.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include "xmlscript/xmlns.h"


namespace xmlscript
{

//==============================================================================
void SAL_CALL exportDialogModel(
    ::com::sun::star::uno::Reference<
    ::com::sun::star::xml::sax::XExtendedDocumentHandler > const & xOut,
    ::com::sun::star::uno::Reference<
    ::com::sun::star::container::XNameContainer > const & xDialogModel,
    ::com::sun::star::uno::Reference<
    ::com::sun::star::frame::XModel > const & xDocument )
    SAL_THROW( (::com::sun::star::uno::Exception) );

//==============================================================================
::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler >
SAL_CALL importDialogModel(
    ::com::sun::star::uno::Reference<
    ::com::sun::star::container::XNameContainer > const & xDialogModel,
    ::com::sun::star::uno::Reference<
    ::com::sun::star::uno::XComponentContext > const & xContext,
    ::com::sun::star::uno::Reference<
    ::com::sun::star::frame::XModel > const & xDocument )
    SAL_THROW( (::com::sun::star::uno::Exception) );

// additional functions for convenience

//==============================================================================
::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStreamProvider >
SAL_CALL exportDialogModel(
    ::com::sun::star::uno::Reference<
    ::com::sun::star::container::XNameContainer > const & xDialogModel,
    ::com::sun::star::uno::Reference<
    ::com::sun::star::uno::XComponentContext > const & xContext,
    ::com::sun::star::uno::Reference<
    ::com::sun::star::frame::XModel > const & xDocument )
    SAL_THROW( (::com::sun::star::uno::Exception) );

//==============================================================================
void SAL_CALL importDialogModel(
    ::com::sun::star::uno::Reference<
    ::com::sun::star::io::XInputStream > const & xInput,
    ::com::sun::star::uno::Reference<
    ::com::sun::star::container::XNameContainer > const & xDialogModel,
    ::com::sun::star::uno::Reference<
    ::com::sun::star::uno::XComponentContext > const & xContext,
    ::com::sun::star::uno::Reference<
    ::com::sun::star::frame::XModel > const & xDocument )
    SAL_THROW( (::com::sun::star::uno::Exception) );

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
