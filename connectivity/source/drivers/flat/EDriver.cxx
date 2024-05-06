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

#include <flat/EDriver.hxx>
#include <flat/EConnection.hxx>
#include <com/sun/star/lang/DisposedException.hpp>
#include <connectivity/dbexception.hxx>
#include <comphelper/sequence.hxx>
#include <strings.hrc>
#include <resource/sharedresources.hxx>


using namespace connectivity::flat;
using namespace connectivity::file;
using namespace css::uno;
using namespace css::beans;
using namespace css::sdbcx;
using namespace css::sdbc;
using namespace css::lang;


// XServiceInfo

OUString SAL_CALL ODriver::getImplementationName(  )
{
    return u"com.sun.star.comp.sdbc.flat.ODriver"_ustr;
}


extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
connectivity_flat_ODriver(
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

    rtl::Reference<OFlatConnection> pCon = new OFlatConnection(this);
    pCon->construct(url,info);
    m_xConnections.emplace_back(*pCon);

    return pCon;
}

sal_Bool SAL_CALL ODriver::acceptsURL( const OUString& url )
{
    return url.startsWith("sdbc:flat:");
}

Sequence< DriverPropertyInfo > SAL_CALL ODriver::getPropertyInfo( const OUString& url, const Sequence< PropertyValue >& info )
{
    if ( acceptsURL(url) )
    {
        Sequence< OUString > aBoolean { u"0"_ustr, u"1"_ustr };

        std::vector< DriverPropertyInfo > aDriverInfo
        {
            {
                u"FieldDelimiter"_ustr
                ,u"Field separator."_ustr
                ,false
                ,{}
                ,{}
            },
            {
                u"HeaderLine"_ustr
                ,u"Text contains headers."_ustr
                ,false
                ,u"0"_ustr
                ,aBoolean
            },
            {
                u"StringDelimiter"_ustr
                ,u"Text separator."_ustr
                ,false
                ,u"0"_ustr
                ,aBoolean
            },
            {
                u"DecimalDelimiter"_ustr
                ,u"Decimal separator."_ustr
                ,false
                ,u"0"_ustr
                ,aBoolean
            },
            {
                u"ThousandDelimiter"_ustr
                ,u"Thousands separator."_ustr
                ,false
                ,u"0"_ustr
                ,aBoolean
            }
        };
        return ::comphelper::concatSequences(OFileDriver::getPropertyInfo(url,info ),
                                             Sequence< DriverPropertyInfo >(aDriverInfo.data(),aDriverInfo.size()));
    }
    ::connectivity::SharedResources aResources;
    const OUString sMessage = aResources.getResourceString(STR_URI_SYNTAX_ERROR);
    ::dbtools::throwGenericSQLException(sMessage ,*this);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
