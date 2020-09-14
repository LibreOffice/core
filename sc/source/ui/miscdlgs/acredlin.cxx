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

#include <svl/undo.hxx>
#include <unotools/textsearch.hxx>
#include <unotools/localedatawrapper.hxx>
#include <unotools/collatorwrapper.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/basedlgs.hxx>
#include <vcl/commandevent.hxx>

#include <acredlin.hxx>
#include <global.hxx>
#include <reffact.hxx>
#include <document.hxx>
#include <docsh.hxx>
#include <scresid.hxx>
#include <strings.hrc>
#include <scmod.hxx>
#include <tabvwsh.hxx>

// defines -------------------------------------------------------------------

#define RD_SPECIAL_NONE         0
#define RD_SPECIAL_CONTENT      1
#define RD_SPECIAL_VISCONTENT   2


ScRedlinData::ScRedlinData()
    :RedlinData()
{
    nInfo=RD_SPECIAL_NONE;
    nActionNo=0;
    pData=nullptr;
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
    pData=nullptr;
    bDisabled=false;
    bIsRejectable=false;
    bIsAcceptable=false;
}


ScAcceptChgDlg::ScAcceptChgDlg(SfxBindings* pB, SfxChildWindow* pCW, weld::Window* pParent,
    ScViewData* ptrViewData)
    : SfxModelessDialogController(pB, pCW, pParent,
        "svx/ui/acceptrejectchangesdialog.ui", "AcceptRejectChangesDialog")
    , aSelectionIdle("ScAcceptChgDlg SelectionIdle")
    , aReOpenIdle("ScAcceptChgDlg ReOpenIdle")
    , pViewData( ptrViewData )
    , pDoc( &ptrViewData->GetDocument() )
    , aStrInsertCols(ScResId(STR_CHG_INSERT_COLS))
    , aStrInsertRows(ScResId(STR_CHG_INSERT_ROWS))
    , aStrInsertTabs(ScResId(STR_CHG_INSERT_TABS))
    , aStrDeleteCols(ScResId(STR_CHG_DELETE_COLS))
    , aStrDeleteRows(ScResId(STR_CHG_DELETE_ROWS))
    , aStrDeleteTabs(ScResId(STR_CHG_DELETE_TABS))
    , aStrMove(ScResId(STR_CHG_MOVE))
    , aStrContent(ScResId(STR_CHG_CONTENT))
    , aStrReject(ScResId(STR_CHG_REJECT))
    , aStrAllAccepted(ScResId(STR_CHG_ACCEPTED))
    , aStrAllRejected(ScResId(STR_CHG_REJECTED))
    , aStrNoEntry(ScResId(STR_CHG_NO_ENTRY))
    , aStrContentWithChild(ScResId(STR_CHG_CONTENT_WITH_CHILD))
    , aStrChildContent(ScResId(STR_CHG_CHILD_CONTENT))
    , aStrChildOrgContent(ScResId(STR_CHG_CHILD_ORGCONTENT))
    , aStrEmpty(ScResId(STR_CHG_EMPTY))
    , aUnknown("Unknown")
    , bIgnoreMsg(false)
    , bNoSelection(false)
    , bHasFilterEntry(false)
    , bUseColor(false)
    , m_xContentArea(m_xDialog->weld_content_area())
    , m_xPopup(m_xBuilder->weld_menu("calcmenu"))
    , m_xSortMenu(m_xBuilder->weld_menu("calcsortmenu"))
{
    m_xAcceptChgCtr.reset(new SvxAcceptChgCtr(m_xContentArea.get(), m_xDialog.get(), m_xBuilder.get()));
    nAcceptCount=0;
    nRejectCount=0;
    aReOpenIdle.SetInvokeHandler(LINK( this, ScAcceptChgDlg, ReOpenTimerHdl ));

    pTPFilter = m_xAcceptChgCtr->GetFilterPage();
    pTPView = m_xAcceptChgCtr->GetViewPage();

    // tdf#136062 Don't use "Reject/Clear formatting" instead of "Reject" buttons in Calc
    pTPView->EnableClearFormat(false);
    pTPView->EnableClearFormatAll(false);

    pTheView = pTPView->GetTableControl();
    pTheView->SetCalcView();
    aSelectionIdle.SetInvokeHandler(LINK( this, ScAcceptChgDlg, UpdateSelectionHdl ));
    aSelectionIdle.SetDebugName( "ScAcceptChgDlg  aSelectionIdle" );

    pTPFilter->SetReadyHdl(LINK( this, ScAcceptChgDlg, FilterHandle ));
    pTPFilter->SetRefHdl(LINK( this, ScAcceptChgDlg, RefHandle ));
    pTPFilter->HideRange(false);
    pTPView->SetRejectClickHdl( LINK( this, ScAcceptChgDlg,RejectHandle));
    pTPView->SetAcceptClickHdl( LINK(this, ScAcceptChgDlg, AcceptHandle));
    pTPView->SetRejectAllClickHdl( LINK( this, ScAcceptChgDlg,RejectAllHandle));
    pTPView->SetAcceptAllClickHdl( LINK(this, ScAcceptChgDlg, AcceptAllHandle));

    weld::TreeView& rTreeView = pTheView->GetWidget();
    rTreeView.connect_expanding(LINK(this, ScAcceptChgDlg, ExpandingHandle));
    rTreeView.connect_changed(LINK(this, ScAcceptChgDlg, SelectHandle));
    rTreeView.connect_popup_menu(LINK(this, ScAcceptChgDlg, CommandHdl));
    rTreeView.set_sort_func([this](const weld::TreeIter& rLeft, const weld::TreeIter& rRight){
        return ColCompareHdl(rLeft, rRight);
    });
    rTreeView.set_selection_mode(SelectionMode::Multiple);

    Init();

    UpdateView();

    std::unique_ptr<weld::TreeIter> xEntry(rTreeView.make_iterator());
    if (rTreeView.get_iter_first(*xEntry))
        rTreeView.select(*xEntry);
}

ScAcceptChgDlg::~ScAcceptChgDlg()
{
    ClearView();
    ScChangeTrack* pChanges=pDoc->GetChangeTrack();

    if (pChanges)
    {
        Link<ScChangeTrack&,void> aLink;
        pChanges->SetModifiedLink(aLink);
    }
}

void ScAcceptChgDlg::ReInit(ScViewData* ptrViewData)
{
    pViewData=ptrViewData;
    if (pViewData)
        pDoc = &ptrViewData->GetDocument();
    else
        pDoc = nullptr;

    bNoSelection=false;
    bIgnoreMsg=false;
    nAcceptCount=0;
    nRejectCount=0;

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
    OSL_ENSURE( pViewData && pDoc, "ViewData or Document not found!" );

    ScChangeTrack* pChanges=pDoc->GetChangeTrack();

    if(pChanges!=nullptr)
    {
        pChanges->SetModifiedLink( LINK( this, ScAcceptChgDlg,ChgTrackModHdl));
        aChangeViewSet.SetTheAuthorToShow(pChanges->GetUser());
        pTPFilter->ClearAuthors();
        const std::set<OUString>& rUserColl = pChanges->GetUserCollection();
        for (const auto& rItem : rUserColl)
            pTPFilter->InsertAuthor(rItem);
    }

    ScChangeViewSettings* pViewSettings=pDoc->GetChangeViewSettings();
    if ( pViewSettings!=nullptr )
        aChangeViewSet = *pViewSettings;
    // adjust TimeField for filter tabpage
    aChangeViewSet.AdjustDateMode( *pDoc );

    pTPFilter->CheckDate(aChangeViewSet.HasDate());

    DateTime aEmpty(DateTime::EMPTY);

    DateTime aDateTime(aChangeViewSet.GetTheFirstDateTime());
    if (aDateTime != aEmpty)
    {
        pTPFilter->SetFirstDate(aDateTime);
        pTPFilter->SetFirstTime(aDateTime);
    }
    aDateTime = aChangeViewSet.GetTheLastDateTime();
    if (aDateTime != aEmpty)
    {
        pTPFilter->SetLastDate(aDateTime);
        pTPFilter->SetLastTime(aDateTime);
    }

    pTPFilter->SetDateMode(static_cast<sal_uInt16>(aChangeViewSet.GetTheDateMode()));
    pTPFilter->CheckComment(aChangeViewSet.HasComment());
    pTPFilter->SetComment(aChangeViewSet.GetTheComment());

    pTPFilter->CheckAuthor(aChangeViewSet.HasAuthor());
    OUString aString=aChangeViewSet.GetTheAuthorToShow();
    if(!aString.isEmpty())
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
        const ScRange & rRangeEntry = aChangeViewSet.GetTheRangeList().front();
        OUString aRefStr(rRangeEntry.Format(*pDoc, ScRefFlags::RANGE_ABS_3D));
        pTPFilter->SetRange(aRefStr);
    }

    // init filter
    if(!(pTPFilter->IsDate()||pTPFilter->IsRange()||
        pTPFilter->IsAuthor()||pTPFilter->IsComment()))
        return;

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
            utl::SearchParam::SearchType::Regexp,false );

    pTheView->SetCommentParams(&aSearchParam);

    pTheView->UpdateFilterTest();
}

void ScAcceptChgDlg::ClearView()
{
    nAcceptCount=0;
    nRejectCount=0;
    weld::TreeView& rTreeView = pTheView->GetWidget();
    rTreeView.freeze();
    rTreeView.clear();
    rTreeView.thaw();
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
    if(pScChangeAction==nullptr) return false;

    bool bFlag = false;

    ScRange aRef=pScChangeAction->GetBigRange().MakeRange();
    OUString aUser=pScChangeAction->GetUser();
    DateTime aDateTime=pScChangeAction->GetDateTime();

    ScChangeActionType eType=pScChangeAction->GetType();
    OUString aDesc;

    OUString aComment = pScChangeAction->GetComment().replaceAll("\n", "");

    if(eType==SC_CAT_CONTENT)
    {
        if(!pScChangeAction->IsDialogParent())
            pScChangeAction->GetDescription(aDesc, pDoc, true);
    }
    else
        pScChangeAction->GetDescription(aDesc, pDoc, !pScChangeAction->IsMasterDelete());

    if (!aDesc.isEmpty())
    {
        aComment += " (" + aDesc + ")";
    }

    if (pTheView->IsValidEntry(aUser, aDateTime, aComment))
    {
        if(pTPFilter->IsRange())
        {
            for ( size_t i = 0, nRanges = aRangeList.size(); i < nRanges; ++i )
            {
                ScRange const & rRangeEntry = aRangeList[ i ];
                if (rRangeEntry.Intersects(aRef)) {
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

std::unique_ptr<weld::TreeIter> ScAcceptChgDlg::AppendChangeAction(
    const ScChangeAction* pScChangeAction, bool bCreateOnDemand,
    const weld::TreeIter* pParent, bool bDelMaster, bool bDisabled)
{
    ScChangeTrack* pChanges=pDoc->GetChangeTrack();

    if(pScChangeAction==nullptr || pChanges==nullptr) return nullptr;

    bool bFlag = false;

    ScRange aRef=pScChangeAction->GetBigRange().MakeRange();
    OUString aUser=pScChangeAction->GetUser();
    DateTime aDateTime=pScChangeAction->GetDateTime();

    OUString aRefStr;
    ScChangeActionType eType=pScChangeAction->GetType();
    OUStringBuffer aBuf;
    OUString aDesc;

    std::unique_ptr<ScRedlinData> pNewData(new ScRedlinData);
    pNewData->pData=const_cast<ScChangeAction *>(pScChangeAction);
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

    aBuf.append('\t');
    aBuf.append(aRefStr);
    aBuf.append('\t');

    bool bIsGenerated = false;

    if(!pChanges->IsGenerated(pScChangeAction->GetActionNumber()))
    {
        aBuf.append(aUser);
        aBuf.append('\t');
        aBuf.append(ScGlobal::getLocaleDataPtr()->getDate(aDateTime));
        aBuf.append(' ');
        aBuf.append(ScGlobal::getLocaleDataPtr()->getTime(aDateTime));
        aBuf.append('\t');

        bIsGenerated = false;
    }
    else
    {
        aBuf.append('\t');
        aBuf.append('\t');
        bIsGenerated = true;
    }

    OUString aComment = pScChangeAction->GetComment().replaceAll("\n", "");

    if (!aDesc.isEmpty())
    {
        aComment +=  " (" + aDesc + ")";
    }

    aBuf.append(aComment);

    if (pTheView->IsValidEntry(aUser, aDateTime) || bIsGenerated)
    {
        if (pTheView->IsValidComment(aComment))
        {
            if(pTPFilter->IsRange())
            {
                for ( size_t i = 0, nRanges = aRangeList.size(); i < nRanges; ++i )
                {
                    ScRange const & rRangeEntry = aRangeList[ i ];
                    if( rRangeEntry.Intersects(aRef) )
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

    weld::TreeView& rTreeView = pTheView->GetWidget();
    std::unique_ptr<weld::TreeIter> xEntry(rTreeView.make_iterator());
    OUString sString(aBuf.makeStringAndClear());
    OUString sId(OUString::number(reinterpret_cast<sal_Int64>(pNewData.release())));
    rTreeView.insert(pParent, -1, &sString, &sId, nullptr, nullptr, bCreateOnDemand, xEntry.get());
    if (!bFlag && bUseColor && !pParent)
    {
        rTreeView.set_font_color(*xEntry, COL_LIGHTBLUE);
    }
    else if (bFlag && bUseColor && pParent)
    {
        rTreeView.set_font_color(*xEntry, COL_GREEN);

        std::unique_ptr<weld::TreeIter> xExpEntry(rTreeView.make_iterator(pParent));

        while (!rTreeView.get_row_expanded(*xExpEntry))
        {
            if (rTreeView.get_iter_depth(*xExpEntry))
                rTreeView.expand_row(*xExpEntry);

            if (!rTreeView.iter_parent(*xExpEntry))
                break;
        }
    }
    return xEntry;
}

std::unique_ptr<weld::TreeIter> ScAcceptChgDlg::AppendFilteredAction(
    const ScChangeAction* pScChangeAction, ScChangeActionState eState,
    bool bCreateOnDemand,
    const weld::TreeIter* pParent, bool bDelMaster, bool bDisabled)
{
    ScChangeTrack* pChanges=pDoc->GetChangeTrack();

    if(pScChangeAction==nullptr || pChanges==nullptr) return nullptr;

    bool bIsGenerated = pChanges->IsGenerated(pScChangeAction->GetActionNumber());

    bool bFlag = false;

    ScRange aRef=pScChangeAction->GetBigRange().MakeRange();
    OUString aUser=pScChangeAction->GetUser();
    DateTime aDateTime=pScChangeAction->GetDateTime();

    if (pTheView->IsValidEntry(aUser, aDateTime) || bIsGenerated)
    {
        if(pTPFilter->IsRange())
        {
            for ( size_t i = 0, nRanges = aRangeList.size(); i < nRanges; ++i )
            {
                ScRange const & rRangeEntry=aRangeList[ i ];
                if( rRangeEntry.Intersects(aRef) )
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

    std::unique_ptr<weld::TreeIter> xEntry;
    if(bFlag)
    {
        ScChangeActionType eType=pScChangeAction->GetType();
        OUString aActionString;
        OUString aDesc;

        std::unique_ptr<ScRedlinData> pNewData(new ScRedlinData);
        pNewData->pData=const_cast<ScChangeAction *>(pScChangeAction);
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
                aActionString=aStrContentWithChild;
                pNewData->nInfo=RD_SPECIAL_VISCONTENT;
                pNewData->bIsRejectable=false;
                pNewData->bIsAcceptable=false;
            }
            else
            {
                aActionString=*MakeTypeString(eType);
                pScChangeAction->GetDescription( aDesc, pDoc, true);
            }
        }
        else
        {
            aActionString=*MakeTypeString(eType);

            if(bDelMaster)
            {
                pScChangeAction->GetDescription( aDesc, pDoc,true);
                pNewData->bDisabled=true;
                pNewData->bIsRejectable=false;
            }
            else
                pScChangeAction->GetDescription( aDesc, pDoc,!pScChangeAction->IsMasterDelete());

        }


        OUString aComment = pScChangeAction->GetComment().replaceAll("\n", "");
        if (!aDesc.isEmpty())
        {
            aComment += " (" + aDesc + ")";
        }
        if (pTheView->IsValidComment(aComment))
        {
            weld::TreeView& rTreeView = pTheView->GetWidget();
            xEntry = rTreeView.make_iterator();
            OUString sId(OUString::number(reinterpret_cast<sal_Int64>(pNewData.release())));
            rTreeView.insert(pParent, -1, &aActionString, &sId, nullptr, nullptr, bCreateOnDemand, xEntry.get());

            OUString aRefStr;
            pScChangeAction->GetRefString(aRefStr, pDoc, true);
            rTreeView.set_text(*xEntry, aRefStr, 1);

            if (!bIsGenerated)
            {
                rTreeView.set_text(*xEntry, aUser, 2);
                OUString sDate = ScGlobal::getLocaleDataPtr()->getDate(aDateTime) + " " + ScGlobal::getLocaleDataPtr()->getTime(aDateTime);
                rTreeView.set_text(*xEntry, sDate, 3);
            }

            rTreeView.set_text(*xEntry, aComment, 4);
        }
    }
    return xEntry;
}

std::unique_ptr<weld::TreeIter> ScAcceptChgDlg::InsertChangeActionContent(const ScChangeActionContent* pScChangeAction,
                                                                          const weld::TreeIter& rParent, sal_uLong nSpecial)
{
    ScChangeTrack* pChanges=pDoc->GetChangeTrack();

    if(pScChangeAction==nullptr || pChanges==nullptr) return nullptr;

    bool bIsGenerated = pChanges->IsGenerated(pScChangeAction->GetActionNumber());

    bool bFlag = false;

    ScRange aRef=pScChangeAction->GetBigRange().MakeRange();
    OUString aUser=pScChangeAction->GetUser();
    DateTime aDateTime=pScChangeAction->GetDateTime();

    if (pTheView->IsValidEntry(aUser, aDateTime) || bIsGenerated)
    {
        if(pTPFilter->IsRange())
        {
            for ( size_t i = 0, nRanges = aRangeList.size(); i < nRanges; ++i )
            {
                ScRange const & rRangeEntry = aRangeList[ i ];
                if( rRangeEntry.Intersects(aRef) )
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
    OUString aString;
    OUString a2String;
    OUString aDesc;

    if(nSpecial==RD_SPECIAL_CONTENT)
    {
        OUString aTmp;
        pScChangeAction->GetOldString(aTmp, pDoc);
        a2String = aTmp;
        if(a2String.isEmpty()) a2String=aStrEmpty;

        //aString+="\'";
        aString+=a2String;
        //aString+="\'";

        aDesc = aStrChildOrgContent + ": ";
    }
    else
    {
        OUString aTmp;
        pScChangeAction->GetNewString(aTmp, pDoc);
        a2String = aTmp;
        if(a2String.isEmpty())
        {
            a2String = aStrEmpty;
            aString += a2String;
        }
        else
        {
            aString += "\'" + a2String + "\'";
            a2String = aString;
        }
        aDesc = aStrChildContent;

    }

    aDesc += a2String;
    aString += "\t";
    pScChangeAction->GetRefString(aRefStr, pDoc, true);
    aString += aRefStr + "\t";

    if(!bIsGenerated)
    {
        aString += aUser + "\t"
                +  ScGlobal::getLocaleDataPtr()->getDate(aDateTime) + " "
                +  ScGlobal::getLocaleDataPtr()->getTime(aDateTime) + "\t";
    }
    else
    {
        aString += "\t\t";
    }

    OUString aComment = pScChangeAction->GetComment().replaceAll("\n", "");

    if(!aDesc.isEmpty())
    {
        aComment += " (" + aDesc + ")";
    }

    aString += aComment;

    std::unique_ptr<ScRedlinData> pNewData(new ScRedlinData);
    pNewData->nInfo=nSpecial;
    pNewData->pData=const_cast<ScChangeActionContent *>(pScChangeAction);
    pNewData->nActionNo=pScChangeAction->GetActionNumber();
    pNewData->bIsAcceptable=pScChangeAction->IsClickable();
    pNewData->bIsRejectable=false;
    pNewData->bDisabled=!pNewData->bIsAcceptable;
    pNewData->aDateTime=aDateTime;
    pNewData->nRow  = aRef.aStart.Row();
    pNewData->nCol  = aRef.aStart.Col();
    pNewData->nTable= aRef.aStart.Tab();

    weld::TreeView& rTreeView = pTheView->GetWidget();
    std::unique_ptr<weld::TreeIter> xEntry(rTreeView.make_iterator());
    OUString sId(OUString::number(reinterpret_cast<sal_Int64>(pNewData.release())));
    rTreeView.insert(&rParent, -1, &aString, &sId, nullptr, nullptr, false, xEntry.get());
    if (pTheView->IsValidComment(aComment) && bFlag)
        bHasFilterEntry=true;
    else
    {
        rTreeView.set_font_color(*xEntry, COL_LIGHTBLUE);
    }
    return xEntry;
}

void ScAcceptChgDlg::UpdateView()
{
    std::unique_ptr<weld::TreeIter> xParent;
    ScChangeTrack* pChanges=nullptr;
    const ScChangeAction* pScChangeAction=nullptr;
    m_xDialog->set_busy_cursor(true);
    weld::TreeView& rTreeView = pTheView->GetWidget();
    rTreeView.freeze();
    bool bFilterFlag = pTPFilter->IsDate() || pTPFilter->IsRange() ||
        pTPFilter->IsAuthor() || pTPFilter->IsComment();

    bUseColor = bFilterFlag;

    if(pDoc!=nullptr)
    {
        pChanges=pDoc->GetChangeTrack();
        if(pChanges!=nullptr)
            pScChangeAction=pChanges->GetFirst();
    }
    bool bTheFlag = false;

    while(pScChangeAction!=nullptr)
    {
        bHasFilterEntry=false;
        switch (pScChangeAction->GetState())
        {
            case SC_CAS_VIRGIN:

                if (pScChangeAction->IsDialogRoot())
                {
                    bool bOnDemandChildren = !bFilterFlag && pScChangeAction->IsDialogParent();
                    if (pScChangeAction->IsDialogParent())
                        xParent = AppendChangeAction(pScChangeAction, bOnDemandChildren);
                    else
                        xParent = AppendFilteredAction(pScChangeAction, SC_CAS_VIRGIN, bOnDemandChildren);
                }
                else
                    xParent.reset();

                bTheFlag=true;
                break;

            case SC_CAS_ACCEPTED:
                xParent.reset();
                nAcceptCount++;
                break;

            case SC_CAS_REJECTED:
                xParent.reset();
                nRejectCount++;
                break;
        }

        if (xParent && pScChangeAction->IsDialogParent() && bFilterFlag)
        {
            bool bTestFlag = bHasFilterEntry;
            bHasFilterEntry=false;
            if (Expand(pChanges, pScChangeAction, *xParent, !bTestFlag) && !bTestFlag)
                rTreeView.remove(*xParent);
        }

        pScChangeAction=pScChangeAction->GetNext();
    }

    if( bTheFlag && (!pDoc->IsDocEditable() || pChanges->IsProtected()) )
        bTheFlag=false;

    pTPView->EnableAccept(bTheFlag);
    pTPView->EnableAcceptAll(bTheFlag);
    pTPView->EnableReject(bTheFlag);
    pTPView->EnableRejectAll(bTheFlag);

    if (nAcceptCount>0)
        rTreeView.insert(nullptr, -1, &aStrAllAccepted, nullptr, nullptr, nullptr, true, nullptr);
    if (nRejectCount>0)
        rTreeView.insert(nullptr, -1, &aStrAllRejected, nullptr, nullptr, nullptr, true, nullptr);
    rTreeView.thaw();
    m_xDialog->set_busy_cursor(false);
    std::unique_ptr<weld::TreeIter> xEntry(rTreeView.make_iterator());
    if (rTreeView.get_iter_first(*xEntry))
        rTreeView.select(*xEntry);
}

IMPL_LINK_NOARG(ScAcceptChgDlg, RefHandle, SvxTPFilter*, void)
{
    sal_uInt16 nId  =ScSimpleRefDlgWrapper::GetChildWindowId();

    SC_MOD()->SetRefDialog( nId, true );

    SfxViewFrame* pViewFrm = pViewData->GetViewShell()->GetViewFrame();
    ScSimpleRefDlgWrapper* pWnd = static_cast<ScSimpleRefDlgWrapper*>(pViewFrm->GetChildWindow( nId ));

    if(pWnd!=nullptr)
    {
        sal_uInt16 nAcceptId=ScAcceptChgDlgWrapper::GetChildWindowId();
        pViewFrm->ShowChildWindow(nAcceptId,false);
        pWnd->SetCloseHdl(LINK( this, ScAcceptChgDlg,RefInfoHandle));
        pWnd->SetRefString(pTPFilter->GetRange());
        ScSimpleRefDlgWrapper::SetAutoReOpen(false);
        auto xWin = pWnd->GetController();
        m_xDialog->hide();
        xWin->set_title(m_xDialog->get_title());
        pWnd->StartRefInput();
    }
}

IMPL_LINK( ScAcceptChgDlg, RefInfoHandle, const OUString*, pResult, void)
{
    sal_uInt16 nId = ScAcceptChgDlgWrapper::GetChildWindowId();

    ScSimpleRefDlgWrapper::SetAutoReOpen(true);

    SfxViewFrame* pViewFrm = pViewData->GetViewShell()->GetViewFrame();
    if (pResult)
    {
        pTPFilter->SetRange(*pResult);
        FilterHandle(pTPFilter);

        pViewFrm->ShowChildWindow(nId);
    }
    else
    {
        pViewFrm->SetChildWindow(nId, false);
    }
}

IMPL_LINK( ScAcceptChgDlg, FilterHandle, SvxTPFilter*, pRef, void )
{
    if(pRef!=nullptr)
    {
        ClearView();
        aRangeList.RemoveAll();
        aRangeList.Parse(pTPFilter->GetRange(), *pDoc);
        UpdateView();
    }
}

IMPL_LINK( ScAcceptChgDlg, RejectHandle, SvxTPView*, pRef, void )
{
    m_xDialog->set_busy_cursor(true);

    bIgnoreMsg=true;
    ScChangeTrack* pChanges=pDoc->GetChangeTrack();

    if(pRef!=nullptr)
    {
        weld::TreeView& rTreeView = pTheView->GetWidget();
        rTreeView.selected_foreach([this, pChanges, &rTreeView](weld::TreeIter& rEntry){
            ScRedlinData *pEntryData = reinterpret_cast<ScRedlinData*>(rTreeView.get_id(rEntry).toInt64());
            if (pEntryData)
            {
                ScChangeAction* pScChangeAction= static_cast<ScChangeAction*>(pEntryData->pData);
                if (pScChangeAction->GetType()==SC_CAT_INSERT_TABS)
                    pViewData->SetTabNo(0);
                pChanges->Reject(pScChangeAction);
            }
            return false;
        });
        ScDocShell* pDocSh=pViewData->GetDocShell();
        pDocSh->PostPaintExtras();
        pDocSh->PostPaintGridAll();
        pDocSh->GetUndoManager()->Clear();
        pDocSh->SetDocumentModified();
        ClearView();
        UpdateView();
    }

    m_xDialog->set_busy_cursor(false);

    bIgnoreMsg=false;
}
IMPL_LINK( ScAcceptChgDlg, AcceptHandle, SvxTPView*, pRef, void )
{
    m_xDialog->set_busy_cursor(true);

    ScChangeTrack* pChanges=pDoc->GetChangeTrack();
    bIgnoreMsg=true;
    if(pRef!=nullptr)
    {
        weld::TreeView& rTreeView = pTheView->GetWidget();
        rTreeView.selected_foreach([pChanges, &rTreeView](weld::TreeIter& rEntry) {
            ScRedlinData *pEntryData = reinterpret_cast<ScRedlinData*>(rTreeView.get_id(rEntry).toInt64());
            if (pEntryData)
            {
                ScChangeAction* pScChangeAction=
                        static_cast<ScChangeAction*>(pEntryData->pData);
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
            return false;
        });
        ScDocShell* pDocSh=pViewData->GetDocShell();
        pDocSh->PostPaintExtras();
        pDocSh->PostPaintGridAll();
        pDocSh->SetDocumentModified();
        ClearView();
        UpdateView();
    }
    m_xDialog->set_busy_cursor(false);
    bIgnoreMsg=false;
}

void ScAcceptChgDlg::RejectFiltered()
{
    if(pDoc==nullptr) return;
    ScChangeTrack* pChanges=pDoc->GetChangeTrack();
    const ScChangeAction* pScChangeAction=nullptr;

    if(pChanges!=nullptr)
    {
        pScChangeAction=pChanges->GetLast();
    }

    while(pScChangeAction!=nullptr)
    {
        if(pScChangeAction->IsDialogRoot())
            if(IsValidAction(pScChangeAction))
                pChanges->Reject(const_cast<ScChangeAction*>(pScChangeAction));

        pScChangeAction=pScChangeAction->GetPrev();
    }
}
void ScAcceptChgDlg::AcceptFiltered()
{
    if(pDoc==nullptr) return;
    ScChangeTrack* pChanges=pDoc->GetChangeTrack();
    const ScChangeAction* pScChangeAction=nullptr;

    if(pChanges!=nullptr)
        pScChangeAction=pChanges->GetLast();

    while(pScChangeAction!=nullptr)
    {
        if(pScChangeAction->IsDialogRoot())
            if(IsValidAction(pScChangeAction))
                pChanges->Accept(const_cast<ScChangeAction*>(pScChangeAction));

        pScChangeAction=pScChangeAction->GetPrev();
    }
}

IMPL_LINK_NOARG(ScAcceptChgDlg, RejectAllHandle, SvxTPView*, void)
{
    m_xDialog->set_busy_cursor(true);
    bIgnoreMsg=true;
    ScChangeTrack* pChanges=pDoc->GetChangeTrack();
    if(pChanges!=nullptr)
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
    m_xDialog->set_busy_cursor(false);

    bIgnoreMsg=false;
}

IMPL_LINK_NOARG(ScAcceptChgDlg, AcceptAllHandle, SvxTPView*, void)
{
    m_xDialog->set_busy_cursor(true);

    bIgnoreMsg=true;
    ScChangeTrack* pChanges=pDoc->GetChangeTrack();
    if(pChanges!=nullptr)
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

    m_xDialog->set_busy_cursor(false);
}

IMPL_LINK_NOARG(ScAcceptChgDlg, SelectHandle, weld::TreeView&, void)
{
    if (!bNoSelection)
        aSelectionIdle.Start();

    bNoSelection=false;
}

void ScAcceptChgDlg::GetDependents(const ScChangeAction* pScChangeAction,
                                   ScChangeActionMap& aActionMap,
                                   const weld::TreeIter& rEntry)
{
    ScChangeTrack* pChanges=pDoc->GetChangeTrack();

    weld::TreeView& rTreeView = pTheView->GetWidget();
    std::unique_ptr<weld::TreeIter> xParent(rTreeView.make_iterator(&rEntry));
    if (rTreeView.iter_parent(*xParent))
    {
        ScRedlinData *pParentData = reinterpret_cast<ScRedlinData*>(rTreeView.get_id(*xParent).toInt64());
        ScChangeAction* pParentAction=static_cast<ScChangeAction*>(pParentData->pData);

        if(pParentAction!=pScChangeAction)
            pChanges->GetDependents(const_cast<ScChangeAction*>(pScChangeAction),
                        aActionMap,pScChangeAction->IsMasterDelete());
        else
            pChanges->GetDependents( const_cast<ScChangeAction*>(pScChangeAction),
                        aActionMap );
    }
    else
        pChanges->GetDependents(const_cast<ScChangeAction*>(pScChangeAction),
                    aActionMap, pScChangeAction->IsMasterDelete() );
}

bool ScAcceptChgDlg::InsertContentChildren(ScChangeActionMap* pActionMap, const weld::TreeIter& rParent)
{
    bool bTheTestFlag = true;
    weld::TreeView& rTreeView = pTheView->GetWidget();
    ScRedlinData *pEntryData = reinterpret_cast<ScRedlinData*>(rTreeView.get_id(rParent).toInt64());
    const ScChangeAction* pScChangeAction = static_cast<ScChangeAction*>(pEntryData->pData);
    bool bParentInserted = false;
    // If the parent is a MatrixOrigin then place it in the right order before
    // the MatrixReferences. Also if it is the first content change at this
    // position don't insert the first dependent MatrixReference as the special
    // content (original value) but insert the predecessor of the MatrixOrigin
    // itself instead.
    if ( pScChangeAction->GetType() == SC_CAT_CONTENT &&
            static_cast<const ScChangeActionContent*>(pScChangeAction)->IsMatrixOrigin() )
    {
        pActionMap->insert( ::std::make_pair( pScChangeAction->GetActionNumber(),
            const_cast<ScChangeAction*>( pScChangeAction ) ) );
        bParentInserted = true;
    }

    ScChangeActionMap::iterator itChangeAction = std::find_if(pActionMap->begin(), pActionMap->end(),
        [](const std::pair<sal_uLong, ScChangeAction*>& rEntry) { return rEntry.second->GetState() == SC_CAS_VIRGIN; });

    if( itChangeAction == pActionMap->end() )
        return true;

    std::unique_ptr<weld::TreeIter> xOriginal = InsertChangeActionContent(
        dynamic_cast<const ScChangeActionContent*>( itChangeAction->second ),
        rParent, RD_SPECIAL_CONTENT );

    if (xOriginal)
    {
        bTheTestFlag=false;
        ScRedlinData *pParentData = reinterpret_cast<ScRedlinData*>(rTreeView.get_id(*xOriginal).toInt64());
        pParentData->pData=const_cast<ScChangeAction *>(pScChangeAction);
        pParentData->nActionNo=pScChangeAction->GetActionNumber();
        pParentData->bIsAcceptable=pScChangeAction->IsRejectable(); // select old value
        pParentData->bIsRejectable=false;
        pParentData->bDisabled=false;
    }
    while( itChangeAction != pActionMap->end() )
    {
        if( itChangeAction->second->GetState() == SC_CAS_VIRGIN )
        {
            std::unique_ptr<weld::TreeIter> xEntry =
                InsertChangeActionContent( dynamic_cast<const ScChangeActionContent*>( itChangeAction->second ),
                    rParent, RD_SPECIAL_NONE );

            if (xEntry)
                bTheTestFlag=false;
        }
        ++itChangeAction;
    }

    if ( !bParentInserted )
    {
        std::unique_ptr<weld::TreeIter> xEntry =
            InsertChangeActionContent(static_cast<const ScChangeActionContent*>(
                                pScChangeAction),rParent,RD_SPECIAL_NONE);

        if (xEntry)
        {
            bTheTestFlag=false;
            ScRedlinData *pParentData = reinterpret_cast<ScRedlinData*>(rTreeView.get_id(*xEntry).toInt64());
            pParentData->pData=const_cast<ScChangeAction *>(pScChangeAction);
            pParentData->nActionNo=pScChangeAction->GetActionNumber();
            pParentData->bIsAcceptable=pScChangeAction->IsClickable();
            pParentData->bIsRejectable=false;
            pParentData->bDisabled=false;
        }
    }

    return bTheTestFlag;
}

bool ScAcceptChgDlg::InsertAcceptedORejected(const weld::TreeIter& rParent)
{
    ScChangeTrack* pChanges=pDoc->GetChangeTrack();
    bool bTheTestFlag = true;

    ScChangeActionState eState = SC_CAS_VIRGIN;
    weld::TreeView& rTreeView = pTheView->GetWidget();
    OUString aString = rTreeView.get_text(rParent, 0);
    OUString a2String = aString.copy(0, aStrAllAccepted.getLength());
    if (a2String == aStrAllAccepted)
        eState=SC_CAS_ACCEPTED;
    else
    {
        a2String = aString.copy(0, aStrAllRejected.getLength());
        if (a2String == aStrAllRejected)
            eState=SC_CAS_REJECTED;
    }

    ScChangeAction* pScChangeAction = pChanges->GetFirst();
    while (pScChangeAction)
    {
        if (pScChangeAction->GetState()==eState &&
            AppendFilteredAction(pScChangeAction, eState, false, &rParent))
            bTheTestFlag=false;
        pScChangeAction=pScChangeAction->GetNext();
    }
    return bTheTestFlag;
}

bool ScAcceptChgDlg::InsertChildren(ScChangeActionMap* pActionMap, const weld::TreeIter& rParent)
{
    ScChangeTrack* pChanges=pDoc->GetChangeTrack();
    bool bTheTestFlag = true;

    for( const auto& rChangeAction : *pActionMap )
    {
        std::unique_ptr<weld::TreeIter> xEntry = AppendChangeAction(rChangeAction.second, false, &rParent, false, true);

        if (xEntry)
        {
            bTheTestFlag=false;

            weld::TreeView& rTreeView = pTheView->GetWidget();
            ScRedlinData *pEntryData = reinterpret_cast<ScRedlinData*>(rTreeView.get_id(*xEntry).toInt64());
            pEntryData->bIsRejectable=false;
            pEntryData->bIsAcceptable=false;
            pEntryData->bDisabled=true;

            if (rChangeAction.second->IsDialogParent())
                Expand(pChanges, rChangeAction.second, *xEntry);
        }
    }
    return bTheTestFlag;
}

bool ScAcceptChgDlg::InsertDeletedChildren(const ScChangeAction* pScChangeAction,
                                           ScChangeActionMap* pActionMap, const weld::TreeIter& rParent)
{
    ScChangeTrack* pChanges=pDoc->GetChangeTrack();
    bool bTheTestFlag = true;
    std::unique_ptr<weld::TreeIter> xEntry;

    for( const auto& rChangeAction : *pActionMap )
    {

        if( pScChangeAction != rChangeAction.second )
            xEntry = AppendChangeAction(rChangeAction.second, false, &rParent, false, true);
        else
            xEntry = AppendChangeAction(rChangeAction.second, false, &rParent, true, true);

        if (xEntry)
        {
            weld::TreeView& rTreeView = pTheView->GetWidget();
            ScRedlinData *pEntryData = reinterpret_cast<ScRedlinData*>(rTreeView.get_id(*xEntry).toInt64());
            pEntryData->bIsRejectable=false;
            pEntryData->bIsAcceptable=false;
            pEntryData->bDisabled=true;

            bTheTestFlag=false;

            if (rChangeAction.second->IsDialogParent())
                Expand(pChanges, rChangeAction.second, *xEntry);
        }
    }
    return bTheTestFlag;
}

bool ScAcceptChgDlg::Expand(const ScChangeTrack* pChanges, const ScChangeAction* pScChangeAction,
                            const weld::TreeIter& rEntry, bool bFilter)
{
    bool bTheTestFlag = true;

    if (pChanges && pScChangeAction)
    {
        ScChangeActionMap aActionMap;

        GetDependents(pScChangeAction, aActionMap, rEntry);

        switch(pScChangeAction->GetType())
        {
            case SC_CAT_CONTENT:
            {
                InsertContentChildren(&aActionMap, rEntry);
                bTheTestFlag=!bHasFilterEntry;
                break;
            }
            case SC_CAT_DELETE_COLS:
            case SC_CAT_DELETE_ROWS:
            case SC_CAT_DELETE_TABS:
            {
                InsertDeletedChildren(pScChangeAction, &aActionMap, rEntry);
                bTheTestFlag=!bHasFilterEntry;
                break;
            }
            default:
            {
                if(!bFilter)
                    bTheTestFlag = InsertChildren(&aActionMap, rEntry);
                break;
            }
        }
        aActionMap.clear();
    }
    return bTheTestFlag;
}

IMPL_LINK(ScAcceptChgDlg, ExpandingHandle, const weld::TreeIter&, rEntry, bool)
{
    ScChangeTrack* pChanges=pDoc->GetChangeTrack();
    if (pChanges)
    {
        m_xDialog->set_busy_cursor(true);
        ScChangeActionMap aActionMap;
        weld::TreeView& rTreeView = pTheView->GetWidget();
        ScRedlinData *pEntryData = reinterpret_cast<ScRedlinData*>(rTreeView.get_id(rEntry).toInt64());
        if (!rTreeView.iter_has_child(rEntry))
        {
            bool bTheTestFlag = true;

            if (pEntryData)
            {
                ScChangeAction* pScChangeAction=static_cast<ScChangeAction*>(pEntryData->pData);

                GetDependents(pScChangeAction, aActionMap, rEntry);

                switch (pScChangeAction->GetType())
                {
                    case SC_CAT_CONTENT:
                    {
                        bTheTestFlag = InsertContentChildren( &aActionMap, rEntry );
                        break;
                    }
                    case SC_CAT_DELETE_COLS:
                    case SC_CAT_DELETE_ROWS:
                    case SC_CAT_DELETE_TABS:
                    {
                        bTheTestFlag = InsertDeletedChildren( pScChangeAction, &aActionMap, rEntry );
                        break;
                    }
                    default:
                    {
                        bTheTestFlag = InsertChildren( &aActionMap, rEntry );
                        break;
                    }
                }
                aActionMap.clear();

            }
            else
            {
                bTheTestFlag = InsertAcceptedORejected(rEntry);
            }
            if (bTheTestFlag)
            {
                std::unique_ptr<weld::TreeIter> xEntry(rTreeView.make_iterator());
                rTreeView.insert(&rEntry, -1, &aStrNoEntry, nullptr, nullptr, nullptr, false, xEntry.get());
                rTreeView.set_font_color(*xEntry, COL_GRAY);
            }
        }
        m_xDialog->set_busy_cursor(false);
    }
    return true;
}

void ScAcceptChgDlg::AppendChanges(const ScChangeTrack* pChanges,sal_uLong nStartAction,
                                   sal_uLong nEndAction)
{
    if(pChanges==nullptr)
        return;

    std::unique_ptr<weld::TreeIter> xParent;
    m_xDialog->set_busy_cursor(true);
    weld::TreeView& rTreeView = pTheView->GetWidget();
    rTreeView.freeze();

    bool bTheFlag = false;

    bool bFilterFlag = pTPFilter->IsDate() || pTPFilter->IsRange() ||
        pTPFilter->IsAuthor() || pTPFilter->IsComment();

    bUseColor = bFilterFlag;

    for(sal_uLong i=nStartAction;i<=nEndAction;i++)
    {
        const ScChangeAction* pScChangeAction=pChanges->GetAction(i);
        if(pScChangeAction==nullptr) continue;

        switch (pScChangeAction->GetState())
        {
            case SC_CAS_VIRGIN:

                if (pScChangeAction->IsDialogRoot())
                {
                    bool bOnDemandChildren = !bFilterFlag && pScChangeAction->IsDialogParent();
                    if (pScChangeAction->IsDialogParent())
                        xParent = AppendChangeAction(pScChangeAction, bOnDemandChildren);
                    else
                        xParent = AppendFilteredAction(pScChangeAction, SC_CAS_VIRGIN, bOnDemandChildren);
                }
                else
                    xParent.reset();

                bTheFlag=true;
                break;

            case SC_CAS_ACCEPTED:
                xParent.reset();
                nAcceptCount++;
                break;

            case SC_CAS_REJECTED:
                xParent.reset();
                nRejectCount++;
                break;
        }

        if (xParent && pScChangeAction->IsDialogParent() && bFilterFlag)
        {
            bool bTestFlag = bHasFilterEntry;
            bHasFilterEntry = false;
            if (Expand(pChanges,pScChangeAction,*xParent,!bTestFlag)&&!bTestFlag)
                rTreeView.remove(*xParent);
        }
    }

    if( bTheFlag && (!pDoc->IsDocEditable() || pChanges->IsProtected()) )
        bTheFlag=false;

    pTPView->EnableAccept(bTheFlag);
    pTPView->EnableAcceptAll(bTheFlag);
    pTPView->EnableReject(bTheFlag);
    pTPView->EnableRejectAll(bTheFlag);

    rTreeView.thaw();
    m_xDialog->set_busy_cursor(false);
}

void ScAcceptChgDlg::RemoveEntries(sal_uLong nStartAction,sal_uLong nEndAction)
{
    weld::TreeView& rTreeView = pTheView->GetWidget();

    ScRedlinData *pEntryData=nullptr;
    std::unique_ptr<weld::TreeIter> xEntry(rTreeView.make_iterator());
    if (rTreeView.get_cursor(xEntry.get()))
        pEntryData = reinterpret_cast<ScRedlinData*>(rTreeView.get_id(*xEntry).toInt64());

    if (!rTreeView.get_iter_first(*xEntry))
        return;

    sal_uLong nAction=0;
    if (pEntryData)
        nAction=pEntryData->nActionNo;

    if (nAction>=nStartAction && nAction<=nEndAction)
        rTreeView.set_cursor(*xEntry);

    std::vector<OUString> aIdsToRemove;

    do
    {
        OUString sId(rTreeView.get_id(*xEntry));
        pEntryData = reinterpret_cast<ScRedlinData *>(sId.toInt64());
        if (pEntryData)
        {
            nAction = pEntryData->nActionNo;
            if (nStartAction <= nAction && nAction <= nEndAction)
                aIdsToRemove.push_back(sId);
        }
    }
    while (rTreeView.iter_next(*xEntry));

    rTreeView.freeze();

    // MUST do it backwards, don't delete parents before children and GPF
    for (auto it = aIdsToRemove.rbegin(); it != aIdsToRemove.rend(); ++it)
        rTreeView.remove_id(*it);

    rTreeView.thaw();
}

void ScAcceptChgDlg::UpdateEntries(const ScChangeTrack* pChgTrack, sal_uLong nStartAction,sal_uLong nEndAction)
{
    weld::TreeView& rTreeView = pTheView->GetWidget();
    rTreeView.freeze();

    std::unique_ptr<weld::TreeIter> xEntry(rTreeView.make_iterator());
    std::unique_ptr<weld::TreeIter> xLastEntry(rTreeView.make_iterator());
    std::unique_ptr<weld::TreeIter> xNextEntry(rTreeView.make_iterator());

    bool bEntry = rTreeView.get_iter_first(*xEntry);
    bool bLastEntry = false;

    while (bEntry)
    {
        bool bRemove = false;
        ScRedlinData *pEntryData = reinterpret_cast<ScRedlinData*>(rTreeView.get_id(*xEntry).toInt64());
        if (pEntryData)
        {
            ScChangeAction* pScChangeAction=
                    static_cast<ScChangeAction*>(pEntryData->pData);

            sal_uLong nAction=pScChangeAction->GetActionNumber();

            if(nStartAction<=nAction && nAction<=nEndAction) bRemove=true;
        }

        bool bNextEntry;
        if (bRemove)
        {
            rTreeView.remove(*xEntry);

            if (!bLastEntry)
                bLastEntry = rTreeView.get_iter_first(*xLastEntry);
            if (bLastEntry)
            {
                rTreeView.copy_iterator(*xLastEntry, *xNextEntry);
                bNextEntry = rTreeView.iter_next(*xNextEntry);
                if (!bNextEntry)
                {
                    rTreeView.copy_iterator(*xLastEntry, *xNextEntry);
                    bLastEntry = false;
                }
            }
            else
                bNextEntry = false;
        }
        else
        {
            rTreeView.copy_iterator(*xEntry, *xLastEntry);
            bLastEntry = true;

            rTreeView.copy_iterator(*xEntry, *xNextEntry);
            bNextEntry = rTreeView.iter_next(*xNextEntry);
        }

        rTreeView.copy_iterator(*xNextEntry, *xEntry);
        bEntry = bNextEntry;
    }

    AppendChanges(pChgTrack,nStartAction,nEndAction);

    rTreeView.thaw();
}

IMPL_LINK( ScAcceptChgDlg, ChgTrackModHdl, ScChangeTrack&, rChgTrack, void)
{
    ScChangeTrackMsgQueue& aMsgQueue= rChgTrack.GetMsgQueue();

    sal_uLong   nStartAction;
    sal_uLong   nEndAction;

    for (const auto& rMsg : aMsgQueue)
    {
        nStartAction = rMsg.nStartAction;
        nEndAction = rMsg.nEndAction;

        if(!bIgnoreMsg)
        {
            bNoSelection=true;

            switch(rMsg.eMsgType)
            {
                case ScChangeTrackMsgType::Append:
                                    AppendChanges(&rChgTrack,nStartAction,nEndAction);
                                    break;
                case ScChangeTrackMsgType::Remove:
                                    RemoveEntries(nStartAction,nEndAction);
                                    break;
                case ScChangeTrackMsgType::Parent:
                case ScChangeTrackMsgType::Change: //bNeedsUpdate=true;
                                    UpdateEntries(&rChgTrack,nStartAction,nEndAction);
                                    break;
                default: assert(false); break;
            }
        }
    }

    aMsgQueue.clear();
}

IMPL_LINK_NOARG(ScAcceptChgDlg, ReOpenTimerHdl, Timer *, void)
{
    ScSimpleRefDlgWrapper::SetAutoReOpen(true);
    m_xAcceptChgCtr->ShowFilterPage();
    RefHandle(nullptr);
}

IMPL_LINK_NOARG(ScAcceptChgDlg, UpdateSelectionHdl, Timer *, void)
{
    ScTabView* pTabView = pViewData->GetView();

    bool bAcceptFlag = true;
    bool bRejectFlag = true;

    pTabView->DoneBlockMode();  // clears old marking
    weld::TreeView& rTreeView = pTheView->GetWidget();
    std::vector<const ScChangeAction*> aActions;
    rTreeView.selected_foreach([&rTreeView, &bAcceptFlag, &bRejectFlag, &aActions](weld::TreeIter& rEntry){
        ScRedlinData* pEntryData = reinterpret_cast<ScRedlinData*>(rTreeView.get_id(rEntry).toInt64());
        if (pEntryData)
        {
            bRejectFlag &= pEntryData->bIsRejectable;
            bAcceptFlag &= pEntryData->bIsAcceptable;

            const ScChangeAction* pScChangeAction = static_cast<ScChangeAction*>(pEntryData->pData);
            if( pScChangeAction && (pScChangeAction->GetType() != SC_CAT_DELETE_TABS) &&
                    (!pEntryData->bDisabled || pScChangeAction->IsVisible()) )
            {
                aActions.push_back(pScChangeAction);
            }
        }
        else
        {
            bAcceptFlag = false;
            bRejectFlag = false;
        }
        return false;
    });

    bool bContMark = false;
    for (size_t i = 0, nCount = aActions.size(); i < nCount; ++i)
    {
        const ScBigRange& rBigRange = aActions[i]->GetBigRange();
        if (rBigRange.IsValid(pDoc) && m_xDialog->has_toplevel_focus())
        {
            bool bSetCursor = i == nCount - 1;
            pTabView->MarkRange(rBigRange.MakeRange(), bSetCursor, bContMark);
            bContMark = true;
        }
    }

    ScChangeTrack* pChanges = pDoc->GetChangeTrack();
    bool bEnable = pDoc->IsDocEditable() && pChanges && !pChanges->IsProtected();
    pTPView->EnableAccept( bAcceptFlag && bEnable );
    pTPView->EnableReject( bRejectFlag && bEnable );
}

IMPL_LINK(ScAcceptChgDlg, CommandHdl, const CommandEvent&, rCEvt, bool)
{
    if (rCEvt.GetCommand() != CommandEventId::ContextMenu)
        return false;

    weld::TreeView& rTreeView = pTheView->GetWidget();
    std::unique_ptr<weld::TreeIter> xEntry(rTreeView.make_iterator());
    bool bEntry = rTreeView.get_cursor(xEntry.get());
    if (bEntry)
        rTreeView.select(*xEntry);

    int nSortedCol = rTreeView.get_sort_column();
    for (sal_Int32 i = 0; i < 5; ++i)
        m_xSortMenu->set_active("calcsort" + OString::number(i), i == nSortedCol);

    m_xPopup->set_sensitive("calcedit", false);

    if (pDoc->IsDocEditable() && bEntry)
    {
        ScRedlinData *pEntryData = reinterpret_cast<ScRedlinData*>(rTreeView.get_id(*xEntry).toInt64());
        if (pEntryData)
        {
            ScChangeAction* pScChangeAction = static_cast<ScChangeAction*>(pEntryData->pData);
            if (pScChangeAction && !rTreeView.get_iter_depth(*xEntry))
                m_xPopup->set_sensitive("calcedit", true);
        }
    }

    OString sCommand = m_xPopup->popup_at_rect(&rTreeView, tools::Rectangle(rCEvt.GetMousePosPixel(), Size(1,1)));

    if (!sCommand.isEmpty())
    {
        if (sCommand == "calcedit")
        {
            if (bEntry)
            {
                ScRedlinData *pEntryData = reinterpret_cast<ScRedlinData*>(rTreeView.get_id(*xEntry).toInt64());
                if (pEntryData)
                {
                    ScChangeAction* pScChangeAction = static_cast<ScChangeAction*>(pEntryData->pData);
                    pViewData->GetDocShell()->ExecuteChangeCommentDialog(pScChangeAction, m_xDialog.get(), false);
                }
            }
        }
        else
        {
            int nDialogCol = sCommand.copy(8).toInt32();
            pTheView->HeaderBarClick(nDialogCol);
        }
    }

    return true;
}

namespace
{
    //at one point we were writing multiple AcceptChgDat strings,
    //so strip all of them and keep the results of the last one
    OUString lcl_StripAcceptChgDat(OUString &rExtraString)
    {
        OUString aStr;
        while (true)
        {
            sal_Int32 nPos = rExtraString.indexOf("AcceptChgDat:");
            if (nPos == -1)
                break;
            // Try to read the alignment string "ALIGN:(...)"; if it is missing
            // we have an old version
            sal_Int32 n1 = rExtraString.indexOf('(', nPos);
            if ( n1 != -1 )
            {
                sal_Int32 n2 = rExtraString.indexOf(')', n1);
                if ( n2 != -1 )
                {
                    // cut out alignment string
                    aStr = rExtraString.copy(nPos, n2 - nPos + 1);
                    rExtraString = rExtraString.replaceAt(nPos, n2 - nPos + 1, "");
                    aStr = aStr.copy( n1-nPos+1 );
                }
            }
        }
        return aStr;
    }
}

void ScAcceptChgDlg::Initialize(SfxChildWinInfo* pInfo)
{
    OUString aStr;
    if (pInfo && !pInfo->aExtraString.isEmpty())
        aStr = lcl_StripAcceptChgDat(pInfo->aExtraString);

    SfxModelessDialogController::Initialize(pInfo);

    if (aStr.isEmpty())
        return;

    int nCount = aStr.toInt32();
    if (nCount <= 2)
        return;

    std::vector<int> aEndPos;

    for (int i = 0; i < nCount; ++i)
    {
        sal_Int32 n1 = aStr.indexOf(';');
        aStr = aStr.copy( n1+1 );
        aEndPos.push_back(aStr.toInt32());
    }

    std::vector<int> aWidths;
    for (int i = 1; i < nCount; ++i)
        aWidths.push_back(aEndPos[i] - aEndPos[i - 1]);

    // turn column end points back to column widths, ignoring the small
    // value used for the expander column
    weld::TreeView& rTreeView = pTheView->GetWidget();
    rTreeView.set_column_fixed_widths(aWidths);
}

void ScAcceptChgDlg::FillInfo(SfxChildWinInfo& rInfo) const
{
    SfxModelessDialogController::FillInfo(rInfo);
    //remove any old one before adding a new one
    lcl_StripAcceptChgDat(rInfo.aExtraString);
    rInfo.aExtraString += "AcceptChgDat:(";

    const int nTabCount = 5;

    rInfo.aExtraString += OUString::number(nTabCount);
    rInfo.aExtraString += ";";

    weld::TreeView& rTreeView = pTheView->GetWidget();
    std::vector<int> aWidths;
    // turn column widths back into column end points for compatibility
    // with how they used to be stored, including a small value for the
    // expander column
    aWidths.push_back(rTreeView.get_checkbox_column_width());
    for (int i = 0; i < nTabCount - 1; ++i)
        aWidths.push_back(aWidths.back() + rTreeView.get_column_width(i));

    for (auto a : aWidths)
    {
        rInfo.aExtraString += OUString::number(a);
        rInfo.aExtraString += ";";
    }
    rInfo.aExtraString += ")";
}

#define CALC_DATE       3
#define CALC_POS        1

int ScAcceptChgDlg::ColCompareHdl(const weld::TreeIter& rLeft, const weld::TreeIter& rRight) const
{
    weld::TreeView& rTreeView = pTheView->GetWidget();

    sal_Int32 nCompare = 0;
    SCCOL nSortCol = rTreeView.get_sort_column();

    if (CALC_DATE == nSortCol)
    {
        RedlinData *pLeftData = reinterpret_cast<RedlinData*>(rTreeView.get_id(rLeft).toInt64());
        RedlinData *pRightData = reinterpret_cast<RedlinData*>(rTreeView.get_id(rRight).toInt64());
        if (pLeftData && pRightData)
        {
            if(pLeftData->aDateTime < pRightData->aDateTime)
                nCompare = -1;
            else if(pLeftData->aDateTime > pRightData->aDateTime)
                nCompare = 1;
            return nCompare;
        }
    }
    else if (CALC_POS == nSortCol)
    {
        ScRedlinData *pLeftData = reinterpret_cast<ScRedlinData*>(rTreeView.get_id(rLeft).toInt64());
        ScRedlinData *pRightData = reinterpret_cast<ScRedlinData*>(rTreeView.get_id(rRight).toInt64());
        if (pLeftData && pRightData)
        {
            nCompare = 1;

            if(pLeftData->nTable < pRightData->nTable)
                nCompare = -1;
            else if(pLeftData->nTable == pRightData->nTable)
            {
                if(pLeftData->nRow < pRightData->nRow)
                    nCompare = -1;
                else if(pLeftData->nRow == pRightData->nRow)
                {
                    if(pLeftData->nCol < pRightData->nCol)
                        nCompare = -1;
                    else if(pLeftData->nCol == pRightData->nCol)
                        nCompare = 0;
                }
            }

            return nCompare;
        }
    }

    return ScGlobal::GetCaseCollator()->compareString(rTreeView.get_text(rLeft, nSortCol),
                                                      rTreeView.get_text(rRight, nSortCol));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
