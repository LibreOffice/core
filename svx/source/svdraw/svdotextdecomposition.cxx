/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include <svx/svdotext.hxx>
#include <svx/svdoutl.hxx>
#include <basegfx/vector/b2dvector.hxx>
#include <svx/sdr/primitive2d/sdrtextprimitive2d.hxx>
#include <drawinglayer/primitive2d/textprimitive2d.hxx>
#include <drawinglayer/primitive2d/textdecoratedprimitive2d.hxx>
#include <basegfx/range/b2drange.hxx>
#include <editeng/editstat.hxx>
#include <vcl/salbtype.hxx>
#include <svx/sdtfchim.hxx>
#include <svl/itemset.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <drawinglayer/animation/animationtiming.hxx>
#include <basegfx/color/bcolor.hxx>
#include <vcl/svapp.hxx>
#include <editeng/eeitemid.hxx>
#include <editeng/escpitem.hxx>
#include <editeng/svxenum.hxx>
#include <editeng/flditem.hxx>
#include <drawinglayer/primitive2d/texthierarchyprimitive2d.hxx>
#include <vcl/metaact.hxx>
#include <drawinglayer/primitive2d/wrongspellprimitive2d.hxx>
#include <drawinglayer/primitive2d/graphicprimitive2d.hxx>
#include <drawinglayer/primitive2d/textlayoutdevice.hxx>
#include <svx/unoapi.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <editeng/outlobj.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>

//////////////////////////////////////////////////////////////////////////////
// primitive decompositions

void SdrTextObj::impDecomposeContourTextPrimitive(
    drawinglayer::primitive2d::Primitive2DSequence& rTarget,
    const drawinglayer::primitive2d::SdrContourTextPrimitive2D& rSdrContourTextPrimitive,
    const drawinglayer::geometry::ViewInformation2D& aViewInformation) const
{
    // decompose matrix to have position and size of text
    basegfx::B2DVector aScale;
    basegfx::B2DPoint aTranslate;
    double fRotate, fShearX;
    rSdrContourTextPrimitive.getObjectTransform().decompose(aScale, aTranslate, fRotate, fShearX);

    // remember mirroring
    const bool bMirrorX(basegfx::fTools::less(aScale.getX(), 0.0));
    const bool bMirrorY(basegfx::fTools::less(aScale.getY(), 0.0));

    // use scale absolute from here
    aScale = basegfx::absolute(aScale);

    // prepare contour polygon, force to non-mirrored for layouting
    basegfx::B2DPolyPolygon aPolyPolygon(rSdrContourTextPrimitive.getUnitPolyPolygon());
    aPolyPolygon.transform(basegfx::tools::createScaleB2DHomMatrix(aScale.getX(), aScale.getY()));

    // prepare outliner
    const Size aNullSize;
    SdrOutliner& rOutliner = ImpGetDrawOutliner();

    rOutliner.SetPaperSize(aNullSize);
    rOutliner.SetPolygon(aPolyPolygon);
    rOutliner.SetUpdateMode(true);
    rOutliner.SetText(rSdrContourTextPrimitive.getOutlinerParaObject());

    // set visualizing page at Outliner; needed e.g. for PageNumberField decomposition
    rOutliner.setVisualizedPage(GetSdrPageFromXDrawPage(aViewInformation.getVisualizedPage()));

    // break up text primitives.
    rOutliner.getPrimitive2DSequence(rTarget, &aScale);

    if(rTarget.hasElements())
    {
        // prepare embedding transformation for new text primitives. Absolute Scale
        // is already applied, add mirror, shear, rotate and translate
        const basegfx::B2DHomMatrix aEmbeddingTransform(
            basegfx::tools::createScaleShearXRotateTranslateB2DHomMatrix(
                bMirrorX ? -1.0 : 1.0, bMirrorY ? -1.0 : 1.0,
                fShearX,
                fRotate,
                aTranslate.getX(), aTranslate.getY()));

        const drawinglayer::primitive2d::Primitive2DReference xRef(
            new drawinglayer::primitive2d::TransformPrimitive2D(
                aEmbeddingTransform,
                rTarget));

        rTarget = drawinglayer::primitive2d::Primitive2DSequence(&xRef, 1);
    }

    // cleanup outliner
    rOutliner.Clear();
    rOutliner.setVisualizedPage(0);
}

void SdrTextObj::impDecomposeBlockTextPrimitive(
    drawinglayer::primitive2d::Primitive2DSequence& rTarget,
    const drawinglayer::primitive2d::SdrBlockTextPrimitive2D& rSdrBlockTextPrimitive,
    const drawinglayer::geometry::ViewInformation2D& aViewInformation) const
{
    // decompose matrix to have position and size of text
    basegfx::B2DVector aScale;
    basegfx::B2DPoint aTranslate;
    double fRotate, fShearX;
    rSdrBlockTextPrimitive.getTextRangeTransform().decompose(aScale, aTranslate, fRotate, fShearX);

    // use B2DRange aAnchorTextRange for calculations
    basegfx::B2DRange aAnchorTextRange(aTranslate);
    aAnchorTextRange.expand(aTranslate + aScale);

    // prepare outliner
    const bool bIsCell(rSdrBlockTextPrimitive.getCellText());
    SdrOutliner& rOutliner = ImpGetDrawOutliner();
    SdrTextHorzAdjust eHAdj = rSdrBlockTextPrimitive.getSdrTextHorzAdjust();
    SdrTextVertAdjust eVAdj = rSdrBlockTextPrimitive.getSdrTextVertAdjust();
    const sal_uInt32 nOriginalControlWord(rOutliner.GetControlWord());
    const Size aNullSize;

    // set visualizing page at Outliner; needed e.g. for PageNumberField decomposition
    rOutliner.setVisualizedPage(GetSdrPageFromXDrawPage(aViewInformation.getVisualizedPage()));
    rOutliner.SetFixedCellHeight(rSdrBlockTextPrimitive.isFixedCellHeight());
    rOutliner.SetControlWord(nOriginalControlWord|EE_CNTRL_AUTOPAGESIZE);
    rOutliner.SetMinAutoPaperSize(aNullSize);
    rOutliner.SetMaxAutoPaperSize(Size(1000000,1000000));

    // add one to rage sizes to get back to the old Rectangle and outliner measurements
    const sal_uInt32 nAnchorTextWidth(FRound(aAnchorTextRange.getWidth() + 1L));
    const sal_uInt32 nAnchorTextHeight(FRound(aAnchorTextRange.getHeight() + 1L));
    const bool bVerticalWritintg(rSdrBlockTextPrimitive.getOutlinerParaObject().IsVertical());
    const Size aAnchorTextSize(Size(nAnchorTextWidth, nAnchorTextHeight));

    if(bIsCell)
    {
        // cell text is formated neither like a text object nor like a object
        // text, so use a special setup here
        rOutliner.SetMaxAutoPaperSize(aAnchorTextSize);

        // #i106214# To work with an unchangeable PaperSize (CellSize in
        // this case) Set(Min|Max)AutoPaperSize and SetPaperSize have to be used.
        // #i106214# This was not completely correct; to still measure the real
        // text height to allow vertical adjust (and vice versa for VerticalWritintg)
        // only one aspect has to be set, but the other one to zero
        if(bVerticalWritintg)
        {
            // measure the horizontal text size
            rOutliner.SetMinAutoPaperSize(Size(0, aAnchorTextSize.Height()));
        }
        else
        {
            // measure the vertical text size
            rOutliner.SetMinAutoPaperSize(Size(aAnchorTextSize.Width(), 0));
        }

        rOutliner.SetPaperSize(aAnchorTextSize);
        rOutliner.SetUpdateMode(true);
        rOutliner.SetText(rSdrBlockTextPrimitive.getOutlinerParaObject());
    }
    else
    {
        // check if block text is used (only one of them can be true)
        const bool bHorizontalIsBlock(SDRTEXTHORZADJUST_BLOCK == eHAdj && !bVerticalWritintg);
        const bool bVerticalIsBlock(SDRTEXTVERTADJUST_BLOCK == eVAdj && bVerticalWritintg);

        // set minimal paper size hor/ver if needed
        if(bHorizontalIsBlock)
        {
            rOutliner.SetMinAutoPaperSize(Size(nAnchorTextWidth, 0));
        }
        else if(bVerticalIsBlock)
        {
            rOutliner.SetMinAutoPaperSize(Size(0, nAnchorTextHeight));
        }

        if((rSdrBlockTextPrimitive.getWordWrap() || IsTextFrame()) && !rSdrBlockTextPrimitive.getUnlimitedPage())
        {
            // #i103454# maximal paper size hor/ver needs to be limited to text
            // frame size. If it's block text, still allow the 'other' direction
            // to grow to get a correct real text size when using GetPaperSize().
            // When just using aAnchorTextSize as maximum, GetPaperSize()
            // would just return aAnchorTextSize again: this means, the wanted
            // 'measurement' of the real size of block text would not work
            Size aMaxAutoPaperSize(aAnchorTextSize);

            if(bHorizontalIsBlock)
            {
                // allow to grow vertical for horizontal blocks
                aMaxAutoPaperSize.setHeight(1000000);
            }
            else if(bVerticalIsBlock)
            {
                // allow to grow horizontal for vertical blocks
                aMaxAutoPaperSize.setWidth(1000000);
            }

            rOutliner.SetMaxAutoPaperSize(aMaxAutoPaperSize);
        }

        rOutliner.SetPaperSize(aNullSize);
        rOutliner.SetUpdateMode(true);
        rOutliner.SetText(rSdrBlockTextPrimitive.getOutlinerParaObject());
    }

    rOutliner.SetControlWord(nOriginalControlWord);

    // now get back the layouted text size from outliner
    const Size aOutlinerTextSiz(rOutliner.GetPaperSize());
    const basegfx::B2DVector aOutlinerScale(aOutlinerTextSiz.Width(), aOutlinerTextSiz.Height());
    basegfx::B2DPoint aAdjustTranslate(0.0, 0.0);

    // For draw objects containing text correct hor/ver alignment if text is bigger
    // than the object itself. Without that correction, the text would always be
    // formatted to the left edge (or top edge when vertical) of the draw object.
    if(!IsTextFrame() && !bIsCell)
    {
        if(aAnchorTextRange.getWidth() < aOutlinerScale.getX() && !bVerticalWritintg)
        {
            // Horizontal case here. Correct only if eHAdj == SDRTEXTHORZADJUST_BLOCK,
            // else the alignment is wanted.
            if(SDRTEXTHORZADJUST_BLOCK == eHAdj)
            {
                eHAdj = SDRTEXTHORZADJUST_CENTER;
            }
        }

        if(aAnchorTextRange.getHeight() < aOutlinerScale.getY() && bVerticalWritintg)
        {
            // Vertical case here. Correct only if eHAdj == SDRTEXTVERTADJUST_BLOCK,
            // else the alignment is wanted.
            if(SDRTEXTVERTADJUST_BLOCK == eVAdj)
            {
                eVAdj = SDRTEXTVERTADJUST_CENTER;
            }
        }
    }

    // correct horizontal translation using the now known text size
    if(SDRTEXTHORZADJUST_CENTER == eHAdj || SDRTEXTHORZADJUST_RIGHT == eHAdj)
    {
        const double fFree(aAnchorTextRange.getWidth() - aOutlinerScale.getX());

        if(SDRTEXTHORZADJUST_CENTER == eHAdj)
        {
            aAdjustTranslate.setX(fFree / 2.0);
        }

        if(SDRTEXTHORZADJUST_RIGHT == eHAdj)
        {
            aAdjustTranslate.setX(fFree);
        }
    }

    // correct vertical translation using the now known text size
    if(SDRTEXTVERTADJUST_CENTER == eVAdj || SDRTEXTVERTADJUST_BOTTOM == eVAdj)
    {
        const double fFree(aAnchorTextRange.getHeight() - aOutlinerScale.getY());

        if(SDRTEXTVERTADJUST_CENTER == eVAdj)
        {
            aAdjustTranslate.setY(fFree / 2.0);
        }

        if(SDRTEXTVERTADJUST_BOTTOM == eVAdj)
        {
            aAdjustTranslate.setY(fFree);
        }
    }

    // prepare matrices to apply to newly created primitives. aNewTransformA
    // will get coordinates in aOutlinerScale size and positive in X, Y.
    // Translate relative to given primitive to get same rotation and shear
    // as the master shape we are working on. For vertical, use the top-right
    // corner
    const double fStartInX(bVerticalWritintg ? aAdjustTranslate.getX() + aOutlinerScale.getX() : aAdjustTranslate.getX());
    const basegfx::B2DTuple aAdjOffset(fStartInX, aAdjustTranslate.getY());
    basegfx::B2DHomMatrix aEmbeddingTransform(
        basegfx::tools::createTranslateB2DHomMatrix(
            aAdjOffset.getX(),
            aAdjOffset.getY()));

    // mirroring. We are now in aAnchorTextRange sizes. When mirroring in X and Y,
    // move the null point which was top left to bottom right.
    const bool bMirrorX(basegfx::fTools::less(aScale.getX(), 0.0));
    const bool bMirrorY(basegfx::fTools::less(aScale.getY(), 0.0));

    // in-between the translations of the single primitives will take place. Afterwards,
    // the object's transformations need to be applied
    aEmbeddingTransform = basegfx::tools::createScaleShearXRotateTranslateB2DHomMatrix(
        bMirrorX ? -1.0 : 1.0, bMirrorY ? -1.0 : 1.0,
        fShearX,
        fRotate,
        aTranslate.getX(), aTranslate.getY()) * aEmbeddingTransform;

    // #SJ# create ClipRange (if needed)
    basegfx::B2DRange aClipRange;

    if(rSdrBlockTextPrimitive.getClipOnBounds())
    {
        aClipRange.expand(-aAdjOffset);
        aClipRange.expand(basegfx::B2DTuple(aAnchorTextSize.Width(), aAnchorTextSize.Height()) - aAdjOffset);
    }

    // now break up text primitives.
    rOutliner.getPrimitive2DSequence(rTarget, 0, aClipRange.isEmpty() ? 0 : &aClipRange);

    if(rTarget.hasElements())
    {
        const drawinglayer::primitive2d::Primitive2DReference xRef(
            new drawinglayer::primitive2d::TransformPrimitive2D(
                aEmbeddingTransform,
                rTarget));

        rTarget = drawinglayer::primitive2d::Primitive2DSequence(&xRef, 1);
    }

    // cleanup outliner
    rOutliner.Clear();
    rOutliner.setVisualizedPage(0);
}

void SdrTextObj::impDecomposeStretchTextPrimitive(
    drawinglayer::primitive2d::Primitive2DSequence& rTarget,
    const drawinglayer::primitive2d::SdrStretchTextPrimitive2D& rSdrStretchTextPrimitive,
    const drawinglayer::geometry::ViewInformation2D& aViewInformation) const
{
    // decompose matrix to have position and size of text
    basegfx::B2DVector aScale;
    basegfx::B2DPoint aTranslate;
    double fRotate, fShearX;
    rSdrStretchTextPrimitive.getTextRangeTransform().decompose(aScale, aTranslate, fRotate, fShearX);

    // use non-mirrored B2DRange aAnchorTextRange for calculations
    basegfx::B2DRange aAnchorTextRange(aTranslate);
    aAnchorTextRange.expand(aTranslate + aScale);

    // prepare outliner
    SdrOutliner& rOutliner = ImpGetDrawOutliner();
    const sal_uInt32 nOriginalControlWord(rOutliner.GetControlWord());
    const Size aNullSize;

    rOutliner.SetControlWord(nOriginalControlWord|EE_CNTRL_STRETCHING|EE_CNTRL_AUTOPAGESIZE);
    rOutliner.SetFixedCellHeight(rSdrStretchTextPrimitive.isFixedCellHeight());
    rOutliner.SetMinAutoPaperSize(aNullSize);
    rOutliner.SetMaxAutoPaperSize(Size(1000000,1000000));
    rOutliner.SetPaperSize(aNullSize);
    rOutliner.SetUpdateMode(true);
    rOutliner.SetText(rSdrStretchTextPrimitive.getOutlinerParaObject());

    // set visualizing page at Outliner; needed e.g. for PageNumberField decomposition
    rOutliner.setVisualizedPage(GetSdrPageFromXDrawPage(aViewInformation.getVisualizedPage()));

    // now get back the layouted text size from outliner
    const Size aOutlinerTextSiz(rOutliner.CalcTextSize());
    const basegfx::B2DVector aOutlinerScale(
        basegfx::fTools::equalZero(aOutlinerTextSiz.Width()) ? 1.0 : aOutlinerTextSiz.Width(),
        basegfx::fTools::equalZero(aOutlinerTextSiz.Height()) ? 1.0 : aOutlinerTextSiz.Height());

    // prepare matrices to apply to newly created primitives
    basegfx::B2DHomMatrix aEmbeddingTransform;

    // #i101957# Check for vertical text. If used, aEmbeddingTransform
    // needs to translate the text initially around object width to orient
    // it relative to the topper right instead of the topper left
    const bool bVertical(rSdrStretchTextPrimitive.getOutlinerParaObject().IsVertical());

    if(bVertical)
    {
        aEmbeddingTransform.translate(aScale.getX(), 0.0);
    }

    // calculate global char stretching scale parameters. Use non-mirrored sizes
    // to layout without mirroring
    const double fScaleX(fabs(aScale.getX()) / aOutlinerScale.getX());
    const double fScaleY(fabs(aScale.getY()) / aOutlinerScale.getY());

    rOutliner.SetGlobalCharStretching((sal_Int16)basegfx::fround(fScaleX * 100.0), (sal_Int16)basegfx::fround(fScaleY * 100.0));

    // mirroring. We are now in aAnchorTextRange sizes. When mirroring in X and Y,
    // move the null point which was top left to bottom right.
    const bool bMirrorX(basegfx::fTools::less(aScale.getX(), 0.0));
    const bool bMirrorY(basegfx::fTools::less(aScale.getY(), 0.0));

    // in-between the translations of the single primitives will take place. Afterwards,
    // the object's transformations need to be applied
    aEmbeddingTransform = basegfx::tools::createScaleShearXRotateTranslateB2DHomMatrix(
        bMirrorX ? -1.0 : 1.0, bMirrorY ? -1.0 : 1.0,
        fShearX,
        fRotate,
        aTranslate.getX(), aTranslate.getY()) * aEmbeddingTransform;

    // now break up text primitives.
    rOutliner.getPrimitive2DSequence(rTarget);

    if(rTarget.hasElements())
    {
        const drawinglayer::primitive2d::Primitive2DReference xRef(
            new drawinglayer::primitive2d::TransformPrimitive2D(
                aEmbeddingTransform,
                rTarget));

        rTarget = drawinglayer::primitive2d::Primitive2DSequence(&xRef, 1);
    }

    // cleanup outliner
    rOutliner.SetControlWord(nOriginalControlWord);
    rOutliner.Clear();
    rOutliner.setVisualizedPage(0);
}

//////////////////////////////////////////////////////////////////////////////
// timing generators
#define ENDLESS_LOOP    (0xffffffff)
#define ENDLESS_TIME    ((double)0xffffffff)
#define PIXEL_DPI       (96.0)

void SdrTextObj::impGetBlinkTextTiming(drawinglayer::animation::AnimationEntryList& rAnimList) const
{
    if(SDRTEXTANI_BLINK == GetTextAniKind())
    {
        // get values
        const SfxItemSet& rSet = GetObjectItemSet();
        const sal_uInt32 nRepeat((sal_uInt32)((SfxUInt16Item&)rSet.Get(SDRATTR_TEXT_ANICOUNT)).GetValue());
        bool bVisisbleWhenStopped(((SdrYesNoItem&)rSet.Get(SDRATTR_TEXT_ANISTOPINSIDE)).GetValue());
        double fDelay((double)((SdrTextAniDelayItem&)rSet.Get(SDRATTR_TEXT_ANIDELAY)).GetValue());

        if(0.0 == fDelay)
        {
            // use default
            fDelay = 250.0;
        }

        // prepare loop and add
        drawinglayer::animation::AnimationEntryLoop  aLoop(nRepeat ? nRepeat : ENDLESS_LOOP);
        drawinglayer::animation::AnimationEntryFixed aStart(fDelay, 0.0);
        aLoop.append(aStart);
        drawinglayer::animation::AnimationEntryFixed aEnd(fDelay, 1.0);
        aLoop.append(aEnd);
        rAnimList.append(aLoop);

        // add stopped state if loop is not endless
        if(0L != nRepeat)
        {
            drawinglayer::animation::AnimationEntryFixed aStop(ENDLESS_TIME, bVisisbleWhenStopped ? 0.0 : 1.0);
            rAnimList.append(aStop);
        }
    }
}

void impCreateScrollTiming(const SfxItemSet& rSet, drawinglayer::animation::AnimationEntryList& rAnimList, bool bForward, double fTimeFullPath, double fFrequency)
{
    bool bVisisbleWhenStopped(((SdrYesNoItem&)rSet.Get(SDRATTR_TEXT_ANISTOPINSIDE)).GetValue());
    bool bVisisbleWhenStarted(((SdrYesNoItem&)rSet.Get(SDRATTR_TEXT_ANISTOPINSIDE )).GetValue());
    const sal_uInt32 nRepeat(((SfxUInt16Item&)rSet.Get(SDRATTR_TEXT_ANICOUNT)).GetValue());

    if(bVisisbleWhenStarted)
    {
        // move from center to outside
        drawinglayer::animation::AnimationEntryLinear aInOut(fTimeFullPath * 0.5, fFrequency, 0.5, bForward ? 1.0 : 0.0);
        rAnimList.append(aInOut);
    }

    // loop. In loop, move through
    if(nRepeat || 0L == nRepeat)
    {
        drawinglayer::animation::AnimationEntryLoop aLoop(nRepeat ? nRepeat : ENDLESS_LOOP);
        drawinglayer::animation::AnimationEntryLinear aThrough(fTimeFullPath, fFrequency, bForward ? 0.0 : 1.0, bForward ? 1.0 : 0.0);
        aLoop.append(aThrough);
        rAnimList.append(aLoop);
    }

    if(0L != nRepeat && bVisisbleWhenStopped)
    {
        // move from outside to center
        drawinglayer::animation::AnimationEntryLinear aOutIn(fTimeFullPath * 0.5, fFrequency, bForward ? 0.0 : 1.0, 0.5);
        rAnimList.append(aOutIn);

        // add timing for staying at the end
        drawinglayer::animation::AnimationEntryFixed aEnd(ENDLESS_TIME, 0.5);
        rAnimList.append(aEnd);
    }
}

void impCreateAlternateTiming(const SfxItemSet& rSet, drawinglayer::animation::AnimationEntryList& rAnimList, double fRelativeTextLength, bool bForward, double fTimeFullPath, double fFrequency)
{
    if(basegfx::fTools::more(fRelativeTextLength, 0.5))
    {
        // this is the case when fTextLength > fFrameLength, text is bigger than animation frame.
        // In that case, correct direction
        bForward = !bForward;
    }

    const double fStartPosition(bForward ? fRelativeTextLength : 1.0 - fRelativeTextLength);
    const double fEndPosition(bForward ? 1.0 - fRelativeTextLength : fRelativeTextLength);
    bool bVisisbleWhenStopped(((SdrYesNoItem&)rSet.Get(SDRATTR_TEXT_ANISTOPINSIDE)).GetValue());
    bool bVisisbleWhenStarted(((SdrYesNoItem&)rSet.Get(SDRATTR_TEXT_ANISTOPINSIDE )).GetValue());
    const sal_uInt32 nRepeat(((SfxUInt16Item&)rSet.Get(SDRATTR_TEXT_ANICOUNT)).GetValue());

    if(!bVisisbleWhenStarted)
    {
        // move from outside to center
        drawinglayer::animation::AnimationEntryLinear aOutIn(fTimeFullPath * 0.5, fFrequency, bForward ? 0.0 : 1.0, 0.5);
        rAnimList.append(aOutIn);
    }

    // loop. In loop, move out and in again. fInnerMovePath may be negative when text is bigger then frame,
    // so use absolute value
    const double fInnerMovePath(fabs(1.0 - (fRelativeTextLength * 2.0)));
    const double fTimeForInnerPath(fTimeFullPath * fInnerMovePath);
    const double fHalfInnerPath(fTimeForInnerPath * 0.5);
    const sal_uInt32 nDoubleRepeat(nRepeat / 2L);

    if(nDoubleRepeat || 0L == nRepeat)
    {
        // double forth and back loop
        drawinglayer::animation::AnimationEntryLoop aLoop(nDoubleRepeat ? nDoubleRepeat : ENDLESS_LOOP);
        drawinglayer::animation::AnimationEntryLinear aTime0(fHalfInnerPath, fFrequency, 0.5, fEndPosition);
        aLoop.append(aTime0);
        drawinglayer::animation::AnimationEntryLinear aTime1(fTimeForInnerPath, fFrequency, fEndPosition, fStartPosition);
        aLoop.append(aTime1);
        drawinglayer::animation::AnimationEntryLinear aTime2(fHalfInnerPath, fFrequency, fStartPosition, 0.5);
        aLoop.append(aTime2);
        rAnimList.append(aLoop);
    }

    if(nRepeat % 2L)
    {
        // repeat is uneven, so we need one more forth and back to center
        drawinglayer::animation::AnimationEntryLinear aTime0(fHalfInnerPath, fFrequency, 0.5, fEndPosition);
        rAnimList.append(aTime0);
        drawinglayer::animation::AnimationEntryLinear aTime1(fHalfInnerPath, fFrequency, fEndPosition, 0.5);
        rAnimList.append(aTime1);
    }

    if(0L != nRepeat)
    {
        if(bVisisbleWhenStopped)
        {
            // add timing for staying at the end
            drawinglayer::animation::AnimationEntryFixed aEnd(ENDLESS_TIME, 0.5);
            rAnimList.append(aEnd);
        }
        else
        {
            // move from center to outside
            drawinglayer::animation::AnimationEntryLinear aInOut(fTimeFullPath * 0.5, fFrequency, 0.5, bForward ? 1.0 : 0.0);
            rAnimList.append(aInOut);
        }
    }
}

void impCreateSlideTiming(const SfxItemSet& rSet, drawinglayer::animation::AnimationEntryList& rAnimList, bool bForward, double fTimeFullPath, double fFrequency)
{
    // move in from outside, start outside
    const double fStartPosition(bForward ? 0.0 : 1.0);
    const sal_uInt32 nRepeat(((SfxUInt16Item&)rSet.Get(SDRATTR_TEXT_ANICOUNT)).GetValue());

    // move from outside to center
    drawinglayer::animation::AnimationEntryLinear aOutIn(fTimeFullPath * 0.5, fFrequency, fStartPosition, 0.5);
    rAnimList.append(aOutIn);

    // loop. In loop, move out and in again
    if(nRepeat > 1L || 0L == nRepeat)
    {
        drawinglayer::animation::AnimationEntryLoop aLoop(nRepeat ? nRepeat - 1L : ENDLESS_LOOP);
        drawinglayer::animation::AnimationEntryLinear aTime0(fTimeFullPath * 0.5, fFrequency, 0.5, fStartPosition);
        aLoop.append(aTime0);
        drawinglayer::animation::AnimationEntryLinear aTime1(fTimeFullPath * 0.5, fFrequency, fStartPosition, 0.5);
        aLoop.append(aTime1);
        rAnimList.append(aLoop);
    }

    // always visible when stopped, so add timing for staying at the end when not endless
    if(0L != nRepeat)
    {
        drawinglayer::animation::AnimationEntryFixed aEnd(ENDLESS_TIME, 0.5);
        rAnimList.append(aEnd);
    }
}

void SdrTextObj::impGetScrollTextTiming(drawinglayer::animation::AnimationEntryList& rAnimList, double fFrameLength, double fTextLength) const
{
    const SdrTextAniKind eAniKind(GetTextAniKind());

    if(SDRTEXTANI_SCROLL == eAniKind || SDRTEXTANI_ALTERNATE == eAniKind || SDRTEXTANI_SLIDE == eAniKind)
    {
        // get data. Goal is to calculate fTimeFullPath which is the time needed to
        // move animation from (0.0) to (1.0) state
        const SfxItemSet& rSet = GetObjectItemSet();
        double fAnimationDelay((double)((SdrTextAniDelayItem&)rSet.Get(SDRATTR_TEXT_ANIDELAY)).GetValue());
        double fSingleStepWidth((double)((SdrTextAniAmountItem&)rSet.Get(SDRATTR_TEXT_ANIAMOUNT)).GetValue());
        const SdrTextAniDirection eDirection(GetTextAniDirection());
        const bool bForward(SDRTEXTANI_RIGHT == eDirection || SDRTEXTANI_DOWN == eDirection);

        if(basegfx::fTools::equalZero(fAnimationDelay))
        {
            // default to 1/20 second
            fAnimationDelay = 50.0;
        }

        if(basegfx::fTools::less(fSingleStepWidth, 0.0))
        {
            // data is in pixels, convert to logic. Imply PIXEL_DPI dpi.
            // It makes no sense to keep the view-transformation centered
            // definitions, so get rid of them here.
            fSingleStepWidth = (-fSingleStepWidth * (2540.0 / PIXEL_DPI));
        }

        if(basegfx::fTools::equalZero(fSingleStepWidth))
        {
            // default to 1 milimeter
            fSingleStepWidth = 100.0;
        }

        // use the length of the full animation path and the number of steps
        // to get the full path time
        const double fFullPathLength(fFrameLength + fTextLength);
        const double fNumberOfSteps(fFullPathLength / fSingleStepWidth);
        double fTimeFullPath(fNumberOfSteps * fAnimationDelay);

        if(fTimeFullPath < fAnimationDelay)
        {
            fTimeFullPath = fAnimationDelay;
        }

        switch(eAniKind)
        {
            case SDRTEXTANI_SCROLL :
            {
                impCreateScrollTiming(rSet, rAnimList, bForward, fTimeFullPath, fAnimationDelay);
                break;
            }
            case SDRTEXTANI_ALTERNATE :
            {
                double fRelativeTextLength(fTextLength / (fFrameLength + fTextLength));
                impCreateAlternateTiming(rSet, rAnimList, fRelativeTextLength, bForward, fTimeFullPath, fAnimationDelay);
                break;
            }
            case SDRTEXTANI_SLIDE :
            {
                impCreateSlideTiming(rSet, rAnimList, bForward, fTimeFullPath, fAnimationDelay);
                break;
            }
            default : break; // SDRTEXTANI_NONE, SDRTEXTANI_BLINK
        }
    }
}

//////////////////////////////////////////////////////////////////////////////
// eof
