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

// static ----------------------------------------------------------------

// class SwJavaEditDialog ------------------------------------------------

SwJavaEditDialog::SwJavaEditDialog(Window* pParent, SwWrtShell* pWrtSh) :
    SvxStandardDialog(pParent, "InsertScriptDialog", "modules/swriter/ui/insertscript.ui"),

    bNew(sal_True),
    bIsUrl(sal_False),

    pSh(pWrtSh),
    pFileDlg(NULL),
    pOldDefDlgParent(NULL)
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

    Link aLk = LINK(this, SwJavaEditDialog, RadioButtonHdl);
    m_pUrlRB->SetClickHdl(aLk);
    m_pEditRB->SetClickHdl(aLk);
    m_pUrlPB->SetClickHdl(LINK(this, SwJavaEditDialog, InsertFileHdl));

    Font aFont( m_pEditED->GetFont() );
    aFont.SetWeight( WEIGHT_LIGHT );
    m_pEditED->SetFont( aFont );

    pMgr = new SwFldMgr;
    pFld = (SwScriptField*)pMgr->GetCurFld();

    bNew = !(pFld && pFld->GetTyp()->Which() == RES_SCRIPTFLD);

    CheckTravel();

    if( !bNew )
        SetText( SW_RES( STR_JAVA_EDIT ) );

    RadioButtonHdl(NULL);
}

SwJavaEditDialog::~SwJavaEditDialog()
{
    delete pMgr;
    delete pFileDlg;
    Application::SetDefDialogParent( pOldDefDlgParent );
}

IMPL_LINK_NOARG_INLINE_START(SwJavaEditDialog, PrevHdl)
{
    SetFld();
    pMgr->GoPrev();
    pFld = (SwScriptField*)pMgr->GetCurFld();
    CheckTravel();
    RadioButtonHdl(NULL);

    return 0;
}
IMPL_LINK_NOARG_INLINE_END(SwJavaEditDialog, PrevHdl)

IMPL_LINK_NOARG_INLINE_START(SwJavaEditDialog, NextHdl)
{
    SetFld();
    pMgr->GoNext();
    pFld = (SwScriptField*)pMgr->GetCurFld();
    CheckTravel();
    RadioButtonHdl(NULL);

    return 0;
}
IMPL_LINK_NOARG_INLINE_END(SwJavaEditDialog, NextHdl)

IMPL_LINK_NOARG(SwJavaEditDialog, OKHdl)
{
    SetFld();
    EndDialog( RET_OK );
    return 0;
}

void SwJavaEditDialog::Apply()
{
}

void SwJavaEditDialog::CheckTravel()
{
    sal_Bool bTravel = sal_False;
    sal_Bool bNext(sal_False), bPrev(sal_False);

    if(!bNew)
    {
        // Traveling only when more than one field
        pSh->StartAction();
        pSh->CreateCrsr();

        bNext = pMgr->GoNext();
        if( bNext )
            pMgr->GoPrev();

        if( 0 != ( bPrev = pMgr->GoPrev() ) )
            pMgr->GoNext();
        bTravel |= bNext|bPrev;

        pSh->DestroyCrsr();
        pSh->EndAction();

        if (pFld->IsCodeURL())
        {
            OUString sURL(pFld->GetPar2());
            if(!sURL.isEmpty())
            {
                INetURLObject aINetURL(sURL);
                if(INET_PROT_FILE == aINetURL.GetProtocol())
                    sURL = aINetURL.PathToFileName();
            }
            m_pUrlED->SetText(sURL);
            m_pEditED->SetText(OUString());
            m_pUrlRB->Check();
        }
        else
        {
            m_pEditED->SetText(pFld->GetPar2());
            m_pUrlED->SetText(OUString());
            m_pEditRB->Check();
        }
        m_pTypeED->SetText(pFld->GetPar1());
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

void SwJavaEditDialog::SetFld()
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
    return pFld && ( bIsUrl != pFld->GetFormat() || pFld->GetPar2() != aType || pFld->GetPar1() != aText );
}

IMPL_LINK_NOARG(SwJavaEditDialog, RadioButtonHdl)
{
    sal_Bool bEnable = m_pUrlRB->IsChecked();
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
    return 0;
}

IMPL_LINK( SwJavaEditDialog, InsertFileHdl, PushButton *, pBtn )
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
    return 0;
}

IMPL_LINK_NOARG(SwJavaEditDialog, DlgClosedHdl)
{
    if ( pFileDlg->GetError() == ERRCODE_NONE )
    {
        OUString sFileName = pFileDlg->GetPath();
        if ( !sFileName.isEmpty() )
        {
            INetURLObject aINetURL( sFileName );
            if ( INET_PROT_FILE == aINetURL.GetProtocol() )
                sFileName = aINetURL.PathToFileName();
        }
        m_pUrlED->SetText( sFileName );
    }

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
