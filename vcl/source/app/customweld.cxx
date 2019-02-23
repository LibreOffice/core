/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vcl/customweld.hxx>

namespace weld
{
CustomWidgetController::~CustomWidgetController() {}

CustomWeld::CustomWeld(weld::Builder& rBuilder, const OString& rDrawingId,
                       CustomWidgetController& rWidgetController)
    : m_rWidgetController(rWidgetController)
    , m_xDrawingArea(rBuilder.weld_drawing_area(rDrawingId, rWidgetController.CreateAccessible(),
                                                rWidgetController.GetUITestFactory(),
                                                &rWidgetController))
{
    m_xDrawingArea->connect_size_allocate(LINK(this, CustomWeld, DoResize));
    m_xDrawingArea->connect_draw(LINK(this, CustomWeld, DoPaint));
    m_xDrawingArea->connect_mouse_press(LINK(this, CustomWeld, DoMouseButtonDown));
    m_xDrawingArea->connect_mouse_move(LINK(this, CustomWeld, DoMouseMove));
    m_xDrawingArea->connect_mouse_release(LINK(this, CustomWeld, DoMouseButtonUp));
    m_xDrawingArea->connect_focus_in(LINK(this, CustomWeld, DoGetFocus));
    m_xDrawingArea->connect_focus_out(LINK(this, CustomWeld, DoLoseFocus));
    m_xDrawingArea->connect_key_press(LINK(this, CustomWeld, DoKeyPress));
    m_xDrawingArea->connect_focus_rect(LINK(this, CustomWeld, DoFocusRect));
    m_xDrawingArea->connect_style_updated(LINK(this, CustomWeld, DoStyleUpdated));
    m_xDrawingArea->connect_popup_menu(LINK(this, CustomWeld, DoPopupMenu));
    m_xDrawingArea->connect_query_tooltip(LINK(this, CustomWeld, DoRequestHelp));
    m_rWidgetController.SetDrawingArea(m_xDrawingArea.get());
}

IMPL_LINK(CustomWeld, DoResize, const Size&, rSize, void)
{
    m_rWidgetController.SetOutputSizePixel(rSize);
    m_rWidgetController.Resize();
}

IMPL_LINK(CustomWeld, DoPaint, weld::DrawingArea::draw_args, aPayload, void)
{
    m_rWidgetController.Paint(aPayload.first, aPayload.second);
}

IMPL_LINK(CustomWeld, DoMouseButtonDown, const MouseEvent&, rMEvt, bool)
{
    return m_rWidgetController.MouseButtonDown(rMEvt);
}

IMPL_LINK(CustomWeld, DoMouseMove, const MouseEvent&, rMEvt, bool)
{
    return m_rWidgetController.MouseMove(rMEvt);
}

IMPL_LINK(CustomWeld, DoMouseButtonUp, const MouseEvent&, rMEvt, bool)
{
    return m_rWidgetController.MouseButtonUp(rMEvt);
}

IMPL_LINK_NOARG(CustomWeld, DoGetFocus, weld::Widget&, void) { m_rWidgetController.GetFocus(); }

IMPL_LINK_NOARG(CustomWeld, DoLoseFocus, weld::Widget&, void) { m_rWidgetController.LoseFocus(); }

IMPL_LINK(CustomWeld, DoKeyPress, const KeyEvent&, rKEvt, bool)
{
    return m_rWidgetController.KeyInput(rKEvt);
}

IMPL_LINK_NOARG(CustomWeld, DoFocusRect, weld::Widget&, tools::Rectangle)
{
    return m_rWidgetController.GetFocusRect();
}

IMPL_LINK_NOARG(CustomWeld, DoStyleUpdated, weld::Widget&, void)
{
    m_rWidgetController.StyleUpdated();
}

IMPL_LINK(CustomWeld, DoPopupMenu, const Point&, rPos, bool)
{
    return m_rWidgetController.ContextMenu(rPos);
}

IMPL_LINK(CustomWeld, DoRequestHelp, tools::Rectangle&, rHelpArea, OUString)
{
    return m_rWidgetController.RequestHelp(rHelpArea);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
