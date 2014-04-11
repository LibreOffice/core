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

#ifndef INCLUDED_XMLSCRIPT_SOURCE_INC_UNOSERVICES_HXX
#define INCLUDED_XMLSCRIPT_SOURCE_INC_UNOSERVICES_HXX

#include <sal/config.h>

#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>

namespace com { namespace sun { namespace star { namespace uno {
    class XComponentContext;
    class XInterface;
} } } }

namespace xmlscript {

css::uno::Sequence<OUString> SAL_CALL
getSupportedServiceNames_DocumentHandlerImpl();

OUString SAL_CALL getImplementationName_DocumentHandlerImpl();

css::uno::Reference<css::uno::XInterface> SAL_CALL create_DocumentHandlerImpl(
    css::uno::Reference<css::uno::XComponentContext> const & xContext)
    SAL_THROW((css::uno::Exception));

css::uno::Sequence<OUString> SAL_CALL
getSupportedServiceNames_XMLBasicExporter();

OUString SAL_CALL getImplementationName_XMLBasicExporter();

css::uno::Reference<css::uno::XInterface> SAL_CALL create_XMLBasicExporter(
    css::uno::Reference<css::uno::XComponentContext> const & xContext)
    SAL_THROW((css::uno::Exception));

css::uno::Sequence<OUString> SAL_CALL
getSupportedServiceNames_XMLOasisBasicExporter();

OUString SAL_CALL getImplementationName_XMLOasisBasicExporter();

css::uno::Reference<css::uno::XInterface> SAL_CALL create_XMLOasisBasicExporter(
    css::uno::Reference<css::uno::XComponentContext> const & xContext)
    SAL_THROW((css::uno::Exception));

css::uno::Sequence<OUString> SAL_CALL
getSupportedServiceNames_XMLBasicImporter();

OUString SAL_CALL getImplementationName_XMLBasicImporter();

css::uno::Reference<css::uno::XInterface> SAL_CALL create_XMLBasicImporter(
    css::uno::Reference<css::uno::XComponentContext> const & xContext)
    SAL_THROW((css::uno::Exception));

css::uno::Sequence<OUString> SAL_CALL
getSupportedServiceNames_XMLOasisBasicImporter();

OUString SAL_CALL getImplementationName_XMLOasisBasicImporter();

css::uno::Reference<css::uno::XInterface> SAL_CALL create_XMLOasisBasicImporter(
    css::uno::Reference<css::uno::XComponentContext> const & xContext)
        SAL_THROW((css::uno::Exception));

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
