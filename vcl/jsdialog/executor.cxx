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

void SendFullUpdate(const std::string& nWindowId, const OString& rWidget)
{
    weld::Widget* pWidget = JSInstanceBuilder::FindWeldWidgetsMap(nWindowId, rWidget);
    if (auto pJSWidget = dynamic_cast<BaseJSWidget*>(pWidget))
        pJSWidget->sendFullUpdate();
}

void SendAction(const std::string& nWindowId, const OString& rWidget,
                std::unique_ptr<ActionDataMap> pData)
{
    weld::Widget* pWidget = JSInstanceBuilder::FindWeldWidgetsMap(nWindowId, rWidget);
    if (auto pJSWidget = dynamic_cast<BaseJSWidget*>(pWidget))
        pJSWidget->sendAction(std::move(pData));
}

bool ExecuteAction(const std::string& nWindowId, const OString& rWidget, StringMap& rData)
{
    weld::Widget* pWidget = JSInstanceBuilder::FindWeldWidgetsMap(nWindowId, rWidget);

    OUString sControlType = rData["type"];
    OUString sAction = rData["cmd"];

    if (sControlType == "responsebutton")
    {
        if (pWidget == nullptr)
        {
            // welded wrapper not found - use response code instead
            pWidget = JSInstanceBuilder::FindWeldWidgetsMap(nWindowId, "__DIALOG__");
            sControlType = "dialog";
            sAction = "response";
        }
        else
        {
            // welded wrapper for button found - use it
            sControlType = "pushbutton";
        }
    }

    if (pWidget != nullptr)
    {
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
                    // it might be other class than JSComboBox
                    auto pJSCombobox = dynamic_cast<JSComboBox*>(pWidget);
                    if (pJSCombobox)
                        pJSCombobox->set_entry_text_without_notify(rData["data"]);
                    else
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
        else if (sControlType == "menubutton")
        {
            auto pButton = dynamic_cast<weld::MenuButton*>(pWidget);
            if (pButton)
            {
                if (sAction == "toggle")
                {
                    if (pButton->get_active())
                        pButton->set_active(false);
                    else
                        pButton->set_active(true);

                    BaseJSWidget* pMenuButton = dynamic_cast<BaseJSWidget*>(pButton);
                    if (pMenuButton)
                        pMenuButton->sendUpdate(true);

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
                    LOKTrigger::trigger_toggled(*static_cast<weld::Toggleable*>(pCheckButton));
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
                    int nSeparatorPos = rData["data"].indexOf(';');
                    if (nSeparatorPos > 0)
                    {
                        // x;y
                        std::u16string_view nClickPosX = rData["data"].subView(0, nSeparatorPos);
                        std::u16string_view nClickPosY = rData["data"].subView(nSeparatorPos + 1);

                        if (nClickPosX.empty() || nClickPosY.empty())
                            return true;

                        double posX = std::atof(
                            OUStringToOString(nClickPosX.data(), RTL_TEXTENCODING_ASCII_US)
                                .getStr());
                        double posY = std::atof(
                            OUStringToOString(nClickPosY.data(), RTL_TEXTENCODING_ASCII_US)
                                .getStr());
                        OutputDevice& rRefDevice = pArea->get_ref_device();
                        // We send OutPutSize for the drawing area bitmap
                        // get_size_request is not necessarily updated
                        // therefore it may be incorrect.
                        Size size = rRefDevice.GetOutputSize();
                        posX = posX * size.Width();
                        posY = posY * size.Height();
                        LOKTrigger::trigger_click(*pArea, Point(posX, posY));
                        return true;
                    }

                    LOKTrigger::trigger_click(*pArea, Point(10, 10));
                    return true;
                }
                else if (sAction == "keypress")
                {
                    LOKTrigger::trigger_key_press(
                        *pArea,
                        KeyEvent(rData["data"].toUInt32(), vcl::KeyCode(rData["data"].toUInt32())));
                    LOKTrigger::trigger_key_release(
                        *pArea,
                        KeyEvent(rData["data"].toUInt32(), vcl::KeyCode(rData["data"].toUInt32())));
                    return true;
                }
                else if (sAction == "textselection")
                {
                    int nSeparatorPos = rData["data"].indexOf(';');
                    if (nSeparatorPos <= 0)
                        return true;

                    int nSeparator2Pos = rData["data"].indexOf(';', nSeparatorPos + 1);
                    int nSeparator3Pos = 0;

                    if (nSeparator2Pos > 0)
                    {
                        // start;end;startPara;endPara
                        nSeparator3Pos = rData["data"].indexOf(';', nSeparator2Pos + 1);
                        if (nSeparator3Pos <= 0)
                            return true;
                    }
                    else
                    {
                        // start;end
                        nSeparator2Pos = 0;
                        nSeparator3Pos = 0;
                    }

                    std::u16string_view aStartPos = rData["data"].subView(0, nSeparatorPos);
                    std::u16string_view aEndPos = rData["data"].subView(
                        nSeparatorPos + 1, nSeparator2Pos - nSeparatorPos + 1);

                    if (aStartPos.empty() || aEndPos.empty())
                        return true;

                    int nStart = std::atoi(
                        OUStringToOString(aStartPos.data(), RTL_TEXTENCODING_ASCII_US).getStr());
                    int nEnd = std::atoi(
                        OUStringToOString(aEndPos.data(), RTL_TEXTENCODING_ASCII_US).getStr());
                    int nStartPara = 0;
                    int nEndPara = 0;

                    // multiline case
                    if (nSeparator2Pos && nSeparator3Pos)
                    {
                        std::u16string_view aStartPara = rData["data"].subView(
                            nSeparator2Pos + 1, nSeparator3Pos - nSeparator2Pos + 1);
                        std::u16string_view aEndPara = rData["data"].subView(nSeparator3Pos + 1);

                        if (aStartPara.empty() || aEndPara.empty())
                            return true;

                        nStartPara = std::atoi(
                            OUStringToOString(aStartPara.data(), RTL_TEXTENCODING_ASCII_US)
                                .getStr());
                        nEndPara = std::atoi(
                            OUStringToOString(aEndPara.data(), RTL_TEXTENCODING_ASCII_US).getStr());
                    }

                    // pass information about paragraph number in the additional data
                    // handled in sc/source/ui/app/inputwin.cxx
                    Point* pParaPoint = new Point(nStartPara, nEndPara);
                    const void* pCmdData = pParaPoint;

                    Point aPos(nStart, nEnd);
                    CommandEvent aCEvt(aPos, CommandEventId::CursorPos, false, pCmdData);
                    LOKTrigger::command(*pArea, aCEvt);

                    return true;
                }
            }
        }
        else if (sControlType == "spinfield")
        {
            auto pSpinField = dynamic_cast<weld::SpinButton*>(pWidget);
            if (pSpinField)
            {
                if (sAction == "change" || sAction == "value")
                {
                    if (rData["data"] == "undefined")
                        return true;

                    OString sValue = OUStringToOString(rData["data"], RTL_TEXTENCODING_ASCII_US);
                    double nValue = std::atof(sValue.getStr());
                    pSpinField->set_value(nValue
                                          * weld::SpinButton::Power10(pSpinField->get_digits()));
                    LOKTrigger::trigger_value_changed(*pSpinField);
                    return true;
                }
                if (sAction == "plus")
                {
                    pSpinField->set_value(pSpinField->get_value() + 1);
                    LOKTrigger::trigger_value_changed(*pSpinField);
                    return true;
                }
                else if (sAction == "minus")
                {
                    pSpinField->set_value(pSpinField->get_value() - 1);
                    LOKTrigger::trigger_value_changed(*pSpinField);
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
                else if (sAction == "togglemenu")
                {
                    pToolbar->set_menu_item_active(
                        OUStringToOString(rData["data"], RTL_TEXTENCODING_ASCII_US), true);
                    return true;
                }
            }
        }
        else if (sControlType == "edit")
        {
            auto pEdit = dynamic_cast<JSEntry*>(pWidget);
            if (pEdit)
            {
                if (sAction == "change")
                {
                    pEdit->set_text_without_notify(rData["data"]);
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

                    pTreeView->unselect_all();

                    int nAbsPos = std::atoi(nRowString.getStr());

                    std::unique_ptr<weld::TreeIter> itEntry(pTreeView->make_iterator());
                    pTreeView->get_iter_abs_pos(*itEntry, nAbsPos);
                    pTreeView->select(*itEntry);
                    pTreeView->set_cursor(*itEntry);
                    LOKTrigger::trigger_changed(*pTreeView);
                    return true;
                }
                else if (sAction == "activate")
                {
                    OString nRowString
                        = OUStringToOString(rData["data"], RTL_TEXTENCODING_ASCII_US);
                    int nRow = std::atoi(nRowString.getStr());

                    pTreeView->unselect_all();
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
        else if (sControlType == "iconview")
        {
            auto pIconView = dynamic_cast<weld::IconView*>(pWidget);
            if (pIconView)
            {
                if (sAction == "select")
                {
                    OString nPosString
                        = OUStringToOString(rData["data"], RTL_TEXTENCODING_ASCII_US);
                    int nPos = std::atoi(nPosString.getStr());

                    pIconView->select(nPos);
                    LOKTrigger::trigger_changed(*pIconView);

                    return true;
                }
                else if (sAction == "activate")
                {
                    OString nPosString
                        = OUStringToOString(rData["data"], RTL_TEXTENCODING_ASCII_US);
                    int nPos = std::atoi(nPosString.getStr());

                    pIconView->select(nPos);
                    LOKTrigger::trigger_changed(*pIconView);
                    LOKTrigger::trigger_item_activated(*pIconView);

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
                else if (sAction == "response")
                {
                    OString nResponseString
                        = OUStringToOString(rData["data"], RTL_TEXTENCODING_ASCII_US);
                    int nResponse = std::atoi(nResponseString.getStr());
                    pDialog->response(nResponse);
                    return true;
                }
            }
        }
        else if (sControlType == "popover")
        {
            auto pPopover = dynamic_cast<weld::Popover*>(pWidget);
            if (pPopover)
            {
                if (sAction == "close")
                {
                    LOKTrigger::trigger_closed(*pPopover);
                    return true;
                }
            }
        }
        else if (sControlType == "radiobutton")
        {
            auto pRadioButton = dynamic_cast<weld::RadioButton*>(pWidget);
            if (pRadioButton)
            {
                if (sAction == "change")
                {
                    bool bChecked = rData["data"] == "true";
                    pRadioButton->set_state(bChecked ? TRISTATE_TRUE : TRISTATE_FALSE);
                    LOKTrigger::trigger_toggled(*static_cast<weld::Toggleable*>(pRadioButton));
                    return true;
                }
            }
        }
    }

    return false;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
