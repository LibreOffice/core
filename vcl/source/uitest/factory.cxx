/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "uitest/factory.hxx"
#include "uitest/uiobject_impl.hxx"

#include <vcl/tabpage.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/combobox.hxx>
#include <vcl/spin.hxx>
#include <vcl/spinfld.hxx>
#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <vcl/edit.hxx>

std::unique_ptr<UIObject> UITestWrapperFactory::createObject(vcl::Window* pWindow)
{
    if (!pWindow)
        return nullptr;

    switch (pWindow->GetType())
    {
        case WINDOW_BUTTON:
        case WINDOW_PUSHBUTTON:
        case WINDOW_OKBUTTON:
        case WINDOW_CANCELBUTTON:
        case WINDOW_HELPBUTTON:
        {
            Button* pButton = dynamic_cast<Button*>(pWindow);
            assert(pButton);
            return std::unique_ptr<UIObject>(new ButtonUIObject(pButton));
        }
        break;
        case WINDOW_MESSBOX:
        case WINDOW_INFOBOX:
        case WINDOW_WARNINGBOX:
        case WINDOW_ERRORBOX:
        case WINDOW_QUERYBOX:
        case WINDOW_DIALOG:
        case WINDOW_MODALDIALOG:
        case WINDOW_MODELESSDIALOG:
        case WINDOW_SYSTEMDIALOG:
        case WINDOW_PATHDIALOG:
        case WINDOW_FILEDIALOG:
        case WINDOW_PRINTERSETUPDIALOG:
        case WINDOW_PRINTDIALOG:
        case WINDOW_COLORDIALOG:
        case WINDOW_FONTDIALOG:
        case WINDOW_TABDIALOG:
        {
            Dialog* pDialog = dynamic_cast<Dialog*>(pWindow);
            assert(pDialog);
            return std::unique_ptr<UIObject>(new DialogUIObject(pDialog));
        }
        break;
        case WINDOW_EDIT:
        case WINDOW_MULTILINEEDIT:
        {
            Edit* pEdit = dynamic_cast<Edit*>(pWindow);
            assert(pEdit);
            return std::unique_ptr<UIObject>(new EditUIObject(pEdit));
        }
        break;
        case WINDOW_CHECKBOX:
        {
            CheckBox* pCheckBox = dynamic_cast<CheckBox*>(pWindow);
            assert(pCheckBox);
            return std::unique_ptr<UIObject>(new CheckBoxUIObject(pCheckBox));
        }
        break;
        case WINDOW_COMBOBOX:
        {
            ComboBox* pComboBox = dynamic_cast<ComboBox*>(pWindow);
            assert(pComboBox);
            return std::unique_ptr<UIObject>(new ComboBoxUIObject(pComboBox));
        }
        break;
        case WINDOW_LISTBOX:
        {
            ListBox* pListBox = dynamic_cast<ListBox*>(pWindow);
            assert(pListBox);
            return std::unique_ptr<UIObject>(new ListBoxUIObject(pListBox));
        }
        break;
        case WINDOW_TABPAGE:
        {
            //TabPage* pTabPage = dynamic_cast<TabPage*>(pWindow);
            //assert(pTabPage);
         //   return std::unique_ptr<UIObject>(new TabPageUIObject(pTabPage));
        }
        break;
        case WINDOW_SPINBUTTON:
        {
            SpinButton* pSpinButton = dynamic_cast<SpinButton*>(pWindow);
            assert(pSpinButton);
            return std::unique_ptr<UIObject>(new SpinUIObject(pSpinButton));
        }
        break;
        case WINDOW_SPINFIELD:
        case WINDOW_PATTERNFIELD:
        case WINDOW_NUMERICFIELD:
        case WINDOW_METRICFIELD:
        case WINDOW_CURRENCYFIELD:
        case WINDOW_DATEFIELD:
        case WINDOW_TIMEFIELD:
        {
            SpinField* pSpinField = dynamic_cast<SpinField*>(pWindow);
            assert(pSpinField);
            return std::unique_ptr<UIObject>(new SpinFieldUIObject(pSpinField));
        }
        break;
        default:
        break;
    }

    return std::unique_ptr<UIObject>(new WindowUIObject(pWindow));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
