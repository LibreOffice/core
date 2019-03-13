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
#include <vcl/imapobj.hxx>
#include <vcl/imap.hxx>
#include <svl/urihelper.hxx>
#include <svtools/soerr.hxx>
#include <sfx2/progress.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/printer.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/boxitem.hxx>
#include <svx/xoutbmp.hxx>
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
#include <strings.hrc>
#include <accessibilityoptions.hxx>
#include <com/sun/star/embed/EmbedMisc.hpp>
#include <com/sun/star/embed/EmbedStates.hpp>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <svtools/embedhlp.hxx>
#include <dview.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <drawinglayer/processor2d/baseprocessor2d.hxx>
#include <drawinglayer/primitive2d/graphicprimitive2d.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/utils/b2dclipstate.hxx>
#include <drawinglayer/processor2d/processor2dtools.hxx>
#include <txtfly.hxx>
#include <vcl/graphicfilter.hxx>
#include <vcl/pdfextoutdevdata.hxx>
#include <drawinglayer/primitive2d/maskprimitive2d.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <drawinglayer/primitive2d/objectinfoprimitive2d.hxx>

using namespace com::sun::star;

static bool GetRealURL( const SwGrfNode& rNd, OUString& rText )
{
    bool bRet = rNd.GetFileFilterNms( &rText, nullptr );
    if( bRet )
        rText = URIHelper::removePassword( rText, INetURLObject::EncodeMechanism::WasEncoded,
                                           INetURLObject::DecodeMechanism::Unambiguous);
    if (rText.startsWith("data:image")) rText = "inline image";

    return bRet;
}

static void lcl_PaintReplacement( const SwRect &rRect, const OUString &rText,
                           const SwViewShell &rSh, const SwNoTextFrame *pFrame,
                           bool bDefect )
{
    static vcl::Font aFont = [&]()
    {
        vcl::Font tmp;
        tmp.SetWeight( WEIGHT_BOLD );
        tmp.SetStyleName( OUString() );
        tmp.SetFamilyName("Arial Unicode");
        tmp.SetFamily( FAMILY_SWISS );
        tmp.SetTransparent( true );
        return tmp;
    }();

    Color aCol( COL_RED );
    FontLineStyle eUnderline = LINESTYLE_NONE;
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

    aFont.SetUnderline( eUnderline );
    aFont.SetColor( aCol );

    const BitmapEx& rBmp = const_cast<SwViewShell&>(rSh).GetReplacementBitmap(bDefect);
    Graphic::DrawEx( rSh.GetOut(), rText, aFont, rBmp, rRect.Pos(), rRect.SSize() );
}

SwNoTextFrame::SwNoTextFrame(SwNoTextNode * const pNode, SwFrame* pSib )
:   SwContentFrame( pNode, pSib ),
    // RotateFlyFrame3
    mpTransformableSwFrame()
{
    mnFrameType = SwFrameType::NoTxt;
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
        const SvxBrushItem *pItem;
        const Color *pCol;
        SwRect aOrigRect;
        drawinglayer::attribute::SdrAllFillAttributesHelperPtr aFillAttributes;

        if ( rFrame.GetBackgroundBrush( aFillAttributes, pItem, pCol, aOrigRect, false, /*bConsiderTextBox=*/false ) )
        {
            SwRegionRects const region(rPtArea);
            basegfx::utils::B2DClipState aClipState;
            const bool bDone(::DrawFillAttributes(aFillAttributes, aOrigRect, region, aClipState, rOut));

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

void SwNoTextFrame::PaintSwFrame(vcl::RenderContext& rRenderContext, SwRect const& rRect, SwPrintData const*const) const
{
    if ( getFrameArea().IsEmpty() )
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
            lcl_PaintReplacement( getFrameArea(), aText, *pSh, this, false );
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
        aOrigPaint = getFrameArea(); aOrigPaint += getFramePrintArea().Pos();
    }

    SwRect aGrfArea( getFrameArea() );
    SwRect aPaintArea( aGrfArea );

    // In case the picture fly frm was clipped, render it with the origin
    // size instead of scaling it
    if ( pGrfNd && rNoTNd.getIDocumentSettingAccess()->get( DocumentSettingId::CLIPPED_PICTURES ) )
    {
        const SwFlyFreeFrame *pFly = dynamic_cast< const SwFlyFreeFrame* >( FindFlyFrame() );
        if( pFly )
        {
            bool bGetUnclippedFrame=true;
            const SfxPoolItem* pItem;
            if( pFly->GetFormat() && SfxItemState::SET == pFly->GetFormat()->GetItemState(RES_BOX, false, &pItem) )
            {
                const SvxBoxItem& rBox = *static_cast<const SvxBoxItem*>(pItem);
                if( rBox.HasBorder( /*bTreatPaddingAsBorder*/true) )
                    bGetUnclippedFrame = false;
            }

            if( bGetUnclippedFrame )
                aGrfArea = SwRect( getFrameArea().Pos( ), pFly->GetUnclippedFrame( ).SSize( ) );
        }
    }

    aPaintArea.Intersection_( aOrigPaint );

    SwRect aNormal( getFrameArea().Pos() + getFramePrintArea().Pos(), getFramePrintArea().SSize() );
    aNormal.Justify(); // Normalized rectangle for the comparisons

    if( aPaintArea.IsOver( aNormal ) )
    {
        // Calculate the four to-be-deleted rectangles
        if( pSh->GetWin() )
            ::lcl_ClearArea( *this, rRenderContext, aPaintArea, aNormal );

        // The intersection of the PaintArea and the Bitmap contains the absolutely visible area of the Frame
        aPaintArea.Intersection_( aNormal );

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
static void lcl_CalcRect( Point& rPt, Size& rDim, MirrorGraph nMirror )
{
    if( nMirror == MirrorGraph::Vertical || nMirror == MirrorGraph::Both )
    {
        rPt.setX(rPt.getX() + rDim.Width() -1);
        rDim.setWidth( -rDim.Width() );
    }

    if( nMirror == MirrorGraph::Horizontal || nMirror == MirrorGraph::Both )
    {
        rPt.setY(rPt.getY() + rDim.Height() -1);
        rDim.setHeight( -rDim.Height() );
    }
}

/** Calculate the Bitmap's position and the size within the passed rectangle */
void SwNoTextFrame::GetGrfArea( SwRect &rRect, SwRect* pOrigRect ) const
{
    // Currently only used for scaling, cropping and mirroring the contour of graphics!
    // Everything else is handled by GraphicObject
    // We put the graphic's visible rectangle into rRect.
    // pOrigRect contains position and size of the whole graphic.

    // RotateFlyFrame3: SwFrame may be transformed. Get untransformed
    // SwRect(s) as base of calculation
    const TransformableSwFrame* pTransformableSwFrame(getTransformableSwFrame());
    const SwRect aFrameArea(pTransformableSwFrame ? pTransformableSwFrame->getUntransformedFrameArea() : getFrameArea());
    const SwRect aFramePrintArea(pTransformableSwFrame ? pTransformableSwFrame->getUntransformedFramePrintArea() : getFramePrintArea());

    const SwAttrSet& rAttrSet = GetNode()->GetSwAttrSet();
    const SwCropGrf& rCrop = rAttrSet.GetCropGrf();
    MirrorGraph nMirror = rAttrSet.GetMirrorGrf().GetValue();

    if( rAttrSet.GetMirrorGrf().IsGrfToggle() )
    {
        if( !(FindPageFrame()->GetVirtPageNum() % 2) )
        {
            switch ( nMirror )
            {
                case MirrorGraph::Dont: nMirror = MirrorGraph::Vertical; break;
                case MirrorGraph::Vertical: nMirror = MirrorGraph::Dont; break;
                case MirrorGraph::Horizontal: nMirror = MirrorGraph::Both; break;
                default: nMirror = MirrorGraph::Horizontal; break;
            }
        }
    }

    // We read graphic from the Node, if needed.
    // It may fail, however.
    long nLeftCrop, nRightCrop, nTopCrop, nBottomCrop;
    Size aOrigSz( static_cast<const SwNoTextNode*>(GetNode())->GetTwipSize() );
    if ( !aOrigSz.Width() )
    {
        aOrigSz.setWidth( aFramePrintArea.Width() );
        nLeftCrop  = -rCrop.GetLeft();
        nRightCrop = -rCrop.GetRight();
    }
    else
    {
        nLeftCrop = std::max( aOrigSz.Width() -
                            (rCrop.GetRight() + rCrop.GetLeft()), long(1) );
        const double nScale = double(aFramePrintArea.Width())  / double(nLeftCrop);
        nLeftCrop  = long(nScale * -rCrop.GetLeft() );
        nRightCrop = long(nScale * -rCrop.GetRight() );
    }

    // crop values have to be mirrored too
    if( nMirror == MirrorGraph::Vertical || nMirror == MirrorGraph::Both )
    {
        long nTmpCrop = nLeftCrop;
        nLeftCrop = nRightCrop;
        nRightCrop= nTmpCrop;
    }

    if( !aOrigSz.Height() )
    {
        aOrigSz.setHeight( aFramePrintArea.Height() );
        nTopCrop   = -rCrop.GetTop();
        nBottomCrop= -rCrop.GetBottom();
    }
    else
    {
        nTopCrop = std::max( aOrigSz.Height() - (rCrop.GetTop() + rCrop.GetBottom()), long(1) );
        const double nScale = double(aFramePrintArea.Height()) / double(nTopCrop);
        nTopCrop   = long(nScale * -rCrop.GetTop() );
        nBottomCrop= long(nScale * -rCrop.GetBottom() );
    }

    // crop values have to be mirrored too
    if( nMirror == MirrorGraph::Horizontal || nMirror == MirrorGraph::Both )
    {
        long nTmpCrop = nTopCrop;
        nTopCrop   = nBottomCrop;
        nBottomCrop= nTmpCrop;
    }

    Size  aVisSz( aFramePrintArea.SSize() );
    Size  aGrfSz( aVisSz );
    Point aVisPt( aFrameArea.Pos() + aFramePrintArea.Pos() );
    Point aGrfPt( aVisPt );

    // Set the "visible" rectangle first
    if ( nLeftCrop > 0 )
    {
        aVisPt.setX(aVisPt.getX() + nLeftCrop);
        aVisSz.AdjustWidth( -nLeftCrop );
    }
    if ( nTopCrop > 0 )
    {
        aVisPt.setY(aVisPt.getY() + nTopCrop);
        aVisSz.AdjustHeight( -nTopCrop );
    }
    if ( nRightCrop > 0 )
        aVisSz.AdjustWidth( -nRightCrop );
    if ( nBottomCrop > 0 )
        aVisSz.AdjustHeight( -nBottomCrop );

    rRect.Pos  ( aVisPt );
    rRect.SSize( aVisSz );

    // Calculate the whole graphic if needed
    if ( pOrigRect )
    {
        Size aTmpSz( aGrfSz );
        aGrfPt.setX(aGrfPt.getX() + nLeftCrop);
        aTmpSz.AdjustWidth( -(nLeftCrop + nRightCrop) );
        aGrfPt.setY(aGrfPt.getY() + nTopCrop);
        aTmpSz.AdjustHeight( -(nTopCrop + nBottomCrop) );

        if( MirrorGraph::Dont != nMirror )
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
    return pFly->getFramePrintArea().SSize();
}

void SwNoTextFrame::MakeAll(vcl::RenderContext* pRenderContext)
{
    // RotateFlyFrame3 - inner frame. Get rotation and check if used
    const double fRotation(getLocalFrameRotation());
    const bool bRotated(!basegfx::fTools::equalZero(fRotation));

    if(bRotated)
    {
        SwFlyFreeFrame* pUpperFly(dynamic_cast< SwFlyFreeFrame* >(GetUpper()));

        if(pUpperFly)
        {
            if(!pUpperFly->isFrameAreaDefinitionValid())
            {
                // RotateFlyFrame3: outer frame *needs* to be layouted first, force this by calling
                // it's ::Calc directly
                pUpperFly->Calc(pRenderContext);
            }

            // Reset outer frame to unrotated state. This is necessary to make the
            // layouting below work as currently implemented in Writer. As expected
            // using Transformations allows to do this on the fly due to all information
            // being included there.
            // The full solution would be to adapt the whole layouting
            // process of Writer to take care of Transformations, but that
            // is currently beyond scope
            if(pUpperFly->isTransformableSwFrame())
            {
                pUpperFly->getTransformableSwFrame()->restoreFrameAreas();
            }
        }

        // Re-layout may be partially (see all isFrameAreaDefinitionValid() flags),
        // so resetting the local SwFrame(s) in the local SwFrameAreaDefinition is also
        // needed (e.g. for PrintPreview).
        // Reset to BoundAreas will be done below automatically
        if(isTransformableSwFrame())
        {
            getTransformableSwFrame()->restoreFrameAreas();
        }
    }

    SwContentNotify aNotify( this );
    SwBorderAttrAccess aAccess( SwFrame::GetCache(), this );
    const SwBorderAttrs &rAttrs = *aAccess.Get();

    while ( !isFrameAreaPositionValid() || !isFrameAreaSizeValid() || !isFramePrintAreaValid() )
    {
        MakePos();

        if ( !isFrameAreaSizeValid() )
        {
            SwFrameAreaDefinition::FrameAreaWriteAccess aFrm(*this);
            aFrm.Width( GetUpper()->getFramePrintArea().Width() );
        }

        MakePrtArea( rAttrs );

        if ( !isFrameAreaSizeValid() )
        {
            setFrameAreaSizeValid(true);
            Format(getRootFrame()->GetCurrShell()->GetOut());
        }
    }

    // RotateFlyFrame3 - inner frame
    if(bRotated)
    {
        SwFlyFreeFrame* pUpperFly(dynamic_cast< SwFlyFreeFrame* >(GetUpper()));

        if(pUpperFly)
        {
            // restore outer frame back to Transformed state, that means
            // set the SwFrameAreaDefinition(s) back to BoundAreas of
            // the transformed SwFrame. All needed information is part
            // of the already correctly created Transformations of the
            // upper frame, so it can bre re-created on the fly
            if(pUpperFly->isTransformableSwFrame())
            {
                pUpperFly->getTransformableSwFrame()->adaptFrameAreasToTransformations();
            }
        }

        // After the unrotated layout is finished, apply possible set rotation to it
        // get center from outer frame (layout frame) to be on the safe side
        const Point aCenter(GetUpper() ? GetUpper()->getFrameArea().Center() : getFrameArea().Center());
        const basegfx::B2DPoint aB2DCenter(aCenter.X(), aCenter.Y());

        if(!mpTransformableSwFrame)
        {
            mpTransformableSwFrame.reset(new TransformableSwFrame(*this));
        }

        getTransformableSwFrame()->createFrameAreaTransformations(
            fRotation,
            aB2DCenter);
        getTransformableSwFrame()->adaptFrameAreasToTransformations();
    }
    else
    {
        // reset transformations to show that they are not used
        mpTransformableSwFrame.reset();
    }
}

// RotateFlyFrame3 - Support for Transformations - outer frame
basegfx::B2DHomMatrix SwNoTextFrame::getFrameAreaTransformation() const
{
    if(isTransformableSwFrame())
    {
        // use pre-created transformation
        return getTransformableSwFrame()->getLocalFrameAreaTransformation();
    }

    // call parent
    return SwContentFrame::getFrameAreaTransformation();
}

basegfx::B2DHomMatrix SwNoTextFrame::getFramePrintAreaTransformation() const
{
    if(isTransformableSwFrame())
    {
        // use pre-created transformation
        return getTransformableSwFrame()->getLocalFramePrintAreaTransformation();
    }

    // call parent
    return SwContentFrame::getFramePrintAreaTransformation();
}

// RotateFlyFrame3 - Support for Transformations
void SwNoTextFrame::transform_translate(const Point& rOffset)
{
    // call parent - this will do the basic transform for SwRect(s)
    // in the SwFrameAreaDefinition
    SwContentFrame::transform_translate(rOffset);

    // check if the Transformations need to be adapted
    if(isTransformableSwFrame())
    {
        const basegfx::B2DHomMatrix aTransform(
            basegfx::utils::createTranslateB2DHomMatrix(
                rOffset.X(), rOffset.Y()));

        // transform using TransformableSwFrame
        getTransformableSwFrame()->transform(aTransform);
    }
}

// RotateFlyFrame3 - inner frame
// Check if we contain a SwGrfNode and get possible rotation from it
double SwNoTextFrame::getLocalFrameRotation() const
{
    const SwNoTextNode* pSwNoTextNode(nullptr != GetNode() ? GetNode()->GetNoTextNode() : nullptr);

    if(nullptr != pSwNoTextNode)
    {
        const SwGrfNode* pSwGrfNode(pSwNoTextNode->GetGrfNode());

        if(nullptr != pSwGrfNode)
        {
            const SwAttrSet& rSwAttrSet(pSwGrfNode->GetSwAttrSet());
            const SwRotationGrf& rSwRotationGrf(rSwAttrSet.GetRotationGrf());
            const double fRotate(static_cast< double >(-rSwRotationGrf.GetValue()) * (M_PI/1800.0));

            return basegfx::normalizeToRange(fRotate, F_2PI);
        }
    }

    // no rotation
    return 0.0;
}

/** Calculate the Bitmap's site, if needed */
void SwNoTextFrame::Format( vcl::RenderContext* /*pRenderContext*/, const SwBorderAttrs * )
{
    const Size aNewSize( GetSize() );

    // Did the height change?
    SwTwips nChgHght = IsVertical() ?
        static_cast<SwTwips>(aNewSize.Width() - getFramePrintArea().Width()) :
        static_cast<SwTwips>(aNewSize.Height() - getFramePrintArea().Height());
    if( nChgHght > 0)
        Grow( nChgHght );
    else if( nChgHght < 0)
        Shrink( std::min(getFramePrintArea().Height(), -nChgHght) );
}

bool SwNoTextFrame::GetCharRect( SwRect &rRect, const SwPosition& rPos,
                              SwCursorMoveState *pCMS, bool /*bAllowFarAway*/ ) const
{
    if ( &rPos.nNode.GetNode() != static_cast<SwNode const *>(GetNode()) )
        return false;

    Calc(getRootFrame()->GetCurrShell()->GetOut());
    SwRect aFrameRect( getFrameArea() );
    rRect = aFrameRect;
    rRect.Pos( getFrameArea().Pos() + getFramePrintArea().Pos() );
    rRect.SSize( getFramePrintArea().SSize() );

    rRect.Justify();

    // Is the Bitmap in the visible area at all?
    if( !aFrameRect.IsOver( rRect ) )
    {
        // If not, then the Cursor is on the Frame
        rRect = aFrameRect;
        rRect.Width( 1 );
    }
    else
        rRect.Intersection_( aFrameRect );

    if ( pCMS && pCMS->m_bRealHeight )
    {
        pCMS->m_aRealHeight.setY(rRect.Height());
        pCMS->m_aRealHeight.setX(0);
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

void SwNoTextFrame::ClearCache()
{
    SwFlyFrame* pFly = FindFlyFrame();
    if( pFly && pFly->GetFormat()->GetSurround().IsContour() )
    {
        ClrContourCache( pFly->GetVirtDrawObj() );
        pFly->NotifyBackground( FindPageFrame(), getFramePrintArea(), PREP_FLY_ATTR_CHG );
    }
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
        if( SwNodeType::Grf == GetNode()->GetNodeType() )
        {
            // TODO: Remove - due to GraphicObject refactoring
            bComplete = false;
        }
        break;

    case RES_UPDATE_ATTR:
        if (GetNode()->GetNodeType() != SwNodeType::Grf) {
            break;
        }
        [[fallthrough]];
    case RES_FMT_CHG:
        ClearCache();
        break;

    case RES_ATTRSET_CHG:
        {
            sal_uInt16 n;
            for( n = RES_GRFATR_BEGIN; n < RES_GRFATR_END; ++n )
                if( SfxItemState::SET == static_cast<const SwAttrSetChg*>(pOld)->GetChgSet()->
                                GetItemState( n, false ))
                {
                    ClearCache();

                    if(RES_GRFATR_ROTATION == n)
                    {
                        // RotGrfFlyFrame: Update Handles in view, these may be rotation-dependent
                        // (e.g. crop handles) and need a visualisation update
                        if ( GetNode()->GetNodeType() == SwNodeType::Grf )
                        {
                            SwGrfNode* pNd = static_cast<SwGrfNode*>( GetNode());
                            SwViewShell *pVSh = pNd->GetDoc()->getIDocumentLayoutAccess().GetCurrentViewShell();

                            if(pVSh)
                            {
                                SdrView* pDrawView = pVSh->GetDrawView();

                                if(pDrawView)
                                {
                                    pDrawView->AdjustMarkHdl(nullptr);
                                }
                            }

                            // RotateFlyFrame3 - invalidate needed for ContentFrame (inner, this)
                            // and LayoutFrame (outer, GetUpper). It is possible to only invalidate
                            // the outer frame, but that leads to an in-between state that gets
                            // potentially painted
                            if(GetUpper())
                            {
                                GetUpper()->InvalidateAll_();
                            }

                            InvalidateAll_();
                        }
                    }
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
        if ( GetNode()->GetNodeType() == SwNodeType::Grf )
        {
            bComplete = false;
            SwGrfNode* pNd = static_cast<SwGrfNode*>( GetNode());

            ClearCache();

            SwRect aRect( getFrameArea() );

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

static void lcl_correctlyAlignRect( SwRect& rAlignedGrfArea, const SwRect& rInArea, vcl::RenderContext const * pOut )
{

    if(!pOut)
        return;
    tools::Rectangle aPxRect = pOut->LogicToPixel( rInArea.SVRect() );
    tools::Rectangle aNewPxRect( aPxRect );
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

static bool paintUsingPrimitivesHelper(
    vcl::RenderContext& rOutputDevice,
    const drawinglayer::primitive2d::Primitive2DContainer& rSequence,
    const basegfx::B2DRange& rSourceRange,
    const basegfx::B2DRange& rTargetRange)
{
    if(!rSequence.empty() && !basegfx::fTools::equalZero(rSourceRange.getWidth()) && !basegfx::fTools::equalZero(rSourceRange.getHeight()))
    {
        if(!basegfx::fTools::equalZero(rTargetRange.getWidth()) && !basegfx::fTools::equalZero(rTargetRange.getHeight()))
        {
            // map graphic range to target range. This will e.g. automatically include
            // the mapping from 1/100th mm content to twips if needed when the target
            // range is defined in twips
            const basegfx::B2DHomMatrix aMappingTransform(
                basegfx::utils::createSourceRangeTargetRangeTransform(
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
            std::unique_ptr<drawinglayer::processor2d::BaseProcessor2D> pProcessor2D(
                drawinglayer::processor2d::createProcessor2DFromOutputDevice(
                                                rOutputDevice, aViewInformation2D) );
            if(pProcessor2D)
            {
                // render and cleanup
                pProcessor2D->process(rSequence);
                return true;
            }
        }
    }

    return false;
}

void paintGraphicUsingPrimitivesHelper(
    vcl::RenderContext & rOutputDevice,
    GraphicObject const& rGrfObj,
    GraphicAttr const& rGraphicAttr,
    const basegfx::B2DHomMatrix& rGraphicTransform,
    const OUString& rName,
    const OUString& rTitle,
    const OUString& rDescription)
{
    // RotGrfFlyFrame: unify using GraphicPrimitive2D
    // -> the primitive handles all crop and mirror stuff
    // -> the primitive renderer will create the needed pdf export data
    // -> if bitmap content, it will be cached system-dependent
    drawinglayer::primitive2d::Primitive2DContainer aContent(1);

    aContent[0] = new drawinglayer::primitive2d::GraphicPrimitive2D(
        rGraphicTransform,
        rGrfObj,
        rGraphicAttr);

    // RotateFlyFrame3: If ClipRegion is set at OutputDevice, we
    // need to use that. Usually the renderer would be a VCL-based
    // PrimitiveRenderer, but there are system-specific shortcuts that
    // will *not* use the VCL-Paint of Bitmap and thus ignore this.
    // Anyways, indirectly using a CLipRegion set at the target OutDev
    // when using a PrimitiveRenderer is a non-valid implication.
    // First tried only to use when HasPolyPolygonOrB2DPolyPolygon(),
    // but there is an optimization at ClipRegion creation that detects
    // a single Rectangle in a tools::PolyPolygon and forces to a simple
    // RegionBand-based implementation, so cannot use it here.
    if(rOutputDevice.IsClipRegion())
    {
        const basegfx::B2DPolyPolygon aClip(rOutputDevice.GetClipRegion().GetAsB2DPolyPolygon());

        if(0 != aClip.count())
        {
            // tdf#114076: Expand ClipRange to next PixelBound
            // Do this by going to basegfx::B2DRange, adding a
            // single pixel size and using floor/ceil to go to
            // full integer (as needed for pixels). Also need
            // to go back to basegfx::B2DPolyPolygon for the
            // creation of the needed MaskPrimitive2D.
            // The general problem is that Writer is scrolling
            // using blitting the unchanged parts, this forces
            // this part of the scroll to pixel coordinate steps,
            // while the ViewTransformation for paint nowadays has
            // a sub-pixel precision. This results in an offset
            // up to one pixel in radius. To solve this for now,
            // we need to expand to the next outer pixel bound.
            // Hopefully in the future we will someday be able to
            // stay on the full available precision, but this
            // will need a change in the repaint/scroll paradigm.
            const basegfx::B2DRange aClipRange(aClip.getB2DRange());
            const basegfx::B2DVector aSinglePixelXY(rOutputDevice.GetInverseViewTransformation() * basegfx::B2DVector(1.0, 1.0));
            const basegfx::B2DRange aExpandedClipRange(
                floor(aClipRange.getMinX() - aSinglePixelXY.getX()),
                floor(aClipRange.getMinY() - aSinglePixelXY.getY()),
                ceil(aClipRange.getMaxX() + aSinglePixelXY.getX()),
                ceil(aClipRange.getMaxY() + aSinglePixelXY.getY()));

            aContent[0] = new drawinglayer::primitive2d::MaskPrimitive2D(
                basegfx::B2DPolyPolygon(
                    basegfx::utils::createPolygonFromRect(aExpandedClipRange)),
                aContent);
        }
    }

    if(!rName.isEmpty() || !rTitle.isEmpty() || !rDescription.isEmpty())
    {
        // Embed to ObjectInfoPrimitive2D when we have Name/Title/Description
        // information available
        aContent[0] = new drawinglayer::primitive2d::ObjectInfoPrimitive2D(
            aContent,
            rName,
            rTitle,
            rDescription);
    }

    basegfx::B2DRange aTargetRange(0.0, 0.0, 1.0, 1.0);
    aTargetRange.transform(rGraphicTransform);

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
            else if ( ( rGrfObj.GetType() == GraphicType::Default ||
                        rGrfObj.GetType() == GraphicType::NONE ) &&
                      pGrfNd->IsLinkedFile() &&
                      pGrfNd->IsAsyncRetrieveInputStreamPossible() )
            {
                Size aTmpSz;
                ::sfx2::SvLinkSource* pGrfObj = pGrfNd->GetLink()->GetObj();
                if( !pGrfObj ||
                    !pGrfObj->IsDataComplete() ||
                    !(aTmpSz = pGrfNd->GetTwipSize()).Width() ||
                    !aTmpSz.Height())
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
                    else if( pShell->GetWin() && pOut->IsVirtual() )
                    {
                        pVout = pOut;
                        pOut = pShell->GetWin();
                    }
                    else
                        pVout = nullptr;

                    OSL_ENSURE( !pOut->IsVirtual() ||
                            pShell->GetViewOptions()->IsPDFExport() || pShell->isOutputToWindow(),
                            "pOut should not be a virtual device" );

                    pGrfNd->StartGraphicAnimation(pOut, aAlignedGrfArea.Pos(),
                                        aAlignedGrfArea.SSize(), sal_IntPtr(this),
                                        pVout );
                }
                else
                {
                    const basegfx::B2DHomMatrix aGraphicTransform(getFrameAreaTransformation());

                    paintGraphicUsingPrimitivesHelper(
                        *pOut,
                        rGrfObj,
                        aGrfAttr,
                        aGraphicTransform,
                        nullptr == pGrfNd->GetFlyFormat() ? OUString() : pGrfNd->GetFlyFormat()->GetName(),
                        rNoTNd.GetTitle(),
                        rNoTNd.GetDescription());
                }
            }
            else
            {
                const char* pResId = nullptr;

                if( GraphicType::NONE == rGrfObj.GetType() )
                    pResId = STR_COMCORE_READERROR;
                else if ( !rGrfObj.GetGraphic().IsSupportedGraphic() )
                    pResId = STR_COMCORE_CANT_SHOW;

                OUString aText;
                if ( !pResId &&
                     (aText = pGrfNd->GetTitle()).isEmpty() &&
                     (!GetRealURL( *pGrfNd, aText ) || aText.isEmpty()))
                {
                    pResId = STR_COMCORE_READERROR;
                }
                if (pResId)
                    aText = SwResId(pResId);

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
            basegfx::B2DRange aSourceRange;
            const drawinglayer::primitive2d::Primitive2DContainer aSequence(
                pOLENd->GetOLEObj().tryToGetChartContentAsPrimitive2DSequence(
                    aSourceRange,
                    bPrn));

            if(!aSequence.empty() && !aSourceRange.isEmpty())
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

        if(!bDone && pOLENd)
        {
            // SwOLENode does not have a known GraphicObject, need to
            // work with Graphic instead
            const Graphic* pGraphic = pOLENd->GetGraphic();

            if ( pGraphic && pGraphic->GetType() != GraphicType::NONE )
            {
                GraphicObject aTempGraphicObject(*pGraphic);
                GraphicAttr aGrfAttr;
                const basegfx::B2DHomMatrix aGraphicTransform(
                    basegfx::utils::createScaleTranslateB2DHomMatrix(
                        aAlignedGrfArea.Width(), aAlignedGrfArea.Height(),
                        aAlignedGrfArea.Left(), aAlignedGrfArea.Top()));

                paintGraphicUsingPrimitivesHelper(
                    *pOut,
                    aTempGraphicObject,
                    aGrfAttr,
                    aGraphicTransform,
                    nullptr == pOLENd->GetFlyFormat() ? OUString() : pOLENd->GetFlyFormat()->GetName(),
                    rNoTNd.GetTitle(),
                    rNoTNd.GetDescription());

                // shade the representation if the object is activated outplace
                uno::Reference < embed::XEmbeddedObject > xObj = pOLENd->GetOLEObj().GetOleRef();
                if ( xObj.is() && xObj->getCurrentState() == embed::EmbedStates::ACTIVE )
                {
                    const Point aPosition(aAlignedGrfArea.Pos());
                    const Size aSize(aAlignedGrfArea.SSize());

                    ::svt::EmbeddedObjectRef::DrawShading(
                        tools::Rectangle(
                            aPosition,
                            aSize),
                        pOut);
                }
            }
            else
            {
                const Point aPosition(aAlignedGrfArea.Pos());
                const Size aSize(aAlignedGrfArea.SSize());

                ::svt::EmbeddedObjectRef::DrawPaintReplacement(
                    tools::Rectangle(aPosition, aSize),
                    pOLENd->GetOLEObj().GetCurrentPersistName(),
                    pOut);
            }

            sal_Int64 nMiscStatus = pOLENd->GetOLEObj().GetOleRef()->getStatus( pOLENd->GetAspect() );
            if ( !bPrn && dynamic_cast< const SwCursorShell *>( pShell ) !=  nullptr &&
                    (nMiscStatus & embed::EmbedMisc::MS_EMBED_ACTIVATEWHENVISIBLE))
            {
                const SwFlyFrame *pFly = FindFlyFrame();
                assert( pFly != nullptr );
                static_cast<SwFEShell*>(pShell)->ConnectObj( pOLENd->GetOLEObj().GetObject(), pFly->getFramePrintArea(), pFly->getFrameArea());
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
    {
        return true;
    }

    const SwGrfNode *pNd;

    if( nullptr != (pNd = GetNode()->GetGrfNode()) )
    {
        if(pNd->IsTransparent())
        {
            return true;
        }
    }

    // RotateFlyFrame3: If we are transformed, there are 'free' areas between
    // the Graphic and the Border/Padding stuff - at least as long as those
    // (Border and Padding) are not transformed, too
    if(isTransformableSwFrame())
    {
        // we can be more specific - rotations of multiples of
        // 90 degrees will leave no gaps. Go from [0.0 .. F_2PI]
        // to [0 .. 360] and check modulo 90
        const long nRot(static_cast<long>(basegfx::rad2deg(getLocalFrameRotation())));
        const bool bMultipleOf90(0 == (nRot % 90));

        if(!bMultipleOf90)
        {
            return true;
        }
    }

    //#29381# OLE are always transparent
    if(nullptr != GetNode()->GetOLENode())
    {
        return true;
    }

    // return false by default to avoid background paint
    return false;
}

void SwNoTextFrame::StopAnimation( OutputDevice* pOut ) const
{
    // Stop animated graphics
    const SwGrfNode* pGrfNd = GetNode()->GetGrfNode();

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
