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

namespace jsdialog
{
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
                    if (separatorPos)
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
    }

    return false;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
