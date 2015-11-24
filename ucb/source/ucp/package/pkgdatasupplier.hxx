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

#ifndef INCLUDED_UCB_SOURCE_UCP_PACKAGE_PKGDATASUPPLIER_HXX
#define INCLUDED_UCB_SOURCE_UCP_PACKAGE_PKGDATASUPPLIER_HXX

#include <rtl/ref.hxx>
#include <ucbhelper/resultset.hxx>
#include <memory>

namespace package_ucp {

struct DataSupplier_Impl;
class Content;

class DataSupplier : public ::ucbhelper::ResultSetDataSupplier
{
    std::unique_ptr<DataSupplier_Impl> m_pImpl;

public:
    DataSupplier( const css::uno::Reference< css::uno::XComponentContext >& rxContext,
                  const rtl::Reference< Content >& rContent );
    virtual ~DataSupplier();

    virtual OUString queryContentIdentifierString( sal_uInt32 nIndex ) override;
    virtual css::uno::Reference< css::ucb::XContentIdentifier >
    queryContentIdentifier( sal_uInt32 nIndex ) override;
    virtual css::uno::Reference< css::ucb::XContent >
    queryContent( sal_uInt32 nIndex ) override;

    virtual bool getResult( sal_uInt32 nIndex ) override;

    virtual sal_uInt32 totalCount() override;
    virtual sal_uInt32 currentCount() override;
    virtual bool isCountFinal() override;

    virtual css::uno::Reference< css::sdbc::XRow >
    queryPropertyValues( sal_uInt32 nIndex  ) override;
    virtual void releasePropertyValues( sal_uInt32 nIndex ) override;

    virtual void close() override;

    virtual void validate()
        throw( css::ucb::ResultSetException ) override;

    OUString assembleChildURL( const OUString& aName );
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
