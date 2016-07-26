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
#include <com/sun/star/form/runtime/FormOperations.hpp>
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
#include <sfx2/app.hxx>
#include <sfx2/sfx.hrc>
#include <svx/fmsearch.hxx>
#include <svx/svxdlg.hxx>
#include <tools/diagnose_ex.h>
#include <osl/diagnose.h>
#include <vcl/layout.hxx>
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
using namespace ::com::sun::star::form::runtime;
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
        successflag = false;                                                \
        action;                                                             \
        successflag = true;                                                 \
    }                                                                       \
    catch(SQLException& e)                                                  \
    {                                                                       \
        SQLException aError = ::dbtools::prependErrorInfo(e, *this, context); \
        css::sdb::SQLErrorEvent aEvent;                        \
        aEvent.Reason <<= aError;                                           \
        errorOccured(aEvent);                                               \
    }                                                                       \
    catch(Exception&)                                                       \
    {                                                                       \
        DBG_UNHANDLED_EXCEPTION();                                          \
    }                                                                       \

#define DO_SAFE( action, message ) try { action; } catch(Exception&) { SAL_WARN("dbaccess.ui",message); } ;

namespace dbaui
{

// OParameterContinuation
class OParameterContinuation : public OInteraction< XInteractionSupplyParameters >
{
    Sequence< PropertyValue >       m_aValues;

public:
    OParameterContinuation() { }

    const Sequence< PropertyValue >&  getValues() const { return m_aValues; }

// XInteractionSupplyParameters
    virtual void SAL_CALL setParameters( const Sequence< PropertyValue >& _rValues ) throw(RuntimeException, std::exception) override;
};

void SAL_CALL OParameterContinuation::setParameters( const Sequence< PropertyValue >& _rValues ) throw(RuntimeException, std::exception)
{
    m_aValues = _rValues;
}

// a helper class implementing a runtime::XFormController, will be aggregated by SbaXDataBrowserController
// (we can't derive from XFormController as it's base class is XTabController and the XTabController::getModel collides
// with the XController::getModel implemented in our base class SbaXDataBrowserController)
class SbaXDataBrowserController::FormControllerImpl
    : public ::cppu::WeakAggImplHelper2< css::form::runtime::XFormController,
                                         css::frame::XFrameActionListener >
{
    friend class SbaXDataBrowserController;
    ::comphelper::OInterfaceContainerHelper2   m_aActivateListeners;
    SbaXDataBrowserController*          m_pOwner;

public:
    explicit FormControllerImpl(SbaXDataBrowserController* pOwner);

    // XFormController
    virtual css::uno::Reference< css::form::runtime::XFormOperations > SAL_CALL getFormOperations() throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::awt::XControl >  SAL_CALL getCurrentControl() throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL addActivateListener(const css::uno::Reference< css::form::XFormControllerListener > & l) throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL removeActivateListener(const css::uno::Reference< css::form::XFormControllerListener > & l) throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL addChildController( const css::uno::Reference< css::form::runtime::XFormController >& ChildController ) throw( css::uno::RuntimeException, css::lang::IllegalArgumentException, std::exception ) override;
    virtual css::uno::Reference< css::form::runtime::XFormControllerContext > SAL_CALL getContext() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setContext( const css::uno::Reference< css::form::runtime::XFormControllerContext >& _context ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::task::XInteractionHandler > SAL_CALL getInteractionHandler() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setInteractionHandler( const css::uno::Reference< css::task::XInteractionHandler >& _interactionHandler ) throw (css::uno::RuntimeException, std::exception) override;

    // XChild, base of XFormController
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL getParent(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setParent( const css::uno::Reference< css::uno::XInterface >& Parent ) throw (css::lang::NoSupportException, css::uno::RuntimeException, std::exception) override;

    // XComponent, base of XFormController
    virtual void SAL_CALL dispose(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener ) throw (css::uno::RuntimeException, std::exception) override;

    // XIndexAccess, base of XFormController
    virtual ::sal_Int32 SAL_CALL getCount(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getByIndex( ::sal_Int32 Index ) throw (css::lang::IndexOutOfBoundsException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

    // XElementAccess, base of XIndexAccess
    virtual css::uno::Type SAL_CALL getElementType(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements(  ) throw (css::uno::RuntimeException, std::exception) override;

    // XEnumerationAccess, base of XElementAccess
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration(  ) throw (css::uno::RuntimeException, std::exception) override;

    // XModifyBroadcaster, base of XFormController
    virtual void SAL_CALL addModifyListener( const css::uno::Reference< css::util::XModifyListener >& aListener ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeModifyListener( const css::uno::Reference< css::util::XModifyListener >& aListener ) throw (css::uno::RuntimeException, std::exception) override;

    // XConfirmDeleteBroadcaster, base of XFormController
    virtual void SAL_CALL addConfirmDeleteListener( const css::uno::Reference< css::form::XConfirmDeleteListener >& aListener ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeConfirmDeleteListener( const css::uno::Reference< css::form::XConfirmDeleteListener >& aListener ) throw (css::uno::RuntimeException, std::exception) override;

    // XSQLErrorBroadcaster, base of XFormController
    virtual void SAL_CALL addSQLErrorListener( const css::uno::Reference< css::sdb::XSQLErrorListener >& Listener ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeSQLErrorListener( const css::uno::Reference< css::sdb::XSQLErrorListener >& Listener ) throw (css::uno::RuntimeException, std::exception) override;

    // XRowSetApproveBroadcaster, base of XFormController
    virtual void SAL_CALL addRowSetApproveListener( const css::uno::Reference< css::sdb::XRowSetApproveListener >& listener ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeRowSetApproveListener( const css::uno::Reference< css::sdb::XRowSetApproveListener >& listener ) throw (css::uno::RuntimeException, std::exception) override;

    // XDatabaseParameterBroadcaster2, base of XFormController
    virtual void SAL_CALL addDatabaseParameterListener( const css::uno::Reference< css::form::XDatabaseParameterListener >& aListener ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeDatabaseParameterListener( const css::uno::Reference< css::form::XDatabaseParameterListener >& aListener ) throw (css::uno::RuntimeException, std::exception) override;

    // XDatabaseParameterBroadcaster, base of XDatabaseParameterBroadcaster2
    virtual void SAL_CALL addParameterListener( const css::uno::Reference< css::form::XDatabaseParameterListener >& aListener ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeParameterListener( const css::uno::Reference< css::form::XDatabaseParameterListener >& aListener ) throw (css::uno::RuntimeException, std::exception) override;

    // XModeSelector, base of XFormController
    virtual void SAL_CALL setMode( const OUString& aMode ) throw (css::lang::NoSupportException, css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getMode(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedModes(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsMode( const OUString& aMode ) throw (css::uno::RuntimeException, std::exception) override;

    // XTabController, base of XFormController
    virtual void SAL_CALL setModel(const css::uno::Reference< css::awt::XTabControllerModel > & Model) throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Reference< css::awt::XTabControllerModel >  SAL_CALL getModel() throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL setContainer(const css::uno::Reference< css::awt::XControlContainer > & Container) throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Reference< css::awt::XControlContainer >  SAL_CALL getContainer() throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< css::uno::Reference< css::awt::XControl >  > SAL_CALL getControls() throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL autoTabOrder() throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL activateTabOrder() throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL activateFirst() throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL activateLast() throw( css::uno::RuntimeException, std::exception ) override;

    // XFrameActionListener
    virtual void SAL_CALL frameAction(const css::frame::FrameActionEvent& aEvent) throw( css::uno::RuntimeException, std::exception ) override;

    // XEventListener
    virtual void SAL_CALL disposing(const css::lang::EventObject& Source) throw( css::uno::RuntimeException, std::exception ) override;

protected:
    virtual ~FormControllerImpl();
};

SbaXDataBrowserController::FormControllerImpl::FormControllerImpl(SbaXDataBrowserController* _pOwner)
    :m_aActivateListeners(_pOwner->getMutex())
    ,m_pOwner(_pOwner)
{

    OSL_ENSURE(m_pOwner, "SbaXDataBrowserController::FormControllerImpl::FormControllerImpl : invalid Owner !");
}

SbaXDataBrowserController::FormControllerImpl::~FormControllerImpl()
{

}

Reference< runtime::XFormOperations > SAL_CALL SbaXDataBrowserController::FormControllerImpl::getFormOperations() throw (RuntimeException, std::exception)
{
    return FormOperations::createWithFormController( m_pOwner->m_xContext, this );
}

Reference< css::awt::XControl >  SbaXDataBrowserController::FormControllerImpl::getCurrentControl() throw( RuntimeException, std::exception )
{
    return m_pOwner->getBrowserView() ? m_pOwner->getBrowserView()->getGridControl() : Reference< css::awt::XControl > ();
}

void SAL_CALL SbaXDataBrowserController::FormControllerImpl::addActivateListener(const Reference< css::form::XFormControllerListener > & l) throw( RuntimeException, std::exception )
{
    m_aActivateListeners.addInterface(l);
}

void SAL_CALL SbaXDataBrowserController::FormControllerImpl::removeActivateListener(const Reference< css::form::XFormControllerListener > & l) throw( RuntimeException, std::exception )
{
    m_aActivateListeners.removeInterface(l);
}

void SAL_CALL SbaXDataBrowserController::FormControllerImpl::addChildController( const Reference< runtime::XFormController >& ) throw( RuntimeException, IllegalArgumentException, std::exception )
{
    // not supported
    throw IllegalArgumentException( OUString(), *this, 1 );
}

Reference< runtime::XFormControllerContext > SAL_CALL SbaXDataBrowserController::FormControllerImpl::getContext() throw (RuntimeException, std::exception)
{
    SAL_WARN("dbaccess.ui", "SbaXDataBrowserController::FormControllerImpl::getContext: no support!!" );
    return nullptr;
}

void SAL_CALL SbaXDataBrowserController::FormControllerImpl::setContext( const Reference< runtime::XFormControllerContext >& /*_context*/ ) throw (RuntimeException, std::exception)
{
    SAL_WARN("dbaccess.ui", "SbaXDataBrowserController::FormControllerImpl::setContext: no support!!" );
}

Reference< XInteractionHandler > SAL_CALL SbaXDataBrowserController::FormControllerImpl::getInteractionHandler() throw (RuntimeException, std::exception)
{
    SAL_WARN("dbaccess.ui", "SbaXDataBrowserController::FormControllerImpl::getInteractionHandler: no support!!" );
    return nullptr;
}

void SAL_CALL SbaXDataBrowserController::FormControllerImpl::setInteractionHandler( const Reference< XInteractionHandler >& /*_interactionHandler*/ ) throw (RuntimeException, std::exception)
{
    SAL_WARN("dbaccess.ui", "SbaXDataBrowserController::FormControllerImpl::setInteractionHandler: no support!!" );
}

Reference< XInterface > SAL_CALL SbaXDataBrowserController::FormControllerImpl::getParent(  ) throw (RuntimeException, std::exception)
{
    // don't have any parent form controllers
    return nullptr;
}

void SAL_CALL SbaXDataBrowserController::FormControllerImpl::setParent( const Reference< XInterface >& /*Parent*/ ) throw (NoSupportException, RuntimeException, std::exception)
{
    throw NoSupportException( OUString(), *this );
}

void SAL_CALL SbaXDataBrowserController::FormControllerImpl::dispose(  ) throw (RuntimeException, std::exception)
{
    SAL_WARN("dbaccess.ui", "SbaXDataBrowserController::FormControllerImpl::dispose: no, you do *not* want to do this!" );
}

void SAL_CALL SbaXDataBrowserController::FormControllerImpl::addEventListener( const Reference< XEventListener >& /*xListener*/ ) throw (RuntimeException, std::exception)
{
    SAL_WARN("dbaccess.ui", "SbaXDataBrowserController::FormControllerImpl::addEventListener: no support!!" );
}

void SAL_CALL SbaXDataBrowserController::FormControllerImpl::removeEventListener( const Reference< XEventListener >& /*aListener*/ ) throw (RuntimeException, std::exception)
{
    SAL_WARN("dbaccess.ui", "SbaXDataBrowserController::FormControllerImpl::removeEventListener: no support!!" );
}

::sal_Int32 SAL_CALL SbaXDataBrowserController::FormControllerImpl::getCount(  ) throw (RuntimeException, std::exception)
{
    // no sub controllers, never
    return 0;
}

Any SAL_CALL SbaXDataBrowserController::FormControllerImpl::getByIndex( ::sal_Int32 /*Index*/ ) throw (IndexOutOfBoundsException, WrappedTargetException, RuntimeException, std::exception)
{
    // no sub controllers, never
    throw IndexOutOfBoundsException( OUString(), *this );
}

Type SAL_CALL SbaXDataBrowserController::FormControllerImpl::getElementType(  ) throw (RuntimeException, std::exception)
{
    return ::cppu::UnoType< runtime::XFormController >::get();
}

sal_Bool SAL_CALL SbaXDataBrowserController::FormControllerImpl::hasElements(  ) throw (RuntimeException, std::exception)
{
    // no sub controllers, never
    return false;
}

Reference< XEnumeration > SAL_CALL SbaXDataBrowserController::FormControllerImpl::createEnumeration(  ) throw (RuntimeException, std::exception)
{
    return new ::comphelper::OEnumerationByIndex( this );
}

void SAL_CALL SbaXDataBrowserController::FormControllerImpl::addModifyListener( const Reference< XModifyListener >& /*_Listener*/ ) throw (RuntimeException, std::exception)
{
    SAL_WARN("dbaccess.ui", "SbaXDataBrowserController::FormControllerImpl::addModifyListener: no support!" );
}

void SAL_CALL SbaXDataBrowserController::FormControllerImpl::removeModifyListener( const Reference< XModifyListener >& /*_Listener*/ ) throw (RuntimeException, std::exception)
{
    SAL_WARN("dbaccess.ui", "SbaXDataBrowserController::FormControllerImpl::removeModifyListener: no support!" );
}

void SAL_CALL SbaXDataBrowserController::FormControllerImpl::addConfirmDeleteListener( const Reference< XConfirmDeleteListener >& /*_Listener*/ ) throw (RuntimeException, std::exception)
{
    SAL_WARN("dbaccess.ui", "SbaXDataBrowserController::FormControllerImpl::addConfirmDeleteListener: no support!" );
}

void SAL_CALL SbaXDataBrowserController::FormControllerImpl::removeConfirmDeleteListener( const Reference< XConfirmDeleteListener >& /*_Listener*/ ) throw (RuntimeException, std::exception)
{
    SAL_WARN("dbaccess.ui", "SbaXDataBrowserController::FormControllerImpl::removeConfirmDeleteListener: no support!" );
}

void SAL_CALL SbaXDataBrowserController::FormControllerImpl::addSQLErrorListener( const Reference< XSQLErrorListener >& /*_Listener*/ ) throw (RuntimeException, std::exception)
{
    SAL_WARN("dbaccess.ui", "SbaXDataBrowserController::FormControllerImpl::addSQLErrorListener: no support!" );
}

void SAL_CALL SbaXDataBrowserController::FormControllerImpl::removeSQLErrorListener( const Reference< XSQLErrorListener >& /*_Listener*/ ) throw (RuntimeException, std::exception)
{
    SAL_WARN("dbaccess.ui", "SbaXDataBrowserController::FormControllerImpl::removeSQLErrorListener: no support!" );
}

void SAL_CALL SbaXDataBrowserController::FormControllerImpl::addRowSetApproveListener( const Reference< XRowSetApproveListener >& /*_Listener*/ ) throw (RuntimeException, std::exception)
{
    SAL_WARN("dbaccess.ui", "SbaXDataBrowserController::FormControllerImpl::addRowSetApproveListener: no support!" );
}

void SAL_CALL SbaXDataBrowserController::FormControllerImpl::removeRowSetApproveListener( const Reference< XRowSetApproveListener >& /*_Listener*/ ) throw (RuntimeException, std::exception)
{
    SAL_WARN("dbaccess.ui", "SbaXDataBrowserController::FormControllerImpl::removeRowSetApproveListener: no support!" );
}

void SAL_CALL SbaXDataBrowserController::FormControllerImpl::addDatabaseParameterListener( const Reference< XDatabaseParameterListener >& /*_Listener*/ ) throw (RuntimeException, std::exception)
{
    SAL_WARN("dbaccess.ui", "SbaXDataBrowserController::FormControllerImpl::addDatabaseParameterListener: no support!" );
}

void SAL_CALL SbaXDataBrowserController::FormControllerImpl::removeDatabaseParameterListener( const Reference< XDatabaseParameterListener >& /*_Listener*/ ) throw (RuntimeException, std::exception)
{
    SAL_WARN("dbaccess.ui", "SbaXDataBrowserController::FormControllerImpl::removeDatabaseParameterListener: no support!" );
}

void SAL_CALL SbaXDataBrowserController::FormControllerImpl::addParameterListener( const Reference< XDatabaseParameterListener >& /*_Listener*/ ) throw (RuntimeException, std::exception)
{
    SAL_WARN("dbaccess.ui", "SbaXDataBrowserController::FormControllerImpl::addParameterListener: no support!" );
}

void SAL_CALL SbaXDataBrowserController::FormControllerImpl::removeParameterListener( const Reference< XDatabaseParameterListener >& /*_Listener*/ ) throw (RuntimeException, std::exception)
{
    SAL_WARN("dbaccess.ui", "SbaXDataBrowserController::FormControllerImpl::removeParameterListener: no support!" );
}

void SAL_CALL SbaXDataBrowserController::FormControllerImpl::setMode( const OUString& _rMode ) throw (NoSupportException, RuntimeException, std::exception)
{
    if ( !supportsMode( _rMode ) )
        throw NoSupportException();
}

OUString SAL_CALL SbaXDataBrowserController::FormControllerImpl::getMode(  ) throw (RuntimeException, std::exception)
{
    return OUString( "DataMode" );
}

Sequence< OUString > SAL_CALL SbaXDataBrowserController::FormControllerImpl::getSupportedModes(  ) throw (RuntimeException, std::exception)
{
    Sequence< OUString > aModes { "DataMode" };
    return aModes;
}

sal_Bool SAL_CALL SbaXDataBrowserController::FormControllerImpl::supportsMode( const OUString& aMode ) throw (RuntimeException, std::exception)
{
    return aMode == "DataMode";
}

void SAL_CALL SbaXDataBrowserController::FormControllerImpl::setModel(const Reference< css::awt::XTabControllerModel > & /*Model*/) throw( RuntimeException, std::exception )
{
    SAL_WARN("dbaccess.ui","SbaXDataBrowserController::FormControllerImpl::setModel : invalid call, can't change my model !");
}

Reference< css::awt::XTabControllerModel >  SAL_CALL SbaXDataBrowserController::FormControllerImpl::getModel() throw( RuntimeException, std::exception )
{
    return Reference< XTabControllerModel >(m_pOwner->getRowSet(), UNO_QUERY);
}

void SAL_CALL SbaXDataBrowserController::FormControllerImpl::setContainer(const Reference< css::awt::XControlContainer > &) throw( RuntimeException, std::exception )
{
    SAL_WARN("dbaccess.ui","SbaXDataBrowserController::FormControllerImpl::setContainer : invalid call, can't change my container !");
}

Reference< css::awt::XControlContainer >  SAL_CALL SbaXDataBrowserController::FormControllerImpl::getContainer() throw( RuntimeException, std::exception )
{
    if (m_pOwner->getBrowserView())
        return m_pOwner->getBrowserView()->getContainer();
    return Reference< css::awt::XControlContainer > ();
}

Sequence< Reference< css::awt::XControl > > SAL_CALL SbaXDataBrowserController::FormControllerImpl::getControls() throw( RuntimeException, std::exception )
{
    if (m_pOwner->getBrowserView())
    {
        Reference< css::awt::XControl >  xGrid = m_pOwner->getBrowserView()->getGridControl();
        return Sequence< Reference< css::awt::XControl > >(&xGrid, 1);
    }
    return Sequence< Reference< css::awt::XControl > >();
}

void SAL_CALL SbaXDataBrowserController::FormControllerImpl::autoTabOrder() throw( RuntimeException, std::exception )
{
    SAL_WARN("dbaccess.ui","SbaXDataBrowserController::FormControllerImpl::autoTabOrder : nothing to do (always have only one control) !");
}

void SAL_CALL SbaXDataBrowserController::FormControllerImpl::activateTabOrder() throw( RuntimeException, std::exception )
{
    SAL_WARN("dbaccess.ui","SbaXDataBrowserController::FormControllerImpl::activateTabOrder : nothing to do (always have only one control) !");
}

void SAL_CALL SbaXDataBrowserController::FormControllerImpl::activateFirst() throw( RuntimeException, std::exception )
{
    if (m_pOwner->getBrowserView())
        m_pOwner->getBrowserView()->getVclControl()->ActivateCell();
}

void SAL_CALL SbaXDataBrowserController::FormControllerImpl::activateLast() throw( RuntimeException, std::exception )
{
    if (m_pOwner->getBrowserView())
        m_pOwner->getBrowserView()->getVclControl()->ActivateCell();
}

void SAL_CALL SbaXDataBrowserController::FormControllerImpl::frameAction(const css::frame::FrameActionEvent& /*aEvent*/) throw( RuntimeException, std::exception )
{
}

void SAL_CALL SbaXDataBrowserController::FormControllerImpl::disposing(const css::lang::EventObject& /*Source*/) throw( RuntimeException, std::exception )
{
    // nothing to do
    // we don't add ourself as listener to any broadcasters, so we are not resposible for removing us
}

// SbaXDataBrowserController
Sequence< Type > SAL_CALL SbaXDataBrowserController::getTypes(  ) throw (RuntimeException, std::exception)
{
    return ::comphelper::concatSequences(
        SbaXDataBrowserController_Base::getTypes(),
        m_pFormControllerImpl->getTypes()
    );
}

Sequence< sal_Int8 > SAL_CALL SbaXDataBrowserController::getImplementationId(  ) throw (RuntimeException, std::exception)
{
    return css::uno::Sequence<sal_Int8>();
}

Any SAL_CALL SbaXDataBrowserController::queryInterface(const Type& _rType) throw (RuntimeException, std::exception)
{
    // check for our additional interfaces
    Any aRet = SbaXDataBrowserController_Base::queryInterface(_rType);

    // check for our aggregate (implementing the XFormController)
    if (!aRet.hasValue())
        aRet = m_xFormControllerImpl->queryAggregation(_rType);

    // no more to offer
    return aRet;
}

SbaXDataBrowserController::SbaXDataBrowserController(const Reference< css::uno::XComponentContext >& _rM)
    :SbaXDataBrowserController_Base(_rM)
    ,m_nRowSetPrivileges(0)
    ,m_pClipbordNotifier( nullptr )
    ,m_aAsyncGetCellFocus(LINK(this, SbaXDataBrowserController, OnAsyncGetCellFocus))
    ,m_aAsyncDisplayError( LINK( this, SbaXDataBrowserController, OnAsyncDisplayError ) )
    ,m_sStateSaveRecord(ModuleRes(RID_STR_SAVE_CURRENT_RECORD))
    ,m_sStateUndoRecord(ModuleRes(RID_STR_UNDO_MODIFY_RECORD))
    ,m_sModuleIdentifier( OUString( "com.sun.star.sdb.DataSourceBrowser" ) )
    ,m_pFormControllerImpl(nullptr)
    ,m_nFormActionNestingLevel(0)
    ,m_bLoadCanceled( false )
    ,m_bCannotSelectUnfiltered( true )
{
    // create the form controller aggregate
    osl_atomic_increment(&m_refCount);
    {
        m_pFormControllerImpl = new FormControllerImpl(this);
        m_xFormControllerImpl = m_pFormControllerImpl;
        m_xFormControllerImpl->setDelegator(*this);
    }
    osl_atomic_decrement(&m_refCount);

    m_aInvalidateClipboard.SetTimeoutHdl(LINK(this, SbaXDataBrowserController, OnInvalidateClipboard));
    m_aInvalidateClipboard.SetTimeout(300);
}

SbaXDataBrowserController::~SbaXDataBrowserController()
{
    //  deleteView();
    // release the aggregated form controller
    if (m_xFormControllerImpl.is())
    {
        Reference< XInterface >  xEmpty;
        m_xFormControllerImpl->setDelegator(xEmpty);
    }

}

void SbaXDataBrowserController::startFrameListening( const Reference< XFrame >& _rxFrame )
{
    SbaXDataBrowserController_Base::startFrameListening( _rxFrame );

    Reference< XFrameActionListener >   xAggListener;
    if ( m_xFormControllerImpl.is() )
        m_xFormControllerImpl->queryAggregation( cppu::UnoType<XFrameActionListener>::get() ) >>= xAggListener;

    if ( _rxFrame.is() && xAggListener.is() )
        _rxFrame->addFrameActionListener( xAggListener );
}

void SbaXDataBrowserController::stopFrameListening( const Reference< XFrame >& _rxFrame )
{
    SbaXDataBrowserController_Base::stopFrameListening( _rxFrame );

    Reference< XFrameActionListener >   xAggListener;
    if ( m_xFormControllerImpl.is() )
        m_xFormControllerImpl->queryAggregation( cppu::UnoType<XFrameActionListener>::get() ) >>= xAggListener;

    if ( _rxFrame.is() && xAggListener.is() )
        _rxFrame->removeFrameActionListener( xAggListener );
}

void SbaXDataBrowserController::onStartLoading( const Reference< XLoadable >& _rxLoadable )
{
    m_bLoadCanceled = false;
    m_bCannotSelectUnfiltered = false;

    Reference< XWarningsSupplier > xWarnings( _rxLoadable, UNO_QUERY );
    if ( xWarnings.is() )
    {
        try
        {
            xWarnings->clearWarnings();
        }
        catch(const SQLException& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }
}

void SbaXDataBrowserController::impl_checkForCannotSelectUnfiltered( const SQLExceptionInfo& _rError )
{
    ::connectivity::SQLError aError( getORB() );
    ::connectivity::ErrorCode nErrorCode( connectivity::SQLError::getErrorCode( sdb::ErrorCondition::DATA_CANNOT_SELECT_UNFILTERED ) );
    if ( static_cast<const SQLException*>(_rError)->ErrorCode == nErrorCode )
    {
        m_bCannotSelectUnfiltered = true;
        InvalidateFeature( ID_BROWSER_FILTERCRIT );
    }
}

bool SbaXDataBrowserController::reloadForm( const Reference< XLoadable >& _rxLoadable )
{
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
                OUString sColumnName;
                OSL_VERIFY( xOrderColumn->getPropertyValue( PROPERTY_NAME ) >>= sColumnName);
                OUString sTableName;
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
        catch(const SQLException& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    return _rxLoadable->isLoaded();
}

void SbaXDataBrowserController::initFormatter()
{
    // create a formatter working with the connections format supplier
    Reference< css::util::XNumberFormatsSupplier >  xSupplier(::dbtools::getNumberFormats(::dbtools::getConnection(m_xRowSet), true, getORB()));

    if(xSupplier.is())
    {
        // create a new formatter
        m_xFormatter.set(util::NumberFormatter::create(getORB()), UNO_QUERY_THROW);
        m_xFormatter->attachNumberFormatsSupplier(xSupplier);
    }
    else // clear the formatter
        m_xFormatter = nullptr;
}

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

bool SbaXDataBrowserController::Construct(vcl::Window* pParent)
{
    // create/initialize the form and the grid model
    m_xRowSet = CreateForm();
    if (!m_xRowSet.is())
        return false;

    m_xColumnsSupplier.set(m_xRowSet,UNO_QUERY);
    m_xLoadable.set(m_xRowSet,UNO_QUERY);

    Reference< XPropertySet > xFormProperties( m_xRowSet, UNO_QUERY );
    if ( !InitializeForm( xFormProperties ) )
        return false;

    m_xGridModel = CreateGridModel();
    if (!m_xGridModel.is())
        return false;

    // set the formatter if available
    initFormatter();

    // we want to have a grid with a "flat" border
    Reference< XPropertySet >  xGridSet(m_xGridModel, UNO_QUERY);
    if ( xGridSet.is() )
        xGridSet->setPropertyValue(PROPERTY_BORDER, makeAny((sal_Int16)2));


    // marry them
    Reference< css::container::XNameContainer >  xNameCont(m_xRowSet, UNO_QUERY);
    {
        OUString sText(ModuleRes(STR_DATASOURCE_GRIDCONTROL_NAME));
        xNameCont->insertByName(sText, makeAny(m_xGridModel));
    }

    // create the view
    setView( VclPtr<UnoDataBrowserView>::Create( pParent, *this, getORB() ) );
    if (!getBrowserView())
        return false;

    // late construction
    bool bSuccess = false;
    try
    {
        getBrowserView()->Construct(getControlModel());
        bSuccess = true;
    }
    catch(SQLException&)
    {
    }
    catch(Exception&)
    {
        SAL_WARN("dbaccess.ui","SbaXDataBrowserController::Construct : the construction of UnoDataBrowserView failed !");
    }

    if (!bSuccess)
    {
        //  deleteView();
        return false;
    }

    // now that we have a view we can create the clipboard listener
    m_aSystemClipboard = TransferableDataHelper::CreateFromSystemClipboard( getView() );
    m_aSystemClipboard.StartClipboardListening( );

    m_pClipbordNotifier = new TransferableClipboardListener( LINK( this, SbaXDataBrowserController, OnClipboardChanged ) );
    m_pClipbordNotifier->acquire();
    m_pClipbordNotifier->AddRemoveListener( getView(), true );

    // this call create the toolbox
    SbaXDataBrowserController_Base::Construct(pParent);

    getBrowserView()->Show();

    // set the callbacks for the grid control
    SbaGridControl* pVclGrid = getBrowserView()->getVclControl();
    OSL_ENSURE(pVclGrid, "SbaXDataBrowserController::Construct : have no VCL control !");
    pVclGrid->SetMasterListener(this);

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
    Reference< css::sdb::XSQLErrorBroadcaster >  xFormError(getRowSet(), UNO_QUERY);
    if (xFormError.is())
        xFormError->addSQLErrorListener(static_cast<css::sdb::XSQLErrorListener*>(this));

    if (m_xLoadable.is())
        m_xLoadable->addLoadListener(this);

    Reference< css::form::XDatabaseParameterBroadcaster >  xFormParameter(getRowSet(), UNO_QUERY);
    if (xFormParameter.is())
        xFormParameter->addParameterListener(static_cast<css::form::XDatabaseParameterListener*>(this));

    addModelListeners(getControlModel());
    addControlListeners(getBrowserView()->getGridControl());

    // load the form
    return LoadForm();
}

bool SbaXDataBrowserController::LoadForm()
{
    reloadForm( m_xLoadable );
    return true;
}

void SbaXDataBrowserController::AddColumnListener(const Reference< XPropertySet > & /*xCol*/)
{
    // we're not interested in any column properties ...
}

void SbaXDataBrowserController::RemoveColumnListener(const Reference< XPropertySet > & /*xCol*/)
{
}

Reference< XRowSet >  SbaXDataBrowserController::CreateForm()
{
    return Reference< XRowSet > (
      getORB()->getServiceManager()->createInstanceWithContext("com.sun.star.form.component.Form", getORB()),
      UNO_QUERY);
}

Reference< css::form::XFormComponent >  SbaXDataBrowserController::CreateGridModel()
{
    return Reference< css::form::XFormComponent > (
      getORB()->getServiceManager()->createInstanceWithContext("com.sun.star.form.component.GridControl", getORB()),
      UNO_QUERY);
}

void SbaXDataBrowserController::addModelListeners(const Reference< css::awt::XControlModel > & _xGridControlModel)
{
    // ... all the grid columns
    addColumnListeners(_xGridControlModel);

    // (we are interested in all columns the grid has (and only in these) so we have to listen to the container, too)
    Reference< css::container::XContainer >  xColContainer(_xGridControlModel, UNO_QUERY);
    if (xColContainer.is())
        xColContainer->addContainerListener(static_cast<css::container::XContainerListener*>(this));

    Reference< css::form::XReset >  xReset(_xGridControlModel, UNO_QUERY);
    if (xReset.is())
        xReset->addResetListener(static_cast<css::form::XResetListener*>(this));
}

void SbaXDataBrowserController::removeModelListeners(const Reference< XControlModel > & _xGridControlModel)
{
    // every single column model
    Reference< XIndexContainer >  xColumns(_xGridControlModel, UNO_QUERY);
    if (xColumns.is())
    {
        sal_Int32 nCount = xColumns->getCount();
        for (sal_Int32 i=0; i < nCount; ++i)
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

void SbaXDataBrowserController::addControlListeners(const Reference< css::awt::XControl > & _xGridControl)
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

void SbaXDataBrowserController::removeControlListeners(const Reference< css::awt::XControl > & _xGridControl)
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

void SAL_CALL SbaXDataBrowserController::focusGained(const FocusEvent& /*e*/) throw( RuntimeException, std::exception )
{
    // notify our activate listeners (registered on the form controller aggregate)
    EventObject aEvt(*this);
    ::comphelper::OInterfaceIteratorHelper2 aIter(m_pFormControllerImpl->m_aActivateListeners);
    while (aIter.hasMoreElements())
        static_cast<XFormControllerListener*>(aIter.next())->formActivated(aEvt);
}

void SAL_CALL SbaXDataBrowserController::focusLost(const FocusEvent& e) throw( RuntimeException, std::exception )
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
    ::comphelper::OInterfaceIteratorHelper2 aIter(m_pFormControllerImpl->m_aActivateListeners);
    while (aIter.hasMoreElements())
        static_cast<XFormControllerListener*>(aIter.next())->formDeactivated(aEvt);

    // commit the changes of the grid control (as we're deactivated)
    Reference< XBoundComponent >  xCommitable(getBrowserView()->getGridControl(), UNO_QUERY);
    if (xCommitable.is())
        xCommitable->commit();
    else
        SAL_WARN("dbaccess.ui", "SbaXDataBrowserController::focusLost : why is my control not commitable ?");
}

void SbaXDataBrowserController::disposingGridControl(const css::lang::EventObject& /*Source*/)
{
    removeControlListeners(getBrowserView()->getGridControl());
}

void SbaXDataBrowserController::disposingGridModel(const css::lang::EventObject& /*Source*/)
{
    removeModelListeners(getControlModel());
}

void SbaXDataBrowserController::disposingFormModel(const css::lang::EventObject& Source)
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

    Reference< css::sdb::XSQLErrorBroadcaster >  xFormError(Source.Source, UNO_QUERY);
    if (xFormError.is())
        xFormError->removeSQLErrorListener(static_cast<css::sdb::XSQLErrorListener*>(this));

    if (m_xLoadable.is())
        m_xLoadable->removeLoadListener(this);

    Reference< css::form::XDatabaseParameterBroadcaster >  xFormParameter(Source.Source, UNO_QUERY);
    if (xFormParameter.is())
        xFormParameter->removeParameterListener(static_cast<css::form::XDatabaseParameterListener*>(this));
}

void SbaXDataBrowserController::disposingColumnModel(const css::lang::EventObject& Source)
{
    RemoveColumnListener(Reference< XPropertySet > (Source.Source, UNO_QUERY));
}

void SbaXDataBrowserController::disposing(const EventObject& Source) throw( RuntimeException, std::exception )
{
    // if it's a component other than our aggregate, forward it to the aggregate
    if ( m_xFormControllerImpl != Source.Source )
    {
        Reference< XEventListener > xAggListener;
        m_xFormControllerImpl->queryAggregation( cppu::UnoType<decltype(xAggListener)>::get() ) >>= xAggListener;
        if ( xAggListener.is( ))
            xAggListener->disposing( Source );
    }

    // is it the grid control ?
    if (getBrowserView())
    {
        Reference< css::awt::XControl >  xSourceControl(Source.Source, UNO_QUERY);
        if (xSourceControl == getBrowserView()->getGridControl())
            disposingGridControl(Source);
    }

    // its model (the container of the columns) ?
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

void SAL_CALL SbaXDataBrowserController::setIdentifier( const OUString& Identifier ) throw (RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( getMutex() );
    m_sModuleIdentifier = Identifier;
}

OUString SAL_CALL SbaXDataBrowserController::getIdentifier(  ) throw (RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( getMutex() );
    return m_sModuleIdentifier;
}

void SbaXDataBrowserController::propertyChange(const PropertyChangeEvent& evt) throw ( RuntimeException, std::exception )
{
    Reference< XPropertySet >  xSource(evt.Source, UNO_QUERY);
    if (!xSource.is())
        return;

    SolarMutexGuard aGuard;
    // the IsModified changed to sal_False ?
    if  (   evt.PropertyName == PROPERTY_ISMODIFIED
        &&  !::comphelper::getBOOL(evt.NewValue)
        )
    {   // -> the current field isn't modified anymore, too
        setCurrentModified( false );
    }

    // switching to a new record ?
    if  (   evt.PropertyName == PROPERTY_ISNEW
        &&  ::comphelper::getBOOL(evt.NewValue)
        )
    {
        if (::comphelper::getINT32(xSource->getPropertyValue(PROPERTY_ROWCOUNT)) == 0)
            // if we're switching to a new record and didn't have any records before we need to invalidate
            // all slots (as the cursor was invalid before the mode change and so the slots were disabled)
            InvalidateAll();
    }

    if (evt.PropertyName == PROPERTY_FILTER)
    {
        InvalidateFeature(ID_BROWSER_REMOVEFILTER);
    }
    else if (evt.PropertyName == PROPERTY_HAVING_CLAUSE)
    {
        InvalidateFeature(ID_BROWSER_REMOVEFILTER);
    }
    else if (evt.PropertyName == PROPERTY_ORDER)
    {
        InvalidateFeature(ID_BROWSER_REMOVEFILTER);
    }

    // a new record count ? -> may be our search availability has changed
    if (evt.PropertyName == PROPERTY_ROWCOUNT)
    {
        sal_Int32 nNewValue = 0, nOldValue = 0;
        evt.NewValue >>= nNewValue;
        evt.OldValue >>= nOldValue;
        if((nOldValue == 0 && nNewValue != 0) || (nOldValue != 0 && nNewValue == 0))
            InvalidateAll();
    }
}

void SbaXDataBrowserController::modified(const css::lang::EventObject& /*aEvent*/) throw( RuntimeException, std::exception )
{
    setCurrentModified( true );
}

void SbaXDataBrowserController::elementInserted(const css::container::ContainerEvent& evt) throw( RuntimeException, std::exception )
{
    OSL_ENSURE(Reference< XInterface >(evt.Source, UNO_QUERY).get() == Reference< XInterface >(getControlModel(), UNO_QUERY).get(),
        "SbaXDataBrowserController::elementInserted: where did this come from (not from the grid model)?!");
    Reference< XPropertySet >  xNewColumn(evt.Element,UNO_QUERY);
    if ( xNewColumn.is() )
        AddColumnListener(xNewColumn);
}

void SbaXDataBrowserController::elementRemoved(const css::container::ContainerEvent& evt) throw( RuntimeException, std::exception )
{
    OSL_ENSURE(Reference< XInterface >(evt.Source, UNO_QUERY).get() == Reference< XInterface >(getControlModel(), UNO_QUERY).get(),
        "SbaXDataBrowserController::elementRemoved: where did this come from (not from the grid model)?!");
    Reference< XPropertySet >  xOldColumn(evt.Element,UNO_QUERY);
    if ( xOldColumn.is() )
        RemoveColumnListener(xOldColumn);
}

void SbaXDataBrowserController::elementReplaced(const css::container::ContainerEvent& evt) throw( RuntimeException, std::exception )
{
    OSL_ENSURE(Reference< XInterface >(evt.Source, UNO_QUERY).get() == Reference< XInterface >(getControlModel(), UNO_QUERY).get(),
        "SbaXDataBrowserController::elementReplaced: where did this come from (not from the grid model)?!");
    Reference< XPropertySet >  xOldColumn(evt.ReplacedElement,UNO_QUERY);
    if ( xOldColumn.is() )
        RemoveColumnListener(xOldColumn);

    Reference< XPropertySet >  xNewColumn(evt.Element,UNO_QUERY);
    if ( xNewColumn.is() )
        AddColumnListener(xNewColumn);
}

sal_Bool SbaXDataBrowserController::suspend(sal_Bool /*bSuspend*/) throw( RuntimeException, std::exception )
{
    m_aAsyncGetCellFocus.CancelCall();
    m_aAsyncDisplayError.CancelCall();
    m_aAsyncInvalidateAll.CancelCall();

    bool bSuccess = SaveModified();
    return bSuccess;
}

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

    Reference< css::sdb::XSQLErrorBroadcaster >  xFormError(getRowSet(), UNO_QUERY);
    if (xFormError.is())
        xFormError->removeSQLErrorListener(static_cast<css::sdb::XSQLErrorListener*>(this));

    if (m_xLoadable.is())
        m_xLoadable->removeLoadListener(this);

    Reference< css::form::XDatabaseParameterBroadcaster >  xFormParameter(getRowSet(), UNO_QUERY);
    if (xFormParameter.is())
        xFormParameter->removeParameterListener(static_cast<css::form::XDatabaseParameterListener*>(this));

    removeModelListeners(getControlModel());

    if ( getView() && m_pClipbordNotifier  )
    {
        m_pClipbordNotifier->ClearCallbackLink();
        m_pClipbordNotifier->AddRemoveListener( getView(), false );
        m_pClipbordNotifier->release();
        m_pClipbordNotifier = nullptr;
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

        m_xRowSet           = nullptr;
        m_xColumnsSupplier  = nullptr;
        m_xLoadable         = nullptr;
    }
    catch(Exception&)
    {
        DBG_UNHANDLED_EXCEPTION();
    }
    m_xParser.clear();
        // don't dispose, just reset - it's owned by the RowSet
}

void SbaXDataBrowserController::frameAction(const css::frame::FrameActionEvent& aEvent) throw( RuntimeException, std::exception )
{
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
                    OnInvalidateClipboard( nullptr );
                }
                break;
            case FrameAction_FRAME_DEACTIVATING:
            case FrameAction_FRAME_UI_DEACTIVATING:
                // stop the clipboard invalidator
                if (getBrowserView() && getBrowserView()->getVclControl() && m_aInvalidateClipboard.IsActive())
                {
                    m_aInvalidateClipboard.Stop();
                    OnInvalidateClipboard( nullptr );
                }
                // remove the "get cell focus"-event
                m_aAsyncGetCellFocus.CancelCall();
                break;
            default:
                break;
        }
}

IMPL_LINK_NOARG_TYPED( SbaXDataBrowserController, OnAsyncDisplayError, void*, void )
{
    if ( m_aCurrentError.isValid() )
    {
        ScopedVclPtrInstance< OSQLMessageBox > aDlg( getBrowserView(), m_aCurrentError );
        aDlg->Execute();
    }
}

void SbaXDataBrowserController::errorOccured(const css::sdb::SQLErrorEvent& aEvent) throw( RuntimeException, std::exception )
{
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

sal_Bool SbaXDataBrowserController::approveParameter(const css::form::DatabaseParameterEvent& aEvent) throw( RuntimeException, std::exception )
{
    if (aEvent.Source != getRowSet())
    {
        // not my data source -> allow anything
        SAL_WARN("dbaccess.ui","SbaXDataBrowserController::approveParameter : invalid event source !");
        return true;
    }

    Reference< css::container::XIndexAccess >  xParameters = aEvent.Parameters;
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
        Reference< XInteractionHandler2 > xHandler( InteractionHandler::createWithParent(getORB(), nullptr) );
        xHandler->handle(xParamRequest);

        if (!pParamValues->wasSelected())
        {   // canceled
            setLoadingCancelled();
            return false;
        }

        // transfer the values into the parameter supplier
        Sequence< PropertyValue > aFinalValues = pParamValues->getValues();
        if (aFinalValues.getLength() != aRequest.Parameters->getCount())
        {
            SAL_WARN("dbaccess.ui","SbaXDataBrowserController::approveParameter: the InteractionHandler returned nonsense!");
            setLoadingCancelled();
            return false;
        }
        const PropertyValue* pFinalValues = aFinalValues.getConstArray();
        for (sal_Int32 i=0; i<aFinalValues.getLength(); ++i, ++pFinalValues)
        {
            Reference< XPropertySet > xParam(
                aRequest.Parameters->getByIndex(i), css::uno::UNO_QUERY);
            OSL_ENSURE(xParam.is(), "SbaXDataBrowserController::approveParameter: one of the parameters is no property set!");
            if (xParam.is())
            {
#ifdef DBG_UTIL
                OUString sName;
                xParam->getPropertyValue(PROPERTY_NAME) >>= sName;
                OSL_ENSURE(sName.equals(pFinalValues->Name), "SbaXDataBrowserController::approveParameter: suspicious value names!");
#endif
                try { xParam->setPropertyValue(PROPERTY_VALUE, pFinalValues->Value); }
                catch(Exception&)
                {
                    SAL_WARN("dbaccess.ui", "SbaXDataBrowserController::approveParameter: setting one of the properties failed!");
                }
            }
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    return true;
}

sal_Bool SbaXDataBrowserController::approveReset(const css::lang::EventObject& /*rEvent*/) throw( RuntimeException, std::exception )
{
    return true;
}

void SbaXDataBrowserController::resetted(const css::lang::EventObject& rEvent) throw( RuntimeException, std::exception )
{
    OSL_ENSURE(rEvent.Source == getControlModel(), "SbaXDataBrowserController::resetted : where did this come from ?");
    (void)rEvent;
    setCurrentModified( false );
}

sal_Bool SbaXDataBrowserController::confirmDelete(const css::sdb::RowChangeEvent& /*aEvent*/) throw( RuntimeException, std::exception )
{
    if (ScopedVclPtrInstance<MessageDialog>(getBrowserView(), ModuleRes(STR_QUERY_BRW_DELETE_ROWS), VCL_MESSAGE_QUESTION, VCL_BUTTONS_YES_NO)->Execute() != RET_YES)
        return false;

    return true;
}

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
                    bool bInsertPrivilege = ( m_nRowSetPrivileges & Privilege::INSERT) != 0;
                    bool bAllowInsertions = true;
                    try
                    {
                        Reference< XPropertySet > xRowSetProps( getRowSet(), UNO_QUERY_THROW );
                        OSL_VERIFY( xRowSetProps->getPropertyValue("AllowInserts") >>= bAllowInsertions );
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
                    bool bDeletePrivilege = ( m_nRowSetPrivileges & Privilege::INSERT) != 0;
                    bool bAllowDeletions = true;
                    sal_Int32 nRowCount = 0;
                    bool bInsertionRow = false;
                    try
                    {
                        Reference< XPropertySet > xRowSetProps( getRowSet(), UNO_QUERY_THROW );
                        OSL_VERIFY( xRowSetProps->getPropertyValue("AllowDeletes") >>= bAllowDeletions );
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
                SAL_FALLTHROUGH;
            case ID_BROWSER_PASTE:
            case ID_BROWSER_CUT:
            {
                CellControllerRef xCurrentController = getBrowserView()->getVclControl()->Controller();
                if (xCurrentController.Is() && nullptr != dynamic_cast< const EditCellController* >(xCurrentController.get()))
                {
                    Edit& rEdit = static_cast<Edit&>(xCurrentController->GetWindow());
                    bool bHasLen = (rEdit.GetSelection().Len() != 0);
                    bool bIsReadOnly = rEdit.IsReadOnly();
                    switch (nId)
                    {
                        case ID_BROWSER_CUT:    aReturn.bEnabled = m_aCurrentFrame.isActive() && bHasLen && !bIsReadOnly; break;
                        case SID_COPY   :       aReturn.bEnabled = m_aCurrentFrame.isActive() && bHasLen; break;
                        case ID_BROWSER_PASTE:
                            aReturn.bEnabled = m_aCurrentFrame.isActive() && !bIsReadOnly;
                            if(aReturn.bEnabled)
                            {
                                aReturn.bEnabled = aReturn.bEnabled && IsFormatSupported( m_aSystemClipboard.GetDataFlavorExVector(), SotClipboardFormatId::STRING );
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
                    aReturn.bEnabled = true;
                    break;
                }
                SAL_FALLTHROUGH;
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
                aReturn.bEnabled = true;
                break;

            case ID_BROWSER_REDO:
                aReturn.bEnabled = false;   // simply forget it ;). no redo possible.
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
                    aReturn.bEnabled = true;

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
                bool bInsertAllowedAndPossible = ((nDataSourcePrivileges & css::sdbcx::Privilege::INSERT) != 0) && ::comphelper::getBOOL(xDataSourceSet->getPropertyValue("AllowInserts"));
                bool bUpdateAllowedAndPossible = ((nDataSourcePrivileges & css::sdbcx::Privilege::UPDATE) != 0) && ::comphelper::getBOOL(xDataSourceSet->getPropertyValue("AllowUpdates"));
                bool bDeleteAllowedAndPossible = ((nDataSourcePrivileges & css::sdbcx::Privilege::DELETE) != 0) && ::comphelper::getBOOL(xDataSourceSet->getPropertyValue("AllowDeletes"));
                if (!bInsertAllowedAndPossible && !bUpdateAllowedAndPossible && !bDeleteAllowedAndPossible)
                    break;  // no insert/update/delete -> no edit mode

                if (!isValidCursor() || !isLoaded())
                    break;  // no cursor -> no edit mode

                aReturn.bEnabled = true;

                sal_Int16 nGridMode = getBrowserView()->getVclControl()->GetOptions();
                aReturn.bChecked = nGridMode > DbGridControl::OPT_READONLY;
            }
            break;
            case ID_BROWSER_FILTERED:
            {
                aReturn.bEnabled = false;
                Reference< XPropertySet >  xActiveSet(getRowSet(), UNO_QUERY);
                OUString aFilter = ::comphelper::getString(xActiveSet->getPropertyValue(PROPERTY_FILTER));
                OUString aHaving = ::comphelper::getString(xActiveSet->getPropertyValue(PROPERTY_HAVING_CLAUSE));
                if ( !(aFilter.isEmpty() && aHaving.isEmpty()) )
                {
                    xActiveSet->getPropertyValue( PROPERTY_APPLYFILTER ) >>= aReturn.bChecked;
                    aReturn.bEnabled = true;
                }
                else
                {
                    aReturn.bChecked = false;
                    aReturn.bEnabled = false;
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

void SbaXDataBrowserController::applyParserOrder(const OUString& _rOldOrder,const Reference< XSingleSelectQueryComposer >& _xParser)
{
    Reference< XPropertySet > xFormSet(getRowSet(), UNO_QUERY);
    if (!m_xLoadable.is())
    {
        SAL_WARN("dbaccess.ui","SbaXDataBrowserController::applyParserOrder: invalid row set!");
        return;
    }

    sal_uInt16 nPos = getCurrentColumnPosition();
    bool bSuccess = false;
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

void SbaXDataBrowserController::applyParserFilter(const OUString& _rOldFilter, bool _bOldFilterApplied,const ::OUString& _sOldHaving,const Reference< XSingleSelectQueryComposer >& _xParser)
{
    Reference< XPropertySet >  xFormSet(getRowSet(), UNO_QUERY);
    if (!m_xLoadable.is())
    {
        SAL_WARN("dbaccess.ui", "SbaXDataBrowserController::applyParserFilter: invalid row set!");
        return;
    }

    sal_uInt16 nPos = getCurrentColumnPosition();

    bool bSuccess = false;
    try
    {
        FormErrorHelper aError(this);
        xFormSet->setPropertyValue(PROPERTY_FILTER, makeAny(_xParser->getFilter()));
        xFormSet->setPropertyValue(PROPERTY_HAVING_CLAUSE, makeAny(_xParser->getHavingClause()));
        xFormSet->setPropertyValue(PROPERTY_APPLYFILTER, css::uno::Any(true));

        bSuccess = reloadForm(m_xLoadable);
    }
    catch(Exception&)
    {
    }

    if (!bSuccess)
    {
        xFormSet->setPropertyValue(PROPERTY_FILTER, makeAny(_rOldFilter));
        xFormSet->setPropertyValue(PROPERTY_HAVING_CLAUSE, makeAny(_sOldHaving));
        xFormSet->setPropertyValue(PROPERTY_APPLYFILTER, css::uno::Any(_bOldFilterApplied));

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

Reference< XSingleSelectQueryComposer > SbaXDataBrowserController::createParser_nothrow()
{
    Reference< XSingleSelectQueryComposer > xComposer;
    try
    {
        const Reference< XPropertySet > xRowSetProps( getRowSet(), UNO_QUERY_THROW );
        const Reference< XMultiServiceFactory > xFactory(
            xRowSetProps->getPropertyValue( PROPERTY_ACTIVE_CONNECTION ), UNO_QUERY_THROW );
        xComposer.set( xFactory->createInstance( SERVICE_NAME_SINGLESELECTQUERYCOMPOSER ), UNO_QUERY_THROW );

        OUString sActiveCommand;
        OSL_VERIFY( xRowSetProps->getPropertyValue( PROPERTY_ACTIVECOMMAND ) >>= sActiveCommand );
        if ( !sActiveCommand.isEmpty() )
        {
            xComposer->setElementaryQuery( sActiveCommand );
        }
        else
        {
            OUString sCommand;
            OSL_VERIFY( xRowSetProps->getPropertyValue( PROPERTY_COMMAND ) >>= sCommand );
            sal_Int32 nCommandType = CommandType::COMMAND;
            OSL_VERIFY( xRowSetProps->getPropertyValue( PROPERTY_COMMAND_TYPE ) >>= nCommandType );
            xComposer->setCommand( sCommand, nCommandType );
        }

        OUString sFilter;
        OSL_VERIFY( xRowSetProps->getPropertyValue( PROPERTY_FILTER ) >>= sFilter );
        xComposer->setFilter( sFilter );

        OUString sHavingClause;
        OSL_VERIFY( xRowSetProps->getPropertyValue( PROPERTY_HAVING_CLAUSE ) >>= sHavingClause );
        xComposer->setHavingClause( sHavingClause );

        OUString sOrder;
        OSL_VERIFY( xRowSetProps->getPropertyValue( PROPERTY_ORDER ) >>= sOrder );
        xComposer->setOrder( sOrder );
    }
    catch ( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
    return xComposer;
}

void SbaXDataBrowserController::ExecuteFilterSortCrit(bool bFilter)
{
    if (!SaveModified())
        return;

    Reference< XPropertySet >  xFormSet(getRowSet(), UNO_QUERY);

    const OUString sOldVal = bFilter ? m_xParser->getFilter() : m_xParser->getOrder();
    const OUString sOldHaving = m_xParser->getHavingClause();
    Reference< XSingleSelectQueryComposer > xParser = createParser_nothrow();
    try
    {
        Reference< css::sdbcx::XColumnsSupplier> xSup = getColumnsSupplier();
        Reference< XConnection> xCon(xFormSet->getPropertyValue(PROPERTY_ACTIVE_CONNECTION),UNO_QUERY);
        if(bFilter)
        {
            ScopedVclPtrInstance< DlgFilterCrit > aDlg( getBrowserView(), getORB(), xCon, xParser, xSup->getColumns() );
            if ( !aDlg->Execute() )
                return; // if so we don't need to update the grid
            aDlg->BuildWherePart();
        }
        else
        {
            ScopedVclPtrInstance< DlgOrderCrit > aDlg( getBrowserView(),xCon,xParser,xSup->getColumns() );
            if(!aDlg->Execute())
            {
                return; // if so we don't need to actualize the grid
            }
            aDlg->BuildOrderPart();
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

    OUString sNewVal = bFilter ? xParser->getFilter() : xParser->getOrder();
    bool bOldFilterApplied(false);
    if (bFilter)
    {
        try { bOldFilterApplied = ::comphelper::getBOOL(xFormSet->getPropertyValue(PROPERTY_APPLYFILTER)); } catch(Exception&) { } ;
    }

    OUString sNewHaving = xParser->getHavingClause();
    if ( sOldVal.equals(sNewVal) && (!bFilter || sOldHaving.equals(sNewHaving)) )
        // nothing to be done
        return;

    if (bFilter)
        applyParserFilter(sOldVal, bOldFilterApplied,sOldHaving,xParser);
    else
        applyParserOrder(sOldVal,xParser);

    ::comphelper::disposeComponent(xParser);
}

void SbaXDataBrowserController::ExecuteSearch()
{
    // calculate the control source of the active field
    Reference< css::form::XGrid >  xGrid(getBrowserView()->getGridControl(), UNO_QUERY);
    OSL_ENSURE(xGrid.is(), "SbaXDataBrowserController::ExecuteSearch : the control should have an css::form::XGrid interface !");

    Reference< css::form::XGridPeer >  xGridPeer(getBrowserView()->getGridControl()->getPeer(), UNO_QUERY);
    Reference< css::container::XIndexContainer >  xColumns = xGridPeer->getColumns();
    OSL_ENSURE(xGridPeer.is() && xColumns.is(), "SbaXDataBrowserController::ExecuteSearch : invalid peer !");

    sal_Int16 nViewCol = xGrid->getCurrentColumnPosition();
    sal_Int16 nModelCol = getBrowserView()->View2ModelPos(nViewCol);

    Reference< XPropertySet >  xCurrentCol(xColumns->getByIndex(nModelCol),UNO_QUERY);
    OUString sActiveField = ::comphelper::getString(xCurrentCol->getPropertyValue(PROPERTY_CONTROLSOURCE));

    // the text within the current cell
    OUString sInitialText;
    Reference< css::container::XIndexAccess >  xColControls(xGridPeer, UNO_QUERY);
    Reference< XInterface >  xCurControl(xColControls->getByIndex(nViewCol),UNO_QUERY);
    OUString aInitialText;
    if (IsSearchableControl(xCurControl, &aInitialText))
        sInitialText = aInitialText;

    // prohibit the synchronization of the grid's display with the cursor's position
    Reference< XPropertySet >  xModelSet(getControlModel(), UNO_QUERY);
    OSL_ENSURE(xModelSet.is(), "SbaXDataBrowserController::ExecuteSearch : no model set ?!");
    xModelSet->setPropertyValue("DisplayIsSynchron", css::uno::Any(false));
    xModelSet->setPropertyValue("AlwaysShowCursor", css::uno::Any(true));
    xModelSet->setPropertyValue("CursorColor", makeAny(sal_Int32(COL_LIGHTRED)));

    Reference< css::util::XNumberFormatsSupplier >  xNFS(::dbtools::getNumberFormats(::dbtools::getConnection(m_xRowSet), true, getORB()));

    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    AbstractFmSearchDialog* pDialog = nullptr;
    if ( pFact )
    {
        ::std::vector< OUString > aContextNames;
        aContextNames.push_back( OUString("Standard") );
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
    xModelSet->setPropertyValue("DisplayIsSynchron", css::uno::Any(true));
    xModelSet->setPropertyValue("AlwaysShowCursor", css::uno::Any(false));
    xModelSet->setPropertyValue("CursorColor", Any());
}

void SbaXDataBrowserController::Execute(sal_uInt16 nId, const Sequence< PropertyValue >& _rArgs)
{
    bool bSortUp = true;

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
                SAL_WARN("dbaccess.ui", "Exception caught!");
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
                bool bApplied = ::comphelper::getBOOL(xActiveSet->getPropertyValue(PROPERTY_APPLYFILTER));
                xActiveSet->setPropertyValue(PROPERTY_APPLYFILTER, css::uno::Any(!bApplied));
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
                    // give the user a chance to save the current record (if necessary)
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
            SAL_FALLTHROUGH;
        case ID_BROWSER_CUT:
        case ID_BROWSER_PASTE:
        {
            CellControllerRef xCurrentController = getBrowserView()->getVclControl()->Controller();
            if (!xCurrentController.Is())
                // should be intercepted by GetState. Normally.
                // Unfortunately ID_BROWSER_PASTE is a 'fast call' slot, which means it may be executed without checking if it is
                // enabled. This would be really deadly herein if the current cell has no controller ...
                return;

            Edit& rEdit = static_cast<Edit&>(xCurrentController->GetWindow());
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
            bSortUp = false;
            SAL_FALLTHROUGH;
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
            const OUString sOldSort = xParser->getOrder();
            bool bParserSuccess = false;
            HANDLE_SQL_ERRORS(
                xParser->setOrder(OUString()); xParser->appendOrderByColumn(xField, bSortUp),
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
            bool bHaving = false;
            OUString sName;
            xField->getPropertyValue(PROPERTY_NAME) >>= sName;
            Reference< XColumnsSupplier > xColumnsSupplier(m_xParser, UNO_QUERY);
            Reference< css::container::XNameAccess >  xCols = xColumnsSupplier.is() ? xColumnsSupplier->getColumns() : Reference< css::container::XNameAccess > ();
            if ( xCols.is() && xCols->hasByName(sName) )
            {
                Reference<XPropertySet> xProp(xCols->getByName(sName),UNO_QUERY);
                static const char sAgg[] = "AggregateFunction";
                if ( xProp->getPropertySetInfo()->hasPropertyByName(sAgg) )
                    xProp->getPropertyValue(sAgg) >>= bHaving;
            }

            Reference< XSingleSelectQueryComposer > xParser = createParser_nothrow();
            const OUString sOldFilter = xParser->getFilter();
            const OUString sOldHaving = xParser->getHavingClause();

            Reference< XPropertySet >  xFormSet(getRowSet(), UNO_QUERY);
            bool bApplied = ::comphelper::getBOOL(xFormSet->getPropertyValue(PROPERTY_APPLYFILTER));
            // do we have a filter but it's not applied ?
            // -> completely overwrite it, else append one
            if (!bApplied)
            {
                DO_SAFE( (bHaving ? xParser->setHavingClause(OUString()) : xParser->setFilter(::OUString())), "SbaXDataBrowserController::Execute : caught an exception while resetting the new filter !" );
            }

            bool bParserSuccess = false;

            const sal_Int32 nOp = SQLFilterOperator::EQUAL;

            if ( bHaving )
            {
                HANDLE_SQL_ERRORS(
                    xParser->appendHavingClauseByColumn(xField,true,nOp),
                    bParserSuccess,
                    ModuleRes(SBA_BROWSER_SETTING_FILTER).toString(),
                    "SbaXDataBrowserController::Execute : caught an exception while composing the new filter !"
                )
            }
            else
            {
                HANDLE_SQL_ERRORS(
                    xParser->appendFilterByColumn(xField,true,nOp),
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
            ExecuteFilterSortCrit(false);
            break;

        case ID_BROWSER_FILTERCRIT:
            ExecuteFilterSortCrit(true);
            InvalidateFeature(ID_BROWSER_FILTERED);
            break;

        case ID_BROWSER_REMOVEFILTER:
        {
            if (!SaveModified())
                break;

            bool bNeedPostReload = preReloadForm();
            // reset the filter and the sort property simultaneously so only _one_ new statement has to be
            // sent
            Reference< XPropertySet >  xSet(getRowSet(), UNO_QUERY);
            if ( xSet.is() )
            {
                xSet->setPropertyValue(PROPERTY_FILTER,makeAny(OUString()));
                xSet->setPropertyValue(PROPERTY_HAVING_CLAUSE,makeAny(OUString()));
                xSet->setPropertyValue(PROPERTY_ORDER,makeAny(OUString()));
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
            if ( SaveModified( false ) )
                setCurrentModified( false );
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
                    Reference< css::form::XReset >  xReset(getControlModel(), UNO_QUERY);
                    if (xReset.is())
                        xReset->reset();
                }
            }
            catch(SQLException&)
            {
            }

            setCurrentModified( false );
        }
    }
}

bool SbaXDataBrowserController::SaveModified(bool bAskFor)
{
    if ( bAskFor && GetState(ID_BROWSER_SAVERECORD).bEnabled )
    {
        getBrowserView()->getVclControl()->GrabFocus();

        ScopedVclPtrInstance<MessageDialog> aQry( getBrowserView()->getVclControl(),
                                                  "SaveModifiedDialog",
                                                  "dbaccess/ui/savemodifieddialog.ui" );

        switch (aQry->Execute())
        {
            case RET_NO:
                Execute(ID_BROWSER_UNDORECORD,Sequence<PropertyValue>());
                return true;
            case RET_CANCEL:
                return false;
        }
    }

    if ( !CommitCurrent() ) // das aktuelle Control committen lassen
        return false;

    Reference< XPropertySet >  xFormSet(getRowSet(), UNO_QUERY);
    bool bResult = false;
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
        bResult = true;
    }
    catch(SQLException&)
    {
    }
    catch(Exception&)
    {
        SAL_WARN("dbaccess.ui", "SbaXDataBrowserController::SaveModified : could not save the current record !");
        bResult = false;
    }

    InvalidateFeature(ID_BROWSER_SAVERECORD);
    InvalidateFeature(ID_BROWSER_UNDORECORD);
    return bResult;
}

bool SbaXDataBrowserController::CommitCurrent()
{
    if (!getBrowserView())
        return true;

    Reference< css::awt::XControl >  xActiveControl(getBrowserView()->getGridControl());
    Reference< css::form::XBoundControl >  xLockingTest(xActiveControl, UNO_QUERY);
    bool bControlIsLocked = xLockingTest.is() && xLockingTest->getLock();
    if (xActiveControl.is() && !bControlIsLocked)
    {
        // zunaechst das Control fragen ob es das IFace unterstuetzt
        Reference< css::form::XBoundComponent >  xBoundControl(xActiveControl, UNO_QUERY);
        if (!xBoundControl.is())
            xBoundControl.set(xActiveControl->getModel(), UNO_QUERY);
        if (xBoundControl.is() && !xBoundControl->commit())
            return false;
    }
    return true;
}

void SbaXDataBrowserController::setCurrentModified( bool _bSet )
{
    m_bCurrentlyModified = _bSet;
    InvalidateFeature( ID_BROWSER_SAVERECORD );
    InvalidateFeature( ID_BROWSER_UNDORECORD );
}

void SbaXDataBrowserController::RowChanged()
{
    setCurrentModified( false );
}

void SbaXDataBrowserController::ColumnChanged()
{
    InvalidateFeature(ID_BROWSER_SORTUP);
    InvalidateFeature(ID_BROWSER_SORTDOWN);
    InvalidateFeature(ID_BROWSER_ORDERCRIT);
    InvalidateFeature(ID_BROWSER_FILTERCRIT);
    InvalidateFeature(ID_BROWSER_AUTOFILTER);
    InvalidateFeature(ID_BROWSER_REMOVEFILTER);

    setCurrentModified( false );
}

void SbaXDataBrowserController::SelectionChanged()
{
    // not interested in
}

void SbaXDataBrowserController::CellActivated()
{
    m_aInvalidateClipboard.Start();
    OnInvalidateClipboard( nullptr );
}

void SbaXDataBrowserController::CellDeactivated()
{
    m_aInvalidateClipboard.Stop();
    OnInvalidateClipboard( nullptr );
}

IMPL_LINK_NOARG_TYPED(SbaXDataBrowserController, OnClipboardChanged, TransferableDataHelper*, void)
{
    SolarMutexGuard aGuard;
    OnInvalidateClipboard( nullptr );
}

IMPL_LINK_TYPED(SbaXDataBrowserController, OnInvalidateClipboard, Timer*, _pTimer, void)
{
    InvalidateFeature(ID_BROWSER_CUT);
    InvalidateFeature(ID_BROWSER_COPY);

    // if the invalidation was triggered by the timer, we do not need to invalidate PASTE.
    // The timer is only for checking the CUT/COPY slots regulary, which depend on the
    // selection state of the active cell
    // TODO: get a callback at the Edit which allows to be notified when the selection
    // changes. This would be much better than this cycle-eating polling mechanism here ....
    if ( _pTimer != &m_aInvalidateClipboard )
        InvalidateFeature(ID_BROWSER_PASTE);
}

Reference< XPropertySet >  SbaXDataBrowserController::getBoundField() const
{
    Reference< XPropertySet >  xEmptyReturn;

    // get the current column from the grid
    Reference< css::form::XGrid >  xGrid(getBrowserView()->getGridControl(), UNO_QUERY);
    if (!xGrid.is())
            return xEmptyReturn;
    sal_uInt16 nViewPos = xGrid->getCurrentColumnPosition();
    sal_uInt16 nCurrentCol = getBrowserView()->View2ModelPos(nViewPos);
    if (nCurrentCol == (sal_uInt16)-1)
        return xEmptyReturn;

    // get the according column from the model
    Reference< css::container::XIndexContainer >  xCols(getControlModel(), UNO_QUERY);
    Reference< XPropertySet >  xCurrentCol(xCols->getByIndex(nCurrentCol),UNO_QUERY);
    if (!xCurrentCol.is())
        return xEmptyReturn;

    xEmptyReturn.set(xCurrentCol->getPropertyValue(PROPERTY_BOUNDFIELD) ,UNO_QUERY);
    return xEmptyReturn;
}

IMPL_LINK_TYPED(SbaXDataBrowserController, OnSearchContextRequest, FmSearchContext&, rContext, sal_uInt32)
{
    Reference< css::container::XIndexAccess >  xPeerContainer(getBrowserView()->getGridControl(), UNO_QUERY);

    // check all grid columns for their control source
    Reference< css::container::XIndexAccess >  xModelColumns(getFormComponent(), UNO_QUERY);
    OSL_ENSURE(xModelColumns.is(), "SbaXDataBrowserController::OnSearchContextRequest : there is a grid control without columns !");
        // the case 'no columns' should be indicated with an empty container, I think ...
    OSL_ENSURE(xModelColumns->getCount() >= xPeerContainer->getCount(), "SbaXDataBrowserController::OnSearchContextRequest : impossible : have more view than model columns !");

    OUString sFieldList;
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
        OUString aName = ::comphelper::getString(xCurrentColModel->getPropertyValue(PROPERTY_CONTROLSOURCE));

        sFieldList += aName + ";";

        rContext.arrFields.push_back(xCurrentColumn);
    }
    sFieldList = comphelper::string::stripEnd(sFieldList, ';');

    rContext.xCursor.set(getRowSet(),UNO_QUERY);
    rContext.strUsedFields = sFieldList;

    // if the cursor is in a mode other than STANDARD -> reset
    Reference< XPropertySet >  xCursorSet(rContext.xCursor, UNO_QUERY);
    OSL_ENSURE(xCursorSet.is() && !::comphelper::getBOOL(xCursorSet->getPropertyValue(PROPERTY_ISMODIFIED)),
        "SbaXDataBrowserController::OnSearchContextRequest : please do not call for cursors with modified rows !");
    if (xCursorSet.is() && ::comphelper::getBOOL(xCursorSet->getPropertyValue(PROPERTY_ISNEW)))
    {
        Reference< XResultSetUpdate >  xUpdateCursor(rContext.xCursor, UNO_QUERY);
        xUpdateCursor->moveToCurrentRow();
    }
    return rContext.arrFields.size();
}

IMPL_LINK_TYPED(SbaXDataBrowserController, OnFoundData, FmFoundRecordInformation&, rInfo, void)
{
    Reference< css::sdbcx::XRowLocate >  xCursor(getRowSet(), UNO_QUERY);
    OSL_ENSURE(xCursor.is(), "SbaXDataBrowserController::OnFoundData : xCursor is empty");

    // move the cursor
    xCursor->moveToBookmark(rInfo.aPosition);

    // let the grid sync its display with the cursor
    Reference< XPropertySet >  xModelSet(getControlModel(), UNO_QUERY);
    OSL_ENSURE(xModelSet.is(), "SbaXDataBrowserController::OnFoundData : no model set ?!");
    Any aOld = xModelSet->getPropertyValue("DisplayIsSynchron");
    xModelSet->setPropertyValue("DisplayIsSynchron", css::uno::Any(true));
    xModelSet->setPropertyValue("DisplayIsSynchron", aOld);

    // and move to the field
    Reference< css::container::XIndexAccess >  aColumnControls(getBrowserView()->getGridControl()->getPeer(), UNO_QUERY);
    sal_uInt16 nViewPos;

    for ( nViewPos = 0; nViewPos < aColumnControls->getCount(); ++nViewPos )
    {
        Reference< XInterface >  xCurrent(aColumnControls->getByIndex(nViewPos),UNO_QUERY);
        if (IsSearchableControl(xCurrent))
        {
            if (rInfo.nFieldPos)
                --rInfo.nFieldPos;
            else
                break;
        }
    }

    Reference< css::form::XGrid >  xGrid(getBrowserView()->getGridControl(), UNO_QUERY);
    xGrid->setCurrentColumnPosition(nViewPos);
}

IMPL_LINK_TYPED(SbaXDataBrowserController, OnCanceledNotFound, FmFoundRecordInformation&, rInfo, void)
{
    Reference< css::sdbcx::XRowLocate >  xCursor(getRowSet(), UNO_QUERY);

    try
    {
        OSL_ENSURE(xCursor.is(), "SbaXDataBrowserController::OnCanceledNotFound : xCursor is empty");
        // move the cursor
        xCursor->moveToBookmark(rInfo.aPosition);
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
        Any aOld = xModelSet->getPropertyValue("DisplayIsSynchron");
        xModelSet->setPropertyValue("DisplayIsSynchron", css::uno::Any(true));
        xModelSet->setPropertyValue("DisplayIsSynchron", aOld);
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

IMPL_LINK_NOARG_TYPED(SbaXDataBrowserController, OnAsyncGetCellFocus, void*, void)
{
    SbaGridControl* pVclGrid = getBrowserView() ? getBrowserView()->getVclControl() : nullptr;
    // if we have a controller, but the window for the controller doesn't have the focus, we correct this
    if (pVclGrid && pVclGrid->IsEditing() && pVclGrid->HasChildPathFocus())
        pVclGrid->Controller()->GetWindow().GrabFocus();
}

void SbaXDataBrowserController::criticalFail()
{
    InvalidateAll();
    m_nRowSetPrivileges = 0;
}

void SbaXDataBrowserController::LoadFinished(bool /*bWasSynch*/)
{
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

        // switch the control to alive mode
        getBrowserView()->getGridControl()->setDesignMode(false);

        initializeParser();

        InvalidateAll();

        m_aAsyncGetCellFocus.Call();
    }
}

void SbaXDataBrowserController::initializeParser() const
{
    if ( !m_xParser.is() )
    {
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
            m_xParser = nullptr;
            // no further handling, we ignore the error
        }
    }
}

void SbaXDataBrowserController::loaded(const EventObject& /*aEvent*/) throw( RuntimeException, std::exception )
{
    // not interested in
    // we're loading within an separate thread and have a handling for its "finished event"
}

void SbaXDataBrowserController::unloading(const EventObject& /*aEvent*/) throw( RuntimeException, std::exception )
{
    // not interested in
}

void SbaXDataBrowserController::unloaded(const EventObject& /*aEvent*/) throw( RuntimeException, std::exception )
{
    m_xParser.clear();
    InvalidateAll();
        // do this asynchronously, there are other listeners reacting on this message ...
        // (it's a little hack : the grid columns are listening to this event, too, and their bound field may
        // change as a reaction on that event. as we have no chance to be notified of this change (which is
        // the one we're interested in) we give them time to do what they want to before invalidating our
        // bound-field-dependent slots ....
}

void SbaXDataBrowserController::reloading(const EventObject& /*aEvent*/) throw( RuntimeException, std::exception )
{
    // not interested in
}

void SbaXDataBrowserController::reloaded(const EventObject& /*aEvent*/) throw( RuntimeException, std::exception )
{
    InvalidateAll();
        // do this asynchronously, there are other listeners reacting on this message ...
        // (it's a little hack : the grid columns are listening to this event, too, and their bound field may
        // change as a reaction on that event. as we have no chance to be notified of this change (which is
        // the one we're interested in) we give them time to do what they want to before invalidating our
        // bound-field-dependent slots ....
}

void SbaXDataBrowserController::enterFormAction()
{
    if ( !m_nFormActionNestingLevel )
        // first action -> reset
        m_aCurrentError.clear();

    ++m_nFormActionNestingLevel;
}

void SbaXDataBrowserController::leaveFormAction()
{
    OSL_ENSURE( m_nFormActionNestingLevel > 0, "SbaXDataBrowserController::leaveFormAction : invalid call !" );
    if ( --m_nFormActionNestingLevel > 0 )
        return;

    if ( !m_aCurrentError.isValid() )
        return;

    m_aAsyncDisplayError.Call();
}

bool SbaXDataBrowserController::isLoaded() const
{
    return m_xLoadable.is() && m_xLoadable->isLoaded();
}

bool SbaXDataBrowserController::isValidCursor() const
{
    if (!m_xColumnsSupplier.is())
        return false;
    Reference< css::container::XNameAccess >  xCols = m_xColumnsSupplier->getColumns();
    if (!xCols.is() || !xCols->hasElements())
        return false;

    bool bIsValid = !(m_xRowSet->isBeforeFirst() || m_xRowSet->isAfterLast());
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

sal_Int16 SbaXDataBrowserController::getCurrentColumnPosition()
{
    Reference< css::form::XGrid >  xGrid(getBrowserView()->getGridControl(), UNO_QUERY);
    sal_Int16 nViewPos = -1;
    try
    {
        if ( xGrid.is() )
            nViewPos = xGrid->getCurrentColumnPosition();
    }
    catch(Exception&) {}
    return nViewPos;
}

void SbaXDataBrowserController::setCurrentColumnPosition( sal_Int16 _nPos )
{
    Reference< css::form::XGrid >  xGrid(getBrowserView()->getGridControl(), UNO_QUERY);
    try
    {
        if ( -1 != _nPos )
            xGrid->setCurrentColumnPosition(_nPos);
    }
    catch(Exception&) {}
}

void SbaXDataBrowserController::BeforeDrop()
{
    Reference< css::sdb::XSQLErrorBroadcaster >  xFormError(getRowSet(), UNO_QUERY);
    if (xFormError.is())
        xFormError->removeSQLErrorListener(static_cast<css::sdb::XSQLErrorListener*>(this));
}

void SbaXDataBrowserController::AfterDrop()
{
    Reference< css::sdb::XSQLErrorBroadcaster >  xFormError(getRowSet(), UNO_QUERY);
    if (xFormError.is())
        xFormError->addSQLErrorListener(static_cast<css::sdb::XSQLErrorListener*>(this));
}

void SbaXDataBrowserController::addColumnListeners(const Reference< css::awt::XControlModel > & _xGridControlModel)
{
// ... all the grid columns
    Reference< css::container::XIndexContainer >  xColumns(_xGridControlModel, UNO_QUERY);
    if (xColumns.is())
    {
        sal_Int32 nCount = xColumns->getCount();
        for (sal_Int32 i=0; i < nCount; ++i)
        {
            Reference< XPropertySet >  xCol(xColumns->getByIndex(i),UNO_QUERY);
            AddColumnListener(xCol);
        }
    }
}

bool SbaXDataBrowserController::InitializeGridModel(const Reference< css::form::XFormComponent > & /*xGrid*/)
{
    return true;
}

}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
