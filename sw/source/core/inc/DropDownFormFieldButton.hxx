/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SW_SOURCE_CORE_TEXT_DROPDOWNFORMEFIELDBUTTO_HXX
#define INCLUDED_SW_SOURCE_CORE_TEXT_DROPDOWNFORMEFIELDBUTTO_HXX

#include <vcl/menubtn.hxx>
#include <swrect.hxx>

class SwFieldFormDropDownPortion;
class SwEditWin;
class FloatingWindow;
namespace sw
{
namespace mark
{
class DropDownFieldmark;
}
}

/**
 * This button is shown when the cursor is in a drop-down form field.
 * The user can select an item of the field using this button while filling in a form.
 */
class DropDownFormFieldButton : public MenuButton
{
public:
    DropDownFormFieldButton(SwEditWin* pEditWin, sw::mark::DropDownFieldmark& rFieldMark);
    virtual ~DropDownFormFieldButton() override;
    virtual void dispose() override;

    void CalcPosAndSize(const SwRect& rPortionPaintArea);

    virtual void MouseButtonUp(const MouseEvent& rMEvt) override;
    DECL_LINK(FieldPopupModeEndHdl, FloatingWindow*, void);

    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect) override;
    virtual WindowHitTest ImplHitTest(const Point& rFramePos) override;

private:
    tools::Rectangle m_aFieldFramePixel;
    sw::mark::DropDownFieldmark& m_rFieldmark;
    VclPtr<FloatingWindow> m_pFieldPopup;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
