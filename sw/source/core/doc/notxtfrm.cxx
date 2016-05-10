/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <hintids.hxx>
#include <tools/urlobj.hxx>
#include <vcl/print.hxx>
#include <vcl/virdev.hxx>
#include <vcl/svapp.hxx>
#include <svtools/imapobj.hxx>
#include <svtools/imap.hxx>
#include <svl/urihelper.hxx>
#include <svtools/soerr.hxx>
#include <sfx2/progress.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/printer.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/colritem.hxx>
#include <svx/xoutbmp.hxx>
#include <vcl/window.hxx>
#include <fmturl.hxx>
#include <fmtsrnd.hxx>
#include <frmfmt.hxx>
#include <swrect.hxx>
#include <fesh.hxx>
#include <doc.hxx>
#include <IDocumentSettingAccess.hxx>
#include <IDocumentStylePoolAccess.hxx>
#include <IDocumentDeviceAccess.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <flyfrm.hxx>
#include <flyfrms.hxx>
#include <frmtool.hxx>
#include <viewopt.hxx>
#include <viewimp.hxx>
#include <pam.hxx>
#include <hints.hxx>
#include <rootfrm.hxx>
#include <dflyobj.hxx>
#include <pagefrm.hxx>
#include <notxtfrm.hxx>
#include <grfatr.hxx>
#include <charatr.hxx>
#include <fmtornt.hxx>
#include <ndnotxt.hxx>
#include <ndgrf.hxx>
#include <ndole.hxx>
#include <swregion.hxx>
#include <poolfmt.hxx>
#include <mdiexp.hxx>
#include <swwait.hxx>
#include <comcore.hrc>
#include <accessibilityoptions.hxx>
#include <com/sun/star/embed/EmbedMisc.hpp>
#include <com/sun/star/embed/EmbedStates.hpp>
#include <svtools/embedhlp.hxx>
#include <svx/charthelper.hxx>
#include <dview.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <drawinglayer/processor2d/baseprocessor2d.hxx>
#include <drawinglayer/primitive2d/graphicprimitive2d.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <drawinglayer/processor2d/processor2dtools.hxx>
#include <txtfly.hxx>
#include <vcl/graphicfilter.hxx>
#include <vcl/pdfextoutdevdata.hxx>

using namespace com::sun::star;

inline bool GetRealURL( const SwGrfNode& rNd, OUString& rText )
{
    bool bRet = rNd.GetFileFilterNms( &rText, nullptr );
    if( bRet )
        rText = URIHelper::removePassword( rText, INetURLObject::WAS_ENCODED,
                                           INetURLObject::DECODE_UNAMBIGUOUS);
    if (rText.startsWith("data:image")) rText = "inline image";

    return bRet;
}

static void lcl_PaintReplacement( const SwRect &rRect, const OUString &rText,
                           const SwViewShell &rSh, const SwNoTextFrame *pFrame,
                           bool bDefect )
{
    static vcl::Font *pFont = nullptr;
    if ( !pFont )
    {
        pFont = new vcl::Font();
        pFont->SetWeight( WEIGHT_BOLD );
        pFont->SetStyleName( OUString() );
        pFont->SetName("Arial Unicode");
        pFont->SetFamily( FAMILY_SWISS );
        pFont->SetTransparent( true );
    }

    Color aCol( COL_RED );
    FontUnderline eUnderline = UNDERLINE_NONE;
    const SwFormatURL &rURL = pFrame->FindFlyFrame()->GetFormat()->GetURL();
    if( !rURL.GetURL().isEmpty() || rURL.GetMap() )
    {
        bool bVisited = false;
        if ( rURL.GetMap() )
        {
            ImageMap *pMap = const_cast<ImageMap*>(rURL.GetMap());
            for( size_t i = 0; i < pMap->GetIMapObjectCount(); ++i )
            {
                IMapObject *pObj = pMap->GetIMapObject( i );
                if( rSh.GetDoc()->IsVisitedURL( pObj->GetURL() ) )
                {
                    bVisited = true;
                    break;
                }
            }
        }
        else if ( !rURL.GetURL().isEmpty() )
            bVisited = rSh.GetDoc()->IsVisitedURL( rURL.GetURL() );

        SwFormat *pFormat = rSh.GetDoc()->getIDocumentStylePoolAccess().GetFormatFromPool( static_cast<sal_uInt16>
            (bVisited ? RES_POOLCHR_INET_VISIT : RES_POOLCHR_INET_NORMAL ) );
        aCol = pFormat->GetColor().GetValue();
        eUnderline = pFormat->GetUnderline().GetLineStyle();
    }

    pFont->SetUnderline( eUnderline );
    pFont->SetColor( aCol );

    const BitmapEx& rBmp = const_cast<SwViewShell&>(rSh).GetReplacementBitmap(bDefect);
    Graphic::DrawEx( rSh.GetOut(), rText, *pFont, rBmp, rRect.Pos(), rRect.SSize() );
}

SwNoTextFrame::SwNoTextFrame(SwNoTextNode * const pNode, SwFrame* pSib )
    : SwContentFrame( pNode, pSib )
{
    InitCtor();
}

/// Initialization: Currently add the Frame to the Cache
void SwNoTextFrame::InitCtor()
{
    mnFrameType = FRM_NOTXT;
}

SwContentFrame *SwNoTextNode::MakeFrame( SwFrame* pSib )
{
    return new SwNoTextFrame(this, pSib);
}

void SwNoTextFrame::DestroyImpl()
{
    StopAnimation();

    SwContentFrame::DestroyImpl();
}

SwNoTextFrame::~SwNoTextFrame()
{
}

void SetOutDev( SwViewShell *pSh, OutputDevice *pOut )
{
    pSh->mpOut = pOut;
}

static void lcl_ClearArea( const SwFrame &rFrame,
                    vcl::RenderContext &rOut, const SwRect& rPtArea,
                    const SwRect &rGrfArea )
{
    SwRegionRects aRegion( rPtArea, 4 );
    aRegion -= rGrfArea;

    if ( !aRegion.empty() )
    {
        const SvxBrushItem *pItem; const Color *pCol; SwRect aOrigRect;

        //UUUU
        drawinglayer::attribute::SdrAllFillAttributesHelperPtr aFillAttributes;

        if ( rFrame.GetBackgroundBrush( aFillAttributes, pItem, pCol, aOrigRect, false ) )
        {
            SwRegionRects const region(rPtArea);
            const bool bDone(::DrawFillAttributes(aFillAttributes, aOrigRect, region, rOut));

            if(!bDone)
            {
                for( const auto &rRegion : aRegion )
                {
                    ::DrawGraphic( pItem, &rOut, aOrigRect, rRegion );
                }
            }
        }
        else
        {
            rOut.Push( PushFlags::FILLCOLOR|PushFlags::LINECOLOR );
            rOut.SetFillColor( rFrame.getRootFrame()->GetCurrShell()->Imp()->GetRetoucheColor());
            rOut.SetLineColor();
            for( const auto &rRegion : aRegion )
                rOut.DrawRect( rRegion.SVRect() );
            rOut.Pop();
        }
    }
}

void SwNoTextFrame::Paint(vcl::RenderContext& rRenderContext, SwRect const& rRect, SwPrintData const*const) const
{
    if ( Frame().IsEmpty() )
        return;

    const SwViewShell* pSh = getRootFrame()->GetCurrShell();
    if( !pSh->GetViewOptions()->IsGraphic() )
    {
        StopAnimation();
        // #i6467# - no paint of placeholder for page preview
        if ( pSh->GetWin() && !pSh->IsPreview() )
        {
            const SwNoTextNode* pNd = GetNode()->GetNoTextNode();
            OUString aText( pNd->GetTitle() );
            if ( aText.isEmpty() && pNd->IsGrfNode() )
                GetRealURL( *static_cast<const SwGrfNode*>(pNd), aText );
            if( aText.isEmpty() )
                aText = FindFlyFrame()->GetFormat()->GetName();
            lcl_PaintReplacement( Frame(), aText, *pSh, this, false );
        }
        return;
    }

    if( pSh->GetAccessibilityOptions()->IsStopAnimatedGraphics() ||
    // #i9684# Stop animation during printing/pdf export
       !pSh->GetWin() )
        StopAnimation();

    SfxProgress::EnterLock(); // No progress reschedules in paint (SwapIn)

    rRenderContext.Push();
    bool bClip = true;
    tools::PolyPolygon aPoly;

    SwNoTextNode& rNoTNd = const_cast<SwNoTextNode&>(*static_cast<const SwNoTextNode*>(GetNode()));
    SwGrfNode* pGrfNd = rNoTNd.GetGrfNode();
    if( pGrfNd )
        pGrfNd->SetFrameInPaint( true );

    // #i13147# - add 2nd parameter with value <true> to
    // method call <FindFlyFrame().GetContour(..)> to indicate that it is called
    // for paint in order to avoid load of the intrinsic graphic.
    if ( ( !rRenderContext.GetConnectMetaFile() ||
           !pSh->GetWin() ) &&
         FindFlyFrame()->GetContour( aPoly, true )
       )
    {
        rRenderContext.SetClipRegion(vcl::Region(aPoly));
        bClip = false;
    }

    SwRect aOrigPaint( rRect );
    if ( HasAnimation() && pSh->GetWin() )
    {
        aOrigPaint = Frame(); aOrigPaint += Prt().Pos();
    }

    SwRect aGrfArea( Frame() );
    SwRect aPaintArea( aGrfArea );

    // In case the picture fly frm was clipped, render it with the origin
    // size instead of scaling it
    if ( rNoTNd.getIDocumentSettingAccess()->get( DocumentSettingId::CLIPPED_PICTURES ) )
    {
        const SwFlyFreeFrame *pFly = dynamic_cast< const SwFlyFreeFrame* >( FindFlyFrame() );
        if( pFly )
            aGrfArea = SwRect( Frame().Pos( ), pFly->GetUnclippedFrame( ).SSize( ) );
    }

    aPaintArea._Intersection( aOrigPaint );

    SwRect aNormal( Frame().Pos() + Prt().Pos(), Prt().SSize() );
    aNormal.Justify(); // Normalized rectangle for the comparisons

    if( aPaintArea.IsOver( aNormal ) )
    {
        // Calculate the four to-be-deleted rectangles
        if( pSh->GetWin() )
            ::lcl_ClearArea( *this, rRenderContext, aPaintArea, aNormal );

        // The intersection of the PaintArea and the Bitmap contains the absolutely visible area of the Frame
        aPaintArea._Intersection( aNormal );

        if ( bClip )
            rRenderContext.IntersectClipRegion( aPaintArea.SVRect() );
        /// delete unused 3rd parameter
        PaintPicture( &rRenderContext, aGrfArea );
    }
    else
        // If it's not visible, simply delete the given Area
        lcl_ClearArea( *this, rRenderContext, aPaintArea, SwRect() );
    if( pGrfNd )
        pGrfNd->SetFrameInPaint( false );

    rRenderContext.Pop();
    SfxProgress::LeaveLock();
}

/** Calculate the position and the size of the graphic in the Frame,
    corresponding to the current graphic attributes

    @param Point  the position in the Frame (also returned)
    @param Size   the graphic's size (also returned)
    @param nMirror the current mirror attribute
*/
static void lcl_CalcRect( Point& rPt, Size& rDim, sal_uInt16 nMirror )
{
    if( nMirror == RES_MIRROR_GRAPH_VERT || nMirror == RES_MIRROR_GRAPH_BOTH )
    {
        rPt.setX(rPt.getX() + rDim.Width() -1);
        rDim.Width() = -rDim.Width();
    }

    if( nMirror == RES_MIRROR_GRAPH_HOR || nMirror == RES_MIRROR_GRAPH_BOTH )
    {
        rPt.setY(rPt.getY() + rDim.Height() -1);
        rDim.Height() = -rDim.Height();
    }
}

/** Calculate the Bitmap's position and the size within the passed rectangle */
void SwNoTextFrame::GetGrfArea( SwRect &rRect, SwRect* pOrigRect,
                             bool ) const
{
    // Currently only used for scaling, cropping and mirroring the contour of graphics!
    // Everything else is handled by GraphicObject

    // We put the graphic's visible rectangle into rRect.
    // pOrigRect contains position and size of the whole graphic.

    const SwAttrSet& rAttrSet = GetNode()->GetSwAttrSet();
    const SwCropGrf& rCrop = rAttrSet.GetCropGrf();
    sal_uInt16 nMirror = rAttrSet.GetMirrorGrf().GetValue();

    if( rAttrSet.GetMirrorGrf().IsGrfToggle() )
    {
        if( !(FindPageFrame()->GetVirtPageNum() % 2) )
        {
            switch ( nMirror )
            {
                case RES_MIRROR_GRAPH_DONT: nMirror = RES_MIRROR_GRAPH_VERT; break;
                case RES_MIRROR_GRAPH_VERT: nMirror = RES_MIRROR_GRAPH_DONT; break;
                case RES_MIRROR_GRAPH_HOR: nMirror = RES_MIRROR_GRAPH_BOTH; break;
                default: nMirror = RES_MIRROR_GRAPH_HOR; break;
            }
        }
    }

    // We read graphic from the Node, if needed.
    // It may fail, however.
    long nLeftCrop, nRightCrop, nTopCrop, nBottomCrop;
    Size aOrigSz( static_cast<const SwNoTextNode*>(GetNode())->GetTwipSize() );
    if ( !aOrigSz.Width() )
    {
        aOrigSz.Width() = Prt().Width();
        nLeftCrop  = -rCrop.GetLeft();
        nRightCrop = -rCrop.GetRight();
    }
    else
    {
        nLeftCrop = std::max( aOrigSz.Width() -
                            (rCrop.GetRight() + rCrop.GetLeft()), long(1) );
        const double nScale = double(Prt().Width())  / double(nLeftCrop);
        nLeftCrop  = long(nScale * -rCrop.GetLeft() );
        nRightCrop = long(nScale * -rCrop.GetRight() );
    }

    // crop values have to be mirrored too
    if( nMirror == RES_MIRROR_GRAPH_VERT || nMirror == RES_MIRROR_GRAPH_BOTH )
    {
        long nTmpCrop = nLeftCrop;
        nLeftCrop = nRightCrop;
        nRightCrop= nTmpCrop;
    }

    if( !aOrigSz.Height() )
    {
        aOrigSz.Height() = Prt().Height();
        nTopCrop   = -rCrop.GetTop();
        nBottomCrop= -rCrop.GetBottom();
    }
    else
    {
        nTopCrop = std::max( aOrigSz.Height() - (rCrop.GetTop() + rCrop.GetBottom()), long(1) );
        const double nScale = double(Prt().Height()) / double(nTopCrop);
        nTopCrop   = long(nScale * -rCrop.GetTop() );
        nBottomCrop= long(nScale * -rCrop.GetBottom() );
    }

    // crop values have to be mirrored too
    if( nMirror == RES_MIRROR_GRAPH_HOR || nMirror == RES_MIRROR_GRAPH_BOTH )
    {
        long nTmpCrop = nTopCrop;
        nTopCrop   = nBottomCrop;
        nBottomCrop= nTmpCrop;
    }

    Size  aVisSz( Prt().SSize() );
    Size  aGrfSz( aVisSz );
    Point aVisPt( Frame().Pos() + Prt().Pos() );
    Point aGrfPt( aVisPt );

    // Set the "visible" rectangle first
    if ( nLeftCrop > 0 )
    {
        aVisPt.setX(aVisPt.getX() + nLeftCrop);
        aVisSz.Width() -= nLeftCrop;
    }
    if ( nTopCrop > 0 )
    {
        aVisPt.setY(aVisPt.getY() + nTopCrop);
        aVisSz.Height() -= nTopCrop;
    }
    if ( nRightCrop > 0 )
        aVisSz.Width() -= nRightCrop;
    if ( nBottomCrop > 0 )
        aVisSz.Height() -= nBottomCrop;

    rRect.Pos  ( aVisPt );
    rRect.SSize( aVisSz );

    // Calculate the whole graphic if needed
    if ( pOrigRect )
    {
        Size aTmpSz( aGrfSz );
        aGrfPt.setX(aGrfPt.getX() + nLeftCrop);
        aTmpSz.Width() -= nLeftCrop + nRightCrop;
        aGrfPt.setY(aGrfPt.getY() + nTopCrop);
        aTmpSz.Height()-= nTopCrop + nBottomCrop;

        if( RES_MIRROR_GRAPH_DONT != nMirror )
            lcl_CalcRect( aGrfPt, aTmpSz, nMirror );

        pOrigRect->Pos  ( aGrfPt );
        pOrigRect->SSize( aTmpSz );
    }
}

/** By returning the surrounding Fly's size which equals the graphic's size */
const Size& SwNoTextFrame::GetSize() const
{
    // Return the Frame's size
    const SwFrame *pFly = FindFlyFrame();
    if( !pFly )
        pFly = this;
    return pFly->Prt().SSize();
}

void SwNoTextFrame::MakeAll(vcl::RenderContext* /*pRenderContext*/)
{
    SwContentNotify aNotify( this );
    SwBorderAttrAccess aAccess( SwFrame::GetCache(), this );
    const SwBorderAttrs &rAttrs = *aAccess.Get();

    while ( !mbValidPos || !mbValidSize || !mbValidPrtArea )
    {
        MakePos();

        if ( !mbValidSize )
            Frame().Width( GetUpper()->Prt().Width() );

        MakePrtArea( rAttrs );

        if ( !mbValidSize )
        {   mbValidSize = true;
            Format(getRootFrame()->GetCurrShell()->GetOut());
        }
    }
}

/** Calculate the Bitmap's site, if needed */
void SwNoTextFrame::Format( vcl::RenderContext* /*pRenderContext*/, const SwBorderAttrs * )
{
    const Size aNewSize( GetSize() );

    // Did the height change?
    SwTwips nChgHght = IsVertical() ?
        (SwTwips)(aNewSize.Width() - Prt().Width()) :
        (SwTwips)(aNewSize.Height() - Prt().Height());
    if( nChgHght > 0)
        Grow( nChgHght );
    else if( nChgHght < 0)
        Shrink( std::min(Prt().Height(), -nChgHght) );
}

bool SwNoTextFrame::GetCharRect( SwRect &rRect, const SwPosition& rPos,
                              SwCursorMoveState *pCMS ) const
{
    if ( &rPos.nNode.GetNode() != static_cast<SwNode const *>(GetNode()) )
        return false;

    Calc(getRootFrame()->GetCurrShell()->GetOut());
    SwRect aFrameRect( Frame() );
    rRect = aFrameRect;
    rRect.Pos( Frame().Pos() + Prt().Pos() );
    rRect.SSize( Prt().SSize() );

    rRect.Justify();

    // Is the Bitmap in the visible area at all?
    if( !aFrameRect.IsOver( rRect ) )
    {
        // If not, then the Cursor is on the Frame
        rRect = aFrameRect;
        rRect.Width( 1 );
    }
    else
        rRect._Intersection( aFrameRect );

    if ( pCMS )
    {
        if ( pCMS->m_bRealHeight )
        {
            pCMS->m_aRealHeight.setY(rRect.Height());
            pCMS->m_aRealHeight.setX(0);
        }
    }

    return true;
}

bool SwNoTextFrame::GetCursorOfst(SwPosition* pPos, Point& ,
                             SwCursorMoveState*, bool ) const
{
    SwContentNode* pCNd = const_cast<SwContentNode*>(GetNode());
    pPos->nNode = *pCNd;
    pPos->nContent.Assign( pCNd, 0 );
    return true;
}

#define CLEARCACHE {\
    SwFlyFrame* pFly = FindFlyFrame();\
    if( pFly && pFly->GetFormat()->GetSurround().IsContour() )\
    {\
        ClrContourCache( pFly->GetVirtDrawObj() );\
        pFly->NotifyBackground( FindPageFrame(), Prt(), PREP_FLY_ATTR_CHG );\
    }\
}

void SwNoTextFrame::Modify( const SfxPoolItem* pOld, const SfxPoolItem* pNew )
{
    sal_uInt16 nWhich = pNew ? pNew->Which() : pOld ? pOld->Which() : 0;

    // #i73788#
    // no <SwContentFrame::Modify(..)> for RES_LINKED_GRAPHIC_STREAM_ARRIVED
    if ( RES_GRAPHIC_PIECE_ARRIVED != nWhich &&
         RES_GRAPHIC_ARRIVED != nWhich &&
         RES_GRF_REREAD_AND_INCACHE != nWhich &&
         RES_LINKED_GRAPHIC_STREAM_ARRIVED != nWhich )
    {
        SwContentFrame::Modify( pOld, pNew );
    }

    bool bComplete = true;

    switch( nWhich )
    {
    case RES_OBJECTDYING:
        break;

    case RES_GRF_REREAD_AND_INCACHE:
        if( ND_GRFNODE == GetNode()->GetNodeType() )
        {
            bComplete = false;
            SwGrfNode* pNd = static_cast<SwGrfNode*>( GetNode());

            SwViewShell* pVSh = pNd->GetDoc()->getIDocumentLayoutAccess().GetCurrentViewShell();
            if( pVSh )
            {
                GraphicAttr aAttr;
                if( pNd->GetGrfObj().IsCached( pVSh->GetOut(), Point(),
                            Prt().SSize(), &pNd->GetGraphicAttr( aAttr, this ) ))
                {
                    for(SwViewShell& rShell : pVSh->GetRingContainer())
                    {
                        SET_CURR_SHELL( &rShell );
                        if( rShell.GetWin() )
                        {
                            if( rShell.IsPreview() )
                                ::RepaintPagePreview( &rShell, Frame().SVRect() );
                            else
                                rShell.GetWin()->Invalidate( Frame().SVRect() );
                        }
                    }
                }
            }
        }
        break;

    case RES_UPDATE_ATTR:
        if (GetNode()->GetNodeType() != ND_GRFNODE) {
            break;
        }
        SAL_FALLTHROUGH;
    case RES_FMT_CHG:
        CLEARCACHE
        break;

    case RES_ATTRSET_CHG:
        {
            sal_uInt16 n;
            for( n = RES_GRFATR_BEGIN; n < RES_GRFATR_END; ++n )
                if( SfxItemState::SET == static_cast<const SwAttrSetChg*>(pOld)->GetChgSet()->
                                GetItemState( n, false ))
                {
                    CLEARCACHE
                    break;
                }
            if( RES_GRFATR_END == n )           // not found
                return ;
        }
        break;

    case RES_GRAPHIC_PIECE_ARRIVED:
    case RES_GRAPHIC_ARRIVED:
    // i73788# - handle RES_LINKED_GRAPHIC_STREAM_ARRIVED as RES_GRAPHIC_ARRIVED
    case RES_LINKED_GRAPHIC_STREAM_ARRIVED:
        if ( GetNode()->GetNodeType() == ND_GRFNODE )
        {
            bComplete = false;
            SwGrfNode* pNd = static_cast<SwGrfNode*>( GetNode());

            CLEARCACHE

            SwRect aRect( Frame() );

            SwViewShell *pVSh = pNd->GetDoc()->getIDocumentLayoutAccess().GetCurrentViewShell();
            if( !pVSh )
                break;

            for(SwViewShell& rShell : pVSh->GetRingContainer())
            {
                SET_CURR_SHELL( &rShell );
                if( rShell.IsPreview() )
                {
                    if( rShell.GetWin() )
                        ::RepaintPagePreview( &rShell, aRect );
                }
                else if ( rShell.VisArea().IsOver( aRect ) &&
                   OUTDEV_WINDOW == rShell.GetOut()->GetOutDevType() )
                {
                    // invalidate instead of painting
                    rShell.GetWin()->Invalidate( aRect.SVRect() );
                }
            }
        }
        break;

    default:
        if ( !pNew || !isGRFATR(nWhich) )
            return;
    }

    if( bComplete )
    {
        InvalidatePrt();
        SetCompletePaint();
    }
}

static void lcl_correctlyAlignRect( SwRect& rAlignedGrfArea, const SwRect& rInArea, vcl::RenderContext* pOut )
{

    if(!pOut)
        return;
    Rectangle aPxRect = pOut->LogicToPixel( rInArea.SVRect() );
    Rectangle aNewPxRect( aPxRect );
    while( aNewPxRect.Left() < aPxRect.Left() )
    {
        rAlignedGrfArea.Left( rAlignedGrfArea.Left()+1 );
        aNewPxRect = pOut->LogicToPixel( rAlignedGrfArea.SVRect() );
    }
    while( aNewPxRect.Top() < aPxRect.Top() )
    {
        rAlignedGrfArea.Top( rAlignedGrfArea.Top()+1 );
        aNewPxRect = pOut->LogicToPixel( rAlignedGrfArea.SVRect() );
    }
    while( aNewPxRect.Bottom() > aPxRect.Bottom() )
    {
        rAlignedGrfArea.Bottom( rAlignedGrfArea.Bottom()-1 );
        aNewPxRect = pOut->LogicToPixel( rAlignedGrfArea.SVRect() );
    }
    while( aNewPxRect.Right() > aPxRect.Right() )
    {
        rAlignedGrfArea.Right( rAlignedGrfArea.Right()-1 );
        aNewPxRect = pOut->LogicToPixel( rAlignedGrfArea.SVRect() );
    }
}

bool paintUsingPrimitivesHelper(
    vcl::RenderContext& rOutputDevice,
    const drawinglayer::primitive2d::Primitive2DSequence& rSequence,
    const basegfx::B2DRange& rSourceRange,
    const basegfx::B2DRange& rTargetRange)
{
    if(rSequence.hasElements() && !basegfx::fTools::equalZero(rSourceRange.getWidth()) && !basegfx::fTools::equalZero(rSourceRange.getHeight()))
    {
        if(!basegfx::fTools::equalZero(rTargetRange.getWidth()) && !basegfx::fTools::equalZero(rTargetRange.getHeight()))
        {
            // map graphic range to target range. This will e.g. automatically include
            // the mapping from 1/100th mm content to twips if needed when the target
            // range is defined in twips
            const basegfx::B2DHomMatrix aMappingTransform(
                basegfx::tools::createSourceRangeTargetRangeTransform(
                    rSourceRange,
                    rTargetRange));

            // Fill ViewInformation. Use MappingTransform here, so there is no need to
            // embed the primitives to it. Use original TargetRange here so there is also
            // no need to embed the primitives to a MaskPrimitive for cropping. This works
            // only in this case where the graphic object cannot be rotated, though.
            const drawinglayer::geometry::ViewInformation2D aViewInformation2D(
                aMappingTransform,
                rOutputDevice.GetViewTransformation(),
                rTargetRange,
                nullptr,
                0.0,
                uno::Sequence< beans::PropertyValue >());

            // get a primitive processor for rendering
            drawinglayer::processor2d::BaseProcessor2D* pProcessor2D =
                drawinglayer::processor2d::createProcessor2DFromOutputDevice(
                                                rOutputDevice, aViewInformation2D);

            if(pProcessor2D)
            {
                // render and cleanup
                pProcessor2D->process(rSequence);
                delete pProcessor2D;
                return true;
            }
        }
    }

    return false;
}

void paintGraphicUsingPrimitivesHelper(vcl::RenderContext & rOutputDevice,
         GraphicObject const& rGrfObj, GraphicAttr const& rGraphicAttr,
         SwRect const& rAlignedGrfArea)
{
    // unify using GraphicPrimitive2D
    // -> the primitive handles all crop and mirror stuff
    // -> the primitive renderer will create the needed pdf export data
    // -> if bitmap content, it will be cached system-dependent
    const basegfx::B2DRange aTargetRange(
        rAlignedGrfArea.Left(), rAlignedGrfArea.Top(),
        rAlignedGrfArea.Right(), rAlignedGrfArea.Bottom());
    const basegfx::B2DHomMatrix aTargetTransform(
        basegfx::tools::createScaleTranslateB2DHomMatrix(
            aTargetRange.getRange(),
            aTargetRange.getMinimum()));

    drawinglayer::primitive2d::Primitive2DSequence aContent(1);
    bool bDone(false);

    // #i125171# The mechanism to get lossless jpegs into pdf is based on having the original
    // file data (not the bitmap data) at the Graphic in the GfxLink (which has *nothing* to
    // do with the graphic being linked). This works well for DrawingLayer GraphicObjects (linked
    // and unlinked) but fails for linked Writer GraphicObjects. These have the URL in the
    // GraphicObject, but no GfxLink with the original file data when it's a linked graphic.
    // Since this blows up PDF size by a factor of 10 (the graphics get embedded as pixel maps
    // then) it is okay to add this workarund: In the needed case, load the graphic in a way to
    // get the GfxLink in the needed form and use that Graphic temporarily. Do this only when
    // - we have PDF export
    // - the GraphicObject is linked
    // - the Graphic has no GfxLink
    // - LosslessCompression is activated
    // - it's indeed a jpeg graphic (could be checked by the url ending, but is more reliable to check later)
    // In all other cases (normal repaint, print, etc...) use the available Graphic with the
    // already loaded pixel graphic as before this change.
    if (rOutputDevice.GetExtOutDevData() && rGrfObj.HasLink() && !rGrfObj.GetGraphic().IsLink())
    {
        const vcl::PDFExtOutDevData* pPDFExt = dynamic_cast< const vcl::PDFExtOutDevData* >(rOutputDevice.GetExtOutDevData());

        if (pPDFExt && pPDFExt->GetIsLosslessCompression())
        {
            Graphic aTempGraphic;
            INetURLObject aURL(rGrfObj.GetLink());

            if (GRFILTER_OK == GraphicFilter::GetGraphicFilter().ImportGraphic(aTempGraphic, aURL))
            {
                if(aTempGraphic.IsLink() && GFX_LINK_TYPE_NATIVE_JPG == aTempGraphic.GetLink().GetType())
                {
                    aContent[0] = new drawinglayer::primitive2d::GraphicPrimitive2D(
                        aTargetTransform,
                        aTempGraphic,
                        rGraphicAttr);
                    bDone = true;
                }
            }
        }
    }

    if(!bDone)
    {
        aContent[0] = new drawinglayer::primitive2d::GraphicPrimitive2D(
            aTargetTransform,
            rGrfObj,
            rGraphicAttr);
    }

    paintUsingPrimitivesHelper(
        rOutputDevice,
        aContent,
        aTargetRange,
        aTargetRange);
}

/** Paint the graphic.

    We require either a QuickDraw-Bitmap or a graphic here. If we do not have
    either, we return a replacement.

    @todo use aligned rectangle for drawing graphic.
    @todo pixel-align coordinations for drawing graphic. */
void SwNoTextFrame::PaintPicture( vcl::RenderContext* pOut, const SwRect &rGrfArea ) const
{
    SwViewShell* pShell = getRootFrame()->GetCurrShell();

    SwNoTextNode& rNoTNd = const_cast<SwNoTextNode&>(*static_cast<const SwNoTextNode*>(GetNode()));
    SwGrfNode* pGrfNd = rNoTNd.GetGrfNode();
    SwOLENode* pOLENd = rNoTNd.GetOLENode();

    const bool bPrn = pOut == rNoTNd.getIDocumentDeviceAccess().getPrinter( false ) ||
                          pOut->GetConnectMetaFile();

    const bool bIsChart = pOLENd && pOLENd->GetOLEObj().GetObject().IsChart();

    // calculate aligned rectangle from parameter <rGrfArea>.
    //     Use aligned rectangle <aAlignedGrfArea> instead of <rGrfArea> in
    //     the following code.
    SwRect aAlignedGrfArea = rGrfArea;
    ::SwAlignRect( aAlignedGrfArea,  pShell, pOut );

    if( !bIsChart )
    {
        // Because for drawing a graphic left-top-corner and size coordinations are
        // used, these coordinations have to be determined on pixel level.
        ::SwAlignGrfRect( &aAlignedGrfArea, *pOut );
    }
    else //if( bIsChart )
    {
        // #i78025# charts own borders are not completely visible
        // the above pixel correction is not correct - at least not for charts
        // so a different pixel correction is chosen here
        // this might be a good idea for all other OLE objects also,
        // but as I cannot oversee the consequences I fix it only for charts for now
        lcl_correctlyAlignRect( aAlignedGrfArea, rGrfArea, pOut );
    }

    if( pGrfNd )
    {
        // Fix for bug fdo#33781
        const AntialiasingFlags nFormerAntialiasingAtOutput( pOut->GetAntialiasing() );
        if (pShell->Imp()->GetDrawView()->IsAntiAliasing())
        {
            pOut->SetAntialiasing( nFormerAntialiasingAtOutput | AntialiasingFlags::EnableB2dDraw );
        }

        bool bContinue = true;
        const GraphicObject& rGrfObj = pGrfNd->GetGrfObj(bPrn);

        GraphicAttr aGrfAttr;
        pGrfNd->GetGraphicAttr( aGrfAttr, this );

        if( !bPrn )
        {
            // #i73788#
            if ( pGrfNd->IsLinkedInputStreamReady() )
            {
                pGrfNd->UpdateLinkWithInputStream();
            }
            // #i85717#, #i90395# - check, if asynchronous retrieval
            // if input stream for the graphic is possible
            else if ( ( rGrfObj.GetType() == GRAPHIC_DEFAULT ||
                        rGrfObj.GetType() == GRAPHIC_NONE ) &&
                      pGrfNd->IsLinkedFile() &&
                      pGrfNd->IsAsyncRetrieveInputStreamPossible() )
            {
                Size aTmpSz;
                ::sfx2::SvLinkSource* pGrfObj = pGrfNd->GetLink()->GetObj();
                if( !pGrfObj ||
                    !pGrfObj->IsDataComplete() ||
                    !(aTmpSz = pGrfNd->GetTwipSize()).Width() ||
                    !aTmpSz.Height() || !pGrfNd->GetAutoFormatLvl() )
                {
                    pGrfNd->TriggerAsyncRetrieveInputStream(); // #i73788#
                }
                OUString aText( pGrfNd->GetTitle() );
                if ( aText.isEmpty() )
                    GetRealURL( *pGrfNd, aText );
                ::lcl_PaintReplacement( aAlignedGrfArea, aText, *pShell, this, false );
                bContinue = false;
            }
        }

        if( bContinue )
        {
            if( rGrfObj.GetGraphic().IsSupportedGraphic())
            {
                const bool bAnimate = rGrfObj.IsAnimated() &&
                                         !pShell->IsPreview() &&
                                         !pShell->GetAccessibilityOptions()->IsStopAnimatedGraphics() &&
                // #i9684# Stop animation during printing/pdf export
                                          pShell->GetWin();

                if( bAnimate &&
                    FindFlyFrame() != ::GetFlyFromMarked( nullptr, pShell ))
                {
                    OutputDevice* pVout;
                    if( pOut == pShell->GetOut() && SwRootFrame::FlushVout() )
                    {
                        pVout = pOut;
                        pOut = pShell->GetOut();
                    }
                    else if( pShell->GetWin() &&
                             OUTDEV_VIRDEV == pOut->GetOutDevType() )
                    {
                        pVout = pOut;
                        pOut = pShell->GetWin();
                    }
                    else
                        pVout = nullptr;

                    OSL_ENSURE( OUTDEV_VIRDEV != pOut->GetOutDevType() ||
                            pShell->GetViewOptions()->IsPDFExport() || pShell->isOutputToWindow(),
                            "pOut should not be a virtual device" );

                    pGrfNd->StartGraphicAnimation(pOut, aAlignedGrfArea.Pos(),
                                        aAlignedGrfArea.SSize(), sal_IntPtr(this),
                                        nullptr, GraphicManagerDrawFlags::STANDARD, pVout );
                }
                else
                {
                    paintGraphicUsingPrimitivesHelper(*pOut,
                            rGrfObj, aGrfAttr, aAlignedGrfArea);
                }
            }
            else
            {
                sal_uInt16 nResId = 0;

                if( GRAPHIC_NONE == rGrfObj.GetType() )
                    nResId = STR_COMCORE_READERROR;
                else if ( !rGrfObj.GetGraphic().IsSupportedGraphic() )
                    nResId = STR_COMCORE_CANT_SHOW;

                OUString aText;
                if ( !nResId &&
                     (aText = pGrfNd->GetTitle()).isEmpty() &&
                     (!GetRealURL( *pGrfNd, aText ) || aText.isEmpty()))
                {
                    nResId = STR_COMCORE_READERROR;
                }
                if ( nResId )
                    aText = SW_RESSTR( nResId );

                ::lcl_PaintReplacement( aAlignedGrfArea, aText, *pShell, this, true );
            }
        }

        if ( pShell->Imp()->GetDrawView()->IsAntiAliasing() )
            pOut->SetAntialiasing( nFormerAntialiasingAtOutput );
    }
    else // bIsChart || pOLENd
    {
        // Fix for bug fdo#33781
        const AntialiasingFlags nFormerAntialiasingAtOutput( pOut->GetAntialiasing() );
        if (pShell->Imp()->GetDrawView()->IsAntiAliasing())
        {
            AntialiasingFlags nNewAntialiasingAtOutput = nFormerAntialiasingAtOutput | AntialiasingFlags::EnableB2dDraw;

            // #i99665#
            // Adjust AntiAliasing mode at output device for chart OLE
            if ( pOLENd->IsChart() )
                nNewAntialiasingAtOutput |= AntialiasingFlags::PixelSnapHairline;

            pOut->SetAntialiasing( nNewAntialiasingAtOutput );
        }

        bool bDone(false);

        if(bIsChart)
        {
            const uno::Reference< frame::XModel > aXModel(pOLENd->GetOLEObj().GetOleRef()->getComponent(), uno::UNO_QUERY);

            if(aXModel.is())
            {
                basegfx::B2DRange aSourceRange;

                const drawinglayer::primitive2d::Primitive2DSequence aSequence(
                    ChartHelper::tryToGetChartContentAsPrimitive2DSequence(
                        aXModel,
                        aSourceRange));

                if(aSequence.hasElements() && !aSourceRange.isEmpty())
                {
                    const basegfx::B2DRange aTargetRange(
                        aAlignedGrfArea.Left(), aAlignedGrfArea.Top(),
                        aAlignedGrfArea.Right(), aAlignedGrfArea.Bottom());

                    bDone = paintUsingPrimitivesHelper(
                        *pOut,
                        aSequence,
                        aSourceRange,
                        aTargetRange);
                }
            }
        }

        if(!bDone && pOLENd)
        {
            Point aPosition(aAlignedGrfArea.Pos());
            Size aSize(aAlignedGrfArea.SSize());

            const Graphic* pGraphic = pOLENd->GetGraphic();
            if ( pGraphic && pGraphic->GetType() != GRAPHIC_NONE )
            {
                pGraphic->Draw( pOut, aPosition, aSize );

                // shade the representation if the object is activated outplace
                uno::Reference < embed::XEmbeddedObject > xObj = pOLENd->GetOLEObj().GetOleRef();
                if ( xObj.is() && xObj->getCurrentState() == embed::EmbedStates::ACTIVE )
                {
                    ::svt::EmbeddedObjectRef::DrawShading( Rectangle( aPosition, aSize ), pOut );
                }
            }
            else
                ::svt::EmbeddedObjectRef::DrawPaintReplacement( Rectangle( aPosition, aSize ), pOLENd->GetOLEObj().GetCurrentPersistName(), pOut );

            sal_Int64 nMiscStatus = pOLENd->GetOLEObj().GetOleRef()->getStatus( pOLENd->GetAspect() );
            if ( !bPrn && dynamic_cast< const SwCursorShell *>( pShell ) !=  nullptr && (
                    (nMiscStatus & embed::EmbedMisc::MS_EMBED_ACTIVATEWHENVISIBLE) ||
                    pOLENd->GetOLEObj().GetObject().IsGLChart()))
            {
                const SwFlyFrame *pFly = FindFlyFrame();
                assert( pFly != nullptr );
                static_cast<SwFEShell*>(pShell)->ConnectObj( pOLENd->GetOLEObj().GetObject(), pFly->Prt(), pFly->Frame());
            }
        }

        // see #i99665#
        if (pShell->Imp()->GetDrawView()->IsAntiAliasing())
        {
            pOut->SetAntialiasing( nFormerAntialiasingAtOutput );
        }
    }
}

bool SwNoTextFrame::IsTransparent() const
{
    const SwViewShell* pSh = getRootFrame()->GetCurrShell();
    if ( !pSh || !pSh->GetViewOptions()->IsGraphic() )
        return true;

    const SwGrfNode *pNd;
    if( nullptr != (pNd = GetNode()->GetGrfNode()) )
        return pNd->IsTransparent();

    //#29381# OLE are always transparent
    return true;
}

void SwNoTextFrame::StopAnimation( OutputDevice* pOut ) const
{
    // Stop animated graphics
    const SwGrfNode* pGrfNd = dynamic_cast< const SwGrfNode* >(GetNode()->GetGrfNode());

    if( pGrfNd && pGrfNd->IsAnimated() )
    {
        const_cast< SwGrfNode* >(pGrfNd)->StopGraphicAnimation( pOut, sal_IntPtr(this) );
    }
}

bool SwNoTextFrame::HasAnimation() const
{
    const SwGrfNode* pGrfNd = GetNode()->GetGrfNode();
    return pGrfNd && pGrfNd->IsAnimated();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
