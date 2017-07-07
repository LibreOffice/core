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
#include <vcl/msgbox.hxx>
#include <unotools/charclass.hxx>
#include <editeng/unolingu.hxx>
#include <wrtsh.hxx>
#include <fldbas.hxx>
#include <expfld.hxx>
#include <usrfld.hxx>
#include <inpdlg.hxx>
#include <fldmgr.hxx>

#include <fldui.hrc>

// edit field-insert
SwFieldInputDlg::SwFieldInputDlg( vcl::Window *pParent, SwWrtShell &rS,
                              SwField* pField, bool bNextButton )
    : SvxStandardDialog( pParent, "InputFieldDialog",
        "modules/swriter/ui/inputfielddialog.ui")
    , rSh( rS )
    , pInpField(nullptr)
    , pSetField(nullptr)
    , pUsrType(nullptr)
{
    get(m_pLabelED, "name");
    get(m_pEditED, "text");
    m_pEditED->set_height_request(m_pEditED->GetTextHeight() * 9);
    get(m_pNextBT, "next");
    get(m_pOKBT, "ok");
    // switch font for Edit
    vcl::Font aFont(m_pEditED->GetFont());
    aFont.SetWeight(WEIGHT_LIGHT);
    m_pEditED->SetFont(aFont);

    if( bNextButton )
    {
        m_pNextBT->Show();
        m_pNextBT->SetClickHdl(LINK(this, SwFieldInputDlg, NextHdl));
    }

    // evaluation here
    OUString aStr;
    if( SwFieldIds::Input == pField->GetTyp()->Which() )
    {   // it is an input field

        pInpField = static_cast<SwInputField*>(pField);
        m_pLabelED->SetText( pInpField->GetPar2() );
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
            aStr = pSetField->ExpandField(true);
        }
        else
            aStr = sFormula;
        m_pLabelED->SetText( pSetField->GetPromptText() );
    }

    // JP 31.3.00: Inputfields in readonly regions must be allowed to
    //              input any content. - 74639
    bool bEnable = !rSh.IsCursorReadonly();

    m_pOKBT->Enable( bEnable );
    m_pEditED->SetReadOnly( !bEnable );

    if( !aStr.isEmpty() )
        m_pEditED->SetText(convertLineEnd(aStr, GetSystemLineEnd()));
}

SwFieldInputDlg::~SwFieldInputDlg()
{
    disposeOnce();
}

void SwFieldInputDlg::dispose()
{
    m_pLabelED.clear();
    m_pEditED.clear();
    m_pOKBT.clear();
    m_pNextBT.clear();
    SvxStandardDialog::dispose();
}

void SwFieldInputDlg::StateChanged( StateChangedType nType )
{
    if ( nType == StateChangedType::InitShow )
        m_pEditED->GrabFocus();
    SvxStandardDialog::StateChanged( nType );
}

// Close
void SwFieldInputDlg::Apply()
{
    OUString aTmp = m_pEditED->GetText().replaceAll("\r", "");
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
            rSh.SwEditShell::UpdateFields(*pInpField);
            bModified = true;
        }
    }
    else if( aTmp != pSetField->GetPar2())
    {
        pSetField->SetPar2(aTmp);
        rSh.SwEditShell::UpdateFields(*pSetField);
        bModified = true;
    }

    if( bModified )
        rSh.SetUndoNoResetModified();

    rSh.EndAllAction();
}

IMPL_LINK_NOARG(SwFieldInputDlg, NextHdl, Button*, void)
{
    EndDialog(RET_OK);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
