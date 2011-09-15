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
#include <editeng/boxitem.hxx>
#include <svtools/svtdata.hxx>
#include <svx/hdft.hxx>
#include <vcl/decoview.hxx>
#include <vcl/gradient.hxx>
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

    basegfx::BColor lcl_GetLighterGradientColor( basegfx::BColor aDarkColor )
    {
        basegfx::BColor aHslDark = basegfx::tools::rgb2hsl( aDarkColor );
        double nLuminance = aHslDark.getZ() * 255 + 20;
        aHslDark.setZ( nLuminance / 255.0 );
        return basegfx::tools::hsl2rgb( aHslDark );
    }

    void lcl_DrawBackground( OutputDevice* pOut, const Rectangle& rRect, bool bHeader )
    {
        basegfx::BColor aLineColor = SwViewOption::GetHeaderFooterMarkColor().getBColor();

        const StyleSettings& rSettings = Application::GetSettings().GetStyleSettings();
        if ( rSettings.GetHighContrastMode() )
        {
            aLineColor = rSettings.GetDialogTextColor().getBColor();

            pOut->SetFillColor( rSettings.GetDialogColor( ) );
            pOut->SetLineColor( rSettings.GetDialogTextColor( ) );

            pOut->DrawRect( rRect );
        }
        else
        {
            // Colors
            basegfx::BColor aFillColor = lcl_GetFillColor( aLineColor );
            basegfx::BColor aLighterColor = lcl_GetLighterGradientColor( aFillColor );
            // Draw the background gradient
            Gradient aGradient;
            if ( bHeader )
                aGradient = Gradient( GRADIENT_LINEAR,
                       Color( aLighterColor ), Color( aFillColor ) );
            else
                aGradient = Gradient( GRADIENT_LINEAR,
                       Color( aFillColor ), Color( aLighterColor ) );

            pOut->DrawGradient( rRect, aGradient );

            pOut->SetFillColor( Color ( aFillColor ) );
            pOut->SetLineColor( Color ( aFillColor ) );
        }

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


SwHeaderFooterWin::SwHeaderFooterWin( SwEditWin* pEditWin, const SwPageFrm* pPageFrm, bool bHeader ) :
    MenuButton( pEditWin, WB_DIALOGCONTROL  ),
    m_pEditWin( pEditWin ),
    m_sLabel( ),
    m_pPageFrm( pPageFrm ),
    m_bIsHeader( bHeader ),
    m_bReadonly( false ),
    m_pPopupMenu( NULL )
{
    // Define the readonly member
    const SwViewOption* pViewOpt = m_pEditWin->GetView().GetWrtShell().GetViewOptions();
    m_bReadonly = pViewOpt->IsReadonly();

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

    // Create and set the PopupMenu
    m_pPopupMenu = new PopupMenu( SW_RES( MN_HEADERFOOTER_BUTTON ) );

    // Rewrite the menu entries' text
    String sType = SW_RESSTR( STR_FOOTER );
    if ( m_bIsHeader )
        sType = SW_RESSTR( STR_HEADER );
    SwRewriter aRewriter;
    aRewriter.AddRule( String::CreateFromAscii( "$1" ), sType );

    String aText = m_pPopupMenu->GetItemText( FN_HEADERFOOTER_EDIT );
    m_pPopupMenu->SetItemText( FN_HEADERFOOTER_EDIT, aRewriter.Apply( aText ) );

    aText = m_pPopupMenu->GetItemText( FN_HEADERFOOTER_DELETE );
    m_pPopupMenu->SetItemText( FN_HEADERFOOTER_DELETE, aRewriter.Apply( aText ) );
    SetPopupMenu( m_pPopupMenu );
}

SwHeaderFooterWin::~SwHeaderFooterWin( )
{
    delete m_pPopupMenu;
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
    if ( !m_bIsHeader )
        nYFooterOff = aBoxSize.Height();

    Point aBoxPos( aOffset.X() - aBoxSize.Width() - BOX_DISTANCE,
                   aOffset.Y() - nYFooterOff );

    // Set the position & Size of the window
    SetPosSizePixel( aBoxPos, aBoxSize );
}

void SwHeaderFooterWin::Paint( const Rectangle& )
{
    const Rectangle aRect( Rectangle( Point( 0, 0 ), PixelToLogic( GetSizePixel() ) ) );
    lcl_DrawBackground( this, aRect, m_bIsHeader );

    // Draw the text
    Rectangle aTextRect;
    GetTextBoundRect( aTextRect, String( m_sLabel ) );
    Point aTextPos = aTextRect.TopLeft() + Point( TEXT_PADDING, 0 );

    const StyleSettings& rSettings = Application::GetSettings().GetStyleSettings();
    basegfx::BColor aLineColor = SwViewOption::GetHeaderFooterMarkColor().getBColor();
    if ( rSettings.GetHighContrastMode( ) )
        aLineColor = rSettings.GetDialogTextColor().getBColor();
    SetTextColor( Color( aLineColor ) );
    DrawText( aTextPos, String( m_sLabel ) );

    // Paint the symbol if not readonly button
    if ( !m_bReadonly )
    {
        Point aPicPos( aRect.getWidth() - BUTTON_WIDTH, 0 );
        Size aPicSize( BUTTON_WIDTH, aRect.getHeight() );
        Rectangle aSymbolRect( aPicPos, aPicSize );

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

        SymbolType nSymbol = SYMBOL_SPIN_DOWN;
        if ( IsEmptyHeaderFooter( ) )
            nSymbol = SYMBOL_PLUS;
        DecorationView aDecoView( this );
        aDecoView.DrawSymbol( aSymbolRect, nSymbol,
                              ( Application::GetSettings().GetStyleSettings().GetHighContrastMode()
                                ? Color( COL_WHITE )
                                : Color( COL_BLACK ) ) );
    }
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

void SwHeaderFooterWin::ExecuteCommand( sal_uInt16 nSlot )
{
    SwView& rView = m_pEditWin->GetView();
    SwWrtShell& rSh = rView.GetWrtShell();

    const String& rStyleName = GetPageFrame()->GetPageDesc()->GetName();
    switch ( nSlot )
    {
        case FN_HEADERFOOTER_EDIT:
            {
                sal_uInt16 nPageId = TP_FOOTER_PAGE;
                if ( m_bIsHeader )
                    nPageId = TP_HEADER_PAGE;

                rView.GetDocShell()->FormatPage(
                        rStyleName,
                        nPageId, &rSh );
            }
            break;
        case FN_HEADERFOOTER_BORDERBACK:
            {
                const SwPageDesc* pDesc = m_pPageFrm->GetPageDesc();
                const SwFrmFmt& rMaster = pDesc->GetMaster();
                SwFrmFmt* pHFFmt = const_cast< SwFrmFmt* >( rMaster.GetFooter().GetFooterFmt() );
                if ( m_bIsHeader )
                    pHFFmt = const_cast< SwFrmFmt* >( rMaster.GetHeader().GetHeaderFmt() );


                SfxItemPool* pPool = pHFFmt->GetAttrSet().GetPool();
                SfxItemSet aSet( *pPool,
                       RES_BACKGROUND, RES_BACKGROUND,
                       RES_BOX, RES_BOX,
                       SID_ATTR_BORDER_INNER, SID_ATTR_BORDER_INNER,
                       RES_SHADOW, RES_SHADOW, 0 );

                aSet.Put( pHFFmt->GetAttrSet() );

                // Create a box info item... needed by the dialog
                SvxBoxInfoItem aBoxInfo( SID_ATTR_BORDER_INNER );
                const SfxPoolItem *pBoxInfo;
                if ( SFX_ITEM_SET == pHFFmt->GetAttrSet().GetItemState( SID_ATTR_BORDER_INNER,
                                                        sal_True, &pBoxInfo) )
                    aBoxInfo = *(SvxBoxInfoItem*)pBoxInfo;

                aBoxInfo.SetTable( sal_False );
                aBoxInfo.SetDist( sal_True);
                aBoxInfo.SetMinDist( sal_False );
                aBoxInfo.SetDefDist( MIN_BORDER_DIST );
                aBoxInfo.SetValid( VALID_DISABLE );
                aSet.Put( aBoxInfo );

                if ( svx::ShowBorderBackgroundDlg( this, &aSet, true ) )
                {
                    const SfxPoolItem* pItem;
                    if ( SFX_ITEM_SET == aSet.GetItemState( RES_BACKGROUND, sal_False, &pItem ) )
                        pHFFmt->SetFmtAttr( *pItem );

                    if ( SFX_ITEM_SET == aSet.GetItemState( RES_BOX, sal_False, &pItem ) )
                        pHFFmt->SetFmtAttr( *pItem );

                    if ( SFX_ITEM_SET == aSet.GetItemState( RES_SHADOW, sal_False, &pItem ) )
                        pHFFmt->SetFmtAttr( *pItem );
                }
            }
            break;
        case FN_HEADERFOOTER_DELETE:
            {
                rSh.ChangeHeaderOrFooter( rStyleName, m_bIsHeader, false, true );
            }
            break;
        default:
            break;
    }
}

void SwHeaderFooterWin::SetReadonly( bool bReadonly )
{
    m_bReadonly = bReadonly;
    Update();
}

void SwHeaderFooterWin::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( IsEmptyHeaderFooter( ) )
    {
        SwView& rView = m_pEditWin->GetView();
        SwWrtShell& rSh = rView.GetWrtShell();

        const String& rStyleName = GetPageFrame()->GetPageDesc()->GetName();
        rSh.ChangeHeaderOrFooter( rStyleName, m_bIsHeader, true, false );
    }
    else
        MenuButton::MouseButtonDown( rMEvt );
}

void SwHeaderFooterWin::Select( )
{
    ExecuteCommand( GetCurItemId() );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
