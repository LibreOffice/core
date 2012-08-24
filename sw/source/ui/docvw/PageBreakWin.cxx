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
#include <globals.hrc>
#include <popup.hrc>
#include <utlui.hrc>

#include <cmdid.h>
#include <cntfrm.hxx>
#include <DashedLine.hxx>
#include <doc.hxx>
#include <edtwin.hxx>
#include <fmtpdsc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <PageBreakWin.hxx>
#include <pagefrm.hxx>
#include <PostItMgr.hxx>
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
#include <editeng/brkitem.hxx>
#include <sfx2/dispatch.hxx>
#include <drawinglayer/processor2d/processorfromoutputdevice.hxx>
#include <vcl/svapp.hxx>

#define BUTTON_WIDTH 30
#define BUTTON_HEIGHT 19
#define ARROW_WIDTH 9

using namespace basegfx;
using namespace basegfx::tools;
using namespace drawinglayer::primitive2d;

namespace
{
    class SwBreakDashedLine : public SwDashedLine
    {
        private:
            SwPageBreakWin* m_pWin;

        public:
            SwBreakDashedLine( Window* pParent, Color& ( *pColorFn )(), SwPageBreakWin* pWin ) :
                SwDashedLine( pParent, pColorFn ),
                m_pWin( pWin ) {};

            virtual void MouseMove( const MouseEvent& rMEvt );
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
            Point* pPtr = new Point( rMEvt.GetPosPixel() );
            m_pWin->UpdatePosition( pPtr );
        }
    }
}

SwPageBreakWin::SwPageBreakWin( SwEditWin* pEditWin, const SwPageFrm* pPageFrm ) :
    MenuButton( pEditWin, WB_DIALOGCONTROL ),
    SwFrameControl( pEditWin, pPageFrm ),
    m_pPopupMenu( NULL ),
    m_pLine( NULL ),
    m_bIsAppearing( false ),
    m_nFadeRate( 100 ),
    m_nDelayAppearing( 0 ),
    m_bDestroyed( false ),
    m_pMousePt( NULL )
{
    // Use pixels for the rest of the drawing
    SetMapMode( MapMode ( MAP_PIXEL ) );

    // Create the line control
    m_pLine = new SwBreakDashedLine( GetEditWin(), &SwViewOption::GetPageBreakColor, this );

    // Create the popup menu
    m_pPopupMenu = new PopupMenu( SW_RES( MN_PAGEBREAK_BUTTON ) );
    m_pPopupMenu->SetDeactivateHdl( LINK( this, SwPageBreakWin, HideHandler ) );
    SetPopupMenu( m_pPopupMenu );

    m_aFadeTimer.SetTimeout( 50 );
    m_aFadeTimer.SetTimeoutHdl( LINK( this, SwPageBreakWin, FadeHandler ) );
}

SwPageBreakWin::~SwPageBreakWin( )
{
    m_bDestroyed = true;
    m_aFadeTimer.Stop();

    delete m_pPopupMenu;
    delete m_pLine;
    delete m_pMousePt;
}

void SwPageBreakWin::Paint( const Rectangle& )
{
    const Rectangle aRect( Rectangle( Point( 0, 0 ), PixelToLogic( GetSizePixel() ) ) );

    // Properly paint the control
    BColor aColor = SwViewOption::GetPageBreakColor().getBColor();

    BColor aHslLine = rgb2hsl( aColor );
    double nLuminance = aHslLine.getZ();
    nLuminance += ( 1.0 - nLuminance ) * 0.75;
    if ( aHslLine.getZ() > 0.7 )
        nLuminance = aHslLine.getZ() * 0.7;
    aHslLine.setZ( nLuminance );
    BColor aOtherColor = hsl2rgb( aHslLine );

    const StyleSettings& rSettings = Application::GetSettings().GetStyleSettings();
    if ( rSettings.GetHighContrastMode( ) )
    {
        aColor = rSettings.GetDialogTextColor().getBColor();
        aOtherColor = rSettings.GetDialogColor( ).getBColor();
    }

    bool bRtl = Application::GetSettings().GetLayoutRTL();

    Primitive2DSequence aSeq( 3 );
    B2DRectangle aBRect( double( aRect.Left() ), double( aRect.Top( ) ),
           double( aRect.Right() ), double( aRect.Bottom( ) ) );
    B2DPolygon aPolygon = createPolygonFromRect( aBRect, 3.0 / BUTTON_WIDTH, 3.0 / BUTTON_HEIGHT );

    // Create the polygon primitives
    aSeq[0] = Primitive2DReference( new PolyPolygonColorPrimitive2D(
            B2DPolyPolygon( aPolygon ), aOtherColor ) );
    aSeq[1] = Primitive2DReference( new PolygonHairlinePrimitive2D(
            aPolygon, aColor ) );

    // Create the primitive for the image
    Image aImg( SW_RES( IMG_PAGE_BREAK ) );
    double nImgOfstX = 3.0;
    if ( bRtl )
        nImgOfstX = aRect.Right() - aImg.GetSizePixel().Width() - 3.0;
    aSeq[2] = Primitive2DReference( new DiscreteBitmapPrimitive2D(
            aImg.GetBitmapEx(), B2DPoint( nImgOfstX, 1.0 ) ) );

    double nTop = double( aRect.getHeight() ) / 2.0;
    double nBottom = nTop + 4.0;
    double nLeft = aRect.getWidth( ) - ARROW_WIDTH - 6.0;
    if ( bRtl )
        nLeft = ARROW_WIDTH - 2.0;
    double nRight = nLeft + 8.0;

    B2DPolygon aTriangle;
    aTriangle.append( B2DPoint( nLeft, nTop ) );
    aTriangle.append( B2DPoint( nRight, nTop ) );
    aTriangle.append( B2DPoint( ( nLeft + nRight ) / 2.0, nBottom ) );
    aTriangle.setClosed( true );

    BColor aTriangleColor = Color( COL_BLACK ).getBColor( );
    if ( Application::GetSettings().GetStyleSettings().GetHighContrastMode() )
        aTriangleColor = Color( COL_WHITE ).getBColor( );

    aSeq.realloc( aSeq.getLength() + 1 );
    aSeq[ aSeq.getLength() - 1 ] = Primitive2DReference( new PolyPolygonColorPrimitive2D(
               B2DPolyPolygon( aTriangle ), aTriangleColor ) );

    Primitive2DSequence aGhostedSeq( 1 );
    double nFadeRate = double( m_nFadeRate ) / 100.0;
    aGhostedSeq[0] = Primitive2DReference( new ModifiedColorPrimitive2D(
                aSeq, BColorModifier( Color( COL_WHITE ).getBColor(), 1.0 - nFadeRate, BCOLORMODIFYMODE_INTERPOLATE ) ) );

    // Create the processor and process the primitives
    const drawinglayer::geometry::ViewInformation2D aNewViewInfos;
    drawinglayer::processor2d::BaseProcessor2D * pProcessor =
        drawinglayer::processor2d::createBaseProcessor2DFromOutputDevice(
                    *this, aNewViewInfos );

    pProcessor->process( aGhostedSeq );
    delete pProcessor;
}

void SwPageBreakWin::Select( )
{
    SwFrameControlPtr pThis = GetEditWin()->GetFrameControlsManager( ).GetControl( PageBreak, GetFrame() );

    switch( GetCurItemId( ) )
    {
        case FN_PAGEBREAK_EDIT:
            {
                const SwLayoutFrm* pBodyFrm = static_cast< const SwLayoutFrm* >( GetPageFrame()->Lower() );
                while ( pBodyFrm && !pBodyFrm->IsBodyFrm() )
                    pBodyFrm = static_cast< const SwLayoutFrm* >( pBodyFrm->GetNext() );

                SwEditWin* pEditWin = GetEditWin();

                if ( pBodyFrm )
                {
                    SwWrtShell& rSh = pEditWin->GetView().GetWrtShell();
                    sal_Bool bOldLock = rSh.IsViewLocked();
                    rSh.LockView( sal_True );

                    if ( pBodyFrm->Lower()->IsTabFrm() )
                    {
                        rSh.Push( );
                        rSh.ClearMark();

                        SwCntntFrm *pCnt = const_cast< SwCntntFrm* >( pBodyFrm->ContainsCntnt() );
                        SwCntntNode* pNd = pCnt->GetNode();
                        rSh.SetSelection( *pNd );

                        SfxUInt16Item aItem( pEditWin->GetView().GetPool( ).GetWhich( FN_FORMAT_TABLE_DLG ), TP_TABLE_TEXTFLOW );
                        pEditWin->GetView().GetViewFrame()->GetDispatcher()->Execute(
                                FN_FORMAT_TABLE_DLG, SFX_CALLMODE_SYNCHRON|SFX_CALLMODE_RECORD, &aItem, NULL );

                        rSh.Pop( sal_False );
                    }
                    else
                    {
                        SwCntntFrm *pCnt = const_cast< SwCntntFrm* >( pBodyFrm->ContainsCntnt() );
                        SwCntntNode* pNd = pCnt->GetNode();

                        SwPaM aPaM( *pNd );
                        SwPaMItem aPaMItem( pEditWin->GetView().GetPool( ).GetWhich( FN_PARAM_PAM ), &aPaM );
                        SfxUInt16Item aItem( pEditWin->GetView().GetPool( ).GetWhich( SID_PARA_DLG ), TP_PARA_EXT );
                        pEditWin->GetView().GetViewFrame()->GetDispatcher()->Execute(
                                SID_PARA_DLG, SFX_CALLMODE_SYNCHRON|SFX_CALLMODE_RECORD, &aItem, &aPaMItem, NULL );
                    }
                    rSh.LockView( bOldLock );
                    pEditWin->GrabFocus( );
                }
            }
            break;
        case FN_PAGEBREAK_DELETE:
            {
                const SwLayoutFrm* pBodyFrm = static_cast< const SwLayoutFrm* >( GetPageFrame()->Lower() );
                while ( pBodyFrm && !pBodyFrm->IsBodyFrm() )
                    pBodyFrm = static_cast< const SwLayoutFrm* >( pBodyFrm->GetNext() );

                if ( pBodyFrm )
                {
                    SwCntntFrm *pCnt = const_cast< SwCntntFrm* >( pBodyFrm->ContainsCntnt() );
                    SwCntntNode* pNd = pCnt->GetNode();

                    pNd->GetDoc()->GetIDocumentUndoRedo( ).StartUndo( UNDO_UI_DELETE_PAGE_BREAK, NULL );

                    SfxItemSet aSet( GetEditWin()->GetView().GetWrtShell().GetAttrPool(),
                            RES_PAGEDESC, RES_PAGEDESC,
                            RES_BREAK, RES_BREAK,
                            NULL );
                    aSet.Put( SvxFmtBreakItem( SVX_BREAK_NONE, RES_BREAK ) );
                    aSet.Put( SwFmtPageDesc( NULL ) );

                    SwPaM aPaM( *pNd );
                    pNd->GetDoc()->InsertItemSet( aPaM, aSet, nsSetAttrMode::SETATTR_DEFAULT );

                    pNd->GetDoc()->GetIDocumentUndoRedo( ).EndUndo( UNDO_UI_DELETE_PAGE_BREAK, NULL );
                }
            }
            break;
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

void SwPageBreakWin::UpdatePosition( const Point* pEvtPt )
{
    if ( pEvtPt != NULL )
    {
        if ( pEvtPt == m_pMousePt )
            return;
        delete m_pMousePt;
        m_pMousePt = pEvtPt;
    }

    const SwPageFrm* pPageFrm = GetPageFrame();
    const SwFrm* pPrevPage = pPageFrm;
    do
    {
        pPrevPage = pPrevPage->GetPrev();
    }
    while ( pPrevPage && ( ( pPrevPage->Frm().Top( ) == pPageFrm->Frm().Top( ) )
                || static_cast< const SwPageFrm* >( pPrevPage )->IsEmptyPage( ) ) );

    Rectangle aBoundRect = GetEditWin()->LogicToPixel( pPageFrm->GetBoundRect().SVRect() );
    Rectangle aFrmRect = GetEditWin()->LogicToPixel( pPageFrm->Frm().SVRect() );

    long nYLineOffset = ( aBoundRect.Top() + aFrmRect.Top() ) / 2;
    if ( pPrevPage )
    {
        Rectangle aPrevFrmRect = GetEditWin()->LogicToPixel( pPrevPage->Frm().SVRect() );
        nYLineOffset = ( aPrevFrmRect.Bottom() + aFrmRect.Top() ) / 2;
    }

    // Get the page + sidebar coords
    long nPgLeft = aFrmRect.Left();
    long nPgRight = aFrmRect.Right();

    unsigned long nSidebarWidth = 0;
    const SwPostItMgr* pPostItMngr = GetEditWin()->GetView().GetWrtShell().GetPostItMgr();
    if ( pPostItMngr && pPostItMngr->HasNotes() && pPostItMngr->ShowNotes() )
        nSidebarWidth = pPostItMngr->GetSidebarBorderWidth( true ) + pPostItMngr->GetSidebarWidth( true );

    if ( pPageFrm->SidebarPosition( ) == sw::sidebarwindows::SIDEBAR_LEFT )
        nPgLeft -= nSidebarWidth;
    else if ( pPageFrm->SidebarPosition( ) == sw::sidebarwindows::SIDEBAR_RIGHT )
        nPgRight += nSidebarWidth;

    Size aBtnSize( BUTTON_WIDTH + ARROW_WIDTH, BUTTON_HEIGHT );

    // Place the button on the left or right?
    Rectangle aVisArea = GetEditWin()->LogicToPixel( GetEditWin()->GetView().GetVisArea() );

    long nLineLeft = std::max( nPgLeft, aVisArea.Left() );
    long nLineRight = std::min( nPgRight, aVisArea.Right() );
    long nBtnLeft = nLineLeft;

    if ( m_pMousePt )
    {
        nBtnLeft = nLineLeft + m_pMousePt->X() - aBtnSize.getWidth() / 2;

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
    Rectangle aRect( GetPosPixel(), GetSizePixel() );
    if ( aRect.IsInside( rDocPt ) )
        return true;

    Rectangle aLineRect( m_pLine->GetPosPixel(), m_pLine->GetSizePixel() );
    if ( aLineRect.IsInside( rDocPt ) )
        return true;

    return false;
}

const SwPageFrm* SwPageBreakWin::GetPageFrame( )
{
    return static_cast< const SwPageFrm * >( GetFrame( ) );
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

IMPL_LINK_NOARG(SwPageBreakWin, HideHandler)
{
    Fade( false );

    return 0;
}

IMPL_LINK_NOARG(SwPageBreakWin, FadeHandler)
{
    const int TICKS_BEFORE_WE_APPEAR = 10;
    if ( m_bIsAppearing && m_nDelayAppearing < TICKS_BEFORE_WE_APPEAR )
    {
        ++m_nDelayAppearing;
        m_aFadeTimer.Start();
        return 0;
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

    if ( IsVisible( ) && m_nFadeRate > 0 && m_nFadeRate < 100 )
        m_aFadeTimer.Start();

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
