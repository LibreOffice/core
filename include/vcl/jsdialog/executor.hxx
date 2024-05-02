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
#include <vcl/uitest/uiobject.hxx>
#include <vcl/weld.hxx>
#include <unordered_map>

class LOKTrigger
{
public:
    static void trigger_changed(weld::TextView& rView) { rView.signal_changed(); }

    static void trigger_changed(weld::Entry& rEdit) { rEdit.signal_changed(); }

    static void trigger_changed(weld::ComboBox& rComboBox) { rComboBox.signal_changed(); }

    static void trigger_changed(weld::TreeView& rTreeView) { rTreeView.signal_changed(); }

    static void trigger_changed(weld::IconView& rIconView) { rIconView.signal_selection_changed(); }

    static void trigger_scrollv(weld::ScrolledWindow& rScrolledWindow)
    {
        rScrolledWindow.signal_vadjustment_changed();
    }

    static void trigger_scrollh(weld::ScrolledWindow& rScrolledWindow)
    {
        rScrolledWindow.signal_hadjustment_changed();
    }

    static void trigger_toggled(weld::Toggleable& rButton) { rButton.signal_toggled(); }

    static void trigger_row_activated(weld::TreeView& rTreeView)
    {
        rTreeView.signal_row_activated();
    }

    static void trigger_item_activated(weld::IconView& rIconView)
    {
        rIconView.signal_item_activated();
    }

    static void trigger_clicked(weld::Toolbar& rToolbar, const OUString& rIdent)
    {
        rToolbar.signal_clicked(rIdent);
    }

    static void trigger_clicked(weld::Button& rButton) { rButton.signal_clicked(); }

    static void trigger_click(weld::DrawingArea& rDrawingArea, const Point& rPos)
    {
        rDrawingArea.click(rPos);
    }

    static void trigger_dblclick(weld::DrawingArea& rDrawingArea, const Point& rPos)
    {
        rDrawingArea.dblclick(rPos);
    }

    static void trigger_mouse_up(weld::DrawingArea& rDrawingArea, const Point& rPos)
    {
        rDrawingArea.mouse_up(rPos);
    }

    static void trigger_mouse_down(weld::DrawingArea& rDrawingArea, const Point& rPos)
    {
        rDrawingArea.mouse_down(rPos);
    }

    static void trigger_mouse_move(weld::DrawingArea& rDrawingArea, const Point& rPos)
    {
        rDrawingArea.mouse_move(rPos);
    }

    static void trigger_selected(weld::MenuButton& rButton, const OUString& rIdent)
    {
        rButton.signal_selected(rIdent);
    }

    static void trigger_selected(weld::Calendar& rCalendar) { rCalendar.signal_selected(); }

    static void trigger_activated(weld::Calendar& rCalendar) { rCalendar.signal_activated(); }

    static void trigger_value_changed(weld::SpinButton& rSpinButton)
    {
        rSpinButton.signal_value_changed();
    }

    static void trigger_value_changed(weld::FormattedSpinButton& rSpinButton)
    {
        rSpinButton.signal_value_changed();
    }

    static void trigger_closed(weld::Popover& rPopover) { rPopover.popdown(); }

    static void trigger_key_press(weld::Widget& rWidget, const KeyEvent& rEvent)
    {
        rWidget.m_aKeyPressHdl.Call(rEvent);
    }

    static void trigger_key_release(weld::Widget& rWidget, const KeyEvent& rEvent)
    {
        rWidget.m_aKeyReleaseHdl.Call(rEvent);
    }

    static void command(weld::DrawingArea& rArea, const CommandEvent& rCmd)
    {
        rArea.m_aCommandHdl.Call(rCmd);
    }

    static void enter_page(weld::Notebook& rNotebook, const OUString& rPage)
    {
        rNotebook.m_aEnterPageHdl.Call(rPage);
    }

    static void leave_page(weld::Notebook& rNotebook, const OUString& rPage)
    {
        rNotebook.m_aLeavePageHdl.Call(rPage);
    }

    static bool activate_link(weld::LinkButton& rLinkButton)
    {
        return rLinkButton.signal_activate_link();
    }
};

namespace jsdialog
{
// type used to store key-value pairs to put in the generated messages
typedef std::unordered_map<OString, OUString> ActionDataMap;

/// execute action on a widget
VCL_DLLPUBLIC bool ExecuteAction(const OUString& nWindowId, const OUString& rWidget,
                                 StringMap& rData);
/// send full update message to the client
VCL_DLLPUBLIC void SendFullUpdate(const OUString& nWindowId, const OUString& rWidget);
/// send action message to the client
VCL_DLLPUBLIC void SendAction(const OUString& nWindowId, const OUString& rWidget,
                              std::unique_ptr<ActionDataMap> pData);
VCL_DLLPUBLIC StringMap jsonToStringMap(const char* pJSON);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
