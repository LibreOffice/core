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
#include <vcl/imapobj.hxx>
#include <vcl/imap.hxx>
#include <svl/urihelper.hxx>
#include <sfx2/progress.hxx>
#include <sfx2/printer.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/boxitem.hxx>
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
#include <ndnotxt.hxx>
#include <ndgrf.hxx>
#include <ndole.hxx>
#include <swregion.hxx>
#include <poolfmt.hxx>
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
#include <drawinglayer/primitive2d/maskprimitive2d.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <drawinglayer/primitive2d/objectinfoprimitive2d.hxx>
#include <osl/diagnose.h>

// MM02 needed for VOC mechanism and getting the OC - may be moved to an own file
#include <svx/sdrpagewindow.hxx>
#include <svx/svdpagv.hxx>
#include <svx/sdr/contact/viewcontact.hxx>
#include <svx/sdr/contact/viewobjectcontact.hxx>
#include <svx/sdr/contact/objectcontact.hxx>
#include <svx/sdr/contact/displayinfo.hxx>

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
    static vcl::Font aFont = []()
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

        SwFormat *pFormat = rSh.GetDoc()->getIDocumentStylePoolAccess().GetFormatFromPool( o3tl::narrowing<sal_uInt16>
            (bVisited ? RES_POOLCHR_INET_VISIT : RES_POOLCHR_INET_NORMAL ) );
        aCol = pFormat->GetColor().GetValue();
        eUnderline = pFormat->GetUnderline().GetLineStyle();
    }

    aFont.SetUnderline( eUnderline );
    aFont.SetColor( aCol );

    const BitmapEx& rBmp = const_cast<SwViewShell&>(rSh).GetReplacementBitmap(bDefect);
    Graphic::DrawEx(*rSh.GetOut(), rText, aFont, rBmp, rRect.Pos(), rRect.SSize());
}

SwNoTextFrame::SwNoTextFrame(SwNoTextNode * const pNode, SwFrame* pSib )
:   SwContentFrame( pNode, pSib )
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

    if ( aRegion.empty() )
        return;

    const SvxBrushItem *pItem;
    std::optional<Color> xCol;
    SwRect aOrigRect;
    drawinglayer::attribute::SdrAllFillAttributesHelperPtr aFillAttributes;

    if ( rFrame.GetBackgroundBrush( aFillAttributes, pItem, xCol, aOrigRect, false, /*bConsiderTextBox=*/false ) )
    {
        SwRegionRects const region(rPtArea);
        basegfx::utils::B2DClipState aClipState;
        const bool bDone(::DrawFillAttributes(aFillAttributes, aOrigRect, region, aClipState, rOut));

        if(!bDone)
        {
            for( const auto &rRegion : aRegion )
            {
                ::DrawGraphic(pItem, rOut, aOrigRect, rRegion);
            }
        }
    }
    else
    {
        rOut.Push( vcl::PushFlags::FILLCOLOR|vcl::PushFlags::LINECOLOR );
        rOut.SetFillColor( rFrame.getRootFrame()->GetCurrShell()->Imp()->GetRetoucheColor());
        rOut.SetLineColor();
        for( const auto &rRegion : aRegion )
            rOut.DrawRect( rRegion.SVRect() );
        rOut.Pop();
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
        auto pFindFly = FindFlyFrame();
        if (pFindFly && pFindFly->IsFlyFreeFrame())
        {
            const SwFlyFreeFrame *pFly = static_cast< const SwFlyFreeFrame* >( pFindFly );
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

    if( aPaintArea.Overlaps( aNormal ) )
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
    tools::Long nLeftCrop, nRightCrop, nTopCrop, nBottomCrop;
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
                            (rCrop.GetRight() + rCrop.GetLeft()), tools::Long(1) );
        const double nScale = double(aFramePrintArea.Width())  / double(nLeftCrop);
        nLeftCrop  = tools::Long(nScale * -rCrop.GetLeft() );
        nRightCrop = tools::Long(nScale * -rCrop.GetRight() );
    }

    // crop values have to be mirrored too
    if( nMirror == MirrorGraph::Vertical || nMirror == MirrorGraph::Both )
    {
        tools::Long nTmpCrop = nLeftCrop;
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
        nTopCrop = std::max( aOrigSz.Height() - (rCrop.GetTop() + rCrop.GetBottom()), tools::Long(1) );
        const double nScale = double(aFramePrintArea.Height()) / double(nTopCrop);
        nTopCrop   = tools::Long(nScale * -rCrop.GetTop() );
        nBottomCrop= tools::Long(nScale * -rCrop.GetBottom() );
    }

    // crop values have to be mirrored too
    if( nMirror == MirrorGraph::Horizontal || nMirror == MirrorGraph::Both )
    {
        tools::Long nTmpCrop = nTopCrop;
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
    if ( !pOrigRect )
        return;

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
            // upper frame, so it can be re-created on the fly
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
            const double fRotate = -toRadians(rSwRotationGrf.GetValue());

            return basegfx::normalizeToRange(fRotate, 2 * M_PI);
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
        Shrink( std::min(getFramePrintArea().Height(), tools::Long(-nChgHght)) );
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
    if( !aFrameRect.Overlaps( rRect ) )
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

bool SwNoTextFrame::GetModelPositionForViewPoint(SwPosition* pPos, Point& ,
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
        pFly->NotifyBackground( FindPageFrame(), getFramePrintArea(), PrepareHint::FlyFrameAttributesChanged );
    }
}

void SwNoTextFrame::OnGraphicArrived()
{
    if(GetNode()->GetNodeType() != SwNodeType::Grf)
    {
        InvalidatePrt();
        SetCompletePaint();
        return;
    }
    SwGrfNode* pNd = static_cast<SwGrfNode*>(GetNode());
    ClearCache();
    auto pVSh = pNd->GetDoc().getIDocumentLayoutAccess().GetCurrentViewShell();
    if(pVSh)
        pVSh->OnGraphicArrived(getFrameArea());
}

void SwNoTextFrame::SwClientNotify(const SwModify& rModify, const SfxHint& rHint)
{
    if(dynamic_cast<const sw::GrfRereadAndInCacheHint*>(&rHint))
    {
        if(SwNodeType::Grf != GetNode()->GetNodeType())
        {
            InvalidatePrt();
            SetCompletePaint();
        }
        return;
    }
    if(dynamic_cast<const sw::PreGraphicArrivedHint*>(&rHint))
    {
        OnGraphicArrived();
        return;
    }
    if (rHint.GetId() != SfxHintId::SwLegacyModify)
        return;
    auto pLegacy = static_cast<const sw::LegacyModifyHint*>(&rHint);
    sal_uInt16 nWhich = pLegacy->GetWhich();

    // #i73788#
    // no <SwContentFrame::Modify(..)> for RES_LINKED_GRAPHIC_STREAM_ARRIVED
    if ( RES_GRAPHIC_PIECE_ARRIVED != nWhich &&
         RES_LINKED_GRAPHIC_STREAM_ARRIVED != nWhich )
    {
        SwContentFrame::SwClientNotify(rModify, rHint);
    }

    bool bComplete = true;

    switch( nWhich )
    {
    case RES_GRAPHIC_PIECE_ARRIVED:
    case RES_LINKED_GRAPHIC_STREAM_ARRIVED:
        OnGraphicArrived();
        return;

    case RES_OBJECTDYING:
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
                if( SfxItemState::SET == static_cast<const SwAttrSetChg*>(pLegacy->m_pOld)->GetChgSet()->
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
                            SwViewShell *pVSh = pNd->GetDoc().getIDocumentLayoutAccess().GetCurrentViewShell();

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

    default:
        if ( !pLegacy->m_pNew || !isGRFATR(nWhich) )
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
        rAlignedGrfArea.AddLeft( 1 );
        aNewPxRect = pOut->LogicToPixel( rAlignedGrfArea.SVRect() );
    }
    while( aNewPxRect.Top() < aPxRect.Top() )
    {
        rAlignedGrfArea.AddTop(+1);
        aNewPxRect = pOut->LogicToPixel( rAlignedGrfArea.SVRect() );
    }
    while( aNewPxRect.Bottom() > aPxRect.Bottom() )
    {
        rAlignedGrfArea.AddBottom( -1 );
        aNewPxRect = pOut->LogicToPixel( rAlignedGrfArea.SVRect() );
    }
    while( aNewPxRect.Right() > aPxRect.Right() )
    {
        rAlignedGrfArea.AddRight(-1);
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
                0.0);

            // get a primitive processor for rendering
            std::unique_ptr<drawinglayer::processor2d::BaseProcessor2D> pProcessor2D(
                drawinglayer::processor2d::createProcessor2DFromOutputDevice(
                                                rOutputDevice, aViewInformation2D) );

            // render and cleanup
            pProcessor2D->process(rSequence);
            return true;
        }
    }

    return false;
}

// MM02 original using falölback to VOC and primitive-based version
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

    // MM02 use primitive-based version for visualization
    paintGraphicUsingPrimitivesHelper(
        rOutputDevice,
        aContent,
        rGraphicTransform,
        rName,
        rTitle,
        rDescription);
}

// MM02 new VOC and primitive-based version
void paintGraphicUsingPrimitivesHelper(
    vcl::RenderContext & rOutputDevice,
    drawinglayer::primitive2d::Primitive2DContainer& rContent,
    const basegfx::B2DHomMatrix& rGraphicTransform,
    const OUString& rName,
    const OUString& rTitle,
    const OUString& rDescription)
{
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
            rContent.resize(1);
            rContent[0] =
                new drawinglayer::primitive2d::MaskPrimitive2D(
                    aClip,
                    drawinglayer::primitive2d::Primitive2DContainer(rContent));
        }
    }

    if(!rName.isEmpty() || !rTitle.isEmpty() || !rDescription.isEmpty())
    {
        // Embed to ObjectInfoPrimitive2D when we have Name/Title/Description
        // information available
        rContent.resize(1);
        rContent[0] =
            new drawinglayer::primitive2d::ObjectInfoPrimitive2D(
                drawinglayer::primitive2d::Primitive2DContainer(rContent),
                rName,
                rTitle,
                rDescription);
    }

    basegfx::B2DRange aTargetRange(0.0, 0.0, 1.0, 1.0);
    aTargetRange.transform(rGraphicTransform);

    paintUsingPrimitivesHelper(
        rOutputDevice,
        rContent,
        aTargetRange,
        aTargetRange);
}

// DrawContact section
namespace { // anonymous namespace
class ViewObjectContactOfSwNoTextFrame : public sdr::contact::ViewObjectContact
{
protected:
    virtual void createPrimitive2DSequence(
        const sdr::contact::DisplayInfo& rDisplayInfo,
        drawinglayer::primitive2d::Primitive2DDecompositionVisitor& rVisitor) const override;

public:
    ViewObjectContactOfSwNoTextFrame(
        sdr::contact::ObjectContact& rObjectContact,
        sdr::contact::ViewContact& rViewContact);
};

class ViewContactOfSwNoTextFrame : public sdr::contact::ViewContact
{
private:
    // owner
    const SwNoTextFrame&        mrSwNoTextFrame;

protected:
    // Create an Object-Specific ViewObjectContact, set ViewContact and
    // ObjectContact. Always needs to return something.
    virtual sdr::contact::ViewObjectContact& CreateObjectSpecificViewObjectContact(
        sdr::contact::ObjectContact& rObjectContact) override;

public:
    // read-access to owner
    const SwNoTextFrame& getSwNoTextFrame() const { return mrSwNoTextFrame; }

    // basic constructor, used from SwNoTextFrame.
    explicit ViewContactOfSwNoTextFrame(const SwNoTextFrame& rSwNoTextFrame);
};

void ViewObjectContactOfSwNoTextFrame::createPrimitive2DSequence(
    const sdr::contact::DisplayInfo& /*rDisplayInfo*/,
    drawinglayer::primitive2d::Primitive2DDecompositionVisitor& rVisitor) const
{
    // MM02 get all the parameters formally used in paintGraphicUsingPrimitivesHelper
    ViewContactOfSwNoTextFrame& rVCOfNTF(static_cast<ViewContactOfSwNoTextFrame&>(GetViewContact()));
    const SwNoTextFrame& rSwNoTextFrame(rVCOfNTF.getSwNoTextFrame());
    SwNoTextNode& rNoTNd(const_cast<SwNoTextNode&>(*static_cast<const SwNoTextNode*>(rSwNoTextFrame.GetNode())));
    SwGrfNode* pGrfNd(rNoTNd.GetGrfNode());

    if(nullptr != pGrfNd)
    {
        const bool bPrn(GetObjectContact().isOutputToPrinter() || GetObjectContact().isOutputToRecordingMetaFile());
        const GraphicObject& rGrfObj(pGrfNd->GetGrfObj(bPrn));
        GraphicAttr aGraphicAttr;
        pGrfNd->GetGraphicAttr(aGraphicAttr, &rSwNoTextFrame);
        const basegfx::B2DHomMatrix aGraphicTransform(rSwNoTextFrame.getFrameAreaTransformation());

        // MM02 this is the right place in the VOC-Mechanism to create
        // the primitives for visualization - these will be automatically
        // buffered and reused
        rVisitor.visit(new drawinglayer::primitive2d::GraphicPrimitive2D(
            aGraphicTransform,
            rGrfObj,
            aGraphicAttr));
    }
}

ViewObjectContactOfSwNoTextFrame::ViewObjectContactOfSwNoTextFrame(
    sdr::contact::ObjectContact& rObjectContact,
    sdr::contact::ViewContact& rViewContact)
:   sdr::contact::ViewObjectContact(rObjectContact, rViewContact)
{
}

sdr::contact::ViewObjectContact& ViewContactOfSwNoTextFrame::CreateObjectSpecificViewObjectContact(
    sdr::contact::ObjectContact& rObjectContact)
{
    sdr::contact::ViewObjectContact* pRetval = new ViewObjectContactOfSwNoTextFrame(rObjectContact, *this);
    return *pRetval;
}

ViewContactOfSwNoTextFrame::ViewContactOfSwNoTextFrame(
    const SwNoTextFrame& rSwNoTextFrame
)
: mrSwNoTextFrame(rSwNoTextFrame)
{
}
} // end of anonymous namespace

sdr::contact::ViewContact& SwNoTextFrame::GetViewContact() const
{
    if(!mpViewContact)
    {
        const_cast< SwNoTextFrame* >(this)->mpViewContact =
            std::make_unique<ViewContactOfSwNoTextFrame>(*this);
    }

    return *mpViewContact;
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
        if (SwDrawView::IsAntiAliasing())
        {
            pOut->SetAntialiasing( nFormerAntialiasingAtOutput | AntialiasingFlags::Enable );
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
                        pOut = pShell->GetWin()->GetOutDev();
                    }
                    else
                        pVout = nullptr;

                    OSL_ENSURE( !pOut->IsVirtual() ||
                            pShell->GetViewOptions()->IsPDFExport() || pShell->isOutputToWindow(),
                            "pOut should not be a virtual device" );

                    pGrfNd->StartGraphicAnimation(pOut, aAlignedGrfArea.Pos(),
                                        aAlignedGrfArea.SSize(), reinterpret_cast<sal_IntPtr>(this),
                                        pVout );
                }
                else
                {
                    // MM02 To allow system-dependent buffering of the involved
                    // bitmaps it is necessary to re-use the involved primitives
                    // and their already executed decomposition (also for
                    // performance reasons). This is usually done in DrawingLayer
                    // by using the VOC-Mechanism (see descriptions elsewhere).
                    // To get that here, make the involved SwNoTextFrame (this)
                    // a sdr::contact::ViewContact supplier by supporting
                    // a GetViewContact() - call. For ObjectContact we can use
                    // the already existing ObjectContact from the involved
                    // DrawingLayer. For this, the helper classes
                    //     ViewObjectContactOfSwNoTextFrame
                    //     ViewContactOfSwNoTextFrame
                    // are created which support the VOC-mechanism in its minimal
                    // form. This allows automatic and view-dependent (multiple edit
                    // windows, print, etc.) re-use of the created primitives.
                    // Also: Will be very useful when completely changing the Writer
                    // repaint to VOC and Primitives, too.
                    static const char* pDisableMM02Goodies(getenv("SAL_DISABLE_MM02_GOODIES"));
                    static bool bUseViewObjectContactMechanism(nullptr == pDisableMM02Goodies);
                    // tdf#130951 for safety reasons use fallback if ViewObjectContactMechanism
                    // fails for some reason - usually could only be not to find the correct
                    // SdrPageWindow
                    bool bSucceeded(false);

                    if(bUseViewObjectContactMechanism)
                    {
                        // MM02 use VOC-mechanism and buffer primitives
                        SwViewShellImp* pImp(pShell->Imp());
                        SdrPageView* pPageView(nullptr != pImp
                            ? pImp->GetPageView()
                            : nullptr);
                        // tdf#130951 caution - target may be Window, use the correct OutputDevice
                        OutputDevice* pTarget(pShell->isOutputToWindow()
                            ? pShell->GetWin()->GetOutDev()
                            : pShell->GetOut());
                        SdrPageWindow* pPageWindow(nullptr != pPageView && nullptr != pTarget
                            ? pPageView->FindPageWindow(*pTarget)
                            : nullptr);

                        if(nullptr != pPageWindow)
                        {
                            sdr::contact::ObjectContact& rOC(pPageWindow->GetObjectContact());
                            sdr::contact::ViewContact& rVC(GetViewContact());
                            sdr::contact::ViewObjectContact& rVOC(rVC.GetViewObjectContact(rOC));
                            sdr::contact::DisplayInfo aDisplayInfo;

                            drawinglayer::primitive2d::Primitive2DContainer aPrimitives(rVOC.getPrimitive2DSequence(aDisplayInfo));
                            const basegfx::B2DHomMatrix aGraphicTransform(getFrameAreaTransformation());

                            paintGraphicUsingPrimitivesHelper(
                                *pOut,
                                aPrimitives,
                                aGraphicTransform,
                                nullptr == pGrfNd->GetFlyFormat() ? OUString() : pGrfNd->GetFlyFormat()->GetName(),
                                rNoTNd.GetTitle(),
                                rNoTNd.GetDescription());
                            bSucceeded = true;
                        }
                    }

                    if(!bSucceeded)
                    {
                        // MM02 fallback to direct paint with primitive-recreation
                        // which will block reusage of system-dependent bitmap data
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
            }
            else
            {
                TranslateId pResId;

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

        if ( SwDrawView::IsAntiAliasing() )
            pOut->SetAntialiasing( nFormerAntialiasingAtOutput );
    }
    else // bIsChart || pOLENd
    {
        // Fix for bug fdo#33781
        const AntialiasingFlags nFormerAntialiasingAtOutput( pOut->GetAntialiasing() );
        if (SwDrawView::IsAntiAliasing())
        {
            AntialiasingFlags nNewAntialiasingAtOutput = nFormerAntialiasingAtOutput | AntialiasingFlags::Enable;

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
            const Point aPosition(aAlignedGrfArea.Pos());
            const Size aSize(aAlignedGrfArea.SSize());

            if ( pGraphic && pGraphic->GetType() != GraphicType::NONE )
            {
                pGraphic->Draw(*pOut, aPosition, aSize);

                // shade the representation if the object is activated outplace
                uno::Reference < embed::XEmbeddedObject > xObj = pOLENd->GetOLEObj().GetOleRef();
                if ( xObj.is() && xObj->getCurrentState() == embed::EmbedStates::ACTIVE )
                {

                    ::svt::EmbeddedObjectRef::DrawShading(
                        tools::Rectangle(
                            aPosition,
                            aSize),
                        pOut);
                }
            }
            else
            {
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
        if (SwDrawView::IsAntiAliasing())
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
        // 90 degrees will leave no gaps. Go from [0.0 .. 2PI]
        // to [0 .. 360] and check modulo 90
        const tools::Long nRot(static_cast<tools::Long>(basegfx::rad2deg(getLocalFrameRotation())));
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

void SwNoTextFrame::StopAnimation( const OutputDevice* pOut ) const
{
    // Stop animated graphics
    const SwGrfNode* pGrfNd = GetNode()->GetGrfNode();

    if( pGrfNd && pGrfNd->IsAnimated() )
    {
        const_cast< SwGrfNode* >(pGrfNd)->StopGraphicAnimation( pOut, reinterpret_cast<sal_IntPtr>(this) );
    }
}

bool SwNoTextFrame::HasAnimation() const
{
    const SwGrfNode* pGrfNd = GetNode()->GetGrfNode();
    return pGrfNd && pGrfNd->IsAnimated();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
