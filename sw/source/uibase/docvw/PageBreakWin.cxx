/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <globals.hrc>
#include <bitmaps.hlst>

#include <cmdid.h>
#include <cntfrm.hxx>
#include <txtfrm.hxx>
#include <notxtfrm.hxx>
#include <DashedLine.hxx>
#include <doc.hxx>
#include <edtwin.hxx>
#include <fmtpdsc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <IDocumentContentOperations.hxx>
#include <PageBreakWin.hxx>
#include <pagefrm.hxx>
#include <PostItMgr.hxx>
#include <FrameControlsManager.hxx>
#include <uiitems.hxx>
#include <view.hxx>
#include <viewopt.hxx>
#include <wrtsh.hxx>

#include <basegfx/color/bcolortools.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/range/b2drectangle.hxx>
#include <drawinglayer/primitive2d/discretebitmapprimitive2d.hxx>
#include <drawinglayer/primitive2d/modifiedcolorprimitive2d.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#include <drawinglayer/processor2d/baseprocessor2d.hxx>
#include <drawinglayer/processor2d/processorfromoutputdevice.hxx>
#include <editeng/formatbreakitem.hxx>
#include <sfx2/dispatch.hxx>
#include <svl/stritem.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <memory>

#define BUTTON_WIDTH 30
#define BUTTON_HEIGHT 19
#define ARROW_WIDTH 9

using namespace basegfx;
using namespace basegfx::utils;

namespace
{
    class SwBreakDashedLine : public SwDashedLine
    {
        private:
            VclPtr<SwPageBreakWin> m_pWin;

        public:
            SwBreakDashedLine( vcl::Window* pParent, Color& ( *pColorFn )(), SwPageBreakWin* pWin ) :
                SwDashedLine( pParent, pColorFn ),
                m_pWin( pWin ) {};
            virtual ~SwBreakDashedLine() override { disposeOnce(); }
            virtual void dispose() override { m_pWin.clear(); SwDashedLine::dispose(); }

            virtual void MouseMove( const MouseEvent& rMEvt ) override;
    };

    void SwBreakDashedLine::MouseMove( const MouseEvent& rMEvt )
    {
        if ( rMEvt.IsLeaveWindow() )
        {
            // don't fade if we just move to the 'button'
            Point aEventPos( GetPosPixel() + rMEvt.GetPosPixel() );
            if ( !m_pWin->Contains( aEventPos ) || !m_pWin->IsVisible() )
                m_pWin->Fade( false );
        }
        else if ( !m_pWin->IsVisible() )
        {
            m_pWin->Fade( true );
        }

        if ( !rMEvt.IsSynthetic() && !m_pWin->IsVisible() )
        {
            m_pWin->UpdatePosition( rMEvt.GetPosPixel() );
        }
    }
}

SwPageBreakWin::SwPageBreakWin( SwEditWin* pEditWin, const SwFrame *pFrame ) :
    SwFrameMenuButtonBase( pEditWin, pFrame ),
    m_aBuilder(nullptr, VclBuilderContainer::getUIRootDir(), "modules/swriter/ui/pagebreakmenu.ui", ""),
    m_pPopupMenu(m_aBuilder.get_menu("menu")),
    m_pLine( nullptr ),
    m_bIsAppearing( false ),
    m_nFadeRate( 100 ),
    m_nDelayAppearing( 0 ),
    m_bDestroyed( false )
{
    // Use pixels for the rest of the drawing
    SetMapMode( MapMode ( MapUnit::MapPixel ) );

    // Create the line control
    m_pLine = VclPtr<SwBreakDashedLine>::Create( GetEditWin(), &SwViewOption::GetPageBreakColor, this );

    // Set the popup menu
    m_pPopupMenu->SetDeactivateHdl( LINK( this, SwPageBreakWin, HideHandler ) );
    SetPopupMenu(m_pPopupMenu);

    m_aFadeTimer.SetTimeout( 50 );
    m_aFadeTimer.SetInvokeHandler( LINK( this, SwPageBreakWin, FadeHandler ) );
}

SwPageBreakWin::~SwPageBreakWin( )
{
    disposeOnce();
}

void SwPageBreakWin::dispose()
{
    m_bDestroyed = true;
    m_aFadeTimer.Stop();

    m_pLine.disposeAndClear();
    m_pPopupMenu.clear();
    m_aBuilder.disposeBuilder();

    SwFrameMenuButtonBase::dispose();
}

void SwPageBreakWin::Paint(vcl::RenderContext& rRenderContext, const ::tools::Rectangle&)
{
    const ::tools::Rectangle aRect(::tools::Rectangle(Point(0, 0), rRenderContext.PixelToLogic(GetSizePixel())));

    // Properly paint the control
    BColor aColor = SwViewOption::GetPageBreakColor().getBColor();

    BColor aHslLine = rgb2hsl(aColor);
    double nLuminance = aHslLine.getZ();
    nLuminance += (1.0 - nLuminance) * 0.75;
    if ( aHslLine.getZ() > 0.7 )
        nLuminance = aHslLine.getZ() * 0.7;
    aHslLine.setZ(nLuminance);
    BColor aOtherColor = hsl2rgb(aHslLine);

    const StyleSettings& rSettings = Application::GetSettings().GetStyleSettings();
    if (rSettings.GetHighContrastMode())
    {
        aColor = rSettings.GetDialogTextColor().getBColor();
        aOtherColor = rSettings.GetDialogColor().getBColor();
    }

    bool bRtl = AllSettings::GetLayoutRTL();

    drawinglayer::primitive2d::Primitive2DContainer aSeq(3);
    B2DRectangle aBRect(double(aRect.Left()), double(aRect.Top()),
                        double(aRect.Right()), double(aRect.Bottom()));
    B2DPolygon aPolygon = createPolygonFromRect(aBRect, 3.0 / BUTTON_WIDTH, 3.0 / BUTTON_HEIGHT);

    // Create the polygon primitives
    aSeq[0].set(new drawinglayer::primitive2d::PolyPolygonColorPrimitive2D(
                                        B2DPolyPolygon(aPolygon), aOtherColor));
    aSeq[1].set(new drawinglayer::primitive2d::PolygonHairlinePrimitive2D(
                                        aPolygon, aColor));

    // Create the primitive for the image
    BitmapEx aBmpEx(RID_BMP_PAGE_BREAK);
    double nImgOfstX = 3.0;
    if (bRtl)
        nImgOfstX = aRect.Right() - aBmpEx.GetSizePixel().Width() - 3.0;
    aSeq[2].set(new drawinglayer::primitive2d::DiscreteBitmapPrimitive2D(
                                        aBmpEx, B2DPoint(nImgOfstX, 1.0)));

    double nTop = double(aRect.getHeight()) / 2.0;
    double nBottom = nTop + 4.0;
    double nLeft = aRect.getWidth() - ARROW_WIDTH - 6.0;
    if (bRtl)
        nLeft = ARROW_WIDTH - 2.0;
    double nRight = nLeft + 8.0;

    B2DPolygon aTriangle;
    aTriangle.append(B2DPoint(nLeft, nTop));
    aTriangle.append(B2DPoint(nRight, nTop));
    aTriangle.append(B2DPoint((nLeft + nRight) / 2.0, nBottom));
    aTriangle.setClosed(true);

    BColor aTriangleColor = COL_BLACK.getBColor();
    if (Application::GetSettings().GetStyleSettings().GetHighContrastMode())
        aTriangleColor = COL_WHITE.getBColor();

    aSeq.emplace_back();
    aSeq.back().set( new drawinglayer::primitive2d::PolyPolygonColorPrimitive2D(
                                        B2DPolyPolygon(aTriangle), aTriangleColor));

    drawinglayer::primitive2d::Primitive2DContainer aGhostedSeq(1);
    double nFadeRate = double(m_nFadeRate) / 100.0;
    const basegfx::BColorModifierSharedPtr aBColorModifier(
                new basegfx::BColorModifier_interpolate(COL_WHITE.getBColor(),
                                                        1.0 - nFadeRate));
    aGhostedSeq[0].set( new drawinglayer::primitive2d::ModifiedColorPrimitive2D(
                            aSeq, aBColorModifier));

    // Create the processor and process the primitives
    const drawinglayer::geometry::ViewInformation2D aNewViewInfos;
    std::unique_ptr<drawinglayer::processor2d::BaseProcessor2D> pProcessor(
        drawinglayer::processor2d::createBaseProcessor2DFromOutputDevice(rRenderContext, aNewViewInfos));

    pProcessor->process(aGhostedSeq);
}

void SwPageBreakWin::Select()
{
    SwFrameControlPtr pThis = GetEditWin()->GetFrameControlsManager( ).GetControl( PageBreak, GetFrame() );

    OString sIdent = GetCurItemIdent();
    if (sIdent == "edit")
    {
        const SwLayoutFrame* pBodyFrame = static_cast< const SwLayoutFrame* >( GetPageFrame()->Lower() );
        while ( pBodyFrame && !pBodyFrame->IsBodyFrame() )
            pBodyFrame = static_cast< const SwLayoutFrame* >( pBodyFrame->GetNext() );

        SwEditWin* pEditWin = GetEditWin();

        if ( pBodyFrame )
        {
            SwWrtShell& rSh = pEditWin->GetView().GetWrtShell();
            bool bOldLock = rSh.IsViewLocked();
            rSh.LockView( true );

            if ( pBodyFrame->Lower()->IsTabFrame() )
            {
                rSh.Push( );
                rSh.ClearMark();

                SwContentFrame *pCnt = const_cast< SwContentFrame* >( pBodyFrame->ContainsContent() );
                SwContentNode* pNd = pCnt->IsTextFrame()
                    ? static_cast<SwTextFrame*>(pCnt)->GetTextNodeFirst()
                    : static_cast<SwNoTextFrame*>(pCnt)->GetNode();
                rSh.SetSelection( *pNd );

                SfxStringItem aItem(pEditWin->GetView().GetPool().GetWhich(FN_FORMAT_TABLE_DLG), "textflow");
                pEditWin->GetView().GetViewFrame()->GetDispatcher()->ExecuteList(
                        FN_FORMAT_TABLE_DLG,
                        SfxCallMode::SYNCHRON | SfxCallMode::RECORD,
                        { &aItem });

                rSh.Pop(SwCursorShell::PopMode::DeleteCurrent);
            }
            else
            {
                SwContentFrame *pCnt = const_cast< SwContentFrame* >( pBodyFrame->ContainsContent() );
                SwContentNode* pNd = pCnt->IsTextFrame()
                    ? static_cast<SwTextFrame*>(pCnt)->GetTextNodeFirst()
                    : static_cast<SwNoTextFrame*>(pCnt)->GetNode();

                SwPaM aPaM( *pNd );
                SwPaMItem aPaMItem( pEditWin->GetView().GetPool( ).GetWhich( FN_PARAM_PAM ), &aPaM );
                SfxStringItem aItem( pEditWin->GetView().GetPool( ).GetWhich( SID_PARA_DLG ), "textflow" );
                pEditWin->GetView().GetViewFrame()->GetDispatcher()->ExecuteList(
                        SID_PARA_DLG,
                        SfxCallMode::SYNCHRON | SfxCallMode::RECORD,
                        { &aItem, &aPaMItem });
            }
            rSh.LockView( bOldLock );
            pEditWin->GrabFocus( );
        }
    }
    else if (sIdent == "delete")
    {
        const SwLayoutFrame* pBodyFrame = static_cast< const SwLayoutFrame* >( GetPageFrame()->Lower() );
        while ( pBodyFrame && !pBodyFrame->IsBodyFrame() )
            pBodyFrame = static_cast< const SwLayoutFrame* >( pBodyFrame->GetNext() );

        if ( pBodyFrame )
        {

            SwContentFrame *pCnt = const_cast< SwContentFrame* >( pBodyFrame->ContainsContent() );
            SwContentNode* pNd = pCnt->IsTextFrame()
                ? static_cast<SwTextFrame*>(pCnt)->GetTextNodeFirst()
                : static_cast<SwNoTextFrame*>(pCnt)->GetNode();

            pNd->GetDoc()->GetIDocumentUndoRedo( ).StartUndo( SwUndoId::UI_DELETE_PAGE_BREAK, nullptr );

            SfxItemSet aSet(
                GetEditWin()->GetView().GetWrtShell().GetAttrPool(),
                svl::Items<RES_PAGEDESC, RES_BREAK>{});
            aSet.Put( SvxFormatBreakItem( SvxBreak::NONE, RES_BREAK ) );
            aSet.Put( SwFormatPageDesc( nullptr ) );

            SwPaM aPaM( *pNd );
            pNd->GetDoc()->getIDocumentContentOperations().InsertItemSet(
                aPaM, aSet, SetAttrMode::DEFAULT, GetPageFrame()->getRootFrame());

            pNd->GetDoc()->GetIDocumentUndoRedo( ).EndUndo( SwUndoId::UI_DELETE_PAGE_BREAK, nullptr );
        }
    }

    // Only fade if there is more than this temporary shared pointer:
    // The main reference has been deleted due to a page break removal
    if ( pThis.use_count() > 1 )
        Fade( false );
}

void SwPageBreakWin::MouseMove( const MouseEvent& rMEvt )
{
    if ( rMEvt.IsLeaveWindow() )
    {
        // don't fade if we just move to the 'line', or the popup menu is open
        Point aEventPos( rMEvt.GetPosPixel() + rMEvt.GetPosPixel() );
        if ( !Contains( aEventPos ) && !PopupMenu::IsInExecute() )
            Fade( false );
    }
    else if ( !IsVisible() )
        Fade( true );
}

void SwPageBreakWin::Activate( )
{
    Fade( true );
    MenuButton::Activate();
}

void SwPageBreakWin::UpdatePosition(const boost::optional<Point>& xEvtPt)
{
    if ( xEvtPt )
    {
        if ( xEvtPt == m_xMousePt )
            return;
        m_xMousePt = xEvtPt;
    }

    const SwPageFrame* pPageFrame = GetPageFrame();
    const SwFrame* pPrevPage = pPageFrame;
    do
    {
        pPrevPage = pPrevPage->GetPrev();
    }
    while ( pPrevPage && ( ( pPrevPage->getFrameArea().Top( ) == pPageFrame->getFrameArea().Top( ) )
                || static_cast< const SwPageFrame* >( pPrevPage )->IsEmptyPage( ) ) );

    ::tools::Rectangle aBoundRect = GetEditWin()->LogicToPixel( pPageFrame->GetBoundRect(GetEditWin()).SVRect() );
    ::tools::Rectangle aFrameRect = GetEditWin()->LogicToPixel( pPageFrame->getFrameArea().SVRect() );

    long nYLineOffset = ( aBoundRect.Top() + aFrameRect.Top() ) / 2;
    if ( pPrevPage )
    {
        ::tools::Rectangle aPrevFrameRect = GetEditWin()->LogicToPixel( pPrevPage->getFrameArea().SVRect() );
        nYLineOffset = ( aPrevFrameRect.Bottom() + aFrameRect.Top() ) / 2;
    }

    // Get the page + sidebar coords
    long nPgLeft = aFrameRect.Left();
    long nPgRight = aFrameRect.Right();

    unsigned long nSidebarWidth = 0;
    const SwPostItMgr* pPostItMngr = GetEditWin()->GetView().GetWrtShell().GetPostItMgr();
    if ( pPostItMngr && pPostItMngr->HasNotes() && pPostItMngr->ShowNotes() )
        nSidebarWidth = pPostItMngr->GetSidebarBorderWidth( true ) + pPostItMngr->GetSidebarWidth( true );

    if ( pPageFrame->SidebarPosition( ) == sw::sidebarwindows::SidebarPosition::LEFT )
        nPgLeft -= nSidebarWidth;
    else if ( pPageFrame->SidebarPosition( ) == sw::sidebarwindows::SidebarPosition::RIGHT )
        nPgRight += nSidebarWidth;

    Size aBtnSize( BUTTON_WIDTH + ARROW_WIDTH, BUTTON_HEIGHT );

    // Place the button on the left or right?
    ::tools::Rectangle aVisArea = GetEditWin()->LogicToPixel( GetEditWin()->GetView().GetVisArea() );

    long nLineLeft = std::max( nPgLeft, aVisArea.Left() );
    long nLineRight = std::min( nPgRight, aVisArea.Right() );
    long nBtnLeft = nLineLeft;

    if ( m_xMousePt )
    {
        nBtnLeft = nLineLeft + m_xMousePt->X() - aBtnSize.getWidth() / 2;

        if ( nBtnLeft < nLineLeft )
            nBtnLeft = nLineLeft;
        else if ( ( nBtnLeft + aBtnSize.getWidth() ) > nLineRight )
            nBtnLeft = nLineRight - aBtnSize.getWidth();
    }

    // Set the button position
    Point aBtnPos( nBtnLeft, nYLineOffset - BUTTON_HEIGHT / 2 );
    SetPosSizePixel( aBtnPos, aBtnSize );

    // Set the line position
    Point aLinePos( nLineLeft, nYLineOffset - 5 );
    Size aLineSize( nLineRight - nLineLeft, 10 );
    m_pLine->SetPosSizePixel( aLinePos, aLineSize );
}

void SwPageBreakWin::ShowAll( bool bShow )
{
    m_pLine->Show( bShow );
}

bool SwPageBreakWin::Contains( const Point &rDocPt ) const
{
    ::tools::Rectangle aRect( GetPosPixel(), GetSizePixel() );
    if ( aRect.IsInside( rDocPt ) )
        return true;

    ::tools::Rectangle aLineRect( m_pLine->GetPosPixel(), m_pLine->GetSizePixel() );
    return aLineRect.IsInside( rDocPt );
}

void SwPageBreakWin::SetReadonly( bool bReadonly )
{
    ShowAll( !bReadonly );
}

void SwPageBreakWin::Fade( bool bFadeIn )
{
    m_bIsAppearing = bFadeIn;
    if ( bFadeIn )
        m_nDelayAppearing = 0;

    if ( !m_bDestroyed && m_aFadeTimer.IsActive( ) )
        m_aFadeTimer.Stop();
    if ( !m_bDestroyed )
        m_aFadeTimer.Start( );
}

IMPL_LINK_NOARG(SwPageBreakWin, HideHandler, Menu *, bool)
{
    Fade( false );

    return false;
}

IMPL_LINK_NOARG(SwPageBreakWin, FadeHandler, Timer *, void)
{
    const int TICKS_BEFORE_WE_APPEAR = 10;
    if ( m_bIsAppearing && m_nDelayAppearing < TICKS_BEFORE_WE_APPEAR )
    {
        ++m_nDelayAppearing;
        m_aFadeTimer.Start();
        return;
    }

    if ( m_bIsAppearing && m_nFadeRate > 0 )
        m_nFadeRate -= 25;
    else if ( !m_bIsAppearing && m_nFadeRate < 100 )
        m_nFadeRate += 25;

    if ( m_nFadeRate != 100 && !IsVisible() )
        Show();
    else if ( m_nFadeRate == 100 && IsVisible( ) )
        Hide();
    else
    {
        UpdatePosition();
        Invalidate();
    }

    if (IsVisible( ) && m_nFadeRate > 0 && m_nFadeRate < 100)
        m_aFadeTimer.Start();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
