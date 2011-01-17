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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"



//----------------------------------------------------------------------------

#include "pvlaydlg.hxx"
#include "dbdocfun.hxx"

#include <sfx2/dispatch.hxx>
#include <vcl/msgbox.hxx>

#include <com/sun/star/sheet/DataPilotFieldOrientation.hpp>
#include <com/sun/star/sheet/DataPilotFieldSortMode.hpp>

#include "uiitems.hxx"
#include "rangeutl.hxx"
#include "document.hxx"
#include "viewdata.hxx"
#include "tabvwsh.hxx"
#include "reffact.hxx"
#include "scresid.hxx"
#include "pvglob.hxx"
//CHINA001 #include "pvfundlg.hxx"
#include "globstr.hrc"
#include "pivot.hrc"
#include "dpobject.hxx"
#include "dpsave.hxx"
#include "dpshttab.hxx"
#include "scmod.hxx"

#include "sc.hrc" //CHINA001
#include "scabstdlg.hxx" //CHINA001
using namespace com::sun::star;
using ::rtl::OUString;
using ::std::vector;

//----------------------------------------------------------------------------

#define FSTR(index) aFuncNameArr[index-1]
#define STD_FORMAT   SCA_VALID | SCA_TAB_3D \
                    | SCA_COL_ABSOLUTE | SCA_ROW_ABSOLUTE | SCA_TAB_ABSOLUTE

long PivotGlobal::nObjHeight = 0;    // initialized with resource data
long PivotGlobal::nObjWidth  = 0;
long PivotGlobal::nSelSpace  = 0;


//============================================================================

namespace {

void lcl_FillToPivotField( PivotField& rPivotField, const ScDPFuncData& rFuncData )
{
    rPivotField.nCol = rFuncData.mnCol;
    rPivotField.nFuncMask = rFuncData.mnFuncMask;
    rPivotField.maFieldRef = rFuncData.maFieldRef;
}

PointerStyle lclGetPointerForField( ScDPFieldType eType )
{
    switch( eType )
    {
        case TYPE_PAGE:     return POINTER_PIVOT_FIELD;
        case TYPE_COL:      return POINTER_PIVOT_COL;
        case TYPE_ROW:      return POINTER_PIVOT_ROW;
        case TYPE_DATA:     return POINTER_PIVOT_FIELD;
        case TYPE_SELECT:   return POINTER_PIVOT_FIELD;
    }
    return POINTER_ARROW;
}

} // namespace

//============================================================================

//----------------------------------------------------------------------------

ScDPLayoutDlg::ScDPLayoutDlg( SfxBindings* pB, SfxChildWindow* pCW, Window* pParent,
                                    const ScDPObject& rDPObject )
    :   ScAnyRefDlg ( pB, pCW, pParent, RID_SCDLG_PIVOT_LAYOUT ),
        aFlLayout       ( this, ScResId( FL_LAYOUT ) ),
        aFtPage         ( this, ScResId( FT_PAGE ) ),
        aWndPage        ( this, ScResId( WND_PAGE ),   TYPE_PAGE,   &aFtPage ),
        aFtCol          ( this, ScResId( FT_COL ) ),
        aWndCol         ( this, ScResId( WND_COL ),    TYPE_COL,    &aFtCol ),
        aFtRow          ( this, ScResId( FT_ROW ) ),
        aWndRow         ( this, ScResId( WND_ROW ),    TYPE_ROW,    &aFtRow ),
        aFtData         ( this, ScResId( FT_DATA ) ),
        aWndData        ( this, ScResId( WND_DATA ),   TYPE_DATA,   &aFtData ),
        aWndSelect      ( this, ScResId( WND_SELECT ), TYPE_SELECT, String(ScResId(STR_SELECT)) ),
        aSlider         ( this, ScResId( WND_HSCROLL ) ),
        aFtInfo         ( this, ScResId( FT_INFO ) ),

        aFlAreas        ( this, ScResId( FL_OUTPUT ) ),

        aFtInArea       ( this, ScResId( FT_INAREA) ),
        aEdInPos        ( this, ScResId( ED_INAREA) ),
        aRbInPos        ( this, ScResId( RB_INAREA ), &aEdInPos, this ),

        aLbOutPos       ( this, ScResId( LB_OUTAREA ) ),
        aFtOutArea      ( this, ScResId( FT_OUTAREA ) ),
        aEdOutPos       ( this, this, ScResId( ED_OUTAREA ) ),
        aRbOutPos       ( this, ScResId( RB_OUTAREA ), &aEdOutPos, this ),
        aBtnIgnEmptyRows( this, ScResId( BTN_IGNEMPTYROWS ) ),
        aBtnDetectCat   ( this, ScResId( BTN_DETECTCAT ) ),
        aBtnTotalCol    ( this, ScResId( BTN_TOTALCOL ) ),
        aBtnTotalRow    ( this, ScResId( BTN_TOTALROW ) ),
        aBtnFilter      ( this, ScResId( BTN_FILTER ) ),
        aBtnDrillDown   ( this, ScResId( BTN_DRILLDOWN ) ),

        aBtnOk          ( this, ScResId( BTN_OK ) ),
        aBtnCancel      ( this, ScResId( BTN_CANCEL ) ),
        aBtnHelp        ( this, ScResId( BTN_HELP ) ),
        aBtnRemove      ( this, ScResId( BTN_REMOVE ) ),
        aBtnOptions     ( this, ScResId( BTN_OPTIONS ) ),
        aBtnMore        ( this, ScResId( BTN_MORE ) ),

        aStrUndefined   ( ScResId( SCSTR_UNDEFINED ) ),
        aStrNewTable    ( ScResId( SCSTR_NEWTABLE ) ),

        bIsDrag         ( sal_False ),

        pEditActive     ( NULL ),

        eLastActiveType ( TYPE_SELECT ),
        nOffset         ( 0 ),
        //
        xDlgDPObject    ( new ScDPObject( rDPObject ) ),
        pViewData       ( ((ScTabViewShell*)SfxViewShell::Current())->
                                GetViewData() ),
        pDoc            ( ((ScTabViewShell*)SfxViewShell::Current())->
                                GetViewData()->GetDocument() ),
        bRefInputMode   ( sal_False )
{
    xDlgDPObject->SetAlive( sal_True );     // needed to get structure information
    xDlgDPObject->FillOldParam( thePivotData, sal_False );
    xDlgDPObject->FillLabelData( thePivotData );

    Init();
    FreeResource();
}


//----------------------------------------------------------------------------

ScDPLayoutDlg::~ScDPLayoutDlg()
{
    sal_uInt16 nEntries = aLbOutPos.GetEntryCount();
    sal_uInt16 i;

    for ( i=2; i<nEntries; i++ )
        delete (String*)aLbOutPos.GetEntryData( i );
}


//----------------------------------------------------------------------------

ScDPFieldWindow& ScDPLayoutDlg::GetFieldWindow( ScDPFieldType eType )
{
    switch( eType )
    {
        case TYPE_PAGE: return aWndPage;
        case TYPE_ROW:  return aWndRow;
        case TYPE_COL:  return aWndCol;
        case TYPE_DATA: return aWndData;
        default:
        {
            // added to avoid warnings
        }
    }
    return aWndSelect;
}

void __EXPORT ScDPLayoutDlg::Init()
{
    DBG_ASSERT( pViewData && pDoc,
                "Ctor-Initialisierung fehlgeschlagen!" );

    aBtnRemove.SetClickHdl( LINK( this, ScDPLayoutDlg, ClickHdl ) );
    aBtnOptions.SetClickHdl( LINK( this, ScDPLayoutDlg, ClickHdl ) );

    aFuncNameArr.reserve( FUNC_COUNT );
    for ( sal_uInt16 i = 0; i < FUNC_COUNT; ++i )
        aFuncNameArr.push_back( String( ScResId( i + 1 ) ) );

    aBtnMore.AddWindow( &aFlAreas );
    aBtnMore.AddWindow( &aFtInArea );
    aBtnMore.AddWindow( &aEdInPos );
    aBtnMore.AddWindow( &aRbInPos );
    aBtnMore.AddWindow( &aFtOutArea );
    aBtnMore.AddWindow( &aLbOutPos );
    aBtnMore.AddWindow( &aEdOutPos );
    aBtnMore.AddWindow( &aRbOutPos );
    aBtnMore.AddWindow( &aBtnIgnEmptyRows );
    aBtnMore.AddWindow( &aBtnDetectCat );
    aBtnMore.AddWindow( &aBtnTotalCol );
    aBtnMore.AddWindow( &aBtnTotalRow );
    aBtnMore.AddWindow( &aBtnFilter );
    aBtnMore.AddWindow( &aBtnDrillDown );
    aBtnMore.SetClickHdl( LINK( this, ScDPLayoutDlg, MoreClickHdl ) );

    {
        Size aFieldSize( Window( this, ScResId( WND_FIELD ) ).GetSizePixel() );
        OHEIGHT = aFieldSize.Height();
        OWIDTH  = aFieldSize.Width();
    }
    SSPACE = Window( this, ScResId( WND_FIELD_SPACE ) ).GetSizePixel().Width();

    CalcWndSizes();

    aSelectArr.resize( MAX_LABELS );
    aPageArr.resize( MAX_PAGEFIELDS );
    aColArr.resize( MAX_FIELDS );
    aRowArr.resize( MAX_FIELDS );
    aDataArr.resize( MAX_FIELDS );

    ScRange inRange;
    String inString;
    if (xDlgDPObject->GetSheetDesc())
    {
        aEdInPos.Enable();
        aRbInPos.Enable();
        aOldRange = xDlgDPObject->GetSheetDesc()->aSourceRange;
        aOldRange.Format( inString, SCR_ABS_3D, pDoc, pDoc->GetAddressConvention() );
        aEdInPos.SetText(inString);
    }
    else
    {
        /* Data is not reachable, so could be a remote database */
        aEdInPos.Disable();
        aRbInPos.Disable();
    }

    InitFields();

    aLbOutPos .SetSelectHdl( LINK( this, ScDPLayoutDlg, SelAreaHdl ) );
    aEdOutPos .SetModifyHdl( LINK( this, ScDPLayoutDlg, EdModifyHdl ) );
    aEdInPos  .SetModifyHdl( LINK( this, ScDPLayoutDlg, EdInModifyHdl ) );
    aBtnOk    .SetClickHdl ( LINK( this, ScDPLayoutDlg, OkHdl ) );
    aBtnCancel.SetClickHdl ( LINK( this, ScDPLayoutDlg, CancelHdl ) );
    Link aLink = LINK( this, ScDPLayoutDlg, GetFocusHdl );
    if ( aEdInPos.IsEnabled() )
        // Once disabled it will never get enabled, so no need to handle focus.
        aEdInPos.SetGetFocusHdl( aLink );
    aEdOutPos.SetGetFocusHdl( aLink );

    if ( pViewData && pDoc )
    {
        /*
         * Aus den RangeNames des Dokumentes werden nun die
         * in einem Zeiger-Array gemerkt, bei denen es sich
         * um sinnvolle Bereiche handelt
         */

        aLbOutPos.Clear();
        aLbOutPos.InsertEntry( aStrUndefined, 0 );
        aLbOutPos.InsertEntry( aStrNewTable,  1 );

        ScAreaNameIterator aIter( pDoc );
        String aName;
        ScRange aRange;
        String aRefStr;
        while ( aIter.Next( aName, aRange ) )
        {
            if ( !aIter.WasDBName() )       // hier keine DB-Bereiche !
            {
                sal_uInt16 nInsert = aLbOutPos.InsertEntry( aName );

                aRange.aStart.Format( aRefStr, SCA_ABS_3D, pDoc, pDoc->GetAddressConvention() );
                aLbOutPos.SetEntryData( nInsert, new String( aRefStr ) );
            }
        }
    }

    if ( thePivotData.nTab != MAXTAB+1 )
    {
        String aStr;
        ScAddress( thePivotData.nCol,
                   thePivotData.nRow,
                   thePivotData.nTab ).Format( aStr, STD_FORMAT, pDoc, pDoc->GetAddressConvention() );
        aEdOutPos.SetText( aStr );
        EdModifyHdl(0);
    }
    else
    {
        aLbOutPos.SelectEntryPos( aLbOutPos.GetEntryCount()-1 );
        SelAreaHdl(NULL);
    }

    aBtnIgnEmptyRows.Check( thePivotData.bIgnoreEmptyRows );
    aBtnDetectCat   .Check( thePivotData.bDetectCategories );
    aBtnTotalCol    .Check( thePivotData.bMakeTotalCol );
    aBtnTotalRow    .Check( thePivotData.bMakeTotalRow );

    if( const ScDPSaveData* pSaveData = xDlgDPObject->GetSaveData() )
    {
        aBtnFilter.Check( pSaveData->GetFilterButton() );
        aBtnDrillDown.Check( pSaveData->GetDrillDown() );
    }
    else
    {
        aBtnFilter.Check();
        aBtnDrillDown.Check();
    }

    aWndPage.SetHelpId( HID_SC_DPLAY_PAGE );
    aWndCol.SetHelpId( HID_SC_DPLAY_COLUMN );
    aWndRow.SetHelpId( HID_SC_DPLAY_ROW );
    aWndData.SetHelpId( HID_SC_DPLAY_DATA );
    aWndSelect.SetHelpId( HID_SC_DPLAY_SELECT );

    InitFocus();

//  SetDispatcherLock( sal_True ); // Modal-Modus einschalten

    //@BugID 54702 Enablen/Disablen nur noch in Basisklasse
    //SFX_APPWINDOW->Disable(sal_False);        //! allgemeine Methode im ScAnyRefDlg
}


//----------------------------------------------------------------------------

sal_Bool __EXPORT ScDPLayoutDlg::Close()
{
    return DoClose( ScPivotLayoutWrapper::GetChildWindowId() );
}

//----------------------------------------------------------------------------

void ScDPLayoutDlg::StateChanged( StateChangedType nStateChange )
{
    ScAnyRefDlg::StateChanged( nStateChange );

    if ( nStateChange == STATE_CHANGE_INITSHOW )
    {
        // #124828# Hiding the FixedTexts and clearing the tab stop style bits
        // has to be done after assigning the mnemonics, but Paint is too late,
        // because the test tool may send key events to the dialog when it isn't visible.
        // Mnemonics are assigned in the Dialog::StateChanged for STATE_CHANGE_INITSHOW,
        // so this can be done immediately afterwards.

        aWndPage.UseMnemonic();
        aWndCol.UseMnemonic();
        aWndRow.UseMnemonic();
        aWndData.UseMnemonic();
    }
}

//----------------------------------------------------------------------------

void ScDPLayoutDlg::InitWndSelect( const vector<ScDPLabelDataRef>& rLabels )
{
    size_t nLabelCount = rLabels.size();
    if (nLabelCount > MAX_LABELS)
        nLabelCount = MAX_LABELS;
    size_t nLast = (nLabelCount > PAGE_SIZE) ? (PAGE_SIZE - 1) : (nLabelCount - 1);

    aLabelDataArr.clear();
    aLabelDataArr.reserve( nLabelCount );
    for ( size_t i=0; i < nLabelCount; i++ )
    {
        aLabelDataArr.push_back(*rLabels[i]);

        if ( i <= nLast )
        {
            aWndSelect.AddField(aLabelDataArr[i].getDisplayName(), i);
            aSelectArr[i].reset( new ScDPFuncData( aLabelDataArr[i].mnCol, aLabelDataArr[i].mnFuncMask ) );
        }
    }
}


//----------------------------------------------------------------------------

void ScDPLayoutDlg::InitWnd( PivotField* pArr, long nCount, ScDPFieldType eType )
{
    if ( pArr && (eType != TYPE_SELECT) )
    {
        ScDPFuncDataVec*    pInitArr = NULL;
        ScDPFieldWindow*    pInitWnd = NULL;
        sal_Bool                bDataArr = sal_False;

        switch ( eType )
        {
            case TYPE_PAGE:
                pInitArr = &aPageArr;
                pInitWnd = &aWndPage;
                break;

            case TYPE_COL:
                pInitArr = &aColArr;
                pInitWnd = &aWndCol;
                break;

            case TYPE_ROW:
                pInitArr = &aRowArr;
                pInitWnd = &aWndRow;
                break;

            case TYPE_DATA:
                pInitArr = &aDataArr;
                pInitWnd = &aWndData;
                bDataArr = sal_True;
                break;
            default:
            break;
        }

        if ( pInitArr && pInitWnd )
        {
            long j=0;
            for ( long i=0; (i<nCount); i++ )
            {
                SCCOL nCol = pArr[i].nCol;
                sal_uInt16 nMask = pArr[i].nFuncMask;

                if ( nCol != PIVOT_DATA_FIELD )
                {
                    (*pInitArr)[j].reset( new ScDPFuncData( nCol, nMask, pArr[i].maFieldRef ) );

                    if ( !bDataArr )
                    {
                        pInitWnd->AddField( GetLabelString( nCol ), j );
                    }
                    else
                    {
                        ScDPLabelData* pData = GetLabelData( nCol );
                        DBG_ASSERT( pData, "ScDPLabelData not found" );
                        if (pData)
                        {
                            String aStr( GetFuncString( (*pInitArr)[j]->mnFuncMask,
                                                         pData->mbIsValue ) );

                            aStr += GetLabelString( nCol );
                            pInitWnd->AddField( aStr, j );

                            pData->mnFuncMask = nMask;
                        }
                    }
                    ++j;
                }
            }
// Do not redraw here -> first the FixedText has to get its mnemonic char
//            pInitWnd->Redraw();
        }
    }
}


//----------------------------------------------------------------------------

void ScDPLayoutDlg::InitFocus()
{
    if( aWndSelect.IsEmpty() )
    {
        aBtnOk.GrabFocus();
        NotifyFieldFocus( TYPE_SELECT, sal_False );
    }
    else
        aWndSelect.GrabFocus();
}

void ScDPLayoutDlg::InitFields()
{
    InitWndSelect(thePivotData.maLabelArray);
    InitWnd( thePivotData.aPageArr, static_cast<long>(thePivotData.nPageCount), TYPE_PAGE );
    InitWnd( thePivotData.aColArr,  static_cast<long>(thePivotData.nColCount),  TYPE_COL );
    InitWnd( thePivotData.aRowArr,  static_cast<long>(thePivotData.nRowCount),  TYPE_ROW );
    InitWnd( thePivotData.aDataArr, static_cast<long>(thePivotData.nDataCount), TYPE_DATA );

    size_t nLabels = thePivotData.maLabelArray.size();
    aSlider.SetPageSize( PAGE_SIZE );
    aSlider.SetVisibleSize( PAGE_SIZE );
    aSlider.SetLineSize( LINE_SIZE );
    aSlider.SetRange( Range( 0, static_cast<long>(((nLabels+LINE_SIZE-1)/LINE_SIZE)*LINE_SIZE) ) );

    if ( nLabels > PAGE_SIZE )
    {
        aSlider.SetEndScrollHdl( LINK( this, ScDPLayoutDlg, ScrollHdl ) );
        aSlider.Show();
    }
    else
        aSlider.Hide();
}

//----------------------------------------------------------------------------

void ScDPLayoutDlg::AddField( size_t nFromIndex, ScDPFieldType eToType, const Point& rAtPos )
{
    ScDPFuncData        fData( *(aSelectArr[nFromIndex]) );
    size_t              nAt   = 0;
    ScDPFieldWindow*    toWnd = NULL;
    ScDPFieldWindow*    rmWnd1 = NULL;
    ScDPFieldWindow*    rmWnd2 = NULL;
    ScDPFuncDataVec*    toArr = NULL;
    ScDPFuncDataVec*    rmArr1 = NULL;
    ScDPFuncDataVec*    rmArr2 = NULL;
    sal_Bool                bDataArr = sal_False;

    switch ( eToType )
    {
        case TYPE_PAGE:
            toWnd  = &aWndPage;
            rmWnd1 = &aWndRow;
            rmWnd2 = &aWndCol;
            toArr  = &aPageArr;
            rmArr1 = &aRowArr;
            rmArr2 = &aColArr;
            break;

        case TYPE_COL:
            toWnd  = &aWndCol;
            rmWnd1 = &aWndPage;
            rmWnd2 = &aWndRow;
            toArr  = &aColArr;
            rmArr1 = &aPageArr;
            rmArr2 = &aRowArr;
            break;

        case TYPE_ROW:
            toWnd  = &aWndRow;
            rmWnd1 = &aWndPage;
            rmWnd2 = &aWndCol;
            toArr  = &aRowArr;
            rmArr1 = &aPageArr;
            rmArr2 = &aColArr;
            break;

        case TYPE_DATA:
            toWnd = &aWndData;
            toArr = &aDataArr;
            bDataArr = sal_True;
            break;

        default:
        {
            // added to avoid warnings
        }
    }

    bool bAllowed = IsOrientationAllowed( fData.mnCol, eToType );
    if ( bAllowed
        && (toArr->back().get() == NULL)
        && (!Contains( toArr, fData.mnCol, nAt )) )
    {
        // ggF. in anderem Fenster entfernen
        if ( rmArr1 )
        {
            if ( Contains( rmArr1, fData.mnCol, nAt ) )
            {
                rmWnd1->DelField( nAt );
                Remove( rmArr1, nAt );
            }
        }
        if ( rmArr2 )
        {
            if ( Contains( rmArr2, fData.mnCol, nAt ) )
            {
                rmWnd2->DelField( nAt );
                Remove( rmArr2, nAt );
            }
        }

        ScDPLabelData&  rData = aLabelDataArr[nFromIndex+nOffset];
        size_t      nAddedAt = 0;

        if ( !bDataArr )
        {
            if ( toWnd->AddField( rData.getDisplayName(),
                                  DlgPos2WndPos( rAtPos, *toWnd ),
                                  nAddedAt ) )
            {
                Insert( toArr, fData, nAddedAt );
                toWnd->GrabFocus();
            }
        }
        else
        {
            sal_uInt16 nMask = fData.mnFuncMask;
            OUString aStr = GetFuncString( nMask, rData.mbIsValue );

            aStr += rData.getDisplayName();

            if ( toWnd->AddField( aStr,
                                  DlgPos2WndPos( rAtPos, *toWnd ),
                                  nAddedAt ) )
            {
                fData.mnFuncMask = nMask;
                Insert( toArr, fData, nAddedAt );
                toWnd->GrabFocus();
            }
        }

    }
}


//----------------------------------------------------------------------------

void ScDPLayoutDlg::MoveField( ScDPFieldType eFromType, size_t nFromIndex, ScDPFieldType eToType, const Point& rAtPos )
{
    if ( eFromType == TYPE_SELECT )
        AddField( nFromIndex, eToType, rAtPos );
    else if ( eFromType != eToType )
    {
        ScDPFieldWindow*    fromWnd  = NULL;
        ScDPFieldWindow*    toWnd    = NULL;
        ScDPFieldWindow*    rmWnd1   = NULL;
        ScDPFieldWindow*    rmWnd2   = NULL;
        ScDPFuncDataVec*    fromArr  = NULL;
        ScDPFuncDataVec*    toArr    = NULL;
        ScDPFuncDataVec*    rmArr1   = NULL;
        ScDPFuncDataVec*    rmArr2   = NULL;
        size_t              nAt      = 0;
        sal_Bool                bDataArr = sal_False;

        switch ( eFromType )
        {
            case TYPE_PAGE:
                fromWnd = &aWndPage;
                fromArr = &aPageArr;
                break;

            case TYPE_COL:
                fromWnd = &aWndCol;
                fromArr = &aColArr;
                break;

            case TYPE_ROW:
                fromWnd = &aWndRow;
                fromArr = &aRowArr;
                break;

            case TYPE_DATA:
                fromWnd = &aWndData;
                fromArr = &aDataArr;
                break;

            default:
            {
                // added to avoid warnings
            }
        }

        switch ( eToType )
        {
            case TYPE_PAGE:
                toWnd  = &aWndPage;
                toArr  = &aPageArr;
                rmWnd1 = &aWndCol;
                rmWnd2 = &aWndRow;
                rmArr1 = &aColArr;
                rmArr2 = &aRowArr;
                break;

            case TYPE_COL:
                toWnd  = &aWndCol;
                toArr  = &aColArr;
                rmWnd1 = &aWndPage;
                rmWnd2 = &aWndRow;
                rmArr1 = &aPageArr;
                rmArr2 = &aRowArr;
                break;

            case TYPE_ROW:
                toWnd  = &aWndRow;
                toArr  = &aRowArr;
                rmWnd1 = &aWndPage;
                rmWnd2 = &aWndCol;
                rmArr1 = &aPageArr;
                rmArr2 = &aColArr;
                break;

            case TYPE_DATA:
                toWnd = &aWndData;
                toArr = &aDataArr;
                bDataArr = sal_True;
                break;

            default:
            {
                // added to avoid warnings
            }
        }

        if ( fromArr && toArr && fromWnd && toWnd )
        {
            ScDPFuncData fData( *((*fromArr)[nFromIndex]) );

            bool bAllowed = IsOrientationAllowed( fData.mnCol, eToType );
            if ( bAllowed && Contains( fromArr, fData.mnCol, nAt ) )
            {
                fromWnd->DelField( nAt );
                Remove( fromArr, nAt );

                if (   (toArr->back().get() == NULL)
                    && (!Contains( toArr, fData.mnCol, nAt )) )
                {
                    size_t nAddedAt = 0;
                    if ( !bDataArr )
                    {
                        // ggF. in anderem Fenster entfernen
                        if ( rmArr1 )
                        {
                            if ( Contains( rmArr1, fData.mnCol, nAt ) )
                            {
                                rmWnd1->DelField( nAt );
                                Remove( rmArr1, nAt );
                            }
                        }
                        if ( rmArr2 )
                        {
                            if ( Contains( rmArr2, fData.mnCol, nAt ) )
                            {
                                rmWnd2->DelField( nAt );
                                Remove( rmArr2, nAt );
                            }
                        }

                        if ( toWnd->AddField( GetLabelString( fData.mnCol ),
                                              DlgPos2WndPos( rAtPos, *toWnd ),
                                              nAddedAt ) )
                        {
                            Insert( toArr, fData, nAddedAt );
                            toWnd->GrabFocus();
                        }
                    }
                    else
                    {
                        String aStr;
                        sal_uInt16 nMask = fData.mnFuncMask;
                        aStr  = GetFuncString( nMask );
                        aStr += GetLabelString( fData.mnCol );

                        if ( toWnd->AddField( aStr,
                                              DlgPos2WndPos( rAtPos, *toWnd ),
                                              nAddedAt ) )
                        {
                            fData.mnFuncMask = nMask;
                            Insert( toArr, fData, nAddedAt );
                            toWnd->GrabFocus();
                        }
                    }
                }
            }
        }
    }
    else // -> eFromType == eToType
    {
        ScDPFieldWindow*    theWnd  = NULL;
        ScDPFuncDataVec*    theArr   = NULL;
        size_t              nAt      = 0;
        size_t              nToIndex = 0;
        Point               aToPos;
        sal_Bool                bDataArr = sal_False;

        switch ( eFromType )
        {
            case TYPE_PAGE:
                theWnd = &aWndPage;
                theArr = &aPageArr;
                break;

            case TYPE_COL:
                theWnd = &aWndCol;
                theArr = &aColArr;
                break;

            case TYPE_ROW:
                theWnd = &aWndRow;
                theArr = &aRowArr;
                break;

            case TYPE_DATA:
                theWnd = &aWndData;
                theArr = &aDataArr;
                bDataArr = sal_True;
                break;

            default:
            {
                // added to avoid warnings
            }
        }

        ScDPFuncData fData( *((*theArr)[nFromIndex]) );

        if ( Contains( theArr, fData.mnCol, nAt ) )
        {
            aToPos = DlgPos2WndPos( rAtPos, *theWnd );
            theWnd->GetExistingIndex( aToPos, nToIndex );

            if ( nToIndex != nAt )
            {
                size_t nAddedAt = 0;

                theWnd->DelField( nAt );
                Remove( theArr, nAt );

                if ( !bDataArr )
                {
                    if ( theWnd->AddField( GetLabelString( fData.mnCol ),
                                           aToPos,
                                           nAddedAt ) )
                    {
                        Insert( theArr, fData, nAddedAt );
                    }
                }
                else
                {
                    String aStr;
                    sal_uInt16 nMask = fData.mnFuncMask;
                    aStr  = GetFuncString( nMask );
                    aStr += GetLabelString( fData.mnCol );

                    if ( theWnd->AddField( aStr,
                                           DlgPos2WndPos( rAtPos, *theWnd ),
                                           nAddedAt ) )
                    {
                        fData.mnFuncMask = nMask;
                        Insert( theArr, fData, nAddedAt );
                    }
                }
            }
        }
    }
}

//----------------------------------------------------------------------------

void ScDPLayoutDlg::RemoveField( ScDPFieldType eFromType, size_t nIndex )
{
    ScDPFuncDataVec* pArr = NULL;
    switch( eFromType )
    {
        case TYPE_PAGE: pArr = &aPageArr;    break;
        case TYPE_COL:  pArr = &aColArr;     break;
        case TYPE_ROW:  pArr = &aRowArr;     break;
        case TYPE_DATA: pArr = &aDataArr;    break;
        default:
        {
            // added to avoid warnings
        }
    }

    if( pArr )
    {
        ScDPFieldWindow& rWnd = GetFieldWindow( eFromType );
        rWnd.DelField( nIndex );
        Remove( pArr, nIndex );
        if( rWnd.IsEmpty() ) InitFocus();
    }
}

//----------------------------------------------------------------------------

void ScDPLayoutDlg::NotifyMouseButtonUp( const Point& rAt )
{
    if ( bIsDrag )
    {
        bIsDrag = sal_False;

        ScDPFieldType   eDnDToType = TYPE_SELECT;
        Point           aPos = ScreenToOutputPixel( rAt );
        sal_Bool            bDel = sal_False;

        if ( aRectPage.IsInside( aPos ) )
        {
            eDnDToType = TYPE_PAGE;
            bDel = sal_False;
        }
        else if ( aRectCol.IsInside( aPos ) )
        {
            eDnDToType = TYPE_COL;
            bDel = sal_False;
        }
        else if ( aRectRow.IsInside( aPos ) )
        {
            eDnDToType = TYPE_ROW;
            bDel = sal_False;
        }
        else if ( aRectData.IsInside( aPos ) )
        {
            eDnDToType = TYPE_DATA;
            bDel = sal_False;
        }
        else if ( aRectSelect.IsInside( aPos ) )
        {
            eDnDToType = TYPE_SELECT;
            bDel = sal_True;
        }
        else
            bDel = sal_True;

        if ( bDel )
            RemoveField( eDnDFromType, nDnDFromIndex );
        else
            MoveField( eDnDFromType, nDnDFromIndex, eDnDToType, aPos );
    }
}


//----------------------------------------------------------------------------

PointerStyle ScDPLayoutDlg::NotifyMouseMove( const Point& rAt )
{
    PointerStyle ePtr = POINTER_ARROW;

    if ( bIsDrag )
    {
        Point aPos = ScreenToOutputPixel( rAt );
        ScDPFieldType eCheckTarget = TYPE_SELECT;

        if ( aRectPage.IsInside( aPos ) )
            eCheckTarget = TYPE_PAGE;
        else if ( aRectCol.IsInside( aPos ) )
            eCheckTarget = TYPE_COL;
        else if ( aRectRow.IsInside( aPos ) )
            eCheckTarget = TYPE_ROW;
        else if ( aRectData.IsInside( aPos ) )
            eCheckTarget = TYPE_DATA;
        else if ( eDnDFromType != TYPE_SELECT )
            ePtr = POINTER_PIVOT_DELETE;
        else if ( aRectSelect.IsInside( aPos ) )
            ePtr = lclGetPointerForField( TYPE_SELECT );
        else
            ePtr = POINTER_NOTALLOWED;

        if ( eCheckTarget != TYPE_SELECT )
        {
            // check if the target orientation is allowed for this field
            ScDPFuncDataVec* fromArr = NULL;
            switch ( eDnDFromType )
            {
                case TYPE_PAGE:   fromArr = &aPageArr;   break;
                case TYPE_COL:    fromArr = &aColArr;    break;
                case TYPE_ROW:    fromArr = &aRowArr;    break;
                case TYPE_DATA:   fromArr = &aDataArr;   break;
                case TYPE_SELECT: fromArr = &aSelectArr; break;
            }
            ScDPFuncData fData( *((*fromArr)[nDnDFromIndex]) );
            if (IsOrientationAllowed( fData.mnCol, eCheckTarget ))
                ePtr = lclGetPointerForField( eCheckTarget );
            else
                ePtr = POINTER_NOTALLOWED;
        }
    }

    return ePtr;
}


//----------------------------------------------------------------------------

PointerStyle ScDPLayoutDlg::NotifyMouseButtonDown( ScDPFieldType eType, size_t nFieldIndex )
{
    bIsDrag       = sal_True;
    eDnDFromType  = eType;
    nDnDFromIndex = nFieldIndex;
    return lclGetPointerForField( eType );
}


//----------------------------------------------------------------------------

void ScDPLayoutDlg::NotifyDoubleClick( ScDPFieldType eType, size_t nFieldIndex )
{
    ScDPFuncDataVec* pArr = NULL;
    switch ( eType )
    {
        case TYPE_PAGE:     pArr = &aPageArr;   break;
        case TYPE_COL:      pArr = &aColArr;    break;
        case TYPE_ROW:      pArr = &aRowArr;    break;
        case TYPE_DATA:     pArr = &aDataArr;   break;
        default:
        {
            // added to avoid warnings
        }
    }

    if ( pArr )
    {
        if ( nFieldIndex >= pArr->size() )
        {
            DBG_ERROR("invalid selection");
            return;
        }

        size_t nArrPos = 0;
        if( ScDPLabelData* pData = GetLabelData( (*pArr)[nFieldIndex]->mnCol, &nArrPos ) )
        {
            ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
            DBG_ASSERT(pFact, "ScAbstractFactory create fail!");//CHINA001

            switch ( eType )
            {
                case TYPE_PAGE:
                case TYPE_COL:
                case TYPE_ROW:
                {
                    // list of names of all data fields
                    std::vector< String > aDataFieldNames;
                    for( ScDPFuncDataVec::const_iterator aIt = aDataArr.begin(), aEnd = aDataArr.end();
                            (aIt != aEnd) && aIt->get(); ++aIt )
                    {
                        String aName( GetLabelString( (*aIt)->mnCol ) );
                        if( aName.Len() )
                            aDataFieldNames.push_back( aName );
                    }

                    bool bLayout = (eType == TYPE_ROW) &&
                        ((aDataFieldNames.size() > 1) || ((nFieldIndex + 1 < pArr->size()) && (*pArr)[nFieldIndex+1].get()));

                    AbstractScDPSubtotalDlg* pDlg = pFact->CreateScDPSubtotalDlg(
                        this, RID_SCDLG_PIVOTSUBT,
                        *xDlgDPObject, *pData, *(*pArr)[nFieldIndex], aDataFieldNames, bLayout );

                    if ( pDlg->Execute() == RET_OK )
                    {
                        pDlg->FillLabelData( *pData );
                        (*pArr)[nFieldIndex]->mnFuncMask = pData->mnFuncMask;
                    }
                    delete pDlg;
                }
                break;

                case TYPE_DATA:
                {
                    AbstractScDPFunctionDlg* pDlg = pFact->CreateScDPFunctionDlg(
                        this, RID_SCDLG_DPDATAFIELD,
                        aLabelDataArr, *pData, *(*pArr)[nFieldIndex] );

                    if ( pDlg->Execute() == RET_OK )
                    {
                        (*pArr)[nFieldIndex]->mnFuncMask = pData->mnFuncMask = pDlg->GetFuncMask();
                        (*pArr)[nFieldIndex]->maFieldRef = pDlg->GetFieldRef();

                        String aStr( GetFuncString ( aDataArr[nFieldIndex]->mnFuncMask ) );
                        aStr += GetLabelString( aDataArr[nFieldIndex]->mnCol );
                        aWndData.SetFieldText( aStr, nFieldIndex );
                    }
                    delete pDlg;
                }
                break;

                default:
                {
                    // added to avoid warnings
                }
            }
        }
    }
}

//----------------------------------------------------------------------------

void ScDPLayoutDlg::NotifyFieldFocus( ScDPFieldType eType, sal_Bool bGotFocus )
{
    /*  Enable Remove/Options buttons on GetFocus in field window.
        #107616# Enable them also, if dialog is deactivated (click into document).
        The !IsActive() condition handles the case that a LoseFocus event of a
        field window would follow the Deactivate event of this dialog. */
    sal_Bool bEnable = (bGotFocus || !IsActive()) && (eType != TYPE_SELECT);

    // #128113# The TestTool may set the focus into an empty field.
    // Then the Remove/Options buttons must be disabled.
    if ( bEnable && bGotFocus && GetFieldWindow( eType ).IsEmpty() )
        bEnable = sal_False;

    aBtnRemove.Enable( bEnable );
    aBtnOptions.Enable( bEnable );
    if( bGotFocus )
        eLastActiveType = eType;
}

//----------------------------------------------------------------------------

void ScDPLayoutDlg::NotifyMoveField( ScDPFieldType eToType )
{
    ScDPFieldWindow& rWnd = GetFieldWindow( eLastActiveType );
    if( (eToType != TYPE_SELECT) && !rWnd.IsEmpty() )
    {
        MoveField( eLastActiveType, rWnd.GetSelectedField(), eToType, GetFieldWindow( eToType ).GetLastPosition() );
        if( rWnd.IsEmpty() )
            NotifyFieldFocus( eToType, sal_True );
        else
            rWnd.GrabFocus();
        if( eLastActiveType == TYPE_SELECT )
            aWndSelect.SelectNext();
    }
    else
        InitFocus();
}

//----------------------------------------------------------------------------

void ScDPLayoutDlg::NotifyRemoveField( ScDPFieldType eType, size_t nFieldIndex )
{
    if( eType != TYPE_SELECT )
        RemoveField( eType, nFieldIndex );
}

//----------------------------------------------------------------------------

sal_Bool ScDPLayoutDlg::NotifyMoveSlider( sal_uInt16 nKeyCode )
{
    long nOldPos = aSlider.GetThumbPos();
    switch( nKeyCode )
    {
        case KEY_HOME:  aSlider.DoScroll( 0 );                      break;
        case KEY_END:   aSlider.DoScroll( aSlider.GetRangeMax() );  break;
        case KEY_UP:
        case KEY_LEFT:  aSlider.DoScrollAction( SCROLL_LINEUP );    break;
        case KEY_DOWN:
        case KEY_RIGHT: aSlider.DoScrollAction( SCROLL_LINEDOWN );  break;
    }
    return nOldPos != aSlider.GetThumbPos();
}

//----------------------------------------------------------------------------

void ScDPLayoutDlg::Deactivate()
{
    /*  #107616# If the dialog has been deactivated (click into document), the LoseFocus
        event from field window disables Remove/Options buttons. Re-enable them here by
        simulating a GetFocus event. Event order of LoseFocus and Deactivate is not important.
        The last event will enable the buttons in both cases (see NotifyFieldFocus). */
    NotifyFieldFocus( eLastActiveType, sal_True );
}

//----------------------------------------------------------------------------

sal_Bool ScDPLayoutDlg::Contains( ScDPFuncDataVec* pArr, SCsCOL nCol, size_t& nAt )
{
    if ( !pArr )
        return sal_False;

    sal_Bool    bFound  = sal_False;
    size_t  i       = 0;

    while ( (i<pArr->size()) && ((*pArr)[i].get() != NULL) && !bFound )
    {
        bFound = ((*pArr)[i]->mnCol == nCol);
        if ( bFound )
            nAt = i;
        i++;
    }

    return bFound;
}


//----------------------------------------------------------------------------

void ScDPLayoutDlg::Remove( ScDPFuncDataVec* pArr, size_t nAt )
{
    if ( !pArr || (nAt>=pArr->size()) )
        return;

    pArr->erase( pArr->begin() + nAt );
    pArr->push_back( ScDPFuncDataRef() );
}


//----------------------------------------------------------------------------

void ScDPLayoutDlg::Insert( ScDPFuncDataVec* pArr, const ScDPFuncData& rFData, size_t nAt )
{
    if ( !pArr || (nAt>=pArr->size()) )
        return;

    if ( (*pArr)[nAt].get() == NULL )
    {
        (*pArr)[nAt].reset( new ScDPFuncData( rFData ) );
    }
    else
    {
        if ( pArr->back().get() == NULL ) // mind. ein Slot frei?
        {
            pArr->insert( pArr->begin() + nAt, ScDPFuncDataRef( new ScDPFuncData( rFData ) ) );
            pArr->erase( pArr->end() - 1 );
        }
    }
}


//----------------------------------------------------------------------------

ScDPLabelData* ScDPLayoutDlg::GetLabelData( SCsCOL nCol, size_t* pnPos )
{
    ScDPLabelData* pData = 0;
    for( ScDPLabelDataVec::iterator aIt = aLabelDataArr.begin(), aEnd = aLabelDataArr.end(); !pData && (aIt != aEnd); ++aIt )
    {
        if( aIt->mnCol == nCol )
        {
            pData = &*aIt;
            if( pnPos ) *pnPos = aIt - aLabelDataArr.begin();
        }
    }
    return pData;
}


//----------------------------------------------------------------------------

String ScDPLayoutDlg::GetLabelString( SCsCOL nCol )
{
    ScDPLabelData* pData = GetLabelData( nCol );
    DBG_ASSERT( pData, "LabelData not found" );
    if (pData)
        return pData->getDisplayName();
    return String();
}

//----------------------------------------------------------------------------

bool ScDPLayoutDlg::IsOrientationAllowed( SCsCOL nCol, ScDPFieldType eType )
{
    bool bAllowed = true;
    ScDPLabelData* pData = GetLabelData( nCol );
    DBG_ASSERT( pData, "LabelData not found" );
    if (pData)
    {
        sheet::DataPilotFieldOrientation eOrient = sheet::DataPilotFieldOrientation_HIDDEN;
        switch (eType)
        {
            case TYPE_PAGE:   eOrient = sheet::DataPilotFieldOrientation_PAGE;   break;
            case TYPE_COL:    eOrient = sheet::DataPilotFieldOrientation_COLUMN; break;
            case TYPE_ROW:    eOrient = sheet::DataPilotFieldOrientation_ROW;    break;
            case TYPE_DATA:   eOrient = sheet::DataPilotFieldOrientation_DATA;   break;
            case TYPE_SELECT: eOrient = sheet::DataPilotFieldOrientation_HIDDEN; break;
        }
        bAllowed = ScDPObject::IsOrientationAllowed( (sal_uInt16)eOrient, pData->mnFlags );
    }
    return bAllowed;
}

//----------------------------------------------------------------------------

String ScDPLayoutDlg::GetFuncString( sal_uInt16& rFuncMask, sal_Bool bIsValue )
{
    String aStr;

    if (   rFuncMask == PIVOT_FUNC_NONE
        || rFuncMask == PIVOT_FUNC_AUTO )
    {
        if ( bIsValue )
        {
            aStr = FSTR(PIVOTSTR_SUM);
            rFuncMask = PIVOT_FUNC_SUM;
        }
        else
        {
            aStr = FSTR(PIVOTSTR_COUNT);
            rFuncMask = PIVOT_FUNC_COUNT;
        }
    }
    else if ( rFuncMask == PIVOT_FUNC_SUM )       aStr = FSTR(PIVOTSTR_SUM);
    else if ( rFuncMask == PIVOT_FUNC_COUNT )     aStr = FSTR(PIVOTSTR_COUNT);
    else if ( rFuncMask == PIVOT_FUNC_AVERAGE )   aStr = FSTR(PIVOTSTR_AVG);
    else if ( rFuncMask == PIVOT_FUNC_MAX )       aStr = FSTR(PIVOTSTR_MAX);
    else if ( rFuncMask == PIVOT_FUNC_MIN )       aStr = FSTR(PIVOTSTR_MIN);
    else if ( rFuncMask == PIVOT_FUNC_PRODUCT )   aStr = FSTR(PIVOTSTR_PROD);
    else if ( rFuncMask == PIVOT_FUNC_COUNT_NUM ) aStr = FSTR(PIVOTSTR_COUNT2);
    else if ( rFuncMask == PIVOT_FUNC_STD_DEV )   aStr = FSTR(PIVOTSTR_DEV);
    else if ( rFuncMask == PIVOT_FUNC_STD_DEVP )  aStr = FSTR(PIVOTSTR_DEV2);
    else if ( rFuncMask == PIVOT_FUNC_STD_VAR )   aStr = FSTR(PIVOTSTR_VAR);
    else if ( rFuncMask == PIVOT_FUNC_STD_VARP )  aStr = FSTR(PIVOTSTR_VAR2);
    else
    {
        aStr  = ScGlobal::GetRscString( STR_TABLE_ERGEBNIS );
        aStr.AppendAscii(RTL_CONSTASCII_STRINGPARAM( " - " ));
    }

    return aStr;
}


//----------------------------------------------------------------------------

Point ScDPLayoutDlg::DlgPos2WndPos( const Point& rPt, Window& rWnd )
{
    Point aWndPt( rPt );
    aWndPt.X() = rPt.X()-rWnd.GetPosPixel().X();
    aWndPt.Y() = rPt.Y()-rWnd.GetPosPixel().Y();

    return aWndPt;
}


//----------------------------------------------------------------------------

void ScDPLayoutDlg::CalcWndSizes()
{
    // row/column/data area sizes
    aWndPage.SetSizePixel( Size( MAX_PAGEFIELDS * OWIDTH / 2, 2 * OHEIGHT ) );
    aWndRow.SetSizePixel( Size( OWIDTH, MAX_FIELDS * OHEIGHT ) );
    aWndCol.SetSizePixel( Size( MAX_FIELDS * OWIDTH / 2, 2 * OHEIGHT ) );
    aWndData.SetSizePixel( Size( MAX_FIELDS * OWIDTH / 2, MAX_FIELDS * OHEIGHT ) );

    // #i29203# align right border of page window with data window
    long nDataPosX = aWndData.GetPosPixel().X() + aWndData.GetSizePixel().Width();
    aWndPage.SetPosPixel( Point( nDataPosX - aWndPage.GetSizePixel().Width(), aWndPage.GetPosPixel().Y() ) );

    // selection area
    aWndSelect.SetSizePixel( Size(
        2 * OWIDTH + SSPACE, LINE_SIZE * OHEIGHT + (LINE_SIZE - 1) * SSPACE ) );

    // scroll bar
    Point aSliderPos( aWndSelect.GetPosPixel() );
    Size aSliderSize( aWndSelect.GetSizePixel() );
    aSliderPos.Y() += aSliderSize.Height() + SSPACE;
    aSliderSize.Height() = GetSettings().GetStyleSettings().GetScrollBarSize();
    aSlider.SetPosSizePixel( aSliderPos, aSliderSize );

    aRectPage   = Rectangle( aWndPage.GetPosPixel(),    aWndPage.GetSizePixel() );
    aRectRow    = Rectangle( aWndRow.GetPosPixel(),     aWndRow.GetSizePixel() );
    aRectCol    = Rectangle( aWndCol.GetPosPixel(),     aWndCol.GetSizePixel() );
    aRectData   = Rectangle( aWndData.GetPosPixel(),    aWndData.GetSizePixel() );
    aRectSelect = Rectangle( aWndSelect.GetPosPixel(),  aWndSelect.GetSizePixel() );
}


//----------------------------------------------------------------------------

sal_Bool ScDPLayoutDlg::GetPivotArrays(    PivotField*  pPageArr,
                                       PivotField*  pColArr,
                                       PivotField*  pRowArr,
                                       PivotField*  pDataArr,
                                       sal_uInt16&      rPageCount,
                                       sal_uInt16&      rColCount,
                                       sal_uInt16&      rRowCount,
                                       sal_uInt16&      rDataCount )
{
    sal_Bool bFit = sal_True;
    sal_uInt16 i=0;

    for ( i=0; (i<aDataArr.size()) && (aDataArr[i].get() != NULL ); i++ )
        lcl_FillToPivotField( pDataArr[i], *aDataArr[i] );
    rDataCount = i;

    for ( i=0; (i<aPageArr.size()) && (aPageArr[i].get() != NULL ); i++ )
        lcl_FillToPivotField( pPageArr[i], *aPageArr[i] );
    rPageCount = i;

    for ( i=0; (i<aColArr.size()) && (aColArr[i].get() != NULL ); i++ )
        lcl_FillToPivotField( pColArr[i], *aColArr[i] );
    rColCount = i;

    for ( i=0; (i<aRowArr.size()) && (aRowArr[i].get() != NULL ); i++ )
        lcl_FillToPivotField( pRowArr[i], *aRowArr[i] );
    rRowCount = i;

    if ( rRowCount < aRowArr.size() )
        pRowArr[rRowCount++].nCol = PIVOT_DATA_FIELD;
    else if ( rColCount < aColArr.size() )
        pColArr[rColCount++].nCol = PIVOT_DATA_FIELD;
    else
        bFit = sal_False;       // kein Platz fuer Datenfeld

    return bFit;
}

void ScDPLayoutDlg::UpdateSrcRange()
{
    String  theCurPosStr = aEdInPos.GetText();
    sal_uInt16  nResult = ScRange().Parse(theCurPosStr, pDoc, pDoc->GetAddressConvention());

    if ( SCA_VALID != (nResult & SCA_VALID) )
        // invalid source range.
        return;

    ScRefAddress start, end;
    ConvertDoubleRef(pDoc, theCurPosStr, 1,  start, end, pDoc->GetAddressConvention());
    ScRange aNewRange(start.GetAddress(), end.GetAddress());
    ScSheetSourceDesc inSheet = *xDlgDPObject->GetSheetDesc();

    if (inSheet.aSourceRange == aNewRange)
        // new range is identical to the current range.  Nothing to do.
        return;

    ScTabViewShell * pTabViewShell = pViewData->GetViewShell();
    inSheet.aSourceRange = aNewRange;
    xDlgDPObject->SetSheetDesc(inSheet);
    xDlgDPObject->FillOldParam( thePivotData, sal_False );
    xDlgDPObject->FillLabelData(thePivotData);

    pTabViewShell->SetDialogDPObject(xDlgDPObject.get());
    aLabelDataArr.clear();
    aWndSelect.ClearFields();
    aWndData.ClearFields();
    aWndRow.ClearFields();
    aWndCol.ClearFields();
    aWndPage.ClearFields();

    for (size_t i = 0; i < MAX_LABELS; ++i)
        aSelectArr[i].reset();

    for (size_t i = 0; i < MAX_FIELDS; ++i)
    {
        aRowArr[i].reset();
        aColArr[i].reset();
        aDataArr[i].reset();
    }

    for (size_t i = 0; i < MAX_PAGEFIELDS; ++i)
        aPageArr[i].reset();

    InitFields();
}

//----------------------------------------------------------------------------

void ScDPLayoutDlg::SetReference( const ScRange& rRef, ScDocument* pDocP )
{
    if ( !bRefInputMode || !pEditActive )
        return;

    if ( rRef.aStart != rRef.aEnd )
        RefInputStart( pEditActive );

    if ( pEditActive == &aEdInPos )
    {
        String aRefStr;
        rRef.Format( aRefStr, SCR_ABS_3D, pDocP, pDocP->GetAddressConvention() );
        pEditActive->SetRefString( aRefStr );
    }
    else if ( pEditActive == &aEdOutPos )
    {
        String aRefStr;
        rRef.aStart.Format( aRefStr, STD_FORMAT, pDocP, pDocP->GetAddressConvention() );
        pEditActive->SetRefString( aRefStr );
    }
}


//----------------------------------------------------------------------------

void ScDPLayoutDlg::SetActive()
{
    if ( bRefInputMode )
    {
        if ( pEditActive )
            pEditActive->GrabFocus();

        if ( pEditActive == &aEdInPos )
            EdInModifyHdl( NULL );
        else if ( pEditActive == &aEdOutPos )
        EdModifyHdl( NULL );
    }
    else
    {
        GrabFocus();
    }

    RefInputDone();
}

//----------------------------------------------------------------------------
// Handler:
//----------------------------------------------------------------------------

IMPL_LINK( ScDPLayoutDlg, ClickHdl, PushButton *, pBtn )
{
    if( pBtn == &aBtnRemove )
    {
        ScDPFieldWindow& rWnd = GetFieldWindow( eLastActiveType );
        RemoveField( eLastActiveType, rWnd.GetSelectedField() );
        if( !rWnd.IsEmpty() ) rWnd.GrabFocus();
    }
    else if( pBtn == &aBtnOptions )
    {
        ScDPFieldWindow& rWnd = GetFieldWindow( eLastActiveType );
        NotifyDoubleClick( eLastActiveType, rWnd.GetSelectedField() );
        rWnd.GrabFocus();
    }
    return 0;
}

//----------------------------------------------------------------------------

IMPL_LINK( ScDPLayoutDlg, OkHdl, OKButton *, EMPTYARG )
{
    String      aOutPosStr( aEdOutPos.GetText() );
    ScAddress   aAdrDest;
    sal_Bool        bToNewTable = (aLbOutPos.GetSelectEntryPos() == 1);
    sal_uInt16      nResult     = !bToNewTable ? aAdrDest.Parse( aOutPosStr, pDoc, pDoc->GetAddressConvention() ) : 0;

    if (   bToNewTable
        || ( (aOutPosStr.Len() > 0) && (SCA_VALID == (nResult & SCA_VALID)) ) )
    {
        //@BugID 54702 Enablen/Disablen nur noch in Basisklasse
        //SFX_APPWINDOW->Enable();

        ScPivotParam    theOutParam;
        PivotPageFieldArr aPageFieldArr;
        PivotFieldArr   aColFieldArr;
        PivotFieldArr   aRowFieldArr;
        PivotFieldArr   aDataFieldArr;
        sal_uInt16          nPageCount;
        sal_uInt16          nColCount;
        sal_uInt16          nRowCount;
        sal_uInt16          nDataCount;

        sal_Bool bFit = GetPivotArrays( aPageFieldArr, aColFieldArr, aRowFieldArr, aDataFieldArr,
                                    nPageCount,    nColCount,    nRowCount,    nDataCount );
        if ( bFit )
        {
            ScDPSaveData* pOldSaveData = xDlgDPObject->GetSaveData();

            ScRange aOutRange( aAdrDest );      // bToNewTable is passed separately

            ScDPSaveData aSaveData;
            aSaveData.SetIgnoreEmptyRows( aBtnIgnEmptyRows.IsChecked() );
            aSaveData.SetRepeatIfEmpty( aBtnDetectCat.IsChecked() );
            aSaveData.SetColumnGrand( aBtnTotalCol.IsChecked() );
            aSaveData.SetRowGrand( aBtnTotalRow.IsChecked() );
            aSaveData.SetFilterButton( aBtnFilter.IsChecked() );
            aSaveData.SetDrillDown( aBtnDrillDown.IsChecked() );

            uno::Reference<sheet::XDimensionsSupplier> xSource = xDlgDPObject->GetSource();

            ScDPObject::ConvertOrientation( aSaveData, aPageFieldArr, nPageCount,
                            sheet::DataPilotFieldOrientation_PAGE,   NULL, 0, 0, xSource, sal_False );
            ScDPObject::ConvertOrientation( aSaveData, aColFieldArr,  nColCount,
                            sheet::DataPilotFieldOrientation_COLUMN, NULL, 0, 0, xSource, sal_False );
            ScDPObject::ConvertOrientation( aSaveData, aRowFieldArr,  nRowCount,
                            sheet::DataPilotFieldOrientation_ROW,    NULL, 0, 0, xSource, sal_False );
            ScDPObject::ConvertOrientation( aSaveData, aDataFieldArr, nDataCount,
                            sheet::DataPilotFieldOrientation_DATA,   NULL, 0, 0, xSource, sal_False,
                            aColFieldArr, nColCount, aRowFieldArr, nRowCount, aPageFieldArr, nPageCount );

            for( ScDPLabelDataVec::const_iterator aIt = aLabelDataArr.begin(), aEnd = aLabelDataArr.end(); aIt != aEnd; ++aIt )
            {
                if( ScDPSaveDimension* pDim = aSaveData.GetExistingDimensionByName( aIt->maName ) )
                {
                    pDim->SetUsedHierarchy( aIt->mnUsedHier );
                    pDim->SetShowEmpty( aIt->mbShowAll );
                    pDim->SetSortInfo( &aIt->maSortInfo );
                    pDim->SetLayoutInfo( &aIt->maLayoutInfo );
                    pDim->SetAutoShowInfo( &aIt->maShowInfo );
                    ScDPSaveDimension* pOldDim = NULL;
                    if (pOldSaveData)
                    {
                        // Transfer the existing layout names to new dimension instance.
                        pOldDim = pOldSaveData->GetExistingDimensionByName(aIt->maName);
                        if (pOldDim)
                        {
                            const OUString* pLayoutName = pOldDim->GetLayoutName();
                            if (pLayoutName)
                                pDim->SetLayoutName(*pLayoutName);

                            const OUString* pSubtotalName = pOldDim->GetSubtotalName();
                            if (pSubtotalName)
                                pDim->SetSubtotalName(*pSubtotalName);
                        }
                    }

                    bool bManualSort = ( aIt->maSortInfo.Mode == sheet::DataPilotFieldSortMode::MANUAL );

                    // visibility of members
                    for (vector<ScDPLabelData::Member>::const_iterator itr = aIt->maMembers.begin(), itrEnd = aIt->maMembers.end();
                          itr != itrEnd; ++itr)
                    {
                        ScDPSaveMember* pMember = pDim->GetMemberByName(itr->maName);

                        // #i40054# create/access members only if flags are not default
                        // (or in manual sorting mode - to keep the order)
                        if (bManualSort || !itr->mbVisible || !itr->mbShowDetails)
                        {
                            pMember->SetIsVisible(itr->mbVisible);
                            pMember->SetShowDetails(itr->mbShowDetails);
                        }
                        if (pOldDim)
                        {
                            // Transfer the existing layout name.
                            ScDPSaveMember* pOldMember = pOldDim->GetMemberByName(itr->maName);
                            if (pOldMember)
                            {
                                const OUString* pLayoutName = pOldMember->GetLayoutName();
                                if (pLayoutName)
                                    pMember->SetLayoutName(*pLayoutName);
                            }
                        }
                    }
                }
            }
            ScDPSaveDimension* pDim = aSaveData.GetDataLayoutDimension();
            if (pDim && pOldSaveData)
            {
                ScDPSaveDimension* pOldDim = pOldSaveData->GetDataLayoutDimension();
                if (pOldDim)
                {
                    const OUString* pLayoutName = pOldDim->GetLayoutName();
                    if (pLayoutName)
                        pDim->SetLayoutName(*pLayoutName);
                }
            }

            sal_uInt16 nWhichPivot = SC_MOD()->GetPool().GetWhich( SID_PIVOT_TABLE );
            ScPivotItem aOutItem( nWhichPivot, &aSaveData, &aOutRange, bToNewTable );

            bRefInputMode = sal_False;      // to allow deselecting when switching sheets

            SetDispatcherLock( sal_False );
            SwitchToDocument();

            //  #95513# don't hide the dialog before executing the slot, instead it is used as
            //  parent for message boxes in ScTabViewShell::GetDialogParent

            const SfxPoolItem* pRet = GetBindings().GetDispatcher()->Execute(
                SID_PIVOT_TABLE, SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD, &aOutItem, 0L, 0L );

            bool bSuccess = true;
            if (pRet)
            {
                const SfxBoolItem* pItem = dynamic_cast<const SfxBoolItem*>(pRet);
                if (pItem)
                    bSuccess = pItem->GetValue();
            }
            if (bSuccess)
                // Table successfully inserted.
                Close();
            else
            {
                // Table insertion failed.  Keep the dialog open.
                bRefInputMode = true;
                SetDispatcherLock(true);
            }
        }
        else
        {
            ErrorBox( this, WinBits( WB_OK | WB_DEF_OK ),
                     ScGlobal::GetRscString( STR_PIVOT_ERROR )
                    ).Execute();
        }
    }
    else
    {
        if ( !aBtnMore.GetState() )
            aBtnMore.SetState( sal_True );

        ErrorBox( this, WinBits( WB_OK | WB_DEF_OK ),
                 ScGlobal::GetRscString( STR_INVALID_TABREF )
                ).Execute();
        aEdOutPos.GrabFocus();
    }
    return 0;
}


//----------------------------------------------------------------------------

IMPL_LINK( ScDPLayoutDlg, CancelHdl, CancelButton *, EMPTYARG )
{
    Close();
    return 0;
}


//----------------------------------------------------------------------------

IMPL_LINK( ScDPLayoutDlg, MoreClickHdl, MoreButton *, EMPTYARG )
{
    if ( aBtnMore.GetState() )
    {
        bRefInputMode = sal_True;
        //@BugID 54702 Enablen/Disablen nur noch in Basisklasse
        //SFX_APPWINDOW->Enable();
        if ( aEdInPos.IsEnabled() )
        {
            aEdInPos.Enable();
            aEdInPos.GrabFocus();
            aEdInPos.Enable();
        }
        else
        {
        aEdOutPos.Enable();
        aEdOutPos.GrabFocus();
            aEdOutPos.Enable();
        }
    }
    else
    {
        bRefInputMode = sal_False;
        //@BugID 54702 Enablen/Disablen nur noch in Basisklasse
        //SFX_APPWINDOW->Disable(sal_False);        //! allgemeine Methode im ScAnyRefDlg
    }
    return 0;
}


//----------------------------------------------------------------------------

IMPL_LINK( ScDPLayoutDlg, EdModifyHdl, Edit *, EMPTYARG )
{
    String  theCurPosStr = aEdOutPos.GetText();
    sal_uInt16  nResult = ScAddress().Parse( theCurPosStr, pDoc, pDoc->GetAddressConvention() );

    if ( SCA_VALID == (nResult & SCA_VALID) )
    {
        String* pStr    = NULL;
        sal_Bool    bFound  = sal_False;
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
    return 0;
}


IMPL_LINK( ScDPLayoutDlg, EdInModifyHdl, Edit *, EMPTYARG )
{
    UpdateSrcRange();
    return 0;
}


//----------------------------------------------------------------------------

IMPL_LINK( ScDPLayoutDlg, SelAreaHdl, ListBox *, EMPTYARG )
{
    String  aString;
    sal_uInt16  nSelPos = aLbOutPos.GetSelectEntryPos();

    if ( nSelPos > 1 )
    {
        aString = *(String*)aLbOutPos.GetEntryData( nSelPos );
    }
    else if ( nSelPos == aLbOutPos.GetEntryCount()-1 ) // auf neue Tabelle?
    {
        aEdOutPos.Disable();
        aRbOutPos.Disable();
    }
    else
    {
        aEdOutPos.Enable();
        aRbOutPos.Enable();
    }

    aEdOutPos.SetText( aString );
    return 0;
}


//----------------------------------------------------------------------------

IMPL_LINK( ScDPLayoutDlg, ScrollHdl, ScrollBar *, EMPTYARG )
{
    long nNewOffset = aSlider.GetThumbPos();
    long nOffsetDiff = nNewOffset - nOffset;
    nOffset = nNewOffset;

    size_t nFields = std::min< size_t >( aLabelDataArr.size() - nOffset, PAGE_SIZE );

    aWndSelect.ClearFields();

    size_t i=0;
    for ( i=0; i<nFields; i++ )
    {
        const ScDPLabelData& rData = aLabelDataArr[nOffset+i];
        aWndSelect.AddField(rData.getDisplayName(), i);
        aSelectArr[i].reset( new ScDPFuncData( rData.mnCol, rData.mnFuncMask ) );
    }
    for ( ; i<aSelectArr.size(); i++ )
        aSelectArr[i].reset();

    aWndSelect.ModifySelectionOffset( nOffsetDiff );    // adjusts selection & redraws
    return 0;
}

//----------------------------------------------------------------------------

IMPL_LINK( ScDPLayoutDlg, GetFocusHdl, Control*, pCtrl )
{
    pEditActive = NULL;
    if ( pCtrl == &aEdInPos )
        pEditActive = &aEdInPos;
    else if ( pCtrl == &aEdOutPos )
        pEditActive = &aEdOutPos;

    return 0;
}

