/*************************************************************************
 *
 *  $RCSfile: highred.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:02 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

// System - Includes ---------------------------------------------------------

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#ifndef PCH
#include <segmentc.hxx>
#endif



// INCLUDE -------------------------------------------------------------------

#include "global.hxx"
#include "reffact.hxx"
#include "document.hxx"
#include "docsh.hxx"
#include "scresid.hxx"
#include "globstr.hrc"
#include "highred.hrc"

#include "highred.hxx"

#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif

// defines -------------------------------------------------------------------

#define ABS_SREF          SCA_VALID \
                        | SCA_COL_ABSOLUTE | SCA_ROW_ABSOLUTE | SCA_TAB_ABSOLUTE
#define ABS_DREF          ABS_SREF \
                        | SCA_COL2_ABSOLUTE | SCA_ROW2_ABSOLUTE | SCA_TAB2_ABSOLUTE
#define ABS_SREF3D      ABS_SREF | SCA_TAB_3D
#define ABS_DREF3D      ABS_DREF | SCA_TAB_3D



#define ERRORBOX(s) ErrorBox(this,WinBits(WB_OK|WB_DEF_OK),s).Execute();

inline void EnableDisable( Window& rWin, BOOL bEnable )
{
    if (bEnable)
        rWin.Enable();
    else
        rWin.Disable();
}

//============================================================================
//  class ScHighlightChgDlg

//----------------------------------------------------------------------------
ScHighlightChgDlg::ScHighlightChgDlg( SfxBindings* pB, SfxChildWindow* pCW, Window* pParent,
                      ScViewData*       ptrViewData)

    :   ScAnyRefDlg ( pB, pCW, pParent, RID_SCDLG_HIGHLIGHT_CHANGES ),
        //
        aFtAssign       ( this, ResId( FT_ASSIGN ) ),
        aEdAssign       ( this, ResId( ED_ASSIGN ) ),
        aRbAssign       ( this, ResId( RB_ASSIGN ), &aEdAssign ),
        aHighlightBox   ( this, ResId( CB_HIGHLIGHT)),
        aGroupBox       ( this, ResId( GB_REDLINING)),
        aCbAccept       ( this, ResId( CB_HIGHLIGHT_ACCEPT)),
        aCbReject       ( this, ResId( CB_HIGHLIGHT_REJECT)),
        aHighlightFrame ( this, ResId( GB_TO_HIGHLIGHT    )),

        aOkButton       ( this, ResId( BTN_OK ) ),
        aCancelButton   ( this, ResId( BTN_CANCEL ) ),
        aHelpButton     ( this, ResId( BTN_HELP ) ),
        //
        aFilterCtr      ( this),
        pViewData       ( ptrViewData ),
        pDoc            ( ptrViewData->GetDocument() ),
        aLocalRangeName ( *(pDoc->GetRangeName()) )
{
    FreeResource();

    Point aPos=aGroupBox.GetPosPixel();
    aFilterCtr.SetPosSizePixel(aPos,aGroupBox.GetSizePixel());
    aGroupBox.Hide();
    MinSize=aFilterCtr.GetSizePixel();
    MinSize.Height()+=2;
    MinSize.Width()+=2;
    aOkButton.SetClickHdl(LINK( this, ScHighlightChgDlg, OKBtnHdl));
    aHighlightBox.SetClickHdl(LINK( this, ScHighlightChgDlg, HighLightHandle ));
    aFilterCtr.SetRefHdl(LINK( this, ScHighlightChgDlg, RefHandle ));
    aFilterCtr.HideRange(FALSE);
    aFilterCtr.HideGroupBox();
    aFilterCtr.Show();
    SFX_APP()->LockDispatcher( TRUE);
    //SFX_APPWINDOW->Disable(FALSE);

    Init();

}
ScHighlightChgDlg::~ScHighlightChgDlg()
{
    SFX_APP()->LockDispatcher( FALSE);
    //SFX_APPWINDOW->Enable();
}

void __EXPORT ScHighlightChgDlg::Init()
{
    String  aAreaStr;
    ScRange aRange;

    DBG_ASSERT( pViewData && pDoc, "ViewData oder Document nicht gefunden!" );

    ScChangeTrack* pChanges=pDoc->GetChangeTrack();
    if(pChanges!=NULL)
    {
        aChangeViewSet.SetTheAuthorToShow(pChanges->GetUser());
        aFilterCtr.ClearAuthors();
        StrCollection aUserColl=pChanges->GetUserCollection();
        for(USHORT  i=0;i<aUserColl.GetCount();i++)
            aFilterCtr.InsertAuthor(aUserColl[i]->GetString());
    }


    ScChangeViewSettings* pViewSettings=pDoc->GetChangeViewSettings();

    if(pViewSettings!=NULL)
        aChangeViewSet=*pViewSettings;
    aHighlightBox.Check(aChangeViewSet.ShowChanges());
    aFilterCtr.CheckDate(aChangeViewSet.HasDate());
    aFilterCtr.SetFirstDate(aChangeViewSet.GetTheFirstDateTime());
    aFilterCtr.SetFirstTime(aChangeViewSet.GetTheFirstDateTime());
    aFilterCtr.SetLastDate(aChangeViewSet.GetTheLastDateTime());
    aFilterCtr.SetLastTime(aChangeViewSet.GetTheLastDateTime());
    aFilterCtr.SetDateMode((USHORT)aChangeViewSet.GetTheDateMode());
    aFilterCtr.CheckAuthor(aChangeViewSet.HasAuthor());
    aFilterCtr.CheckComment(aChangeViewSet.HasComment());
    aFilterCtr.SetComment(aChangeViewSet.GetTheComment());

    aCbAccept.Check(aChangeViewSet.IsShowAccepted());
    aCbReject.Check(aChangeViewSet.IsShowRejected());

    String aString=aChangeViewSet.GetTheAuthorToShow();
    if(aString.Len()!=0)
    {
        aFilterCtr.SelectAuthor(aString);
    }
    else
    {
        aFilterCtr.SelectedAuthorPos(0);
    }

    aFilterCtr.CheckRange(aChangeViewSet.HasRange());
    ScRange* pRangeEntry=aChangeViewSet.GetTheRangeList().GetObject(0);


    if(pRangeEntry!=NULL)
    {
        String aRefStr;
        pRangeEntry->Format( aRefStr, ABS_DREF3D, pDoc );
        aFilterCtr.SetRange(aRefStr);
    }
    aFilterCtr.Enable(TRUE,TRUE);
    HighLightHandle(&aHighlightBox);
}

//----------------------------------------------------------------------------
// Uebergabe eines mit der Maus selektierten Tabellenbereiches, der dann als
// neue Selektion im Referenz-Edit angezeigt wird.

void ScHighlightChgDlg::SetReference( const ScRange& rRef, ScDocument* pDoc )
{
    if ( aEdAssign.IsVisible() )
    {
        if ( rRef.aStart != rRef.aEnd )
            RefInputStart(&aEdAssign);
        String aRefStr;
        rRef.Format( aRefStr, ABS_DREF3D, pDoc );
        aEdAssign.SetRefString( aRefStr );
        aFilterCtr.SetRange(aRefStr);
    }
}

//----------------------------------------------------------------------------
BOOL __EXPORT ScHighlightChgDlg::Close()
{
    return DoClose( ScHighlightChgDlgWrapper::GetChildWindowId() );
}

void ScHighlightChgDlg::RefInputDone( BOOL bForced)
{
    ScAnyRefDlg::RefInputDone(bForced);
    if(bForced || !aRbAssign.IsVisible())
    {
        aFilterCtr.SetRange(aEdAssign.GetText());
        aFilterCtr.SetFocusToRange();
        aEdAssign.Hide();
        aRbAssign.Hide();
    }
}

void ScHighlightChgDlg::SetActive()
{
    /*
    if(pTPFilter!=NULL)
    {
        aAcceptChgCtr.GetFilterPage()->SetFocusToRange();
        aEdAssign.Hide();
        aRbAssign.Hide();
        SFX_APPWINDOW->Enable();
        SFX_APP()->LockDispatcher( FALSE);
    }
    //RefInputDone();
    */
}

BOOL ScHighlightChgDlg::IsRefInputMode() const
{
    return aEdAssign.IsVisible();
}

IMPL_LINK( ScHighlightChgDlg, HighLightHandle, CheckBox*, pCb )
{
    if(pCb!=NULL)
    {
        if(aHighlightBox.IsChecked())
        {
            aFilterCtr.Enable(TRUE,TRUE);
            aCbAccept.Enable();
            aCbReject.Enable();
        }
        else
        {
            aFilterCtr.Disable(TRUE);
            aCbAccept.Disable();
            aCbReject.Disable();
        }
    }
    return 0;
}

IMPL_LINK( ScHighlightChgDlg, RefHandle, SvxTPFilter*, pRef )
{
    if(pRef!=NULL)
    {
        SFX_APP()->LockDispatcher( TRUE );
        //SFX_APPWINDOW->Disable(FALSE);
        aEdAssign.Show();
        aRbAssign.Show();
        aEdAssign.SetText(aFilterCtr.GetRange());
        ScAnyRefDlg::RefInputStart(&aEdAssign,&aRbAssign);
    }
    return 0;
}

IMPL_LINK( ScHighlightChgDlg, OKBtnHdl, PushButton*, pOKBtn )
{
    if ( pOKBtn == &aOkButton)
    {
        aChangeViewSet.SetShowChanges(aHighlightBox.IsChecked());
        aChangeViewSet.SetHasDate(aFilterCtr.IsDate());
        ScChgsDateMode eMode = (ScChgsDateMode) aFilterCtr.GetDateMode();
        aChangeViewSet.SetTheDateMode( eMode );
        Date aFirstDate( aFilterCtr.GetFirstDate() );
        Time aFirstTime( aFilterCtr.GetFirstTime() );
        Date aLastDate( aFilterCtr.GetLastDate() );
        Time aLastTime( aFilterCtr.GetLastTime() );
        switch ( eMode )
        {   // korrespondiert mit ScViewUtil::IsActionShown
            case SCDM_DATE_EQUAL :
            case SCDM_DATE_NOTEQUAL :
                aFirstTime.SetTime( 0 );
                aLastDate = aFirstDate;
                aLastTime.SetTime( 23595999 );
            break;
        }
        aChangeViewSet.SetTheFirstDateTime( DateTime( aFirstDate, aFirstTime ) );
        aChangeViewSet.SetTheLastDateTime( DateTime( aLastDate, aLastTime ) );
        aChangeViewSet.SetHasAuthor(aFilterCtr.IsAuthor());
        aChangeViewSet.SetTheAuthorToShow(aFilterCtr.GetSelectedAuthor());
        aChangeViewSet.SetHasRange(aFilterCtr.IsRange());
        aChangeViewSet.SetShowAccepted(aCbAccept.IsChecked());
        aChangeViewSet.SetShowRejected(aCbReject.IsChecked());
        aChangeViewSet.SetHasComment(aFilterCtr.IsComment());
        aChangeViewSet.SetTheComment(aFilterCtr.GetComment());
        ScRangeList aRangeList;
        aRangeList.Parse(aFilterCtr.GetRange());
        aChangeViewSet.SetTheRangeList(aRangeList);
        pDoc->SetChangeViewSettings(aChangeViewSet);
        pViewData->GetDocShell()->PostPaintGridAll();
        Close();
    }
    return 0;
}



