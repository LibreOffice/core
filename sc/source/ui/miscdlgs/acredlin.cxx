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
#include <svl/undo.hxx>
#include <unotools/textsearch.hxx>
#include <unotools/localedatawrapper.hxx>
#include <unotools/collatorwrapper.hxx>
#include <vcl/msgbox.hxx>
#include <sfx2/app.hxx>
#include <sfx2/viewfrm.hxx>

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
#define ABS_DREF3D      ABS_DREF | SCA_TAB_3D

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
    bDisabled=false;
    bIsRejectable=false;
    bIsAcceptable=false;
    nTable=SCTAB_MAX;
    nCol=SCCOL_MAX;
    nRow=SCROW_MAX;
}

ScRedlinData::~ScRedlinData()
{
    nInfo=RD_SPECIAL_NONE;
    nActionNo=0;
    pData=NULL;
    bDisabled=false;
    bIsRejectable=false;
    bIsAcceptable=false;
}

//============================================================================
//  class ScAcceptChgDlg
//----------------------------------------------------------------------------
ScAcceptChgDlg::ScAcceptChgDlg(SfxBindings* pB, SfxChildWindow* pCW, Window* pParent,
    ScViewData* ptrViewData)
    : SfxModelessDialog(pB, pCW, pParent,
        "AcceptRejectChangesDialog", "svx/ui/acceptrejectchangesdialog.ui"),
        pViewData       ( ptrViewData ),
        pDoc            ( ptrViewData->GetDocument() ),
        aLocalRangeName ( *(pDoc->GetRangeName()) ),
        aStrInsertCols       (SC_RESSTR(STR_CHG_INSERT_COLS)),
        aStrInsertRows       (SC_RESSTR(STR_CHG_INSERT_ROWS)),
        aStrInsertTabs       (SC_RESSTR(STR_CHG_INSERT_TABS)),
        aStrDeleteCols       (SC_RESSTR(STR_CHG_DELETE_COLS)),
        aStrDeleteRows       (SC_RESSTR(STR_CHG_DELETE_ROWS)),
        aStrDeleteTabs       (SC_RESSTR(STR_CHG_DELETE_TABS)),
        aStrMove             (SC_RESSTR(STR_CHG_MOVE)),
        aStrContent          (SC_RESSTR(STR_CHG_CONTENT)),
        aStrReject           (SC_RESSTR(STR_CHG_REJECT)),
        aStrAllAccepted      (SC_RESSTR(STR_CHG_ACCEPTED)),
        aStrAllRejected      (SC_RESSTR(STR_CHG_REJECTED)),
        aStrNoEntry          (SC_RESSTR(STR_CHG_NO_ENTRY)),
        aStrContentWithChild (SC_RESSTR(STR_CHG_CONTENT_WITH_CHILD)),
        aStrChildContent     (SC_RESSTR(STR_CHG_CHILD_CONTENT)),
        aStrChildOrgContent  (SC_RESSTR(STR_CHG_CHILD_ORGCONTENT)),
        aStrEmpty            (SC_RESSTR(STR_CHG_EMPTY)),
        aUnknown("Unknown"),
        bAcceptEnableFlag(true),
        bRejectEnableFlag(true),
        bNeedsUpdate(false),
        bIgnoreMsg(false),
        bNoSelection(false),
        bHasFilterEntry(false),
        bUseColor(false)
{
    m_pAcceptChgCtr = new SvxAcceptChgCtr(get_content_area());
    nAcceptCount=0;
    nRejectCount=0;
    aReOpenTimer.SetTimeout(50);
    aReOpenTimer.SetTimeoutHdl(LINK( this, ScAcceptChgDlg, ReOpenTimerHdl ));

    pTPFilter=m_pAcceptChgCtr->GetFilterPage();
    pTPView=m_pAcceptChgCtr->GetViewPage();
    pTheView=pTPView->GetTableControl();
    aSelectionTimer.SetTimeout(100);
    aSelectionTimer.SetTimeoutHdl(LINK( this, ScAcceptChgDlg, UpdateSelectionHdl ));

    pTPFilter->SetReadyHdl(LINK( this, ScAcceptChgDlg, FilterHandle ));
    pTPFilter->SetRefHdl(LINK( this, ScAcceptChgDlg, RefHandle ));
    pTPFilter->SetModifyHdl(LINK( this, ScAcceptChgDlg, FilterModified));
    pTPFilter->HideRange(false);
    pTPView->InsertCalcHeader();
    pTPView->SetRejectClickHdl( LINK( this, ScAcceptChgDlg,RejectHandle));
    pTPView->SetAcceptClickHdl( LINK(this, ScAcceptChgDlg, AcceptHandle));
    pTPView->SetRejectAllClickHdl( LINK( this, ScAcceptChgDlg,RejectAllHandle));
    pTPView->SetAcceptAllClickHdl( LINK(this, ScAcceptChgDlg, AcceptAllHandle));
    pTheView->SetCalcView();
    pTheView->SetStyle(pTheView->GetStyle()|WB_HASLINES|WB_CLIPCHILDREN|WB_HASBUTTONS|WB_HASBUTTONSATROOT|WB_HSCROLL);
    pTheView->SetExpandingHdl( LINK(this, ScAcceptChgDlg, ExpandingHandle));
    pTheView->SetSelectHdl( LINK(this, ScAcceptChgDlg, SelectHandle));
    pTheView->SetDeselectHdl( LINK(this, ScAcceptChgDlg, SelectHandle));
    pTheView->SetCommandHdl( LINK(this, ScAcceptChgDlg, CommandHdl));
    pTheView->SetColCompareHdl( LINK(this, ScAcceptChgDlg,ColCompareHdl));
    pTheView->SetSelectionMode(MULTIPLE_SELECTION);
    pTheView->SetHighlightRange(1);

    Init();

    UpdateView();
    SvTreeListEntry* pEntry=pTheView->First();
    if(pEntry!=NULL)
    {
        pTheView->Select(pEntry);
    }
}

ScAcceptChgDlg::~ScAcceptChgDlg()
{
    ClearView();
    ScChangeTrack* pChanges=pDoc->GetChangeTrack();

    if(pChanges!=NULL)
    {
        Link aLink;
        pChanges->SetModifiedLink(aLink);
    }

    delete m_pAcceptChgCtr;
}

void ScAcceptChgDlg::ReInit(ScViewData* ptrViewData)
{
    pViewData=ptrViewData;
    if(pViewData!=NULL)
        pDoc=ptrViewData->GetDocument();
    else
        pDoc=NULL;

    bNoSelection=false;
    bNeedsUpdate=false;
    bIgnoreMsg=false;
    nAcceptCount=0;
    nRejectCount=0;
    bAcceptEnableFlag=true;
    bRejectEnableFlag=true;

    //  don't call Init here (switching between views), just set link below
    //  (dialog is just hidden, not deleted anymore, when switching views)
    ClearView();
    UpdateView();

    if ( pDoc )
    {
        ScChangeTrack* pChanges = pDoc->GetChangeTrack();
        if ( pChanges )
            pChanges->SetModifiedLink( LINK( this, ScAcceptChgDlg, ChgTrackModHdl ) );
    }
}

void ScAcceptChgDlg::Init()
{
    ScRange aRange;

    OSL_ENSURE( pViewData && pDoc, "ViewData oder Document nicht gefunden!" );

    ScChangeTrack* pChanges=pDoc->GetChangeTrack();

    if(pChanges!=NULL)
    {
        pChanges->SetModifiedLink( LINK( this, ScAcceptChgDlg,ChgTrackModHdl));
        aChangeViewSet.SetTheAuthorToShow(pChanges->GetUser());
        pTPFilter->ClearAuthors();
        const std::set<OUString>& rUserColl = pChanges->GetUserCollection();
        std::set<OUString>::const_iterator it = rUserColl.begin(), itEnd = rUserColl.end();
        for (; it != itEnd; ++it)
            pTPFilter->InsertAuthor(*it);
    }

    ScChangeViewSettings* pViewSettings=pDoc->GetChangeViewSettings();
    if ( pViewSettings!=NULL )
        aChangeViewSet = *pViewSettings;
    // adjust TimeField for filter tabpage
    aChangeViewSet.AdjustDateMode( *pDoc );

    pTPFilter->CheckDate(aChangeViewSet.HasDate());
    pTPFilter->SetFirstDate(aChangeViewSet.GetTheFirstDateTime());
    pTPFilter->SetFirstTime(aChangeViewSet.GetTheFirstDateTime());
    pTPFilter->SetLastDate(aChangeViewSet.GetTheLastDateTime());
    pTPFilter->SetLastTime(aChangeViewSet.GetTheLastDateTime());
    pTPFilter->SetDateMode((sal_uInt16)aChangeViewSet.GetTheDateMode());
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
        pTPFilter->SelectedAuthorPos(0);

    pTPFilter->CheckRange(aChangeViewSet.HasRange());

    aRangeList=aChangeViewSet.GetTheRangeList();

    if( !aChangeViewSet.GetTheRangeList().empty() )
    {
        const ScRange* pRangeEntry = aChangeViewSet.GetTheRangeList().front();
        String aRefStr;
        pRangeEntry->Format( aRefStr, ABS_DREF3D, pDoc );
        pTPFilter->SetRange(aRefStr);
    }

    InitFilter();
}

void ScAcceptChgDlg::ClearView()
{
    nAcceptCount=0;
    nRejectCount=0;
    pTheView->SetUpdateMode(false);

    pTheView->Clear();
    pTheView->SetUpdateMode(true);
}

OUString* ScAcceptChgDlg::MakeTypeString(ScChangeActionType eType)
{
    OUString* pStr;

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


bool ScAcceptChgDlg::IsValidAction(const ScChangeAction* pScChangeAction)
{
    if(pScChangeAction==NULL) return false;

    bool bFlag = false;

    ScRange aRef=pScChangeAction->GetBigRange().MakeRange();
    String aUser=pScChangeAction->GetUser();
    DateTime aDateTime=pScChangeAction->GetDateTime();

    ScChangeActionType eType=pScChangeAction->GetType();
    OUString aDesc;

    String aComment = comphelper::string::remove(pScChangeAction->GetComment(), '\n');

    if(eType==SC_CAT_CONTENT)
    {
        if(!pScChangeAction->IsDialogParent())
            pScChangeAction->GetDescription(aDesc, pDoc, true);
    }
    else
        pScChangeAction->GetDescription(aDesc, pDoc, !pScChangeAction->IsMasterDelete());

    if (!aDesc.isEmpty())
    {
        aComment.AppendAscii(RTL_CONSTASCII_STRINGPARAM( " (" ));
        aComment += String(aDesc);
        aComment += ')';
    }

    if(pTheView->IsValidEntry(&aUser,&aDateTime,&aComment))
    {
        if(pTPFilter->IsRange())
        {
            for ( size_t i = 0, nRanges = aRangeList.size(); i < nRanges; ++i )
            {
                ScRange* pRangeEntry = aRangeList[ i ];
                if (pRangeEntry->Intersects(aRef)) {
                    bFlag = true;
                    break;
                }
            }
        }
        else
            bFlag=true;
    }

    return bFlag;
}

SvTreeListEntry* ScAcceptChgDlg::InsertChangeAction(
    const ScChangeAction* pScChangeAction, ScChangeActionState /*eState*/,
    SvTreeListEntry* pParent, bool bDelMaster,bool bDisabled, sal_uLong nPos)
{
    ScChangeTrack* pChanges=pDoc->GetChangeTrack();

    if(pScChangeAction==NULL || pChanges==NULL) return NULL;

    SvTreeListEntry* pEntry=NULL;

    bool bFlag = false;

    ScRange aRef=pScChangeAction->GetBigRange().MakeRange();
    String aUser=pScChangeAction->GetUser();
    DateTime aDateTime=pScChangeAction->GetDateTime();

    OUString aRefStr;
    ScChangeActionType eType=pScChangeAction->GetType();
    OUStringBuffer aBuf;
    OUString aDesc;

    ScRedlinData* pNewData=new ScRedlinData;
    pNewData->pData=(void *)pScChangeAction;
    pNewData->nActionNo=pScChangeAction->GetActionNumber();
    pNewData->bIsAcceptable=pScChangeAction->IsClickable();
    pNewData->bIsRejectable=pScChangeAction->IsRejectable();
    pNewData->bDisabled=!pNewData->bIsAcceptable || bDisabled;
    pNewData->aDateTime=aDateTime;
    pNewData->nRow  = aRef.aStart.Row();
    pNewData->nCol  = aRef.aStart.Col();
    pNewData->nTable= aRef.aStart.Tab();

    if(eType==SC_CAT_CONTENT)
    {
        if(pScChangeAction->IsDialogParent())
        {
            aBuf.append(aStrContentWithChild);
            pNewData->nInfo=RD_SPECIAL_VISCONTENT;
            pNewData->bIsRejectable=false;
            pNewData->bIsAcceptable=false;
        }
        else
        {
            aBuf.append(*MakeTypeString(eType));
            pScChangeAction->GetDescription( aDesc, pDoc, true);
        }
    }
    else
    {
        aBuf.append(aStrContentWithChild);

        if(bDelMaster)
        {
            pScChangeAction->GetDescription( aDesc, pDoc,true);
            pNewData->bDisabled=true;
            pNewData->bIsRejectable=false;
        }
        else
            pScChangeAction->GetDescription( aDesc, pDoc,!pScChangeAction->IsMasterDelete());

    }

    pScChangeAction->GetRefString(aRefStr, pDoc, true);

    aBuf.append(sal_Unicode('\t'));
    aBuf.append(aRefStr);
    aBuf.append(sal_Unicode('\t'));

    bool bIsGenerated = false;

    if(!pChanges->IsGenerated(pScChangeAction->GetActionNumber()))
    {
        aBuf.append(aUser);
        aBuf.append(sal_Unicode('\t'));
        aBuf.append(ScGlobal::pLocaleData->getDate(aDateTime));
        aBuf.append(sal_Unicode(' '));
        aBuf.append(ScGlobal::pLocaleData->getTime(aDateTime));
        aBuf.append(sal_Unicode('\t'));

        bIsGenerated = false;
    }
    else
    {
        aBuf.append(sal_Unicode('\t'));
        aBuf.append(sal_Unicode('\t'));
        bIsGenerated = true;
    }

    String aComment = comphelper::string::remove(pScChangeAction->GetComment(), '\n');

    if (!aDesc.isEmpty())
    {
        aComment.AppendAscii(RTL_CONSTASCII_STRINGPARAM( " (" ));
        aComment += String(aDesc);
        aComment += ')';
    }

    aBuf.append(aComment);

    if(pTheView->IsValidEntry(&aUser,&aDateTime)|| bIsGenerated)
    {
        if(pTheView->IsValidComment(&aComment))
        {
            if(pTPFilter->IsRange())
            {
                for ( size_t i = 0, nRanges = aRangeList.size(); i < nRanges; ++i )
                {
                    ScRange* pRangeEntry = aRangeList[ i ];
                    if( pRangeEntry->Intersects(aRef) )
                    {
                        bHasFilterEntry=true;
                        bFlag=true;
                        break;
                    }
                }
            }
            else if(!bIsGenerated)
            {
                bHasFilterEntry=true;
                bFlag=true;
            }
        }
    }

    if(!bFlag&& bUseColor&& pParent==NULL)
    {
        pEntry = pTheView->InsertEntry(
            aBuf.makeStringAndClear() ,pNewData, Color(COL_LIGHTBLUE), pParent, nPos);
    }
    else if(bFlag&& bUseColor&& pParent!=NULL)
    {
        pEntry = pTheView->InsertEntry(
            aBuf.makeStringAndClear(), pNewData, Color(COL_GREEN), pParent, nPos);
        SvTreeListEntry* pExpEntry=pParent;

        while(pExpEntry!=NULL && !pTheView->IsExpanded(pExpEntry))
        {
            SvTreeListEntry* pTmpEntry=pTheView->GetParent(pExpEntry);

            if(pTmpEntry!=NULL) pTheView->Expand(pExpEntry);

            pExpEntry=pTmpEntry;
        }
    }
    else
    {
        pEntry = pTheView->InsertEntry(
            aBuf.makeStringAndClear(), pNewData, pParent, nPos);
    }
    return pEntry;
}

SvTreeListEntry* ScAcceptChgDlg::InsertFilteredAction(
    const ScChangeAction* pScChangeAction, ScChangeActionState eState,
    SvTreeListEntry* pParent, bool bDelMaster, bool bDisabled, sal_uLong nPos)
{

    ScChangeTrack* pChanges=pDoc->GetChangeTrack();

    if(pScChangeAction==NULL || pChanges==NULL) return NULL;

    bool bIsGenerated = pChanges->IsGenerated(pScChangeAction->GetActionNumber());

    SvTreeListEntry* pEntry=NULL;

    bool bFlag = false;

    ScRange aRef=pScChangeAction->GetBigRange().MakeRange();
    String aUser=pScChangeAction->GetUser();
    DateTime aDateTime=pScChangeAction->GetDateTime();

    if(pTheView->IsValidEntry(&aUser,&aDateTime)||bIsGenerated)
    {
        if(pTPFilter->IsRange())
        {
            for ( size_t i = 0, nRanges = aRangeList.size(); i < nRanges; ++i )
            {
                ScRange* pRangeEntry=aRangeList[ i ];
                if( pRangeEntry->Intersects(aRef) )
                {
                    if( pScChangeAction->GetState()==eState )
                        bFlag = true;
                    break;
                }
            }
        }
        else if(pScChangeAction->GetState()==eState && !bIsGenerated)
            bFlag = true;
    }

    if(bFlag)
    {

        OUString aRefStr;
        ScChangeActionType eType=pScChangeAction->GetType();
        String aString;
        OUString aDesc;

        ScRedlinData* pNewData=new ScRedlinData;
        pNewData->pData=(void *)pScChangeAction;
        pNewData->nActionNo=pScChangeAction->GetActionNumber();
        pNewData->bIsAcceptable=pScChangeAction->IsClickable();
        pNewData->bIsRejectable=pScChangeAction->IsRejectable();
        pNewData->bDisabled=!pNewData->bIsAcceptable || bDisabled;
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
                pNewData->bIsRejectable=false;
                pNewData->bIsAcceptable=false;
            }
            else
            {
                aString=*MakeTypeString(eType);
                pScChangeAction->GetDescription( aDesc, pDoc, true);
            }
        }
        else
        {
            aString=*MakeTypeString(eType);

            if(bDelMaster)
            {
                pScChangeAction->GetDescription( aDesc, pDoc,true);
                pNewData->bDisabled=true;
                pNewData->bIsRejectable=false;
            }
            else
                pScChangeAction->GetDescription( aDesc, pDoc,!pScChangeAction->IsMasterDelete());

        }

        aString+='\t';
        pScChangeAction->GetRefString(aRefStr, pDoc, true);
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

        String aComment = comphelper::string::remove(pScChangeAction->GetComment(), '\n');

        if (!aDesc.isEmpty())
        {
            aComment.AppendAscii(RTL_CONSTASCII_STRINGPARAM( " (" ));
            aComment += String(aDesc);
            aComment += ')';
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

SvTreeListEntry* ScAcceptChgDlg::InsertChangeActionContent(const ScChangeActionContent* pScChangeAction,
                                                          SvTreeListEntry* pParent, sal_uLong nSpecial)
{
    ScChangeTrack* pChanges=pDoc->GetChangeTrack();
    SvTreeListEntry* pEntry=NULL;

    if(pScChangeAction==NULL || pChanges==NULL) return NULL;

    bool bIsGenerated = pChanges->IsGenerated(pScChangeAction->GetActionNumber());

    bool bFlag = false;

    ScRange aRef=pScChangeAction->GetBigRange().MakeRange();
    String aUser=pScChangeAction->GetUser();
    DateTime aDateTime=pScChangeAction->GetDateTime();

    if(pTheView->IsValidEntry(&aUser,&aDateTime)||bIsGenerated)
    {
        if(pTPFilter->IsRange())
        {
            for ( size_t i = 0, nRanges = aRangeList.size(); i < nRanges; ++i )
            {
                ScRange* pRangeEntry = aRangeList[ i ];
                if( pRangeEntry->Intersects(aRef) )
                {
                    bFlag=true;
                    break;
                }
            }
        }
        else if(!bIsGenerated)
            bFlag=true;
    }

    OUString aRefStr;
    String aString;
    String a2String;
    String aDesc;

    if(nSpecial==RD_SPECIAL_CONTENT)
    {
        OUString aTmp;
        pScChangeAction->GetOldString(aTmp, pDoc);
        a2String = aTmp;
        if(a2String.Len()==0) a2String=aStrEmpty;

        //aString+="\'";
        aString+=a2String;
        //aString+="\'";

        aDesc=aStrChildOrgContent;
        aDesc.AppendAscii(RTL_CONSTASCII_STRINGPARAM( ": " ));
    }
    else
    {
        OUString aTmp;
        pScChangeAction->GetNewString(aTmp, pDoc);
        a2String = aTmp;
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
    pScChangeAction->GetRefString(aRefStr, pDoc, true);
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

    String aComment = comphelper::string::remove(pScChangeAction->GetComment(), '\n');

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
    pNewData->bIsRejectable=false;
    pNewData->bDisabled=!pNewData->bIsAcceptable;
    pNewData->aDateTime=aDateTime;
    pNewData->nRow  = aRef.aStart.Row();
    pNewData->nCol  = aRef.aStart.Col();
    pNewData->nTable= aRef.aStart.Tab();

    if(pTheView->IsValidComment(&aComment) && bFlag)
    {
        bHasFilterEntry=true;
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
        bNoSelection=false;
    }

    return SfxModelessDialog::PreNotify(rNEvt);
}


void ScAcceptChgDlg::UpdateView()
{
    bNeedsUpdate=false;
    SvTreeListEntry* pParent=NULL;
    ScChangeTrack* pChanges=NULL;
    const ScChangeAction* pScChangeAction=NULL;
    bAcceptEnableFlag=true;
    bRejectEnableFlag=true;
    SetPointer(Pointer(POINTER_WAIT));
    pTheView->SetUpdateMode(false);
    bool bFilterFlag = pTPFilter->IsDate() || pTPFilter->IsRange() ||
        pTPFilter->IsAuthor() || pTPFilter->IsComment();

    bUseColor = bFilterFlag;

    if(pDoc!=NULL)
    {
        pChanges=pDoc->GetChangeTrack();
        if(pChanges!=NULL)
            pScChangeAction=pChanges->GetFirst();
    }
    bool bTheFlag = false;

    while(pScChangeAction!=NULL)
    {
        bHasFilterEntry=false;
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

                bTheFlag=true;
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
                pParent->EnableChildrenOnDemand(true);
            else
            {
                bool bTestFlag = bHasFilterEntry;
                bHasFilterEntry=false;
                if(Expand(pChanges,pScChangeAction,pParent,!bTestFlag)&&!bTestFlag)
                    pTheView->RemoveEntry(pParent);
            }
        }

        pScChangeAction=pScChangeAction->GetNext();
    }

    if( bTheFlag && (!pDoc->IsDocEditable() || pChanges->IsProtected()) )
        bTheFlag=false;

    pTPView->EnableAccept(bTheFlag);
    pTPView->EnableAcceptAll(bTheFlag);
    pTPView->EnableReject(bTheFlag);
    pTPView->EnableRejectAll(bTheFlag);

    if(nAcceptCount>0)
    {
        pParent=pTheView->InsertEntry(
            aStrAllAccepted, static_cast< RedlinData * >(NULL),
            static_cast< SvTreeListEntry * >(NULL));
        pParent->EnableChildrenOnDemand(true);
    }
    if(nRejectCount>0)
    {
        pParent=pTheView->InsertEntry(
            aStrAllRejected, static_cast< RedlinData * >(NULL),
            static_cast< SvTreeListEntry * >(NULL));
        pParent->EnableChildrenOnDemand(true);
    }
    pTheView->SetUpdateMode(true);
    SetPointer(Pointer(POINTER_ARROW));
    SvTreeListEntry* pEntry=pTheView->First();
    if(pEntry!=NULL)
        pTheView->Select(pEntry);
}

IMPL_LINK_NOARG(ScAcceptChgDlg, RefHandle)
{
    sal_uInt16 nId  =ScSimpleRefDlgWrapper::GetChildWindowId();

    ScSimpleRefDlgWrapper::SetDefaultPosSize(GetPosPixel(),GetSizePixel(),true);

    SC_MOD()->SetRefDialog( nId, true );

    SfxViewFrame* pViewFrm = pViewData->GetViewShell()->GetViewFrame();
    ScSimpleRefDlgWrapper* pWnd =(ScSimpleRefDlgWrapper*)pViewFrm->GetChildWindow( nId );

    if(pWnd!=NULL)
    {
        sal_uInt16 nAcceptId=ScAcceptChgDlgWrapper::GetChildWindowId();
        pViewFrm->ShowChildWindow(nAcceptId,false);
        pWnd->SetCloseHdl(LINK( this, ScAcceptChgDlg,RefInfoHandle));
        pWnd->SetRefString(pTPFilter->GetRange());
        pWnd->SetAutoReOpen(false);
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
    sal_uInt16 nId;

    ScSimpleRefDlgWrapper::SetAutoReOpen(true);

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
            Size aWinSize=pWin->GetSizePixel();
            aWinSize.Width()=GetSizePixel().Width();
            SetPosSizePixel(pWin->GetPosPixel(),aWinSize);
            Invalidate();
        }
        nId = ScAcceptChgDlgWrapper::GetChildWindowId();
        pViewFrm->ShowChildWindow( nId, true );
    }
    else
    {
        nId = ScAcceptChgDlgWrapper::GetChildWindowId();
        pViewFrm->SetChildWindow( nId, false );
    }
    return 0;
}

IMPL_LINK( ScAcceptChgDlg, FilterHandle, SvxTPFilter*, pRef )
{
    if(pRef!=NULL)
    {
        ClearView();
        aRangeList.RemoveAll();
        aRangeList.Parse(pTPFilter->GetRange(),pDoc);
        UpdateView();
    }
    return 0;
}

IMPL_LINK( ScAcceptChgDlg, RejectHandle, SvxTPView*, pRef )
{
    SetPointer(Pointer(POINTER_WAIT));

    bIgnoreMsg=true;
    ScChangeTrack* pChanges=pDoc->GetChangeTrack();

    if(pRef!=NULL)
    {
        SvTreeListEntry* pEntry=pTheView->FirstSelected();
        while(pEntry!=NULL)
        {
            ScRedlinData *pEntryData=(ScRedlinData *)(pEntry->GetUserData());
            if(pEntryData!=NULL)
            {
                ScChangeAction* pScChangeAction=
                        (ScChangeAction*) pEntryData->pData;

                if(pScChangeAction->GetType()==SC_CAT_INSERT_TABS)
                    pViewData->SetTabNo(0);

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

    bIgnoreMsg=false;
    return 0;
}
IMPL_LINK( ScAcceptChgDlg, AcceptHandle, SvxTPView*, pRef )
{
    SetPointer(Pointer(POINTER_WAIT));

    ScChangeTrack* pChanges=pDoc->GetChangeTrack();
    bIgnoreMsg=true;
    if(pRef!=NULL)
    {
        SvTreeListEntry* pEntry=pTheView->FirstSelected();
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
                        pChanges->SelectContent(pScChangeAction,true);
                    else
                        pChanges->SelectContent(pScChangeAction);
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
    bIgnoreMsg=false;

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
            if(IsValidAction(pScChangeAction))
                pChanges->Reject((ScChangeAction*)pScChangeAction);

        pScChangeAction=pScChangeAction->GetPrev();
    }
}
void ScAcceptChgDlg::AcceptFiltered()
{
    if(pDoc==NULL) return;
    ScChangeTrack* pChanges=pDoc->GetChangeTrack();
    const ScChangeAction* pScChangeAction=NULL;

    if(pChanges!=NULL)
        pScChangeAction=pChanges->GetLast();

    while(pScChangeAction!=NULL)
    {
        if(pScChangeAction->IsDialogRoot())
            if(IsValidAction(pScChangeAction))
                pChanges->Accept((ScChangeAction*)pScChangeAction);

        pScChangeAction=pScChangeAction->GetPrev();
    }
}

IMPL_LINK_NOARG(ScAcceptChgDlg, RejectAllHandle)
{
    SetPointer(Pointer(POINTER_WAIT));
    bIgnoreMsg=true;
    ScChangeTrack* pChanges=pDoc->GetChangeTrack();
    if(pChanges!=NULL)
    {
        if(pTPFilter->IsDate()||pTPFilter->IsAuthor()||pTPFilter->IsRange()||pTPFilter->IsComment())
            RejectFiltered();
        else
            pChanges->RejectAll();

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

    bIgnoreMsg=false;

    return 0;
}

IMPL_LINK_NOARG(ScAcceptChgDlg, AcceptAllHandle)
{
    SetPointer(Pointer(POINTER_WAIT));

    bIgnoreMsg=true;
    ScChangeTrack* pChanges=pDoc->GetChangeTrack();
    if(pChanges!=NULL)
    {
        if(pTPFilter->IsDate()||pTPFilter->IsAuthor()||pTPFilter->IsRange()||pTPFilter->IsComment())
            AcceptFiltered();
        else
            pChanges->AcceptAll();

        ScDocShell* pDocSh=pViewData->GetDocShell();
        pDocSh->PostPaintExtras();
        pDocSh->PostPaintGridAll();
        pDocSh->SetDocumentModified();
        ClearView();
        UpdateView();
    }
    bIgnoreMsg=false;
    SetPointer(Pointer(POINTER_ARROW));

    return 0;
}

IMPL_LINK_NOARG(ScAcceptChgDlg, SelectHandle)
{
    if(!bNoSelection)
        aSelectionTimer.Start();

    bNoSelection=false;
    return 0;
}

void ScAcceptChgDlg::GetDependents(  const ScChangeAction* pScChangeAction,
                                    ScChangeActionMap& aActionMap,
                                    SvTreeListEntry* pEntry)
{
    ScChangeTrack* pChanges=pDoc->GetChangeTrack();

    SvTreeListEntry* pParent=pTheView->GetParent(pEntry);
    if(pParent!=NULL)
    {
        ScRedlinData *pParentData=(ScRedlinData *)(pParent->GetUserData());
        ScChangeAction* pParentAction=(ScChangeAction*) pParentData->pData;

        if(pParentAction!=pScChangeAction)
            pChanges->GetDependents((ScChangeAction*) pScChangeAction,
                        aActionMap,pScChangeAction->IsMasterDelete());
        else
            pChanges->GetDependents( (ScChangeAction*) pScChangeAction,
                        aActionMap );
    }
    else
        pChanges->GetDependents((ScChangeAction*) pScChangeAction,
                    aActionMap, pScChangeAction->IsMasterDelete() );
}

bool ScAcceptChgDlg::InsertContentChildren(ScChangeActionMap* pActionMap,SvTreeListEntry* pParent)
{
    bool bTheTestFlag = true;
    ScRedlinData *pEntryData=(ScRedlinData *)(pParent->GetUserData());
    const ScChangeAction* pScChangeAction = (ScChangeAction*) pEntryData->pData;
    bool bParentInserted = false;
    // If the parent is a MatrixOrigin then place it in the right order before
    // the MatrixReferences. Also if it is the first content change at this
    // position don't insert the first dependent MatrixReference as the special
    // content (original value) but insert the predecessor of the MatrixOrigin
    // itself instead.
    if ( pScChangeAction->GetType() == SC_CAT_CONTENT &&
            ((const ScChangeActionContent*)pScChangeAction)->IsMatrixOrigin() )
    {
        pActionMap->insert( ::std::make_pair( pScChangeAction->GetActionNumber(),
            const_cast<ScChangeAction*>( pScChangeAction ) ) );
        bParentInserted = true;
    }
    SvTreeListEntry* pEntry=NULL;

    ScChangeActionMap::iterator itChangeAction = pActionMap->begin();
    while( itChangeAction != pActionMap->end() )
    {
        if( itChangeAction->second->GetState()==SC_CAS_VIRGIN )
            break;
        ++itChangeAction;
    }

    if( itChangeAction == pActionMap->end() )
        return true;

    SvTreeListEntry* pOriginal = InsertChangeActionContent(
        dynamic_cast<const ScChangeActionContent*>( itChangeAction->second ),
        pParent, RD_SPECIAL_CONTENT );

    if(pOriginal!=NULL)
    {
        bTheTestFlag=false;
        ScRedlinData *pParentData=(ScRedlinData *)(pOriginal->GetUserData());
        pParentData->pData=(void *)pScChangeAction;
        pParentData->nActionNo=pScChangeAction->GetActionNumber();
        pParentData->bIsAcceptable=pScChangeAction->IsRejectable(); // select old value
        pParentData->bIsRejectable=false;
        pParentData->bDisabled=false;
    }
    while( itChangeAction != pActionMap->end() )
    {
        if( itChangeAction->second->GetState() == SC_CAS_VIRGIN )
        {
            pEntry = InsertChangeActionContent( dynamic_cast<const ScChangeActionContent*>( itChangeAction->second ),
                pParent, RD_SPECIAL_NONE );

            if(pEntry!=NULL)
                bTheTestFlag=false;
        }
        ++itChangeAction;
    }

    if ( !bParentInserted )
    {
        pEntry=InsertChangeActionContent((const ScChangeActionContent*)
                                pScChangeAction,pParent,RD_SPECIAL_NONE);

        if(pEntry!=NULL)
        {
            bTheTestFlag=false;
            ScRedlinData *pParentData=(ScRedlinData *)(pEntry->GetUserData());
            pParentData->pData=(void *)pScChangeAction;
            pParentData->nActionNo=pScChangeAction->GetActionNumber();
            pParentData->bIsAcceptable=pScChangeAction->IsClickable();
            pParentData->bIsRejectable=false;
            pParentData->bDisabled=false;
        }
    }

    return bTheTestFlag;

}

bool ScAcceptChgDlg::InsertAcceptedORejected(SvTreeListEntry* pParent)
{
    ScChangeTrack* pChanges=pDoc->GetChangeTrack();
    bool bTheTestFlag = true;

    ScChangeActionState eState = SC_CAS_VIRGIN;
    OUString aString = pTheView->GetEntryText(pParent);
    OUString a2String = aString.copy(0, aStrAllAccepted.getLength());
    if (a2String.equals(aStrAllAccepted))
        eState=SC_CAS_ACCEPTED;
    else
    {
        a2String = aString.copy(0, aStrAllRejected.getLength());
        if (a2String.equals(aStrAllRejected))
            eState=SC_CAS_REJECTED;
    }

    ScChangeAction* pScChangeAction=pChanges->GetFirst();
    while(pScChangeAction!=NULL)
    {
        if(pScChangeAction->GetState()==eState &&
            InsertFilteredAction(pScChangeAction,eState,pParent)!=NULL)
            bTheTestFlag=false;
        pScChangeAction=pScChangeAction->GetNext();
    }
    return bTheTestFlag;
}

bool ScAcceptChgDlg::InsertChildren(ScChangeActionMap* pActionMap,SvTreeListEntry* pParent)
{
    ScChangeTrack* pChanges=pDoc->GetChangeTrack();
    bool bTheTestFlag = true;
    SvTreeListEntry* pEntry=NULL;
    ScChangeActionMap::iterator itChangeAction;

    for( itChangeAction = pActionMap->begin(); itChangeAction != pActionMap->end(); ++itChangeAction )
    {
        pEntry=InsertChangeAction( itChangeAction->second, SC_CAS_VIRGIN, pParent, false, true );

        if(pEntry!=NULL)
        {
            bTheTestFlag=false;

            ScRedlinData *pEntryData=(ScRedlinData *)(pEntry->GetUserData());
            pEntryData->bIsRejectable=false;
            pEntryData->bIsAcceptable=false;
            pEntryData->bDisabled=true;

            if( itChangeAction->second->IsDialogParent() )
                Expand( pChanges, itChangeAction->second, pEntry );
        }
    }
    return bTheTestFlag;
}

bool ScAcceptChgDlg::InsertDeletedChildren(const ScChangeAction* pScChangeAction,
                                         ScChangeActionMap* pActionMap,SvTreeListEntry* pParent)
{
    ScChangeTrack* pChanges=pDoc->GetChangeTrack();
    bool bTheTestFlag = true;
    SvTreeListEntry* pEntry=NULL;
    ScChangeActionMap::iterator itChangeAction;

    for( itChangeAction = pActionMap->begin(); itChangeAction != pActionMap->end(); ++itChangeAction )
    {

        if( pScChangeAction != itChangeAction->second )
            pEntry = InsertChangeAction( itChangeAction->second, SC_CAS_VIRGIN, pParent, false, true );
        else
            pEntry = InsertChangeAction( itChangeAction->second, SC_CAS_VIRGIN, pParent, true, true );

        if(pEntry!=NULL)
        {
            ScRedlinData *pEntryData=(ScRedlinData *)(pEntry->GetUserData());
            pEntryData->bIsRejectable=false;
            pEntryData->bIsAcceptable=false;
            pEntryData->bDisabled=true;

            bTheTestFlag=false;

            if( itChangeAction->second->IsDialogParent() )
                Expand( pChanges, itChangeAction->second, pEntry );
        }
    }
    return bTheTestFlag;
}

bool ScAcceptChgDlg::Expand(
    ScChangeTrack* pChanges, const ScChangeAction* pScChangeAction,
    SvTreeListEntry* pEntry, bool bFilter)
{
    bool bTheTestFlag = true;

    if(pChanges!=NULL &&pEntry!=NULL &&pScChangeAction!=NULL)
    {
        ScChangeActionMap aActionMap;

        GetDependents( pScChangeAction, aActionMap, pEntry );

        switch(pScChangeAction->GetType())
        {
            case SC_CAT_CONTENT:
            {
                InsertContentChildren( &aActionMap, pEntry );
                bTheTestFlag=!bHasFilterEntry;
                break;
            }
            case SC_CAT_DELETE_COLS:
            case SC_CAT_DELETE_ROWS:
            case SC_CAT_DELETE_TABS:
            {
                InsertDeletedChildren( pScChangeAction, &aActionMap, pEntry );
                bTheTestFlag=!bHasFilterEntry;
                break;
            }
            default:
            {
                if(!bFilter)
                    bTheTestFlag = InsertChildren( &aActionMap, pEntry );
                break;
            }
        }
        aActionMap.clear();
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
        ScChangeActionMap aActionMap;
        SvTreeListEntry* pEntry=pTheView->GetHdlEntry();
        if(pEntry!=NULL)
        {
            ScRedlinData *pEntryData=(ScRedlinData *)(pEntry->GetUserData());
            if(pEntryData!=NULL)
                pScChangeAction=(ScChangeAction*) pEntryData->pData;

            if(pEntry->HasChildrenOnDemand())
            {
                bool bTheTestFlag = true;
                pEntry->EnableChildrenOnDemand(false);
                pTheView->RemoveEntry(pTheView->FirstChild(pEntry));

                if(pEntryData!=NULL)
                {
                    pScChangeAction=(ScChangeAction*) pEntryData->pData;

                    GetDependents( pScChangeAction, aActionMap, pEntry );

                    switch(pScChangeAction->GetType())
                    {
                        case SC_CAT_CONTENT:
                        {
                            bTheTestFlag = InsertContentChildren( &aActionMap, pEntry );
                            break;
                        }
                        case SC_CAT_DELETE_COLS:
                        case SC_CAT_DELETE_ROWS:
                        case SC_CAT_DELETE_TABS:
                        {
                            bTheTestFlag = InsertDeletedChildren( pScChangeAction, &aActionMap, pEntry );
                            break;
                        }
                        default:
                        {
                            bTheTestFlag = InsertChildren( &aActionMap, pEntry );
                            break;
                        }
                    }
                    aActionMap.clear();

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
    return (sal_uLong) true;
}


void ScAcceptChgDlg::AppendChanges(ScChangeTrack* pChanges,sal_uLong nStartAction,
                                   sal_uLong nEndAction, sal_uLong /* nPos */)
{
    if(pChanges!=NULL)
    {
        SvTreeListEntry* pParent=NULL;
        const ScChangeAction* pScChangeAction=NULL;
        bAcceptEnableFlag=true;
        bRejectEnableFlag=true;
        SetPointer(Pointer(POINTER_WAIT));
        pTheView->SetUpdateMode(false);

        bool bTheFlag = false;

        bool bFilterFlag = pTPFilter->IsDate() || pTPFilter->IsRange() ||
            pTPFilter->IsAuthor() || pTPFilter->IsComment();

        bUseColor = bFilterFlag;

        for(sal_uLong i=nStartAction;i<=nEndAction;i++)
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

                    bTheFlag=true;
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
                    pParent->EnableChildrenOnDemand(true);
                else
                {
                    bool bTestFlag = bHasFilterEntry;
                    bHasFilterEntry = false;
                    if(Expand(pChanges,pScChangeAction,pParent,!bTestFlag)&&!bTestFlag)
                        pTheView->RemoveEntry(pParent);
                }
            }
        }

        if( bTheFlag && (!pDoc->IsDocEditable() || pChanges->IsProtected()) )
            bTheFlag=false;

        pTPView->EnableAccept(bTheFlag);
        pTPView->EnableAcceptAll(bTheFlag);
        pTPView->EnableReject(bTheFlag);
        pTPView->EnableRejectAll(bTheFlag);

        pTheView->SetUpdateMode(true);
        SetPointer(Pointer(POINTER_ARROW));
    }
}

void ScAcceptChgDlg::RemoveEntrys(sal_uLong nStartAction,sal_uLong nEndAction)
{

    pTheView->SetUpdateMode(false);

    SvTreeListEntry* pEntry=pTheView->GetCurEntry();

    ScRedlinData *pEntryData=NULL;

    if(pEntry!=NULL)
        pEntryData=(ScRedlinData *)(pEntry->GetUserData());

    sal_uLong nAction=0;
    if(pEntryData!=NULL)
        nAction=pEntryData->nActionNo;

    if(nAction>=nStartAction && nAction<=nEndAction)
        pTheView->SetCurEntry(pTheView->GetModel()->GetEntry(0));

    bool bRemove = false;

    // MUST do it backwards, don't delete parents before children and GPF
    pEntry=pTheView->Last();
    while(pEntry!=NULL)
    {
        bRemove=false;
        pEntryData=(ScRedlinData *)(pEntry->GetUserData());
        if(pEntryData!=NULL)
        {
            nAction=pEntryData->nActionNo;

            if(nStartAction<=nAction && nAction<=nEndAction) bRemove=true;


        }
        SvTreeListEntry* pPrevEntry = pTheView->Prev(pEntry);

        if(bRemove)
            pTheView->RemoveEntry(pEntry);

        pEntry=pPrevEntry;
    }
    pTheView->SetUpdateMode(true);

}

void ScAcceptChgDlg::UpdateEntrys(ScChangeTrack* pChgTrack, sal_uLong nStartAction,sal_uLong nEndAction)
{
    pTheView->SetUpdateMode(false);

    sal_uLong nPos=LIST_APPEND;

    bool bRemove = false;

    SvTreeListEntry* pEntry=pTheView->First();
    SvTreeListEntry* pNextEntry = (pEntry ? pTheView->NextSibling(pEntry) : NULL);
    SvTreeListEntry* pLastEntry=NULL;
    while(pEntry!=NULL)
    {
        bRemove=false;
        ScRedlinData *pEntryData=(ScRedlinData *)(pEntry->GetUserData());
        if(pEntryData!=NULL)
        {
            ScChangeAction* pScChangeAction=
                    (ScChangeAction*) pEntryData->pData;

            sal_uLong nAction=pScChangeAction->GetActionNumber();

            if(nStartAction<=nAction && nAction<=nEndAction) bRemove=true;
        }

        if(bRemove)
        {
            nPos=pEntry->GetChildListPos();
            pTheView->RemoveEntry(pEntry);

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

    pTheView->SetUpdateMode(true);

}

IMPL_LINK( ScAcceptChgDlg, ChgTrackModHdl, ScChangeTrack*, pChgTrack)
{
    ScChangeTrackMsgQueue::iterator iter;
    ScChangeTrackMsgQueue& aMsgQueue= pChgTrack->GetMsgQueue();

    sal_uLong   nStartAction;
    sal_uLong   nEndAction;

    for (iter = aMsgQueue.begin(); iter != aMsgQueue.end(); ++iter)
    {
        nStartAction=(*iter)->nStartAction;
        nEndAction=(*iter)->nEndAction;

        if(!bIgnoreMsg)
        {
            bNoSelection=true;

            switch((*iter)->eMsgType)
            {
                case SC_CTM_APPEND: AppendChanges(pChgTrack,nStartAction,nEndAction);
                                    break;
                case SC_CTM_REMOVE: RemoveEntrys(nStartAction,nEndAction);
                                    break;
                case SC_CTM_PARENT:
                case SC_CTM_CHANGE: //bNeedsUpdate=true;
                                    UpdateEntrys(pChgTrack,nStartAction,nEndAction);
                                    break;
                default:
                {
                    // added to avoid warnings
                }
            }
        }
        delete *iter;
    }

    aMsgQueue.clear();

    return 0;
}
IMPL_LINK_NOARG(ScAcceptChgDlg, ReOpenTimerHdl)
{
    ScSimpleRefDlgWrapper::SetAutoReOpen(true);
    m_pAcceptChgCtr->ShowFilterPage();
    RefHandle(NULL);

    return 0;
}

IMPL_LINK_NOARG(ScAcceptChgDlg, UpdateSelectionHdl)
{
    ScTabView* pTabView = pViewData->GetView();

    bool bAcceptFlag = true;
    bool bRejectFlag = true;
    bool bContMark = false;

    pTabView->DoneBlockMode();  // clears old marking
    SvTreeListEntry* pEntry = pTheView->FirstSelected();
    while( pEntry )
    {
        ScRedlinData* pEntryData = (ScRedlinData*) pEntry->GetUserData();
        if( pEntryData )
        {
            bRejectFlag &= (bool) pEntryData->bIsRejectable;
            bAcceptFlag &= (bool) pEntryData->bIsAcceptable;

            const ScChangeAction* pScChangeAction = (ScChangeAction*) pEntryData->pData;
            if( pScChangeAction && (pScChangeAction->GetType() != SC_CAT_DELETE_TABS) &&
                    (!pEntryData->bDisabled || pScChangeAction->IsVisible()) )
            {
                const ScBigRange& rBigRange = pScChangeAction->GetBigRange();
                if( rBigRange.IsValid( pDoc ) && IsActive() )
                {
                    bool bSetCursor = !pTheView->NextSelected( pEntry );
                    pTabView->MarkRange( rBigRange.MakeRange(), bSetCursor, bContMark );
                    bContMark = true;
                }
            }
        }
        else
        {
            bAcceptFlag = false;
            bRejectFlag = false;
        }
        bAcceptEnableFlag = bAcceptFlag;
        bRejectEnableFlag = bRejectFlag;

        pEntry = pTheView->NextSelected( pEntry );
    }

    ScChangeTrack* pChanges = pDoc->GetChangeTrack();
    bool bEnable = pDoc->IsDocEditable() && pChanges && !pChanges->IsProtected();
    pTPView->EnableAccept( bAcceptFlag && bEnable );
    pTPView->EnableReject( bRejectFlag && bEnable );

    return 0;
}

IMPL_LINK_NOARG(ScAcceptChgDlg, CommandHdl)
{

    const CommandEvent aCEvt(pTheView->GetCommandEvent());

    if(aCEvt.GetCommand()==COMMAND_CONTEXTMENU)
    {
        ScPopupMenu aPopup(ScResId(RID_POPUP_CHANGES));

        aPopup.SetMenuFlags(MENU_FLAG_HIDEDISABLEDENTRIES);

        SvTreeListEntry* pEntry=pTheView->GetCurEntry();
        if(pEntry!=NULL)
        {
            pTheView->Select(pEntry);
        }
        else
        {
            aPopup.Deactivate();
        }

        sal_uInt16 nSortedCol= pTheView->GetSortedCol();

        if(nSortedCol!=0xFFFF)
        {
            sal_uInt16 nItemId=nSortedCol+SC_SUB_SORT+1;

            aPopup.CheckItem(nItemId);

            PopupMenu *pSubMenu = aPopup.GetPopupMenu(SC_SUB_SORT);

            if (pSubMenu)
                pSubMenu->CheckItem(nItemId);
        }

        aPopup.EnableItem(SC_CHANGES_COMMENT,false);

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

        sal_uInt16 nCommand=aPopup.Execute( this, GetPointerPosPixel() );


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

                        pViewData->GetDocShell()->ExecuteChangeCommentDialog( pScChangeAction, this,false);
                    }
                }
            }
            else
            {
                bool bSortDir = pTheView->GetSortDirection();
                sal_uInt16 nDialogCol=nCommand-SC_SUB_SORT-1;
                if(nSortedCol==nDialogCol) bSortDir=!bSortDir;
                pTheView->SortByCol(nDialogCol,bSortDir);
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
    return 0;
}

void ScAcceptChgDlg::Initialize(SfxChildWinInfo *pInfo)
{
    String aStr;
    if(pInfo!=NULL)
    {
        if ( pInfo->aExtraString.Len() )
        {
            xub_StrLen nPos = pInfo->aExtraString.Search(
                OUString("AcceptChgDat:"));

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
        sal_uInt16 nCount=(sal_uInt16)aStr.ToInt32();

        for(sal_uInt16 i=0;i<nCount;i++)
        {
            xub_StrLen n1 = aStr.Search(';');
            aStr.Erase(0, n1+1);
            pTheView->SetTab(i,(sal_uInt16)aStr.ToInt32(),MAP_PIXEL);
        }
    }
}

//-------------------------------------------------------------------------

void ScAcceptChgDlg::FillInfo(SfxChildWinInfo& rInfo) const
{
    SfxModelessDialog::FillInfo(rInfo);
    rInfo.aExtraString.AppendAscii(RTL_CONSTASCII_STRINGPARAM( "AcceptChgDat:(" ));

    sal_uInt16  nCount=pTheView->TabCount();

    rInfo.aExtraString += OUString::number(nCount);
    rInfo.aExtraString += ';';
    for(sal_uInt16 i=0;i<nCount;i++)
    {
        rInfo.aExtraString += OUString::number(pTheView->GetTab(i));
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
                utl::SearchParam::SRCH_REGEXP,false,false,false );

        pTheView->SetCommentParams(&aSearchParam);

        pTheView->UpdateFilterTest();
    }
}

IMPL_LINK_NOARG(ScAcceptChgDlg, FilterModified)
{
    return 0;
}

#define CALC_DATE       3
#define CALC_POS        1

IMPL_LINK( ScAcceptChgDlg, ColCompareHdl, SvSortData*, pSortData )
{
    StringCompare eCompare=COMPARE_EQUAL;
    SCCOL nSortCol= static_cast<SCCOL>(pTheView->GetSortedCol());

    if(pSortData)
    {
        SvTreeListEntry* pLeft = (SvTreeListEntry*)(pSortData->pLeft );
        SvTreeListEntry* pRight = (SvTreeListEntry*)(pSortData->pRight );

        if(CALC_DATE==nSortCol)
        {
            RedlinData *pLeftData=(RedlinData *)(pLeft->GetUserData());
            RedlinData *pRightData=(RedlinData *)(pRight->GetUserData());

            if(pLeftData!=NULL && pRightData!=NULL)
            {
                if(pLeftData->aDateTime < pRightData->aDateTime)
                    eCompare=COMPARE_LESS;
                else if(pLeftData->aDateTime > pRightData->aDateTime)
                    eCompare=COMPARE_GREATER;

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
                    eCompare=COMPARE_LESS;
                else if(pLeftData->nTable == pRightData->nTable)
                {
                    if(pLeftData->nRow < pRightData->nRow)
                        eCompare=COMPARE_LESS;
                    else if(pLeftData->nRow == pRightData->nRow)
                    {
                        if(pLeftData->nCol < pRightData->nCol)
                            eCompare=COMPARE_LESS;
                        else if(pLeftData->nCol == pRightData->nCol)
                            eCompare=COMPARE_EQUAL;
                    }
                }

                return eCompare;
            }
        }

        SvLBoxItem* pLeftItem = pTheView->GetEntryAtPos( pLeft, static_cast<sal_uInt16>(nSortCol));
        SvLBoxItem* pRightItem = pTheView->GetEntryAtPos( pRight, static_cast<sal_uInt16>(nSortCol));

        if(pLeftItem != NULL && pRightItem != NULL)
        {
            sal_uInt16 nLeftKind = pLeftItem->GetType();
            sal_uInt16 nRightKind = pRightItem->GetType();

            if(nRightKind == SV_ITEM_ID_LBOXSTRING &&
                nLeftKind == SV_ITEM_ID_LBOXSTRING )
            {
                eCompare= (StringCompare) ScGlobal::GetCaseCollator()->compareString(
                                        ((SvLBoxString*)pLeftItem)->GetText(),
                                        ((SvLBoxString*)pRightItem)->GetText());

                if(eCompare==COMPARE_EQUAL) eCompare=COMPARE_LESS;
            }
        }


    }
    return eCompare;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
