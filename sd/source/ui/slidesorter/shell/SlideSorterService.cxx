/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: SlideSorterService.cxx,v $
 *
 * $Revision: 1.3 $
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

#include "precompiled_sd.hxx"

#include "SlideSorterService.hxx"
#include "controller/SlideSorterController.hxx"
#include "controller/SlsProperties.hxx"
#include "controller/SlsCurrentSlideManager.hxx"
#include "model/SlideSorterModel.hxx"
#include "model/SlsPageDescriptor.hxx"
#include "view/SlideSorterView.hxx"
#include "DrawController.hxx"
#include <toolkit/helper/vclunohelper.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <vos/mutex.hxx>
#include <vcl/svapp.hxx>
#include <cppuhelper/proptypehlp.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;
using ::rtl::OUString;
using ::sd::slidesorter::view::SlideSorterView;

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




//===== Service ===============================================================

Reference<XInterface> SAL_CALL SlideSorterService_createInstance (
    const Reference<XComponentContext>& rxContext)
{
    return Reference<XInterface>(static_cast<drawing::XDrawView*>(new SlideSorterService(rxContext)));
}




::rtl::OUString SlideSorterService_getImplementationName (void) throw(RuntimeException)
{
    return OUString::createFromAscii("com.sun.star.comp.Draw.SlideSorter");
}




Sequence<rtl::OUString> SAL_CALL SlideSorterService_getSupportedServiceNames (void)
    throw (RuntimeException)
{
    static const ::rtl::OUString sServiceName(
        ::rtl::OUString::createFromAscii("com.sun.star.drawing.SlideSorter"));
    return Sequence<rtl::OUString>(&sServiceName, 1);
}




//===== SlideSorterService ==========================================================

SlideSorterService::SlideSorterService (const Reference<XComponentContext>& rxContext)
    : SlideSorterServiceInterfaceBase(m_aMutex),
      mpSlideSorter(),
      mxParentWindow()
{
    (void)rxContext;
}




SlideSorterService::~SlideSorterService (void)
{
}




void SAL_CALL SlideSorterService::disposing (void)
{
    mpSlideSorter.reset();

    if (mxParentWindow.is())
    {
        mxParentWindow->removeWindowListener(this);
    }
}




//----- XInitialization -------------------------------------------------------

void SAL_CALL SlideSorterService::initialize (const Sequence<Any>& rArguments)
    throw (Exception, RuntimeException)
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
            ::Window* pParentWindow = VCLUnoHelper::GetWindow(mxParentWindow);

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
        throw RuntimeException(
            OUString::createFromAscii("SlideSorterService: invalid number of arguments"),
            static_cast<drawing::XDrawView*>(this));
    }
}




//----- XView -----------------------------------------------------------------

Reference<XResourceId> SAL_CALL SlideSorterService::getResourceId (void)
    throw (RuntimeException)
{
    return mxViewId;
}




sal_Bool SAL_CALL SlideSorterService::isAnchorOnly (void)
    throw (RuntimeException)
{
    return sal_False;
}




//----- XWindowListener -------------------------------------------------------

void SAL_CALL SlideSorterService::windowResized (const awt::WindowEvent& rEvent)
    throw (RuntimeException)
{
    (void)rEvent;
    ThrowIfDisposed();

    Resize();
}





void SAL_CALL SlideSorterService::windowMoved (const awt::WindowEvent& rEvent)
    throw (RuntimeException)
{
    (void)rEvent;
}




void SAL_CALL SlideSorterService::windowShown (const lang::EventObject& rEvent)
    throw (RuntimeException)
{
    (void)rEvent;
    ThrowIfDisposed();
    Resize();
}




void SAL_CALL SlideSorterService::windowHidden (const lang::EventObject& rEvent)
    throw (RuntimeException)
{
    (void)rEvent;
    ThrowIfDisposed();
}




//----- lang::XEventListener --------------------------------------------------

void SAL_CALL SlideSorterService::disposing (const lang::EventObject& rEvent)
    throw (RuntimeException)
{
    if (rEvent.Source == mxParentWindow)
        mxParentWindow = NULL;
}




//----- XDrawView -------------------------------------------------------------

void SAL_CALL SlideSorterService::setCurrentPage(const Reference<drawing::XDrawPage>& rxSlide)
    throw (RuntimeException)
{
    ThrowIfDisposed();
    if (mpSlideSorter.get() != NULL)
        mpSlideSorter->GetController().GetCurrentSlideManager()->CurrentSlideHasChanged(
            mpSlideSorter->GetModel().GetIndex(rxSlide));
}




Reference<drawing::XDrawPage> SAL_CALL SlideSorterService::getCurrentPage (void)
    throw (RuntimeException)
{
    ThrowIfDisposed();
    if (mpSlideSorter.get() != NULL)
        return mpSlideSorter->GetController().GetCurrentSlideManager()->GetCurrentSlide()->GetXDrawPage();
    else
        return NULL;
}




//----- attributes ------------------------------------------------------------


Reference<container::XIndexAccess> SAL_CALL SlideSorterService::getDocumentSlides (void)
    throw (RuntimeException)
{
    return mpSlideSorter->GetModel().GetDocumentSlides();
}




void SAL_CALL SlideSorterService::setDocumentSlides (
    const Reference<container::XIndexAccess >& rxSlides)
    throw (RuntimeException)
{
    ThrowIfDisposed();
    if (mpSlideSorter.get() != NULL && mpSlideSorter->IsValid())
        mpSlideSorter->GetController().SetDocumentSlides(rxSlides);
}




sal_Bool SAL_CALL SlideSorterService::getIsHighlightCurrentSlide (void)
    throw (RuntimeException)
{
    ThrowIfDisposed();
    if (mpSlideSorter.get() == NULL || ! mpSlideSorter->IsValid())
        return false;
    else
        return mpSlideSorter->GetController().GetProperties()->IsHighlightCurrentSlide();
}




void SAL_CALL SlideSorterService::setIsHighlightCurrentSlide (sal_Bool bValue)
    throw (RuntimeException)
{
    ThrowIfDisposed();
    if (mpSlideSorter.get() != NULL && mpSlideSorter->IsValid())
    {
        mpSlideSorter->GetController().GetProperties()->SetHighlightCurrentSlide(bValue);
        controller::SlideSorterController::ModelChangeLock aLock (mpSlideSorter->GetController());
        mpSlideSorter->GetController().HandleModelChange();
    }
}




sal_Bool SAL_CALL SlideSorterService::getIsShowSelection (void)
    throw (RuntimeException)
{
    ThrowIfDisposed();
    if (mpSlideSorter.get() == NULL || ! mpSlideSorter->IsValid())
        return false;
    else
        return mpSlideSorter->GetController().GetProperties()->IsShowSelection();
}




void SAL_CALL SlideSorterService::setIsShowSelection (sal_Bool bValue)
    throw (RuntimeException)
{
    ThrowIfDisposed();
    if (mpSlideSorter.get() != NULL && mpSlideSorter->IsValid())
        mpSlideSorter->GetController().GetProperties()->SetShowSelection(bValue);
}




sal_Bool SAL_CALL SlideSorterService::getIsShowFocus (void)
    throw (RuntimeException)
{
    ThrowIfDisposed();
    if (mpSlideSorter.get() == NULL || ! mpSlideSorter->IsValid())
        return false;
    else
        return mpSlideSorter->GetController().GetProperties()->IsShowFocus();
}




void SAL_CALL SlideSorterService::setIsShowFocus (sal_Bool bValue)
    throw (RuntimeException)
{
    ThrowIfDisposed();
    if (mpSlideSorter.get() != NULL && mpSlideSorter->IsValid())
        mpSlideSorter->GetController().GetProperties()->SetShowFocus(bValue);
}




sal_Bool SAL_CALL SlideSorterService::getIsCenterSelection (void)
    throw (RuntimeException)
{
    ThrowIfDisposed();
    if (mpSlideSorter.get() == NULL || ! mpSlideSorter->IsValid())
        return false;
    else
        return mpSlideSorter->GetController().GetProperties()->IsCenterSelection();
}




void SAL_CALL SlideSorterService::setIsCenterSelection (sal_Bool bValue)
    throw (RuntimeException)
{
    ThrowIfDisposed();
    if (mpSlideSorter.get() != NULL && mpSlideSorter->IsValid())
        mpSlideSorter->GetController().GetProperties()->SetCenterSelection(bValue);
}




sal_Bool SAL_CALL SlideSorterService::getIsSuspendPreviewUpdatesDuringFullScreenPresentation (void)
    throw (RuntimeException)
{
    ThrowIfDisposed();
    if (mpSlideSorter.get() == NULL || ! mpSlideSorter->IsValid())
        return true;
    else
        return mpSlideSorter->GetController().GetProperties()
            ->IsSuspendPreviewUpdatesDuringFullScreenPresentation();
}




void SAL_CALL SlideSorterService::setIsSuspendPreviewUpdatesDuringFullScreenPresentation (
    sal_Bool bValue)
    throw (RuntimeException)
{
    ThrowIfDisposed();
    if (mpSlideSorter.get() != NULL && mpSlideSorter->IsValid())
        mpSlideSorter->GetController().GetProperties()
            ->SetSuspendPreviewUpdatesDuringFullScreenPresentation(bValue);
}




sal_Bool SAL_CALL SlideSorterService::getIsOrientationVertical (void)
    throw (RuntimeException)
{
    ThrowIfDisposed();
    if (mpSlideSorter.get() == NULL || ! mpSlideSorter->IsValid())
        return true;
    else
        return mpSlideSorter->GetView().GetOrientation() == SlideSorterView::VERTICAL;
}




void SAL_CALL SlideSorterService::setIsOrientationVertical (sal_Bool bValue)
    throw (RuntimeException)
{
    ThrowIfDisposed();
    if (mpSlideSorter.get() != NULL && mpSlideSorter->IsValid())
        mpSlideSorter->GetView().SetOrientation(bValue
            ? SlideSorterView::VERTICAL
            : SlideSorterView::HORIZONTAL);
}




sal_Bool SAL_CALL SlideSorterService::getIsSmoothScrolling (void)
    throw (RuntimeException)
{
    ThrowIfDisposed();
    if (mpSlideSorter.get() == NULL || ! mpSlideSorter->IsValid())
        return false;
    else
        return mpSlideSorter->GetController().GetProperties()->IsSmoothSelectionScrolling();
}




void SAL_CALL SlideSorterService::setIsSmoothScrolling (sal_Bool bValue)
    throw (RuntimeException)
{
    ThrowIfDisposed();
    if (mpSlideSorter.get() != NULL && mpSlideSorter->IsValid())
        mpSlideSorter->GetController().GetProperties()->SetSmoothSelectionScrolling(bValue);
}




util::Color SAL_CALL SlideSorterService::getBackgroundColor (void)
    throw (RuntimeException)
{
    ThrowIfDisposed();
    if (mpSlideSorter.get() == NULL || ! mpSlideSorter->IsValid())
        return util::Color();
    else
        return util::Color(
            mpSlideSorter->GetController().GetProperties()->GetBackgroundColor().GetColor());
}




void SAL_CALL SlideSorterService::setBackgroundColor (util::Color aBackgroundColor)
    throw (RuntimeException)
{
    ThrowIfDisposed();
    if (mpSlideSorter.get() != NULL && mpSlideSorter->IsValid())
        mpSlideSorter->GetController().GetProperties()->SetBackgroundColor(
            Color(aBackgroundColor));
}




util::Color SAL_CALL SlideSorterService::getTextColor (void)
    throw (css::uno::RuntimeException)
{
    ThrowIfDisposed();
    if (mpSlideSorter.get() == NULL || ! mpSlideSorter->IsValid())
        return util::Color();
    else
        return util::Color(
            mpSlideSorter->GetController().GetProperties()->GetTextColor().GetColor());
}




void SAL_CALL SlideSorterService::setTextColor (util::Color aTextColor)
    throw (RuntimeException)
{
    ThrowIfDisposed();
    if (mpSlideSorter.get() != NULL && mpSlideSorter->IsValid())
        mpSlideSorter->GetController().GetProperties()->SetTextColor(
            Color(aTextColor));
}




util::Color SAL_CALL SlideSorterService::getSelectionColor (void)
    throw (RuntimeException)
{
    ThrowIfDisposed();
    if (mpSlideSorter.get() == NULL || ! mpSlideSorter->IsValid())
        return util::Color();
    else
        return util::Color(
            mpSlideSorter->GetController().GetProperties()->GetSelectionColor().GetColor());
}




void SAL_CALL SlideSorterService::setSelectionColor (util::Color aSelectionColor)
    throw (RuntimeException)
{
    ThrowIfDisposed();
    if (mpSlideSorter.get() != NULL && mpSlideSorter->IsValid())
        mpSlideSorter->GetController().GetProperties()->SetSelectionColor(
            Color(aSelectionColor));
}




util::Color SAL_CALL SlideSorterService::getHighlightColor (void)
    throw (RuntimeException)
{
    ThrowIfDisposed();
    if (mpSlideSorter.get() == NULL || ! mpSlideSorter->IsValid())
        return util::Color();
    else
        return util::Color(
            mpSlideSorter->GetController().GetProperties()->GetHighlightColor().GetColor());
}




void SAL_CALL SlideSorterService::setHighlightColor (util::Color aHighlightColor)
    throw (RuntimeException)
{
    ThrowIfDisposed();
    if (mpSlideSorter.get() != NULL && mpSlideSorter->IsValid())
        mpSlideSorter->GetController().GetProperties()->SetHighlightColor(
            Color(aHighlightColor));
}



sal_Bool SAL_CALL SlideSorterService::getIsUIReadOnly (void)
    throw (RuntimeException)
{
    ThrowIfDisposed();
    if (mpSlideSorter.get() == NULL || ! mpSlideSorter->IsValid())
        return true;
    else
        return mpSlideSorter->GetController().GetProperties()->IsUIReadOnly();
}




void SAL_CALL SlideSorterService::setIsUIReadOnly (sal_Bool bIsUIReadOnly)
    throw (RuntimeException)
{
    ThrowIfDisposed();
    if (mpSlideSorter.get() != NULL && mpSlideSorter->IsValid())
        mpSlideSorter->GetController().GetProperties()->SetUIReadOnly(
            bIsUIReadOnly);
}




//-----------------------------------------------------------------------------

void SlideSorterService::Resize (void)
{
    if (mxParentWindow.is())
    {
        awt::Rectangle aWindowBox = mxParentWindow->getPosSize();
        mpSlideSorter->ArrangeGUIElements(
            Point(0,0),
            Size(aWindowBox.Width, aWindowBox.Height));
    }
}




void SlideSorterService::Rearrange (void)
{
    if (mxParentWindow.is())
    {
        awt::Rectangle aWindowBox = mxParentWindow->getPosSize();
        mpSlideSorter->GetController().Rearrange();
    }
}




void SlideSorterService::ThrowIfDisposed (void)
    throw (::com::sun::star::lang::DisposedException)
{
    if (SlideSorterServiceInterfaceBase::rBHelper.bDisposed || SlideSorterServiceInterfaceBase::rBHelper.bInDispose)
    {
        throw lang::DisposedException (
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                "SlideSorterService object has already been disposed")),
            static_cast<drawing::XDrawView*>(this));
    }
}


} } // end of namespace ::sd::presenter

