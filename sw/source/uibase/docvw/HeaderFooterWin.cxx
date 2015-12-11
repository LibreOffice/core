/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <app.hrc>
#include <docvw.hrc>
#include <globals.hrc>
#include <popup.hrc>
#include <svtools/svtools.hrc>

#include <cmdid.h>
#include <DashedLine.hxx>
#include <docsh.hxx>
#include <edtwin.hxx>
#include <fmthdft.hxx>
#include <HeaderFooterWin.hxx>
#include <pagedesc.hxx>
#include <pagefrm.hxx>
#include <SwRewriter.hxx>
#include <view.hxx>
#include <viewopt.hxx>
#include <wrtsh.hxx>

#include <basegfx/color/bcolortools.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/range/b2drectangle.hxx>
#include <basegfx/vector/b2dsize.hxx>
#include <drawinglayer/attribute/fillgradientattribute.hxx>
#include <drawinglayer/attribute/fontattribute.hxx>
#include <drawinglayer/primitive2d/fillgradientprimitive2d.hxx>
#include <drawinglayer/primitive2d/modifiedcolorprimitive2d.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/textlayoutdevice.hxx>
#include <drawinglayer/primitive2d/textprimitive2d.hxx>
#include <editeng/boxitem.hxx>
#include <svtools/svtresid.hxx>
#include <svx/hdft.hxx>
#include <drawinglayer/processor2d/processorfromoutputdevice.hxx>
#include <vcl/decoview.hxx>
#include <vcl/gradient.hxx>
#include <vcl/menubtn.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <memory>

#define TEXT_PADDING 5
#define BOX_DISTANCE 10
#define BUTTON_WIDTH 18

using namespace basegfx;
using namespace basegfx::tools;
using namespace drawinglayer::attribute;

namespace
{
    static basegfx::BColor lcl_GetFillColor(const basegfx::BColor& rLineColor)
    {
        basegfx::BColor aHslLine = basegfx::tools::rgb2hsl(rLineColor);
        double nLuminance = aHslLine.getZ() * 2.5;
        if ( nLuminance == 0 )
            nLuminance = 0.5;
        else if ( nLuminance >= 1.0 )
            nLuminance = aHslLine.getZ() * 0.4;
        aHslLine.setZ( nLuminance );
        return basegfx::tools::hsl2rgb( aHslLine );
    }

    static basegfx::BColor lcl_GetLighterGradientColor(const basegfx::BColor& rDarkColor)
    {
        basegfx::BColor aHslDark = basegfx::tools::rgb2hsl(rDarkColor);
        double nLuminance = aHslDark.getZ() * 255 + 20;
        aHslDark.setZ( nLuminance / 255.0 );
        return basegfx::tools::hsl2rgb( aHslDark );
    }

    static B2DPolygon lcl_GetPolygon( const Rectangle& rRect, bool bHeader )
    {
        const double nRadius = 3;
        const double nKappa((M_SQRT2 - 1.0) * 4.0 / 3.0);

        B2DPolygon aPolygon;
        aPolygon.append( B2DPoint( rRect.Left(), rRect.Top() ) );

        {
            B2DPoint aCorner( rRect.Left(), rRect.Bottom() );
            B2DPoint aStart( rRect.Left(), rRect.Bottom() - nRadius );
            B2DPoint aEnd( rRect.Left() + nRadius, rRect.Bottom() );
            aPolygon.append( aStart );
            aPolygon.appendBezierSegment(
                    interpolate( aStart, aCorner, nKappa ),
                    interpolate( aEnd, aCorner, nKappa ),
                    aEnd );
        }

        {
            B2DPoint aCorner( rRect.Right(), rRect.Bottom() );
            B2DPoint aStart( rRect.Right() - nRadius, rRect.Bottom() );
            B2DPoint aEnd( rRect.Right(), rRect.Bottom() - nRadius );
            aPolygon.append( aStart );
            aPolygon.appendBezierSegment(
                    interpolate( aStart, aCorner, nKappa ),
                    interpolate( aEnd, aCorner, nKappa ),
                    aEnd );
        }

        aPolygon.append( B2DPoint( rRect.Right(), rRect.Top() ) );

        if ( !bHeader )
        {
            B2DRectangle aBRect( rRect.Left(), rRect.Top(), rRect.Right(), rRect.Bottom() );
            B2DHomMatrix aRotation = createRotateAroundPoint(
                   aBRect.getCenterX(), aBRect.getCenterY(), M_PI );
            aPolygon.transform( aRotation );
        }

        return aPolygon;
    }
}

SwHeaderFooterWin::SwHeaderFooterWin( SwEditWin* pEditWin, const SwFrame *pFrame, bool bHeader ) :
    SwFrameMenuButtonBase( pEditWin, pFrame ),
    m_bIsHeader( bHeader ),
    m_pPopupMenu( nullptr ),
    m_pLine( nullptr ),
    m_bIsAppearing( false ),
    m_nFadeRate( 100 ),
    m_aFadeTimer( )
{
    //FIXME RenderContext

    // Get the font and configure it
    vcl::Font aFont = Application::GetSettings().GetStyleSettings().GetToolFont();
    SetZoomedPointFont(*this, aFont);

    // Create the line control
    m_pLine = VclPtr<SwDashedLine>::Create(GetEditWin(), &SwViewOption::GetHeaderFooterMarkColor);
    m_pLine->SetZOrder(this, ZOrderFlags::Before);

    // Create and set the PopupMenu
    m_pPopupMenu = new PopupMenu(SW_RES(MN_HEADERFOOTER_BUTTON));

    // Rewrite the menu entries' text
    if (m_bIsHeader)
    {
        m_pPopupMenu->SetItemText(FN_HEADERFOOTER_EDIT, SW_RESSTR(STR_FORMAT_HEADER));
        m_pPopupMenu->SetItemText(FN_HEADERFOOTER_DELETE, SW_RESSTR(STR_DELETE_HEADER));
    }
    else
    {
        m_pPopupMenu->SetItemText(FN_HEADERFOOTER_EDIT, SW_RESSTR(STR_FORMAT_FOOTER));
        m_pPopupMenu->SetItemText(FN_HEADERFOOTER_DELETE, SW_RESSTR(STR_DELETE_FOOTER));
    }

    SetPopupMenu(m_pPopupMenu);

    m_aFadeTimer.SetTimeout(50);
    m_aFadeTimer.SetTimeoutHdl(LINK(this, SwHeaderFooterWin, FadeHandler));
}

SwHeaderFooterWin::~SwHeaderFooterWin( )
{
    disposeOnce();
}

void SwHeaderFooterWin::dispose()
{
    delete m_pPopupMenu;
    m_pLine.disposeAndClear();
    SwFrameMenuButtonBase::dispose();
}

void SwHeaderFooterWin::SetOffset(Point aOffset, long nXLineStart, long nXLineEnd)
{
    // Compute the text to show
    const SwPageDesc* pDesc = GetPageFrame()->GetPageDesc();
    bool bIsFirst = !pDesc->IsFirstShared() && GetPageFrame()->OnFirstPage();
    bool bIsLeft  = !pDesc->IsHeaderShared() && !GetPageFrame()->OnRightPage();
    bool bIsRight = !pDesc->IsHeaderShared() && GetPageFrame()->OnRightPage();
    m_sLabel = SW_RESSTR(STR_HEADER_TITLE);
    if (!m_bIsHeader)
        m_sLabel = bIsFirst ? SW_RESSTR(STR_FIRST_FOOTER_TITLE)
            : bIsLeft  ? SW_RESSTR(STR_LEFT_FOOTER_TITLE)
            : bIsRight ? SW_RESSTR(STR_RIGHT_FOOTER_TITLE)
            : SW_RESSTR(STR_FOOTER_TITLE );
    else
        m_sLabel = bIsFirst ? SW_RESSTR(STR_FIRST_HEADER_TITLE)
            : bIsLeft  ? SW_RESSTR(STR_LEFT_HEADER_TITLE)
            : bIsRight ? SW_RESSTR(STR_RIGHT_HEADER_TITLE)
            : SW_RESSTR(STR_HEADER_TITLE);

    sal_Int32 nPos = m_sLabel.lastIndexOf("%1");
    m_sLabel = m_sLabel.replaceAt(nPos, 2, pDesc->GetName());

    // Compute the text size and get the box position & size from it
    Rectangle aTextRect;
    GetTextBoundRect(aTextRect, OUString(m_sLabel));
    Rectangle aTextPxRect = LogicToPixel(aTextRect);
    FontMetric aFontMetric = GetFontMetric(GetFont());
    Size aBoxSize (aTextPxRect.GetWidth() + BUTTON_WIDTH + TEXT_PADDING * 2,
                   aFontMetric.GetLineHeight() + TEXT_PADDING  * 2 );

    long nYFooterOff = 0;
    if (!m_bIsHeader)
        nYFooterOff = aBoxSize.Height();

    Point aBoxPos(aOffset.X() - aBoxSize.Width() - BOX_DISTANCE,
                  aOffset.Y() - nYFooterOff);

    if (AllSettings::GetLayoutRTL())
    {
        aBoxPos.setX( aOffset.X() + BOX_DISTANCE );
    }

    // Set the position & Size of the window
    SetPosSizePixel(aBoxPos, aBoxSize);

    double nYLinePos = aBoxPos.Y();
    if (!m_bIsHeader)
        nYLinePos += aBoxSize.Height();
    Point aLinePos(nXLineStart, nYLinePos);
    Size aLineSize(nXLineEnd - nXLineStart, 1);
    m_pLine->SetPosSizePixel(aLinePos, aLineSize);
}

void SwHeaderFooterWin::ShowAll(bool bShow)
{
    if (!PopupMenu::IsInExecute())
    {
        m_bIsAppearing = bShow;

        if (m_aFadeTimer.IsActive())
            m_aFadeTimer.Stop();
        m_aFadeTimer.Start();
    }
}

bool SwHeaderFooterWin::Contains( const Point &rDocPt ) const
{
    Rectangle aRect(GetPosPixel(), GetSizePixel());
    if (aRect.IsInside(rDocPt))
        return true;

    Rectangle aLineRect(m_pLine->GetPosPixel(), m_pLine->GetSizePixel());
    if (aLineRect.IsInside(rDocPt))
        return true;

    return false;
}

void SwHeaderFooterWin::Paint(vcl::RenderContext& rRenderContext, const Rectangle&)
{
    // Use pixels for the rest of the drawing
    SetMapMode(MapMode(MAP_PIXEL));

    const Rectangle aRect(Rectangle(Point(0, 0), rRenderContext.PixelToLogic(GetSizePixel())));
    drawinglayer::primitive2d::Primitive2DContainer aSeq(3);

    B2DPolygon aPolygon = lcl_GetPolygon(aRect, m_bIsHeader);

    // Colors
    basegfx::BColor aLineColor = SwViewOption::GetHeaderFooterMarkColor().getBColor();
    basegfx::BColor aFillColor = lcl_GetFillColor(aLineColor);
    basegfx::BColor aLighterColor = lcl_GetLighterGradientColor(aFillColor);

    const StyleSettings& rSettings = Application::GetSettings().GetStyleSettings();
    if (rSettings.GetHighContrastMode())
    {
        aFillColor = rSettings.GetDialogColor().getBColor();
        aLineColor = rSettings.GetDialogTextColor().getBColor();

        aSeq[0] = drawinglayer::primitive2d::Primitive2DReference(
                    new drawinglayer::primitive2d::PolyPolygonColorPrimitive2D(B2DPolyPolygon(aPolygon), aFillColor));
    }
    else
    {
        B2DRectangle aGradientRect(aRect.Left(), aRect.Top(), aRect.Right(), aRect.Bottom());
        double nAngle = M_PI;
        if (m_bIsHeader)
            nAngle = 0;
        FillGradientAttribute aFillAttrs(GRADIENTSTYLE_LINEAR, 0.0, 0.0, 0.0, nAngle, aLighterColor, aFillColor, 10);
        aSeq[0] = drawinglayer::primitive2d::Primitive2DReference(
                    new drawinglayer::primitive2d::FillGradientPrimitive2D(aGradientRect, aFillAttrs));
    }

    // Create the border lines primitive
    aSeq[1] = drawinglayer::primitive2d::Primitive2DReference(
                new drawinglayer::primitive2d::PolygonHairlinePrimitive2D(aPolygon, aLineColor));

    // Create the text primitive
    B2DVector aFontSize;
    FontAttribute aFontAttr = drawinglayer::primitive2d::getFontAttributeFromVclFont(aFontSize, rRenderContext.GetFont(), false, false);

    FontMetric aFontMetric = rRenderContext.GetFontMetric(rRenderContext.GetFont());
    double nTextOffsetY = aFontMetric.GetAscent() + TEXT_PADDING;
    Point aTextPos(TEXT_PADDING, nTextOffsetY);

    basegfx::B2DHomMatrix aTextMatrix(createScaleTranslateB2DHomMatrix(
                                            aFontSize.getX(), aFontSize.getY(),
                                            double(aTextPos.X()), double(aTextPos.Y())));

    aSeq[2] = drawinglayer::primitive2d::Primitive2DReference(
                    new drawinglayer::primitive2d::TextSimplePortionPrimitive2D(
                        aTextMatrix, OUString(m_sLabel), 0, m_sLabel.getLength(),
                        std::vector<double>(), aFontAttr, css::lang::Locale(), aLineColor));

    // Create the 'plus' or 'arrow' primitive
    B2DRectangle aSignArea(B2DPoint(aRect.Right() - BUTTON_WIDTH, 0.0),
                           B2DSize(aRect.Right(), aRect.getHeight()));

    B2DPolygon aSign;
    if (IsEmptyHeaderFooter())
    {
        // Create the + polygon
        double nLeft = aSignArea.getMinX() + TEXT_PADDING;
        double nRight = aSignArea.getMaxX() - TEXT_PADDING;
        double nHalfW = ( nRight - nLeft ) / 2.0;

        double nTop = aSignArea.getCenterY() - nHalfW;
        double nBottom = aSignArea.getCenterY() + nHalfW;

        aSign.append(B2DPoint(nLeft, aSignArea.getCenterY() - 1.0));
        aSign.append(B2DPoint(aSignArea.getCenterX() - 1.0, aSignArea.getCenterY() - 1.0));
        aSign.append(B2DPoint(aSignArea.getCenterX() - 1.0, nTop));
        aSign.append(B2DPoint(aSignArea.getCenterX() + 1.0, nTop));
        aSign.append(B2DPoint(aSignArea.getCenterX() + 1.0, aSignArea.getCenterY() - 1.0));
        aSign.append(B2DPoint(nRight, aSignArea.getCenterY() - 1.0));
        aSign.append(B2DPoint(nRight, aSignArea.getCenterY() + 1.0));
        aSign.append(B2DPoint(aSignArea.getCenterX() + 1.0, aSignArea.getCenterY() + 1.0));
        aSign.append(B2DPoint(aSignArea.getCenterX() + 1.0, nBottom));
        aSign.append(B2DPoint(aSignArea.getCenterX() - 1.0, nBottom));
        aSign.append(B2DPoint(aSignArea.getCenterX() - 1.0, aSignArea.getCenterY() + 1.0));
        aSign.append(B2DPoint(nLeft, aSignArea.getCenterY() + 1.0));
        aSign.setClosed(true);
    }
    else
    {
        // Create the v polygon
        B2DPoint aLeft(aSignArea.getMinX() + TEXT_PADDING, aSignArea.getCenterY());
        B2DPoint aRight(aSignArea.getMaxX() - TEXT_PADDING, aSignArea.getCenterY());
        B2DPoint aBottom((aLeft.getX() + aRight.getX()) / 2.0, aLeft.getY() + 4.0);
        aSign.append(aLeft);
        aSign.append(aRight);
        aSign.append(aBottom);
        aSign.setClosed(true);
    }

    BColor aSignColor = Color(COL_BLACK).getBColor();
    if (Application::GetSettings().GetStyleSettings().GetHighContrastMode())
        aSignColor = Color(COL_WHITE).getBColor();

    aSeq.resize(aSeq.size() + 1);
    aSeq[aSeq.size() - 1] = drawinglayer::primitive2d::Primitive2DReference(
                                    new drawinglayer::primitive2d::PolyPolygonColorPrimitive2D(
                                        B2DPolyPolygon(aSign), aSignColor));

    // Create the processor and process the primitives
    const drawinglayer::geometry::ViewInformation2D aNewViewInfos;
    std::unique_ptr<drawinglayer::processor2d::BaseProcessor2D> pProcessor(
        drawinglayer::processor2d::createBaseProcessor2DFromOutputDevice(rRenderContext, aNewViewInfos));

    // TODO Ghost it all if needed
    drawinglayer::primitive2d::Primitive2DContainer aGhostedSeq(1);
    double nFadeRate = double(m_nFadeRate) / 100.0;

    const basegfx::BColorModifierSharedPtr aBColorModifier(
        new basegfx::BColorModifier_interpolate(Color(COL_WHITE).getBColor(),
                                                1.0 - nFadeRate));

    aGhostedSeq[0] = drawinglayer::primitive2d::Primitive2DReference(
                        new drawinglayer::primitive2d::ModifiedColorPrimitive2D(aSeq, aBColorModifier));

    pProcessor->process(aGhostedSeq);
}

bool SwHeaderFooterWin::IsEmptyHeaderFooter( )
{
    bool bResult = true;

    // Actually check it
    const SwPageDesc* pDesc = GetPageFrame()->GetPageDesc();

    bool const bFirst(GetPageFrame()->OnFirstPage());
    const SwFrameFormat *const pFormat = (GetPageFrame()->OnRightPage())
        ? pDesc->GetRightFormat(bFirst)
        : pDesc->GetLeftFormat(bFirst);

    if ( pFormat )
    {
        if ( m_bIsHeader )
            bResult = !pFormat->GetHeader().IsActive();
        else
            bResult = !pFormat->GetFooter().IsActive();
    }

    return bResult;
}

void SwHeaderFooterWin::ExecuteCommand( sal_uInt16 nSlot )
{
    SwView& rView = GetEditWin()->GetView();
    SwWrtShell& rSh = rView.GetWrtShell();

    const OUString& rStyleName = GetPageFrame()->GetPageDesc()->GetName();
    switch ( nSlot )
    {
        case FN_HEADERFOOTER_EDIT:
        {
            OString sPageId = m_bIsHeader ? OString("header") : OString("footer");
            rView.GetDocShell()->FormatPage(rStyleName, sPageId, rSh);
        }
        break;
        case FN_HEADERFOOTER_BORDERBACK:
            {
                const SwPageDesc* pDesc = GetPageFrame()->GetPageDesc();
                const SwFrameFormat& rMaster = pDesc->GetMaster();
                SwFrameFormat* pHFFormat = const_cast< SwFrameFormat* >( rMaster.GetFooter().GetFooterFormat() );
                if ( m_bIsHeader )
                    pHFFormat = const_cast< SwFrameFormat* >( rMaster.GetHeader().GetHeaderFormat() );

                SfxItemPool* pPool = pHFFormat->GetAttrSet().GetPool();
                SfxItemSet aSet( *pPool,
                       RES_BACKGROUND, RES_BACKGROUND,
                       RES_BOX, RES_BOX,
                       SID_ATTR_BORDER_INNER, SID_ATTR_BORDER_INNER,
                       RES_SHADOW, RES_SHADOW, 0 );

            aSet.Put( pHFFormat->GetAttrSet() );

            // Create a box info item... needed by the dialog
            SvxBoxInfoItem aBoxInfo( SID_ATTR_BORDER_INNER );
            const SfxPoolItem *pBoxInfo;
            if ( SfxItemState::SET == pHFFormat->GetAttrSet().GetItemState( SID_ATTR_BORDER_INNER,
                                                    true, &pBoxInfo) )
                aBoxInfo = *static_cast<const SvxBoxInfoItem*>(pBoxInfo);

            aBoxInfo.SetTable( false );
            aBoxInfo.SetDist( true);
            aBoxInfo.SetMinDist( false );
            aBoxInfo.SetDefDist( MIN_BORDER_DIST );
            aBoxInfo.SetValid( SvxBoxInfoItemValidFlags::DISABLE );
            aSet.Put( aBoxInfo );

            if ( svx::ShowBorderBackgroundDlg( this, &aSet, true ) )
            {
                const SfxPoolItem* pItem;
                if ( SfxItemState::SET == aSet.GetItemState( RES_BACKGROUND, false, &pItem ) ) {
                    pHFFormat->SetFormatAttr( *pItem );
                    rView.GetDocShell()->SetModified();
                }

                if ( SfxItemState::SET == aSet.GetItemState( RES_BOX, false, &pItem ) ) {
                    pHFFormat->SetFormatAttr( *pItem );
                    rView.GetDocShell()->SetModified();
                }

                if ( SfxItemState::SET == aSet.GetItemState( RES_SHADOW, false, &pItem ) ) {
                    pHFFormat->SetFormatAttr( *pItem );
                    rView.GetDocShell()->SetModified();
                }
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
    ShowAll( !bReadonly );
}

void SwHeaderFooterWin::MouseButtonDown( const MouseEvent& rMEvt )
{
    if (IsEmptyHeaderFooter())
    {
        SwView& rView = GetEditWin()->GetView();
        SwWrtShell& rSh = rView.GetWrtShell();

        const OUString& rStyleName = GetPageFrame()->GetPageDesc()->GetName();
        rSh.ChangeHeaderOrFooter( rStyleName, m_bIsHeader, true, false );
    }
    else
        MenuButton::MouseButtonDown( rMEvt );
}

void SwHeaderFooterWin::Select()
{
    ExecuteCommand(GetCurItemId());
}

IMPL_LINK_NOARG_TYPED(SwHeaderFooterWin, FadeHandler, Timer *, void)
{
    if (m_bIsAppearing && m_nFadeRate > 0)
        m_nFadeRate -= 25;
    else if (!m_bIsAppearing && m_nFadeRate < 100)
        m_nFadeRate += 25;

    if (m_nFadeRate != 100 && !IsVisible())
    {
        Show();
        m_pLine->Show();
    }
    else if (m_nFadeRate == 100 && IsVisible())
    {
        Show(false);
        m_pLine->Show(false);
    }
    else
        Invalidate();

    if (IsVisible() && m_nFadeRate > 0 && m_nFadeRate < 100)
        m_aFadeTimer.Start();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
