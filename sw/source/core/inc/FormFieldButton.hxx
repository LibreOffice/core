/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SW_SOURCE_CORE_TEXT_FORMEFIELDBUTTO_HXX
#define INCLUDED_SW_SOURCE_CORE_TEXT_FORMEFIELDBUTTO_HXX

#include <vcl/menubtn.hxx>
#include <swrect.hxx>

class SwEditWin;
class FloatingWindow;
namespace sw
{
namespace mark
{
class Fieldmark;
}
} // namespace sw

/**
 * This button is shown when the cursor is on a form field with drop-down capability.
 */
class FormFieldButton : public MenuButton
{
public:
    FormFieldButton(SwEditWin* pEditWin, sw::mark::Fieldmark& rFieldMark);
    virtual ~FormFieldButton() override;
    virtual void dispose() override;

    void CalcPosAndSize(const SwRect& rPortionPaintArea);

    virtual void MouseButtonUp(const MouseEvent& rMEvt) override;
    DECL_LINK(FieldPopupModeEndHdl, FloatingWindow*, void);

    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect) override;
    virtual WindowHitTest ImplHitTest(const Point& rFramePos) override;

    virtual void InitPopup() = 0;

private:
    tools::Rectangle m_aFieldFramePixel;

protected:
    sw::mark::Fieldmark& m_rFieldmark;
    VclPtr<FloatingWindow> m_pFieldPopup;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
