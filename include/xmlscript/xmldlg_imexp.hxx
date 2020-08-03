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
#pragma once

#include <com/sun/star/uno/Reference.hxx>

#include <xmlscript/xmlscriptdllapi.h>

namespace com::sun::star::container { class XNameContainer; }
namespace com::sun::star::frame { class XModel; }
namespace com::sun::star::io { class XInputStream; }
namespace com::sun::star::io { class XInputStreamProvider; }
namespace com::sun::star::uno { class XComponentContext; }
namespace com::sun::star::xml::sax { class XDocumentHandler; }
namespace com::sun::star::xml::sax { class XExtendedDocumentHandler; }

namespace xmlscript
{

XMLSCRIPT_DLLPUBLIC void exportDialogModel(
    css::uno::Reference< css::xml::sax::XExtendedDocumentHandler > const & xOut,
    css::uno::Reference< css::container::XNameContainer > const & xDialogModel,
    css::uno::Reference< css::frame::XModel > const & xDocument );

XMLSCRIPT_DLLPUBLIC css::uno::Reference< css::xml::sax::XDocumentHandler >
importDialogModel(
    css::uno::Reference< css::container::XNameContainer > const & xDialogModel,
    css::uno::Reference< css::uno::XComponentContext > const & xContext,
    css::uno::Reference< css::frame::XModel > const & xDocument );

// additional functions for convenience

XMLSCRIPT_DLLPUBLIC css::uno::Reference< css::io::XInputStreamProvider >
exportDialogModel(
    css::uno::Reference< css::container::XNameContainer > const & xDialogModel,
    css::uno::Reference< css::uno::XComponentContext > const & xContext,
    css::uno::Reference< css::frame::XModel > const & xDocument );

XMLSCRIPT_DLLPUBLIC void importDialogModel(
    css::uno::Reference< css::io::XInputStream > const & xInput,
    css::uno::Reference< css::container::XNameContainer > const & xDialogModel,
    css::uno::Reference< css::uno::XComponentContext > const & xContext,
    css::uno::Reference< css::frame::XModel > const & xDocument );

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
