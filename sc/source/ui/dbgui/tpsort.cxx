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

#undef SC_DLLIMPLEMENTATION

#include <vcl/weld.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <svtools/collatorres.hxx>
#include <unotools/collatorwrapper.hxx>
#include <comphelper/processfactory.hxx>

#include <scitems.hxx>
#include <uiitems.hxx>
#include <viewdata.hxx>
#include <document.hxx>
#include <global.hxx>
#include <dbdata.hxx>
#include <userlist.hxx>
#include <rangeutl.hxx>
#include <scresid.hxx>
#include <sc.hrc>
#include <strings.hrc>
#include <globstr.hrc>

#include <sortkeydlg.hxx>

#include <sortdlg.hxx>

#include <tpsort.hxx>

using namespace com::sun::star;

/*
 * Since the settings on the second Tab Page (Options) effects
 * the first Tab Page, there must be a way for it to communicate with the
 * other Page.
 *
 * At the moment this problem is solved through using two data members of the
 * Tab Pages. If a page is enabled / disabled, it compares this data member
 * with its own state (-> Activate() / Deactivate()).
 *
 * In the meantime the class SfxTabPage offers the following method:
 *
 * virtual sal_Bool HasExchangeSupport() const; -> return sal_True;
 * virtual void ActivatePage(const SfxItemSet &);
 * virtual int  DeactivatePage(SfxItemSet * = 0);
 *
 * This still needs to be changed!
 */

// Sort Criteria Tab page

ScTabPageSortFields::ScTabPageSortFields(TabPageParent pParent, const SfxItemSet& rArgSet)
    : SfxTabPage(pParent, "modules/scalc/ui/sortcriteriapage.ui", "SortCriteriaPage", &rArgSet)
    ,

        aStrUndefined   ( ScResId( SCSTR_UNDEFINED ) ),
        aStrColumn      ( ScResId( SCSTR_COLUMN ) ),
        aStrRow         ( ScResId( SCSTR_ROW ) ),

        nWhichSort      ( rArgSet.GetPool()->GetWhich( SID_SORT ) ),
        pViewData       ( nullptr ),
        aSortData       ( static_cast<const ScSortItem&>(
                           rArgSet.Get( nWhichSort )).
                                GetSortData() ),
        nFieldCount     ( 0 ),
        nSortKeyCount   ( DEFSORT ),
        bHasHeader      ( false ),
        bSortByRows     ( false )

    , m_xScrolledWindow(m_xBuilder->weld_scrolled_window("SortCriteriaPage"))
    , m_xBox(m_xBuilder->weld_container("SortKeyWindow"))
    , m_aSortWin(m_xBox.get())
{
    Init();

    m_aIdle.SetInvokeHandler(LINK(this, ScTabPageSortFields, ScrollToEndHdl));
    m_aIdle.SetDebugName("ScTabPageSortFields Scroll To End Idle");

    SetExchangeSupport();
}

ScTabPageSortFields::~ScTabPageSortFields()
{
    disposeOnce();
}

void ScTabPageSortFields::dispose()
{
    m_aSortWin.m_aSortKeyItems.clear();
    m_xBox.reset();
    m_xScrolledWindow.reset();
    SfxTabPage::dispose();
}

void ScTabPageSortFields::Init()
{
    const ScSortItem& rSortItem = static_cast<const ScSortItem&>(
                                  GetItemSet().Get( nWhichSort ));

    pViewData = rSortItem.GetViewData();
    OSL_ENSURE( pViewData, "ViewData not found!" );

    nFieldArr.push_back( 0 );

    // Create three sort key dialogs by default
    for ( sal_uInt16 i=0; i<nSortKeyCount; i++ )
    {
        AddSortKey(i+1);
        m_aSortWin.m_aSortKeyItems[i]->m_xLbSort->connect_changed(LINK(this, ScTabPageSortFields, SelectHdl));
    }
}

VclPtr<SfxTabPage> ScTabPageSortFields::Create(TabPageParent pParent, const SfxItemSet* pArgSet)
{
    return VclPtr<ScTabPageSortFields>::Create(pParent, *pArgSet);
}

void ScTabPageSortFields::Reset( const SfxItemSet* /* rArgSet */ )
{
    bSortByRows = aSortData.bByRow;
    bHasHeader  = aSortData.bHasHeader;

    if (m_aSortWin.m_aSortKeyItems[0]->m_xLbSort->get_count() == 0)
        FillFieldLists(0);

    // ListBox selection:
    if (!aSortData.maKeyState.empty() && aSortData.maKeyState[0].bDoSort)
    {
        // Make sure that the all sort keys are reset
        for ( sal_uInt16 i=nSortKeyCount; i<aSortData.GetSortKeyCount(); i++ )
        {
            AddSortKey(i+1);
            m_aSortWin.m_aSortKeyItems[i]->m_xLbSort->connect_changed( LINK( this,
                                 ScTabPageSortFields, SelectHdl ) );
        }
        nSortKeyCount = aSortData.GetSortKeyCount();
        FillFieldLists(0);

        for ( sal_uInt16 i=0; i<nSortKeyCount; i++ )
        {
            if (aSortData.maKeyState[i].bDoSort )
            {
                m_aSortWin.m_aSortKeyItems[i]->m_xLbSort->set_active( GetFieldSelPos(
                                    aSortData.maKeyState[i].nField ) );
                (aSortData.maKeyState[i].bAscending)
                    ? m_aSortWin.m_aSortKeyItems[i]->m_xBtnUp->set_active(true)
                    : m_aSortWin.m_aSortKeyItems[i]->m_xBtnDown->set_active(true);
            }
            else
            {
                m_aSortWin.m_aSortKeyItems[i]->m_xLbSort->set_active(0); // Select none
                m_aSortWin.m_aSortKeyItems[i]->m_xBtnUp->set_active(true);
            }
        }

        // Enable or disable field depending on preceding Listbox selection
        m_aSortWin.m_aSortKeyItems[0]->EnableField();
        for ( sal_uInt16 i=1; i<nSortKeyCount; i++ )
            if ( m_aSortWin.m_aSortKeyItems[i - 1]->m_xLbSort->get_active() == 0 )
                m_aSortWin.m_aSortKeyItems[i]->DisableField();
            else
                m_aSortWin.m_aSortKeyItems[i]->EnableField();
    }
    else
    {
        SCCOL  nCol = pViewData->GetCurX();

        if( nCol < aSortData.nCol1 )
            nCol = aSortData.nCol1;
        else if( nCol > aSortData.nCol2 )
            nCol = aSortData.nCol2;

        sal_uInt16  nSort1Pos = nCol - aSortData.nCol1+1;

        m_aSortWin.m_aSortKeyItems[0]->m_xLbSort->set_active(nSort1Pos);
        for ( sal_uInt16 i=1; i<nSortKeyCount; i++ )
            m_aSortWin.m_aSortKeyItems[i]->m_xLbSort->set_active(0);

        for ( sal_uInt16 i=0; i<nSortKeyCount; i++ )
            m_aSortWin.m_aSortKeyItems[i]->m_xBtnUp->set_active(true);

        m_aSortWin.m_aSortKeyItems[0]->EnableField();
        m_aSortWin.m_aSortKeyItems[1]->EnableField();
        for ( sal_uInt16 i=2; i<nSortKeyCount; i++ )
            m_aSortWin.m_aSortKeyItems[i]->DisableField();
    }

    if (ScSortDlg* pDlg = static_cast<ScSortDlg*>(GetDialogController()))
    {
        pDlg->SetByRows ( bSortByRows );
        pDlg->SetHeaders( bHasHeader );
    }

    // Make sure that there is always a last undefined sort key
    if (m_aSortWin.m_aSortKeyItems[nSortKeyCount - 1]->m_xLbSort->get_active() > 0)
        SetLastSortKey( nSortKeyCount );
}

bool ScTabPageSortFields::FillItemSet( SfxItemSet* rArgSet )
{
    ScSortParam aNewSortData = aSortData;

    const SfxItemSet* pExample = GetDialogExampleSet();
    if (pExample)
    {
        const SfxPoolItem* pItem;
        if (pExample->GetItemState(nWhichSort, true, &pItem) == SfxItemState::SET)
        {
            ScSortParam aTempData = static_cast<const ScSortItem*>(pItem)->GetSortData();
            aTempData.maKeyState = aNewSortData.maKeyState;
            aNewSortData = aTempData;
        }
    }
    std::vector<sal_Int32>  nSortPos;

    for ( sal_uInt16 i=0; i<nSortKeyCount; i++ )
    {
        nSortPos.push_back(m_aSortWin.m_aSortKeyItems[i]->m_xLbSort->get_active());
        if (nSortPos[i] == -1) nSortPos[i] = 0;
    }

    if( nSortKeyCount >= aNewSortData.GetSortKeyCount() )
        aNewSortData.maKeyState.resize(nSortKeyCount);

    if ( nSortPos[0] > 0 )
    {
        for ( sal_uInt16 i=0; i<nSortKeyCount; i++ )
            aNewSortData.maKeyState[i].bDoSort = (nSortPos[i] > 0);

        // If the "OK" was selected on the Options page while the sort
        // direction was changed, then the first field (i.e. nFieldArr[0])
        // of the respective direction is chosen as the sorting criterion:
        ScSortDlg* pDlg = static_cast<ScSortDlg*>(GetDialogController());
        if ( pDlg && bSortByRows != pDlg->GetByRows() )
        {
            for ( sal_uInt16 i=0; i<nSortKeyCount; i++ )
                aNewSortData.maKeyState[i].nField = 0;
        }
        else
        {
           for ( sal_uInt16 i=0; i<nSortKeyCount; i++ )
               aNewSortData.maKeyState[i].nField = nFieldArr[nSortPos[i]];
        }

        for ( sal_uInt16 i=0; i<nSortKeyCount; i++ )
            aNewSortData.maKeyState[i].bAscending = m_aSortWin.m_aSortKeyItems[i]->m_xBtnUp->get_active();

        // bHasHeader is in ScTabPageSortOptions::FillItemSet, where it belongs
    }
    else
    {
        for ( sal_uInt16 i=0; i<nSortKeyCount; i++ )
            aNewSortData.maKeyState[i].bDoSort = false;
    }

    rArgSet->Put( ScSortItem( SCITEM_SORTDATA, nullptr, &aNewSortData ) );

    return true;
}

// for data exchange without dialogue detour:
void ScTabPageSortFields::ActivatePage( const SfxItemSet& rSet )
{
    // Refresh local copy with shared data
    aSortData = static_cast<const ScSortItem&>(rSet.Get( SCITEM_SORTDATA )).GetSortData();
    if (ScSortDlg* pDlg = static_cast<ScSortDlg*>(GetDialogController()))
    {
        if ( bHasHeader  != pDlg->GetHeaders()
             || bSortByRows != pDlg->GetByRows() )
        {
            std::vector<sal_uInt16> nCurSel;
            for ( sal_uInt16 i=0; i<nSortKeyCount; i++ )
                nCurSel.push_back( m_aSortWin.m_aSortKeyItems[i]->m_xLbSort->get_active() );

            bHasHeader  = pDlg->GetHeaders();
            bSortByRows = pDlg->GetByRows();
            FillFieldLists(0);

            for ( sal_uInt16 i=0; i<nSortKeyCount; i++ )
                m_aSortWin.m_aSortKeyItems[i]->m_xLbSort->set_active(nCurSel[i]);
        }
    }
}

DeactivateRC ScTabPageSortFields::DeactivatePage( SfxItemSet* pSetP )
{
    if (ScSortDlg* pDlg = static_cast<ScSortDlg*>(GetDialogController()))
    {
        if ( bHasHeader != pDlg->GetHeaders() )
            pDlg->SetHeaders( bHasHeader );

        if ( bSortByRows != pDlg->GetByRows() )
            pDlg->SetByRows( bSortByRows );
    }

    if ( pSetP )
        FillItemSet( pSetP );

    return DeactivateRC::LeavePage;
}

void ScTabPageSortFields::FillFieldLists( sal_uInt16 nStartField )
{
    if ( pViewData )
    {
        ScDocument* pDoc = pViewData->GetDocument();

        if ( pDoc )
        {
            for ( sal_uInt16 i=nStartField; i<nSortKeyCount; i++ )
            {
                m_aSortWin.m_aSortKeyItems[i]->m_xLbSort->clear();
                m_aSortWin.m_aSortKeyItems[i]->m_xLbSort->append_text(aStrUndefined);
            }

            SCCOL   nFirstSortCol   = aSortData.nCol1;
            SCROW   nFirstSortRow   = aSortData.nRow1;
            SCTAB   nTab        = pViewData->GetTabNo();
            sal_uInt16  i           = 1;
            nFieldArr.clear();
            nFieldArr.push_back(0);

            if ( bSortByRows )
            {
                OUString  aFieldName;
                SCCOL   nMaxCol = aSortData.nCol2;
                SCCOL   col;

                for ( col=nFirstSortCol; col<=nMaxCol && i<SC_MAXFIELDS; col++ )
                {
                    aFieldName = pDoc->GetString(col, nFirstSortRow, nTab);
                    if ( !bHasHeader || aFieldName.isEmpty() )
                    {
                        aFieldName = ScGlobal::ReplaceOrAppend( aStrColumn, "%1", ScColToAlpha( col ));
                    }
                    nFieldArr.push_back( col );

                    for ( sal_uInt16 j=nStartField; j<nSortKeyCount; j++ )
                        m_aSortWin.m_aSortKeyItems[j]->m_xLbSort->insert_text(i, aFieldName);

                    i++;
                }
            }
            else
            {
                OUString  aFieldName;
                SCROW   nMaxRow = aSortData.nRow2;
                SCROW   row;

                for ( row=nFirstSortRow; row<=nMaxRow && i<SC_MAXFIELDS; row++ )
                {
                    aFieldName = pDoc->GetString(nFirstSortCol, row, nTab);
                    if ( !bHasHeader || aFieldName.isEmpty() )
                    {
                        aFieldName = ScGlobal::ReplaceOrAppend( aStrRow, "%1", OUString::number( row+1));
                    }
                    nFieldArr.push_back( row );

                    for ( sal_uInt16 j=nStartField; j<nSortKeyCount; j++ )
                        m_aSortWin.m_aSortKeyItems[j]->m_xLbSort->insert_text(i, aFieldName);

                    i++;
                }
            }
            nFieldCount = i;
        }
    }
}

sal_uInt16 ScTabPageSortFields::GetFieldSelPos( SCCOLROW nField )
{
    sal_uInt16  nFieldPos   = 0;
    bool    bFound      = false;

    for ( sal_uInt16 n=1; n<nFieldCount && !bFound; n++ )
    {
        if ( nFieldArr[n] == nField )
        {
            nFieldPos = n;
            bFound = true;
        }
    }

    return nFieldPos;
}

void ScTabPageSortFields::SetLastSortKey( sal_uInt16 nItem )
{
    // Extend local SortParam copy
    const ScSortKeyState atempKeyState = { 0, false, true };
    aSortData.maKeyState.push_back( atempKeyState );

    // Add Sort Key Item
    ++nSortKeyCount;
    AddSortKey( nSortKeyCount );
    m_aSortWin.m_aSortKeyItems[nItem]->m_xLbSort->connect_changed(
                     LINK( this, ScTabPageSortFields, SelectHdl ) );

    FillFieldLists( nItem );

    // Set Status
    m_aSortWin.m_aSortKeyItems[nItem]->m_xBtnUp->set_active(true);
    m_aSortWin.m_aSortKeyItems[nItem]->m_xLbSort->set_active(0);
}

// Handler:

IMPL_LINK( ScTabPageSortFields, SelectHdl, weld::ComboBox&, rLb, void )
{
    OUString aSelEntry = rLb.get_active_text();
    ScSortKeyItems::iterator pIter;

    // If last listbox is enabled add one item
    if (m_aSortWin.m_aSortKeyItems.back()->m_xLbSort.get() == &rLb)
    {
        if ( aSelEntry != aStrUndefined )
        {
            SetLastSortKey( nSortKeyCount );
            return;
        }
    }

    // Find selected listbox
    pIter = std::find_if(m_aSortWin.m_aSortKeyItems.begin(), m_aSortWin.m_aSortKeyItems.end(),
        [&rLb](const ScSortKeyItems::value_type& rItem) { return rItem->m_xLbSort.get() == &rLb; });

    if (pIter == m_aSortWin.m_aSortKeyItems.end())
        return;

    // If not selecting the last Listbox, modify the succeeding ones
    ++pIter;
    if ( std::distance(m_aSortWin.m_aSortKeyItems.begin(), pIter) < nSortKeyCount )
    {
        if ( aSelEntry == aStrUndefined )
        {
            for ( ; pIter != m_aSortWin.m_aSortKeyItems.end(); ++pIter )
            {
                (*pIter)->m_xLbSort->set_active(0);

                (*pIter)->DisableField();
            }
        }
        else
        {
            (*pIter)->EnableField();
        }
     }
}

IMPL_LINK_NOARG(ScTabPageSortFields, ScrollToEndHdl, Timer*, void)
{
    m_xScrolledWindow->vadjustment_set_value(m_xScrolledWindow->vadjustment_get_upper());
}

void ScTabPageSortFields::AddSortKey( sal_uInt16 nItem )
{
    m_aSortWin.AddSortKey(nItem);
    m_aIdle.Start();
}

// Sort option Tab Page:

ScTabPageSortOptions::ScTabPageSortOptions(TabPageParent pParent, const SfxItemSet& rArgSet)
    : SfxTabPage(pParent, "modules/scalc/ui/sortoptionspage.ui", "SortOptionsPage", &rArgSet)
    , aStrRowLabel(ScResId(SCSTR_ROW_LABEL))
    , aStrColLabel(ScResId(SCSTR_COL_LABEL))
    , aStrUndefined(ScResId(SCSTR_UNDEFINED))
    , nWhichSort(rArgSet.GetPool()->GetWhich(SID_SORT))
    , aSortData(static_cast<const ScSortItem&>(rArgSet.Get(nWhichSort)).GetSortData())
    , pViewData(nullptr)
    , pDoc(nullptr)
    , m_xBtnCase(m_xBuilder->weld_check_button("case"))
    , m_xBtnHeader(m_xBuilder->weld_check_button("header"))
    , m_xBtnFormats(m_xBuilder->weld_check_button("formats"))
    , m_xBtnNaturalSort(m_xBuilder->weld_check_button("naturalsort"))
    , m_xBtnCopyResult(m_xBuilder->weld_check_button("copyresult"))
    , m_xLbOutPos(m_xBuilder->weld_combo_box("outarealb"))
    , m_xEdOutPos(m_xBuilder->weld_entry("outareaed"))
    , m_xBtnSortUser(m_xBuilder->weld_check_button("sortuser"))
    , m_xLbSortUser(m_xBuilder->weld_combo_box("sortuserlb"))
    , m_xLbLanguage(new LanguageBox(m_xBuilder->weld_combo_box("language")))
    , m_xFtAlgorithm(m_xBuilder->weld_label("algorithmft"))
    , m_xLbAlgorithm(m_xBuilder->weld_combo_box("algorithmlb"))
    , m_xBtnTopDown(m_xBuilder->weld_radio_button("topdown"))
    , m_xBtnLeftRight(m_xBuilder->weld_radio_button("leftright"))
    , m_xBtnIncComments(m_xBuilder->weld_check_button("includenotes"))
    , m_xBtnIncImages(m_xBuilder->weld_check_button("includeimages"))
{
    m_xLbSortUser->set_size_request(m_xLbSortUser->get_approximate_digit_width() * 50, -1);
    Init();
    SetExchangeSupport();
}

void ScTabPageSortOptions::Init()
{
    //  CollatorResource has user-visible names for sort algorithms
    m_xColRes.reset(new CollatorResource);

    //! use CollatorWrapper from document?
    m_xColWrap.reset(new CollatorWrapper(comphelper::getProcessComponentContext()));

    const ScSortItem&   rSortItem = static_cast<const ScSortItem&>(
                                    GetItemSet().Get( nWhichSort ));

    m_xLbOutPos->connect_changed( LINK( this, ScTabPageSortOptions, SelOutPosHdl ) );
    m_xBtnCopyResult->connect_toggled( LINK( this, ScTabPageSortOptions, EnableHdl ) );
    m_xBtnSortUser->connect_toggled( LINK( this, ScTabPageSortOptions, EnableHdl ) );
    m_xBtnTopDown->connect_toggled( LINK( this, ScTabPageSortOptions, SortDirHdl ) );
    m_xBtnLeftRight->connect_toggled( LINK( this, ScTabPageSortOptions, SortDirHdl ) );
    m_xLbLanguage->connect_changed( LINK( this, ScTabPageSortOptions, FillAlgorHdl ) );

    pViewData = rSortItem.GetViewData();
    pDoc      = pViewData ? pViewData->GetDocument() : nullptr;

    OSL_ENSURE( pViewData, "ViewData not found! :-/" );

    if ( pViewData && pDoc )
    {
        ScDBCollection* pDBColl     = pDoc->GetDBCollection();
        const SCTAB     nCurTab     = pViewData->GetTabNo();
        const formula::FormulaGrammar::AddressConvention eConv = pDoc->GetAddressConvention();

        m_xLbOutPos->clear();
        m_xLbOutPos->append_text(aStrUndefined);
        m_xLbOutPos->set_sensitive(false);

        ScAreaNameIterator aIter( pDoc );
        OUString aName;
        ScRange aRange;
        while ( aIter.Next( aName, aRange ) )
        {
            OUString aRefStr(aRange.aStart.Format(ScRefFlags::ADDR_ABS_3D, pDoc, eConv));
            m_xLbOutPos->append(aRefStr, aName);
        }

        m_xLbOutPos->set_active(0);
        m_xEdOutPos->set_text(EMPTY_OUSTRING);

        // Check whether the field that is passed on is a database field:

        if ( pDBColl )
        {
            ScDBData* pDBData
                    = pDBColl->GetDBAtArea( nCurTab,
                                            aSortData.nCol1, aSortData.nRow1,
                                            aSortData.nCol2, aSortData.nRow2 );
            if ( pDBData )
            {
                m_xBtnHeader->set_active(pDBData->HasHeader());
            }
        }

        m_xBtnHeader->set_label(aStrColLabel);
    }

    FillUserSortListBox();

    //  get available languages

    m_xLbLanguage->SetLanguageList( SvxLanguageListFlags::ALL | SvxLanguageListFlags::ONLY_KNOWN, false );
    m_xLbLanguage->InsertLanguage( LANGUAGE_SYSTEM );
}

VclPtr<SfxTabPage> ScTabPageSortOptions::Create(TabPageParent pParent, const SfxItemSet* rArgSet)
{
    return VclPtr<ScTabPageSortOptions>::Create(pParent, *rArgSet);
}

void ScTabPageSortOptions::Reset( const SfxItemSet* /* rArgSet */ )
{
    if ( aSortData.bUserDef )
    {
        m_xBtnSortUser->set_active(true);
        m_xLbSortUser->set_sensitive(true);
        m_xLbSortUser->set_active(aSortData.nUserIndex);
    }
    else
    {
        m_xBtnSortUser->set_active(false);
        m_xLbSortUser->set_sensitive(false);
        m_xLbSortUser->set_active(0);
    }

    m_xBtnCase->set_active( aSortData.bCaseSens );
    m_xBtnFormats->set_active( aSortData.bIncludePattern );
    m_xBtnHeader->set_active( aSortData.bHasHeader );
    m_xBtnNaturalSort->set_active( aSortData.bNaturalSort );
    m_xBtnIncComments->set_active( aSortData.bIncludeComments );
    m_xBtnIncImages->set_active( aSortData.bIncludeGraphicObjects );

    if ( aSortData.bByRow )
    {
        m_xBtnTopDown->set_active(true);
        m_xBtnHeader->set_label( aStrColLabel );
    }
    else
    {
        m_xBtnLeftRight->set_active(true);
        m_xBtnHeader->set_label( aStrRowLabel );
    }

    LanguageType eLang = LanguageTag::convertToLanguageType( aSortData.aCollatorLocale, false);
    if ( eLang == LANGUAGE_DONTKNOW )
        eLang = LANGUAGE_SYSTEM;
    m_xLbLanguage->set_active_id(eLang);
    FillAlgor();               // get algorithms, select default
    if ( !aSortData.aCollatorAlgorithm.isEmpty() )
        m_xLbAlgorithm->set_active_text(m_xColRes->GetTranslation(aSortData.aCollatorAlgorithm));

    if ( pDoc && !aSortData.bInplace )
    {
        ScRefFlags nFormat = (aSortData.nDestTab != pViewData->GetTabNo())
                            ? ScRefFlags::RANGE_ABS_3D
                            : ScRefFlags::RANGE_ABS;

        theOutPos.Set( aSortData.nDestCol,
                       aSortData.nDestRow,
                       aSortData.nDestTab );

        OUString aStr(theOutPos.Format(nFormat, pDoc, pDoc->GetAddressConvention()));
        m_xBtnCopyResult->set_active(true);
        m_xLbOutPos->set_sensitive(true);
        m_xEdOutPos->set_sensitive(true);
        m_xEdOutPos->set_text( aStr );
        EdOutPosModHdl();
        m_xEdOutPos->grab_focus();
        m_xEdOutPos->select_region(0, -1);
    }
    else
    {
        m_xBtnCopyResult->set_active( false );
        m_xLbOutPos->set_sensitive(false);
        m_xEdOutPos->set_sensitive(false);
        m_xEdOutPos->set_text( EMPTY_OUSTRING );
    }
}

bool ScTabPageSortOptions::FillItemSet( SfxItemSet* rArgSet )
{
    // Create local copy of ScParam
    ScSortParam aNewSortData = aSortData;

    const SfxItemSet* pExample = GetDialogExampleSet();
    if (pExample)
    {
        const SfxPoolItem* pItem;
        if (pExample->GetItemState(nWhichSort, true, &pItem) == SfxItemState::SET)
            aNewSortData = static_cast<const ScSortItem*>(pItem)->GetSortData();
    }
    aNewSortData.bByRow          = m_xBtnTopDown->get_active();
    aNewSortData.bHasHeader      = m_xBtnHeader->get_active();
    aNewSortData.bCaseSens       = m_xBtnCase->get_active();
    aNewSortData.bNaturalSort    = m_xBtnNaturalSort->get_active();
    aNewSortData.bIncludeComments= m_xBtnIncComments->get_active();
    aNewSortData.bIncludeGraphicObjects = m_xBtnIncImages->get_active();
    aNewSortData.bIncludePattern = m_xBtnFormats->get_active();
    aNewSortData.bInplace        = !m_xBtnCopyResult->get_active();
    aNewSortData.nDestCol        = theOutPos.Col();
    aNewSortData.nDestRow        = theOutPos.Row();
    aNewSortData.nDestTab        = theOutPos.Tab();
    aNewSortData.bUserDef        = m_xBtnSortUser->get_active();
    aNewSortData.nUserIndex      = (m_xBtnSortUser->get_active())
                                   ? m_xLbSortUser->get_active()
                                   : 0;

    // get locale
    LanguageType eLang = m_xLbLanguage->get_active_id();
    aNewSortData.aCollatorLocale = LanguageTag::convertToLocale( eLang, false);

    // get algorithm
    OUString sAlg;
    if ( eLang != LANGUAGE_SYSTEM )
    {
        uno::Sequence<OUString> aAlgos = m_xColWrap->listCollatorAlgorithms(
                aNewSortData.aCollatorLocale );
        const int nSel = m_xLbAlgorithm->get_active();
        if ( nSel < aAlgos.getLength() )
            sAlg = aAlgos[nSel];
    }
    aNewSortData.aCollatorAlgorithm = sAlg;

    rArgSet->Put( ScSortItem( SCITEM_SORTDATA, &aNewSortData ) );

    return true;
}

// for data exchange without dialogue detour:
void ScTabPageSortOptions::ActivatePage( const SfxItemSet& rSet )
{
    // Refresh local copy with shared data
    aSortData = static_cast<const ScSortItem&>(rSet.Get( SCITEM_SORTDATA )).GetSortData();
    if (ScSortDlg* pDlg = static_cast<ScSortDlg*>(GetDialogController()))
    {
        if ( m_xBtnHeader->get_active() != pDlg->GetHeaders() )
        {
            m_xBtnHeader->set_active( pDlg->GetHeaders() );
        }

        if ( m_xBtnTopDown->get_active() != pDlg->GetByRows() )
        {
            m_xBtnTopDown->set_active( pDlg->GetByRows() );
            m_xBtnLeftRight->set_active( !pDlg->GetByRows() );
        }

        m_xBtnHeader->set_label( (pDlg->GetByRows())
                            ? aStrColLabel
                            : aStrRowLabel );
    }
}

DeactivateRC ScTabPageSortOptions::DeactivatePage( SfxItemSet* pSetP )
{
    bool bPosInputOk = true;

    if ( m_xBtnCopyResult->get_active() )
    {
        OUString    thePosStr = m_xEdOutPos->get_text();
        ScAddress   thePos;
        sal_Int32   nColonPos = thePosStr.indexOf( ':' );

        if ( -1 != nColonPos )
            thePosStr = thePosStr.copy( 0, nColonPos );

        if ( pViewData )
        {
            //  visible table is default for input without table
            //  must be changed to GetRefTabNo when sorting has RefInput!
            thePos.SetTab( pViewData->GetTabNo() );
        }

        ScRefFlags nResult = thePos.Parse( thePosStr, pDoc, pDoc->GetAddressConvention() );

        bPosInputOk = (nResult & ScRefFlags::VALID) == ScRefFlags::VALID;

        if ( !bPosInputOk )
        {
            std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(GetDialogFrameWeld(),
                                                      VclMessageType::Warning, VclButtonsType::Ok,
                                                      ScResId(STR_INVALID_TABREF)));
            xBox->run();
            m_xEdOutPos->grab_focus();
            m_xEdOutPos->select_region(0, -1);
            theOutPos.Set(0,0,0);
        }
        else
        {
            m_xEdOutPos->set_text(thePosStr);
            theOutPos = thePos;
        }
    }

    ScSortDlg* pDlg = static_cast<ScSortDlg*>(GetDialogController());
    if (pDlg && bPosInputOk)
    {
        pDlg->SetHeaders( m_xBtnHeader->get_active() );
        pDlg->SetByRows ( m_xBtnTopDown->get_active() );
    }

    if ( pSetP && bPosInputOk )
        FillItemSet( pSetP );

    return bPosInputOk ? DeactivateRC::LeavePage : DeactivateRC::KeepPage;
}

void ScTabPageSortOptions::FillUserSortListBox()
{
    ScUserList* pUserLists = ScGlobal::GetUserList();

    m_xLbSortUser->clear();
    if ( pUserLists )
    {
        size_t nCount = pUserLists->size();
        for (size_t i=0; i<nCount; ++i)
            m_xLbSortUser->append_text((*pUserLists)[i].GetString());
    }
}

// Handler:

IMPL_LINK( ScTabPageSortOptions, EnableHdl, weld::ToggleButton&, rButton, void )
{
    if (&rButton == m_xBtnCopyResult.get())
    {
        if (rButton.get_active())
        {
            m_xLbOutPos->set_sensitive(true);
            m_xEdOutPos->set_sensitive(true);
            m_xEdOutPos->grab_focus();
        }
        else
        {
            m_xLbOutPos->set_sensitive(false);
            m_xEdOutPos->set_sensitive(false);
        }
    }
    else if (&rButton == m_xBtnSortUser.get())
    {
        if (rButton.get_active())
        {
            m_xLbSortUser->set_sensitive(true);
            m_xLbSortUser->grab_focus();
        }
        else
            m_xLbSortUser->set_sensitive(false);
    }
}

IMPL_LINK(ScTabPageSortOptions, SelOutPosHdl, weld::ComboBox&, rLb, void)
{
    if (&rLb == m_xLbOutPos.get())
    {
        OUString  aString;
        const int nSelPos = m_xLbOutPos->get_active();

        if (nSelPos > 0)
            aString = m_xLbOutPos->get_id(nSelPos);

        m_xEdOutPos->set_text(aString);
    }
}

IMPL_LINK_NOARG(ScTabPageSortOptions, SortDirHdl, weld::ToggleButton&, void)
{
    if (m_xBtnTopDown->get_active())
        m_xBtnHeader->set_label(aStrColLabel);
    else
        m_xBtnHeader->set_label(aStrRowLabel);
}

void ScTabPageSortOptions::EdOutPosModHdl()
{
    OUString  theCurPosStr = m_xEdOutPos->get_text();
    ScRefFlags  nResult = ScAddress().Parse( theCurPosStr, pDoc, pDoc->GetAddressConvention() );

    if ( (nResult & ScRefFlags::VALID) == ScRefFlags::VALID )
    {
        bool    bFound  = false;
        sal_Int32 i = 0;
        const int nCount = m_xLbOutPos->get_count();

        for ( i=2; i<nCount && !bFound; i++ )
        {
            OUString aStr = m_xLbOutPos->get_id(i);
            bFound = (theCurPosStr == aStr);
        }

        if ( bFound )
            m_xLbOutPos->set_active(--i);
        else
            m_xLbOutPos->set_active(0);
    }
}

void ScTabPageSortOptions::FillAlgor()
{
    m_xLbAlgorithm->freeze();
    m_xLbAlgorithm->clear();

    LanguageType eLang = m_xLbLanguage->get_active_id();
    if ( eLang == LANGUAGE_SYSTEM )
    {
        //  for LANGUAGE_SYSTEM no algorithm can be selected because
        //  it wouldn't necessarily exist for other languages
        //  -> leave list box empty if LANGUAGE_SYSTEM is selected
        m_xFtAlgorithm->set_sensitive( false );           // nothing to select
        m_xLbAlgorithm->set_sensitive( false );           // nothing to select
    }
    else
    {
        lang::Locale aLocale( LanguageTag::convertToLocale( eLang ));
        uno::Sequence<OUString> aAlgos = m_xColWrap->listCollatorAlgorithms( aLocale );

        long nCount = aAlgos.getLength();
        const OUString* pArray = aAlgos.getConstArray();
        for (long i=0; i<nCount; i++)
        {
            OUString sAlg = pArray[i];
            OUString sUser = m_xColRes->GetTranslation( sAlg );
            m_xLbAlgorithm->append_text(sUser);
        }
        m_xLbAlgorithm->set_active(0);       // first entry is default
        m_xFtAlgorithm->set_sensitive(nCount > 1);      // enable only if there is a choice
        m_xLbAlgorithm->set_sensitive(nCount > 1);      // enable only if there is a choice
    }

    m_xLbAlgorithm->thaw();
}

IMPL_LINK_NOARG(ScTabPageSortOptions, FillAlgorHdl, weld::ComboBox&, void)
{
    FillAlgor();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
