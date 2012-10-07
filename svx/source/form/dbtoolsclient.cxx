/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdbc/XDataSource.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <com/sun/star/sdb/SQLContext.hpp>
#include "svx/dbtoolsclient.hxx"
#include <osl/diagnose.h>
#include <rtl/instance.hxx>
#include <connectivity/formattedcolumnvalue.hxx>

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
    using namespace ::com::sun::star::container;

    //====================================================================
    //= ODbtoolsClient
    //====================================================================

    namespace
    {
        struct theODbtoolsClientMutex : public rtl::Static< osl::Mutex, theODbtoolsClientMutex> {};
    }

    sal_Int32       ODbtoolsClient::s_nClients = 0;
    oslModule       ODbtoolsClient::s_hDbtoolsModule = NULL;
    createDataAccessToolsFactoryFunction
                    ODbtoolsClient::s_pFactoryCreationFunc = NULL;

    //--------------------------------------------------------------------
    ODbtoolsClient::ODbtoolsClient()
    {
        m_bCreateAlready = sal_False;
    }

    //--------------------------------------------------------------------
    bool ODbtoolsClient::ensureLoaded() const
    {
        if ( !m_bCreateAlready )
        {
            m_bCreateAlready = true;

            registerClient();
            if ( s_pFactoryCreationFunc )
            {   // loading the lib succeeded
                void* pUntypedFactory = (*s_pFactoryCreationFunc)();
                IDataAccessToolsFactory* pDBTFactory = static_cast< IDataAccessToolsFactory* >( pUntypedFactory );
                OSL_ENSURE( pDBTFactory, "ODbtoolsClient::ODbtoolsClient: no factory returned!" );
                if ( pDBTFactory )
                {
                    m_xDataAccessFactory = pDBTFactory;
                    // by definition, the factory was aquired once
                    m_xDataAccessFactory->release();
                }
            }
        }
        return m_xDataAccessFactory.is();
    }

    //--------------------------------------------------------------------
    ODbtoolsClient::~ODbtoolsClient()
    {
        // clear the factory _before_ revoking the client
        // (the revocation may unload the DBT lib)
        m_xDataAccessFactory = NULL;
        // revoke the client
        if ( m_bCreateAlready )
            revokeClient();
    }

    //--------------------------------------------------------------------

#ifndef DISABLE_DYNLOADING

    extern "C" { static void SAL_CALL thisModule() {} }

#else

    extern "C" void * createDataAccessToolsFactory();

#endif

    void ODbtoolsClient::registerClient()
    {
        ::osl::MutexGuard aGuard(theODbtoolsClientMutex::get());
        if (1 == ++s_nClients)
        {
            OSL_ENSURE(NULL == s_hDbtoolsModule, "ODbtoolsClient::registerClient: inconsistence: already have a module!");
            OSL_ENSURE(NULL == s_pFactoryCreationFunc, "ODbtoolsClient::registerClient: inconsistence: already have a factory function!");

#ifndef DISABLE_DYNLOADING
            const ::rtl::OUString sModuleName( SVLIBRARY( "dbtools" )
            );

            // load the dbtools library
            s_hDbtoolsModule = osl_loadModuleRelative(
                &thisModule, sModuleName.pData, 0);
            OSL_ENSURE(NULL != s_hDbtoolsModule, "ODbtoolsClient::registerClient: could not load the dbtools library!");
            if (NULL != s_hDbtoolsModule)
            {
                // get the symbol for the method creating the factory
                const ::rtl::OUString sFactoryCreationFunc( "createDataAccessToolsFactory" );
                //  reinterpret_cast<createDataAccessToolsFactoryFunction>
                s_pFactoryCreationFunc = (createDataAccessToolsFactoryFunction)(
                    osl_getFunctionSymbol(s_hDbtoolsModule, sFactoryCreationFunc.pData));

                if (NULL == s_pFactoryCreationFunc)
                {   // did not find the symbol
                    OSL_FAIL("ODbtoolsClient::registerClient: could not find the symbol for creating the factory!");
                    osl_unloadModule(s_hDbtoolsModule);
                    s_hDbtoolsModule = NULL;
                }
            }
#else
            s_pFactoryCreationFunc = createDataAccessToolsFactory;
#endif
        }
    }

    //--------------------------------------------------------------------
    void ODbtoolsClient::revokeClient()
    {
        ::osl::MutexGuard aGuard(theODbtoolsClientMutex::get());
        if (0 == --s_nClients)
        {
#ifndef DISABLE_DYNLOADING
            s_pFactoryCreationFunc = NULL;
            if (s_hDbtoolsModule)
                osl_unloadModule(s_hDbtoolsModule);
#endif
            s_hDbtoolsModule = NULL;
        }

        OSL_ENSURE(s_nClients >= 0,"Illegall call of revokeClient()");
    }

    //====================================================================
    //= OStaticDataAccessTools
    //====================================================================
    //--------------------------------------------------------------------
    OStaticDataAccessTools::OStaticDataAccessTools()
    {
    }

    //--------------------------------------------------------------------

    bool OStaticDataAccessTools::ensureLoaded() const
    {
        if ( !ODbtoolsClient::ensureLoaded() )
            return false;
         m_xDataAccessTools = getFactory()->getDataAccessTools();
        return m_xDataAccessTools.is();
    }

    //--------------------------------------------------------------------
    Reference< XNumberFormatsSupplier > OStaticDataAccessTools::getNumberFormats(const Reference< XConnection>& _rxConn, sal_Bool _bAllowDefault) const
    {
        Reference< XNumberFormatsSupplier > xReturn;
        if ( ensureLoaded() )
            xReturn = m_xDataAccessTools->getNumberFormats(_rxConn, _bAllowDefault);
        return xReturn;
    }

    //--------------------------------------------------------------------
    sal_Int32 OStaticDataAccessTools::getDefaultNumberFormat( const Reference< XPropertySet >& _xColumn, const Reference< XNumberFormatTypes >& _xTypes, const Locale& _rLocale )
    {
        sal_Int32 nReturn = 0;
        if ( ensureLoaded() )
            nReturn = m_xDataAccessTools->getDefaultNumberFormat( _xColumn, _xTypes, _rLocale );
        return nReturn;
    }

    //--------------------------------------------------------------------
    Reference< XConnection> OStaticDataAccessTools::getConnection_withFeedback(const ::rtl::OUString& _rDataSourceName,
        const ::rtl::OUString& _rUser, const ::rtl::OUString& _rPwd, const Reference<XComponentContext>& _rxContext) const
            SAL_THROW ( (SQLException) )
    {
        Reference< XConnection > xReturn;
        if ( ensureLoaded() )
            xReturn = m_xDataAccessTools->getConnection_withFeedback(_rDataSourceName, _rUser, _rPwd, _rxContext);
        return xReturn;
    }

    //--------------------------------------------------------------------
    Reference< XConnection > OStaticDataAccessTools::connectRowset( const Reference< XRowSet >& _rxRowSet,
        const Reference< XMultiServiceFactory >& _rxFactory, sal_Bool _bSetAsActiveConnection ) const
        SAL_THROW ( ( SQLException, WrappedTargetException, RuntimeException ) )
    {
        Reference< XConnection > xReturn;
        if ( ensureLoaded() )
            xReturn = m_xDataAccessTools->connectRowset( _rxRowSet, _rxFactory, _bSetAsActiveConnection );
        return xReturn;
    }

    //--------------------------------------------------------------------
    Reference< XConnection > OStaticDataAccessTools::getRowSetConnection(const Reference< XRowSet >& _rxRowSet) const SAL_THROW ( (RuntimeException) )
    {
        Reference< XConnection > xReturn;
        if ( ensureLoaded() )
            xReturn = m_xDataAccessTools->getRowSetConnection(_rxRowSet);
        return xReturn;
    }

    //--------------------------------------------------------------------
    void OStaticDataAccessTools::TransferFormComponentProperties(const Reference< XPropertySet>& _rxOld,
        const Reference< XPropertySet>& _rxNew, const Locale& _rLocale) const
    {
        if ( ensureLoaded() )
            m_xDataAccessTools->TransferFormComponentProperties(_rxOld, _rxNew, _rLocale);
    }

    //--------------------------------------------------------------------
    ::rtl::OUString OStaticDataAccessTools::quoteName(const ::rtl::OUString& _rQuote, const ::rtl::OUString& _rName) const
    {
        ::rtl::OUString sReturn;
        if ( ensureLoaded() )
            sReturn = m_xDataAccessTools->quoteName(_rQuote, _rName);
        return sReturn;
    }

    // ------------------------------------------------
    ::rtl::OUString OStaticDataAccessTools::composeTableNameForSelect( const Reference< XConnection >& _rxConnection, const Reference< XPropertySet>& _xTable ) const
    {
        ::rtl::OUString sReturn;
        if ( ensureLoaded() )
            sReturn = m_xDataAccessTools->composeTableNameForSelect( _rxConnection, _xTable );
        return sReturn;
    }

    //----------------------------------------------------------------
    Reference< XDataSource > OStaticDataAccessTools::getDataSource( const ::rtl::OUString& _rsRegisteredName, const Reference< XComponentContext>& _rxContext ) const
    {
        Reference< XDataSource > xReturn;
        if ( ensureLoaded() )
            xReturn = m_xDataAccessTools->getDataSource(_rsRegisteredName,_rxContext);
        return xReturn;
    }

    //----------------------------------------------------------------
    sal_Bool OStaticDataAccessTools::canInsert(const Reference< XPropertySet>& _rxCursorSet) const
    {
        sal_Bool bRet = sal_False;
        if ( ensureLoaded() )
            bRet = m_xDataAccessTools->canInsert( _rxCursorSet );
        return bRet;
    }

    //----------------------------------------------------------------
    sal_Bool OStaticDataAccessTools::canUpdate(const Reference< XPropertySet>& _rxCursorSet) const
    {
        sal_Bool bRet = sal_False;
        if ( ensureLoaded() )
            bRet = m_xDataAccessTools->canUpdate( _rxCursorSet );
        return bRet;
    }

    //----------------------------------------------------------------
    Reference< XNameAccess > OStaticDataAccessTools::getFieldsByCommandDescriptor( const Reference< XConnection >& _rxConnection,
        const sal_Int32 _nCommandType, const ::rtl::OUString& _rCommand,
            Reference< XComponent >& _rxKeepFieldsAlive, ::dbtools::SQLExceptionInfo* _pErrorInfo ) SAL_THROW( ( ) )
    {
        Reference< XNameAccess > aFields;
        if ( ensureLoaded() )
            aFields = m_xDataAccessTools->getFieldsByCommandDescriptor( _rxConnection, _nCommandType,
                _rCommand, _rxKeepFieldsAlive, _pErrorInfo );

        return aFields;
    }

    //----------------------------------------------------------------
    bool OStaticDataAccessTools::isEmbeddedInDatabase( const Reference< XInterface >& _rxComponent, Reference< XConnection >& _rxActualConnection )
    {
        bool bReturn = false;
        if ( ensureLoaded() )
            bReturn = m_xDataAccessTools->isEmbeddedInDatabase( _rxComponent, _rxActualConnection );
        return bReturn;
    }

    //----------------------------------------------------------------
    bool OStaticDataAccessTools::isEmbeddedInDatabase( const Reference< XInterface >& _rxComponent )
    {
        bool bReturn = false;
        if ( ensureLoaded() )
        {
            Reference< XConnection > xDummy;
            bReturn = m_xDataAccessTools->isEmbeddedInDatabase( _rxComponent, xDummy );
        }
        return bReturn;
    }

    //====================================================================
    //= DBToolsObjectFactory
    //====================================================================
    //----------------------------------------------------------------
    DBToolsObjectFactory::DBToolsObjectFactory()
    {
    }

    //----------------------------------------------------------------
    DBToolsObjectFactory::~DBToolsObjectFactory()
    {
    }

    //----------------------------------------------------------------
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    ::std::auto_ptr< ::dbtools::FormattedColumnValue > DBToolsObjectFactory::createFormattedColumnValue(
        const ::comphelper::ComponentContext& _rContext, const Reference< XRowSet >& _rxRowSet, const Reference< XPropertySet >& _rxColumn )
    {
        ::std::auto_ptr< ::dbtools::FormattedColumnValue > pValue;
        if ( ensureLoaded() )
            pValue = getFactory()->createFormattedColumnValue( _rContext, _rxRowSet, _rxColumn );
        return pValue;
    }
    SAL_WNODEPRECATED_DECLARATIONS_POP

//........................................................................
}   // namespace svxform
//........................................................................


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
