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

#include <rtl/ref.hxx>
#include <ucbhelper/resultset.hxx>
#include <optional>
#include <memory>
#include <vector>
#include <string_view>

namespace tdoc_ucp {

struct DataSupplier_Impl;
class Content;

class ResultSetDataSupplier : public ::ucbhelper::ResultSetDataSupplier
{
    struct ResultListEntry
    {
        OUString                                  aURL;
        css::uno::Reference< css::ucb::XContentIdentifier > xId;
        css::uno::Reference< css::ucb::XContent >           xContent;
        css::uno::Reference< css::sdbc::XRow >              xRow;

        explicit ResultListEntry( const OUString& rURL ) : aURL( rURL ) {}
    };

    osl::Mutex                                   m_aMutex;
    std::vector< ResultListEntry >               m_aResults;
    rtl::Reference< Content >                    m_xContent;
    css::uno::Reference< css::uno::XComponentContext >     m_xContext;
    std::optional<css::uno::Sequence< OUString > >    m_xNamesOfChildren;
    bool                                         m_bCountFinal;
    bool                                         m_bThrowException;

private:
    bool queryNamesOfChildren();
    OUString assembleChildURL( std::u16string_view aName );

public:
    ResultSetDataSupplier(
        const css::uno::Reference< css::uno::XComponentContext >& rxContext,
        const rtl::Reference< Content >& rContent );
    virtual ~ResultSetDataSupplier() override;

    virtual OUString queryContentIdentifierString( sal_uInt32 nIndex ) override;
    virtual css::uno::Reference< css::ucb::XContentIdentifier >
    queryContentIdentifier( sal_uInt32 nIndex ) override;
    virtual css::uno::Reference< css::ucb::XContent >
    queryContent( sal_uInt32 nIndex ) override;

    virtual bool getResult( sal_uInt32 nIndex ) override;

    virtual sal_uInt32 totalCount() override;
    virtual sal_uInt32 currentCount() override;
    virtual bool   isCountFinal() override;

    virtual css::uno::Reference< css::sdbc::XRow >
    queryPropertyValues( sal_uInt32 nIndex  ) override;
    virtual void releasePropertyValues( sal_uInt32 nIndex ) override;

    virtual void close() override;

    virtual void validate() override;
};

} // namespace tdoc_ucp

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
