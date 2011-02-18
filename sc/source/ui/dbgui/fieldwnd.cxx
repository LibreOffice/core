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
#include "pvglob.hxx"
#include "AccessibleDataPilotControl.hxx"
#include "scresid.hxx"
#include "sc.hrc"

const size_t INVALID_INDEX = static_cast< size_t >( -1 );

//===================================================================

ScDPFieldWindow::ScDPFieldWindow(
        ScDPLayoutDlg* pDialog,
        const ResId& rResId,
        ScDPFieldType eFieldType,
        FixedText* pFtFieldCaption ) :
    Control( pDialog, rResId ),
    pDlg( pDialog ),
    pFtCaption( pFtFieldCaption ),
    eType( eFieldType ),
    nFieldSelected( 0 ),
    pAccessible( NULL )
{
    Init();
    if (eType != TYPE_SELECT && pFtCaption)
        aName = MnemonicGenerator::EraseAllMnemonicChars( pFtCaption->GetText() );
}

ScDPFieldWindow::ScDPFieldWindow(
        ScDPLayoutDlg* pDialog,
        const ResId& rResId,
        ScDPFieldType eFieldType,
        const String& rName ) :
    Control( pDialog, rResId ),
    aName(rName),
    pDlg( pDialog ),
    pFtCaption( NULL ),
    eType( eFieldType ),
    nFieldSelected( 0 ),
    pAccessible( NULL )
{
    Init();
}

void ScDPFieldWindow::Init()
{
    aWndRect = Rectangle( GetPosPixel(), GetSizePixel() );
    nFieldSize = (eType == TYPE_SELECT) ? PAGE_SIZE : ((eType == TYPE_PAGE) ? MAX_PAGEFIELDS : MAX_FIELDS);

    if( pFtCaption )
    {
        Size aWinSize( aWndRect.GetSize() );
        Size aTextSize( GetTextWidth( pFtCaption->GetText() ), GetTextHeight() );
        aTextPos.X() = (aWinSize.Width() - aTextSize.Width()) / 2;
        aTextPos.Y() = (aWinSize.Height() - aTextSize.Height()) / 2;
    }

    GetStyleSettings();
}

__EXPORT ScDPFieldWindow::~ScDPFieldWindow()
{
    if (pAccessible)
    {
        com::sun::star::uno::Reference < com::sun::star::accessibility::XAccessible > xTempAcc = xAccessible;
        if (xTempAcc.is())
            pAccessible->dispose();
    }
}

//-------------------------------------------------------------------

void ScDPFieldWindow::GetStyleSettings()
{
    const StyleSettings& rStyleSet = GetSettings().GetStyleSettings();
    aFaceColor = rStyleSet.GetFaceColor();
    aWinColor = rStyleSet.GetWindowColor();
    aTextColor = rStyleSet.GetButtonTextColor();
    aWinTextColor = rStyleSet.GetWindowTextColor();
}

//-------------------------------------------------------------------

Point ScDPFieldWindow::GetFieldPosition( size_t nIndex ) const
{
    Point aPos;
    switch( eType )
    {
        case TYPE_PAGE:
            aPos.X() = OWIDTH * (nIndex % (MAX_PAGEFIELDS / 2));
            aPos.Y() = OHEIGHT * (nIndex / (MAX_PAGEFIELDS / 2));
        break;
        case TYPE_COL:
            aPos.X() = OWIDTH * (nIndex % (MAX_FIELDS / 2));
            aPos.Y() = OHEIGHT * (nIndex / (MAX_FIELDS / 2));
        break;
        case TYPE_ROW:
        case TYPE_DATA:
            aPos.X() = 0;
            aPos.Y() = OHEIGHT * nIndex;
        break;
        case TYPE_SELECT:
            aPos.X() = (OWIDTH + SSPACE) * (nIndex / LINE_SIZE);
            aPos.Y() = (OHEIGHT + SSPACE) * (nIndex % LINE_SIZE);
        break;
    }
    return aPos;
}

Size ScDPFieldWindow::GetFieldSize() const
{
    return Size( (eType == TYPE_DATA) ? GetSizePixel().Width() : OWIDTH, OHEIGHT );
}

Point ScDPFieldWindow::GetLastPosition() const
{
    return OutputToScreenPixel( GetFieldPosition( nFieldSize - 1 ) );
}

bool ScDPFieldWindow::GetFieldIndex( const Point& rPos, size_t& rnIndex ) const
{
    rnIndex = INVALID_INDEX;
    if( (rPos.X() >= 0) && (rPos.Y() >= 0) )
    {
        switch( eType )
        {
            case TYPE_ROW:
            case TYPE_DATA:
                rnIndex = rPos.Y() / OHEIGHT;
            break;
            case TYPE_PAGE:
            {
                size_t nRow = rPos.Y() / OHEIGHT;
                size_t nCol = rPos.X() / OWIDTH;
                rnIndex = nRow * MAX_PAGEFIELDS / 2 + nCol;
            }
            break;
            case TYPE_COL:
            {
                size_t nRow = rPos.Y() / OHEIGHT;
                size_t nCol = rPos.X() / OWIDTH;
                rnIndex = nRow * MAX_FIELDS / 2 + nCol;
            }
            break;
            case TYPE_SELECT:
            {
                size_t nRow = rPos.Y() / (OHEIGHT + SSPACE);
                size_t nCol = rPos.X() / (OWIDTH + SSPACE);
                // is not between controls?
                if( (rPos.Y() % (OHEIGHT + SSPACE) < OHEIGHT) && (rPos.X() % (OWIDTH + SSPACE) < OWIDTH) )
                    rnIndex = nCol * LINE_SIZE + nRow;
            }
            break;
        }
    }
    return IsValidIndex( rnIndex );
}

//-------------------------------------------------------------------

void ScDPFieldWindow::DrawBackground( OutputDevice& rDev )
{
    Point aPos0;
    Size aSize( GetSizePixel() );

    if ( eType == TYPE_SELECT )
    {
        rDev.SetLineColor();
        rDev.SetFillColor( aFaceColor );
        rDev.DrawRect( Rectangle( aPos0, aSize ) );
    }
    else
    {
        rDev.SetLineColor( aWinTextColor );
        rDev.SetFillColor( aWinColor );
        rDev.DrawRect( Rectangle( aPos0, aSize ) );

        rDev.SetTextColor( aWinTextColor );

        /*  Draw the caption text. This needs some special handling, because we
            support hard line breaks here. This part will draw each line of the
            text for itself. */

        xub_StrLen nTokenCnt = GetText().GetTokenCount( '\n' );
        long nY = (aSize.Height() - nTokenCnt * rDev.GetTextHeight()) / 2;
        for( xub_StrLen nToken = 0, nStringIx = 0; nToken < nTokenCnt; ++nToken )
        {
            String aLine( GetText().GetToken( 0, '\n', nStringIx ) );
            Point aLinePos( (aSize.Width() - rDev.GetCtrlTextWidth( aLine )) / 2, nY );
            rDev.DrawCtrlText( aLinePos, aLine );
            nY += rDev.GetTextHeight();
        }
    }
}

void ScDPFieldWindow::DrawField(
        OutputDevice& rDev, const Rectangle& rRect, FieldString& rText, bool bFocus )
{
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

void ScDPFieldWindow::Redraw()
{
    VirtualDevice   aVirDev;
    // #i97623# VirtualDevice is always LTR while other windows derive direction from parent
    aVirDev.EnableRTL( IsRTLEnabled() );
    aVirDev.SetMapMode( MAP_PIXEL );

    Point           aPos0;
    Size            aSize( GetSizePixel() );
    Font            aFont( GetFont() );         // Font vom Window
    aFont.SetTransparent( sal_True );
    aVirDev.SetFont( aFont );
    aVirDev.SetOutputSizePixel( aSize );

    DrawBackground( aVirDev );

    if( !aFieldArr.empty() && (nFieldSelected >= aFieldArr.size()) )
        nFieldSelected = aFieldArr.size() - 1;
    Rectangle aFieldRect( aPos0, GetFieldSize() );
    for( size_t nIx = 0; nIx < aFieldArr.size(); ++nIx )
    {
        aFieldRect.SetPos( GetFieldPosition( nIx ) );
        bool bFocus = HasFocus() && (nIx == nFieldSelected);
        DrawField( aVirDev, aFieldRect, aFieldArr[ nIx ], bFocus );
    }
    DrawBitmap( aPos0, aVirDev.GetBitmap( aPos0, aSize ) );

    if( HasFocus() && (nFieldSelected < aFieldArr.size()) )
    {
        long nFieldWidth = aFieldRect.GetWidth();
        long nSelectionWidth = Min( GetTextWidth( aFieldArr[ nFieldSelected ].first ) + 4, nFieldWidth - 6 );
        Rectangle aSelection(
            GetFieldPosition( nFieldSelected ) + Point( (nFieldWidth - nSelectionWidth) / 2, 3 ),
            Size( nSelectionWidth, aFieldRect.GetHeight() - 6 ) );
        InvertTracking( aSelection, SHOWTRACK_SMALL | SHOWTRACK_WINDOW );
    }

    UpdateStyle();
}

void ScDPFieldWindow::UpdateStyle()
{
    WinBits nMask = ~(WB_TABSTOP | WB_NOTABSTOP);
    SetStyle( (GetStyle() & nMask) | (IsEmpty() ? WB_NOTABSTOP : WB_TABSTOP) );
}

//-------------------------------------------------------------------

bool ScDPFieldWindow::IsValidIndex( size_t nIndex ) const
{
    return nIndex < nFieldSize;
}

bool ScDPFieldWindow::IsExistingIndex( size_t nIndex ) const
{
    return nIndex < aFieldArr.size();
}

bool ScDPFieldWindow::IsShortenedText( size_t nIndex ) const
{
    return (nIndex < aFieldArr.size()) && !aFieldArr[ nIndex ].second;
}

size_t ScDPFieldWindow::CalcNewFieldIndex( SCsCOL nDX, SCsROW nDY ) const
{
    size_t nNewField = nFieldSelected;
    switch( eType )
    {
        case TYPE_PAGE:
            nNewField += static_cast<SCsCOLROW>(nDX) + nDY * MAX_PAGEFIELDS / 2;
        break;
        case TYPE_COL:
            nNewField += static_cast<SCsCOLROW>(nDX) + nDY * MAX_FIELDS / 2;
        break;
        case TYPE_ROW:
        case TYPE_DATA:
            nNewField += nDY;
        break;
        case TYPE_SELECT:
            nNewField += static_cast<SCsCOLROW>(nDX) * LINE_SIZE + nDY;
        break;
    }

    return IsExistingIndex( nNewField ) ? nNewField : nFieldSelected;
}

void ScDPFieldWindow::SetSelection( size_t nIndex )
{
    if( !aFieldArr.empty() )
    {
        if( nFieldSelected >= aFieldArr.size() )
            nFieldSelected = aFieldArr.size() - 1;
        if( nFieldSelected != nIndex )
        {
            sal_Int32 nOldSelected(nFieldSelected);
            nFieldSelected = nIndex;
            Redraw();

            if (pAccessible && HasFocus())
            {
                com::sun::star::uno::Reference < com::sun::star::accessibility::XAccessible > xTempAcc = xAccessible;
                if (xTempAcc.is())
                    pAccessible->FieldFocusChange(nOldSelected, nFieldSelected);
                else
                    pAccessible = NULL;
            }
        }
    }
}

void ScDPFieldWindow::SetSelectionHome()
{
    if( !aFieldArr.empty() )
    {
        if( eType == TYPE_SELECT )
            pDlg->NotifyMoveSlider( KEY_HOME );
        SetSelection( 0 );
    }
}

void ScDPFieldWindow::SetSelectionEnd()
{
    if( !aFieldArr.empty() )
    {
        if( eType == TYPE_SELECT )
            pDlg->NotifyMoveSlider( KEY_END );
        SetSelection( aFieldArr.size() - 1 );
    }
}

void ScDPFieldWindow::MoveSelection( sal_uInt16 nKeyCode, SCsCOL nDX, SCsROW nDY )
{
    size_t nNewIndex = CalcNewFieldIndex( nDX, nDY );
    if( (eType == TYPE_SELECT) && (nNewIndex == nFieldSelected) )
    {
        if( pDlg->NotifyMoveSlider( nKeyCode ) )
        {
            switch( nKeyCode )
            {
                case KEY_UP:    nNewIndex += (LINE_SIZE - 1);   break;
                case KEY_DOWN:  nNewIndex -= (LINE_SIZE - 1);   break;
            }
        }
    }
    SetSelection( nNewIndex );
}

void ScDPFieldWindow::ModifySelectionOffset( long nOffsetDiff )
{
    nFieldSelected -= nOffsetDiff;
    Redraw();
}

void ScDPFieldWindow::SelectNext()
{
    if( eType == TYPE_SELECT )
        MoveSelection( KEY_DOWN, 0, 1 );
}

void ScDPFieldWindow::GrabFocusWithSel( size_t nIndex )
{
    SetSelection( nIndex );
    if( !HasFocus() )
        GrabFocus();
}

void ScDPFieldWindow::MoveField( size_t nDestIndex )
{
    if( nDestIndex != nFieldSelected )
    {
        // "recycle" existing functionality
        pDlg->NotifyMouseButtonDown( eType, nFieldSelected );
        pDlg->NotifyMouseButtonUp( OutputToScreenPixel( GetFieldPosition( nDestIndex ) ) );
    }
}

void ScDPFieldWindow::MoveFieldRel( SCsCOL nDX, SCsROW nDY )
{
    MoveField( CalcNewFieldIndex( nDX, nDY ) );
}

//-------------------------------------------------------------------

void __EXPORT ScDPFieldWindow::Paint( const Rectangle& /* rRect */ )
{
    // #124828# hiding the caption is now done from StateChanged
    Redraw();
}

void ScDPFieldWindow::UseMnemonic()
{
    // Now the FixedText has its mnemonic char. Grab the text and hide the
    // FixedText to be able to handle tabstop and mnemonics separately.
    if( pFtCaption )
    {
        SetText( pFtCaption->GetText() );
        pFtCaption->Hide();
    }

    // after reading the mnemonics, tab stop style bits can be updated
    UpdateStyle();
}

void __EXPORT ScDPFieldWindow::DataChanged( const DataChangedEvent& rDCEvt )
{
    if( (rDCEvt.GetType() == DATACHANGED_SETTINGS) && (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        GetStyleSettings();
        Redraw();
    }
    Control::DataChanged( rDCEvt );
}

void __EXPORT ScDPFieldWindow::MouseButtonDown( const MouseEvent& rMEvt )
{
    if( rMEvt.IsLeft() )
    {
        size_t nIndex = 0;
        if( GetFieldIndex( rMEvt.GetPosPixel(), nIndex ) && IsExistingIndex( nIndex ) )
        {
            GrabFocusWithSel( nIndex );

            if( rMEvt.GetClicks() == 1 )
            {
                PointerStyle ePtr = pDlg->NotifyMouseButtonDown( eType, nIndex );
                CaptureMouse();
                SetPointer( Pointer( ePtr ) );
            }
            else
                pDlg->NotifyDoubleClick( eType, nIndex );
        }
    }
}

void __EXPORT ScDPFieldWindow::MouseButtonUp( const MouseEvent& rMEvt )
{
    if( rMEvt.IsLeft() )
    {
        if( rMEvt.GetClicks() == 1 )
        {
            pDlg->NotifyMouseButtonUp( OutputToScreenPixel( rMEvt.GetPosPixel() ) );
            SetPointer( Pointer( POINTER_ARROW ) );
        }

        if( IsMouseCaptured() )
            ReleaseMouse();
    }
}

void __EXPORT ScDPFieldWindow::MouseMove( const MouseEvent& rMEvt )
{
    if( IsMouseCaptured() )
    {
        PointerStyle ePtr = pDlg->NotifyMouseMove( OutputToScreenPixel( rMEvt.GetPosPixel() ) );
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

void __EXPORT ScDPFieldWindow::KeyInput( const KeyEvent& rKEvt )
{
    const KeyCode& rKeyCode = rKEvt.GetKeyCode();
    sal_uInt16 nCode = rKeyCode.GetCode();
    sal_Bool bKeyEvaluated = sal_False;

    if( rKeyCode.IsMod1() && (eType != TYPE_SELECT) )
    {
        bKeyEvaluated = sal_True;
        switch( nCode )
        {
            case KEY_UP:    MoveFieldRel( 0, -1 );              break;
            case KEY_DOWN:  MoveFieldRel( 0, 1 );               break;
            case KEY_LEFT:  MoveFieldRel( -1, 0 );              break;
            case KEY_RIGHT: MoveFieldRel( 1, 0 );               break;
            case KEY_HOME:  MoveField( 0 );                     break;
            case KEY_END:   MoveField( aFieldArr.size() - 1 );  break;
            default:        bKeyEvaluated = sal_False;
        }
    }
    else
    {
        bKeyEvaluated = sal_True;
        switch( nCode )
        {
            case KEY_UP:    MoveSelection( nCode, 0, -1 );          break;
            case KEY_DOWN:  MoveSelection( nCode, 0, 1 );           break;
            case KEY_LEFT:  MoveSelection( nCode, -1, 0 );          break;
            case KEY_RIGHT: MoveSelection( nCode, 1, 0 );           break;
            case KEY_HOME:  SetSelectionHome();                     break;
            case KEY_END:   SetSelectionEnd();                      break;
            case KEY_DELETE:
                pDlg->NotifyRemoveField( eType, nFieldSelected );   break;
            default:        bKeyEvaluated = sal_False;
        }
    }

    if( !bKeyEvaluated )
        Control::KeyInput( rKEvt );
}

void __EXPORT ScDPFieldWindow::GetFocus()
{
    Control::GetFocus();
    Redraw();
    if( GetGetFocusFlags() & GETFOCUS_MNEMONIC )    // move field on shortcut
        pDlg->NotifyMoveField( eType );
    else                                            // else change focus
        pDlg->NotifyFieldFocus( eType, sal_True );

    if (pAccessible)
    {
        com::sun::star::uno::Reference < com::sun::star::accessibility::XAccessible > xTempAcc = xAccessible;
        if (xTempAcc.is())
            pAccessible->GotFocus();
        else
            pAccessible = NULL;
    }
}

void __EXPORT ScDPFieldWindow::LoseFocus()
{
    Control::LoseFocus();
    Redraw();
    pDlg->NotifyFieldFocus( eType, sal_False );

    if (pAccessible)
    {
        com::sun::star::uno::Reference < com::sun::star::accessibility::XAccessible > xTempAcc = xAccessible;
        if (xTempAcc.is())
            pAccessible->LostFocus();
        else
            pAccessible = NULL;
    }
}

//-------------------------------------------------------------------

void ScDPFieldWindow::AddField( const String& rText, size_t nNewIndex )
{
    DBG_ASSERT( nNewIndex == aFieldArr.size(), "ScDPFieldWindow::AddField - invalid index" );
    if( IsValidIndex( nNewIndex ) )
    {
        aFieldArr.push_back( FieldString( rText, true ) );
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

void ScDPFieldWindow::DelField( size_t nDelIndex )
{
    if( IsExistingIndex( nDelIndex  ) )
    {
        if (pAccessible) // before decrement fieldcount
        {
            com::sun::star::uno::Reference < com::sun::star::accessibility::XAccessible > xTempAcc = xAccessible;
            if (xTempAcc.is())
                pAccessible->RemoveField(nDelIndex);
            else
                pAccessible = NULL;
        }
        aFieldArr.erase( aFieldArr.begin() + nDelIndex );
        Redraw();
    }
}

void ScDPFieldWindow::ClearFields()
{
    if( eType == TYPE_SELECT || eType == TYPE_PAGE || eType == TYPE_COL || eType == TYPE_ROW || eType == TYPE_DATA)
    {
        com::sun::star::uno::Reference < com::sun::star::accessibility::XAccessible > xTempAcc = xAccessible;
        if (!xTempAcc.is() && pAccessible)
            pAccessible = NULL;
        if (pAccessible)
            for( size_t nIdx = aFieldArr.size(); nIdx > 0; --nIdx )
                pAccessible->RemoveField( nIdx - 1 );

        aFieldArr.clear();
    }
}

void ScDPFieldWindow::SetFieldText( const String& rText, size_t nIndex )
{
    if( IsExistingIndex( nIndex ) )
    {
        aFieldArr[ nIndex ] = FieldString( rText, true );
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

const String& ScDPFieldWindow::GetFieldText( size_t nIndex ) const
{
    if( IsExistingIndex( nIndex ) )
        return aFieldArr[ nIndex ].first;
    return EMPTY_STRING;
}

//-------------------------------------------------------------------

bool ScDPFieldWindow::AddField( const String& rText, const Point& rPos, size_t& rnIndex )
{
    if ( aFieldArr.size() == nFieldSize )
        return sal_False;

    size_t nNewIndex = 0;
    if( GetFieldIndex( rPos, nNewIndex ) )
    {
        if( nNewIndex > aFieldArr.size() )
            nNewIndex = aFieldArr.size();

        aFieldArr.insert( aFieldArr.begin() + nNewIndex, FieldString( rText, true ) );
        nFieldSelected = nNewIndex;
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

void ScDPFieldWindow::GetExistingIndex( const Point& rPos, size_t& rnIndex )
{
    if( !aFieldArr.empty() && (eType != TYPE_SELECT) && GetFieldIndex( rPos, rnIndex ) )
    {
        if( rnIndex >= aFieldArr.size() )
            rnIndex = aFieldArr.size() - 1;
    }
    else
        rnIndex = 0;
}

String ScDPFieldWindow::GetDescription() const
{
    String sDescription;
    switch( eType )
    {
        case TYPE_COL:
            sDescription = ScResId(STR_ACC_DATAPILOT_COL_DESCR);
        break;
        case TYPE_ROW:
            sDescription = ScResId(STR_ACC_DATAPILOT_ROW_DESCR);
        break;
        case TYPE_DATA:
            sDescription = ScResId(STR_ACC_DATAPILOT_DATA_DESCR);
        break;
        case TYPE_SELECT:
            sDescription = ScResId(STR_ACC_DATAPILOT_SEL_DESCR);
        break;
        default:
        {
            // added to avoid warnings
        }
    }
    return sDescription;
}

::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > ScDPFieldWindow::CreateAccessible()
{
    pAccessible =
        new ScAccessibleDataPilotControl(GetAccessibleParentWindow()->GetAccessible(), this);

    com::sun::star::uno::Reference < ::com::sun::star::accessibility::XAccessible > xReturn = pAccessible;

    pAccessible->Init();
    xAccessible = xReturn;

    return xReturn;
}

//===================================================================

