/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <strings.hrc>

#include <doc.hxx>
#include <drawdoc.hxx>
#include <cmdid.h>
#include <DashedLine.hxx>
#include <docsh.hxx>
#include <edtwin.hxx>
#include <fmthdft.hxx>
#include <HeaderFooterWin.hxx>
#include <pagedesc.hxx>
#include <pagefrm.hxx>
#include <view.hxx>
#include <viewopt.hxx>
#include <wrtsh.hxx>
#include <IDocumentDrawModelAccess.hxx>

#include <basegfx/color/bcolortools.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/range/b2drectangle.hxx>
#include <basegfx/vector/b2dsize.hxx>
#include <drawinglayer/attribute/fillgradientattribute.hxx>
#include <drawinglayer/attribute/fontattribute.hxx>
#include <drawinglayer/primitive2d/fillgradientprimitive2d.hxx>
#include <drawinglayer/primitive2d/modifiedcolorprimitive2d.hxx>
#include <drawinglayer/primitive2d/PolygonHairlinePrimitive2D.hxx>
#include <drawinglayer/primitive2d/PolyPolygonColorPrimitive2D.hxx>
#include <drawinglayer/primitive2d/textlayoutdevice.hxx>
#include <drawinglayer/primitive2d/textprimitive2d.hxx>
#include <editeng/boxitem.hxx>
#include <svx/hdft.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/viewfrm.hxx>
#include <drawinglayer/processor2d/baseprocessor2d.hxx>
#include <drawinglayer/processor2d/processorfromoutputdevice.hxx>
#include <vcl/canvastools.hxx>
#include <vcl/metric.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <vcl/virdev.hxx>
#include <memory>

#define TEXT_PADDING 5
#define BOX_DISTANCE 10
#define BUTTON_WIDTH 18

using namespace basegfx;
using namespace basegfx::utils;
using namespace drawinglayer::attribute;

namespace
{
    basegfx::BColor lcl_GetFillColor(const basegfx::BColor& rLineColor)
    {
        basegfx::BColor aHslLine = basegfx::utils::rgb2hsl(rLineColor);
        double nLuminance = aHslLine.getZ() * 2.5;
        if ( nLuminance == 0 )
            nLuminance = 0.5;
        else if ( nLuminance >= 1.0 )
            nLuminance = aHslLine.getZ() * 0.4;
        aHslLine.setZ( nLuminance );
        return basegfx::utils::hsl2rgb( aHslLine );
    }

    basegfx::BColor lcl_GetLighterGradientColor(const basegfx::BColor& rDarkColor)
    {
        basegfx::BColor aHslDark = basegfx::utils::rgb2hsl(rDarkColor);
        double nLuminance = aHslDark.getZ() * 255 + 20;
        aHslDark.setZ( nLuminance / 255.0 );
        return basegfx::utils::hsl2rgb( aHslDark );
    }

    B2DPolygon lcl_GetPolygon( const ::tools::Rectangle& rRect, bool bOnTop )
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

        if ( !bOnTop )
        {
            B2DRectangle aBRect = vcl::unotools::b2DRectangleFromRectangle(rRect);
            B2DHomMatrix aRotation = createRotateAroundPoint(
                   aBRect.getCenterX(), aBRect.getCenterY(), M_PI );
            aPolygon.transform( aRotation );
        }

        return aPolygon;
    }
}

void SwFrameButtonPainter::PaintButton(drawinglayer::primitive2d::Primitive2DContainer& rSeq,
                                       const tools::Rectangle& rRect, bool bOnTop)
{
    rSeq.clear();
    B2DPolygon aPolygon = lcl_GetPolygon(rRect, bOnTop);

    // Colors
    basegfx::BColor aLineColor = SwViewOption::GetHeaderFooterMarkColor().getBColor();
    basegfx::BColor aFillColor = lcl_GetFillColor(aLineColor);
    basegfx::BColor aLighterColor = lcl_GetLighterGradientColor(aFillColor);

    const StyleSettings& rSettings = Application::GetSettings().GetStyleSettings();
    if (rSettings.GetHighContrastMode())
    {
        aFillColor = rSettings.GetDialogColor().getBColor();
        aLineColor = rSettings.GetDialogTextColor().getBColor();

        rSeq.push_back(drawinglayer::primitive2d::Primitive2DReference(
                            new drawinglayer::primitive2d::PolyPolygonColorPrimitive2D(B2DPolyPolygon(aPolygon), aFillColor)));
    }
    else
    {
        B2DRectangle aGradientRect = vcl::unotools::b2DRectangleFromRectangle(rRect);
        double nAngle = M_PI;
        if (bOnTop)
            nAngle = 0;
        FillGradientAttribute aFillAttrs(drawinglayer::attribute::GradientStyle::Linear, 0.0, 0.0, 0.0, nAngle, aLighterColor, aFillColor);
        rSeq.push_back(drawinglayer::primitive2d::Primitive2DReference(
                            new drawinglayer::primitive2d::FillGradientPrimitive2D(aGradientRect, aFillAttrs)));
    }

    // Create the border lines primitive
    rSeq.push_back(drawinglayer::primitive2d::Primitive2DReference(
                new drawinglayer::primitive2d::PolygonHairlinePrimitive2D(aPolygon, aLineColor)));
}

SwHeaderFooterWin::SwHeaderFooterWin( SwEditWin* pEditWin, const SwFrame *pFrame, bool bHeader ) :
    SwFrameMenuButtonBase(pEditWin, pFrame, "modules/swriter/ui/hfmenubutton.ui", "HFMenuButton"),
    m_xMenuButton(m_xBuilder->weld_menu_button("menubutton")),
    m_xPushButton(m_xBuilder->weld_button("button")),
    m_bIsHeader( bHeader ),
    m_pLine( nullptr ),
    m_bIsAppearing( false ),
    m_nFadeRate( 100 ),
    m_aFadeTimer("SwHeaderFooterWin m_aFadeTimer")
{
    m_xVirDev = m_xMenuButton->create_virtual_device();
    SetVirDevFont();

    m_xPushButton->connect_clicked(LINK(this, SwHeaderFooterWin, ClickHdl));
    m_xMenuButton->connect_selected(LINK(this, SwHeaderFooterWin, SelectHdl));

    // Create the line control
    m_pLine = VclPtr<SwDashedLine>::Create(GetEditWin(), &SwViewOption::GetHeaderFooterMarkColor);
    m_pLine->SetZOrder(this, ZOrderFlags::Before);

    // set the PopupMenu
    // Rewrite the menu entries' text
    if (m_bIsHeader)
    {
        m_xMenuButton->set_item_label("edit", SwResId(STR_FORMAT_HEADER));
        m_xMenuButton->set_item_label("delete", SwResId(STR_DELETE_HEADER));
    }
    else
    {
        m_xMenuButton->set_item_label("edit", SwResId(STR_FORMAT_FOOTER));
        m_xMenuButton->set_item_label("delete", SwResId(STR_DELETE_FOOTER));
    }

    m_aFadeTimer.SetTimeout(50);
    m_aFadeTimer.SetInvokeHandler(LINK(this, SwHeaderFooterWin, FadeHandler));
}

SwHeaderFooterWin::~SwHeaderFooterWin( )
{
    disposeOnce();
}

void SwHeaderFooterWin::dispose()
{
    m_pLine.disposeAndClear();
    m_xPushButton.reset();
    m_xMenuButton.reset();
    m_xVirDev.disposeAndClear();
    SwFrameMenuButtonBase::dispose();
}

void SwHeaderFooterWin::SetOffset(Point aOffset, tools::Long nXLineStart, tools::Long nXLineEnd)
{
    // Compute the text to show
    const SwPageDesc* pDesc = GetPageFrame()->GetPageDesc();
    bool bIsFirst = !pDesc->IsFirstShared() && GetPageFrame()->OnFirstPage();
    bool bIsLeft  = !pDesc->IsHeaderShared() && !GetPageFrame()->OnRightPage();
    bool bIsRight = !pDesc->IsHeaderShared() && GetPageFrame()->OnRightPage();
    m_sLabel = SwResId(STR_HEADER_TITLE);
    if (!m_bIsHeader)
        m_sLabel = bIsFirst ? SwResId(STR_FIRST_FOOTER_TITLE)
            : bIsLeft  ? SwResId(STR_LEFT_FOOTER_TITLE)
            : bIsRight ? SwResId(STR_RIGHT_FOOTER_TITLE)
            : SwResId(STR_FOOTER_TITLE );
    else
        m_sLabel = bIsFirst ? SwResId(STR_FIRST_HEADER_TITLE)
            : bIsLeft  ? SwResId(STR_LEFT_HEADER_TITLE)
            : bIsRight ? SwResId(STR_RIGHT_HEADER_TITLE)
            : SwResId(STR_HEADER_TITLE);

    sal_Int32 nPos = m_sLabel.lastIndexOf("%1");
    m_sLabel = m_sLabel.replaceAt(nPos, 2, pDesc->GetName());
    m_xMenuButton->set_accessible_name(m_sLabel);

    // Compute the text size and get the box position & size from it
    ::tools::Rectangle aTextRect;
    m_xVirDev->GetTextBoundRect(aTextRect, m_sLabel);
    ::tools::Rectangle aTextPxRect = m_xVirDev->LogicToPixel(aTextRect);
    FontMetric aFontMetric = m_xVirDev->GetFontMetric(m_xVirDev->GetFont());
    Size aBoxSize (aTextPxRect.GetWidth() + BUTTON_WIDTH + TEXT_PADDING * 2,
                   aFontMetric.GetLineHeight() + TEXT_PADDING  * 2 );

    tools::Long nYFooterOff = 0;
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

    m_xVirDev->SetOutputSizePixel(aBoxSize);
    PaintButton();

    double nYLinePos = aBoxPos.Y();
    if (!m_bIsHeader)
        nYLinePos += aBoxSize.Height();
    Point aLinePos(nXLineStart, nYLinePos);
    Size aLineSize(nXLineEnd - nXLineStart, 1);
    m_pLine->SetPosSizePixel(aLinePos, aLineSize);
}

void SwHeaderFooterWin::ShowAll(bool bShow)
{
    bool bIsEmptyHeaderFooter = IsEmptyHeaderFooter();
    m_xMenuButton->set_visible(!bIsEmptyHeaderFooter);
    m_xPushButton->set_visible(bIsEmptyHeaderFooter);

    m_bIsAppearing = bShow;

    if (m_aFadeTimer.IsActive())
        m_aFadeTimer.Stop();
    m_aFadeTimer.Start();
}

bool SwHeaderFooterWin::Contains( const Point &rDocPt ) const
{
    ::tools::Rectangle aRect(GetPosPixel(), GetSizePixel());
    if (aRect.Contains(rDocPt))
        return true;

    ::tools::Rectangle aLineRect(m_pLine->GetPosPixel(), m_pLine->GetSizePixel());
    return aLineRect.Contains(rDocPt);
}

void SwHeaderFooterWin::PaintButton()
{
    if (!m_xVirDev)
        return;

    // Use pixels for the rest of the drawing
    SetMapMode(MapMode(MapUnit::MapPixel));
    drawinglayer::primitive2d::Primitive2DContainer aSeq;
    const ::tools::Rectangle aRect(::tools::Rectangle(Point(0, 0), m_xVirDev->PixelToLogic(GetSizePixel())));

    SwFrameButtonPainter::PaintButton(aSeq, aRect, m_bIsHeader);

    // Create the text primitive
    basegfx::BColor aLineColor = SwViewOption::GetHeaderFooterMarkColor().getBColor();
    B2DVector aFontSize;
    FontAttribute aFontAttr = drawinglayer::primitive2d::getFontAttributeFromVclFont(aFontSize, m_xVirDev->GetFont(), false, false);

    FontMetric aFontMetric = m_xVirDev->GetFontMetric(m_xVirDev->GetFont());
    double nTextOffsetY = aFontMetric.GetAscent() + TEXT_PADDING;
    Point aTextPos(TEXT_PADDING, nTextOffsetY);

    basegfx::B2DHomMatrix aTextMatrix(createScaleTranslateB2DHomMatrix(
                                            aFontSize.getX(), aFontSize.getY(),
                                            double(aTextPos.X()), double(aTextPos.Y())));

    aSeq.push_back(drawinglayer::primitive2d::Primitive2DReference(
                    new drawinglayer::primitive2d::TextSimplePortionPrimitive2D(
                        aTextMatrix, m_sLabel, 0, m_sLabel.getLength(),
                        std::vector<double>(), aFontAttr, css::lang::Locale(), aLineColor)));

    // Create the 'plus' or 'arrow' primitive
    B2DRectangle aSignArea(B2DPoint(aRect.Right() - BUTTON_WIDTH, 0.0),
                           B2DSize(aRect.Right(), aRect.getHeight()));

    B2DPolygon aSign;
    bool bIsEmptyHeaderFooter = IsEmptyHeaderFooter();
    if (bIsEmptyHeaderFooter)
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

    BColor aSignColor = COL_BLACK.getBColor();
    if (Application::GetSettings().GetStyleSettings().GetHighContrastMode())
        aSignColor = COL_WHITE.getBColor();

    aSeq.push_back( drawinglayer::primitive2d::Primitive2DReference(
                                    new drawinglayer::primitive2d::PolyPolygonColorPrimitive2D(
                                        B2DPolyPolygon(aSign), aSignColor)) );

    // Create the processor and process the primitives
    const drawinglayer::geometry::ViewInformation2D aNewViewInfos;
    std::unique_ptr<drawinglayer::processor2d::BaseProcessor2D> pProcessor(
        drawinglayer::processor2d::createBaseProcessor2DFromOutputDevice(*m_xVirDev, aNewViewInfos));

    // TODO Ghost it all if needed
    drawinglayer::primitive2d::Primitive2DContainer aGhostedSeq(1);
    double nFadeRate = double(m_nFadeRate) / 100.0;

    const basegfx::BColorModifierSharedPtr aBColorModifier =
        std::make_shared<basegfx::BColorModifier_interpolate>(COL_WHITE.getBColor(),
                                                1.0 - nFadeRate);

    aGhostedSeq[0] = drawinglayer::primitive2d::Primitive2DReference(
                        new drawinglayer::primitive2d::ModifiedColorPrimitive2D(std::move(aSeq), aBColorModifier));

    pProcessor->process(aGhostedSeq);

    if (bIsEmptyHeaderFooter)
        m_xPushButton->set_custom_button(m_xVirDev.get());
    else
        m_xMenuButton->set_custom_button(m_xVirDev.get());
}

bool SwHeaderFooterWin::IsEmptyHeaderFooter( ) const
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

void SwHeaderFooterWin::ExecuteCommand(std::string_view rIdent)
{
    SwView& rView = GetEditWin()->GetView();
    SwWrtShell& rSh = rView.GetWrtShell();

    const OUString& rStyleName = GetPageFrame()->GetPageDesc()->GetName();
    if (rIdent == "edit")
    {
        OString sPageId = m_bIsHeader ? OString("header") : OString("footer");
        rView.GetDocShell()->FormatPage(rView.GetFrameWeld(), rStyleName, sPageId, rSh);
    }
    else if (rIdent == "borderback")
    {
        const SwPageDesc* pDesc = GetPageFrame()->GetPageDesc();
        const SwFrameFormat& rMaster = pDesc->GetMaster();
        SwFrameFormat* pHFFormat = const_cast< SwFrameFormat* >( rMaster.GetFooter().GetFooterFormat() );
        if ( m_bIsHeader )
            pHFFormat = const_cast< SwFrameFormat* >( rMaster.GetHeader().GetHeaderFormat() );
        SfxItemSet aSet( pHFFormat->GetAttrSet() );

        // Items to hand over XPropertyList things like XColorList,
        // XHatchList, XGradientList, and XBitmapList to the Area TabPage:
        aSet.MergeRange( SID_COLOR_TABLE, SID_PATTERN_LIST );
        // create needed items for XPropertyList entries from the DrawModel so that
        // the Area TabPage can access them
        rSh.GetDoc()->getIDocumentDrawModelAccess().GetDrawModel()->PutAreaListItems( aSet );

        aSet.MergeRange(SID_ATTR_BORDER_INNER, SID_ATTR_BORDER_INNER);
        // Create a box info item... needed by the dialog
        std::shared_ptr<SvxBoxInfoItem> aBoxInfo(std::make_shared<SvxBoxInfoItem>(SID_ATTR_BORDER_INNER));
        const SfxPoolItem *pBoxInfo;
        if (SfxItemState::SET == pHFFormat->GetAttrSet().GetItemState(SID_ATTR_BORDER_INNER, true, &pBoxInfo))
            aBoxInfo.reset(static_cast<SvxBoxInfoItem*>(pBoxInfo->Clone()));

        aBoxInfo->SetTable(false);
        aBoxInfo->SetDist(true);
        aBoxInfo->SetMinDist(false);
        aBoxInfo->SetDefDist(MIN_BORDER_DIST);
        aBoxInfo->SetValid(SvxBoxInfoItemValidFlags::DISABLE);
        aSet.Put(*aBoxInfo);

        if (svx::ShowBorderBackgroundDlg( GetFrameWeld(), &aSet ) )
        {
            pHFFormat->SetFormatAttr( aSet );
            rView.GetDocShell()->SetModified();
        }
    }
    else if (rIdent == "delete")
    {
        rSh.ChangeHeaderOrFooter( rStyleName, m_bIsHeader, false, true );
        // warning: "this" may be disposed now
        rSh.GetWin()->GrabFocusToDocument();
    }
    else if (rIdent == "insert_pagenumber")
    {
        SfxViewFrame* pVFrame = rSh.GetView().GetViewFrame();
        pVFrame->GetBindings().Execute(FN_INSERT_FLD_PGNUMBER);
    }
    else if (rIdent == "insert_pagecount")
    {
        SfxViewFrame* pVFrame = rSh.GetView().GetViewFrame();
        pVFrame->GetBindings().Execute(FN_INSERT_FLD_PGCOUNT);
    }
}

void SwHeaderFooterWin::SetReadonly( bool bReadonly )
{
    ShowAll( !bReadonly );
}

IMPL_LINK_NOARG(SwHeaderFooterWin, ClickHdl, weld::Button&, void)
{
    SwView& rView = GetEditWin()->GetView();
    SwWrtShell& rSh = rView.GetWrtShell();

    const OUString& rStyleName = GetPageFrame()->GetPageDesc()->GetName();
    rSh.ChangeHeaderOrFooter( rStyleName, m_bIsHeader, true, false );

    m_xPushButton->hide();
    m_xMenuButton->show();
    PaintButton();
}

IMPL_LINK(SwHeaderFooterWin, SelectHdl, const OString&, rIdent, void)
{
    ExecuteCommand(rIdent);
}

IMPL_LINK_NOARG(SwHeaderFooterWin, FadeHandler, Timer *, void)
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
        PaintButton();

    if (IsVisible() && m_nFadeRate > 0 && m_nFadeRate < 100)
        m_aFadeTimer.Start();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
