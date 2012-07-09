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
#ifndef _XMLSCRIPT_XMLDLG_IMEXP_HXX_
#define _XMLSCRIPT_XMLDLG_IMEXP_HXX_

#include <com/sun/star/frame/XModel.hpp>

#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/io/XInputStreamProvider.hpp>
#include <com/sun/star/xml/sax/XExtendedDocumentHandler.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include "xmlscript/xmlns.h"
#include "xmlscript/xmlscriptdllapi.h"

namespace xmlscript
{

XMLSCRIPT_DLLPUBLIC void SAL_CALL exportDialogModel(
    ::com::sun::star::uno::Reference<
    ::com::sun::star::xml::sax::XExtendedDocumentHandler > const & xOut,
    ::com::sun::star::uno::Reference<
    ::com::sun::star::container::XNameContainer > const & xDialogModel,
    ::com::sun::star::uno::Reference<
    ::com::sun::star::frame::XModel > const & xDocument )
    SAL_THROW( (::com::sun::star::uno::Exception) );

XMLSCRIPT_DLLPUBLIC ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler >
SAL_CALL importDialogModel(
    ::com::sun::star::uno::Reference<
    ::com::sun::star::container::XNameContainer > const & xDialogModel,
    ::com::sun::star::uno::Reference<
    ::com::sun::star::uno::XComponentContext > const & xContext,
    ::com::sun::star::uno::Reference<
    ::com::sun::star::frame::XModel > const & xDocument )
    SAL_THROW( (::com::sun::star::uno::Exception) );

// additional functions for convenience

XMLSCRIPT_DLLPUBLIC ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStreamProvider >
SAL_CALL exportDialogModel(
    ::com::sun::star::uno::Reference<
    ::com::sun::star::container::XNameContainer > const & xDialogModel,
    ::com::sun::star::uno::Reference<
    ::com::sun::star::uno::XComponentContext > const & xContext,
    ::com::sun::star::uno::Reference<
    ::com::sun::star::frame::XModel > const & xDocument )
    SAL_THROW( (::com::sun::star::uno::Exception) );

XMLSCRIPT_DLLPUBLIC void SAL_CALL importDialogModel(
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
