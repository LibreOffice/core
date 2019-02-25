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

#include <sfx2/dispatch.hxx>
#include <vcl/waitobj.hxx>
#include <sal/log.hxx>

#include <uiitems.hxx>
#include <reffact.hxx>
#include <viewdata.hxx>
#include <document.hxx>
#include <docsh.hxx>
#include <scresid.hxx>
#include <queryentry.hxx>

#include <foptmgr.hxx>

#include <globstr.hrc>
#include <strings.hrc>

#include <filtdlg.hxx>
#include <vcl/weld.hxx>
#include <svl/sharedstringpool.hxx>

#include <limits>

#define QUERY_ENTRY_COUNT 4
#define INVALID_HEADER_POS std::numeric_limits<size_t>::max()

ScFilterDlg::EntryList::EntryList() :
    mnHeaderPos(INVALID_HEADER_POS) {}

ScFilterDlg::ScFilterDlg(SfxBindings* pB, SfxChildWindow* pCW, vcl::Window* pParent,
    const SfxItemSet& rArgSet)
    : ScAnyRefDlg ( pB, pCW, pParent, "StandardFilterDialog",
        "modules/scalc/ui/standardfilterdialog.ui" )
    , aStrUndefined(ScResId(SCSTR_UNDEFINED))
    , aStrNone(ScResId(SCSTR_NONE))
    , aStrEmpty(ScResId(SCSTR_FILTER_EMPTY))
    , aStrNotEmpty(ScResId(SCSTR_FILTER_NOTEMPTY))
    , aStrColumn(ScResId(SCSTR_COLUMN))
    , nWhichQuery(rArgSet.GetPool()->GetWhich(SID_QUERY))
    , theQueryData(static_cast<const ScQueryItem&>(rArgSet.Get(nWhichQuery)).GetQueryData())
    , pViewData(nullptr)
    , pDoc(nullptr)
    , nSrcTab(0)
    , bRefInputMode(false)
{
    get(pLbConnect1,"connect1");
    get(pLbField1,"field1");
    get(pLbCond1,"cond1");
    get(pEdVal1,"val1");
    get(pLbConnect2,"connect2");
    get(pLbField2,"field2");
    get(pLbCond2,"cond2");
    get(pEdVal2,"val2");
    get(pLbConnect3,"connect3");
    get(pLbField3,"field3");
    get(pLbCond3,"cond3");
    get(pEdVal3,"val3");
    get(pLbConnect4,"connect4");
    get(pLbField4,"field4");
    get(pLbCond4,"cond4");
    get(pEdVal4,"val4");
    get(pScrollBar,"scrollbar");
    get(pExpander,"more");
    pExpander->SetExpandedHdl(LINK(this, ScFilterDlg, MoreExpandedHdl));
    get(pBtnOk,"ok");
    get(pBtnCancel,"cancel");
    get(pBtnCase,"case");
    get(pBtnRegExp,"regexp");
    get(pBtnHeader,"header");
    get(pBtnUnique,"unique");
    get(pBtnCopyResult,"copyresult");
    get(pLbCopyArea,"lbcopyarea");
    get(pEdCopyArea,"edcopyarea");
    pEdCopyArea->SetReferences(this, pBtnCopyResult);
    get(pRbCopyArea,"rbcopyarea");
    pRbCopyArea->SetReferences(this, pEdCopyArea);
    get(pBtnDestPers,"destpers");
    get(pFtDbAreaLabel,"dbarealabel");
    get(pFtDbArea,"dbarea");

    Init( rArgSet );

    // Hack: RefInput control
    pTimer.reset( new Timer("ScFilterTimer") );
    pTimer->SetTimeout( 50 ); // Wait 50ms
    pTimer->SetInvokeHandler( LINK( this, ScFilterDlg, TimeOutHdl ) );
}

ScFilterDlg::~ScFilterDlg()
{
    disposeOnce();
}

void ScFilterDlg::dispose()
{
    pOptionsMgr.reset();
    pOutItem.reset();

    // Hack: RefInput control
    pTimer->Stop();
    pTimer.reset();

    pLbConnect1.clear();
    pLbField1.clear();
    pLbCond1.clear();
    pEdVal1.clear();
    pLbConnect2.clear();
    pLbField2.clear();
    pLbCond2.clear();
    pEdVal2.clear();
    pLbConnect3.clear();
    pLbField3.clear();
    pLbCond3.clear();
    pEdVal3.clear();
    pLbConnect4.clear();
    pLbField4.clear();
    pLbCond4.clear();
    pEdVal4.clear();
    pScrollBar.clear();
    pExpander.clear();
    pBtnOk.clear();
    pBtnCancel.clear();
    pBtnCase.clear();
    pBtnRegExp.clear();
    pBtnHeader.clear();
    pBtnUnique.clear();
    pBtnCopyResult.clear();
    pLbCopyArea.clear();
    pEdCopyArea.clear();
    pRbCopyArea.clear();
    pBtnDestPers.clear();
    pFtDbAreaLabel.clear();
    pFtDbArea.clear();
    maValueEdArr.clear();
    maFieldLbArr.clear();
    maCondLbArr.clear();
    maConnLbArr.clear();
    ScAnyRefDlg::dispose();
}

void ScFilterDlg::Init( const SfxItemSet& rArgSet )
{
    const ScQueryItem& rQueryItem = static_cast<const ScQueryItem&>(
                                    rArgSet.Get( nWhichQuery ));

    pBtnOk->SetClickHdl      ( LINK( this, ScFilterDlg, EndDlgHdl ) );
    pBtnCancel->SetClickHdl  ( LINK( this, ScFilterDlg, EndDlgHdl ) );
    pBtnHeader->SetClickHdl  ( LINK( this, ScFilterDlg, CheckBoxHdl ) );
    pBtnCase->SetClickHdl    ( LINK( this, ScFilterDlg, CheckBoxHdl ) );

    pLbField1->SetSelectHdl  ( LINK( this, ScFilterDlg, LbSelectHdl ) );
    pLbField2->SetSelectHdl  ( LINK( this, ScFilterDlg, LbSelectHdl ) );
    pLbField3->SetSelectHdl  ( LINK( this, ScFilterDlg, LbSelectHdl ) );
    pLbField4->SetSelectHdl  ( LINK( this, ScFilterDlg, LbSelectHdl ) );
    pLbConnect1->SetSelectHdl( LINK( this, ScFilterDlg, LbSelectHdl ) );
    pLbConnect2->SetSelectHdl( LINK( this, ScFilterDlg, LbSelectHdl ) );
    pLbConnect3->SetSelectHdl( LINK( this, ScFilterDlg, LbSelectHdl ) );
    pLbConnect4->SetSelectHdl( LINK( this, ScFilterDlg, LbSelectHdl ) );

    pLbField1->setMaxWidthChars(10);
    pLbField2->setMaxWidthChars(10);
    pLbField3->setMaxWidthChars(10);
    pLbField4->setMaxWidthChars(10);

    pLbCond1->SetSelectHdl( LINK( this, ScFilterDlg, LbSelectHdl ) );
    pLbCond2->SetSelectHdl( LINK( this, ScFilterDlg, LbSelectHdl ) );
    pLbCond3->SetSelectHdl( LINK( this, ScFilterDlg, LbSelectHdl ) );
    pLbCond4->SetSelectHdl( LINK( this, ScFilterDlg, LbSelectHdl ) );

    pViewData   = rQueryItem.GetViewData();
    pDoc        = pViewData ? pViewData->GetDocument() : nullptr;
    nSrcTab     = pViewData ? pViewData->GetTabNo() : static_cast<SCTAB>(0);

    // for easier access:
    maFieldLbArr.reserve(QUERY_ENTRY_COUNT);
    maFieldLbArr.push_back(pLbField1);
    maFieldLbArr.push_back(pLbField2);
    maFieldLbArr.push_back(pLbField3);
    maFieldLbArr.push_back(pLbField4);
    maValueEdArr.reserve(QUERY_ENTRY_COUNT);
    maValueEdArr.push_back(pEdVal1);
    maValueEdArr.push_back(pEdVal2);
    maValueEdArr.push_back(pEdVal3);
    maValueEdArr.push_back(pEdVal4);
    maCondLbArr.reserve(QUERY_ENTRY_COUNT);
    maCondLbArr.push_back(pLbCond1);
    maCondLbArr.push_back(pLbCond2);
    maCondLbArr.push_back(pLbCond3);
    maCondLbArr.push_back(pLbCond4);
    maConnLbArr.reserve(QUERY_ENTRY_COUNT);
    maConnLbArr.push_back(pLbConnect1);
    maConnLbArr.push_back(pLbConnect2);
    maConnLbArr.push_back(pLbConnect3);
    maConnLbArr.push_back(pLbConnect4);

    // Option initialization:
    pOptionsMgr.reset( new ScFilterOptionsMgr(
                            pViewData,
                            theQueryData,
                            pBtnCase,
                            pBtnRegExp,
                            pBtnHeader,
                            pBtnUnique,
                            pBtnCopyResult,
                            pBtnDestPers,
                            pLbCopyArea,
                            pEdCopyArea,
                            pRbCopyArea,
                            pFtDbAreaLabel,
                            pFtDbArea,
                            aStrUndefined ) );
    // Read in field lists and select entries

    FillFieldLists();

    for (size_t i = 0; i < QUERY_ENTRY_COUNT; ++i)
    {
        OUString aValStr;
        size_t nCondPos = 0;
        size_t nFieldSelPos = 0;

        ScQueryEntry& rEntry = theQueryData.GetEntry(i);
        if ( rEntry.bDoQuery )
        {
            const ScQueryEntry::Item& rItem = rEntry.GetQueryItem();
            nCondPos = static_cast<size_t>(rEntry.eOp);
            nFieldSelPos = GetFieldSelPos( static_cast<SCCOL>(rEntry.nField) );
            if (rEntry.IsQueryByEmpty())
            {
                aValStr = aStrEmpty;
                maCondLbArr[i]->Disable();
            }
            else if (rEntry.IsQueryByNonEmpty())
            {
                aValStr = aStrNotEmpty;
                maCondLbArr[i]->Disable();
            }
            else
            {
                OUString aQueryStr = rItem.maString.getString();
                if (aQueryStr.isEmpty())
                {
                    if (rItem.meType == ScQueryEntry::ByValue)
                    {
                        if (pDoc)
                        {
                            pDoc->GetFormatTable()->GetInputLineString(rItem.mfVal, 0, aValStr);
                        }
                    }
                    else if (rItem.meType == ScQueryEntry::ByDate)
                    {
                        if (pDoc)
                        {
                            SvNumberFormatter* pFormatter = pDoc->GetFormatTable();
                            pFormatter->GetInputLineString(rItem.mfVal,
                                                           pFormatter->GetStandardFormat( SvNumFormatType::DATE), aValStr);
                        }
                    }
                    else
                    {
                        SAL_WARN( "sc", "ScFilterDlg::Init: empty query string, really?");
                        aValStr = aQueryStr;
                    }
                }
                else
                {
                    // XXX NOTE: if not ByString we just assume this has been
                    // set to a proper string corresponding to the numeric
                    // value earlier!
                    aValStr = aQueryStr;
                }
            }
        }
        else if ( i == 0 )
        {
            nFieldSelPos = pViewData ? GetFieldSelPos(pViewData->GetCurX()) : 0;
            rEntry.nField = nFieldSelPos ? (theQueryData.nCol1 +
                static_cast<SCCOL>(nFieldSelPos) - 1) : static_cast<SCCOL>(0);
            rEntry.bDoQuery=true;
            if (maRefreshExceptQuery.size() < i + 1)
                maRefreshExceptQuery.resize(i + 1, false);
            maRefreshExceptQuery[i] = true;

        }
        maFieldLbArr[i]->SelectEntryPos( nFieldSelPos );
        maCondLbArr [i]->SelectEntryPos( nCondPos );
        maValueEdArr[i]->SetText( aValStr );
        maValueEdArr[i]->EnableAutocomplete( false );
        maValueEdArr[i]->SetModifyHdl( LINK( this, ScFilterDlg, ValModifyHdl ) );
        UpdateValueList(i+1);
    }

    pScrollBar->SetEndScrollHdl( LINK( this, ScFilterDlg, ScrollHdl ) );
    pScrollBar->SetScrollHdl( LINK( this, ScFilterDlg, ScrollHdl ) );

    pScrollBar->SetRange( Range( 0, 4 ) );
    pScrollBar->SetLineSize( 1 );
    pLbConnect1->Hide();
    // Disable/Enable Logic:

    (pLbField1->GetSelectedEntryPos() != 0)
    && (pLbField2->GetSelectedEntryPos() != 0)
        ? pLbConnect2->SelectEntryPos( static_cast<sal_uInt16>(theQueryData.GetEntry(1).eConnect) )
        : pLbConnect2->SetNoSelection();

    (pLbField2->GetSelectedEntryPos() != 0)
    && (pLbField3->GetSelectedEntryPos() != 0)
        ? pLbConnect3->SelectEntryPos( static_cast<sal_uInt16>(theQueryData.GetEntry(2).eConnect) )
        : pLbConnect3->SetNoSelection();

    (pLbField3->GetSelectedEntryPos() != 0)
    && (pLbField4->GetSelectedEntryPos() != 0)
        ? pLbConnect4->SelectEntryPos( static_cast<sal_uInt16>(theQueryData.GetEntry(3).eConnect) )
        : pLbConnect4->SetNoSelection();
    if ( pLbField1->GetSelectedEntryPos() == 0 )
    {
        pLbConnect2->Disable();
        pLbField2->Disable();
        pLbCond2->Disable();
        pEdVal2->Disable();
    }
    else if ( pLbConnect2->GetSelectedEntryCount() == 0 )
    {
        pLbField2->Disable();
        pLbCond2->Disable();
        pEdVal2->Disable();
    }

    if ( pLbField2->GetSelectedEntryPos() == 0 )
    {
        pLbConnect3->Disable();
        pLbField3->Disable();
        pLbCond3->Disable();
        pEdVal3->Disable();
    }
    else if ( pLbConnect3->GetSelectedEntryCount() == 0 )
    {
        pLbField3->Disable();
        pLbCond3->Disable();
        pEdVal3->Disable();
    }
    if ( pLbField3->GetSelectedEntryPos() == 0 )
    {
        pLbConnect4->Disable();
        pLbField4->Disable();
        pLbCond4->Disable();
        pEdVal4->Disable();
    }
    else if ( pLbConnect4->GetSelectedEntryCount() == 0 )
    {
        pLbField4->Disable();
        pLbCond4->Disable();
        pEdVal4->Disable();
    }

    pEdVal1->setMaxWidthChars(10);
    pEdVal2->setMaxWidthChars(10);
    pEdVal3->setMaxWidthChars(10);
    pEdVal4->setMaxWidthChars(10);

    if (pDoc != nullptr && pDoc->GetChangeTrack() != nullptr)
        pBtnCopyResult->Disable();
}

bool ScFilterDlg::Close()
{
    if (pViewData)
        pViewData->GetDocShell()->CancelAutoDBRange();

    return DoClose( ScFilterDlgWrapper::GetChildWindowId() );
}

// Mouse-selected cell area becomes the new selection and is shown in the
// reference text box

void ScFilterDlg::SetReference( const ScRange& rRef, ScDocument* pDocP )
{
    if ( bRefInputMode )    // Only possible if in reference edit mode
    {
        if ( rRef.aStart != rRef.aEnd )
            RefInputStart( pEdCopyArea );
        OUString aRefStr(rRef.aStart.Format(ScRefFlags::ADDR_ABS_3D, pDocP, pDocP->GetAddressConvention()));
        pEdCopyArea->SetRefString( aRefStr );
    }
}

void ScFilterDlg::SetActive()
{
    if ( bRefInputMode )
    {
        pEdCopyArea->GrabFocus();
        pEdCopyArea->GetModifyHdl().Call( *pEdCopyArea );
    }
    else
        GrabFocus();

    RefInputDone();
}

void ScFilterDlg::FillFieldLists()
{
    pLbField1->Clear();
    pLbField2->Clear();
    pLbField3->Clear();
    pLbField4->Clear();
    pLbField1->InsertEntry( aStrNone, 0 );
    pLbField2->InsertEntry( aStrNone, 0 );
    pLbField3->InsertEntry( aStrNone, 0 );
    pLbField4->InsertEntry( aStrNone, 0 );

    if ( pDoc )
    {
        OUString aFieldName;
        SCTAB   nTab        = nSrcTab;
        SCCOL   nFirstCol   = theQueryData.nCol1;
        SCROW   nFirstRow   = theQueryData.nRow1;
        SCCOL   nMaxCol     = theQueryData.nCol2;
        SCCOL   col = 0;
        sal_uInt16  i=1;

        for ( col=nFirstCol; col<=nMaxCol; col++ )
        {
            aFieldName = pDoc->GetString(col, nFirstRow, nTab);
            if (!pBtnHeader->IsChecked() || aFieldName.isEmpty())
            {
                aFieldName = ScGlobal::ReplaceOrAppend( aStrColumn, "%1", ScColToAlpha( col ));
            }
            pLbField1->InsertEntry( aFieldName, i );
            pLbField2->InsertEntry( aFieldName, i );
            pLbField3->InsertEntry( aFieldName, i );
            pLbField4->InsertEntry( aFieldName, i );
            i++;
        }
    }
}

void ScFilterDlg::UpdateValueList( size_t nList )
{
    bool bCaseSens = pBtnCase->IsChecked();

    if (pDoc && nList > 0 && nList <= QUERY_ENTRY_COUNT)
    {
        ComboBox*   pValList        = maValueEdArr[nList-1].get();
        const sal_Int32 nFieldSelPos = maFieldLbArr[nList-1]->GetSelectedEntryPos();
        sal_Int32 nListPos = 0;
        OUString aCurValue = pValList->GetText();

        pValList->Clear();
        pValList->InsertEntry( aStrNotEmpty, nListPos++ );
        pValList->InsertEntry( aStrEmpty, nListPos++ );

        if ( nFieldSelPos )
        {
            WaitObject aWaiter( this );     // even if only the list box has content

            SCCOL nColumn = theQueryData.nCol1 + static_cast<SCCOL>(nFieldSelPos) - 1;
            EntryList* pList = nullptr;
            if (!m_EntryLists.count(nColumn))
            {
                size_t nOffset = GetSliderPos();
                SCTAB nTab       = nSrcTab;
                SCROW nFirstRow = theQueryData.nRow1;
                SCROW nLastRow   = theQueryData.nRow2;
                if (maHasDates.size() < nOffset+nList)
                    maHasDates.resize(nOffset+nList, false);
                maHasDates[nOffset+nList-1] = false;

                // first without the first line
                std::pair<EntryListsMap::iterator, bool> r =
                    m_EntryLists.insert(std::make_pair(nColumn, std::make_unique<EntryList>()));
                if (!r.second)
                    // insertion failed.
                    return;

                pList = r.first->second.get();
                pDoc->GetFilterEntriesArea(
                    nColumn, nFirstRow+1, nLastRow,
                    nTab, bCaseSens, pList->maFilterEntries);
                maHasDates[nOffset+nList-1] = pList->maFilterEntries.mbHasDates;

                // Entry for the first line
                //! Entry (pHdrEntry) doesn't generate collection?

                pList->mnHeaderPos = INVALID_HEADER_POS;
                ScFilterEntries aHdrColl;
                pDoc->GetFilterEntriesArea(
                    nColumn, nFirstRow, nFirstRow, nTab, true, aHdrColl );
                if (!aHdrColl.empty())
                {
                    // See if the header value is already in the list.
                    std::vector<ScTypedStrData>::iterator itBeg = pList->maFilterEntries.begin(), itEnd = pList->maFilterEntries.end();
                    if (std::none_of(itBeg, itEnd, FindTypedStrData(aHdrColl.front(), bCaseSens)))
                    {
                        // Not in the list. Insert it.
                        pList->maFilterEntries.push_back(aHdrColl.front());
                        if (bCaseSens)
                            std::sort(pList->maFilterEntries.begin(), pList->maFilterEntries.end(), ScTypedStrData::LessCaseSensitive());
                        else
                            std::sort(pList->maFilterEntries.begin(), pList->maFilterEntries.end(), ScTypedStrData::LessCaseInsensitive());

                        // Record its position.
                        itBeg = pList->maFilterEntries.begin();
                        itEnd = pList->maFilterEntries.end();
                        auto it = std::find_if(itBeg, itEnd, FindTypedStrData(aHdrColl.front(), bCaseSens));
                        pList->mnHeaderPos = std::distance(itBeg, it);
                    }
                }
            }
            else
                pList = m_EntryLists[nColumn].get();

            OSL_ASSERT(pList);

            for (const auto& rEntry : pList->maFilterEntries)
            {
                pValList->InsertEntry(rEntry.GetString(), nListPos++);
            }
        }
        pValList->SetText( aCurValue );
        pValList->EnableDDAutoWidth(false);
    }

    UpdateHdrInValueList( nList );
}

void ScFilterDlg::UpdateHdrInValueList( size_t nList )
{
    //! GetText / SetText ??

    if (!pDoc)
        return;

    if (nList == 0 || nList > QUERY_ENTRY_COUNT)
        return;

    size_t nFieldSelPos = maFieldLbArr[nList-1]->GetSelectedEntryPos();
    if (!nFieldSelPos)
        return;

    SCCOL nColumn = theQueryData.nCol1 + static_cast<SCCOL>(nFieldSelPos) - 1;
    if (!m_EntryLists.count(nColumn))
    {
        OSL_FAIL("column not yet initialized");
        return;
    }

    size_t const nPos = m_EntryLists[nColumn]->mnHeaderPos;
    if (nPos == INVALID_HEADER_POS)
        return;

    ComboBox* pValList = maValueEdArr[nList-1].get();
    size_t nListPos = nPos + 2;                 // for "empty" and "non-empty"

    const ScTypedStrData& rHdrEntry = m_EntryLists[nColumn]->maFilterEntries.maStrData[nPos];

    const OUString& aHdrStr = rHdrEntry.GetString();
    bool bWasThere = aHdrStr == pValList->GetEntry(nListPos);
    bool bInclude = !pBtnHeader->IsChecked();

    if (bInclude)           // Include entry
    {
        if (!bWasThere)
            pValList->InsertEntry(aHdrStr, nListPos);
    }
    else                    // Omit entry
    {
        if (bWasThere)
            pValList->RemoveEntryAt(nListPos);
    }
}

void ScFilterDlg::ClearValueList( size_t nList )
{
    if (nList > 0 && nList <= QUERY_ENTRY_COUNT)
    {
        ComboBox* pValList = maValueEdArr[nList-1].get();
        pValList->Clear();
        pValList->InsertEntry( aStrNotEmpty, 0 );
        pValList->InsertEntry( aStrEmpty, 1 );
        pValList->SetText( EMPTY_OUSTRING );
    }
}

size_t ScFilterDlg::GetFieldSelPos( SCCOL nField )
{
    if ( nField >= theQueryData.nCol1 && nField <= theQueryData.nCol2 )
        return static_cast<size_t>(nField - theQueryData.nCol1 + 1);
    else
        return 0;
}

ScQueryItem* ScFilterDlg::GetOutputItem()
{
    ScAddress       theCopyPos;
    ScQueryParam    theParam( theQueryData );
    bool            bCopyPosOk = false;

    if ( pBtnCopyResult->IsChecked() )
    {
        ScRefFlags nResult = theCopyPos.Parse(
            pEdCopyArea->GetText(), pDoc, pDoc->GetAddressConvention());
        bCopyPosOk = (nResult & ScRefFlags::VALID) == ScRefFlags::VALID;
    }

    if ( pBtnCopyResult->IsChecked() && bCopyPosOk )
    {
        theParam.bInplace   = false;
        theParam.nDestTab   = theCopyPos.Tab();
        theParam.nDestCol   = theCopyPos.Col();
        theParam.nDestRow   = theCopyPos.Row();
    }
    else
    {
        theParam.bInplace   = true;
        theParam.nDestTab   = 0;
        theParam.nDestCol   = 0;
        theParam.nDestRow   = 0;
    }

    theParam.bHasHeader     = pBtnHeader->IsChecked();
    theParam.bByRow         = true;
    theParam.bDuplicate     = !pBtnUnique->IsChecked();
    theParam.bCaseSens      = pBtnCase->IsChecked();
    theParam.eSearchType    = pBtnRegExp->IsChecked() ? utl::SearchParam::SearchType::Regexp : utl::SearchParam::SearchType::Normal;
    theParam.bDestPers      = pBtnDestPers->IsChecked();

    // only set the three - reset everything else

    pOutItem.reset( new ScQueryItem( nWhichQuery, &theParam ) );

    return pOutItem.get();
}

bool ScFilterDlg::IsRefInputMode() const
{
    return bRefInputMode;
}

// Handler:

IMPL_LINK( ScFilterDlg, EndDlgHdl, Button*, pBtn, void )
{
    if ( pBtn == pBtnOk )
    {
        bool bAreaInputOk = true;

        if ( pBtnCopyResult->IsChecked() )
        {
            if ( !pOptionsMgr->VerifyPosStr( pEdCopyArea->GetText() ) )
            {
                if (!pExpander->get_expanded())
                  pExpander->set_expanded(true);

                std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                          VclMessageType::Warning, VclButtonsType::Ok,
                                                          ScResId(STR_INVALID_TABREF)));
                xBox->run();
                pEdCopyArea->GrabFocus();
                bAreaInputOk = false;
            }
        }

        if ( bAreaInputOk )
        {
            SetDispatcherLock( false );
            SwitchToDocument();
            GetBindings().GetDispatcher()->ExecuteList(FID_FILTER_OK,
                                      SfxCallMode::SLOT | SfxCallMode::RECORD,
                                      { GetOutputItem() });
            Close();
        }
    }
    else if ( pBtn == pBtnCancel )
    {
        Close();
    }
}

IMPL_LINK_NOARG(ScFilterDlg, MoreExpandedHdl, VclExpander&, void)
{
    if ( pExpander->get_expanded() )
        pTimer->Start();
    else
    {
        pTimer->Stop();
        bRefInputMode = false;
        //@BugID 54702 Enable/disable only in Basic class
        //SFX_APPWINDOW->Disable(FALSE);        //! general method in ScAnyRefDlg
    }
}

IMPL_LINK( ScFilterDlg, TimeOutHdl, Timer*, _pTimer, void )
{
    // Check if RefInputMode is still true every 50ms

    if( _pTimer == pTimer.get() && IsActive() )
        bRefInputMode = (pEdCopyArea->HasFocus() || pRbCopyArea->HasFocus());

    if ( pExpander->get_expanded() )
        pTimer->Start();
}

IMPL_LINK( ScFilterDlg, LbSelectHdl, ListBox&, rLb, void )
{
    /*
     * Handle enable/disable logic depending on which ListBox was selected
     */
    sal_uInt16 nOffset = GetSliderPos();

    if ( &rLb == pLbConnect1 )
    {
        pLbField1->Enable();
        pLbCond1->Enable();
        pEdVal1->Enable();

        const sal_Int32 nConnect1 = pLbConnect1->GetSelectedEntryPos();
        size_t nQE = nOffset;
        theQueryData.GetEntry(nQE).eConnect =static_cast<ScQueryConnect>(nConnect1);
        if (maRefreshExceptQuery.size() < nQE + 1)
            maRefreshExceptQuery.resize(nQE + 1, false);
        maRefreshExceptQuery[nQE] = true;
    }

    else if ( &rLb == pLbConnect2 )
    {
        pLbField2->Enable();
        pLbCond2->Enable();
        pEdVal2->Enable();

        const sal_Int32 nConnect2 = pLbConnect2->GetSelectedEntryPos();
        size_t nQE = 1+nOffset;
        theQueryData.GetEntry(nQE).eConnect =static_cast<ScQueryConnect>(nConnect2);
        if (maRefreshExceptQuery.size() < nQE + 1)
            maRefreshExceptQuery.resize(nQE + 1, false);
        maRefreshExceptQuery[nQE]=true;
    }
    else if ( &rLb == pLbConnect3 )
    {
        pLbField3->Enable();
        pLbCond3->Enable();
        pEdVal3->Enable();

        const sal_Int32 nConnect3 = pLbConnect3->GetSelectedEntryPos();
        size_t nQE = 2 + nOffset;
        theQueryData.GetEntry(nQE).eConnect = static_cast<ScQueryConnect>(nConnect3);
        if (maRefreshExceptQuery.size() < nQE + 1)
            maRefreshExceptQuery.resize(nQE + 1, false);
        maRefreshExceptQuery[nQE] = true;

    }
    else if ( &rLb == pLbConnect4 )
    {
        pLbField4->Enable();
        pLbCond4->Enable();
        pEdVal4->Enable();

        const sal_Int32 nConnect4 = pLbConnect4->GetSelectedEntryPos();
        size_t nQE = 3 + nOffset;
        theQueryData.GetEntry(nQE).eConnect = static_cast<ScQueryConnect>(nConnect4);
        if (maRefreshExceptQuery.size() < nQE + 1)
            maRefreshExceptQuery.resize(nQE + 1, false);
        maRefreshExceptQuery[nQE] = true;
    }
    else if ( &rLb == pLbField1 )
    {
        if ( pLbField1->GetSelectedEntryPos() == 0 )
        {
            pLbConnect2->SetNoSelection();
            pLbConnect3->SetNoSelection();
            pLbConnect4->SetNoSelection();
            pLbField2->SelectEntryPos( 0 );
            pLbField3->SelectEntryPos( 0 );
            pLbField4->SelectEntryPos( 0 );
            pLbCond2->SelectEntryPos( 0 );
            pLbCond3->SelectEntryPos( 0 );
            pLbCond4->SelectEntryPos( 0 );
            ClearValueList( 1 );
            ClearValueList( 2 );
            ClearValueList( 3 );
            ClearValueList( 4 );

            pLbConnect2->Disable();
            pLbConnect3->Disable();
            pLbConnect4->Disable();
            pLbField2->Disable();
            pLbField3->Disable();
            pLbField4->Disable();
            pLbCond2->Disable();
            pLbCond3->Disable();
            pLbCond4->Disable();
            pEdVal2->Disable();
            pEdVal3->Disable();
            pEdVal4->Disable();
            SCSIZE nCount = theQueryData.GetEntryCount();
            if (maRefreshExceptQuery.size() < nCount + 1)
                maRefreshExceptQuery.resize(nCount + 1, false);
            for (SCSIZE i = nOffset; i < nCount; ++i)
            {
                theQueryData.GetEntry(i).bDoQuery = false;
                maRefreshExceptQuery[i] = false;
                theQueryData.GetEntry(i).nField =  static_cast<SCCOL>(0);
            }
            maRefreshExceptQuery[nOffset] = true;
        }
        else
        {
            UpdateValueList( 1 );
            if ( !pLbConnect2->IsEnabled() )
            {
                pLbConnect2->Enable();
            }
            theQueryData.GetEntry(nOffset).bDoQuery = true;
            const sal_Int32 nField  = rLb.GetSelectedEntryPos();
            theQueryData.GetEntry(nOffset).nField = theQueryData.nCol1 + static_cast<SCCOL>(nField) - 1 ;
        }
    }
    else if ( &rLb == pLbField2 )
    {
        if ( pLbField2->GetSelectedEntryPos() == 0 )
        {
            pLbConnect3->SetNoSelection();
            pLbConnect4->SetNoSelection();
            pLbField3->SelectEntryPos( 0 );
            pLbField4->SelectEntryPos( 0 );
            pLbCond3->SelectEntryPos( 0 );
            pLbCond4->SelectEntryPos( 0 );
            ClearValueList( 2 );
            ClearValueList( 3 );
            ClearValueList( 4 );

            pLbConnect3->Disable();
            pLbConnect4->Disable();
            pLbField3->Disable();
            pLbField4->Disable();
            pLbCond3->Disable();
            pLbCond4->Disable();
            pEdVal3->Disable();
            pEdVal4->Disable();

            sal_uInt16 nTemp=nOffset+1;
            SCSIZE nCount = theQueryData.GetEntryCount();
            if (maRefreshExceptQuery.size() < nCount)
                maRefreshExceptQuery.resize(nCount, false);
            for (SCSIZE i= nTemp; i< nCount; i++)
            {
                theQueryData.GetEntry(i).bDoQuery = false;
                maRefreshExceptQuery[i] = false;
                theQueryData.GetEntry(i).nField =  static_cast<SCCOL>(0);
            }
            maRefreshExceptQuery[nTemp] = true;
        }
        else
        {
            UpdateValueList( 2 );
            if ( !pLbConnect3->IsEnabled() )
            {
                pLbConnect3->Enable();
            }
            const sal_Int32 nField = rLb.GetSelectedEntryPos();
            sal_uInt16 nQ=1+nOffset;
            theQueryData.GetEntry(nQ).bDoQuery = true;
            theQueryData.GetEntry(nQ).nField = theQueryData.nCol1 + static_cast<SCCOL>(nField) - 1 ;
        }
    }
    else if ( &rLb == pLbField3 )
    {
        if ( pLbField3->GetSelectedEntryPos() == 0 )
        {
            pLbConnect4->SetNoSelection();
            pLbField4->SelectEntryPos( 0 );
            pLbCond4->SelectEntryPos( 0 );
            ClearValueList( 3 );
            ClearValueList( 4 );

            pLbConnect4->Disable();
            pLbField4->Disable();
            pLbCond4->Disable();
            pEdVal4->Disable();

            sal_uInt16 nTemp=nOffset+2;
            SCSIZE nCount = theQueryData.GetEntryCount();
            if (maRefreshExceptQuery.size() < nCount)
                maRefreshExceptQuery.resize(nCount, false);
            for (SCSIZE i = nTemp; i < nCount; ++i)
            {
                theQueryData.GetEntry(i).bDoQuery = false;
                maRefreshExceptQuery[i] = false;
                theQueryData.GetEntry(i).nField =  static_cast<SCCOL>(0);
            }
            maRefreshExceptQuery[nTemp] = true;
        }
        else
        {
            UpdateValueList( 3 );
            if ( !pLbConnect4->IsEnabled() )
            {
                pLbConnect4->Enable();
            }

            const sal_Int32 nField = rLb.GetSelectedEntryPos();
            sal_uInt16 nQ=2+nOffset;
            theQueryData.GetEntry(nQ).bDoQuery = true;
            theQueryData.GetEntry(nQ).nField = theQueryData.nCol1 + static_cast<SCCOL>(nField) - 1 ;

        }
    }
    else if ( &rLb == pLbField4 )
    {
        if ( pLbField4->GetSelectedEntryPos() == 0 )
        {
            ClearValueList( 4 );
            sal_uInt16 nTemp=nOffset+3;
            SCSIZE nCount = theQueryData.GetEntryCount();
            if (maRefreshExceptQuery.size() < nCount)
                maRefreshExceptQuery.resize(nCount, false);
            for (SCSIZE i = nTemp; i < nCount; ++i)
            {
                theQueryData.GetEntry(i).bDoQuery = false;
                maRefreshExceptQuery[i] = false;
                theQueryData.GetEntry(i).nField =  static_cast<SCCOL>(0);
            }
            maRefreshExceptQuery[nTemp] = true;
        }
        else
        {
            UpdateValueList( 4 );
            const sal_Int32 nField = rLb.GetSelectedEntryPos();
            sal_uInt16 nQ=3+nOffset;
            theQueryData.GetEntry(nQ).bDoQuery = true;
            theQueryData.GetEntry(nQ).nField = theQueryData.nCol1 + static_cast<SCCOL>(nField) - 1 ;
        }

    }
    else if ( &rLb == pLbCond1)
    {
        theQueryData.GetEntry(nOffset).eOp=static_cast<ScQueryOp>(rLb.GetSelectedEntryPos());
    }
    else if ( &rLb == pLbCond2)
    {
        sal_uInt16 nQ=1+nOffset;
        theQueryData.GetEntry(nQ).eOp=static_cast<ScQueryOp>(rLb.GetSelectedEntryPos());
    }
    else if ( &rLb == pLbCond3)
    {
        sal_uInt16 nQ=2+nOffset;
        theQueryData.GetEntry(nQ).eOp=static_cast<ScQueryOp>(rLb.GetSelectedEntryPos());
    }
    else
    {
        sal_uInt16 nQ=3+nOffset;
        theQueryData.GetEntry(nQ).eOp=static_cast<ScQueryOp>(rLb.GetSelectedEntryPos());
    }
}

IMPL_LINK( ScFilterDlg, CheckBoxHdl, Button*, pBox, void )
{
    //  Column headers:
    //      Field list: Columnxx <-> column header string
    //      Value list: Column header value not applicable.
    //  Upper/lower case:
    //      Value list: completely new

    if ( pBox == pBtnHeader )              // Field list and value list
    {
        const sal_Int32 nCurSel1 = pLbField1->GetSelectedEntryPos();
        const sal_Int32 nCurSel2 = pLbField2->GetSelectedEntryPos();
        const sal_Int32 nCurSel3 = pLbField3->GetSelectedEntryPos();
        const sal_Int32 nCurSel4 = pLbField4->GetSelectedEntryPos();
        FillFieldLists();
        pLbField1->SelectEntryPos( nCurSel1 );
        pLbField2->SelectEntryPos( nCurSel2 );
        pLbField3->SelectEntryPos( nCurSel3 );
        pLbField4->SelectEntryPos( nCurSel4 );

        UpdateHdrInValueList( 1 );
        UpdateHdrInValueList( 2 );
        UpdateHdrInValueList( 3 );
        UpdateHdrInValueList( 4 );
    }

    if ( pBox == pBtnCase )            // Complete value list
    {
        m_EntryLists.clear();
        UpdateValueList( 1 );       // current text is recorded
        UpdateValueList( 2 );
        UpdateValueList( 3 );
        UpdateValueList( 4 );
    }
}

IMPL_LINK( ScFilterDlg, ValModifyHdl, Edit&, rEd, void )
{
    size_t nOffset = GetSliderPos();
    size_t i = 0;
    size_t nQE = i + nOffset;
    OUString aStrVal = rEd.GetText();
    ListBox*  pLbCond   = pLbCond1;
    ListBox*  pLbField  = pLbField1;
    if ( &rEd == pEdVal2 )
    {
        pLbCond  = pLbCond2;
        pLbField = pLbField2;
        i=1;
        nQE=i+nOffset;
    }
    if ( &rEd == pEdVal3 )
    {
        pLbCond = pLbCond3;
        pLbField = pLbField3;
        i=2;
        nQE=i+nOffset;
    }
    if ( &rEd == pEdVal4 )
    {
        pLbCond = pLbCond4;
        pLbField = pLbField4;
        i=3;
        nQE=i+nOffset;
    }

    if ( aStrEmpty == aStrVal || aStrNotEmpty == aStrVal )
    {
        pLbCond->SelectEntry(OUString('='));
        pLbCond->Disable();
    }
    else
        pLbCond->Enable();

    if (maHasDates.size() < nQE + 1)
        maHasDates.resize(nQE + 1, false);
    if (maRefreshExceptQuery.size() < nQE + 1)
        maRefreshExceptQuery.resize(nQE + 1, false);

    ScQueryEntry& rEntry = theQueryData.GetEntry( nQE );
    ScQueryEntry::Item& rItem = rEntry.GetQueryItem();
    bool bDoThis = (pLbField->GetSelectedEntryPos() != 0);
    rEntry.bDoQuery = bDoThis;

    if ( rEntry.bDoQuery || maRefreshExceptQuery[nQE] )
    {
        bool bByEmptyOrNotByEmpty = false;
        if ( aStrEmpty == aStrVal )
        {
            bByEmptyOrNotByEmpty = true;
            rEntry.SetQueryByEmpty();
        }
        else if ( aStrNotEmpty == aStrVal )
        {
            bByEmptyOrNotByEmpty = true;
            rEntry.SetQueryByNonEmpty();
        }
        else
        {
            rItem.maString = pDoc->GetSharedStringPool().intern(aStrVal);
            rItem.mfVal = 0.0;

            sal_uInt32 nIndex = 0;
            bool bNumber = pDoc->GetFormatTable()->IsNumberFormat(
                rItem.maString.getString(), nIndex, rItem.mfVal);
            rItem.meType = bNumber ? ScQueryEntry::ByValue : ScQueryEntry::ByString;
        }

        const sal_Int32 nField = pLbField->GetSelectedEntryPos();
        rEntry.nField = nField ? (theQueryData.nCol1 +
            static_cast<SCCOL>(nField) - 1) : static_cast<SCCOL>(0);

        ScQueryOp eOp  = static_cast<ScQueryOp>(pLbCond->GetSelectedEntryPos());
        rEntry.eOp     = eOp;
        if (maHasDates[nQE] && !bByEmptyOrNotByEmpty)
            rItem.meType = ScQueryEntry::ByDate;
    }
}

IMPL_LINK_NOARG(ScFilterDlg, ScrollHdl, ScrollBar*, void)
{
    SliderMoved();
}

void ScFilterDlg::SliderMoved()
{
    size_t nOffset = GetSliderPos();
    RefreshEditRow( nOffset);
}

size_t ScFilterDlg::GetSliderPos()
{
    return static_cast<size_t>(pScrollBar->GetThumbPos());
}

void ScFilterDlg::RefreshEditRow( size_t nOffset )
{
    if (nOffset==0)
        maConnLbArr[0]->Hide();
    else
        maConnLbArr[0]->Show();

    for (size_t i = 0; i < QUERY_ENTRY_COUNT; ++i)
    {
        OUString aValStr;
        size_t nCondPos = 0;
        size_t nFieldSelPos = 0;
        size_t nQE = i + nOffset;

        if (maRefreshExceptQuery.size() < nQE + 1)
            maRefreshExceptQuery.resize(nQE + 1, false);

        ScQueryEntry& rEntry = theQueryData.GetEntry( nQE);
        if ( rEntry.bDoQuery || maRefreshExceptQuery[nQE] )
        {
            nCondPos = static_cast<size_t>(rEntry.eOp);
            if(rEntry.bDoQuery)
               nFieldSelPos = GetFieldSelPos( static_cast<SCCOL>(rEntry.nField) );

            const ScQueryEntry::Item& rItem = rEntry.GetQueryItem();
            OUString aQueryStr = rItem.maString.getString();
            if (rEntry.IsQueryByEmpty())
            {
                aValStr = aStrEmpty;
                maCondLbArr[i]->Disable();
            }
            else if (rEntry.IsQueryByNonEmpty())
            {
                aValStr = aStrNotEmpty;
                maCondLbArr[i]->Disable();
            }
            else
            {
                aValStr = aQueryStr;
                maCondLbArr[i]->Enable();
            }
            maFieldLbArr[i]->Enable();
            maValueEdArr[i]->Enable();

            if (nOffset==0)
            {
                if (i<3)
                {
                    if(rEntry.bDoQuery)
                        maConnLbArr[i+1]->Enable();
                    else
                        maConnLbArr[i+1]->Disable();
                    size_t nQENext = nQE + 1;
                    if (maRefreshExceptQuery.size() < nQENext + 1)
                        maRefreshExceptQuery.resize(nQENext + 1, false);
                    if (theQueryData.GetEntry(nQENext).bDoQuery || maRefreshExceptQuery[nQENext])
                        maConnLbArr[i+1]->SelectEntryPos( static_cast<sal_uInt16>(theQueryData.GetEntry(nQENext).eConnect) );
                    else
                        maConnLbArr[i+1]->SetNoSelection();
                }
            }
            else
            {
                if(theQueryData.GetEntry( nQE-1).bDoQuery)
                    maConnLbArr[i]->Enable();
                else
                    maConnLbArr[i]->Disable();

                if (maRefreshExceptQuery.size() < nQE + 1)
                    maRefreshExceptQuery.resize(nQE + 1, false);
                if(rEntry.bDoQuery || maRefreshExceptQuery[nQE])
                    maConnLbArr[i]->SelectEntryPos( static_cast<sal_uInt16>(rEntry.eConnect) );
                else
                    maConnLbArr[i]->SetNoSelection();
            }

        }
        else
        {
            if (nOffset==0)
            {
                if(i<3)
                {
                    maConnLbArr[i+1]->SetNoSelection();
                    maConnLbArr[i+1]->Disable();
                }
            }
            else
            {
                if(theQueryData.GetEntry( nQE-1).bDoQuery)
                    maConnLbArr[i]->Enable();
                else
                    maConnLbArr[i]->Disable();
                maConnLbArr[i]->SetNoSelection();
            }
            maFieldLbArr[i]->Disable();
            maCondLbArr[i]->Disable();
            maValueEdArr[i]->Disable();
        }
        maFieldLbArr[i]->SelectEntryPos( nFieldSelPos );
        maCondLbArr [i]->SelectEntryPos( nCondPos );
        maValueEdArr[i]->SetText( aValStr );
        UpdateValueList(i+1);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
