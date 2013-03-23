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
#if 1

#include <rtl/ref.hxx>
#include <ucbhelper/resultset.hxx>
#include <ucbhelper/resultsethelper.hxx>
#include "gvfs_content.hxx"

namespace gvfs {

class DynamicResultSet : public ::ucbhelper::ResultSetImplHelper
{
    rtl::Reference< Content > m_xContent;
    com::sun::star::uno::Reference<
        com::sun::star::ucb::XCommandEnvironment > m_xEnv;

private:
    virtual void initStatic();
    virtual void initDynamic();

public:
    DynamicResultSet( const com::sun::star::uno::Reference<
                        com::sun::star::uno::XComponentContext >& rxContext,
                      const rtl::Reference< Content >& rxContent,
                      const com::sun::star::ucb::OpenCommandArgument2& rCommand,
                      const com::sun::star::uno::Reference<
                        com::sun::star::ucb::XCommandEnvironment >& rxEnv );
};


struct DataSupplier_Impl;
class DataSupplier : public ucbhelper::ResultSetDataSupplier
{
private:
    gvfs::DataSupplier_Impl *m_pImpl;
    sal_Bool getData();

public:
    DataSupplier(const rtl::Reference< Content >& rContent,
              sal_Int32 nOpenMode);

    virtual ~DataSupplier();

    virtual rtl::OUString queryContentIdentifierString( sal_uInt32 nIndex );
    virtual com::sun::star::uno::Reference<
                com::sun::star::ucb::XContentIdentifier >
                          queryContentIdentifier( sal_uInt32 nIndex );
    virtual com::sun::star::uno::Reference< com::sun::star::ucb::XContent >
                          queryContent( sal_uInt32 nIndex );

    virtual sal_Bool      getResult( sal_uInt32 nIndex );

    virtual sal_uInt32    totalCount();
    virtual sal_uInt32    currentCount();
    virtual sal_Bool      isCountFinal();

    virtual com::sun::star::uno::Reference< com::sun::star::sdbc::XRow >
                          queryPropertyValues( sal_uInt32 nIndex  );
    virtual void          releasePropertyValues( sal_uInt32 nIndex );
    virtual void          close();
    virtual void          validate()
        throw( com::sun::star::ucb::ResultSetException );
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
