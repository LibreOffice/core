/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: singledoccontroller.cxx,v $
 *
 *  $Revision: 1.24 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-04 15:02:02 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
/** === begin UNO includes === **/
#include <com/sun/star/frame/XUntitledNumbers.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/sdb/XDocumentDataSource.hpp>
#include <com/sun/star/sdb/XOfficeDatabaseDocument.hpp>
#include <com/sun/star/sdbc/XDataSource.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
/** === end UNO includes === **/

#include <comphelper/sequence.hxx>
#include <comphelper/types.hxx>
#include <vcl/msgbox.hxx>
#include "dbu_misc.hrc"
#include "dataview.hxx"
#include "UITools.hxx"
#include <cppuhelper/typeprovider.hxx>
#include <connectivity/dbexception.hxx>
#include <connectivity/dbtools.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <toolkit/unohlp.hxx>
#include <tools/debug.hxx>
#include <vcl/msgbox.hxx>

#include <rtl/ustrbuf.hxx>
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
        Any                             m_aCurrentError; // contains the current error which can be set through IEnvironment

        // <properties>
        SharedConnection                m_xConnection;
        ::dbtools::DatabaseMetaData     m_aSdbMetaData;
        // </properties>
        ::rtl::OUString                 m_sDataSourceName;      // the data source we're working for
        DataSourceHolder                m_aDataSource;
        Reference< XModel >             m_xDocument;
        Reference< XNumberFormatter >   m_xFormatter;   // a number formatter working with the connection's NumberFormatsSupplier
        sal_Int32                       m_nDocStartNumber;
        sal_Bool                        m_bSuspended;   // is true when the controller was already suspended
        sal_Bool                        m_bEditable;    // is the control readonly or not
        sal_Bool                        m_bModified;    // is the data modified

        OSingleDocumentControllerImpl()
            :m_aDocScriptSupport()
            , m_nDocStartNumber(1)
            ,m_bSuspended( sal_False )
            ,m_bEditable(sal_True)
            ,m_bModified(sal_False)
        {
        }

        bool    documentHasScriptSupport() const
        {
            // TODO: revert to the disabled code. The current version is just to be able
            // to integrate an intermediate version of the CWS, which should behave as
            // if no macros in DB docs are allowed
            return false;
//            OSL_PRECOND( !!m_aDocScriptSupport,
//                "OSingleDocumentControllerImpl::documentHasScriptSupport: not completely initialized, yet - don't know!?" );
//            return !!m_aDocScriptSupport && *m_aDocScriptSupport;
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
        ,m_pImpl(new OSingleDocumentControllerImpl())
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
        Reference< XUntitledNumbers > xUntitledProvider(getModel(), UNO_QUERY      );
        m_pImpl->m_nDocStartNumber = 1;
        if ( xUntitledProvider.is() )
            m_pImpl->m_nDocStartNumber = xUntitledProvider->leaseNumber(static_cast<XWeak*>(this));
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
            DBG_ERROR( "OSingleDocumentController::initializeConnection: caught an exception!" );
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
            m_pImpl->m_xConnection.reset( connect( m_pImpl->m_aDataSource.getDataSource(), sal_True ), SharedConnection::TakeOwnership );
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
    }

    //--------------------------------------------------------------------
    void SAL_CALL OSingleDocumentController::disposing()
    {
        OSingleDocumentController_Base::disposing();
        m_aUndoManager.Clear();

        disconnect();

        attachFrame( Reference < XFrame >() );

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
    namespace
    {
        void concatSQLExceptions(Any& _rChainLeft, const Any& _rChainRight)
        {
            if (!_rChainLeft.hasValue())
                _rChainLeft = _rChainRight;
            else
            {
                // to travel the chain by reference (and not by value), we need the getValue ...
                // looks like a hack, but the meaning of getValue is documented, and it's the only chance for reference-traveling ....

                DBG_ASSERT(::dbtools::SQLExceptionInfo(_rChainLeft).isValid(), "concatSQLExceptions: invalid warnings chain (this will crash)!");

                const SQLException* pChainTravel = static_cast<const SQLException*>(_rChainLeft.getValue());
                ::dbtools::SQLExceptionIteratorHelper aReferenceIterHelper(*pChainTravel);
                while (aReferenceIterHelper.hasMoreElements())
                    pChainTravel = aReferenceIterHelper.next();

                // reached the end of the chain, and pChainTravel points to the last element
                const_cast<SQLException*>(pChainTravel)->NextException = _rChainRight;
            }
        }

    }
    //--------------------------------------------------------------------
    /** appends an error in the current environment.
        @param  _aException
            contains a description of the error or the error directly
    */
    void OSingleDocumentController::appendError(const SQLException& _aException)
    {
        concatSQLExceptions(m_pImpl->m_aCurrentError,makeAny(_aException));
    }
    //--------------------------------------------------------------------
    /** clears the error state.
    */
    void OSingleDocumentController::clearError()
    {
        m_pImpl->m_aCurrentError = Any();
    }
    //--------------------------------------------------------------------
    /** set the current error in the given parameter.
        @param  _rException
            will contain the current error
    */
    void OSingleDocumentController::getError(SQLException& _rException ) const
    {
        m_pImpl->m_aCurrentError >>= _rException;
    }
    //--------------------------------------------------------------------
    sal_Bool OSingleDocumentController::hasError() const
    {
        return m_pImpl->m_aCurrentError.hasValue();
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
    FeatureState OSingleDocumentController::GetState(sal_uInt16 _nId) const
    {
        FeatureState aReturn;
            // (disabled automatically)
        aReturn.bEnabled = sal_True;

        switch (_nId)
        {
            case ID_BROWSER_UNDO:
                aReturn.bEnabled = m_pImpl->m_bEditable && m_aUndoManager.GetUndoActionCount() != 0;
                if ( aReturn.bEnabled )
                {
                    String sUndo(ModuleRes(STR_UNDO_COLON));
                    sUndo += String(RTL_CONSTASCII_USTRINGPARAM(" "));
                    sUndo += m_aUndoManager.GetUndoActionComment();
                    aReturn.sTitle = sUndo;
                }
                break;
            case ID_BROWSER_REDO:
                aReturn.bEnabled = m_pImpl->m_bEditable && m_aUndoManager.GetRedoActionCount() != 0;
                if ( aReturn.bEnabled )
                {
                    String sRedo(ModuleRes(STR_REDO_COLON));
                    sRedo += String(RTL_CONSTASCII_USTRINGPARAM(" "));
                    sRedo += m_aUndoManager.GetRedoActionComment();
                    aReturn.sTitle = sRedo;
                }
                break;
            default:
                aReturn = OSingleDocumentController_Base::GetState(_nId);
        }
        return aReturn;
    }
    // -----------------------------------------------------------------------------
    void OSingleDocumentController::Execute(sal_uInt16 _nId, const Sequence< PropertyValue >& /*aArgs*/)
    {
        switch(_nId)
        {
            case ID_BROWSER_CLOSE:
                closeTask();
                return;
            case ID_BROWSER_UNDO:
                m_aUndoManager.Undo();
                InvalidateFeature(ID_BROWSER_REDO);
                break;
            case ID_BROWSER_REDO:
                m_aUndoManager.Redo();
                InvalidateFeature(ID_BROWSER_UNDO);
                break;
        }
        InvalidateFeature(_nId);
    }
    // -----------------------------------------------------------------------------
    SfxUndoManager* OSingleDocumentController::getUndoMgr()
    {
        return &m_aUndoManager;
    }
    // -----------------------------------------------------------------------------
    void OSingleDocumentController::addUndoActionAndInvalidate(SfxUndoAction *_pAction)
    {
        // add undo action
        m_aUndoManager.AddUndoAction(_pAction);
        // when we add an undo action the controller was modified
        setModified(sal_True);
        // now inform me that or states changed
        InvalidateFeature(ID_BROWSER_UNDO);
        InvalidateFeature(ID_BROWSER_REDO);
    }
    // -----------------------------------------------------------------------------
    void OSingleDocumentController::setModified(sal_Bool _bModified)
    {
        m_pImpl->m_bModified = _bModified;
        InvalidateFeature(ID_BROWSER_SAVEDOC);

        if ( isFeatureSupported( ID_BROWSER_SAVEASDOC ) )
            InvalidateFeature(ID_BROWSER_SAVEASDOC);
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
    sal_Bool OSingleDocumentController::isModified() const
    {
        return m_pImpl->m_bModified;
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
        return isConnected() ? m_pImpl->m_xConnection->getMetaData() : Reference< XDatabaseMetaData >();
    }

    // -----------------------------------------------------------------------------
    const Reference< XPropertySet >& OSingleDocumentController::getDataSource() const
    {
        return m_pImpl->m_aDataSource.getDataSourceProps();
    }
<<<<<<< singledoccontroller.cxx

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
    Reference< XEmbeddedScripts > SAL_CALL OSingleDocumentController::getScriptContainer() throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if ( !m_pImpl->documentHasScriptSupport() )
            return NULL;

        return Reference< XEmbeddedScripts >( getDatabaseDocument(), UNO_QUERY_THROW );
    }

    // -----------------------------------------------------------------------------
    uno::Reference< frame::XModel > OSingleDocumentController::getPrivateModel() const
    {
        return uno::Reference< frame::XModel >(dbaui::getDataSourceOrModel(m_pImpl->m_xDataSource),uno::UNO_QUERY);
    }
    // -----------------------------------------------------------------------------
    // XTitle
    ::rtl::OUString SAL_CALL OSingleDocumentController::getTitle()
        throw (uno::RuntimeException)
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        if ( m_bExternalTitle )
            return impl_getTitleHelper_throw()->getTitle ();

        ::rtl::OUStringBuffer sTitle;
        Reference< XTitle > xTitle(getPrivateModel(),uno::UNO_QUERY);
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
    sal_Int32 OSingleDocumentController::getCurrentStartNumber() const
    {
        return m_pImpl->m_nDocStartNumber;
    }
//........................................................................
}   // namespace dbaui
//........................................................................

