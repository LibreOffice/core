/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <jsdialog/jsdialogbuilder.hxx>
#include <vcl/weld.hxx>
#include <vcl/jsdialog/executor.hxx>
#include <sal/log.hxx>
#include <rtl/uri.hxx>
#include <boost/property_tree/json_parser.hpp>

namespace jsdialog
{
StringMap jsonToStringMap(const char* pJSON)
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

bool ExecuteAction(sal_uInt64 nWindowId, const OString& rWidget, StringMap& rData)
{
    weld::Widget* pWidget = JSInstanceBuilder::FindWeldWidgetsMap(nWindowId, rWidget);

    if (pWidget != nullptr)
    {
        OUString sControlType = rData["type"];
        OUString sAction = rData["cmd"];

        if (sControlType == "tabcontrol")
        {
            auto pNotebook = dynamic_cast<weld::Notebook*>(pWidget);
            if (pNotebook)
            {
                if (sAction == "selecttab")
                {
                    OString pageId = OUStringToOString(rData["data"], RTL_TEXTENCODING_ASCII_US);
                    int page = std::atoi(pageId.getStr());

                    pNotebook->set_current_page(page);

                    return true;
                }
            }
        }
        else if (sControlType == "combobox")
        {
            auto pCombobox = dynamic_cast<weld::ComboBox*>(pWidget);
            if (pCombobox)
            {
                if (sAction == "selected")
                {
                    int separatorPos = rData["data"].indexOf(';');
                    if (separatorPos > 0)
                    {
                        OUString entryPos = rData["data"].copy(0, separatorPos);
                        OString posString = OUStringToOString(entryPos, RTL_TEXTENCODING_ASCII_US);
                        int pos = std::atoi(posString.getStr());
                        pCombobox->set_active(pos);
                        LOKTrigger::trigger_changed(*pCombobox);
                        return true;
                    }
                }
                else if (sAction == "change")
                {
                    pCombobox->set_entry_text(rData["data"]);
                    LOKTrigger::trigger_changed(*pCombobox);
                    return true;
                }
            }
        }
        else if (sControlType == "pushbutton")
        {
            auto pButton = dynamic_cast<weld::Button*>(pWidget);
            if (pButton)
            {
                if (sAction == "click")
                {
                    pButton->clicked();
                    return true;
                }
            }
        }
        else if (sControlType == "checkbox")
        {
            auto pCheckButton = dynamic_cast<weld::CheckButton*>(pWidget);
            if (pCheckButton)
            {
                if (sAction == "change")
                {
                    bool bChecked = rData["data"] == "true";
                    pCheckButton->set_state(bChecked ? TRISTATE_TRUE : TRISTATE_FALSE);
                    LOKTrigger::trigger_toggled(*static_cast<weld::ToggleButton*>(pCheckButton));
                    return true;
                }
            }
        }
        else if (sControlType == "drawingarea")
        {
            auto pArea = dynamic_cast<weld::DrawingArea*>(pWidget);
            if (pArea)
            {
                if (sAction == "click")
                {
                    LOKTrigger::trigger_click(*pArea, Point(10, 10));
                    return true;
                }
            }
        }
        else if (sControlType == "spinfield")
        {
            auto pSpinField = dynamic_cast<weld::SpinButton*>(pWidget);
            if (pSpinField)
            {
                if (sAction == "plus")
                {
                    pSpinField->set_value(pSpinField->get_value() + 1);
                    return true;
                }
                else if (sAction == "minus")
                {
                    pSpinField->set_value(pSpinField->get_value() - 1);
                    return true;
                }
            }
        }
        else if (sControlType == "toolbox")
        {
            auto pToolbar = dynamic_cast<weld::Toolbar*>(pWidget);
            if (pToolbar)
            {
                if (sAction == "click")
                {
                    LOKTrigger::trigger_clicked(
                        *pToolbar, OUStringToOString(rData["data"], RTL_TEXTENCODING_ASCII_US));
                    return true;
                }
            }
        }
        else if (sControlType == "edit")
        {
            auto pEdit = dynamic_cast<weld::Entry*>(pWidget);
            if (pEdit)
            {
                if (sAction == "change")
                {
                    pEdit->set_text(rData["data"]);
                    LOKTrigger::trigger_changed(*pEdit);
                    return true;
                }
            }

            auto pTextView = dynamic_cast<weld::TextView*>(pWidget);
            if (pTextView)
            {
                if (sAction == "change")
                {
                    pTextView->set_text(rData["data"]);
                    LOKTrigger::trigger_changed(*pTextView);
                    return true;
                }
            }
        }
        else if (sControlType == "treeview")
        {
            auto pTreeView = dynamic_cast<JSTreeView*>(pWidget);
            if (pTreeView)
            {
                if (sAction == "change")
                {
                    OUString sDataJSON = rtl::Uri::decode(
                        rData["data"], rtl_UriDecodeMechanism::rtl_UriDecodeWithCharset,
                        RTL_TEXTENCODING_UTF8);
                    StringMap aMap(jsonToStringMap(
                        OUStringToOString(sDataJSON, RTL_TEXTENCODING_ASCII_US).getStr()));

                    OString nRowString = OUStringToOString(aMap["row"], RTL_TEXTENCODING_ASCII_US);
                    int nRow = std::atoi(nRowString.getStr());
                    bool bValue = aMap["value"] == "true";

                    pTreeView->set_toggle(nRow, bValue ? TRISTATE_TRUE : TRISTATE_FALSE);

                    return true;
                }
                else if (sAction == "select")
                {
                    OString nRowString
                        = OUStringToOString(rData["data"], RTL_TEXTENCODING_ASCII_US);
                    int nRow = std::atoi(nRowString.getStr());

                    pTreeView->unselect(pTreeView->get_selected_index());
                    pTreeView->select(nRow);
                    pTreeView->set_cursor(nRow);
                    LOKTrigger::trigger_changed(*pTreeView);
                    LOKTrigger::trigger_row_activated(*pTreeView);
                    return true;
                }
                else if (sAction == "expand")
                {
                    OString nRowString
                        = OUStringToOString(rData["data"], RTL_TEXTENCODING_ASCII_US);
                    int nAbsPos = std::atoi(nRowString.getStr());
                    std::unique_ptr<weld::TreeIter> itEntry(pTreeView->make_iterator());
                    pTreeView->get_iter_abs_pos(*itEntry, nAbsPos);
                    pTreeView->expand_row(*itEntry);
                    return true;
                }
                else if (sAction == "dragstart")
                {
                    OString nRowString
                        = OUStringToOString(rData["data"], RTL_TEXTENCODING_ASCII_US);
                    int nRow = std::atoi(nRowString.getStr());

                    pTreeView->select(nRow);
                    pTreeView->drag_start();

                    return true;
                }
                else if (sAction == "dragend")
                {
                    pTreeView->drag_end();
                    return true;
                }
            }
        }
        else if (sControlType == "expander")
        {
            auto pExpander = dynamic_cast<weld::Expander*>(pWidget);
            if (pExpander)
            {
                if (sAction == "toggle")
                {
                    pExpander->set_expanded(!pExpander->get_expanded());
                    return true;
                }
            }
        }
        else if (sControlType == "dialog")
        {
            auto pDialog = dynamic_cast<weld::Dialog*>(pWidget);
            if (pDialog)
            {
                if (sAction == "close")
                {
                    pDialog->response(RET_CANCEL);
                    return true;
                }
            }
        }
    }

    return false;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
