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

#include "fieldwnd.hxx"

#include <tools/debug.hxx>
#include <vcl/decoview.hxx>
#include <vcl/help.hxx>
#include <vcl/svapp.hxx>
#include <vcl/virdev.hxx>

#include "pvlaydlg.hxx"
#include "AccessibleDataPilotControl.hxx"
#include "scresid.hxx"
#include "sc.hrc"

// ============================================================================

using namespace ::com::sun::star;
using ::rtl::OUString;

// ============================================================================

namespace {

/** Line width for insertion cursor in pixels. */
const long CURSOR_WIDTH             = 3;

/** Number of tracking events before auto scrolling starts. */
const size_t INITIAL_TRACKING_DELAY = 20;

} // namespace

// ============================================================================

ScPivotFieldWindow::ScPivotWindowField::ScPivotWindowField( const ScDPLabelData& rLabelData ) :
    maFuncData( rLabelData.mnCol, rLabelData.mnFuncMask ),
    maFieldName( rLabelData.getDisplayName() )
{
}

ScPivotFieldWindow::ScPivotWindowField::ScPivotWindowField( ScPivotLayoutDlg& rDialog, const ScPivotField& rField, bool bDataWindow ) :
    maFuncData( rField.nCol, rField.nFuncMask, rField.maFieldRef )
{
    InitFieldName( rDialog, bDataWindow );
}

ScPivotFieldWindow::ScPivotWindowField::ScPivotWindowField( ScPivotLayoutDlg& rDialog, const ScPivotFuncData& rFuncData, bool bDataWindow ) :
    maFuncData( rFuncData )
{
    InitFieldName( rDialog, bDataWindow );
}

void ScPivotFieldWindow::ScPivotWindowField::InitFieldName( ScPivotLayoutDlg& rDialog, bool bDataWindow )
{
    if( maFuncData.mnCol != PIVOT_DATA_FIELD )
    {
        ScDPLabelData* pLabelData = rDialog.GetLabelData( maFuncData.mnCol );
        DBG_ASSERT( pLabelData, "ScPivotWindowField::InitFieldName - no label data found" );
        if( pLabelData )
        {
            if( bDataWindow )
            {
                // write original nFuncMask to label data
                pLabelData->mnFuncMask = maFuncData.mnFuncMask;
                // GetFuncString() modifies nFuncMask (e.g. auto to sum or count)
                maFieldName = rDialog.GetFuncString( maFuncData.mnFuncMask, pLabelData->mbIsValue );
            }
            maFieldName += pLabelData->getDisplayName();
        }
    }
}

// ============================================================================

ScPivotFieldWindow::ScPivotFieldWindow( ScPivotLayoutDlg* pDialog, const ResId& rResId,
        ScrollBar& rScrollBar, FixedText* pFtCaption, const OUString& rName,
        ScPivotFieldType eFieldType, const sal_Char* pcHelpId, PointerStyle eDropPointer,
        size_t nColCount, size_t nRowCount, long nFieldWidthFactor, long nSpaceSize ) :
    Control( pDialog, rResId ),
    mpDialog( pDialog ),
    mpAccessible( 0 ),
    mrScrollBar( rScrollBar ),
    mpFtCaption( pFtCaption ),
    maName( rName ),
    meFieldType( eFieldType ),
    meDropPointer( eDropPointer ),
    mnColCount( nColCount ),
    mnRowCount( nRowCount ),
    mnFirstVisIndex( 0 ),
    mnSelectIndex( 0 ),
    mnInsCursorIndex( PIVOTFIELD_INVALID ),
    mnOldFirstVisIndex( 0 ),
    mnAutoScrollDelay( 0 ),
    mbVertical( eFieldType == PIVOTFIELDTYPE_SELECT ),
    mbIsTrackingSource( false )
{
    SetHelpId( pcHelpId );

    mnLineSize = mbVertical ? mnRowCount : mnColCount;
    mnPageSize = mnColCount * mnRowCount;

    // a single field is 36x12 appfont units
    maFieldSize = LogicToPixel( Size( 36, 12 ), MapMode( MAP_APPFONT ) );
    maFieldSize.Width() *= nFieldWidthFactor;
    maSpaceSize = LogicToPixel( Size( nSpaceSize, nSpaceSize ), MapMode( MAP_APPFONT ) );

    // set window size
    long nWinWidth  = static_cast< long >( mnColCount * maFieldSize.Width()  + (mnColCount - 1) * maSpaceSize.Width() );
    long nWinHeight = static_cast< long >( mnRowCount * maFieldSize.Height() + (mnRowCount - 1) * maSpaceSize.Height() );
    SetSizePixel( Size( nWinWidth, nWinHeight ) );

    // scroll bar
    Point aScrollBarPos = GetPosPixel();
    Size aScrollBarSize( nWinWidth, nWinHeight );
    if( mbVertical )
    {
        aScrollBarPos.Y() += nWinHeight + maSpaceSize.Height();
        aScrollBarSize.Height() = GetSettings().GetStyleSettings().GetScrollBarSize();
    }
    else
    {
        aScrollBarPos.X() += nWinWidth + maSpaceSize.Width();
        aScrollBarSize.Width() = GetSettings().GetStyleSettings().GetScrollBarSize();
    }
    mrScrollBar.SetPosSizePixel( aScrollBarPos, aScrollBarSize );
    mrScrollBar.SetLineSize( 1 );
    mrScrollBar.SetPageSize( static_cast< long >( mbVertical ? mnColCount : mnRowCount ) );
    mrScrollBar.SetVisibleSize( static_cast< long >( mbVertical ? mnColCount : mnRowCount ) );
    mrScrollBar.SetScrollHdl( LINK( this, ScPivotFieldWindow, ScrollHdl ) );
    mrScrollBar.SetEndScrollHdl( LINK( this, ScPivotFieldWindow, ScrollHdl ) );
}

ScPivotFieldWindow::~ScPivotFieldWindow()
{
    ::rtl::Reference< ScAccessibleDataPilotControl > xAcc = GetAccessibleControl();
    if( xAcc.is() )
        xAcc->dispose();
}

void ScPivotFieldWindow::ReadDataLabels( const ScDPLabelDataVector& rLabels )
{
    maFields.clear();
    maFields.reserve( rLabels.size() );
    for( ScDPLabelDataVector::const_iterator aIt = rLabels.begin(), aEnd = rLabels.end(); aIt != aEnd; ++aIt )
    {
        ScPivotWindowField aField( *aIt );
        if( aField.maFieldName.getLength() > 0 )
            maFields.push_back( aField );
    }
    Invalidate();
}

void ScPivotFieldWindow::ReadPivotFields( const ScPivotFieldVector& rPivotFields )
{
    maFields.clear();
    maFields.reserve( rPivotFields.size() );
    for( ScPivotFieldVector::const_iterator aIt = rPivotFields.begin(), aEnd = rPivotFields.end(); aIt != aEnd; ++aIt )
    {
        ScPivotWindowField aField( *mpDialog, *aIt, meFieldType == PIVOTFIELDTYPE_DATA );
        if( aField.maFieldName.getLength() > 0 )
            maFields.push_back( aField );
    }
    Invalidate();
}

void ScPivotFieldWindow::WriteFieldNames( ScDPNameVec& rFieldNames ) const
{
    rFieldNames.clear();
    rFieldNames.reserve( maFields.size() );
    // do not use the names stored in maFields, but generate plain display names from label data
    for( ScPivotWindowFieldVector::const_iterator aIt = maFields.begin(), aEnd = maFields.end(); aIt != aEnd; ++aIt )
    {
        if( ScDPLabelData* pLabelData = mpDialog->GetLabelData( aIt->maFuncData.mnCol ) )
        {
            OUString aDisplayName = pLabelData->getDisplayName();
            if( aDisplayName.getLength() > 0 )
                rFieldNames.push_back( aDisplayName );
        }
    }
}

void ScPivotFieldWindow::WritePivotFields( ScPivotFieldVector& rPivotFields ) const
{
    rPivotFields.resize( maFields.size() );
    ScPivotFieldVector::iterator aOutIt = rPivotFields.begin();
    for( ScPivotWindowFieldVector::const_iterator aIt = maFields.begin(), aEnd = maFields.end(); aIt != aEnd; ++aIt, ++aOutIt )
    {
        aOutIt->nCol = aIt->maFuncData.mnCol;
        aOutIt->nFuncMask = aIt->maFuncData.mnFuncMask;
        aOutIt->maFieldRef = aIt->maFuncData.maFieldRef;
    }
}

OUString ScPivotFieldWindow::GetDescription() const
{
    switch( meFieldType )
    {
        case PIVOTFIELDTYPE_COL:      return String( ScResId( STR_ACC_DATAPILOT_COL_DESCR ) );
        case PIVOTFIELDTYPE_ROW:      return String( ScResId( STR_ACC_DATAPILOT_ROW_DESCR ) );
        case PIVOTFIELDTYPE_DATA:     return String( ScResId( STR_ACC_DATAPILOT_DATA_DESCR ) );
        case PIVOTFIELDTYPE_SELECT:   return String( ScResId( STR_ACC_DATAPILOT_SEL_DESCR ) );
        default:;
    }
    return OUString();
}

OUString ScPivotFieldWindow::GetFieldText( size_t nFieldIndex ) const
{
    return (nFieldIndex < maFields.size()) ? maFields[ nFieldIndex ].maFieldName : OUString();
}

ScPivotFuncDataEntry ScPivotFieldWindow::FindFuncDataByCol( SCCOL nCol ) const
{
    for( ScPivotWindowFieldVector::const_iterator aIt = maFields.begin(), aEnd = maFields.end(); aIt != aEnd; ++aIt )
        if( aIt->maFuncData.mnCol == nCol )
            return ScPivotFuncDataEntry( &aIt->maFuncData, aIt - maFields.begin() );
    return ScPivotFuncDataEntry( 0, PIVOTFIELD_INVALID );
}

Point ScPivotFieldWindow::GetFieldPosition( size_t nFieldIndex ) const
{
    long nRelIndex = static_cast< long >( nFieldIndex ) - mnFirstVisIndex;
    long nCol = static_cast< long >( mbVertical ? (nRelIndex / mnRowCount) : (nRelIndex % mnColCount) );
    long nRow = static_cast< long >( mbVertical ? (nRelIndex % mnRowCount) : (nRelIndex / mnColCount) );
    return Point( nCol * (maFieldSize.Width() + maSpaceSize.Width()), nRow * (maFieldSize.Height() + maSpaceSize.Height()) );
}

size_t ScPivotFieldWindow::GetFieldIndex( const Point& rWindowPos ) const
{
    if( (rWindowPos.X() >= 0) && (rWindowPos.Y() >= 0) )
    {
        long nGridWidth = maFieldSize.Width() + maSpaceSize.Width();
        long nGridHeight = maFieldSize.Height() + maSpaceSize.Height();
        size_t nCol = static_cast< size_t >( rWindowPos.X() / nGridWidth );
        size_t nRow = static_cast< size_t >( rWindowPos.Y() / nGridHeight );
        if( (nCol < mnColCount) && (nRow < mnRowCount) )
        {
            long nColOffset = rWindowPos.X() % nGridWidth;
            long nRowOffset = rWindowPos.Y() % nGridHeight;
            // check that passed position is not in the space between the fields
            if( (nColOffset < maFieldSize.Width()) && (nRowOffset < maFieldSize.Height()) )
            {
                size_t nFieldIndex = mnFirstVisIndex + (mbVertical ? (nCol * mnRowCount + nRow) : (nRow * mnColCount + nCol));
                return (nFieldIndex < maFields.size()) ? nFieldIndex : PIVOTFIELD_INVALID;
            }
        }
    }
    return PIVOTFIELD_INVALID;
}

size_t ScPivotFieldWindow::GetDropIndex( const Point& rWindowPos ) const
{
    if( (rWindowPos.X() >= 0) && (rWindowPos.Y() >= 0) )
    {
        long nGridWidth = maFieldSize.Width() + maSpaceSize.Width();
        long nGridHeight = maFieldSize.Height() + maSpaceSize.Height();
        size_t nCol = static_cast< size_t >( rWindowPos.X() / nGridWidth );
        size_t nRow = static_cast< size_t >( rWindowPos.Y() / nGridHeight );
        if( (nCol < mnColCount) && (nRow < mnRowCount) )
        {
            size_t nFieldIndex = mnFirstVisIndex + (mbVertical ? (nCol * mnRowCount + nRow) : (nRow * mnColCount + nCol));
            long nColOffset = rWindowPos.X() % nGridWidth;
            long nRowOffset = rWindowPos.Y() % nGridHeight;
            // take next field, if position is in right/lower third
            if( (mnColCount == 1) ? (nRowOffset * 3 > nGridHeight * 2) : (nColOffset * 3 > nGridWidth * 2) )
                ++nFieldIndex;
            return ::std::min( nFieldIndex, maFields.size() );
        }
    }
    return maFields.size();
}

void ScPivotFieldWindow::GrabFocusAndSelect( size_t nSelectIndex )
{
    if( !HasFocus() ) GrabFocus();
    MoveSelection( nSelectIndex );
}

void ScPivotFieldWindow::SelectNextField()
{
    MoveSelection( NEXT_FIELD );
}

void ScPivotFieldWindow::InsertField( size_t nInsertIndex, const ScPivotFuncData& rFuncData )
{
    if( (meFieldType != PIVOTFIELDTYPE_SELECT) && (nInsertIndex <= maFields.size()) )
    {
        size_t nFieldIndex = FindFuncDataByCol( rFuncData.mnCol ).second;
        if( nFieldIndex < maFields.size() )
        {
            // field exists already in this window, move it to the specified position
            MoveField( nFieldIndex, nInsertIndex );
        }
        else
        {
            // insert the field into the vector and notify accessibility object
            ScPivotWindowField aField( *mpDialog, rFuncData, meFieldType == PIVOTFIELDTYPE_DATA );
            if( aField.maFieldName.getLength() > 0 )
            {
                InsertFieldUnchecked( nInsertIndex, aField );
                // adjust selection and scroll position
                MoveSelection( nInsertIndex );
                Invalidate();
            }
        }
    }
}

bool ScPivotFieldWindow::RemoveField( size_t nRemoveIndex )
{
    if( (meFieldType != PIVOTFIELDTYPE_SELECT) && (nRemoveIndex < maFields.size()) )
    {
        // remove the field from the vector and notify accessibility object
        RemoveFieldUnchecked( nRemoveIndex );
        // adjust selection and scroll position, if last field is removed
        if( !maFields.empty() )
            MoveSelection( (mnSelectIndex < maFields.size()) ? mnSelectIndex : (maFields.size() - 1) );
        Invalidate();
        return true;
    }
    return false;
}

bool ScPivotFieldWindow::MoveField( size_t nFieldIndex, size_t nInsertIndex )
{
    /*  If field is moved behind current position, insertion index needs to be
        adjusted, because the field is first removed from the vector. This is
        done before nFieldIndex and nInsertIndex are checked for equality, to
        catch the cases "move before ourselves" and "move bedind ourselves"
        which are both no-ops. */
    if( nFieldIndex < nInsertIndex )
        --nInsertIndex;

    if( (meFieldType != PIVOTFIELDTYPE_SELECT) && (nFieldIndex != nInsertIndex) && (nFieldIndex < maFields.size()) && (nInsertIndex < maFields.size()) )
    {
        // move the field in the vector and notify accessibility object
        ScPivotWindowField aField = maFields[ nFieldIndex ];
        RemoveFieldUnchecked( nFieldIndex );
        InsertFieldUnchecked( nInsertIndex, aField );
        // adjust selection and scroll position
        MoveSelection( nInsertIndex );
        Invalidate();
        return true;
    }
    return false;
}

const ScPivotFuncData* ScPivotFieldWindow::GetSelectedFuncData() const
{
    return (mnSelectIndex < maFields.size()) ? &maFields[ mnSelectIndex ].maFuncData : 0;
}

void ScPivotFieldWindow::ModifySelectedField( const ScPivotFuncData& rFuncData )
{
    if( mnSelectIndex < maFields.size() )
    {
        maFields[ mnSelectIndex ].maFuncData = rFuncData;
        maFields[ mnSelectIndex ].InitFieldName( *mpDialog, meFieldType == PIVOTFIELDTYPE_DATA );
        Invalidate();
    }
}

bool ScPivotFieldWindow::RemoveSelectedField()
{
    return RemoveField( mnSelectIndex );
}

bool ScPivotFieldWindow::MoveSelectedField( size_t nInsertIndex )
{
    return MoveField( mnSelectIndex, nInsertIndex );
}

void ScPivotFieldWindow::NotifyStartTracking()
{
    // rescue old scrolling index, to be able to restore it when tracking is cancelled
    mnOldFirstVisIndex = mnFirstVisIndex;
}

void ScPivotFieldWindow::NotifyTracking( const Point& rWindowPos )
{
    size_t nFieldIndex = GetDropIndex( rWindowPos );

    // insertion index changed: draw new cursor and exit
    if( nFieldIndex != mnInsCursorIndex )
    {
        mnInsCursorIndex = nFieldIndex;
        mnAutoScrollDelay = INITIAL_TRACKING_DELAY;
        Invalidate();
        return;
    }

    // insertion index unchanged: countdown for auto scrolling
    if( mnAutoScrollDelay > 0 )
    {
        --mnAutoScrollDelay;
        return;
    }

    // check if tracking happens on first or last field
    long nScrollDelta = 0;
    if( (mnInsCursorIndex > 0) && (mnInsCursorIndex == mnFirstVisIndex) )
        nScrollDelta = -static_cast< long >( mnLineSize );
    else if( (mnInsCursorIndex < maFields.size()) && (mnInsCursorIndex == mnFirstVisIndex + mnPageSize) )
        nScrollDelta = static_cast< long >( mnLineSize );
    if( nScrollDelta != 0 )
    {
        // update mnInsCursorIndex, so it will be drawn at the same position after scrolling
        mnInsCursorIndex += nScrollDelta;
        mnFirstVisIndex += nScrollDelta;
        // delay auto scroll by line size, to slow down scrolling in column/page windows
        mnAutoScrollDelay = mnLineSize - 1;
        Invalidate();
    }
}

void ScPivotFieldWindow::NotifyEndTracking( ScPivotFieldEndTracking eEndType )
{
    if( eEndType != ENDTRACKING_DROP )
        mnFirstVisIndex = mnOldFirstVisIndex;
    if( eEndType != ENDTRACKING_SUSPEND )
    {
        mnOldFirstVisIndex = PIVOTFIELD_INVALID;
        mbIsTrackingSource = false;
    }
    mnInsCursorIndex = PIVOTFIELD_INVALID;
    Invalidate();
}

// protected ------------------------------------------------------------------

void ScPivotFieldWindow::Paint( const Rectangle& /*rRect*/ )
{
    // prepare a virtual device for buffered painting
    VirtualDevice aVirDev;
    // #i97623# VirtualDevice is always LTR on construction while other windows derive direction from parent
    aVirDev.EnableRTL( IsRTLEnabled() );
    aVirDev.SetMapMode( MAP_PIXEL );
    aVirDev.SetOutputSizePixel( GetSizePixel() );
    Font aFont = GetFont();
    aFont.SetTransparent( true );
    aVirDev.SetFont( aFont );

    // draw the background and all fields
    DrawBackground( aVirDev );
    for( size_t nFieldIndex = mnFirstVisIndex, nEndIndex = mnFirstVisIndex + mnPageSize; nFieldIndex < nEndIndex; ++nFieldIndex )
        DrawField( aVirDev, nFieldIndex );
    DrawInsertionCursor( aVirDev );
    DrawBitmap( Point( 0, 0 ), aVirDev.GetBitmap( Point( 0, 0 ), GetSizePixel() ) );

    // draw field text focus
    if( HasFocus() && (mnSelectIndex < maFields.size()) && (mnFirstVisIndex <= mnSelectIndex) && (mnSelectIndex < mnFirstVisIndex + mnPageSize) )
    {
        long nFieldWidth = maFieldSize.Width();
        long nSelectionWidth = Min( GetTextWidth( maFields[ mnSelectIndex ].maFieldName ) + 4, nFieldWidth - 6 );
        Rectangle aSelection(
            GetFieldPosition( mnSelectIndex ) + Point( (nFieldWidth - nSelectionWidth) / 2, 3 ),
            Size( nSelectionWidth, maFieldSize.Height() - 6 ) );
        InvertTracking( aSelection, SHOWTRACK_SMALL | SHOWTRACK_WINDOW );
    }

    // update scrollbar
    size_t nFieldCount = maFields.size();
    /*  Already show the scrollbar if window is full but no fields are hidden
        (yet). This gives the user the hint that it is now possible to add more
        fields to the window. */
    mrScrollBar.Show( nFieldCount >= mnPageSize );
    mrScrollBar.Enable( nFieldCount > mnPageSize );
    if( mrScrollBar.IsVisible() )
    {
        mrScrollBar.SetRange( Range( 0, static_cast< long >( (nFieldCount - 1) / mnLineSize + 1 ) ) );
        mrScrollBar.SetThumbPos( static_cast< long >( mnFirstVisIndex / mnLineSize ) );
    }

    /*  Exclude empty fields from tab chain, but do not disable them. They need
        to be enabled because they still act as target for field movement via
        keyboard shortcuts. */
    WinBits nMask = ~(WB_TABSTOP | WB_NOTABSTOP);
    SetStyle( (GetStyle() & nMask) | (IsEmpty() ? WB_NOTABSTOP : WB_TABSTOP) );
}

void ScPivotFieldWindow::StateChanged( StateChangedType nStateChange )
{
    Control::StateChanged( nStateChange );

    if( nStateChange == STATE_CHANGE_INITSHOW )
    {
        /*  After the fixed text associated to this control has received its
            unique mnemonic from VCL dialog initialization code, put this text
            into the field windows.
            #124828# Hiding the FixedTexts and clearing the tab stop style bits
            has to be done after assigning the mnemonics, but Paint() is too
            late, because the test tool may send key events to the dialog when
            it isn't visible. Mnemonics are assigned in Dialog::StateChanged()
            for STATE_CHANGE_INITSHOW, so this can be done immediately
            afterwards. */
        if( mpFtCaption )
        {
            SetText( mpFtCaption->GetText() );
            mpFtCaption->Hide();
        }
    }
}

void ScPivotFieldWindow::DataChanged( const DataChangedEvent& rDCEvt )
{
    Control::DataChanged( rDCEvt );
    if( (rDCEvt.GetType() == DATACHANGED_SETTINGS) && (rDCEvt.GetFlags() & SETTINGS_STYLE) )
        Invalidate();
}

void ScPivotFieldWindow::KeyInput( const KeyEvent& rKEvt )
{
    bool bKeyEvaluated = false;

    if( !maFields.empty() )
    {
        const KeyCode& rKeyCode = rKEvt.GetKeyCode();
        sal_uInt16 nCode = rKeyCode.GetCode();

        // do not move fields in selection window
        if( rKeyCode.IsMod1() && (meFieldType != PIVOTFIELDTYPE_SELECT) )
        {
            bKeyEvaluated = true;
            switch( nCode )
            {
                case KEY_UP:        MoveSelectedField( mbVertical ? PREV_FIELD : PREV_LINE );   break;
                case KEY_DOWN:      MoveSelectedField( mbVertical ? NEXT_FIELD : NEXT_LINE );   break;
                case KEY_LEFT:      MoveSelectedField( mbVertical ? PREV_LINE : PREV_FIELD );   break;
                case KEY_RIGHT:     MoveSelectedField( mbVertical ? NEXT_LINE : NEXT_FIELD );   break;
                case KEY_HOME:      MoveSelectedField( FIRST_FIELD );                           break;
                case KEY_END:       MoveSelectedField( LAST_FIELD );                            break;
                default:            bKeyEvaluated = false;
            }
        }
        else
        {
            bKeyEvaluated = true;
            switch( nCode )
            {
                case KEY_UP:        MoveSelection( mbVertical ? PREV_FIELD : PREV_LINE );           break;
                case KEY_DOWN:      MoveSelection( mbVertical ? NEXT_FIELD : NEXT_LINE );           break;
                case KEY_LEFT:      MoveSelection( mbVertical ? PREV_LINE : PREV_FIELD );           break;
                case KEY_RIGHT:     MoveSelection( mbVertical ? NEXT_LINE : NEXT_FIELD );           break;
                case KEY_PAGEUP:    MoveSelection( PREV_PAGE );                                     break;
                case KEY_PAGEDOWN:  MoveSelection( NEXT_PAGE );                                     break;
                case KEY_HOME:      MoveSelection( FIRST_FIELD );                                   break;
                case KEY_END:       MoveSelection( LAST_FIELD );                                    break;
                // delete field per DEL key - dialog needs to change focus if window becomes empty
                case KEY_DELETE:    RemoveSelectedField(); mpDialog->NotifyFieldRemoved( *this );   break;
                default:            bKeyEvaluated = false;
            }
        }
    }

    if( !bKeyEvaluated )
        Control::KeyInput( rKEvt );
}

void ScPivotFieldWindow::MouseButtonDown( const MouseEvent& rMEvt )
{
    if( rMEvt.IsLeft() )
    {
        size_t nNewSelectIndex = GetFieldIndex( rMEvt.GetPosPixel() );
        if( nNewSelectIndex < maFields.size() )
        {
            // grabbing after GetFieldIndex() will prevent to focus empty window
            GrabFocusAndSelect( nNewSelectIndex );
            if( rMEvt.GetClicks() == 1 )
            {
                // one click: start tracking
                mbIsTrackingSource = true;
                mnOldFirstVisIndex = mnFirstVisIndex;
                mpDialog->NotifyStartTracking( *this );
            }
            else
            {
                // two clicks: open field options dialog
                mpDialog->NotifyDoubleClick( *this );
            }
        }
    }
}

void ScPivotFieldWindow::RequestHelp( const HelpEvent& rHEvt )
{
    if( (rHEvt.GetMode() & HELPMODE_QUICK) != 0 )
    {
        // show a tooltip with full field name, if field text is clipped
        size_t nFieldIndex = GetFieldIndex( rHEvt.GetMousePosPixel() - GetPosPixel() );
        if( (nFieldIndex < maFields.size()) && maFields[ nFieldIndex ].mbClipped )
        {
            Rectangle aRect( rHEvt.GetMousePosPixel(), GetSizePixel() );
            Help::ShowQuickHelp( this, aRect, maFields[ nFieldIndex ].maFieldName );
            return;
        }
    }
    Control::RequestHelp( rHEvt );
}

void ScPivotFieldWindow::GetFocus()
{
    Control::GetFocus();
    Invalidate();
    ::rtl::Reference< ScAccessibleDataPilotControl > xAcc = GetAccessibleControl();
    if( xAcc.is() )
        xAcc->GotFocus();
}

void ScPivotFieldWindow::LoseFocus()
{
    Control::LoseFocus();
    Invalidate();
    ::rtl::Reference< ScAccessibleDataPilotControl > xAcc = GetAccessibleControl();
    if( xAcc.is() )
        xAcc->LostFocus();
}

uno::Reference< accessibility::XAccessible > ScPivotFieldWindow::CreateAccessible()
{
    mpAccessible = new ScAccessibleDataPilotControl( GetAccessibleParentWindow()->GetAccessible(), this );
    uno::Reference< accessibility::XAccessible > xReturn( mpAccessible );
    mpAccessible->Init();
    mxAccessible = xReturn;
    return xReturn;
}

// private --------------------------------------------------------------------

size_t ScPivotFieldWindow::RecalcVisibleIndex( size_t nSelectIndex ) const
{
    // calculate a scrolling offset that shows the selected field
    size_t nNewFirstVisIndex = mnFirstVisIndex;
    if( nSelectIndex < nNewFirstVisIndex )
        nNewFirstVisIndex = static_cast< size_t >( (nSelectIndex / mnLineSize) * mnLineSize );
    else if( nSelectIndex >= nNewFirstVisIndex + mnPageSize )
        nNewFirstVisIndex = static_cast< size_t >( (nSelectIndex / mnLineSize + 1) * mnLineSize ) - mnPageSize;
    // check if there are complete empty lines in the bottom/right
    size_t nMaxFirstVisIndex = (maFields.size() <= mnPageSize) ? 0 : (((maFields.size() - 1) / mnLineSize + 1) * mnLineSize - mnPageSize);
    return ::std::min( nNewFirstVisIndex, nMaxFirstVisIndex );
}

void ScPivotFieldWindow::SetSelectionUnchecked( size_t nSelectIndex, size_t nFirstVisIndex )
{
    if( !maFields.empty() && (nSelectIndex < maFields.size()) )
    {
        bool bScrollPosChanged = mnFirstVisIndex != nFirstVisIndex;
        bool bSelectionChanged = mnSelectIndex != nSelectIndex;

        sal_Int32 nOldSelected = static_cast< sal_Int32 >( mnSelectIndex );
        mnFirstVisIndex = nFirstVisIndex;
        mnSelectIndex = nSelectIndex;

        if( bScrollPosChanged || bSelectionChanged )
            Invalidate();

        // TODO: accessibility action for changed scrolling position?

        // notify accessibility object about changed selection
        if( bSelectionChanged && HasFocus() )
        {
            ::rtl::Reference< ScAccessibleDataPilotControl > xAcc = GetAccessibleControl();
            if( xAcc.is() )
                xAcc->FieldFocusChange( nOldSelected, static_cast< sal_Int32 >( mnSelectIndex ) );
        }
    }
}

void ScPivotFieldWindow::MoveSelection( size_t nSelectIndex )
{
    if( nSelectIndex < maFields.size() )
        SetSelectionUnchecked( nSelectIndex, RecalcVisibleIndex( nSelectIndex ) );
}

void ScPivotFieldWindow::MoveSelection( MoveType eMoveType )
{
    if( maFields.empty() )
        return;

    size_t nLastIndex = maFields.size() - 1;
    size_t nNewSelectIndex = mnSelectIndex;
    switch( eMoveType )
    {
        case PREV_FIELD:
            nNewSelectIndex = (nNewSelectIndex > 0) ? (nNewSelectIndex - 1) : 0;
        break;
        case NEXT_FIELD:
            nNewSelectIndex = (nNewSelectIndex < nLastIndex) ? (nNewSelectIndex + 1) : nLastIndex;
        break;
        case PREV_LINE:
            nNewSelectIndex = (nNewSelectIndex > mnLineSize) ? (nNewSelectIndex - mnLineSize) : 0;
        break;
        case NEXT_LINE:
            nNewSelectIndex = (nNewSelectIndex + mnLineSize < nLastIndex) ? (nNewSelectIndex + mnLineSize) : nLastIndex;
        break;
        case PREV_PAGE:
            nNewSelectIndex = (nNewSelectIndex > mnPageSize) ? (nNewSelectIndex - mnPageSize) : 0;
        break;
        case NEXT_PAGE:
            nNewSelectIndex = (nNewSelectIndex + mnPageSize < nLastIndex) ? (nNewSelectIndex + mnPageSize) : nLastIndex;
        break;
        case FIRST_FIELD:
            nNewSelectIndex = 0;
        break;
        case LAST_FIELD:
            nNewSelectIndex = nLastIndex;
        break;
    }

    // SetSelectionUnchecked() redraws the control and updates the scrollbar
    SetSelectionUnchecked( nNewSelectIndex, RecalcVisibleIndex( nNewSelectIndex ) );
}

void ScPivotFieldWindow::MoveSelectedField( MoveType eMoveType )
{
    if( mnSelectIndex < maFields.size() )
    {
        // find position to insert the field by changing the selection first
        size_t nOldSelectIndex = mnSelectIndex;
        MoveSelection( eMoveType );
        MoveField( nOldSelectIndex, (nOldSelectIndex < mnSelectIndex) ? (mnSelectIndex + 1) : mnSelectIndex );
    }
}

void ScPivotFieldWindow::InsertFieldUnchecked( size_t nInsertIndex, const ScPivotWindowField& rField )
{
    maFields.insert( maFields.begin() + nInsertIndex, rField );
    ::rtl::Reference< ScAccessibleDataPilotControl > xAcc = GetAccessibleControl();
    if( xAcc.is() )
        xAcc->AddField( static_cast< sal_Int32 >( nInsertIndex ) );
}

void ScPivotFieldWindow::RemoveFieldUnchecked( size_t nRemoveIndex )
{
    ::rtl::Reference< ScAccessibleDataPilotControl > xAcc = GetAccessibleControl();
    if( xAcc.is() )
        xAcc->RemoveField( static_cast< sal_Int32 >( nRemoveIndex ) );
    maFields.erase( maFields.begin() + nRemoveIndex );
}

void ScPivotFieldWindow::DrawBackground( OutputDevice& rDev )
{
    Size aDevSize = rDev.GetOutputSizePixel();
    const StyleSettings& rStyleSett = GetSettings().GetStyleSettings();

    if( meFieldType == PIVOTFIELDTYPE_SELECT )
    {
        rDev.SetLineColor();
        rDev.SetFillColor( rStyleSett.GetFaceColor() );
        rDev.DrawRect( Rectangle( Point( 0, 0 ), aDevSize ) );
    }
    else
    {
        rDev.SetLineColor( rStyleSett.GetWindowTextColor() );
        rDev.SetFillColor( rStyleSett.GetWindowColor() );
        rDev.DrawRect( Rectangle( Point( 0, 0 ), aDevSize ) );

        /*  Draw the caption text. This needs some special handling, because we
            support hard line breaks here. This part will draw each line of the
            text for itself. */
        rDev.SetTextColor( rStyleSett.GetWindowTextColor() );
        xub_StrLen nTokenCnt = GetText().GetTokenCount( '\n' );
        long nY = (aDevSize.Height() - nTokenCnt * rDev.GetTextHeight()) / 2;
        for( xub_StrLen nToken = 0, nStringIx = 0; nToken < nTokenCnt; ++nToken )
        {
            String aLine = GetText().GetToken( 0, '\n', nStringIx );
            Point aLinePos( (aDevSize.Width() - rDev.GetCtrlTextWidth( aLine )) / 2, nY );
            rDev.DrawCtrlText( aLinePos, aLine );
            nY += rDev.GetTextHeight();
        }
    }
}

void ScPivotFieldWindow::DrawField( OutputDevice& rDev, size_t nFieldIndex )
{
    if( (nFieldIndex < maFields.size()) && (mnFirstVisIndex <= nFieldIndex) && (nFieldIndex < mnFirstVisIndex + mnPageSize) )
    {
        // draw the button
        Point aFieldPos = GetFieldPosition( nFieldIndex );
        bool bFocus = HasFocus() && (nFieldIndex == mnSelectIndex);
        DecorationView aDecoView( &rDev );
        aDecoView.DrawButton( Rectangle( aFieldPos, maFieldSize ), bFocus ? BUTTON_DRAW_DEFAULT : 0 );

        // #i31600# if text is too long, cut and add ellipsis
        const OUString& rFullText = maFields[ nFieldIndex ].maFieldName;
        OUString aClippedText = rFullText;
        long nLabelWidth = rDev.GetTextWidth( rFullText );
        if( (maFields[ nFieldIndex ].mbClipped = nLabelWidth + 6 > maFieldSize.Width()) == true )
        {
            sal_Int32 nMinLen = 0;
            sal_Int32 nMaxLen = rFullText.getLength();
            bool bFits = false;
            do
            {
                sal_Int32 nCurrLen = (nMinLen + nMaxLen) / 2;
                aClippedText = rFullText.copy( 0, nCurrLen ) + OUString( RTL_CONSTASCII_USTRINGPARAM( "..." ) );
                nLabelWidth = rDev.GetTextWidth( aClippedText );
                bFits = nLabelWidth + 6 <= maFieldSize.Width();
                (bFits ? nMinLen : nMaxLen) = nCurrLen;
            }
            while( !bFits || (nMinLen + 1 < nMaxLen) );
        }

        // draw the button text
        Point aLabelOffset( (maFieldSize.Width() - nLabelWidth) / 2, ::std::max< long >( (maFieldSize.Height() - rDev.GetTextHeight()) / 2, 3 ) );
        rDev.SetTextColor( GetSettings().GetStyleSettings().GetButtonTextColor() );
        rDev.DrawText( aFieldPos + aLabelOffset, aClippedText );
    }
}

void ScPivotFieldWindow::DrawInsertionCursor( OutputDevice& rDev )
{
    if( (mnInsCursorIndex <= maFields.size()) && (mnFirstVisIndex <= mnInsCursorIndex) && (mnInsCursorIndex <= mnFirstVisIndex + mnPageSize) &&
        (!mbIsTrackingSource || (mnInsCursorIndex < mnSelectIndex) || (mnInsCursorIndex > mnSelectIndex + 1)) )
    {
        Color aTextColor = GetSettings().GetStyleSettings().GetButtonTextColor();
        rDev.SetLineColor( aTextColor );
        rDev.SetFillColor( aTextColor );

        bool bVerticalCursor = mnColCount > 1;
        long nCursorLength = bVerticalCursor ? maFieldSize.Height() : maFieldSize.Width();

        bool bEndOfLastField = mnInsCursorIndex == mnFirstVisIndex + mnPageSize;
        Point aMainLinePos = GetFieldPosition( bEndOfLastField ? (mnInsCursorIndex - 1) : mnInsCursorIndex );
        if( bEndOfLastField )
            (bVerticalCursor ? aMainLinePos.X() : aMainLinePos.Y()) += ((bVerticalCursor ? maFieldSize.Width() : maFieldSize.Height()) - CURSOR_WIDTH);
        else if( (bVerticalCursor ? aMainLinePos.X() : aMainLinePos.Y()) > 0 )
            (bVerticalCursor ? aMainLinePos.X() : aMainLinePos.Y()) -= ((CURSOR_WIDTH + 1) / 2);
        Size aMainLineSize( bVerticalCursor ? CURSOR_WIDTH : nCursorLength, bVerticalCursor ? nCursorLength : CURSOR_WIDTH );
        rDev.DrawRect( Rectangle( aMainLinePos, aMainLineSize ) );

        Point aSubLinePos = aMainLinePos;
        (bVerticalCursor ? aSubLinePos.X() : aSubLinePos.Y()) -= CURSOR_WIDTH;
        Size aSubLineSize( bVerticalCursor ? (3 * CURSOR_WIDTH) : CURSOR_WIDTH, bVerticalCursor ? CURSOR_WIDTH : (3 * CURSOR_WIDTH) );
        rDev.DrawRect( Rectangle( aSubLinePos, aSubLineSize ) );

        (bVerticalCursor ? aSubLinePos.Y() : aSubLinePos.X()) += (nCursorLength - CURSOR_WIDTH);
        rDev.DrawRect( Rectangle( aSubLinePos, aSubLineSize ) );
    }
}

::rtl::Reference< ScAccessibleDataPilotControl > ScPivotFieldWindow::GetAccessibleControl()
{
    ::rtl::Reference< ScAccessibleDataPilotControl > xAccImpl;
    if( mpAccessible )
    {
        // try to resolve the weak reference mxAccessible
        uno::Reference< accessibility::XAccessible > xAcc = mxAccessible;
        if( xAcc.is() )
            xAccImpl.set( mpAccessible );   // the rtl reference keeps the object alive
        else
            mpAccessible = 0;               // object is dead, forget the pointer
    }
    return xAccImpl;
 }

// handlers -------------------------------------------------------------------

IMPL_LINK( ScPivotFieldWindow, ScrollHdl, ScrollBar*, pScrollBar )
{
    // scrollbar may return negative values, if it is too small
    long nThumbPos = pScrollBar->GetThumbPos();
    if( nThumbPos >= 0 )
    {
        size_t nNewFirstVisIndex = static_cast< size_t >( nThumbPos * mnLineSize );
        // keep the selection index on same relative position inside row/column
        size_t nSelectLineOffset = mnSelectIndex % mnLineSize;
        size_t nNewSelectIndex = mnSelectIndex;
        if( nNewSelectIndex < nNewFirstVisIndex )
            nNewSelectIndex = nNewFirstVisIndex + nSelectLineOffset;
        else if( nNewSelectIndex >= nNewFirstVisIndex + mnPageSize )
            nNewSelectIndex = nNewFirstVisIndex + mnPageSize - mnLineSize + nSelectLineOffset;
        nNewSelectIndex = ::std::min( nNewSelectIndex, maFields.size() - 1 );
        SetSelectionUnchecked( nNewSelectIndex, nNewFirstVisIndex );
    }
    GrabFocus();
    return 0;
}

// ============================================================================
