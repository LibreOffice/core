/*************************************************************************
 *
 *  $RCSfile: dbtoolsclient.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2001-07-27 08:06:47 $
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
#ifndef _COM_SUN_STAR_SDB_SQLCONTEXT_HPP_
#include <com/sun/star/sdb/SQLContext.hpp>
#endif
#ifndef SVX_DBTOOLSCLIENT_HXX
#include "dbtoolsclient.hxx"
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

//........................................................................
namespace svxform
{
//........................................................................

    using namespace ::connectivity::simple;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::util;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::sdb;

    //====================================================================
    //= ODbtoolsClient
    //====================================================================
    ::osl::Mutex    ODbtoolsClient::s_aMutex;
    sal_Int32       ODbtoolsClient::s_nClients = 0;
    oslModule       ODbtoolsClient::s_hDbtoolsModule = NULL;
    createDataAccessToolsFactoryFunction
                    ODbtoolsClient::s_pFactoryCreationFunc = NULL;

    //--------------------------------------------------------------------
    ODbtoolsClient::ODbtoolsClient()
    {
        registerClient();
        if (s_pFactoryCreationFunc)
        {   // loading the lib succeeded
            void* pUntypedFactory = (*s_pFactoryCreationFunc)();
            IDataAccessToolsFactory* pDBTFactory = static_cast<IDataAccessToolsFactory*>(pUntypedFactory);
            OSL_ENSURE(pDBTFactory, "ODbtoolsClient::ODbtoolsClient: no factory returned!");
            if (pDBTFactory)
            {
                m_xDataAccessFactory = pDBTFactory;
                // by definition, the factory was aquired once
                m_xDataAccessFactory->release();
            }

        }
    }

    //--------------------------------------------------------------------
    ODbtoolsClient::~ODbtoolsClient()
    {
        // clear the factory _before_ revoking the client
        // (the revocation may unload the DBT lib)
        m_xDataAccessFactory = NULL;
        // revoke the client
        revokeClient();
    }

    //--------------------------------------------------------------------
    void ODbtoolsClient::registerClient()
    {
        ::osl::MutexGuard aGuard(s_aMutex);
        if (1 == ++s_nClients)
        {
            OSL_ENSURE(NULL == s_hDbtoolsModule, "ODbtoolsClient::registerClient: inconsistence: already have a module!");
            OSL_ENSURE(NULL == s_pFactoryCreationFunc, "ODbtoolsClient::registerClient: inconsistence: already have a factory function!");

            const ::rtl::OUString sModuleName = ::rtl::OUString::createFromAscii(
                SAL_MODULENAME_WITH_VERSION( "dbtools", "2" )
            );

            // load the dbtools library
            s_hDbtoolsModule = osl_loadModule(sModuleName.pData, 0);
            OSL_ENSURE(NULL != s_hDbtoolsModule, "ODbtoolsClient::registerClient: could not load the dbtools library!");
            if (NULL != s_hDbtoolsModule)
            {
                // get the symbol for the method creating the factory
                const ::rtl::OUString sFactoryCreationFunc = ::rtl::OUString::createFromAscii("createDataAccessToolsFactory");
                s_pFactoryCreationFunc = reinterpret_cast<createDataAccessToolsFactoryFunction>(
                    osl_getSymbol(s_hDbtoolsModule, sFactoryCreationFunc.pData));

                if (NULL == s_pFactoryCreationFunc)
                {   // did not find the symbol
                    OSL_ENSURE(sal_False, "ODbtoolsClient::registerClient: could not find the symbol for creating the factory!");
                    osl_unloadModule(s_hDbtoolsModule);
                    s_hDbtoolsModule = NULL;
                }
            }
        }
    }

    //--------------------------------------------------------------------
    void ODbtoolsClient::revokeClient()
    {
        ::osl::MutexGuard aGuard(s_aMutex);
        if (0 == --s_nClients)
        {
            s_pFactoryCreationFunc = NULL;
            if (s_hDbtoolsModule)
                osl_unloadModule(s_hDbtoolsModule);
            s_hDbtoolsModule = NULL;
        }
    }

    //====================================================================
    //= OStaticDataAccessTools
    //====================================================================
    //--------------------------------------------------------------------
    OStaticDataAccessTools::OStaticDataAccessTools()
    {
        if (getFactory().is())
            m_xDataAccessTools = getFactory()->getDataAccessTools();
    }

    //--------------------------------------------------------------------
    Reference< XNumberFormatsSupplier > OStaticDataAccessTools::getNumberFormats(const Reference< XConnection>& _rxConn, sal_Bool _bAllowDefault) const
    {
        Reference< XNumberFormatsSupplier > xReturn;
        if (m_xDataAccessTools.is())
            xReturn = m_xDataAccessTools->getNumberFormats(_rxConn, _bAllowDefault);
        return xReturn;
    }

    //--------------------------------------------------------------------
    Reference< XConnection> OStaticDataAccessTools::getConnection_withFeedback(const ::rtl::OUString& _rDataSourceName,
        const ::rtl::OUString& _rUser, const ::rtl::OUString& _rPwd, const Reference< XMultiServiceFactory>& _rxFactory) const
            SAL_THROW ( (SQLException) )
    {
        Reference< XConnection > xReturn;
        if (m_xDataAccessTools.is())
            xReturn = m_xDataAccessTools->getConnection_withFeedback(_rDataSourceName, _rUser, _rPwd, _rxFactory);
        return xReturn;
    }

    //--------------------------------------------------------------------
    Reference< XConnection > OStaticDataAccessTools::calcConnection(const Reference< XRowSet >& _rxRowSet, const Reference< XMultiServiceFactory >& _rxFactory) const SAL_THROW ( (SQLException, RuntimeException) )
    {
        Reference< XConnection > xReturn;
        if (m_xDataAccessTools.is())
            xReturn = m_xDataAccessTools->calcConnection(_rxRowSet, _rxFactory);
        return xReturn;
    }

    //--------------------------------------------------------------------
    void OStaticDataAccessTools::TransferFormComponentProperties(const Reference< XPropertySet>& _rxOld,
        const Reference< XPropertySet>& _rxNew, const Locale& _rLocale) const
    {
        if (m_xDataAccessTools.is())
            m_xDataAccessTools->TransferFormComponentProperties(_rxOld, _rxNew, _rLocale);
    }

    //--------------------------------------------------------------------
    ::rtl::OUString OStaticDataAccessTools::quoteName(const ::rtl::OUString& _rQuote, const ::rtl::OUString& _rName) const
    {
        ::rtl::OUString sReturn;
        if (m_xDataAccessTools.is())
            sReturn = m_xDataAccessTools->quoteName(_rQuote, _rName);
        return sReturn;
    }

    //--------------------------------------------------------------------
    ::rtl::OUString OStaticDataAccessTools::quoteTableName(const Reference< XDatabaseMetaData>& _rxMeta, const ::rtl::OUString& _rName) const
    {
        ::rtl::OUString sReturn;
        if (m_xDataAccessTools.is())
            sReturn = m_xDataAccessTools->quoteTableName(_rxMeta, _rName);
        return sReturn;
    }

    //--------------------------------------------------------------------
    SQLContext OStaticDataAccessTools::prependContextInfo(SQLException& _rException, const Reference< XInterface >& _rxContext,
        const ::rtl::OUString& _rContextDescription, const ::rtl::OUString& _rContextDetails) const
    {
        SQLContext aReturn;
        if (m_xDataAccessTools.is())
            aReturn = m_xDataAccessTools->prependContextInfo(_rException, _rxContext, _rContextDescription, _rContextDetails);
        return aReturn;
    }

//........................................................................
}   // namespace svxform
//........................................................................

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.1  2001/07/25 13:34:38  fs
 *  initial checkin - base class for accessing DBTOOLS with load-on-demand
 *
 *
 *  Revision 1.0 24.07.01 16:55:54  fs
 ************************************************************************/
