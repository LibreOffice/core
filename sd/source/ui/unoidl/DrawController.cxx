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

#include "DrawController.hxx"
#include "DrawDocShell.hxx"

#include "DrawSubController.hxx"
#include "sdpage.hxx"
#include "ViewShellBase.hxx"
#include "ViewShellManager.hxx"
#include "FormShellManager.hxx"
#include "Window.hxx"

#include <comphelper/anytostring.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/servicehelper.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/drawing/framework/ConfigurationController.hpp>
#include <com/sun/star/drawing/framework/ModuleController.hpp>
#include <com/sun/star/lang/XInitialization.hpp>

#include "slideshow.hxx"

#include <svx/fmshell.hxx>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <sfx2/sidebar/EnumContext.hxx>
#include <svx/sidebar/ContextChangeEventMultiplexer.hxx>

#include <memory>

using namespace ::std;
using namespace ::cppu;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;
using ::sfx2::sidebar::EnumContext;

namespace sd {

DrawController::DrawController (ViewShellBase& rBase) throw()
    : DrawControllerInterfaceBase(&rBase),
      BroadcastHelperOwner(SfxBaseController::m_aMutex),
      OPropertySetHelper( static_cast<OBroadcastHelperVar<
          OMultiTypeInterfaceContainerHelper,
          OMultiTypeInterfaceContainerHelper::keyType>& >(
              BroadcastHelperOwner::maBroadcastHelper)),
      m_aSelectionTypeIdentifier(
        cppu::UnoType<view::XSelectionChangeListener>::get()),
      mpBase(&rBase),
      maLastVisArea(),
      mpCurrentPage(nullptr),
      mbMasterPageMode(false),
      mbLayerMode(false),
      mbDisposing(false),
      mxSubController(),
      mxConfigurationController(),
      mxModuleController()
{
    ProvideFrameworkControllers();
}

DrawController::~DrawController() throw()
{
}

void DrawController::SetSubController (
    const Reference<drawing::XDrawSubController>& rxSubController)
{
    // Update the internal state.
    mxSubController = rxSubController;
    mpPropertyArrayHelper.reset();
    maLastVisArea = Rectangle();

    // Inform listeners about the changed state.
    FireSelectionChangeListener();
}

// XInterface

IMPLEMENT_FORWARD_XINTERFACE2(
    DrawController,
    DrawControllerInterfaceBase,
    OPropertySetHelper);

// XTypeProvider

Sequence<Type> SAL_CALL DrawController::getTypes()
    throw (css::uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    // OPropertySetHelper does not provide getTypes, so we have to
    // implement this method manually and list its three interfaces.
    OTypeCollection aTypeCollection (
        cppu::UnoType<beans::XMultiPropertySet>::get(),
        cppu::UnoType<beans::XFastPropertySet>::get(),
        cppu::UnoType<beans::XPropertySet>::get());

    return ::comphelper::concatSequences(
        SfxBaseController::getTypes(),
        aTypeCollection.getTypes(),
        DrawControllerInterfaceBase::getTypes());
}

IMPLEMENT_GET_IMPLEMENTATION_ID(DrawController);

// XComponent

void SAL_CALL DrawController::dispose()
    throw( RuntimeException, std::exception )
{
    if( !mbDisposing )
    {
        SolarMutexGuard aGuard;

        if( !mbDisposing )
        {
            mbDisposing = true;

            std::shared_ptr<ViewShell> pViewShell;
            if (mpBase)
                pViewShell = mpBase->GetMainViewShell();
            if ( pViewShell )
            {
                pViewShell->DeactivateCurrentFunction();
                DrawDocShell* pDocShell = pViewShell->GetDocSh();
                if ( pDocShell != nullptr )
                    pDocShell->SetDocShellFunction(nullptr);
            }
            pViewShell.reset();

            // When the controller has not been detached from its view
            // shell, i.e. mpViewShell is not NULL, then tell PaneManager
            // and ViewShellManager to clear the shell stack.
            if (mxSubController.is() && mpBase!=nullptr)
            {
                mpBase->DisconnectAllClients();
                mpBase->GetViewShellManager()->Shutdown();
            }

            OPropertySetHelper::disposing();

            DisposeFrameworkControllers();

            SfxBaseController::dispose();
        }
    }
}

void SAL_CALL DrawController::addEventListener(
    const Reference<lang::XEventListener >& xListener)
    throw (RuntimeException, std::exception)
{
    ThrowIfDisposed();
    SfxBaseController::addEventListener( xListener );
}

void SAL_CALL DrawController::removeEventListener (
    const Reference<lang::XEventListener >& aListener)
    throw (RuntimeException, std::exception)
{
    if(!rBHelper.bDisposed && !rBHelper.bInDispose && !mbDisposing)
        SfxBaseController::removeEventListener( aListener );
}

// XController
sal_Bool SAL_CALL DrawController::suspend( sal_Bool Suspend ) throw (css::uno::RuntimeException, std::exception)
{
    if( Suspend )
    {
        ViewShellBase* pViewShellBase = GetViewShellBase();
        if( pViewShellBase )
        {
            // do not allow suspend if a slideshow needs this controller!
            rtl::Reference< SlideShow > xSlideShow( SlideShow::GetSlideShow( *pViewShellBase ) );
            if( xSlideShow.is() && xSlideShow->dependsOn(pViewShellBase) )
                return sal_False;
        }
    }

    return SfxBaseController::suspend( Suspend );
}

// XServiceInfo
OUString SAL_CALL DrawController::getImplementationName(  ) throw(RuntimeException, std::exception)
{
    // Do not throw an excepetion at the moment.  This leads to a crash
    // under Solaris on reload.  See issue i70929 for details.
    //    ThrowIfDisposed();
    return OUString("DrawController") ;
}

static const char ssServiceName[] = "com.sun.star.drawing.DrawingDocumentDrawView";

sal_Bool SAL_CALL DrawController::supportsService (const OUString& rsServiceName)
    throw(RuntimeException, std::exception)
{
    return cppu::supportsService(this, rsServiceName);
}

Sequence<OUString> SAL_CALL DrawController::getSupportedServiceNames()
    throw(RuntimeException, std::exception)
{
    ThrowIfDisposed();
    Sequence<OUString> aSupportedServices { ssServiceName };
    return aSupportedServices;
}

//------ XSelectionSupplier --------------------------------------------
sal_Bool SAL_CALL DrawController::select (const Any& aSelection)
    throw(lang::IllegalArgumentException, RuntimeException, std::exception)
{
    ThrowIfDisposed();
    SolarMutexGuard aGuard;

    if (mxSubController.is())
        return mxSubController->select(aSelection);
    else
        return false;
}

Any SAL_CALL DrawController::getSelection()
    throw(RuntimeException, std::exception)
{
    ThrowIfDisposed();
    SolarMutexGuard aGuard;

    if (mxSubController.is())
        return mxSubController->getSelection();
    else
        return Any();
}

void SAL_CALL DrawController::addSelectionChangeListener(
    const Reference< view::XSelectionChangeListener >& xListener)
    throw(RuntimeException, std::exception)
{
    if( mbDisposing )
        throw lang::DisposedException();

    BroadcastHelperOwner::maBroadcastHelper.addListener (m_aSelectionTypeIdentifier, xListener);
}

void SAL_CALL DrawController::removeSelectionChangeListener(
    const Reference< view::XSelectionChangeListener >& xListener )
    throw(RuntimeException, std::exception)
{
    if (rBHelper.bDisposed)
        throw lang::DisposedException();

    BroadcastHelperOwner::maBroadcastHelper.removeListener (m_aSelectionTypeIdentifier, xListener);
}

//=====  lang::XEventListener  ================================================

void SAL_CALL
    DrawController::disposing (const lang::EventObject& )
    throw (uno::RuntimeException, std::exception)
{
}

//=====  view::XSelectionChangeListener  ======================================

void  SAL_CALL
    DrawController::selectionChanged (const lang::EventObject& rEvent)
        throw (uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    // Have to forward the event to our selection change listeners.
    OInterfaceContainerHelper* pListeners = BroadcastHelperOwner::maBroadcastHelper.getContainer(
        cppu::UnoType<view::XSelectionChangeListener>::get());
    if (pListeners)
    {
        // Re-send the event to all of our listeners.
        OInterfaceIteratorHelper aIterator (*pListeners);
        while (aIterator.hasMoreElements())
        {
            try
            {
                view::XSelectionChangeListener* pListener =
                    static_cast<view::XSelectionChangeListener*>(
                        aIterator.next());
                if (pListener != nullptr)
                    pListener->selectionChanged (rEvent);
            }
            catch (const RuntimeException&)
            {
            }
        }
    }
}

// XDrawView

void SAL_CALL DrawController::setCurrentPage( const Reference< drawing::XDrawPage >& xPage )
    throw(RuntimeException, std::exception)
{
    ThrowIfDisposed();
    SolarMutexGuard aGuard;

    if (mxSubController.is())
        mxSubController->setCurrentPage(xPage);
}

Reference< drawing::XDrawPage > SAL_CALL DrawController::getCurrentPage()
    throw(RuntimeException, std::exception)
{
    ThrowIfDisposed();
    SolarMutexGuard aGuard;
    Reference<drawing::XDrawPage> xPage;

    // Get current page from sub controller.
    if (mxSubController.is())
        xPage = mxSubController->getCurrentPage();

    // When there is not yet a sub controller (during initialization) then fall back
    // to the current page in mpCurrentPage.
    if ( ! xPage.is() && mpCurrentPage.is())
        xPage.set(mpCurrentPage->getUnoPage(), UNO_QUERY);

    return xPage;
}

void DrawController::FireVisAreaChanged (const Rectangle& rVisArea) throw()
{
    if( maLastVisArea != rVisArea )
    {
        Any aNewValue;
        aNewValue <<= awt::Rectangle(
            rVisArea.Left(),
            rVisArea.Top(),
            rVisArea.GetWidth(),
            rVisArea.GetHeight() );

        Any aOldValue;
        aOldValue <<= awt::Rectangle(
            maLastVisArea.Left(),
            maLastVisArea.Top(),
            maLastVisArea.GetWidth(),
            maLastVisArea.GetHeight() );

        FirePropertyChange (PROPERTY_WORKAREA, aNewValue, aOldValue);

        maLastVisArea = rVisArea;
    }
}

void DrawController::FireSelectionChangeListener() throw()
{
    OInterfaceContainerHelper * pLC = BroadcastHelperOwner::maBroadcastHelper.getContainer(
        m_aSelectionTypeIdentifier);
    if( pLC )
    {
        Reference< XInterface > xSource( static_cast<XWeak*>(this) );
        const lang::EventObject aEvent( xSource );

        // iterate over all listeners and send events
        OInterfaceIteratorHelper aIt( *pLC);
        while( aIt.hasMoreElements() )
        {
            try
            {
                view::XSelectionChangeListener * pL =
                    static_cast<view::XSelectionChangeListener*>(aIt.next());
                if (pL != nullptr)
                    pL->selectionChanged( aEvent );
            }
            catch (const RuntimeException&)
            {
            }
        }
    }
}

void DrawController::FireChangeEditMode (bool bMasterPageMode) throw()
{
    if (bMasterPageMode != mbMasterPageMode )
    {
        FirePropertyChange(
            PROPERTY_MASTERPAGEMODE,
            makeAny(bMasterPageMode),
            makeAny(mbMasterPageMode));

        mbMasterPageMode = bMasterPageMode;
    }
}

void DrawController::FireChangeLayerMode (bool bLayerMode) throw()
{
    if (bLayerMode != mbLayerMode)
    {
        FirePropertyChange(
            PROPERTY_LAYERMODE,
            makeAny(bLayerMode),
            makeAny(mbLayerMode));

        mbLayerMode = bLayerMode;
    }
}

void DrawController::FireSwitchCurrentPage (SdPage* pNewCurrentPage) throw()
{
    SdrPage* pCurrentPage  = mpCurrentPage.get();
    if (pNewCurrentPage != pCurrentPage)
    {
        try
        {
            Any aNewValue (
                makeAny(Reference<drawing::XDrawPage>(pNewCurrentPage->getUnoPage(), UNO_QUERY)));

            Any aOldValue;
            if (pCurrentPage != nullptr)
            {
                Reference<drawing::XDrawPage> xOldPage (pCurrentPage->getUnoPage(), UNO_QUERY);
                aOldValue <<= xOldPage;
            }

            FirePropertyChange(PROPERTY_CURRENTPAGE, aNewValue, aOldValue);

            mpCurrentPage.reset(pNewCurrentPage);
        }
        catch (const uno::Exception& e)
        {
            SAL_WARN("sd", "sd::SdUnoDrawView::FireSwitchCurrentPage(), exception caught:  " << e.Message);
        }
    }
}

void DrawController::NotifyAccUpdate()
{
    sal_Int32 nHandle = PROPERTY_UPDATEACC;
    Any aNewValue, aOldValue;
    fire (&nHandle, &aNewValue, &aOldValue, 1, sal_False);
}

void DrawController::fireChangeLayer( css::uno::Reference< css::drawing::XLayer>* pCurrentLayer ) throw()
{
    if( pCurrentLayer != mpCurrentLayer )
    {
        sal_Int32 nHandle = PROPERTY_ACTIVE_LAYER;

        Any aNewValue (makeAny( *pCurrentLayer) );

        Any aOldValue ;

        fire (&nHandle, &aNewValue, &aOldValue, 1, sal_False);

        mpCurrentLayer = pCurrentLayer;
    }
}

// This method is only called in slide show and outline view
//void DrawController::fireSwitchCurrentPage(String pageName ) throw()
void DrawController::fireSwitchCurrentPage(sal_Int32 pageIndex ) throw()
{
        Any aNewValue;
        Any aOldValue;
        //OUString aPageName(  pageName );
        //aNewValue <<= aPageName ;
        aNewValue <<= pageIndex;

        // Use new property to handle page change event
        sal_Int32 nHandles = PROPERTY_PAGE_CHANGE;
        fire( &nHandles, &aNewValue, &aOldValue, 1, sal_False );
}

void DrawController::FirePropertyChange (
    sal_Int32 nHandle,
    const Any& rNewValue,
    const Any& rOldValue)
{
    try
    {
        fire (&nHandle, &rNewValue, &rOldValue, 1, sal_False);
    }
    catch (const RuntimeException&)
    {
        // Ignore this exception.  Exceptions should be handled in the
        // fire() function so that all listeners are called.  This is
        // not the case at the moment, so we simply ignore the
        // exception.
    }

}

void DrawController::BroadcastContextChange() const
{
    std::shared_ptr<ViewShell> pViewShell (mpBase->GetMainViewShell());
    if ( ! pViewShell)
        return;

    EnumContext::Context eContext (EnumContext::Context_Unknown);
    switch (pViewShell->GetShellType())
    {
        case ViewShell::ST_IMPRESS:
        case ViewShell::ST_DRAW:
            if (mbMasterPageMode)
                eContext = EnumContext::Context_MasterPage;
            else
                eContext = EnumContext::Context_DrawPage;
            break;

        case ViewShell::ST_NOTES:
            eContext = EnumContext::Context_NotesPage;
            break;

        case ViewShell::ST_HANDOUT:
            eContext = EnumContext::Context_HandoutPage;
            break;

        case ViewShell::ST_OUTLINE:
            eContext = EnumContext::Context_OutlineText;
            break;

        case ViewShell::ST_SLIDE_SORTER:
            eContext = EnumContext::Context_SlidesorterPage;
            break;

        case ViewShell::ST_PRESENTATION:
        case ViewShell::ST_NONE:
        default:
            eContext = EnumContext::Context_Empty;
            break;
    }

    ContextChangeEventMultiplexer::NotifyContextChange(mpBase, eContext);
}

void DrawController::ReleaseViewShellBase()
{
    DisposeFrameworkControllers();
    mpBase = nullptr;
}

//===== XControllerManager ==============================================================

Reference<XConfigurationController> SAL_CALL
    DrawController::getConfigurationController()
    throw (RuntimeException, std::exception)
{
    ThrowIfDisposed();

    return mxConfigurationController;
}

Reference<XModuleController> SAL_CALL
    DrawController::getModuleController()
    throw (RuntimeException, std::exception)
{
    ThrowIfDisposed();

    return mxModuleController;
}

//===== XUnoTunnel ============================================================

namespace
{
    class theDrawControllerUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theDrawControllerUnoTunnelId> {};
}

const Sequence<sal_Int8>& DrawController::getUnoTunnelId()
{
    return theDrawControllerUnoTunnelId::get().getSeq();
}

sal_Int64 SAL_CALL DrawController::getSomething (const Sequence<sal_Int8>& rId)
    throw (RuntimeException, std::exception)
{
    sal_Int64 nResult = 0;

    if (rId.getLength() == 16
        && memcmp(getUnoTunnelId().getConstArray(), rId.getConstArray(), 16) == 0)
    {
        nResult = sal::static_int_cast<sal_Int64>(reinterpret_cast<sal_IntPtr>(this));
    }

    return nResult;
}

//===== Properties ============================================================

void DrawController::FillPropertyTable (
    ::std::vector<beans::Property>& rProperties)
{
    rProperties.push_back(
        beans::Property("VisibleArea",
            PROPERTY_WORKAREA,
            ::cppu::UnoType< css::awt::Rectangle>::get(),
            beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY));
    rProperties.push_back(
        beans::Property(
            "SubController",
            PROPERTY_SUB_CONTROLLER,
            cppu::UnoType<drawing::XDrawSubController>::get(),
            beans::PropertyAttribute::BOUND));
    rProperties.push_back(
        beans::Property(
            "CurrentPage",
            PROPERTY_CURRENTPAGE,
            cppu::UnoType<drawing::XDrawPage>::get(),
            beans::PropertyAttribute::BOUND ));
    rProperties.push_back(
        beans::Property("IsLayerMode",
            PROPERTY_LAYERMODE,
            cppu::UnoType<bool>::get(),
            beans::PropertyAttribute::BOUND ));
    rProperties.push_back(
        beans::Property("IsMasterPageMode",
            PROPERTY_MASTERPAGEMODE,
            cppu::UnoType<bool>::get(),
            beans::PropertyAttribute::BOUND ));
    rProperties.push_back(
        beans::Property("ActiveLayer",
            PROPERTY_ACTIVE_LAYER,
            cppu::UnoType<drawing::XLayer>::get(),
            beans::PropertyAttribute::BOUND ));
    rProperties.push_back(
        beans::Property("ZoomValue",
            PROPERTY_ZOOMVALUE,
            ::cppu::UnoType<sal_Int16>::get(),
            beans::PropertyAttribute::BOUND ));
    rProperties.push_back(
        beans::Property("ZoomType",
            PROPERTY_ZOOMTYPE,
            ::cppu::UnoType<sal_Int16>::get(),
            beans::PropertyAttribute::BOUND ));
    rProperties.push_back(
        beans::Property("ViewOffset",
            PROPERTY_VIEWOFFSET,
            ::cppu::UnoType< css::awt::Point>::get(),
            beans::PropertyAttribute::BOUND ));
    rProperties.push_back(
        beans::Property("DrawViewMode",
            PROPERTY_DRAWVIEWMODE,
            ::cppu::UnoType< css::awt::Point>::get(),
            beans::PropertyAttribute::BOUND|beans::PropertyAttribute::READONLY|beans::PropertyAttribute::MAYBEVOID ));
    // add new property to update current page's acc information
    rProperties.push_back(
        beans::Property( OUString( RTL_CONSTASCII_USTRINGPARAM("UpdateAcc") ),
            PROPERTY_UPDATEACC,
            ::cppu::UnoType<sal_Int16>::get(),
            beans::PropertyAttribute::BOUND ));
    rProperties.push_back(
        beans::Property( OUString( RTL_CONSTASCII_USTRINGPARAM("PageChange") ),
            PROPERTY_PAGE_CHANGE,
            ::cppu::UnoType<sal_Int16>::get(),
            beans::PropertyAttribute::BOUND ));
}

IPropertyArrayHelper & DrawController::getInfoHelper()
{
    SolarMutexGuard aGuard;

    if (mpPropertyArrayHelper.get() == nullptr)
    {
        ::std::vector<beans::Property> aProperties;
        FillPropertyTable (aProperties);
        Sequence<beans::Property> aPropertySequence (aProperties.size());
        for (size_t i=0; i<aProperties.size(); i++)
            aPropertySequence[i] = aProperties[i];
        mpPropertyArrayHelper.reset(new OPropertyArrayHelper(aPropertySequence, sal_False));
    }

    return *mpPropertyArrayHelper.get();
}

Reference < beans::XPropertySetInfo >  DrawController::getPropertySetInfo()
        throw ( css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    static Reference < beans::XPropertySetInfo >  xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

uno::Reference< form::runtime::XFormController > SAL_CALL DrawController::getFormController( const uno::Reference< form::XForm >& Form ) throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    FmFormShell* pFormShell = mpBase->GetFormShellManager()->GetFormShell();
    SdrView* pSdrView = mpBase->GetDrawView();
    std::shared_ptr<ViewShell> pViewShell = mpBase->GetMainViewShell();
    ::sd::Window* pWindow = pViewShell ? pViewShell->GetActiveWindow() : nullptr;

    uno::Reference< form::runtime::XFormController > xController( nullptr );
    if ( pFormShell && pSdrView && pWindow )
        xController = FmFormShell::GetFormController( Form, *pSdrView, *pWindow );
    return xController;
}

sal_Bool SAL_CALL DrawController::isFormDesignMode(  ) throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    bool bIsDesignMode = true;

    FmFormShell* pFormShell = mpBase->GetFormShellManager()->GetFormShell();
    if ( pFormShell )
        bIsDesignMode = pFormShell->IsDesignMode();

    return bIsDesignMode;
}

void SAL_CALL DrawController::setFormDesignMode( sal_Bool _DesignMode ) throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    FmFormShell* pFormShell = mpBase->GetFormShellManager()->GetFormShell();
    if ( pFormShell )
        pFormShell->SetDesignMode( _DesignMode );
}

uno::Reference< awt::XControl > SAL_CALL DrawController::getControl( const uno::Reference< awt::XControlModel >& xModel ) throw (container::NoSuchElementException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    FmFormShell* pFormShell = mpBase->GetFormShellManager()->GetFormShell();
    SdrView* pSdrView = mpBase->GetDrawView();
    std::shared_ptr<ViewShell> pViewShell = mpBase->GetMainViewShell();
    ::sd::Window* pWindow = pViewShell ? pViewShell->GetActiveWindow() : nullptr;

    uno::Reference< awt::XControl > xControl( nullptr );
    if ( pFormShell && pSdrView && pWindow )
        pFormShell->GetFormControl( xModel, *pSdrView, *pWindow, xControl );
    return xControl;
}

sal_Bool DrawController::convertFastPropertyValue (
    Any & rConvertedValue,
    Any & rOldValue,
    sal_Int32 nHandle,
    const Any& rValue)
    throw ( css::lang::IllegalArgumentException)
{
    bool bResult = false;

    if (nHandle == PROPERTY_SUB_CONTROLLER)
    {
        rOldValue <<= mxSubController;
        rConvertedValue <<= Reference<drawing::XDrawSubController>(rValue, UNO_QUERY);
        bResult = (rOldValue != rConvertedValue);
    }
    else if (mxSubController.is())
    {
        rConvertedValue = rValue;
        try
        {
            rOldValue = mxSubController->getFastPropertyValue(nHandle);
            bResult = (rOldValue != rConvertedValue);
        }
        catch (const beans::UnknownPropertyException&)
        {
            // The property is unknown and thus an illegal argument to this method.
            throw css::lang::IllegalArgumentException();
        }
    }

    return bResult;
}

void DrawController::setFastPropertyValue_NoBroadcast (
    sal_Int32 nHandle,
    const Any& rValue)
    throw ( css::uno::Exception, std::exception)
{
    SolarMutexGuard aGuard;
    if (nHandle == PROPERTY_SUB_CONTROLLER)
        SetSubController(Reference<drawing::XDrawSubController>(rValue, UNO_QUERY));
    else if (mxSubController.is())
        mxSubController->setFastPropertyValue(nHandle, rValue);
}

void DrawController::getFastPropertyValue (
    Any & rRet,
    sal_Int32 nHandle ) const
{
    SolarMutexGuard aGuard;

    switch( nHandle )
    {
        case PROPERTY_WORKAREA:
            rRet <<= awt::Rectangle(
                maLastVisArea.Left(),
                maLastVisArea.Top(),
                maLastVisArea.GetWidth(),
                maLastVisArea.GetHeight());
            break;

        case PROPERTY_SUB_CONTROLLER:
            rRet <<= mxSubController;
            break;

        default:
            if (mxSubController.is())
                rRet = mxSubController->getFastPropertyValue(nHandle);
            break;
    }
}

void DrawController::ProvideFrameworkControllers()
{
    SolarMutexGuard aGuard;
    try
    {
        Reference<XController> xController (this);
        const Reference<XComponentContext> xContext (
            ::comphelper::getProcessComponentContext() );
        mxConfigurationController = ConfigurationController::create(
            xContext,
            xController);
        mxModuleController = ModuleController::create(
            xContext,
            xController);
    }
    catch (const RuntimeException&)
    {
        mxConfigurationController = nullptr;
        mxModuleController = nullptr;
    }
}

void DrawController::DisposeFrameworkControllers()
{
    Reference<XComponent> xComponent (mxModuleController, UNO_QUERY);
    if (xComponent.is())
        xComponent->dispose();

    xComponent.set(mxConfigurationController, UNO_QUERY);
    if (xComponent.is())
        xComponent->dispose();
}

void DrawController::ThrowIfDisposed() const
    throw (css::lang::DisposedException)
{
    if (rBHelper.bDisposed || rBHelper.bInDispose || mbDisposing)
    {
        OSL_TRACE ("Calling disposed DrawController object. Throwing exception:");
        throw lang::DisposedException (
            "DrawController object has already been disposed",
            const_cast<uno::XWeak*>(static_cast<const uno::XWeak*>(this)));
    }
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
