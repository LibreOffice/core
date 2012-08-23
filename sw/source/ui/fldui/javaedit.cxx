/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
#include <javaedit.hrc>


using namespace ::com::sun::star;

// static ----------------------------------------------------------------

// class SwJavaEditDialog ------------------------------------------------



SwJavaEditDialog::SwJavaEditDialog(Window* pParent, SwWrtShell* pWrtSh) :

    SvxStandardDialog(pParent, SW_RES(DLG_JAVAEDIT)),

    aTypeFT         ( this, SW_RES( FT_TYPE ) ),
    aTypeED         ( this, SW_RES( ED_TYPE ) ),
    aUrlRB          ( this, SW_RES( RB_URL ) ),
    aEditRB         ( this, SW_RES( RB_EDIT ) ),
    aUrlPB          ( this, SW_RES( PB_URL ) ),
    aUrlED          ( this, SW_RES( ED_URL ) ),
    aEditED         ( this, SW_RES( ED_EDIT ) ),
    aPostItFL       ( this, SW_RES( FL_POSTIT ) ),

    aOKBtn          ( this, SW_RES( BTN_POST_OK ) ),
    aCancelBtn      ( this, SW_RES( BTN_POST_CANCEL ) ),
    aPrevBtn        ( this, SW_RES( BTN_PREV ) ),
    aNextBtn        ( this, SW_RES( BTN_NEXT ) ),
    aHelpBtn        ( this, SW_RES( BTN_POST_HELP ) ),

    bNew(sal_True),
    bIsUrl(sal_False),

    pSh(pWrtSh),
    pFileDlg(NULL),
    pOldDefDlgParent(NULL)
{
    // install handler
    aPrevBtn.SetClickHdl( LINK( this, SwJavaEditDialog, PrevHdl ) );
    aNextBtn.SetClickHdl( LINK( this, SwJavaEditDialog, NextHdl ) );
    aOKBtn.SetClickHdl( LINK( this, SwJavaEditDialog, OKHdl ) );

    Link aLk = LINK(this, SwJavaEditDialog, RadioButtonHdl);
    aUrlRB.SetClickHdl(aLk);
    aEditRB.SetClickHdl(aLk);
    aUrlPB.SetClickHdl(LINK(this, SwJavaEditDialog, InsertFileHdl));

    Font aFont( aEditED.GetFont() );
    aFont.SetWeight( WEIGHT_LIGHT );
    aEditED.SetFont( aFont );

    pMgr = new SwFldMgr;
    pFld = (SwScriptField*)pMgr->GetCurFld();

    bNew = !(pFld && pFld->GetTyp()->Which() == RES_SCRIPTFLD);

    CheckTravel();

    if( !bNew )
        SetText( SW_RES( STR_JAVA_EDIT ) );
    else
        // newly create
        SetText( SW_RES( STR_JAVA_INSERT ) );

    FreeResource();

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
            String sURL(pFld->GetPar2());
            if(sURL.Len())
            {
                INetURLObject aINetURL(sURL);
                if(INET_PROT_FILE == aINetURL.GetProtocol())
                    sURL = aINetURL.PathToFileName();
            }
            aUrlED.SetText(sURL);
            aEditED.SetText(aEmptyStr);
            aUrlRB.Check();
        }
        else
        {
            aEditED.SetText(pFld->GetPar2());
            aUrlED.SetText(aEmptyStr);
            aEditRB.Check();
        }
        aTypeED.SetText(pFld->GetPar1());
    }

    if ( !bTravel )
    {
        aPrevBtn.Hide();
        aNextBtn.Hide();
    }
    else
    {
        aPrevBtn.Enable(bPrev);
        aNextBtn.Enable(bNext);
    }
}

void SwJavaEditDialog::SetFld()
{
    if( !aOKBtn.IsEnabled() )
        return ;

    aType = aTypeED.GetText();
    bIsUrl = aUrlRB.IsChecked();

    if( bIsUrl )
    {
        aText = aUrlED.GetText();
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
        aText = aEditED.GetText();

    if( aType.isEmpty() )
        aType = "JavaScript";
}

sal_Bool SwJavaEditDialog::IsUpdate()
{
    return pFld && ( bIsUrl != pFld->GetFormat() || pFld->GetPar2() != aType || pFld->GetPar1() != aText );
}

IMPL_LINK_NOARG(SwJavaEditDialog, RadioButtonHdl)
{
    sal_Bool bEnable = aUrlRB.IsChecked();
    aUrlPB.Enable(bEnable);
    aUrlED.Enable(bEnable);
    aEditED.Enable(!bEnable);

    if( !bNew )
    {
        bEnable = !pSh->IsReadOnlyAvailable() || !pSh->HasReadonlySel();
        aOKBtn.Enable( bEnable );
        aUrlED.SetReadOnly( !bEnable );
        aEditED.SetReadOnly( !bEnable);
        aTypeED.SetReadOnly( !bEnable);
        if( aUrlPB.IsEnabled() && !bEnable )
            aUrlPB.Enable( sal_False );
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
            SFXWB_INSERT, rtl::OUString("swriter") );
    }

    pFileDlg->StartExecuteModal( LINK( this, SwJavaEditDialog, DlgClosedHdl ) );
    return 0;
}

IMPL_LINK_NOARG(SwJavaEditDialog, DlgClosedHdl)
{
    if ( pFileDlg->GetError() == ERRCODE_NONE )
    {
        String sFileName = pFileDlg->GetPath();
        if ( sFileName.Len() > 0 )
        {
            INetURLObject aINetURL( sFileName );
            if ( INET_PROT_FILE == aINetURL.GetProtocol() )
                sFileName = aINetURL.PathToFileName();
        }
        aUrlED.SetText( sFileName );
    }

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
