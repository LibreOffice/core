/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: highred.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-21 14:06:01 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

// System - Includes ---------------------------------------------------------



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
#include <sfx2/app.hxx>

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
        aEdAssign       ( this, ResId( ED_ASSIGN ) ),
        aRbAssign       ( this, ResId( RB_ASSIGN ), &aEdAssign ),
        aHighlightBox   ( this, ResId( CB_HIGHLIGHT)),
        aFlFilter       ( this, ResId( FL_FILTER)),
        aCbAccept       ( this, ResId( CB_HIGHLIGHT_ACCEPT)),
        aCbReject       ( this, ResId( CB_HIGHLIGHT_REJECT)),

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

    Point aFlFilterPt( aFlFilter.GetPosPixel() );
    aFlFilterPt.Y() += aFlFilter.GetSizePixel().Height();
    aFilterCtr.SetPosPixel( aFlFilterPt );
    MinSize=aFilterCtr.GetSizePixel();
    MinSize.Height()+=2;
    MinSize.Width()+=2;
    aOkButton.SetClickHdl(LINK( this, ScHighlightChgDlg, OKBtnHdl));
    aHighlightBox.SetClickHdl(LINK( this, ScHighlightChgDlg, HighLightHandle ));
    aFilterCtr.SetRefHdl(LINK( this, ScHighlightChgDlg, RefHandle ));
    aFilterCtr.HideRange(FALSE);
    aFilterCtr.Show();
    SetDispatcherLock( TRUE );
    //SFX_APPWINDOW->Disable(FALSE);

    Init();

}
ScHighlightChgDlg::~ScHighlightChgDlg()
{
    SetDispatcherLock( FALSE );
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
        SetDispatcherLock( FALSE );
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
        SetDispatcherLock( TRUE );
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
        aRangeList.Parse(aFilterCtr.GetRange(), pDoc);
        aChangeViewSet.SetTheRangeList(aRangeList);
        aChangeViewSet.AdjustDateMode( *pDoc );
        pDoc->SetChangeViewSettings(aChangeViewSet);
        pViewData->GetDocShell()->PostPaintGridAll();
        Close();
    }
    return 0;
}



