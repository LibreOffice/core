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

#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <hintids.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/svapp.hxx>
#include <svl/urihelper.hxx>
#include <view.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/filedlghelper.hxx>
#include <docsh.hxx>
#include <wrtsh.hxx>
#include <fldbas.hxx>
#include <fldmgr.hxx>
#include <docufld.hxx>
#include <uitool.hxx>
#include <javaedit.hxx>

#include <fldui.hrc>
#include <globals.hrc>

using namespace ::com::sun::star;

SwJavaEditDialog::SwJavaEditDialog(vcl::Window* pParent, SwWrtShell* pWrtSh) :
    SvxStandardDialog(pParent, "InsertScriptDialog", "modules/swriter/ui/insertscript.ui"),

    bNew(true),
    bIsUrl(false),

    pSh(pWrtSh),
    pFileDlg(nullptr),
    pOldDefDlgParent(nullptr)
{
    get(m_pTypeED, "scripttype");
    get(m_pUrlRB, "url");
    get(m_pUrlED, "urlentry");
    get(m_pUrlPB, "browse");
    get(m_pEditRB, "text");
    get(m_pEditED, "textentry");

    get(m_pOKBtn, "ok");
    get(m_pPrevBtn, "previous");
    get(m_pNextBtn, "next");

    // install handler
    m_pPrevBtn->SetClickHdl( LINK( this, SwJavaEditDialog, PrevHdl ) );
    m_pNextBtn->SetClickHdl( LINK( this, SwJavaEditDialog, NextHdl ) );
    m_pOKBtn->SetClickHdl( LINK( this, SwJavaEditDialog, OKHdl ) );

    Link<Button*,void> aLk = LINK(this, SwJavaEditDialog, RadioButtonHdl);
    m_pUrlRB->SetClickHdl(aLk);
    m_pEditRB->SetClickHdl(aLk);
    m_pUrlPB->SetClickHdl(LINK(this, SwJavaEditDialog, InsertFileHdl));

    vcl::Font aFont( m_pEditED->GetFont() );
    aFont.SetWeight( WEIGHT_LIGHT );
    m_pEditED->SetFont( aFont );

    pMgr = new SwFieldMgr(pSh);
    pField = static_cast<SwScriptField*>(pMgr->GetCurField());

    bNew = !(pField && pField->GetTyp()->Which() == RES_SCRIPTFLD);

    CheckTravel();

    if( !bNew )
        SetText( SW_RES( STR_JAVA_EDIT ) );

    RadioButtonHdl(nullptr);
}

SwJavaEditDialog::~SwJavaEditDialog()
{
    disposeOnce();
}

void SwJavaEditDialog::dispose()
{
    pSh->EnterStdMode();
    delete pMgr;
    delete pFileDlg;
    Application::SetDefDialogParent( pOldDefDlgParent );
    m_pTypeED.clear();
    m_pUrlRB.clear();
    m_pEditRB.clear();
    m_pUrlPB.clear();
    m_pUrlED.clear();
    m_pEditED.clear();
    m_pOKBtn.clear();
    m_pPrevBtn.clear();
    m_pNextBtn.clear();
    pOldDefDlgParent.clear();
    SvxStandardDialog::dispose();
}

IMPL_LINK_NOARG_TYPED(SwJavaEditDialog, PrevHdl, Button*, void)
{
    pSh->EnterStdMode();

    SetField();
    pMgr->GoPrev();
    pField = static_cast<SwScriptField*>(pMgr->GetCurField());
    CheckTravel();
    RadioButtonHdl(nullptr);
}

IMPL_LINK_NOARG_TYPED(SwJavaEditDialog, NextHdl, Button*, void)
{
    pSh->EnterStdMode();

    SetField();
    pMgr->GoNext();
    pField = static_cast<SwScriptField*>(pMgr->GetCurField());
    CheckTravel();
    RadioButtonHdl(nullptr);
}

IMPL_LINK_NOARG_TYPED(SwJavaEditDialog, OKHdl, Button*, void)
{
    SetField();
    EndDialog( RET_OK );
}

void SwJavaEditDialog::Apply()
{
}

void SwJavaEditDialog::CheckTravel()
{
    bool bTravel = false;
    bool bNext(false), bPrev(false);

    if(!bNew)
    {
        // Traveling only when more than one field
        pSh->StartAction();
        pSh->CreateCrsr();

        bNext = pMgr->GoNext();
        if( bNext )
            pMgr->GoPrev();

        if( ( bPrev = pMgr->GoPrev() ) )
            pMgr->GoNext();
        bTravel |= bNext || bPrev;

        pSh->DestroyCrsr();
        pSh->EndAction();

        if (pField->IsCodeURL())
        {
            OUString sURL(pField->GetPar2());
            if(!sURL.isEmpty())
            {
                INetURLObject aINetURL(sURL);
                if(INetProtocol::File == aINetURL.GetProtocol())
                    sURL = aINetURL.PathToFileName();
            }
            m_pUrlED->SetText(sURL);
            m_pEditED->SetText(OUString());
            m_pUrlRB->Check();
        }
        else
        {
            m_pEditED->SetText(pField->GetPar2());
            m_pUrlED->SetText(OUString());
            m_pEditRB->Check();
        }
        m_pTypeED->SetText(pField->GetPar1());
    }

    if ( !bTravel )
    {
        m_pPrevBtn->Hide();
        m_pNextBtn->Hide();
    }
    else
    {
        m_pPrevBtn->Enable(bPrev);
        m_pNextBtn->Enable(bNext);
    }
}

void SwJavaEditDialog::SetField()
{
    if( !m_pOKBtn->IsEnabled() )
        return ;

    aType = m_pTypeED->GetText();
    bIsUrl = m_pUrlRB->IsChecked();

    if( bIsUrl )
    {
        aText = m_pUrlED->GetText();
        if (!aText.isEmpty())
        {
            SfxMedium* pMedium = pSh->GetView().GetDocShell()->GetMedium();
            INetURLObject aAbs;
            if( pMedium )
                aAbs = pMedium->GetURLObject();

            aText = URIHelper::SmartRel2Abs(
                aAbs, aText, URIHelper::GetMaybeFileHdl());
        }
    }
    else
        aText = m_pEditED->GetText();

    if( aType.isEmpty() )
        aType = "JavaScript";
}

bool SwJavaEditDialog::IsUpdate() const
{
    return pField && ( sal_uInt32(bIsUrl ? 1 : 0) != pField->GetFormat() || pField->GetPar2() != aType || pField->GetPar1() != aText );
}

IMPL_LINK_NOARG_TYPED(SwJavaEditDialog, RadioButtonHdl, Button*, void)
{
    bool bEnable = m_pUrlRB->IsChecked();
    m_pUrlPB->Enable(bEnable);
    m_pUrlED->Enable(bEnable);
    m_pEditED->Enable(!bEnable);

    if( !bNew )
    {
        bEnable = !pSh->IsReadOnlyAvailable() || !pSh->HasReadonlySel();
        m_pOKBtn->Enable( bEnable );
        m_pUrlED->SetReadOnly( !bEnable );
        m_pEditED->SetReadOnly( !bEnable);
        m_pTypeED->SetReadOnly( !bEnable);
        if( m_pUrlPB->IsEnabled() && !bEnable )
            m_pUrlPB->Enable( false );
    }
}

IMPL_LINK_TYPED( SwJavaEditDialog, InsertFileHdl, Button *, pBtn, void )
{
    if ( !pFileDlg )
    {
        pOldDefDlgParent = Application::GetDefDialogParent();
        Application::SetDefDialogParent( pBtn );

        pFileDlg = new ::sfx2::FileDialogHelper(
            ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE,
            SFXWB_INSERT, OUString("swriter") );
    }

    pFileDlg->StartExecuteModal( LINK( this, SwJavaEditDialog, DlgClosedHdl ) );
}

IMPL_LINK_NOARG_TYPED(SwJavaEditDialog, DlgClosedHdl, sfx2::FileDialogHelper *, void)
{
    if ( pFileDlg->GetError() == ERRCODE_NONE )
    {
        OUString sFileName = pFileDlg->GetPath();
        if ( !sFileName.isEmpty() )
        {
            INetURLObject aINetURL( sFileName );
            if ( INetProtocol::File == aINetURL.GetProtocol() )
                sFileName = aINetURL.PathToFileName();
        }
        m_pUrlED->SetText( sFileName );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
