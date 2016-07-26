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

#include "browserids.hxx"
#include "commontypes.hxx"
#include <dbaccess/dataview.hxx>
#include "dbu_misc.hrc"
#include "dbustrings.hrc"
#include "moduledbu.hxx"
#include <dbaccess/dbsubcomponentcontroller.hxx>

#include <com/sun/star/frame/XUntitledNumbers.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/sdb/XDocumentDataSource.hpp>
#include <com/sun/star/sdb/XOfficeDatabaseDocument.hpp>
#include <com/sun/star/sdbc/XDataSource.hpp>
#include <com/sun/star/util/NumberFormatter.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>

#include <comphelper/processfactory.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/types.hxx>
#include <connectivity/dbexception.hxx>
#include <connectivity/dbtools.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <comphelper/interfacecontainer2.hxx>
#include <rtl/ustrbuf.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include <vcl/layout.hxx>

namespace dbaui
{

    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::beans::XPropertySet;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::Type;
    using ::com::sun::star::uno::XComponentContext;
    using ::com::sun::star::sdbc::XConnection;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::container::XChild;
    using ::com::sun::star::sdbc::XDataSource;
    using ::com::sun::star::util::NumberFormatter;
    using ::com::sun::star::util::XNumberFormatter;
    using ::com::sun::star::util::XNumberFormatsSupplier;
    using ::com::sun::star::frame::XFrame;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::sdbc::SQLException;
    using ::com::sun::star::lang::EventObject;
    using ::com::sun::star::beans::PropertyValue;
    using ::com::sun::star::frame::XModel;
    using ::com::sun::star::sdb::XOfficeDatabaseDocument;
    using ::com::sun::star::awt::XWindow;
    using ::com::sun::star::sdbc::XDatabaseMetaData;
    using ::com::sun::star::sdb::XDocumentDataSource;
    using ::com::sun::star::document::XEmbeddedScripts;
    using ::com::sun::star::lang::IllegalArgumentException;
    using ::com::sun::star::uno::UNO_SET_THROW;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::frame::XUntitledNumbers;
    using ::com::sun::star::beans::PropertyVetoException;

    class DataSourceHolder
    {
    public:
        DataSourceHolder()
        {
        }

        explicit DataSourceHolder(const Reference< XDataSource >& _rxDataSource)
        {
            m_xDataSource = _rxDataSource;
            Reference< XDocumentDataSource > xDocDS( m_xDataSource, UNO_QUERY );
            if ( xDocDS.is() )
                m_xDocument = xDocDS->getDatabaseDocument();

            m_xDataSourceProps.set( m_xDataSource, UNO_QUERY );
        }

        const Reference< XDataSource >&             getDataSource() const { return m_xDataSource; }
        const Reference< XPropertySet >&            getDataSourceProps() const { return m_xDataSourceProps; }
        const Reference< XOfficeDatabaseDocument >&  getDatabaseDocument() const { return m_xDocument; }

        bool is() const { return m_xDataSource.is(); }

        void clear()
        {
            m_xDataSource.clear();
            m_xDocument.clear();
        }

    private:
        Reference< XDataSource >                m_xDataSource;
        Reference< XPropertySet >               m_xDataSourceProps;
        Reference< XOfficeDatabaseDocument >    m_xDocument;
    };

    struct DBSubComponentController_Impl
    {
    private:
        ::boost::optional< bool >       m_aDocScriptSupport;

    public:
        OModuleClient                   m_aModuleClient;
        ::dbtools::SQLExceptionInfo     m_aCurrentError;

        ::comphelper::OInterfaceContainerHelper2
                                        m_aModifyListeners;

        // <properties>
        SharedConnection                m_xConnection;
        ::dbtools::DatabaseMetaData     m_aSdbMetaData;
        // </properties>
        OUString                        m_sDataSourceName;  // the data source we're working for
        DataSourceHolder                m_aDataSource;
        Reference< XNumberFormatter >   m_xFormatter;   // a number formatter working with the connection's NumberFormatsSupplier
        sal_Int32                       m_nDocStartNumber;
        bool                            m_bSuspended;   // is true when the controller was already suspended
        bool                            m_bEditable;    // is the control readonly or not
        bool                            m_bModified;    // is the data modified
        bool                            m_bNotAttached;

        explicit DBSubComponentController_Impl(osl::Mutex& i_rMutex)
            :m_aDocScriptSupport()
            ,m_aModifyListeners( i_rMutex )
            ,m_nDocStartNumber(0)
            ,m_bSuspended( false )
            ,m_bEditable(true)
            ,m_bModified(false)
            ,m_bNotAttached(true)
        {
        }

        bool    documentHasScriptSupport() const
        {
            OSL_PRECOND( !!m_aDocScriptSupport,
                "DBSubComponentController_Impl::documentHasScriptSupport: not completely initialized, yet - don't know!?" );
            return !!m_aDocScriptSupport && *m_aDocScriptSupport;
        }

        void    setDocumentScriptSupport( const bool _bSupport )
        {
            OSL_PRECOND( !m_aDocScriptSupport,
                "DBSubComponentController_Impl::setDocumentScriptSupport: already initialized!" );
            m_aDocScriptSupport = ::boost::optional< bool >( _bSupport );
        }
    };

    // DBSubComponentController
    DBSubComponentController::DBSubComponentController(const Reference< XComponentContext >& _rxORB)
        :DBSubComponentController_Base( _rxORB )
        ,m_pImpl( new DBSubComponentController_Impl( getMutex() ) )
    {
    }

    DBSubComponentController::~DBSubComponentController()
    {
    }

    void DBSubComponentController::impl_initialize()
    {
        OGenericUnoController::impl_initialize();

        const ::comphelper::NamedValueCollection& rArguments( getInitParams() );

        Reference< XConnection > xConnection;
        xConnection = rArguments.getOrDefault( PROPERTY_ACTIVE_CONNECTION, xConnection );

        if ( !xConnection.is() )
            ::dbtools::isEmbeddedInDatabase( getModel(), xConnection );

        if ( xConnection.is() )
            initializeConnection( xConnection );

        bool bShowError = true;
        if ( !isConnected() )
        {
            reconnect( false );
            bShowError = false;
        }
        if ( !isConnected() )
        {
            if ( bShowError )
                connectionLostMessage();
            throw IllegalArgumentException();
        }
    }

    Any SAL_CALL DBSubComponentController::queryInterface(const Type& _rType) throw (RuntimeException, std::exception)
    {
        if ( _rType.equals( cppu::UnoType<XScriptInvocationContext>::get() ) )
        {
            if ( m_pImpl->documentHasScriptSupport() )
                return makeAny( Reference< XScriptInvocationContext >( this ) );
            return Any();
        }

        return DBSubComponentController_Base::queryInterface( _rType );
    }

    Sequence< Type > SAL_CALL DBSubComponentController::getTypes(  ) throw (RuntimeException, std::exception)
    {
        Sequence< Type > aTypes( DBSubComponentController_Base::getTypes() );
        if ( !m_pImpl->documentHasScriptSupport() )
        {
            Sequence< Type > aStrippedTypes( aTypes.getLength() - 1 );
            ::std::remove_copy_if(
                aTypes.getConstArray(),
                aTypes.getConstArray() + aTypes.getLength(),
                aStrippedTypes.getArray(),
                ::std::bind2nd( ::std::equal_to< Type >(), cppu::UnoType<XScriptInvocationContext>::get() )
            );
            aTypes = aStrippedTypes;
        }
        return aTypes;
    }

    void DBSubComponentController::initializeConnection( const Reference< XConnection >& _rxForeignConn )
    {
        DBG_ASSERT( !isConnected(), "DBSubComponentController::initializeConnection: not to be called when already connected!" );
            // usually this gets called from within initialize of derived classes ...
        if ( isConnected() )
            disconnect();

        m_pImpl->m_xConnection.reset( _rxForeignConn, SharedConnection::NoTakeOwnership );
        m_pImpl->m_aSdbMetaData.reset( m_pImpl->m_xConnection );
        startConnectionListening( m_pImpl->m_xConnection );

        // get the data source the connection belongs to
        try
        {
            // determine our data source
            OSL_PRECOND( !m_pImpl->m_aDataSource.is(), "DBSubComponentController::initializeConnection: already a data source in this phase?" );
            {
                Reference< XChild > xConnAsChild( m_pImpl->m_xConnection, UNO_QUERY );
                Reference< XDataSource > xDS;
                if ( xConnAsChild.is() )
                    xDS.set( xConnAsChild->getParent(), UNO_QUERY );

                // (take the indirection through XDataSource to ensure we have a correct object ....)
                m_pImpl->m_aDataSource = DataSourceHolder(xDS);
            }
            SAL_WARN_IF( !m_pImpl->m_aDataSource.is(), "dbaccess.ui", "DBSubComponentController::initializeConnection: unable to obtain the data source object!" );

            if ( m_pImpl->m_bNotAttached )
            {
                Reference< XUntitledNumbers > xUntitledProvider( getDatabaseDocument(), UNO_QUERY );
                m_pImpl->m_nDocStartNumber = 1;
                if ( xUntitledProvider.is() )
                    m_pImpl->m_nDocStartNumber = xUntitledProvider->leaseNumber( static_cast< XWeak* >( this ) );
            }

            // determine the availability of script support in our document. Our own XScriptInvocationContext
            // interface depends on this
            m_pImpl->setDocumentScriptSupport( Reference< XEmbeddedScripts >( getDatabaseDocument(), UNO_QUERY ).is() );

            // get a number formatter
            Reference< XPropertySet > xDataSourceProps( m_pImpl->m_aDataSource.getDataSourceProps(), UNO_SET_THROW );
            xDataSourceProps->getPropertyValue( PROPERTY_NAME ) >>= m_pImpl->m_sDataSourceName;
            DBG_ASSERT( !m_pImpl->m_sDataSourceName.isEmpty(), "DBSubComponentController::initializeConnection: invalid data source name!" );
            Reference< XNumberFormatsSupplier> xSupplier = ::dbtools::getNumberFormats(m_pImpl->m_xConnection);
            if(xSupplier.is())
            {
                m_pImpl->m_xFormatter.set(NumberFormatter::create(getORB()), UNO_QUERY_THROW);
                m_pImpl->m_xFormatter->attachNumberFormatsSupplier(xSupplier);
            }
            OSL_ENSURE(m_pImpl->m_xFormatter.is(),"No NumberFormatter!");
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    void DBSubComponentController::reconnect( bool _bUI )
    {
        OSL_ENSURE(!m_pImpl->m_bSuspended, "Cannot reconnect while suspended!");

        stopConnectionListening( m_pImpl->m_xConnection );
        m_pImpl->m_aSdbMetaData.reset( nullptr );
        m_pImpl->m_xConnection.clear();

        // reconnect
        bool bReConnect = true;
        if ( _bUI )
        {
            ScopedVclPtrInstance< MessageDialog > aQuery(getView(), ModuleRes(STR_QUERY_CONNECTION_LOST), VCL_MESSAGE_QUESTION, VCL_BUTTONS_YES_NO);
            bReConnect = ( RET_YES == aQuery->Execute() );
        }

        // now really reconnect ...
        if ( bReConnect )
        {
            m_pImpl->m_xConnection.reset( connect( m_pImpl->m_aDataSource.getDataSource(), nullptr ), SharedConnection::TakeOwnership );
            m_pImpl->m_aSdbMetaData.reset( m_pImpl->m_xConnection );
        }

        // invalidate all slots
        InvalidateAll();
    }

    void DBSubComponentController::disconnect()
    {
        stopConnectionListening(m_pImpl->m_xConnection);
        m_pImpl->m_aSdbMetaData.reset( nullptr );
        m_pImpl->m_xConnection.clear();

        InvalidateAll();
    }

    void DBSubComponentController::losingConnection()
    {
        // our connection was disposed so we need a new one
        reconnect( true );
        InvalidateAll();
    }

    void SAL_CALL DBSubComponentController::disposing()
    {
        DBSubComponentController_Base::disposing();

        disconnect();

        attachFrame( Reference < XFrame >() );

        m_pImpl->m_aDataSource.clear();
    }

    void SAL_CALL DBSubComponentController::disposing(const EventObject& _rSource) throw( RuntimeException, std::exception )
    {
        if ( _rSource.Source == getConnection() )
        {
            if (    !m_pImpl->m_bSuspended // when already suspended then we don't have to reconnect
                &&  !getBroadcastHelper().bInDispose
                &&  !getBroadcastHelper().bDisposed
                &&  isConnected()
                )
            {
                losingConnection();
            }
            else
            {
                m_pImpl->m_xConnection.reset( m_pImpl->m_xConnection, SharedConnection::NoTakeOwnership );
                    // this prevents the "disposeComponent" call in disconnect
                disconnect();
            }
        }
        else
            DBSubComponentController_Base::disposing( _rSource );
    }

    void DBSubComponentController::appendError( const OUString& _rErrorMessage )
    {
        m_pImpl->m_aCurrentError.append( ::dbtools::SQLExceptionInfo::TYPE::SQLException, _rErrorMessage,
                                        getStandardSQLState( ::dbtools::StandardSQLState::GENERAL_ERROR ),
                                        1000 );
    }
    void DBSubComponentController::clearError()
    {
        m_pImpl->m_aCurrentError = ::dbtools::SQLExceptionInfo();
    }

    bool DBSubComponentController::hasError() const
    {
        return m_pImpl->m_aCurrentError.isValid();
    }

    const ::dbtools::SQLExceptionInfo& DBSubComponentController::getError() const
    {
        return m_pImpl->m_aCurrentError;
    }

    void DBSubComponentController::displayError()
    {
        showError( m_pImpl->m_aCurrentError );
    }

    sal_Bool SAL_CALL DBSubComponentController::suspend(sal_Bool bSuspend) throw( RuntimeException, std::exception )
    {
        m_pImpl->m_bSuspended = bSuspend;
        if ( !bSuspend && !isConnected() )
            reconnect(true);

        return true;
    }

    sal_Bool SAL_CALL DBSubComponentController::attachModel( const Reference< XModel > & _rxModel) throw( RuntimeException, std::exception )
    {
        if ( !_rxModel.is() )
            return false;
        if ( !DBSubComponentController_Base::attachModel( _rxModel ) )
            return false;

        m_pImpl->m_bNotAttached = false;
        if ( m_pImpl->m_nDocStartNumber == 1 )
            releaseNumberForComponent();

        Reference< XUntitledNumbers > xUntitledProvider( _rxModel, UNO_QUERY );
        m_pImpl->m_nDocStartNumber = 1;
        if ( xUntitledProvider.is() )
            m_pImpl->m_nDocStartNumber = xUntitledProvider->leaseNumber( static_cast< XWeak* >( this ) );

        return true;
    }

    void DBSubComponentController::Execute(sal_uInt16 _nId, const Sequence< PropertyValue >& _rArgs)
    {
        if ( _nId == ID_BROWSER_CLOSE )
        {
            closeTask();
            return;
        }

        DBSubComponentController_Base::Execute( _nId, _rArgs );
        InvalidateFeature( _nId );
    }

    OUString DBSubComponentController::getDataSourceName() const
    {
        OUString sName;
        Reference< XPropertySet > xDataSourceProps( m_pImpl->m_aDataSource.getDataSourceProps() );
        if ( xDataSourceProps.is() )
            xDataSourceProps->getPropertyValue(PROPERTY_NAME) >>= sName;
        return sName;
    }
    void DBSubComponentController::connectionLostMessage() const
    {
        OUString aMessage(ModuleRes(RID_STR_CONNECTION_LOST));
        Reference< XWindow > xWindow = getTopMostContainerWindow();
        vcl::Window* pWin = nullptr;
        if ( xWindow.is() )
            pWin = VCLUnoHelper::GetWindow(xWindow);
        if ( !pWin )
            pWin = getView()->Window::GetParent();

        ScopedVclPtrInstance<MessageDialog>(pWin, aMessage, VCL_MESSAGE_INFO)->Execute();
    }
    const Reference< XConnection >& DBSubComponentController::getConnection() const
    {
        return m_pImpl->m_xConnection;
    }

    bool DBSubComponentController::isReadOnly() const
    {
        return !m_pImpl->m_bEditable;
    }

    bool DBSubComponentController::isEditable() const
    {
        return m_pImpl->m_bEditable;
    }

    void DBSubComponentController::setEditable(bool _bEditable)
    {
        m_pImpl->m_bEditable = _bEditable;
    }

    const ::dbtools::DatabaseMetaData& DBSubComponentController::getSdbMetaData() const
    {
        return m_pImpl->m_aSdbMetaData;
    }

    bool DBSubComponentController::isConnected() const
    {
        return m_pImpl->m_xConnection.is();
    }

    Reference< XDatabaseMetaData > DBSubComponentController::getMetaData( ) const
    {
        Reference< XDatabaseMetaData > xMeta;
        try
        {
            if ( isConnected() )
                xMeta.set( m_pImpl->m_xConnection->getMetaData(), UNO_SET_THROW );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        return xMeta;
    }

    const Reference< XPropertySet >& DBSubComponentController::getDataSource() const
    {
        return m_pImpl->m_aDataSource.getDataSourceProps();
    }

    bool DBSubComponentController::haveDataSource() const
    {
        return m_pImpl->m_aDataSource.is();
    }

    Reference< XModel > DBSubComponentController::getDatabaseDocument() const
    {
        return Reference< XModel >( m_pImpl->m_aDataSource.getDatabaseDocument(), UNO_QUERY );
    }

    Reference< XNumberFormatter > DBSubComponentController::getNumberFormatter() const
    {
        return m_pImpl->m_xFormatter;
    }

    Reference< XModel > DBSubComponentController::getPrivateModel() const
    {
        return getDatabaseDocument();
    }
    // XTitle
    OUString SAL_CALL DBSubComponentController::getTitle()
        throw (RuntimeException, std::exception)
    {
        ::osl::MutexGuard aGuard( getMutex() );
        if ( m_bExternalTitle )
            return impl_getTitleHelper_throw()->getTitle ();

        OUStringBuffer sTitle;
        Reference< XTitle > xTitle(getPrivateModel(),UNO_QUERY);
        if ( xTitle.is() )
        {
            sTitle.append( xTitle->getTitle() );
            sTitle.append(" : ");
        }
        sTitle.append( getPrivateTitle() );
        return sTitle.makeStringAndClear();
    }

    sal_Int32 DBSubComponentController::getCurrentStartNumber() const
    {
        return m_pImpl->m_nDocStartNumber;
    }

    Reference< XEmbeddedScripts > SAL_CALL DBSubComponentController::getScriptContainer() throw (RuntimeException, std::exception)
    {
        ::osl::MutexGuard aGuard( getMutex() );
        if ( !m_pImpl->documentHasScriptSupport() )
            return nullptr;

        return Reference< XEmbeddedScripts >( getDatabaseDocument(), UNO_QUERY_THROW );
    }

    void SAL_CALL DBSubComponentController::addModifyListener( const Reference< XModifyListener >& i_Listener ) throw (RuntimeException, std::exception)
    {
        ::osl::MutexGuard aGuard( getMutex() );
        m_pImpl->m_aModifyListeners.addInterface( i_Listener );
    }

    void SAL_CALL DBSubComponentController::removeModifyListener( const Reference< XModifyListener >& i_Listener ) throw (RuntimeException, std::exception)
    {
        ::osl::MutexGuard aGuard( getMutex() );
        m_pImpl->m_aModifyListeners.removeInterface( i_Listener );
    }

    sal_Bool SAL_CALL DBSubComponentController::isModified(  ) throw (RuntimeException, std::exception)
    {
        ::osl::MutexGuard aGuard( getMutex() );
        return impl_isModified();
    }

    void SAL_CALL DBSubComponentController::setModified( sal_Bool i_bModified ) throw (PropertyVetoException, RuntimeException, std::exception)
    {
        ::osl::ClearableMutexGuard aGuard( getMutex() );

        if ( m_pImpl->m_bModified == bool(i_bModified) )
            return;

        m_pImpl->m_bModified = i_bModified;
        impl_onModifyChanged();

        EventObject aEvent( *this );
        aGuard.clear();
        m_pImpl->m_aModifyListeners.notifyEach( &XModifyListener::modified, aEvent );
    }

    bool DBSubComponentController::impl_isModified() const
    {
        return m_pImpl->m_bModified;
    }

    void DBSubComponentController::impl_onModifyChanged()
    {
        InvalidateFeature( ID_BROWSER_SAVEDOC );
        if ( isFeatureSupported( ID_BROWSER_SAVEASDOC ) )
            InvalidateFeature( ID_BROWSER_SAVEASDOC );
    }

}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
