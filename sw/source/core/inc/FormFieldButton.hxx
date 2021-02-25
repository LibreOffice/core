/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <vcl/ctrl.hxx>
#include <vcl/weld.hxx>
#include <swrect.hxx>

class SwEditWin;
namespace sw::mark
{
class Fieldmark;
}

/**
 * This button is shown when the cursor is on a form field with drop-down capability.
 */
class FormFieldButton : public Control
{
public:
    FormFieldButton(SwEditWin* pEditWin, sw::mark::Fieldmark& rFieldMark);
    virtual ~FormFieldButton() override;
    virtual void dispose() override;

    void CalcPosAndSize(const SwRect& rPortionPaintArea);

    virtual void MouseButtonDown(const MouseEvent& rMEvt) override;
    DECL_LINK(FieldPopupModeEndHdl, weld::Popover&, void);

    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect) override;
    virtual WindowHitTest ImplHitTest(const Point& rFramePos) override;

    virtual void LaunchPopup();
    virtual void DestroyPopup();

private:
    tools::Rectangle m_aFieldFramePixel;

protected:
    sw::mark::Fieldmark& m_rFieldmark;
    std::unique_ptr<weld::Builder> m_xFieldPopupBuilder;
    std::unique_ptr<weld::Popover> m_xFieldPopup;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
