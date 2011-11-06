/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#ifdef SW_DLLIMPLEMENTATION
#undef SW_DLLIMPLEMENTATION
#endif

// include ---------------------------------------------------------------



#define _JAVAEDIT_CXX
#include <hintids.hxx>

#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#include <vcl/svapp.hxx>
#include <svl/urihelper.hxx>
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#include <sfx2/docfile.hxx>
#include <sfx2/filedlghelper.hxx>
#include <docsh.hxx>
#include <wrtsh.hxx>
#include <fldbas.hxx>
#include <fldmgr.hxx>
#include <docufld.hxx>
#include <uitool.hxx>
#ifndef _JAVAEDIT_HXX
#include <javaedit.hxx>
#endif

#ifndef _FLDUI_HRC
#include <fldui.hrc>
#endif
#ifndef _JAVAEDIT_HRC
#include <javaedit.hrc>
#endif

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
    // Handler installieren
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
        // neu anlegen
        SetText( SW_RES( STR_JAVA_INSERT ) );

    FreeResource();

    RadioButtonHdl(NULL);
}

/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/



SwJavaEditDialog::~SwJavaEditDialog()
{
    delete pMgr;
    delete pFileDlg;
    Application::SetDefDialogParent( pOldDefDlgParent );
}

/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/



IMPL_LINK_INLINE_START( SwJavaEditDialog, PrevHdl, Button *, EMPTYARG )
{
    SetFld();
    pMgr->GoPrev();
    pFld = (SwScriptField*)pMgr->GetCurFld();
    CheckTravel();
    RadioButtonHdl(NULL);

    return 0;
}
IMPL_LINK_INLINE_END( SwJavaEditDialog, PrevHdl, Button *, EMPTYARG )

/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/



IMPL_LINK_INLINE_START( SwJavaEditDialog, NextHdl, Button *, EMPTYARG )
{
    SetFld();
    pMgr->GoNext();
    pFld = (SwScriptField*)pMgr->GetCurFld();
    CheckTravel();
    RadioButtonHdl(NULL);

    return 0;
}
IMPL_LINK_INLINE_END( SwJavaEditDialog, NextHdl, Button *, EMPTYARG )

/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/



IMPL_LINK( SwJavaEditDialog, OKHdl, Button *, EMPTYARG )
{
    SetFld();
    EndDialog( RET_OK );
    return 0;
}

/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/



void SwJavaEditDialog::Apply()
{
}

/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/



void SwJavaEditDialog::CheckTravel()
{
    sal_Bool bTravel = sal_False;
    sal_Bool bNext(sal_False), bPrev(sal_False);

    if(!bNew)
    {
        // Traveling nur bei mehr als einem Feld
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

/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/



void SwJavaEditDialog::SetFld()
{
    if( !aOKBtn.IsEnabled() )
        return ;

    aType = aTypeED.GetText();
    bIsUrl = aUrlRB.IsChecked();

    if( bIsUrl )
    {
        aText = aUrlED.GetText();
        if(aText.Len())
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

    if( !aType.Len() )
        aType = String::CreateFromAscii("JavaScript");
}

sal_Bool SwJavaEditDialog::IsUpdate()
{
    return pFld && ( bIsUrl != pFld->GetFormat() || pFld->GetPar2() != aType || pFld->GetPar1() != aText );
}

/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/

IMPL_LINK( SwJavaEditDialog, RadioButtonHdl, RadioButton *, EMPTYARG )
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

/***************************************************************************
    Beschreibung:
***************************************************************************/

IMPL_LINK( SwJavaEditDialog, InsertFileHdl, PushButton *, pBtn )
{
    if ( !pFileDlg )
    {
        pOldDefDlgParent = Application::GetDefDialogParent();
        Application::SetDefDialogParent( pBtn );

        pFileDlg = new ::sfx2::FileDialogHelper(
            (SFXWB_INSERT | WB_3DLOOK), String::CreateFromAscii("swriter") );
    }

    pFileDlg->StartExecuteModal( LINK( this, SwJavaEditDialog, DlgClosedHdl ) );
    return 0;
}

IMPL_LINK( SwJavaEditDialog, DlgClosedHdl, sfx2::FileDialogHelper *, EMPTYARG )
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

