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
#include <SlideSorter.hxx>
#include <facreg.hxx>
#include <controller/SlideSorterController.hxx>
#include <controller/SlsProperties.hxx>
#include <controller/SlsCurrentSlideManager.hxx>
#include <model/SlideSorterModel.hxx>
#include <model/SlsPageDescriptor.hxx>
#include <view/SlideSorterView.hxx>
#include <view/SlsLayouter.hxx>
#include <DrawController.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <vcl/svapp.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;
using ::sd::slidesorter::view::Layouter;

namespace sd { namespace slidesorter {

//===== SlideSorterService ==========================================================

SlideSorterService::SlideSorterService()
    : SlideSorterServiceInterfaceBase(m_aMutex),
      mpSlideSorter(),
      mxParentWindow()
{
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
{
    ThrowIfDisposed();

    if (rArguments.getLength() != 3)
    {
        throw RuntimeException("SlideSorterService: invalid number of arguments",
            static_cast<drawing::XDrawView*>(this));
    }

    mxViewId.set(rArguments[0], UNO_QUERY_THROW);

    // Get the XController.
    Reference<frame::XController> xController (rArguments[1], UNO_QUERY_THROW);

    // Tunnel through the controller to obtain a ViewShellBase.
    ViewShellBase* pBase = nullptr;
    Reference<lang::XUnoTunnel> xTunnel (xController, UNO_QUERY_THROW);
    ::sd::DrawController* pController = reinterpret_cast<sd::DrawController*>(
        xTunnel->getSomething(sd::DrawController::getUnoTunnelId()));
    if (pController != nullptr)
        pBase = pController->GetViewShellBase();

    // Get the parent window.
    mxParentWindow.set(rArguments[2], UNO_QUERY_THROW);
    VclPtr<vcl::Window> pParentWindow = VCLUnoHelper::GetWindow(mxParentWindow);

    mxParentWindow->addWindowListener(this);

    if (pBase != nullptr && pParentWindow)
        mpSlideSorter = SlideSorter::CreateSlideSorter(
            *pBase,
            *pParentWindow);

    Resize();
}

//----- XView -----------------------------------------------------------------

Reference<XResourceId> SAL_CALL SlideSorterService::getResourceId()
{
    return mxViewId;
}

sal_Bool SAL_CALL SlideSorterService::isAnchorOnly()
{
    return false;
}

//----- XWindowListener -------------------------------------------------------

void SAL_CALL SlideSorterService::windowResized (const awt::WindowEvent&)
{
    ThrowIfDisposed();

    Resize();
}

void SAL_CALL SlideSorterService::windowMoved (const awt::WindowEvent&) {}

void SAL_CALL SlideSorterService::windowShown (const lang::EventObject&)
{
    ThrowIfDisposed();
    Resize();
}

void SAL_CALL SlideSorterService::windowHidden (const lang::EventObject&)
{
    ThrowIfDisposed();
}

//----- lang::XEventListener --------------------------------------------------

void SAL_CALL SlideSorterService::disposing (const lang::EventObject& rEvent)
{
    if (rEvent.Source == mxParentWindow)
        mxParentWindow = nullptr;
}

//----- XDrawView -------------------------------------------------------------

void SAL_CALL SlideSorterService::setCurrentPage(const Reference<drawing::XDrawPage>& rxSlide)
{
    ThrowIfDisposed();
    if (mpSlideSorter != nullptr)
        mpSlideSorter->GetController().GetCurrentSlideManager()->NotifyCurrentSlideChange(
            mpSlideSorter->GetModel().GetIndex(rxSlide));
}

Reference<drawing::XDrawPage> SAL_CALL SlideSorterService::getCurrentPage()
{
    ThrowIfDisposed();
    if (mpSlideSorter != nullptr)
        return mpSlideSorter->GetController().GetCurrentSlideManager()->GetCurrentSlide()->GetXDrawPage();
    else
        return nullptr;
}

//----- attributes ------------------------------------------------------------

Reference<container::XIndexAccess> SAL_CALL SlideSorterService::getDocumentSlides()
{
    return mpSlideSorter->GetModel().GetDocumentSlides();
}

void SAL_CALL SlideSorterService::setDocumentSlides (
    const Reference<container::XIndexAccess >& rxSlides)
{
    ThrowIfDisposed();
    if (mpSlideSorter != nullptr && mpSlideSorter->IsValid())
        mpSlideSorter->GetController().SetDocumentSlides(rxSlides);
}

sal_Bool SAL_CALL SlideSorterService::getIsHighlightCurrentSlide()
{
    ThrowIfDisposed();
    if (mpSlideSorter == nullptr || !mpSlideSorter->IsValid())
        return false;
    else
        return mpSlideSorter->GetProperties()->IsHighlightCurrentSlide();
}

void SAL_CALL SlideSorterService::setIsHighlightCurrentSlide (sal_Bool bValue)
{
    ThrowIfDisposed();
    if (mpSlideSorter != nullptr && mpSlideSorter->IsValid())
    {
        mpSlideSorter->GetProperties()->SetHighlightCurrentSlide(bValue);
        controller::SlideSorterController::ModelChangeLock aLock (mpSlideSorter->GetController());
        mpSlideSorter->GetController().HandleModelChange();
    }
}

sal_Bool SAL_CALL SlideSorterService::getIsShowSelection()
{
    ThrowIfDisposed();
    if (mpSlideSorter == nullptr || !mpSlideSorter->IsValid())
        return false;
    else
        return mpSlideSorter->GetProperties()->IsShowSelection();
}

void SAL_CALL SlideSorterService::setIsShowSelection (sal_Bool bValue)
{
    ThrowIfDisposed();
    if (mpSlideSorter != nullptr && mpSlideSorter->IsValid())
        mpSlideSorter->GetProperties()->SetShowSelection(bValue);
}

sal_Bool SAL_CALL SlideSorterService::getIsShowFocus()
{
    ThrowIfDisposed();
    if (mpSlideSorter == nullptr || !mpSlideSorter->IsValid())
        return false;
    else
        return mpSlideSorter->GetProperties()->IsShowFocus();
}

void SAL_CALL SlideSorterService::setIsShowFocus (sal_Bool bValue)
{
    ThrowIfDisposed();
    if (mpSlideSorter != nullptr && mpSlideSorter->IsValid())
        mpSlideSorter->GetProperties()->SetShowFocus(bValue);
}

sal_Bool SAL_CALL SlideSorterService::getIsCenterSelection()
{
    ThrowIfDisposed();
    if (mpSlideSorter == nullptr || !mpSlideSorter->IsValid())
        return false;
    else
        return mpSlideSorter->GetProperties()->IsCenterSelection();
}

void SAL_CALL SlideSorterService::setIsCenterSelection (sal_Bool bValue)
{
    ThrowIfDisposed();
    if (mpSlideSorter != nullptr && mpSlideSorter->IsValid())
        mpSlideSorter->GetProperties()->SetCenterSelection(bValue);
}

sal_Bool SAL_CALL SlideSorterService::getIsSuspendPreviewUpdatesDuringFullScreenPresentation()
{
    ThrowIfDisposed();
    if (mpSlideSorter == nullptr || !mpSlideSorter->IsValid())
        return true;
    else
        return mpSlideSorter->GetProperties()
            ->IsSuspendPreviewUpdatesDuringFullScreenPresentation();
}

void SAL_CALL SlideSorterService::setIsSuspendPreviewUpdatesDuringFullScreenPresentation (
    sal_Bool bValue)
{
    ThrowIfDisposed();
    if (mpSlideSorter != nullptr && mpSlideSorter->IsValid())
        mpSlideSorter->GetProperties()
            ->SetSuspendPreviewUpdatesDuringFullScreenPresentation(bValue);
}

sal_Bool SAL_CALL SlideSorterService::getIsOrientationVertical()
{
    ThrowIfDisposed();
    if (mpSlideSorter == nullptr || !mpSlideSorter->IsValid())
        return true;
    else
        return mpSlideSorter->GetView().GetOrientation() != Layouter::HORIZONTAL;
}

void SAL_CALL SlideSorterService::setIsOrientationVertical (sal_Bool bValue)
{
    ThrowIfDisposed();
    if (mpSlideSorter != nullptr && mpSlideSorter->IsValid())
        mpSlideSorter->GetView().SetOrientation(bValue
            ? Layouter::GRID
            : Layouter::HORIZONTAL);
}

sal_Bool SAL_CALL SlideSorterService::getIsSmoothScrolling()
{
    ThrowIfDisposed();
    if (mpSlideSorter == nullptr || !mpSlideSorter->IsValid())
        return false;
    else
        return mpSlideSorter->GetProperties()->IsSmoothSelectionScrolling();
}

void SAL_CALL SlideSorterService::setIsSmoothScrolling (sal_Bool bValue)
{
    ThrowIfDisposed();
    if (mpSlideSorter != nullptr && mpSlideSorter->IsValid())
        mpSlideSorter->GetProperties()->SetSmoothSelectionScrolling(bValue);
}

sal_Int32 SAL_CALL SlideSorterService::getBackgroundColor()
{
    ThrowIfDisposed();
    if (mpSlideSorter == nullptr || !mpSlideSorter->IsValid())
        return util::Color();
    else
        return util::Color(
            mpSlideSorter->GetProperties()->GetBackgroundColor());
}

void SAL_CALL SlideSorterService::setBackgroundColor (sal_Int32 aBackgroundColor)
{
    ThrowIfDisposed();
    if (mpSlideSorter != nullptr && mpSlideSorter->IsValid())
        mpSlideSorter->GetProperties()->SetBackgroundColor(Color(aBackgroundColor));
}

sal_Int32 SAL_CALL SlideSorterService::getTextColor()
{
    ThrowIfDisposed();
    if (mpSlideSorter == nullptr || !mpSlideSorter->IsValid())
        return util::Color();
    else
        return util::Color(
            mpSlideSorter->GetProperties()->GetTextColor());
}

void SAL_CALL SlideSorterService::setTextColor (sal_Int32 aTextColor)
{
    ThrowIfDisposed();
    if (mpSlideSorter != nullptr && mpSlideSorter->IsValid())
        mpSlideSorter->GetProperties()->SetTextColor(Color(aTextColor));
}

sal_Int32 SAL_CALL SlideSorterService::getSelectionColor()
{
    ThrowIfDisposed();
    if (mpSlideSorter == nullptr || !mpSlideSorter->IsValid())
        return util::Color();
    else
        return util::Color(
            mpSlideSorter->GetProperties()->GetSelectionColor());
}

void SAL_CALL SlideSorterService::setSelectionColor (sal_Int32 aSelectionColor)
{
    ThrowIfDisposed();
    if (mpSlideSorter != nullptr && mpSlideSorter->IsValid())
        mpSlideSorter->GetProperties()->SetSelectionColor(Color(aSelectionColor));
}

sal_Int32 SAL_CALL SlideSorterService::getHighlightColor()
{
    ThrowIfDisposed();
    if (mpSlideSorter == nullptr || !mpSlideSorter->IsValid())
        return util::Color();
    else
        return util::Color(
            mpSlideSorter->GetProperties()->GetHighlightColor());
}

void SAL_CALL SlideSorterService::setHighlightColor (sal_Int32 aHighlightColor)
{
    ThrowIfDisposed();
    if (mpSlideSorter != nullptr && mpSlideSorter->IsValid())
        mpSlideSorter->GetProperties()->SetHighlightColor(Color(aHighlightColor));
}

sal_Bool SAL_CALL SlideSorterService::getIsUIReadOnly()
{
    ThrowIfDisposed();
    if (mpSlideSorter == nullptr || !mpSlideSorter->IsValid())
        return true;
    else
        return mpSlideSorter->GetProperties()->IsUIReadOnly();
}

void SAL_CALL SlideSorterService::setIsUIReadOnly (sal_Bool bIsUIReadOnly)
{
    ThrowIfDisposed();
    if (mpSlideSorter != nullptr && mpSlideSorter->IsValid())
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
{
    if (SlideSorterServiceInterfaceBase::rBHelper.bDisposed || SlideSorterServiceInterfaceBase::rBHelper.bInDispose)
    {
        throw lang::DisposedException ("SlideSorterService object has already been disposed",
            static_cast<drawing::XDrawView*>(this));
    }
}

} } // end of namespace ::sd::slidesorter


extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_Draw_SlideSorter_get_implementation(css::uno::XComponentContext* /*context*/,
                                                      css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new sd::slidesorter::SlideSorterService);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
