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

    static void trigger_toggled(weld::Toggleable& rButton) { rButton.signal_toggled(); }

    static void trigger_row_activated(weld::TreeView& rTreeView)
    {
        rTreeView.signal_row_activated();
    }

    static void trigger_item_activated(weld::IconView& rIconView)
    {
        rIconView.signal_item_activated();
    }

    static void trigger_clicked(weld::Toolbar& rToolbar, const OString& rIdent)
    {
        rToolbar.signal_clicked(rIdent);
    }

    static void trigger_clicked(weld::Button& rButton) { rButton.signal_clicked(); }

    static void trigger_click(weld::DrawingArea& rDrawingArea, const Point& rPos)
    {
        rDrawingArea.click(rPos);
    }

    static void trigger_value_changed(weld::SpinButton& rSpinButton)
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
};

namespace jsdialog
{
// type used to store key-value pairs to put in the generated messages
typedef std::unordered_map<std::string, OUString> ActionDataMap;

/// execute action on a widget
VCL_DLLPUBLIC bool ExecuteAction(const std::string& nWindowId, const OString& rWidget,
                                 StringMap& rData);
/// send full update message to the client
VCL_DLLPUBLIC void SendFullUpdate(const std::string& nWindowId, const OString& rWidget);
/// send action message to the client
VCL_DLLPUBLIC void SendAction(const std::string& nWindowId, const OString& rWidget,
                              std::unique_ptr<ActionDataMap> pData);
VCL_DLLPUBLIC StringMap jsonToStringMap(const char* pJSON);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
