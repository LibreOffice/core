/*************************************************************************
 *
 *  $RCSfile: singledoccontroller.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 16:11:35 $
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

#ifndef DBAUI_SINGLEDOCCONTROLLER_HXX
#include "singledoccontroller.hxx"
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#ifndef DBACCESS_SHARED_DBUSTRINGS_HRC
#include "dbustrings.hrc"
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XDATASOURCE_HPP_
#include <com/sun/star/sdbc/XDataSource.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCHILD_HPP_
#include <com/sun/star/container/XChild.hpp>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _DBU_MISC_HRC_
#include "dbu_misc.hrc"
#endif
#ifndef DBAUI_DATAVIEW_HXX
#include "dataview.hxx"
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include <connectivity/dbexception.hxx>
#endif
#ifndef DBACCESS_UI_BROWSER_ID_HXX
#include "browserids.hxx"
#endif

//........................................................................
namespace dbaui
{
//........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::frame;

    //====================================================================
    //= OConnectionChangeBroadcaster
    //====================================================================
    class OConnectionChangeBroadcaster
    {
    private:
        OSingleDocumentController*  m_pController;
        Reference< XConnection >    m_xOldConnection;

    public:
        OConnectionChangeBroadcaster( OSingleDocumentController* _pController );
        ~OConnectionChangeBroadcaster();
    };

    DBG_NAME(OConnectionChangeBroadcaster)
    //--------------------------------------------------------------------
    OConnectionChangeBroadcaster::OConnectionChangeBroadcaster( OSingleDocumentController* _pController )
        :m_pController( _pController )
    {
        DBG_CTOR(OConnectionChangeBroadcaster,NULL);
        DBG_ASSERT( m_pController, "OConnectionChangeBroadcaster::OConnectionChangeBroadcaster: invalid controller!" );
        if ( m_pController )
            m_xOldConnection = m_pController->getConnection();
    }

    //--------------------------------------------------------------------
    OConnectionChangeBroadcaster::~OConnectionChangeBroadcaster()
    {
        DBG_DTOR(OConnectionChangeBroadcaster,NULL);
        if ( m_pController )
        {
            // has the connection change while we were constructed?
            Reference< XConnection > xNewConnection = m_pController->getConnection();
            if ( m_xOldConnection.get() != xNewConnection.get() )
            {
                // yes -> fire the property change
                sal_Int32 mHandle = PROPERTY_ID_ACTIVECONNECTION;
                Any aNewValue = makeAny( xNewConnection );
                Any aOldValue = makeAny( m_xOldConnection );
                m_pController->fire( &mHandle, &aNewValue, &aOldValue, 1, sal_False );
            }
        }
    }

    //====================================================================
    //= OSingleDocumentController
    //====================================================================
    //--------------------------------------------------------------------
    OSingleDocumentController::OSingleDocumentController(const Reference< XMultiServiceFactory >& _rxORB)
        :OSingleDocumentController_CBASE( _rxORB )
        ,OSingleDocumentController_PBASE( getBroadcastHelper() )
        ,m_bOwnConnection( sal_False )
        ,m_bSuspended( sal_False )
        ,m_bEditable(sal_True)
        ,m_bModified(sal_False)

    {
        registerProperty( PROPERTY_ACTIVECONNECTION, PROPERTY_ID_ACTIVECONNECTION, PropertyAttribute::READONLY | PropertyAttribute::BOUND,
            &m_xConnection, ::getCppuType( &m_xConnection ) );
    }

    //--------------------------------------------------------------------
    Any SAL_CALL OSingleDocumentController::queryInterface( const Type& _rType ) throw (RuntimeException)
    {
        Any aReturn = OSingleDocumentController_CBASE::queryInterface( _rType );
        if ( !aReturn.hasValue() )
            aReturn = OSingleDocumentController_PBASE::queryInterface( _rType );
        return aReturn;
    }

    //--------------------------------------------------------------------
    void SAL_CALL OSingleDocumentController::acquire(  ) throw ()
    {
        OSingleDocumentController_CBASE::acquire();
    }

    //--------------------------------------------------------------------
    void SAL_CALL OSingleDocumentController::release(  ) throw ()
    {
        OSingleDocumentController_CBASE::release();
    }

    //--------------------------------------------------------------------
    Sequence<sal_Int8> SAL_CALL OSingleDocumentController::getImplementationId(  ) throw(RuntimeException)
    {
        static ::cppu::OImplementationId * pId = 0;
        if (! pId)
        {
            ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
            if (! pId)
            {
                static ::cppu::OImplementationId aId;
                pId = &aId;
            }
        }
        return pId->getImplementationId();
    }

    //--------------------------------------------------------------------
    Sequence< Type > SAL_CALL OSingleDocumentController::getTypes(  ) throw (RuntimeException)
    {
        return ::comphelper::concatSequences(
            OSingleDocumentController_CBASE::getTypes(),
            OSingleDocumentController_PBASE::getTypes()
        );
    }

    //-------------------------------------------------------------------------
    Reference<XPropertySetInfo>  SAL_CALL OSingleDocumentController::getPropertySetInfo() throw(RuntimeException)
    {
        Reference<XPropertySetInfo>  xInfo( createPropertySetInfo( getInfoHelper() ) );
        return xInfo;
    }

    //-------------------------------------------------------------------------
    ::cppu::IPropertyArrayHelper& OSingleDocumentController::getInfoHelper()
    {
        return *const_cast<OSingleDocumentController*>(this)->getArrayHelper();
    }

    //--------------------------------------------------------------------
    ::cppu::IPropertyArrayHelper* OSingleDocumentController::createArrayHelper( ) const
    {
        Sequence< Property > aProps;
        describeProperties(aProps);
        return new ::cppu::OPropertyArrayHelper(aProps);
    }
    //--------------------------------------------------------------------
    void OSingleDocumentController::initializeConnection( const Reference< XConnection >& _rxForeignConn )
    {
        OConnectionChangeBroadcaster( this );

        DBG_ASSERT( !isConnected(), "OSingleDocumentController::initializeConnection: not to be called when already connected!" );
            // usually this gets called from within initialize of derived classes ...
        if ( isConnected() )
            disconnect();

        m_xConnection       = _rxForeignConn;
        m_bOwnConnection    = sal_False;
        startConnectionListening( m_xConnection );

        // get the data source the connection belongs to
        if ( !m_xDataSource.is() )
        {
            try
            {
                Reference< XChild > xConnAsChild( m_xConnection, UNO_QUERY );
                Reference< XDataSource > xDS;
                if ( xConnAsChild.is() )
                    xDS = Reference< XDataSource >( xConnAsChild->getParent(), UNO_QUERY );

                // (take the indirection through XDataSource to ensure we have a correct object ....)
                m_xDataSource.set(xDS,UNO_QUERY);
                DBG_ASSERT( m_xDataSource.is(), "OSingleDocumentController::initializeConnection: could not retrieve the data source!" );

                if ( m_xDataSource.is() )
                {
                    m_xDataSource->getPropertyValue( PROPERTY_NAME ) >>= m_sDataSourceName;
                    DBG_ASSERT( m_sDataSourceName.getLength(), "OSingleDocumentController::initializeConnection: invalid data source name!" );
                }
            }
            catch( const Exception& )
            {
                DBG_ERROR( "OSingleDocumentController::initializeConnection: caught an exception!" );
            }
        }
    }

    //--------------------------------------------------------------------
    void OSingleDocumentController::reconnect( sal_Bool _bUI )
    {
        OSL_ENSURE(!m_bSuspended, "Cannot reconnect while suspended!");
        OConnectionChangeBroadcaster( this );

        stopConnectionListening(m_xConnection);
        if ( m_bOwnConnection )
        {
            try
            {
                ::comphelper::disposeComponent( m_xConnection );
            }
            catch( const Exception& ) { /* allowed: if we reconnect because the old connection has been disposed .... */ }
        }
        m_xConnection       = NULL;
        m_bOwnConnection    = sal_False;


        // reconnect
        sal_Bool bReConnect = sal_True;
        if ( _bUI )
        {
            QueryBox aQuery( getView(), ModuleRes(TABLE_QUERY_CONNECTION_LOST) );
            bReConnect = ( RET_YES == aQuery.Execute() );
        }

        // now really reconnect ...
        if ( bReConnect )
        {
            m_xConnection = connect( Reference<XDataSource>(m_xDataSource,UNO_QUERY), sal_True );
            m_bOwnConnection = m_xConnection.is();
        }

        // invalidate all slots
        InvalidateAll();
    }

    //--------------------------------------------------------------------
    void OSingleDocumentController::disconnect()
    {
        OConnectionChangeBroadcaster( this );

        stopConnectionListening(m_xConnection);
        try
        {
            if ( m_bOwnConnection )
                ::comphelper::disposeComponent( m_xConnection );
        }
        catch(const Exception&)
        {
        }
        m_xConnection = NULL;
        m_bOwnConnection = sal_False;

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
        OSingleDocumentController_CBASE::disposing();
        OSingleDocumentController_PBASE::disposing();
        m_aUndoManager.Clear();

        disconnect();
        Reference<XModel> xModel(m_xDataSource,UNO_QUERY);
        if ( xModel.is() )
            xModel->disconnectController( this );

        Reference < XFrame > xFrame;
        attachFrame( xFrame );


        m_xDataSource.clear();
    }

    //--------------------------------------------------------------------
    sal_Bool OSingleDocumentController::Construct(Window* _pParent)
    {
        DBG_ASSERT( getView(), "OSingleDocumentController::Construct: have no view!" );
        if ( getView() )
            getView()->enableSeparator( );

        return OSingleDocumentController_CBASE::Construct( _pParent );
    }

    //--------------------------------------------------------------------
    void SAL_CALL OSingleDocumentController::disposing(const EventObject& _rSource) throw( RuntimeException )
    {
        if ( _rSource.Source == getConnection() )
        {
            if (    !m_bSuspended // when already suspended then we don't have to reconnect
                &&  !getBroadcastHelper().bInDispose
                &&  !getBroadcastHelper().bDisposed
                &&  isConnected()
                )
            {
                losingConnection();
            }
            else
            {
                m_bOwnConnection = sal_False;   // this prevents the "disposeComponent" call in disconnect
                disconnect();
            }
        }
        else
            OSingleDocumentController_CBASE::disposing( _rSource );
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
                ::dbtools::SQLExceptionIteratorHelper aReferenceIterHelper(pChainTravel);
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
    void OSingleDocumentController::appendError(const ::com::sun::star::sdbc::SQLException& _aException)
    {
        concatSQLExceptions(m_aCurrentError,makeAny(_aException));
    }
    //--------------------------------------------------------------------
    /** clears the error state.
    */
    void OSingleDocumentController::clearError()
    {
        m_aCurrentError = Any();
    }
    //--------------------------------------------------------------------
    /** set the current error in the given parameter.
        @param  _rException
            will contain the current error
    */
    void OSingleDocumentController::getError(::com::sun::star::sdbc::SQLException& _rException ) const
    {
        m_aCurrentError >>= _rException;
    }
    //--------------------------------------------------------------------
    sal_Bool OSingleDocumentController::hasError() const
    {
        return m_aCurrentError.hasValue();
    }
    //--------------------------------------------------------------------
    sal_Bool SAL_CALL OSingleDocumentController::suspend(sal_Bool bSuspend) throw( RuntimeException )
    {
        m_bSuspended = bSuspend;
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
                aReturn.bEnabled = m_bEditable && m_aUndoManager.GetUndoActionCount() != 0;
                if ( aReturn.bEnabled )
                {
                    String sUndo(ModuleRes(STR_UNDO_COLON));
                    sUndo += String(RTL_CONSTASCII_USTRINGPARAM(" "));
                    sUndo += m_aUndoManager.GetUndoActionComment();
                    aReturn.aState <<= ::rtl::OUString(sUndo);
                }
                break;
            case ID_BROWSER_REDO:
                aReturn.bEnabled = m_bEditable && m_aUndoManager.GetRedoActionCount() != 0;
                if ( aReturn.bEnabled )
                {
                    String sRedo(ModuleRes(STR_REDO_COLON));
                    sRedo += String(RTL_CONSTASCII_USTRINGPARAM(" "));
                    sRedo += m_aUndoManager.GetRedoActionComment();
                    aReturn.aState <<= ::rtl::OUString(sRedo);
                }
                break;
            default:
                aReturn = OSingleDocumentController_CBASE::GetState(_nId);
        }
        return aReturn;
    }
    // -----------------------------------------------------------------------------
    void OSingleDocumentController::Execute(sal_uInt16 _nId)
    {
        switch(_nId)
        {
            case ID_BROWSER_CLOSE:
                closeTask();
                return;
                break;
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
        m_bModified = _bModified;
        InvalidateFeature(ID_BROWSER_SAVEDOC);
        InvalidateFeature(ID_BROWSER_SAVEASDOC);
    }
    // -----------------------------------------------------------------------------
    Reference< XModel >  SAL_CALL OSingleDocumentController::getModel(void) throw( RuntimeException )
    {
        return NULL;//Reference< XModel >(m_xDataSource,UNO_QUERY); // OJ: i31891
    }
    // -----------------------------------------------------------------------------
    sal_Bool SAL_CALL OSingleDocumentController::attachModel(const Reference< XModel > & xModel) throw( RuntimeException )
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        m_xDataSource.set(xModel,UNO_QUERY);
        return m_xDataSource.is();
    }
    // -----------------------------------------------------------------------------
     ::rtl::OUString OSingleDocumentController::getDataSourceName() const
     {
         ::rtl::OUString sName;
         if ( m_xDataSource.is() )
             m_xDataSource->getPropertyValue(PROPERTY_NAME) >>= sName;
         return sName;
     }
//........................................................................
}   // namespace dbaui
//........................................................................

