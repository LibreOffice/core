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

static StringMap jsonToStringMap(const char* pJSON)
{
    StringMap aArgs;
    if (pJSON && pJSON[0] != '\0')
    {
        std::stringstream aStream(pJSON);
        boost::property_tree::ptree aTree;
        boost::property_tree::read_json(aStream, aTree);

        for (const auto& rPair : aTree)
        {
            aArgs[OUString::fromUtf8(rPair.first.c_str())]
                = OUString::fromUtf8(rPair.second.get_value<std::string>(".").c_str());
        }
    }
    return aArgs;
}
class LOKTrigger
{
public:
    static void trigger_changed(weld::TextView& rView) { rView.signal_changed(); }

    static void trigger_changed(weld::Entry& rEdit) { rEdit.signal_changed(); }

    static void trigger_changed(weld::ComboBox& rComboBox) { rComboBox.signal_changed(); }

    static void trigger_row_activated(weld::TreeView& rTreeView)
    {
        rTreeView.signal_row_activated();
    }

    static void trigger_clicked(weld::Toolbar& rToolbar, const OString& rIdent)
    {
        rToolbar.signal_clicked(rIdent);
    }

    static void trigger_click(weld::DrawingArea& rDrawingArea, const Point& rPos)
    {
        rDrawingArea.click(rPos);
    }
};

namespace jsdialog
{
VCL_DLLPUBLIC bool ExecuteAction(sal_uInt64 nWindowId, const OString& rWidget, StringMap& rData);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
