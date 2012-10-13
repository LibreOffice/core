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
#include "brwctrlr.hxx"
#include "brwview.hxx"
#include "dbu_brw.hrc"
#include "dbustrings.hrc"
#include "queryfilter.hxx"
#include "queryorder.hxx"
#include "sqlmessage.hxx"

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/form/FormButtonType.hpp>
#include <com/sun/star/form/FormSubmitEncoding.hpp>
#include <com/sun/star/form/FormSubmitMethod.hpp>
#include <com/sun/star/form/XApproveActionBroadcaster.hpp>
#include <com/sun/star/form/XBoundControl.hpp>
#include <com/sun/star/form/XChangeBroadcaster.hpp>
#include <com/sun/star/form/XChangeListener.hpp>
#include <com/sun/star/form/XDatabaseParameterBroadcaster.hpp>
#include <com/sun/star/form/XLoadable.hpp>
#include <com/sun/star/form/XReset.hpp>
#include <com/sun/star/form/XResetListener.hpp>
#include <com/sun/star/form/XSubmit.hpp>
#include <com/sun/star/form/XSubmitListener.hpp>
#include <com/sun/star/form/runtime/XFormController.hpp>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/sdb/ErrorCondition.hpp>
#include <com/sun/star/sdb/ParametersRequest.hpp>
#include <com/sun/star/sdb/SQLContext.hpp>
#include <com/sun/star/sdb/XInteractionSupplyParameters.hpp>
#include <com/sun/star/sdb/XSQLErrorBroadcaster.hpp>
#include <com/sun/star/sdb/XSingleSelectQueryComposer.hpp>
#include <com/sun/star/sdb/SQLFilterOperator.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdbc/XResultSetUpdate.hpp>
#include <com/sun/star/sdbc/XRowSetListener.hpp>
#include <com/sun/star/sdbc/XWarningsSupplier.hpp>
#include <com/sun/star/sdbcx/Privilege.hpp>
#include <com/sun/star/sdbcx/XRowLocate.hpp>
#include <com/sun/star/task/InteractionHandler.hpp>
#include <com/sun/star/uno/TypeClass.hpp>
#include <com/sun/star/util/NumberFormatter.hpp>
#include <com/sun/star/util/XCancellable.hpp>

#include <comphelper/enumhelper.hxx>
#include <comphelper/extract.hxx>
#include <comphelper/interaction.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/string.hxx>
#include <connectivity/dbexception.hxx>
#include <connectivity/dbtools.hxx>
#include <connectivity/sqlerror.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <osl/mutex.hxx>
#include <rtl/logfile.hxx>
#include <sfx2/app.hxx>
#include <sfx2/sfx.hrc>
#include <svx/fmsearch.hxx>
#include <svx/svxdlg.hxx>
#include <tools/diagnose_ex.h>
#include <osl/diagnose.h>
#include <vcl/msgbox.hxx>
#include <vcl/waitobj.hxx>

using namespace ::com::sun::star;
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

#define DO_SAFE( action, message ) try { action; } catch(Exception&) { OSL_FAIL(message); } ;

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
// a helper class implementing a runtime::XFormController, will be aggregated by SbaXDataBrowserController
// (we can't derive from XFormController as it's base class is XTabController and the XTabController::getModel collides
// with the XController::getModel implemented in our base class SbaXDataBrowserController)
class SbaXDataBrowserController::FormControllerImpl
    : public ::cppu::WeakAggImplHelper2< ::com::sun::star::form::runtime::XFormController,
                                         ::com::sun::star::frame::XFrameActionListener >
{
    friend class SbaXDataBrowserController;
    ::cppu::OInterfaceContainerHelper   m_aActivateListeners;
    SbaXDataBrowserController*          m_pOwner;

public:
    FormControllerImpl(SbaXDataBrowserController* pOwner);

    // XFormController
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::form::runtime::XFormOperations > SAL_CALL getFormOperations() throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >  SAL_CALL getCurrentControl(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL addActivateListener(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormControllerListener > & l) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL removeActivateListener(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormControllerListener > & l) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL addChildController( const ::com::sun::star::uno::Reference< ::com::sun::star::form::runtime::XFormController >& _ChildController ) throw( ::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IllegalArgumentException );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::form::runtime::XFormControllerContext > SAL_CALL getContext() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setContext( const ::com::sun::star::uno::Reference< ::com::sun::star::form::runtime::XFormControllerContext >& _context ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler > SAL_CALL getInteractionHandler() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setInteractionHandler( const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& _interactionHandler ) throw (::com::sun::star::uno::RuntimeException);

    // XChild, base of XFormController
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL getParent(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setParent( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& Parent ) throw (::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException);

    // XComponent, base of XFormController
    virtual void SAL_CALL dispose(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw (::com::sun::star::uno::RuntimeException);

    // XIndexAccess, base of XFormController
    virtual ::sal_Int32 SAL_CALL getCount(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( ::sal_Int32 Index ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // XElementAccess, base of XIndexAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL hasElements(  ) throw (::com::sun::star::uno::RuntimeException);

    // XEnumerationAccess, base of XElementAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration > SAL_CALL createEnumeration(  ) throw (::com::sun::star::uno::RuntimeException);

    // XModifyBroadcaster, base of XFormController
    virtual void SAL_CALL addModifyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& aListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeModifyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& aListener ) throw (::com::sun::star::uno::RuntimeException);

    // XConfirmDeleteBroadcaster, base of XFormController
    virtual void SAL_CALL addConfirmDeleteListener( const ::com::sun::star::uno::Reference< ::com::sun::star::form::XConfirmDeleteListener >& aListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeConfirmDeleteListener( const ::com::sun::star::uno::Reference< ::com::sun::star::form::XConfirmDeleteListener >& aListener ) throw (::com::sun::star::uno::RuntimeException);

    // XSQLErrorBroadcaster, base of XFormController
    virtual void SAL_CALL addSQLErrorListener( const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XSQLErrorListener >& Listener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeSQLErrorListener( const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XSQLErrorListener >& Listener ) throw (::com::sun::star::uno::RuntimeException);

    // XRowSetApproveBroadcaster, base of XFormController
    virtual void SAL_CALL addRowSetApproveListener( const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XRowSetApproveListener >& listener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeRowSetApproveListener( const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XRowSetApproveListener >& listener ) throw (::com::sun::star::uno::RuntimeException);

    // XDatabaseParameterBroadcaster2, base of XFormController
    virtual void SAL_CALL addDatabaseParameterListener( const ::com::sun::star::uno::Reference< ::com::sun::star::form::XDatabaseParameterListener >& aListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeDatabaseParameterListener( const ::com::sun::star::uno::Reference< ::com::sun::star::form::XDatabaseParameterListener >& aListener ) throw (::com::sun::star::uno::RuntimeException);

    // XDatabaseParameterBroadcaster, base of XDatabaseParameterBroadcaster2
    virtual void SAL_CALL addParameterListener( const ::com::sun::star::uno::Reference< ::com::sun::star::form::XDatabaseParameterListener >& aListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeParameterListener( const ::com::sun::star::uno::Reference< ::com::sun::star::form::XDatabaseParameterListener >& aListener ) throw (::com::sun::star::uno::RuntimeException);

    // XModeSelector, base of XFormController
    virtual void SAL_CALL setMode( const ::rtl::OUString& aMode ) throw (::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getMode(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedModes(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL supportsMode( const ::rtl::OUString& aMode ) throw (::com::sun::star::uno::RuntimeException);

    // XTabController, base of XFormController
    virtual void SAL_CALL setModel(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTabControllerModel > & Model) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTabControllerModel >  SAL_CALL getModel(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL setContainer(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer > & _Container) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer >  SAL_CALL getContainer(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >  > SAL_CALL getControls(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL autoTabOrder(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL activateTabOrder(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL activateFirst(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL activateLast(void) throw( ::com::sun::star::uno::RuntimeException );

    // XFrameActionListener
    virtual void SAL_CALL frameAction(const ::com::sun::star::frame::FrameActionEvent& aEvent) throw( ::com::sun::star::uno::RuntimeException );

    // XEventListener
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
Reference< runtime::XFormOperations > SAL_CALL SbaXDataBrowserController::FormControllerImpl::getFormOperations() throw (RuntimeException)
{
    OSL_FAIL( "SbaXDataBrowserController::FormControllerImpl::getFormOperations: not supported!" );
    return NULL;
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
void SAL_CALL SbaXDataBrowserController::FormControllerImpl::addChildController( const Reference< runtime::XFormController >& /*_ChildController*/ ) throw( RuntimeException, IllegalArgumentException )
{
    // not supported
    throw IllegalArgumentException( ::rtl::OUString(), *this, 1 );
}

//------------------------------------------------------------------
Reference< runtime::XFormControllerContext > SAL_CALL SbaXDataBrowserController::FormControllerImpl::getContext() throw (RuntimeException)
{
    OSL_FAIL( "SbaXDataBrowserController::FormControllerImpl::getContext: no support!!" );
    return NULL;
}

//------------------------------------------------------------------
void SAL_CALL SbaXDataBrowserController::FormControllerImpl::setContext( const Reference< runtime::XFormControllerContext >& /*_context*/ ) throw (RuntimeException)
{
    OSL_FAIL( "SbaXDataBrowserController::FormControllerImpl::setContext: no support!!" );
}

//------------------------------------------------------------------
Reference< XInteractionHandler > SAL_CALL SbaXDataBrowserController::FormControllerImpl::getInteractionHandler() throw (RuntimeException)
{
    OSL_FAIL( "SbaXDataBrowserController::FormControllerImpl::getInteractionHandler: no support!!" );
    return NULL;
}

//------------------------------------------------------------------
void SAL_CALL SbaXDataBrowserController::FormControllerImpl::setInteractionHandler( const Reference< XInteractionHandler >& /*_interactionHandler*/ ) throw (RuntimeException)
{
    OSL_FAIL( "SbaXDataBrowserController::FormControllerImpl::setInteractionHandler: no support!!" );
}

//------------------------------------------------------------------
Reference< XInterface > SAL_CALL SbaXDataBrowserController::FormControllerImpl::getParent(  ) throw (RuntimeException)
{
    // don't have any parent form controllers
    return NULL;
}

//------------------------------------------------------------------
void SAL_CALL SbaXDataBrowserController::FormControllerImpl::setParent( const Reference< XInterface >& /*Parent*/ ) throw (NoSupportException, RuntimeException)
{
    throw NoSupportException( ::rtl::OUString(), *this );
}

//------------------------------------------------------------------
void SAL_CALL SbaXDataBrowserController::FormControllerImpl::dispose(  ) throw (RuntimeException)
{
    OSL_FAIL( "SbaXDataBrowserController::FormControllerImpl::dispose: no, you do *not* want to do this!" );
}

//------------------------------------------------------------------
void SAL_CALL SbaXDataBrowserController::FormControllerImpl::addEventListener( const Reference< XEventListener >& /*xListener*/ ) throw (RuntimeException)
{
    OSL_FAIL( "SbaXDataBrowserController::FormControllerImpl::addEventListener: no support!!" );
}

//------------------------------------------------------------------
void SAL_CALL SbaXDataBrowserController::FormControllerImpl::removeEventListener( const Reference< XEventListener >& /*aListener*/ ) throw (RuntimeException)
{
    OSL_FAIL( "SbaXDataBrowserController::FormControllerImpl::removeEventListener: no support!!" );
}

//------------------------------------------------------------------
::sal_Int32 SAL_CALL SbaXDataBrowserController::FormControllerImpl::getCount(  ) throw (RuntimeException)
{
    // no sub controllers, never
    return 0;
}

//------------------------------------------------------------------
Any SAL_CALL SbaXDataBrowserController::FormControllerImpl::getByIndex( ::sal_Int32 /*Index*/ ) throw (IndexOutOfBoundsException, WrappedTargetException, RuntimeException)
{
    // no sub controllers, never
    throw IndexOutOfBoundsException( ::rtl::OUString(), *this );
}

//------------------------------------------------------------------
Type SAL_CALL SbaXDataBrowserController::FormControllerImpl::getElementType(  ) throw (RuntimeException)
{
    return ::cppu::UnoType< runtime::XFormController >::get();
}

//------------------------------------------------------------------
::sal_Bool SAL_CALL SbaXDataBrowserController::FormControllerImpl::hasElements(  ) throw (RuntimeException)
{
    // no sub controllers, never
    return false;
}

//------------------------------------------------------------------
Reference< XEnumeration > SAL_CALL SbaXDataBrowserController::FormControllerImpl::createEnumeration(  ) throw (RuntimeException)
{
    return new ::comphelper::OEnumerationByIndex( this );
}

//------------------------------------------------------------------
void SAL_CALL SbaXDataBrowserController::FormControllerImpl::addModifyListener( const Reference< XModifyListener >& /*_Listener*/ ) throw (RuntimeException)
{
    OSL_FAIL( "SbaXDataBrowserController::FormControllerImpl::addModifyListener: no support!" );
}

//------------------------------------------------------------------
void SAL_CALL SbaXDataBrowserController::FormControllerImpl::removeModifyListener( const Reference< XModifyListener >& /*_Listener*/ ) throw (RuntimeException)
{
    OSL_FAIL( "SbaXDataBrowserController::FormControllerImpl::removeModifyListener: no support!" );
}

//------------------------------------------------------------------
void SAL_CALL SbaXDataBrowserController::FormControllerImpl::addConfirmDeleteListener( const Reference< XConfirmDeleteListener >& /*_Listener*/ ) throw (RuntimeException)
{
    OSL_FAIL( "SbaXDataBrowserController::FormControllerImpl::addConfirmDeleteListener: no support!" );
}

//------------------------------------------------------------------
void SAL_CALL SbaXDataBrowserController::FormControllerImpl::removeConfirmDeleteListener( const Reference< XConfirmDeleteListener >& /*_Listener*/ ) throw (RuntimeException)
{
    OSL_FAIL( "SbaXDataBrowserController::FormControllerImpl::removeConfirmDeleteListener: no support!" );
}

//------------------------------------------------------------------
void SAL_CALL SbaXDataBrowserController::FormControllerImpl::addSQLErrorListener( const Reference< XSQLErrorListener >& /*_Listener*/ ) throw (RuntimeException)
{
    OSL_FAIL( "SbaXDataBrowserController::FormControllerImpl::addSQLErrorListener: no support!" );
}

//------------------------------------------------------------------
void SAL_CALL SbaXDataBrowserController::FormControllerImpl::removeSQLErrorListener( const Reference< XSQLErrorListener >& /*_Listener*/ ) throw (RuntimeException)
{
    OSL_FAIL( "SbaXDataBrowserController::FormControllerImpl::removeSQLErrorListener: no support!" );
}

//------------------------------------------------------------------
void SAL_CALL SbaXDataBrowserController::FormControllerImpl::addRowSetApproveListener( const Reference< XRowSetApproveListener >& /*_Listener*/ ) throw (RuntimeException)
{
    OSL_FAIL( "SbaXDataBrowserController::FormControllerImpl::addRowSetApproveListener: no support!" );
}

//------------------------------------------------------------------
void SAL_CALL SbaXDataBrowserController::FormControllerImpl::removeRowSetApproveListener( const Reference< XRowSetApproveListener >& /*_Listener*/ ) throw (RuntimeException)
{
    OSL_FAIL( "SbaXDataBrowserController::FormControllerImpl::removeRowSetApproveListener: no support!" );
}

//------------------------------------------------------------------
void SAL_CALL SbaXDataBrowserController::FormControllerImpl::addDatabaseParameterListener( const Reference< XDatabaseParameterListener >& /*_Listener*/ ) throw (RuntimeException)
{
    OSL_FAIL( "SbaXDataBrowserController::FormControllerImpl::addDatabaseParameterListener: no support!" );
}

//------------------------------------------------------------------
void SAL_CALL SbaXDataBrowserController::FormControllerImpl::removeDatabaseParameterListener( const Reference< XDatabaseParameterListener >& /*_Listener*/ ) throw (RuntimeException)
{
    OSL_FAIL( "SbaXDataBrowserController::FormControllerImpl::removeDatabaseParameterListener: no support!" );
}

//------------------------------------------------------------------
void SAL_CALL SbaXDataBrowserController::FormControllerImpl::addParameterListener( const Reference< XDatabaseParameterListener >& /*_Listener*/ ) throw (RuntimeException)
{
    OSL_FAIL( "SbaXDataBrowserController::FormControllerImpl::addParameterListener: no support!" );
}

//------------------------------------------------------------------
void SAL_CALL SbaXDataBrowserController::FormControllerImpl::removeParameterListener( const Reference< XDatabaseParameterListener >& /*_Listener*/ ) throw (RuntimeException)
{
    OSL_FAIL( "SbaXDataBrowserController::FormControllerImpl::removeParameterListener: no support!" );
}

//------------------------------------------------------------------
void SAL_CALL SbaXDataBrowserController::FormControllerImpl::setMode( const ::rtl::OUString& _rMode ) throw (NoSupportException, RuntimeException)
{
    if ( !supportsMode( _rMode ) )
        throw NoSupportException();
}

//------------------------------------------------------------------
::rtl::OUString SAL_CALL SbaXDataBrowserController::FormControllerImpl::getMode(  ) throw (RuntimeException)
{
    return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DataMode" ) );
}

//------------------------------------------------------------------
Sequence< ::rtl::OUString > SAL_CALL SbaXDataBrowserController::FormControllerImpl::getSupportedModes(  ) throw (RuntimeException)
{
    Sequence< ::rtl::OUString > aModes(1);
    aModes[1] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DataMode" ) );
    return aModes;
}

//------------------------------------------------------------------
::sal_Bool SAL_CALL SbaXDataBrowserController::FormControllerImpl::supportsMode( const ::rtl::OUString& aMode ) throw (RuntimeException)
{
    return aMode.compareToAscii( "DataMode" ) == 0;
}

//------------------------------------------------------------------
void SAL_CALL SbaXDataBrowserController::FormControllerImpl::setModel(const Reference< ::com::sun::star::awt::XTabControllerModel > & /*Model*/) throw( RuntimeException )
{
    OSL_FAIL("SbaXDataBrowserController::FormControllerImpl::setModel : invalid call, can't change my model !");
}

//------------------------------------------------------------------
Reference< ::com::sun::star::awt::XTabControllerModel >  SAL_CALL SbaXDataBrowserController::FormControllerImpl::getModel(void) throw( RuntimeException )
{
    return Reference< XTabControllerModel >(m_pOwner->getRowSet(), UNO_QUERY);
}

//------------------------------------------------------------------
void SAL_CALL SbaXDataBrowserController::FormControllerImpl::setContainer(const Reference< ::com::sun::star::awt::XControlContainer > & /*_Container*/) throw( RuntimeException )
{
    OSL_FAIL("SbaXDataBrowserController::FormControllerImpl::setContainer : invalid call, can't change my container !");
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
    OSL_FAIL("SbaXDataBrowserController::FormControllerImpl::autoTabOrder : nothing to do (always have only one control) !");
}

//------------------------------------------------------------------
void SAL_CALL SbaXDataBrowserController::FormControllerImpl::activateTabOrder(void) throw( RuntimeException )
{
    OSL_FAIL("SbaXDataBrowserController::FormControllerImpl::activateTabOrder : nothing to do (always have only one control) !");
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
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaui", "Ocke.Janssen@sun.com", "SbaXDataBrowserController::getTypes" );
    return ::comphelper::concatSequences(
        SbaXDataBrowserController_Base::getTypes(),
        m_pFormControllerImpl->getTypes()
    );
}

//------------------------------------------------------------------
Sequence< sal_Int8 > SAL_CALL SbaXDataBrowserController::getImplementationId(  ) throw (RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaui", "Ocke.Janssen@sun.com", "SbaXDataBrowserController::getImplementationId" );
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

    // check for our aggregate (implementing the XFormController)
    if (!aRet.hasValue())
        aRet = m_xFormControllerImpl->queryAggregation(_rType);

    // no more to offer
    return aRet;
}

DBG_NAME(SbaXDataBrowserController)
//------------------------------------------------------------------------------
SbaXDataBrowserController::SbaXDataBrowserController(const Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rM)
    :SbaXDataBrowserController_Base(_rM)
    ,m_nRowSetPrivileges(0)
    ,m_pClipbordNotifier( NULL )
    ,m_aAsyncGetCellFocus(LINK(this, SbaXDataBrowserController, OnAsyncGetCellFocus))
    ,m_aAsyncDisplayError( LINK( this, SbaXDataBrowserController, OnAsyncDisplayError ) )
    ,m_sStateSaveRecord(ModuleRes(RID_STR_SAVE_CURRENT_RECORD))
    ,m_sStateUndoRecord(ModuleRes(RID_STR_UNDO_MODIFY_RECORD))
    ,m_sModuleIdentifier( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.sdb.DataSourceBrowser" ) ) )
    ,m_pFormControllerImpl(NULL)
    ,m_nPendingLoadFinished(0)
    ,m_nFormActionNestingLevel(0)
    ,m_bLoadCanceled( sal_False )
    ,m_bCannotSelectUnfiltered( true )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaui", "Ocke.Janssen@sun.com", "SbaXDataBrowserController::SbaXDataBrowserController" );
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
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaui", "Ocke.Janssen@sun.com", "SbaXDataBrowserController::~SbaXDataBrowserController" );
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
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaui", "Ocke.Janssen@sun.com", "SbaXDataBrowserController::startFrameListening" );
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
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaui", "Ocke.Janssen@sun.com", "SbaXDataBrowserController::stopFrameListening" );
    SbaXDataBrowserController_Base::stopFrameListening( _rxFrame );

    Reference< XFrameActionListener >   xAggListener;
    if ( m_xFormControllerImpl.is() )
        m_xFormControllerImpl->queryAggregation( XFrameActionListener::static_type() ) >>= xAggListener;

    if ( _rxFrame.is() && xAggListener.is() )
        _rxFrame->removeFrameActionListener( xAggListener );
}

// -----------------------------------------------------------------------------
void SbaXDataBrowserController::onStartLoading( const Reference< XLoadable >& _rxLoadable )
{
    m_bLoadCanceled = sal_False;
    m_bCannotSelectUnfiltered = false;

    Reference< XWarningsSupplier > xWarnings( _rxLoadable, UNO_QUERY );
    if ( xWarnings.is() )
        xWarnings->clearWarnings();
}

// -----------------------------------------------------------------------------
void SbaXDataBrowserController::impl_checkForCannotSelectUnfiltered( const SQLExceptionInfo& _rError )
{
    ::connectivity::SQLError aError( getORB() );
    ::connectivity::ErrorCode nErrorCode( aError.getErrorCode( ErrorCondition::DATA_CANNOT_SELECT_UNFILTERED ) );
    if ( ((const SQLException*)_rError)->ErrorCode == nErrorCode )
    {
        m_bCannotSelectUnfiltered = true;
        InvalidateFeature( ID_BROWSER_FILTERCRIT );
    }
}

// -----------------------------------------------------------------------------
sal_Bool SbaXDataBrowserController::reloadForm( const Reference< XLoadable >& _rxLoadable )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaui", "Ocke.Janssen@sun.com", "SbaXDataBrowserController::reloadForm" );
    WaitObject aWO(getBrowserView());

    onStartLoading( _rxLoadable );

    FormErrorHelper aReportError(this);
    if (_rxLoadable->isLoaded())
        _rxLoadable->reload();
    else
        _rxLoadable->load();

    m_xParser.clear();
    const Reference< XPropertySet >  xFormSet(getRowSet(), UNO_QUERY);
    if (::comphelper::getBOOL(xFormSet->getPropertyValue(PROPERTY_ESCAPE_PROCESSING)))
        xFormSet->getPropertyValue(PROPERTY_SINGLESELECTQUERYCOMPOSER) >>= m_xParser;
#if 0
    {
        const Reference< XPropertySet > xRowSetProps( getRowSet(), UNO_QUERY );
        const Reference< XSingleSelectQueryAnalyzer > xAnalyzer( xRowSetProps->getPropertyValue( PROPERTY_SINGLESELECTQUERYCOMPOSER ), UNO_QUERY );
        if ( xAnalyzer.is() )
        {
            const Reference< XIndexAccess > xOrderColumns( xAnalyzer->getOrderColumns(), UNO_SET_THROW );
            const sal_Int32 nOrderColumns( xOrderColumns->getCount() );
            for ( sal_Int32 c=0; c<nOrderColumns; ++c )
            {
                const Reference< XPropertySet > xOrderColumn( xOrderColumns->getByIndex(c), UNO_QUERY_THROW );
                ::rtl::OUString sColumnName;
                OSL_VERIFY( xOrderColumn->getPropertyValue( PROPERTY_NAME ) >>= sColumnName);
                ::rtl::OUString sTableName;
                OSL_VERIFY( xOrderColumn->getPropertyValue( PROPERTY_TABLENAME ) >>= sTableName);
                (void)sColumnName;
                (void)sTableName;
            }
        }
    }
#endif

    Reference< XWarningsSupplier > xWarnings( _rxLoadable, UNO_QUERY );
    if ( xWarnings.is() )
    {
        try
        {
            SQLExceptionInfo aInfo( xWarnings->getWarnings() );
            if ( aInfo.isValid() )
            {
                showError( aInfo );
                impl_checkForCannotSelectUnfiltered( aInfo );
            }
        }
        catch(const SQLException& e)
        {
            (void)e;
        }
    }

    return _rxLoadable->isLoaded();
}

// -----------------------------------------------------------------------------
void SbaXDataBrowserController::initFormatter()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaui", "Ocke.Janssen@sun.com", "SbaXDataBrowserController::initFormatter" );
    // ---------------------------------------------------------------
    // create a formatter working with the connections format supplier
    Reference< ::com::sun::star::util::XNumberFormatsSupplier >  xSupplier(::dbtools::getNumberFormats(::dbtools::getConnection(m_xRowSet), sal_True,getORB()));

    if(xSupplier.is())
    {
        // create a new formatter
        m_xFormatter = Reference< util::XNumberFormatter > (
            util::NumberFormatter::create(comphelper::getComponentContext(getORB())), UNO_QUERY_THROW);
        m_xFormatter->attachNumberFormatsSupplier(xSupplier);
    }
    else // clear the formatter
        m_xFormatter = NULL;
}
// -----------------------------------------------------------------------------
void SbaXDataBrowserController::describeSupportedFeatures()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaui", "Ocke.Janssen@sun.com", "SbaXDataBrowserController::describeSupportedFeatures" );
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
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaui", "Ocke.Janssen@sun.com", "SbaXDataBrowserController::Construct" );
    // ---------------------------------------------
    // create/initialize the form and the grid model
    m_xRowSet = CreateForm();
    if (!m_xRowSet.is())
        return sal_False;

    m_xColumnsSupplier.set(m_xRowSet,UNO_QUERY);
    m_xLoadable.set(m_xRowSet,UNO_QUERY);

    Reference< XPropertySet > xFormProperties( m_xRowSet, UNO_QUERY );
    if ( !InitializeForm( xFormProperties ) )
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
    setView( * new UnoDataBrowserView( pParent, *this, getORB() ) );
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
        OSL_FAIL("SbaXDataBrowserController::Construct : the construction of UnoDataBrowserView failed !");
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
    OSL_ENSURE(pVclGrid, "SbaXDataBrowserController::Construct : have no VCL control !");
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
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaui", "Ocke.Janssen@sun.com", "SbaXDataBrowserController::LoadForm" );
    reloadForm( m_xLoadable );
    return sal_True;
}
//------------------------------------------------------------------------------
void SbaXDataBrowserController::AddColumnListener(const Reference< XPropertySet > & /*xCol*/)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaui", "Ocke.Janssen@sun.com", "SbaXDataBrowserController::AddColumnListener" );
    // we're not interested in any column properties ...
}

//------------------------------------------------------------------------------
void SbaXDataBrowserController::RemoveColumnListener(const Reference< XPropertySet > & /*xCol*/)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaui", "Ocke.Janssen@sun.com", "SbaXDataBrowserController::RemoveColumnListener" );
}
//------------------------------------------------------------------------------
Reference< XRowSet >  SbaXDataBrowserController::CreateForm()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaui", "Ocke.Janssen@sun.com", "SbaXDataBrowserController::CreateForm" );
    return Reference< XRowSet > (getORB()->createInstance(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.form.component.Form"))), UNO_QUERY);
}

//------------------------------------------------------------------------------
Reference< ::com::sun::star::form::XFormComponent >  SbaXDataBrowserController::CreateGridModel()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaui", "Ocke.Janssen@sun.com", "SbaXDataBrowserController::CreateGridModel" );
    return Reference< ::com::sun::star::form::XFormComponent > (getORB()->createInstance(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.form.component.GridControl"))), UNO_QUERY);
}

// -------------------------------------------------------------------------
void SbaXDataBrowserController::addModelListeners(const Reference< ::com::sun::star::awt::XControlModel > & _xGridControlModel)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaui", "Ocke.Janssen@sun.com", "SbaXDataBrowserController::addModelListeners" );
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
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaui", "Ocke.Janssen@sun.com", "SbaXDataBrowserController::removeModelListeners" );
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
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaui", "Ocke.Janssen@sun.com", "SbaXDataBrowserController::addControlListeners" );
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
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaui", "Ocke.Janssen@sun.com", "SbaXDataBrowserController::removeControlListeners" );
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
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaui", "Ocke.Janssen@sun.com", "SbaXDataBrowserController::focusGained" );
    // notify our activate listeners (registered on the form controller aggregate)
    EventObject aEvt(*this);
    ::cppu::OInterfaceIteratorHelper aIter(m_pFormControllerImpl->m_aActivateListeners);
    while (aIter.hasMoreElements())
        static_cast<XFormControllerListener*>(aIter.next())->formActivated(aEvt);
}

//------------------------------------------------------------------
void SAL_CALL SbaXDataBrowserController::focusLost(const FocusEvent& e) throw( RuntimeException )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaui", "Ocke.Janssen@sun.com", "SbaXDataBrowserController::focusLost" );
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
        OSL_FAIL("SbaXDataBrowserController::focusLost : why is my control not commitable ?");
}

// -------------------------------------------------------------------------
void SbaXDataBrowserController::disposingGridControl(const ::com::sun::star::lang::EventObject& /*Source*/)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaui", "Ocke.Janssen@sun.com", "SbaXDataBrowserController::disposingGridControl" );
    removeControlListeners(getBrowserView()->getGridControl());
}

// -------------------------------------------------------------------------
void SbaXDataBrowserController::disposingGridModel(const ::com::sun::star::lang::EventObject& /*Source*/)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaui", "Ocke.Janssen@sun.com", "SbaXDataBrowserController::disposingGridModel" );
    removeModelListeners(getControlModel());
}

// -------------------------------------------------------------------------
void SbaXDataBrowserController::disposingFormModel(const ::com::sun::star::lang::EventObject& Source)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaui", "Ocke.Janssen@sun.com", "SbaXDataBrowserController::disposingFormModel" );
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
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaui", "Ocke.Janssen@sun.com", "SbaXDataBrowserController::disposingColumnModel" );
    RemoveColumnListener(Reference< XPropertySet > (Source.Source, UNO_QUERY));
}

// -------------------------------------------------------------------------
void SbaXDataBrowserController::disposing(const EventObject& Source) throw( RuntimeException )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaui", "Ocke.Janssen@sun.com", "SbaXDataBrowserController::disposing" );
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
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaui", "Ocke.Janssen@sun.com", "SbaXDataBrowserController::setIdentifier" );
    ::osl::MutexGuard aGuard( getMutex() );
    m_sModuleIdentifier = _Identifier;
}

// -----------------------------------------------------------------------
::rtl::OUString SAL_CALL SbaXDataBrowserController::getIdentifier(  ) throw (RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaui", "Ocke.Janssen@sun.com", "SbaXDataBrowserController::getIdentifier" );
    ::osl::MutexGuard aGuard( getMutex() );
    return m_sModuleIdentifier;
}

// -----------------------------------------------------------------------
void SbaXDataBrowserController::propertyChange(const PropertyChangeEvent& evt) throw ( RuntimeException )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaui", "Ocke.Janssen@sun.com", "SbaXDataBrowserController::propertyChange" );
    Reference< XPropertySet >  xSource(evt.Source, UNO_QUERY);
    if (!xSource.is())
        return;

    SolarMutexGuard aGuard;
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


    if (evt.PropertyName.equals(PROPERTY_FILTER))
    {
        InvalidateFeature(ID_BROWSER_REMOVEFILTER);
    }
    else if (evt.PropertyName.equals(PROPERTY_HAVING_CLAUSE))
    {
        InvalidateFeature(ID_BROWSER_REMOVEFILTER);
    }
    else if (evt.PropertyName.equals(PROPERTY_ORDER))
    {
        InvalidateFeature(ID_BROWSER_REMOVEFILTER);
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
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaui", "Ocke.Janssen@sun.com", "SbaXDataBrowserController::modified" );
    setCurrentModified( sal_True );
}

// -----------------------------------------------------------------------
void SbaXDataBrowserController::elementInserted(const ::com::sun::star::container::ContainerEvent& evt) throw( RuntimeException )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaui", "Ocke.Janssen@sun.com", "SbaXDataBrowserController::elementInserted" );
    OSL_ENSURE(Reference< XInterface >(evt.Source, UNO_QUERY).get() == Reference< XInterface >(getControlModel(), UNO_QUERY).get(),
        "SbaXDataBrowserController::elementInserted: where did this come from (not from the grid model)?!");
    Reference< XPropertySet >  xNewColumn(evt.Element,UNO_QUERY);
    if ( xNewColumn.is() )
        AddColumnListener(xNewColumn);
}

// -----------------------------------------------------------------------
void SbaXDataBrowserController::elementRemoved(const ::com::sun::star::container::ContainerEvent& evt) throw( RuntimeException )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaui", "Ocke.Janssen@sun.com", "SbaXDataBrowserController::elementRemoved" );
    OSL_ENSURE(Reference< XInterface >(evt.Source, UNO_QUERY).get() == Reference< XInterface >(getControlModel(), UNO_QUERY).get(),
        "SbaXDataBrowserController::elementRemoved: where did this come from (not from the grid model)?!");
    Reference< XPropertySet >  xOldColumn(evt.Element,UNO_QUERY);
    if ( xOldColumn.is() )
        RemoveColumnListener(xOldColumn);
}

// -----------------------------------------------------------------------
void SbaXDataBrowserController::elementReplaced(const ::com::sun::star::container::ContainerEvent& evt) throw( RuntimeException )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaui", "Ocke.Janssen@sun.com", "SbaXDataBrowserController::elementReplaced" );
    OSL_ENSURE(Reference< XInterface >(evt.Source, UNO_QUERY).get() == Reference< XInterface >(getControlModel(), UNO_QUERY).get(),
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
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaui", "Ocke.Janssen@sun.com", "SbaXDataBrowserController::suspend" );
    OSL_ENSURE(m_nPendingLoadFinished == 0, "SbaXDataBrowserController::suspend : there shouldn't be a pending load !");

    m_aAsyncGetCellFocus.CancelCall();
    m_aAsyncDisplayError.CancelCall();
    m_aAsyncInvalidateAll.CancelCall();

    sal_Bool bSuccess = SaveModified();
    return bSuccess;
}
// -----------------------------------------------------------------------
void SbaXDataBrowserController::disposing()
{
    // the base class
    SbaXDataBrowserController_Base::OGenericUnoController::disposing();

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
        clearView();
    }

    if(m_aInvalidateClipboard.IsActive())
        m_aInvalidateClipboard.Stop();

    // dispose the row set
    try
    {
        ::comphelper::disposeComponent(m_xRowSet);

        m_xRowSet           = NULL;
        m_xColumnsSupplier  = NULL;
        m_xLoadable         = NULL;
    }
    catch(Exception&)
    {
        DBG_UNHANDLED_EXCEPTION();
    }
    m_xParser.clear();
        // don't dispose, just reset - it's owned by the RowSet
}
//------------------------------------------------------------------------------
void SbaXDataBrowserController::frameAction(const ::com::sun::star::frame::FrameActionEvent& aEvent) throw( RuntimeException )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaui", "Ocke.Janssen@sun.com", "SbaXDataBrowserController::frameAction" );
    ::osl::MutexGuard aGuard( getMutex() );

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
IMPL_LINK( SbaXDataBrowserController, OnAsyncDisplayError, void*, /* _pNotInterestedIn */ )
{
    if ( m_aCurrentError.isValid() )
    {
        OSQLMessageBox aDlg( getBrowserView(), m_aCurrentError );
        aDlg.Execute();
    }
    return 0L;
}

//------------------------------------------------------------------------------
void SbaXDataBrowserController::errorOccured(const ::com::sun::star::sdb::SQLErrorEvent& aEvent) throw( RuntimeException )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaui", "Ocke.Janssen@sun.com", "SbaXDataBrowserController::errorOccurred" );
    ::osl::MutexGuard aGuard( getMutex() );

    SQLExceptionInfo aInfo( aEvent.Reason );
    if ( !aInfo.isValid() )
        return;

    if ( m_nFormActionNestingLevel )
    {
        OSL_ENSURE( !m_aCurrentError.isValid(), "SbaXDataBrowserController::errorOccurred: can handle one error per transaction only!" );
        m_aCurrentError = aInfo;
    }
    else
    {
        m_aCurrentError = aInfo;
        m_aAsyncDisplayError.Call();
    }
}

//------------------------------------------------------------------------------
sal_Bool SbaXDataBrowserController::approveParameter(const ::com::sun::star::form::DatabaseParameterEvent& aEvent) throw( RuntimeException )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaui", "Ocke.Janssen@sun.com", "SbaXDataBrowserController::approveParameter" );
    if (aEvent.Source != getRowSet())
    {
        // not my data source -> allow anything
        OSL_FAIL("SbaXDataBrowserController::approveParameter : invalid event source !");
        return sal_True;
    }

    Reference< ::com::sun::star::container::XIndexAccess >  xParameters = aEvent.Parameters;
    SolarMutexGuard aSolarGuard;

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
        Reference< XInteractionHandler2 > xHandler( InteractionHandler::createWithParent(comphelper::getComponentContext(getORB()), 0) );
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
            OSL_FAIL("SbaXDataBrowserController::approveParameter: the InteractionHandler returned nonsense!");
            setLoadingCancelled();
            return sal_False;
        }
        const PropertyValue* pFinalValues = aFinalValues.getConstArray();
        for (sal_Int32 i=0; i<aFinalValues.getLength(); ++i, ++pFinalValues)
        {
            Reference< XPropertySet > xParam;
            ::cppu::extractInterface(xParam, aRequest.Parameters->getByIndex(i));
            OSL_ENSURE(xParam.is(), "SbaXDataBrowserController::approveParameter: one of the parameters is no property set!");
            if (xParam.is())
            {
#ifdef DBG_UTIL
                ::rtl::OUString sName;
                xParam->getPropertyValue(PROPERTY_NAME) >>= sName;
                OSL_ENSURE(sName.equals(pFinalValues->Name), "SbaXDataBrowserController::approveParameter: suspicious value names!");
#endif
                try { xParam->setPropertyValue(PROPERTY_VALUE, pFinalValues->Value); }
                catch(Exception&)
                {
                    OSL_FAIL("SbaXDataBrowserController::approveParameter: setting one of the properties failed!");
                }
            }
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    return sal_True;
}


//------------------------------------------------------------------------------
sal_Bool SbaXDataBrowserController::approveReset(const ::com::sun::star::lang::EventObject& /*rEvent*/) throw( RuntimeException )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaui", "Ocke.Janssen@sun.com", "SbaXDataBrowserController::approveReset" );
    return sal_True;
}

//------------------------------------------------------------------------------
void SbaXDataBrowserController::resetted(const ::com::sun::star::lang::EventObject& rEvent) throw( RuntimeException )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaui", "Ocke.Janssen@sun.com", "SbaXDataBrowserController::resetted" );
    OSL_ENSURE(rEvent.Source == getControlModel(), "SbaXDataBrowserController::resetted : where did this come from ?");
    (void)rEvent;
    setCurrentModified( sal_False );
}

//------------------------------------------------------------------------------
sal_Bool SbaXDataBrowserController::confirmDelete(const ::com::sun::star::sdb::RowChangeEvent& /*aEvent*/) throw( RuntimeException )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaui", "Ocke.Janssen@sun.com", "SbaXDataBrowserController::confirmDelete" );
    if (QueryBox(getBrowserView(), ModuleRes(QUERY_BRW_DELETE_ROWS)).Execute() != RET_YES)
        return sal_False;

    return sal_True;
}
//------------------------------------------------------------------------------
FeatureState SbaXDataBrowserController::GetState(sal_uInt16 nId) const
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaui", "Ocke.Janssen@sun.com", "SbaXDataBrowserController::GetState" );
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
                {
                    aReturn.bEnabled = false;
                    return aReturn;
                }
                // any filter or sort order set ?
                aReturn.bEnabled = m_xParser->getFilter().getLength() || m_xParser->getHavingClause().getLength() || m_xParser->getOrder().getLength();
                return aReturn;
        }
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
                    sal_Bool bInsertPrivilege = ( m_nRowSetPrivileges & Privilege::INSERT) != 0;
                    sal_Bool bAllowInsertions = sal_True;
                    try
                    {
                        Reference< XPropertySet > xRowSetProps( getRowSet(), UNO_QUERY_THROW );
                        OSL_VERIFY( xRowSetProps->getPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AllowInserts")) ) >>= bAllowInsertions );
                    }
                    catch( const Exception& )
                    {
                        DBG_UNHANDLED_EXCEPTION();
                    }
                    aReturn.bEnabled = bInsertPrivilege && bAllowInsertions;
                }
                break;
            case SID_FM_DELETEROWS:
                {
                    // check if it is available
                    sal_Bool bDeletePrivilege = ( m_nRowSetPrivileges & Privilege::INSERT) != 0;
                    sal_Bool bAllowDeletions = sal_True;
                    sal_Int32 nRowCount = 0;
                    sal_Bool bInsertionRow = sal_False;
                    try
                    {
                        Reference< XPropertySet > xRowSetProps( getRowSet(), UNO_QUERY_THROW );
                        OSL_VERIFY( xRowSetProps->getPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AllowDeletes")) ) >>= bAllowDeletions );
                        OSL_VERIFY( xRowSetProps->getPropertyValue( PROPERTY_ROWCOUNT ) >>= nRowCount );
                        OSL_VERIFY( xRowSetProps->getPropertyValue( PROPERTY_ISNEW ) >>= bInsertionRow );
                    }
                    catch( const Exception& )
                    {
                        DBG_UNHANDLED_EXCEPTION();
                    }
                    aReturn.bEnabled = bDeletePrivilege && bAllowDeletions && ( nRowCount != 0 ) && !bInsertionRow;
                }
                break;

            case ID_BROWSER_COPY:
                if ( getBrowserView()->getVclControl()->GetSelectRowCount() )
                {
                    aReturn.bEnabled = m_aCurrentFrame.isActive();
                    break;
                }
                // run through
            case ID_BROWSER_PASTE:
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
                const Reference< XPropertySet >  xFormSet(getRowSet(), UNO_QUERY);
                if ( !::comphelper::getBOOL(xFormSet->getPropertyValue(PROPERTY_ESCAPE_PROCESSING)) || !m_xParser.is() )
                    break;

                Reference< XPropertySet >  xCurrentField = getBoundField();
                if (!xCurrentField.is())
                    break;

                aReturn.bEnabled = ::comphelper::getBOOL(xCurrentField->getPropertyValue(PROPERTY_ISSEARCHABLE));
                const Reference< XRowSet > xRow = getRowSet();
                aReturn.bEnabled =  aReturn.bEnabled
                                &&  xRow.is()
                                &&  !xRow->isBeforeFirst()
                                &&  !xRow->isAfterLast()
                                &&  !xRow->rowDeleted()
                                &&  ( ::comphelper::getINT32( xFormSet->getPropertyValue( PROPERTY_ROWCOUNT ) ) != 0 );
            }
            break;

            case ID_BROWSER_FILTERCRIT:
                if ( m_bCannotSelectUnfiltered && m_xParser.is() )
                {
                    aReturn.bEnabled = sal_True;
                    break;
                }
                // no break
            case ID_BROWSER_ORDERCRIT:
                {
                    const Reference< XPropertySet >  xFormSet(getRowSet(), UNO_QUERY);
                    if ( !::comphelper::getBOOL(xFormSet->getPropertyValue(PROPERTY_ESCAPE_PROCESSING)) || !m_xParser.is() )
                        break;

                    aReturn.bEnabled =  getRowSet().is()
                                    &&  ( ::comphelper::getINT32( xFormSet->getPropertyValue( PROPERTY_ROWCOUNT ) ) != 0 );
                }
                break;

            case ID_BROWSER_REFRESH:
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
                sal_Bool bInsertAllowedAndPossible = ((nDataSourcePrivileges & ::com::sun::star::sdbcx::Privilege::INSERT) != 0) && ::comphelper::getBOOL(xDataSourceSet->getPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AllowInserts"))));
                sal_Bool bUpdateAllowedAndPossible = ((nDataSourcePrivileges & ::com::sun::star::sdbcx::Privilege::UPDATE) != 0) && ::comphelper::getBOOL(xDataSourceSet->getPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AllowUpdates"))));
                sal_Bool bDeleteAllowedAndPossible = ((nDataSourcePrivileges & ::com::sun::star::sdbcx::Privilege::DELETE) != 0) && ::comphelper::getBOOL(xDataSourceSet->getPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AllowDeletes"))));
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
                if ( !(aFilter.isEmpty() && aHaving.isEmpty()) )
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
void SbaXDataBrowserController::applyParserOrder(const ::rtl::OUString& _rOldOrder,const Reference< XSingleSelectQueryComposer >& _xParser)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaui", "Ocke.Janssen@sun.com", "SbaXDataBrowserController::applyParserOrder" );
    Reference< XPropertySet > xFormSet(getRowSet(), UNO_QUERY);
    if (!m_xLoadable.is())
    {
        OSL_FAIL("SbaXDataBrowserController::applyParserOrder: invalid row set!");
        return;
    }

    sal_uInt16 nPos = getCurrentColumnPosition();
    sal_Bool bSuccess = sal_False;
    try
    {
        xFormSet->setPropertyValue(PROPERTY_ORDER, makeAny(_xParser->getOrder()));
        bSuccess = reloadForm(m_xLoadable);
    }
    catch(Exception&)
    {
    }

    if (!bSuccess)
    {
        xFormSet->setPropertyValue(PROPERTY_ORDER, makeAny(_rOldOrder));

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
void SbaXDataBrowserController::applyParserFilter(const ::rtl::OUString& _rOldFilter, sal_Bool _bOldFilterApplied,const ::rtl::OUString& _sOldHaving,const Reference< XSingleSelectQueryComposer >& _xParser)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaui", "Ocke.Janssen@sun.com", "SbaXDataBrowserController::applyParserFilter" );
    Reference< XPropertySet >  xFormSet(getRowSet(), UNO_QUERY);
    if (!m_xLoadable.is())
    {
        OSL_FAIL("SbaXDataBrowserController::applyParserFilter: invalid row set!");
        return;
    }

    sal_uInt16 nPos = getCurrentColumnPosition();

    sal_Bool bSuccess = sal_False;
    try
    {
        FormErrorHelper aError(this);
        xFormSet->setPropertyValue(PROPERTY_FILTER, makeAny(_xParser->getFilter()));
        xFormSet->setPropertyValue(PROPERTY_HAVING_CLAUSE, makeAny(_xParser->getHavingClause()));
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
Reference< XSingleSelectQueryComposer > SbaXDataBrowserController::createParser_nothrow()
{
    Reference< XSingleSelectQueryComposer > xComposer;
    try
    {
        const Reference< XPropertySet > xRowSetProps( getRowSet(), UNO_QUERY_THROW );
        const Reference< XMultiServiceFactory > xFactory(
            xRowSetProps->getPropertyValue( PROPERTY_ACTIVE_CONNECTION ), UNO_QUERY_THROW );
        xComposer.set( xFactory->createInstance( SERVICE_NAME_SINGLESELECTQUERYCOMPOSER ), UNO_QUERY_THROW );

        ::rtl::OUString sActiveCommand;
        OSL_VERIFY( xRowSetProps->getPropertyValue( PROPERTY_ACTIVECOMMAND ) >>= sActiveCommand );
        if ( !sActiveCommand.isEmpty() )
        {
            xComposer->setElementaryQuery( sActiveCommand );
        }
        else
        {
            ::rtl::OUString sCommand;
            OSL_VERIFY( xRowSetProps->getPropertyValue( PROPERTY_COMMAND ) >>= sCommand );
            sal_Int32 nCommandType = CommandType::COMMAND;
            OSL_VERIFY( xRowSetProps->getPropertyValue( PROPERTY_COMMAND_TYPE ) >>= nCommandType );
            xComposer->setCommand( sCommand, nCommandType );
        }

        ::rtl::OUString sFilter;
        OSL_VERIFY( xRowSetProps->getPropertyValue( PROPERTY_FILTER ) >>= sFilter );
        xComposer->setFilter( sFilter );

        ::rtl::OUString sHavingClause;
        OSL_VERIFY( xRowSetProps->getPropertyValue( PROPERTY_HAVING_CLAUSE ) >>= sHavingClause );
        xComposer->setHavingClause( sHavingClause );

        ::rtl::OUString sOrder;
        OSL_VERIFY( xRowSetProps->getPropertyValue( PROPERTY_ORDER ) >>= sOrder );
        xComposer->setOrder( sOrder );
    }
    catch ( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
    return xComposer;
}
//------------------------------------------------------------------------------
void SbaXDataBrowserController::ExecuteFilterSortCrit(sal_Bool bFilter)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaui", "Ocke.Janssen@sun.com", "SbaXDataBrowserController::ExecuteFilterSortCrit" );
    if (!SaveModified())
        return;

    Reference< XPropertySet >  xFormSet(getRowSet(), UNO_QUERY);

    const ::rtl::OUString sOldVal = bFilter ? m_xParser->getFilter() : m_xParser->getOrder();
    const ::rtl::OUString sOldHaving = m_xParser->getHavingClause();
    Reference< XSingleSelectQueryComposer > xParser = createParser_nothrow();
    try
    {
        Reference< ::com::sun::star::sdbcx::XColumnsSupplier> xSup = getColumnsSupplier();
        Reference< XConnection> xCon(xFormSet->getPropertyValue(PROPERTY_ACTIVE_CONNECTION),UNO_QUERY);
        if(bFilter)
        {
            DlgFilterCrit aDlg( getBrowserView(), getORB(), xCon, xParser, xSup->getColumns() );
            String aFilter;
            if ( !aDlg.Execute() )
                return; // if so we don't need to update the grid
            aDlg.BuildWherePart();
        }
        else
        {
            DlgOrderCrit aDlg( getBrowserView(),xCon,xParser,xSup->getColumns() );
            String aOrder;
            if(!aDlg.Execute())
            {
                return; // if so we don't need to actualize the grid
            }
            aDlg.BuildOrderPart();
        }
    }
    catch(const SQLException& )
    {
        SQLExceptionInfo aError( ::cppu::getCaughtException() );
        showError( aError );
        return;
    }
    catch(Exception&)
    {
        return;
    }

    ::rtl::OUString sNewVal = bFilter ? xParser->getFilter() : xParser->getOrder();
    sal_Bool bOldFilterApplied(sal_False);
    if (bFilter)
    {
        try { bOldFilterApplied = ::comphelper::getBOOL(xFormSet->getPropertyValue(PROPERTY_APPLYFILTER)); } catch(Exception&) { } ;
    }

    ::rtl::OUString sNewHaving = xParser->getHavingClause();
    if ( sOldVal.equals(sNewVal) && (!bFilter || sOldHaving.equals(sNewHaving)) )
        // nothing to be done
        return;

    if (bFilter)
        applyParserFilter(sOldVal, bOldFilterApplied,sOldHaving,xParser);
    else
        applyParserOrder(sOldVal,xParser);

    ::comphelper::disposeComponent(xParser);
}

//------------------------------------------------------------------------------
void SbaXDataBrowserController::ExecuteSearch()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaui", "Ocke.Janssen@sun.com", "SbaXDataBrowserController::ExecuteSearch" );
    // calculate the control source of the active field
    Reference< ::com::sun::star::form::XGrid >  xGrid(getBrowserView()->getGridControl(), UNO_QUERY);
    OSL_ENSURE(xGrid.is(), "SbaXDataBrowserController::ExecuteSearch : the control should have an ::com::sun::star::form::XGrid interface !");

    Reference< ::com::sun::star::form::XGridPeer >  xGridPeer(getBrowserView()->getGridControl()->getPeer(), UNO_QUERY);
    Reference< ::com::sun::star::container::XIndexContainer >  xColumns = xGridPeer->getColumns();
    OSL_ENSURE(xGridPeer.is() && xColumns.is(), "SbaXDataBrowserController::ExecuteSearch : invalid peer !");

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
        sInitialText = aInitialText;

    // prohibit the synchronization of the grid's display with the cursor's position
    Reference< XPropertySet >  xModelSet(getControlModel(), UNO_QUERY);
    OSL_ENSURE(xModelSet.is(), "SbaXDataBrowserController::ExecuteSearch : no model set ?!");
    xModelSet->setPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DisplayIsSynchron")), ::comphelper::makeBoolAny(sal_Bool(sal_False)));
    xModelSet->setPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AlwaysShowCursor")), ::comphelper::makeBoolAny(sal_Bool(sal_True)));
    xModelSet->setPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CursorColor")), makeAny(sal_Int32(COL_LIGHTRED)));

    Reference< ::com::sun::star::util::XNumberFormatsSupplier >  xNFS(::dbtools::getNumberFormats(::dbtools::getConnection(m_xRowSet), sal_True,getORB()));

    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    AbstractFmSearchDialog* pDialog = NULL;
    if ( pFact )
    {
        ::std::vector< String > aContextNames;
        aContextNames.push_back( rtl::OUString("Standard") );
        pDialog = pFact->CreateFmSearchDialog(getBrowserView(), sInitialText, aContextNames, 0, LINK(this, SbaXDataBrowserController, OnSearchContextRequest));
    }
    OSL_ENSURE( pDialog, "SbaXDataBrowserController::ExecuteSearch: could not get the search dialog!" );
    if ( pDialog )
    {
        pDialog->SetActiveField( sActiveField );
        pDialog->SetFoundHandler( LINK( this, SbaXDataBrowserController, OnFoundData ) );
        pDialog->SetCanceledNotFoundHdl( LINK( this, SbaXDataBrowserController, OnCanceledNotFound ) );
        pDialog->Execute();
        delete pDialog;
    }

    // restore the grid's normal operating state
    xModelSet->setPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DisplayIsSynchron")), ::comphelper::makeBoolAny(sal_Bool(sal_True)));
    xModelSet->setPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AlwaysShowCursor")), ::comphelper::makeBoolAny(sal_Bool(sal_False)));
    xModelSet->setPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CursorColor")), Any());
}

//------------------------------------------------------------------------------
void SbaXDataBrowserController::Execute(sal_uInt16 nId, const Sequence< PropertyValue >& _rArgs)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaui", "Ocke.Janssen@sun.com", "SbaXDataBrowserController::Execute" );
    sal_Bool bSortUp = sal_True;

    switch (nId)
    {
        default:
            SbaXDataBrowserController_Base::Execute( nId, _rArgs );
            return;

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
                OSL_FAIL("Exception caught!");
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
            if ( getBrowserView()->getVclControl()->GetSelectRowCount() > 0 )
            {
                getBrowserView()->getVclControl()->CopySelectedRowsToClipboard();
                break;
            }
            // run through
        case ID_BROWSER_CUT:
        case ID_BROWSER_PASTE:
        {
            CellControllerRef xCurrentController = getBrowserView()->getVclControl()->Controller();
            if (!xCurrentController.Is())
                // should be intercepted by GetState. Normally.
                // Unfortunately ID_BROWSER_PASTE is a 'fast call' slot, which means it may be executed without checking if it is
                // enabled. This would be really deadly herein if the current cell has no controller ...
                return;

            Edit& rEdit = (Edit&)xCurrentController->GetWindow();
            switch (nId)
            {
                case ID_BROWSER_CUT :       rEdit.Cut();    break;
                case SID_COPY   :           rEdit.Copy();   break;
                case ID_BROWSER_PASTE   :   rEdit.Paste();  break;
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

            Reference< XSingleSelectQueryComposer > xParser = createParser_nothrow();
            const ::rtl::OUString sOldSort = xParser->getOrder();
            sal_Bool bParserSuccess = sal_False;
            HANDLE_SQL_ERRORS(
                xParser->setOrder(::rtl::OUString()); xParser->appendOrderByColumn(xField, bSortUp),
                bParserSuccess,
                ModuleRes(SBA_BROWSER_SETTING_ORDER).toString(),
                "SbaXDataBrowserController::Execute : caught an exception while composing the new filter !"
            )

            if (bParserSuccess)
                applyParserOrder(sOldSort,xParser);
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

            Reference< XSingleSelectQueryComposer > xParser = createParser_nothrow();
            const ::rtl::OUString sOldFilter = xParser->getFilter();
            const ::rtl::OUString sOldHaving = xParser->getHavingClause();

            Reference< XPropertySet >  xFormSet(getRowSet(), UNO_QUERY);
            sal_Bool bApplied = ::comphelper::getBOOL(xFormSet->getPropertyValue(PROPERTY_APPLYFILTER));
            // do we have a filter but it's not applied ?
            // -> completely overwrite it, else append one
            if (!bApplied)
            {
                DO_SAFE( (bHaving ? xParser->setHavingClause(::rtl::OUString()) : xParser->setFilter(::rtl::OUString())), "SbaXDataBrowserController::Execute : caught an exception while resetting the new filter !" );
            }

            sal_Bool bParserSuccess = sal_False;

            const sal_Int32 nOp = SQLFilterOperator::EQUAL;

            if ( bHaving )
            {
                HANDLE_SQL_ERRORS(
                    xParser->appendHavingClauseByColumn(xField,sal_True,nOp),
                    bParserSuccess,
                    ModuleRes(SBA_BROWSER_SETTING_FILTER).toString(),
                    "SbaXDataBrowserController::Execute : caught an exception while composing the new filter !"
                )
            }
            else
            {
                HANDLE_SQL_ERRORS(
                    xParser->appendFilterByColumn(xField,sal_True,nOp),
                    bParserSuccess,
                    ModuleRes(SBA_BROWSER_SETTING_FILTER).toString(),
                    "SbaXDataBrowserController::Execute : caught an exception while composing the new filter !"
                )
            }

            if (bParserSuccess)
                applyParserFilter(sOldFilter, bApplied,sOldHaving,xParser);

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
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaui", "Ocke.Janssen@sun.com", "SbaXDataBrowserController::SaveModified" );
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
        OSL_FAIL("SbaXDataBrowserController::SaveModified : could not save the current record !");
        bResult = sal_False;
    }

    InvalidateFeature(ID_BROWSER_SAVERECORD);
    InvalidateFeature(ID_BROWSER_UNDORECORD);
    return bResult;
}

//------------------------------------------------------------------------------
sal_Bool SbaXDataBrowserController::CommitCurrent()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaui", "Ocke.Janssen@sun.com", "SbaXDataBrowserController::CommitCurrent" );
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
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaui", "Ocke.Janssen@sun.com", "SbaXDataBrowserController::setCurrentModified" );
    m_bCurrentlyModified = _bSet;
    InvalidateFeature( ID_BROWSER_SAVERECORD );
    InvalidateFeature( ID_BROWSER_UNDORECORD );
}

//------------------------------------------------------------------------------
void SbaXDataBrowserController::RowChanged()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaui", "Ocke.Janssen@sun.com", "SbaXDataBrowserController::RowChanged" );
    setCurrentModified( sal_False );
}

//------------------------------------------------------------------------------
void SbaXDataBrowserController::ColumnChanged()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaui", "Ocke.Janssen@sun.com", "SbaXDataBrowserController::ColumnChanged" );
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
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaui", "Ocke.Janssen@sun.com", "SbaXDataBrowserController::SelectionChanged" );
    // not interested in
}

//------------------------------------------------------------------------------
void SbaXDataBrowserController::CellActivated()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaui", "Ocke.Janssen@sun.com", "SbaXDataBrowserController::CellActivated" );
    m_aInvalidateClipboard.Start();
    OnInvalidateClipboard( NULL );
}

//------------------------------------------------------------------------------
void SbaXDataBrowserController::CellDeactivated()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaui", "Ocke.Janssen@sun.com", "SbaXDataBrowserController::CellDeactivated" );
    m_aInvalidateClipboard.Stop();
    OnInvalidateClipboard( NULL );
}

//------------------------------------------------------------------------------
IMPL_LINK_NOARG(SbaXDataBrowserController, OnClipboardChanged)
{
    SolarMutexGuard aGuard;
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

// -------------------------------------------------------------------------
Reference< XPropertySet >  SbaXDataBrowserController::getBoundField(sal_uInt16 nViewPos) const
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaui", "Ocke.Janssen@sun.com", "SbaXDataBrowserController::SaveData" );
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
    OSL_ENSURE(xModelColumns.is(), "SbaXDataBrowserController::OnSearchContextRequest : there is a grid control without columns !");
        // the case 'no columns' should be indicated with an empty container, I think ...
    OSL_ENSURE(xModelColumns->getCount() >= xPeerContainer->getCount(), "SbaXDataBrowserController::OnSearchContextRequest : impossible : have more view than model columns !");

    String sFieldList;
    for (sal_Int32 nViewPos=0; nViewPos<xPeerContainer->getCount(); ++nViewPos)
    {
        Reference< XInterface >  xCurrentColumn(xPeerContainer->getByIndex(nViewPos),UNO_QUERY);
        if (!xCurrentColumn.is())
            continue;

        // can we use this column control for searching ?
        if (!IsSearchableControl(xCurrentColumn))
            continue;

        sal_uInt16 nModelPos = getBrowserView()->View2ModelPos((sal_uInt16)nViewPos);
        Reference< XPropertySet >  xCurrentColModel(xModelColumns->getByIndex(nModelPos),UNO_QUERY);
        String aName = ::comphelper::getString(xCurrentColModel->getPropertyValue(PROPERTY_CONTROLSOURCE));

        sFieldList += aName;
        sFieldList += ';';

        pContext->arrFields.push_back(xCurrentColumn);
    }
    sFieldList = comphelper::string::stripEnd(sFieldList, ';');

    pContext->xCursor.set(getRowSet(),UNO_QUERY);
    pContext->strUsedFields = sFieldList;

    // if the cursor is in a mode other than STANDARD -> reset
    Reference< XPropertySet >  xCursorSet(pContext->xCursor, UNO_QUERY);
    OSL_ENSURE(xCursorSet.is() && !::comphelper::getBOOL(xCursorSet->getPropertyValue(PROPERTY_ISMODIFIED)),
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
    OSL_ENSURE(xCursor.is(), "SbaXDataBrowserController::OnFoundData : shit happens. sometimes. but this is simply impossible !");

    // move the cursor
    xCursor->moveToBookmark(pInfo->aPosition);

    // let the grid snyc it's display with the cursor
    Reference< XPropertySet >  xModelSet(getControlModel(), UNO_QUERY);
    OSL_ENSURE(xModelSet.is(), "SbaXDataBrowserController::OnFoundData : no model set ?!");
    Any aOld = xModelSet->getPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DisplayIsSynchron")));
    xModelSet->setPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DisplayIsSynchron")), ::comphelper::makeBoolAny(sal_Bool(sal_True)));
    xModelSet->setPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DisplayIsSynchron")), aOld);

    // and move to the field
    Reference< ::com::sun::star::container::XIndexAccess >  aColumnControls(getBrowserView()->getGridControl()->getPeer(), UNO_QUERY);
    sal_uInt16 nViewPos;

    for ( nViewPos = 0; nViewPos < aColumnControls->getCount(); ++nViewPos )
    {
        Reference< XInterface >  xCurrent(aColumnControls->getByIndex(nViewPos),UNO_QUERY);
        if (IsSearchableControl(xCurrent))
        {
            if (pInfo->nFieldPos)
                --pInfo->nFieldPos;
            else
                break;
        }
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
        OSL_ENSURE(xCursor.is(), "SbaXDataBrowserController::OnCanceledNotFound : shit happens. sometimes. but this is simply impossible !");
        // move the cursor
        xCursor->moveToBookmark(pInfo->aPosition);
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    try
    {
        // let the grid snyc its display with the cursor
        Reference< XPropertySet >  xModelSet(getControlModel(), UNO_QUERY);
        OSL_ENSURE(xModelSet.is(), "SbaXDataBrowserController::OnCanceledNotFound : no model set ?!");
        Any aOld = xModelSet->getPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DisplayIsSynchron")));
        xModelSet->setPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DisplayIsSynchron")), ::comphelper::makeBoolAny(sal_Bool(sal_True)));
        xModelSet->setPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DisplayIsSynchron")), aOld);
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    return 0L;
}

//------------------------------------------------------------------------------
IMPL_LINK_NOARG(SbaXDataBrowserController, OnAsyncGetCellFocus)
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
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaui", "Ocke.Janssen@sun.com", "SbaXDataBrowserController::criticalFail" );
    InvalidateAll();
    m_nRowSetPrivileges = 0;
}

//------------------------------------------------------------------------------
void SbaXDataBrowserController::LoadFinished(sal_Bool /*bWasSynch*/)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaui", "Ocke.Janssen@sun.com", "SbaXDataBrowserController::LoadFinished" );
    m_nRowSetPrivileges = 0;

    if (isValid() && !loadingCancelled())
    {
        // obtain cached values
        try
        {
            Reference< XPropertySet > xFormProps( m_xLoadable, UNO_QUERY_THROW );
            OSL_VERIFY( xFormProps->getPropertyValue( PROPERTY_PRIVILEGES ) >>= m_nRowSetPrivileges );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }

        // --------------------------------
        // switch the control to alive mode
        getBrowserView()->getGridControl()->setDesignMode(sal_False);

        // -------------------------------
        initializeParser();

        // -------------------------------
        InvalidateAll();

        m_aAsyncGetCellFocus.Call();
    }
}
// -----------------------------------------------------------------------------
void SbaXDataBrowserController::initializeParser() const
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaui", "Ocke.Janssen@sun.com", "SbaXDataBrowserController::initializeParser" );
    if ( !m_xParser.is() )
    {
        // ----------------------------------------------
        // create a parser (needed for filtering/sorting)
        try
        {
            const Reference< XPropertySet >  xFormSet(getRowSet(), UNO_QUERY);
            if (::comphelper::getBOOL(xFormSet->getPropertyValue(PROPERTY_ESCAPE_PROCESSING)))
            {   // (only if the statement isn't native)
                // (it is allowed to use the PROPERTY_ISPASSTHROUGH : _after_ loading a form it is valid)
                xFormSet->getPropertyValue(PROPERTY_SINGLESELECTQUERYCOMPOSER) >>= m_xParser;
            }
        }
        catch(Exception&)
        {
            DBG_UNHANDLED_EXCEPTION();
            m_xParser = NULL;
            // no further handling, we ignore the error
        }
    }
}
//------------------------------------------------------------------------------
void SbaXDataBrowserController::loaded(const EventObject& /*aEvent*/) throw( RuntimeException )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaui", "Ocke.Janssen@sun.com", "SbaXDataBrowserController::loaded" );
    // not interested in
    // we're loading within an separate thread and have a handling  for it's "finished event"
}

//------------------------------------------------------------------------------
void SbaXDataBrowserController::unloading(const EventObject& /*aEvent*/) throw( RuntimeException )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaui", "Ocke.Janssen@sun.com", "SbaXDataBrowserController::unloading" );
    // not interested in
}

//------------------------------------------------------------------------------
void SbaXDataBrowserController::unloaded(const EventObject& /*aEvent*/) throw( RuntimeException )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaui", "Ocke.Janssen@sun.com", "SbaXDataBrowserController::unloaded" );
    m_xParser.clear();
    InvalidateAll();
        // do this asynchronously, there are other listeners reacting on this message ...
        // (it's a little hack : the grid columns are listening to this event, too, and their bound field may
        // change as a reaction on that event. as we have no chance to be notified of this change (which is
        // the one we're interested in) we give them time to do what they want to before invalidating our
        // bound-field-dependent slots ....
}

//------------------------------------------------------------------------------
void SbaXDataBrowserController::reloading(const EventObject& /*aEvent*/) throw( RuntimeException )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaui", "Ocke.Janssen@sun.com", "SbaXDataBrowserController::reloading" );
    // not interested in
}

//------------------------------------------------------------------------------
void SbaXDataBrowserController::reloaded(const EventObject& /*aEvent*/) throw( RuntimeException )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaui", "Ocke.Janssen@sun.com", "SbaXDataBrowserController::reloaded" );
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
    if ( !m_nFormActionNestingLevel )
        // first action -> reset
        m_aCurrentError.clear();

    ++m_nFormActionNestingLevel;
}

//------------------------------------------------------------------------------
void SbaXDataBrowserController::leaveFormAction()
{
    OSL_ENSURE( m_nFormActionNestingLevel > 0, "SbaXDataBrowserController::leaveFormAction : invalid call !" );
    if ( --m_nFormActionNestingLevel > 0 )
        return;

    if ( !m_aCurrentError.isValid() )
        return;

    m_aAsyncDisplayError.Call();
}

// -------------------------------------------------------------------------
sal_Bool SbaXDataBrowserController::isLoaded() const
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaui", "Ocke.Janssen@sun.com", "SbaXDataBrowserController::isLoaded" );
    return m_xLoadable.is() && m_xLoadable->isLoaded();
}

// -------------------------------------------------------------------------
sal_Bool SbaXDataBrowserController::isValidCursor() const
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaui", "Ocke.Janssen@sun.com", "SbaXDataBrowserController::isValidCursor" );
    if (!m_xColumnsSupplier.is())
        return sal_False;
    Reference< ::com::sun::star::container::XNameAccess >  xCols = m_xColumnsSupplier->getColumns();
    if (!xCols.is() || !xCols->hasElements())
        return sal_False;

    sal_Bool bIsValid = !(m_xRowSet->isBeforeFirst() || m_xRowSet->isAfterLast());
    if ( !bIsValid )
    {
        Reference<XPropertySet> xProp(m_xRowSet,UNO_QUERY);
        bIsValid = ::cppu::any2bool(xProp->getPropertyValue(PROPERTY_ISNEW));
        if ( !bIsValid )
        {
            bIsValid = m_xParser.is();
        }
    }
    return bIsValid;
}

// -----------------------------------------------------------------------------
sal_Int16 SbaXDataBrowserController::getCurrentColumnPosition()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaui", "Ocke.Janssen@sun.com", "SbaXDataBrowserController::getCurrentColumnPosition" );
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
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaui", "Ocke.Janssen@sun.com", "SbaXDataBrowserController::setCurrentColumnPosition" );
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
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaui", "Ocke.Janssen@sun.com", "SbaXDataBrowserController::BeforeDrop" );
    Reference< ::com::sun::star::sdb::XSQLErrorBroadcaster >  xFormError(getRowSet(), UNO_QUERY);
    if (xFormError.is())
        xFormError->removeSQLErrorListener((::com::sun::star::sdb::XSQLErrorListener*)this);
}
// -----------------------------------------------------------------------------
void SbaXDataBrowserController::AfterDrop()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaui", "Ocke.Janssen@sun.com", "SbaXDataBrowserController::AfterDrop" );
    Reference< ::com::sun::star::sdb::XSQLErrorBroadcaster >  xFormError(getRowSet(), UNO_QUERY);
    if (xFormError.is())
        xFormError->addSQLErrorListener((::com::sun::star::sdb::XSQLErrorListener*)this);
}
// -----------------------------------------------------------------------------
void SbaXDataBrowserController::addColumnListeners(const Reference< ::com::sun::star::awt::XControlModel > & _xGridControlModel)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaui", "Ocke.Janssen@sun.com", "SbaXDataBrowserController::addColumnListeners" );
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
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaui", "Ocke.Janssen@sun.com", "SbaXDataBrowserController::InitializeGridModel" );
    return sal_True;
}
//..................................................................
}   // namespace dbaui
//..................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
