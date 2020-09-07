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
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <svl/sharedstringpool.hxx>

#include <limits>

#define QUERY_ENTRY_COUNT 4
#define INVALID_HEADER_POS std::numeric_limits<size_t>::max()

ScFilterDlg::EntryList::EntryList() :
    mnHeaderPos(INVALID_HEADER_POS) {}

ScFilterDlg::ScFilterDlg(SfxBindings* pB, SfxChildWindow* pCW, weld::Window* pParent,
    const SfxItemSet& rArgSet)
    : ScAnyRefDlgController(pB, pCW, pParent,
        "modules/scalc/ui/standardfilterdialog.ui", "StandardFilterDialog")
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
    , m_xLbConnect1(m_xBuilder->weld_combo_box("connect1"))
    , m_xLbField1(m_xBuilder->weld_combo_box("field1"))
    , m_xLbCond1(m_xBuilder->weld_combo_box("cond1"))
    , m_xEdVal1(m_xBuilder->weld_combo_box("val1"))
    , m_xLbConnect2(m_xBuilder->weld_combo_box("connect2"))
    , m_xLbField2(m_xBuilder->weld_combo_box("field2"))
    , m_xLbCond2(m_xBuilder->weld_combo_box("cond2"))
    , m_xEdVal2(m_xBuilder->weld_combo_box("val2"))
    , m_xLbConnect3(m_xBuilder->weld_combo_box("connect3"))
    , m_xLbField3(m_xBuilder->weld_combo_box("field3"))
    , m_xLbCond3(m_xBuilder->weld_combo_box("cond3"))
    , m_xEdVal3(m_xBuilder->weld_combo_box("val3"))
    , m_xLbConnect4(m_xBuilder->weld_combo_box("connect4"))
    , m_xLbField4(m_xBuilder->weld_combo_box("field4"))
    , m_xLbCond4(m_xBuilder->weld_combo_box("cond4"))
    , m_xEdVal4(m_xBuilder->weld_combo_box("val4"))
    , m_xContents(m_xBuilder->weld_widget("grid"))
    , m_xScrollBar(m_xBuilder->weld_scrolled_window("scrollbar"))
    , m_xExpander(m_xBuilder->weld_expander("more"))
    , m_xBtnClear(m_xBuilder->weld_button("clear"))
    , m_xBtnOk(m_xBuilder->weld_button("ok"))
    , m_xBtnCancel(m_xBuilder->weld_button("cancel"))
    , m_xBtnCase(m_xBuilder->weld_check_button("case"))
    , m_xBtnRegExp(m_xBuilder->weld_check_button("regexp"))
    , m_xBtnHeader(m_xBuilder->weld_check_button("header"))
    , m_xBtnUnique(m_xBuilder->weld_check_button("unique"))
    , m_xBtnCopyResult(m_xBuilder->weld_check_button("copyresult"))
    , m_xLbCopyArea(m_xBuilder->weld_combo_box("lbcopyarea"))
    , m_xEdCopyArea(new formula::RefEdit(m_xBuilder->weld_entry("edcopyarea")))
    , m_xRbCopyArea(new formula::RefButton(m_xBuilder->weld_button("rbcopyarea")))
    , m_xBtnDestPers(m_xBuilder->weld_check_button("destpers"))
    , m_xFtDbAreaLabel(m_xBuilder->weld_label("dbarealabel"))
    , m_xFtDbArea(m_xBuilder->weld_label("dbarea"))
{
    m_xExpander->connect_expanded(LINK(this, ScFilterDlg, MoreExpandedHdl));
    m_xEdCopyArea->SetReferences(this, m_xFtDbAreaLabel.get());
    m_xRbCopyArea->SetReferences(this, m_xEdCopyArea.get());

    Init( rArgSet );

    // Hack: RefInput control
    pTimer.reset( new Timer("ScFilterTimer") );
    pTimer->SetTimeout( 50 ); // Wait 50ms
    pTimer->SetInvokeHandler( LINK( this, ScFilterDlg, TimeOutHdl ) );
}

ScFilterDlg::~ScFilterDlg()
{
    pOptionsMgr.reset();
    pOutItem.reset();

    // Hack: RefInput control
    pTimer->Stop();
    pTimer.reset();
}

void ScFilterDlg::Init( const SfxItemSet& rArgSet )
{
    const ScQueryItem& rQueryItem = static_cast<const ScQueryItem&>(
                                    rArgSet.Get( nWhichQuery ));

    m_xBtnClear->connect_clicked   ( LINK( this, ScFilterDlg, BtnClearHdl ) );
    m_xBtnOk->connect_clicked      ( LINK( this, ScFilterDlg, EndDlgHdl ) );
    m_xBtnCancel->connect_clicked  ( LINK( this, ScFilterDlg, EndDlgHdl ) );
    m_xBtnHeader->connect_clicked  ( LINK( this, ScFilterDlg, CheckBoxHdl ) );
    m_xBtnCase->connect_clicked    ( LINK( this, ScFilterDlg, CheckBoxHdl ) );

    m_xLbField1->connect_changed  ( LINK( this, ScFilterDlg, LbSelectHdl ) );
    m_xLbField2->connect_changed  ( LINK( this, ScFilterDlg, LbSelectHdl ) );
    m_xLbField3->connect_changed  ( LINK( this, ScFilterDlg, LbSelectHdl ) );
    m_xLbField4->connect_changed  ( LINK( this, ScFilterDlg, LbSelectHdl ) );
    m_xLbConnect1->connect_changed( LINK( this, ScFilterDlg, LbSelectHdl ) );
    m_xLbConnect2->connect_changed( LINK( this, ScFilterDlg, LbSelectHdl ) );
    m_xLbConnect3->connect_changed( LINK( this, ScFilterDlg, LbSelectHdl ) );
    m_xLbConnect4->connect_changed( LINK( this, ScFilterDlg, LbSelectHdl ) );

    m_xLbField1->append_text("0000000000");
    m_xLbField1->set_active(0);
    auto nPrefWidth = m_xLbField1->get_preferred_size().Width();
    m_xLbField1->clear();

    m_xLbField1->set_size_request(nPrefWidth, -1);
    m_xLbField2->set_size_request(nPrefWidth, -1);
    m_xLbField3->set_size_request(nPrefWidth, -1);
    m_xLbField4->set_size_request(nPrefWidth, -1);

    m_xLbCond1->connect_changed( LINK( this, ScFilterDlg, LbSelectHdl ) );
    m_xLbCond2->connect_changed( LINK( this, ScFilterDlg, LbSelectHdl ) );
    m_xLbCond3->connect_changed( LINK( this, ScFilterDlg, LbSelectHdl ) );
    m_xLbCond4->connect_changed( LINK( this, ScFilterDlg, LbSelectHdl ) );

    pViewData   = rQueryItem.GetViewData();
    pDoc        = pViewData ? pViewData->GetDocument() : nullptr;
    nSrcTab     = pViewData ? pViewData->GetTabNo() : static_cast<SCTAB>(0);

    // for easier access:
    maFieldLbArr.reserve(QUERY_ENTRY_COUNT);
    maFieldLbArr.push_back(m_xLbField1.get());
    maFieldLbArr.push_back(m_xLbField2.get());
    maFieldLbArr.push_back(m_xLbField3.get());
    maFieldLbArr.push_back(m_xLbField4.get());
    maValueEdArr.reserve(QUERY_ENTRY_COUNT);
    maValueEdArr.push_back(m_xEdVal1.get());
    maValueEdArr.push_back(m_xEdVal2.get());
    maValueEdArr.push_back(m_xEdVal3.get());
    maValueEdArr.push_back(m_xEdVal4.get());
    maCondLbArr.reserve(QUERY_ENTRY_COUNT);
    maCondLbArr.push_back(m_xLbCond1.get());
    maCondLbArr.push_back(m_xLbCond2.get());
    maCondLbArr.push_back(m_xLbCond3.get());
    maCondLbArr.push_back(m_xLbCond4.get());
    maConnLbArr.reserve(QUERY_ENTRY_COUNT);
    maConnLbArr.push_back(m_xLbConnect1.get());
    maConnLbArr.push_back(m_xLbConnect2.get());
    maConnLbArr.push_back(m_xLbConnect3.get());
    maConnLbArr.push_back(m_xLbConnect4.get());

    // Option initialization:
    pOptionsMgr.reset( new ScFilterOptionsMgr(
                            pViewData,
                            theQueryData,
                            m_xBtnCase.get(),
                            m_xBtnRegExp.get(),
                            m_xBtnHeader.get(),
                            m_xBtnUnique.get(),
                            m_xBtnCopyResult.get(),
                            m_xBtnDestPers.get(),
                            m_xLbCopyArea.get(),
                            m_xEdCopyArea.get(),
                            m_xRbCopyArea.get(),
                            m_xFtDbAreaLabel.get(),
                            m_xFtDbArea.get(),
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
                maCondLbArr[i]->set_sensitive(false);
            }
            else if (rEntry.IsQueryByNonEmpty())
            {
                aValStr = aStrNotEmpty;
                maCondLbArr[i]->set_sensitive(false);
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
        maFieldLbArr[i]->set_active( nFieldSelPos );
        maCondLbArr [i]->set_active( nCondPos );
        maValueEdArr[i]->set_entry_text( aValStr );
        maValueEdArr[i]->set_entry_completion(false);
        maValueEdArr[i]->connect_changed( LINK( this, ScFilterDlg, ValModifyHdl ) );
        UpdateValueList(i+1);
    }

    m_xScrollBar->set_user_managed_scrolling();
    m_xScrollBar->connect_vadjustment_changed( LINK( this, ScFilterDlg, ScrollHdl ) );
    m_xScrollBar->vadjustment_configure(0, 0, 8, 1, 3, 4);
    Size aSize(m_xContents->get_preferred_size());
    m_xContents->set_size_request(aSize.Width(), aSize.Height());

    m_xLbConnect1->hide();
    // Disable/Enable Logic:

    (m_xLbField1->get_active() != 0)
    && (m_xLbField2->get_active() != 0)
        ? m_xLbConnect2->set_active( static_cast<sal_uInt16>(theQueryData.GetEntry(1).eConnect) )
        : m_xLbConnect2->set_active(-1);

    (m_xLbField2->get_active() != 0)
    && (m_xLbField3->get_active() != 0)
        ? m_xLbConnect3->set_active( static_cast<sal_uInt16>(theQueryData.GetEntry(2).eConnect) )
        : m_xLbConnect3->set_active(-1);

    (m_xLbField3->get_active() != 0)
    && (m_xLbField4->get_active() != 0)
        ? m_xLbConnect4->set_active( static_cast<sal_uInt16>(theQueryData.GetEntry(3).eConnect) )
        : m_xLbConnect4->set_active(-1);
    if ( m_xLbField1->get_active() == 0 )
    {
        m_xLbConnect2->set_sensitive(false);
        m_xLbField2->set_sensitive(false);
        m_xLbCond2->set_sensitive(false);
        m_xEdVal2->set_sensitive(false);
    }
    else if ( m_xLbConnect2->get_active() == -1 )
    {
        m_xLbField2->set_sensitive(false);
        m_xLbCond2->set_sensitive(false);
        m_xEdVal2->set_sensitive(false);
    }

    if ( m_xLbField2->get_active() == 0 )
    {
        m_xLbConnect3->set_sensitive(false);
        m_xLbField3->set_sensitive(false);
        m_xLbCond3->set_sensitive(false);
        m_xEdVal3->set_sensitive(false);
    }
    else if ( m_xLbConnect3->get_active() == -1 )
    {
        m_xLbField3->set_sensitive(false);
        m_xLbCond3->set_sensitive(false);
        m_xEdVal3->set_sensitive(false);
    }
    if ( m_xLbField3->get_active() == 0 )
    {
        m_xLbConnect4->set_sensitive(false);
        m_xLbField4->set_sensitive(false);
        m_xLbCond4->set_sensitive(false);
        m_xEdVal4->set_sensitive(false);
    }
    else if ( m_xLbConnect4->get_active() == -1 )
    {
        m_xLbField4->set_sensitive(false);
        m_xLbCond4->set_sensitive(false);
        m_xEdVal4->set_sensitive(false);
    }

    m_xEdVal1->set_entry_width_chars(10);
    m_xEdVal2->set_entry_width_chars(10);
    m_xEdVal3->set_entry_width_chars(10);
    m_xEdVal4->set_entry_width_chars(10);

    if (pDoc != nullptr && pDoc->GetChangeTrack() != nullptr)
        m_xBtnCopyResult->set_sensitive(false);
}

void ScFilterDlg::Close()
{
    if (pViewData)
        pViewData->GetDocShell()->CancelAutoDBRange();

    DoClose( ScFilterDlgWrapper::GetChildWindowId() );
}

// Mouse-selected cell area becomes the new selection and is shown in the
// reference text box

void ScFilterDlg::SetReference( const ScRange& rRef, ScDocument& rDocP )
{
    if ( bRefInputMode )    // Only possible if in reference edit mode
    {
        if ( rRef.aStart != rRef.aEnd )
            RefInputStart( m_xEdCopyArea.get() );
        OUString aRefStr(rRef.aStart.Format(ScRefFlags::ADDR_ABS_3D, &rDocP, rDocP.GetAddressConvention()));
        m_xEdCopyArea->SetRefString( aRefStr );
    }
}

void ScFilterDlg::SetActive()
{
    if ( bRefInputMode )
    {
        m_xEdCopyArea->GrabFocus();
        m_xEdCopyArea->GetModifyHdl().Call( *m_xEdCopyArea );
    }
    else
        m_xDialog->grab_focus();

    RefInputDone();
}

void ScFilterDlg::FillFieldLists()
{
    m_xLbField1->freeze();
    m_xLbField2->freeze();
    m_xLbField3->freeze();
    m_xLbField4->freeze();

    m_xLbField1->clear();
    m_xLbField2->clear();
    m_xLbField3->clear();
    m_xLbField4->clear();
    m_xLbField1->append_text( aStrNone );
    m_xLbField2->append_text( aStrNone );
    m_xLbField3->append_text( aStrNone );
    m_xLbField4->append_text( aStrNone );

    if ( pDoc )
    {
        OUString aFieldName;
        SCTAB   nTab        = nSrcTab;
        SCCOL   nFirstCol   = theQueryData.nCol1;
        SCROW   nFirstRow   = theQueryData.nRow1;
        SCCOL   nMaxCol     = theQueryData.nCol2;
        SCCOL   col = 0;

        for ( col=nFirstCol; col<=nMaxCol; col++ )
        {
            aFieldName = pDoc->GetString(col, nFirstRow, nTab);
            if (!m_xBtnHeader->get_active() || aFieldName.isEmpty())
            {
                aFieldName = ScGlobal::ReplaceOrAppend( aStrColumn, "%1", ScColToAlpha( col ));
            }
            m_xLbField1->append_text( aFieldName );
            m_xLbField2->append_text( aFieldName );
            m_xLbField3->append_text( aFieldName );
            m_xLbField4->append_text( aFieldName );
        }
    }

    m_xLbField4->thaw();
    m_xLbField3->thaw();
    m_xLbField2->thaw();
    m_xLbField1->thaw();
}

void ScFilterDlg::UpdateValueList( size_t nList )
{
    bool bCaseSens = m_xBtnCase->get_active();

    if (pDoc && nList > 0 && nList <= QUERY_ENTRY_COUNT)
    {
        weld::ComboBox* pValList = maValueEdArr[nList-1];
        const sal_Int32 nFieldSelPos = maFieldLbArr[nList-1]->get_active();
        OUString aCurValue = pValList->get_active_text();

        std::unique_ptr<weld::WaitObject> xWaiter;
        std::vector<weld::ComboBoxEntry> aEntries;
        aEntries.emplace_back(aStrNotEmpty);
        aEntries.emplace_back(aStrEmpty);

        if (nFieldSelPos)
        {
            xWaiter.reset(new weld::WaitObject(m_xDialog.get()));     // even if only the list box has content
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

            assert(pList);

            for (const auto& rEntry : pList->maFilterEntries)
                aEntries.emplace_back(rEntry.GetString());
        }
        pValList->insert_vector(aEntries, false);
        pValList->set_entry_text(aCurValue);
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

    size_t nFieldSelPos = maFieldLbArr[nList-1]->get_active();
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

    weld::ComboBox* pValList = maValueEdArr[nList-1];
    int nListPos = nPos + 2;                 // for "empty" and "non-empty"

    const ScTypedStrData& rHdrEntry = m_EntryLists[nColumn]->maFilterEntries.maStrData[nPos];

    const OUString& aHdrStr = rHdrEntry.GetString();
    bool bWasThere = nListPos < pValList->get_count() && aHdrStr == pValList->get_text(nListPos);
    bool bInclude = !m_xBtnHeader->get_active();

    if (bInclude)           // Include entry
    {
        if (!bWasThere)
            pValList->insert_text(nListPos, aHdrStr);
    }
    else                    // Omit entry
    {
        if (bWasThere)
            pValList->remove(nListPos);
    }
}

void ScFilterDlg::ClearValueList( size_t nList )
{
    if (nList > 0 && nList <= QUERY_ENTRY_COUNT)
    {
        weld::ComboBox* pValList = maValueEdArr[nList-1];
        pValList->clear();
        pValList->append_text( aStrNotEmpty );
        pValList->append_text( aStrEmpty );
        pValList->set_entry_text( EMPTY_OUSTRING );
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

    if ( m_xBtnCopyResult->get_active() )
    {
        ScRefFlags nResult = theCopyPos.Parse(
            m_xEdCopyArea->GetText(), pDoc, pDoc->GetAddressConvention());
        bCopyPosOk = (nResult & ScRefFlags::VALID) == ScRefFlags::VALID;
    }

    if ( m_xBtnCopyResult->get_active() && bCopyPosOk )
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

    theParam.bHasHeader     = m_xBtnHeader->get_active();
    theParam.bByRow         = true;
    theParam.bDuplicate     = !m_xBtnUnique->get_active();
    theParam.bCaseSens      = m_xBtnCase->get_active();
    theParam.eSearchType    = m_xBtnRegExp->get_active() ? utl::SearchParam::SearchType::Regexp : utl::SearchParam::SearchType::Normal;
    theParam.bDestPers      = m_xBtnDestPers->get_active();

    // only set the three - reset everything else

    pOutItem.reset( new ScQueryItem( nWhichQuery, &theParam ) );

    return pOutItem.get();
}

bool ScFilterDlg::IsRefInputMode() const
{
    return bRefInputMode;
}

// Handler:

IMPL_LINK( ScFilterDlg, BtnClearHdl, weld::Button&, rBtn, void )
{
    if ( &rBtn != m_xBtnClear.get() )
        return;

    // scroll to the top
    m_xScrollBar->vadjustment_set_value(0);
    size_t nOffset = 0;
    RefreshEditRow( nOffset);

    // clear all conditions
    m_xLbConnect1->set_active(-1);
    m_xLbConnect2->set_active(-1);
    m_xLbConnect3->set_active(-1);
    m_xLbConnect4->set_active(-1);
    m_xLbField1->set_active(0);
    m_xLbField2->set_active(0);
    m_xLbField3->set_active(0);
    m_xLbField4->set_active(0);
    m_xLbCond1->set_active(0);
    m_xLbCond2->set_active(0);
    m_xLbCond3->set_active(0);
    m_xLbCond4->set_active(0);
    ClearValueList( 1 );
    ClearValueList( 2 );
    ClearValueList( 3 );
    ClearValueList( 4 );

    // disable fields for second row onward
    m_xLbConnect2->set_sensitive(false);
    m_xLbConnect3->set_sensitive(false);
    m_xLbConnect4->set_sensitive(false);
    m_xLbField2->set_sensitive(false);
    m_xLbField3->set_sensitive(false);
    m_xLbField4->set_sensitive(false);
    m_xLbCond2->set_sensitive(false);
    m_xLbCond3->set_sensitive(false);
    m_xLbCond4->set_sensitive(false);
    m_xEdVal2->set_sensitive(false);
    m_xEdVal3->set_sensitive(false);
    m_xEdVal4->set_sensitive(false);

    // clear query data objects
    SCSIZE nCount = theQueryData.GetEntryCount();
    if (maRefreshExceptQuery.size() < nCount + 1)
        maRefreshExceptQuery.resize(nCount + 1, false);
    for (SCSIZE i = 0; i < nCount; ++i)
    {
        theQueryData.GetEntry(i).bDoQuery = false;
        maRefreshExceptQuery[i] = false;
        theQueryData.GetEntry(i).nField = static_cast<SCCOL>(0);
    }
    maRefreshExceptQuery[0] = true;
}

IMPL_LINK( ScFilterDlg, EndDlgHdl, weld::Button&, rBtn, void )
{
    if ( &rBtn == m_xBtnOk.get() )
    {
        bool bAreaInputOk = true;

        if ( m_xBtnCopyResult->get_active() )
        {
            if ( !pOptionsMgr->VerifyPosStr( m_xEdCopyArea->GetText() ) )
            {
                if (!m_xExpander->get_expanded())
                  m_xExpander->set_expanded(true);

                std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(m_xDialog.get(),
                                                          VclMessageType::Warning, VclButtonsType::Ok,
                                                          ScResId(STR_INVALID_TABREF)));
                xBox->run();
                m_xEdCopyArea->GrabFocus();
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
            response(RET_OK);
        }
    }
    else if ( &rBtn == m_xBtnCancel.get() )
    {
        response(RET_CANCEL);
    }
}

IMPL_LINK_NOARG(ScFilterDlg, MoreExpandedHdl, weld::Expander&, void)
{
    if ( m_xExpander->get_expanded() )
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
    if (_pTimer == pTimer.get() && m_xDialog->has_toplevel_focus())
        bRefInputMode = (m_xEdCopyArea->GetWidget()->has_focus() || m_xRbCopyArea->GetWidget()->has_focus());

    if ( m_xExpander->get_expanded() )
        pTimer->Start();
}

IMPL_LINK(ScFilterDlg, LbSelectHdl, weld::ComboBox&, rLb, void)
{
    /*
     * Handle enable/disable logic depending on which ListBox was selected
     */
    sal_uInt16 nOffset = GetSliderPos();

    if ( &rLb == m_xLbConnect1.get() )
    {
        m_xLbField1->set_sensitive(true);
        m_xLbCond1->set_sensitive(true);
        m_xEdVal1->set_sensitive(true);

        const sal_Int32 nConnect1 = m_xLbConnect1->get_active();
        size_t nQE = nOffset;
        theQueryData.GetEntry(nQE).eConnect =static_cast<ScQueryConnect>(nConnect1);
        if (maRefreshExceptQuery.size() < nQE + 1)
            maRefreshExceptQuery.resize(nQE + 1, false);
        maRefreshExceptQuery[nQE] = true;
    }
    else if ( &rLb == m_xLbConnect2.get() )
    {
        m_xLbField2->set_sensitive(true);
        m_xLbCond2->set_sensitive(true);
        m_xEdVal2->set_sensitive(true);

        const sal_Int32 nConnect2 = m_xLbConnect2->get_active();
        size_t nQE = 1+nOffset;
        theQueryData.GetEntry(nQE).eConnect =static_cast<ScQueryConnect>(nConnect2);
        if (maRefreshExceptQuery.size() < nQE + 1)
            maRefreshExceptQuery.resize(nQE + 1, false);
        maRefreshExceptQuery[nQE]=true;
    }
    else if ( &rLb == m_xLbConnect3.get() )
    {
        m_xLbField3->set_sensitive(true);
        m_xLbCond3->set_sensitive(true);
        m_xEdVal3->set_sensitive(true);

        const sal_Int32 nConnect3 = m_xLbConnect3->get_active();
        size_t nQE = 2 + nOffset;
        theQueryData.GetEntry(nQE).eConnect = static_cast<ScQueryConnect>(nConnect3);
        if (maRefreshExceptQuery.size() < nQE + 1)
            maRefreshExceptQuery.resize(nQE + 1, false);
        maRefreshExceptQuery[nQE] = true;

    }
    else if ( &rLb == m_xLbConnect4.get() )
    {
        m_xLbField4->set_sensitive(true);
        m_xLbCond4->set_sensitive(true);
        m_xEdVal4->set_sensitive(true);

        const sal_Int32 nConnect4 = m_xLbConnect4->get_active();
        size_t nQE = 3 + nOffset;
        theQueryData.GetEntry(nQE).eConnect = static_cast<ScQueryConnect>(nConnect4);
        if (maRefreshExceptQuery.size() < nQE + 1)
            maRefreshExceptQuery.resize(nQE + 1, false);
        maRefreshExceptQuery[nQE] = true;
    }
    else if ( &rLb == m_xLbField1.get() )
    {
        if ( m_xLbField1->get_active() == 0 )
        {
            m_xLbConnect2->set_active(-1);
            m_xLbConnect3->set_active(-1);
            m_xLbConnect4->set_active(-1);
            m_xLbField2->set_active( 0 );
            m_xLbField3->set_active( 0 );
            m_xLbField4->set_active( 0 );
            m_xLbCond2->set_active( 0 );
            m_xLbCond3->set_active( 0 );
            m_xLbCond4->set_active( 0 );
            ClearValueList( 1 );
            ClearValueList( 2 );
            ClearValueList( 3 );
            ClearValueList( 4 );

            m_xLbConnect2->set_sensitive(false);
            m_xLbConnect3->set_sensitive(false);
            m_xLbConnect4->set_sensitive(false);
            m_xLbField2->set_sensitive(false);
            m_xLbField3->set_sensitive(false);
            m_xLbField4->set_sensitive(false);
            m_xLbCond2->set_sensitive(false);
            m_xLbCond3->set_sensitive(false);
            m_xLbCond4->set_sensitive(false);
            m_xEdVal2->set_sensitive(false);
            m_xEdVal3->set_sensitive(false);
            m_xEdVal4->set_sensitive(false);
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
            if ( !m_xLbConnect2->get_sensitive() )
            {
                m_xLbConnect2->set_sensitive(true);
            }
            theQueryData.GetEntry(nOffset).bDoQuery = true;
            const sal_Int32 nField  = rLb.get_active();
            theQueryData.GetEntry(nOffset).nField = theQueryData.nCol1 + static_cast<SCCOL>(nField) - 1 ;
        }
    }
    else if ( &rLb == m_xLbField2.get() )
    {
        if ( m_xLbField2->get_active() == 0 )
        {
            m_xLbConnect3->set_active(-1);
            m_xLbConnect4->set_active(-1);
            m_xLbField3->set_active( 0 );
            m_xLbField4->set_active( 0 );
            m_xLbCond3->set_active( 0 );
            m_xLbCond4->set_active( 0 );
            ClearValueList( 2 );
            ClearValueList( 3 );
            ClearValueList( 4 );

            m_xLbConnect3->set_sensitive(false);
            m_xLbConnect4->set_sensitive(false);
            m_xLbField3->set_sensitive(false);
            m_xLbField4->set_sensitive(false);
            m_xLbCond3->set_sensitive(false);
            m_xLbCond4->set_sensitive(false);
            m_xEdVal3->set_sensitive(false);
            m_xEdVal4->set_sensitive(false);

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
            if ( !m_xLbConnect3->get_sensitive() )
            {
                m_xLbConnect3->set_sensitive(true);
            }
            const sal_Int32 nField = rLb.get_active();
            sal_uInt16 nQ=1+nOffset;
            theQueryData.GetEntry(nQ).bDoQuery = true;
            theQueryData.GetEntry(nQ).nField = theQueryData.nCol1 + static_cast<SCCOL>(nField) - 1 ;
        }
    }
    else if ( &rLb == m_xLbField3.get() )
    {
        if ( m_xLbField3->get_active() == 0 )
        {
            m_xLbConnect4->set_active(-1);
            m_xLbField4->set_active( 0 );
            m_xLbCond4->set_active( 0 );
            ClearValueList( 3 );
            ClearValueList( 4 );

            m_xLbConnect4->set_sensitive(false);
            m_xLbField4->set_sensitive(false);
            m_xLbCond4->set_sensitive(false);
            m_xEdVal4->set_sensitive(false);

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
            if ( !m_xLbConnect4->get_sensitive() )
            {
                m_xLbConnect4->set_sensitive(true);
            }

            const sal_Int32 nField = rLb.get_active();
            sal_uInt16 nQ=2+nOffset;
            theQueryData.GetEntry(nQ).bDoQuery = true;
            theQueryData.GetEntry(nQ).nField = theQueryData.nCol1 + static_cast<SCCOL>(nField) - 1 ;

        }
    }
    else if ( &rLb == m_xLbField4.get() )
    {
        if ( m_xLbField4->get_active() == 0 )
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
            const sal_Int32 nField = rLb.get_active();
            sal_uInt16 nQ=3+nOffset;
            theQueryData.GetEntry(nQ).bDoQuery = true;
            theQueryData.GetEntry(nQ).nField = theQueryData.nCol1 + static_cast<SCCOL>(nField) - 1 ;
        }

    }
    else if ( &rLb == m_xLbCond1.get())
    {
        theQueryData.GetEntry(nOffset).eOp=static_cast<ScQueryOp>(rLb.get_active());
    }
    else if ( &rLb == m_xLbCond2.get())
    {
        sal_uInt16 nQ=1+nOffset;
        theQueryData.GetEntry(nQ).eOp=static_cast<ScQueryOp>(rLb.get_active());
    }
    else if ( &rLb == m_xLbCond3.get())
    {
        sal_uInt16 nQ=2+nOffset;
        theQueryData.GetEntry(nQ).eOp=static_cast<ScQueryOp>(rLb.get_active());
    }
    else
    {
        sal_uInt16 nQ=3+nOffset;
        theQueryData.GetEntry(nQ).eOp=static_cast<ScQueryOp>(rLb.get_active());
    }
}

IMPL_LINK( ScFilterDlg, CheckBoxHdl, weld::Button&, rBox, void )
{
    //  Column headers:
    //      Field list: Columnxx <-> column header string
    //      Value list: Column header value not applicable.
    //  Upper/lower case:
    //      Value list: completely new

    if ( &rBox == m_xBtnHeader.get() )              // Field list and value list
    {
        const sal_Int32 nCurSel1 = m_xLbField1->get_active();
        const sal_Int32 nCurSel2 = m_xLbField2->get_active();
        const sal_Int32 nCurSel3 = m_xLbField3->get_active();
        const sal_Int32 nCurSel4 = m_xLbField4->get_active();
        FillFieldLists();
        m_xLbField1->set_active( nCurSel1 );
        m_xLbField2->set_active( nCurSel2 );
        m_xLbField3->set_active( nCurSel3 );
        m_xLbField4->set_active( nCurSel4 );

        UpdateHdrInValueList( 1 );
        UpdateHdrInValueList( 2 );
        UpdateHdrInValueList( 3 );
        UpdateHdrInValueList( 4 );
    }

    if ( &rBox == m_xBtnCase.get() )            // Complete value list
    {
        m_EntryLists.clear();
        UpdateValueList( 1 );       // current text is recorded
        UpdateValueList( 2 );
        UpdateValueList( 3 );
        UpdateValueList( 4 );
    }
}

IMPL_LINK( ScFilterDlg, ValModifyHdl, weld::ComboBox&, rEd, void )
{
    size_t nOffset = GetSliderPos();
    size_t i = 0;
    size_t nQE = i + nOffset;
    OUString aStrVal = rEd.get_active_text();
    weld::ComboBox*  pLbCond   = m_xLbCond1.get();
    weld::ComboBox*  pLbField  = m_xLbField1.get();
    if ( &rEd == m_xEdVal2.get() )
    {
        pLbCond  = m_xLbCond2.get();
        pLbField = m_xLbField2.get();
        i=1;
        nQE=i+nOffset;
    }
    if ( &rEd == m_xEdVal3.get() )
    {
        pLbCond = m_xLbCond3.get();
        pLbField = m_xLbField3.get();
        i=2;
        nQE=i+nOffset;
    }
    if ( &rEd == m_xEdVal4.get() )
    {
        pLbCond = m_xLbCond4.get();
        pLbField = m_xLbField4.get();
        i=3;
        nQE=i+nOffset;
    }

    if ( aStrEmpty == aStrVal || aStrNotEmpty == aStrVal )
    {
        pLbCond->set_active_text(OUString('='));
        pLbCond->set_sensitive(false);
    }
    else
        pLbCond->set_sensitive(true);

    if (maHasDates.size() < nQE + 1)
        maHasDates.resize(nQE + 1, false);
    if (maRefreshExceptQuery.size() < nQE + 1)
        maRefreshExceptQuery.resize(nQE + 1, false);

    ScQueryEntry& rEntry = theQueryData.GetEntry( nQE );
    ScQueryEntry::Item& rItem = rEntry.GetQueryItem();
    bool bDoThis = (pLbField->get_active() != 0);
    rEntry.bDoQuery = bDoThis;

    if ( !(rEntry.bDoQuery || maRefreshExceptQuery[nQE]) )
        return;

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

    const sal_Int32 nField = pLbField->get_active();
    rEntry.nField = nField ? (theQueryData.nCol1 +
        static_cast<SCCOL>(nField) - 1) : static_cast<SCCOL>(0);

    ScQueryOp eOp  = static_cast<ScQueryOp>(pLbCond->get_active());
    rEntry.eOp     = eOp;
    if (maHasDates[nQE] && !bByEmptyOrNotByEmpty)
        rItem.meType = ScQueryEntry::ByDate;
}

IMPL_LINK_NOARG(ScFilterDlg, ScrollHdl, weld::ScrolledWindow&, void)
{
    SliderMoved();
}

void ScFilterDlg::SliderMoved()
{
    size_t nOffset = GetSliderPos();
    RefreshEditRow( nOffset);
}

size_t ScFilterDlg::GetSliderPos() const
{
    return static_cast<size_t>(m_xScrollBar->vadjustment_get_value());
}

void ScFilterDlg::RefreshEditRow( size_t nOffset )
{
    if (nOffset==0)
        maConnLbArr[0]->hide();
    else
        maConnLbArr[0]->show();

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
                maCondLbArr[i]->set_sensitive(false);
            }
            else if (rEntry.IsQueryByNonEmpty())
            {
                aValStr = aStrNotEmpty;
                maCondLbArr[i]->set_sensitive(false);
            }
            else
            {
                aValStr = aQueryStr;
                maCondLbArr[i]->set_sensitive(true);
            }
            maFieldLbArr[i]->set_sensitive(true);
            maValueEdArr[i]->set_sensitive(true);

            if (nOffset==0)
            {
                if (i<3)
                {
                    if(rEntry.bDoQuery)
                        maConnLbArr[i+1]->set_sensitive(true);
                    else
                        maConnLbArr[i+1]->set_sensitive(false);
                    size_t nQENext = nQE + 1;
                    if (maRefreshExceptQuery.size() < nQENext + 1)
                        maRefreshExceptQuery.resize(nQENext + 1, false);
                    if (theQueryData.GetEntry(nQENext).bDoQuery || maRefreshExceptQuery[nQENext])
                        maConnLbArr[i+1]->set_active( static_cast<sal_uInt16>(theQueryData.GetEntry(nQENext).eConnect) );
                    else
                        maConnLbArr[i+1]->set_active(-1);
                }
            }
            else
            {
                if(theQueryData.GetEntry( nQE-1).bDoQuery)
                    maConnLbArr[i]->set_sensitive(true);
                else
                    maConnLbArr[i]->set_sensitive(false);

                if (maRefreshExceptQuery.size() < nQE + 1)
                    maRefreshExceptQuery.resize(nQE + 1, false);
                if(rEntry.bDoQuery || maRefreshExceptQuery[nQE])
                    maConnLbArr[i]->set_active( static_cast<sal_uInt16>(rEntry.eConnect) );
                else
                    maConnLbArr[i]->set_active(-1);
            }

        }
        else
        {
            if (nOffset==0)
            {
                if(i<3)
                {
                    maConnLbArr[i+1]->set_active(-1);
                    maConnLbArr[i+1]->set_sensitive(false);
                }
            }
            else
            {
                if(theQueryData.GetEntry( nQE-1).bDoQuery)
                    maConnLbArr[i]->set_sensitive(true);
                else
                    maConnLbArr[i]->set_sensitive(false);
                maConnLbArr[i]->set_active(-1);
            }
            maFieldLbArr[i]->set_sensitive(false);
            maCondLbArr[i]->set_sensitive(false);
            maValueEdArr[i]->set_sensitive(false);
        }
        maFieldLbArr[i]->set_active( nFieldSelPos );
        maCondLbArr [i]->set_active( nCondPos );
        maValueEdArr[i]->set_entry_text( aValStr );
        UpdateValueList(i+1);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
