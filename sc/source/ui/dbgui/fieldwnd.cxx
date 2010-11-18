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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

#include <vcl/virdev.hxx>
#include <vcl/decoview.hxx>
#include <vcl/svapp.hxx>
#include <vcl/mnemonic.hxx>
#include <vcl/help.hxx>
#include <tools/debug.hxx>

#include "fieldwnd.hxx"
#include "pvlaydlg.hxx"
#include "dpuiglobal.hxx"
#include "AccessibleDataPilotControl.hxx"
#include "scresid.hxx"
#include "pivot.hrc"

using ::rtl::OUString;
using ::std::vector;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::WeakReference;
using ::com::sun::star::accessibility::XAccessible;

#include <stdio.h>
#include <string>
#include <sys/time.h>

namespace {

class StackPrinter
{
public:
    explicit StackPrinter(const char* msg) :
        msMsg(msg)
    {
        fprintf(stdout, "%s: --begin\n", msMsg.c_str());
        mfStartTime = getTime();
    }

    ~StackPrinter()
    {
        double fEndTime = getTime();
        fprintf(stdout, "%s: --end (duration: %g sec)\n", msMsg.c_str(), (fEndTime-mfStartTime));
    }

    void printTime(int line) const
    {
        double fEndTime = getTime();
        fprintf(stdout, "%s: --(%d) (duration: %g sec)\n", msMsg.c_str(), line, (fEndTime-mfStartTime));
    }

private:
    double getTime() const
    {
        timeval tv;
        gettimeofday(&tv, NULL);
        return tv.tv_sec + tv.tv_usec / 1000000.0;
    }

    ::std::string msMsg;
    double mfStartTime;
};

}

const size_t INVALID_INDEX = static_cast<size_t>(-1);

ScDPFieldControlBase::ScDPFieldControlBase( ScDPLayoutDlg* pParent, const ResId& rResId, FixedText* pCaption ) :
    Control(pParent, rResId),
    mpDlg(pParent),
    mpCaption(pCaption),
    mnFieldSelected(0),
    pAccessible(NULL)
{
    if (pCaption)
        maName = MnemonicGenerator::EraseAllMnemonicChars( pCaption->GetText() );
}

ScDPFieldControlBase::~ScDPFieldControlBase()
{
    if (pAccessible)
    {
        com::sun::star::uno::Reference < com::sun::star::accessibility::XAccessible > xTempAcc = xAccessible;
        if (xTempAcc.is())
            pAccessible->dispose();
    }
}

void ScDPFieldControlBase::UseMnemonic()
{
    // Now the FixedText has its mnemonic char. Grab the text and hide the
    // FixedText to be able to handle tabstop and mnemonics separately.
    if (mpCaption)
    {
        SetText(mpCaption->GetText());
        mpCaption->Hide();
    }

    // after reading the mnemonics, tab stop style bits can be updated
    UpdateStyle();
}

OUString ScDPFieldControlBase::GetName() const
{
    return maName;
}

void ScDPFieldControlBase::SetName(const OUString& rName)
{
    maName = rName;
}

bool ScDPFieldControlBase::IsExistingIndex( size_t nIndex ) const
{
    return nIndex < maFieldNames.size();
}

void ScDPFieldControlBase::AddField( const String& rText, size_t nNewIndex )
{
    DBG_ASSERT( nNewIndex == maFieldNames.size(), "ScDPFieldWindow::AddField - invalid index" );
    if( IsValidIndex( nNewIndex ) )
    {
        maFieldNames.push_back( FieldName( rText, true ) );
        if (pAccessible)
        {
            com::sun::star::uno::Reference < com::sun::star::accessibility::XAccessible > xTempAcc = xAccessible;
            if (xTempAcc.is())
                pAccessible->AddField(nNewIndex);
            else
                pAccessible = NULL;
        }
    }
}

bool ScDPFieldControlBase::AddField( const String& rText, const Point& rPos, size_t& rnIndex )
{
    size_t nNewIndex = 0;
    if( GetFieldIndex( rPos, nNewIndex ) )
    {
        if( nNewIndex > maFieldNames.size() )
            nNewIndex = maFieldNames.size();

        maFieldNames.insert( maFieldNames.begin() + nNewIndex, FieldName( rText, true ) );
        mnFieldSelected = nNewIndex;
        ResetScrollBar();
        Redraw();
        rnIndex = nNewIndex;

        if (pAccessible)
        {
            com::sun::star::uno::Reference < com::sun::star::accessibility::XAccessible > xTempAcc = xAccessible;
            if (xTempAcc.is())
                pAccessible->AddField(nNewIndex);
            else
                pAccessible = NULL;
        }

        return true;
    }

    return false;
}

bool ScDPFieldControlBase::AppendField(const String& rText, size_t& rnIndex)
{
    if (!IsValidIndex(maFieldNames.size()))
        return false;

    maFieldNames.push_back(FieldName(rText, true));
    mnFieldSelected = maFieldNames.size() - 1;
    ResetScrollBar();
    Redraw();

    rnIndex = mnFieldSelected;
    return true;
}

void ScDPFieldControlBase::DelField( size_t nDelIndex )
{
    if ( IsExistingIndex(nDelIndex) )
    {
        if (pAccessible) // before decrement fieldcount
        {
            com::sun::star::uno::Reference < com::sun::star::accessibility::XAccessible > xTempAcc = xAccessible;
            if (xTempAcc.is())
                pAccessible->RemoveField(nDelIndex);
            else
                pAccessible = NULL;
        }
        maFieldNames.erase( maFieldNames.begin() + nDelIndex );
        if (mnFieldSelected >= maFieldNames.size())
            mnFieldSelected = maFieldNames.size() - 1;

        ResetScrollBar();
        Redraw();
    }
}

size_t ScDPFieldControlBase::GetFieldCount() const
{
    return maFieldNames.size();
}

bool ScDPFieldControlBase::IsEmpty() const
{
    return maFieldNames.empty();
}

void ScDPFieldControlBase::ClearFields()
{
    com::sun::star::uno::Reference < com::sun::star::accessibility::XAccessible > xTempAcc = xAccessible;
    if (!xTempAcc.is() && pAccessible)
        pAccessible = NULL;
    if (pAccessible)
        for( size_t nIdx = maFieldNames.size(); nIdx > 0; --nIdx )
            pAccessible->RemoveField( nIdx - 1 );

    maFieldNames.clear();
}

void ScDPFieldControlBase::SetFieldText( const String& rText, size_t nIndex )
{
    if( IsExistingIndex( nIndex ) )
    {
        maFieldNames[ nIndex ] = FieldName( rText, true );
        Redraw();

        if (pAccessible)
        {
            com::sun::star::uno::Reference < com::sun::star::accessibility::XAccessible > xTempAcc = xAccessible;
            if (xTempAcc.is())
                pAccessible->FieldNameChange(nIndex);
            else
                pAccessible = NULL;
        }
    }
}

const String& ScDPFieldControlBase::GetFieldText( size_t nIndex ) const
{
    if( IsExistingIndex( nIndex ) )
        return maFieldNames[ nIndex ].first;
    return EMPTY_STRING;
}

void ScDPFieldControlBase::GetExistingIndex( const Point& rPos, size_t& rnIndex )
{
    if( !maFieldNames.empty() && (GetFieldType() != TYPE_SELECT) && GetFieldIndex( rPos, rnIndex ) )
    {
        if( rnIndex >= maFieldNames.size() )
            rnIndex = maFieldNames.size() - 1;
    }
    else
        rnIndex = 0;
}

size_t ScDPFieldControlBase::GetSelectedField() const
{
    return mnFieldSelected;
}

void ScDPFieldControlBase::SetSelectedField(size_t nSelected)
{
    mnFieldSelected = nSelected;
}

vector<ScDPFieldControlBase::FieldName>& ScDPFieldControlBase::GetFieldNames()
{
    return maFieldNames;
}

const vector<ScDPFieldControlBase::FieldName>& ScDPFieldControlBase::GetFieldNames() const
{
    return maFieldNames;
}

void ScDPFieldControlBase::Paint( const Rectangle& /* rRect */ )
{
    // #124828# hiding the caption is now done from StateChanged
    Redraw();
}

void ScDPFieldControlBase::DataChanged( const DataChangedEvent& rDCEvt )
{
    if( (rDCEvt.GetType() == DATACHANGED_SETTINGS) && (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        Redraw();
    }
    Control::DataChanged( rDCEvt );
}

void ScDPFieldControlBase::MouseButtonDown( const MouseEvent& rMEvt )
{
    if( rMEvt.IsLeft() )
    {
        size_t nIndex = 0;
        if( GetFieldIndex( rMEvt.GetPosPixel(), nIndex ) && IsExistingIndex( nIndex ) )
        {
            GrabFocusWithSel( nIndex );

            if( rMEvt.GetClicks() == 1 )
            {
                PointerStyle ePtr = mpDlg->NotifyMouseButtonDown( GetFieldType(), nIndex );
                CaptureMouse();
                SetPointer( Pointer( ePtr ) );
            }
            else
                mpDlg->NotifyDoubleClick( GetFieldType(), nIndex );
        }
    }
}

void ScDPFieldControlBase::MouseButtonUp( const MouseEvent& rMEvt )
{
    if( rMEvt.IsLeft() )
    {
        if( rMEvt.GetClicks() == 1 )
        {
            mpDlg->NotifyMouseButtonUp( OutputToScreenPixel( rMEvt.GetPosPixel() ) );
            SetPointer( Pointer( POINTER_ARROW ) );
        }

        if( IsMouseCaptured() )
            ReleaseMouse();
    }
}

void ScDPFieldControlBase::MouseMove( const MouseEvent& rMEvt )
{
    if( IsMouseCaptured() )
    {
        PointerStyle ePtr = mpDlg->NotifyMouseMove( OutputToScreenPixel( rMEvt.GetPosPixel() ) );
        SetPointer( Pointer( ePtr ) );
    }
    size_t nIndex = 0;
    if( GetFieldIndex( rMEvt.GetPosPixel(), nIndex ) && IsShortenedText( nIndex ) )
    {
        Point aPos = OutputToScreenPixel( rMEvt.GetPosPixel() );
        Rectangle   aRect( aPos, GetSizePixel() );
        String aHelpText = GetFieldText(nIndex);
        Help::ShowQuickHelp( this, aRect, aHelpText );
    }
}

void ScDPFieldControlBase::KeyInput( const KeyEvent& rKEvt )
{
    const KeyCode& rKeyCode = rKEvt.GetKeyCode();
    USHORT nCode = rKeyCode.GetCode();
    bool bKeyEvaluated = false;

    const FieldNames& rFields = GetFieldNames();
    if( rKeyCode.IsMod1() && (GetFieldType() != TYPE_SELECT) )
    {
        bKeyEvaluated = true;
        switch( nCode )
        {
            case KEY_UP:    MoveFieldRel( 0, -1 );              break;
            case KEY_DOWN:  MoveFieldRel( 0, 1 );               break;
            case KEY_LEFT:  MoveFieldRel( -1, 0 );              break;
            case KEY_RIGHT: MoveFieldRel( 1, 0 );               break;
            case KEY_HOME:  MoveField( 0 );                     break;
            case KEY_END:   MoveField( rFields.size() - 1 );  break;
            default:        bKeyEvaluated = false;
        }
    }
    else
    {
        bKeyEvaluated = true;
        switch( nCode )
        {
            case KEY_UP:    MoveSelection( nCode, 0, -1 );          break;
            case KEY_DOWN:  MoveSelection( nCode, 0, 1 );           break;
            case KEY_LEFT:  MoveSelection( nCode, -1, 0 );          break;
            case KEY_RIGHT: MoveSelection( nCode, 1, 0 );           break;
            case KEY_HOME:  SetSelectionHome();                     break;
            case KEY_END:   SetSelectionEnd();                      break;
            case KEY_DELETE:
                mpDlg->NotifyRemoveField( GetFieldType(), mnFieldSelected );   break;
            default:        bKeyEvaluated = false;
        }
    }

    if (bKeyEvaluated)
    {
        ScrollToShowSelection();
        Redraw();
    }
    else
        Control::KeyInput( rKEvt );
}

void ScDPFieldControlBase::GetFocus()
{
    Control::GetFocus();
    Redraw();
    if( GetGetFocusFlags() & GETFOCUS_MNEMONIC )    // move field on shortcut
    {
        size_t nOldCount = GetFieldCount();
        mpDlg->NotifyMoveFieldToEnd( GetFieldType() );
        if (GetFieldCount() > nOldCount)
            // Scroll to the end only when a new field is inserted.
            ScrollToEnd();
    }
    else                                            // else change focus
        mpDlg->NotifyFieldFocus( GetFieldType(), TRUE );

    AccessibleSetFocus(true);
}

void ScDPFieldControlBase::LoseFocus()
{
    Control::LoseFocus();
    Redraw();
    mpDlg->NotifyFieldFocus( GetFieldType(), FALSE );

    AccessibleSetFocus(false);
}

Reference<XAccessible> ScDPFieldControlBase::CreateAccessible()
{
    pAccessible =
        new ScAccessibleDataPilotControl(GetAccessibleParentWindow()->GetAccessible(), this);

    com::sun::star::uno::Reference < ::com::sun::star::accessibility::XAccessible > xReturn = pAccessible;

    pAccessible->Init();
    xAccessible = xReturn;

    return xReturn;
}

void ScDPFieldControlBase::FieldFocusChanged(size_t nOldSelected, size_t nFieldSelected)
{
    if (!pAccessible)
        return;

    com::sun::star::uno::Reference < com::sun::star::accessibility::XAccessible > xTempAcc = xAccessible;
    if (xTempAcc.is())
        pAccessible->FieldFocusChange(nOldSelected, nFieldSelected);
    else
        pAccessible = NULL;
}

void ScDPFieldControlBase::AccessibleSetFocus(bool bOn)
{
    if (!pAccessible)
        return;

    com::sun::star::uno::Reference < com::sun::star::accessibility::XAccessible > xTempAcc = xAccessible;
    if (!xTempAcc.is())
    {
        pAccessible = NULL;
        return;
    }

    if (bOn)
        pAccessible->GotFocus();
    else
        pAccessible->LostFocus();
}

void ScDPFieldControlBase::UpdateStyle()
{
    WinBits nMask = ~(WB_TABSTOP | WB_NOTABSTOP);
    SetStyle( (GetStyle() & nMask) | (IsEmpty() ? WB_NOTABSTOP : WB_TABSTOP) );
}

void ScDPFieldControlBase::DrawBackground( OutputDevice& rDev )
{
    const StyleSettings& rStyleSet = GetSettings().GetStyleSettings();
    Color aFaceColor = rStyleSet.GetFaceColor();
    Color aWinColor = rStyleSet.GetWindowColor();
    Color aTextColor = rStyleSet.GetButtonTextColor();
    Color aWinTextColor = rStyleSet.GetWindowTextColor();

    Point aPos0;
    Size aSize( GetSizePixel() );

    rDev.SetLineColor( aWinTextColor );
    rDev.SetFillColor( aWinColor );
    rDev.DrawRect( Rectangle( aPos0, aSize ) );

    rDev.SetTextColor( aWinTextColor );

    /*  Draw the caption text. This needs some special handling, because we
        support hard line breaks here. This part will draw each line of the
        text for itself. */

    xub_StrLen nTokenCnt = GetText().GetTokenCount('\n');
    long nY = (aSize.Height() - nTokenCnt * rDev.GetTextHeight()) / 2;
    for( xub_StrLen nToken = 0, nStringIx = 0; nToken < nTokenCnt; ++nToken )
    {
        String aLine( GetText().GetToken( 0, '\n', nStringIx ) );
        Point aLinePos( (aSize.Width() - rDev.GetCtrlTextWidth( aLine )) / 2, nY );
        rDev.DrawCtrlText( aLinePos, aLine );
        nY += rDev.GetTextHeight();
    }
}

void ScDPFieldControlBase::DrawField(
        OutputDevice& rDev, const Rectangle& rRect, FieldName& rText, bool bFocus )
{
    const StyleSettings& rStyleSet = GetSettings().GetStyleSettings();
    Color aTextColor = rStyleSet.GetButtonTextColor();

    VirtualDevice aVirDev( rDev );
    // #i97623# VirtualDevice is always LTR while other windows derive direction from parent
    aVirDev.EnableRTL( IsRTLEnabled() );

    String aText = rText.first;
    Size aDevSize( rRect.GetSize() );
    long    nWidth       = aDevSize.Width();
    long    nHeight      = aDevSize.Height();
    long    nLabelWidth  = rDev.GetTextWidth( aText );
    long    nLabelHeight = rDev.GetTextHeight();

    // #i31600# if text is too long, cut and add ellipsis
    rText.second = nLabelWidth + 6 <= nWidth;
    if( !rText.second )
    {
        xub_StrLen nMinLen = 0;
        xub_StrLen nMaxLen = aText.Len();
        bool bFits = false;
        do
        {
            xub_StrLen nCurrLen = (nMinLen + nMaxLen) / 2;
            aText = String( rText.first, 0, nCurrLen ).AppendAscii( "..." );
            nLabelWidth = rDev.GetTextWidth( aText );
            bFits = nLabelWidth + 6 <= nWidth;
            (bFits ? nMinLen : nMaxLen) = nCurrLen;
        }
        while( !bFits || (nMinLen + 1 < nMaxLen) );
    }
    Point aLabelPos( (nWidth - nLabelWidth) / 2, ::std::max< long >( (nHeight - nLabelHeight) / 2, 3 ) );

    aVirDev.SetOutputSizePixel( aDevSize );
    aVirDev.SetFont( rDev.GetFont() );
    DecorationView aDecoView( &aVirDev );
    aDecoView.DrawButton( Rectangle( Point( 0, 0 ), aDevSize ), bFocus ? BUTTON_DRAW_DEFAULT : 0 );
    aVirDev.SetTextColor( aTextColor );
    aVirDev.DrawText( aLabelPos, aText );
    rDev.DrawBitmap( rRect.TopLeft(), aVirDev.GetBitmap( Point( 0, 0 ), aDevSize ) );
}

ScDPLayoutDlg* ScDPFieldControlBase::GetParentDlg() const
{
    return mpDlg;
}

void ScDPFieldControlBase::AppendPaintable(Window* p)
{
    maPaintables.push_back(p);
}

void ScDPFieldControlBase::DrawPaintables()
{
    Rectangle aRect(GetPosPixel(), GetSizePixel());
    Paintables::iterator itr = maPaintables.begin(), itrEnd = maPaintables.end();
    for (; itr != itrEnd; ++itr)
    {
        Window* p = *itr;
        if (!p->IsVisible())
            continue;

        p->Paint(aRect);
    }
}

void ScDPFieldControlBase::DrawInvertSelection()
{
    if (!HasFocus())
        return;

    if (mnFieldSelected >= maFieldNames.size())
        return;

    size_t nPos = GetDisplayPosition(mnFieldSelected);
    if (nPos == INVALID_INDEX)
        return;

    Size aFldSize = GetFieldSize();
    long nFldWidth = aFldSize.Width();
    long nSelWidth = std::min<long>(
        GetTextWidth(maFieldNames[mnFieldSelected].first) + 4, nFldWidth - 6);

    Point aPos = GetFieldPosition(nPos);
    aPos += Point((nFldWidth - nSelWidth) / 2, 3);
    Size aSize(nSelWidth, aFldSize.Height() - 6);

    Rectangle aSel(aPos, aSize);
    InvertTracking(aSel, SHOWTRACK_SMALL | SHOWTRACK_WINDOW);
}

bool ScDPFieldControlBase::IsShortenedText( size_t nIndex ) const
{
    const FieldNames& rFields = GetFieldNames();
    return (nIndex < rFields.size()) && !rFields[nIndex].second;
}

void ScDPFieldControlBase::MoveField( size_t nDestIndex )
{
    if (nDestIndex != mnFieldSelected)
    {
        swap(maFieldNames[nDestIndex], maFieldNames[mnFieldSelected]);
        mnFieldSelected = nDestIndex;
    }
}

void ScDPFieldControlBase::MoveFieldRel( SCsCOL nDX, SCsROW nDY )
{
    MoveField( CalcNewFieldIndex( nDX, nDY ) );
}

void ScDPFieldControlBase::SetSelection(size_t nIndex)
{
    FieldNames& rFields = GetFieldNames();
    if (rFields.empty())
        return;

    if( mnFieldSelected >= rFields.size() )
        mnFieldSelected = rFields.size() - 1;
    if( mnFieldSelected != nIndex )
    {
        size_t nOldSelected = mnFieldSelected;
        mnFieldSelected = nIndex;
        Redraw();

        if (HasFocus())
            FieldFocusChanged(nOldSelected, mnFieldSelected);
    }
}

void ScDPFieldControlBase::SetSelectionHome()
{
    const FieldNames& rFields = GetFieldNames();
    if( !rFields.empty() )
    {
        SetSelection( 0 );
    }
}

void ScDPFieldControlBase::SetSelectionEnd()
{
    const FieldNames& rFields = GetFieldNames();
    if( !rFields.empty() )
    {
        SetSelection( rFields.size() - 1 );
    }
}

void ScDPFieldControlBase::MoveSelection( USHORT nKeyCode, SCsCOL nDX, SCsROW nDY )
{
    size_t nNewIndex = CalcNewFieldIndex( nDX, nDY );
    SetSelection( nNewIndex );
}

void ScDPFieldControlBase::ModifySelectionOffset( long nOffsetDiff )
{
    mnFieldSelected -= nOffsetDiff;
    Redraw();
}

void ScDPFieldControlBase::SelectNext()
{
    if( GetFieldType() == TYPE_SELECT )
        MoveSelection( KEY_DOWN, 0, 1 );
}

void ScDPFieldControlBase::GrabFocusWithSel( size_t nIndex )
{
    SetSelection( nIndex );
    if( !HasFocus() )
        GrabFocus();
}

//=============================================================================

ScDPHorFieldControl::ScDPHorFieldControl(
    ScDPLayoutDlg* pDialog, const ResId& rResId, FixedText* pCaption) :
    ScDPFieldControlBase(pDialog, rResId, pCaption),
    maScroll(this, WB_HORZ | WB_DRAG),
    mnFieldBtnRowCount(0),
    mnFieldBtnColCount(0)
{
    maScroll.SetScrollHdl( LINK(this, ScDPHorFieldControl, ScrollHdl) );
    maScroll.SetEndScrollHdl( LINK(this, ScDPHorFieldControl, EndScrollHdl) );
    maScroll.Show();

    AppendPaintable(&maScroll);
}

ScDPHorFieldControl::~ScDPHorFieldControl()
{
}

Point ScDPHorFieldControl::GetFieldPosition( size_t nIndex )
{
    Point aPos;
    Size aSize;
    GetFieldBtnPosSize(nIndex, aPos, aSize);
    return aPos;
}

Size ScDPHorFieldControl::GetFieldSize() const
{
    return Size(FIELD_BTN_WIDTH, FIELD_BTN_HEIGHT);
}

bool ScDPHorFieldControl::GetFieldIndex( const Point& rPos, size_t& rnIndex )
{
    rnIndex = INVALID_INDEX;
    if (rPos.X() < 0 || rPos.Y() < 0)
        return false;

    Size aWndSize = GetSizePixel();
    if (rPos.X() > aWndSize.Width() || rPos.Y() > aWndSize.Height())
        return false;

    size_t nX = rPos.X();
    size_t nY = rPos.Y();
    size_t nW = aWndSize.Width();
    size_t nH = aWndSize.Height();

    size_t nCurX = OUTER_MARGIN_HOR + FIELD_BTN_WIDTH + ROW_FIELD_BTN_GAP/2;
    size_t nCurY = OUTER_MARGIN_VER + FIELD_BTN_HEIGHT + ROW_FIELD_BTN_GAP/2;
    size_t nCol = 0;
    size_t nRow = 0;
    while (nX > nCurX && nCurX <= nW)
    {
        nCurX += FIELD_BTN_WIDTH + ROW_FIELD_BTN_GAP;
        ++nCol;
    }
    while (nY > nCurY && nCurY <= nH)
    {
        nCurY += FIELD_BTN_HEIGHT + ROW_FIELD_BTN_GAP;
        ++nRow;
    }

    size_t nOffset = maScroll.GetThumbPos();
    nCol += nOffset; // convert to logical column ID.
    rnIndex = nCol * mnFieldBtnRowCount + nRow;
    size_t nFldCount = GetFieldCount();
    if (rnIndex > nFldCount)
        rnIndex = nFldCount;
    return IsValidIndex(rnIndex);
}

void ScDPHorFieldControl::Redraw()
{
    const StyleSettings& rStyleSet = GetSettings().GetStyleSettings();
    Color aFaceColor = rStyleSet.GetFaceColor();
    Color aWinColor = rStyleSet.GetWindowColor();
    Color aTextColor = rStyleSet.GetButtonTextColor();
    Color aWinTextColor = rStyleSet.GetWindowTextColor();

    VirtualDevice   aVirDev;
    // #i97623# VirtualDevice is always LTR while other windows derive direction from parent
    aVirDev.EnableRTL( IsRTLEnabled() );
    aVirDev.SetMapMode( MAP_PIXEL );

    Point           aPos0;
    Size            aSize( GetSizePixel() );
    Font            aFont( GetFont() );         // Font vom Window
    aFont.SetTransparent( TRUE );
    aVirDev.SetFont( aFont );
    aVirDev.SetOutputSizePixel( aSize );

    DrawBackground( aVirDev );

    FieldNames& rFields = GetFieldNames();
    {
        long nScrollOffset = maScroll.GetThumbPos();
        FieldNames::iterator itr = rFields.begin(), itrEnd = rFields.end();
        if (nScrollOffset)
            ::std::advance(itr, nScrollOffset*mnFieldBtnRowCount);

        for (size_t i = 0; itr != itrEnd; ++itr, ++i)
        {
            Point aFldPt;
            Size aFldSize;
            if (!GetFieldBtnPosSize(i, aFldPt, aFldSize))
                break;

            size_t nField = i + nScrollOffset*mnFieldBtnRowCount;
            bool bFocus = HasFocus() && (nField == GetSelectedField());
            DrawField(aVirDev, Rectangle(aFldPt, aFldSize), *itr, bFocus);
        }
    }

    DrawBitmap( aPos0, aVirDev.GetBitmap( aPos0, aSize ) );
    DrawPaintables();
    DrawInvertSelection();
    UpdateStyle();
}

void ScDPHorFieldControl::CalcSize()
{
    Size aWndSize = GetSizePixel();

    long nScrollSize = GetSettings().GetStyleSettings().GetScrollBarSize();
    maScroll.SetSizePixel(Size(aWndSize.Width() - OUTER_MARGIN_HOR*2, nScrollSize));
    maScroll.SetPosPixel(Point(OUTER_MARGIN_HOR, aWndSize.Height() - OUTER_MARGIN_VER - nScrollSize));

    long nTotalH = aWndSize.Height() - nScrollSize - OUTER_MARGIN_VER*2;
    long nTotalW = aWndSize.Width() - OUTER_MARGIN_HOR*2;
    mnFieldBtnRowCount = nTotalH / (FIELD_BTN_HEIGHT + ROW_FIELD_BTN_GAP);
    mnFieldBtnColCount = (nTotalW + ROW_FIELD_BTN_GAP) / (FIELD_BTN_WIDTH + ROW_FIELD_BTN_GAP);

    maScroll.SetLineSize(mnFieldBtnRowCount);
    maScroll.SetVisibleSize(mnFieldBtnColCount);
    maScroll.SetPageSize(mnFieldBtnColCount);
    maScroll.SetRange(Range(0, mnFieldBtnColCount));
}

bool ScDPHorFieldControl::IsValidIndex(size_t /*nIndex*/) const
{
    return true;
}

size_t ScDPHorFieldControl::CalcNewFieldIndex(SCsCOL nDX, SCsROW nDY) const
{
    size_t nSel = GetSelectedField();
    size_t nFldCount = GetFieldCount();
    SCsROW nRow = nSel % mnFieldBtnRowCount;
    SCsCOL nCol = nSel / mnFieldBtnRowCount;
    SCsCOL nColUpper = ceil(
        static_cast<double>(nFldCount) / static_cast<double>(mnFieldBtnRowCount)) - 1;
    SCsROW nRowUpper = mnFieldBtnRowCount - 1;

    nCol += nDX;
    if (nCol < 0)
        nCol = 0;
    else if (nColUpper < nCol)
        nCol = nColUpper;
    nRow += nDY;
    if (nRow < 0)
        nRow = 0;
    else if (nRowUpper < nRow)
        nRow = nRowUpper;

    nSel = nCol*mnFieldBtnRowCount + nRow;
    if (nSel >= nFldCount)
        nSel = nFldCount - 1;

    return nSel;
}

size_t ScDPHorFieldControl::GetDisplayPosition(size_t nIndex) const
{
    size_t nColFirst = maScroll.GetThumbPos();
    size_t nColLast = nColFirst + mnFieldBtnColCount - 1;
    size_t nCol = nIndex / mnFieldBtnRowCount;
    size_t nRow = nIndex % mnFieldBtnRowCount;
    if (nCol < nColFirst || nColLast < nCol)
        // index is outside the visible area.
        return INVALID_INDEX;

    size_t nPos = (nCol - nColFirst)*mnFieldBtnRowCount + nRow;
    return nPos;
}

String ScDPHorFieldControl::GetDescription() const
{
    return ScResId(STR_ACC_DATAPILOT_COL_DESCR);
}

void ScDPHorFieldControl::ScrollToEnd()
{
    maScroll.DoScroll(maScroll.GetRangeMax());
}

void ScDPHorFieldControl::ScrollToShowSelection()
{
    size_t nLower = maScroll.GetThumbPos();
    size_t nUpper = nLower + mnFieldBtnColCount - 1;
    size_t nCol = GetSelectedField() / mnFieldBtnRowCount;
    if (nCol < nLower)
    {
        // scroll to left.
        maScroll.DoScroll(nCol);
    }
    else if (nUpper < nCol)
    {
        // scroll to right.
        maScroll.DoScroll(nCol - mnFieldBtnColCount + 1);
    }
}

void ScDPHorFieldControl::ResetScrollBar()
{
    long nOldMax = maScroll.GetRangeMax();
    long nNewMax = ceil(
        static_cast<double>(GetFieldCount()) / static_cast<double>(mnFieldBtnRowCount));

    if (nOldMax != nNewMax)
        maScroll.SetRangeMax(nNewMax);
}

bool ScDPHorFieldControl::GetFieldBtnPosSize(size_t nPos, Point& rPos, Size& rSize)
{
    if (nPos >= mnFieldBtnColCount*mnFieldBtnRowCount)
        return false;

    Point aPos = Point(OUTER_MARGIN_HOR, OUTER_MARGIN_VER);
    size_t nRow = nPos % mnFieldBtnRowCount;
    size_t nCol = nPos / mnFieldBtnRowCount;

    aPos.X() += nCol*(FIELD_BTN_WIDTH + ROW_FIELD_BTN_GAP);
    aPos.Y() += nRow*(FIELD_BTN_HEIGHT + ROW_FIELD_BTN_GAP);

    rPos = aPos;
    rSize = Size(FIELD_BTN_WIDTH, FIELD_BTN_HEIGHT);
    return true;
}

void ScDPHorFieldControl::HandleScroll()
{
    Redraw();
}

IMPL_LINK(ScDPHorFieldControl, ScrollHdl, ScrollBar*, EMPTYARG)
{
    HandleScroll();
    return 0;
}

IMPL_LINK(ScDPHorFieldControl, EndScrollHdl, ScrollBar*, EMPTYARG)
{
    HandleScroll();
    return 0;
}

//=============================================================================

ScDPPageFieldControl::ScDPPageFieldControl(
    ScDPLayoutDlg* pDialog, const ResId& rResId, FixedText* pCaption) :
    ScDPHorFieldControl(pDialog, rResId, pCaption)
{
}

ScDPPageFieldControl::~ScDPPageFieldControl()
{
}

ScDPFieldType ScDPPageFieldControl::GetFieldType() const
{
    return TYPE_PAGE;
}

//=============================================================================

ScDPColFieldControl::ScDPColFieldControl(
    ScDPLayoutDlg* pDialog, const ResId& rResId, FixedText* pCaption) :
    ScDPHorFieldControl(pDialog, rResId, pCaption)
{
}

ScDPColFieldControl::~ScDPColFieldControl()
{
}

ScDPFieldType ScDPColFieldControl::GetFieldType() const
{
    return TYPE_COL;
}

//=============================================================================

ScDPRowFieldControl::ScDPRowFieldControl(
    ScDPLayoutDlg* pDialog, const ResId& rResId, FixedText* pCaption ) :
    ScDPFieldControlBase( pDialog, rResId, pCaption ),
    maScroll(this, WB_VERT | WB_DRAG),
    mnColumnBtnCount(0)
{
    maScroll.SetScrollHdl( LINK(this, ScDPRowFieldControl, ScrollHdl) );
    maScroll.SetEndScrollHdl( LINK(this, ScDPRowFieldControl, EndScrollHdl) );
    maScroll.Show();

    AppendPaintable(&maScroll);
}

ScDPRowFieldControl::~ScDPRowFieldControl()
{
}

//-------------------------------------------------------------------

Point ScDPRowFieldControl::GetFieldPosition(size_t nIndex)
{
    Point aPos;
    Size aSize;
    GetFieldBtnPosSize(nIndex, aPos, aSize);
    return aPos;
}

Size ScDPRowFieldControl::GetFieldSize() const
{
    return Size(FIELD_BTN_WIDTH, FIELD_BTN_HEIGHT);
}

bool ScDPRowFieldControl::GetFieldIndex( const Point& rPos, size_t& rnIndex )
{
    rnIndex = INVALID_INDEX;
    if (rPos.X() < 0 || rPos.Y() < 0)
        return false;

    rnIndex = rPos.Y() / FIELD_BTN_HEIGHT + maScroll.GetThumbPos();
    return IsValidIndex(rnIndex);
}

void ScDPRowFieldControl::Redraw()
{
    VirtualDevice   aVirDev;
    // #i97623# VirtualDevice is always LTR while other windows derive direction from parent
    aVirDev.EnableRTL( IsRTLEnabled() );
    aVirDev.SetMapMode( MAP_PIXEL );

    Point aPos0;
    Size aWndSize = GetSizePixel();
    Font aFont = GetFont();
    aFont.SetTransparent(true);
    aVirDev.SetFont(aFont);
    aVirDev.SetOutputSizePixel(aWndSize);

    DrawBackground(aVirDev);

    FieldNames& rFields = GetFieldNames();
    {
        long nScrollOffset = maScroll.GetThumbPos();
        FieldNames::iterator itr = rFields.begin(), itrEnd = rFields.end();
        if (nScrollOffset)
            ::std::advance(itr, nScrollOffset);

        for (size_t i = 0; itr != itrEnd; ++itr, ++i)
        {
            Point aFldPt;
            Size aFldSize;
            if (!GetFieldBtnPosSize(i, aFldPt, aFldSize))
                break;

            size_t nField = i + nScrollOffset;
            bool bFocus = HasFocus() && (nField == GetSelectedField());
            DrawField(aVirDev, Rectangle(aFldPt, aFldSize), *itr, bFocus);
        }
    }

    // Create a bitmap from the virtual device, and place that bitmap onto
    // this control.
    DrawBitmap(aPos0, aVirDev.GetBitmap(aPos0, aWndSize));

    DrawPaintables();
    DrawInvertSelection();
    UpdateStyle();
}

void ScDPRowFieldControl::CalcSize()
{
    Size aWndSize = GetSizePixel();

    long nTotal = aWndSize.Height() - OUTER_MARGIN_VER;
    mnColumnBtnCount = nTotal / (FIELD_BTN_HEIGHT + ROW_FIELD_BTN_GAP);

    long nScrollSize = GetSettings().GetStyleSettings().GetScrollBarSize();

    maScroll.SetSizePixel(Size(nScrollSize, aWndSize.Height() - OUTER_MARGIN_VER*2));
    maScroll.SetPosPixel(Point(aWndSize.Width() - nScrollSize - OUTER_MARGIN_HOR, OUTER_MARGIN_VER));
    maScroll.SetLineSize(1);
    maScroll.SetVisibleSize(mnColumnBtnCount);
    maScroll.SetPageSize(mnColumnBtnCount);
    maScroll.SetRange(Range(0, mnColumnBtnCount));
    maScroll.DoScroll(0);

}

bool ScDPRowFieldControl::IsValidIndex(size_t /*nIndex*/) const
{
    return true;
}

size_t ScDPRowFieldControl::CalcNewFieldIndex(SCsCOL /*nDX*/, SCsROW nDY) const
{
    size_t nNewField = GetSelectedField();
    nNewField += nDY;
    return IsExistingIndex(nNewField) ? nNewField : GetSelectedField();
}

size_t ScDPRowFieldControl::GetDisplayPosition(size_t nIndex) const
{
    size_t nLower = maScroll.GetThumbPos();
    size_t nUpper = nLower + mnColumnBtnCount;
    if (nLower <= nIndex && nIndex <= nUpper)
        return nIndex - nLower;

    return INVALID_INDEX;
}

//-------------------------------------------------------------------

String ScDPRowFieldControl::GetDescription() const
{
    return ScResId(STR_ACC_DATAPILOT_ROW_DESCR);
}

ScDPFieldType ScDPRowFieldControl::GetFieldType() const
{
    return TYPE_ROW;
}

void ScDPRowFieldControl::ScrollToEnd()
{
    maScroll.DoScroll(maScroll.GetRangeMax());
}

void ScDPRowFieldControl::ScrollToShowSelection()
{
    size_t nLower = maScroll.GetThumbPos();
    size_t nUpper = nLower + mnColumnBtnCount - 1;
    size_t nSel = GetSelectedField();
    if (nSel < nLower)
    {
        // scroll up
        maScroll.DoScroll(nSel);
    }
    else if (nUpper < nSel)
    {
        // scroll down
        size_t nD = nSel - nUpper;
        maScroll.DoScroll(nLower + nD);
    }
}

void ScDPRowFieldControl::ResetScrollBar()
{
    long nOldMax = maScroll.GetRangeMax();
    long nNewMax = std::max<long>(mnColumnBtnCount, GetFieldCount());

    if (nOldMax != nNewMax)
        maScroll.SetRangeMax(nNewMax);
}

bool ScDPRowFieldControl::GetFieldBtnPosSize(size_t nPos, Point& rPos, Size& rSize)
{
    if (nPos >= mnColumnBtnCount)
        return false;

    size_t nOffset = maScroll.GetThumbPos();
    if (nPos + nOffset >= GetFieldCount())
        return false;

    rSize = Size(FIELD_BTN_WIDTH, FIELD_BTN_HEIGHT);
    rPos = Point(OUTER_MARGIN_HOR, OUTER_MARGIN_VER);
    rPos.Y() += nPos * (FIELD_BTN_HEIGHT + ROW_FIELD_BTN_GAP);
    return true;
}

void ScDPRowFieldControl::HandleScroll()
{
    Redraw();
}

IMPL_LINK(ScDPRowFieldControl, ScrollHdl, ScrollBar*, EMPTYARG)
{
    HandleScroll();
    return 0;
}

IMPL_LINK(ScDPRowFieldControl, EndScrollHdl, ScrollBar*, EMPTYARG)
{
    HandleScroll();
    return 0;
}

//=============================================================================

#if NEW_SELECT_FIELD

ScDPSelectFieldControl::ScDPSelectFieldControl(
        ScDPLayoutDlg* pDialog, const ResId& rResId, FixedText* pCaption) :
    ScDPHorFieldControl(pDialog, rResId, pCaption)
{
    SetName(String(ScResId(STR_SELECT)));
}

ScDPSelectFieldControl::~ScDPSelectFieldControl()
{
}

ScDPFieldType ScDPSelectFieldControl::GetFieldType() const
{
    return TYPE_SELECT;
}

#else

ScDPSelectFieldControl::ScDPSelectFieldControl(
    ScDPLayoutDlg* pDialog, const ResId& rResId, const String& rName ) :
    ScDPFieldControlBase( pDialog, rResId, NULL )
{
    SetName(rName);
}

ScDPSelectFieldControl::~ScDPSelectFieldControl()
{
}

Point ScDPSelectFieldControl::GetFieldPosition( size_t nIndex )
{
    Point aPos;
    long nFldW = FIELD_BTN_WIDTH;
    long nFldH = FIELD_BTN_HEIGHT;
    long nSpace = SELECT_FIELD_BTN_SPACE;

    aPos.X() = (nFldW + nSpace) * (nIndex / LINE_SIZE);
    aPos.Y() = (nFldH + nSpace) * (nIndex % LINE_SIZE);
    return aPos;
}

Size ScDPSelectFieldControl::GetFieldSize() const
{
    return Size(FIELD_BTN_WIDTH, FIELD_BTN_HEIGHT);
}

bool ScDPSelectFieldControl::GetFieldIndex( const Point& rPos, size_t& rnIndex )
{
    rnIndex = INVALID_INDEX;
    long nFldW = FIELD_BTN_WIDTH;
    long nFldH = FIELD_BTN_HEIGHT;
    long nSpace = SELECT_FIELD_BTN_SPACE;
    if( (rPos.X() >= 0) && (rPos.Y() >= 0) )
    {
        size_t nRow = rPos.Y() / (nFldH + nSpace);
        size_t nCol = rPos.X() / (nFldW + nSpace);
        // is not between controls?
        if( (rPos.Y() % (nFldH + nSpace) < nFldH) && (rPos.X() % (nFldW + nSpace) < nFldW) )
            rnIndex = nCol * LINE_SIZE + nRow;
    }
    return IsValidIndex( rnIndex );
}

void ScDPSelectFieldControl::Redraw()
{
    const StyleSettings& rStyleSet = GetSettings().GetStyleSettings();
    Color aFaceColor = rStyleSet.GetFaceColor();
    Color aWinColor = rStyleSet.GetWindowColor();
    Color aTextColor = rStyleSet.GetButtonTextColor();
    Color aWinTextColor = rStyleSet.GetWindowTextColor();

    VirtualDevice   aVirDev;
    // #i97623# VirtualDevice is always LTR while other windows derive direction from parent
    aVirDev.EnableRTL( IsRTLEnabled() );
    aVirDev.SetMapMode( MAP_PIXEL );

    Point           aPos0;
    Size            aSize( GetSizePixel() );
    Font            aFont( GetFont() );         // Font vom Window
    aFont.SetTransparent( TRUE );
    aVirDev.SetFont( aFont );
    aVirDev.SetOutputSizePixel( aSize );

    aVirDev.SetLineColor();
    aVirDev.SetFillColor( aFaceColor );
    aVirDev.DrawRect( Rectangle(aPos0, aSize) );

    size_t nFieldSelected = GetSelectedField();
    FieldNames& rFields = GetFieldNames();

    Rectangle aFieldRect( aPos0, GetFieldSize() );
    for( size_t nIx = 0; nIx < rFields.size(); ++nIx )
    {
        aFieldRect.SetPos( GetFieldPosition( nIx ) );
        bool bFocus = HasFocus() && (nIx == nFieldSelected);
        DrawField( aVirDev, aFieldRect, rFields[ nIx ], bFocus );
    }

    DrawBitmap( aPos0, aVirDev.GetBitmap( aPos0, aSize ) );
    DrawInvertSelection();
    UpdateStyle();
}

//-------------------------------------------------------------------

bool ScDPSelectFieldControl::IsValidIndex( size_t nIndex ) const
{
    return nIndex < PAGE_SIZE;
}

size_t ScDPSelectFieldControl::CalcNewFieldIndex( SCsCOL nDX, SCsROW nDY ) const
{
    size_t nNewField = GetSelectedField();
    nNewField += static_cast<SCsCOLROW>(nDX) * LINE_SIZE + nDY;
    return IsExistingIndex( nNewField ) ? nNewField : GetSelectedField();
}

//-------------------------------------------------------------------

String ScDPSelectFieldControl::GetDescription() const
{
    return ScResId(STR_ACC_DATAPILOT_SEL_DESCR);
}

ScDPFieldType ScDPSelectFieldControl::GetFieldType() const
{
    return TYPE_SELECT;
}

#endif

//=============================================================================

ScDPDataFieldControl::ScDPDataFieldControl( ScDPLayoutDlg* pParent, const ResId& rResId, FixedText* pCaption ) :
    ScDPFieldControlBase(pParent, rResId, pCaption),
    maScroll(this, WB_HORZ | WB_DRAG),
    mpParent(pParent),
    mnScrollMarginHeight(0),
    mnColumnBtnCount(0),
    mnTotalBtnCount(0)
{
    maScroll.SetLineSize(1);
    maScroll.SetVisibleSize(2);
    maScroll.SetPageSize(2);
    maScroll.SetRange(Range(0, 2));
    maScroll.DoScroll(0);
    maScroll.SetScrollHdl( LINK(this, ScDPDataFieldControl, ScrollHdl) );
    maScroll.SetEndScrollHdl( LINK(this, ScDPDataFieldControl, EndScrollHdl) );
    maScroll.Show();

    AppendPaintable(&maScroll);
}

ScDPDataFieldControl::~ScDPDataFieldControl()
{
}

void ScDPDataFieldControl::CalcSize()
{
    long nScrollSize = GetSettings().GetStyleSettings().GetScrollBarSize();
    Size aWndSize = GetSizePixel();
    mnScrollMarginHeight = nScrollSize + OUTER_MARGIN_VER;

    maScroll.SetSizePixel(Size(aWndSize.Width() - OUTER_MARGIN_HOR*2, nScrollSize));
    maScroll.SetPosPixel(Point(OUTER_MARGIN_HOR, aWndSize.Height() - mnScrollMarginHeight));

    long nH = FIELD_BTN_HEIGHT + DATA_FIELD_BTN_GAP;
    long nTotalH = aWndSize.Height() - mnScrollMarginHeight - OUTER_MARGIN_VER;
    mnColumnBtnCount = nTotalH / nH;
    mnTotalBtnCount = mnColumnBtnCount * 2;
}

bool ScDPDataFieldControl::IsValidIndex(size_t /*nIndex*/) const
{
    // We support unlimited number of data fields.  If we ever want to put a
    // cap on the number of data fields, this is the right place.
    return true;
}

Point ScDPDataFieldControl::GetFieldPosition(size_t nIndex)
{
    Point aPos;
    Size aSize;
    GetFieldBtnPosSize(nIndex, aPos, aSize);
    return aPos;
}

bool ScDPDataFieldControl::GetFieldIndex(const Point& rPos, size_t& rnIndex)
{
    size_t nCol, nRow;
    GetFieldBtnColRow(rPos, nCol, nRow);
    nCol += static_cast<size_t>(maScroll.GetThumbPos());
    size_t nIndex = nCol * mnColumnBtnCount;

    nIndex += nRow;
    size_t nFldCount = GetFieldCount();
    if (nIndex >= nFldCount)
        nIndex = nFldCount;

    rnIndex = nIndex;
    return true;
}

Size ScDPDataFieldControl::GetFieldSize() const
{
    Size aWndSize = GetSizePixel();
    long nFieldObjWidth = aWndSize.Width() / 2.0 - OUTER_MARGIN_HOR - DATA_FIELD_BTN_GAP/2;
    Size aFieldSize(nFieldObjWidth, FIELD_BTN_HEIGHT);
    return aFieldSize;
}

String ScDPDataFieldControl::GetDescription() const
{
    return ScResId(STR_ACC_DATAPILOT_DATA_DESCR);
}

ScDPFieldType ScDPDataFieldControl::GetFieldType() const
{
    return TYPE_DATA;
}

void ScDPDataFieldControl::ScrollToEnd()
{
    maScroll.DoScroll(maScroll.GetRangeMax());
}

void ScDPDataFieldControl::ScrollToShowSelection()
{
    long nNewOffset = CalcOffsetToShowSelection();
    if (nNewOffset != maScroll.GetThumbPos())
        maScroll.DoScroll(nNewOffset);
}

void ScDPDataFieldControl::ResetScrollBar()
{
    long nOldMax = maScroll.GetRangeMax();
    long nNewMax = ceil(
        static_cast<double>(GetFieldCount()) / static_cast<double>(mnColumnBtnCount));

    if (nOldMax != nNewMax)
    {
        maScroll.SetRangeMax(nNewMax);
        if (nNewMax < nOldMax)
        {
            // We lost a column.  Scroll to the right frame.
            long nNewOffset = CalcOffsetToShowSelection();
            nNewOffset -= 2;
            if (nNewOffset < 0)
                nNewOffset = 0;
            maScroll.DoScroll(nNewOffset);
        }
    }
}

void ScDPDataFieldControl::Paint(const Rectangle& /*rRect*/)
{
    Redraw();
}

void ScDPDataFieldControl::DataChanged( const DataChangedEvent& rDCEvt )
{
    ScDPFieldControlBase::DataChanged(rDCEvt);
}

void ScDPDataFieldControl::MouseButtonDown(const MouseEvent& rMEvt)
{
    if (maScroll.IsVisible())
    {
        Rectangle aRect(maScroll.GetPosPixel(), maScroll.GetSizePixel());
        if (aRect.IsInside(rMEvt.GetPosPixel()))
        {
            maScroll.MouseButtonDown(rMEvt);
            return;
        }
    }
    ScDPFieldControlBase::MouseButtonDown(rMEvt);
}

void ScDPDataFieldControl::MouseButtonUp(const MouseEvent& rMEvt)
{
    if (maScroll.IsVisible())
    {
        Rectangle aRect(maScroll.GetPosPixel(), maScroll.GetSizePixel());
        if (aRect.IsInside(rMEvt.GetPosPixel()))
        {
            maScroll.MouseButtonUp(rMEvt);
            return;
        }
    }
    ScDPFieldControlBase::MouseButtonUp(rMEvt);
}

void ScDPDataFieldControl::MouseMove(const MouseEvent& rMEvt)
{
    if (maScroll.IsVisible())
    {
        Rectangle aRect(maScroll.GetPosPixel(), maScroll.GetSizePixel());
        if (aRect.IsInside(rMEvt.GetPosPixel()))
        {
            maScroll.MouseMove(rMEvt);
            return;
        }
    }
    ScDPFieldControlBase::MouseMove(rMEvt);
}

bool ScDPDataFieldControl::GetFieldBtnPosSize(size_t nPos, Point& rPos, Size& rSize)
{
    if (nPos >= mnTotalBtnCount)
        return false;

    Size aFieldSize = GetFieldSize();

    size_t nCol = nPos / mnColumnBtnCount;
    size_t nRow = nPos % mnColumnBtnCount;

    long nX = OUTER_MARGIN_HOR + aFieldSize.Width() * nCol;
    if (nCol > 0)
        nX += DATA_FIELD_BTN_GAP * nCol;

    long nY = OUTER_MARGIN_VER + aFieldSize.Height() * nRow;
    if (nRow > 0)
        nY += DATA_FIELD_BTN_GAP * nRow;

    rPos = Point(nX, nY);
    rSize = aFieldSize;
    return true;
}

void ScDPDataFieldControl::GetFieldBtnColRow(const Point& rPos, size_t& rCol, size_t& rRow)
{
    Size aSize = GetSizePixel();
    long nBtnH = FIELD_BTN_HEIGHT;

    size_t nCol = (rPos.X() <= aSize.Width()/2.0) ? 0 : 1;
    size_t nRow = 0;

    long nVBound = OUTER_MARGIN_VER + nBtnH + DATA_FIELD_BTN_GAP;
    while (true)
    {
        if (rPos.Y() <= nVBound)
            break;

        nVBound += nBtnH + DATA_FIELD_BTN_GAP;
        if (nVBound > aSize.Height())
            break;

        ++nRow;
    }

    rCol = nCol;
    rRow = nRow;
}

long ScDPDataFieldControl::CalcOffsetToShowSelection()
{
    long nOffset = maScroll.GetThumbPos();
    size_t nSel = GetSelectedField();
    size_t nLower = nOffset*mnColumnBtnCount;
    size_t nUpper = nLower + mnColumnBtnCount * 2 - 1;
    long nNewOffset = nOffset;
    if (nSel < nLower)
    {
        // scroll to left.  The selected field should be in the left column.
        nNewOffset = floor(
            static_cast<double>(nSel) / static_cast<double>(mnColumnBtnCount));
    }
    else if (nUpper < nSel)
    {
        // scroll to right.  The selected field should be in the right column.
        nNewOffset = floor(
            static_cast<double>(nSel) / static_cast<double>(mnColumnBtnCount))-1;
    }
    return nNewOffset;
}

void ScDPDataFieldControl::HandleScroll()
{
    Redraw();
}

void ScDPDataFieldControl::Redraw()
{
    VirtualDevice   aVirDev;
    // #i97623# VirtualDevice is always LTR while other windows derive direction from parent
    aVirDev.EnableRTL( IsRTLEnabled() );
    aVirDev.SetMapMode( MAP_PIXEL );

    Point aPos0;
    Size aWndSize = GetSizePixel();
    Font aFont = GetFont();
    aFont.SetTransparent(true);
    aVirDev.SetFont(aFont);
    aVirDev.SetOutputSizePixel(aWndSize);

    DrawBackground(aVirDev);

    long nOffset = maScroll.GetThumbPos();
    FieldNames& rFields = GetFieldNames();
    {
        FieldNames::iterator itr = rFields.begin(), itrEnd = rFields.end();
        if (nOffset)
            ::std::advance(itr, nOffset*mnColumnBtnCount);

        for (size_t i = 0; itr != itrEnd; ++itr, ++i)
        {
            Point aFldPt;
            Size aFldSize;
            if (!GetFieldBtnPosSize(i, aFldPt, aFldSize))
                break;

            size_t nField = i + nOffset*mnColumnBtnCount;
            bool bFocus = HasFocus() && (nField == GetSelectedField());
            DrawField(aVirDev, Rectangle(aFldPt, aFldSize), *itr, bFocus);
        }
    }

    // Create a bitmap from the virtual device, and place that bitmap onto
    // this control.
    DrawBitmap(aPos0, aVirDev.GetBitmap(aPos0, aWndSize));

    DrawPaintables();
    DrawInvertSelection();
    UpdateStyle();
}

size_t ScDPDataFieldControl::CalcNewFieldIndex(SCsCOL nDX, SCsROW nDY) const
{
    size_t nField = GetSelectedField();
    if (nField < mnColumnBtnCount)
    {
        // selected button in the first column.
        if (nDX < 0)
            // in the left most column.  We can't move any more to the left.
            return nField;
        if (nField == 0 && nDY < 0)
            // at the top left most position.  We can't move up any more.
            return nField;
    }

    size_t nFldCount = GetFieldCount();
    size_t nFirstInLastCol = nFldCount / mnColumnBtnCount * mnColumnBtnCount;
    if (nFirstInLastCol <= nField)
    {
        // selected button in the last column.
        if (nDX > 0)
            // can't move to the right any more.
            return nField;
        if (nDY > 0 && nField >= nFldCount)
            // at the last position.  Can't move down any more.
            return nField;
    }

    nDY += nDX * mnColumnBtnCount;
    nField += nDY;

    if (nField >= nFldCount)
        // Don't exceed the upper bound.
        nField = nFldCount - 1;
    return nField;
}

size_t ScDPDataFieldControl::GetDisplayPosition(size_t nIndex) const
{
    long nOffset = maScroll.GetThumbPos();
    size_t nLower = mnColumnBtnCount * nOffset;
    size_t nUpper = nLower + mnColumnBtnCount*2 - 1;
    if (nLower <= nIndex && nIndex <= nUpper)
        return nIndex - nLower;

    return INVALID_INDEX;
}

IMPL_LINK(ScDPDataFieldControl, ScrollHdl, ScrollBar*, EMPTYARG)
{
    HandleScroll();
    return 0;
}

IMPL_LINK(ScDPDataFieldControl, EndScrollHdl, ScrollBar*, EMPTYARG)
{
    HandleScroll();
    return 0;
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
