/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * [ Copyright (C) 2011 SUSE <cbosdonnat@suse.com> (initial developer) ]
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include <app.hrc>
#include <docvw.hrc>
#include <globals.hrc>
#include <popup.hrc>
#include <svtools/svtools.hrc>

#include <cmdid.h>
#include <docsh.hxx>
#include <edtwin.hxx>
#include <fmthdft.hxx>
#include <HeaderFooterWin.hxx>
#include <pagefrm.hxx>
#include <SwRewriter.hxx>
#include <view.hxx>
#include <viewopt.hxx>
#include <wrtsh.hxx>

#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/color/bcolortools.hxx>
#include <editeng/ulspitem.hxx>
#include <svtools/svtdata.hxx>
#include <vcl/decoview.hxx>
#include <vcl/menubtn.hxx>
#include <vcl/svapp.hxx>

#define TEXT_PADDING 5
#define BOX_DISTANCE 10
#define BUTTON_WIDTH 18

namespace
{
    basegfx::BColor lcl_GetFillColor( basegfx::BColor aLineColor )
    {
        basegfx::BColor aHslLine = basegfx::tools::rgb2hsl( aLineColor );
        double nLuminance = aHslLine.getZ() * 2.5;
        if ( nLuminance == 0 )
            nLuminance = 0.5;
        else if ( nLuminance >= 1.0 )
            nLuminance = aHslLine.getZ() * 0.4;
        aHslLine.setZ( nLuminance );
        return basegfx::tools::hsl2rgb( aHslLine );
    }

    void lcl_DrawBackground( OutputDevice* pOut, const Rectangle& rRect, bool bHeader )
    {
        // Colors
        basegfx::BColor aLineColor = SwViewOption::GetHeaderFooterMarkColor().getBColor();
        basegfx::BColor aFillColor = lcl_GetFillColor( aLineColor );

        // Draw the background rect
        pOut->SetFillColor( Color ( aFillColor ) );
        pOut->SetLineColor( Color ( aFillColor ) );
        pOut->DrawRect( rRect );

        // Draw the lines around the rect
        pOut->SetLineColor( Color( aLineColor ) );
        basegfx::B2DPolygon aPolygon;
        aPolygon.append( basegfx::B2DPoint( rRect.Left(), rRect.Top() ) );
        aPolygon.append( basegfx::B2DPoint( rRect.Left(), rRect.Bottom() ) );
        pOut->DrawPolyLine( aPolygon, 1.0 );

        aPolygon.clear();
        aPolygon.append( basegfx::B2DPoint( rRect.Right(), rRect.Top() ) );
        aPolygon.append( basegfx::B2DPoint( rRect.Right(), rRect.Bottom() ) );
        pOut->DrawPolyLine( aPolygon, 1.0 );

        long nYLine = rRect.Bottom();
        if ( !bHeader )
            nYLine = rRect.Top();
        aPolygon.clear();
        aPolygon.append( basegfx::B2DPoint( rRect.Left(), nYLine ) );
        aPolygon.append( basegfx::B2DPoint( rRect.Right(), nYLine ) );
        pOut->DrawPolyLine( aPolygon, 1.0 );
    }
}

class SwHeaderFooterButton : public MenuButton
{
    SwHeaderFooterWin* m_pWindow;
    PopupMenu*         m_pPopupMenu;

    public:
        SwHeaderFooterButton( SwHeaderFooterWin* pWindow );
        ~SwHeaderFooterButton( );

        // overloaded <MenuButton> methods
        virtual void Select();

        // overloaded <Window> methods
        virtual void Paint( const Rectangle& rRect );
        virtual void MouseButtonDown( const MouseEvent& rMEvt );
};


SwHeaderFooterWin::SwHeaderFooterWin( SwEditWin* pEditWin, const SwPageFrm* pPageFrm, bool bHeader ) :
    Window( pEditWin, WB_DIALOGCONTROL  ),
    m_pEditWin( pEditWin ),
    m_sLabel( ),
    m_pPageFrm( pPageFrm ),
    m_bIsHeader( bHeader ),
    m_pButton( NULL )
{
    // Get the font and configure it
    Font aFont = GetSettings().GetStyleSettings().GetToolFont();
    SetZoomedPointFont( aFont );

    // Use pixels for the rest of the drawing
    SetMapMode( MapMode ( MAP_PIXEL ) );

    // Compute the text to show
    m_sLabel = ResId::toString( SW_RES( STR_HEADER_TITLE ) );
    if ( !m_bIsHeader )
        m_sLabel = ResId::toString( SW_RES( STR_FOOTER_TITLE ) );
    sal_Int32 nPos = m_sLabel.lastIndexOf( rtl::OUString::createFromAscii( "%1" ) );
    m_sLabel = m_sLabel.replaceAt( nPos, 2, m_pPageFrm->GetPageDesc()->GetName() );
}

SwHeaderFooterWin::~SwHeaderFooterWin( )
{
    delete m_pButton;
}

MenuButton* SwHeaderFooterWin::GetMenuButton()
{
    if ( !m_pButton )
    {
        m_pButton = new SwHeaderFooterButton( this );

        // Don't blindly show it: check for readonly document
        const SwViewOption* pViewOpt = m_pEditWin->GetView().GetWrtShell().GetViewOptions();
        m_pButton->Show( !pViewOpt->IsReadonly() );
    }

    return m_pButton;
}

void SwHeaderFooterWin::SetOffset( Point aOffset )
{
    // Compute the text size and get the box position & size from it
    Rectangle aTextRect;
    GetTextBoundRect( aTextRect, String( m_sLabel ) );
    Rectangle aTextPxRect = LogicToPixel( aTextRect );

    Size  aBoxSize ( aTextPxRect.GetWidth() + BUTTON_WIDTH + TEXT_PADDING * 2,
                     aTextPxRect.GetHeight() + TEXT_PADDING  * 2 );

    long nYFooterOff = 0;
    if ( !IsHeader() )
        nYFooterOff = aBoxSize.Height();

    Point aBoxPos( aOffset.X() - aBoxSize.Width() - BOX_DISTANCE,
                   aOffset.Y() - nYFooterOff );

    // Set the position & Size of the window
    SetPosSizePixel( aBoxPos, aBoxSize );

    // Set the button position and size
    Point aBtnPos( aBoxSize.getWidth() - BUTTON_WIDTH, 0 );
    Size aBtnSize( BUTTON_WIDTH, aBoxSize.getHeight() );
    GetMenuButton()->SetPosSizePixel( aBtnPos, aBtnSize );
}

void SwHeaderFooterWin::Paint( const Rectangle& )
{
    const Rectangle aRect( Rectangle( Point( 0, 0 ), PixelToLogic( GetSizePixel() ) ) );
    lcl_DrawBackground( this, aRect, m_bIsHeader );

    // Draw the text
    Rectangle aTextRect;
    GetTextBoundRect( aTextRect, String( m_sLabel ) );
    Point aTextPos = aTextRect.TopLeft() + Point( TEXT_PADDING, 0 );

    basegfx::BColor aLineColor = SwViewOption::GetHeaderFooterMarkColor().getBColor();
    SetTextColor( Color( aLineColor ) );
    DrawText( aTextPos, String( m_sLabel ) );
}

bool SwHeaderFooterWin::IsEmptyHeaderFooter( )
{
    bool bResult = true;

    // Actually check it
    const SwPageDesc* pDesc = m_pPageFrm->GetPageDesc();

    const SwFrmFmt* pFmt = pDesc->GetLeftFmt();
    if ( m_pPageFrm->OnRightPage() )
         pFmt = pDesc->GetRightFmt();

    if ( pFmt )
    {
        if ( m_bIsHeader )
            bResult = !pFmt->GetHeader().IsActive();
        else
            bResult = !pFmt->GetFooter().IsActive();
    }

    return bResult;
}

void SwHeaderFooterWin::ChangeHeaderOrFooter( bool bAdd )
{
    SwWrtShell& rSh = m_pEditWin->GetView().GetWrtShell();
    rSh.addCurrentPosition();
    rSh.StartAllAction();
    rSh.StartUndo( UNDO_HEADER_FOOTER );

    const SwPageDesc* pPageDesc = GetPageFrame()->GetPageDesc();
    SwFrmFmt& rMaster = const_cast< SwFrmFmt& > (pPageDesc->GetMaster() );

    if ( m_bIsHeader )
        rMaster.SetFmtAttr( SwFmtHeader( bAdd ) );
    else
        rMaster.SetFmtAttr( SwFmtFooter( bAdd ) );

    if ( bAdd )
    {
        SvxULSpaceItem aUL( m_bIsHeader ? 0 : MM50, m_bIsHeader ? MM50 : 0, RES_UL_SPACE );
        SwFrmFmt* pFmt = m_bIsHeader ?
            ( SwFrmFmt* )rMaster.GetHeader().GetHeaderFmt():
            ( SwFrmFmt* )rMaster.GetFooter().GetFooterFmt();
        pFmt->SetFmtAttr( aUL );
    }


    rSh.EndUndo( UNDO_HEADER_FOOTER );
    rSh.EndAllAction();
}

void SwHeaderFooterWin::ExecuteCommand( sal_uInt16 nSlot )
{
    switch ( nSlot )
    {
        case FN_HEADERFOOTER_EDIT:
            {
                SwView& rView = m_pEditWin->GetView();
                SwWrtShell& rSh = rView.GetWrtShell();
                sal_uInt16 nPageId = TP_FOOTER_PAGE;
                if ( IsHeader() )
                    nPageId = TP_HEADER_PAGE;

                rView.GetDocShell()->FormatPage(
                        GetPageFrame()->GetPageDesc()->GetName(),
                        nPageId, &rSh );
            }
            break;
        case FN_HEADERFOOTER_DELETE:
            ChangeHeaderOrFooter( false );
            break;
        default:
            break;
    }
}

void SwHeaderFooterWin::SetReadonly( bool bReadonly )
{
    if ( bReadonly )
        m_pButton->Hide();
    else
        m_pButton->Show();
    Update();
}

SwHeaderFooterButton::SwHeaderFooterButton( SwHeaderFooterWin* pWindow ) :
    MenuButton( pWindow ),
    m_pWindow( pWindow )
{
    // Create and set the PopupMenu
    m_pPopupMenu = new PopupMenu( SW_RES( MN_HEADERFOOTER_BUTTON ) );

    // Rewrite the menu entries' text
    String sType = SW_RESSTR( STR_FOOTER );
    if ( m_pWindow->IsHeader() )
        sType = SW_RESSTR( STR_HEADER );
    SwRewriter aRewriter;
    aRewriter.AddRule( String::CreateFromAscii( "$1" ), sType );

    String aText = m_pPopupMenu->GetItemText( FN_HEADERFOOTER_EDIT );
    m_pPopupMenu->SetItemText( FN_HEADERFOOTER_EDIT, aRewriter.Apply( aText ) );

    aText = m_pPopupMenu->GetItemText( FN_HEADERFOOTER_DELETE );
    m_pPopupMenu->SetItemText( FN_HEADERFOOTER_DELETE, aRewriter.Apply( aText ) );

    SetPopupMenu( m_pPopupMenu );
}

SwHeaderFooterButton::~SwHeaderFooterButton( )
{
    delete m_pPopupMenu;
}

void SwHeaderFooterButton::Paint( const Rectangle& )
{
    const Rectangle aRect( Rectangle( Point( 0, 0 ), PixelToLogic( GetSizePixel() ) ) );

    lcl_DrawBackground( this, aRect, m_pWindow->IsHeader() );

    Rectangle aSymbolRect( aRect );
    // 25% distance to the left and right button border
    const long nBorderDistanceLeftAndRight = ((aSymbolRect.GetWidth()*250)+500)/1000;
    aSymbolRect.Left()+=nBorderDistanceLeftAndRight;
    aSymbolRect.Right()-=nBorderDistanceLeftAndRight;
    // 30% distance to the top button border
    const long nBorderDistanceTop = ((aSymbolRect.GetHeight()*300)+500)/1000;
    aSymbolRect.Top()+=nBorderDistanceTop;
    // 25% distance to the bottom button border
    const long nBorderDistanceBottom = ((aSymbolRect.GetHeight()*250)+500)/1000;
    aSymbolRect.Bottom()-=nBorderDistanceBottom;

    if ( m_pWindow->IsEmptyHeaderFooter( ) )
    {
        SvtResId id( BMP_LIST_ADD );
        Image aPlusImg( id );
        Size aSize = aPlusImg.GetSizePixel();
        Point aPt = aRect.TopLeft();
        long nXOffset = ( aRect.GetWidth() - aSize.Width() ) / 2;
        long nYOffset = ( aRect.GetHeight() - aSize.Height() ) / 2;
        aPt += Point( nXOffset, nYOffset );
        DrawImage(aPt, aPlusImg);
    }
    else
    {
        DecorationView aDecoView( this );
        aDecoView.DrawSymbol( aSymbolRect, SYMBOL_SPIN_DOWN,
                              ( Application::GetSettings().GetStyleSettings().GetHighContrastMode()
                                ? Color( COL_WHITE )
                                : Color( COL_BLACK ) ) );
    }
}

void SwHeaderFooterButton::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( m_pWindow->IsEmptyHeaderFooter( ) )
    {
        // Add the header / footer
        m_pWindow->ChangeHeaderOrFooter( true );
    }
    else
        MenuButton::MouseButtonDown( rMEvt );
}

void SwHeaderFooterButton::Select( )
{
    m_pWindow->ExecuteCommand( GetCurItemId() );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
