/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


#undef SC_DLLIMPLEMENTATION

#include <vcl/msgbox.hxx>
#include <i18npool/mslangid.hxx>
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
#include "sc.hrc"       // -> Slot IDs
#include "globstr.hrc"

#include "sortkeydlg.hxx"

#include "sortdlg.hxx"
#include "sortdlg.hrc"

#define _TPSORT_CXX
#include "tpsort.hxx"
#undef _TPSORT_CXX

using namespace com::sun::star;

// -----------------------------------------------------------------------

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

//========================================================================
//========================================================================
// Sort Criteria Tab page

ScTabPageSortFields::ScTabPageSortFields( Window*           pParent,
                                          const SfxItemSet& rArgSet )

    :   SfxTabPage      ( pParent,
                          ScResId( RID_SCPAGE_SORT_FIELDS ),
                          rArgSet ),
        //
        aStrUndefined   ( SC_RESSTR( SCSTR_UNDEFINED ) ),
        aStrColumn      ( SC_RESSTR( SCSTR_COLUMN ) ),
        aStrRow         ( SC_RESSTR( SCSTR_ROW ) ),
        //
        nWhichSort      ( rArgSet.GetPool()->GetWhich( SID_SORT ) ),
        pDlg            ( (ScSortDlg*)(GetParentDialog()) ),
        pViewData       ( NULL ),
        aSortData       ( ((const ScSortItem&)
                           rArgSet.Get( nWhichSort )).
                                GetSortData() ),
        nFieldCount     ( 0 ),
        nSortKeyCount   ( DEFSORT ),
        bHasHeader      ( false ),
        bSortByRows     ( false ),
        maSortKeyCtrl   ( this, ScResId( CTRL_MANAGESORTKEY ), maSortKeyItems )
{
    Init();
    FreeResource();
    SetExchangeSupport();
}

// -----------------------------------------------------------------------

ScTabPageSortFields::~ScTabPageSortFields()
{
}

// -----------------------------------------------------------------------

void ScTabPageSortFields::Init()
{
    const ScSortItem& rSortItem = (const ScSortItem&)
                                  GetItemSet().Get( nWhichSort );

    pViewData = rSortItem.GetViewData();
    OSL_ENSURE( pViewData, "ViewData not found!" );

    nFieldArr.push_back( 0 );
    nFirstCol = 0;
    nFirstRow = 0;

    // Create three sort key dialogs by default
    for ( sal_uInt16 i=0; i<nSortKeyCount; i++ )
    {
        maSortKeyCtrl.AddSortKey(i+1);
        maSortKeyItems[i].aLbSort.SetSelectHdl( LINK( this, ScTabPageSortFields, SelectHdl ) );
    }
}

// -----------------------------------------------------------------------

SfxTabPage* ScTabPageSortFields::Create( Window*    pParent,
                                         const SfxItemSet&  rArgSet )
{
    return ( new ScTabPageSortFields( pParent, rArgSet ) );
}

// -----------------------------------------------------------------------

void ScTabPageSortFields::Reset( const SfxItemSet& /* rArgSet */ )
{
    bSortByRows = aSortData.bByRow;
    bHasHeader  = aSortData.bHasHeader;

    if ( maSortKeyItems[0].aLbSort.GetEntryCount() == 0 )
        FillFieldLists(0);

    // ListBox selection:
    if ( aSortData.maKeyState[0].bDoSort )
    {
        for ( sal_uInt16 i=0; i<nSortKeyCount; i++ )
        {
            if ( aSortData.maKeyState[i].bDoSort )
            {
                maSortKeyItems[i].aLbSort.SelectEntryPos( GetFieldSelPos(
                                    aSortData.maKeyState[i].nField ) );

                (aSortData.maKeyState[i].bAscending)
                    ? maSortKeyItems[i].aBtnUp.Check()
                    : maSortKeyItems[i].aBtnDown.Check();
            }
            else
            {
                maSortKeyItems[i].aLbSort.SelectEntryPos( 0 ); // Select none
                maSortKeyItems[i].aBtnUp.Check();
            }
        }

        // Enable or disable field depending on preceding Listbox selection
        maSortKeyItems[0].EnableField();
        for ( sal_uInt16 i=1; i<nSortKeyCount; i++ )
            if ( maSortKeyItems[i - 1].aLbSort.GetSelectEntryPos() == 0 )
                maSortKeyItems[i].DisableField();
            else
                maSortKeyItems[i].EnableField();

    }
    else
    {
        SCCOL  nCol = pViewData->GetCurX();

        if( nCol < aSortData.nCol1 )
            nCol = aSortData.nCol1;
        else if( nCol > aSortData.nCol2 )
            nCol = aSortData.nCol2;

        sal_uInt16  nSort1Pos = nCol - aSortData.nCol1+1;

        maSortKeyItems[0].aLbSort.SelectEntryPos( nSort1Pos );
        for ( sal_uInt16 i=1; i<nSortKeyCount; i++ )
            maSortKeyItems[i].aLbSort.SelectEntryPos( 0 );

        for ( sal_uInt16 i=0; i<nSortKeyCount; i++ )
            maSortKeyItems[i].aBtnUp.Check();


        maSortKeyItems[0].EnableField();
        maSortKeyItems[1].EnableField();
        for ( sal_uInt16 i=2; i<nSortKeyCount; i++ )
            maSortKeyItems[i].DisableField();
    }

    if ( pDlg )
    {
        pDlg->SetByRows ( bSortByRows );
        pDlg->SetHeaders( bHasHeader );
    }
}

// -----------------------------------------------------------------------

sal_Bool ScTabPageSortFields::FillItemSet( SfxItemSet& rArgSet )
{
    ScSortParam aNewSortData = aSortData;

    if (pDlg)
    {
        const SfxItemSet* pExample = pDlg->GetExampleSet();
        const SfxPoolItem* pItem;
        if ( pExample && pExample->GetItemState( nWhichSort, sal_True, &pItem ) == SFX_ITEM_SET )
        {
            ScSortParam aTempData = static_cast<const ScSortItem*>(pItem)->GetSortData();
            aTempData.maKeyState = aNewSortData.maKeyState;
            aNewSortData = aTempData;
        }
    }
    std::vector<sal_uInt16>  nSortPos;

    for ( sal_uInt16 i=0; i<nSortKeyCount; i++ )
    {
        nSortPos.push_back( maSortKeyItems[i].aLbSort.GetSelectEntryPos() );

        if ( nSortPos[i] == LISTBOX_ENTRY_NOTFOUND ) nSortPos[i] = 0;
    }

    if ( nSortPos[0] > 0 )
    {
        for ( sal_uInt16 i=0; i<nSortKeyCount; i++ )
            aNewSortData.maKeyState[i].bDoSort = (nSortPos[i] > 0);

        // If the "OK" was selected on the Options page while the sort
        // direction was changed, then the first field (i.e. nFieldArr[0])
        // of the respective direction is chosen as the sorting criterion:
        if ( bSortByRows != pDlg->GetByRows() )
        {
            for ( sal_uInt16 i=0; i<nSortKeyCount; i++ )
                aNewSortData.maKeyState[i].nField = ( bSortByRows ?
                        static_cast<SCCOLROW>(nFirstRow) :
                        static_cast<SCCOLROW>(nFirstCol) );
        }
        else
        {
           for ( sal_uInt16 i=0; i<nSortKeyCount; i++ )
               aNewSortData.maKeyState[i].nField = nFieldArr[nSortPos[i]];
        }

        for ( sal_uInt16 i=0; i<nSortKeyCount; i++ )
            aNewSortData.maKeyState[i].bAscending = maSortKeyItems[i].aBtnUp.IsChecked();

        // bHasHeader is in ScTabPageSortOptions::FillItemSet, where it belongs
    }
    else
    {
        for ( sal_uInt16 i=0; i<nSortKeyCount; i++ )
            aNewSortData.maKeyState[i].bDoSort = false;
    }

    rArgSet.Put( ScSortItem( SCITEM_SORTDATA, NULL, &aNewSortData ) );

    return sal_True;
}

// -----------------------------------------------------------------------

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
                nCurSel.push_back( maSortKeyItems[i].aLbSort.GetSelectEntryPos() );

            bHasHeader  = pDlg->GetHeaders();
            bSortByRows = pDlg->GetByRows();
            FillFieldLists(0);

            for ( sal_uInt16 i=0; i<nSortKeyCount; i++ )
                maSortKeyItems[i].aLbSort.SelectEntryPos( nCurSel[i] );
        }
    }
}

// -----------------------------------------------------------------------

int ScTabPageSortFields::DeactivatePage( SfxItemSet* pSetP )
{
    if ( pDlg )
    {
        if ( bHasHeader != pDlg->GetHeaders() )
            pDlg->SetHeaders( bHasHeader );

        if ( bSortByRows != pDlg->GetByRows() )
            pDlg->SetByRows( bSortByRows );
    }

    if ( pSetP )
        FillItemSet( *pSetP );

    return SfxTabPage::LEAVE_PAGE;
}

// -----------------------------------------------------------------------

void ScTabPageSortFields::FillFieldLists( sal_uInt16 nStartField )
{
    if ( pViewData )
    {
        ScDocument* pDoc = pViewData->GetDocument();

        if ( pDoc )
        {
            for ( sal_uInt16 i=nStartField; i<nSortKeyCount; i++ )
            {
                maSortKeyItems[i].aLbSort.Clear();
                maSortKeyItems[i].aLbSort.InsertEntry( aStrUndefined, 0 );
            }

            SCCOL   nFirstSortCol   = aSortData.nCol1;
            SCROW   nFirstSortRow   = aSortData.nRow1;
            SCTAB   nTab        = pViewData->GetTabNo();
            sal_uInt16  i           = 1;

            if ( bSortByRows )
            {
                rtl::OUString  aFieldName;
                SCCOL   nMaxCol = aSortData.nCol2;
                SCCOL   col;

                for ( col=nFirstSortCol; col<=nMaxCol && i<SC_MAXFIELDS; col++ )
                {
                    pDoc->GetString( col, nFirstSortRow, nTab, aFieldName );
                    if ( !bHasHeader || aFieldName.isEmpty() )
                    {
                        rtl::OUStringBuffer aBuf;
                        aBuf.append(aStrColumn);
                        aFieldName = aBuf.makeStringAndClear().replaceAll("%1", ScColToAlpha( col ));
                    }
                    nFieldArr.push_back( col );

                    for ( sal_uInt16 j=nStartField; j<nSortKeyCount; j++ )
                        maSortKeyItems[j].aLbSort.InsertEntry( aFieldName, i );

                    i++;
                }
            }
            else
            {
                rtl::OUString  aFieldName;
                SCROW   nMaxRow = aSortData.nRow2;
                SCROW   row;

                for ( row=nFirstSortRow; row<=nMaxRow && i<SC_MAXFIELDS; row++ )
                {
                    pDoc->GetString( nFirstSortCol, row, nTab, aFieldName );
                    if ( !bHasHeader || aFieldName.isEmpty() )
                    {
                        rtl::OUStringBuffer aBuf;
                        rtl::OUStringBuffer tempBuf;

                        aBuf.append(aStrRow);
                        tempBuf.append(row+1 );
                        aFieldName = aBuf.makeStringAndClear().replaceAll("%1", tempBuf.makeStringAndClear());
                    }
                    nFieldArr.push_back( row );

                    for ( sal_uInt16 j=nStartField; j<nSortKeyCount; j++ )
                        maSortKeyItems[j].aLbSort.InsertEntry( aFieldName, i );

                    i++;
                }
            }
            nFieldCount = i;
        }
    }
}

//------------------------------------------------------------------------

sal_uInt16 ScTabPageSortFields::GetFieldSelPos( SCCOLROW nField )
{
    sal_uInt16  nFieldPos   = 0;
    sal_Bool    bFound      = false;

    for ( sal_uInt16 n=1; n<nFieldCount && !bFound; n++ )
    {
        if ( nFieldArr[n] == nField )
        {
            nFieldPos = n;
            bFound = sal_True;
        }
    }

    return nFieldPos;
}

// -----------------------------------------------------------------------
// Handler:
//---------

IMPL_LINK( ScTabPageSortFields, SelectHdl, ListBox *, pLb )
{
    rtl::OUString aSelEntry = pLb->GetSelectEntry();
    ScSortKeyItems::iterator pIter;
    sal_uInt16 nSortKeyIndex = nSortKeyCount;

    // If last listbox is enabled add one item
    if ( &maSortKeyItems.back().aLbSort == pLb )
        if ( aSelEntry != aStrUndefined )
        {
            // Extend local SortParam copy
            const ScSortKeyState atempKeyState = { false, 0, true };
            aSortData.maKeyState.push_back( atempKeyState );

            // Add Sort Key Item
            ++nSortKeyCount;
            maSortKeyCtrl.AddSortKey( nSortKeyCount );
            maSortKeyItems[nSortKeyIndex].aLbSort.SetSelectHdl( LINK( this, ScTabPageSortFields, SelectHdl ) );

            FillFieldLists( nSortKeyIndex );

            // Set Status
            maSortKeyItems[nSortKeyIndex].aBtnUp.Check();
            maSortKeyItems[nSortKeyIndex].aLbSort.SelectEntryPos( 0 );
            return 0;
        }

    // Find selected listbox
    for ( pIter = maSortKeyItems.begin(); pIter != maSortKeyItems.end(); ++pIter )
    {
        if ( &pIter->aLbSort == pLb ) break;
    }

    // If not selecting the last Listbox, modify the succeeding ones
    ++pIter;
    if ( std::distance(maSortKeyItems.begin(), pIter) < nSortKeyCount )
    {
        if ( aSelEntry == aStrUndefined )
        {
            for ( ; pIter != maSortKeyItems.end(); ++pIter )
            {
                pIter->aLbSort.SelectEntryPos( 0 );

                if ( pIter->aFlSort.IsEnabled() )
                    pIter->DisableField();
            }
        }
        else
        {
            if ( !pIter->aFlSort.IsEnabled() )
                    pIter->EnableField();
        }
     }
    return 0;
}

//========================================================================
// Sort option Tab Page:
//========================================================================

ScTabPageSortOptions::ScTabPageSortOptions( Window*             pParent,
                                            const SfxItemSet&   rArgSet )

    :   SfxTabPage      ( pParent,
                          ScResId( RID_SCPAGE_SORT_OPTIONS ),
                          rArgSet ),
        //
        aBtnCase        ( this, ScResId( BTN_CASESENSITIVE ) ),
        aBtnHeader      ( this, ScResId( BTN_LABEL ) ),
        aBtnFormats     ( this, ScResId( BTN_FORMATS ) ),
        aBtnNaturalSort ( this, ScResId( BTN_NATURALSORT ) ),
        aBtnCopyResult  ( this, ScResId( BTN_COPYRESULT ) ),
        aLbOutPos       ( this, ScResId( LB_OUTAREA ) ),
        aEdOutPos       ( this, ScResId( ED_OUTAREA ) ),
        aBtnSortUser    ( this, ScResId( BTN_SORT_USER ) ),
        aLbSortUser     ( this, ScResId( LB_SORT_USER ) ),
        aFtLanguage     ( this, ScResId( FT_LANGUAGE ) ),
        aLbLanguage     ( this, ScResId( LB_LANGUAGE ) ),
        aFtAlgorithm    ( this, ScResId( FT_ALGORITHM ) ),
        aLbAlgorithm    ( this, ScResId( LB_ALGORITHM ) ),
        aLineDirection  ( this, ScResId( FL_DIRECTION ) ),
        aBtnTopDown     ( this, ScResId( BTN_TOP_DOWN ) ),
        aBtnLeftRight   ( this, ScResId( BTN_LEFT_RIGHT ) ),
        //
        aStrRowLabel    ( SC_RESSTR( STR_ROW_LABEL ) ),
        aStrColLabel    ( SC_RESSTR( STR_COL_LABEL ) ),
        aStrUndefined   ( SC_RESSTR( SCSTR_UNDEFINED ) ),
        //
        nWhichSort      ( rArgSet.GetPool()->GetWhich( SID_SORT ) ),
        aSortData       ( ((const ScSortItem&)
                          rArgSet.Get( nWhichSort )).GetSortData() ),
        pViewData       ( NULL ),
        pDoc            ( NULL ),
        pDlg            ( (ScSortDlg*)(GetParentDialog()) ),
        pColRes         ( NULL ),
        pColWrap        ( NULL )
{
    Init();
    FreeResource();
    SetExchangeSupport();

    aLbOutPos.SetAccessibleRelationLabeledBy(&aBtnCopyResult);
    aLbOutPos.SetAccessibleName(aBtnCopyResult.GetText());
    aEdOutPos.SetAccessibleRelationLabeledBy(&aBtnCopyResult);
    aEdOutPos.SetAccessibleName(aBtnCopyResult.GetText());
    aLbSortUser.SetAccessibleRelationLabeledBy(&aBtnSortUser);
    aLbSortUser.SetAccessibleName(aBtnSortUser.GetText());
}

// -----------------------------------------------------------------------

ScTabPageSortOptions::~ScTabPageSortOptions()
{
    sal_uInt16 nEntries = aLbOutPos.GetEntryCount();

    for ( sal_uInt16 i=1; i<nEntries; i++ )
        delete (rtl::OUString*)aLbOutPos.GetEntryData( i );

    delete pColRes;
    delete pColWrap;        //! not if from document
}

// -----------------------------------------------------------------------

void ScTabPageSortOptions::Init()
{
    //  CollatorResource has user-visible names for sort algorithms
    pColRes = new CollatorResource();

    //! use CollatorWrapper from document?
    pColWrap = new CollatorWrapper( comphelper::getProcessServiceFactory() );

    const ScSortItem&   rSortItem = (const ScSortItem&)
                                    GetItemSet().Get( nWhichSort );

    aLbOutPos.SetSelectHdl    ( LINK( this, ScTabPageSortOptions, SelOutPosHdl ) );
    aBtnCopyResult.SetClickHdl( LINK( this, ScTabPageSortOptions, EnableHdl ) );
    aBtnSortUser.SetClickHdl  ( LINK( this, ScTabPageSortOptions, EnableHdl ) );
    aBtnTopDown.SetClickHdl   ( LINK( this, ScTabPageSortOptions, SortDirHdl ) );
    aBtnLeftRight.SetClickHdl ( LINK( this, ScTabPageSortOptions, SortDirHdl ) );
    aLbLanguage.SetSelectHdl  ( LINK( this, ScTabPageSortOptions, FillAlgorHdl ) );

    pViewData = rSortItem.GetViewData();
    pDoc      = pViewData ? pViewData->GetDocument() : NULL;

    OSL_ENSURE( pViewData, "ViewData not found! :-/" );


    if ( pViewData && pDoc )
    {
        String          theArea;
        ScDBCollection* pDBColl     = pDoc->GetDBCollection();
        rtl::OUString          theDbArea;
        const SCTAB     nCurTab     = pViewData->GetTabNo();
        rtl::OUString          theDbName   = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(STR_DB_LOCAL_NONAME));
        const formula::FormulaGrammar::AddressConvention eConv = pDoc->GetAddressConvention();

        aLbOutPos.Clear();
        aLbOutPos.InsertEntry( aStrUndefined, 0 );
        aLbOutPos.Disable();

        ScAreaNameIterator aIter( pDoc );
        rtl::OUString aName;
        ScRange aRange;
        rtl::OUString aRefStr;
        while ( aIter.Next( aName, aRange ) )
        {
            sal_uInt16 nInsert = aLbOutPos.InsertEntry( aName );

            aRange.aStart.Format( aRefStr, SCA_ABS_3D, pDoc, eConv );
            aLbOutPos.SetEntryData( nInsert, new rtl::OUString( aRefStr ) );
        }

        aLbOutPos.SelectEntryPos( 0 );
        aEdOutPos.SetText( EMPTY_STRING );

        // Check whether the field that is passed on is a database field:

        ScAddress aScAddress( aSortData.nCol1, aSortData.nRow1, nCurTab );
        ScRange( aScAddress,
                 ScAddress( aSortData.nCol2, aSortData.nRow2, nCurTab )
               ).Format( theArea, SCR_ABS, pDoc, eConv );

        if ( pDBColl )
        {
            ScDBData* pDBData
                    = pDBColl->GetDBAtArea( nCurTab,
                                            aSortData.nCol1, aSortData.nRow1,
                                            aSortData.nCol2, aSortData.nRow2 );
            if ( pDBData )
            {
                theDbName = pDBData->GetName();
                aBtnHeader.Check( pDBData->HasHeader() );
            }
        }

        theArea.AppendAscii(RTL_CONSTASCII_STRINGPARAM(" ("));
        theArea += theDbName;
        theArea += ')';

        aBtnHeader.SetText( aStrColLabel );
    }

    FillUserSortListBox();

    //  get available languages

    aLbLanguage.SetLanguageList( LANG_LIST_ALL | LANG_LIST_ONLY_KNOWN, false );
    aLbLanguage.InsertLanguage( LANGUAGE_SYSTEM );
}

// -----------------------------------------------------------------------

SfxTabPage* ScTabPageSortOptions::Create(
                                            Window*             pParent,
                                            const SfxItemSet&   rArgSet )
{
    return ( new ScTabPageSortOptions( pParent, rArgSet ) );
}

// -----------------------------------------------------------------------

void ScTabPageSortOptions::Reset( const SfxItemSet& /* rArgSet */ )
{
    if ( aSortData.bUserDef )
    {
        aBtnSortUser.Check( sal_True );
        aLbSortUser.Enable();
        aLbSortUser.SelectEntryPos( aSortData.nUserIndex );
    }
    else
    {
        aBtnSortUser.Check( false );
        aLbSortUser.Disable();
        aLbSortUser.SelectEntryPos( 0 );
    }

    aBtnCase.Check          ( aSortData.bCaseSens );
    aBtnFormats.Check       ( aSortData.bIncludePattern );
    aBtnHeader.Check        ( aSortData.bHasHeader );
    aBtnNaturalSort.Check   ( aSortData.bNaturalSort );

    if ( aSortData.bByRow )
    {
        aBtnTopDown.Check();
        aBtnHeader.SetText( aStrColLabel );
    }
    else
    {
        aBtnLeftRight.Check();
        aBtnHeader.SetText( aStrRowLabel );
    }

    LanguageType eLang = MsLangId::convertLocaleToLanguage( aSortData.aCollatorLocale );
    if ( eLang == LANGUAGE_DONTKNOW )
        eLang = LANGUAGE_SYSTEM;
    aLbLanguage.SelectLanguage( eLang );
    FillAlgorHdl( &aLbLanguage );               // get algorithms, select default
    if ( !aSortData.aCollatorAlgorithm.isEmpty() )
        aLbAlgorithm.SelectEntry( pColRes->GetTranslation( aSortData.aCollatorAlgorithm ) );

    if ( pDoc && !aSortData.bInplace )
    {
        rtl::OUString aStr;
        sal_uInt16 nFormat = (aSortData.nDestTab != pViewData->GetTabNo())
                            ? SCR_ABS_3D
                            : SCR_ABS;

        theOutPos.Set( aSortData.nDestCol,
                       aSortData.nDestRow,
                       aSortData.nDestTab );

        theOutPos.Format( aStr, nFormat, pDoc, pDoc->GetAddressConvention() );
        aBtnCopyResult.Check();
        aLbOutPos.Enable();
        aEdOutPos.Enable();
        aEdOutPos.SetText( aStr );
        EdOutPosModHdl( &aEdOutPos );
        aEdOutPos.GrabFocus();
        aEdOutPos.SetSelection( Selection( 0, SELECTION_MAX ) );
    }
    else
    {
        aBtnCopyResult.Check( false );
        aLbOutPos.Disable();
        aEdOutPos.Disable();
        aEdOutPos.SetText( EMPTY_STRING );
    }
}

// -----------------------------------------------------------------------

sal_Bool ScTabPageSortOptions::FillItemSet( SfxItemSet& rArgSet )
{
    // Create local copy of ScParam
    ScSortParam aNewSortData = aSortData;

    if (pDlg)
    {
        const SfxItemSet* pExample = pDlg->GetExampleSet();
        const SfxPoolItem* pItem;
        if ( pExample && pExample->GetItemState( nWhichSort, sal_True, &pItem ) == SFX_ITEM_SET )
            aNewSortData = static_cast<const ScSortItem*>(pItem)->GetSortData();
    }
    aNewSortData.bByRow          = aBtnTopDown.IsChecked();
    aNewSortData.bHasHeader      = aBtnHeader.IsChecked();
    aNewSortData.bCaseSens       = aBtnCase.IsChecked();
    aNewSortData.bNaturalSort    = aBtnNaturalSort.IsChecked();
    aNewSortData.bIncludePattern = aBtnFormats.IsChecked();
    aNewSortData.bInplace        = !aBtnCopyResult.IsChecked();
    aNewSortData.nDestCol        = theOutPos.Col();
    aNewSortData.nDestRow        = theOutPos.Row();
    aNewSortData.nDestTab        = theOutPos.Tab();
    aNewSortData.bUserDef        = aBtnSortUser.IsChecked();
    aNewSortData.nUserIndex      = (aBtnSortUser.IsChecked())
                                   ? aLbSortUser.GetSelectEntryPos()
                                   : 0;

    // get locale
    LanguageType eLang = aLbLanguage.GetSelectLanguage();
    aNewSortData.aCollatorLocale = MsLangId::convertLanguageToLocale( eLang, false );

    // get algorithm
    rtl::OUString sAlg;
    if ( eLang != LANGUAGE_SYSTEM )
    {
        uno::Sequence<rtl::OUString> aAlgos = pColWrap->listCollatorAlgorithms(
                aNewSortData.aCollatorLocale );
        sal_uInt16 nSel = aLbAlgorithm.GetSelectEntryPos();
        if ( nSel < aAlgos.getLength() )
            sAlg = aAlgos[nSel];
    }
    aNewSortData.aCollatorAlgorithm = sAlg;

    rArgSet.Put( ScSortItem( SCITEM_SORTDATA, &aNewSortData ) );

    return sal_True;
}

// -----------------------------------------------------------------------

// for data exchange without dialogue detour:
void ScTabPageSortOptions::ActivatePage( const SfxItemSet& rSet )
{
    // Refresh local copy with shared data
    aSortData = static_cast<const ScSortItem&>(rSet.Get( SCITEM_SORTDATA )).GetSortData();
    if ( pDlg )
    {
        if ( aBtnHeader.IsChecked() != pDlg->GetHeaders() )
        {
            aBtnHeader.Check( pDlg->GetHeaders() );
        }

        if ( aBtnTopDown.IsChecked() != pDlg->GetByRows() )
        {
            aBtnTopDown.Check( pDlg->GetByRows() );
            aBtnLeftRight.Check( !pDlg->GetByRows() );
        }

        aBtnHeader.SetText( (pDlg->GetByRows())
                            ? aStrColLabel
                            : aStrRowLabel );
    }
}

// -----------------------------------------------------------------------

int ScTabPageSortOptions::DeactivatePage( SfxItemSet* pSetP )
{
    sal_Bool bPosInputOk = sal_True;

    if ( aBtnCopyResult.IsChecked() )
    {
        String      thePosStr = aEdOutPos.GetText();
        ScAddress   thePos;
        xub_StrLen  nColonPos = thePosStr.Search( ':' );

        if ( STRING_NOTFOUND != nColonPos )
            thePosStr.Erase( nColonPos );

        if ( pViewData )
        {
            //  visible table is default for input without table
            //  must be changed to GetRefTabNo when sorting has RefInput!
            thePos.SetTab( pViewData->GetTabNo() );
        }

        sal_uInt16 nResult = thePos.Parse( thePosStr, pDoc, pDoc->GetAddressConvention() );

        bPosInputOk = ( SCA_VALID == (nResult & SCA_VALID) );

        if ( !bPosInputOk )
        {
            ErrorBox( this, WinBits( WB_OK | WB_DEF_OK ),
                     ScGlobal::GetRscString( STR_INVALID_TABREF )
                    ).Execute();
            aEdOutPos.GrabFocus();
            aEdOutPos.SetSelection( Selection( 0, SELECTION_MAX ) );
            theOutPos.Set(0,0,0);
        }
        else
        {
            aEdOutPos.SetText( thePosStr );
            theOutPos = thePos;
        }
    }

    if ( pDlg && bPosInputOk )
    {
        pDlg->SetHeaders( aBtnHeader.IsChecked() );
        pDlg->SetByRows ( aBtnTopDown.IsChecked() );
    }

    if ( pSetP && bPosInputOk )
        FillItemSet( *pSetP );

    return bPosInputOk ? SfxTabPage::LEAVE_PAGE : SfxTabPage::KEEP_PAGE;
}

// -----------------------------------------------------------------------

void ScTabPageSortOptions::FillUserSortListBox()
{
    ScUserList* pUserLists = ScGlobal::GetUserList();

    aLbSortUser.Clear();
    if ( pUserLists )
    {
        size_t nCount = pUserLists->size();
        if ( nCount > 0 )
            for ( size_t i=0; i<nCount; ++i )
                aLbSortUser.InsertEntry( (*pUserLists)[i]->GetString() );
    }
}

// -----------------------------------------------------------------------
// Handler:

IMPL_LINK( ScTabPageSortOptions, EnableHdl, CheckBox *, pBox )
{
    if ( pBox == &aBtnCopyResult )
    {
        if ( pBox->IsChecked() )
        {
            aLbOutPos.Enable();
            aEdOutPos.Enable();
            aEdOutPos.GrabFocus();
        }
        else
        {
            aLbOutPos.Disable();
            aEdOutPos.Disable();
        }
    }
    else if ( pBox == &aBtnSortUser )
    {
        if ( pBox->IsChecked() )
        {
            aLbSortUser.Enable();
            aLbSortUser.GrabFocus();
        }
        else
            aLbSortUser.Disable();
    }
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( ScTabPageSortOptions, SelOutPosHdl, ListBox *, pLb )
{
    if ( pLb == &aLbOutPos )
    {
        rtl::OUString  aString;
        sal_uInt16  nSelPos = aLbOutPos.GetSelectEntryPos();

        if ( nSelPos > 0 )
            aString = *(String*)aLbOutPos.GetEntryData( nSelPos );

        aEdOutPos.SetText( aString );
    }
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( ScTabPageSortOptions, SortDirHdl, RadioButton *, pBtn )
{
    if ( pBtn == &aBtnTopDown )
    {
        aBtnHeader.SetText( aStrColLabel );
    }
    else if ( pBtn == &aBtnLeftRight )
    {
        aBtnHeader.SetText( aStrRowLabel );
    }
    return 0;
}

// -----------------------------------------------------------------------

void ScTabPageSortOptions::EdOutPosModHdl( Edit* pEd )
{
    if ( pEd == &aEdOutPos )
    {
       String  theCurPosStr = aEdOutPos.GetText();
        sal_uInt16  nResult = ScAddress().Parse( theCurPosStr, pDoc, pDoc->GetAddressConvention() );

        if ( SCA_VALID == (nResult & SCA_VALID) )
        {
            String* pStr    = NULL;
            sal_Bool    bFound  = false;
            sal_uInt16  i       = 0;
            sal_uInt16  nCount  = aLbOutPos.GetEntryCount();

            for ( i=2; i<nCount && !bFound; i++ )
            {
                pStr = (String*)aLbOutPos.GetEntryData( i );
                bFound = (theCurPosStr == *pStr);
            }

            if ( bFound )
                aLbOutPos.SelectEntryPos( --i );
            else
                aLbOutPos.SelectEntryPos( 0 );
        }
    }
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(ScTabPageSortOptions, FillAlgorHdl)
{
    aLbAlgorithm.SetUpdateMode( false );
    aLbAlgorithm.Clear();

    LanguageType eLang = aLbLanguage.GetSelectLanguage();
    if ( eLang == LANGUAGE_SYSTEM )
    {
        //  for LANGUAGE_SYSTEM no algorithm can be selected because
        //  it wouldn't necessarily exist for other languages
        //  -> leave list box empty if LANGUAGE_SYSTEM is selected
        aFtAlgorithm.Enable( false );           // nothing to select
        aLbAlgorithm.Enable( false );           // nothing to select
    }
    else
    {
        lang::Locale aLocale( MsLangId::convertLanguageToLocale( eLang ));
        uno::Sequence<rtl::OUString> aAlgos = pColWrap->listCollatorAlgorithms( aLocale );

        long nCount = aAlgos.getLength();
        const rtl::OUString* pArray = aAlgos.getConstArray();
        for (long i=0; i<nCount; i++)
        {
            String sAlg = pArray[i];
            String sUser = pColRes->GetTranslation( sAlg );
            aLbAlgorithm.InsertEntry( sUser, LISTBOX_APPEND );
        }
        aLbAlgorithm.SelectEntryPos( 0 );       // first entry is default
        aFtAlgorithm.Enable( nCount > 1 );      // enable only if there is a choice
        aLbAlgorithm.Enable( nCount > 1 );      // enable only if there is a choice
    }

    aLbAlgorithm.SetUpdateMode( sal_True );
    return 0;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
