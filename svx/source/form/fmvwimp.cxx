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


#include "fmdocumentclassification.hxx"
#include "fmobj.hxx"
#include "fmpgeimp.hxx"
#include "fmprop.hrc"
#include "svx/fmresids.hrc"
#include "fmservs.hxx"
#include "fmshimp.hxx"
#include "svx/fmtools.hxx"
#include "fmundo.hxx"
#include "fmvwimp.hxx"
#include "formcontrolfactory.hxx"
#include "svx/sdrpaintwindow.hxx"
#include "svx/svditer.hxx"
#include "svx/dataaccessdescriptor.hxx"
#include "svx/dialmgr.hxx"
#include "svx/fmglob.hxx"
#include "svx/fmmodel.hxx"
#include "svx/fmpage.hxx"
#include "svx/fmshell.hxx"
#include "svx/fmview.hxx"
#include "svx/sdrpagewindow.hxx"
#include "svx/svdogrp.hxx"
#include "svx/svdpagv.hxx"
#include "svx/xmlexchg.hxx"

#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/style/VerticalAlignment.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/sdbc/XRowSet.hpp>
#include <com/sun/star/form/XLoadable.hpp>
#include <com/sun/star/awt/VisualEffect.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <com/sun/star/util/XNumberFormats.hpp>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <com/sun/star/form/FormComponentType.hpp>
#include <com/sun/star/form/FormButtonType.hpp>
#include <com/sun/star/form/XReset.hpp>
#include <com/sun/star/form/binding/XBindableValue.hpp>
#include <com/sun/star/form/binding/XValueBinding.hpp>
#include <com/sun/star/form/submission/XSubmissionSupplier.hpp>
#include <com/sun/star/awt/XTabControllerModel.hpp>
#include <com/sun/star/awt/XControlContainer.hpp>
#include <com/sun/star/awt/XTabController.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/awt/XControl.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/sdbc/XPreparedStatement.hpp>
#include <com/sun/star/sdb/XQueriesSupplier.hpp>
#include <com/sun/star/container/XContainer.hpp>

#include <comphelper/enumhelper.hxx>
#include <comphelper/extract.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <comphelper/numbers.hxx>
#include <comphelper/property.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <unotools/moduleoptions.hxx>
#include <tools/diagnose_ex.h>
#include <vcl/msgbox.hxx>
#include <vcl/stdtext.hxx>
#include <osl/mutex.hxx>
#include <rtl/logfile.hxx>

#include <algorithm>

using namespace ::comphelper;
using namespace ::svx;
using namespace ::svxform;

    using namespace ::com::sun::star;
    /** === begin UNO using === **/
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::UNO_SET_THROW;
    using ::com::sun::star::uno::Type;
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::makeAny;
    using ::com::sun::star::style::VerticalAlignment_MIDDLE;
    using ::com::sun::star::form::FormButtonType_SUBMIT;
    using ::com::sun::star::form::binding::XValueBinding;
    using ::com::sun::star::form::binding::XBindableValue;
    using ::com::sun::star::lang::XComponent;
    using ::com::sun::star::container::XIndexAccess;
    using ::com::sun::star::form::XForm;
    using ::com::sun::star::form::runtime::XFormController;
    using ::com::sun::star::script::XEventAttacherManager;
    using ::com::sun::star::awt::XTabControllerModel;
    using ::com::sun::star::container::XChild;
    using ::com::sun::star::container::XEnumeration;
    using ::com::sun::star::task::XInteractionHandler;
    using ::com::sun::star::lang::XInitialization;
    using ::com::sun::star::awt::XTabController;
    using ::com::sun::star::lang::XUnoTunnel;
    using ::com::sun::star::awt::XControlContainer;
    using ::com::sun::star::awt::XControl;
    using ::com::sun::star::form::XFormComponent;
    using ::com::sun::star::form::XForm;
    using ::com::sun::star::lang::IndexOutOfBoundsException;
    using ::com::sun::star::lang::WrappedTargetException;
    using ::com::sun::star::container::XContainer;
    using ::com::sun::star::container::ContainerEvent;
    using ::com::sun::star::lang::EventObject;
    using ::com::sun::star::beans::NamedValue;
    using ::com::sun::star::sdb::SQLErrorEvent;
    using ::com::sun::star::sdbc::XRowSet;
    using ::com::sun::star::beans::XPropertySet;
    using ::com::sun::star::container::XElementAccess;
    using ::com::sun::star::awt::XWindow;
    using ::com::sun::star::awt::FocusEvent;
    using ::com::sun::star::ui::dialogs::XExecutableDialog;
    using ::com::sun::star::sdbc::XDataSource;
    using ::com::sun::star::container::XIndexContainer;
    using ::com::sun::star::sdbc::XConnection;
    using ::com::sun::star::container::XNameAccess;
    using ::com::sun::star::sdb::SQLContext;
    using ::com::sun::star::sdbc::SQLWarning;
    using ::com::sun::star::sdbc::SQLException;
    using ::com::sun::star::util::XNumberFormatsSupplier;
    using ::com::sun::star::util::XNumberFormats;
    using ::com::sun::star::beans::XPropertySetInfo;
    /** === end UNO using === **/
    namespace FormComponentType = ::com::sun::star::form::FormComponentType;
    namespace CommandType = ::com::sun::star::sdb::CommandType;
    namespace DataType = ::com::sun::star::sdbc::DataType;

//------------------------------------------------------------------------------
class FmXFormView::ObjectRemoveListener : public SfxListener
{
    FmXFormView* m_pParent;
public:
    ObjectRemoveListener( FmXFormView* pParent );
    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint );
};

//========================================================================
DBG_NAME(FormViewPageWindowAdapter)
//------------------------------------------------------------------------
FormViewPageWindowAdapter::FormViewPageWindowAdapter( const ::comphelper::ComponentContext& _rContext, const SdrPageWindow& _rWindow, FmXFormView* _pViewImpl )
:   m_xControlContainer( _rWindow.GetControlContainer() ),
    m_aContext( _rContext ),
    m_pViewImpl( _pViewImpl ),
    m_pWindow( dynamic_cast< Window* >( &_rWindow.GetPaintWindow().GetOutputDevice() ) )
{
    DBG_CTOR(FormViewPageWindowAdapter,NULL);

    // create an XFormController for every form
    FmFormPage* pFormPage = dynamic_cast< FmFormPage* >( _rWindow.GetPageView().GetPage() );
    DBG_ASSERT( pFormPage, "FormViewPageWindowAdapter::FormViewPageWindowAdapter: no FmFormPage found!" );
    if ( pFormPage )
    {
        try
        {
            Reference< XIndexAccess > xForms( pFormPage->GetForms(), UNO_QUERY_THROW );
            sal_uInt32 nLength = xForms->getCount();
            for (sal_uInt32 i = 0; i < nLength; i++)
            {
                Reference< XForm > xForm( xForms->getByIndex(i), UNO_QUERY );
                if ( xForm.is() )
                    setController( xForm, NULL );
            }
        }
        catch (const Exception&)
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }
}
// -----------------------------------------------------------------------------
FormViewPageWindowAdapter::~FormViewPageWindowAdapter()
{
    DBG_DTOR(FormViewPageWindowAdapter,NULL);
}

//------------------------------------------------------------------
void FormViewPageWindowAdapter::dispose()
{
    for (   ::std::vector< Reference< XFormController > >::const_iterator i = m_aControllerList.begin();
            i != m_aControllerList.end();
            ++i
        )
    {
        try
        {
            Reference< XFormController > xController( *i, UNO_QUERY_THROW );

            // detaching the events
            Reference< XChild > xControllerModel( xController->getModel(), UNO_QUERY );
            if ( xControllerModel.is() )
            {
                Reference< XEventAttacherManager >  xEventManager( xControllerModel->getParent(), UNO_QUERY_THROW );
                Reference< XInterface > xControllerNormalized( xController, UNO_QUERY_THROW );
                xEventManager->detach( i - m_aControllerList.begin(), xControllerNormalized );
            }

            // dispose the formcontroller
            Reference< XComponent > xComp( xController, UNO_QUERY_THROW );
            xComp->dispose();
        }
        catch (const Exception&)
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    m_aControllerList.clear();
}


//------------------------------------------------------------------------------
sal_Bool SAL_CALL FormViewPageWindowAdapter::hasElements(void) throw( RuntimeException )
{
    return getCount() != 0;
}

//------------------------------------------------------------------------------
Type SAL_CALL  FormViewPageWindowAdapter::getElementType(void) throw( RuntimeException )
{
    return ::getCppuType((const Reference< XFormController>*)0);
}

// XEnumerationAccess
//------------------------------------------------------------------------------
Reference< XEnumeration >  SAL_CALL FormViewPageWindowAdapter::createEnumeration(void) throw( RuntimeException )
{
    return new ::comphelper::OEnumerationByIndex(this);
}

// XIndexAccess
//------------------------------------------------------------------------------
sal_Int32 SAL_CALL FormViewPageWindowAdapter::getCount(void) throw( RuntimeException )
{
    return m_aControllerList.size();
}

//------------------------------------------------------------------------------
Any SAL_CALL FormViewPageWindowAdapter::getByIndex(sal_Int32 nIndex) throw( IndexOutOfBoundsException, WrappedTargetException, RuntimeException )
{
    if (nIndex < 0 ||
        nIndex >= getCount())
        throw IndexOutOfBoundsException();

    Any aElement;
    aElement <<= m_aControllerList[nIndex];
    return aElement;
}

//------------------------------------------------------------------------
void SAL_CALL FormViewPageWindowAdapter::makeVisible( const Reference< XControl >& _Control ) throw (RuntimeException)
{
    SolarMutexGuard aSolarGuard;

    Reference< XWindow >  xWindow( _Control, UNO_QUERY );
    if ( xWindow.is() && m_pViewImpl->getView() && m_pWindow )
    {
        awt::Rectangle aRect = xWindow->getPosSize();
        ::Rectangle aNewRect( aRect.X, aRect.Y, aRect.X + aRect.Width, aRect.Y + aRect.Height );
        aNewRect = m_pWindow->PixelToLogic( aNewRect );
        m_pViewImpl->getView()->MakeVisible( aNewRect, *m_pWindow );
    }
}

//------------------------------------------------------------------------
Reference< XFormController >  getControllerSearchChildren( const Reference< XIndexAccess > & xIndex, const Reference< XTabControllerModel > & xModel)
{
    if (xIndex.is() && xIndex->getCount())
    {
        Reference< XFormController >  xController;

        for (sal_Int32 n = xIndex->getCount(); n-- && !xController.is(); )
        {
            xIndex->getByIndex(n) >>= xController;
            if ((XTabControllerModel*)xModel.get() == (XTabControllerModel*)xController->getModel().get())
                return xController;
            else
            {
                xController = getControllerSearchChildren(Reference< XIndexAccess > (xController, UNO_QUERY), xModel);
                if ( xController.is() )
                    return xController;
            }
        }
    }
    return Reference< XFormController > ();
}

// Search the according controller
//------------------------------------------------------------------------
Reference< XFormController >  FormViewPageWindowAdapter::getController( const Reference< XForm > & xForm ) const
{
    Reference< XTabControllerModel >  xModel(xForm, UNO_QUERY);
    for (::std::vector< Reference< XFormController > >::const_iterator i = m_aControllerList.begin();
         i != m_aControllerList.end(); ++i)
    {
        if ((XTabControllerModel*)(*i)->getModel().get() == (XTabControllerModel*)xModel.get())
            return *i;

        // the current-round controller isn't the right one. perhaps one of it's children ?
        Reference< XFormController >  xChildSearch = getControllerSearchChildren(Reference< XIndexAccess > (*i, UNO_QUERY), xModel);
        if (xChildSearch.is())
            return xChildSearch;
    }
    return Reference< XFormController > ();
}

//------------------------------------------------------------------------
void FormViewPageWindowAdapter::setController(const Reference< XForm > & xForm, const Reference< XFormController >& _rxParentController )
{
    DBG_ASSERT( xForm.is(), "FormViewPageWindowAdapter::setController: there should be a form!" );
    Reference< XIndexAccess >  xFormCps(xForm, UNO_QUERY);
    if (!xFormCps.is())
        return;

    Reference< XTabControllerModel >  xTabOrder(xForm, UNO_QUERY);

    // create a form controller
    Reference< XFormController > xController( m_aContext.createComponent( FM_FORM_CONTROLLER ), UNO_QUERY );
    if ( !xController.is() )
    {
        ShowServiceNotAvailableError( m_pWindow, FM_FORM_CONTROLLER, sal_True );
        return;
    }

    Reference< XInteractionHandler > xHandler;
    if ( _rxParentController.is() )
        xHandler = _rxParentController->getInteractionHandler();
    else
    {
        // TODO: should we create a default handler? Not really necessary, since the
        // FormController itself has a default fallback
    }
    if ( xHandler.is() )
        xController->setInteractionHandler( xHandler );

    xController->setContext( this );

    xController->setModel( xTabOrder );
    xController->setContainer( m_xControlContainer );
    xController->activateTabOrder();
    xController->addActivateListener( m_pViewImpl );

    if ( _rxParentController.is() )
        _rxParentController->addChildController( xController );
    else
    {
        m_aControllerList.push_back(xController);

        xController->setParent( *this );

        // attaching the events
        Reference< XEventAttacherManager > xEventManager( xForm->getParent(), UNO_QUERY );
        Reference< XInterface >  xIfc(xController, UNO_QUERY);
        xEventManager->attach(m_aControllerList.size() - 1, xIfc, makeAny(xController) );
    }

    // jetzt die Subforms durchgehen
    sal_uInt32 nLength = xFormCps->getCount();
    Reference< XForm >  xSubForm;
    for (sal_uInt32 i = 0; i < nLength; i++)
    {
        if ( xFormCps->getByIndex(i) >>= xSubForm )
            setController( xSubForm, xController );
    }
}

//------------------------------------------------------------------------
void FormViewPageWindowAdapter::updateTabOrder( const Reference< XForm >& _rxForm )
{
    OSL_PRECOND( _rxForm.is(), "FormViewPageWindowAdapter::updateTabOrder: illegal argument!" );
    if ( !_rxForm.is() )
        return;

    try
    {
        Reference< XTabController > xTabCtrl( getController( _rxForm ).get() );
        if ( xTabCtrl.is() )
        {   // if there already is a TabController for this form, then delegate the "updateTabOrder" request
            xTabCtrl->activateTabOrder();
        }
        else
        {   // otherwise, create a TabController

            // if it's a sub form, then we must ensure there exist TabControllers
            // for all its ancestors, too
            Reference< XForm > xParentForm( _rxForm->getParent(), UNO_QUERY );
            // there is a parent form -> look for the respective controller
            Reference< XFormController > xParentController;
            if ( xParentForm.is() )
                xParentController.set( getController( xParentForm ), UNO_QUERY );

            setController( _rxForm, xParentController );
        }
    }
    catch (const Exception&)
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

//------------------------------------------------------------------------
FmXFormView::FmXFormView(const ::comphelper::ComponentContext& _rContext, FmFormView* _pView )
    :m_aContext( _rContext )
    ,m_pMarkedGrid(NULL)
    ,m_pView(_pView)
    ,m_nActivationEvent(0)
    ,m_nErrorMessageEvent( 0 )
    ,m_nAutoFocusEvent( 0 )
    ,m_nControlWizardEvent( 0 )
    ,m_pWatchStoredList( NULL )
    ,m_bFirstActivation( true )
    ,m_isTabOrderUpdateSuspended( false )
{
}

//------------------------------------------------------------------------
void FmXFormView::cancelEvents()
{
    if ( m_nActivationEvent )
    {
        Application::RemoveUserEvent( m_nActivationEvent );
        m_nActivationEvent = 0;
    }

    if ( m_nErrorMessageEvent )
    {
        Application::RemoveUserEvent( m_nErrorMessageEvent );
        m_nErrorMessageEvent = 0;
    }

    if ( m_nAutoFocusEvent )
    {
        Application::RemoveUserEvent( m_nAutoFocusEvent );
        m_nAutoFocusEvent = 0;
    }

    if ( m_nControlWizardEvent )
    {
        Application::RemoveUserEvent( m_nControlWizardEvent );
        m_nControlWizardEvent = 0;
    }
}

//------------------------------------------------------------------------
void FmXFormView::notifyViewDying( )
{
    DBG_ASSERT( m_pView, "FmXFormView::notifyViewDying: my view already died!" );
    m_pView = NULL;
    cancelEvents();
}

//------------------------------------------------------------------------
FmXFormView::~FmXFormView()
{
    DBG_ASSERT( m_aPageWindowAdapters.empty(), "FmXFormView::~FmXFormView: Window list not empty!" );
    if ( !m_aPageWindowAdapters.empty() )
    {
        for (   PageWindowAdapterList::const_iterator loop = m_aPageWindowAdapters.begin();
                loop != m_aPageWindowAdapters.end();
                ++loop
            )
        {
            (*loop)->dispose();
        }
    }

    cancelEvents();

    delete m_pWatchStoredList;
    m_pWatchStoredList = NULL;
}

//      EventListener
//------------------------------------------------------------------------------
void SAL_CALL FmXFormView::disposing(const EventObject& Source) throw( RuntimeException )
{
    if ( m_xWindow.is() && Source.Source == m_xWindow )
        removeGridWindowListening();
}

// XFormControllerListener
//------------------------------------------------------------------------------
void SAL_CALL FmXFormView::formActivated(const EventObject& rEvent) throw( RuntimeException )
{
    if ( m_pView && m_pView->GetFormShell() && m_pView->GetFormShell()->GetImpl() )
        m_pView->GetFormShell()->GetImpl()->formActivated( rEvent );
}

//------------------------------------------------------------------------------
void SAL_CALL FmXFormView::formDeactivated(const EventObject& rEvent) throw( RuntimeException )
{
    if ( m_pView && m_pView->GetFormShell() && m_pView->GetFormShell()->GetImpl() )
        m_pView->GetFormShell()->GetImpl()->formDeactivated( rEvent );
}

// XContainerListener
//------------------------------------------------------------------------------
void SAL_CALL FmXFormView::elementInserted(const ContainerEvent& evt) throw( RuntimeException )
{
    try
    {
        Reference< XControlContainer > xControlContainer( evt.Source, UNO_QUERY_THROW );
        Reference< XControl > xControl( evt.Element, UNO_QUERY_THROW );
        Reference< XFormComponent > xControlModel( xControl->getModel(), UNO_QUERY_THROW );
        Reference< XForm > xForm( xControlModel->getParent(), UNO_QUERY_THROW );

        if ( m_isTabOrderUpdateSuspended )
        {
            // remember the container and the control, so we can update the tab order on resumeTabOrderUpdate
            m_aNeedTabOrderUpdate[ xControlContainer ].insert( xForm );
        }
        else
        {
            PFormViewPageWindowAdapter pAdapter = findWindow( xControlContainer );
            if ( pAdapter.is() )
                pAdapter->updateTabOrder( xForm );
        }
    }
    catch (const Exception&)
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

//------------------------------------------------------------------------------
void SAL_CALL FmXFormView::elementReplaced(const ContainerEvent& evt) throw( RuntimeException )
{
    elementInserted(evt);
}

//------------------------------------------------------------------------------
void SAL_CALL FmXFormView::elementRemoved(const ContainerEvent& /*evt*/) throw( RuntimeException )
{
}

//------------------------------------------------------------------------------
PFormViewPageWindowAdapter FmXFormView::findWindow( const Reference< XControlContainer >& _rxCC )  const
{
    for (   PageWindowAdapterList::const_iterator i = m_aPageWindowAdapters.begin();
            i != m_aPageWindowAdapters.end();
            ++i
        )
    {
        if ( _rxCC == (*i)->getControlContainer() )
            return *i;
    }
    return NULL;
}

//------------------------------------------------------------------------------
void FmXFormView::addWindow(const SdrPageWindow& rWindow)
{
    FmFormPage* pFormPage = PTR_CAST( FmFormPage, rWindow.GetPageView().GetPage() );
    if ( !pFormPage )
        return;

    Reference< XControlContainer > xCC = rWindow.GetControlContainer();
    if  (   xCC.is()
        &&  ( !findWindow( xCC ).is() )
        )
    {
        PFormViewPageWindowAdapter pAdapter = new FormViewPageWindowAdapter( m_aContext, rWindow, this );
        m_aPageWindowAdapters.push_back( pAdapter );

        // Am ControlContainer horchen um Aenderungen mitzbekommen
        Reference< XContainer >  xContainer( xCC, UNO_QUERY );
        if ( xContainer.is() )
            xContainer->addContainerListener( this );
    }
}

//------------------------------------------------------------------------------
void FmXFormView::removeWindow( const Reference< XControlContainer >& _rxCC )
{
    // Wird gerufen, wenn
    // - in den Design-Modus geschaltet wird
    // - ein Window geloescht wird, waehrend man im Design-Modus ist
    // - der Control-Container fuer ein Window entfernt wird, waehrend
    //   der aktive Modus eingeschaltet ist.

    for (   PageWindowAdapterList::iterator i = m_aPageWindowAdapters.begin();
            i != m_aPageWindowAdapters.end();
            ++i
        )
    {
        if ( _rxCC != (*i)->getControlContainer() )
            continue;

        Reference< XContainer >  xContainer( _rxCC, UNO_QUERY );
        if ( xContainer.is() )
            xContainer->removeContainerListener( this );

        (*i)->dispose();
        m_aPageWindowAdapters.erase( i );
        break;
    }
}

//------------------------------------------------------------------------------
void FmXFormView::displayAsyncErrorMessage( const SQLErrorEvent& _rEvent )
{
    DBG_ASSERT( 0 == m_nErrorMessageEvent, "FmXFormView::displayAsyncErrorMessage: not too fast, please!" );
        // This should not happen - usually, the PostUserEvent is faster than any possible user
        // interaction which could trigger a new error. If it happens, we need a queue for the events.
    m_aAsyncError = _rEvent;
    m_nErrorMessageEvent = Application::PostUserEvent( LINK( this, FmXFormView, OnDelayedErrorMessage ) );
}

//------------------------------------------------------------------------------
IMPL_LINK(FmXFormView, OnDelayedErrorMessage, void*, /*EMPTYTAG*/)
{
    m_nErrorMessageEvent = 0;
    displayException( m_aAsyncError );
    return 0L;
}

//------------------------------------------------------------------------------
void FmXFormView::onFirstViewActivation( const FmFormModel* _pDocModel )
{
    if ( _pDocModel && _pDocModel->GetAutoControlFocus() )
        m_nAutoFocusEvent = Application::PostUserEvent( LINK( this, FmXFormView, OnAutoFocus ) );
}

//------------------------------------------------------------------------------
void FmXFormView::suspendTabOrderUpdate()
{
    OSL_ENSURE( !m_isTabOrderUpdateSuspended, "FmXFormView::suspendTabOrderUpdate: nesting not allowed!" );
    m_isTabOrderUpdateSuspended = true;
}

//------------------------------------------------------------------------------
void FmXFormView::resumeTabOrderUpdate()
{
    OSL_ENSURE( m_isTabOrderUpdateSuspended, "FmXFormView::resumeTabOrderUpdate: not suspended!" );
    m_isTabOrderUpdateSuspended = false;

    // update the tab orders for all components which were collected since the suspendTabOrderUpdate call.
    for (   MapControlContainerToSetOfForms::const_iterator container = m_aNeedTabOrderUpdate.begin();
            container != m_aNeedTabOrderUpdate.end();
            ++container
        )
    {
        PFormViewPageWindowAdapter pAdapter = findWindow( container->first );
        if ( !pAdapter.is() )
            continue;

        for (   SetOfForms::const_iterator form = container->second.begin();
                form != container->second.end();
                ++form
            )
        {
            pAdapter->updateTabOrder( *form );
        }
    }
    m_aNeedTabOrderUpdate.clear();
}

//------------------------------------------------------------------------------
IMPL_LINK(FmXFormView, OnActivate, void*, /*EMPTYTAG*/)
{
    m_nActivationEvent = 0;

    if ( !m_pView )
    {
        OSL_FAIL( "FmXFormView::OnActivate: well .... seems we have a timing problem (the view already died)!" );
        return 0;
    }

    // setting the controller to activate
    if (m_pView->GetFormShell() && m_pView->GetActualOutDev() && m_pView->GetActualOutDev()->GetOutDevType() == OUTDEV_WINDOW)
    {
        Window* pWindow = const_cast<Window*>(static_cast<const Window*>(m_pView->GetActualOutDev()));
        PFormViewPageWindowAdapter pAdapter = m_aPageWindowAdapters.empty() ? NULL : m_aPageWindowAdapters[0];
        for (   PageWindowAdapterList::const_iterator i = m_aPageWindowAdapters.begin();
                i != m_aPageWindowAdapters.end();
                ++i
            )
        {
            if ( pWindow == (*i)->getWindow() )
                pAdapter =*i;
        }

        if ( pAdapter.get() )
        {
            for (   ::std::vector< Reference< XFormController > >::const_iterator i = pAdapter->GetList().begin();
                    i != pAdapter->GetList().end();
                    ++i
                )
            {
                const Reference< XFormController > & xController = *i;
                if ( !xController.is() )
                    continue;

                // only database forms are to be activated
                Reference< XRowSet >  xForm(xController->getModel(), UNO_QUERY);
                if ( !xForm.is() || !OStaticDataAccessTools().getRowSetConnection( xForm ).is() )
                    continue;

                Reference< XPropertySet > xFormSet( xForm, UNO_QUERY );
                if ( !xFormSet.is() )
                {
                    SAL_WARN( "svx.form", "FmXFormView::OnActivate: a form which does not have properties?" );
                    continue;
                }

                const ::rtl::OUString aSource = ::comphelper::getString( xFormSet->getPropertyValue( FM_PROP_COMMAND ) );
                if ( !aSource.isEmpty() )
                {
                    FmXFormShell* pShImpl =  m_pView->GetFormShell()->GetImpl();
                    if ( pShImpl )
                        pShImpl->setActiveController( xController );
                    break;
                }
            }
        }
    }
    return 0;
}

//------------------------------------------------------------------------------
void FmXFormView::Activate(sal_Bool bSync)
{
    if (m_nActivationEvent)
    {
        Application::RemoveUserEvent(m_nActivationEvent);
        m_nActivationEvent = 0;
    }

    if (bSync)
    {
        LINK(this,FmXFormView,OnActivate).Call(NULL);
    }
    else
        m_nActivationEvent = Application::PostUserEvent(LINK(this,FmXFormView,OnActivate));
}

//------------------------------------------------------------------------------
void FmXFormView::Deactivate(sal_Bool bDeactivateController)
{
    if (m_nActivationEvent)
    {
        Application::RemoveUserEvent(m_nActivationEvent);
        m_nActivationEvent = 0;
    }

    FmXFormShell* pShImpl =  m_pView->GetFormShell() ? m_pView->GetFormShell()->GetImpl() : NULL;
    if (pShImpl && bDeactivateController)
        pShImpl->setActiveController( NULL );
}

//------------------------------------------------------------------------------
FmFormShell* FmXFormView::GetFormShell() const
{
    return m_pView ? m_pView->GetFormShell() : NULL;
}
// -----------------------------------------------------------------------------
void FmXFormView::AutoFocus( sal_Bool _bSync )
{
    if (m_nAutoFocusEvent)
        Application::RemoveUserEvent(m_nAutoFocusEvent);

    if ( _bSync )
        OnAutoFocus( NULL );
    else
        m_nAutoFocusEvent = Application::PostUserEvent(LINK(this, FmXFormView, OnAutoFocus));
}

// -----------------------------------------------------------------------------
bool FmXFormView::isFocusable( const Reference< XControl >& i_rControl )
{
    if ( !i_rControl.is() )
        return false;

    try
    {
        Reference< XPropertySet > xModelProps( i_rControl->getModel(), UNO_QUERY_THROW );

        // only enabled controls are allowed to participate
        sal_Bool bEnabled = sal_False;
        OSL_VERIFY( xModelProps->getPropertyValue( FM_PROP_ENABLED ) >>= bEnabled );
        if ( !bEnabled )
            return false;

        // check the class id of the control model
        sal_Int16 nClassId = FormComponentType::CONTROL;
        OSL_VERIFY( xModelProps->getPropertyValue( FM_PROP_CLASSID ) >>= nClassId );

        // controls which are not focussable
        if  (   ( FormComponentType::CONTROL != nClassId )
            &&  ( FormComponentType::IMAGEBUTTON != nClassId )
            &&  ( FormComponentType::GROUPBOX != nClassId )
            &&  ( FormComponentType::FIXEDTEXT != nClassId )
            &&  ( FormComponentType::HIDDENCONTROL != nClassId )
            &&  ( FormComponentType::IMAGECONTROL != nClassId )
            &&  ( FormComponentType::SCROLLBAR != nClassId )
            &&  ( FormComponentType::SPINBUTTON!= nClassId )
            )
        {
            return true;
        }
    }
    catch (const Exception&)
    {
        DBG_UNHANDLED_EXCEPTION();
    }
    return false;
}

// -----------------------------------------------------------------------------
static Reference< XControl > lcl_firstFocussableControl( const Sequence< Reference< XControl > >& _rControls )
{
    Reference< XControl > xReturn;

    // loop through all the controls
    const Reference< XControl >* pControls = _rControls.getConstArray();
    const Reference< XControl >* pControlsEnd = _rControls.getConstArray() + _rControls.getLength();
    for ( ; pControls != pControlsEnd; ++pControls )
    {
        if ( !pControls->is() )
            continue;

        if ( FmXFormView::isFocusable( *pControls ) )
        {
            xReturn = *pControls;
            break;
        }
    }

    if ( !xReturn.is() && _rControls.getLength() )
        xReturn = _rControls[0];

    return xReturn;
}

// -----------------------------------------------------------------------------
namespace
{
    // .........................................................................
    void lcl_ensureControlsOfFormExist_nothrow( const SdrPage& _rPage, const SdrView& _rView, const Window& _rWindow, const Reference< XForm >& _rxForm )
    {
        try
        {
            Reference< XInterface > xNormalizedForm( _rxForm, UNO_QUERY_THROW );

            SdrObjListIter aSdrObjectLoop( _rPage, IM_DEEPNOGROUPS );
            while ( aSdrObjectLoop.IsMore() )
            {
                FmFormObj* pFormObject = FmFormObj::GetFormObject( aSdrObjectLoop.Next() );
                if ( !pFormObject )
                    continue;

                Reference< XChild > xModel( pFormObject->GetUnoControlModel(), UNO_QUERY_THROW );
                Reference< XInterface > xModelParent( xModel->getParent(), UNO_QUERY_THROW );

                if ( xNormalizedForm.get() != xModelParent.get() )
                    continue;

                pFormObject->GetUnoControl( _rView, _rWindow );
            }
        }
        catch (const Exception&)
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }
}

// -----------------------------------------------------------------------------
Reference< XFormController > FmXFormView::getFormController( const Reference< XForm >& _rxForm, const OutputDevice& _rDevice ) const
{
    Reference< XFormController > xController;

    for (   PageWindowAdapterList::const_iterator pos = m_aPageWindowAdapters.begin();
            pos != m_aPageWindowAdapters.end();
            ++pos
        )
    {
        const PFormViewPageWindowAdapter pAdapter( *pos );
        if ( !pAdapter.get() )
        {
            SAL_WARN( "svx.form", "FmXFormView::getFormController: invalid page window adapter!" );
            continue;
        }

        if ( pAdapter->getWindow() != &_rDevice )
            // wrong device
            continue;

        xController = pAdapter->getController( _rxForm );
        if ( xController.is() )
            break;
    }
    return xController;
}

// -----------------------------------------------------------------------------
IMPL_LINK(FmXFormView, OnAutoFocus, void*, /*EMPTYTAG*/)
{
    m_nAutoFocusEvent = 0;

    // go to the first form of our page, examine it's TabController, go to it's first (in terms of the tab order)
    // control, give it the focus

    do
    {

    SdrPageView *pPageView = m_pView ? m_pView->GetSdrPageView() : NULL;
    SdrPage *pSdrPage = pPageView ? pPageView->GetPage() : NULL;
    // get the forms collection of the page we belong to
    FmFormPage* pPage = PTR_CAST( FmFormPage, pSdrPage );
    Reference< XIndexAccess > xForms( pPage ? Reference< XIndexAccess >( pPage->GetForms(), UNO_QUERY ) : Reference< XIndexAccess >() );

    const PFormViewPageWindowAdapter pAdapter = m_aPageWindowAdapters.empty() ? NULL : m_aPageWindowAdapters[0];
    const Window* pWindow = pAdapter.get() ? pAdapter->getWindow() : NULL;

    ENSURE_OR_RETURN( xForms.is() && pWindow, "FmXFormView::OnAutoFocus: could not collect all essentials!", 0L );

    try
    {
        // go for the tab controller of the first form
        if ( !xForms->getCount() )
            break;
        Reference< XForm > xForm( xForms->getByIndex( 0 ), UNO_QUERY_THROW );
        Reference< XTabController > xTabController( pAdapter->getController( xForm ), UNO_QUERY_THROW );

        // go for the first control of the controller
        Sequence< Reference< XControl > > aControls( xTabController->getControls() );
        if ( aControls.getLength() == 0 )
        {
            Reference< XElementAccess > xFormElementAccess( xForm, UNO_QUERY_THROW );
            if ( xFormElementAccess->hasElements() )
            {
                // there are control models in the form, but no controls, yet.
                // Well, since some time controls are created on demand only. In particular,
                // they're normally created when they're first painted.
                // Unfortunately, the FormController does not have any way to
                // trigger the creation itself, so we must hack this ...
                lcl_ensureControlsOfFormExist_nothrow( *pPage, *m_pView, *pWindow, xForm );
                aControls = xTabController->getControls();
                OSL_ENSURE( aControls.getLength(), "FmXFormView::OnAutoFocus: no controls at all!" );
            }
        }

        // set the focus to this first control
        Reference< XWindow > xControlWindow( lcl_firstFocussableControl( aControls ), UNO_QUERY );
        if ( !xControlWindow.is() )
            break;

        xControlWindow->setFocus();

        // ensure that the control is visible
        // 80210 - 12/07/00 - FS
        const Window* pCurrentWindow = dynamic_cast< const Window* >( m_pView->GetActualOutDev() );
        if ( pCurrentWindow )
        {
            awt::Rectangle aRect = xControlWindow->getPosSize();
            ::Rectangle aNonUnoRect( aRect.X, aRect.Y, aRect.X + aRect.Width, aRect.Y + aRect.Height );
            m_pView->MakeVisible( pCurrentWindow->PixelToLogic( aNonUnoRect ), *const_cast< Window* >( pCurrentWindow ) );
        }
    }
    catch (const Exception&)
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    }   // do
    while ( false );

    return 1L;
}

// -----------------------------------------------------------------------------
void FmXFormView::onCreatedFormObject( FmFormObj& _rFormObject )
{
    FmFormShell* pShell = m_pView ? m_pView->GetFormShell() : NULL;
    FmXFormShell* pShellImpl = pShell ? pShell->GetImpl() : NULL;
    OSL_ENSURE( pShellImpl, "FmXFormView::onCreatedFormObject: no form shell!" );
    if ( !pShellImpl )
        return;

    // it is valid that the form shell's forms collection is not initialized, yet
    pShellImpl->UpdateForms( sal_True );

    m_xLastCreatedControlModel.set( _rFormObject.GetUnoControlModel(), UNO_QUERY );
    if ( !m_xLastCreatedControlModel.is() )
        return;

    // some initial property defaults
    FormControlFactory aControlFactory( m_aContext );
    aControlFactory.initializeControlModel( pShellImpl->getDocumentType(), _rFormObject );

    if ( !pShellImpl->GetWizardUsing() )
        return;

    // #i31958# don't call wizards in XForms mode
    if ( pShellImpl->isEnhancedForm() )
        return;

    // #i46898# no wizards if there is no Base installed - currently, all wizards are
    // database related
    if ( !SvtModuleOptions().IsModuleInstalled( SvtModuleOptions::E_SDATABASE ) )
        return;

    if ( m_nControlWizardEvent )
        Application::RemoveUserEvent( m_nControlWizardEvent );
    m_nControlWizardEvent = Application::PostUserEvent( LINK( this, FmXFormView, OnStartControlWizard ) );
}

// -----------------------------------------------------------------------------
IMPL_LINK( FmXFormView, OnStartControlWizard, void*, /**/ )
{
    m_nControlWizardEvent = 0;
    OSL_PRECOND( m_xLastCreatedControlModel.is(), "FmXFormView::OnStartControlWizard: illegal call!" );
    if ( !m_xLastCreatedControlModel.is() )
        return 0L;

    sal_Int16 nClassId = FormComponentType::CONTROL;
    try
    {
        OSL_VERIFY( m_xLastCreatedControlModel->getPropertyValue( FM_PROP_CLASSID ) >>= nClassId );
    }
    catch (const Exception&)
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    const sal_Char* pWizardAsciiName = NULL;
    switch ( nClassId )
    {
        case FormComponentType::GRIDCONTROL:
            pWizardAsciiName = "com.sun.star.sdb.GridControlAutoPilot";
            break;
        case FormComponentType::LISTBOX:
        case FormComponentType::COMBOBOX:
            pWizardAsciiName = "com.sun.star.sdb.ListComboBoxAutoPilot";
            break;
        case FormComponentType::GROUPBOX:
            pWizardAsciiName = "com.sun.star.sdb.GroupBoxAutoPilot";
            break;
    }

    if ( pWizardAsciiName )
    {
        // build the argument list
        ::comphelper::NamedValueCollection aWizardArgs;
        aWizardArgs.put( "ObjectModel", m_xLastCreatedControlModel );

        // create the wizard object
        Reference< XExecutableDialog > xWizard;
        try
        {
            m_aContext.createComponentWithArguments( pWizardAsciiName, aWizardArgs.getWrappedPropertyValues(), xWizard );
        }
        catch (const Exception&)
        {
            DBG_UNHANDLED_EXCEPTION();
        }

        if ( !xWizard.is() )
        {
            ShowServiceNotAvailableError( NULL, rtl::OUString::createFromAscii(pWizardAsciiName), sal_True );
        }
        else
        {
            // execute the wizard
            try
            {
                xWizard->execute();
            }
            catch (const Exception&)
            {
                DBG_UNHANDLED_EXCEPTION();
            }
        }
    }

    m_xLastCreatedControlModel.clear();
    return 1L;
}

// -----------------------------------------------------------------------------
namespace
{
    void lcl_insertIntoFormComponentHierarchy_throw( const FmFormView& _rView, const SdrUnoObj& _rSdrObj,
        const Reference< XDataSource >& _rxDataSource = NULL, const ::rtl::OUString& _rDataSourceName = ::rtl::OUString(),
        const ::rtl::OUString& _rCommand = ::rtl::OUString(), const sal_Int32 _nCommandType = -1 )
    {
        FmFormPage& rPage = static_cast< FmFormPage& >( *_rView.GetSdrPageView()->GetPage() );

        Reference< XFormComponent > xFormComponent( _rSdrObj.GetUnoControlModel(), UNO_QUERY_THROW );
        Reference< XForm > xTargetForm(
            rPage.GetImpl().findPlaceInFormComponentHierarchy( xFormComponent, _rxDataSource, _rDataSourceName, _rCommand, _nCommandType ),
            UNO_SET_THROW );

        rPage.GetImpl().setUniqueName( xFormComponent, xTargetForm );

        Reference< XIndexContainer > xFormAsContainer( xTargetForm, UNO_QUERY_THROW );
        xFormAsContainer->insertByIndex( xFormAsContainer->getCount(), makeAny( xFormComponent ) );
    }
}

// -----------------------------------------------------------------------------
SdrObject* FmXFormView::implCreateFieldControl( const ::svx::ODataAccessDescriptor& _rColumnDescriptor )
{
    // not if we're in design mode
    if ( !m_pView->IsDesignMode() )
        return NULL;

    ::rtl::OUString sCommand, sFieldName;
    sal_Int32 nCommandType = CommandType::COMMAND;
    SharedConnection xConnection;

    ::rtl::OUString sDataSource = _rColumnDescriptor.getDataSource();
    _rColumnDescriptor[ daCommand ]     >>= sCommand;
    _rColumnDescriptor[ daColumnName ]  >>= sFieldName;
    _rColumnDescriptor[ daCommandType ] >>= nCommandType;
    {
        Reference< XConnection > xExternalConnection;
        _rColumnDescriptor[ daConnection ]  >>= xExternalConnection;
        xConnection.reset( xExternalConnection, SharedConnection::NoTakeOwnership );
    }

    if  (   sCommand.isEmpty()
        ||  sFieldName.isEmpty()
        ||  (   sDataSource.isEmpty()
            &&  !xConnection.is()
            )
        )
    {
        OSL_FAIL( "FmXFormView::implCreateFieldControl: nonsense!" );
    }

    Reference< XDataSource > xDataSource;
    SQLErrorEvent aError;
    try
    {
        if ( xConnection.is() && !xDataSource.is() && sDataSource.isEmpty() )
        {
            Reference< XChild > xChild( xConnection, UNO_QUERY );
            if ( xChild.is() )
                xDataSource = xDataSource.query( xChild->getParent() );
        }

        // obtain the data source
        if ( !xDataSource.is() )
            xDataSource = OStaticDataAccessTools().getDataSource( sDataSource, m_aContext.getUNOContext() );

        // and the connection, if necessary
        if ( !xConnection.is() )
            xConnection.reset( OStaticDataAccessTools().getConnection_withFeedback(
                sDataSource,
                ::rtl::OUString(),
                ::rtl::OUString(),
                m_aContext.getUNOContext()
            ) );
    }
    catch (const SQLException&)
    {
        aError.Reason = ::cppu::getCaughtException();
    }
    catch (const Exception& )
    {
        /* will be asserted below */
    }
    if (aError.Reason.hasValue())
    {
        displayAsyncErrorMessage( aError );
        return NULL;
    }

    // need a data source and a connection here
    if (!xDataSource.is() || !xConnection.is())
    {
        OSL_FAIL("FmXFormView::implCreateFieldControl : could not retrieve the data source or the connection!");
        return NULL;
    }

    OStaticDataAccessTools aDBATools;
    Reference< XComponent > xKeepFieldsAlive;
    // go
    try
    {
        // determine the table/query field which we should create a control for
        Reference< XPropertySet >   xField;

        Reference< XNameAccess >    xFields = aDBATools.getFieldsByCommandDescriptor(
            xConnection, nCommandType, sCommand, xKeepFieldsAlive );

        if (xFields.is() && xFields->hasByName(sFieldName))
            xFields->getByName(sFieldName) >>= xField;
        if ( !xField.is() )
            return NULL;

        Reference< XNumberFormatsSupplier > xSupplier( aDBATools.getNumberFormats( xConnection, sal_False ), UNO_SET_THROW );
        Reference< XNumberFormats >  xNumberFormats( xSupplier->getNumberFormats(), UNO_SET_THROW );

        ::rtl::OUString sLabelPostfix;

        ////////////////////////////////////////////////////////////////
        // nur fuer Textgroesse
        OutputDevice* pOutDev = NULL;
        if (m_pView->GetActualOutDev() && m_pView->GetActualOutDev()->GetOutDevType() == OUTDEV_WINDOW)
            pOutDev = const_cast<OutputDevice*>(m_pView->GetActualOutDev());
        else
        {// OutDev suchen
            SdrPageView* pPageView = m_pView->GetSdrPageView();
            if( pPageView && !pOutDev )
            {
                // const SdrPageViewWinList& rWinList = pPageView->GetWinList();
                // const SdrPageViewWindows& rPageViewWindows = pPageView->GetPageViewWindows();

                for( sal_uInt32 i = 0L; i < pPageView->PageWindowCount(); i++ )
                {
                    const SdrPageWindow& rPageWindow = *pPageView->GetPageWindow(i);

                    if( rPageWindow.GetPaintWindow().OutputToWindow())
                    {
                        pOutDev = &rPageWindow.GetPaintWindow().GetOutputDevice();
                        break;
                    }
                }
            }
        }

        if ( !pOutDev )
            return NULL;

        sal_Int32 nDataType = ::comphelper::getINT32(xField->getPropertyValue(FM_PROP_FIELDTYPE));
        if ((DataType::BINARY == nDataType) || (DataType::VARBINARY == nDataType))
            return NULL;

        //////////////////////////////////////////////////////////////////////
        // determine the control type by examining the data type of the bound column
        sal_uInt16 nOBJID = 0;
        sal_Bool bDateNTimeField = sal_False;

        sal_Bool bIsCurrency = sal_False;
        if (::comphelper::hasProperty(FM_PROP_ISCURRENCY, xField))
            bIsCurrency = ::comphelper::getBOOL(xField->getPropertyValue(FM_PROP_ISCURRENCY));

        if (bIsCurrency)
            nOBJID = OBJ_FM_CURRENCYFIELD;
        else
            switch (nDataType)
            {
                case DataType::BLOB:
                case DataType::LONGVARBINARY:
                    nOBJID = OBJ_FM_IMAGECONTROL;
                    break;
                case DataType::LONGVARCHAR:
                case DataType::CLOB:
                    nOBJID = OBJ_FM_EDIT;
                    break;
                case DataType::BINARY:
                case DataType::VARBINARY:
                    return NULL;
                case DataType::BIT:
                case DataType::BOOLEAN:
                    nOBJID = OBJ_FM_CHECKBOX;
                    break;
                case DataType::TINYINT:
                case DataType::SMALLINT:
                case DataType::INTEGER:
                    nOBJID = OBJ_FM_NUMERICFIELD;
                    break;
                case DataType::REAL:
                case DataType::DOUBLE:
                case DataType::NUMERIC:
                case DataType::DECIMAL:
                    nOBJID = OBJ_FM_FORMATTEDFIELD;
                    break;
                case DataType::TIMESTAMP:
                    bDateNTimeField = sal_True;
                    sLabelPostfix = String( SVX_RES( RID_STR_POSTFIX_DATE ) );
                    // DON'T break !
                case DataType::DATE:
                    nOBJID = OBJ_FM_DATEFIELD;
                    break;
                case DataType::TIME:
                    nOBJID = OBJ_FM_TIMEFIELD;
                    break;
                case DataType::CHAR:
                case DataType::VARCHAR:
                default:
                    nOBJID = OBJ_FM_EDIT;
                    break;
            }
        if (!nOBJID)
            return NULL;

        SdrUnoObj* pLabel( NULL );
        SdrUnoObj* pControl( NULL );
        if  (   !createControlLabelPair( *pOutDev, 0, 0, xField, xNumberFormats, nOBJID, sLabelPostfix,
                    pLabel, pControl, xDataSource, sDataSource, sCommand, nCommandType )
            )
        {
            return NULL;
        }

        //////////////////////////////////////////////////////////////////////
        // group objects
        bool bCheckbox = ( OBJ_FM_CHECKBOX == nOBJID );
        OSL_ENSURE( !bCheckbox || !pLabel, "FmXFormView::implCreateFieldControl: why was there a label created for a check box?" );
        if ( bCheckbox )
            return pControl;

        SdrObjGroup* pGroup  = new SdrObjGroup();
        SdrObjList* pObjList = pGroup->GetSubList();
        pObjList->InsertObject( pLabel );
        pObjList->InsertObject( pControl );

        if ( bDateNTimeField )
        {   // so far we created a date field only, but we also need a time field
            pLabel = pControl = NULL;
            if  (   createControlLabelPair( *pOutDev, 0, 1000, xField, xNumberFormats, OBJ_FM_TIMEFIELD,
                        String( SVX_RES( RID_STR_POSTFIX_TIME ) ), pLabel, pControl,
                        xDataSource, sDataSource, sCommand, nCommandType )
                )
            {
                pObjList->InsertObject( pLabel );
                pObjList->InsertObject( pControl );
            }
        }

        return pGroup; // und fertig
    }
    catch (const Exception&)
    {
        DBG_UNHANDLED_EXCEPTION();
    }


    return NULL;
}

// -----------------------------------------------------------------------------
SdrObject* FmXFormView::implCreateXFormsControl( const ::svx::OXFormsDescriptor &_rDesc )
{
    // not if we're in design mode
    if ( !m_pView->IsDesignMode() )
        return NULL;

    Reference< XComponent > xKeepFieldsAlive;

    // go
    try
    {
        // determine the table/query field which we should create a control for
        Reference< XNumberFormats > xNumberFormats;
        ::rtl::OUString sLabelPostfix = _rDesc.szName;

        ////////////////////////////////////////////////////////////////
        // nur fuer Textgroesse
        OutputDevice* pOutDev = NULL;
        if (m_pView->GetActualOutDev() && m_pView->GetActualOutDev()->GetOutDevType() == OUTDEV_WINDOW)
            pOutDev = const_cast<OutputDevice*>(m_pView->GetActualOutDev());
        else
        {// OutDev suchen
            SdrPageView* pPageView = m_pView->GetSdrPageView();
            if( pPageView && !pOutDev )
            {
                // const SdrPageViewWinList& rWinList = pPageView->GetWinList();
                // const SdrPageViewWindows& rPageViewWindows = pPageView->GetPageViewWindows();

                for( sal_uInt32 i = 0L; i < pPageView->PageWindowCount(); i++ )
                {
                    const SdrPageWindow& rPageWindow = *pPageView->GetPageWindow(i);

                    if( rPageWindow.GetPaintWindow().GetOutputDevice().GetOutDevType() == OUTDEV_WINDOW)
                    {
                        pOutDev = &rPageWindow.GetPaintWindow().GetOutputDevice();
                        break;
                    }
                }
            }
        }

        if ( !pOutDev )
            return NULL;

        //////////////////////////////////////////////////////////////////////
        // The service name decides which control should be created
        sal_uInt16 nOBJID = OBJ_FM_EDIT;
        if(::rtl::OUString(_rDesc.szServiceName).equals((::rtl::OUString)FM_SUN_COMPONENT_NUMERICFIELD))
            nOBJID = OBJ_FM_NUMERICFIELD;
        if(::rtl::OUString(_rDesc.szServiceName).equals((::rtl::OUString)FM_SUN_COMPONENT_CHECKBOX))
            nOBJID = OBJ_FM_CHECKBOX;
        if(::rtl::OUString(_rDesc.szServiceName).equals((::rtl::OUString)FM_COMPONENT_COMMANDBUTTON))
            nOBJID = OBJ_FM_BUTTON;

        typedef ::com::sun::star::form::submission::XSubmission XSubmission_t;
        Reference< XSubmission_t > xSubmission(_rDesc.xPropSet, UNO_QUERY);

        // xform control or submission button?
        if ( !xSubmission.is() )
        {
            SdrUnoObj* pLabel( NULL );
            SdrUnoObj* pControl( NULL );
            if  (   !createControlLabelPair( *pOutDev, 0, 0, NULL, xNumberFormats, nOBJID, sLabelPostfix,
                        pLabel, pControl )
                )
            {
                return NULL;
            }

            //////////////////////////////////////////////////////////////////////
            // Now build the connection between the control and the data item.
            Reference< XValueBinding > xValueBinding(_rDesc.xPropSet,UNO_QUERY);
            Reference< XBindableValue > xBindableValue(pControl->GetUnoControlModel(),UNO_QUERY);

            DBG_ASSERT( xBindableValue.is(), "FmXFormView::implCreateXFormsControl: control's not bindable!" );
            if ( xBindableValue.is() )
                xBindableValue->setValueBinding(xValueBinding);

            bool bCheckbox = ( OBJ_FM_CHECKBOX == nOBJID );
            OSL_ENSURE( !bCheckbox || !pLabel, "FmXFormView::implCreateXFormsControl: why was there a label created for a check box?" );
            if ( bCheckbox )
                return pControl;

            //////////////////////////////////////////////////////////////////////
            // group objects
            SdrObjGroup* pGroup  = new SdrObjGroup();
            SdrObjList* pObjList = pGroup->GetSubList();
            pObjList->InsertObject(pLabel);
            pObjList->InsertObject(pControl);

            return pGroup;
        }
        else {

            // create a button control
            const MapMode eTargetMode( pOutDev->GetMapMode() );
            const MapMode eSourceMode(MAP_100TH_MM);
            const sal_uInt16 nObjID = OBJ_FM_BUTTON;
            ::Size controlSize(4000, 500);
            FmFormObj *pControl = static_cast<FmFormObj*>(SdrObjFactory::MakeNewObject( FmFormInventor, nObjID, NULL, NULL ));
            controlSize.Width() = Fraction(controlSize.Width(), 1) * eTargetMode.GetScaleX();
            controlSize.Height() = Fraction(controlSize.Height(), 1) * eTargetMode.GetScaleY();
            ::Point controlPos( pOutDev->LogicToLogic( ::Point( controlSize.Width(), 0 ), eSourceMode, eTargetMode ) );
            ::Rectangle controlRect( controlPos, pOutDev->LogicToLogic( controlSize, eSourceMode, eTargetMode ) );
            pControl->SetLogicRect(controlRect);

            // set the button label
            Reference< XPropertySet > xControlSet(pControl->GetUnoControlModel(), UNO_QUERY);
            xControlSet->setPropertyValue(FM_PROP_LABEL, makeAny(::rtl::OUString(_rDesc.szName)));

            // connect the submission with the submission supplier (aka the button)
            xControlSet->setPropertyValue( FM_PROP_BUTTON_TYPE,
                                           makeAny( FormButtonType_SUBMIT ) );
            typedef ::com::sun::star::form::submission::XSubmissionSupplier XSubmissionSupplier_t;
            Reference< XSubmissionSupplier_t > xSubmissionSupplier(pControl->GetUnoControlModel(), UNO_QUERY);
            xSubmissionSupplier->setSubmission(xSubmission);

            return pControl;
        }
    }
    catch (const Exception&)
    {
        OSL_FAIL("FmXFormView::implCreateXFormsControl: caught an exception while creating the control !");
    }


    return NULL;
}

//------------------------------------------------------------------------
bool FmXFormView::createControlLabelPair( OutputDevice& _rOutDev, sal_Int32 _nXOffsetMM, sal_Int32 _nYOffsetMM,
        const Reference< XPropertySet >& _rxField, const Reference< XNumberFormats >& _rxNumberFormats,
        sal_uInt16 _nControlObjectID, const ::rtl::OUString& _rFieldPostfix,
        SdrUnoObj*& _rpLabel, SdrUnoObj*& _rpControl,
        const Reference< XDataSource >& _rxDataSource, const ::rtl::OUString& _rDataSourceName,
        const ::rtl::OUString& _rCommand, const sal_Int32 _nCommandType )
{
    if  (   !createControlLabelPair( m_aContext, _rOutDev, _nXOffsetMM, _nYOffsetMM,
                _rxField, _rxNumberFormats, _nControlObjectID, _rFieldPostfix, FmFormInventor, OBJ_FM_FIXEDTEXT,
                NULL, NULL, NULL, _rpLabel, _rpControl )
        )
        return false;

    // insert the control model(s) into the form component hierachy
    if ( _rpLabel )
        lcl_insertIntoFormComponentHierarchy_throw( *m_pView, *_rpLabel, _rxDataSource, _rDataSourceName, _rCommand, _nCommandType );
    lcl_insertIntoFormComponentHierarchy_throw( *m_pView, *_rpControl, _rxDataSource, _rDataSourceName, _rCommand, _nCommandType );

    // some context-dependent initializations
    FormControlFactory aControlFactory( m_aContext );
    if ( _rpLabel )
        aControlFactory.initializeControlModel( impl_getDocumentType(), *_rpLabel );
    aControlFactory.initializeControlModel( impl_getDocumentType(), *_rpControl );

    return true;
}

//------------------------------------------------------------------------
bool FmXFormView::createControlLabelPair( const ::comphelper::ComponentContext& _rContext,
    OutputDevice& _rOutDev, sal_Int32 _nXOffsetMM, sal_Int32 _nYOffsetMM, const Reference< XPropertySet >& _rxField,
    const Reference< XNumberFormats >& _rxNumberFormats, sal_uInt16 _nControlObjectID,
    const ::rtl::OUString& _rFieldPostfix, sal_uInt32 _nInventor, sal_uInt16 _nLabelObjectID,
    SdrPage* _pLabelPage, SdrPage* _pControlPage, SdrModel* _pModel, SdrUnoObj*& _rpLabel, SdrUnoObj*& _rpControl)
{
    sal_Int32 nDataType = 0;
    ::rtl::OUString sFieldName;
    Any aFieldName;
    if ( _rxField.is() )
    {
        nDataType = ::comphelper::getINT32(_rxField->getPropertyValue(FM_PROP_FIELDTYPE));
        aFieldName = Any(_rxField->getPropertyValue(FM_PROP_NAME));
        aFieldName >>= sFieldName;
    }

    // calculate the positions, respecting the settings of the target device
    ::Size aTextSize( _rOutDev.GetTextWidth(sFieldName + _rFieldPostfix), _rOutDev.GetTextHeight() );

    MapMode   eTargetMode( _rOutDev.GetMapMode() ),
              eSourceMode( MAP_100TH_MM );

    // Textbreite ist mindestens 4cm
    // Texthoehe immer halber cm
    ::Size aDefTxtSize(4000, 500);
    ::Size aDefSize(4000, 500);
    ::Size aDefImageSize(4000, 4000);

    ::Size aRealSize = _rOutDev.LogicToLogic(aTextSize, eTargetMode, eSourceMode);
    aRealSize.Width() = std::max(aRealSize.Width(), aDefTxtSize.Width());
    aRealSize.Height()= aDefSize.Height();

    // adjust to scaling of the target device (#53523#)
    aRealSize.Width() = long(Fraction(aRealSize.Width(), 1) * eTargetMode.GetScaleX());
    aRealSize.Height() = long(Fraction(aRealSize.Height(), 1) * eTargetMode.GetScaleY());

    // for boolean fields, we do not create a label, but just a checkbox
    bool bNeedLabel = ( _nControlObjectID != OBJ_FM_CHECKBOX );

    // the label
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    ::std::auto_ptr< SdrUnoObj > pLabel;
    SAL_WNODEPRECATED_DECLARATIONS_POP
    Reference< XPropertySet > xLabelModel;
    if ( bNeedLabel )
    {
        pLabel.reset( dynamic_cast< SdrUnoObj* >(
            SdrObjFactory::MakeNewObject( _nInventor, _nLabelObjectID, _pLabelPage, _pModel ) ) );
        OSL_ENSURE( pLabel.get(), "FmXFormView::createControlLabelPair: could not create the label!" );
        if ( !pLabel.get() )
            return false;

        xLabelModel.set( pLabel->GetUnoControlModel(), UNO_QUERY );
        if ( xLabelModel.is() )
        {
            ::rtl::OUString sLabel;
            if ( _rxField.is() && _rxField->getPropertySetInfo()->hasPropertyByName(FM_PROP_LABEL) )
                _rxField->getPropertyValue(FM_PROP_LABEL) >>= sLabel;
            if ( sLabel.isEmpty() )
                sLabel = sFieldName;

            xLabelModel->setPropertyValue( FM_PROP_LABEL, makeAny( sLabel + _rFieldPostfix ) );
            String sObjectLabel( SVX_RES( RID_STR_OBJECT_LABEL ) );
            sObjectLabel.SearchAndReplaceAllAscii( "#object#", sFieldName );
            xLabelModel->setPropertyValue( FM_PROP_NAME, makeAny( ::rtl::OUString( sObjectLabel ) ) );
        }

        pLabel->SetLogicRect( ::Rectangle(
            _rOutDev.LogicToLogic( ::Point( _nXOffsetMM, _nYOffsetMM ), eSourceMode, eTargetMode ),
            _rOutDev.LogicToLogic( aRealSize, eSourceMode, eTargetMode )
        ) );
    }

    // the control
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    ::std::auto_ptr< SdrUnoObj > pControl( dynamic_cast< SdrUnoObj* >(
        SdrObjFactory::MakeNewObject( _nInventor, _nControlObjectID, _pControlPage, _pModel ) ) );
    SAL_WNODEPRECATED_DECLARATIONS_POP
    OSL_ENSURE( pControl.get(), "FmXFormView::createControlLabelPair: could not create the control!" );
    if ( !pControl.get() )
        return false;

    Reference< XPropertySet > xControlSet( pControl->GetUnoControlModel(), UNO_QUERY );
    if ( !xControlSet.is() )
        return false;

    // size of the control
    ::Size aControlSize( aDefSize );
    switch ( nDataType )
    {
    case DataType::BIT:
    case DataType::BOOLEAN:
        aControlSize = aDefSize;
        break;
    case DataType::LONGVARCHAR:
    case DataType::CLOB:
    case DataType::LONGVARBINARY:
    case DataType::BLOB:
        aControlSize = aDefImageSize;
        break;
    }

    if ( OBJ_FM_IMAGECONTROL == _nControlObjectID )
        aControlSize = aDefImageSize;

    aControlSize.Width() = long(Fraction(aControlSize.Width(), 1) * eTargetMode.GetScaleX());
    aControlSize.Height() = long(Fraction(aControlSize.Height(), 1) * eTargetMode.GetScaleY());

    pControl->SetLogicRect( ::Rectangle(
        _rOutDev.LogicToLogic( ::Point( aRealSize.Width() + _nXOffsetMM, _nYOffsetMM ), eSourceMode, eTargetMode ),
        _rOutDev.LogicToLogic( aControlSize, eSourceMode, eTargetMode )
    ) );

    // some initializations
    Reference< XPropertySetInfo > xControlPropInfo = xControlSet->getPropertySetInfo();

    if ( aFieldName.hasValue() )
    {
        xControlSet->setPropertyValue( FM_PROP_CONTROLSOURCE, aFieldName );
        xControlSet->setPropertyValue( FM_PROP_NAME, aFieldName );
        if ( !bNeedLabel )
        {
            // no dedicated label control => use the label property
            if ( xControlPropInfo->hasPropertyByName( FM_PROP_LABEL ) )
                xControlSet->setPropertyValue( FM_PROP_LABEL, makeAny( sFieldName + _rFieldPostfix ) );
            else
                OSL_FAIL( "FmXFormView::createControlLabelPair: can't set a label for the control!" );
        }
    }

    if ( (nDataType == DataType::LONGVARCHAR || nDataType == DataType::CLOB) && xControlPropInfo->hasPropertyByName( FM_PROP_MULTILINE ) )
    {
        xControlSet->setPropertyValue( FM_PROP_MULTILINE, makeAny( sal_Bool( sal_True ) ) );
    }

    // announce the label to the control
    if ( xControlPropInfo->hasPropertyByName( FM_PROP_CONTROLLABEL ) && xLabelModel.is() )
    {
        try
        {
            xControlSet->setPropertyValue( FM_PROP_CONTROLLABEL, makeAny( xLabelModel ) );
        }
        catch (const Exception&)
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    if ( _rxField.is() )
    {
        FormControlFactory aControlFactory( _rContext );
        aControlFactory.initializeFieldDependentProperties( _rxField, xControlSet, _rxNumberFormats );
    }

    _rpLabel = pLabel.release();
    _rpControl = pControl.release();
    return true;
}

//------------------------------------------------------------------------------
FmXFormView::ObjectRemoveListener::ObjectRemoveListener( FmXFormView* pParent )
    :m_pParent( pParent )
{
}

//------------------------------------------------------------------------------
void FmXFormView::ObjectRemoveListener::Notify( SfxBroadcaster& /*rBC*/, const SfxHint& rHint )
{
    if (rHint.ISA(SdrHint) && (((SdrHint&)rHint).GetKind() == HINT_OBJREMOVED))
        m_pParent->ObjectRemovedInAliveMode(((SdrHint&)rHint).GetObject());
}

//------------------------------------------------------------------------------
void FmXFormView::ObjectRemovedInAliveMode( const SdrObject* pObject )
{
    // wenn das entfernte Objekt in meiner MarkList, die ich mir beim Umschalten in den Alive-Mode gemerkt habe, steht,
    // muss ich es jetzt da rausnehmen, da ich sonst beim Zurueckschalten versuche, die Markierung wieder zu setzen
    // (interesanterweise geht das nur bei gruppierten Objekten schief (beim Zugriff auf deren ObjList GPF), nicht bei einzelnen)

    sal_uIntPtr nCount = m_aMark.GetMarkCount();
    for (sal_uIntPtr i = 0; i < nCount; ++i)
    {
        SdrMark* pMark = m_aMark.GetMark(i);
        SdrObject* pCurrent = pMark->GetMarkedSdrObj();
        if (pObject == pCurrent)
        {
            m_aMark.DeleteMark(i);
            return;
        }
        // ich brauche nicht in GroupObjects absteigen : wenn dort unten ein Objekt geloescht wird, dann bleibt der
        // Zeiger auf das GroupObject, den ich habe, trotzdem weiter gueltig bleibt ...
    }
}

//------------------------------------------------------------------------------
void FmXFormView::stopMarkListWatching()
{
    if ( m_pWatchStoredList )
    {
        m_pWatchStoredList->EndListeningAll();
        delete m_pWatchStoredList;
        m_pWatchStoredList = NULL;
    }
}

//------------------------------------------------------------------------------
void FmXFormView::startMarkListWatching()
{
    if ( !m_pWatchStoredList )
    {
        m_pWatchStoredList = new ObjectRemoveListener( this );
        FmFormModel* pModel = GetFormShell() ? GetFormShell()->GetFormModel() : NULL;
        DBG_ASSERT( pModel != NULL, "FmXFormView::startMarkListWatching: shell has no model!" );
        m_pWatchStoredList->StartListening( *static_cast< SfxBroadcaster* >( pModel ) );
    }
    else
    {
        OSL_FAIL( "FmXFormView::startMarkListWatching: already listening!" );
    }
}

//------------------------------------------------------------------------------
void FmXFormView::saveMarkList( sal_Bool _bSmartUnmark )
{
    if ( m_pView )
    {
        m_aMark = m_pView->GetMarkedObjectList();
        if ( _bSmartUnmark )
        {
            sal_uIntPtr nCount = m_aMark.GetMarkCount( );
            for ( sal_uIntPtr i = 0; i < nCount; ++i )
            {
                SdrMark*   pMark = m_aMark.GetMark(i);
                SdrObject* pObj  = pMark->GetMarkedSdrObj();

                if ( m_pView->IsObjMarked( pObj ) )
                {
                    if ( pObj->IsGroupObject() )
                    {
                        SdrObjListIter aIter( *pObj->GetSubList() );
                        sal_Bool bMixed = sal_False;
                        while ( aIter.IsMore() && !bMixed )
                            bMixed = ( aIter.Next()->GetObjInventor() != FmFormInventor );

                        if ( !bMixed )
                        {
                            // all objects in the group are form objects
                            m_pView->MarkObj( pMark->GetMarkedSdrObj(), pMark->GetPageView(), sal_True /* unmark! */ );
                        }
                    }
                    else
                    {
                        if ( pObj->GetObjInventor() == FmFormInventor )
                        {   // this is a form layer object
                            m_pView->MarkObj( pMark->GetMarkedSdrObj(), pMark->GetPageView(), sal_True /* unmark! */ );
                        }
                    }
                }
            }
        }
    }
    else
    {
        OSL_FAIL( "FmXFormView::saveMarkList: invalid view!" );
        m_aMark = SdrMarkList();
    }
}

//--------------------------------------------------------------------------
static sal_Bool lcl_hasObject( SdrObjListIter& rIter, SdrObject* pObj )
{
    sal_Bool bFound = sal_False;
    while (rIter.IsMore() && !bFound)
        bFound = pObj == rIter.Next();

    rIter.Reset();
    return bFound;
}

//------------------------------------------------------------------------------
void FmXFormView::restoreMarkList( SdrMarkList& _rRestoredMarkList )
{
    if ( !m_pView )
        return;

    _rRestoredMarkList.Clear();

    const SdrMarkList& rCurrentList = m_pView->GetMarkedObjectList();
    FmFormPage* pPage = GetFormShell() ? GetFormShell()->GetCurPage() : NULL;
    if (pPage)
    {
        if (rCurrentList.GetMarkCount())
        {   // there is a current mark ... hmm. Is it a subset of the mark we remembered in saveMarkList?
            sal_Bool bMisMatch = sal_False;

            // loop through all current marks
            sal_uIntPtr nCurrentCount = rCurrentList.GetMarkCount();
            for ( sal_uIntPtr i=0; i<nCurrentCount&& !bMisMatch; ++i )
            {
                const SdrObject* pCurrentMarked = rCurrentList.GetMark( i )->GetMarkedSdrObj();

                // loop through all saved marks, check for equality
                sal_Bool bFound = sal_False;
                sal_uIntPtr nSavedCount = m_aMark.GetMarkCount();
                for ( sal_uIntPtr j=0; j<nSavedCount && !bFound; ++j )
                {
                    if ( m_aMark.GetMark( j )->GetMarkedSdrObj() == pCurrentMarked )
                        bFound = sal_True;
                }

                // did not find a current mark in the saved marks
                if ( !bFound )
                    bMisMatch = sal_True;
            }

            if ( bMisMatch )
            {
                m_aMark.Clear();
                _rRestoredMarkList = rCurrentList;
                return;
            }
        }
        // wichtig ist das auf die Objecte der markliste nicht zugegriffen wird
        // da diese bereits zerstoert sein koennen
        SdrPageView* pCurPageView = m_pView->GetSdrPageView();
        SdrObjListIter aPageIter( *pPage );
        sal_Bool bFound = sal_True;

        // gibt es noch alle Objecte
        sal_uIntPtr nCount = m_aMark.GetMarkCount();
        for (sal_uIntPtr i = 0; i < nCount && bFound; i++)
        {
            SdrMark*   pMark = m_aMark.GetMark(i);
            SdrObject* pObj  = pMark->GetMarkedSdrObj();
            if (pObj->IsGroupObject())
            {
                SdrObjListIter aIter(*pObj->GetSubList());
                while (aIter.IsMore() && bFound)
                    bFound = lcl_hasObject(aPageIter, aIter.Next());
            }
            else
                bFound = lcl_hasObject(aPageIter, pObj);

            bFound = bFound && pCurPageView == pMark->GetPageView();
        }

        if (bFound)
        {
            // Das LastObject auswerten
            if (nCount) // Objecte jetzt Markieren
            {
                for (sal_uIntPtr i = 0; i < nCount; i++)
                {
                    SdrMark* pMark = m_aMark.GetMark(i);
                    SdrObject* pObj = pMark->GetMarkedSdrObj();
                    if ( pObj->GetObjInventor() == FmFormInventor )
                        if ( !m_pView->IsObjMarked( pObj ) )
                            m_pView->MarkObj( pObj, pMark->GetPageView() );
                }

                _rRestoredMarkList = m_aMark;
            }
        }
        m_aMark.Clear();
    }
}
// -----------------------------------------------------------------------------
void SAL_CALL FmXFormView::focusGained( const FocusEvent& /*e*/ ) throw (RuntimeException)
{
    if ( m_xWindow.is() && m_pView )
    {
        m_pView->SetMoveOutside( sal_True, FmFormView::ImplAccess() );
    }
}
// -----------------------------------------------------------------------------
void SAL_CALL FmXFormView::focusLost( const FocusEvent& /*e*/ ) throw (RuntimeException)
{
    // when switch the focus outside the office the mark didn't change
    // so we can not remove us as focus listener
    if ( m_xWindow.is() && m_pView )
    {
        m_pView->SetMoveOutside( sal_False, FmFormView::ImplAccess() );
    }
}
// -----------------------------------------------------------------------------
void FmXFormView::removeGridWindowListening()
{
    if ( m_xWindow.is() )
    {
        m_xWindow->removeFocusListener(this);
        if ( m_pView )
        {
            m_pView->SetMoveOutside( sal_False, FmFormView::ImplAccess() );
        }
        m_xWindow = NULL;
    }
}

// -----------------------------------------------------------------------------
DocumentType FmXFormView::impl_getDocumentType() const
{
    if ( GetFormShell() && GetFormShell()->GetImpl() )
        return GetFormShell()->GetImpl()->getDocumentType();
    return eUnknownDocumentType;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
