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

class LOKTrigger
{
public:
    static void trigger_changed(weld::TextView& rView) { rView.signal_changed(); }

    static void trigger_changed(weld::Entry& rEdit) { rEdit.signal_changed(); }

    static void trigger_changed(weld::ComboBox& rComboBox) { rComboBox.signal_changed(); }

    static void trigger_changed(weld::TreeView& rTreeView) { rTreeView.signal_changed(); }

    static void trigger_changed(weld::IconView& rIconView) { rIconView.signal_selection_changed(); }

    static void trigger_toggled(weld::ToggleButton& rButton) { rButton.signal_toggled(); }

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
};

namespace jsdialog
{
VCL_DLLPUBLIC bool ExecuteAction(sal_uInt64 nWindowId, const OString& rWidget, StringMap& rData);
VCL_DLLPUBLIC StringMap jsonToStringMap(const char* pJSON);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
