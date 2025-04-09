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

#include <sal/config.h>

#include "ContentProperties.hxx"
#include <memory>
#include <rtl/ref.hxx>
#include <ucbhelper/resultset.hxx>

namespace http_dav_ucp {

class Content;

class DataSupplier : public ucbhelper::ResultSetDataSupplier
{
    bool getData(std::unique_lock<std::mutex>& rResultSetGuard);

public:
    DataSupplier( const css::uno::Reference< css::uno::XComponentContext >& rxContext,
              const rtl::Reference< Content >& rContent,
              sal_Int32 nOpenMode);

    virtual ~DataSupplier() override;

    virtual OUString queryContentIdentifierString( std::unique_lock<std::mutex>& rResultSetGuard, sal_uInt32 nIndex ) override;
    virtual css::uno::Reference< css::ucb::XContentIdentifier >
    queryContentIdentifier( std::unique_lock<std::mutex>& rResultSetGuard, sal_uInt32 nIndex ) override;
    virtual css::uno::Reference< css::ucb::XContent >
    queryContent( std::unique_lock<std::mutex>& rResultSetGuard, sal_uInt32 nIndex ) override;

    virtual bool getResult( std::unique_lock<std::mutex>& rResultSetGuard, sal_uInt32 nIndex ) override;

    virtual sal_uInt32 totalCount(std::unique_lock<std::mutex>& rResultSetGuard) override;
    virtual sal_uInt32 currentCount() override;
    virtual bool isCountFinal() override;

    virtual css::uno::Reference< css::sdbc::XRow >
    queryPropertyValues( std::unique_lock<std::mutex>& rResultSetGuard, sal_uInt32 nIndex  ) override;
    virtual void releasePropertyValues( sal_uInt32 nIndex ) override;

    virtual void close() override;

    virtual void validate() override;

private:
    struct ResultListEntry
    {
        OUString                             aId;
        css::uno::Reference< css::ucb::XContentIdentifier > xId;
        css::uno::Reference< css::ucb::XContent >           xContent;
        css::uno::Reference< css::sdbc::XRow >              xRow;
        std::unique_ptr<ContentProperties> pData;

        explicit ResultListEntry( std::unique_ptr<ContentProperties> && pEntry ) : pData( std::move(pEntry) ) {}
    };

    typedef std::vector<std::unique_ptr<ResultListEntry>> ResultList;

    osl::Mutex                                   m_aMutex;
    ResultList                                   m_Results;
    rtl::Reference< Content >                    m_xContent;
    css::uno::Reference< css::uno::XComponentContext > m_xContext;
    sal_Int32                                    m_nOpenMode;
    bool                                         m_bCountFinal;
    bool                                         m_bThrowException;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
