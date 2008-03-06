/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: brwctrlr.cxx,v $
 *
 *  $Revision: 1.105 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 18:13:16 $
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


#ifndef _SBA_BWRCTRLR_HXX
#include "brwctrlr.hxx"
#endif
#ifndef _SBX_BRWVIEW_HXX
#include "brwview.hxx"
#endif
#ifndef _OSL_MUTEX_HXX_ //autogen wg. MutexGuard
#include <osl/mutex.hxx>
#endif
#ifndef _SFXAPP_HXX //autogen wg. SFX_APP
#include <sfx2/app.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_TYPECLASS_HPP_
#include <com/sun/star/uno/TypeClass.hpp>
#endif
#ifndef _DBAUI_SQLMESSAGE_HXX_
#include "sqlmessage.hxx"
#endif
#ifndef _COM_SUN_STAR_FORM_XFORMCONTROLLER_HPP_
#include <com/sun/star/form/XFormController.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_COMMANDTYPE_HPP_
#include <com/sun/star/sdb/CommandType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROWSETLISTENER_HPP_
#include <com/sun/star/sdbc/XRowSetListener.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XROWLOCATE_HPP_
#include <com/sun/star/sdbcx/XRowLocate.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSETUPDATE_HPP_
#include <com/sun/star/sdbc/XResultSetUpdate.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XSQLERRORBROADCASTER_HPP_
#include <com/sun/star/sdb/XSQLErrorBroadcaster.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XSINGLESELECTQUERYCOMPOSER_HPP_
#include <com/sun/star/sdb/XSingleSelectQueryComposer.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XLOADABLE_HPP_
#include <com/sun/star/form/XLoadable.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_SQLCONTEXT_HPP_
#include <com/sun/star/sdb/SQLContext.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_FORMBUTTONTYPE_HPP_
#include <com/sun/star/form/FormButtonType.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XCHANGELISTENER_HPP_
#include <com/sun/star/form/XChangeListener.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_FORMSUBMITENCODING_HPP_
#include <com/sun/star/form/FormSubmitEncoding.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_FORMSUBMITMETHOD_HPP_
#include <com/sun/star/form/FormSubmitMethod.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XSUBMITLISTENER_HPP_
#include <com/sun/star/form/XSubmitListener.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XRESET_HPP_
#include <com/sun/star/form/XReset.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XSUBMIT_HPP_
#include <com/sun/star/form/XSubmit.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XAPPROVEACTIONBROADCASTER_HPP_
#include <com/sun/star/form/XApproveActionBroadcaster.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XCHANGEBROADCASTER_HPP_
#include <com/sun/star/form/XChangeBroadcaster.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XRESETLISTENER_HPP_
#include <com/sun/star/form/XResetListener.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XCANCELLABLE_HPP_
#include <com/sun/star/util/XCancellable.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XDATABASEPARAMETERBROADCASTER_HPP_
#include <com/sun/star/form/XDatabaseParameterBroadcaster.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XBOUNDCONTROL_HPP_
#include <com/sun/star/form/XBoundControl.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMED_HPP_
#include <com/sun/star/container/XNamed.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_PRIVILEGE_HPP_
#include <com/sun/star/sdbcx/Privilege.hpp>
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _FMSEARCH_HXX
#include <svx/fmsearch.hxx>
#endif
#ifndef _SV_WAITOBJ_HXX
#include <vcl/waitobj.hxx>
#endif
#ifndef _TOOLS_COLOR_HXX
#include <tools/color.hxx>
#endif
#ifndef TOOLS_DIAGNOSE_EX_H
#include <tools/diagnose_ex.h>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif
#ifndef _DBU_BRW_HRC_
#include "dbu_brw.hrc"
#endif
#ifndef _SFX_HRC
#include <sfx2/sfx.hrc>
#endif
#ifndef DBACCESS_UI_BROWSER_ID_HXX
#include "browserids.hxx"
#endif
#ifndef DBACCESS_SHARED_DBUSTRINGS_HRC
#include "dbustrings.hrc"
#endif
#ifndef _COMPHELPER_INTERACTION_HXX_
#include <comphelper/interaction.hxx>
#endif
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include <connectivity/dbexception.hxx>
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef _COM_SUN_STAR_SDB_XINTERACTIONSUPPLYPARAMETERS_HPP_
#include <com/sun/star/sdb/XInteractionSupplyParameters.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_PARAMETERSREQUEST_HPP_
#include <com/sun/star/sdb/ParametersRequest.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONHANDLER_HPP_
#include <com/sun/star/task/XInteractionHandler.hpp>
#endif
#ifndef DBAUI_QUERYFILTER_HXX
#include "queryfilter.hxx"
#endif
#ifndef DBAUI_QUERYORDER_HXX
#include "queryorder.hxx"
#endif

#include <svx/svxdlg.hxx> //CHINA001
//#include <svx/fmresids.hrc> //CHINA001

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::form;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::dbtools;
using namespace ::comphelper;
using namespace ::svt;

#define HANDLE_SQL_ERRORS( action, successflag, context, message )          \
    try                                                                     \
    {                                                                       \
        successflag = sal_False;                                            \
        action;                                                             \
        successflag = sal_True;                                             \
    }                                                                       \
    catch(SQLException& e)                                                  \
    {                                                                       \
        SQLException aError = ::dbtools::prependErrorInfo(e, *this, context); \
        ::com::sun::star::sdb::SQLErrorEvent aEvent;                        \
        aEvent.Reason <<= aError;                                           \
        errorOccured(aEvent);                                               \
    }                                                                       \
    catch(Exception&)                                                       \
    {                                                                       \
        DBG_UNHANDLED_EXCEPTION();                                          \
    }                                                                       \

#define DO_SAFE( action, message ) try { action; } catch(Exception&) { DBG_ERROR(message); } ;

//..................................................................
namespace dbaui
{
//..................................................................

//==================================================================
// OParameterContinuation
//==================================================================
class OParameterContinuation : public OInteraction< XInteractionSupplyParameters >
{
    Sequence< PropertyValue >       m_aValues;

public:
    OParameterContinuation() { }

    Sequence< PropertyValue >   getValues() const { return m_aValues; }

// XInteractionSupplyParameters
    virtual void SAL_CALL setParameters( const Sequence< PropertyValue >& _rValues ) throw(RuntimeException);
};

//------------------------------------------------------------------
void SAL_CALL OParameterContinuation::setParameters( const Sequence< PropertyValue >& _rValues ) throw(RuntimeException)
{
    m_aValues = _rValues;
}


//==============================================================================
// a helper class implementing a ::com::sun::star::form::XFormController, will be aggregated by SbaXDataBrowserController
// (we can't derive from ::com::sun::star::form::XFormController as it's base class is ::com::sun::star::awt::XTabController and the ::com::sun::star::awt::XTabController::getModel collides
// with the ::com::sun::star::frame::XController::getModel implemented in our base class SbaXDataBrowserController)
class SbaXDataBrowserController::FormControllerImpl
    : public ::cppu::WeakAggImplHelper2< ::com::sun::star::form::XFormController,
                                         ::com::sun::star::frame::XFrameActionListener>
{
    friend class SbaXDataBrowserController;
    ::cppu::OInterfaceContainerHelper   m_aActivateListeners;
    SbaXDataBrowserController*          m_pOwner;

public:
    FormControllerImpl(SbaXDataBrowserController* pOwner);

    // ::com::sun::star::form::XFormController
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >  SAL_CALL getCurrentControl(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL addActivateListener(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormControllerListener > & l) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL removeActivateListener(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormControllerListener > & l) throw( ::com::sun::star::uno::RuntimeException );

    // ::com::sun::star::awt::XTabController
    virtual void SAL_CALL setModel(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTabControllerModel > & Model) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTabControllerModel >  SAL_CALL getModel(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL setContainer(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer > & _Container) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer >  SAL_CALL getContainer(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >  > SAL_CALL getControls(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL autoTabOrder(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL activateTabOrder(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL activateFirst(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL activateLast(void) throw( ::com::sun::star::uno::RuntimeException );

    // ::com::sun::star::frame::XFrameActionListener
    virtual void SAL_CALL frameAction(const ::com::sun::star::frame::FrameActionEvent& aEvent) throw( ::com::sun::star::uno::RuntimeException );

    // ::com::sun::star::lang::XEventListener
    virtual void SAL_CALL disposing(const ::com::sun::star::lang::EventObject& Source) throw( ::com::sun::star::uno::RuntimeException );

protected:
    ~FormControllerImpl();
};


DBG_NAME(FormControllerImpl)
//------------------------------------------------------------------
SbaXDataBrowserController::FormControllerImpl::FormControllerImpl(SbaXDataBrowserController* _pOwner)
    :m_aActivateListeners(_pOwner->getMutex())
    ,m_pOwner(_pOwner)
{
    DBG_CTOR(FormControllerImpl,NULL);

    OSL_ENSURE(m_pOwner, "SbaXDataBrowserController::FormControllerImpl::FormControllerImpl : invalid Owner !");
}

//------------------------------------------------------------------
SbaXDataBrowserController::FormControllerImpl::~FormControllerImpl()
{

    DBG_DTOR(FormControllerImpl,NULL);
}

//------------------------------------------------------------------
Reference< ::com::sun::star::awt::XControl >  SbaXDataBrowserController::FormControllerImpl::getCurrentControl(void) throw( RuntimeException )
{
    return m_pOwner->getBrowserView() ? m_pOwner->getBrowserView()->getGridControl() : Reference< ::com::sun::star::awt::XControl > ();
}

//------------------------------------------------------------------
void SAL_CALL SbaXDataBrowserController::FormControllerImpl::addActivateListener(const Reference< ::com::sun::star::form::XFormControllerListener > & l) throw( RuntimeException )
{
    m_aActivateListeners.addInterface(l);
}

//------------------------------------------------------------------
void SAL_CALL SbaXDataBrowserController::FormControllerImpl::removeActivateListener(const Reference< ::com::sun::star::form::XFormControllerListener > & l) throw( RuntimeException )
{
    m_aActivateListeners.removeInterface(l);
}

//------------------------------------------------------------------
void SAL_CALL SbaXDataBrowserController::FormControllerImpl::setModel(const Reference< ::com::sun::star::awt::XTabControllerModel > & /*Model*/) throw( RuntimeException )
{
    OSL_ENSURE(sal_False, "SbaXDataBrowserController::FormControllerImpl::setModel : invalid call, can't change my model !");
}

//------------------------------------------------------------------
Reference< ::com::sun::star::awt::XTabControllerModel >  SAL_CALL SbaXDataBrowserController::FormControllerImpl::getModel(void) throw( RuntimeException )
{
    return Reference< XTabControllerModel >(m_pOwner->getRowSet(), UNO_QUERY);
}

//------------------------------------------------------------------
void SAL_CALL SbaXDataBrowserController::FormControllerImpl::setContainer(const Reference< ::com::sun::star::awt::XControlContainer > & /*_Container*/) throw( RuntimeException )
{
    OSL_ENSURE(sal_False, "SbaXDataBrowserController::FormControllerImpl::setContainer : invalid call, can't change my container !");
}

//------------------------------------------------------------------
Reference< ::com::sun::star::awt::XControlContainer >  SAL_CALL SbaXDataBrowserController::FormControllerImpl::getContainer(void) throw( RuntimeException )
{
    if (m_pOwner->getBrowserView())
        return m_pOwner->getBrowserView()->getContainer();
    return Reference< ::com::sun::star::awt::XControlContainer > ();
}

//------------------------------------------------------------------
Sequence< Reference< ::com::sun::star::awt::XControl > > SAL_CALL SbaXDataBrowserController::FormControllerImpl::getControls(void) throw( RuntimeException )
{
    if (m_pOwner->getBrowserView())
    {
        Reference< ::com::sun::star::awt::XControl >  xGrid = m_pOwner->getBrowserView()->getGridControl();
        return Sequence< Reference< ::com::sun::star::awt::XControl > >(&xGrid, 1);
    }
    return Sequence< Reference< ::com::sun::star::awt::XControl > >();
}

//------------------------------------------------------------------
void SAL_CALL SbaXDataBrowserController::FormControllerImpl::autoTabOrder(void) throw( RuntimeException )
{
    OSL_ENSURE(sal_False, "SbaXDataBrowserController::FormControllerImpl::autoTabOrder : nothing to do (always have only one control) !");
}

//------------------------------------------------------------------
void SAL_CALL SbaXDataBrowserController::FormControllerImpl::activateTabOrder(void) throw( RuntimeException )
{
    OSL_ENSURE(sal_False, "SbaXDataBrowserController::FormControllerImpl::activateTabOrder : nothing to do (always have only one control) !");
}

//------------------------------------------------------------------
void SAL_CALL SbaXDataBrowserController::FormControllerImpl::activateFirst(void) throw( RuntimeException )
{
    if (m_pOwner->getBrowserView())
        m_pOwner->getBrowserView()->getVclControl()->ActivateCell();
}

//------------------------------------------------------------------
void SAL_CALL SbaXDataBrowserController::FormControllerImpl::activateLast(void) throw( RuntimeException )
{
    if (m_pOwner->getBrowserView())
        m_pOwner->getBrowserView()->getVclControl()->ActivateCell();
}

//------------------------------------------------------------------
void SAL_CALL SbaXDataBrowserController::FormControllerImpl::frameAction(const ::com::sun::star::frame::FrameActionEvent& /*aEvent*/) throw( RuntimeException )
{
}

//------------------------------------------------------------------
void SAL_CALL SbaXDataBrowserController::FormControllerImpl::disposing(const ::com::sun::star::lang::EventObject& /*Source*/) throw( RuntimeException )
{
    // nothing to do
    // we don't add ourself as listener to any broadcasters, so we are not resposible for removing us
}

//==================================================================
//= SbaXDataBrowserController
//==================================================================
//------------------------------------------------------------------
Sequence< Type > SAL_CALL SbaXDataBrowserController::getTypes(  ) throw (RuntimeException)
{
    return ::comphelper::concatSequences(
        SbaXDataBrowserController_Base::getTypes(),
        m_pFormControllerImpl->getTypes()
    );
}

//------------------------------------------------------------------
Sequence< sal_Int8 > SAL_CALL SbaXDataBrowserController::getImplementationId(  ) throw (RuntimeException)
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

//------------------------------------------------------------------
Any SAL_CALL SbaXDataBrowserController::queryInterface(const Type& _rType) throw (RuntimeException)
{
    // check for our additional interfaces
    Any aRet = SbaXDataBrowserController_Base::queryInterface(_rType);

    // check for the base controllers interfaces
    if (!aRet.hasValue())
    {
        // check for our aggregate (implementing the XFormController)
        if (!aRet.hasValue())
        {
            aRet = m_xFormControllerImpl->queryAggregation(_rType);
        }
    }

    // no more to offer
    return aRet;
}

DBG_NAME(SbaXDataBrowserController)
//------------------------------------------------------------------------------
SbaXDataBrowserController::SbaXDataBrowserController(const Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rM)
    :SbaXDataBrowserController_Base(_rM)
    ,m_pClipbordNotifier( NULL )
    ,m_aAsyncGetCellFocus(LINK(this, SbaXDataBrowserController, OnAsyncGetCellFocus))
    ,m_sStateSaveRecord(ModuleRes(RID_STR_SAVE_CURRENT_RECORD))
    ,m_sStateUndoRecord(ModuleRes(RID_STR_UNDO_MODIFY_RECORD))
    ,m_sModuleIdentifier( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.sdb.DataSourceBrowser" ) ) )
    ,m_pLoadThread(NULL)
    ,m_pFormControllerImpl(NULL)
    ,m_nPendingLoadFinished(0)
    ,m_nFormActionNestingLevel(0)
    ,m_bLoadCanceled( sal_False )
    ,m_bClosingKillOpen( sal_False )
    ,m_bErrorOccured( sal_False )
{
    DBG_CTOR(SbaXDataBrowserController,NULL);

    // create the form controller aggregate
    ::comphelper::increment(m_refCount);
    {
        m_pFormControllerImpl = new FormControllerImpl(this);
        m_xFormControllerImpl = m_pFormControllerImpl;
        m_xFormControllerImpl->setDelegator(*this);
    }
    ::comphelper::decrement(m_refCount);

    m_aInvalidateClipboard.SetTimeoutHdl(LINK(this, SbaXDataBrowserController, OnInvalidateClipboard));
    m_aInvalidateClipboard.SetTimeout(300);
}

//------------------------------------------------------------------------------
SbaXDataBrowserController::~SbaXDataBrowserController()
{
    //  deleteView();
    // release the aggregated form controller
    if (m_xFormControllerImpl.is())
    {
        Reference< XInterface >  xEmpty;
        m_xFormControllerImpl->setDelegator(xEmpty);
    }

    DBG_DTOR(SbaXDataBrowserController,NULL);
}

// -----------------------------------------------------------------------
void SbaXDataBrowserController::startFrameListening( const Reference< XFrame >& _rxFrame )
{
    SbaXDataBrowserController_Base::startFrameListening( _rxFrame );

    Reference< XFrameActionListener >   xAggListener;
    if ( m_xFormControllerImpl.is() )
        m_xFormControllerImpl->queryAggregation( XFrameActionListener::static_type() ) >>= xAggListener;

    if ( _rxFrame.is() && xAggListener.is() )
        _rxFrame->addFrameActionListener( xAggListener );
}

// -----------------------------------------------------------------------
void SbaXDataBrowserController::stopFrameListening( const Reference< XFrame >& _rxFrame )
{
    SbaXDataBrowserController_Base::stopFrameListening( _rxFrame );

    Reference< XFrameActionListener >   xAggListener;
    if ( m_xFormControllerImpl.is() )
        m_xFormControllerImpl->queryAggregation( XFrameActionListener::static_type() ) >>= xAggListener;

    if ( _rxFrame.is() && xAggListener.is() )
        _rxFrame->removeFrameActionListener( xAggListener );
}

// -----------------------------------------------------------------------------
sal_Bool SbaXDataBrowserController::reloadForm(const Reference< XLoadable >& _rxLoadable)
{
    WaitObject aWO(getBrowserView());

    setLoadingStarted();

    FormErrorHelper aReportError(this);
    if (_rxLoadable->isLoaded())
        _rxLoadable->reload();
    else
        _rxLoadable->load();

    return _rxLoadable->isLoaded() && !errorOccured();
}

// -----------------------------------------------------------------------------
void SbaXDataBrowserController::initFormatter()
{
    // ---------------------------------------------------------------
    // create a formatter working with the connections format supplier
    Reference< ::com::sun::star::util::XNumberFormatsSupplier >  xSupplier(::dbtools::getNumberFormats(::dbtools::getConnection(m_xRowSet), sal_True,getORB()));

    if(xSupplier.is())
    {
        // create a new formatter
        m_xFormatter = Reference< ::com::sun::star::util::XNumberFormatter > (
            getORB()->createInstance(::rtl::OUString::createFromAscii("com.sun.star.util.NumberFormatter")), UNO_QUERY);
        if (m_xFormatter.is())
            m_xFormatter->attachNumberFormatsSupplier(xSupplier);
    }
    else // clear the formatter
        m_xFormatter = NULL;
}
// -----------------------------------------------------------------------------
void SbaXDataBrowserController::describeSupportedFeatures()
{
    SbaXDataBrowserController_Base::describeSupportedFeatures();
    implDescribeSupportedFeature( ".uno:FormSlots/undoRecord",      ID_BROWSER_UNDORECORD,  CommandGroup::CONTROLS );
    implDescribeSupportedFeature( ".uno:FormController/undoRecord", ID_BROWSER_UNDORECORD,  CommandGroup::CONTROLS );
    implDescribeSupportedFeature( ".uno:RecUndo",                   ID_BROWSER_UNDORECORD,  CommandGroup::CONTROLS );
    implDescribeSupportedFeature( ".uno:FormSlots/saveRecord",      ID_BROWSER_SAVERECORD,  CommandGroup::CONTROLS );
    implDescribeSupportedFeature( ".uno:FormController/saveRecord", ID_BROWSER_SAVERECORD,  CommandGroup::CONTROLS );
    implDescribeSupportedFeature( ".uno:RecSave",                   ID_BROWSER_SAVERECORD,  CommandGroup::CONTROLS );
    implDescribeSupportedFeature( ".uno:Save",                      ID_BROWSER_SAVERECORD,  CommandGroup::DOCUMENT );
    implDescribeSupportedFeature( ".uno:RecSearch",                 SID_FM_SEARCH,          CommandGroup::CONTROLS );
    implDescribeSupportedFeature( ".uno:AutoFilter",                SID_FM_AUTOFILTER,      CommandGroup::CONTROLS );
    implDescribeSupportedFeature( ".uno:Refresh",                   SID_FM_REFRESH,         CommandGroup::CONTROLS );
    implDescribeSupportedFeature( ".uno:OrderCrit",                 SID_FM_ORDERCRIT,       CommandGroup::CONTROLS );
    implDescribeSupportedFeature( ".uno:RemoveFilterSort",          SID_FM_REMOVE_FILTER_SORT,CommandGroup::CONTROLS );
    implDescribeSupportedFeature( ".uno:FormFiltered",              SID_FM_FORM_FILTERED,   CommandGroup::CONTROLS );
    implDescribeSupportedFeature( ".uno:FilterCrit",                SID_FM_FILTERCRIT,      CommandGroup::CONTROLS );
    implDescribeSupportedFeature( ".uno:Sortup",                    ID_BROWSER_SORTUP,      CommandGroup::CONTROLS );
    implDescribeSupportedFeature( ".uno:SortDown",                  ID_BROWSER_SORTDOWN,    CommandGroup::CONTROLS );
    implDescribeSupportedFeature( ".uno:FormSlots/deleteRecord",    SID_FM_DELETEROWS,      CommandGroup::EDIT );
    implDescribeSupportedFeature( ".uno:FormSlots/insertRecord",    ID_BROWSER_INSERT_ROW,  CommandGroup::INSERT );
}
//------------------------------------------------------------------------------
sal_Bool SbaXDataBrowserController::Construct(Window* pParent)
{
    // ---------------------------------------------
    // create/initialize the form and the grid model
    m_xRowSet = CreateForm();
    if (!m_xRowSet.is())
        return sal_False;

    m_xColumnsSupplier.set(m_xRowSet,UNO_QUERY);
    m_xLoadable.set(m_xRowSet,UNO_QUERY);

    if (!InitializeForm(m_xRowSet))
        return sal_False;

    m_xGridModel = CreateGridModel();
    if (!m_xGridModel.is())
        return sal_False;

    // set the formatter if available
    initFormatter();
    // ---------------------------------------------------------------

    // we want to have a grid with a "flat" border
    Reference< XPropertySet >  xGridSet(m_xGridModel, UNO_QUERY);
    if ( xGridSet.is() )
        xGridSet->setPropertyValue(PROPERTY_BORDER, makeAny((sal_Int16)2));

    // ----------
    // marry them
    Reference< ::com::sun::star::container::XNameContainer >  xNameCont(m_xRowSet, UNO_QUERY);
    {
        String sText(ModuleRes(STR_DATASOURCE_GRIDCONTROL_NAME));
        xNameCont->insertByName(::rtl::OUString(sText), makeAny(m_xGridModel));
    }

    // ---------------
    // create the view
    m_pView = new UnoDataBrowserView(pParent,this,getORB());
    if (!getBrowserView())
        return sal_False;

    // late construction
    sal_Bool bSuccess = sal_False;
    try
    {
        getBrowserView()->Construct(getControlModel());
        bSuccess = sal_True;
    }
    catch(SQLException&)
    {
    }
    catch(Exception&)
    {
        DBG_ERROR("SbaXDataBrowserController::Construct : the construction of UnoDataBrowserView failed !");
    }

    if (!bSuccess)
    {
        //  deleteView();
        return sal_False;
    }

    // now that we have a view we can create the clipboard listener
    m_aSystemClipboard = TransferableDataHelper::CreateFromSystemClipboard( getView() );
    m_aSystemClipboard.StartClipboardListening( );

    m_pClipbordNotifier = new TransferableClipboardListener( LINK( this, SbaXDataBrowserController, OnClipboardChanged ) );
    m_pClipbordNotifier->acquire();
    m_pClipbordNotifier->AddRemoveListener( getView(), sal_True );

    // this call create the toolbox
    SbaXDataBrowserController_Base::Construct(pParent);

    getBrowserView()->Show();

    // set the callbacks for the grid control
    SbaGridControl* pVclGrid = getBrowserView()->getVclControl();
    DBG_ASSERT(pVclGrid, "SbaXDataBrowserController::Construct : have no VCL control !");
    pVclGrid->SetMasterListener(this);

    // --------------------------
    // add listeners ...

    // ... to the form model
    Reference< XPropertySet >  xFormSet(getRowSet(), UNO_QUERY);
    if (xFormSet.is())
    {
        xFormSet->addPropertyChangeListener(PROPERTY_ISNEW, static_cast<XPropertyChangeListener*>(this));
        xFormSet->addPropertyChangeListener(PROPERTY_ISMODIFIED, static_cast<XPropertyChangeListener*>(this));
        xFormSet->addPropertyChangeListener(PROPERTY_ROWCOUNT, static_cast<XPropertyChangeListener*>(this));
        xFormSet->addPropertyChangeListener(PROPERTY_ACTIVECOMMAND, static_cast<XPropertyChangeListener*>(this));
        xFormSet->addPropertyChangeListener(PROPERTY_ORDER, static_cast<XPropertyChangeListener*>(this));
        xFormSet->addPropertyChangeListener(PROPERTY_FILTER, static_cast<XPropertyChangeListener*>(this));
        xFormSet->addPropertyChangeListener(PROPERTY_HAVING_CLAUSE, static_cast<XPropertyChangeListener*>(this));
        xFormSet->addPropertyChangeListener(PROPERTY_APPLYFILTER, static_cast<XPropertyChangeListener*>(this));
    }
    Reference< ::com::sun::star::sdb::XSQLErrorBroadcaster >  xFormError(getRowSet(), UNO_QUERY);
    if (xFormError.is())
        xFormError->addSQLErrorListener((::com::sun::star::sdb::XSQLErrorListener*)this);

    if (m_xLoadable.is())
        m_xLoadable->addLoadListener(this);

    Reference< ::com::sun::star::form::XDatabaseParameterBroadcaster >  xFormParameter(getRowSet(), UNO_QUERY);
    if (xFormParameter.is())
        xFormParameter->addParameterListener((::com::sun::star::form::XDatabaseParameterListener*)this);

    addModelListeners(getControlModel());
    addControlListeners(getBrowserView()->getGridControl());

    // -------------
    // load the form
    return LoadForm();
}

//------------------------------------------------------------------------------
sal_Bool SbaXDataBrowserController::LoadForm()
{
    reloadForm( m_xLoadable );
    return sal_True;
}
//------------------------------------------------------------------------------
void SbaXDataBrowserController::AddColumnListener(const Reference< XPropertySet > & /*xCol*/)
{
    // we're not interested in any column properties ...
}

//------------------------------------------------------------------------------
void SbaXDataBrowserController::RemoveColumnListener(const Reference< XPropertySet > & /*xCol*/)
{
}
//------------------------------------------------------------------------------
Reference< XRowSet >  SbaXDataBrowserController::CreateForm()
{
    return Reference< XRowSet > (getORB()->createInstance(::rtl::OUString::createFromAscii("com.sun.star.form.component.Form")), UNO_QUERY);
}

//------------------------------------------------------------------------------
Reference< ::com::sun::star::form::XFormComponent >  SbaXDataBrowserController::CreateGridModel()
{
    return Reference< ::com::sun::star::form::XFormComponent > (getORB()->createInstance(::rtl::OUString::createFromAscii("com.sun.star.form.component.GridControl")), UNO_QUERY);
}

// -------------------------------------------------------------------------
void SbaXDataBrowserController::addModelListeners(const Reference< ::com::sun::star::awt::XControlModel > & _xGridControlModel)
{
    // ... all the grid columns
    addColumnListeners(_xGridControlModel);

    // (we are interested in all columns the grid has (and only in these) so we have to listen to the container, too)
    Reference< ::com::sun::star::container::XContainer >  xColContainer(_xGridControlModel, UNO_QUERY);
    if (xColContainer.is())
        xColContainer->addContainerListener((::com::sun::star::container::XContainerListener*)this);

    Reference< ::com::sun::star::form::XReset >  xReset(_xGridControlModel, UNO_QUERY);
    if (xReset.is())
        xReset->addResetListener((::com::sun::star::form::XResetListener*)this);
}

// -------------------------------------------------------------------------
void SbaXDataBrowserController::removeModelListeners(const Reference< XControlModel > & _xGridControlModel)
{
    // every single column model
    Reference< XIndexContainer >  xColumns(_xGridControlModel, UNO_QUERY);
    if (xColumns.is())
    {
        sal_Int32 nCount = xColumns->getCount();
        for (sal_uInt16 i=0; i < nCount; ++i)
        {
            Reference< XPropertySet >  xCol(xColumns->getByIndex(i),UNO_QUERY);
            RemoveColumnListener(xCol);
        }
    }

    Reference< XContainer >  xColContainer(_xGridControlModel, UNO_QUERY);
    if (xColContainer.is())
        xColContainer->removeContainerListener( this );

    Reference< XReset >  xReset(_xGridControlModel, UNO_QUERY);
    if (xReset.is())
        xReset->removeResetListener( this );
}

// -------------------------------------------------------------------------
void SbaXDataBrowserController::addControlListeners(const Reference< ::com::sun::star::awt::XControl > & _xGridControl)
{
    // to ge the 'modified' for the current cell
    Reference< XModifyBroadcaster >  xBroadcaster(getBrowserView()->getGridControl(), UNO_QUERY);
    if (xBroadcaster.is())
        xBroadcaster->addModifyListener(static_cast<XModifyListener*>(this));

    // introduce ourself as dispatch provider for the grid
    Reference< XDispatchProviderInterception >  xInterception(getBrowserView()->getGridControl(), UNO_QUERY);
    if (xInterception.is())
        xInterception->registerDispatchProviderInterceptor(static_cast<XDispatchProviderInterceptor*>(this));

    // add as focus listener to the control (needed for the form controller functionality)
    Reference< XWindow >  xWindow(_xGridControl, UNO_QUERY);
    if (xWindow.is())
        xWindow->addFocusListener(this);
}

// -------------------------------------------------------------------------
void SbaXDataBrowserController::removeControlListeners(const Reference< ::com::sun::star::awt::XControl > & _xGridControl)
{
    Reference< XModifyBroadcaster >  xBroadcaster(_xGridControl, UNO_QUERY);
    if (xBroadcaster.is())
        xBroadcaster->removeModifyListener(static_cast<XModifyListener*>(this));

    Reference< XDispatchProviderInterception >  xInterception(_xGridControl, UNO_QUERY);
    if (xInterception.is())
        xInterception->releaseDispatchProviderInterceptor(static_cast<XDispatchProviderInterceptor*>(this));

    Reference< XWindow >  xWindow(_xGridControl, UNO_QUERY);
    if (xWindow.is())
        xWindow->removeFocusListener(this);
}

//------------------------------------------------------------------
void SAL_CALL SbaXDataBrowserController::focusGained(const FocusEvent& /*e*/) throw( RuntimeException )
{
    // notify our activate listeners (registered on the form controller aggregate)
    EventObject aEvt(*this);
    ::cppu::OInterfaceIteratorHelper aIter(m_pFormControllerImpl->m_aActivateListeners);
    while (aIter.hasMoreElements())
        static_cast<XFormControllerListener*>(aIter.next())->formActivated(aEvt);
}

//------------------------------------------------------------------
void SAL_CALL SbaXDataBrowserController::focusLost(const FocusEvent& e) throw( RuntimeException )
{
    // some general checks
    if (!getBrowserView() || !getBrowserView()->getGridControl().is())
        return;
    Reference< XVclWindowPeer >  xMyGridPeer(getBrowserView()->getGridControl()->getPeer(), UNO_QUERY);
    if (!xMyGridPeer.is())
        return;
    Reference< XWindowPeer >  xNextControlPeer(e.NextFocus, UNO_QUERY);
    if (!xNextControlPeer.is())
        return;

    // don't do a notification if it remains in the family (i.e. a child of the grid control gets the focus)
    if (xMyGridPeer->isChild(xNextControlPeer))
        return;

    if (xMyGridPeer == xNextControlPeer)
        return;

    // notify the listeners that the "form" we represent has been deactivated
    EventObject aEvt(*this);
    ::cppu::OInterfaceIteratorHelper aIter(m_pFormControllerImpl->m_aActivateListeners);
    while (aIter.hasMoreElements())
        static_cast<XFormControllerListener*>(aIter.next())->formDeactivated(aEvt);

    // commit the changes of the grid control (as we're deactivated)
    Reference< XBoundComponent >  xCommitable(getBrowserView()->getGridControl(), UNO_QUERY);
    if (xCommitable.is())
        xCommitable->commit();
    else
        OSL_ENSURE(sal_False, "SbaXDataBrowserController::focusLost : why is my control not commitable ?");
}

// -------------------------------------------------------------------------
void SbaXDataBrowserController::disposingGridControl(const ::com::sun::star::lang::EventObject& /*Source*/)
{
    removeControlListeners(getBrowserView()->getGridControl());
}

// -------------------------------------------------------------------------
void SbaXDataBrowserController::disposingGridModel(const ::com::sun::star::lang::EventObject& /*Source*/)
{
    removeModelListeners(getControlModel());
}

// -------------------------------------------------------------------------
void SbaXDataBrowserController::disposingFormModel(const ::com::sun::star::lang::EventObject& Source)
{
    Reference< XPropertySet >  xSourceSet(Source.Source, UNO_QUERY);
    if (xSourceSet.is())
    {
        xSourceSet->removePropertyChangeListener(PROPERTY_ISNEW, static_cast<XPropertyChangeListener*>(this));
        xSourceSet->removePropertyChangeListener(PROPERTY_ISMODIFIED, static_cast<XPropertyChangeListener*>(this));
        xSourceSet->removePropertyChangeListener(PROPERTY_ROWCOUNT, static_cast<XPropertyChangeListener*>(this));
        xSourceSet->removePropertyChangeListener(PROPERTY_ACTIVECOMMAND, static_cast<XPropertyChangeListener*>(this));
        xSourceSet->removePropertyChangeListener(PROPERTY_ORDER, static_cast<XPropertyChangeListener*>(this));
        xSourceSet->removePropertyChangeListener(PROPERTY_FILTER, static_cast<XPropertyChangeListener*>(this));
        xSourceSet->removePropertyChangeListener(PROPERTY_HAVING_CLAUSE, static_cast<XPropertyChangeListener*>(this));
        xSourceSet->removePropertyChangeListener(PROPERTY_APPLYFILTER, static_cast<XPropertyChangeListener*>(this));
    }

    Reference< ::com::sun::star::sdb::XSQLErrorBroadcaster >  xFormError(Source.Source, UNO_QUERY);
    if (xFormError.is())
        xFormError->removeSQLErrorListener((::com::sun::star::sdb::XSQLErrorListener*)this);

    if (m_xLoadable.is())
        m_xLoadable->removeLoadListener(this);

    Reference< ::com::sun::star::form::XDatabaseParameterBroadcaster >  xFormParameter(Source.Source, UNO_QUERY);
    if (xFormParameter.is())
        xFormParameter->removeParameterListener((::com::sun::star::form::XDatabaseParameterListener*)this);
}

// -------------------------------------------------------------------------
void SbaXDataBrowserController::disposingColumnModel(const ::com::sun::star::lang::EventObject& Source)
{
    RemoveColumnListener(Reference< XPropertySet > (Source.Source, UNO_QUERY));
}

// -------------------------------------------------------------------------
void SbaXDataBrowserController::disposing(const EventObject& Source) throw( RuntimeException )
{
    // if it's a component other than our aggregate, forward it to the aggregate
    if ( m_xFormControllerImpl != Source.Source )
    {
        Reference< XEventListener > xAggListener;
        m_xFormControllerImpl->queryAggregation( ::getCppuType( &xAggListener ) ) >>= xAggListener;
        if ( xAggListener.is( ))
            xAggListener->disposing( Source );
    }

    // is it the grid control ?
    if (getBrowserView())
    {
        Reference< ::com::sun::star::awt::XControl >  xSourceControl(Source.Source, UNO_QUERY);
        if (xSourceControl == getBrowserView()->getGridControl())
            disposingGridControl(Source);
    }

    // it's model (the container of the columns) ?
    if (getControlModel() == Source.Source)
        disposingGridModel(Source);

    // the form's model ?
    if ((getRowSet() == Source.Source))
        disposingFormModel(Source);

    // from a single column model ?
    Reference< XPropertySet >  xSourceSet(Source.Source, UNO_QUERY);
    if (xSourceSet.is())
    {
        Reference< XPropertySetInfo >  xInfo = xSourceSet->getPropertySetInfo();
        // we assume that columns have a Width property and all other sets we are listening to don't have
        if (xInfo->hasPropertyByName(PROPERTY_WIDTH))
            disposingColumnModel(Source);
    }
    SbaXDataBrowserController_Base::OGenericUnoController::disposing( Source );
}

// -----------------------------------------------------------------------
void SAL_CALL SbaXDataBrowserController::setIdentifier( const ::rtl::OUString& _Identifier ) throw (RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    m_sModuleIdentifier = _Identifier;
}

// -----------------------------------------------------------------------
::rtl::OUString SAL_CALL SbaXDataBrowserController::getIdentifier(  ) throw (RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    return m_sModuleIdentifier;
}

// -----------------------------------------------------------------------
void SbaXDataBrowserController::propertyChange(const PropertyChangeEvent& evt) throw ( RuntimeException )
{
    Reference< XPropertySet >  xSource(evt.Source, UNO_QUERY);
    if (!xSource.is())
        return;

    ::vos::OGuard aGuard(Application::GetSolarMutex());
    // the IsModified changed to sal_False ?
    if  (   (evt.PropertyName.equals(PROPERTY_ISMODIFIED))
        &&  (::comphelper::getBOOL(evt.NewValue) == sal_False)
        )
    {   // -> the current field isn't modified anymore, too
        setCurrentModified( sal_False );
    }

    // switching to a new record ?
    if  (   (evt.PropertyName.equals(PROPERTY_ISNEW))
        &&  (::comphelper::getBOOL(evt.NewValue) == sal_True)
        )
    {
        if (::comphelper::getINT32(xSource->getPropertyValue(PROPERTY_ROWCOUNT)) == 0)
            // if we're switching to a new record and didn't have any records before we need to invalidate
            // all slots (as the cursor was invalid before the mode change and so the slots were disabled)
            InvalidateAll();
    }

    // the filter or the sort criterias have changed ? -> update our parser
    if (m_xParser.is())
    {
        if (evt.PropertyName.equals(PROPERTY_ACTIVECOMMAND))
        {
            DO_SAFE( m_xParser->setElementaryQuery(::comphelper::getString(evt.NewValue)), "SbaXDataBrowserController::propertyChange : could not forward the new query to my parser !" );
        }
        else if (evt.PropertyName.equals(PROPERTY_FILTER))
        {
            if (m_xParser->getFilter() != ::comphelper::getString(evt.NewValue))
            {
                DO_SAFE( m_xParser->setFilter(::comphelper::getString(evt.NewValue)), "SbaXDataBrowserController::propertyChange : could not forward the new filter to my parser !" );
            }
            InvalidateFeature(ID_BROWSER_REMOVEFILTER);
        }
        else if (evt.PropertyName.equals(PROPERTY_HAVING_CLAUSE))
        {
            if (m_xParser->getHavingClause() != ::comphelper::getString(evt.NewValue))
            {
                DO_SAFE( m_xParser->setHavingClause(::comphelper::getString(evt.NewValue)), "SbaXDataBrowserController::propertyChange : could not forward the new filter to my parser !" );
            }
            InvalidateFeature(ID_BROWSER_REMOVEFILTER);
        }
        else if (evt.PropertyName.equals(PROPERTY_ORDER))
        {
            if (m_xParser->getOrder() != ::comphelper::getString(evt.NewValue))
            {
                DO_SAFE( m_xParser->setOrder(::comphelper::getString(evt.NewValue)), "SbaXDataBrowserController::propertyChange : could not forward the new order to my parser !" );
            }
            InvalidateFeature(ID_BROWSER_REMOVEFILTER);
        }
    }

    // a new record count ? -> may be our search availability has changed
    if (evt.PropertyName.equals(PROPERTY_ROWCOUNT))
    {
        sal_Int32 nNewValue = 0, nOldValue = 0;
        evt.NewValue >>= nNewValue;
        evt.OldValue >>= nOldValue;
        if((nOldValue == 0 && nNewValue != 0) || (nOldValue != 0 && nNewValue == 0))
            InvalidateAll();
    }
}

//------------------------------------------------------------------------
void SbaXDataBrowserController::modified(const ::com::sun::star::lang::EventObject& /*aEvent*/) throw( RuntimeException )
{
    setCurrentModified( sal_True );
}

// -----------------------------------------------------------------------
void SbaXDataBrowserController::elementInserted(const ::com::sun::star::container::ContainerEvent& evt) throw( RuntimeException )
{
    DBG_ASSERT(Reference< XInterface >(evt.Source, UNO_QUERY).get() == Reference< XInterface >(getControlModel(), UNO_QUERY).get(),
        "SbaXDataBrowserController::elementInserted: where did this come from (not from the grid model)?!");
    Reference< XPropertySet >  xNewColumn(evt.Element,UNO_QUERY);
    if ( xNewColumn.is() )
        AddColumnListener(xNewColumn);
}

// -----------------------------------------------------------------------
void SbaXDataBrowserController::elementRemoved(const ::com::sun::star::container::ContainerEvent& evt) throw( RuntimeException )
{
    DBG_ASSERT(Reference< XInterface >(evt.Source, UNO_QUERY).get() == Reference< XInterface >(getControlModel(), UNO_QUERY).get(),
        "SbaXDataBrowserController::elementRemoved: where did this come from (not from the grid model)?!");
    Reference< XPropertySet >  xOldColumn(evt.Element,UNO_QUERY);
    if ( xOldColumn.is() )
        RemoveColumnListener(xOldColumn);
}

// -----------------------------------------------------------------------
void SbaXDataBrowserController::elementReplaced(const ::com::sun::star::container::ContainerEvent& evt) throw( RuntimeException )
{
    DBG_ASSERT(Reference< XInterface >(evt.Source, UNO_QUERY).get() == Reference< XInterface >(getControlModel(), UNO_QUERY).get(),
        "SbaXDataBrowserController::elementReplaced: where did this come from (not from the grid model)?!");
    Reference< XPropertySet >  xOldColumn(evt.ReplacedElement,UNO_QUERY);
    if ( xOldColumn.is() )
        RemoveColumnListener(xOldColumn);

    Reference< XPropertySet >  xNewColumn(evt.Element,UNO_QUERY);
    if ( xNewColumn.is() )
        AddColumnListener(xNewColumn);
}

// -----------------------------------------------------------------------
sal_Bool SbaXDataBrowserController::suspend(sal_Bool /*bSuspend*/) throw( RuntimeException )
{
    // have a pending open operation ?
    if (PendingLoad())
    {
        ::vos::OGuard aGuard(Application::GetSolarMutex());
        if (m_nPendingLoadFinished != 0)
        {   // clean up directly. Otherwise there may be a pending asynchronous call
            // to OnOpenFinishedMainThread, which won't be executed before we leave
            // this method. Sounds like a classic infinite loop.
            Application::RemoveUserEvent(m_nPendingLoadFinished);
            LINK(this, SbaXDataBrowserController, OnOpenFinishedMainThread).Call(NULL);
        }
        else
        {   // set m_bClosingKillOpen to ensure that the our termination handler reacts according
            // it's context
            m_bClosingKillOpen = sal_True;

            // normally we would now just wait for termination of the load thread, but there is a small problem :
            // In the current thread the global solar mutex is locked (that's for sure). If the loading of the
            // form tries to acquire (blocking) the solar mutex, too, and we loop waiting for the other thread
            // we have a classic deadlock. And bet your ass that ANYBODY in the foreign thread tries to lock
            // the solar mutex. Almost all the UNO-capsules around normal C++ classes use the solar mutex for
            // "thread safety" (which doesn't deserve that name anymore ;), e.g. the XNumberFormatter-implementation
            // does.
            // So we have to do a fake : we tell the loading thread that we aren't interested in the results anymore
            // and the thread deletes itself (and the data source) as soon as it is done. As it holds the last
            // references to the form (and thus, indirectly, to the grid) they will be cleared as soon as the thread dies.
            // So all is fine. Except the small flaw that the form is still loading in the background while the
            // window that should display it is already dead.
            // If we could release the solar mutex in this thread and block it 'til the loader is finished we could
            // solve it in a less dirty way, but uinfortunatelly we don't know how often this thread acquired the mutex.
            // With high effort we could reach this with releasing the mutex until a third thread - which has to be
            // created - can acquire it.Then we block, the third thread releases the mutex (and dies) and the loader
            // thread would be able to finish. But that sounds difficult and fault-prone, so I think it's not worth it ...
            ((LoadFormThread*)m_pLoadThread)->SetTerminationHdl(Link());
            // and of course we tell the thread to stop ....
            ((LoadFormThread*)m_pLoadThread)->StopIt();
        }

    }
    DBG_ASSERT(m_nPendingLoadFinished == 0, "SbaXDataBrowserController::suspend : there shouldn't be a pending load !");

    m_aAsyncGetCellFocus.CancelCall();
    m_aAsyncInvalidateAll.CancelCall();

    sal_Bool bSuccess = SaveModified();
    return bSuccess;
}
// -----------------------------------------------------------------------
void SbaXDataBrowserController::disposing()
{
    // and dispose the aggregate
    if (m_xFormControllerImpl.is())
    {
        Reference< XComponent >  xAggComp;
        m_xFormControllerImpl->queryAggregation(::getCppuType(&xAggComp)) >>= xAggComp;
        if (xAggComp.is())
            xAggComp->dispose();
    }

    // the base class
    SbaXDataBrowserController_Base::OGenericUnoController::disposing();

    if (!PendingLoad())
    {
        // don't do the removeXxxListener calls if there is a pending load, this may lead to a deadlock :
        // as in this thread the SolarMutex is locked (that's for sure) and removeXxxListener locks
        // the form's mutex. But in the loading thread both mutexes are acquired in reverse order.
        // That's no problem that we don't remove ourself here, as the load thread is responsible for the form
        // at the moment. So if the loading is finished, the form will be disposed (by the load thread), and
        // we get the "disposing" event where we can do the removeXxxListener calls.
        // The alternative for this handling would be that the form has two mutexes : one for handling it's
        // listeners and properties and so on, on for it's pure cursor actions

        // the data source
        Reference< XPropertySet >  xFormSet(getRowSet(), UNO_QUERY);
        if (xFormSet.is())
        {
            xFormSet->removePropertyChangeListener(PROPERTY_ISNEW, static_cast<XPropertyChangeListener*>(this));
            xFormSet->removePropertyChangeListener(PROPERTY_ISMODIFIED, static_cast<XPropertyChangeListener*>(this));
            xFormSet->removePropertyChangeListener(PROPERTY_ROWCOUNT, static_cast<XPropertyChangeListener*>(this));
            xFormSet->removePropertyChangeListener(PROPERTY_ACTIVECOMMAND, static_cast<XPropertyChangeListener*>(this));
            xFormSet->removePropertyChangeListener(PROPERTY_ORDER, static_cast<XPropertyChangeListener*>(this));
            xFormSet->removePropertyChangeListener(PROPERTY_FILTER, static_cast<XPropertyChangeListener*>(this));
            xFormSet->removePropertyChangeListener(PROPERTY_HAVING_CLAUSE, static_cast<XPropertyChangeListener*>(this));
            xFormSet->removePropertyChangeListener(PROPERTY_APPLYFILTER, static_cast<XPropertyChangeListener*>(this));
        }

        Reference< ::com::sun::star::sdb::XSQLErrorBroadcaster >  xFormError(getRowSet(), UNO_QUERY);
        if (xFormError.is())
            xFormError->removeSQLErrorListener((::com::sun::star::sdb::XSQLErrorListener*)this);

        if (m_xLoadable.is())
            m_xLoadable->removeLoadListener(this);

        Reference< ::com::sun::star::form::XDatabaseParameterBroadcaster >  xFormParameter(getRowSet(), UNO_QUERY);
        if (xFormParameter.is())
            xFormParameter->removeParameterListener((::com::sun::star::form::XDatabaseParameterListener*)this);
    }

    removeModelListeners(getControlModel());

    if ( getView() && m_pClipbordNotifier  )
    {
        m_pClipbordNotifier->ClearCallbackLink();
        m_pClipbordNotifier->AddRemoveListener( getView(), sal_False );
        m_pClipbordNotifier->release();
        m_pClipbordNotifier = NULL;
    }

    if (getBrowserView())
    {
        removeControlListeners(getBrowserView()->getGridControl());
        // don't delete explicitly, this is done by the owner (and user) of this controller (me hopes ...)
        m_pView = NULL;
    }

    if(m_aInvalidateClipboard.IsActive())
        m_aInvalidateClipboard.Stop();

    // dispose the data source
    // if there is a pending load we decided to give the responsibility for the data source to the open thread
    // (see ::suspend)
    if (!PendingLoad())
    {
        try
        {
            ::comphelper::disposeComponent(m_xRowSet);

            m_xRowSet           = NULL;
            m_xColumnsSupplier  = NULL;
            m_xLoadable         = NULL;
        }
        catch(Exception&)
        {
            OSL_ENSURE(0,"Exception thrown by dispose");
        }
    }
    try
    {
        ::comphelper::disposeComponent(m_xParser);
    }
    catch(Exception&)
    {
        OSL_ENSURE(0,"Exception thrown by dispose");
    }
}
//------------------------------------------------------------------------------
void SbaXDataBrowserController::frameAction(const ::com::sun::star::frame::FrameActionEvent& aEvent) throw( RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    SbaXDataBrowserController_Base::frameAction( aEvent );

    if ( aEvent.Source == getFrame() )
        switch ( aEvent.Action )
        {
            case FrameAction_FRAME_ACTIVATED:
            case FrameAction_FRAME_UI_ACTIVATED:
                // ensure that the active cell (if any) has the focus
                m_aAsyncGetCellFocus.Call();
                // start the clipboard timer
                if (getBrowserView() && getBrowserView()->getVclControl() && !m_aInvalidateClipboard.IsActive())
                {
                    m_aInvalidateClipboard.Start();
                    OnInvalidateClipboard( NULL );
                }
                break;
            case FrameAction_FRAME_DEACTIVATING:
            case FrameAction_FRAME_UI_DEACTIVATING:
                // stop the clipboard invalidator
                if (getBrowserView() && getBrowserView()->getVclControl() && m_aInvalidateClipboard.IsActive())
                {
                    m_aInvalidateClipboard.Stop();
                    OnInvalidateClipboard( NULL );
                }
                // remove the "get cell focus"-event
                m_aAsyncGetCellFocus.CancelCall();
                break;
            default:
                break;
        }
}

//------------------------------------------------------------------------------
void SbaXDataBrowserController::errorOccured(const ::com::sun::star::sdb::SQLErrorEvent& aEvent) throw( RuntimeException )
{
    SQLExceptionInfo aInfo(aEvent.Reason);
    if ( aInfo.isValid() )
    {
        ::vos::OGuard aGuard(Application::GetSolarMutex());
        OSQLMessageBox aDlg(getBrowserView(), aInfo);
        aDlg.Execute();
    }
    if (m_nFormActionNestingLevel)
        m_bErrorOccured = true;
}

//------------------------------------------------------------------------------
sal_Bool SbaXDataBrowserController::approveParameter(const ::com::sun::star::form::DatabaseParameterEvent& aEvent) throw( RuntimeException )
{
    if (aEvent.Source != getRowSet())
    {
        // not my data source -> allow anything
        DBG_ERROR("SbaXDataBrowserController::approveParameter : invalid event source !");
        return sal_True;
    }

    Reference< ::com::sun::star::container::XIndexAccess >  xParameters = aEvent.Parameters;
    ::vos::OGuard aSolarGuard(Application::GetSolarMutex());
        // this may be executed in a non-main thread and we want to use vcl ...
    Window* pParent = Application::GetDefDialogParent();
        // don't use the content as parent if it isn't visible
    // (and don't use NULL as parent : this may be displayed in the beamer and really shouldn't be task-local)
        // 69297 - FS - 25.10.99
    if (getBrowserView() && getBrowserView()->IsVisible())
        pParent = getBrowserView();

    // default handling: instantiate an interaction handler and let it handle the parameter request
    try
    {
        // two continuations allowed: OK and Cancel
        OParameterContinuation* pParamValues = new OParameterContinuation;
        OInteractionAbort* pAbort = new OInteractionAbort;
        // the request
        ParametersRequest aRequest;
        aRequest.Parameters = xParameters;
        aRequest.Connection = getConnection(Reference< XRowSet >(aEvent.Source, UNO_QUERY));
        OInteractionRequest* pParamRequest = new OInteractionRequest(makeAny(aRequest));
        Reference< XInteractionRequest > xParamRequest(pParamRequest);
        // some knittings
        pParamRequest->addContinuation(pParamValues);
        pParamRequest->addContinuation(pAbort);

        // create the handler, let it handle the request
        Reference< XInteractionHandler > xHandler(getORB()->createInstance(SERVICE_SDB_INTERACTION_HANDLER), UNO_QUERY);
        if (xHandler.is())
            xHandler->handle(xParamRequest);

        if (!pParamValues->wasSelected())
        {   // canceled
            setLoadingCancelled();
            return sal_False;
        }

        // transfer the values into the parameter supplier
        Sequence< PropertyValue > aFinalValues = pParamValues->getValues();
        if (aFinalValues.getLength() != aRequest.Parameters->getCount())
        {
            DBG_ERROR("SbaXDataBrowserController::approveParameter: the InteractionHandler returned nonsense!");
            setLoadingCancelled();
            return sal_False;
        }
        const PropertyValue* pFinalValues = aFinalValues.getConstArray();
        for (sal_Int32 i=0; i<aFinalValues.getLength(); ++i, ++pFinalValues)
        {
            Reference< XPropertySet > xParam;
            ::cppu::extractInterface(xParam, aRequest.Parameters->getByIndex(i));
            DBG_ASSERT(xParam.is(), "SbaXDataBrowserController::approveParameter: one of the parameters is no property set!");
            if (xParam.is())
            {
#ifdef DBG_UTIL
                ::rtl::OUString sName;
                xParam->getPropertyValue(PROPERTY_NAME) >>= sName;
                DBG_ASSERT(sName.equals(pFinalValues->Name), "SbaXDataBrowserController::approveParameter: suspicious value names!");
#endif
                try { xParam->setPropertyValue(PROPERTY_VALUE, pFinalValues->Value); }
                catch(Exception&)
                {
                    DBG_ERROR("SbaXDataBrowserController::approveParameter: setting one of the properties failed!");
                }
            }
        }
    }
    catch(Exception&)
    {
        DBG_ERROR("SbaXDataBrowserController::approveParameter: caught an Exception (tried to let the InteractionHandler handle it)!");
    }

    return sal_True;
}


//------------------------------------------------------------------------------
sal_Bool SbaXDataBrowserController::approveReset(const ::com::sun::star::lang::EventObject& /*rEvent*/) throw( RuntimeException )
{
    return sal_True;
}

//------------------------------------------------------------------------------
void SbaXDataBrowserController::resetted(const ::com::sun::star::lang::EventObject& rEvent) throw( RuntimeException )
{
    DBG_ASSERT(rEvent.Source == getControlModel(), "SbaXDataBrowserController::resetted : where did this come from ?");
    (void)rEvent;
    setCurrentModified( sal_False );
}

//------------------------------------------------------------------------------
sal_Bool SbaXDataBrowserController::confirmDelete(const ::com::sun::star::sdb::RowChangeEvent& /*aEvent*/) throw( RuntimeException )
{
    if (QueryBox(getBrowserView(), ModuleRes(QUERY_BRW_DELETE_ROWS)).Execute() != RET_YES)
        return sal_False;

    return sal_True;
}
//------------------------------------------------------------------------------
FeatureState SbaXDataBrowserController::GetState(sal_uInt16 nId) const
{
    FeatureState aReturn;
        // (disabled automatically)

    try
    {
        // no chance without a view
        if (!getBrowserView() || !getBrowserView()->getVclControl())
            return aReturn;

        switch (nId)
        {
            case ID_BROWSER_REMOVEFILTER:
                if (!m_xParser.is())
                    break;
                // any filter or sort order set ?
                aReturn.bEnabled = m_xParser->getFilter().getLength() || m_xParser->getHavingClause().getLength() || m_xParser->getOrder().getLength();
                return aReturn;
        }
        // no chance while loading the form
        if (PendingLoad())
            return aReturn;
        // no chance without valid models
        if (isValid() && !isValidCursor())
            return aReturn;

        switch (nId)
        {
            case ID_BROWSER_SEARCH:
            {
                Reference< XPropertySet >  xFormSet(getRowSet(), UNO_QUERY);
                sal_Int32 nCount = ::comphelper::getINT32(xFormSet->getPropertyValue(PROPERTY_ROWCOUNT));
                aReturn.bEnabled = nCount != 0;
            }
            break;
            case ID_BROWSER_INSERT_ROW:
                {
                    // check if it is available
                    Reference< XPropertySet >  xDataSourceSet(getRowSet(), UNO_QUERY);
                    if (!xDataSourceSet.is())
                        break;  // no datasource -> no edit mode

                    sal_Int32 nDataSourcePrivileges = ::comphelper::getINT32(xDataSourceSet->getPropertyValue(PROPERTY_PRIVILEGES));
                    aReturn.bEnabled = ((nDataSourcePrivileges & ::com::sun::star::sdbcx::Privilege::INSERT) != 0) && ::comphelper::getBOOL(xDataSourceSet->getPropertyValue(::rtl::OUString::createFromAscii("AllowInserts")));
                }
                break;
            case SID_FM_DELETEROWS:
                {
                    Reference< XPropertySet >  xFormSet(getRowSet(), UNO_QUERY);
                    sal_Int32 nCount = ::comphelper::getINT32(xFormSet->getPropertyValue(PROPERTY_ROWCOUNT));
                    sal_Bool bNew = sal_False;
                    xFormSet->getPropertyValue(PROPERTY_ISNEW) >>= bNew;
                    aReturn.bEnabled = nCount != 0 && !bNew;
                }
                break;

            case ID_BROWSER_PASTE:
            case ID_BROWSER_COPY:
            case ID_BROWSER_CUT:
            {
                CellControllerRef xCurrentController = getBrowserView()->getVclControl()->Controller();
                if (xCurrentController.Is() && xCurrentController->ISA(EditCellController))
                {
                    Edit& rEdit = (Edit&)xCurrentController->GetWindow();
                    sal_Bool bHasLen = (rEdit.GetSelection().Len() != 0);
                    sal_Bool bIsReadOnly = rEdit.IsReadOnly();
                    switch (nId)
                    {
                        case ID_BROWSER_CUT:    aReturn.bEnabled = m_aCurrentFrame.isActive() && bHasLen && !bIsReadOnly; break;
                        case SID_COPY   :       aReturn.bEnabled = m_aCurrentFrame.isActive() && bHasLen; break;
                        case ID_BROWSER_PASTE:
                            aReturn.bEnabled = m_aCurrentFrame.isActive() && !bIsReadOnly;
                            if(aReturn.bEnabled)
                            {
                                aReturn.bEnabled = aReturn.bEnabled && IsFormatSupported( m_aSystemClipboard.GetDataFlavorExVector(), FORMAT_STRING );
                            }
                            break;
                    }
                }
            }
            break;

            case ID_BROWSER_SORTUP:
            case ID_BROWSER_SORTDOWN:
            case ID_BROWSER_AUTOFILTER:
            {
                // a native statement can't be filtered or sorted
                if (!m_xParser.is())
                    break;

                Reference< XPropertySet >  xCurrentField = getBoundField();
                if (!xCurrentField.is())
                    break;

                aReturn.bEnabled = ::comphelper::getBOOL(xCurrentField->getPropertyValue(PROPERTY_ISSEARCHABLE));
                Reference< XRowSet > xRow = getRowSet();
                Reference< XPropertySet >  xFormSet(getRowSet(), UNO_QUERY);
                if(aReturn.bEnabled && xRow.is()) // check if we stand on a valid row
                    aReturn.bEnabled = !(xRow->isBeforeFirst() || xRow->isAfterLast() || xRow->rowDeleted() || ::comphelper::getINT32(xFormSet->getPropertyValue(PROPERTY_ROWCOUNT)) == 0);
            }
            break;

            case ID_BROWSER_ORDERCRIT:
            case ID_BROWSER_FILTERCRIT:
                if (!m_xParser.is())
                    break;
                {
                    Reference< XPropertySet >  xCurrentField = getBoundField();
                    // we are not in the handle column
                    aReturn.bEnabled = getBrowserView()->getVclControl()->GetCurColumnId() != 0 &&
                                        xCurrentField.is() &&
                                       ::comphelper::getBOOL(xCurrentField->getPropertyValue(PROPERTY_ISSEARCHABLE));

                    Reference< XRowSet > xRow = getRowSet();
                    Reference< XPropertySet >  xFormSet(getRowSet(), UNO_QUERY);
                    if(aReturn.bEnabled && xRow.is()) // check if we stand on a valid row
                        aReturn.bEnabled = !(xRow->isBeforeFirst() || xRow->isAfterLast() || xRow->rowDeleted() || ::comphelper::getINT32(xFormSet->getPropertyValue(PROPERTY_ROWCOUNT)) == 0);
                    // a native statement can't be filtered or sorted
                    //  aReturn.bEnabled &= m_xParser.is();
                }
                break;

            case ID_BROWSER_REFRESH:
                //  aReturn.bEnabled = isValidCursor();
                aReturn.bEnabled = sal_True;
                break;

            case ID_BROWSER_REDO:
                aReturn.bEnabled = sal_False;   // simply forget it ;). no redo possible.
                break;

            case ID_BROWSER_UNDORECORD:
            case ID_BROWSER_SAVERECORD:
            {
                if (!m_bCurrentlyModified)
                {
                    Reference< XPropertySet >  xFormSet(getRowSet(), UNO_QUERY);
                    if (xFormSet.is())
                        aReturn.bEnabled = ::comphelper::getBOOL(xFormSet->getPropertyValue(PROPERTY_ISMODIFIED));
                }
                else
                    aReturn.bEnabled = sal_True;

                aReturn.sTitle = (ID_BROWSER_UNDORECORD == nId) ? m_sStateUndoRecord : m_sStateSaveRecord;
            }
            break;
            case ID_BROWSER_EDITDOC:
            {
                // check if it is available
                Reference< XPropertySet >  xDataSourceSet(getRowSet(), UNO_QUERY);
                if (!xDataSourceSet.is())
                    break;  // no datasource -> no edit mode

                sal_Int32 nDataSourcePrivileges = ::comphelper::getINT32(xDataSourceSet->getPropertyValue(PROPERTY_PRIVILEGES));
                sal_Bool bInsertAllowedAndPossible = ((nDataSourcePrivileges & ::com::sun::star::sdbcx::Privilege::INSERT) != 0) && ::comphelper::getBOOL(xDataSourceSet->getPropertyValue(::rtl::OUString::createFromAscii("AllowInserts")));
                sal_Bool bUpdateAllowedAndPossible = ((nDataSourcePrivileges & ::com::sun::star::sdbcx::Privilege::UPDATE) != 0) && ::comphelper::getBOOL(xDataSourceSet->getPropertyValue(::rtl::OUString::createFromAscii("AllowUpdates")));
                sal_Bool bDeleteAllowedAndPossible = ((nDataSourcePrivileges & ::com::sun::star::sdbcx::Privilege::DELETE) != 0) && ::comphelper::getBOOL(xDataSourceSet->getPropertyValue(::rtl::OUString::createFromAscii("AllowDeletes")));
                if (!bInsertAllowedAndPossible && !bUpdateAllowedAndPossible && !bDeleteAllowedAndPossible)
                    break;  // no insert/update/delete -> no edit mode

                if (!isValidCursor() || !isLoaded())
                    break;  // no cursor -> no edit mode

                aReturn.bEnabled = sal_True;

                sal_Int16 nGridMode = getBrowserView()->getVclControl()->GetOptions();
                aReturn.bChecked = nGridMode > DbGridControl::OPT_READONLY;
            }
            break;
            case ID_BROWSER_FILTERED:
            {
                aReturn.bEnabled = sal_False;
                Reference< XPropertySet >  xActiveSet(getRowSet(), UNO_QUERY);
                ::rtl::OUString aFilter = ::comphelper::getString(xActiveSet->getPropertyValue(PROPERTY_FILTER));
                ::rtl::OUString aHaving = ::comphelper::getString(xActiveSet->getPropertyValue(PROPERTY_HAVING_CLAUSE));
                if ( aFilter.getLength() || aHaving.getLength() )
                {
                    xActiveSet->getPropertyValue( PROPERTY_APPLYFILTER ) >>= aReturn.bChecked;
                    aReturn.bEnabled = sal_True;
                }
                else
                {
                    aReturn.bChecked = sal_False;
                    aReturn.bEnabled = sal_False;
                }
            }
            break;
            default:
                return SbaXDataBrowserController_Base::GetState(nId);
        }
    }
    catch(const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    return aReturn;
}

//------------------------------------------------------------------------------
void SbaXDataBrowserController::applyParserOrder(const ::rtl::OUString& _rOldOrder)
{
    Reference< XPropertySet > xFormSet(getRowSet(), UNO_QUERY);
    if (!m_xLoadable.is())
    {
        OSL_ENSURE(sal_False, "SbaXDataBrowserController::applyParserOrder: invalid row set!");
        return;
    }

    sal_uInt16 nPos = getCurrentColumnPosition();
    sal_Bool bSuccess = sal_False;
    try
    {
        xFormSet->setPropertyValue(PROPERTY_ORDER, makeAny(m_xParser->getOrder()));
        bSuccess = reloadForm(m_xLoadable);
    }
    catch(Exception&)
    {
    }

    if (!bSuccess)
    {
        xFormSet->setPropertyValue(PROPERTY_ORDER, makeAny(_rOldOrder));
        DO_SAFE( m_xParser->setOrder(_rOldOrder), "SbaXDataBrowserController::applyParserOrder: could not restore the old order of my parser !" );

        try
        {
            if (loadingCancelled() || !reloadForm(m_xLoadable))
                criticalFail();
        }
        catch(Exception&)
        {
            criticalFail();
        }
        InvalidateAll();
    }
    InvalidateFeature(ID_BROWSER_REMOVEFILTER);

    setCurrentColumnPosition(nPos);
}

//------------------------------------------------------------------------------
void SbaXDataBrowserController::applyParserFilter(const ::rtl::OUString& _rOldFilter, sal_Bool _bOldFilterApplied,const ::rtl::OUString& _sOldHaving)
{
    Reference< XPropertySet >  xFormSet(getRowSet(), UNO_QUERY);
    if (!m_xLoadable.is())
    {
        OSL_ENSURE(sal_False, "SbaXDataBrowserController::applyParserFilter: invalid row set!");
        return;
    }

    sal_uInt16 nPos = getCurrentColumnPosition();

    sal_Bool bSuccess = sal_False;
    try
    {
        FormErrorHelper aError(this);
        xFormSet->setPropertyValue(PROPERTY_FILTER, makeAny(m_xParser->getFilter()));
        xFormSet->setPropertyValue(PROPERTY_HAVING_CLAUSE, makeAny(m_xParser->getHavingClause()));
        xFormSet->setPropertyValue(PROPERTY_APPLYFILTER, ::comphelper::makeBoolAny(sal_Bool(sal_True)));

        bSuccess = reloadForm(m_xLoadable);
    }
    catch(Exception&)
    {
    }

    if (!bSuccess)
    {
        xFormSet->setPropertyValue(PROPERTY_FILTER, makeAny(_rOldFilter));
        xFormSet->setPropertyValue(PROPERTY_HAVING_CLAUSE, makeAny(_sOldHaving));
        xFormSet->setPropertyValue(PROPERTY_APPLYFILTER, ::comphelper::makeBoolAny(_bOldFilterApplied));

        try
        {
            if (loadingCancelled() || !reloadForm(m_xLoadable))
                criticalFail();
        }
        catch(Exception&)
        {
            criticalFail();
        }
        InvalidateAll();
    }
    InvalidateFeature(ID_BROWSER_REMOVEFILTER);

    setCurrentColumnPosition(nPos);
}

//------------------------------------------------------------------------------
void SbaXDataBrowserController::ExecuteFilterSortCrit(sal_Bool bFilter)
{
    if (!SaveModified())
        return;

    Reference< XPropertySet >  xFormSet(getRowSet(), UNO_QUERY);

    // no condition for searching
    if (getRowSet()->isBeforeFirst() || getRowSet()->isAfterLast() ||
        getRowSet()->rowDeleted())
        return;

    ::rtl::OUString sOldVal = bFilter ? m_xParser->getFilter() : m_xParser->getOrder();
    ::rtl::OUString sOldHaving = m_xParser->getHavingClause();
    try
    {
        Reference< ::com::sun::star::sdbcx::XColumnsSupplier> xSup = getColumnsSupplier();
        Reference< XConnection> xCon(xFormSet->getPropertyValue(PROPERTY_ACTIVE_CONNECTION),UNO_QUERY);
        if(bFilter)
        {
            DlgFilterCrit aDlg( getBrowserView(), getORB(), xCon, m_xParser, xSup->getColumns() );
            String aFilter;
            if(!aDlg.Execute())
            {
                m_xParser->setFilter(sOldVal);
                m_xParser->setHavingClause(sOldHaving);
                return; // if so we don't need to actualize the grid
            }
            aDlg.BuildWherePart();
        }
        else
        {
            DlgOrderCrit aDlg( getBrowserView(),xCon,m_xParser,xSup->getColumns() );
            String aOrder;
            if(!aDlg.Execute())
            {
                m_xParser->setOrder(sOldVal);
                return; // if so we don't need to actualize the grid
            }
            aDlg.BuildOrderPart();
        }
    }
    catch(SQLException& e) { showError(SQLExceptionInfo(e)); return; }
    catch(Exception&)
    {
        return;
    }

    ::rtl::OUString sNewVal = bFilter ? m_xParser->getFilter() : m_xParser->getOrder();
    sal_Bool bOldFilterApplied(sal_False);
    if (bFilter)
    {
        try { bOldFilterApplied = ::comphelper::getBOOL(xFormSet->getPropertyValue(PROPERTY_APPLYFILTER)); } catch(Exception&) { } ;
    }

    ::rtl::OUString sNewHaving = m_xParser->getHavingClause();
    if ( sOldVal.equals(sNewVal) && (!bFilter || sOldHaving.equals(sNewHaving)) )
        // nothing to be done
        return;

    if (bFilter)
        applyParserFilter(sOldVal, bOldFilterApplied,sOldHaving);
    else
        applyParserOrder(sOldVal);

}

//------------------------------------------------------------------------------
void SbaXDataBrowserController::ExecuteSearch()
{
    // calculate the control source of the active field
    Reference< ::com::sun::star::form::XGrid >  xGrid(getBrowserView()->getGridControl(), UNO_QUERY);
    DBG_ASSERT(xGrid.is(), "SbaXDataBrowserController::ExecuteSearch : the control should have an ::com::sun::star::form::XGrid interface !");

    Reference< ::com::sun::star::form::XGridPeer >  xGridPeer(getBrowserView()->getGridControl()->getPeer(), UNO_QUERY);
    Reference< ::com::sun::star::container::XIndexContainer >  xColumns = xGridPeer->getColumns();
    DBG_ASSERT(xGridPeer.is() && xColumns.is(), "SbaXDataBrowserController::ExecuteSearch : invalid peer !");

    sal_Int16 nViewCol = xGrid->getCurrentColumnPosition();
    sal_Int16 nModelCol = getBrowserView()->View2ModelPos(nViewCol);

    Reference< XPropertySet >  xCurrentCol(xColumns->getByIndex(nModelCol),UNO_QUERY);
    String sActiveField = ::comphelper::getString(xCurrentCol->getPropertyValue(PROPERTY_CONTROLSOURCE));

    // the text within the current cell
    String sInitialText;
    Reference< ::com::sun::star::container::XIndexAccess >  xColControls(xGridPeer, UNO_QUERY);
    Reference< XInterface >  xCurControl(xColControls->getByIndex(nViewCol),UNO_QUERY);
    ::rtl::OUString aInitialText;
    if (IsSearchableControl(xCurControl, &aInitialText))
        sInitialText = (const sal_Unicode*)aInitialText;

    // prohibit the synchronization of the grid's display with the cursor's position
    Reference< XPropertySet >  xModelSet(getControlModel(), UNO_QUERY);
    DBG_ASSERT(xModelSet.is(), "SbaXDataBrowserController::ExecuteSearch : no model set ?!");
    xModelSet->setPropertyValue(::rtl::OUString::createFromAscii("DisplayIsSynchron"), ::comphelper::makeBoolAny(sal_Bool(sal_False)));
    xModelSet->setPropertyValue(::rtl::OUString::createFromAscii("AlwaysShowCursor"), ::comphelper::makeBoolAny(sal_Bool(sal_True)));
    xModelSet->setPropertyValue(::rtl::OUString::createFromAscii("CursorColor"), makeAny(sal_Int32(COL_LIGHTRED)));

    Reference< ::com::sun::star::util::XNumberFormatsSupplier >  xNFS(::dbtools::getNumberFormats(::dbtools::getConnection(m_xRowSet), sal_True,getORB()));

    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    AbstractFmSearchDialog* pDialog = NULL;
    if ( pFact )
    {
        ::std::vector< String > aContextNames;
        aContextNames.push_back( String::CreateFromAscii("Standard") );
        pDialog = pFact->CreateFmSearchDialog(getBrowserView(), sInitialText, aContextNames, 0, LINK(this, SbaXDataBrowserController, OnSearchContextRequest));
    }
    DBG_ASSERT( pDialog, "SbaXDataBrowserController::ExecuteSearch: could not get the search dialog!" );
    if ( pDialog )
    {
        pDialog->SetActiveField( sActiveField );
        pDialog->SetFoundHandler( LINK( this, SbaXDataBrowserController, OnFoundData ) );
        pDialog->SetCanceledNotFoundHdl( LINK( this, SbaXDataBrowserController, OnCanceledNotFound ) );
        pDialog->Execute();
        delete pDialog;
    }

    // restore the grid's normal operating state
    xModelSet->setPropertyValue(::rtl::OUString::createFromAscii("DisplayIsSynchron"), ::comphelper::makeBoolAny(sal_Bool(sal_True)));
    xModelSet->setPropertyValue(::rtl::OUString::createFromAscii("AlwaysShowCursor"), ::comphelper::makeBoolAny(sal_Bool(sal_False)));
    xModelSet->setPropertyValue(::rtl::OUString::createFromAscii("CursorColor"), Any());
}

//------------------------------------------------------------------------------
void SbaXDataBrowserController::Execute(sal_uInt16 nId, const Sequence< PropertyValue >& /*aArgs*/)
{
    sal_Bool bSortUp = sal_True;

    switch (nId)
    {
        case ID_BROWSER_INSERT_ROW:
            try
            {
                if (SaveModified())
                {
                    getRowSet()->afterLast();
                    // check if it is available
                    Reference< XResultSetUpdate >  xUpdateCursor(getRowSet(), UNO_QUERY_THROW);
                    xUpdateCursor->moveToInsertRow();
                }
            }
            catch(Exception&)
            {
                OSL_ENSURE(0,"Exception caught!");
            }
            break;
        case SID_FM_DELETEROWS:

            if (SaveModified())
            {
                SbaGridControl* pVclGrid = getBrowserView()->getVclControl();
                if ( pVclGrid )
                {
                    if( !pVclGrid->GetSelectRowCount() )
                    {
                        pVclGrid->DeactivateCell();
                        pVclGrid->SelectRow(pVclGrid->GetCurRow());
                    }
                    pVclGrid->DeleteSelectedRows();
                }
            }
            break;

        case ID_BROWSER_FILTERED:
            if (SaveModified())
            {
                Reference< XPropertySet >  xActiveSet(getRowSet(), UNO_QUERY);
                sal_Bool bApplied = ::comphelper::getBOOL(xActiveSet->getPropertyValue(PROPERTY_APPLYFILTER));
                xActiveSet->setPropertyValue(PROPERTY_APPLYFILTER, ::comphelper::makeBoolAny(sal_Bool(!bApplied)));
                reloadForm(m_xLoadable);
            }
            InvalidateFeature(ID_BROWSER_FILTERED);
            break;
        case ID_BROWSER_EDITDOC:
        {
            sal_Int16 nGridMode = getBrowserView()->getVclControl()->GetOptions();
            if (nGridMode == DbGridControl::OPT_READONLY)
                getBrowserView()->getVclControl()->SetOptions(DbGridControl::OPT_UPDATE | DbGridControl::OPT_INSERT | DbGridControl::OPT_DELETE);
                    // the options not supported by the data source will be removed automatically
            else
            {
                if ( !SaveModified( ) )
                    // give the user a chance to save the current record (if neccessary)
                    break;

                // maybe the user wanted to reject the modified record ?
                if (GetState(ID_BROWSER_UNDORECORD).bEnabled)
                    Execute(ID_BROWSER_UNDORECORD,Sequence<PropertyValue>());

                getBrowserView()->getVclControl()->SetOptions(DbGridControl::OPT_READONLY);
            }
            InvalidateFeature(ID_BROWSER_EDITDOC);
        }
        break;

        case ID_BROWSER_SEARCH:
            if ( SaveModified( ) )
                ExecuteSearch();
            break;

        case ID_BROWSER_COPY:
        case ID_BROWSER_CUT:
        case ID_BROWSER_PASTE:
        {
            CellControllerRef xCurrentController = getBrowserView()->getVclControl()->Controller();
            if (!xCurrentController.Is())
                // should be intercepted by GetState. Normally.
                // Unfortunately ID_BROWSER_PASTE is a 'fast call' slot, which means it may be executed without checking if it is
                // enabled. This would be really deadly herein if the current cell has no controller ...
                // (FS - 13.04.99 - #64694#)
                return;

            Edit& rEdit = (Edit&)xCurrentController->GetWindow();
            switch (nId)
            {
                case ID_BROWSER_CUT : rEdit.Cut(); break;
                case SID_COPY   : rEdit.Copy(); break;
                case ID_BROWSER_PASTE   : rEdit.Paste(); break;
            }
            if (ID_BROWSER_CUT == nId || ID_BROWSER_PASTE == nId)
            {
                xCurrentController->SetModified();
                rEdit.Modify();
            }
        }
        break;

        case ID_BROWSER_SORTDOWN:
            bSortUp = sal_False;
            // DON'T break
        case ID_BROWSER_SORTUP:
        {
            if (!SaveModified())
                break;

            if (!isValidCursor())
                break;

            // only one sort order
            Reference< XPropertySet >  xField(getBoundField(), UNO_QUERY);
            if (!xField.is())
                break;

            ::rtl::OUString sOldSort = m_xParser->getOrder();
            sal_Bool bParserSuccess = sal_False;
            HANDLE_SQL_ERRORS(
                m_xParser->setOrder(::rtl::OUString()); m_xParser->appendOrderByColumn(xField, bSortUp),
                bParserSuccess,
                UniString(ModuleRes(SBA_BROWSER_SETTING_ORDER)),
                "SbaXDataBrowserController::Execute : caught an exception while composing the new filter !"
            )

            if (bParserSuccess)
                applyParserOrder(sOldSort);
        }
        break;

        case ID_BROWSER_AUTOFILTER:
        {
            if (!SaveModified())
                break;

            if (!isValidCursor())
                break;

            Reference< XPropertySet >  xField(getBoundField(), UNO_QUERY);
            if (!xField.is())
                break;

            // check if the column is a aggregate function
            sal_Bool bHaving = sal_False;
            ::rtl::OUString sName;
            xField->getPropertyValue(PROPERTY_NAME) >>= sName;
            Reference< XColumnsSupplier > xColumnsSupplier(m_xParser, UNO_QUERY);
            Reference< ::com::sun::star::container::XNameAccess >  xCols = xColumnsSupplier.is() ? xColumnsSupplier->getColumns() : Reference< ::com::sun::star::container::XNameAccess > ();
            if ( xCols.is() && xCols->hasByName(sName) )
            {
                Reference<XPropertySet> xProp(xCols->getByName(sName),UNO_QUERY);
                static ::rtl::OUString sAgg(RTL_CONSTASCII_USTRINGPARAM("AggregateFunction"));
                if ( xProp->getPropertySetInfo()->hasPropertyByName(sAgg) )
                    xProp->getPropertyValue(sAgg) >>= bHaving;
            }

            ::rtl::OUString sOldFilter = m_xParser->getFilter();
            ::rtl::OUString sOldHaving = m_xParser->getHavingClause();

            Reference< XPropertySet >  xFormSet(getRowSet(), UNO_QUERY);
            sal_Bool bApplied = ::comphelper::getBOOL(xFormSet->getPropertyValue(PROPERTY_APPLYFILTER));
            // do we have a filter but it's not applied ?
            // -> completely overwrite it, else append one
            if (!bApplied)
            {
                DO_SAFE( (bHaving ? m_xParser->setHavingClause(::rtl::OUString()) : m_xParser->setFilter(::rtl::OUString())), "SbaXDataBrowserController::Execute : caught an exception while resetting the new filter !" );
            }

            sal_Bool bParserSuccess = sal_False;


            if ( bHaving )
            {
                HANDLE_SQL_ERRORS(
                    m_xParser->appendHavingClauseByColumn(xField,sal_True),
                    bParserSuccess,
                    UniString(ModuleRes(SBA_BROWSER_SETTING_FILTER)),
                    "SbaXDataBrowserController::Execute : caught an exception while composing the new filter !"
                )
            }
            else
            {
                HANDLE_SQL_ERRORS(
                    m_xParser->appendFilterByColumn(xField,sal_True),
                    bParserSuccess,
                    UniString(ModuleRes(SBA_BROWSER_SETTING_FILTER)),
                    "SbaXDataBrowserController::Execute : caught an exception while composing the new filter !"
                )
            }

            if (bParserSuccess)
                applyParserFilter(sOldFilter, bApplied,sOldHaving);

            InvalidateFeature(ID_BROWSER_REMOVEFILTER);
            InvalidateFeature(ID_BROWSER_FILTERED);
        }
        break;

        case ID_BROWSER_ORDERCRIT:
            ExecuteFilterSortCrit(sal_False);
            break;

        case ID_BROWSER_FILTERCRIT:
            ExecuteFilterSortCrit(sal_True);
            InvalidateFeature(ID_BROWSER_FILTERED);
            break;

        case ID_BROWSER_REMOVEFILTER:
        {
            if (!SaveModified())
                break;

            sal_Bool bNeedPostReload = preReloadForm();
            // reset the filter and the sort property simutaneously so only _one_ new statement has to be
            // sent
            Reference< XPropertySet >  xSet(getRowSet(), UNO_QUERY);
            if ( xSet.is() )
            {
                xSet->setPropertyValue(PROPERTY_FILTER,makeAny(::rtl::OUString()));
                xSet->setPropertyValue(PROPERTY_HAVING_CLAUSE,makeAny(::rtl::OUString()));
                xSet->setPropertyValue(PROPERTY_ORDER,makeAny(::rtl::OUString()));
            }
            try
            {
                reloadForm(m_xLoadable);
                if ( bNeedPostReload )
                    postReloadForm();
            }
            catch(Exception&)
            {
            }
            InvalidateFeature(ID_BROWSER_REMOVEFILTER);
            InvalidateFeature(ID_BROWSER_FILTERED);
        }
        break;

        case ID_BROWSER_REFRESH:
            if ( SaveModified( ) )
            {
                if (!reloadForm(m_xLoadable))
                    criticalFail();
            }
            break;

        case ID_BROWSER_SAVERECORD:
            if ( SaveModified( sal_False ) )
                setCurrentModified( sal_False );
            break;

        case ID_BROWSER_UNDORECORD:
        {
            try
            {
                // restore the cursor state
                Reference< XResultSetUpdate >  xCursor(getRowSet(), UNO_QUERY);
                Reference< XPropertySet >  xSet(xCursor, UNO_QUERY);
                Any aVal = xSet->getPropertyValue(PROPERTY_ISNEW);
                if (aVal.hasValue() && ::comphelper::getBOOL(aVal))
                {
                    xCursor->moveToInsertRow();
                    // no need to reset the grid model after we moved to the insert row, this is done implicitly by the
                    // form
                    // (and in some cases it may be deadly to do the reset explicitly after the form did it implicitly,
                    // cause the form's reset may be async, and this leads to some nice deadlock scenarios ....)
                }
                else
                {
                    xCursor->cancelRowUpdates();

                    // restore the grids state
                    Reference< ::com::sun::star::form::XReset >  xReset(getControlModel(), UNO_QUERY);
                    if (xReset.is())
                        xReset->reset();
                }
            }
            catch(SQLException&)
            {
            }

            setCurrentModified( sal_False );
        }
    }
}

//------------------------------------------------------------------------------
sal_Bool SbaXDataBrowserController::SaveModified(sal_Bool bAskFor)
{
    if ( bAskFor && GetState(ID_BROWSER_SAVERECORD).bEnabled )
    {
        getBrowserView()->getVclControl()->GrabFocus();

        QueryBox aQry(getBrowserView()->getVclControl(), ModuleRes(QUERY_BRW_SAVEMODIFIED));

        switch (aQry.Execute())
        {
            case RET_NO:
                Execute(ID_BROWSER_UNDORECORD,Sequence<PropertyValue>());
                return sal_True;
            case RET_CANCEL:
                return sal_False;
        }
    }

    if ( !CommitCurrent() ) // das aktuelle Control committen lassen
        return sal_False;

    Reference< XPropertySet >  xFormSet(getRowSet(), UNO_QUERY);
    sal_Bool bResult = sal_False;
    try
    {
        if (::comphelper::getBOOL(xFormSet->getPropertyValue(PROPERTY_ISMODIFIED)))
        {
            Reference< XResultSetUpdate >  xCursor(getRowSet(), UNO_QUERY);
            if (::comphelper::getBOOL(xFormSet->getPropertyValue(PROPERTY_ISNEW)))
                xCursor->insertRow();
            else
                xCursor->updateRow();
        }
        bResult = sal_True;
    }
    catch(SQLException&)
    {
    }
    catch(Exception&)
    {
        DBG_ERROR("SbaXDataBrowserController::SaveModified : could not save the current record !");
        bResult = sal_False;
    }

    InvalidateFeature(ID_BROWSER_SAVERECORD);
    InvalidateFeature(ID_BROWSER_UNDORECORD);
    return bResult;
}

//------------------------------------------------------------------------------
sal_Bool SbaXDataBrowserController::CommitCurrent()
{
    if (!getBrowserView())
        return sal_True;

    Reference< ::com::sun::star::awt::XControl >  xActiveControl(getBrowserView()->getGridControl());
    Reference< ::com::sun::star::form::XBoundControl >  xLockingTest(xActiveControl, UNO_QUERY);
    sal_Bool bControlIsLocked = xLockingTest.is() && xLockingTest->getLock();
    if (xActiveControl.is() && !bControlIsLocked)
    {
        // zunaechst das Control fragen ob es das IFace unterstuetzt
        Reference< ::com::sun::star::form::XBoundComponent >  xBoundControl(xActiveControl, UNO_QUERY);
        if (!xBoundControl.is())
            xBoundControl  = Reference< ::com::sun::star::form::XBoundComponent > (xActiveControl->getModel(), UNO_QUERY);
        if (xBoundControl.is() && !xBoundControl->commit())
            return sal_False;
    }
    return sal_True;
}

//------------------------------------------------------------------------------
void SbaXDataBrowserController::setCurrentModified( sal_Bool _bSet )
{
    m_bCurrentlyModified = _bSet;
    InvalidateFeature( ID_BROWSER_SAVERECORD );
    InvalidateFeature( ID_BROWSER_UNDORECORD );
}

//------------------------------------------------------------------------------
void SbaXDataBrowserController::RowChanged()
{
    setCurrentModified( sal_False );
}

//------------------------------------------------------------------------------
void SbaXDataBrowserController::ColumnChanged()
{
    InvalidateFeature(ID_BROWSER_SORTUP);
    InvalidateFeature(ID_BROWSER_SORTDOWN);
    InvalidateFeature(ID_BROWSER_ORDERCRIT);
    InvalidateFeature(ID_BROWSER_FILTERCRIT);
    InvalidateFeature(ID_BROWSER_AUTOFILTER);
    InvalidateFeature(ID_BROWSER_REMOVEFILTER);

    setCurrentModified( sal_False );
}

//------------------------------------------------------------------------------
void SbaXDataBrowserController::SelectionChanged()
{
    // not interested in
}

//------------------------------------------------------------------------------
void SbaXDataBrowserController::CellActivated()
{
    m_aInvalidateClipboard.Start();
    OnInvalidateClipboard( NULL );
}

//------------------------------------------------------------------------------
void SbaXDataBrowserController::CellDeactivated()
{
    m_aInvalidateClipboard.Stop();
    OnInvalidateClipboard( NULL );
}

//------------------------------------------------------------------------------
IMPL_LINK( SbaXDataBrowserController, OnClipboardChanged, void*, EMPTYARG )
{
    ::vos::OGuard aGuard(Application::GetSolarMutex());
    return OnInvalidateClipboard( NULL );
}

//------------------------------------------------------------------------------
IMPL_LINK(SbaXDataBrowserController, OnInvalidateClipboard, AutoTimer*, _pTimer)
{
    InvalidateFeature(ID_BROWSER_CUT);
    InvalidateFeature(ID_BROWSER_COPY);

    // if the invalidation was triggered by the timer, we do not need to invalidate PASTE.
    // The timer is only for checking the CUT/COPY slots regulariry, which depend on the
    // selection state of the active cell
    // TODO: get a callback at the Edit which allows to be notified when the selection
    // changes. This would be much better than this cycle-eating polling mechanism here ....
    if ( _pTimer != &m_aInvalidateClipboard )
        InvalidateFeature(ID_BROWSER_PASTE);

    return 0L;
}

// ------------------------------------------------------------------------------
//sal_uInt16 SbaXDataBrowserController::SaveData(sal_Bool bUI, sal_Bool bForBrowsing)
//{
//  if (!getBrowserView())
//      return sal_True;
//
//  if (!isValidCursor())
//      return sal_True;
//
//  if (bUI && GetState(ID_BROWSER_SAVEDOC).bEnabled)
//  {
//      getBrowserView()->getVclControl()->GrabFocus();
//
//      QueryBox aQry(getBrowserView()->getVclControl(), ModuleRes(QUERY_BRW_SAVEMODIFIED));
//      if (bForBrowsing)
//          aQry.AddButton(ResId(RID_STR_NEW_TASK), RET_NEWTASK,
//              BUTTONDIALOG_DEFBUTTON | BUTTONDIALOG_FOCUSBUTTON);
//
//      switch (aQry.Execute())
//      {
//          case RET_NO:
//              Execute(ID_BROWSER_UNDO);
//              return sal_True;
//          case RET_CANCEL:
//              return sal_False;
//          case RET_NEWTASK:
//              return RET_NEWTASK;
//      }
//  }
//
//
//  return SbaXDataBrowserController_Base::SaveData(bUI,bForBrowsing);
//}
//
// -------------------------------------------------------------------------
Reference< XPropertySet >  SbaXDataBrowserController::getBoundField(sal_uInt16 nViewPos) const
{
    Reference< XPropertySet >  xEmptyReturn;

    // get the current column from the grid
    if (nViewPos == (sal_uInt16)-1)
    {
        Reference< ::com::sun::star::form::XGrid >  xGrid(getBrowserView()->getGridControl(), UNO_QUERY);
        if (!xGrid.is())
            return xEmptyReturn;
        nViewPos = xGrid->getCurrentColumnPosition();
    }
    sal_uInt16 nCurrentCol = getBrowserView()->View2ModelPos(nViewPos);
    if (nCurrentCol == (sal_uInt16)-1)
        return xEmptyReturn;

    // get the according column from the model
    Reference< ::com::sun::star::container::XIndexContainer >  xCols(getControlModel(), UNO_QUERY);
    Reference< XPropertySet >  xCurrentCol(xCols->getByIndex(nCurrentCol),UNO_QUERY);
    if (!xCurrentCol.is())
        return xEmptyReturn;

    xEmptyReturn.set(xCurrentCol->getPropertyValue(PROPERTY_BOUNDFIELD) ,UNO_QUERY);
    return xEmptyReturn;
}

//------------------------------------------------------------------------------
IMPL_LINK(SbaXDataBrowserController, OnSearchContextRequest, FmSearchContext*, pContext)
{
    Reference< ::com::sun::star::container::XIndexAccess >  xPeerContainer(getBrowserView()->getGridControl(), UNO_QUERY);

    // check all grid columns for their control source
    Reference< ::com::sun::star::container::XIndexAccess >  xModelColumns(getFormComponent(), UNO_QUERY);
    DBG_ASSERT(xModelColumns.is(), "SbaXDataBrowserController::OnSearchContextRequest : there is a grid control without columns !");
        // the case 'no columns' should be indicated with an empty container, I think ...
    DBG_ASSERT(xModelColumns->getCount() >= xPeerContainer->getCount(), "SbaXDataBrowserController::OnSearchContextRequest : impossible : have more view than model columns !");

    String sFieldList;
    for (sal_Int32 nViewPos=0; nViewPos<xPeerContainer->getCount(); ++nViewPos)
    {
        Reference< XInterface >  xCurrentColumn(xPeerContainer->getByIndex(nViewPos),UNO_QUERY);
        if (!xCurrentColumn.is())
            continue;

        // can we use this column control fo searching ?
        if (!IsSearchableControl(xCurrentColumn))
            continue;

        sal_uInt16 nModelPos = getBrowserView()->View2ModelPos((sal_uInt16)nViewPos);
        Reference< XPropertySet >  xCurrentColModel(xModelColumns->getByIndex(nModelPos),UNO_QUERY);
        ::rtl::OUString aName = ::comphelper::getString(xCurrentColModel->getPropertyValue(PROPERTY_CONTROLSOURCE));

        sFieldList += (const sal_Unicode*)aName;
        sFieldList += ';';

        pContext->arrFields.push_back(xCurrentColumn);
    }
    sFieldList.EraseTrailingChars(';');

    pContext->xCursor.set(getRowSet(),UNO_QUERY);
    pContext->strUsedFields = sFieldList;

    // if the cursor is in a mode other than STANDARD -> reset
    Reference< XPropertySet >  xCursorSet(pContext->xCursor, UNO_QUERY);
    DBG_ASSERT(xCursorSet.is() && !::comphelper::getBOOL(xCursorSet->getPropertyValue(PROPERTY_ISMODIFIED)),
        "SbaXDataBrowserController::OnSearchContextRequest : please do not call for cursors with modified rows !");
    if (xCursorSet.is() && ::comphelper::getBOOL(xCursorSet->getPropertyValue(PROPERTY_ISNEW)))
    {
        Reference< XResultSetUpdate >  xUpdateCursor(pContext->xCursor, UNO_QUERY);
        xUpdateCursor->moveToCurrentRow();
    }
    return pContext->arrFields.size();
}

//------------------------------------------------------------------------------
IMPL_LINK(SbaXDataBrowserController, OnFoundData, FmFoundRecordInformation*, pInfo)
{
    Reference< ::com::sun::star::sdbcx::XRowLocate >  xCursor(getRowSet(), UNO_QUERY);
    DBG_ASSERT(xCursor.is(), "SbaXDataBrowserController::OnFoundData : shit happens. sometimes. but this is simply impossible !");

    // move the cursor
    xCursor->moveToBookmark(pInfo->aPosition);

    // let the grid snyc it's display with the cursor
    Reference< XPropertySet >  xModelSet(getControlModel(), UNO_QUERY);
    DBG_ASSERT(xModelSet.is(), "SbaXDataBrowserController::OnFoundData : no model set ?!");
    Any aOld = xModelSet->getPropertyValue(::rtl::OUString::createFromAscii("DisplayIsSynchron"));
    xModelSet->setPropertyValue(::rtl::OUString::createFromAscii("DisplayIsSynchron"), ::comphelper::makeBoolAny(sal_Bool(sal_True)));
    xModelSet->setPropertyValue(::rtl::OUString::createFromAscii("DisplayIsSynchron"), aOld);

    // and move to the field
    Reference< ::com::sun::star::container::XIndexAccess >  aColumnControls(getBrowserView()->getGridControl()->getPeer(), UNO_QUERY);
    sal_uInt16 nViewPos;

    for ( nViewPos = 0; nViewPos < aColumnControls->getCount(); ++nViewPos )
    {
        Reference< XInterface >  xCurrent(aColumnControls->getByIndex(nViewPos),UNO_QUERY);
        if (IsSearchableControl(xCurrent))
            if (pInfo->nFieldPos)
                --pInfo->nFieldPos;
            else
                break;
    }

    Reference< ::com::sun::star::form::XGrid >  xGrid(getBrowserView()->getGridControl(), UNO_QUERY);
    xGrid->setCurrentColumnPosition(nViewPos);

    return 0;
}

//------------------------------------------------------------------------------
IMPL_LINK(SbaXDataBrowserController, OnCanceledNotFound, FmFoundRecordInformation*, pInfo)
{
    Reference< ::com::sun::star::sdbcx::XRowLocate >  xCursor(getRowSet(), UNO_QUERY);

    try
    {
        DBG_ASSERT(xCursor.is(), "SbaXDataBrowserController::OnCanceledNotFound : shit happens. sometimes. but this is simply impossible !");
        // move the cursor
        xCursor->moveToBookmark(pInfo->aPosition);
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    try
    {
        // let the grid snyc it's display with the cursor
        Reference< XPropertySet >  xModelSet(getControlModel(), UNO_QUERY);
        DBG_ASSERT(xModelSet.is(), "SbaXDataBrowserController::OnCanceledNotFound : no model set ?!");
        Any aOld = xModelSet->getPropertyValue(::rtl::OUString::createFromAscii("DisplayIsSynchron"));
        xModelSet->setPropertyValue(::rtl::OUString::createFromAscii("DisplayIsSynchron"), ::comphelper::makeBoolAny(sal_Bool(sal_True)));
        xModelSet->setPropertyValue(::rtl::OUString::createFromAscii("DisplayIsSynchron"), aOld);
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    return 0L;
}

//------------------------------------------------------------------------------
IMPL_LINK(SbaXDataBrowserController, OnOpenFinishedMainThread, void*, EMPTYARG)
{
    ::vos::OGuard aGuard(Application::GetSolarMutex());
    if (!m_nPendingLoadFinished)
        // it's possible that the direct call of this link from within suspend caused this method to be executed
        // in another thread while we were waiting for the mutex in this thread
        return 0;
    m_nPendingLoadFinished = 0;

    if ( static_cast< LoadFormThread* >( m_pLoadThread )->WasCanceled() )
        setLoadingCancelled();

    delete m_pLoadThread;
    m_pLoadThread = NULL;

    LoadFinished(sal_False);

    return 0L;
}

//------------------------------------------------------------------------------
IMPL_LINK(SbaXDataBrowserController, OnOpenFinished, void*, EMPTYARG)
{
    ::osl::MutexGuard aCheckGuard(m_aAsyncLoadSafety);

    if (m_bClosingKillOpen)
    {
        delete m_pLoadThread;
        m_pLoadThread = NULL;
    }
    else
        // all cleaning has to run in the main thread, not here (this is called synchronously from the LoadThread)
        // so we use an user event
        m_nPendingLoadFinished = Application::PostUserEvent(LINK(this, SbaXDataBrowserController, OnOpenFinishedMainThread));

    return 0L;
}

//------------------------------------------------------------------------------
IMPL_LINK(SbaXDataBrowserController, OnAsyncGetCellFocus, void*, EMPTYARG)
{
    SbaGridControl* pVclGrid = getBrowserView() ? getBrowserView()->getVclControl() : NULL;
    // if we have a controller, but the window for the controller doesn't have the focus, we correct this
    if(pVclGrid)
    {
        if (!pVclGrid->IsEditing())
            return 0L;

        if (pVclGrid->HasChildPathFocus())
            pVclGrid->Controller()->GetWindow().GrabFocus();
    }

    return 0L;
}

//------------------------------------------------------------------------------
void SbaXDataBrowserController::criticalFail()
{
    InvalidateAll();
}

//------------------------------------------------------------------------------
void SbaXDataBrowserController::LoadFinished(sal_Bool /*bWasSynch*/)
{
    if (isValid() && !loadingCancelled())
    {
        // --------------------------------
        // switch the control to alive mode
        getBrowserView()->getGridControl()->setDesignMode(sal_False);

        // ----------------------------------------------
        // create a parser (needed for filtering/sorting)
        try
        {
            Reference< XPropertySet >  xFormSet(getRowSet(), UNO_QUERY);
            if (::comphelper::getBOOL(xFormSet->getPropertyValue(PROPERTY_ESCAPE_PROCESSING)))
            {   // (only if the statement isn't native)
                // (it is allowed to use the PROPERTY_ISPASSTHROUGH : _after_ loading a form it is valid)
                Reference<XMultiServiceFactory> xFactory(::dbtools::getConnection(getRowSet()),UNO_QUERY);
                if ( xFactory.is() )
                    m_xParser.set(xFactory->createInstance(SERVICE_NAME_SINGLESELECTQUERYCOMPOSER),UNO_QUERY);
            }

            // initialize the parser with the current sql-statement of the form
            if ( m_xParser.is() )
            {
                m_xParser->setElementaryQuery(::comphelper::getString(xFormSet->getPropertyValue(PROPERTY_ACTIVECOMMAND)));
                m_xParser->setFilter(::comphelper::getString(xFormSet->getPropertyValue(PROPERTY_FILTER)));
                m_xParser->setHavingClause(::comphelper::getString(xFormSet->getPropertyValue(PROPERTY_HAVING_CLAUSE)));
                m_xParser->setOrder(::comphelper::getString(xFormSet->getPropertyValue(PROPERTY_ORDER)));
            }
        }
        catch(Exception&)
        {
            DBG_WARNING("SbaXDataBrowserController::LoadFinished: something went wrong while creating the parser !");
            m_xParser = NULL;
            // no further handling, we ignore the error
        }

        // -------------------------------
        InvalidateAll();

        m_aAsyncGetCellFocus.Call();
    }
}
//------------------------------------------------------------------------------
void SbaXDataBrowserController::loaded(const EventObject& /*aEvent*/) throw( RuntimeException )
{
    // not interested in
    // we're loading within an separate thread and have a handling  for it's "finished event"
}

//------------------------------------------------------------------------------
void SbaXDataBrowserController::unloading(const EventObject& /*aEvent*/) throw( RuntimeException )
{
    // not interested in
}

//------------------------------------------------------------------------------
void SbaXDataBrowserController::unloaded(const EventObject& /*aEvent*/) throw( RuntimeException )
{
    InvalidateAll();
        // do this asynchron, there are other listeners reacting on this message ...
        // (it's a little hack : the grid columns are listening to this event, too, and their bound field may
        // change as a reaction on that event. as we have no chance to be notified of this change (which is
        // the one we're interested in) we give them time to do what they want to before invalidating our
        // bound-field-dependent slots ....
    try
    {
        ::comphelper::disposeComponent(m_xParser);
    }
    catch(Exception&)
    {
        OSL_ENSURE(0,"Exception thrown by dispose");
    }
}

//------------------------------------------------------------------------------
void SbaXDataBrowserController::reloading(const EventObject& /*aEvent*/) throw( RuntimeException )
{
    // not interested in
}

//------------------------------------------------------------------------------
void SbaXDataBrowserController::reloaded(const EventObject& /*aEvent*/) throw( RuntimeException )
{
    InvalidateAll();
        // do this asynchronously, there are other listeners reacting on this message ...
        // (it's a little hack : the grid columns are listening to this event, too, and their bound field may
        // change as a reaction on that event. as we have no chance to be notified of this change (which is
        // the one we're interested in) we give them time to do what they want to before invalidating our
        // bound-field-dependent slots ....
}
//------------------------------------------------------------------------------
void SbaXDataBrowserController::enterFormAction()
{
    if (!m_nFormActionNestingLevel)
        // first action -> reset flag
        m_bErrorOccured = false;

    ++m_nFormActionNestingLevel;
}

//------------------------------------------------------------------------------
void SbaXDataBrowserController::leaveFormAction()
{
    DBG_ASSERT(m_nFormActionNestingLevel > 0, "SbaXDataBrowserController::leaveFormAction : invalid call !");
    --m_nFormActionNestingLevel;
}

// -------------------------------------------------------------------------
sal_Bool SbaXDataBrowserController::isLoaded() const
{
    return m_xLoadable.is() && m_xLoadable->isLoaded();
}

// -------------------------------------------------------------------------
sal_Bool SbaXDataBrowserController::isValidCursor() const
{
    if (!m_xColumnsSupplier.is())
        return sal_False;
    Reference< ::com::sun::star::container::XNameAccess >  xCols = m_xColumnsSupplier->getColumns();
    if (!xCols.is() || !xCols->hasElements())
        return sal_False;

    Reference<XPropertySet> xProp(m_xRowSet,UNO_QUERY);
    return ::cppu::any2bool(xProp->getPropertyValue(PROPERTY_ISNEW)) || !(m_xRowSet->isBeforeFirst() || m_xRowSet->isAfterLast()) ||
            (m_xParser.is() && (m_xParser->getFilter().getLength() || m_xParser->getHavingClause().getLength() || m_xParser->getOrder().getLength()));
}

//==================================================================
// LoadFormHelper
//==================================================================

class LoadFormHelper :public ::cppu::WeakImplHelper2< ::com::sun::star::form::XLoadListener,
                                                      XRowSetListener>
{
    enum STATE { STARTED, LOADED, POSITIONED, DISPOSED };
    STATE   m_eState;

    Reference< XRowSet >                m_xForm;

    ::osl::Mutex    m_aAccessSafety;

public:
    LoadFormHelper(const Reference< XRowSet > & _rxForm);

    // ::com::sun::star::form::XLoadListener
    virtual void SAL_CALL loaded(const ::com::sun::star::lang::EventObject& aEvent) throw( RuntimeException );
    virtual void SAL_CALL unloaded(const ::com::sun::star::lang::EventObject& aEvent) throw( RuntimeException );
    virtual void SAL_CALL unloading(const ::com::sun::star::lang::EventObject& aEvent) throw( RuntimeException );
    virtual void SAL_CALL reloading(const ::com::sun::star::lang::EventObject& aEvent) throw( RuntimeException );
    virtual void SAL_CALL reloaded(const ::com::sun::star::lang::EventObject& aEvent) throw( RuntimeException );

    // XRowSetListener
    virtual void SAL_CALL cursorMoved(const ::com::sun::star::lang::EventObject& event) throw( RuntimeException );
    virtual void SAL_CALL rowChanged(const ::com::sun::star::lang::EventObject& event) throw( RuntimeException );
    virtual void SAL_CALL rowSetChanged(const ::com::sun::star::lang::EventObject& event) throw( RuntimeException );

    // ::com::sun::star::lang::XEventListener
    virtual void SAL_CALL disposing(const ::com::sun::star::lang::EventObject& Source) throw( RuntimeException );


    bool WaitUntilReallyLoaded(bool _bOnlyIfLoaded);
        // waits 'til the first positioned event after the loaded event. returns true if successfull,
        // false if the form was disposed or unloaded before or while waiting
        // if _bOnlyIfLoaded is false and the form isn't loaded already loaded, false will be returned
        // (without any wating)

    void cancel();

protected:
    ~LoadFormHelper();

    void implDispose();
};

DBG_NAME(LoadFormHelper)
//------------------------------------------------------------------------------
LoadFormHelper::LoadFormHelper(const Reference< XRowSet > & _rxForm)
    :m_eState(STARTED)
    ,m_xForm(_rxForm)
{
    DBG_CTOR(LoadFormHelper,NULL);

    Reference< ::com::sun::star::form::XLoadable > (m_xForm, UNO_QUERY)->addLoadListener(this);
    m_xForm->addRowSetListener(this);
}

//------------------------------------------------------------------------------
LoadFormHelper::~LoadFormHelper()
{
    ::osl::MutexGuard aGuard(m_aAccessSafety);
    implDispose();

    DBG_DTOR(LoadFormHelper,NULL);
}

//------------------------------------------------------------------------------
void LoadFormHelper::implDispose()
{
    if (DISPOSED != m_eState)
    {

        Reference< ::com::sun::star::form::XLoadable > (m_xForm, UNO_QUERY)->removeLoadListener(this);
        m_xForm->removeRowSetListener(this);
        m_xForm = NULL;
        m_eState = DISPOSED;
    }
}

//------------------------------------------------------------------------------
void SAL_CALL LoadFormHelper::loaded(const ::com::sun::star::lang::EventObject& /*aEvent*/) throw( RuntimeException )
{
    ::osl::MutexGuard aGuard(m_aAccessSafety);
    DBG_ASSERT(m_eState == STARTED || m_eState == DISPOSED, "LoadFormHelper::loaded : wrong call !");
    if (m_eState == STARTED)
        m_eState = LOADED;
}

//------------------------------------------------------------------------------
void SAL_CALL LoadFormHelper::unloaded(const ::com::sun::star::lang::EventObject& /*aEvent*/) throw( RuntimeException )
{
    ::osl::MutexGuard aGuard(m_aAccessSafety);
    DBG_ERROR("LoadFormHelper::unloaded : shouldn't be called !");
    implDispose();
}

//------------------------------------------------------------------------------
void SAL_CALL LoadFormHelper::unloading(const ::com::sun::star::lang::EventObject& /*aEvent*/) throw( RuntimeException )
{
}

//------------------------------------------------------------------------------
void SAL_CALL LoadFormHelper::reloading(const ::com::sun::star::lang::EventObject& /*aEvent*/) throw( RuntimeException )
{
}

//------------------------------------------------------------------------------
void SAL_CALL LoadFormHelper::reloaded(const ::com::sun::star::lang::EventObject& /*aEvent*/) throw( RuntimeException )
{
}

//------------------------------------------------------------------------------
void SAL_CALL LoadFormHelper::cursorMoved(const ::com::sun::star::lang::EventObject& /*event*/) throw( RuntimeException )
{
    ::osl::MutexGuard aGuard(m_aAccessSafety);
    if (m_eState == LOADED)
        m_eState = POSITIONED;
}

//------------------------------------------------------------------------------
void SAL_CALL LoadFormHelper::rowChanged(const ::com::sun::star::lang::EventObject& /*event*/) throw( RuntimeException )
{
}

//------------------------------------------------------------------------------
void SAL_CALL LoadFormHelper::rowSetChanged(const ::com::sun::star::lang::EventObject& /*event*/) throw( RuntimeException )
{
}

//------------------------------------------------------------------------------
void SAL_CALL LoadFormHelper::disposing(const ::com::sun::star::lang::EventObject& /*Source*/) throw( RuntimeException )
{
    ::osl::MutexGuard aGuard(m_aAccessSafety);
    implDispose();
}

//------------------------------------------------------------------------------
void LoadFormHelper::cancel()
{
    implDispose();
}

//------------------------------------------------------------------------------
bool LoadFormHelper::WaitUntilReallyLoaded(bool _bOnlyIfLoaded)
{
    ::osl::ResettableMutexGuard aGuard( m_aAccessSafety );
    if (DISPOSED == m_eState)
        return false;

    if (_bOnlyIfLoaded && (STARTED == m_eState))
        // we did't get a "loaded" event ....
        return false;

    sal_Bool bDone = (POSITIONED == m_eState);
    aGuard.clear();

    while (!bDone)
    {
        aGuard.reset();
        bDone = (POSITIONED == m_eState);
        aGuard.clear();
    }

    aGuard.reset();
    implDispose();

    return true;
}

//==================================================================
// LoadFormThread - a thread for asynchronously loading a form
//==================================================================
//------------------------------------------------------------------------------
void LoadFormThread::run()
{
    // On instantiation of a SfxCancellable the application is notified and 'switches on' the red stop button.
    // Unfortunally this is conditioned with the acquirement of the solar mutex, and the application tries
    // only once and ignores the notification if it fails.
    // To prevent that we get the solar mutex and _block_ 'til we got it.
    // As we are in the 'top level execution' of this thread (with a rather small stack and no other mutexes locked)
    // we shouldn't experience problems with deadlocks ...
    ::vos::OClearableGuard aSolarGuard(Application::GetSolarMutex());
    ThreadStopper* pStopper = new ThreadStopper(this, m_sStopperCaption);
    aSolarGuard.clear();

    // we're not canceled yet
    ::osl::ClearableMutexGuard aResetGuard(m_aAccessSafety);
    m_bCanceled = sal_False;
    aResetGuard.clear();

    LoadFormHelper* pHelper = new LoadFormHelper(m_xRowSet);
    pHelper->acquire();

    // start it
    bool bErrorOccured = false;
    Reference< XLoadable > xLoadable(m_xRowSet, UNO_QUERY);
    try
    {
        Reference< XRowSet >  xMove(m_xRowSet, UNO_QUERY);
        DBG_ASSERT(xLoadable.is() && xMove.is(), "LoadFormThread::run : invalid cursor !");
        xLoadable->load();
        // go to the first record if the load was successfull.
        Reference< XColumnsSupplier > xColumnsSupplier(m_xRowSet, UNO_QUERY);
        Reference< ::com::sun::star::container::XNameAccess >  xCols = xColumnsSupplier.is() ? xColumnsSupplier->getColumns() : Reference< ::com::sun::star::container::XNameAccess > ();
        if (xCols.is() && xCols->hasElements())
            xMove->first();
        else
            bErrorOccured = true;
    }
    catch(Exception&)
    {
        bErrorOccured = true;
    }

    // check if we were canceled
    ::osl::ClearableMutexGuard aTestGuard(m_aAccessSafety);
    bool bReallyCanceled = m_bCanceled ? true : false;;
    aTestGuard.clear();

    bReallyCanceled |= bErrorOccured;

    // the load on the form is "slightly asyncronous" (which isn't covered by it's specification, anyway), so wait
    // some time ....
    // (though me thinks that the load of the new api is synchronous, so we won't need this LoadFormHelper anymore ...)
    if (!bReallyCanceled)
        pHelper->WaitUntilReallyLoaded(true);

    pHelper->cancel();
    pHelper->release();

    // yes, we were, but eventually the cancel request didn't reach the data source in time
    if (bReallyCanceled && xLoadable.is() && xLoadable->isLoaded())
        xLoadable->unload();

    pStopper->OwnerTerminated();
        // this will cause the stopper to delete itself (in the main thread) so we don't have to take care of the
        // solar mutex
}

//------------------------------------------------------------------------------
void LoadFormThread::onTerminated()
{
    ::osl::ClearableMutexGuard aGuard(m_aAccessSafety);
    if (m_aTerminationHandler.IsSet())
    {
        // within the call of our termination handler we may be deleted, so do anything which is a member
        // access before the call ...
        // FS - #69801# - 02.12.99
        Link aHandler(m_aTerminationHandler);
        aGuard.clear();
        aHandler.Call(this);
    }
    else
    {
        // we are fully responsible for the data source and for ourself, so dispose the former ...
        try
        {
            ::comphelper::disposeComponent(m_xRowSet);

            m_xRowSet           = NULL;
        }
        catch(Exception&)
        {
            OSL_ENSURE(0,"Exception thrown by dispose");
        }
        // ... and delete the latter
        aGuard.clear();     // like above - releasing the mutex is a member access ...
        delete this;
    }
}

//------------------------------------------------------------------------------
void LoadFormThread::StopIt()
{
    ::osl::ClearableMutexGuard aResetGuard(m_aAccessSafety);
    m_bCanceled = sal_True;
    aResetGuard.clear();

    Reference< XColumnsSupplier > xColumnsSupplier(m_xRowSet, UNO_QUERY);
    if (!xColumnsSupplier.is())
    {
        DBG_ERROR("LoadFormThread::StopIt : invalid data source !");
        return;
    }
    Reference< ::com::sun::star::container::XNameAccess >  xCols(xColumnsSupplier->getColumns(), UNO_QUERY);
    if (!xCols.is() || !xCols->hasElements())
        // the cursor isn't alive, don't need to cancel
        return;

    Reference< ::com::sun::star::util::XCancellable >  xCancel(m_xRowSet, UNO_QUERY);
    if (xCancel.is())
    {
        try  { xCancel->cancel(); } catch(SQLException&) {}
        // with this the cursor returns from it's load call, this terminates our run, this get's our termination handler to
        // be called
        // (the try-catch is just in case the cancel wasn't neccessary anymore)
    }
}

//------------------------------------------------------------------------------
LoadFormThread::ThreadStopper::ThreadStopper(LoadFormThread* pOwner, const String& rTitle)
    :SfxCancellable(SFX_APP()->GetCancelManager(), rTitle)
    ,m_pOwner(pOwner)
{
}

//------------------------------------------------------------------------------
void LoadFormThread::ThreadStopper::Cancel()
{
    if (!m_pOwner)
        return;

    ::osl::MutexGuard aGuard(m_pOwner->m_aAccessSafety);
    if (IsCancelled())
        // we already did pass this to our owner
        return;

    SfxCancellable::Cancel();
    m_pOwner->StopIt();
}

//------------------------------------------------------------------------------
void LoadFormThread::ThreadStopper::OwnerTerminated()
{
    m_pOwner = NULL;
    Application::PostUserEvent(LINK(this, LoadFormThread::ThreadStopper, OnDeleteInMainThread), this);
}

//------------------------------------------------------------------------------
IMPL_LINK(LoadFormThread::ThreadStopper, OnDeleteInMainThread, LoadFormThread::ThreadStopper*, pThis)
{
    delete pThis;
    return 0L;
}
// -----------------------------------------------------------------------------
sal_Int16 SbaXDataBrowserController::getCurrentColumnPosition()
{
    Reference< ::com::sun::star::form::XGrid >  xGrid(getBrowserView()->getGridControl(), UNO_QUERY);
    sal_Int16 nViewPos = -1;
    try
    {
        if ( xGrid.is() )
            nViewPos = xGrid->getCurrentColumnPosition();
    }
    catch(Exception&) {}
    return nViewPos;
}
// -----------------------------------------------------------------------------
void SbaXDataBrowserController::setCurrentColumnPosition( sal_Int16 _nPos )
{
    Reference< ::com::sun::star::form::XGrid >  xGrid(getBrowserView()->getGridControl(), UNO_QUERY);
    try
    {
        if ( -1 != _nPos )
            xGrid->setCurrentColumnPosition(_nPos);
    }
    catch(Exception&) {}
}
// -----------------------------------------------------------------------------
void SbaXDataBrowserController::BeforeDrop()
{
    Reference< ::com::sun::star::sdb::XSQLErrorBroadcaster >  xFormError(getRowSet(), UNO_QUERY);
    if (xFormError.is())
        xFormError->removeSQLErrorListener((::com::sun::star::sdb::XSQLErrorListener*)this);
}
// -----------------------------------------------------------------------------
void SbaXDataBrowserController::AfterDrop()
{
    Reference< ::com::sun::star::sdb::XSQLErrorBroadcaster >  xFormError(getRowSet(), UNO_QUERY);
    if (xFormError.is())
        xFormError->addSQLErrorListener((::com::sun::star::sdb::XSQLErrorListener*)this);
}
// -----------------------------------------------------------------------------
void SbaXDataBrowserController::addColumnListeners(const Reference< ::com::sun::star::awt::XControlModel > & _xGridControlModel)
{
// ... all the grid columns
    Reference< ::com::sun::star::container::XIndexContainer >  xColumns(_xGridControlModel, UNO_QUERY);
    if (xColumns.is())
    {
        sal_Int32 nCount = xColumns->getCount();
        for (sal_uInt16 i=0; i < nCount; ++i)
        {
            Reference< XPropertySet >  xCol(xColumns->getByIndex(i),UNO_QUERY);
            AddColumnListener(xCol);
        }
    }
}
// -----------------------------------------------------------------------------
sal_Bool SbaXDataBrowserController::InitializeGridModel(const Reference< ::com::sun::star::form::XFormComponent > & /*xGrid*/)
{
    return sal_True;
}
//..................................................................
}   // namespace dbaui
//..................................................................
