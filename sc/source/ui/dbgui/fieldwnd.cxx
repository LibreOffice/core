/*************************************************************************
 *
 *  $RCSfile: fieldwnd.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-26 18:05:53 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#include <vcl/virdev.hxx>
#include <vcl/decoview.hxx>
#include <vcl/svapp.hxx>

#include <tools/debug.hxx>

#include "fieldwnd.hxx"
#include "pvlaydlg.hxx"
#include "pvglob.hxx"

#ifndef _SC_ACCESSIBLEDATAPILOTCONTROL_HXX
#include "AccessibleDataPilotControl.hxx"
#endif
#ifndef SC_SCRESID_HXX
#include "scresid.hxx"
#endif
#ifndef SC_SC_HRC
#include "sc.hrc"
#endif

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
    nFieldCount( 0 ),
    nFieldSelected( 0 ),
    mbAppRTL( !!Application::GetSettings().GetLayoutRTL() ),
    pAccessible( NULL )
{
    Init();
    if (eType != TYPE_SELECT && pFtCaption)
    {
        aName = pFtCaption->GetText();
        aName.EraseAllChars('~');
    }
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
    nFieldCount( 0 ),
    nFieldSelected( 0 ),
    pAccessible( NULL )
{
    Init();
}

void ScDPFieldWindow::Init()
{
    aWndRect = Rectangle( GetPosPixel(), GetSizePixel() );
    nFieldSize = (eType == TYPE_SELECT) ? PAGE_SIZE : MAX_FIELDS;

    aFieldArr = new String*[ nFieldSize ];
    for( long nIx = 0; nIx < nFieldSize; ++nIx )
        aFieldArr[ nIx ] = NULL;

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
    for( long nIx = 0; nIx < nFieldCount; ++nIx )
        delete aFieldArr[ nIx ];
    delete[] aFieldArr;

    if (pAccessible)
    {
        com::sun::star::uno::Reference < drafts::com::sun::star::accessibility::XAccessible > xTempAcc = xAccessible;
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

Point ScDPFieldWindow::GetFieldPosition( long nIndex ) const
{
    Point aPos;
    switch( eType )
    {
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

BOOL ScDPFieldWindow::GetFieldIndex( const Point& rPos, long& rnIndex ) const
{
    rnIndex = -1;
    switch( eType )
    {
        case TYPE_ROW:
        case TYPE_DATA:
            rnIndex = rPos.Y() / OHEIGHT;
        break;
        case TYPE_COL:
        {
            long nRow = rPos.Y() / OHEIGHT;
            long nCol = rPos.X() / OWIDTH;
            rnIndex = nRow * MAX_FIELDS / 2 + nCol;
        }
        break;
        case TYPE_SELECT:
        {
            long nRow = rPos.Y() / (OHEIGHT + SSPACE);
            long nCol = rPos.X() / (OWIDTH + SSPACE);
            // is not between controls?
            if( (rPos.Y() % (OHEIGHT + SSPACE) < OHEIGHT) && (rPos.X() % (OWIDTH + SSPACE) < OWIDTH) )
                rnIndex = nCol * LINE_SIZE + nRow;
        }
        break;
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
        BOOL bOldRTL = rDev.IsRTLEnabled();
        rDev.EnableRTL( false );
        rDev.DrawCtrlText( aTextPos, GetText() );
        rDev.EnableRTL( bOldRTL );
    }
}

void ScDPFieldWindow::DrawField(
        OutputDevice& rDev,
        const Rectangle& rRect,
        const String& rText,
        BOOL bSelected )
{
    VirtualDevice aVirDev( rDev );
    aVirDev.EnableRTL( true );

    Size aDevSize( rRect.GetSize() );
    long    nWidth       = aDevSize.Width();
    long    nHeight      = aDevSize.Height();
    long    nLabelWidth  = rDev.GetTextWidth( rText );
    long    nLabelHeight = rDev.GetTextHeight();
    Point   aLabelPos(
        ((nWidth > nLabelWidth + 6) ? (nWidth - nLabelWidth) / 2 : 3),
        ((nHeight > nLabelHeight + 6) ? (nHeight - nLabelHeight) / 2 : 3) );

    aVirDev.SetOutputSizePixel( aDevSize );
    aVirDev.SetFont( rDev.GetFont() );
    DecorationView aDecoView( &aVirDev );
    aDecoView.DrawButton( Rectangle( Point( 0, 0 ), aDevSize ), bSelected ? BUTTON_DRAW_DEFAULT : 0 );
    aVirDev.SetTextColor( aTextColor );
    aVirDev.EnableRTL( false );
    aVirDev.DrawText( aLabelPos, rText );
    rDev.DrawBitmap( rRect.TopLeft(), aVirDev.GetBitmap( Point( 0, 0 ), aDevSize ) );
}

void ScDPFieldWindow::Redraw()
{
    VirtualDevice   aVirDev;
    aVirDev.EnableRTL( true );

    Point           aPos0;
    Size            aSize( GetSizePixel() );
    Font            aFont( GetFont() );         // Font vom Window
    aFont.SetTransparent( TRUE );
    aVirDev.SetFont( aFont );
    aVirDev.SetOutputSizePixel( aSize );

    DrawBackground( aVirDev );

    nFieldSelected = Max( Min( nFieldSelected, (long)(nFieldCount - 1) ), 0L );
    Rectangle aFieldRect( aPos0, GetFieldSize() );
    for( long nIx = 0; nIx < nFieldCount; ++nIx )
    {
        if( aFieldArr[ nIx ] )
        {
            aFieldRect.SetPos( GetFieldPosition( nIx ) );
            BOOL bSel = HasFocus() && (nIx == nFieldSelected);
            DrawField( aVirDev, aFieldRect, *(aFieldArr[ nIx ]), bSel );
        }
    }
    DrawBitmap( aPos0, aVirDev.GetBitmap( aPos0, aSize ) );

    if( HasFocus() && aFieldArr[ nFieldSelected ] )
    {
        long nFieldWidth = aFieldRect.GetWidth();
        long nSelectionWidth = Min( GetTextWidth( *(aFieldArr[ nFieldSelected ]) ) + 4, nFieldWidth - 6 );
        Rectangle aSelection(
            GetFieldPosition( nFieldSelected ) + Point( (nFieldWidth - nSelectionWidth) / 2, 3 ),
            Size( nSelectionWidth, aFieldRect.GetHeight() - 6 ) );
        InvertTracking( aSelection, SHOWTRACK_SMALL | SHOWTRACK_WINDOW );
    }

    WinBits nMask = ~(WB_TABSTOP | WB_NOTABSTOP);
    SetStyle( (GetStyle() & nMask) | (nFieldCount ? WB_TABSTOP : WB_NOTABSTOP) );
}

//-------------------------------------------------------------------

BOOL ScDPFieldWindow::IsValidIndex( long nIndex ) const
{
    return (nIndex >= 0) && (nIndex < nFieldSize);
}

BOOL ScDPFieldWindow::IsExistingIndex( long nIndex ) const
{
    return (nIndex >= 0) && (nIndex < nFieldCount);
}

long ScDPFieldWindow::CalcNewFieldIndex( short nDX, short nDY ) const
{
    long nNewField = nFieldSelected;
    switch( eType )
    {
        case TYPE_COL:
            nNewField += nDX + nDY * MAX_FIELDS / 2;
        break;
        case TYPE_ROW:
        case TYPE_DATA:
            nNewField += nDY;
        break;
        case TYPE_SELECT:
            nNewField += nDX * LINE_SIZE + nDY;
        break;
    }

    return IsExistingIndex( nNewField ) ? nNewField : nFieldSelected;
}

void ScDPFieldWindow::SetSelection( long nIndex )
{
    nIndex = Max( Min( nIndex, (long)(nFieldCount - 1) ), 0L );
    if( nFieldSelected != nIndex )
    {
        sal_Int32 nOldSelected(nFieldSelected);
        nFieldSelected = nIndex;
        Redraw();

        if (pAccessible && HasFocus())
        {
            com::sun::star::uno::Reference < drafts::com::sun::star::accessibility::XAccessible > xTempAcc = xAccessible;
            if (xTempAcc.is())
                pAccessible->FieldFocusChange(nOldSelected, nFieldSelected);
            else
                pAccessible = NULL;
        }
    }
}

void ScDPFieldWindow::SetSelectionHome()
{
    if( eType == TYPE_SELECT )
        pDlg->NotifyMoveSlider( KEY_HOME );
    SetSelection( 0 );
}

void ScDPFieldWindow::SetSelectionEnd()
{
    if( eType == TYPE_SELECT )
        pDlg->NotifyMoveSlider( KEY_END );
    SetSelection( nFieldCount - 1 );
}

void ScDPFieldWindow::MoveSelection( USHORT nKeyCode, short nDX, short nDY )
{
    long nNewIndex = CalcNewFieldIndex( nDX, nDY );
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

void ScDPFieldWindow::GrabFocusWithSel( long nIndex )
{
    SetSelection( nIndex );
    if( !HasFocus() )
        GrabFocus();
}

void ScDPFieldWindow::MoveField( long nDestIndex )
{
    if( nDestIndex != nFieldSelected )
    {
        // "recycle" existing functionality
        pDlg->NotifyMouseButtonDown( eType, nFieldSelected );
        pDlg->NotifyMouseButtonUp( OutputToScreenPixel( GetFieldPosition( nDestIndex ) ) );
    }
}

void ScDPFieldWindow::MoveFieldRel( short nDX, short nDY )
{
    MoveField( CalcNewFieldIndex( nDX, nDY ) );
}

//-------------------------------------------------------------------

void __EXPORT ScDPFieldWindow::Paint( const Rectangle& rRect )
{
    // Now the FixedText has its mnemonic char. Grab the text and hide the
    // FixedText to be able to handle tabstop and mnemonics separately.
    if( pFtCaption )
    {
        SetText( pFtCaption->GetText() );
        pFtCaption->Hide();
    }
    Redraw();
}

void __EXPORT ScDPFieldWindow::DataChanged( const DataChangedEvent& rDCEvt )
{
    if( (rDCEvt.GetType() == DATACHANGED_SETTINGS) && (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        GetStyleSettings();
        Redraw();
    }
    else
        Control::DataChanged( rDCEvt );
}

void __EXPORT ScDPFieldWindow::MouseButtonDown( const MouseEvent& rMEvt )
{
    if( rMEvt.IsLeft() )
    {
        long nIndex = 0;
        if( GetFieldIndex( rMEvt.GetPosPixel(), nIndex ) && aFieldArr[ nIndex ] )
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
}

void __EXPORT ScDPFieldWindow::KeyInput( const KeyEvent& rKEvt )
{
    const KeyCode& rKeyCode = rKEvt.GetKeyCode();
    USHORT nCode = rKeyCode.GetCode();
    BOOL bKeyEvaluated = FALSE;

    // revert wrong cursor direction (left/right) in RTL windows
    if( ((nCode == KEY_LEFT) || (nCode == KEY_RIGHT)) && mbAppRTL && IsRTLEnabled() )
        nCode = (nCode == KEY_LEFT) ? KEY_RIGHT : KEY_LEFT;

    if( rKeyCode.IsMod1() && (eType != TYPE_SELECT) )
    {
        bKeyEvaluated = TRUE;
        switch( nCode )
        {
            case KEY_UP:    MoveFieldRel( 0, -1 );          break;
            case KEY_DOWN:  MoveFieldRel( 0, 1 );           break;
            case KEY_LEFT:  MoveFieldRel( -1, 0 );          break;
            case KEY_RIGHT: MoveFieldRel( 1, 0 );           break;
            case KEY_HOME:  MoveField( 0 );                 break;
            case KEY_END:   MoveField( nFieldCount - 1 );   break;
            default:        bKeyEvaluated = FALSE;
        }
    }
    else
    {
        bKeyEvaluated = TRUE;
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
            default:        bKeyEvaluated = FALSE;
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
        pDlg->NotifyFieldFocus( eType, TRUE );

    if (pAccessible)
    {
        com::sun::star::uno::Reference < drafts::com::sun::star::accessibility::XAccessible > xTempAcc = xAccessible;
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
    pDlg->NotifyFieldFocus( eType, FALSE );

    if (pAccessible)
    {
        com::sun::star::uno::Reference < drafts::com::sun::star::accessibility::XAccessible > xTempAcc = xAccessible;
        if (xTempAcc.is())
            pAccessible->LostFocus();
        else
            pAccessible = NULL;
    }
}

//-------------------------------------------------------------------

void ScDPFieldWindow::AddField( const String& rText, long nNewIndex )
{
    if( IsValidIndex( nNewIndex ) && !aFieldArr[ nNewIndex ] )
    {
        aFieldArr[ nNewIndex ] = new String( rText );
        ++nFieldCount;

        if (pAccessible)
        {
            com::sun::star::uno::Reference < drafts::com::sun::star::accessibility::XAccessible > xTempAcc = xAccessible;
            if (xTempAcc.is())
                pAccessible->AddField(nNewIndex);
            else
                pAccessible = NULL;
        }
    }
}

void ScDPFieldWindow::DelField( long nDelIndex )
{
    if( IsExistingIndex( nDelIndex  ) )
    {
        DELETEZ( aFieldArr[ nDelIndex ] );
        for( long nIx = nDelIndex + 1; nIx < nFieldCount; ++nIx )
            aFieldArr[ nIx - 1 ] = aFieldArr[ nIx ];

        if (pAccessible) // before decrement fieldcount
        {
            com::sun::star::uno::Reference < drafts::com::sun::star::accessibility::XAccessible > xTempAcc = xAccessible;
            if (xTempAcc.is())
                pAccessible->RemoveField(nDelIndex);
            else
                pAccessible = NULL;
        }

        --nFieldCount;
        aFieldArr[ nFieldCount ] = NULL;
        Redraw();
    }
}

void ScDPFieldWindow::ClearFields()
{
    if( eType == TYPE_SELECT )
    {
        com::sun::star::uno::Reference < drafts::com::sun::star::accessibility::XAccessible > xTempAcc = xAccessible;
        if (!xTempAcc.is() && pAccessible)
            pAccessible = NULL;

        while( nFieldCount > 0 )
        {
            DELETEZ( aFieldArr[ nFieldCount - 1 ] );
            if (pAccessible)
                pAccessible->RemoveField(nFieldCount - 1);

            --nFieldCount;
        }
    }
}

void ScDPFieldWindow::SetFieldText( const String& rText, long nIndex )
{
    if( IsExistingIndex( nIndex ) && aFieldArr[ nIndex ] )
    {
        *(aFieldArr[ nIndex ]) = rText;
        Redraw();

        if (pAccessible)
        {
            com::sun::star::uno::Reference < drafts::com::sun::star::accessibility::XAccessible > xTempAcc = xAccessible;
            if (xTempAcc.is())
                pAccessible->FieldNameChange(nIndex);
            else
                pAccessible = NULL;
        }
    }
}

const String& ScDPFieldWindow::GetFieldText( long nIndex ) const
{
    if( IsExistingIndex( nIndex ) && aFieldArr[ nIndex] )
        return *(aFieldArr[ nIndex ]);
    return EMPTY_STRING;
}

//-------------------------------------------------------------------

BOOL ScDPFieldWindow::AddField( const String& rText, const Point& rPos, long& rnIndex )
{
    if ( nFieldCount == MAX_FIELDS )
        return FALSE;

    long nNewIndex = 0;
    if( GetFieldIndex( rPos, nNewIndex ) )
    {
        if( nNewIndex > nFieldCount )
            nNewIndex = nFieldCount;

        if( aFieldArr[ nNewIndex ] )
        {
            DBG_ASSERT( !aFieldArr[ nFieldSize - 1 ], "ScDPFieldWindow::AddField - overflow" );
            for( long nIx = nFieldCount; nIx > nNewIndex; --nIx )
                aFieldArr[ nIx ] = aFieldArr[ nIx - 1 ];
        }

        aFieldArr[ nNewIndex ] = new String( rText );
        ++nFieldCount;
        nFieldSelected = nNewIndex;
        Redraw();
        rnIndex = nNewIndex;

        if (pAccessible)
        {
            com::sun::star::uno::Reference < drafts::com::sun::star::accessibility::XAccessible > xTempAcc = xAccessible;
            if (xTempAcc.is())
                pAccessible->AddField(nNewIndex);
            else
                pAccessible = NULL;
        }

        return TRUE;
    }
    else
        return FALSE;
}

void ScDPFieldWindow::GetExistingIndex( const Point& rPos, long& rnIndex )
{
    BOOL bFound = FALSE;
    if ( (eType != TYPE_SELECT) && GetFieldIndex( rPos, rnIndex ) )
    {
        if( rnIndex >= nFieldCount )
            rnIndex = nFieldCount - 1;
    }
    else
        rnIndex = 0;
}

String ScDPFieldWindow::GetDescription()const
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
    }
    return sDescription;
}

::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessible > ScDPFieldWindow::CreateAccessible()
{
    pAccessible =
        new ScAccessibleDataPilotControl(GetAccessibleParentWindow()->GetAccessible(), this);

    com::sun::star::uno::Reference < ::drafts::com::sun::star::accessibility::XAccessible > xReturn = pAccessible;

    pAccessible->Init();
    xAccessible = xReturn;

    return xReturn;
}
//===================================================================

