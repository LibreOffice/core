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
#include <svx/sdr/contact/objectcontacttools.hxx>
#include <vcl/svapp.hxx>

#define BUTTON_WIDTH 30
#define BUTTON_HEIGHT 19
#define ARROW_WIDTH 9

using namespace basegfx;
using namespace basegfx::tools;
using namespace drawinglayer::primitive2d;

namespace
{
    B2DPolygon lcl_CreatePolygon( B2DRectangle aBounds, bool bShowOnRight )
    {
        B2DPolygon aRetval;
        const double nRadius = 1;
        const double nKappa((M_SQRT2 - 1.0) * 4.0 / 3.0);

        // Create the top left corner
        {
            B2DPoint aTLCorner = aBounds.getMinimum();
            B2DPoint aStart( 0.0, nRadius );
            B2DPoint aEnd( nRadius, 0.0 );
            aRetval.append( aStart );
            aRetval.appendBezierSegment(
                    interpolate( aStart, aTLCorner, nKappa ),
                    interpolate( aEnd, aTLCorner, nKappa ),
                    aEnd );
        }

        // Create the top right angle
        {
            B2DPoint aTMCorner( aBounds.getWidth() - ARROW_WIDTH, 0.0 );
            B2DPoint aStart = aTMCorner + B2DVector( - nRadius, 0.0 );
            B2DVector aEndVect( double( ARROW_WIDTH ), aBounds.getHeight() / 2.0 );
            aEndVect.setLength( nRadius );
            B2DPoint aEnd = aTMCorner + aEndVect;
            aRetval.append( aStart );
            aRetval.appendBezierSegment(
                    interpolate( aStart, aTMCorner, nKappa ),
                    interpolate( aEnd, aTMCorner, nKappa ),
                    aEnd );
        }

        // Create the right corner
        {
            B2DPoint aMRCorner( aBounds.getWidth(), aBounds.getHeight() / 2.0 );
            B2DVector aStartVect( double( - ARROW_WIDTH ), - aBounds.getHeight() / 2.0 );
            aStartVect.setLength( nRadius );
            B2DPoint aStart = aMRCorner + aStartVect;
            B2DVector aEndVect( double( - ARROW_WIDTH ), aBounds.getHeight() / 2.0 );
            aEndVect.setLength( nRadius );
            B2DPoint aEnd = aMRCorner + aEndVect;
            aRetval.append( aStart );
            aRetval.appendBezierSegment(
                    interpolate( aStart, aMRCorner, nKappa ),
                    interpolate( aEnd, aMRCorner, nKappa ),
                    aEnd );
        }

        // Create the bottom right angle
        {
            B2DPoint aBMCorner( aBounds.getWidth() - ARROW_WIDTH, aBounds.getHeight() );
            B2DVector aStartVect( double( ARROW_WIDTH ), - aBounds.getHeight() / 2.0 );
            aStartVect.setLength( nRadius );
            B2DPoint aStart = aBMCorner + aStartVect;
            B2DPoint aEnd = aBMCorner + B2DVector( - nRadius, 0.0 );
            aRetval.append( aStart );
            aRetval.appendBezierSegment(
                    interpolate( aStart, aBMCorner, nKappa ),
                    interpolate( aEnd, aBMCorner, nKappa ),
                    aEnd );
        }

        // Create the bottom left corner
        {
            B2DPoint aBLCorner( aBounds.getMinX(), aBounds.getHeight() );
            B2DPoint aStart( nRadius, aBounds.getHeight() );
            B2DPoint aEnd( 0.0, aBounds.getHeight() - nRadius );
            aRetval.append( aStart );
            aRetval.appendBezierSegment(
                    interpolate( aStart, aBLCorner, nKappa ),
                    interpolate( aEnd, aBLCorner, nKappa ),
                    aEnd );
        }

        aRetval.setClosed( true );

        if ( bShowOnRight )
        {
            B2DHomMatrix bRotMatrix = createRotateAroundPoint(
                    aBounds.getCenterX(), aBounds.getCenterY(), M_PI );
            aRetval.transform( bRotMatrix );
        }

        return aRetval;
    }

    class SwBreakDashedLine : public SwDashedLine
    {
        private:
            SwPageBreakWin* m_pWin;

        public:
            SwBreakDashedLine( Window* pParent, const BColor& rColor, SwPageBreakWin* pWin ) :
                SwDashedLine( pParent, rColor ),
                m_pWin( pWin ) {};


            virtual void MouseMove( const MouseEvent& rMEvt );
    };

    void SwBreakDashedLine::MouseMove( const MouseEvent& rMEvt )
    {
        if ( rMEvt.IsLeaveWindow() )
            m_pWin->Fade( false );
        else if ( !m_pWin->IsVisible() )
            m_pWin->Fade( true );
    }
}

SwPageBreakWin::SwPageBreakWin( SwEditWin* pEditWin, const SwPageFrm* pPageFrm ) :
    MenuButton( pEditWin, WB_DIALOGCONTROL ),
    SwFrameControl( pEditWin, pPageFrm ),
    m_pPopupMenu( NULL ),
    m_pLine( NULL ),
    m_bIsAppearing( false ),
    m_nFadeRate( 100 )
{
    // Use pixels for the rest of the drawing
    SetMapMode( MapMode ( MAP_PIXEL ) );

    // Create the line control
    BColor aColor = SwViewOption::GetPageBreakColor().getBColor();
    m_pLine = new SwBreakDashedLine( GetEditWin(), aColor, this );

    // Create the popup menu
    m_pPopupMenu = new PopupMenu( SW_RES( MN_PAGEBREAK_BUTTON ) );
    SetPopupMenu( m_pPopupMenu );

    m_aFadeTimer.SetTimeout( 500 );
    m_aFadeTimer.SetTimeoutHdl( LINK( this, SwPageBreakWin, FadeHandler ) );
}

SwPageBreakWin::~SwPageBreakWin( )
{
    delete m_pPopupMenu;
    delete m_pLine;
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

    bool bShowOnRight = ShowOnRight( );

    Primitive2DSequence aSeq( 3 );
    B2DRectangle aBRect( double( aRect.Left() ), double( aRect.Top( ) ),
           double( aRect.Right() ), double( aRect.Bottom( ) ) );
    B2DPolygon aPolygon = lcl_CreatePolygon( aBRect, bShowOnRight );

    // Create the polygon primitives
    aSeq[0] = Primitive2DReference( new PolyPolygonColorPrimitive2D(
            B2DPolyPolygon( aPolygon ), aOtherColor ) );
    aSeq[1] = Primitive2DReference( new PolygonHairlinePrimitive2D(
            aPolygon, aColor ) );

    // Create the primitive for the image
    Image aImg( SW_RES( IMG_PAGE_BREAK ) );
    double nImgOfstX = 3.0;
    if ( bShowOnRight )
        nImgOfstX = aRect.Right() - aImg.GetSizePixel().Width() - 3.0;
    aSeq[2] = Primitive2DReference( new DiscreteBitmapPrimitive2D(
            aImg.GetBitmapEx(), B2DPoint( nImgOfstX, 1.0 ) ) );

    // Paint the symbol if not readonly button
    if ( IsEnabled() )
    {
        double nTop = double( aRect.getHeight() ) / 2.0;
        double nBottom = nTop + 4.0;
        double nLeft = aRect.getWidth( ) - ARROW_WIDTH - 6.0;
        if ( bShowOnRight )
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
    }

    Primitive2DSequence aGhostedSeq( 1 );
    double nFadeRate = double( m_nFadeRate ) / 100.0;
    aGhostedSeq[0] = Primitive2DReference( new ModifiedColorPrimitive2D(
                aSeq, BColorModifier( Color( COL_WHITE ).getBColor(), 1.0 - nFadeRate, BCOLORMODIFYMODE_INTERPOLATE ) ) );

    // Create the processor and process the primitives
    const drawinglayer::geometry::ViewInformation2D aNewViewInfos;
    drawinglayer::processor2d::BaseProcessor2D * pProcessor =
        sdr::contact::createBaseProcessor2DFromOutputDevice(
                    *this, aNewViewInfos );

    pProcessor->process( aGhostedSeq );
}

void SwPageBreakWin::Select( )
{
    switch( GetCurItemId( ) )
    {
        case FN_PAGEBREAK_EDIT:
            {
                const SwLayoutFrm* pBodyFrm = static_cast< const SwLayoutFrm* >( GetPageFrame()->Lower() );
                while ( pBodyFrm && !pBodyFrm->IsBodyFrm() )
                    pBodyFrm = static_cast< const SwLayoutFrm* >( pBodyFrm->GetNext() );

                if ( pBodyFrm )
                {
                    if ( pBodyFrm->Lower()->IsTabFrm() )
                    {
                        SwWrtShell& rSh = GetEditWin()->GetView().GetWrtShell();
                        rSh.Push( );
                        rSh.ClearMark();
                        sal_Bool bOldLock = rSh.IsViewLocked();
                        rSh.LockView( sal_True );

                        SwCntntFrm *pCnt = const_cast< SwCntntFrm* >( pBodyFrm->ContainsCntnt() );
                        SwCntntNode* pNd = pCnt->GetNode();
                        rSh.SetSelection( *pNd );

                        SfxUInt16Item aItem( GetEditWin()->GetView().GetPool( ).GetWhich( FN_FORMAT_TABLE_DLG ), TP_TABLE_TEXTFLOW );
                        GetEditWin()->GetView().GetViewFrame()->GetDispatcher()->Execute(
                                FN_FORMAT_TABLE_DLG, SFX_CALLMODE_SYNCHRON|SFX_CALLMODE_RECORD, &aItem, NULL );

                        rSh.LockView( bOldLock );
                        rSh.Pop( sal_False );
                    }
                    else
                    {
                        SwCntntFrm *pCnt = const_cast< SwCntntFrm* >( pBodyFrm->ContainsCntnt() );
                        SwCntntNode* pNd = pCnt->GetNode();

                        SwPaM aPaM( *pNd );
                        SwPaMItem aPaMItem( GetEditWin()->GetView().GetPool( ).GetWhich( FN_PARAM_PAM ), &aPaM );
                        SfxUInt16Item aItem( GetEditWin()->GetView().GetPool( ).GetWhich( SID_PARA_DLG ), TP_PARA_EXT );
                        GetEditWin()->GetView().GetViewFrame()->GetDispatcher()->Execute(
                                SID_PARA_DLG, SFX_CALLMODE_SYNCHRON|SFX_CALLMODE_RECORD, &aItem, &aPaMItem, NULL );
                    }
                    GetEditWin()->GrabFocus( );
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
                    //sal_uInt16 nWhich = pCnt->GetAttrSet()->GetPool()->GetWhich( SID_ATTR_PARA_PAGEBREAK );
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
    Fade( false );
}

void SwPageBreakWin::MouseMove( const MouseEvent& rMEvt )
{
    if ( rMEvt.IsLeaveWindow() )
        Fade( false );
    else if ( !IsVisible() )
        Fade( true );
}

void SwPageBreakWin::Activate( )
{
    Fade( true );
    MenuButton::Activate();
}

bool SwPageBreakWin::ShowOnRight( )
{
    bool bOnRight = false;

    // Handle the book mode / columns view case
    const SwViewOption* pViewOpt = GetEditWin()->GetView().GetWrtShell().GetViewOptions();
    bool bBookMode = pViewOpt->IsViewLayoutBookMode();

    if ( bBookMode )
        bOnRight = GetPageFrame()->SidebarPosition( ) == sw::sidebarwindows::SIDEBAR_RIGHT;

    // TODO Handle the RTL case

    return bOnRight;
}

void SwPageBreakWin::UpdatePosition( )
{
    const SwPageFrm* pPageFrm = GetPageFrame();
    const SwFrm* pPrevPage = pPageFrm->GetPrev();
    while ( pPrevPage && ( pPrevPage->Frm().Top( ) == pPageFrm->Frm().Top( ) ) )
        pPrevPage = pPrevPage->GetPrev();

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
    long nBtnLeft = nPgLeft;

    if ( ShowOnRight( ) )
    {
        long nRight = std::min( nPgRight + aBtnSize.getWidth() - ARROW_WIDTH / 2, aVisArea.Right() );
        nBtnLeft = nRight - aBtnSize.getWidth();
        if ( IsVisible() )
           nLineRight = nBtnLeft - ARROW_WIDTH / 2;
    }
    else
    {
        nBtnLeft = std::max( nPgLeft - aBtnSize.Width() + ARROW_WIDTH / 2, aVisArea.Left() );
        if ( IsVisible() )
           nLineLeft = nBtnLeft + aBtnSize.Width( ) + ARROW_WIDTH / 2;
    }

    // Set the button position
    Point aBtnPos( nBtnLeft, nYLineOffset - aBtnSize.Height() / 2 );
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

const SwPageFrm* SwPageBreakWin::GetPageFrame( )
{
    return static_cast< const SwPageFrm * >( GetFrame( ) );
}

void SwPageBreakWin::SetReadonly( bool bReadonly )
{
    Enable( !bReadonly );
}

void SwPageBreakWin::Fade( bool bFadeIn )
{
    if ( !PopupMenu::IsInExecute() )
    {
        m_bIsAppearing = bFadeIn;
        if ( m_aFadeTimer.IsActive( ) )
            m_aFadeTimer.Stop();
        m_aFadeTimer.Start( );
    }
}

IMPL_LINK( SwPageBreakWin, FadeHandler, Timer *, EMPTYARG )
{
    if ( m_bIsAppearing && m_nFadeRate > 0 )
        m_nFadeRate -= 10;
    else if ( !m_bIsAppearing && m_nFadeRate < 100 )
        m_nFadeRate += 10;

    if ( m_nFadeRate != 100 && !IsVisible() )
        Show();
    else if ( m_nFadeRate == 100 && IsVisible( ) )
        Hide();
    else
    {
        UpdatePosition();
        Invalidate();
    }

    if ( IsVisible( ) )
    {
        if ( m_nFadeRate > 0 )
            m_aFadeTimer.SetTimeout( 100 );
        else
        {
            m_aFadeTimer.SetTimeout( 3000 );
        }

        if ( m_nFadeRate > 0 && m_nFadeRate < 100 )
            m_aFadeTimer.Start();
    }
    else
        m_aFadeTimer.SetTimeout( 500 );

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
