/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <PreviewEditMode.hxx>

#include <comphelper/kit.hxx>
#include <o3tl/unit_conversion.hxx>
#include <sfx2/viewsh.hxx>
#include <svx/svdopage.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdpage.hxx>
#include <tools/json_writer.hxx>

#include <DrawViewShell.hxx>
#include <drawdoc.hxx>

namespace sd
{
PreviewEditMode::PreviewEditMode(DrawViewShell& rShell, SdrPageObj& rPreview)
    : mrShell(rShell)
    , mxPreview(&rPreview)
{
    StartListening(rPreview.getSdrModelFromSdrObject());
    SendModeState(true);
    // A fresh mode has no shape selected, so the view starts from an empty selection state.
    SendSelectionState();
}

PreviewEditMode::~PreviewEditMode() { EndListeningAll(); }

SdrPageObj* PreviewEditMode::GetEnteredPreview() const
{
    return static_cast<SdrPageObj*>(mxPreview.get().get());
}

SdrObject* PreviewEditMode::GetSelectedShape() const { return mxSelectedShape.get().get(); }

void PreviewEditMode::SetSelectedShape(SdrObject* pShape)
{
    mxSelectedShape = pShape;
    SendSelectionState();
}

void PreviewEditMode::SendModeState(bool bEntered) const
{
    SfxViewShell* pViewShell = mrShell.GetViewShell();
    if (!pViewShell || !comphelper::COKit::isActive())
        return;
    ::tools::JsonWriter aJsonWriter;
    aJsonWriter.put("commandName", ".uno:PreviewEditMode");
    {
        auto aState = aJsonWriter.startNode("state");
        SdrPageObj* pPreview = GetEnteredPreview();
        if (bEntered && pPreview && pPreview->GetReferencedPage())
        {
            const ::tools::Rectangle aRectangle = pPreview->GetSnapRect();
            const SdrPage* pSlide = pPreview->GetReferencedPage();
            aJsonWriter.put("active", true);
            aJsonWriter.put("part",
                            static_cast<sal_Int32>((pSlide->GetPageNum() - 1) / 2));
            aJsonWriter.put("x", o3tl::toTwips(aRectangle.Left(), o3tl::Length::mm100));
            aJsonWriter.put("y", o3tl::toTwips(aRectangle.Top(), o3tl::Length::mm100));
            aJsonWriter.put("width", o3tl::toTwips(aRectangle.GetWidth(), o3tl::Length::mm100));
            aJsonWriter.put("height",
                            o3tl::toTwips(aRectangle.GetHeight(), o3tl::Length::mm100));
            aJsonWriter.put("slideWidth", o3tl::toTwips(pSlide->GetWidth(), o3tl::Length::mm100));
            aJsonWriter.put("slideHeight",
                            o3tl::toTwips(pSlide->GetHeight(), o3tl::Length::mm100));
        }
        else
        {
            aJsonWriter.put("active", false);
        }
    }
    pViewShell->viewCallback(COKitCallbackType::STATE_CHANGED,
                             aJsonWriter.finishAndGetAsOString());
}

void PreviewEditMode::SendSelectionState() const
{
    SfxViewShell* pViewShell = mrShell.GetViewShell();
    if (!pViewShell || !comphelper::COKit::isActive())
        return;
    ::tools::JsonWriter aJsonWriter;
    aJsonWriter.put("commandName", ".uno:PreviewShapeSelection");
    {
        auto aState = aJsonWriter.startNode("state");
        SdrObject* pShape = GetSelectedShape();
        SdrPageObj* pPreview = GetEnteredPreview();
        if (pShape && pPreview)
        {
            const ::tools::Rectangle aCanvasRectangle
                = SdDrawDocument::ConvertSlideRectangleToCanvas(pShape->GetLogicRect(),
                                                                *pPreview);
            aJsonWriter.put("empty", false);
            aJsonWriter.put("x", o3tl::toTwips(aCanvasRectangle.Left(), o3tl::Length::mm100));
            aJsonWriter.put("y", o3tl::toTwips(aCanvasRectangle.Top(), o3tl::Length::mm100));
            aJsonWriter.put("width",
                            o3tl::toTwips(aCanvasRectangle.GetWidth(), o3tl::Length::mm100));
            aJsonWriter.put("height",
                            o3tl::toTwips(aCanvasRectangle.GetHeight(), o3tl::Length::mm100));
        }
        else
        {
            aJsonWriter.put("empty", true);
        }
    }
    pViewShell->viewCallback(COKitCallbackType::STATE_CHANGED,
                             aJsonWriter.finishAndGetAsOString());
}

void PreviewEditMode::Notify(SfxBroadcaster&, const SfxHint& rHint)
{
    if (rHint.GetId() != SfxHintId::ThisIsAnSdrHint)
        return;
    const SdrHint* pSdrHint = static_cast<const SdrHint*>(&rHint);
    const SdrObject* pHintObject = pSdrHint->GetObject();
    if (!pHintObject)
        return;

    if (pSdrHint->GetKind() == SdrHintKind::ObjectRemoved)
    {
        if (pHintObject == mxPreview.get().get())
        {
            // The entered preview is gone: the weak reference empties by itself, and the state
            // message carries the end of the mode. The send happens from inside the broadcast,
            // where this listener is still alive.
            SendModeState(false);
        }
        else if (pHintObject == mxSelectedShape.get().get())
        {
            mxSelectedShape = nullptr;
            SendSelectionState();
        }
    }
    else if (pSdrHint->GetKind() == SdrHintKind::ObjectChange
             && pHintObject == mxSelectedShape.get().get())
    {
        // A move or resize by any view gives the selected shape a new rectangle, so the
        // selection state goes out again with the new geometry.
        SendSelectionState();
    }
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
