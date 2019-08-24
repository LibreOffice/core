/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <vcl/drawables/RectangleDrawable.hxx>
#include <vcl/outdev.hxx>
#include <vcl/virdev.hxx>

#include <salgdi.hxx>

#include <cassert>

namespace vcl
{
bool RectangleDrawable::DrawCommand(OutputDevice* pRenderContext) const
{
    tools::Rectangle aRect(pRenderContext->ImplLogicToDevicePixel(maRect));

    if (aRect.IsEmpty())
        return false;

    aRect.Justify();

    mpGraphics->DrawRect(aRect.Left(), aRect.Top(), aRect.GetWidth(), aRect.GetHeight(),
                         pRenderContext);

    return true;
}

bool RectangleDrawable::CanDraw(OutputDevice* pRenderContext) const
{
    if (!pRenderContext->IsDeviceOutputNecessary()
        || (!pRenderContext->IsLineColor() && !pRenderContext->IsFillColor())
        || pRenderContext->ImplIsRecordLayout())
        return false;

    return true;
}

} // namespace vcl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
