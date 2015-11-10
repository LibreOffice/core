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

#include "flat/EDriver.hxx"
#include "flat/EConnection.hxx"
#include <com/sun/star/lang/DisposedException.hpp>
#include <connectivity/dbexception.hxx>
#include <comphelper/sequence.hxx>
#include "resource/common_res.hrc"
#include "resource/sharedresources.hxx"
#include <comphelper/processfactory.hxx>


using namespace connectivity::flat;
using namespace connectivity::file;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::lang;


// static ServiceInfo

OUString ODriver::getImplementationName_Static(  ) throw(RuntimeException)
{
    return OUString("com.sun.star.comp.sdbc.flat.ODriver");
}


OUString SAL_CALL ODriver::getImplementationName(  ) throw(RuntimeException, std::exception)
{
    return getImplementationName_Static();
}


::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >  SAL_CALL connectivity::flat::ODriver_CreateInstance(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory) throw( ::com::sun::star::uno::Exception )
{
    return *(new ODriver( comphelper::getComponentContext(_rxFactory) ));
}

Reference< XConnection > SAL_CALL ODriver::connect( const OUString& url, const Sequence< PropertyValue >& info ) throw(SQLException, RuntimeException, std::exception)
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
                throw(SQLException, RuntimeException, std::exception)
{
    return url.startsWith("sdbc:flat:");
}

Sequence< DriverPropertyInfo > SAL_CALL ODriver::getPropertyInfo( const OUString& url, const Sequence< PropertyValue >& info ) throw(SQLException, RuntimeException, std::exception)
{
    if ( acceptsURL(url) )
    {
        ::std::vector< DriverPropertyInfo > aDriverInfo;

        Sequence< OUString > aBoolean(2);
        aBoolean[0] = "0";
        aBoolean[1] = "1";

        aDriverInfo.push_back(DriverPropertyInfo(
                OUString("FieldDelimiter")
                ,OUString("Field separator.")
                ,sal_False
                ,OUString()
                ,Sequence< OUString >())
                );
        aDriverInfo.push_back(DriverPropertyInfo(
                OUString("HeaderLine")
                ,OUString("Text contains headers.")
                ,sal_False
                ,OUString("0")
                ,aBoolean)
                );
        aDriverInfo.push_back(DriverPropertyInfo(
                OUString("StringDelimiter")
                ,OUString("Text separator.")
                ,sal_False
                ,OUString("0")
                ,aBoolean)
                );
        aDriverInfo.push_back(DriverPropertyInfo(
                OUString("DecimalDelimiter")
                ,OUString("Decimal separator.")
                ,sal_False
                ,OUString("0")
                ,aBoolean)
                );
        aDriverInfo.push_back(DriverPropertyInfo(
                OUString("ThousandDelimiter")
                ,OUString("Thousands separator.")
                ,sal_False
                ,OUString("0")
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
