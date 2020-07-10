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

#ifndef INCLUDED_OOX_SOURCE_DOCPROP_OOXMLDOCPROPIMPORT_HXX
#define INCLUDED_OOX_SOURCE_DOCPROP_OOXMLDOCPROPIMPORT_HXX

#include <com/sun/star/document/XOOXMLDocumentPropertiesImporter.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/implbase.hxx>

namespace oox::docprop {

class DocumentPropertiesImport :
    public ::cppu::WeakImplHelper<
        css::lang::XServiceInfo,
        css::document::XOOXMLDocumentPropertiesImporter >
{
public:
    explicit            DocumentPropertiesImport(
                            const css::uno::Reference< css::uno::XComponentContext >& rxContext );

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& rServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // XOOXMLDocumentPropertiesImporter
    virtual void SAL_CALL importProperties(
                            const css::uno::Reference< css::embed::XStorage >& rxSource,
                            const css::uno::Reference< css::document::XDocumentProperties >& rxDocumentProperties ) override;

private:
    css::uno::Reference< css::uno::XComponentContext > mxContext;
};

} // namespace oox::docprop

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
