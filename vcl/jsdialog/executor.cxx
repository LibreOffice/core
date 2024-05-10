/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <jsdialog/jsdialogbuilder.hxx>
#include <o3tl/string_view.hxx>
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
            aArgs[OUString::fromUtf8(rPair.first)]
                = OUString::fromUtf8(rPair.second.get_value<std::string>("."));
        }
    }
    return aArgs;
}

void SendFullUpdate(const OUString& nWindowId, const OUString& rWidget)
{
    weld::Widget* pWidget = JSInstanceBuilder::FindWeldWidgetsMap(nWindowId, rWidget);
    if (auto pJSWidget = dynamic_cast<BaseJSWidget*>(pWidget))
        pJSWidget->sendFullUpdate();
}

void SendAction(const OUString& nWindowId, const OUString& rWidget,
                std::unique_ptr<ActionDataMap> pData)
{
    weld::Widget* pWidget = JSInstanceBuilder::FindWeldWidgetsMap(nWindowId, rWidget);
    if (auto pJSWidget = dynamic_cast<BaseJSWidget*>(pWidget))
        pJSWidget->sendAction(std::move(pData));
}

bool ExecuteAction(const OUString& nWindowId, const OUString& rWidget, StringMap& rData)
{
    weld::Widget* pWidget = JSInstanceBuilder::FindWeldWidgetsMap(nWindowId, rWidget);

    OUString sControlType = rData[u"type"_ustr];
    OUString sAction = rData[u"cmd"_ustr];

    if (sControlType == "responsebutton")
    {
        auto pButton = dynamic_cast<weld::Button*>(pWidget);
        if (pWidget == nullptr || (pButton && !pButton->is_custom_handler_set()))
        {
            // welded wrapper not found - use response code instead
            pWidget = JSInstanceBuilder::FindWeldWidgetsMap(nWindowId, u"__DIALOG__"_ustr);
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
        if (sAction == "grab_focus")
        {
            pWidget->grab_focus();
            return true;
        }

        if (sControlType == "tabcontrol")
        {
            auto pNotebook = dynamic_cast<weld::Notebook*>(pWidget);
            if (pNotebook)
            {
                if (sAction == "selecttab")
                {
                    sal_Int32 page = o3tl::toInt32(rData[u"data"_ustr]);

                    OUString aCurrentPage = pNotebook->get_current_page_ident();
                    LOKTrigger::leave_page(*pNotebook, aCurrentPage);
                    pNotebook->set_current_page(page);
                    LOKTrigger::enter_page(*pNotebook, pNotebook->get_page_ident(page));

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
                    OUString sSelectedData = rData[u"data"_ustr];
                    int separatorPos = sSelectedData.indexOf(';');
                    if (separatorPos > 0)
                    {
                        std::u16string_view entryPos = sSelectedData.subView(0, separatorPos);
                        sal_Int32 pos = o3tl::toInt32(entryPos);
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
                        pJSCombobox->set_entry_text_without_notify(rData[u"data"_ustr]);
                    else
                        pCombobox->set_entry_text(rData[u"data"_ustr]);
                    LOKTrigger::trigger_changed(*pCombobox);
                    return true;
                }
                else if (sAction == "render_entry")
                {
                    auto pJSCombobox = dynamic_cast<JSComboBox*>(pWidget);
                    if (pJSCombobox)
                    {
                        // pos;dpix;dpiy
                        const OUString& sParams = rData[u"data"_ustr];
                        const OUString aPos = sParams.getToken(0, ';');
                        const OUString aDpiScaleX = sParams.getToken(1, ';');
                        const OUString aDpiScaleY = sParams.getToken(2, ';');

                        pJSCombobox->render_entry(o3tl::toInt32(aPos), o3tl::toInt32(aDpiScaleX),
                                                  o3tl::toInt32(aDpiScaleY));
                    }
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
                else if (sAction == "toggle")
                {
                    LOKTrigger::trigger_toggled(dynamic_cast<weld::Toggleable&>(*pWidget));
                    return true;
                }
            }
        }
        else if (sControlType == "linkbutton")
        {
            auto pButton = dynamic_cast<weld::LinkButton*>(pWidget);
            if (pButton)
            {
                if (sAction == "click")
                {
                    LOKTrigger::activate_link(*pButton);
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
                else if (sAction == "select")
                {
                    LOKTrigger::trigger_selected(*pButton, rData[u"data"_ustr]);
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
                    bool bChecked = rData[u"data"_ustr] == "true";
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
                if (sAction == "click" || sAction == "dblclick" || sAction == "mousemove"
                    || sAction == "mousedown" || sAction == "mouseup")
                {
                    OUString sClickData = rData[u"data"_ustr];
                    int nSeparatorPos = sClickData.indexOf(';');
                    if (nSeparatorPos > 0)
                    {
                        // x;y
                        std::u16string_view nClickPosX = sClickData.subView(0, nSeparatorPos);
                        std::u16string_view nClickPosY = sClickData.subView(nSeparatorPos + 1);

                        if (nClickPosX.empty() || nClickPosY.empty())
                            return true;

                        double fPosX = o3tl::toDouble(nClickPosX);
                        double fPosY = o3tl::toDouble(nClickPosY);
                        OutputDevice& rRefDevice = pArea->get_ref_device();
                        // We send OutPutSize for the drawing area bitmap
                        // get_size_request is not necessarily updated
                        // therefore it may be incorrect.
                        Size size = rRefDevice.GetOutputSizePixel();
                        fPosX = fPosX * size.Width();
                        fPosY = fPosY * size.Height();

                        if (sAction == "click")
                            LOKTrigger::trigger_click(*pArea, Point(fPosX, fPosY));
                        else if (sAction == "dblclick")
                            LOKTrigger::trigger_dblclick(*pArea, Point(fPosX, fPosY));
                        else if (sAction == "mouseup")
                            LOKTrigger::trigger_mouse_up(*pArea, Point(fPosX, fPosY));
                        else if (sAction == "mousedown")
                            LOKTrigger::trigger_mouse_down(*pArea, Point(fPosX, fPosY));
                        else if (sAction == "mousemove")
                            LOKTrigger::trigger_mouse_move(*pArea, Point(fPosX, fPosY));
                    }

                    return true;
                }
                else if (sAction == "keypress")
                {
                    sal_uInt32 nKeyNo = rData[u"data"_ustr].toUInt32();
                    LOKTrigger::trigger_key_press(*pArea, KeyEvent(nKeyNo, vcl::KeyCode(nKeyNo)));
                    LOKTrigger::trigger_key_release(*pArea, KeyEvent(nKeyNo, vcl::KeyCode(nKeyNo)));
                    return true;
                }
                else if (sAction == "textselection")
                {
                    OUString sTextData = rData[u"data"_ustr];
                    int nSeparatorPos = sTextData.indexOf(';');
                    if (nSeparatorPos <= 0)
                        return true;

                    int nSeparator2Pos = sTextData.indexOf(';', nSeparatorPos + 1);
                    int nSeparator3Pos = 0;

                    if (nSeparator2Pos > 0)
                    {
                        // start;end;startPara;endPara
                        nSeparator3Pos = sTextData.indexOf(';', nSeparator2Pos + 1);
                        if (nSeparator3Pos <= 0)
                            return true;
                    }
                    else
                    {
                        // start;end
                        nSeparator2Pos = 0;
                        nSeparator3Pos = 0;
                    }

                    std::u16string_view aStartPos = sTextData.subView(0, nSeparatorPos);
                    std::u16string_view aEndPos
                        = sTextData.subView(nSeparatorPos + 1, nSeparator2Pos - nSeparatorPos + 1);

                    if (aStartPos.empty() || aEndPos.empty())
                        return true;

                    sal_Int32 nStart = o3tl::toInt32(aStartPos);
                    sal_Int32 nEnd = o3tl::toInt32(aEndPos);
                    sal_Int32 nStartPara = 0;
                    sal_Int32 nEndPara = 0;

                    // multiline case
                    if (nSeparator2Pos && nSeparator3Pos)
                    {
                        std::u16string_view aStartPara = sTextData.subView(
                            nSeparator2Pos + 1, nSeparator3Pos - nSeparator2Pos + 1);
                        std::u16string_view aEndPara = sTextData.subView(nSeparator3Pos + 1);

                        if (aStartPara.empty() || aEndPara.empty())
                            return true;

                        nStartPara = o3tl::toInt32(aStartPara);
                        nEndPara = o3tl::toInt32(aEndPara);
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
                    if (rData[u"data"_ustr] == "undefined")
                        return true;

                    // The Document will not scroll if that is in focus
                    // maybe we could send a message with: sAction == "grab_focus"
                    pWidget->grab_focus();

                    double nValue = o3tl::toDouble(rData[u"data"_ustr]);
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

            auto pFormattedField = dynamic_cast<weld::FormattedSpinButton*>(pWidget);
            if (pFormattedField)
            {
                if (sAction == "change")
                {
                    pFormattedField->set_text(rData[u"data"_ustr]);
                    LOKTrigger::trigger_changed(*pFormattedField);
                    LOKTrigger::trigger_value_changed(*pFormattedField);
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
                    LOKTrigger::trigger_clicked(*pToolbar, rData[u"data"_ustr]);
                    return true;
                }
                else if (sAction == "togglemenu")
                {
                    const OUString& sId = rData[u"data"_ustr];
                    bool bIsActive = pToolbar->get_menu_item_active(sId);
                    pToolbar->set_menu_item_active(sId, !bIsActive);
                    return true;
                }
                else if (sAction == "closemenu")
                {
                    pToolbar->set_menu_item_active(rData[u"data"_ustr], false);
                    return true;
                }
                else if (sAction == "openmenu")
                {
                    pToolbar->set_menu_item_active(rData[u"data"_ustr], true);
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
                    pEdit->set_text_without_notify(rData[u"data"_ustr]);
                    LOKTrigger::trigger_changed(*pEdit);
                    return true;
                }
            }

            auto pTextView = dynamic_cast<JSTextView*>(pWidget);
            if (pTextView)
            {
                if (sAction == "change")
                {
                    int rStartPos, rEndPos;
                    pTextView->get_selection_bounds(rStartPos, rEndPos);
                    pTextView->set_text_without_notify(rData[u"data"_ustr]);
                    pTextView->select_region(rStartPos, rEndPos);
                    LOKTrigger::trigger_changed(*pTextView);
                    return true;
                }
                else if (sAction == "textselection")
                {
                    // start;end
                    OUString sTextData = rData[u"data"_ustr];
                    int nSeparatorPos = sTextData.indexOf(';');
                    if (nSeparatorPos <= 0)
                        return true;

                    std::u16string_view aStartPos = sTextData.subView(0, nSeparatorPos);
                    std::u16string_view aEndPos = sTextData.subView(nSeparatorPos + 1);

                    if (aStartPos.empty() || aEndPos.empty())
                        return true;

                    sal_Int32 nStart = o3tl::toInt32(aStartPos);
                    sal_Int32 nEnd = o3tl::toInt32(aEndPos);

                    pTextView->select_region(nStart, nEnd);
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
                        rData[u"data"_ustr], rtl_UriDecodeMechanism::rtl_UriDecodeWithCharset,
                        RTL_TEXTENCODING_UTF8);
                    StringMap aMap(jsonToStringMap(
                        OUStringToOString(sDataJSON, RTL_TEXTENCODING_ASCII_US).getStr()));

                    sal_Int32 nRow = o3tl::toInt32(aMap[u"row"_ustr]);
                    bool bValue = aMap[u"value"_ustr] == "true";

                    pTreeView->set_toggle(nRow, bValue ? TRISTATE_TRUE : TRISTATE_FALSE);

                    return true;
                }
                else if (sAction == "select")
                {
                    sal_Int32 nAbsPos = o3tl::toInt32(rData[u"data"_ustr]);

                    pTreeView->unselect_all();

                    std::unique_ptr<weld::TreeIter> itEntry(pTreeView->make_iterator());
                    if (pTreeView->get_iter_abs_pos(*itEntry, nAbsPos))
                    {
                        pTreeView->select(*itEntry);
                        pTreeView->set_cursor_without_notify(*itEntry);
                    }
                    else
                        SAL_WARN("vcl",
                                 "No absolute position found for " << nAbsPos << " in treeview");
                    pTreeView->grab_focus();
                    LOKTrigger::trigger_changed(*pTreeView);
                    return true;
                }
                else if (sAction == "activate")
                {
                    sal_Int32 nRow = o3tl::toInt32(rData[u"data"_ustr]);

                    pTreeView->unselect_all();
                    std::unique_ptr<weld::TreeIter> itEntry(pTreeView->make_iterator());
                    if (pTreeView->get_iter_abs_pos(*itEntry, nRow))
                    {
                        pTreeView->select(nRow);
                        pTreeView->set_cursor_without_notify(*itEntry);
                    }
                    else
                        SAL_WARN("vcl",
                                 "No absolute position found for " << nRow << " in treeview");
                    pTreeView->grab_focus();
                    LOKTrigger::trigger_changed(*pTreeView);
                    LOKTrigger::trigger_row_activated(*pTreeView);
                    return true;
                }
                else if (sAction == "expand")
                {
                    sal_Int32 nAbsPos = o3tl::toInt32(rData[u"data"_ustr]);
                    std::unique_ptr<weld::TreeIter> itEntry(pTreeView->make_iterator());
                    if (pTreeView->get_iter_abs_pos(*itEntry, nAbsPos))
                    {
                        pTreeView->set_cursor_without_notify(*itEntry);
                        pTreeView->grab_focus();
                        pTreeView->expand_row(*itEntry);
                    }
                    else
                        SAL_WARN("vcl",
                                 "No absolute position found for " << nAbsPos << " in treeview");
                    return true;
                }
                else if (sAction == "collapse")
                {
                    sal_Int32 nAbsPos = o3tl::toInt32(rData[u"data"_ustr]);
                    std::unique_ptr<weld::TreeIter> itEntry(pTreeView->make_iterator());
                    if (pTreeView->get_iter_abs_pos(*itEntry, nAbsPos))
                    {
                        pTreeView->set_cursor_without_notify(*itEntry);
                        pTreeView->grab_focus();
                        pTreeView->collapse_row(*itEntry);
                    }
                    else
                        SAL_WARN("vcl",
                                 "No absolute position found for " << nAbsPos << " in treeview");
                    return true;
                }
                else if (sAction == "dragstart")
                {
                    sal_Int32 nRow = o3tl::toInt32(rData[u"data"_ustr]);

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
                    sal_Int32 nPos = o3tl::toInt32(rData[u"data"_ustr]);

                    pIconView->select(nPos);
                    LOKTrigger::trigger_changed(*pIconView);

                    return true;
                }
                else if (sAction == "activate")
                {
                    sal_Int32 nPos = o3tl::toInt32(rData[u"data"_ustr]);

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
                    sal_Int32 nResponse = o3tl::toInt32(rData[u"data"_ustr]);
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
                    bool bChecked = rData[u"data"_ustr] == "true";
                    pRadioButton->set_state(bChecked ? TRISTATE_TRUE : TRISTATE_FALSE);
                    LOKTrigger::trigger_toggled(*static_cast<weld::Toggleable*>(pRadioButton));
                    return true;
                }
            }
        }
        else if (sControlType == "scrolledwindow")
        {
            auto pScrolledWindow = dynamic_cast<JSScrolledWindow*>(pWidget);
            if (pScrolledWindow)
            {
                if (sAction == "scrollv")
                {
                    sal_Int32 nValue = o3tl::toInt32(rData[u"data"_ustr]);
                    pScrolledWindow->vadjustment_set_value_no_notification(nValue);
                    LOKTrigger::trigger_scrollv(*pScrolledWindow);
                    return true;
                }
                else if (sAction == "scrollh")
                {
                    sal_Int32 nValue = o3tl::toInt32(rData[u"data"_ustr]);
                    pScrolledWindow->hadjustment_set_value_no_notification(nValue);
                    LOKTrigger::trigger_scrollh(*pScrolledWindow);
                    return true;
                }
            }
        }
        else if (sControlType == "calendar")
        {
            auto pCalendar = dynamic_cast<weld::Calendar*>(pWidget);
            if (pCalendar && sAction == "selectdate")
            {
                // MM/DD/YYYY
                OUString aDate = rData[u"data"_ustr];

                if (aDate.getLength() < 10)
                    return false;

                sal_Int32 aMonth = o3tl::toInt32(aDate.subView(0, 2));
                sal_Int32 aDay = o3tl::toInt32(aDate.subView(3, 2));
                sal_Int32 aYear = o3tl::toInt32(aDate.subView(6, 4));

                pCalendar->set_date(Date(aDay, aMonth, aYear));
                LOKTrigger::trigger_selected(*pCalendar);
                LOKTrigger::trigger_activated(*pCalendar);
                return true;
            }
        }
    }

    return false;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
