/*************************************************************************
 *
 *  $RCSfile: swdbtoolsclient.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hjs $ $Date: 2004-06-28 16:55:25 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATSSUPPLIER_HPP_
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XDATASOURCE_HPP_
#include <com/sun/star/sdbc/XDataSource.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_SQLCONTEXT_HPP_
#include <com/sun/star/sdb/SQLContext.hpp>
#endif
#ifndef _SWDBTOOLSCLIENT_HXX
#include <swdbtoolsclient.hxx>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

//........................................................................

using namespace ::connectivity::simple;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdb;

//====================================================================
//= SwDbtoolsClient
//====================================================================
namespace
{
    // -----------------------------------------------------------------------------
    // this namespace contains access to all static members of the class SwDbtoolsClient
    // to make the initialize of the dll a little bit faster
    // -----------------------------------------------------------------------------
    ::osl::Mutex& getDbtoolsClientMutex()
    {
        static  ::osl::Mutex aMutex;
        return aMutex;
    }
    // -----------------------------------------------------------------------------
    sal_Int32& getDbToolsClientClients()
    {
        static  sal_Int32 nClients = 0;
        return nClients;
    }
    // -----------------------------------------------------------------------------
    oslModule& getDbToolsClientModule()
    {
        static oslModule hDbtoolsModule = NULL;
        return hDbtoolsModule;
    }
    // -----------------------------------------------------------------------------
    createDataAccessToolsFactoryFunction& getDbToolsClientFactoryFunction()
    {
        static createDataAccessToolsFactoryFunction pFactoryCreationFunc = NULL;
        return pFactoryCreationFunc;
    }
    // -----------------------------------------------------------------------------
}
// -----------------------------------------------------------------------------
SwDbtoolsClient::SwDbtoolsClient()
{
}

//--------------------------------------------------------------------
SwDbtoolsClient::~SwDbtoolsClient()
{
    if(m_xDataAccessFactory.is())
    {
        // clear the factory _before_ revoking the client
        // (the revocation may unload the DBT lib)
        m_xDataAccessFactory = NULL;
        // revoke the client
        revokeClient();
    }
}

//--------------------------------------------------------------------
void SwDbtoolsClient::registerClient()
{
    ::osl::MutexGuard aGuard(getDbtoolsClientMutex());
    if (1 == ++getDbToolsClientClients())
    {
        OSL_ENSURE(NULL == getDbToolsClientModule(), "SwDbtoolsClient::registerClient: inconsistence: already have a module!");
        OSL_ENSURE(NULL == getDbToolsClientFactoryFunction(), "SwDbtoolsClient::registerClient: inconsistence: already have a factory function!");

        const ::rtl::OUString sModuleName = ::rtl::OUString::createFromAscii(
            SVLIBRARY( "dbtools" )
        );

        // load the dbtools library
        getDbToolsClientModule() = osl_loadModule(sModuleName.pData, 0);
        OSL_ENSURE(NULL != getDbToolsClientModule(), "SwDbtoolsClient::registerClient: could not load the dbtools library!");
        if (NULL != getDbToolsClientModule())
        {
            // get the symbol for the method creating the factory
            const ::rtl::OUString sFactoryCreationFunc = ::rtl::OUString::createFromAscii("createDataAccessToolsFactory");
            //  reinterpret_cast<createDataAccessToolsFactoryFunction> removed for gcc permissive
            getDbToolsClientFactoryFunction() = (createDataAccessToolsFactoryFunction)(
                osl_getSymbol(getDbToolsClientModule(), sFactoryCreationFunc.pData));

            if (NULL == getDbToolsClientFactoryFunction())
            {   // did not find the symbol
                OSL_ENSURE(sal_False, "SwDbtoolsClient::registerClient: could not find the symbol for creating the factory!");
                osl_unloadModule(getDbToolsClientModule());
                getDbToolsClientModule() = NULL;
            }
        }
    }
}

//--------------------------------------------------------------------
void SwDbtoolsClient::revokeClient()
{
    ::osl::MutexGuard aGuard(getDbtoolsClientMutex());
    if (0 == --getDbToolsClientClients())
    {
        getDbToolsClientFactoryFunction() = NULL;
        if (getDbToolsClientModule())
            osl_unloadModule(getDbToolsClientModule());
        getDbToolsClientModule() = NULL;
    }
}
/* -----------------------------30.08.2001 14:58------------------------------

 ---------------------------------------------------------------------------*/
void SwDbtoolsClient::getFactory()
{
    if(!m_xDataAccessFactory.is())
    {
        registerClient();
        if(getDbToolsClientFactoryFunction())
        {   // loading the lib succeeded
            void* pUntypedFactory = (*getDbToolsClientFactoryFunction())();
            IDataAccessToolsFactory* pDBTFactory = static_cast<IDataAccessToolsFactory*>(pUntypedFactory);
            OSL_ENSURE(pDBTFactory, "SwDbtoolsClient::SwDbtoolsClient: no factory returned!");
            if (pDBTFactory)
            {
                m_xDataAccessFactory = pDBTFactory;
                // by definition, the factory was aquired once
                m_xDataAccessFactory->release();
            }
        }
    }
}
/* -----------------------------30.08.2001 11:32------------------------------

 ---------------------------------------------------------------------------*/
::rtl::Reference< ::connectivity::simple::IDataAccessTools >
    SwDbtoolsClient::getDataAccessTools()
{
    if(!m_xDataAccessTools.is())
    {
        getFactory();
        if(m_xDataAccessFactory.is())
            m_xDataAccessTools = m_xDataAccessFactory->getDataAccessTools();
    }
    return m_xDataAccessTools;
}
/* -----------------------------30.08.2001 12:40------------------------------

 ---------------------------------------------------------------------------*/
::rtl::Reference< ::connectivity::simple::IDataAccessTypeConversion >
    SwDbtoolsClient::getAccessTypeConversion()
{
    if(!m_xAccessTypeConversion.is())
    {
        getFactory();
        if(m_xDataAccessFactory.is())
            m_xAccessTypeConversion = m_xDataAccessFactory->getTypeConversionHelper();
    }
    return m_xAccessTypeConversion;
}

/* -----------------------------30.08.2001 11:37------------------------------

 ---------------------------------------------------------------------------*/
Reference< XDataSource > SwDbtoolsClient::getDataSource(
        const ::rtl::OUString& rRegisteredName,
        const Reference< XMultiServiceFactory>& xFactory
            )
{
    Reference< XDataSource > xRet;
    ::rtl::Reference< ::connectivity::simple::IDataAccessTools >    xAccess = getDataAccessTools();
    if(xAccess.is())
        xRet = xAccess->getDataSource(rRegisteredName, xFactory);
    return xRet;
}
/* -----------------------------30.08.2001 12:06------------------------------

 ---------------------------------------------------------------------------*/
sal_Int32 SwDbtoolsClient::getDefaultNumberFormat(
        const Reference< XPropertySet >& rxColumn,
        const Reference< XNumberFormatTypes >& rxTypes,
        const Locale& rLocale
            )
{
    sal_Int32 nRet = -1;
    ::rtl::Reference< ::connectivity::simple::IDataAccessTools >    xAccess = getDataAccessTools();
    if(xAccess.is())
        nRet = xAccess->getDefaultNumberFormat( rxColumn, rxTypes, rLocale);
    return nRet;
}
/* -----------------------------30.08.2001 12:38------------------------------

 ---------------------------------------------------------------------------*/
::rtl::OUString SwDbtoolsClient::getValue(
        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _rxColumn,
        const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter>& _rxFormatter,
        const ::com::sun::star::lang::Locale& _rLocale,
        const ::com::sun::star::util::Date& _rNullDate
            )

{
    ::rtl::Reference< ::connectivity::simple::IDataAccessTypeConversion > xConversion =
                    getAccessTypeConversion();
    rtl::OUString sRet;
    if(xConversion.is())
        sRet = xConversion->getValue(_rxColumn, _rxFormatter, _rLocale, _rNullDate);
    return sRet;
}

