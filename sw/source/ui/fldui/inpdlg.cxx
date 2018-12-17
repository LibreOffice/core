/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <comphelper/string.hxx>
#include <unotools/charclass.hxx>
#include <editeng/unolingu.hxx>
#include <wrtsh.hxx>
#include <fldbas.hxx>
#include <expfld.hxx>
#include <usrfld.hxx>
#include <inpdlg.hxx>
#include <fldmgr.hxx>

// edit field-insert
SwFieldInputDlg::SwFieldInputDlg(weld::Window *pParent, SwWrtShell &rS,
                                 SwField* pField, bool bPrevButton, bool bNextButton)
    : GenericDialogController(pParent, "modules/swriter/ui/inputfielddialog.ui", "InputFieldDialog")
    , rSh( rS )
    , pInpField(nullptr)
    , pSetField(nullptr)
    , pUsrType(nullptr)
    , m_pPressedButton(nullptr)
    , m_xLabelED(m_xBuilder->weld_entry("name"))
    , m_xEditED(m_xBuilder->weld_text_view("text"))
    , m_xPrevBT(m_xBuilder->weld_button("prev"))
    , m_xNextBT(m_xBuilder->weld_button("next"))
    , m_xOKBT(m_xBuilder->weld_button("ok"))
{
    m_xEditED->set_size_request(-1, m_xEditED->get_height_rows(8));

    if( bPrevButton || bNextButton )
    {
        m_xPrevBT->show();
        m_xPrevBT->connect_clicked(LINK(this, SwFieldInputDlg, PrevHdl));
        m_xPrevBT->set_sensitive(bPrevButton);

        m_xNextBT->show();
        m_xNextBT->connect_clicked(LINK(this, SwFieldInputDlg, NextHdl));
        m_xNextBT->set_sensitive(bNextButton);
    }

    // evaluation here
    OUString aStr;
    if( SwFieldIds::Input == pField->GetTyp()->Which() )
    {   // it is an input field

        pInpField = static_cast<SwInputField*>(pField);
        m_xLabelED->set_text(pInpField->GetPar2());
        sal_uInt16 nSubType = pInpField->GetSubType();

        switch(nSubType & 0xff)
        {
            case INP_TXT:
                aStr = pInpField->GetPar1();
                break;

            case INP_USR:
                // user field
                if( nullptr != ( pUsrType = static_cast<SwUserFieldType*>(rSh.GetFieldType(
                            SwFieldIds::User, pInpField->GetPar1() ) )  ) )
                    aStr = pUsrType->GetContent();
                break;
        }
    }
    else
    {
        // it is a SetExpression
        pSetField = static_cast<SwSetExpField*>(pField);
        OUString sFormula(pSetField->GetFormula());
        //values are formatted - formulas are not
        CharClass aCC( LanguageTag( pSetField->GetLanguage() ));
        if( aCC.isNumeric( sFormula ))
        {
            aStr = pSetField->ExpandField(true, rS.GetLayout());
        }
        else
            aStr = sFormula;
        m_xLabelED->set_text(pSetField->GetPromptText());
    }

    // JP 31.3.00: Inputfields in readonly regions must be allowed to
    //              input any content. - 74639
    bool bEnable = !rSh.IsCursorReadonly();

    m_xOKBT->set_sensitive( bEnable );
    m_xEditED->set_editable( bEnable );

    if( !aStr.isEmpty() )
        m_xEditED->set_text(convertLineEnd(aStr, GetSystemLineEnd()));
    m_xEditED->grab_focus();
}

SwFieldInputDlg::~SwFieldInputDlg()
{
}

// Close
void SwFieldInputDlg::Apply()
{
    OUString aTmp = m_xEditED->get_text().replaceAll("\r", "");
    rSh.StartAllAction();
    bool bModified = false;
    if(pInpField)
    {
        if(pUsrType)
        {
            if( aTmp != pUsrType->GetContent() )
            {
                pUsrType->SetContent(aTmp);
                pUsrType->UpdateFields();
                bModified = true;
            }
        }
        else if( aTmp != pInpField->GetPar1() )
        {
            pInpField->SetPar1(aTmp);
            rSh.SwEditShell::UpdateOneField(*pInpField);
            bModified = true;
        }
    }
    else if( aTmp != pSetField->GetPar2())
    {
        pSetField->SetPar2(aTmp);
        rSh.SwEditShell::UpdateOneField(*pSetField);
        bModified = true;
    }

    if( bModified )
        rSh.SetUndoNoResetModified();

    rSh.EndAllAction();
}

bool SwFieldInputDlg::PrevButtonPressed() const
{
    return m_pPressedButton == m_xPrevBT.get();
}

bool SwFieldInputDlg::NextButtonPressed() const
{
    return m_pPressedButton == m_xNextBT.get();
}

IMPL_LINK_NOARG(SwFieldInputDlg, PrevHdl, weld::Button&, void)
{
    m_pPressedButton = m_xPrevBT.get();
    m_xDialog->response(RET_OK);
}

IMPL_LINK_NOARG(SwFieldInputDlg, NextHdl, weld::Button&, void)
{
    m_pPressedButton = m_xNextBT.get();
    m_xDialog->response(RET_OK);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
