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

#ifndef GIO_DATASUPPLIER_HXX
#define GIO_DATASUPPLIER_HXX

#include <ucbhelper/resultset.hxx>
#include "gio_content.hxx"
#include <vector>

namespace gio
{

class Content;

struct ResultListEntry
{
    OUString aId;
    com::sun::star::uno::Reference< com::sun::star::ucb::XContentIdentifier > xId;
    com::sun::star::uno::Reference< com::sun::star::ucb::XContent > xContent;
    com::sun::star::uno::Reference< com::sun::star::sdbc::XRow > xRow;
    GFileInfo *pInfo;

    ResultListEntry( GFileInfo *pInInfo ) : pInfo(pInInfo)
    {
        g_object_ref( pInfo );
    }

    ~ResultListEntry()
    {
        g_object_unref( pInfo );
    }
};

typedef std::vector< ResultListEntry* > ResultList;

class DataSupplier : public ucbhelper::ResultSetDataSupplier
{
private:
    com::sun::star::uno::Reference< ::gio::Content > mxContent;
    com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory > m_xSMgr;
    sal_Int32 mnOpenMode;
    bool mbCountFinal;
    bool getData();
    ResultList maResults;
public:
    DataSupplier( const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& rxSMgr,
        const com::sun::star::uno::Reference< Content >& rContent, sal_Int32 nOpenMode );
    virtual ~DataSupplier();

    virtual OUString queryContentIdentifierString( sal_uInt32 nIndex ) SAL_OVERRIDE;
    virtual com::sun::star::uno::Reference< com::sun::star::ucb::XContentIdentifier >
        queryContentIdentifier( sal_uInt32 nIndex ) SAL_OVERRIDE;
    virtual com::sun::star::uno::Reference< com::sun::star::ucb::XContent >
        queryContent( sal_uInt32 nIndex ) SAL_OVERRIDE;

    virtual bool getResult( sal_uInt32 nIndex ) SAL_OVERRIDE;

    virtual sal_uInt32 totalCount() SAL_OVERRIDE;
    virtual sal_uInt32 currentCount() SAL_OVERRIDE;
    virtual bool isCountFinal() SAL_OVERRIDE;

    virtual com::sun::star::uno::Reference< com::sun::star::sdbc::XRow >
        queryPropertyValues( sal_uInt32 nIndex  ) SAL_OVERRIDE;
    virtual void releasePropertyValues( sal_uInt32 nIndex ) SAL_OVERRIDE;

    virtual void close() SAL_OVERRIDE;

    virtual void validate()
        throw( com::sun::star::ucb::ResultSetException ) SAL_OVERRIDE;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
