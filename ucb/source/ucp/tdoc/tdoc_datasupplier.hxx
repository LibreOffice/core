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
#include <mutex>
#include <optional>
#include <string_view>
#include <utility>
#include <vector>

namespace tdoc_ucp {

class Content;

class ResultSetDataSupplier final : public ::ucbhelper::ResultSetDataSupplier
{
    struct ResultListEntry
    {
        OUString                                  aURL;
        css::uno::Reference< css::ucb::XContentIdentifier > xId;
        css::uno::Reference< css::ucb::XContent >           xContent;
        css::uno::Reference< css::sdbc::XRow >              xRow;

        explicit ResultListEntry( OUString _aURL ) : aURL(std::move( _aURL )) {}
    };

    std::mutex                                   m_aMutex;
    std::vector< ResultListEntry >               m_aResults;
    rtl::Reference< Content >                    m_xContent;
    css::uno::Reference< css::uno::XComponentContext >     m_xContext;
    std::optional<css::uno::Sequence< OUString > >    m_xNamesOfChildren;
    bool                                         m_bCountFinal;
    bool                                         m_bThrowException;

private:
    bool queryNamesOfChildren(std::unique_lock<std::mutex>& rGuard);
    OUString assembleChildURL( std::u16string_view aName );

public:
    ResultSetDataSupplier(
        css::uno::Reference< css::uno::XComponentContext > xContext,
        rtl::Reference< Content > xContent );
    virtual ~ResultSetDataSupplier() override;

    virtual OUString queryContentIdentifierString( std::unique_lock<std::mutex>& rResultSetGuard, sal_uInt32 nIndex ) override;
    virtual css::uno::Reference< css::ucb::XContentIdentifier >
    queryContentIdentifier( std::unique_lock<std::mutex>& rResultSetGuard, sal_uInt32 nIndex ) override;
    virtual css::uno::Reference< css::ucb::XContent >
    queryContent( std::unique_lock<std::mutex>& rResultSetGuard, sal_uInt32 nIndex ) override;

    virtual bool getResult( std::unique_lock<std::mutex>& rResultSetGuard, sal_uInt32 nIndex ) override;

    virtual sal_uInt32 totalCount(std::unique_lock<std::mutex>& rResultSetGuard) override;
    virtual sal_uInt32 currentCount() override;
    virtual bool   isCountFinal() override;

    virtual css::uno::Reference< css::sdbc::XRow >
    queryPropertyValues( std::unique_lock<std::mutex>& rResultSetGuard, sal_uInt32 nIndex  ) override;
    virtual void releasePropertyValues( sal_uInt32 nIndex ) override;

    virtual void close() override;

    virtual void validate() override;

private:
    OUString queryContentIdentifierStringImpl( std::unique_lock<std::mutex>& rResultSetGuard, std::unique_lock<std::mutex>& rGuard, sal_uInt32 nIndex );
    css::uno::Reference< css::ucb::XContentIdentifier >
      queryContentIdentifierImpl( std::unique_lock<std::mutex>& rResultSetGuard, std::unique_lock<std::mutex>& rGuard, sal_uInt32 nIndex );
    bool getResultImpl( std::unique_lock<std::mutex>& rResultSetGuard, std::unique_lock<std::mutex>& rGuard, sal_uInt32 nIndex );
};

} // namespace tdoc_ucp

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
