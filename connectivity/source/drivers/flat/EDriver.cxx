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
#include <comphelper/processfactory.hxx>


using namespace connectivity::flat;
using namespace connectivity::file;
using namespace css::uno;
using namespace css::beans;
using namespace css::sdbcx;
using namespace css::sdbc;
using namespace css::lang;


// static ServiceInfo

OUString ODriver::getImplementationName_Static(  )
{
    return OUString("com.sun.star.comp.sdbc.flat.ODriver");
}


OUString SAL_CALL ODriver::getImplementationName(  )
{
    return getImplementationName_Static();
}


css::uno::Reference< css::uno::XInterface > connectivity::flat::ODriver_CreateInstance(const css::uno::Reference< css::lang::XMultiServiceFactory >& _rxFactory)
{
    return *(new ODriver( comphelper::getComponentContext(_rxFactory) ));
}

Reference< XConnection > SAL_CALL ODriver::connect( const OUString& url, const Sequence< PropertyValue >& info )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODriver_BASE::rBHelper.bDisposed)
       throw DisposedException();

    if ( ! acceptsURL(url) )
        return nullptr;

    OFlatConnection* pCon = new OFlatConnection(this);
    pCon->construct(url,info);
    Reference< XConnection > xCon = pCon;
    m_xConnections.push_back(WeakReferenceHelper(*pCon));

    return xCon;
}

sal_Bool SAL_CALL ODriver::acceptsURL( const OUString& url )
{
    return url.startsWith("sdbc:flat:");
}

Sequence< DriverPropertyInfo > SAL_CALL ODriver::getPropertyInfo( const OUString& url, const Sequence< PropertyValue >& info )
{
    if ( acceptsURL(url) )
    {
        std::vector< DriverPropertyInfo > aDriverInfo;

        Sequence< OUString > aBoolean(2);
        aBoolean[0] = "0";
        aBoolean[1] = "1";

        aDriverInfo.push_back(DriverPropertyInfo(
                "FieldDelimiter"
                ,"Field separator."
                ,false
                ,OUString()
                ,Sequence< OUString >())
                );
        aDriverInfo.push_back(DriverPropertyInfo(
                "HeaderLine"
                ,"Text contains headers."
                ,false
                ,"0"
                ,aBoolean)
                );
        aDriverInfo.push_back(DriverPropertyInfo(
                "StringDelimiter"
                ,"Text separator."
                ,false
                ,"0"
                ,aBoolean)
                );
        aDriverInfo.push_back(DriverPropertyInfo(
                "DecimalDelimiter"
                ,"Decimal separator."
                ,false
                ,"0"
                ,aBoolean)
                );
        aDriverInfo.push_back(DriverPropertyInfo(
                "ThousandDelimiter"
                ,"Thousands separator."
                ,false
                ,"0"
                ,aBoolean)
                );
        return ::comphelper::concatSequences(OFileDriver::getPropertyInfo(url,info ),
                                             Sequence< DriverPropertyInfo >(&aDriverInfo[0],aDriverInfo.size()));
    }
    ::connectivity::SharedResources aResources;
    const OUString sMessage = aResources.getResourceString(STR_URI_SYNTAX_ERROR);
    ::dbtools::throwGenericSQLException(sMessage ,*this);
    return Sequence< DriverPropertyInfo >();
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
