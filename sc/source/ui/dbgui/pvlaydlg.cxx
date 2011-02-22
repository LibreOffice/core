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

#include "pvlaydlg.hxx"

#include <com/sun/star/sheet/DataPilotFieldOrientation.hpp>
#include <com/sun/star/sheet/DataPilotFieldSortMode.hpp>

#include <sfx2/dispatch.hxx>
#include <vcl/mnemonic.hxx>
#include <vcl/msgbox.hxx>

#include "dbdocfun.hxx"
#include "uiitems.hxx"
#include "rangeutl.hxx"
#include "document.hxx"
#include "viewdata.hxx"
#include "tabvwsh.hxx"
#include "reffact.hxx"
#include "scresid.hxx"
#include "globstr.hrc"
#include "pivot.hrc"
#include "dpobject.hxx"
#include "dpsave.hxx"
#include "dpshttab.hxx"
#include "scmod.hxx"

#include "sc.hrc" //CHINA001
#include "scabstdlg.hxx" //CHINA001

// ============================================================================

using namespace ::com::sun::star;
using ::rtl::OUString;

// ============================================================================

namespace {

const sal_uInt16 STD_FORMAT = sal_uInt16( SCA_VALID | SCA_TAB_3D | SCA_COL_ABSOLUTE | SCA_ROW_ABSOLUTE | SCA_TAB_ABSOLUTE );

OUString lclGetNameWithoutMnemonic( const FixedText& rFixedText )
{
    return MnemonicGenerator::EraseAllMnemonicChars( rFixedText.GetText() );
}

} // namespace

// ============================================================================

ScPivotLayoutDlg::ScPivotLayoutDlg( SfxBindings* pB, SfxChildWindow* pCW, Window* pParent, const ScDPObject& rDPObject ) :

    ScAnyRefDlg( pB, pCW, pParent, RID_SCDLG_PIVOT_LAYOUT ),

    maFlLayout( this, ScResId( FL_LAYOUT ) ),
    maScrPage( this, ScResId( SCROLL_PAGE ) ),
    maFtPage( this, ScResId( FT_PAGE ) ),
    maWndPage( this, ScResId( WND_PAGE ), maScrPage, &maFtPage, lclGetNameWithoutMnemonic( maFtPage ), PIVOTFIELDTYPE_PAGE, HID_SC_DPLAY_PAGE, POINTER_PIVOT_FIELD, 5, 2, 1, 0 ),
    maScrCol( this, ScResId( SCROLL_COL ) ),
    maFtCol( this, ScResId( FT_COL ) ),
    maWndCol( this, ScResId( WND_COL ), maScrCol, &maFtCol, lclGetNameWithoutMnemonic( maFtCol ), PIVOTFIELDTYPE_COL, HID_SC_DPLAY_COLUMN, POINTER_PIVOT_COL, 4, 2, 1, 0 ),
    maScrRow( this, ScResId( SCROLL_ROW ) ),
    maFtRow( this, ScResId( FT_ROW ) ),
    maWndRow( this, ScResId( WND_ROW ), maScrRow, &maFtRow, lclGetNameWithoutMnemonic( maFtRow ), PIVOTFIELDTYPE_ROW, HID_SC_DPLAY_ROW, POINTER_PIVOT_ROW, 1, 8, 1, 0 ),
    maScrData( this, ScResId( SCROLL_DATA ) ),
    maFtData( this, ScResId( FT_DATA ) ),
    maWndData( this, ScResId( WND_DATA ), maScrData, &maFtData, lclGetNameWithoutMnemonic( maFtData ), PIVOTFIELDTYPE_DATA, HID_SC_DPLAY_DATA, POINTER_PIVOT_FIELD, 1, 8, 4, 0 ),
    maFlSelect( this, ScResId( FL_SELECT ) ),
    maScrSelect( this, ScResId( WND_HSCROLL ) ),
    maWndSelect( this, ScResId( WND_SELECT ), maScrSelect, 0, String( ScResId( STR_SELECT ) ), PIVOTFIELDTYPE_SELECT, HID_SC_DPLAY_SELECT, POINTER_PIVOT_FIELD, 2, 10, 1, 2 ),
    maFtInfo( this, ScResId( FT_INFO ) ),

    maFlAreas( this, ScResId( FL_OUTPUT ) ),
    maFtInArea( this, ScResId( FT_INAREA) ),
    maEdInPos( this, ScResId( ED_INAREA) ),
    maRbInPos( this, ScResId( RB_INAREA ), &maEdInPos, this ),
    maLbOutPos( this, ScResId( LB_OUTAREA ) ),
    maFtOutArea( this, ScResId( FT_OUTAREA ) ),
    maEdOutPos( this, this, ScResId( ED_OUTAREA ) ),
    maRbOutPos( this, ScResId( RB_OUTAREA ), &maEdOutPos, this ),
    maBtnIgnEmptyRows( this, ScResId( BTN_IGNEMPTYROWS ) ),
    maBtnDetectCat( this, ScResId( BTN_DETECTCAT ) ),
    maBtnTotalCol( this, ScResId( BTN_TOTALCOL ) ),
    maBtnTotalRow( this, ScResId( BTN_TOTALROW ) ),
    maBtnFilter( this, ScResId( BTN_FILTER ) ),
    maBtnDrillDown( this, ScResId( BTN_DRILLDOWN ) ),

    maBtnOk( this, ScResId( BTN_OK ) ),
    maBtnCancel( this, ScResId( BTN_CANCEL ) ),
    maBtnHelp( this, ScResId( BTN_HELP ) ),
    maBtnRemove( this, ScResId( BTN_REMOVE ) ),
    maBtnOptions( this, ScResId( BTN_OPTIONS ) ),
    maBtnMore( this, ScResId( BTN_MORE ) ),

    mxDlgDPObject( new ScDPObject( rDPObject ) ),
    mpViewData( ((ScTabViewShell*)SfxViewShell::Current())->GetViewData() ),
    mpDoc( ((ScTabViewShell*)SfxViewShell::Current())->GetViewData()->GetDocument() ),
    mpFocusWindow( 0 ),
    mpTrackingWindow( 0 ),
    mpDropWindow( 0 ),
    mpActiveEdit( 0 ),
    mbRefInputMode( false )
{
    DBG_ASSERT( mpViewData && mpDoc, "ScPivotLayoutDlg::ScPivotLayoutDlg - missing document or view data" );

    mxDlgDPObject->SetAlive( true );     // needed to get structure information
    mxDlgDPObject->FillOldParam( maPivotData );
    mxDlgDPObject->FillLabelData( maPivotData );

    maBtnRemove.SetClickHdl( LINK( this, ScPivotLayoutDlg, ClickHdl ) );
    maBtnOptions.SetClickHdl( LINK( this, ScPivotLayoutDlg, ClickHdl ) );

    // PIVOT_MAXFUNC defined in sc/inc/dpglobal.hxx
    maFuncNames.reserve( PIVOT_MAXFUNC );
    for( sal_uInt16 i = 1; i <= PIVOT_MAXFUNC; ++i )
        maFuncNames.push_back( String( ScResId( i ) ) );

    maBtnMore.AddWindow( &maFlAreas );
    maBtnMore.AddWindow( &maFtInArea );
    maBtnMore.AddWindow( &maEdInPos );
    maBtnMore.AddWindow( &maRbInPos );
    maBtnMore.AddWindow( &maFtOutArea );
    maBtnMore.AddWindow( &maLbOutPos );
    maBtnMore.AddWindow( &maEdOutPos );
    maBtnMore.AddWindow( &maRbOutPos );
    maBtnMore.AddWindow( &maBtnIgnEmptyRows );
    maBtnMore.AddWindow( &maBtnDetectCat );
    maBtnMore.AddWindow( &maBtnTotalCol );
    maBtnMore.AddWindow( &maBtnTotalRow );
    maBtnMore.AddWindow( &maBtnFilter );
    maBtnMore.AddWindow( &maBtnDrillDown );
    maBtnMore.SetClickHdl( LINK( this, ScPivotLayoutDlg, MoreClickHdl ) );

    if( mxDlgDPObject->GetSheetDesc() )
    {
        maEdInPos.Enable();
        maRbInPos.Enable();
        ScRange aRange = mxDlgDPObject->GetSheetDesc()->aSourceRange;
        String aString;
        aRange.Format( aString, SCR_ABS_3D, mpDoc, mpDoc->GetAddressConvention() );
        maEdInPos.SetText( aString );
    }
    else
    {
        // data is not reachable, so could be a remote database
        maEdInPos.Disable();
        maRbInPos.Disable();
    }

    // #i29203# align right border of page window with data window
    long nPagePosX = maWndData.GetPosPixel().X() + maWndData.GetSizePixel().Width() - maWndPage.GetSizePixel().Width();
    maWndPage.SetPosPixel( Point( nPagePosX, maWndPage.GetPosPixel().Y() ) );
    maScrPage.SetPosPixel( Point( maScrData.GetPosPixel().X(), maScrPage.GetPosPixel().Y() ) );

    InitFieldWindows();

    maLbOutPos.SetSelectHdl( LINK( this, ScPivotLayoutDlg, SelAreaHdl ) );
    maEdOutPos.SetModifyHdl( LINK( this, ScPivotLayoutDlg, EdOutModifyHdl ) );
    maEdInPos.SetModifyHdl( LINK( this, ScPivotLayoutDlg, EdInModifyHdl ) );
    maBtnOk.SetClickHdl( LINK( this, ScPivotLayoutDlg, OkHdl ) );
    maBtnCancel.SetClickHdl( LINK( this, ScPivotLayoutDlg, CancelHdl ) );

    if( mpViewData && mpDoc )
    {
        /*
         * Aus den RangeNames des Dokumentes werden nun die
         * in einem Zeiger-Array gemerkt, bei denen es sich
         * um sinnvolle Bereiche handelt
         */

        maLbOutPos.Clear();
        maLbOutPos.InsertEntry( String( ScResId( SCSTR_UNDEFINED ) ), 0 );
        maLbOutPos.InsertEntry( String( ScResId( SCSTR_NEWTABLE ) ), 1 );

        ScAreaNameIterator aIter( mpDoc );
        String aName;
        ScRange aRange;
        String aRefStr;
        while ( aIter.Next( aName, aRange ) )
        {
            if ( !aIter.WasDBName() )       // hier keine DB-Bereiche !
            {
                sal_uInt16 nInsert = maLbOutPos.InsertEntry( aName );

                aRange.aStart.Format( aRefStr, SCA_ABS_3D, mpDoc, mpDoc->GetAddressConvention() );
                maLbOutPos.SetEntryData( nInsert, new String( aRefStr ) );
            }
        }
    }

    if ( maPivotData.nTab != MAXTAB+1 )
    {
        String aStr;
        ScAddress( maPivotData.nCol,
                   maPivotData.nRow,
                   maPivotData.nTab ).Format( aStr, STD_FORMAT, mpDoc, mpDoc->GetAddressConvention() );
        maEdOutPos.SetText( aStr );
        EdOutModifyHdl( 0 );
    }
    else
    {
        maLbOutPos.SelectEntryPos( maLbOutPos.GetEntryCount()-1 );
        SelAreaHdl(NULL);
    }

    maBtnIgnEmptyRows.Check( maPivotData.bIgnoreEmptyRows );
    maBtnDetectCat.Check( maPivotData.bDetectCategories );
    maBtnTotalCol.Check( maPivotData.bMakeTotalCol );
    maBtnTotalRow.Check( maPivotData.bMakeTotalRow );

    const ScDPSaveData* pSaveData = mxDlgDPObject->GetSaveData();
    maBtnFilter.Check( !pSaveData || pSaveData->GetFilterButton() );
    maBtnDrillDown.Check( !pSaveData || pSaveData->GetDrillDown() );

    // child event listener handles field movement when keyboard shortcut is pressed
    AddChildEventListener( LINK( this, ScPivotLayoutDlg, ChildEventListener ) );
    GrabFieldFocus( maWndSelect );

    FreeResource();
}

ScPivotLayoutDlg::~ScPivotLayoutDlg()
{
    RemoveChildEventListener( LINK( this, ScPivotLayoutDlg, ChildEventListener ) );

    for( sal_uInt16 i = 2, nEntries = maLbOutPos.GetEntryCount();  i < nEntries; ++i )
        delete (String*)maLbOutPos.GetEntryData( i );
}

ScDPLabelData* ScPivotLayoutDlg::GetLabelData( SCCOL nCol, size_t* pnIndex )
{
    ScDPLabelData* pLabelData = 0;
    for( ScDPLabelDataVector::iterator aIt = maLabelData.begin(), aEnd = maLabelData.end(); !pLabelData && (aIt != aEnd); ++aIt )
    {
        if( aIt->mnCol == nCol )
        {
            pLabelData = &*aIt;
            if( pnIndex )
                *pnIndex = aIt - maLabelData.begin();
        }
    }
    return pLabelData;
}

String ScPivotLayoutDlg::GetFuncString( sal_uInt16& rnFuncMask, bool bIsValue )
{
    String aStr;

    if( (rnFuncMask == PIVOT_FUNC_NONE) || (rnFuncMask == PIVOT_FUNC_AUTO) )
    {
        if( bIsValue )
        {
            aStr = GetFuncName( PIVOTSTR_SUM );
            rnFuncMask = PIVOT_FUNC_SUM;
        }
        else
        {
            aStr = GetFuncName( PIVOTSTR_COUNT );
            rnFuncMask = PIVOT_FUNC_COUNT;
        }
    }
    else if( rnFuncMask == PIVOT_FUNC_SUM )         aStr = GetFuncName( PIVOTSTR_SUM );
    else if( rnFuncMask == PIVOT_FUNC_COUNT )       aStr = GetFuncName( PIVOTSTR_COUNT );
    else if( rnFuncMask == PIVOT_FUNC_AVERAGE )     aStr = GetFuncName( PIVOTSTR_AVG );
    else if( rnFuncMask == PIVOT_FUNC_MAX )         aStr = GetFuncName( PIVOTSTR_MAX );
    else if( rnFuncMask == PIVOT_FUNC_MIN )         aStr = GetFuncName( PIVOTSTR_MIN );
    else if( rnFuncMask == PIVOT_FUNC_PRODUCT )     aStr = GetFuncName( PIVOTSTR_PROD );
    else if( rnFuncMask == PIVOT_FUNC_COUNT_NUM )   aStr = GetFuncName( PIVOTSTR_COUNT2 );
    else if( rnFuncMask == PIVOT_FUNC_STD_DEV )     aStr = GetFuncName( PIVOTSTR_DEV );
    else if( rnFuncMask == PIVOT_FUNC_STD_DEVP )    aStr = GetFuncName( PIVOTSTR_DEV2 );
    else if( rnFuncMask == PIVOT_FUNC_STD_VAR )     aStr = GetFuncName( PIVOTSTR_VAR );
    else if( rnFuncMask == PIVOT_FUNC_STD_VARP )    aStr = GetFuncName( PIVOTSTR_VAR2 );
    else
    {
        aStr = ScGlobal::GetRscString( STR_TABLE_ERGEBNIS );
        aStr.AppendAscii( RTL_CONSTASCII_STRINGPARAM( " - " ) );
    }

    return aStr;
}

void ScPivotLayoutDlg::NotifyStartTracking( ScPivotFieldWindow& rSourceWindow )
{
    mpTrackingWindow = &rSourceWindow;
    mpDropWindow = 0;
    rSourceWindow.NotifyStartTracking();
    StartTracking( STARTTRACK_BUTTONREPEAT );
    SetPointer( Pointer( rSourceWindow.GetDropPointerStyle() ) );
}

void ScPivotLayoutDlg::NotifyDoubleClick( ScPivotFieldWindow& rSourceWindow )
{
    // nothing to do on double-click in selection window
    if( rSourceWindow.GetType() == PIVOTFIELDTYPE_SELECT )
        return;

    const ScPivotFuncData* pFuncData = rSourceWindow.GetSelectedFuncData();
    DBG_ASSERT( pFuncData, "ScPivotLayoutDlg::NotifyDoubleClick - invalid selection" );
    if( !pFuncData )
        return;

    ScDPLabelData* pLabelData = GetLabelData( pFuncData->mnCol );
    DBG_ASSERT( pLabelData, "ScPivotLayoutDlg::NotifyDoubleClick - missing label data" );
    if( !pLabelData )
        return;

    ScAbstractDialogFactory* pFactory = ScAbstractDialogFactory::Create();
    DBG_ASSERT( pFactory, "ScPivotLayoutDlg::NotifyDoubleClick - ScAbstractDialogFactory creation failed" );
    if( !pFactory )
        return;

    if( rSourceWindow.GetType() == PIVOTFIELDTYPE_DATA )
    {
        ::std::auto_ptr< AbstractScDPFunctionDlg > xDlg( pFactory->CreateScDPFunctionDlg(
            this, RID_SCDLG_DPDATAFIELD, maLabelData, *pLabelData, *pFuncData ) );
        if( xDlg->Execute() == RET_OK )
        {
            ScPivotFuncData aFuncData( *pFuncData );
            aFuncData.mnFuncMask = pLabelData->mnFuncMask = xDlg->GetFuncMask();
            aFuncData.maFieldRef = xDlg->GetFieldRef();
            rSourceWindow.ModifySelectedField( aFuncData );
        }
    }
    else
    {
        // list of plain names of all data fields
        ScDPNameVec aDataFieldNames;
        maWndData.WriteFieldNames( aDataFieldNames );
        // allow to modify layout options for row fields, if multiple data fields exist, or if it is not the last row field
        bool bLayout = (rSourceWindow.GetType() == PIVOTFIELDTYPE_ROW) && ((aDataFieldNames.size() > 1) || (rSourceWindow.GetSelectedIndex() + 1 < rSourceWindow.GetFieldCount()));

        ::std::auto_ptr< AbstractScDPSubtotalDlg > xDlg( pFactory->CreateScDPSubtotalDlg(
            this, RID_SCDLG_PIVOTSUBT, *mxDlgDPObject, *pLabelData, *pFuncData, aDataFieldNames, bLayout ) );
        if( xDlg->Execute() == RET_OK )
        {
            xDlg->FillLabelData( *pLabelData );
            ScPivotFuncData aFuncData( *pFuncData );
            aFuncData.mnFuncMask = pLabelData->mnFuncMask;
            rSourceWindow.ModifySelectedField( aFuncData );
        }
    }
}

void ScPivotLayoutDlg::NotifyFieldRemoved( ScPivotFieldWindow& rSourceWindow )
{
    // update focus: move to selection window, if source window is empty now
    GrabFieldFocus( rSourceWindow );
}

// protected ------------------------------------------------------------------

void ScPivotLayoutDlg::Tracking( const TrackingEvent& rTEvt )
{
    DBG_ASSERT( mpTrackingWindow, "ScPivotLayoutDlg::Tracking - missing tracking source window" );
    if( !mpTrackingWindow )
        return;

    // find target window
    const Point& rDialogPos = rTEvt.GetMouseEvent().GetPosPixel();
    ScPivotFieldWindow* pTargetWindow = dynamic_cast< ScPivotFieldWindow* >( FindWindow( rDialogPos ) );

    // check if the target orientation is allowed for this field
    if( pTargetWindow && (mpTrackingWindow != pTargetWindow) && !IsInsertAllowed( *mpTrackingWindow, *pTargetWindow ) )
        pTargetWindow = 0;

    // tracking from selection window: do not show "delete" mouse pointer
    PointerStyle eTargetPointer = pTargetWindow ? pTargetWindow->GetDropPointerStyle() :
        ((mpTrackingWindow->GetType() == PIVOTFIELDTYPE_SELECT) ? POINTER_NOTALLOWED : POINTER_PIVOT_DELETE);

    // after calculating pointer style, check if target is selection window
    if( pTargetWindow && (pTargetWindow->GetType() == PIVOTFIELDTYPE_SELECT) )
        pTargetWindow = 0;

    // notify windows about tracking
    if( mpDropWindow != pTargetWindow )
    {
        // tracking window changed
        if( mpDropWindow )
            mpDropWindow->NotifyEndTracking( ENDTRACKING_SUSPEND );
        if( pTargetWindow )
            pTargetWindow->NotifyStartTracking();
        mpDropWindow = pTargetWindow;
    }
    if( mpDropWindow )
        mpDropWindow->NotifyTracking( rDialogPos - pTargetWindow->GetPosPixel() );

    // end tracking: move or remove field
    if( rTEvt.IsTrackingEnded() )
    {
        bool bCancelled = rTEvt.IsTrackingCanceled();
        if( mpDropWindow )
        {
            mpDropWindow->NotifyEndTracking( bCancelled ? ENDTRACKING_CANCEL : ENDTRACKING_DROP );
            if( !bCancelled )
            {
                size_t nInsertIndex = mpDropWindow->GetDropIndex( rDialogPos - mpDropWindow->GetPosPixel() );
                bool bMoved = MoveField( *mpTrackingWindow, *mpDropWindow, nInsertIndex, true );
                // focus drop window, if move was successful, otherwise back to source window
                GrabFieldFocus( bMoved ? *mpDropWindow : *mpTrackingWindow );
            }
        }
        else
        {
            // drop target invalid (outside field windows): remove tracked field
            if( !bCancelled )
                mpTrackingWindow->RemoveSelectedField();
            // focus source window (or another window, if it is empty now)
            GrabFieldFocus( *mpTrackingWindow );
        }
        eTargetPointer = POINTER_ARROW;
        if( mpTrackingWindow != mpDropWindow )
            mpTrackingWindow->NotifyEndTracking( ENDTRACKING_CANCEL );
        mpTrackingWindow = mpDropWindow = 0;
    }
    SetPointer( eTargetPointer );
}

void ScPivotLayoutDlg::SetReference( const ScRange& rRef, ScDocument* pDocP )
{
    if( !mbRefInputMode || !mpActiveEdit )
        return;

    if( rRef.aStart != rRef.aEnd )
        RefInputStart( mpActiveEdit );

    if( mpActiveEdit == &maEdInPos )
    {
        String aRefStr;
        rRef.Format( aRefStr, SCR_ABS_3D, pDocP, pDocP->GetAddressConvention() );
        mpActiveEdit->SetRefString( aRefStr );
    }
    else if( mpActiveEdit == &maEdOutPos )
    {
        String aRefStr;
        rRef.aStart.Format( aRefStr, STD_FORMAT, pDocP, pDocP->GetAddressConvention() );
        mpActiveEdit->SetRefString( aRefStr );
    }
}

sal_Bool ScPivotLayoutDlg::IsRefInputMode() const
{
    return mbRefInputMode;
}

void ScPivotLayoutDlg::SetActive()
{
    if( mbRefInputMode )
    {
        if( mpActiveEdit )
            mpActiveEdit->GrabFocus();

        if( mpActiveEdit == &maEdInPos )
            EdInModifyHdl( 0 );
        else if( mpActiveEdit == &maEdOutPos )
            EdOutModifyHdl( 0 );
    }
    else
    {
        GrabFocus();
    }

    RefInputDone();
}

sal_Bool ScPivotLayoutDlg::Close()
{
    return DoClose( ScPivotLayoutWrapper::GetChildWindowId() );
}

// private --------------------------------------------------------------------

ScPivotFieldWindow& ScPivotLayoutDlg::GetFieldWindow( ScPivotFieldType eFieldType )
{
    switch( eFieldType )
    {
        case PIVOTFIELDTYPE_PAGE:   return maWndPage;
        case PIVOTFIELDTYPE_ROW:    return maWndRow;
        case PIVOTFIELDTYPE_COL:    return maWndCol;
        case PIVOTFIELDTYPE_DATA:   return maWndData;
        default:;
    }
    return maWndSelect;
}

bool ScPivotLayoutDlg::IsInsertAllowed( const ScPivotFieldWindow& rSourceWindow, const ScPivotFieldWindow& rTargetWindow )
{
    if( rTargetWindow.GetType() != PIVOTFIELDTYPE_SELECT )
    {
        const ScPivotFuncData* pSourceData = rSourceWindow.GetSelectedFuncData();
        ScDPLabelData* pLabelData = pSourceData ? GetLabelData( pSourceData->mnCol ) : 0;
        DBG_ASSERT( pLabelData, "ScPivotLayoutDlg::IsInsertAllowed - label data not found" );
        if( pLabelData )
        {
            sheet::DataPilotFieldOrientation eOrient = sheet::DataPilotFieldOrientation_HIDDEN;
            switch( rTargetWindow.GetType() )
            {
                case PIVOTFIELDTYPE_PAGE:   eOrient = sheet::DataPilotFieldOrientation_PAGE;    break;
                case PIVOTFIELDTYPE_COL:    eOrient = sheet::DataPilotFieldOrientation_COLUMN;  break;
                case PIVOTFIELDTYPE_ROW:    eOrient = sheet::DataPilotFieldOrientation_ROW;     break;
                case PIVOTFIELDTYPE_DATA:   eOrient = sheet::DataPilotFieldOrientation_DATA;    break;
                default:                    return false;
            }
            return ScDPObject::IsOrientationAllowed( static_cast< sal_uInt16 >( eOrient ), pLabelData->mnFlags );
        }
    }
    return false;
}

void ScPivotLayoutDlg::InitFieldWindows()
{
    maLabelData = maPivotData.maLabelArray;
    maWndSelect.ReadDataLabels( maLabelData );
    maWndPage.ReadPivotFields( maPivotData.maPageArr );
    maWndCol.ReadPivotFields( maPivotData.maColArr );
    maWndRow.ReadPivotFields( maPivotData.maRowArr );
    maWndData.ReadPivotFields( maPivotData.maDataArr );
}

void ScPivotLayoutDlg::GrabFieldFocus( ScPivotFieldWindow& rFieldWindow )
{
    if( rFieldWindow.IsEmpty() )
    {
        if( maWndSelect.IsEmpty() )
            maBtnOk.GrabFocus();
        else
            maWndSelect.GrabFocus();
    }
    else
        rFieldWindow.GrabFocus();
}

namespace {

void lclFindFieldWindow( ScPivotFieldWindow*& rpFieldWindow, const ScPivotFuncData*& rpFuncData, size_t& rnFieldIndex, ScPivotFieldWindow& rFieldWindow )
{
    ScPivotFuncDataEntry aEntry = rFieldWindow.FindFuncDataByCol( rpFuncData->mnCol );
    if( aEntry.first )
    {
        rpFieldWindow = &rFieldWindow;
        rpFuncData = aEntry.first;
        rnFieldIndex = aEntry.second;
    }
}

} // namespace

bool ScPivotLayoutDlg::MoveField( ScPivotFieldWindow& rSourceWindow, ScPivotFieldWindow& rTargetWindow, size_t nInsertIndex, bool bMoveExisting )
{
    // move inside the same window
    if( &rSourceWindow == &rTargetWindow )
        return bMoveExisting && rTargetWindow.MoveSelectedField( nInsertIndex );

    // do not insert if not supported by target window
    if( !IsInsertAllowed( rSourceWindow, rTargetWindow ) )
    {
        rSourceWindow.RemoveSelectedField();
        return false;
    }

    // move from one window to another window
    if( const ScPivotFuncData* pSourceData = rSourceWindow.GetSelectedFuncData() )
    {
        // move to page/col/row window: try to find existing field in another window
        ScPivotFieldWindow* pSourceWindow = &rSourceWindow;
        size_t nSourceIndex = rSourceWindow.GetSelectedIndex();
        if( rTargetWindow.GetType() != PIVOTFIELDTYPE_DATA )
        {
            lclFindFieldWindow( pSourceWindow, pSourceData, nSourceIndex, maWndPage );
            lclFindFieldWindow( pSourceWindow, pSourceData, nSourceIndex, maWndCol );
            lclFindFieldWindow( pSourceWindow, pSourceData, nSourceIndex, maWndRow );
        }

        // found in target window: move to new position
        if( pSourceWindow == &rTargetWindow )
            return bMoveExisting && pSourceWindow->MoveField( nSourceIndex, nInsertIndex );

        // insert field into target window
        rTargetWindow.InsertField( nInsertIndex, *pSourceData );
        // remove field from source window
        pSourceWindow->RemoveField( nSourceIndex );
        // remove field from data window, if it is the original source
        if( (rSourceWindow.GetType() == PIVOTFIELDTYPE_DATA) && (pSourceWindow->GetType() != PIVOTFIELDTYPE_DATA) )
            rSourceWindow.RemoveSelectedField();

        return true;
    }

    return false;
}

// handlers -------------------------------------------------------------------

IMPL_LINK( ScPivotLayoutDlg, ClickHdl, PushButton *, pBtn )
{
    if( mpFocusWindow )
    {
        /*  Raising sub dialogs (from the NotifyDoubleClick function) triggers
            VCL child window focus events from this sub dialog which may
            invalidate the member mpFocusWindow pointing to the target field
            window. This would cause a crash with the following call to the
            GrabFieldFocus function, if mpFocusWindow is used directly. */
        ScPivotFieldWindow& rTargetWindow = *mpFocusWindow;

        if( pBtn == &maBtnRemove )
        {
            rTargetWindow.RemoveSelectedField();
            // focus back to field window
            GrabFieldFocus( rTargetWindow );
        }
        else if( pBtn == &maBtnOptions )
        {
            NotifyDoubleClick( rTargetWindow );
            // focus back to field window
            GrabFieldFocus( rTargetWindow );
        }
    }
    return 0;
}

IMPL_LINK( ScPivotLayoutDlg, OkHdl, OKButton *, EMPTYARG )
{
    String aOutPosStr = maEdOutPos.GetText();
    ScAddress aAdrDest;
    bool bToNewTable = maLbOutPos.GetSelectEntryPos() == 1;
    sal_uInt16 nResult = !bToNewTable ? aAdrDest.Parse( aOutPosStr, mpDoc, mpDoc->GetAddressConvention() ) : 0;

    if( bToNewTable || ((aOutPosStr.Len() > 0) && ((nResult & SCA_VALID) == SCA_VALID)) )
    {
        ScPivotFieldVector aPageFields, aColFields, aRowFields, aDataFields;
        maWndPage.WritePivotFields( aPageFields );
        maWndCol.WritePivotFields( aColFields );
        maWndRow.WritePivotFields( aRowFields );
        maWndData.WritePivotFields( aDataFields );

        // TODO: handle data field in dialog field windows?
        aRowFields.resize( aRowFields.size() + 1 );
        aRowFields.back().nCol = PIVOT_DATA_FIELD;

        ScDPSaveData* pOldSaveData = mxDlgDPObject->GetSaveData();

        ScRange aOutRange( aAdrDest );      // bToNewTable is passed separately

        ScDPSaveData aSaveData;
        aSaveData.SetIgnoreEmptyRows( maBtnIgnEmptyRows.IsChecked() );
        aSaveData.SetRepeatIfEmpty( maBtnDetectCat.IsChecked() );
        aSaveData.SetColumnGrand( maBtnTotalCol.IsChecked() );
        aSaveData.SetRowGrand( maBtnTotalRow.IsChecked() );
        aSaveData.SetFilterButton( maBtnFilter.IsChecked() );
        aSaveData.SetDrillDown( maBtnDrillDown.IsChecked() );

        uno::Reference< sheet::XDimensionsSupplier > xSource = mxDlgDPObject->GetSource();

        ScDPObject::ConvertOrientation( aSaveData, aPageFields, sheet::DataPilotFieldOrientation_PAGE,   0, 0, 0, xSource, false );
        ScDPObject::ConvertOrientation( aSaveData, aColFields,  sheet::DataPilotFieldOrientation_COLUMN, 0, 0, 0, xSource, false );
        ScDPObject::ConvertOrientation( aSaveData, aRowFields,  sheet::DataPilotFieldOrientation_ROW,    0, 0, 0, xSource, false );
        ScDPObject::ConvertOrientation( aSaveData, aDataFields, sheet::DataPilotFieldOrientation_DATA,   0, 0, 0, xSource, false, &aColFields, &aRowFields, &aPageFields );

        for( ScDPLabelDataVector::const_iterator aIt = maLabelData.begin(), aEnd = maLabelData.end(); aIt != aEnd; ++aIt )
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
                for (::std::vector<ScDPLabelData::Member>::const_iterator itr = aIt->maMembers.begin(), itrEnd = aIt->maMembers.end();
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

        mbRefInputMode = false;     // to allow deselecting when switching sheets

        SetDispatcherLock( false );
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
            mbRefInputMode = true;
            SetDispatcherLock(true);
        }
    }
    else
    {
        if( !maBtnMore.GetState() )
            maBtnMore.SetState( true );

        ErrorBox( this, WinBits( WB_OK | WB_DEF_OK ), ScGlobal::GetRscString( STR_INVALID_TABREF ) ).Execute();
        maEdOutPos.GrabFocus();
    }
    return 0;
}

IMPL_LINK( ScPivotLayoutDlg, CancelHdl, CancelButton *, EMPTYARG )
{
    Close();
    return 0;
}

IMPL_LINK( ScPivotLayoutDlg, MoreClickHdl, MoreButton *, EMPTYARG )
{
    if ( maBtnMore.GetState() )
    {
        mbRefInputMode = true;
        if ( maEdInPos.IsEnabled() )
        {
            maEdInPos.Enable();
            maEdInPos.GrabFocus();
            maEdInPos.Enable();
        }
        else
        {
            maEdOutPos.Enable();
            maEdOutPos.GrabFocus();
            maEdOutPos.Enable();
        }
    }
    else
    {
        mbRefInputMode = false;
    }
    return 0;
}

IMPL_LINK( ScPivotLayoutDlg, EdOutModifyHdl, Edit *, EMPTYARG )
{
    String theCurPosStr = maEdOutPos.GetText();
    sal_uInt16 nResult = ScAddress().Parse( theCurPosStr, mpDoc, mpDoc->GetAddressConvention() );

    if ( SCA_VALID == (nResult & SCA_VALID) )
    {
        String* pStr = 0;
        bool bFound = false;
        sal_uInt16 i = 0;
        sal_uInt16 nCount = maLbOutPos.GetEntryCount();

        for ( i=2; i<nCount && !bFound; i++ )
        {
            pStr = (String*)maLbOutPos.GetEntryData( i );
            bFound = (theCurPosStr == *pStr);
        }

        if ( bFound )
            maLbOutPos.SelectEntryPos( --i );
        else
            maLbOutPos.SelectEntryPos( 0 );
    }
    return 0;
}


IMPL_LINK( ScPivotLayoutDlg, EdInModifyHdl, Edit *, EMPTYARG )
{
    String theCurPosStr = maEdInPos.GetText();
    sal_uInt16 nResult = ScRange().Parse( theCurPosStr, mpDoc, mpDoc->GetAddressConvention() );

    // invalid source range
    if( SCA_VALID != (nResult & SCA_VALID) )
        return 0;

    ScRefAddress start, end;
    ConvertDoubleRef( mpDoc, theCurPosStr, 1, start, end, mpDoc->GetAddressConvention() );
    ScRange aNewRange( start.GetAddress(), end.GetAddress() );
    ScSheetSourceDesc inSheet = *mxDlgDPObject->GetSheetDesc();

    // new range is identical to the current range
    if( inSheet.aSourceRange == aNewRange )
        return 0;

    ScTabViewShell* pTabViewShell = mpViewData->GetViewShell();
    inSheet.aSourceRange = aNewRange;
    mxDlgDPObject->SetSheetDesc( inSheet );
    mxDlgDPObject->FillOldParam( maPivotData );
    mxDlgDPObject->FillLabelData( maPivotData );

    // SetDialogDPObject does not take ownership but makes a copy internally
    pTabViewShell->SetDialogDPObject( mxDlgDPObject.get() );

    // re-initialize the field windows from the new data
    InitFieldWindows();

    return 0;
}

IMPL_LINK( ScPivotLayoutDlg, SelAreaHdl, ListBox *, EMPTYARG )
{
    String aString;
    sal_uInt16 nSelPos = maLbOutPos.GetSelectEntryPos();
    if( nSelPos > 1 )
    {
        aString = *(String*)maLbOutPos.GetEntryData( nSelPos );
    }
    else
    {
        // do not allow to specify output position, if target is "new sheet"
        bool bNewSheet = nSelPos == 1;
        maEdOutPos.Enable( !bNewSheet );
        maRbOutPos.Enable( !bNewSheet );
    }

    maEdOutPos.SetText( aString );
    return 0;
}

IMPL_LINK( ScPivotLayoutDlg, ChildEventListener, VclWindowEvent*, pEvent )
{
    Window* pWindow = pEvent->GetWindow();
    // check that this dialog is the parent of the window, to ignore focus events from sub dialogs
    if( (pEvent->GetId() == VCLEVENT_WINDOW_GETFOCUS) && pWindow && (pWindow->GetParent() == this) )
    {
        // check if old window and/or new window are field windows
        ScPivotFieldWindow* pSourceWindow = mpFocusWindow;
        ScPivotFieldWindow* pTargetWindow = dynamic_cast< ScPivotFieldWindow* >( pWindow );

        /*  Enable or disable the Remove/Options buttons. Do nothing if the
            buttons themselves get the focus.
            #128113# The TestTool may set the focus into an empty window. Then
            the Remove/Options buttons must be disabled. */
        if( (pWindow != &maBtnRemove) && (pWindow != &maBtnOptions) )
        {
            bool bEnableButtons = pTargetWindow && (pTargetWindow->GetType() != PIVOTFIELDTYPE_SELECT) && !pTargetWindow->IsEmpty();
            maBtnRemove.Enable( bEnableButtons );
            maBtnOptions.Enable( bEnableButtons );
            /*  Remember the new focus window (will not be changed, if
                Remove/Option buttons are getting focus, because they need to
                know the field window they are working on). */
            mpFocusWindow = pTargetWindow;
        }

        /*  Move the last selected field to target window, if focus changes via
            keyboard shortcut. */
        if( pSourceWindow && pTargetWindow && (pSourceWindow != pTargetWindow) && ((pTargetWindow->GetGetFocusFlags() & GETFOCUS_MNEMONIC) != 0) )
        {
            // append field in target window
            MoveField( *pSourceWindow, *pTargetWindow, pTargetWindow->GetFieldCount(), false );
            // move cursor in selection window to next field
            if( pSourceWindow->GetType() == PIVOTFIELDTYPE_SELECT )
                pSourceWindow->SelectNextField();
            // return focus to source window (if it is not empty)
            GrabFieldFocus( pSourceWindow->IsEmpty() ? *pTargetWindow : *pSourceWindow );
        }

        mpActiveEdit = dynamic_cast< ::formula::RefEdit* >( pEvent->GetWindow() );
    }
    return 0;
}

// ============================================================================
