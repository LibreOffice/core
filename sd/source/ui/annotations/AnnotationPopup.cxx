/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/geometry/RealPoint2D.hpp>
#include <com/sun/star/office/XAnnotation.hpp>

#include "AnnotationPopup.hxx"
#include <rtl/ustrbuf.hxx>

#include <utility>
#include <vcl/commandevent.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <vcl/weldutils.hxx>

#include <svx/sdr/overlay/overlayanimatedbitmapex.hxx>
#include <svx/sdr/overlay/overlaybitmapex.hxx>
#include <svx/sdr/overlay/overlaypolypolygon.hxx>
#include <svx/svdpagv.hxx>
#include <svx/sdrpagewindow.hxx>
#include <svx/sdrpaintwindow.hxx>
#include <svx/svddrgmt.hxx>
#include <tools/debug.hxx>
#include <sfx2/objsh.hxx>

#include <View.hxx>
#include <sdresid.hxx>
#include <strings.hrc>
#include "annotationmanagerimpl.hxx"
#include "annotationwindow.hxx"
#include <svx/annotation/Annotation.hxx>
#include <Annotation.hxx>
#include <ViewShell.hxx>
#include <Window.hxx>
#include <drawdoc.hxx>
#include <DrawDocShell.hxx>

using namespace ::com::sun::star;

namespace sd
{
AnnotationPopup::AnnotationPopup(rtl::Reference<sdr::annotation::Annotation> const& xAnnotation)
    : sdr::annotation::IAnnotationPopup(xAnnotation)
{
}

AnnotationPopup::~AnnotationPopup() {}

IMPL_LINK_NOARG(AnnotationPopup, PopupModeEndHdl, weld::Popover&, void) { closePopup(); }

void AnnotationPopup::closePopup()
{
    if (mpAnnotationWindow)
    {
        mpAnnotationWindow->SaveToDocument();
        mpAnnotationWindow.reset();
    }
}

void AnnotationPopup::openPopup()
{
    if (!mxAnnotation.is())
        return;

    sd::DrawDocShell* pDocShell = dynamic_cast<sd::DrawDocShell*>(SfxObjectShell::Current());
    sd::ViewShell* pViewShell = pDocShell ? pDocShell->GetViewShell() : nullptr;

    if (!pViewShell)
        return;

    auto* pView = pViewShell->GetView();
    if (!pView)
        return;

    if (!mpAnnotationWindow)
    {
        OutputDevice* pOut = pView->GetFirstOutputDevice();
        vcl::Window* pWindow = pOut ? pOut->GetOwnerWindow() : nullptr;
        if (pWindow)
        {
            auto aRealPosition2D = mxAnnotation->getPosition();
            Point aPosition(::tools::Long(aRealPosition2D.X * 100.0),
                            ::tools::Long(aRealPosition2D.Y * 100.0));
            Point aPositionPixel = pWindow->LogicToPixel(aPosition);

            aPositionPixel.AdjustX(4);
            aPositionPixel.AdjustY(1);

            auto aRealSize2D = mxAnnotation->getSize();
            Size aSize(::tools::Long(aRealSize2D.Width * 100.0),
                       ::tools::Long(aRealSize2D.Height * 100.0));
            Size aSizePixel = pWindow->LogicToPixel(aSize);

            ::tools::Rectangle aRectangle(aPositionPixel, aSizePixel);

            weld::Window* pParent = weld::GetPopupParent(*pWindow, aRectangle);
            mpAnnotationWindow.reset(
                new AnnotationWindow(pParent, aRectangle, pDocShell, mxAnnotation));
            mpAnnotationWindow->connect_closed(LINK(this, AnnotationPopup, PopupModeEndHdl));
        }
    }

    if (mpAnnotationWindow)
        mpAnnotationWindow->StartEdit();
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
