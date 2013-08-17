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
#include <dview.hxx> // #i99665#
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <drawinglayer/processor2d/baseprocessor2d.hxx>
#include <drawinglayer/primitive2d/bitmapprimitive2d.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <drawinglayer/processor2d/processor2dtools.hxx>

using namespace com::sun::star;

extern void ClrContourCache( const SdrObject *pObj ); // TxtFly.Cxx

inline bool GetRealURL( const SwGrfNode& rNd, String& rTxt )
{
    bool bRet = rNd.GetFileFilterNms( &rTxt, 0 );
    if( bRet )
        rTxt = URIHelper::removePassword( rTxt, INetURLObject::WAS_ENCODED,
                                           INetURLObject::DECODE_UNAMBIGUOUS);
    return bRet;
}

static void lcl_PaintReplacement( const SwRect &rRect, const String &rText,
                           const ViewShell &rSh, const SwNoTxtFrm *pFrm,
                           bool bDefect )
{
    static Font *pFont = 0;
    if ( !pFont )
    {
        pFont = new Font();
        pFont->SetWeight( WEIGHT_BOLD );
        pFont->SetStyleName( aEmptyStr );
        pFont->SetName(OUString("Arial Unicode"));
        pFont->SetFamily( FAMILY_SWISS );
        pFont->SetTransparent( sal_True );
    }

    Color aCol( COL_RED );
    FontUnderline eUnderline = UNDERLINE_NONE;
    const SwFmtURL &rURL = pFrm->FindFlyFrm()->GetFmt()->GetURL();
    if( !rURL.GetURL().isEmpty() || rURL.GetMap() )
    {
        bool bVisited = false;
        if ( rURL.GetMap() )
        {
            ImageMap *pMap = (ImageMap*)rURL.GetMap();
            for( sal_uInt16 i = 0; i < pMap->GetIMapObjectCount(); i++ )
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

        SwFmt *pFmt = rSh.GetDoc()->GetFmtFromPool( static_cast<sal_uInt16>
            (bVisited ? RES_POOLCHR_INET_VISIT : RES_POOLCHR_INET_NORMAL ) );
        aCol = pFmt->GetColor().GetValue();
        eUnderline = pFmt->GetUnderline().GetLineStyle();
    }

    pFont->SetUnderline( eUnderline );
    pFont->SetColor( aCol );

    const BitmapEx& rBmp = ViewShell::GetReplacementBitmap( bDefect );
    Graphic::DrawEx( rSh.GetOut(), rText, *pFont, rBmp, rRect.Pos(), rRect.SSize() );
}

SwNoTxtFrm::SwNoTxtFrm(SwNoTxtNode * const pNode, SwFrm* pSib )
    : SwCntntFrm( pNode, pSib )
{
    InitCtor();
}

/// Initialization: Currently add the Frame to the Cache
void SwNoTxtFrm::InitCtor()
{
    mnType = FRMC_NOTXT;
    // The graphic's weight is 0 if it has not been read,
    // < 0 if we had a read error and we needed to use the replacement and
    // > 0 if it is available
    nWeight = 0;
}

SwCntntFrm *SwNoTxtNode::MakeFrm( SwFrm* pSib )
{
    return new SwNoTxtFrm(this, pSib);
}

SwNoTxtFrm::~SwNoTxtFrm()
{
    StopAnimation();
}

void SetOutDev( ViewShell *pSh, OutputDevice *pOut )
{
    pSh->mpOut = pOut;
}

static void lcl_ClearArea( const SwFrm &rFrm,
                    OutputDevice &rOut, const SwRect& rPtArea,
                    const SwRect &rGrfArea )
{
    SwRegionRects aRegion( rPtArea, 4 );
    aRegion -= rGrfArea;

    if ( !aRegion.empty() )
    {
        const SvxBrushItem *pItem; const XFillStyleItem* pFillStyleItem; const XFillGradientItem* pFillGradientItem; const Color *pCol; SwRect aOrigRect;
        if ( rFrm.GetBackgroundBrush( pItem, pFillStyleItem, pFillGradientItem, pCol, aOrigRect, sal_False ) )
            for( sal_uInt16 i = 0; i < aRegion.size(); ++i )
                ::DrawGraphic( pItem, pFillStyleItem, pFillGradientItem, &rOut, aOrigRect, aRegion[i] );
        else
        {
            rOut.Push( PUSH_FILLCOLOR|PUSH_LINECOLOR );
            rOut.SetFillColor( rFrm.getRootFrm()->GetCurrShell()->Imp()->GetRetoucheColor());
            rOut.SetLineColor();
            for( sal_uInt16 i = 0; i < aRegion.size(); ++i )
                rOut.DrawRect( aRegion[i].SVRect() );
            rOut.Pop();
        }
    }
}

void SwNoTxtFrm::Paint(SwRect const& rRect, SwPrintData const*const) const
{
    if ( Frm().IsEmpty() )
        return;

    const ViewShell* pSh = getRootFrm()->GetCurrShell();
    if( !pSh->GetViewOptions()->IsGraphic() )
    {
        StopAnimation();
        // #i6467# - no paint of placeholder for page preview
        if ( pSh->GetWin() && !pSh->IsPreView() )
        {
            const SwNoTxtNode* pNd = GetNode()->GetNoTxtNode();
            String aTxt( pNd->GetTitle() );
            if ( !aTxt.Len() && pNd->IsGrfNode() )
                GetRealURL( *(SwGrfNode*)pNd, aTxt );
            if( !aTxt.Len() )
                aTxt = FindFlyFrm()->GetFmt()->GetName();
            lcl_PaintReplacement( Frm(), aTxt, *pSh, this, false );
        }
        return;
    }

    if( pSh->GetAccessibilityOptions()->IsStopAnimatedGraphics() ||
    // #i9684# Stop animation during printing/pdf export
       !pSh->GetWin() )
        StopAnimation();

    SfxProgress::EnterLock(); // No progress reschedules in paint (SwapIn)

    OutputDevice *pOut = pSh->GetOut();
    pOut->Push();
    bool bClip = true;
    PolyPolygon aPoly;

    SwNoTxtNode& rNoTNd = *(SwNoTxtNode*)GetNode();
    SwGrfNode* pGrfNd = rNoTNd.GetGrfNode();
    if( pGrfNd )
        pGrfNd->SetFrameInPaint( sal_True );

    // #i13147# - add 2nd parameter with value <sal_True> to
    // method call <FindFlyFrm().GetContour(..)> to indicate that it is called
    // for paint in order to avoid load of the intrinsic graphic.
    if ( ( !pOut->GetConnectMetaFile() ||
           !pSh->GetWin() ) &&
         FindFlyFrm()->GetContour( aPoly, sal_True )
       )
    {
        pOut->SetClipRegion(Region(aPoly));
        bClip = false;
    }

    SwRect aOrigPaint( rRect );
    if ( HasAnimation() && pSh->GetWin() )
    {
        aOrigPaint = Frm(); aOrigPaint += Prt().Pos();
    }

    SwRect aGrfArea( Frm() );
    SwRect aPaintArea( aGrfArea );

    // In case the picture fly frm was clipped, render it with the origin
    // size instead of scaling it
    if ( rNoTNd.getIDocumentSettingAccess()->get( IDocumentSettingAccess::CLIPPED_PICTURES ) )
    {
        const SwFlyFreeFrm *pFly = dynamic_cast< const SwFlyFreeFrm* >( FindFlyFrm() );
        if( pFly )
            aGrfArea = SwRect( Frm().Pos( ), pFly->GetUnclippedFrm( ).SSize( ) );
    }

    aPaintArea._Intersection( aOrigPaint );

    SwRect aNormal( Frm().Pos() + Prt().Pos(), Prt().SSize() );
    aNormal.Justify(); // Normalized rectangle for the comparisons

    if( aPaintArea.IsOver( aNormal ) )
    {
        // Calculate the four to-be-deleted rectangles
        if( pSh->GetWin() )
            ::lcl_ClearArea( *this, *pSh->GetOut(), aPaintArea, aNormal );

        // The intersection of the PaintArea and the Bitmap contains the absolutely visible area of the Frame
        aPaintArea._Intersection( aNormal );

        if ( bClip )
            pOut->IntersectClipRegion( aPaintArea.SVRect() );
        /// delete unused 3rd parameter
        PaintPicture( pOut, aGrfArea );
    }
    else
        // If it's not visible, simply delete the given Area
        lcl_ClearArea( *this, *pSh->GetOut(), aPaintArea, SwRect() );
    if( pGrfNd )
        pGrfNd->SetFrameInPaint( sal_False );

    pOut->Pop();
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
void SwNoTxtFrm::GetGrfArea( SwRect &rRect, SwRect* pOrigRect,
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
        if( !(FindPageFrm()->GetVirtPageNum() % 2) )
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
    Size aOrigSz( ((SwNoTxtNode*)GetNode())->GetTwipSize() );
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
    Point aVisPt( Frm().Pos() + Prt().Pos() );
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
const Size& SwNoTxtFrm::GetSize() const
{
    // Return the Frame's size
    const SwFrm *pFly = FindFlyFrm();
    if( !pFly )
        pFly = this;
    return pFly->Prt().SSize();
}

void SwNoTxtFrm::MakeAll()
{
    SwCntntNotify aNotify( this );
    SwBorderAttrAccess aAccess( SwFrm::GetCache(), this );
    const SwBorderAttrs &rAttrs = *aAccess.Get();

    while ( !mbValidPos || !mbValidSize || !mbValidPrtArea )
    {
        MakePos();

        if ( !mbValidSize )
            Frm().Width( GetUpper()->Prt().Width() );

        MakePrtArea( rAttrs );

        if ( !mbValidSize )
        {   mbValidSize = sal_True;
            Format();
        }
    }
}

/** Calculate the Bitmap's site, if needed */
void SwNoTxtFrm::Format( const SwBorderAttrs * )
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

sal_Bool SwNoTxtFrm::GetCharRect( SwRect &rRect, const SwPosition& rPos,
                              SwCrsrMoveState *pCMS ) const
{
    if ( &rPos.nNode.GetNode() != (SwNode*)GetNode() )
        return sal_False;

    Calc();
    SwRect aFrameRect( Frm() );
    rRect = aFrameRect;
    rRect.Pos( Frm().Pos() + Prt().Pos() );
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
        if ( pCMS->bRealHeight )
        {
            pCMS->aRealHeight.setY(rRect.Height());
            pCMS->aRealHeight.setX(0);
        }
    }

    return sal_True;
}

sal_Bool SwNoTxtFrm::GetCrsrOfst(SwPosition* pPos, Point& ,
                             SwCrsrMoveState*, bool ) const
{
    SwCntntNode* pCNd = (SwCntntNode*)GetNode();
    pPos->nNode = *pCNd;
    pPos->nContent.Assign( pCNd, 0 );
    return sal_True;
}

#define CLEARCACHE( pNd ) {\
    (pNd)->ReleaseGraphicFromCache();\
    SwFlyFrm* pFly = FindFlyFrm();\
    if( pFly && pFly->GetFmt()->GetSurround().IsContour() )\
    {\
        ClrContourCache( pFly->GetVirtDrawObj() );\
        pFly->NotifyBackground( FindPageFrm(), Prt(), PREP_FLY_ATTR_CHG );\
    }\
}

void SwNoTxtFrm::Modify( const SfxPoolItem* pOld, const SfxPoolItem* pNew )
{
    sal_uInt16 nWhich = pNew ? pNew->Which() : pOld ? pOld->Which() : 0;

    // #i73788#
    // no <SwCntntFrm::Modify(..)> for RES_LINKED_GRAPHIC_STREAM_ARRIVED
    if ( RES_GRAPHIC_PIECE_ARRIVED != nWhich &&
         RES_GRAPHIC_ARRIVED != nWhich &&
         RES_GRF_REREAD_AND_INCACHE != nWhich &&
         RES_LINKED_GRAPHIC_STREAM_ARRIVED != nWhich )
    {
        SwCntntFrm::Modify( pOld, pNew );
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
            SwGrfNode* pNd = (SwGrfNode*) GetNode();

            ViewShell *pVSh = 0;
            pNd->GetDoc()->GetEditShell( &pVSh );
            if( pVSh )
            {
                GraphicAttr aAttr;
                if( pNd->GetGrfObj().IsCached( pVSh->GetOut(), Point(),
                            Prt().SSize(), &pNd->GetGraphicAttr( aAttr, this ) ))
                {
                    ViewShell *pSh = pVSh;
                    do {
                        SET_CURR_SHELL( pSh );
                        if( pSh->GetWin() )
                        {
                            if( pSh->IsPreView() )
                                ::RepaintPagePreview( pSh, Frm().SVRect() );
                            else
                                pSh->GetWin()->Invalidate( Frm().SVRect() );
                        }
                    } while( pVSh != (pSh = (ViewShell*)pSh->GetNext() ));
                }
                else
                    pNd->SwapIn();
            }
        }
        break;

    case RES_UPDATE_ATTR:
    case RES_FMT_CHG:
        CLEARCACHE( (SwGrfNode*) GetNode() )
        break;

    case RES_ATTRSET_CHG:
        {
            sal_uInt16 n;
            for( n = RES_GRFATR_BEGIN; n < RES_GRFATR_END; ++n )
                if( SFX_ITEM_SET == ((SwAttrSetChg*)pOld)->GetChgSet()->
                                GetItemState( n, sal_False ))
                {
                    CLEARCACHE( (SwGrfNode*) GetNode() )
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
            SwGrfNode* pNd = (SwGrfNode*) GetNode();

            CLEARCACHE( pNd )

            SwRect aRect( Frm() );

            ViewShell *pVSh = 0;
            pNd->GetDoc()->GetEditShell( &pVSh );
            if( !pVSh )
                break;

            ViewShell *pSh = pVSh;
            do {
                SET_CURR_SHELL( pSh );
                if( pSh->IsPreView() )
                {
                    if( pSh->GetWin() )
                        ::RepaintPagePreview( pSh, aRect );
                }
                else if ( pSh->VisArea().IsOver( aRect ) &&
                   OUTDEV_WINDOW == pSh->GetOut()->GetOutDevType() )
                {
                    // invalidate instead of painting
                    pSh->GetWin()->Invalidate( aRect.SVRect() );
                }

                pSh = (ViewShell *)pSh->GetNext();
            } while( pSh != pVSh );
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

static void lcl_correctlyAlignRect( SwRect& rAlignedGrfArea, const SwRect& rInArea, OutputDevice* pOut )
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
    OutputDevice& rOutputDevice,
    const drawinglayer::primitive2d::Primitive2DSequence& rSequence,
    const basegfx::B2DRange& rSourceRange,
    const basegfx::B2DRange& rTargetRange,
    const sal_Int32 nLeftCrop = 0,
    const sal_Int32 nTopCrop = 0,
    const sal_Int32 nRightCrop = 0,
    const sal_Int32 nBottomCrop = 0,
    const bool bMirrorX = false,
    const bool bMirrorY = false)
{
    const double fSourceWidth(rSourceRange.getWidth());
    const double fSourceHeight(rSourceRange.getHeight());

    if(rSequence.hasElements() && !basegfx::fTools::equalZero(fSourceWidth) && !basegfx::fTools::equalZero(fSourceHeight))
    {
        // copy target range and apply evtl. cropping
        basegfx::B2DRange aTargetRange(rTargetRange);

        if(nLeftCrop || nTopCrop || nRightCrop || nBottomCrop)
        {
            // calculate original TargetRange
            const double fFactor100thmmToTwips(72.0 / 127.0);

            aTargetRange = basegfx::B2DRange(
                aTargetRange.getMinX() - (nLeftCrop * fFactor100thmmToTwips),
                aTargetRange.getMinY() - (nTopCrop * fFactor100thmmToTwips),
                aTargetRange.getMaxX() + (nRightCrop * fFactor100thmmToTwips),
                aTargetRange.getMaxY() + (nBottomCrop * fFactor100thmmToTwips));
        }

        const double fTargetWidth(aTargetRange.getWidth());
        const double fTargetHeight(aTargetRange.getHeight());

        if(!basegfx::fTools::equalZero(fTargetWidth) && !basegfx::fTools::equalZero(fTargetHeight))
        {
            // map graphic range to target range. This will automatically include
            // tme mapping from Svg 1/100th mm content to twips since the target
            // range is twips already
            basegfx::B2DHomMatrix aMappingTransform(
                basegfx::tools::createTranslateB2DHomMatrix(
                    -rSourceRange.getMinX(),
                    -rSourceRange.getMinY()));

            aMappingTransform.scale(fTargetWidth / fSourceWidth, fTargetHeight / fSourceHeight);
            aMappingTransform.translate(aTargetRange.getMinX(), aTargetRange.getMinY());

            // apply mirrorings
            if(bMirrorX || bMirrorY)
            {
                aMappingTransform.translate(-aTargetRange.getCenterX(), -aTargetRange.getCenterY());
                aMappingTransform.scale(bMirrorX ? -1.0 : 1.0, bMirrorY ? -1.0 : 1.0); // #119176# small typo with X/Y
                aMappingTransform.translate(aTargetRange.getCenterX(), aTargetRange.getCenterY());
            }

            // Fill ViewInformation. Use MappingTransform here, so there is no need to
            // embed the primitives to it. Use original TargetRange here so there is also
            // no need to embed the primitives to a MaskPrimitive for cropping. This works
            // only in this case where the graphic object cannot be rotated, though.
            const drawinglayer::geometry::ViewInformation2D aViewInformation2D(
                aMappingTransform,
                rOutputDevice.GetViewTransformation(),
                aTargetRange,
                0,
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

/** Paint the graphic.

    We require either a QuickDraw-Bitmap or a graphic here. If we do not have
    either, we return a replacement.

    @todo use aligned rectangle for drawing graphic.
    @todo pixel-align coordinations for drawing graphic. */
void SwNoTxtFrm::PaintPicture( OutputDevice* pOut, const SwRect &rGrfArea ) const
{
    ViewShell* pShell = getRootFrm()->GetCurrShell();

    SwNoTxtNode& rNoTNd = *(SwNoTxtNode*)GetNode();
    SwGrfNode* pGrfNd = rNoTNd.GetGrfNode();
    SwOLENode* pOLENd = rNoTNd.GetOLENode();

    const sal_Bool bPrn = pOut == rNoTNd.getIDocumentDeviceAccess()->getPrinter( false ) ||
                          pOut->GetConnectMetaFile();

    const bool bIsChart = pOLENd && ChartHelper::IsChart( pOLENd->GetOLEObj().GetObject() );

    // calculate aligned rectangle from parameter <rGrfArea>.
    //     Use aligned rectangle <aAlignedGrfArea> instead of <rGrfArea> in
    //     the following code.
    SwRect aAlignedGrfArea = rGrfArea;
    ::SwAlignRect( aAlignedGrfArea,  pShell );

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
        const sal_uInt16 nFormerAntialiasingAtOutput( pOut->GetAntialiasing() );
        if (pShell->Imp()->GetDrawView()->IsAntiAliasing())
        {
            pOut->SetAntialiasing( nFormerAntialiasingAtOutput | ANTIALIASING_ENABLE_B2DDRAW );
        }

        bool bForceSwap = false, bContinue = true;
        const GraphicObject& rGrfObj = pGrfNd->GetGrfObj();

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
                    !aTmpSz.Height() || !pGrfNd->GetAutoFmtLvl() )
                {
                    pGrfNd->TriggerAsyncRetrieveInputStream(); // #i73788#
                }
                String aTxt( pGrfNd->GetTitle() );
                if ( !aTxt.Len() )
                    GetRealURL( *pGrfNd, aTxt );
                ::lcl_PaintReplacement( aAlignedGrfArea, aTxt, *pShell, this, false );
                bContinue = false;
            }
            else if( rGrfObj.IsCached( pOut, aAlignedGrfArea.Pos(),
                                    aAlignedGrfArea.SSize(), &aGrfAttr ))
            {
                pGrfNd->DrawGraphicWithPDFHandling(*pOut,
                    aAlignedGrfArea.Pos(), aAlignedGrfArea.SSize(),
                    &aGrfAttr );
                bContinue = false;
            }
        }

        if( bContinue )
        {
            const sal_Bool bSwapped = rGrfObj.IsSwappedOut();
            const bool bSwappedIn = 0 != pGrfNd->SwapIn( bPrn );
            if( bSwappedIn && rGrfObj.GetGraphic().IsSupportedGraphic())
            {
                const bool bAnimate = rGrfObj.IsAnimated() &&
                                         !pShell->IsPreView() &&
                                         !pShell->GetAccessibilityOptions()->IsStopAnimatedGraphics() &&
                // #i9684# Stop animation during printing/pdf export
                                          pShell->GetWin();

                if( bAnimate &&
                    FindFlyFrm() != ::GetFlyFromMarked( 0, pShell ))
                {
                    OutputDevice* pVout;
                    if( pOut == pShell->GetOut() && SwRootFrm::FlushVout() )
                        pVout = pOut, pOut = pShell->GetOut();
                    else if( pShell->GetWin() &&
                             OUTDEV_VIRDEV == pOut->GetOutDevType() )
                        pVout = pOut, pOut = pShell->GetWin();
                    else
                        pVout = 0;

                    OSL_ENSURE( OUTDEV_VIRDEV != pOut->GetOutDevType() ||
                            pShell->GetViewOptions()->IsPDFExport(),
                            "pOut should not be a virtual device" );

                    pGrfNd->StartGraphicAnimation(pOut, aAlignedGrfArea.Pos(),
                                        aAlignedGrfArea.SSize(), sal_IntPtr(this),
                                        0, GRFMGR_DRAW_STANDARD, pVout );
                }
                else
                {
                    const SvgDataPtr& rSvgDataPtr = rGrfObj.GetGraphic().getSvgData();
                    bool bDone(false);

                    if(rSvgDataPtr.get())
                    {
                        // Graphic is Svg and can be painted as primitives (vector graphic)
                        const basegfx::B2DRange aTargetRange(
                            aAlignedGrfArea.Left(), aAlignedGrfArea.Top(),
                            aAlignedGrfArea.Right(), aAlignedGrfArea.Bottom());
                        const bool bCropped(aGrfAttr.IsCropped());
                        drawinglayer::primitive2d::Primitive2DSequence aContent;
                        GraphicAttr aSuppressGraphicAttr(aGrfAttr);

                        aSuppressGraphicAttr.SetCrop(0, 0, 0, 0);
                        aSuppressGraphicAttr.SetRotation(0);
                        aSuppressGraphicAttr.SetMirrorFlags(0);

                        const bool bNeedTransformedGraphic(
                            aSuppressGraphicAttr.IsSpecialDrawMode() ||
                            aSuppressGraphicAttr.IsAdjusted() ||
                            aSuppressGraphicAttr.IsMirrored() ||
                            aSuppressGraphicAttr.IsRotated() ||
                            aSuppressGraphicAttr.IsTransparent());

                        if(bNeedTransformedGraphic)
                        {
                            // #i122039# need to apply graphic transformation if GraphicAttr are used qwhich need this
                            const Graphic aTransformedGraphic(rGrfObj.GetTransformedGraphic(&aSuppressGraphicAttr));
                            const basegfx::B2DRange aRange(rSvgDataPtr->getRange());
                            const basegfx::B2DHomMatrix aTransform(
                                basegfx::tools::createScaleTranslateB2DHomMatrix(
                                    aRange.getRange(),
                                    aRange.getMinimum()));

                            aContent.realloc(1);
                            aContent[0] = new drawinglayer::primitive2d::BitmapPrimitive2D(
                                aTransformedGraphic.GetBitmapEx(),
                                aTransform);
                        }
                        else
                        {
                            aContent = rSvgDataPtr->getPrimitive2DSequence();
                        }

                        bDone = paintUsingPrimitivesHelper(
                            *pOut,
                            aContent,
                            rSvgDataPtr->getRange(),
                            aTargetRange,
                            bCropped ? aGrfAttr.GetLeftCrop() : 0,
                            bCropped ? aGrfAttr.GetTopCrop() : 0,
                            bCropped ? aGrfAttr.GetRightCrop() : 0,
                            bCropped ? aGrfAttr.GetBottomCrop() : 0,
                            aGrfAttr.GetMirrorFlags() & BMP_MIRROR_HORZ,
                            aGrfAttr.GetMirrorFlags() & BMP_MIRROR_VERT);
                    }

                    if(!bDone)
                    {
                        // fallback paint, uses replacement image
                        pGrfNd->DrawGraphicWithPDFHandling(*pOut, aAlignedGrfArea.Pos(), aAlignedGrfArea.SSize(), &aGrfAttr);
                    }
                }
            }
            else
            {
                sal_uInt16 nResId = 0;
                if( bSwappedIn )
                {
                    if( GRAPHIC_NONE == rGrfObj.GetType() )
                        nResId = STR_COMCORE_READERROR;
                    else if ( !rGrfObj.GetGraphic().IsSupportedGraphic() )
                        nResId = STR_COMCORE_CANT_SHOW;
                }
                ((SwNoTxtFrm*)this)->nWeight = -1;
                String aText;
                if ( !nResId &&
                     !(aText = pGrfNd->GetTitle()).Len() &&
                     (!GetRealURL( *pGrfNd, aText ) || !aText.Len()))
                {
                    nResId = STR_COMCORE_READERROR;
                }
                if ( nResId )
                    aText = SW_RESSTR( nResId );

                ::lcl_PaintReplacement( aAlignedGrfArea, aText, *pShell, this, true );
            }

            // When printing, we must not collect the graphics
            if( bSwapped && bPrn )
                bForceSwap = true;
        }

        if( bForceSwap )
            pGrfNd->SwapOut();

        if ( pShell->Imp()->GetDrawView()->IsAntiAliasing() )
            pOut->SetAntialiasing( nFormerAntialiasingAtOutput );
    }
    else // bIsChart || pOLENd
    {
        // Fix for bug fdo#33781
        const sal_uInt16 nFormerAntialiasingAtOutput( pOut->GetAntialiasing() );
        if (pShell->Imp()->GetDrawView()->IsAntiAliasing())
        {
            sal_uInt16 nNewAntialiasingAtOutput = nFormerAntialiasingAtOutput | ANTIALIASING_ENABLE_B2DDRAW;

            // #i99665#
            // Adjust AntiAliasing mode at output device for chart OLE
            if ( pOLENd->IsChart() )
                nNewAntialiasingAtOutput |= ANTIALIASING_PIXELSNAPHAIRLINE;

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
            if ( !bPrn && pShell->ISA( SwCrsrShell ) &&
                    nMiscStatus & embed::EmbedMisc::MS_EMBED_ACTIVATEWHENVISIBLE )
            {
                const SwFlyFrm *pFly = FindFlyFrm();
                assert( pFly != NULL );
                ((SwFEShell*)pShell)->ConnectObj( pOLENd->GetOLEObj().GetObject(), pFly->Prt(), pFly->Frm());
            }
        }

        // see #i99665#
        if (pShell->Imp()->GetDrawView()->IsAntiAliasing())
        {
            pOut->SetAntialiasing( nFormerAntialiasingAtOutput );
        }
    }
}

sal_Bool SwNoTxtFrm::IsTransparent() const
{
    const ViewShell* pSh = getRootFrm()->GetCurrShell();
    if ( !pSh || !pSh->GetViewOptions()->IsGraphic() )
        return sal_True;

    const SwGrfNode *pNd;
    if( 0 != (pNd = GetNode()->GetGrfNode()) )
        return pNd->IsTransparent();

    //#29381# OLE are always transparent
    return sal_True;
}

void SwNoTxtFrm::StopAnimation( OutputDevice* pOut ) const
{
    // Stop animated graphics
    const SwGrfNode* pGrfNd = dynamic_cast< const SwGrfNode* >(GetNode()->GetGrfNode());

    if( pGrfNd && pGrfNd->IsAnimated() )
    {
        const_cast< SwGrfNode* >(pGrfNd)->StopGraphicAnimation( pOut, sal_IntPtr(this) );
    }
}

sal_Bool SwNoTxtFrm::HasAnimation() const
{
    const SwGrfNode* pGrfNd = GetNode()->GetGrfNode();
    return pGrfNd && pGrfNd->IsAnimated();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
