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

#include <vcl/msgbox.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <svtools/collatorres.hxx>
#include <unotools/collatorwrapper.hxx>
#include <unotools/localedatawrapper.hxx>
#include <comphelper/processfactory.hxx>

#include "scitems.hxx"
#include "uiitems.hxx"
#include "viewdata.hxx"
#include "document.hxx"
#include "global.hxx"
#include "globalnames.hxx"
#include "dbdata.hxx"
#include "userlist.hxx"
#include "rangeutl.hxx"
#include "scresid.hxx"
#include "sc.hrc"
#include "globstr.hrc"

#include "sortkeydlg.hxx"

#include "sortdlg.hxx"

#include "tpsort.hxx"

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

ScTabPageSortFields::ScTabPageSortFields(vcl::Window* pParent,
                                          const SfxItemSet& rArgSet)
    : SfxTabPage(pParent, "SortCriteriaPage",
        "modules/scalc/ui/sortcriteriapage.ui", &rArgSet)
    ,

        aStrUndefined   ( SC_RESSTR( SCSTR_UNDEFINED ) ),
        aStrColumn      ( SC_RESSTR( SCSTR_COLUMN ) ),
        aStrRow         ( SC_RESSTR( SCSTR_ROW ) ),

        nWhichSort      ( rArgSet.GetPool()->GetWhich( SID_SORT ) ),
        pDlg            ( static_cast<ScSortDlg*>(GetParentDialog()) ),
        pViewData       ( nullptr ),
        aSortData       ( static_cast<const ScSortItem&>(
                           rArgSet.Get( nWhichSort )).
                                GetSortData() ),
        nFieldCount     ( 0 ),
        nSortKeyCount   ( DEFSORT ),
        bHasHeader      ( false ),
        bSortByRows     ( false ),
        maSortKeyCtrl   ( this, maSortKeyItems )
{
    Init();
    SetExchangeSupport();
}

ScTabPageSortFields::~ScTabPageSortFields()
{
    disposeOnce();
}

void ScTabPageSortFields::dispose()
{
    pDlg.clear();
    maSortKeyItems.clear();
    maSortKeyCtrl.dispose();
    SfxTabPage::dispose();
}

void ScTabPageSortFields::SetPosSizePixel(const Point& rAllocPos, const Size& rAllocation)
{
    SfxTabPage::SetPosSizePixel(rAllocPos, rAllocation);
    maSortKeyCtrl.setScrollRange();
}

void ScTabPageSortFields::SetSizePixel(const Size& rAllocation)
{
    SfxTabPage::SetSizePixel(rAllocation);
    maSortKeyCtrl.setScrollRange();
}

void ScTabPageSortFields::SetPosPixel(const Point& rAllocPos)
{
    SfxTabPage::SetPosPixel(rAllocPos);
    maSortKeyCtrl.setScrollRange();
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
        maSortKeyCtrl.AddSortKey(i+1);
        maSortKeyItems[i]->m_pLbSort->SetSelectHdl( LINK( this, ScTabPageSortFields, SelectHdl ) );
    }
}

VclPtr<SfxTabPage> ScTabPageSortFields::Create( vcl::Window*    pParent,
                                                const SfxItemSet*  rArgSet )
{
    return VclPtr<ScTabPageSortFields>::Create( pParent, *rArgSet );
}

void ScTabPageSortFields::Reset( const SfxItemSet* /* rArgSet */ )
{
    bSortByRows = aSortData.bByRow;
    bHasHeader  = aSortData.bHasHeader;

    if ( maSortKeyItems[0]->m_pLbSort->GetEntryCount() == 0 )
        FillFieldLists(0);

    // ListBox selection:
    if (!aSortData.maKeyState.empty() && aSortData.maKeyState[0].bDoSort)
    {
        // Make sure that the all sort keys are reset
        for ( sal_uInt16 i=nSortKeyCount; i<aSortData.GetSortKeyCount(); i++ )
        {
            maSortKeyCtrl.AddSortKey(i+1);
            maSortKeyItems[i]->m_pLbSort->SetSelectHdl( LINK( this,
                                 ScTabPageSortFields, SelectHdl ) );
        }
        nSortKeyCount = aSortData.GetSortKeyCount();
        FillFieldLists(0);

        for ( sal_uInt16 i=0; i<nSortKeyCount; i++ )
        {
            if (aSortData.maKeyState[i].bDoSort )
            {
                maSortKeyItems[i]->m_pLbSort->SelectEntryPos( GetFieldSelPos(
                                    aSortData.maKeyState[i].nField ) );
                (aSortData.maKeyState[i].bAscending)
                    ? maSortKeyItems[i]->m_pBtnUp->Check()
                    : maSortKeyItems[i]->m_pBtnDown->Check();
            }
            else
            {
                maSortKeyItems[i]->m_pLbSort->SelectEntryPos( 0 ); // Select none
                maSortKeyItems[i]->m_pBtnUp->Check();
            }
        }

        // Enable or disable field depending on preceding Listbox selection
        maSortKeyItems[0]->EnableField();
        for ( sal_uInt16 i=1; i<nSortKeyCount; i++ )
            if ( maSortKeyItems[i - 1]->m_pLbSort->GetSelectEntryPos() == 0 )
                maSortKeyItems[i]->DisableField();
            else
                maSortKeyItems[i]->EnableField();
    }
    else
    {
        SCCOL  nCol = pViewData->GetCurX();

        if( nCol < aSortData.nCol1 )
            nCol = aSortData.nCol1;
        else if( nCol > aSortData.nCol2 )
            nCol = aSortData.nCol2;

        sal_uInt16  nSort1Pos = nCol - aSortData.nCol1+1;

        maSortKeyItems[0]->m_pLbSort->SelectEntryPos( nSort1Pos );
        for ( sal_uInt16 i=1; i<nSortKeyCount; i++ )
            maSortKeyItems[i]->m_pLbSort->SelectEntryPos( 0 );

        for ( sal_uInt16 i=0; i<nSortKeyCount; i++ )
            maSortKeyItems[i]->m_pBtnUp->Check();

        maSortKeyItems[0]->EnableField();
        maSortKeyItems[1]->EnableField();
        for ( sal_uInt16 i=2; i<nSortKeyCount; i++ )
            maSortKeyItems[i]->DisableField();
    }

    if ( pDlg )
    {
        pDlg->SetByRows ( bSortByRows );
        pDlg->SetHeaders( bHasHeader );
    }

    // Make sure that there is always a last undefined sort key
    if ( maSortKeyItems[nSortKeyCount - 1]->m_pLbSort->GetSelectEntryPos() > 0 )
        SetLastSortKey( nSortKeyCount );
}

bool ScTabPageSortFields::FillItemSet( SfxItemSet* rArgSet )
{
    ScSortParam aNewSortData = aSortData;

    if (pDlg)
    {
        const SfxItemSet* pExample = pDlg->GetExampleSet();
        const SfxPoolItem* pItem;
        if ( pExample && pExample->GetItemState( nWhichSort, true, &pItem ) == SfxItemState::SET )
        {
            ScSortParam aTempData = static_cast<const ScSortItem*>(pItem)->GetSortData();
            aTempData.maKeyState = aNewSortData.maKeyState;
            aNewSortData = aTempData;
        }
    }
    std::vector<sal_Int32>  nSortPos;

    for ( sal_uInt16 i=0; i<nSortKeyCount; i++ )
    {
        nSortPos.push_back( maSortKeyItems[i]->m_pLbSort->GetSelectEntryPos() );

        if ( nSortPos[i] == LISTBOX_ENTRY_NOTFOUND ) nSortPos[i] = 0;
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
            aNewSortData.maKeyState[i].bAscending = maSortKeyItems[i]->m_pBtnUp->IsChecked();

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
    if ( pDlg )
    {
        if ( bHasHeader  != pDlg->GetHeaders()
             || bSortByRows != pDlg->GetByRows() )
        {
            std::vector<sal_uInt16> nCurSel;
            for ( sal_uInt16 i=0; i<nSortKeyCount; i++ )
                nCurSel.push_back( maSortKeyItems[i]->m_pLbSort->GetSelectEntryPos() );

            bHasHeader  = pDlg->GetHeaders();
            bSortByRows = pDlg->GetByRows();
            FillFieldLists(0);

            for ( sal_uInt16 i=0; i<nSortKeyCount; i++ )
                maSortKeyItems[i]->m_pLbSort->SelectEntryPos( nCurSel[i] );
        }
    }
}

DeactivateRC ScTabPageSortFields::DeactivatePage( SfxItemSet* pSetP )
{
    if ( pDlg )
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
                maSortKeyItems[i]->m_pLbSort->Clear();
                maSortKeyItems[i]->m_pLbSort->InsertEntry( aStrUndefined, 0 );
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
                        maSortKeyItems[j]->m_pLbSort->InsertEntry( aFieldName, i );

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
                        maSortKeyItems[j]->m_pLbSort->InsertEntry( aFieldName, i );

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
    const ScSortKeyState atempKeyState = { false, 0, true };
    aSortData.maKeyState.push_back( atempKeyState );

    // Add Sort Key Item
    ++nSortKeyCount;
    maSortKeyCtrl.AddSortKey( nSortKeyCount );
    maSortKeyItems[nItem]->m_pLbSort->SetSelectHdl(
                     LINK( this, ScTabPageSortFields, SelectHdl ) );

    FillFieldLists( nItem );

    // Set Status
    maSortKeyItems[nItem]->m_pBtnUp->Check();
    maSortKeyItems[nItem]->m_pLbSort->SelectEntryPos( 0 );
}

// Handler:

IMPL_LINK_TYPED( ScTabPageSortFields, SelectHdl, ListBox&, rLb, void )
{
    OUString aSelEntry = rLb.GetSelectEntry();
    ScSortKeyItems::iterator pIter;

    // If last listbox is enabled add one item
    if ( maSortKeyItems.back()->m_pLbSort == &rLb )
        if ( aSelEntry != aStrUndefined )
        {
            SetLastSortKey( nSortKeyCount );
            return;
        }

    // Find selected listbox
    for ( pIter = maSortKeyItems.begin(); pIter != maSortKeyItems.end(); ++pIter )
    {
        if ( (*pIter)->m_pLbSort == &rLb ) break;
    }

    if (pIter == maSortKeyItems.end())
        return;

    // If not selecting the last Listbox, modify the succeeding ones
    ++pIter;
    if ( std::distance(maSortKeyItems.begin(), pIter) < nSortKeyCount )
    {
        if ( aSelEntry == aStrUndefined )
        {
            for ( ; pIter != maSortKeyItems.end(); ++pIter )
            {
                (*pIter)->m_pLbSort->SelectEntryPos( 0 );

                if ( (*pIter)->m_pFlSort->IsEnabled() )
                    (*pIter)->DisableField();
            }
        }
        else
        {
            if ( !(*pIter)->m_pFlSort->IsEnabled() )
                    (*pIter)->EnableField();
        }
     }
}

// Sort option Tab Page:

ScTabPageSortOptions::ScTabPageSortOptions( vcl::Window*             pParent,
                                            const SfxItemSet&   rArgSet )
    : SfxTabPage(pParent, "SortOptionsPage",
        "modules/scalc/ui/sortoptionspage.ui", &rArgSet)
    , aStrRowLabel(SC_RESSTR(SCSTR_ROW_LABEL))
    , aStrColLabel(SC_RESSTR(SCSTR_COL_LABEL))
    , aStrUndefined(SC_RESSTR(SCSTR_UNDEFINED))
    , nWhichSort(rArgSet.GetPool()->GetWhich(SID_SORT))
    , aSortData(static_cast<const ScSortItem&>(rArgSet.Get(nWhichSort)).GetSortData())
    , pViewData(nullptr)
    , pDoc(nullptr)
    , pDlg(static_cast<ScSortDlg*>(GetParentDialog()))
    , pColRes( nullptr )
    , pColWrap( nullptr )
{
    get(m_pBtnCase, "case");
    get(m_pBtnHeader, "header");
    get(m_pBtnFormats, "formats");
    get(m_pBtnNaturalSort, "naturalsort");
    get(m_pBtnCopyResult, "copyresult");
    get(m_pLbOutPos, "outarealb");
    get(m_pEdOutPos, "outareaed");
    get(m_pBtnSortUser, "sortuser");
    get(m_pLbSortUser, "sortuserlb");
    get(m_pFtAlgorithm, "algorithmft");
    get(m_pLbAlgorithm, "algorithmlb");
    get(m_pBtnTopDown, "topdown");
    get(m_pBtnLeftRight, "leftright");
    get(m_pLbLanguage, "language");
    Init();
    SetExchangeSupport();
}

ScTabPageSortOptions::~ScTabPageSortOptions()
{
    disposeOnce();
}

void ScTabPageSortOptions::dispose()
{
    const sal_Int32 nEntries = m_pLbOutPos->GetEntryCount();

    for ( sal_Int32 i=1; i<nEntries; ++i )
        delete static_cast<OUString*>(m_pLbOutPos->GetEntryData( i ));

    delete pColRes;
    delete pColWrap;        //! not if from document
    m_pBtnCase.clear();
    m_pBtnHeader.clear();
    m_pBtnFormats.clear();
    m_pBtnNaturalSort.clear();
    m_pBtnCopyResult.clear();
    m_pLbOutPos.clear();
    m_pEdOutPos.clear();
    m_pBtnSortUser.clear();
    m_pLbSortUser.clear();
    m_pLbLanguage.clear();
    m_pFtAlgorithm.clear();
    m_pLbAlgorithm.clear();
    m_pBtnTopDown.clear();
    m_pBtnLeftRight.clear();
    pDlg.clear();
    SfxTabPage::dispose();
}

void ScTabPageSortOptions::Init()
{
    //  CollatorResource has user-visible names for sort algorithms
    pColRes = new CollatorResource();

    //! use CollatorWrapper from document?
    pColWrap = new CollatorWrapper( comphelper::getProcessComponentContext() );

    const ScSortItem&   rSortItem = static_cast<const ScSortItem&>(
                                    GetItemSet().Get( nWhichSort ));

    m_pLbOutPos->SetSelectHdl    ( LINK( this, ScTabPageSortOptions, SelOutPosHdl ) );
    m_pBtnCopyResult->SetClickHdl( LINK( this, ScTabPageSortOptions, EnableHdl ) );
    m_pBtnSortUser->SetClickHdl  ( LINK( this, ScTabPageSortOptions, EnableHdl ) );
    m_pBtnTopDown->SetClickHdl   ( LINK( this, ScTabPageSortOptions, SortDirHdl ) );
    m_pBtnLeftRight->SetClickHdl ( LINK( this, ScTabPageSortOptions, SortDirHdl ) );
    m_pLbLanguage->SetSelectHdl  ( LINK( this, ScTabPageSortOptions, FillAlgorHdl ) );

    pViewData = rSortItem.GetViewData();
    pDoc      = pViewData ? pViewData->GetDocument() : nullptr;

    OSL_ENSURE( pViewData, "ViewData not found! :-/" );

    if ( pViewData && pDoc )
    {
        ScDBCollection* pDBColl     = pDoc->GetDBCollection();
        const SCTAB     nCurTab     = pViewData->GetTabNo();
        OUString        theDbName   = STR_DB_LOCAL_NONAME;
        const formula::FormulaGrammar::AddressConvention eConv = pDoc->GetAddressConvention();

        m_pLbOutPos->Clear();
        m_pLbOutPos->InsertEntry( aStrUndefined, 0 );
        m_pLbOutPos->Disable();

        ScAreaNameIterator aIter( pDoc );
        OUString aName;
        ScRange aRange;
        while ( aIter.Next( aName, aRange ) )
        {
            const sal_Int32 nInsert = m_pLbOutPos->InsertEntry( aName );

            OUString aRefStr(aRange.aStart.Format(ScRefFlags::ADDR_ABS_3D, pDoc, eConv));
            m_pLbOutPos->SetEntryData( nInsert, new OUString( aRefStr ) );
        }

        m_pLbOutPos->SelectEntryPos( 0 );
        m_pEdOutPos->SetText( EMPTY_OUSTRING );

        // Check whether the field that is passed on is a database field:

        ScAddress aScAddress( aSortData.nCol1, aSortData.nRow1, nCurTab );
        OUString theArea =
            ScRange( aScAddress,
                 ScAddress( aSortData.nCol2, aSortData.nRow2, nCurTab )
               ).Format(ScRefFlags::RANGE_ABS, pDoc, eConv);

        if ( pDBColl )
        {
            ScDBData* pDBData
                    = pDBColl->GetDBAtArea( nCurTab,
                                            aSortData.nCol1, aSortData.nRow1,
                                            aSortData.nCol2, aSortData.nRow2 );
            if ( pDBData )
            {
                theDbName = pDBData->GetName();
                m_pBtnHeader->Check( pDBData->HasHeader() );
            }
        }

        theArea += " (" + theDbName + ")";

        m_pBtnHeader->SetText( aStrColLabel );
    }

    FillUserSortListBox();

    //  get available languages

    m_pLbLanguage->SetLanguageList( SvxLanguageListFlags::ALL | SvxLanguageListFlags::ONLY_KNOWN, false );
    m_pLbLanguage->InsertLanguage( LANGUAGE_SYSTEM );
}

VclPtr<SfxTabPage> ScTabPageSortOptions::Create( vcl::Window* pParent,
                                                 const SfxItemSet* rArgSet )
{
    return VclPtr<ScTabPageSortOptions>::Create( pParent, *rArgSet );
}

void ScTabPageSortOptions::Reset( const SfxItemSet* /* rArgSet */ )
{
    if ( aSortData.bUserDef )
    {
        m_pBtnSortUser->Check();
        m_pLbSortUser->Enable();
        m_pLbSortUser->SelectEntryPos( aSortData.nUserIndex );
    }
    else
    {
        m_pBtnSortUser->Check( false );
        m_pLbSortUser->Disable();
        m_pLbSortUser->SelectEntryPos( 0 );
    }

    m_pBtnCase->Check          ( aSortData.bCaseSens );
    m_pBtnFormats->Check       ( aSortData.bIncludePattern );
    m_pBtnHeader->Check        ( aSortData.bHasHeader );
    m_pBtnNaturalSort->Check   ( aSortData.bNaturalSort );

    if ( aSortData.bByRow )
    {
        m_pBtnTopDown->Check();
        m_pBtnHeader->SetText( aStrColLabel );
    }
    else
    {
        m_pBtnLeftRight->Check();
        m_pBtnHeader->SetText( aStrRowLabel );
    }

    LanguageType eLang = LanguageTag::convertToLanguageType( aSortData.aCollatorLocale, false);
    if ( eLang == LANGUAGE_DONTKNOW )
        eLang = LANGUAGE_SYSTEM;
    m_pLbLanguage->SelectLanguage( eLang );
    FillAlgorHdl(*m_pLbLanguage.get());               // get algorithms, select default
    if ( !aSortData.aCollatorAlgorithm.isEmpty() )
        m_pLbAlgorithm->SelectEntry( pColRes->GetTranslation( aSortData.aCollatorAlgorithm ) );

    if ( pDoc && !aSortData.bInplace )
    {
        ScRefFlags nFormat = (aSortData.nDestTab != pViewData->GetTabNo())
                            ? ScRefFlags::RANGE_ABS_3D
                            : ScRefFlags::RANGE_ABS;

        theOutPos.Set( aSortData.nDestCol,
                       aSortData.nDestRow,
                       aSortData.nDestTab );

        OUString aStr(theOutPos.Format(nFormat, pDoc, pDoc->GetAddressConvention()));
        m_pBtnCopyResult->Check();
        m_pLbOutPos->Enable();
        m_pEdOutPos->Enable();
        m_pEdOutPos->SetText( aStr );
        EdOutPosModHdl(m_pEdOutPos);
        m_pEdOutPos->GrabFocus();
        m_pEdOutPos->SetSelection( Selection( 0, SELECTION_MAX ) );
    }
    else
    {
        m_pBtnCopyResult->Check( false );
        m_pLbOutPos->Disable();
        m_pEdOutPos->Disable();
        m_pEdOutPos->SetText( EMPTY_OUSTRING );
    }
}

bool ScTabPageSortOptions::FillItemSet( SfxItemSet* rArgSet )
{
    // Create local copy of ScParam
    ScSortParam aNewSortData = aSortData;

    if (pDlg)
    {
        const SfxItemSet* pExample = pDlg->GetExampleSet();
        const SfxPoolItem* pItem;
        if ( pExample && pExample->GetItemState( nWhichSort, true, &pItem ) == SfxItemState::SET )
            aNewSortData = static_cast<const ScSortItem*>(pItem)->GetSortData();
    }
    aNewSortData.bByRow          = m_pBtnTopDown->IsChecked();
    aNewSortData.bHasHeader      = m_pBtnHeader->IsChecked();
    aNewSortData.bCaseSens       = m_pBtnCase->IsChecked();
    aNewSortData.bNaturalSort    = m_pBtnNaturalSort->IsChecked();
    aNewSortData.bIncludePattern = m_pBtnFormats->IsChecked();
    aNewSortData.bInplace        = !m_pBtnCopyResult->IsChecked();
    aNewSortData.nDestCol        = theOutPos.Col();
    aNewSortData.nDestRow        = theOutPos.Row();
    aNewSortData.nDestTab        = theOutPos.Tab();
    aNewSortData.bUserDef        = m_pBtnSortUser->IsChecked();
    aNewSortData.nUserIndex      = (m_pBtnSortUser->IsChecked())
                                   ? m_pLbSortUser->GetSelectEntryPos()
                                   : 0;

    // get locale
    LanguageType eLang = m_pLbLanguage->GetSelectLanguage();
    aNewSortData.aCollatorLocale = LanguageTag::convertToLocale( eLang, false);

    // get algorithm
    OUString sAlg;
    if ( eLang != LANGUAGE_SYSTEM )
    {
        uno::Sequence<OUString> aAlgos = pColWrap->listCollatorAlgorithms(
                aNewSortData.aCollatorLocale );
        const sal_Int32 nSel = m_pLbAlgorithm->GetSelectEntryPos();
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
    if ( pDlg )
    {
        if ( m_pBtnHeader->IsChecked() != pDlg->GetHeaders() )
        {
            m_pBtnHeader->Check( pDlg->GetHeaders() );
        }

        if ( m_pBtnTopDown->IsChecked() != pDlg->GetByRows() )
        {
            m_pBtnTopDown->Check( pDlg->GetByRows() );
            m_pBtnLeftRight->Check( !pDlg->GetByRows() );
        }

        m_pBtnHeader->SetText( (pDlg->GetByRows())
                            ? aStrColLabel
                            : aStrRowLabel );
    }
}

DeactivateRC ScTabPageSortOptions::DeactivatePage( SfxItemSet* pSetP )
{
    bool bPosInputOk = true;

    if ( m_pBtnCopyResult->IsChecked() )
    {
        OUString    thePosStr = m_pEdOutPos->GetText();
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
            ScopedVclPtrInstance<MessageDialog>(this, ScGlobal::GetRscString( STR_INVALID_TABREF))->Execute();
            m_pEdOutPos->GrabFocus();
            m_pEdOutPos->SetSelection( Selection( 0, SELECTION_MAX ) );
            theOutPos.Set(0,0,0);
        }
        else
        {
            m_pEdOutPos->SetText( thePosStr );
            theOutPos = thePos;
        }
    }

    if ( pDlg && bPosInputOk )
    {
        pDlg->SetHeaders( m_pBtnHeader->IsChecked() );
        pDlg->SetByRows ( m_pBtnTopDown->IsChecked() );
    }

    if ( pSetP && bPosInputOk )
        FillItemSet( pSetP );

    return bPosInputOk ? DeactivateRC::LeavePage : DeactivateRC::KeepPage;
}

void ScTabPageSortOptions::FillUserSortListBox()
{
    ScUserList* pUserLists = ScGlobal::GetUserList();

    m_pLbSortUser->Clear();
    if ( pUserLists )
    {
        size_t nCount = pUserLists->size();
        if ( nCount > 0 )
            for ( size_t i=0; i<nCount; ++i )
                m_pLbSortUser->InsertEntry( (*pUserLists)[i].GetString() );
    }
}

// Handler:

IMPL_LINK_TYPED( ScTabPageSortOptions, EnableHdl, Button*, pButton, void )
{
    CheckBox* pBox = static_cast<CheckBox*>(pButton);
    if (pBox == m_pBtnCopyResult)
    {
        if ( pBox->IsChecked() )
        {
            m_pLbOutPos->Enable();
            m_pEdOutPos->Enable();
            m_pEdOutPos->GrabFocus();
        }
        else
        {
            m_pLbOutPos->Disable();
            m_pEdOutPos->Disable();
        }
    }
    else if (pBox == m_pBtnSortUser)
    {
        if ( pBox->IsChecked() )
        {
            m_pLbSortUser->Enable();
            m_pLbSortUser->GrabFocus();
        }
        else
            m_pLbSortUser->Disable();
    }
}

IMPL_LINK_TYPED( ScTabPageSortOptions, SelOutPosHdl, ListBox&, rLb, void )
{
    if (&rLb == m_pLbOutPos)
    {
        OUString  aString;
        const sal_Int32 nSelPos = m_pLbOutPos->GetSelectEntryPos();

        if ( nSelPos > 0 )
            aString = *static_cast<OUString*>(m_pLbOutPos->GetEntryData( nSelPos ));

        m_pEdOutPos->SetText( aString );
    }
}

IMPL_LINK_TYPED( ScTabPageSortOptions, SortDirHdl, Button *, pBtn, void )
{
    if (pBtn == m_pBtnTopDown)
    {
        m_pBtnHeader->SetText( aStrColLabel );
    }
    else if (pBtn == m_pBtnLeftRight)
    {
        m_pBtnHeader->SetText( aStrRowLabel );
    }
}

void ScTabPageSortOptions::EdOutPosModHdl( Edit* pEd )
{
    if (pEd == m_pEdOutPos)
    {
        OUString  theCurPosStr = m_pEdOutPos->GetText();
        ScRefFlags  nResult = ScAddress().Parse( theCurPosStr, pDoc, pDoc->GetAddressConvention() );

        if ( (nResult & ScRefFlags::VALID) == ScRefFlags::VALID )
        {
            bool    bFound  = false;
            sal_Int32 i = 0;
            const sal_Int32 nCount = m_pLbOutPos->GetEntryCount();

            for ( i=2; i<nCount && !bFound; i++ )
            {
                OUString* pStr = static_cast<OUString*>(m_pLbOutPos->GetEntryData( i ));
                bFound = (theCurPosStr == *pStr);
            }

            if ( bFound )
                m_pLbOutPos->SelectEntryPos( --i );
            else
                m_pLbOutPos->SelectEntryPos( 0 );
        }
    }
}

IMPL_LINK_NOARG_TYPED(ScTabPageSortOptions, FillAlgorHdl, ListBox&, void)
{
    m_pLbAlgorithm->SetUpdateMode( false );
    m_pLbAlgorithm->Clear();

    LanguageType eLang = m_pLbLanguage->GetSelectLanguage();
    if ( eLang == LANGUAGE_SYSTEM )
    {
        //  for LANGUAGE_SYSTEM no algorithm can be selected because
        //  it wouldn't necessarily exist for other languages
        //  -> leave list box empty if LANGUAGE_SYSTEM is selected
        m_pFtAlgorithm->Enable( false );           // nothing to select
        m_pLbAlgorithm->Enable( false );           // nothing to select
    }
    else
    {
        lang::Locale aLocale( LanguageTag::convertToLocale( eLang ));
        uno::Sequence<OUString> aAlgos = pColWrap->listCollatorAlgorithms( aLocale );

        long nCount = aAlgos.getLength();
        const OUString* pArray = aAlgos.getConstArray();
        for (long i=0; i<nCount; i++)
        {
            OUString sAlg = pArray[i];
            OUString sUser = pColRes->GetTranslation( sAlg );
            m_pLbAlgorithm->InsertEntry( sUser );
        }
        m_pLbAlgorithm->SelectEntryPos( 0 );       // first entry is default
        m_pFtAlgorithm->Enable( nCount > 1 );      // enable only if there is a choice
        m_pLbAlgorithm->Enable( nCount > 1 );      // enable only if there is a choice
    }

    m_pLbAlgorithm->SetUpdateMode( true );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
