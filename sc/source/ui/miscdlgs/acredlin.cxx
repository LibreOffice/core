/*************************************************************************
 *
 *  $RCSfile: acredlin.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: er $ $Date: 2001-03-14 14:35:33 $
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

#include <svtools/undo.hxx>
#include <unotools/textsearch.hxx>
#include <unotools/localedatawrapper.hxx>
#include <unotools/collatorwrapper.hxx>
#include <vcl/msgbox.hxx>
#include <sfx2/app.hxx>

// INCLUDE -------------------------------------------------------------------

#include "acredlin.hxx"
#include "global.hxx"
#include "reffact.hxx"
#include "document.hxx"
#include "docsh.hxx"
#include "scresid.hxx"
#include "globstr.hrc"
#include "acredlin.hrc"
#include "simpref.hxx"
#include "scmod.hxx"
#include "popmenu.hxx"
#include "tabvwsh.hxx"

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

#define RD_SPECIAL_NONE         0
#define RD_SPECIAL_CONTENT      1
#define RD_SPECIAL_VISCONTENT   2

//============================================================================
//  class ScRedlinData
//----------------------------------------------------------------------------

ScRedlinData::ScRedlinData()
    :RedlinData()
{
    nInfo=RD_SPECIAL_NONE;
    nActionNo=0;
    pData=NULL;
    bDisabled=FALSE;
    bIsRejectable=FALSE;
    bIsAcceptable=FALSE;
    nTable=0xffff;
    nCol=0xffff;
    nRow=0xffff;
}

ScRedlinData::~ScRedlinData()
{
    nInfo=RD_SPECIAL_NONE;
    nActionNo=0;
    pData=NULL;
    bDisabled=FALSE;
    bIsRejectable=FALSE;
    bIsAcceptable=FALSE;
}

static BOOL bSomebodyKilledMe= FALSE;
static BOOL bFilterPageVisible=FALSE;
static BOOL bSimpleRefDlgStarted=FALSE;
static BOOL bScAcceptChgDlgIsDead=TRUE;

static ScChangeViewSettings aPrivChangeViewSet;

//============================================================================
//  class ScAcceptChgDlg
//----------------------------------------------------------------------------
ScAcceptChgDlg::ScAcceptChgDlg( SfxBindings* pB, SfxChildWindow* pCW, Window* pParent,
                      ScViewData*       ptrViewData)

    :   SfxModelessDialog( pB, pCW, pParent, ScResId(RID_SCDLG_CHANGES) ),
        //
        aAcceptChgCtr           ( this, ResId( CTR_REDLINING ) ),
        //
        aStrInsertCols          ( ResId( STR_INSERT_COLS)),
        aStrInsertRows          ( ResId( STR_INSERT_ROWS)),
        aStrInsertTabs          ( ResId( STR_INSERT_TABS)),
        aStrDeleteCols          ( ResId( STR_DELETE_COLS)),
        aStrDeleteRows          ( ResId( STR_DELETE_ROWS)),
        aStrDeleteTabs          ( ResId( STR_DELETE_TABS)),
        aStrMove                ( ResId( STR_MOVE       )),
        aStrContent             ( ResId( STR_CONTENT    )),
        aStrReject              ( ResId( STR_REJECT     )),
        aStrAllAccepted         ( ResId( STR_ACCEPTED   )),
        aStrAllRejected         ( ResId( STR_REJECTED   )),
        aStrNoEntry             ( ResId( STR_NO_ENTRY   )),
        aStrEmpty               ( ResId( STR_EMPTY      )),
        aStrContentWithChild    ( ResId( STR_CONTENT_WITH_CHILD)),
        aStrChildContent        ( ResId( STR_CHILD_CONTENT)),
        aStrChildOrgContent     ( ResId( STR_CHILD_ORGCONTENT)),
        aExpBmp                 ( ResId( RID_BMP_EXPAND ) ),
        aCollBmp                ( ResId( RID_BMP_COLLAPSE ) ),
        aCloseBmp               ( ResId( BMP_STR_CLOSE ) ),
        aOpenBmp                ( ResId( BMP_STR_OPEN  ) ),
        aEndBmp                 ( ResId( BMP_STR_END   ) ),

        //
        pViewData       ( ptrViewData ),
        pDoc            ( ptrViewData->GetDocument() ),
        aLocalRangeName ( *(pDoc->GetRangeName()) )
{
    FreeResource();
    bScAcceptChgDlgIsDead=FALSE;
    bNoSelection=FALSE;
    bNeedsUpdate=FALSE;
    bIgnoreMsg=FALSE;
    nAcceptCount=0;
    nRejectCount=0;
    bAcceptEnableFlag=TRUE;
    bRejectEnableFlag=TRUE;
    bHasFilterEntry=FALSE;
    bUseColor=FALSE;
    aReOpenTimer.SetTimeout(50);
    aReOpenTimer.SetTimeoutHdl(LINK( this, ScAcceptChgDlg, ReOpenTimerHdl ));

    if(bSomebodyKilledMe)
    {
        if(bFilterPageVisible)
            aAcceptChgCtr.ShowFilterPage();

        if(bSimpleRefDlgStarted)
            aReOpenTimer.Start();
    }

    MinSize=aAcceptChgCtr.GetMinSizePixel();
    MinSize.Height()+=2;
    MinSize.Width()+=2;
    SetMinOutputSizePixel(MinSize);
    aUnknown.AssignAscii(RTL_CONSTASCII_STRINGPARAM("Unknown"));

    pTPFilter=aAcceptChgCtr.GetFilterPage();
    pTPView=aAcceptChgCtr.GetViewPage();
    pTheView=pTPView->GetTableControl();
    aSelectionTimer.SetTimeout(100);
    aSelectionTimer.SetTimeoutHdl(LINK( this, ScAcceptChgDlg, UpdateSelectionHdl ));

    pTPFilter->SetReadyHdl(LINK( this, ScAcceptChgDlg, FilterHandle ));
    pTPFilter->SetRefHdl(LINK( this, ScAcceptChgDlg, RefHandle ));
    pTPFilter->SetModifyHdl(LINK( this, ScAcceptChgDlg, FilterModified));
    pTPFilter->HideRange(FALSE);
    pTPView->InsertCalcHeader();
    pTPView->SetRejectClickHdl( LINK( this, ScAcceptChgDlg,RejectHandle));
    pTPView->SetAcceptClickHdl( LINK(this, ScAcceptChgDlg, AcceptHandle));
    pTPView->SetRejectAllClickHdl( LINK( this, ScAcceptChgDlg,RejectAllHandle));
    pTPView->SetAcceptAllClickHdl( LINK(this, ScAcceptChgDlg, AcceptAllHandle));
    pTheView->SetCalcView();
    pTheView->SetWindowBits(WB_HASLINES|WB_CLIPCHILDREN|WB_HASBUTTONS|WB_HASBUTTONSATROOT|WB_HSCROLL);
    pTheView->SetExpandingHdl( LINK(this, ScAcceptChgDlg, ExpandingHandle));
    pTheView->SetSelectHdl( LINK(this, ScAcceptChgDlg, SelectHandle));
    pTheView->SetDeselectHdl( LINK(this, ScAcceptChgDlg, SelectHandle));
    pTheView->SetCommandHdl( LINK(this, ScAcceptChgDlg, CommandHdl));
    pTheView->SetColCompareHdl( LINK(this, ScAcceptChgDlg,ColCompareHdl));
    pTheView->SetNodeBitmaps( aExpBmp, aCollBmp );
    pTheView->SetSelectionMode(MULTIPLE_SELECTION);
    pTheView->SetHighlightRange(1);

    Init();

    aAcceptChgCtr.SetMinSizeHdl( LINK( this, ScAcceptChgDlg, MinSizeHandle ));

    UpdateView();
    SvLBoxEntry* pEntry=pTheView->First();
    if(pEntry!=NULL)
    {
        pTheView->Select(pEntry);
    }
    bSomebodyKilledMe=TRUE;
}
ScAcceptChgDlg::~ScAcceptChgDlg()
{
    bScAcceptChgDlgIsDead=TRUE;
    if(bSomebodyKilledMe)
    {
        SetMyStaticData();
    }
    ClearView();
    ScChangeTrack* pChanges=pDoc->GetChangeTrack();

    if(pChanges!=NULL)
    {
        Link aLink;
        pChanges->SetModifiedLink(aLink);
    }
}

void ScAcceptChgDlg::ReInit(ScViewData* ptrViewData)
{
    pViewData=ptrViewData;
    if(pViewData!=NULL)
    {
        pDoc=ptrViewData->GetDocument();
    }
    else
    {
        pDoc=NULL;
    }

    bNoSelection=FALSE;
    bNeedsUpdate=FALSE;
    bIgnoreMsg=FALSE;
    nAcceptCount=0;
    nRejectCount=0;
    bAcceptEnableFlag=TRUE;
    bRejectEnableFlag=TRUE;

    Init();
    ClearView();
    UpdateView();
}

void __EXPORT ScAcceptChgDlg::Init()
{
    String  aAreaStr;
    ScRange aRange;

    DBG_ASSERT( pViewData && pDoc, "ViewData oder Document nicht gefunden!" );

    ScChangeTrack* pChanges=pDoc->GetChangeTrack();

    if(pChanges!=NULL)
    {
        pChanges->SetModifiedLink( LINK( this, ScAcceptChgDlg,ChgTrackModHdl));
        aChangeViewSet.SetTheAuthorToShow(pChanges->GetUser());
        pTPFilter->ClearAuthors();
        StrCollection aUserColl=pChanges->GetUserCollection();
        for(USHORT  i=0;i<aUserColl.GetCount();i++)
            pTPFilter->InsertAuthor(aUserColl[i]->GetString());
    }


    ScChangeViewSettings* pViewSettings=pDoc->GetChangeViewSettings();

    if(pViewSettings!=NULL &&!bSomebodyKilledMe)
    {
        aChangeViewSet=*pViewSettings;
        aPrivChangeViewSet=aChangeViewSet;
    }
    else
        aChangeViewSet=aPrivChangeViewSet;

    pTPFilter->CheckDate(aChangeViewSet.HasDate());
    pTPFilter->SetFirstDate(aChangeViewSet.GetTheFirstDateTime());
    pTPFilter->SetFirstTime(aChangeViewSet.GetTheFirstDateTime());
    pTPFilter->SetLastDate(aChangeViewSet.GetTheLastDateTime());
    pTPFilter->SetLastTime(aChangeViewSet.GetTheLastDateTime());
    pTPFilter->SetDateMode((USHORT)aChangeViewSet.GetTheDateMode());
    pTPFilter->CheckComment(aChangeViewSet.HasComment());
    pTPFilter->SetComment(aChangeViewSet.GetTheComment());

    pTPFilter->CheckAuthor(aChangeViewSet.HasAuthor());
    String aString=aChangeViewSet.GetTheAuthorToShow();
    if(aString.Len()!=0)
    {
        pTPFilter->SelectAuthor(aString);
        if(pTPFilter->GetSelectedAuthor()!=aString)
        {
            pTPFilter->InsertAuthor(aString);
            pTPFilter->SelectAuthor(aString);
        }
    }
    else
    {
        pTPFilter->SelectedAuthorPos(0);
    }

    pTPFilter->CheckRange(aChangeViewSet.HasRange());

    ScRange* pRangeEntry=aChangeViewSet.GetTheRangeList().GetObject(0);
    aRangeList=aChangeViewSet.GetTheRangeList();

    if(pRangeEntry!=NULL)
    {
        String aRefStr;
        pRangeEntry->Format( aRefStr, ABS_DREF3D, pDoc );
        pTPFilter->SetRange(aRefStr);
    }

    Point aPoint(1,1);
    aAcceptChgCtr.SetPosPixel(aPoint);
    InitFilter();
}



void ScAcceptChgDlg::ClearView()
{
    ULONG nCount=pTheView->GetEntryCount();
    nAcceptCount=0;
    nRejectCount=0;
    pTheView->SetUpdateMode(FALSE);

    pTheView->Clear();
    pTheView->SetUpdateMode(TRUE);
}

String* ScAcceptChgDlg::MakeTypeString(ScChangeActionType eType)
{
    String* pStr;

    switch(eType)
    {

        case SC_CAT_INSERT_COLS:    pStr=&aStrInsertCols;break;

        case SC_CAT_INSERT_ROWS:    pStr=&aStrInsertRows;break;

        case SC_CAT_INSERT_TABS:    pStr=&aStrInsertTabs;break;

        case SC_CAT_DELETE_COLS:    pStr=&aStrDeleteCols;break;

        case SC_CAT_DELETE_ROWS:    pStr=&aStrDeleteRows;break;

        case SC_CAT_DELETE_TABS:    pStr=&aStrDeleteTabs;break;

        case SC_CAT_MOVE:           pStr=&aStrMove;break;

        case SC_CAT_CONTENT:        pStr=&aStrContent;break;

        case SC_CAT_REJECT:         pStr=&aStrReject;break;

        default:                    pStr=&aUnknown;break;
    }
    return pStr;
}


BOOL ScAcceptChgDlg::IsValidAction(const ScChangeAction* pScChangeAction)
{
    if(pScChangeAction==NULL) return FALSE;

    BOOL bFlag=FALSE;

    ScRange aRef=pScChangeAction->GetBigRange().MakeRange();
    String aUser=pScChangeAction->GetUser();
    DateTime aDateTime=pScChangeAction->GetDateTime();

    ScChangeActionType eType=pScChangeAction->GetType();
    String aString;
    String aDesc;

    String aComment=pScChangeAction->GetComment();
    aComment.EraseAllChars('\n');

    if(eType==SC_CAT_CONTENT)
    {
        if(!pScChangeAction->IsDialogParent())
        {
            pScChangeAction->GetDescription( aDesc, pDoc, TRUE);
        }
    }
    else
    {
        pScChangeAction->GetDescription( aDesc, pDoc,!pScChangeAction->IsMasterDelete());
    }

    if(aDesc.Len()>0)
    {
        aComment.AppendAscii(RTL_CONSTASCII_STRINGPARAM( " (" ));
        aComment+=aDesc;
        aComment+=')';
    }

    if(pTheView->IsValidEntry(&aUser,&aDateTime,&aComment))
    {
        if(pTPFilter->IsRange())
        {
            ScRange* pRangeEntry=aRangeList.First();

            while(pRangeEntry!=NULL)
            {
                if(pRangeEntry->Intersects(aRef)) break;
                pRangeEntry=aRangeList.Next();
            }

            if(pRangeEntry!=NULL)
            {
                bFlag=TRUE;
            }
        }
        else
        {
            bFlag=TRUE;
        }
    }

    return bFlag;
}

SvLBoxEntry* ScAcceptChgDlg::InsertChangeAction(const ScChangeAction* pScChangeAction,
                                                   ScChangeActionState eState,SvLBoxEntry* pParent,
                                                   BOOL bDelMaster,BOOL bDisabled,ULONG nPos)
{
    ScChangeTrack* pChanges=pDoc->GetChangeTrack();

    if(pScChangeAction==NULL || pChanges==NULL) return NULL;

    SvLBoxEntry* pEntry=NULL;

    BOOL bFlag=FALSE;

    ScRange aRef=pScChangeAction->GetBigRange().MakeRange();
    String aUser=pScChangeAction->GetUser();
    DateTime aDateTime=pScChangeAction->GetDateTime();

    String aRefStr;
    ScChangeActionType eType=pScChangeAction->GetType();
    String aString;
    String aDesc;

    ScRedlinData* pNewData=new ScRedlinData;
    pNewData->pData=(void *)pScChangeAction;
    pNewData->nActionNo=pScChangeAction->GetActionNumber();
    pNewData->bIsAcceptable=pScChangeAction->IsClickable();
    pNewData->bIsRejectable=pScChangeAction->IsRejectable();
    pNewData->bDisabled=!pNewData->bIsAcceptable | bDisabled;
    pNewData->aDateTime=aDateTime;
    pNewData->nRow  = aRef.aStart.Row();
    pNewData->nCol  = aRef.aStart.Col();
    pNewData->nTable= aRef.aStart.Tab();

    if(eType==SC_CAT_CONTENT)
    {
        if(pScChangeAction->IsDialogParent())
        {
            aString=aStrContentWithChild;
            pNewData->nInfo=RD_SPECIAL_VISCONTENT;
            pNewData->bIsRejectable=FALSE;
            pNewData->bIsAcceptable=FALSE;
        }
        else
        {
            aString=*MakeTypeString(eType);
            pScChangeAction->GetDescription( aDesc, pDoc, TRUE);
        }
    }
    else
    {
        aString=*MakeTypeString(eType);

        if(bDelMaster)
        {
            pScChangeAction->GetDescription( aDesc, pDoc,TRUE);
            pNewData->bDisabled=TRUE;
            pNewData->bIsRejectable=FALSE;
        }
        else
            pScChangeAction->GetDescription( aDesc, pDoc,!pScChangeAction->IsMasterDelete());

    }

    aString+='\t';
    pScChangeAction->GetRefString(aRefStr, pDoc, TRUE);
    aString+=aRefStr;
    aString+='\t';

    BOOL bIsGenerated;

    if(!pChanges->IsGenerated(pScChangeAction->GetActionNumber()))
    {
        aString+=aUser;
        aString+='\t';

        aString+=ScGlobal::pLocaleData->getDate(aDateTime);
        aString+=' ';
        aString+=ScGlobal::pLocaleData->getTime(aDateTime);
        aString+='\t';
        bIsGenerated=FALSE;
    }
    else
    {
        aString+='\t';
        aString+='\t';
        bIsGenerated=TRUE;
    }

    String aComment=pScChangeAction->GetComment();
    aComment.EraseAllChars('\n');
    if(aDesc.Len()>0)
    {
        aComment.AppendAscii(RTL_CONSTASCII_STRINGPARAM( " (" ));
        aComment+=aDesc;
        aComment+=')';
    }

    aString+=aComment;

    if(pTheView->IsValidEntry(&aUser,&aDateTime)|| bIsGenerated)
    {
        if(pTheView->IsValidComment(&aComment))
        {
            if(pTPFilter->IsRange())
            {
                ScRange* pRangeEntry=aRangeList.First();

                while(pRangeEntry!=NULL)
                {
                    if(pRangeEntry->Intersects(aRef)) break;
                    pRangeEntry=aRangeList.Next();
                }
                //SC_CAS_VIRGIN,SC_CAS_ACCEPTED,SC_CAS_REJECTED
                if(pRangeEntry!=NULL)
                {
                    bHasFilterEntry=TRUE;
                    bFlag=TRUE;
                }
            }
            else if(!bIsGenerated)
            {
                bHasFilterEntry=TRUE;
                bFlag=TRUE;
            }
        }
    }

    if(!bFlag&& bUseColor&& pParent==NULL)
    {
        pEntry=pTheView->InsertEntry(aString,pNewData,Color(COL_LIGHTBLUE),pParent,nPos);
    }
    else if(bFlag&& bUseColor&& pParent!=NULL)
    {
        pEntry=pTheView->InsertEntry(aString,pNewData,Color(COL_GREEN),pParent,nPos);
        SvLBoxEntry* pExpEntry=pParent;

        while(pExpEntry!=NULL && !pTheView->IsExpanded(pExpEntry))
        {
            SvLBoxEntry* pTmpEntry=pTheView->GetParent(pExpEntry);

            if(pTmpEntry!=NULL) pTheView->Expand(pExpEntry);

            pExpEntry=pTmpEntry;
        }
    }
    else
    {
        pEntry=pTheView->InsertEntry(aString,pNewData,pParent,nPos);
    }
    return pEntry;
}

SvLBoxEntry* ScAcceptChgDlg::InsertFilteredAction(const ScChangeAction* pScChangeAction,
                                                   ScChangeActionState eState,SvLBoxEntry* pParent,
                                                   BOOL bDelMaster,BOOL bDisabled,ULONG nPos)
{

    ScChangeTrack* pChanges=pDoc->GetChangeTrack();

    if(pScChangeAction==NULL || pChanges==NULL) return NULL;

    BOOL bIsGenerated=pChanges->IsGenerated(pScChangeAction->GetActionNumber());

    SvLBoxEntry* pEntry=NULL;

    int bFlag=FALSE;

    ScRange aRef=pScChangeAction->GetBigRange().MakeRange();
    String aUser=pScChangeAction->GetUser();
    DateTime aDateTime=pScChangeAction->GetDateTime();

    if(pTheView->IsValidEntry(&aUser,&aDateTime)||bIsGenerated)
    {
        if(pTPFilter->IsRange())
        {
            ScRange* pRangeEntry=aRangeList.First();

            while(pRangeEntry!=NULL)
            {
                if(pRangeEntry->Intersects(aRef)) break;
                pRangeEntry=aRangeList.Next();
            }
            //SC_CAS_VIRGIN,SC_CAS_ACCEPTED,SC_CAS_REJECTED
            if(pRangeEntry!=NULL &&
                pScChangeAction->GetState()==eState)
            {
                bFlag=TRUE;
            }
        }
        else if(pScChangeAction->GetState()==eState && !bIsGenerated)
        {
            bFlag=TRUE;
        }
    }

    if(bFlag)
    {

        String aRefStr;
        ScChangeActionType eType=pScChangeAction->GetType();
        String aString;
        String aDesc;


        ScRedlinData* pNewData=new ScRedlinData;
        pNewData->pData=(void *)pScChangeAction;
        pNewData->nActionNo=pScChangeAction->GetActionNumber();
        pNewData->bIsAcceptable=pScChangeAction->IsClickable();
        pNewData->bIsRejectable=pScChangeAction->IsRejectable();
        pNewData->bDisabled=!pNewData->bIsAcceptable | bDisabled;
        pNewData->aDateTime=aDateTime;
        pNewData->nRow  = aRef.aStart.Row();
        pNewData->nCol  = aRef.aStart.Col();
        pNewData->nTable= aRef.aStart.Tab();

        if(eType==SC_CAT_CONTENT)
        {
            if(pScChangeAction->IsDialogParent())
            {
                aString=aStrContentWithChild;
                pNewData->nInfo=RD_SPECIAL_VISCONTENT;
                pNewData->bIsRejectable=FALSE;
                pNewData->bIsAcceptable=FALSE;
            }
            else
            {
                aString=*MakeTypeString(eType);
                pScChangeAction->GetDescription( aDesc, pDoc, TRUE);
            }
        }
        else
        {
            aString=*MakeTypeString(eType);

            if(bDelMaster)
            {
                pScChangeAction->GetDescription( aDesc, pDoc,TRUE);
                pNewData->bDisabled=TRUE;
                pNewData->bIsRejectable=FALSE;
            }
            else
                pScChangeAction->GetDescription( aDesc, pDoc,!pScChangeAction->IsMasterDelete());

        }

        aString+='\t';
        pScChangeAction->GetRefString(aRefStr, pDoc, TRUE);
        aString+=aRefStr;
        aString+='\t';

        if(!bIsGenerated)
        {
            aString+=aUser;
            aString+='\t';
            aString+=ScGlobal::pLocaleData->getDate(aDateTime);
            aString+=' ';
            aString+=ScGlobal::pLocaleData->getTime(aDateTime);
            aString+='\t';
        }
        else
        {
            aString+='\t';
            aString+='\t';
        }

        String aComment=pScChangeAction->GetComment();
        aComment.EraseAllChars('\n');
        if(aDesc.Len()>0)
        {
            aComment.AppendAscii(RTL_CONSTASCII_STRINGPARAM( " (" ));
            aComment+=aDesc;
            aComment+=')';
        }
        if(pTheView->IsValidComment(&aComment))
        {
            aString+=aComment;
            pEntry=pTheView->InsertEntry(aString,pNewData,pParent,nPos);
        }
        else
            delete pNewData;
    }
    return pEntry;
}

SvLBoxEntry* ScAcceptChgDlg::InsertChangeActionContent(const ScChangeActionContent* pScChangeAction,
                                                          SvLBoxEntry* pParent, ULONG nSpecial)
{
    ScChangeTrack* pChanges=pDoc->GetChangeTrack();
    SvLBoxEntry* pEntry=NULL;

    if(pScChangeAction==NULL || pChanges==NULL) return NULL;

    BOOL bIsGenerated=pChanges->IsGenerated(pScChangeAction->GetActionNumber());

    BOOL bFlag=FALSE;

    ScRange aRef=pScChangeAction->GetBigRange().MakeRange();
    String aUser=pScChangeAction->GetUser();
    DateTime aDateTime=pScChangeAction->GetDateTime();

    if(pTheView->IsValidEntry(&aUser,&aDateTime)||bIsGenerated)
    {
        if(pTPFilter->IsRange())
        {
            ScRange* pRangeEntry=aRangeList.First();

            while(pRangeEntry!=NULL)
            {
                if(pRangeEntry->Intersects(aRef)) break;
                pRangeEntry=aRangeList.Next();
            }
            //SC_CAS_VIRGIN,SC_CAS_ACCEPTED,SC_CAS_REJECTED
            if(pRangeEntry!=NULL)
            {
                bFlag=TRUE;
            }
        }
        else if(!bIsGenerated)
            bFlag=TRUE;
    }

    String aRefStr;
    String aString;
    String a2String;
    String aDesc;

    if(nSpecial==RD_SPECIAL_CONTENT)
    {
        pScChangeAction->GetOldString(a2String);
        if(a2String.Len()==0) a2String=aStrEmpty;

        //aString+="\'";
        aString+=a2String;
        //aString+="\'";

        aDesc=aStrChildOrgContent;
        aDesc.AppendAscii(RTL_CONSTASCII_STRINGPARAM( ": " ));
    }
    else
    {
        pScChangeAction->GetNewString(a2String);
        if(a2String.Len()==0)
        {
            a2String=aStrEmpty;
            aString+=a2String;
        }
        else
        {
            aString+='\'';
            aString+=a2String;
            aString+='\'';
            a2String=aString;
        }
        aDesc=aStrChildContent;

    }

    aDesc+=a2String;
    aString+='\t';
    pScChangeAction->GetRefString(aRefStr, pDoc, TRUE);
    aString+=aRefStr;
    aString+='\t';

    if(!bIsGenerated)
    {
        aString+=aUser;
        aString+='\t';

        aString+=ScGlobal::pLocaleData->getDate(aDateTime);
        aString+=' ';
        aString+=ScGlobal::pLocaleData->getTime(aDateTime);
        aString+='\t';
    }
    else
    {
        aString+='\t';
        aString+='\t';
    }
    String aComment=pScChangeAction->GetComment();
    aComment.EraseAllChars('\n');

    if(aDesc.Len()>0)
    {
        aComment.AppendAscii(RTL_CONSTASCII_STRINGPARAM( " (" ));
        aComment+=aDesc;
        aComment+=')';
    }

    aString+=aComment;

    ScRedlinData* pNewData=new ScRedlinData;
    pNewData->nInfo=nSpecial;
    pNewData->pData=(void *)pScChangeAction;
    pNewData->nActionNo=pScChangeAction->GetActionNumber();
    pNewData->bIsAcceptable=pScChangeAction->IsClickable();
    pNewData->bIsRejectable=FALSE;
    pNewData->bDisabled=!pNewData->bIsAcceptable;
    pNewData->aDateTime=aDateTime;
    pNewData->nRow  = aRef.aStart.Row();
    pNewData->nCol  = aRef.aStart.Col();
    pNewData->nTable= aRef.aStart.Tab();

    if(pTheView->IsValidComment(&aComment) && bFlag)
    {
        bHasFilterEntry=TRUE;
        pEntry=pTheView->InsertEntry(aString,pNewData,pParent);
    }
    else
        pEntry=pTheView->InsertEntry(aString,pNewData,Color(COL_LIGHTBLUE),pParent);
    return pEntry;
}

long ScAcceptChgDlg::PreNotify( NotifyEvent& rNEvt )
{
    if(rNEvt.GetType()==EVENT_GETFOCUS && bNeedsUpdate)
    {
        ClearView();
        UpdateView();
        bNoSelection=FALSE;
    }

    return SfxModelessDialog::PreNotify(rNEvt);
}


void ScAcceptChgDlg::UpdateView()
{
    bNeedsUpdate=FALSE;
    DateTime aDateTime;
    SvLBoxEntry* pParent=NULL;
    ScChangeTrack* pChanges=NULL;
    const ScChangeAction* pScChangeAction=NULL;
    bAcceptEnableFlag=TRUE;
    bRejectEnableFlag=TRUE;
    SetPointer(Pointer(POINTER_WAIT));
    pTheView->SetUpdateMode(FALSE);
    BOOL bFilterFlag=pTPFilter->IsDate()||pTPFilter->IsRange()||
                     pTPFilter->IsAuthor()||pTPFilter->IsComment();

    bUseColor=bFilterFlag;

    if(pDoc!=NULL)
    {
        pChanges=pDoc->GetChangeTrack();
        if(pChanges!=NULL)
        {
            pScChangeAction=pChanges->GetFirst();
        }
    }
    ScChangeActionTable ActionTable;
    BOOL bTheFlag=FALSE;

    while(pScChangeAction!=NULL)
    {
        bHasFilterEntry=FALSE;
        switch(pScChangeAction->GetState())
        {
            case SC_CAS_VIRGIN:

                if(pScChangeAction->IsDialogRoot())
                {
                    if(pScChangeAction->IsDialogParent())
                        pParent=InsertChangeAction(pScChangeAction,SC_CAS_VIRGIN);
                    else
                        pParent=InsertFilteredAction(pScChangeAction,SC_CAS_VIRGIN);
                }
                else
                    pParent=NULL;

                bTheFlag=TRUE;
                break;

            case SC_CAS_ACCEPTED:
                pParent=NULL;
                nAcceptCount++;
                break;

            case SC_CAS_REJECTED:
                pParent=NULL;
                nRejectCount++;
                break;
        }

        if(pParent!=NULL && pScChangeAction->IsDialogParent())
        {
            if(!bFilterFlag)
            {
                pParent->EnableChildsOnDemand(TRUE);
            }
            else
            {
                BOOL bTestFlag=bHasFilterEntry;
                bHasFilterEntry=FALSE;
                if(Expand(pChanges,pScChangeAction,pParent,!bTestFlag)&&!bTestFlag)
                    pTheView->GetModel()->Remove(pParent);
            }
        }

        pScChangeAction=pScChangeAction->GetNext();
    }

    if(bTheFlag && !pDoc->IsDocEditable())
        bTheFlag=FALSE;

    pTPView->EnableAccept(bTheFlag);
    pTPView->EnableAcceptAll(bTheFlag);
    pTPView->EnableReject(bTheFlag);
    pTPView->EnableRejectAll(bTheFlag);

    if(nAcceptCount>0)
    {
        pParent=pTheView->InsertEntry(aStrAllAccepted,NULL,NULL);
        pParent->EnableChildsOnDemand(TRUE);
    }
    if(nRejectCount>0)
    {
        pParent=pTheView->InsertEntry(aStrAllRejected,NULL,NULL);
        pParent->EnableChildsOnDemand(TRUE);
    }
    pTheView->SetUpdateMode(TRUE);
    SetPointer(Pointer(POINTER_ARROW));
    SvLBoxEntry* pEntry=pTheView->First();
    if(pEntry!=NULL)
    {
        pTheView->Select(pEntry);
    }
}

//----------------------------------------------------------------------------
BOOL ScAcceptChgDlg::Close()
{
    bSomebodyKilledMe=FALSE;
    return SfxModelessDialog::Close();
}

void ScAcceptChgDlg::Resize()
{
    SfxModelessDialog::Resize();
    Size aSize=GetOutputSizePixel();
    aAcceptChgCtr.SetSizePixel(aSize);
}
/*
void ScAcceptChgDlg::SetActive()
{
    if(pTPFilter!=NULL)
    {
        aAcceptChgCtr.GetFilterPage()->SetFocusToRange();
        aEdAssign.Hide();
        aRbAssign.Hide();
        Application::GetAppWindow()->Enable();
        SFX_APP()->LockDispatcher( FALSE);
    }
    //RefInputDone();
}
*/
IMPL_LINK( ScAcceptChgDlg, MinSizeHandle, SvxAcceptChgCtr*, pCtr )
{
    if(pCtr==&aAcceptChgCtr)
    {
        if(!IsRollUp())
            SetOutputSizePixel(MinSize);
    }
    return 0;
}

IMPL_LINK( ScAcceptChgDlg, RefHandle, SvxTPFilter*, pRef )
{
    USHORT nId  =ScSimpleRefDlgWrapper::GetChildWindowId();

    ScSimpleRefDlgWrapper::SetDefaultPosSize(GetPosPixel(),GetSizePixel(),TRUE);

    SC_MOD()->SetRefDialog( nId, TRUE );

    SfxViewFrame* pViewFrm = pViewData->GetViewShell()->GetViewFrame();
    ScSimpleRefDlgWrapper* pWnd =(ScSimpleRefDlgWrapper*)pViewFrm->GetChildWindow( nId );

    if(pWnd!=NULL)
    {
        bSimpleRefDlgStarted=TRUE;
        USHORT nAcceptId=ScAcceptChgDlgWrapper::GetChildWindowId();
        pViewFrm->ShowChildWindow(nAcceptId,FALSE);
        pWnd->SetCloseHdl(LINK( this, ScAcceptChgDlg,RefInfoHandle));
        pWnd->SetRefString(pTPFilter->GetRange());
        pWnd->SetAutoReOpen(FALSE);
        Window* pWin=pWnd->GetWindow();
        pWin->SetPosSizePixel(GetPosPixel(),GetSizePixel());
        Hide();
        pWin->SetText(GetText());
        pWnd->StartRefInput();
    }
    return 0;
}

IMPL_LINK( ScAcceptChgDlg, RefInfoHandle, String*, pResult)
{
    USHORT nId;

    bSimpleRefDlgStarted=FALSE;
    ScSimpleRefDlgWrapper::SetAutoReOpen(TRUE);

    SfxViewFrame* pViewFrm = pViewData->GetViewShell()->GetViewFrame();
    if(pResult!=NULL)
    {
        pTPFilter->SetRange(*pResult);
        FilterHandle(pTPFilter);

        nId = ScSimpleRefDlgWrapper::GetChildWindowId();
        ScSimpleRefDlgWrapper* pWnd = (ScSimpleRefDlgWrapper*)pViewFrm->GetChildWindow( nId );

        if(pWnd!=NULL)
        {
            Window* pWin=pWnd->GetWindow();
            Size aSize=pWin->GetSizePixel();
            aSize.Width()=GetSizePixel().Width();
            SetPosSizePixel(pWin->GetPosPixel(),aSize);
            Invalidate();
        }
        nId = ScAcceptChgDlgWrapper::GetChildWindowId();
        pViewFrm->ShowChildWindow( nId, TRUE );
    }
    else
    {
        nId = ScAcceptChgDlgWrapper::GetChildWindowId();
        pViewFrm->SetChildWindow( nId, FALSE );
    }
    return 0;
}

IMPL_LINK( ScAcceptChgDlg, FilterHandle, SvxTPFilter*, pRef )
{
    if(pRef!=NULL)
    {
        ULONG nTest=pTheView->GetEntryCount();
        ClearView();
        aRangeList.Clear();
        aRangeList.Parse(pTPFilter->GetRange(),pDoc);
        UpdateView();
    }
    return 0;
}

IMPL_LINK( ScAcceptChgDlg, RejectHandle, SvxTPView*, pRef )
{
    SetPointer(Pointer(POINTER_WAIT));

    bIgnoreMsg=TRUE;
    ScChangeTrack* pChanges=pDoc->GetChangeTrack();

    if(pRef!=NULL)
    {
        SvLBoxEntry* pEntry=pTheView->FirstSelected();
        while(pEntry!=NULL)
        {
            ScRedlinData *pEntryData=(ScRedlinData *)(pEntry->GetUserData());
            if(pEntryData!=NULL)
            {
                ScChangeAction* pScChangeAction=
                        (ScChangeAction*) pEntryData->pData;

                if(pScChangeAction->GetType()==SC_CAT_INSERT_TABS)
                {
                    pViewData->SetTabNo(0);
                }
                pChanges->Reject(pScChangeAction);
            }
            pEntry = pTheView->NextSelected(pEntry);
        }
        ScDocShell* pDocSh=pViewData->GetDocShell();
        pDocSh->PostPaintExtras();
        pDocSh->PostPaintGridAll();
        pDocSh->GetUndoManager()->Clear();
        pDocSh->SetDocumentModified();
        ClearView();
        UpdateView();
    }
    SetPointer(Pointer(POINTER_ARROW));

    bIgnoreMsg=FALSE;
    return 0;
}
IMPL_LINK( ScAcceptChgDlg, AcceptHandle, SvxTPView*, pRef )
{
    SetPointer(Pointer(POINTER_WAIT));

    ScChangeTrack* pChanges=pDoc->GetChangeTrack();
    bIgnoreMsg=TRUE;
    if(pRef!=NULL)
    {
        SvLBoxEntry* pEntry=pTheView->FirstSelected();
        while(pEntry!=NULL)
        {
            ScRedlinData *pEntryData=(ScRedlinData *)(pEntry->GetUserData());
            if(pEntryData!=NULL)
            {
                ScChangeAction* pScChangeAction=
                        (ScChangeAction*) pEntryData->pData;
                if(pScChangeAction->GetType()==SC_CAT_CONTENT)
                {
                    if(pEntryData->nInfo==RD_SPECIAL_CONTENT)
                    {
                        pChanges->SelectContent(pScChangeAction,TRUE);
                    }
                    else
                    {
                        pChanges->SelectContent(pScChangeAction);
                    }
                }
                else
                    pChanges->Accept(pScChangeAction);
            }
            pEntry = pTheView->NextSelected(pEntry);
        }
        ScDocShell* pDocSh=pViewData->GetDocShell();
        pDocSh->PostPaintExtras();
        pDocSh->PostPaintGridAll();
        pDocSh->SetDocumentModified();
        ClearView();
        UpdateView();
    }
    bIgnoreMsg=FALSE;

    return 0;
}

void ScAcceptChgDlg::RejectFiltered()
{
    if(pDoc==NULL) return;
    ScChangeTrack* pChanges=pDoc->GetChangeTrack();
    const ScChangeAction* pScChangeAction=NULL;

    if(pChanges!=NULL)
    {
        pScChangeAction=pChanges->GetLast();
    }

    while(pScChangeAction!=NULL)
    {
        if(pScChangeAction->IsDialogRoot())
        {
            if(IsValidAction(pScChangeAction))
            {
                pChanges->Accept((ScChangeAction*)pScChangeAction);
            }
        }
        pScChangeAction=pScChangeAction->GetPrev();
    }
}
void ScAcceptChgDlg::AcceptFiltered()
{
    if(pDoc==NULL) return;
    ScChangeTrack* pChanges=pDoc->GetChangeTrack();
    const ScChangeAction* pScChangeAction=NULL;

    if(pChanges!=NULL)
    {
        pScChangeAction=pChanges->GetLast();
    }

    while(pScChangeAction!=NULL)
    {
        if(pScChangeAction->IsDialogRoot())
        {
            if(IsValidAction(pScChangeAction))
            {
                pChanges->Accept((ScChangeAction*)pScChangeAction);
            }
        }
        pScChangeAction=pScChangeAction->GetPrev();
    }
}

IMPL_LINK( ScAcceptChgDlg, RejectAllHandle, SvxTPView*, pRef )
{
    SetPointer(Pointer(POINTER_WAIT));
    bIgnoreMsg=TRUE;
    ScChangeTrack* pChanges=pDoc->GetChangeTrack();
    if(pChanges!=NULL)
    {
        if(pTPFilter->IsDate()||pTPFilter->IsAuthor()||pTPFilter->IsRange()||pTPFilter->IsComment())
        {
            RejectFiltered();
        }
        else
        {
            pChanges->RejectAll();
        }
        pViewData->SetTabNo(0);

        ScDocShell* pDocSh=pViewData->GetDocShell();
        pDocSh->PostPaintExtras();
        pDocSh->PostPaintGridAll();
        pDocSh->GetUndoManager()->Clear();
        pDocSh->SetDocumentModified();
        ClearView();
        UpdateView();
    }
    SetPointer(Pointer(POINTER_ARROW));

    bIgnoreMsg=FALSE;

    return 0;
}

IMPL_LINK( ScAcceptChgDlg, AcceptAllHandle, SvxTPView*, pRef )
{
    SetPointer(Pointer(POINTER_WAIT));

    bIgnoreMsg=TRUE;
    ScChangeTrack* pChanges=pDoc->GetChangeTrack();
    if(pChanges!=NULL)
    {
        if(pTPFilter->IsDate()||pTPFilter->IsAuthor()||pTPFilter->IsRange()||pTPFilter->IsComment())
        {
            AcceptFiltered();
        }
        else
        {
            pChanges->AcceptAll();
        }
        ScDocShell* pDocSh=pViewData->GetDocShell();
        pDocSh->PostPaintExtras();
        pDocSh->PostPaintGridAll();
        pDocSh->SetDocumentModified();
        ClearView();
        UpdateView();
    }
    bIgnoreMsg=FALSE;
    SetPointer(Pointer(POINTER_ARROW));

    return 0;
}

IMPL_LINK( ScAcceptChgDlg, SelectHandle, SvxRedlinTable*, pTable )
{
    if(!bNoSelection)
    {
        aSelectionTimer.Start();
    }
    bNoSelection=FALSE;
    return 0;
}

void ScAcceptChgDlg::GetDependents(  const ScChangeAction* pScChangeAction,
                                    ScChangeActionTable& aActionTable,
                                    SvLBoxEntry* pEntry)
{
    ScChangeTrack* pChanges=pDoc->GetChangeTrack();

    SvLBoxEntry* pParent=pTheView->GetParent(pEntry);
    if(pParent!=NULL)
    {
        ScRedlinData *pParentData=(ScRedlinData *)(pParent->GetUserData());
        ScChangeAction* pParentAction=(ScChangeAction*) pParentData->pData;
        if(pParentAction!=pScChangeAction)
        {
            pChanges->GetDependents((ScChangeAction*) pScChangeAction,
                        aActionTable,pScChangeAction->IsMasterDelete());
        }
        else
        {
            pChanges->GetDependents((ScChangeAction*) pScChangeAction,
                        aActionTable);
        }
    }
    else
    {
        pChanges->GetDependents((ScChangeAction*) pScChangeAction,
                    aActionTable,pScChangeAction->IsMasterDelete());
    }
}

BOOL ScAcceptChgDlg::InsertContentChilds(ScChangeActionTable* pActionTable,SvLBoxEntry* pParent)
{
    BOOL bTheTestFlag=TRUE;
    ScRedlinData *pEntryData=(ScRedlinData *)(pParent->GetUserData());
    const ScChangeAction* pScChangeAction = (ScChangeAction*) pEntryData->pData;
    BOOL bParentInserted = FALSE;
    // If the parent is a MatrixOrigin then place it in the right order before
    // the MatrixReferences. Also if it is the first content change at this
    // position don't insert the first dependent MatrixReference as the special
    // content (original value) but insert the predecessor of the MatrixOrigin
    // itself instead.
    if ( pScChangeAction->GetType() == SC_CAT_CONTENT &&
            ((const ScChangeActionContent*)pScChangeAction)->IsMatrixOrigin() )
    {
        pActionTable->Insert( pScChangeAction->GetActionNumber(),
            (ScChangeAction*) pScChangeAction );
        bParentInserted = TRUE;
    }
    SvLBoxEntry* pEntry=NULL;

    const ScChangeActionContent* pCChild=(const ScChangeActionContent*)pActionTable->First();
    while(pCChild!=NULL)
    {
        if( pCChild->GetState()==SC_CAS_VIRGIN )
            break;
        pCChild=(const ScChangeActionContent*)pActionTable->Next();
    }

    if(pCChild==NULL) return TRUE;

    SvLBoxEntry* pOriginal=InsertChangeActionContent(pCChild,pParent,RD_SPECIAL_CONTENT);
    if(pOriginal!=NULL)
    {
        bTheTestFlag=FALSE;
        ScRedlinData *pParentData=(ScRedlinData *)(pOriginal->GetUserData());
        pParentData->pData=(void *)pScChangeAction;
        pParentData->nActionNo=pScChangeAction->GetActionNumber();
        pParentData->bIsAcceptable=pScChangeAction->IsRejectable(); // select old value
        pParentData->bIsRejectable=FALSE;
        pParentData->bDisabled=FALSE;
    }
    while(pCChild!=NULL)
    {
        if(pCChild->GetState()==SC_CAS_VIRGIN)
        {
            pEntry=InsertChangeActionContent(pCChild,pParent,RD_SPECIAL_NONE);

            if(pEntry!=NULL)
                bTheTestFlag=FALSE;
        }
        pCChild=(const ScChangeActionContent*)pActionTable->Next();
    }

    if ( !bParentInserted )
    {
        pEntry=InsertChangeActionContent((const ScChangeActionContent*)
                                pScChangeAction,pParent,RD_SPECIAL_NONE);

        if(pEntry!=NULL)
        {
            bTheTestFlag=FALSE;
            ScRedlinData *pParentData=(ScRedlinData *)(pEntry->GetUserData());
            pParentData->pData=(void *)pScChangeAction;
            pParentData->nActionNo=pScChangeAction->GetActionNumber();
            pParentData->bIsAcceptable=pScChangeAction->IsClickable();
            pParentData->bIsRejectable=FALSE;
            pParentData->bDisabled=FALSE;
        }
    }

    return bTheTestFlag;

}

BOOL ScAcceptChgDlg::InsertAcceptedORejected(SvLBoxEntry* pParent)
{
    ScChangeTrack* pChanges=pDoc->GetChangeTrack();
    BOOL bTheTestFlag=TRUE;

    ScChangeActionState eState;
    String aString=pTheView->GetEntryText( pParent);
    String a2String=aString.Copy(0,aStrAllAccepted.Len());
    if(a2String==aStrAllAccepted)
    {
        eState=SC_CAS_ACCEPTED;
    }
    else
    {
        a2String=aString.Copy(0,aStrAllRejected.Len());
        if(a2String==aStrAllRejected)
        {
            eState=SC_CAS_REJECTED;
        }
    }

    ScChangeAction* pScChangeAction=pChanges->GetFirst();
    while(pScChangeAction!=NULL)
    {
        if(pScChangeAction->GetState()==eState &&
            InsertFilteredAction(pScChangeAction,eState,pParent)!=NULL)
            bTheTestFlag=FALSE;
        pScChangeAction=pScChangeAction->GetNext();
    }
    return bTheTestFlag;
}

BOOL ScAcceptChgDlg::InsertChilds(ScChangeActionTable* pActionTable,SvLBoxEntry* pParent)
{
    ScChangeTrack* pChanges=pDoc->GetChangeTrack();
    BOOL bTheTestFlag=TRUE;
    SvLBoxEntry* pEntry=NULL;
    const ScChangeAction* pChild=(const ScChangeAction*)pActionTable->First();
    while(pChild!=NULL)
    {
        pEntry=InsertChangeAction(pChild,SC_CAS_VIRGIN,pParent,FALSE,TRUE);

        if(pEntry!=NULL)
        {
            bTheTestFlag=FALSE;

            ScRedlinData *pEntryData=(ScRedlinData *)(pEntry->GetUserData());
            pEntryData->bIsRejectable=FALSE;
            pEntryData->bIsAcceptable=FALSE;
            pEntryData->bDisabled=TRUE;

            if(pChild->IsDialogParent())
            {

                //pEntry->EnableChildsOnDemand(TRUE);
                Expand(pChanges,pChild,pEntry);
            }
        }
        pChild=pActionTable->Next();
    }
    return bTheTestFlag;
}
BOOL ScAcceptChgDlg::InsertDeletedChilds(const ScChangeAction* pScChangeAction,
                                         ScChangeActionTable* pActionTable,SvLBoxEntry* pParent)
{
    ScChangeTrack* pChanges=pDoc->GetChangeTrack();
    BOOL bTheTestFlag=TRUE;
    SvLBoxEntry* pEntry=NULL;
    ScChangeActionTable aDelActionTable;
    const ScChangeAction* pChild=(const ScChangeAction*)pActionTable->First();

    while(pChild!=NULL)
    {

        if(pScChangeAction!=pChild)
            pEntry=InsertChangeAction(pChild,SC_CAS_VIRGIN,pParent,FALSE,TRUE);
        else
            pEntry=InsertChangeAction(pChild,SC_CAS_VIRGIN,pParent,TRUE,TRUE);

        if(pEntry!=NULL)
        {
            ScRedlinData *pEntryData=(ScRedlinData *)(pEntry->GetUserData());
            pEntryData->bIsRejectable=FALSE;
            pEntryData->bIsAcceptable=FALSE;
            pEntryData->bDisabled=TRUE;

            bTheTestFlag=FALSE;
            if ( pChild->IsDialogParent() )
            {
                Expand(pChanges,pChild,pEntry);
                /*
                pChanges->GetDependents((ScChangeAction*) pChild,aDelActionTable);
                if(aDelActionTable.First()!=NULL)
                {
                    pEntry->EnableChildsOnDemand(TRUE);
                }
                aDelActionTable.Clear();
                */
            }
        }
        pChild=pActionTable->Next();
    }
    return bTheTestFlag;
}

BOOL ScAcceptChgDlg::Expand(ScChangeTrack* pChanges,const ScChangeAction* pScChangeAction,
                            SvLBoxEntry* pEntry,BOOL bFilter)
{
    BOOL bTheTestFlag=TRUE;

    if(pChanges!=NULL &&pEntry!=NULL &&pScChangeAction!=NULL)
    {
        ScChangeActionTable aActionTable;

        GetDependents( pScChangeAction,aActionTable,pEntry);

        switch(pScChangeAction->GetType())
        {
            case SC_CAT_CONTENT:
            {
                InsertContentChilds(&aActionTable,pEntry);
                bTheTestFlag=!bHasFilterEntry;
                break;
            }
            case SC_CAT_DELETE_COLS:
            case SC_CAT_DELETE_ROWS:
            case SC_CAT_DELETE_TABS:
            {
                InsertDeletedChilds(pScChangeAction,&aActionTable,pEntry);
                bTheTestFlag=!bHasFilterEntry;
                break;
            }
            default:
            {
                if(!bFilter)
                    bTheTestFlag=InsertChilds(&aActionTable,pEntry);
                break;
            }
        }
        aActionTable.Clear();
    }
    return bTheTestFlag;
}

IMPL_LINK( ScAcceptChgDlg, ExpandingHandle, SvxRedlinTable*, pTable )
{
    ScChangeTrack* pChanges=pDoc->GetChangeTrack();
    ScChangeAction* pScChangeAction=NULL;
    SetPointer(Pointer(POINTER_WAIT));
    if(pTable!=NULL && pChanges!=NULL)
    {
        ScChangeActionTable aActionTable;
        SvLBoxEntry* pEntry=pTheView->GetHdlEntry();
        if(pEntry!=NULL)
        {
            ScRedlinData *pEntryData=(ScRedlinData *)(pEntry->GetUserData());
            if(pEntryData!=NULL)
            {
                pScChangeAction=(ScChangeAction*) pEntryData->pData;
            }

            if(pEntry->HasChildsOnDemand())
            {
                BOOL bTheTestFlag=TRUE;
                pEntry->EnableChildsOnDemand(FALSE);
                pTheView->GetModel()->Remove(pTheView->FirstChild(pEntry));

                SvLBoxEntry* pParent=NULL;
                SvLBoxEntry* pOriginal=NULL;
                if(pEntryData!=NULL)
                {
                    pScChangeAction=(ScChangeAction*) pEntryData->pData;

                    GetDependents( pScChangeAction,aActionTable,pEntry);

                    switch(pScChangeAction->GetType())
                    {
                        case SC_CAT_CONTENT:
                        {
                            bTheTestFlag=InsertContentChilds(&aActionTable,pEntry);
                            break;
                        }
                        case SC_CAT_DELETE_COLS:
                        case SC_CAT_DELETE_ROWS:
                        case SC_CAT_DELETE_TABS:
                        {
                            bTheTestFlag=InsertDeletedChilds(pScChangeAction,&aActionTable,pEntry);
                            break;
                        }
                        default:
                        {
                            bTheTestFlag=InsertChilds(&aActionTable,pEntry);
                            break;
                        }
                    }
                    aActionTable.Clear();

                }
                else
                {
                    bTheTestFlag=InsertAcceptedORejected(pEntry);
                }
                if(bTheTestFlag) pTheView->InsertEntry(aStrNoEntry,NULL,Color(COL_GRAY),pEntry);
            }

        }
    }
    SetPointer(Pointer(POINTER_ARROW));
    return (ULONG) TRUE;
}


void ScAcceptChgDlg::AppendChanges(ScChangeTrack* pChanges,ULONG nStartAction,
                                   ULONG nEndAction,ULONG nPos)
{
    if(pChanges!=NULL)
    {
        DateTime aDateTime;
        SvLBoxEntry* pParent=NULL;
        const ScChangeAction* pScChangeAction=NULL;
        bAcceptEnableFlag=TRUE;
        bRejectEnableFlag=TRUE;
        SetPointer(Pointer(POINTER_WAIT));
        pTheView->SetUpdateMode(FALSE);

        ScChangeActionTable ActionTable;
        BOOL bTheFlag=FALSE;

        BOOL bFilterFlag=pTPFilter->IsDate()||pTPFilter->IsRange()||
                     pTPFilter->IsAuthor()||pTPFilter->IsComment();

        bUseColor=bFilterFlag;

        for(ULONG i=nStartAction;i<=nEndAction;i++)
        {
            pScChangeAction=pChanges->GetAction(i);
            if(pScChangeAction==NULL) continue;


            switch(pScChangeAction->GetState())
            {
                case SC_CAS_VIRGIN:

                    if(pScChangeAction->IsDialogRoot())
                    {
                        if(pScChangeAction->IsDialogParent())
                            pParent=InsertChangeAction(pScChangeAction,SC_CAS_VIRGIN);
                        else
                            pParent=InsertFilteredAction(pScChangeAction,SC_CAS_VIRGIN);
                    }
                    else
                        pParent=NULL;

                    bTheFlag=TRUE;
                    break;

                case SC_CAS_ACCEPTED:
                    pParent=NULL;
                    nAcceptCount++;
                    break;

                case SC_CAS_REJECTED:
                    pParent=NULL;
                    nRejectCount++;
                    break;
            }

            if(pParent!=NULL && pScChangeAction->IsDialogParent())
            {
                if(!bFilterFlag)
                {
                    pParent->EnableChildsOnDemand(TRUE);
                }
                else
                {
                    BOOL bTestFlag=bHasFilterEntry;
                    bHasFilterEntry=FALSE;
                    if(Expand(pChanges,pScChangeAction,pParent,!bTestFlag)&&!bTestFlag)
                        pTheView->GetModel()->Remove(pParent);
                }
            }

            pScChangeAction=pScChangeAction->GetNext();
        }

        if(bTheFlag && !pDoc->IsDocEditable())
            bTheFlag=FALSE;

        pTPView->EnableAccept(bTheFlag);
        pTPView->EnableAcceptAll(bTheFlag);
        pTPView->EnableReject(bTheFlag);
        pTPView->EnableRejectAll(bTheFlag);

        pTheView->SetUpdateMode(TRUE);
        SetPointer(Pointer(POINTER_ARROW));
    }
}

void ScAcceptChgDlg::RemoveEntrys(ULONG nStartAction,ULONG nEndAction)
{

    pTheView->SetUpdateMode(FALSE);

    SvLBoxEntry* pEntry=pTheView->GetCurEntry();

    ScRedlinData *pEntryData=NULL;

    if(pEntry!=NULL)
        pEntryData=(ScRedlinData *)(pEntry->GetUserData());

    ULONG nAction=0;
    if(pEntryData!=NULL)
    {
        nAction=pEntryData->nActionNo;
    }

    if(nAction>=nStartAction && nAction<=nEndAction)
    {
        pTheView->SetCurEntry(pTheView->GetModel()->GetEntry(0));
    }

    BOOL bRemove=FALSE;

    // MUST do it backwards, don't delete parents before children and GPF
    pEntry=pTheView->Last();
    while(pEntry!=NULL)
    {
        bRemove=FALSE;
        pEntryData=(ScRedlinData *)(pEntry->GetUserData());
        if(pEntryData!=NULL)
        {
            nAction=pEntryData->nActionNo;

            if(nStartAction<=nAction && nAction<=nEndAction) bRemove=TRUE;


        }
        SvLBoxEntry* pPrevEntry = pTheView->Prev(pEntry);

        if(bRemove)
        {
            //delete pEntryData;
            pTheView->GetModel()->Remove(pEntry);
        }
        pEntry=pPrevEntry;
    }
    pTheView->SetUpdateMode(TRUE);

}

void ScAcceptChgDlg::UpdateEntrys(ScChangeTrack* pChgTrack, ULONG nStartAction,ULONG nEndAction)
{
    pTheView->SetUpdateMode(FALSE);

    ULONG nPos=LIST_APPEND;

    BOOL bRemove=FALSE;

    SvLBoxEntry* pEntry=pTheView->First();
    SvLBoxEntry* pNextEntry = (pEntry ? pTheView->NextSibling(pEntry) : NULL);
    SvLBoxEntry* pLastEntry=NULL;
    while(pEntry!=NULL)
    {
        bRemove=FALSE;
        ScRedlinData *pEntryData=(ScRedlinData *)(pEntry->GetUserData());
        if(pEntryData!=NULL)
        {
            ScChangeAction* pScChangeAction=
                    (ScChangeAction*) pEntryData->pData;

            ULONG nAction=pScChangeAction->GetActionNumber();

            if(nStartAction<=nAction && nAction<=nEndAction) bRemove=TRUE;
        }

        if(bRemove)
        {
            nPos=pEntry->GetChildListPos();
            pTheView->GetModel()->Remove(pEntry);

            if(pLastEntry==NULL) pLastEntry=pTheView->First();
            if(pLastEntry!=NULL)
            {
                pNextEntry=pTheView->Next(pLastEntry);

                if(pNextEntry==NULL)
                {
                    pNextEntry=pLastEntry;
                    pLastEntry=NULL;
                }
            }
            else
                pNextEntry=NULL;

        }
        else
        {
            pLastEntry = pEntry;
            pNextEntry = pTheView->Next(pEntry);
        }
        pEntry=pNextEntry;
    }

    if(nStartAction==nEndAction)
        AppendChanges(pChgTrack,nStartAction,nEndAction,nPos);
    else
        AppendChanges(pChgTrack,nStartAction,nEndAction);

    pTheView->SetUpdateMode(TRUE);

}

IMPL_LINK( ScAcceptChgDlg, ChgTrackModHdl, ScChangeTrack*, pChgTrack)
{
    if(bScAcceptChgDlgIsDead) return 0;

    ScChangeTrackMsgQueue& aMsgQueue= pChgTrack->GetMsgQueue();

    ScChangeTrackMsgInfo* pTrackInfo=aMsgQueue.Get();
    ULONG   nStartAction;
    ULONG   nEndAction;

    while(pTrackInfo!=NULL)
    {
        nStartAction=pTrackInfo->nStartAction;
        nEndAction=pTrackInfo->nEndAction;

        if(!bIgnoreMsg)
        {
            bNoSelection=TRUE;

            switch(pTrackInfo->eMsgType)
            {
                case SC_CTM_APPEND: AppendChanges(pChgTrack,nStartAction,nEndAction);
                                    break;
                case SC_CTM_REMOVE: RemoveEntrys(nStartAction,nEndAction);
                                    break;
                case SC_CTM_PARENT:
                case SC_CTM_CHANGE: //bNeedsUpdate=TRUE;
                                    UpdateEntrys(pChgTrack,nStartAction,nEndAction);
                                    break;
            }
        }
        delete pTrackInfo;
        pTrackInfo=aMsgQueue.Get();
    }

    return 0;
}
IMPL_LINK( ScAcceptChgDlg, ReOpenTimerHdl, Timer*, pTi)
{
    ScSimpleRefDlgWrapper::SetAutoReOpen(TRUE);
    aAcceptChgCtr.ShowFilterPage();
    RefHandle(NULL);

    return 0;
}
IMPL_LINK( ScAcceptChgDlg, UpdateSelectionHdl, Timer*, pTi)
{
    const ScChangeAction* pScChangeAction=NULL;
    ULONG nCount=pTheView->GetSelectionCount();

    SvLBoxEntry* pEntry=pTheView->FirstSelected();
    ScRedlinData *pEntryData=NULL;
    BOOL bAcceptFlag;
    BOOL bRejectFlag;

    bAcceptFlag=TRUE;
    bRejectFlag=TRUE;

    while(pEntry!=NULL)
    {
/*      if(nCount!=1)
        {
            bAcceptFlag=bAcceptEnableFlag;
            bRejectFlag=bRejectEnableFlag;
        }
        else
        {
            bAcceptFlag=TRUE;
            bRejectFlag=TRUE;
        }
*/
        pEntryData=(ScRedlinData *)(pEntry->GetUserData());
        if(pEntryData!=NULL)
        {
            if(bRejectFlag) bRejectFlag=pEntryData->bIsRejectable;
            if(bAcceptFlag) bAcceptFlag=pEntryData->bIsAcceptable;
        }
        else
        {
            bAcceptFlag=FALSE;
            bRejectFlag=FALSE;
        }
        bAcceptEnableFlag=bAcceptFlag;
        bRejectEnableFlag=bRejectFlag;

        pEntry=pTheView->NextSelected(pEntry);
    }

    pEntry=pTheView->GetCurEntry();

    if(pEntry!=NULL)
    {
        pEntryData=(ScRedlinData *)(pEntry->GetUserData());

        if(pEntryData!=NULL)
        {
            if(bRejectFlag) bRejectFlag=pEntryData->bIsRejectable;
            if(bAcceptFlag) bAcceptFlag=pEntryData->bIsAcceptable;

            pScChangeAction=(ScChangeAction*) pEntryData->pData;
            if(pScChangeAction && pScChangeAction->GetType()!=SC_CAT_DELETE_TABS
                && (!pEntryData->bDisabled || pScChangeAction->IsVisible()))
            {
                const ScBigRange& rRange = pScChangeAction->GetBigRange();
                if(rRange.IsValid(pDoc) && IsActive())
                {
                    ScRange aRef=rRange.MakeRange();
                    ScTabView* pTabView=pViewData->GetView();
                    pTabView->MarkRange(aRef);
                }
            }
        }
    }

    pTPView->EnableAccept(pDoc->IsDocEditable() && bAcceptFlag);
    pTPView->EnableReject(pDoc->IsDocEditable() && bRejectFlag);

    return 0;
}

IMPL_LINK( ScAcceptChgDlg, CommandHdl, Control*, pCtr)
{

    const CommandEvent aCEvt(pTheView->GetCommandEvent());

    if(aCEvt.GetCommand()==COMMAND_CONTEXTMENU)
    {
        ScPopupMenu aPopup(ScResId(RID_POPUP_CHANGES));

        aPopup.SetMenuFlags(MENU_FLAG_HIDEDISABLEDENTRIES);

        SvLBoxEntry* pEntry=pTheView->GetCurEntry();
        if(pEntry!=NULL)
        {
            pTheView->Select(pEntry);
        }
        else
        {
            aPopup.Deactivate();
        }

        USHORT  nSortedCol=pTheView->GetSortedCol();

        if(nSortedCol!=0xFFFF)
        {
            USHORT nItemId=nSortedCol+SC_SUB_SORT+1;

            aPopup.CheckItem(nItemId);

            PopupMenu *pSubMenu = aPopup.GetPopupMenu(SC_SUB_SORT);
            if (pSubMenu)
            {
                pSubMenu->CheckItem(nItemId);
            }
        }

        aPopup.EnableItem(SC_CHANGES_COMMENT,FALSE);

        if(pDoc->IsDocEditable() && pEntry!=NULL)
        {
            ScRedlinData *pEntryData=(ScRedlinData *)(pEntry->GetUserData());
            if(pEntryData!=NULL)
            {
                ScChangeAction* pScChangeAction=
                        (ScChangeAction*) pEntryData->pData;
                if(pScChangeAction!=NULL && !pTheView->GetParent(pEntry))
                    aPopup.EnableItem(SC_CHANGES_COMMENT);
            }
        }

        USHORT nCommand=aPopup.Execute( this, GetPointerPosPixel() );


        if(nCommand)
        {
            if(nCommand==SC_CHANGES_COMMENT)
            {
                if(pEntry!=NULL)
                {
                    ScRedlinData *pEntryData=(ScRedlinData *)(pEntry->GetUserData());
                    if(pEntryData!=NULL)
                    {
                        ScChangeAction* pScChangeAction=
                                (ScChangeAction*) pEntryData->pData;

                        pViewData->GetDocShell()->ExecuteChangeCommentDialog( pScChangeAction, this,FALSE);
                    }
                }
            }
            else
            {
                BOOL bSortDir=pTheView->GetSortDirection();
                USHORT nCol=nCommand-SC_SUB_SORT-1;
                if(nSortedCol==nCol) bSortDir=!bSortDir;
                pTheView->SortByCol(nCol,bSortDir);
                /*
                SC_SUB_SORT
                SC_SORT_ACTION
                SC_SORT_POSITION
                SC_SORT_AUTHOR
                SC_SORT_DATE
                SC_SORT_COMMENT
                */
            }
        }
    }
    return NULL;
}

void ScAcceptChgDlg::Initialize(SfxChildWinInfo *pInfo)
{
    String aStr;
    if(pInfo!=NULL)
    {
        if ( pInfo->aExtraString.Len() )
        {
            xub_StrLen nPos = pInfo->aExtraString.Search(
                String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("AcceptChgDat:")));

            // Versuche, den Alignment-String "ALIGN:(...)" einzulesen; wenn
            // er nicht vorhanden ist, liegt eine "altere Version vor
            if ( nPos != STRING_NOTFOUND )
            {
                xub_StrLen n1 = pInfo->aExtraString.Search('(', nPos);
                if ( n1 != STRING_NOTFOUND )
                {
                    xub_StrLen n2 = pInfo->aExtraString.Search(')', n1);
                    if ( n2 != STRING_NOTFOUND )
                    {
                        // Alignment-String herausschneiden
                        aStr = pInfo->aExtraString.Copy(nPos, n2 - nPos + 1);
                        pInfo->aExtraString.Erase(nPos, n2 - nPos + 1);
                        aStr.Erase(0, n1-nPos+1);
                    }
                }
            }
        }
    }
    SfxModelessDialog::Initialize(pInfo);

    if ( aStr.Len())
    {
        USHORT nCount=(USHORT)aStr.ToInt32();

        for(USHORT i=0;i<nCount;i++)
        {
            xub_StrLen n1 = aStr.Search(';');
            aStr.Erase(0, n1+1);
            pTheView->SetTab(i,(USHORT)aStr.ToInt32(),MAP_PIXEL);
        }
    }
}

//-------------------------------------------------------------------------

void ScAcceptChgDlg::FillInfo(SfxChildWinInfo& rInfo) const
{
    SfxModelessDialog::FillInfo(rInfo);
    rInfo.aExtraString.AppendAscii(RTL_CONSTASCII_STRINGPARAM( "AcceptChgDat:(" ));

    USHORT  nCount=pTheView->TabCount();

    rInfo.aExtraString += String::CreateFromInt32(nCount);
    rInfo.aExtraString += ';';
    for(USHORT i=0;i<nCount;i++)
    {
        rInfo.aExtraString += String::CreateFromInt32(pTheView->GetTab(i));
        rInfo.aExtraString += ';';
    }
    rInfo.aExtraString += ')';
}

void ScAcceptChgDlg::InitFilter()
{
    if(pTPFilter->IsDate()||pTPFilter->IsRange()||
        pTPFilter->IsAuthor()||pTPFilter->IsComment())
    {
        pTheView->SetFilterDate(pTPFilter->IsDate());
        pTheView->SetDateTimeMode(pTPFilter->GetDateMode());
        pTheView->SetFirstDate(pTPFilter->GetFirstDate());
        pTheView->SetLastDate(pTPFilter->GetLastDate());
        pTheView->SetFirstTime(pTPFilter->GetFirstTime());
        pTheView->SetLastTime(pTPFilter->GetLastTime());
        pTheView->SetFilterAuthor(pTPFilter->IsAuthor());
        pTheView->SetAuthor(pTPFilter->GetSelectedAuthor());

        pTheView->SetFilterComment(pTPFilter->IsComment());

        utl::SearchParam aSearchParam( pTPFilter->GetComment(),
                utl::SearchParam::SRCH_REGEXP,FALSE,FALSE,FALSE );

        pTheView->SetCommentParams(&aSearchParam);

        pTheView->UpdateFilterTest();
    }
}

void ScAcceptChgDlg::SetMyStaticData()
{
    bFilterPageVisible=aAcceptChgCtr.IsFilterPageVisible();
    aPrivChangeViewSet.SetHasDate(pTPFilter->IsDate());
    ScChgsDateMode eMode = (ScChgsDateMode) pTPFilter->GetDateMode();
    aPrivChangeViewSet.SetTheDateMode( eMode );
    Date aFirstDate( pTPFilter->GetFirstDate() );
    Time aFirstTime( pTPFilter->GetFirstTime() );
    Date aLastDate( pTPFilter->GetLastDate() );
    Time aLastTime( pTPFilter->GetLastTime() );
    switch ( eMode )
    {   // korrespondiert mit ScViewUtil::IsActionShown
        case SCDM_DATE_EQUAL :
        case SCDM_DATE_NOTEQUAL :
            aFirstTime.SetTime( 0 );
            aLastDate = aFirstDate;
            aLastTime.SetTime( 23595999 );
        break;
    }
    aPrivChangeViewSet.SetTheFirstDateTime( DateTime( aFirstDate, aFirstTime ) );
    aPrivChangeViewSet.SetTheLastDateTime( DateTime( aLastDate, aLastTime ) );
    aPrivChangeViewSet.SetHasAuthor(pTPFilter->IsAuthor());
    aPrivChangeViewSet.SetTheAuthorToShow(pTPFilter->GetSelectedAuthor());
    aPrivChangeViewSet.SetHasRange(pTPFilter->IsRange());
    aPrivChangeViewSet.SetHasComment(pTPFilter->IsComment());
    aPrivChangeViewSet.SetTheComment(pTPFilter->GetComment());

    ScRangeList aRangeList;

    String aRange;

    if(!bSimpleRefDlgStarted)
    {
        aRange=pTPFilter->GetRange();
    }
    else
    {
        USHORT nId = ScSimpleRefDlgWrapper::GetChildWindowId();
        SfxViewFrame* pViewFrm = pViewData->GetViewShell()->GetViewFrame();

        ScSimpleRefDlgWrapper* pWnd =(ScSimpleRefDlgWrapper*)pViewFrm->GetChildWindow( nId );

        if(pWnd!=NULL && pWnd->GetWindow()!=NULL)
        {
            aRange=pWnd->GetRefString();
        }
        else
        {
            aRange=ScSimpleRefDlg::GetLastRefString();
        }
    }

    aRangeList.Parse(aRange);
    aPrivChangeViewSet.SetTheRangeList(aRangeList);
}

IMPL_LINK( ScAcceptChgDlg, FilterModified, SvxTPFilter*, pRef )
{
    if(pRef!=NULL)
    {
        SetMyStaticData();
    }

    return 0;
}

#define CALC_DATE       3
#define CALC_POS        1

IMPL_LINK( ScAcceptChgDlg, ColCompareHdl, SvSortData*, pSortData )
{
    StringCompare eCompare=COMPARE_EQUAL;
    USHORT nSortCol=pTheView->GetSortedCol();

    if(pSortData)
    {
        SvLBoxEntry* pLeft = (SvLBoxEntry*)(pSortData->pLeft );
        SvLBoxEntry* pRight = (SvLBoxEntry*)(pSortData->pRight );

        if(CALC_DATE==nSortCol)
        {
            RedlinData *pLeftData=(RedlinData *)(pLeft->GetUserData());
            RedlinData *pRightData=(RedlinData *)(pRight->GetUserData());

            if(pLeftData!=NULL && pRightData!=NULL)
            {
                if(pLeftData->aDateTime < pRightData->aDateTime)
                {
                    eCompare=COMPARE_LESS;
                }
                else if(pLeftData->aDateTime > pRightData->aDateTime)
                {
                    eCompare=COMPARE_GREATER;
                }
                return eCompare;
            }
        }
        else if(CALC_POS==nSortCol)
        {
            ScRedlinData *pLeftData=(ScRedlinData *)(pLeft->GetUserData());
            ScRedlinData *pRightData=(ScRedlinData *)(pRight->GetUserData());

            if(pLeftData!=NULL && pRightData!=NULL)
            {
                eCompare=COMPARE_GREATER;

                if(pLeftData->nTable < pRightData->nTable)
                {
                    eCompare=COMPARE_LESS;
                }
                else if(pLeftData->nTable == pRightData->nTable)
                {
                    if(pLeftData->nRow < pRightData->nRow)
                    {
                        eCompare=COMPARE_LESS;
                    }
                    else if(pLeftData->nRow == pRightData->nRow)
                    {
                        if(pLeftData->nCol < pRightData->nCol)
                        {
                            eCompare=COMPARE_LESS;
                        }
                        else if(pLeftData->nCol == pRightData->nCol)
                        {
                            eCompare=COMPARE_EQUAL;
                        }
                    }
                }

                return eCompare;
            }
        }

        SvLBoxItem* pLeftItem = pTheView->GetEntryAtPos( pLeft, nSortCol);
        SvLBoxItem* pRightItem = pTheView->GetEntryAtPos( pRight, nSortCol);

        if(pLeftItem != NULL && pRightItem != NULL)
        {
            USHORT nLeftKind=pLeftItem->IsA();
            USHORT nRightKind=pRightItem->IsA();

            if(nRightKind == SV_ITEM_ID_LBOXSTRING &&
                nLeftKind == SV_ITEM_ID_LBOXSTRING )
            {
                eCompare= (StringCompare) ScGlobal::pCaseCollator->compareString(
                                        ((SvLBoxString*)pLeftItem)->GetText(),
                                        ((SvLBoxString*)pRightItem)->GetText());

                if(eCompare==COMPARE_EQUAL) eCompare=COMPARE_LESS;
            }
        }


    }
    return eCompare;
}

