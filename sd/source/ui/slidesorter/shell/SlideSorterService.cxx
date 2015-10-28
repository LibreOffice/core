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

#include "SlideSorterService.hxx"
#include "facreg.hxx"
#include "controller/SlideSorterController.hxx"
#include "controller/SlsProperties.hxx"
#include "controller/SlsCurrentSlideManager.hxx"
#include "model/SlideSorterModel.hxx"
#include "model/SlsPageDescriptor.hxx"
#include "view/SlideSorterView.hxx"
#include "view/SlsLayouter.hxx"
#include "DrawController.hxx"
#include <toolkit/helper/vclunohelper.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <cppuhelper/proptypehlp.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;
using ::sd::slidesorter::view::Layouter;

namespace sd { namespace slidesorter {

namespace {
    enum Properties
    {
        PropertyDocumentSlides,
        PropertyHighlightCurrentSlide,
        PropertyShowSelection,
        PropertyCenterSelection,
        PropertySuspendPreviewUpdatesDuringFullScreenPresentation,
        PropertyOrientationVertical
    };
}

//===== SlideSorterService ==========================================================

SlideSorterService::SlideSorterService (const Reference<XComponentContext>& rxContext)
    : SlideSorterServiceInterfaceBase(m_aMutex),
      mpSlideSorter(),
      mxParentWindow()
{
    (void)rxContext;
}

SlideSorterService::~SlideSorterService()
{
}

void SAL_CALL SlideSorterService::disposing()
{
    mpSlideSorter.reset();

    if (mxParentWindow.is())
    {
        mxParentWindow->removeWindowListener(this);
    }
}

//----- XInitialization -------------------------------------------------------

void SAL_CALL SlideSorterService::initialize (const Sequence<Any>& rArguments)
    throw (Exception, RuntimeException, std::exception)
{
    ThrowIfDisposed();

    if (rArguments.getLength() == 3)
    {
        try
        {
            mxViewId = Reference<XResourceId>(rArguments[0], UNO_QUERY_THROW);

            // Get the XController.
            Reference<frame::XController> xController (rArguments[1], UNO_QUERY_THROW);

            // Tunnel through the controller to obtain a ViewShellBase.
            ViewShellBase* pBase = NULL;
            Reference<lang::XUnoTunnel> xTunnel (xController, UNO_QUERY_THROW);
            ::sd::DrawController* pController = reinterpret_cast<sd::DrawController*>(
                xTunnel->getSomething(sd::DrawController::getUnoTunnelId()));
            if (pController != NULL)
                pBase = pController->GetViewShellBase();

            // Get the parent window.
            mxParentWindow = Reference<awt::XWindow>(rArguments[2], UNO_QUERY_THROW);
            vcl::Window* pParentWindow = VCLUnoHelper::GetWindow(mxParentWindow);

            mxParentWindow->addWindowListener(this);

            if (pBase != NULL && pParentWindow!=NULL)
                mpSlideSorter = SlideSorter::CreateSlideSorter(
                    *pBase,
                    NULL,
                    *pParentWindow);

            Resize();
        }
        catch (RuntimeException&)
        {
            throw;
        }
    }
    else
    {
        throw RuntimeException("SlideSorterService: invalid number of arguments",
            static_cast<drawing::XDrawView*>(this));
    }
}

//----- XView -----------------------------------------------------------------

Reference<XResourceId> SAL_CALL SlideSorterService::getResourceId()
    throw (RuntimeException, std::exception)
{
    return mxViewId;
}

sal_Bool SAL_CALL SlideSorterService::isAnchorOnly()
    throw (RuntimeException, std::exception)
{
    return sal_False;
}

//----- XWindowListener -------------------------------------------------------

void SAL_CALL SlideSorterService::windowResized (const awt::WindowEvent& rEvent)
    throw (RuntimeException, std::exception)
{
    (void)rEvent;
    ThrowIfDisposed();

    Resize();
}

void SAL_CALL SlideSorterService::windowMoved (const awt::WindowEvent& rEvent)
    throw (RuntimeException, std::exception)
{
    (void)rEvent;
}

void SAL_CALL SlideSorterService::windowShown (const lang::EventObject& rEvent)
    throw (RuntimeException, std::exception)
{
    (void)rEvent;
    ThrowIfDisposed();
    Resize();
}

void SAL_CALL SlideSorterService::windowHidden (const lang::EventObject& rEvent)
    throw (RuntimeException, std::exception)
{
    (void)rEvent;
    ThrowIfDisposed();
}

//----- lang::XEventListener --------------------------------------------------

void SAL_CALL SlideSorterService::disposing (const lang::EventObject& rEvent)
    throw (RuntimeException, std::exception)
{
    if (rEvent.Source == mxParentWindow)
        mxParentWindow = NULL;
}

//----- XDrawView -------------------------------------------------------------

void SAL_CALL SlideSorterService::setCurrentPage(const Reference<drawing::XDrawPage>& rxSlide)
    throw (RuntimeException, std::exception)
{
    ThrowIfDisposed();
    if (mpSlideSorter.get() != NULL)
        mpSlideSorter->GetController().GetCurrentSlideManager()->NotifyCurrentSlideChange(
            mpSlideSorter->GetModel().GetIndex(rxSlide));
}

Reference<drawing::XDrawPage> SAL_CALL SlideSorterService::getCurrentPage()
    throw (RuntimeException, std::exception)
{
    ThrowIfDisposed();
    if (mpSlideSorter.get() != NULL)
        return mpSlideSorter->GetController().GetCurrentSlideManager()->GetCurrentSlide()->GetXDrawPage();
    else
        return NULL;
}

//----- attributes ------------------------------------------------------------

Reference<container::XIndexAccess> SAL_CALL SlideSorterService::getDocumentSlides()
    throw (RuntimeException, std::exception)
{
    return mpSlideSorter->GetModel().GetDocumentSlides();
}

void SAL_CALL SlideSorterService::setDocumentSlides (
    const Reference<container::XIndexAccess >& rxSlides)
    throw (RuntimeException, std::exception)
{
    ThrowIfDisposed();
    if (mpSlideSorter.get() != NULL && mpSlideSorter->IsValid())
        mpSlideSorter->GetController().SetDocumentSlides(rxSlides);
}

sal_Bool SAL_CALL SlideSorterService::getIsHighlightCurrentSlide()
    throw (RuntimeException, std::exception)
{
    ThrowIfDisposed();
    if (mpSlideSorter.get() == NULL || ! mpSlideSorter->IsValid())
        return false;
    else
        return mpSlideSorter->GetProperties()->IsHighlightCurrentSlide();
}

void SAL_CALL SlideSorterService::setIsHighlightCurrentSlide (sal_Bool bValue)
    throw (RuntimeException, std::exception)
{
    ThrowIfDisposed();
    if (mpSlideSorter.get() != NULL && mpSlideSorter->IsValid())
    {
        mpSlideSorter->GetProperties()->SetHighlightCurrentSlide(bValue);
        controller::SlideSorterController::ModelChangeLock aLock (mpSlideSorter->GetController());
        mpSlideSorter->GetController().HandleModelChange();
    }
}

sal_Bool SAL_CALL SlideSorterService::getIsShowSelection()
    throw (RuntimeException, std::exception)
{
    ThrowIfDisposed();
    if (mpSlideSorter.get() == NULL || ! mpSlideSorter->IsValid())
        return false;
    else
        return mpSlideSorter->GetProperties()->IsShowSelection();
}

void SAL_CALL SlideSorterService::setIsShowSelection (sal_Bool bValue)
    throw (RuntimeException, std::exception)
{
    ThrowIfDisposed();
    if (mpSlideSorter.get() != NULL && mpSlideSorter->IsValid())
        mpSlideSorter->GetProperties()->SetShowSelection(bValue);
}

sal_Bool SAL_CALL SlideSorterService::getIsShowFocus()
    throw (RuntimeException, std::exception)
{
    ThrowIfDisposed();
    if (mpSlideSorter.get() == NULL || ! mpSlideSorter->IsValid())
        return false;
    else
        return mpSlideSorter->GetProperties()->IsShowFocus();
}

void SAL_CALL SlideSorterService::setIsShowFocus (sal_Bool bValue)
    throw (RuntimeException, std::exception)
{
    ThrowIfDisposed();
    if (mpSlideSorter.get() != NULL && mpSlideSorter->IsValid())
        mpSlideSorter->GetProperties()->SetShowFocus(bValue);
}

sal_Bool SAL_CALL SlideSorterService::getIsCenterSelection()
    throw (RuntimeException, std::exception)
{
    ThrowIfDisposed();
    if (mpSlideSorter.get() == NULL || ! mpSlideSorter->IsValid())
        return false;
    else
        return mpSlideSorter->GetProperties()->IsCenterSelection();
}

void SAL_CALL SlideSorterService::setIsCenterSelection (sal_Bool bValue)
    throw (RuntimeException, std::exception)
{
    ThrowIfDisposed();
    if (mpSlideSorter.get() != NULL && mpSlideSorter->IsValid())
        mpSlideSorter->GetProperties()->SetCenterSelection(bValue);
}

sal_Bool SAL_CALL SlideSorterService::getIsSuspendPreviewUpdatesDuringFullScreenPresentation()
    throw (RuntimeException, std::exception)
{
    ThrowIfDisposed();
    if (mpSlideSorter.get() == NULL || ! mpSlideSorter->IsValid())
        return true;
    else
        return mpSlideSorter->GetProperties()
            ->IsSuspendPreviewUpdatesDuringFullScreenPresentation();
}

void SAL_CALL SlideSorterService::setIsSuspendPreviewUpdatesDuringFullScreenPresentation (
    sal_Bool bValue)
    throw (RuntimeException, std::exception)
{
    ThrowIfDisposed();
    if (mpSlideSorter.get() != NULL && mpSlideSorter->IsValid())
        mpSlideSorter->GetProperties()
            ->SetSuspendPreviewUpdatesDuringFullScreenPresentation(bValue);
}

sal_Bool SAL_CALL SlideSorterService::getIsOrientationVertical()
    throw (RuntimeException, std::exception)
{
    ThrowIfDisposed();
    if (mpSlideSorter.get() == NULL || ! mpSlideSorter->IsValid())
        return true;
    else
        return mpSlideSorter->GetView().GetOrientation() != Layouter::HORIZONTAL;
}

void SAL_CALL SlideSorterService::setIsOrientationVertical (sal_Bool bValue)
    throw (RuntimeException, std::exception)
{
    ThrowIfDisposed();
    if (mpSlideSorter.get() != NULL && mpSlideSorter->IsValid())
        mpSlideSorter->GetView().SetOrientation(bValue
            ? Layouter::GRID
            : Layouter::HORIZONTAL);
}

sal_Bool SAL_CALL SlideSorterService::getIsSmoothScrolling()
    throw (RuntimeException, std::exception)
{
    ThrowIfDisposed();
    if (mpSlideSorter.get() == NULL || ! mpSlideSorter->IsValid())
        return false;
    else
        return mpSlideSorter->GetProperties()->IsSmoothSelectionScrolling();
}

void SAL_CALL SlideSorterService::setIsSmoothScrolling (sal_Bool bValue)
    throw (RuntimeException, std::exception)
{
    ThrowIfDisposed();
    if (mpSlideSorter.get() != NULL && mpSlideSorter->IsValid())
        mpSlideSorter->GetProperties()->SetSmoothSelectionScrolling(bValue);
}

util::Color SAL_CALL SlideSorterService::getBackgroundColor()
    throw (RuntimeException, std::exception)
{
    ThrowIfDisposed();
    if (mpSlideSorter.get() == NULL || ! mpSlideSorter->IsValid())
        return util::Color();
    else
        return util::Color(
            mpSlideSorter->GetProperties()->GetBackgroundColor().GetColor());
}

void SAL_CALL SlideSorterService::setBackgroundColor (util::Color aBackgroundColor)
    throw (RuntimeException, std::exception)
{
    ThrowIfDisposed();
    if (mpSlideSorter.get() != NULL && mpSlideSorter->IsValid())
        mpSlideSorter->GetProperties()->SetBackgroundColor(Color(aBackgroundColor));
}

util::Color SAL_CALL SlideSorterService::getTextColor()
    throw (css::uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    if (mpSlideSorter.get() == NULL || ! mpSlideSorter->IsValid())
        return util::Color();
    else
        return util::Color(
            mpSlideSorter->GetProperties()->GetTextColor().GetColor());
}

void SAL_CALL SlideSorterService::setTextColor (util::Color aTextColor)
    throw (RuntimeException, std::exception)
{
    ThrowIfDisposed();
    if (mpSlideSorter.get() != NULL && mpSlideSorter->IsValid())
        mpSlideSorter->GetProperties()->SetTextColor(Color(aTextColor));
}

util::Color SAL_CALL SlideSorterService::getSelectionColor()
    throw (RuntimeException, std::exception)
{
    ThrowIfDisposed();
    if (mpSlideSorter.get() == NULL || ! mpSlideSorter->IsValid())
        return util::Color();
    else
        return util::Color(
            mpSlideSorter->GetProperties()->GetSelectionColor().GetColor());
}

void SAL_CALL SlideSorterService::setSelectionColor (util::Color aSelectionColor)
    throw (RuntimeException, std::exception)
{
    ThrowIfDisposed();
    if (mpSlideSorter.get() != NULL && mpSlideSorter->IsValid())
        mpSlideSorter->GetProperties()->SetSelectionColor(Color(aSelectionColor));
}

util::Color SAL_CALL SlideSorterService::getHighlightColor()
    throw (RuntimeException, std::exception)
{
    ThrowIfDisposed();
    if (mpSlideSorter.get() == NULL || ! mpSlideSorter->IsValid())
        return util::Color();
    else
        return util::Color(
            mpSlideSorter->GetProperties()->GetHighlightColor().GetColor());
}

void SAL_CALL SlideSorterService::setHighlightColor (util::Color aHighlightColor)
    throw (RuntimeException, std::exception)
{
    ThrowIfDisposed();
    if (mpSlideSorter.get() != NULL && mpSlideSorter->IsValid())
        mpSlideSorter->GetProperties()->SetHighlightColor(Color(aHighlightColor));
}

sal_Bool SAL_CALL SlideSorterService::getIsUIReadOnly()
    throw (RuntimeException, std::exception)
{
    ThrowIfDisposed();
    if (mpSlideSorter.get() == NULL || ! mpSlideSorter->IsValid())
        return true;
    else
        return mpSlideSorter->GetProperties()->IsUIReadOnly();
}

void SAL_CALL SlideSorterService::setIsUIReadOnly (sal_Bool bIsUIReadOnly)
    throw (RuntimeException, std::exception)
{
    ThrowIfDisposed();
    if (mpSlideSorter.get() != NULL && mpSlideSorter->IsValid())
        mpSlideSorter->GetProperties()->SetUIReadOnly(bIsUIReadOnly);
}

void SlideSorterService::Resize()
{
    if (mxParentWindow.is())
    {
        awt::Rectangle aWindowBox = mxParentWindow->getPosSize();
        mpSlideSorter->ArrangeGUIElements(
            Point(0,0),
            Size(aWindowBox.Width, aWindowBox.Height));
    }
}

void SlideSorterService::ThrowIfDisposed()
    throw (css::lang::DisposedException)
{
    if (SlideSorterServiceInterfaceBase::rBHelper.bDisposed || SlideSorterServiceInterfaceBase::rBHelper.bInDispose)
    {
        throw lang::DisposedException ("SlideSorterService object has already been disposed",
            static_cast<drawing::XDrawView*>(this));
    }
}

} } // end of namespace ::sd::slidesorter


extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface* SAL_CALL
com_sun_star_comp_Draw_SlideSorter_get_implementation(css::uno::XComponentContext* context,
                                                      css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new sd::slidesorter::SlideSorterService(context));
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
