/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <vcl/dllapi.h>
#include <vcl/weld/weld.hxx>

namespace weld
{
class VCL_DLLPUBLIC DrawingArea : virtual public Widget
{
public:
    typedef std::pair<vcl::RenderContext&, const tools::Rectangle&> draw_args;

protected:
    Link<draw_args, void> m_aDrawHdl;
    Link<Widget&, tools::Rectangle> m_aGetFocusRectHdl;
    Link<tools::Rectangle&, OUString> m_aQueryTooltipHdl;
    // if handler returns true, drag is disallowed
    Link<DrawingArea&, bool> m_aDragBeginHdl;
    // return position of cursor, fill OUString& with surrounding text
    Link<OUString&, int> m_aGetSurroundingHdl;
    // attempt to delete the range, return true if successful
    Link<const Selection&, bool> m_aDeleteSurroundingHdl;

    OUString signal_query_tooltip(tools::Rectangle& rHelpArea)
    {
        return m_aQueryTooltipHdl.Call(rHelpArea);
    }

    int signal_im_context_get_surrounding(OUString& rSurroundingText)
    {
        if (!m_aGetSurroundingHdl.IsSet())
            return -1;
        return m_aGetSurroundingHdl.Call(rSurroundingText);
    }

    bool signal_im_context_delete_surrounding(const Selection& rRange)
    {
        return m_aDeleteSurroundingHdl.Call(rRange);
    }

public:
    void connect_draw(const Link<draw_args, void>& rLink) { m_aDrawHdl = rLink; }
    void connect_focus_rect(const Link<Widget&, tools::Rectangle>& rLink)
    {
        m_aGetFocusRectHdl = rLink;
    }
    void connect_query_tooltip(const Link<tools::Rectangle&, OUString>& rLink)
    {
        m_aQueryTooltipHdl = rLink;
    }
    void connect_drag_begin(const Link<DrawingArea&, bool>& rLink) { m_aDragBeginHdl = rLink; }
    void connect_im_context_get_surrounding(const Link<OUString&, int>& rLink)
    {
        m_aGetSurroundingHdl = rLink;
    }
    void connect_im_context_delete_surrounding(const Link<const Selection&, bool>& rLink)
    {
        m_aDeleteSurroundingHdl = rLink;
    }
    virtual void queue_draw() = 0;
    virtual void queue_draw_area(int x, int y, int width, int height) = 0;

    virtual void enable_drag_source(rtl::Reference<TransferDataContainer>& rTransferable,
                                    sal_uInt8 eDNDConstants)
        = 0;

    virtual void set_cursor(PointerStyle ePointerStyle) = 0;

    virtual Point get_pointer_position() const = 0;

    virtual void set_input_context(const InputContext& rInputContext) = 0;
    virtual void im_context_set_cursor_location(const tools::Rectangle& rCursorRect,
                                                int nExtTextInputWidth)
        = 0;

    // use return here just to generate matching VirtualDevices
    virtual OutputDevice& get_ref_device() = 0;

    virtual rtl::Reference<comphelper::OAccessible> get_accessible_parent() = 0;
    virtual a11yrelationset get_accessible_relation_set() = 0;
    virtual AbsoluteScreenPixelPoint get_accessible_location_on_screen() = 0;

private:
    friend class ::LOKTrigger;

    virtual void click(const Point&) = 0;

    virtual void dblclick(const Point&){};

    virtual void mouse_up(const Point&){};

    virtual void mouse_down(const Point&){};

    virtual void mouse_move(const Point&){};
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
