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

#include <dbase/DDriver.hxx>
#include <dbase/DConnection.hxx>
#include <com/sun/star/lang/DisposedException.hpp>
#include <connectivity/dbexception.hxx>
#include <strings.hrc>

using namespace connectivity::dbase;
using namespace connectivity::file;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::lang;


// XServiceInfo

OUString SAL_CALL ODriver::getImplementationName(  )
{
    return u"com.sun.star.comp.sdbc.dbase.ODriver"_ustr;
}


extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
connectivity_dbase_ODriver(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const&)
{
    rtl::Reference<ODriver> ret;
    try {
        ret = new ODriver(context);
    } catch (...) {
    }
    if (ret)
        ret->acquire();
    return getXWeak(ret.get());
}


Reference< XConnection > SAL_CALL ODriver::connect( const OUString& url, const Sequence< PropertyValue >& info )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODriver_BASE::rBHelper.bDisposed)
        throw DisposedException();

    if ( ! acceptsURL(url) )
        return nullptr;

    rtl::Reference<ODbaseConnection> pCon = new ODbaseConnection(this);
    pCon->construct(url,info);
    m_xConnections.push_back(WeakReferenceHelper(*pCon));

    return pCon;
}

sal_Bool SAL_CALL ODriver::acceptsURL( const OUString& url )
{
    return url.startsWith("sdbc:dbase:");
}

Sequence< DriverPropertyInfo > SAL_CALL ODriver::getPropertyInfo( const OUString& url, const Sequence< PropertyValue >& /*info*/ )
{
    if ( acceptsURL(url) )
    {
        Sequence< OUString > aBoolean { u"0"_ustr, u"1"_ustr };

        return
        {
            {
                u"CharSet"_ustr
                ,u"CharSet of the database."_ustr
                ,false
                ,OUString()
                ,Sequence< OUString >()
            },
            {
                u"ShowDeleted"_ustr
                ,u"Display inactive records."_ustr
                ,false
                ,u"0"_ustr
                ,aBoolean
            },
            {
                u"EnableSQL92Check"_ustr
                ,u"Use SQL92 naming constraints."_ustr
                ,false
                ,u"0"_ustr
                ,aBoolean
            }
        };
    }

    SharedResources aResources;
    const OUString sMessage = aResources.getResourceString(STR_URI_SYNTAX_ERROR);
    ::dbtools::throwGenericSQLException(sMessage ,*this);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
