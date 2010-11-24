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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"

#include "browserids.hxx"
#include "commontypes.hxx"
#include "dataview.hxx"
#include "dbu_misc.hrc"
#include "dbustrings.hrc"
#include "moduledbu.hxx"
#include "singledoccontroller.hxx"
#include "dbaundomanager.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/frame/XUntitledNumbers.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/sdb/XDocumentDataSource.hpp>
#include <com/sun/star/sdb/XOfficeDatabaseDocument.hpp>
#include <com/sun/star/sdbc/XDataSource.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/frame/XUntitledNumbers.hpp>
/** === end UNO includes === **/

#include <comphelper/sequence.hxx>
#include <comphelper/types.hxx>
#include <connectivity/dbexception.hxx>
#include <connectivity/dbtools.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <rtl/ustrbuf.hxx>
#include <toolkit/unohlp.hxx>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include <vcl/msgbox.hxx>

//........................................................................
namespace dbaui
{
//........................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::beans::XPropertySet;
    using ::com::sun::star::util::XNumberFormatter;
    using ::com::sun::star::lang::XMultiServiceFactory;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::Type;
    using ::com::sun::star::sdbc::XConnection;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::container::XChild;
    using ::com::sun::star::sdbc::XDataSource;
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
    using ::com::sun::star::document::XUndoManager;
    /** === end UNO using === **/

    class DataSourceHolder
    {
    public:
        DataSourceHolder()
        {
        }

        DataSourceHolder( const Reference< XDataSource >& _rxDataSource )
        {
            m_xDataSource = _rxDataSource;
            Reference< XDocumentDataSource > xDocDS( m_xDataSource, UNO_QUERY );
            if ( xDocDS.is() )
                m_xDocument = xDocDS->getDatabaseDocument();

            m_xDataSourceProps.set( m_xDataSource, UNO_QUERY );
        }

        const Reference< XDataSource >&             getDataSource() const { return m_xDataSource; }
        const Reference< XPropertySet >&            getDataSourceProps() const { return m_xDataSourceProps; }
        const Reference< XOfficeDatabaseDocument >  getDatabaseDocument() const { return m_xDocument; }

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

    struct OSingleDocumentControllerImpl
    {
    private:
        ::boost::optional< bool >       m_aDocScriptSupport;

    public:
        OModuleClient                   m_aModuleClient;
        ::dbtools::SQLExceptionInfo     m_aCurrentError;

        ::cppu::OInterfaceContainerHelper
                                        m_aModifyListeners;

        // <properties>
        SharedConnection                m_xConnection;
        ::dbtools::DatabaseMetaData     m_aSdbMetaData;
        // </properties>
        ::boost::scoped_ptr< UndoManager >
                                        m_pUndoManager;
        ::rtl::OUString                 m_sDataSourceName;      // the data source we're working for
        DataSourceHolder                m_aDataSource;
        Reference< XModel >             m_xDocument;
        Reference< XNumberFormatter >   m_xFormatter;   // a number formatter working with the connection's NumberFormatsSupplier
        sal_Int32                       m_nDocStartNumber;
        sal_Bool                        m_bSuspended;   // is true when the controller was already suspended
        sal_Bool                        m_bEditable;    // is the control readonly or not
        sal_Bool                        m_bModified;    // is the data modified
        bool                            m_bNotAttached;

        OSingleDocumentControllerImpl( OSingleDocumentController& i_antiImpl, ::osl::Mutex& i_rMutex )
            :m_aDocScriptSupport()
            ,m_aModifyListeners( i_rMutex )
            ,m_pUndoManager( new UndoManager( i_antiImpl, i_rMutex ) )
            ,m_nDocStartNumber(0)
            ,m_bSuspended( sal_False )
            ,m_bEditable(sal_True)
            ,m_bModified(sal_False)
            ,m_bNotAttached(true)
        {
        }

        bool    documentHasScriptSupport() const
        {
            OSL_PRECOND( !!m_aDocScriptSupport,
                "OSingleDocumentControllerImpl::documentHasScriptSupport: not completely initialized, yet - don't know!?" );
            return !!m_aDocScriptSupport && *m_aDocScriptSupport;
        }

        void    setDocumentScriptSupport( const bool _bSupport )
        {
            OSL_PRECOND( !m_aDocScriptSupport,
                "OSingleDocumentControllerImpl::setDocumentScriptSupport: already initialized!" );
            m_aDocScriptSupport = ::boost::optional< bool >( _bSupport );
        }
    };

    //====================================================================
    //= OSingleDocumentController
    //====================================================================
    //--------------------------------------------------------------------
    OSingleDocumentController::OSingleDocumentController(const Reference< XMultiServiceFactory >& _rxORB)
        :OSingleDocumentController_Base( _rxORB )
        ,m_pImpl( new OSingleDocumentControllerImpl( *this, getMutex() ) )
    {
    }

    //--------------------------------------------------------------------
    OSingleDocumentController::~OSingleDocumentController()
    {
    }

    //--------------------------------------------------------------------
    void OSingleDocumentController::impl_initialize()
    {
        OGenericUnoController::impl_initialize();

        const ::comphelper::NamedValueCollection& rArguments( getInitParams() );

        Reference< XConnection > xConnection;
        xConnection = rArguments.getOrDefault( (::rtl::OUString)PROPERTY_ACTIVE_CONNECTION, xConnection );

        if ( !xConnection.is() )
            ::dbtools::isEmbeddedInDatabase( getModel(), xConnection );

        if ( xConnection.is() )
            initializeConnection( xConnection );

        bool bShowError = true;
        if ( !isConnected() )
        {
            reconnect( sal_False );
            bShowError = false;
        }
        if ( !isConnected() )
        {
            if ( bShowError )
                connectionLostMessage();
            throw IllegalArgumentException();
        }
    }

    //--------------------------------------------------------------------
    Any SAL_CALL OSingleDocumentController::queryInterface(const Type& _rType) throw (RuntimeException)
    {
        if ( _rType.equals( XScriptInvocationContext::static_type() ) )
        {
            if ( m_pImpl->documentHasScriptSupport() )
                return makeAny( Reference< XScriptInvocationContext >( this ) );
            return Any();
        }

        return OSingleDocumentController_Base::queryInterface( _rType );
    }

    //--------------------------------------------------------------------
    Sequence< Type > SAL_CALL OSingleDocumentController::getTypes(  ) throw (RuntimeException)
    {
        Sequence< Type > aTypes( OSingleDocumentController_Base::getTypes() );
        if ( !m_pImpl->documentHasScriptSupport() )
        {
            Sequence< Type > aStrippedTypes( aTypes.getLength() - 1 );
            ::std::remove_copy_if(
                aTypes.getConstArray(),
                aTypes.getConstArray() + aTypes.getLength(),
                aStrippedTypes.getArray(),
                ::std::bind2nd( ::std::equal_to< Type >(), XScriptInvocationContext::static_type() )
            );
            aTypes = aStrippedTypes;
        }
        return aTypes;
    }

    //--------------------------------------------------------------------
    void OSingleDocumentController::initializeConnection( const Reference< XConnection >& _rxForeignConn )
    {
        DBG_ASSERT( !isConnected(), "OSingleDocumentController::initializeConnection: not to be called when already connected!" );
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
            OSL_PRECOND( !m_pImpl->m_aDataSource.is(), "OSingleDocumentController::initializeConnection: already a data source in this phase?" );
            {
                Reference< XChild > xConnAsChild( m_pImpl->m_xConnection, UNO_QUERY );
                Reference< XDataSource > xDS;
                if ( xConnAsChild.is() )
                    xDS = Reference< XDataSource >( xConnAsChild->getParent(), UNO_QUERY );

                // (take the indirection through XDataSource to ensure we have a correct object ....)
                m_pImpl->m_aDataSource = xDS;
            }
            OSL_POSTCOND( m_pImpl->m_aDataSource.is(), "OSingleDocumentController::initializeConnection: unable to obtain the data source object!" );

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
            DBG_ASSERT( m_pImpl->m_sDataSourceName.getLength(), "OSingleDocumentController::initializeConnection: invalid data source name!" );
            Reference< XNumberFormatsSupplier> xSupplier = ::dbtools::getNumberFormats(m_pImpl->m_xConnection);
            if(xSupplier.is())
            {
                m_pImpl->m_xFormatter = Reference< XNumberFormatter >(getORB()
                    ->createInstance(::rtl::OUString::createFromAscii("com.sun.star.util.NumberFormatter")), UNO_QUERY);
                m_pImpl->m_xFormatter->attachNumberFormatsSupplier(xSupplier);
            }
            OSL_ENSURE(m_pImpl->m_xFormatter.is(),"No NumberFormatter!");
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    //--------------------------------------------------------------------
    void OSingleDocumentController::reconnect( sal_Bool _bUI )
    {
        OSL_ENSURE(!m_pImpl->m_bSuspended, "Cannot reconnect while suspended!");

        stopConnectionListening( m_pImpl->m_xConnection );
        m_pImpl->m_aSdbMetaData.reset( NULL );
        m_pImpl->m_xConnection.clear();

        // reconnect
        sal_Bool bReConnect = sal_True;
        if ( _bUI )
        {
            QueryBox aQuery( getView(), ModuleRes(QUERY_CONNECTION_LOST) );
            bReConnect = ( RET_YES == aQuery.Execute() );
        }

        // now really reconnect ...
        if ( bReConnect )
        {
            m_pImpl->m_xConnection.reset( connect( m_pImpl->m_aDataSource.getDataSource(), NULL ), SharedConnection::TakeOwnership );
            m_pImpl->m_aSdbMetaData.reset( m_pImpl->m_xConnection );
        }

        // invalidate all slots
        InvalidateAll();
    }

    //--------------------------------------------------------------------
    void OSingleDocumentController::disconnect()
    {
        stopConnectionListening(m_pImpl->m_xConnection);
        m_pImpl->m_aSdbMetaData.reset( NULL );
        m_pImpl->m_xConnection.clear();

        InvalidateAll();
    }

    //--------------------------------------------------------------------
    void OSingleDocumentController::losingConnection()
    {
        // our connection was disposed so we need a new one
        reconnect( sal_True );
        InvalidateAll();
    }

    //--------------------------------------------------------------------
    void SAL_CALL OSingleDocumentController::disposing()
    {
        OSingleDocumentController_Base::disposing();
        GetUndoManager().Clear();

        disconnect();

        attachFrame( Reference < XFrame >() );

        m_pImpl->m_pUndoManager->disposing();
        m_pImpl->m_aDataSource.clear();
    }

    //--------------------------------------------------------------------
    sal_Bool OSingleDocumentController::Construct(Window* _pParent)
    {
        DBG_ASSERT( getView(), "OSingleDocumentController::Construct: have no view!" );
        if ( getView() )
            getView()->enableSeparator( );

        return OSingleDocumentController_Base::Construct( _pParent );
    }

    //--------------------------------------------------------------------
    void SAL_CALL OSingleDocumentController::disposing(const EventObject& _rSource) throw( RuntimeException )
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
            OSingleDocumentController_Base::disposing( _rSource );
    }

    //--------------------------------------------------------------------
    void OSingleDocumentController::appendError( const ::rtl::OUString& _rErrorMessage, const ::dbtools::StandardSQLState _eSQLState,
            const sal_Int32 _nErrorCode )
    {
        m_pImpl->m_aCurrentError.append( ::dbtools::SQLExceptionInfo::SQL_EXCEPTION, _rErrorMessage, getStandardSQLStateAscii( _eSQLState ),
            _nErrorCode );
    }
    //--------------------------------------------------------------------
    void OSingleDocumentController::clearError()
    {
        m_pImpl->m_aCurrentError = ::dbtools::SQLExceptionInfo();
    }

    //--------------------------------------------------------------------
    sal_Bool OSingleDocumentController::hasError() const
    {
        return m_pImpl->m_aCurrentError.isValid();
    }

    //--------------------------------------------------------------------
    const ::dbtools::SQLExceptionInfo& OSingleDocumentController::getError() const
    {
        return m_pImpl->m_aCurrentError;
    }

    //--------------------------------------------------------------------
    void OSingleDocumentController::displayError()
    {
        showError( m_pImpl->m_aCurrentError );
    }

    //--------------------------------------------------------------------
    sal_Bool SAL_CALL OSingleDocumentController::suspend(sal_Bool bSuspend) throw( RuntimeException )
    {
        m_pImpl->m_bSuspended = bSuspend;
        if ( !bSuspend && !isConnected() )
            reconnect(sal_True);


        return sal_True;
    }

    // -----------------------------------------------------------------------------
    sal_Bool SAL_CALL OSingleDocumentController::attachModel( const Reference< XModel > & _rxModel) throw( RuntimeException )
    {
        if ( !_rxModel.is() )
            return sal_False;
        if ( !OSingleDocumentController_Base::attachModel( _rxModel ) )
            return sal_False;

        m_pImpl->m_bNotAttached = false;
        if ( m_pImpl->m_nDocStartNumber == 1 )
            releaseNumberForComponent();

        Reference< XUntitledNumbers > xUntitledProvider( _rxModel, UNO_QUERY );
        m_pImpl->m_nDocStartNumber = 1;
        if ( xUntitledProvider.is() )
            m_pImpl->m_nDocStartNumber = xUntitledProvider->leaseNumber( static_cast< XWeak* >( this ) );

        return sal_True;
    }

    // -----------------------------------------------------------------------------
    FeatureState OSingleDocumentController::GetState(sal_uInt16 _nId) const
    {
        FeatureState aReturn;
            // (disabled automatically)
        aReturn.bEnabled = sal_True;

        switch (_nId)
        {
            case ID_BROWSER_UNDO:
                aReturn.bEnabled = m_pImpl->m_bEditable && GetUndoManager().GetUndoActionCount() != 0;
                if ( aReturn.bEnabled )
                {
                    String sUndo(ModuleRes(STR_UNDO_COLON));
                    sUndo += String(RTL_CONSTASCII_USTRINGPARAM(" "));
                    sUndo += GetUndoManager().GetUndoActionComment();
                    aReturn.sTitle = sUndo;
                }
                break;
            case ID_BROWSER_REDO:
                aReturn.bEnabled = m_pImpl->m_bEditable && GetUndoManager().GetRedoActionCount() != 0;
                if ( aReturn.bEnabled )
                {
                    String sRedo(ModuleRes(STR_REDO_COLON));
                    sRedo += String(RTL_CONSTASCII_USTRINGPARAM(" "));
                    sRedo += GetUndoManager().GetRedoActionComment();
                    aReturn.sTitle = sRedo;
                }
                break;
            default:
                aReturn = OSingleDocumentController_Base::GetState(_nId);
        }
        return aReturn;
    }
    // -----------------------------------------------------------------------------
    void OSingleDocumentController::Execute(sal_uInt16 _nId, const Sequence< PropertyValue >& _rArgs)
    {
        switch(_nId)
        {
            case ID_BROWSER_CLOSE:
                closeTask();
                return;
            case ID_BROWSER_UNDO:
                GetUndoManager().Undo();
                InvalidateFeature(ID_BROWSER_REDO);
                break;
            case ID_BROWSER_REDO:
                GetUndoManager().Redo();
                InvalidateFeature(ID_BROWSER_UNDO);
                break;
            default:
                OSingleDocumentController_Base::Execute( _nId, _rArgs );
                break;
        }
        InvalidateFeature(_nId);
    }
    // -----------------------------------------------------------------------------
    SfxUndoManager& OSingleDocumentController::GetUndoManager() const
    {
        return m_pImpl->m_pUndoManager->GetSfxUndoManager();
    }
    // -----------------------------------------------------------------------------
    void OSingleDocumentController::addUndoActionAndInvalidate(SfxUndoAction *_pAction)
    {
        // add undo action
        GetUndoManager().AddUndoAction(_pAction);
        // when we add an undo action the controller was modified
        setModified(sal_True);
        // now inform me that or states changed
        InvalidateFeature(ID_BROWSER_UNDO);
        InvalidateFeature(ID_BROWSER_REDO);
    }

    // -----------------------------------------------------------------------------
    ::rtl::OUString OSingleDocumentController::getDataSourceName() const
    {
        ::rtl::OUString sName;
        Reference< XPropertySet > xDataSourceProps( m_pImpl->m_aDataSource.getDataSourceProps() );
        if ( xDataSourceProps.is() )
            xDataSourceProps->getPropertyValue(PROPERTY_NAME) >>= sName;
        return sName;
    }
    // -----------------------------------------------------------------------------
    void OSingleDocumentController::connectionLostMessage() const
    {
        String aMessage(ModuleRes(RID_STR_CONNECTION_LOST));
        Reference< XWindow > xWindow = getTopMostContainerWindow();
        Window* pWin = NULL;
        if ( xWindow.is() )
            pWin = VCLUnoHelper::GetWindow(xWindow);
        if ( !pWin )
            pWin = getView()->Window::GetParent();

        InfoBox(pWin, aMessage).Execute();
    }
    // -----------------------------------------------------------------------------
    const Reference< XConnection >& OSingleDocumentController::getConnection() const
    {
        return m_pImpl->m_xConnection;
    }

    // -----------------------------------------------------------------------------
    sal_Bool OSingleDocumentController::isReadOnly() const
    {
        return !m_pImpl->m_bEditable;
    }

    // -----------------------------------------------------------------------------
    sal_Bool OSingleDocumentController::isEditable() const
    {
        return m_pImpl->m_bEditable;
    }

    // -----------------------------------------------------------------------------
    void OSingleDocumentController::setEditable(sal_Bool _bEditable)
    {
        m_pImpl->m_bEditable = _bEditable;
    }

    // -----------------------------------------------------------------------------
    const ::dbtools::DatabaseMetaData& OSingleDocumentController::getSdbMetaData() const
    {
        return m_pImpl->m_aSdbMetaData;
    }

    // -----------------------------------------------------------------------------
    sal_Bool OSingleDocumentController::isConnected() const
    {
        return m_pImpl->m_xConnection.is();
    }

    // -----------------------------------------------------------------------------
    Reference< XDatabaseMetaData > OSingleDocumentController::getMetaData( ) const
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

    // -----------------------------------------------------------------------------
    const Reference< XPropertySet >& OSingleDocumentController::getDataSource() const
    {
        return m_pImpl->m_aDataSource.getDataSourceProps();
    }

    // -----------------------------------------------------------------------------
    sal_Bool OSingleDocumentController::haveDataSource() const
    {
        return m_pImpl->m_aDataSource.is();
    }

    // -----------------------------------------------------------------------------
    Reference< XModel > OSingleDocumentController::getDatabaseDocument() const
    {
        return Reference< XModel >( m_pImpl->m_aDataSource.getDatabaseDocument(), UNO_QUERY );
    }

    // -----------------------------------------------------------------------------
    Reference< XNumberFormatter > OSingleDocumentController::getNumberFormatter() const
    {
        return m_pImpl->m_xFormatter;
    }

    // -----------------------------------------------------------------------------
    Reference< XModel > OSingleDocumentController::getPrivateModel() const
    {
        return getDatabaseDocument();
    }
    // -----------------------------------------------------------------------------
    // XTitle
    ::rtl::OUString SAL_CALL OSingleDocumentController::getTitle()
        throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( getMutex() );
        if ( m_bExternalTitle )
            return impl_getTitleHelper_throw()->getTitle ();

        ::rtl::OUStringBuffer sTitle;
        Reference< XTitle > xTitle(getPrivateModel(),UNO_QUERY);
        if ( xTitle.is() )
        {
            sTitle.append( xTitle->getTitle() );
            sTitle.appendAscii(" : ");
        }
        sTitle.append( getPrivateTitle() );
        // There can be only one view with the same object
        //const sal_Int32 nCurrentView = getCurrentStartNumber();
        //if ( nCurrentView > 1 )
        //{
        //    sTitle.appendAscii(" : ");
        //    sTitle.append(nCurrentView);
        //}

        return sTitle.makeStringAndClear();
    }

    // -----------------------------------------------------------------------------
    Reference< XUndoManager > SAL_CALL OSingleDocumentController::getUndoManager(  ) throw (RuntimeException)
    {
        return m_pImpl->m_pUndoManager.get();
    }

    // -----------------------------------------------------------------------------
    sal_Int32 OSingleDocumentController::getCurrentStartNumber() const
    {
        return m_pImpl->m_nDocStartNumber;
    }

    // -----------------------------------------------------------------------------
    Reference< XEmbeddedScripts > SAL_CALL OSingleDocumentController::getScriptContainer() throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( getMutex() );
        if ( !m_pImpl->documentHasScriptSupport() )
            return NULL;

        return Reference< XEmbeddedScripts >( getDatabaseDocument(), UNO_QUERY_THROW );
    }

    // -----------------------------------------------------------------------------
    void SAL_CALL OSingleDocumentController::addModifyListener( const Reference< XModifyListener >& i_Listener ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( getMutex() );
        m_pImpl->m_aModifyListeners.addInterface( i_Listener );
    }

    // -----------------------------------------------------------------------------
    void SAL_CALL OSingleDocumentController::removeModifyListener( const Reference< XModifyListener >& i_Listener ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( getMutex() );
        m_pImpl->m_aModifyListeners.removeInterface( i_Listener );
    }

    // -----------------------------------------------------------------------------
    ::sal_Bool SAL_CALL OSingleDocumentController::isModified(  ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( getMutex() );
        return impl_isModified();
    }

    // -----------------------------------------------------------------------------
    void SAL_CALL OSingleDocumentController::setModified( ::sal_Bool i_bModified ) throw (PropertyVetoException, RuntimeException)
    {
        ::osl::ClearableMutexGuard aGuard( getMutex() );

        if ( m_pImpl->m_bModified == i_bModified )
            return;

        m_pImpl->m_bModified = i_bModified;
        impl_onModifyChanged();

        EventObject aEvent( *this );
        aGuard.clear();
        m_pImpl->m_aModifyListeners.notifyEach( &XModifyListener::modified, aEvent );
    }

    // -----------------------------------------------------------------------------
    sal_Bool OSingleDocumentController::impl_isModified() const
    {
        return m_pImpl->m_bModified;
    }

    // -----------------------------------------------------------------------------
    void OSingleDocumentController::impl_onModifyChanged()
    {
        InvalidateFeature( ID_BROWSER_SAVEDOC );
        if ( isFeatureSupported( ID_BROWSER_SAVEASDOC ) )
            InvalidateFeature( ID_BROWSER_SAVEASDOC );
    }

//........................................................................
}   // namespace dbaui
//........................................................................

