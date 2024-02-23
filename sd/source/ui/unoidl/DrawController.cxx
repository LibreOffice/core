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

#include <DrawController.hxx>

#include <sdpage.hxx>
#include <ViewShell.hxx>
#include <ViewShellBase.hxx>
#include <ViewShellManager.hxx>
#include <FormShellManager.hxx>
#include <Window.hxx>
#include <framework/ConfigurationController.hxx>
#include <framework/ModuleController.hxx>

#include <comphelper/processfactory.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/servicehelper.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/typeprovider.hxx>

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/drawing/XDrawSubController.hpp>
#include <com/sun/star/drawing/XLayer.hpp>

#include <slideshow.hxx>

#include <sal/log.hxx>
#include <svx/fmshell.hxx>
#include <vcl/svapp.hxx>
#include <vcl/EnumContext.hxx>
#include <svx/sidebar/ContextChangeEventMultiplexer.hxx>
#include <comphelper/diagnose_ex.hxx>

#include <memory>

using namespace ::cppu;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;
using vcl::EnumContext;

namespace sd {

DrawController::DrawController (ViewShellBase& rBase) noexcept
    : DrawControllerInterfaceBase(&rBase),
      BroadcastHelperOwner(SfxBaseController::m_aMutex),
      OPropertySetHelper(BroadcastHelperOwner::maBroadcastHelper),
      mpCurrentLayer(nullptr),
      m_aSelectionTypeIdentifier(
        cppu::UnoType<view::XSelectionChangeListener>::get()),
      mpBase(&rBase),
      mpCurrentPage(nullptr),
      mbMasterPageMode(false),
      mbLayerMode(false),
      mbDisposing(false)
{
    ProvideFrameworkControllers();
}

DrawController::~DrawController() noexcept
{
}

void DrawController::SetSubController (
    const Reference<drawing::XDrawSubController>& rxSubController)
{
    // Update the internal state.
    mxSubController = rxSubController;
    mpPropertyArrayHelper.reset();
    maLastVisArea = ::tools::Rectangle();

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
{
    if( mbDisposing )
        return;

    SolarMutexGuard aGuard;

    if( mbDisposing )
        return;

    mbDisposing = true;

    std::shared_ptr<ViewShell> pViewShell;
    if (mpBase)
        pViewShell = mpBase->GetMainViewShell();
    if ( pViewShell )
    {
        pViewShell->DeactivateCurrentFunction();
        auto* pView = pViewShell->GetView();
        if (pView)
            pView->getSearchContext().resetSearchFunction();
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

void SAL_CALL DrawController::addEventListener(
    const Reference<lang::XEventListener >& xListener)
{
    ThrowIfDisposed();
    SfxBaseController::addEventListener( xListener );
}

void SAL_CALL DrawController::removeEventListener (
    const Reference<lang::XEventListener >& aListener)
{
    if(!rBHelper.bDisposed && !rBHelper.bInDispose && !mbDisposing)
        SfxBaseController::removeEventListener( aListener );
}

// XController
sal_Bool SAL_CALL DrawController::suspend( sal_Bool Suspend )
{
    if( Suspend )
    {
        ViewShellBase* pViewShellBase = GetViewShellBase();
        if( pViewShellBase )
        {
            // do not allow suspend if a slideshow needs this controller!
            rtl::Reference< SlideShow > xSlideShow( SlideShow::GetSlideShow( *pViewShellBase ) );
            if (xSlideShow.is())
            {
                if (xSlideShow->IsInteractiveSlideshow())
                {
                    // IASS mode: If preview mode, end it
                    if (xSlideShow->isInteractiveSetup())
                        xSlideShow->endInteractivePreview();

                    // end the SlideShow
                    xSlideShow->end();

                    // use SfxBaseController::suspend( Suspend ) below
                    // for normal processing and return value
                }
                else
                {
                    // original reaction - prevent exit
                    if (xSlideShow->dependsOn(pViewShellBase))
                        return false;
                }
            }
        }
    }

    return SfxBaseController::suspend( Suspend );
}

// XServiceInfo
OUString SAL_CALL DrawController::getImplementationName(  )
{
    // Do not throw an exception at the moment.  This leads to a crash
    // under Solaris on reload.  See issue i70929 for details.
    //    ThrowIfDisposed();
    return "DrawController" ;
}

constexpr OUString ssServiceName = u"com.sun.star.drawing.DrawingDocumentDrawView"_ustr;

sal_Bool SAL_CALL DrawController::supportsService (const OUString& rsServiceName)
{
    return cppu::supportsService(this, rsServiceName);
}

Sequence<OUString> SAL_CALL DrawController::getSupportedServiceNames()
{
    ThrowIfDisposed();
    Sequence<OUString> aSupportedServices { ssServiceName };
    return aSupportedServices;
}

//------ XSelectionSupplier --------------------------------------------
sal_Bool SAL_CALL DrawController::select (const Any& aSelection)
{
    ThrowIfDisposed();
    SolarMutexGuard aGuard;

    if (mxSubController.is())
        return mxSubController->select(aSelection);
    else
        return false;
}

Any SAL_CALL DrawController::getSelection()
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
{
    if( mbDisposing )
        throw lang::DisposedException();

    BroadcastHelperOwner::maBroadcastHelper.addListener (m_aSelectionTypeIdentifier, xListener);
}

void SAL_CALL DrawController::removeSelectionChangeListener(
    const Reference< view::XSelectionChangeListener >& xListener )
{
    if (rBHelper.bDisposed)
        throw lang::DisposedException();

    BroadcastHelperOwner::maBroadcastHelper.removeListener (m_aSelectionTypeIdentifier, xListener);
}

//=====  lang::XEventListener  ================================================

void SAL_CALL
    DrawController::disposing (const lang::EventObject& )
{
}

//=====  view::XSelectionChangeListener  ======================================

void  SAL_CALL
    DrawController::selectionChanged (const lang::EventObject& rEvent)
{
    ThrowIfDisposed();
    // Have to forward the event to our selection change listeners.
    OInterfaceContainerHelper* pListeners = BroadcastHelperOwner::maBroadcastHelper.getContainer(
        cppu::UnoType<view::XSelectionChangeListener>::get());
    if (!pListeners)
        return;

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

// XDrawView

void SAL_CALL DrawController::setCurrentPage( const Reference< drawing::XDrawPage >& xPage )
{
    ThrowIfDisposed();
    SolarMutexGuard aGuard;

    if (mxSubController.is())
        mxSubController->setCurrentPage(xPage);
}

Reference< drawing::XDrawPage > SAL_CALL DrawController::getCurrentPage()
{
    ThrowIfDisposed();
    SolarMutexGuard aGuard;
    Reference<drawing::XDrawPage> xPage;

    // Get current page from sub controller.
    if (mxSubController.is())
        xPage = mxSubController->getCurrentPage();

    // When there is not yet a sub controller (during initialization) then fall back
    // to the current page in mpCurrentPage.
    if ( ! xPage.is() )
        if (rtl::Reference<SdPage> pPage = mpCurrentPage.get())
            xPage.set(pPage->getUnoPage(), UNO_QUERY);

    return xPage;
}

void DrawController::FireVisAreaChanged (const ::tools::Rectangle& rVisArea) noexcept
{
    if( maLastVisArea == rVisArea )
        return;

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

void DrawController::FireSelectionChangeListener() noexcept
{
    OInterfaceContainerHelper * pLC = BroadcastHelperOwner::maBroadcastHelper.getContainer(
        m_aSelectionTypeIdentifier);
    if( !pLC )
        return;

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

void DrawController::FireChangeEditMode (bool bMasterPageMode) noexcept
{
    if (bMasterPageMode != mbMasterPageMode )
    {
        FirePropertyChange(
            PROPERTY_MASTERPAGEMODE,
            Any(bMasterPageMode),
            Any(mbMasterPageMode));

        mbMasterPageMode = bMasterPageMode;
    }
}

void DrawController::FireChangeLayerMode (bool bLayerMode) noexcept
{
    if (bLayerMode != mbLayerMode)
    {
        FirePropertyChange(
            PROPERTY_LAYERMODE,
            Any(bLayerMode),
            Any(mbLayerMode));

        mbLayerMode = bLayerMode;
    }
}

void DrawController::FireSwitchCurrentPage (SdPage* pNewCurrentPage) noexcept
{
    rtl::Reference<SdrPage> pCurrentPage  = mpCurrentPage.get();
    if (pNewCurrentPage == pCurrentPage.get())
        return;

    try
    {
        Any aNewValue (
            Any(Reference<drawing::XDrawPage>(pNewCurrentPage->getUnoPage(), UNO_QUERY)));

        Any aOldValue;
        if (pCurrentPage != nullptr)
        {
            Reference<drawing::XDrawPage> xOldPage (pCurrentPage->getUnoPage(), UNO_QUERY);
            aOldValue <<= xOldPage;
        }

        FirePropertyChange(PROPERTY_CURRENTPAGE, aNewValue, aOldValue);

        mpCurrentPage = pNewCurrentPage;
    }
    catch (const uno::Exception&)
    {
        TOOLS_WARN_EXCEPTION("sd", "sd::SdUnoDrawView::FireSwitchCurrentPage()");
    }
}

void DrawController::NotifyAccUpdate()
{
    sal_Int32 nHandle = PROPERTY_UPDATEACC;
    Any aNewValue, aOldValue;
    fire (&nHandle, &aNewValue, &aOldValue, 1, false);
}

void DrawController::fireChangeLayer( css::uno::Reference< css::drawing::XLayer>* pCurrentLayer ) noexcept
{
    if( pCurrentLayer != mpCurrentLayer )
    {
        sal_Int32 nHandle = PROPERTY_ACTIVE_LAYER;

        Any aNewValue ( *pCurrentLayer);

        Any aOldValue ;

        fire (&nHandle, &aNewValue, &aOldValue, 1, false);

        mpCurrentLayer = pCurrentLayer;
    }
}

// This method is only called in slide show and outline view
//void DrawController::fireSwitchCurrentPage(String pageName ) throw()
void DrawController::fireSwitchCurrentPage(sal_Int32 pageIndex ) noexcept
{
        Any aNewValue;
        Any aOldValue;
        //OUString aPageName(  pageName );
        //aNewValue <<= aPageName ;
        aNewValue <<= pageIndex;

        // Use new property to handle page change event
        sal_Int32 nHandles = PROPERTY_PAGE_CHANGE;
        fire( &nHandles, &aNewValue, &aOldValue, 1, false );
}

void DrawController::FirePropertyChange (
    sal_Int32 nHandle,
    const Any& rNewValue,
    const Any& rOldValue)
{
    try
    {
        fire (&nHandle, &rNewValue, &rOldValue, 1, false);
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

    EnumContext::Context eContext (EnumContext::Context::Unknown);
    switch (pViewShell->GetShellType())
    {
        case ViewShell::ST_IMPRESS:
        case ViewShell::ST_DRAW:
            if (mbMasterPageMode)
                eContext = EnumContext::Context::MasterPage;
            else
                eContext = EnumContext::Context::DrawPage;
            break;

        case ViewShell::ST_NOTES:
            eContext = EnumContext::Context::NotesPage;
            break;

        case ViewShell::ST_HANDOUT:
            eContext = EnumContext::Context::HandoutPage;
            break;

        case ViewShell::ST_OUTLINE:
            eContext = EnumContext::Context::OutlineText;
            break;

        case ViewShell::ST_SLIDE_SORTER:
            eContext = EnumContext::Context::SlidesorterPage;
            break;

        case ViewShell::ST_PRESENTATION:
        case ViewShell::ST_NONE:
        default:
            eContext = EnumContext::Context::Empty;
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
{
    ThrowIfDisposed();

    return mxConfigurationController;
}

Reference<XModuleController> SAL_CALL
    DrawController::getModuleController()
{
    ThrowIfDisposed();

    return mxModuleController;
}

//===== Properties ============================================================

void DrawController::FillPropertyTable (
    ::std::vector<beans::Property>& rProperties)
{
    rProperties.emplace_back("VisibleArea",
            PROPERTY_WORKAREA,
            ::cppu::UnoType< css::awt::Rectangle>::get(),
            beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY);
    rProperties.emplace_back(
            "SubController",
            PROPERTY_SUB_CONTROLLER,
            cppu::UnoType<drawing::XDrawSubController>::get(),
            beans::PropertyAttribute::BOUND);
    rProperties.emplace_back(
            "CurrentPage",
            PROPERTY_CURRENTPAGE,
            cppu::UnoType<drawing::XDrawPage>::get(),
            beans::PropertyAttribute::BOUND );
    rProperties.emplace_back("IsLayerMode",
            PROPERTY_LAYERMODE,
            cppu::UnoType<bool>::get(),
            beans::PropertyAttribute::BOUND );
    rProperties.emplace_back("IsMasterPageMode",
            PROPERTY_MASTERPAGEMODE,
            cppu::UnoType<bool>::get(),
            beans::PropertyAttribute::BOUND );
    rProperties.emplace_back("ActiveLayer",
            PROPERTY_ACTIVE_LAYER,
            cppu::UnoType<drawing::XLayer>::get(),
            beans::PropertyAttribute::BOUND );
    rProperties.emplace_back("ZoomValue",
            PROPERTY_ZOOMVALUE,
            ::cppu::UnoType<sal_Int16>::get(),
            beans::PropertyAttribute::BOUND );
    rProperties.emplace_back("ZoomType",
            PROPERTY_ZOOMTYPE,
            ::cppu::UnoType<sal_Int16>::get(),
            beans::PropertyAttribute::BOUND );
    rProperties.emplace_back("ViewOffset",
            PROPERTY_VIEWOFFSET,
            ::cppu::UnoType< css::awt::Point>::get(),
            beans::PropertyAttribute::BOUND );
    rProperties.emplace_back("DrawViewMode",
            PROPERTY_DRAWVIEWMODE,
            ::cppu::UnoType< css::awt::Point>::get(),
            beans::PropertyAttribute::BOUND|beans::PropertyAttribute::READONLY|beans::PropertyAttribute::MAYBEVOID );
    // add new property to update current page's acc information
    rProperties.emplace_back( "UpdateAcc",
            PROPERTY_UPDATEACC,
            ::cppu::UnoType<sal_Int16>::get(),
            beans::PropertyAttribute::BOUND );
    rProperties.emplace_back( "PageChange",
            PROPERTY_PAGE_CHANGE,
            ::cppu::UnoType<sal_Int16>::get(),
            beans::PropertyAttribute::BOUND );
}

IPropertyArrayHelper & DrawController::getInfoHelper()
{
    SolarMutexGuard aGuard;

    if (mpPropertyArrayHelper == nullptr)
    {
        ::std::vector<beans::Property> aProperties;
        FillPropertyTable(aProperties);
        mpPropertyArrayHelper.reset(new OPropertyArrayHelper(comphelper::containerToSequence(aProperties), false));
    }

    return *mpPropertyArrayHelper;
}

Reference < beans::XPropertySetInfo >  DrawController::getPropertySetInfo()
{
    SolarMutexGuard aGuard;

    static Reference < beans::XPropertySetInfo >  xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

uno::Reference< form::runtime::XFormController > SAL_CALL DrawController::getFormController( const uno::Reference< form::XForm >& Form )
{
    SolarMutexGuard aGuard;

    FmFormShell* pFormShell = mpBase->GetFormShellManager()->GetFormShell();
    SdrView* pSdrView = mpBase->GetDrawView();
    std::shared_ptr<ViewShell> pViewShell = mpBase->GetMainViewShell();
    ::sd::Window* pWindow = pViewShell ? pViewShell->GetActiveWindow() : nullptr;

    uno::Reference< form::runtime::XFormController > xController;
    if ( pFormShell && pSdrView && pWindow )
        xController = FmFormShell::GetFormController( Form, *pSdrView, *pWindow->GetOutDev() );
    return xController;
}

sal_Bool SAL_CALL DrawController::isFormDesignMode(  )
{
    SolarMutexGuard aGuard;

    bool bIsDesignMode = true;

    FmFormShell* pFormShell = mpBase->GetFormShellManager()->GetFormShell();
    if ( pFormShell )
        bIsDesignMode = pFormShell->IsDesignMode();

    return bIsDesignMode;
}

void SAL_CALL DrawController::setFormDesignMode( sal_Bool DesignMode )
{
    SolarMutexGuard aGuard;

    FmFormShell* pFormShell = mpBase->GetFormShellManager()->GetFormShell();
    if ( pFormShell )
        pFormShell->SetDesignMode( DesignMode );
}

uno::Reference< awt::XControl > SAL_CALL DrawController::getControl( const uno::Reference< awt::XControlModel >& xModel )
{
    SolarMutexGuard aGuard;

    FmFormShell* pFormShell = mpBase->GetFormShellManager()->GetFormShell();
    SdrView* pSdrView = mpBase->GetDrawView();
    std::shared_ptr<ViewShell> pViewShell = mpBase->GetMainViewShell();
    ::sd::Window* pWindow = pViewShell ? pViewShell->GetActiveWindow() : nullptr;

    uno::Reference< awt::XControl > xControl;
    if ( pFormShell && pSdrView && pWindow )
        pFormShell->GetFormControl( xModel, *pSdrView, *pWindow->GetOutDev(), xControl );
    return xControl;
}

sal_Bool DrawController::convertFastPropertyValue (
    Any & rConvertedValue,
    Any & rOldValue,
    sal_Int32 nHandle,
    const Any& rValue)
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
        mxConfigurationController = new sd::framework::ConfigurationController(this);
        mxModuleController = new sd::framework::ModuleController(this);
    }
    catch (const RuntimeException&)
    {
        mxConfigurationController = nullptr;
        mxModuleController = nullptr;
    }
}

void DrawController::DisposeFrameworkControllers()
{
    if (mxModuleController.is())
        mxModuleController->dispose();

    if (mxConfigurationController.is())
        mxConfigurationController->dispose();
}

void DrawController::ThrowIfDisposed() const
{
    if (rBHelper.bDisposed || rBHelper.bInDispose || mbDisposing)
    {
        SAL_WARN("sd", "Calling disposed DrawController object. Throwing exception.");
        throw lang::DisposedException (
            "DrawController object has already been disposed",
            const_cast<uno::XWeak*>(static_cast<const uno::XWeak*>(this)));
    }
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
