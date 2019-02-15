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


#include <tools/diagnose_ex.h>
#include <tools/helpers.hxx>
#include <canvas/elapsedtime.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>

#include <comphelper/anytostring.hxx>
#include <cppuhelper/exc_hlp.hxx>

#include <rtl/math.hxx>
#include <sal/log.hxx>
#include <vcl/metric.hxx>
#include <vcl/canvastools.hxx>
#include <vcl/metaact.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/TextAnimationKind.hpp>
#include <com/sun/star/drawing/TextAnimationDirection.hpp>
#include <com/sun/star/drawing/TextHorizontalAdjust.hpp>
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#include <com/sun/star/drawing/HomogenMatrix3.hpp>
#include <com/sun/star/awt/Rectangle.hpp>

#include <activity.hxx>
#include <wakeupevent.hxx>
#include <eventqueue.hxx>
#include "drawinglayeranimation.hxx"
#include "drawshapesubsetting.hxx"
#include "drawshape.hxx"
#include <shapesubset.hxx>
#include <shapeattributelayerholder.hxx>
#include <slideshowcontext.hxx>
#include <tools.hxx>
#include "gdimtftools.hxx"
#include <eventmultiplexer.hxx>
#include "intrinsicanimationactivity.hxx"
#include <intrinsicanimationeventhandler.hxx>

#include <vector>
#include <memory>

using namespace com::sun::star;
using namespace ::slideshow::internal;

namespace {

class ScrollTextAnimNode
{
    sal_uInt32 const  mnDuration; // single duration
    sal_uInt32 const  mnRepeat; // 0 -> endless
    double const      mfStart;
    double const      mfStop;
    sal_uInt32 const  mnFrequency; // in ms
    // forth and back change at mnRepeat%2:
    bool const        mbAlternate;

public:
    ScrollTextAnimNode(
        sal_uInt32 nDuration, sal_uInt32 nRepeat, double fStart, double fStop,
        sal_uInt32 nFrequency, bool bAlternate)
        :   mnDuration(nDuration),
            mnRepeat(nRepeat),
            mfStart(fStart),
            mfStop(fStop),
            mnFrequency(nFrequency),
            mbAlternate(bAlternate)
        {}

    sal_uInt32 GetRepeat() const { return mnRepeat; }
    sal_uInt32 GetFullTime() const { return mnDuration * mnRepeat; }
    double GetStop() const { return mfStop; }
    sal_uInt32 GetFrequency() const { return mnFrequency; }
    bool DoAlternate() const { return mbAlternate; }

    double GetStateAtRelativeTime(sal_uInt32 nRelativeTime) const;
};

double ScrollTextAnimNode::GetStateAtRelativeTime(
    sal_uInt32 nRelativeTime) const
{
    // Avoid division by zero.
    if( mnDuration == 0 )
        return mfStop;

    if(mnRepeat)
    {
        // ending
        const sal_uInt32 nRepeatCount(nRelativeTime / mnDuration);
        sal_uInt32 nFrameTime(nRelativeTime - (nRepeatCount * mnDuration));

        if(DoAlternate() && (nRepeatCount + 1) % 2L)
            nFrameTime = mnDuration - nFrameTime;

        return mfStart + ((mfStop - mfStart) *
                          (double(nFrameTime) / mnDuration));
    }
    else
    {
        // endless
        sal_uInt32 nFrameTime(nRelativeTime % mnDuration);

        if(DoAlternate())
        {
            const sal_uInt32 nRepeatCount(nRelativeTime / mnDuration);

            if((nRepeatCount + 1) % 2L)
                nFrameTime = mnDuration - nFrameTime;
        }

        return mfStart + ((mfStop - mfStart) * (double(nFrameTime) / mnDuration));
    }
}

class ActivityImpl : public Activity
{
public:
    ActivityImpl(
        SlideShowContext const& rContext,
        std::shared_ptr<WakeupEvent> const& pWakeupEvent,
        std::shared_ptr<DrawShape> const& pDrawShape );

    ActivityImpl(const ActivityImpl&) = delete;
    ActivityImpl& operator=(const ActivityImpl&) = delete;

    bool enableAnimations();

    // Disposable:
    virtual void dispose() override;
    // Activity:
    virtual double calcTimeLag() const override;
    virtual bool perform() override;
    virtual bool isActive() const override;
    virtual void dequeued() override;
    virtual void end() override;

private:
    void updateShapeAttributes( double fTime,
                                basegfx::B2DRectangle const& parentBounds );

    // scroll horizontal? if sal_False, scroll is vertical.
    bool ScrollHorizontal() const {
        return (drawing::TextAnimationDirection_LEFT == meDirection ||
                drawing::TextAnimationDirection_RIGHT == meDirection);
    }

    // Access to StepWidth in logical units
    sal_uInt32 GetStepWidthLogic() const;

    // is the animation direction opposite?
    bool DoScrollForward() const {
        return (drawing::TextAnimationDirection_RIGHT == meDirection ||
                drawing::TextAnimationDirection_DOWN == meDirection);
    }

    // do alternate text directions?
    bool DoAlternate() const { return mbAlternate; }

    // do scroll in?
    bool DoScrollIn() const { return mbScrollIn; }

    // Scroll helper methods
    void ImpForceScrollTextAnimNodes();
    ScrollTextAnimNode* ImpGetScrollTextAnimNode(
        sal_uInt32 nTime, sal_uInt32& rRelativeTime );
    sal_uInt32 ImpRegisterAgainScrollTextMixerState(
        sal_uInt32 nTime);

    // calculate the MixerState value for given time
    double GetMixerState(sal_uInt32 nTime);


    SlideShowContext                            maContext;
    std::shared_ptr<WakeupEvent>              mpWakeupEvent;
    std::weak_ptr<DrawShape>                  mpParentDrawShape;
    DrawShapeSharedPtr                          mpDrawShape;
    ShapeAttributeLayerHolder                   maShapeAttrLayer;
    GDIMetaFileSharedPtr                        mpMetaFile;
    IntrinsicAnimationEventHandlerSharedPtr const mpListener;
    canvas::tools::ElapsedTime                  maTimer;
    double                                      mfRotationAngle;
    bool                                        mbIsShapeAnimated;
    bool                                        mbIsDisposed;
    bool                                        mbIsActive;
    drawing::TextAnimationKind                  meAnimKind;

    // The blink frequency in ms
    sal_uInt32                                  mnFrequency;

    // The repeat count, init to 0L which means endless
    sal_uInt32                                  mnRepeat;

    // Flag to decide if text will be shown when animation has ended
    bool                                        mbVisibleWhenStopped;
    bool                                        mbVisibleWhenStarted;

    // Flag decides if TextScroll alternates. Default is sal_False.
    bool                                        mbAlternate;

    // Flag to remember if this is a simple scrolling text
    bool                                        mbScrollIn;

    // The AnimationDirection
    drawing::TextAnimationDirection             meDirection;

    // Get width per Step. Negative means pixel, positive logical units
    sal_Int32                                   mnStepWidth;

    // The single anim steps
    std::vector< ScrollTextAnimNode >           maVector;

    // the scroll rectangle
    tools::Rectangle                                   maScrollRectangleLogic;

    // the paint rectangle
    tools::Rectangle                                   maPaintRectangleLogic;
};


class IntrinsicAnimationListener : public IntrinsicAnimationEventHandler
{
public:
    explicit IntrinsicAnimationListener( ActivityImpl& rActivity ) :
        mrActivity( rActivity )
    {}

    IntrinsicAnimationListener(const IntrinsicAnimationListener&) = delete;
    IntrinsicAnimationListener& operator=(const IntrinsicAnimationListener&) = delete;

private:

    virtual bool enableAnimations() override { return mrActivity.enableAnimations(); }
    virtual bool disableAnimations() override { mrActivity.end(); return true; }

    ActivityImpl& mrActivity;
};


double ActivityImpl::GetMixerState( sal_uInt32 nTime )
{
    if( meAnimKind == drawing::TextAnimationKind_BLINK )
    {
        // from AInfoBlinkText:
        double fRetval(0.0);
        bool bDone(false);
        const sal_uInt32 nLoopTime(2 * mnFrequency);

        if(mnRepeat)
        {
            const sal_uInt32 nEndTime(mnRepeat * nLoopTime);

            if(nTime >= nEndTime)
            {
                if(mbVisibleWhenStopped)
                    fRetval = 0.0;
                else
                    fRetval = 1.0;

                bDone = true;
            }
        }

        if(!bDone)
        {
            sal_uInt32 nTimeInLoop(nTime % nLoopTime);
            fRetval = double(nTimeInLoop) / nLoopTime;
        }

        return fRetval;
    }
    else
    {
        // from AInfoScrollText:
        double fRetval(0.0);
        ImpForceScrollTextAnimNodes();

        if(!maVector.empty())
        {
            sal_uInt32 nRelativeTime;
            ScrollTextAnimNode* pNode =
                ImpGetScrollTextAnimNode(nTime, nRelativeTime);

            if(pNode)
            {
                // use node
                fRetval = pNode->GetStateAtRelativeTime(nRelativeTime);
            }
            else
            {
                // end of animation, take last entry's end
                fRetval = maVector[maVector.size() - 1].GetStop();
            }
        }

        return fRetval;
    }
}

// Access to StepWidth in logical units
sal_uInt32 ActivityImpl::GetStepWidthLogic() const
{
    // #i69847# Assuming higher DPI
    sal_uInt32 const PIXEL_TO_LOGIC = 30;

    sal_uInt32 nRetval(0);

    if(mnStepWidth < 0)
    {
        // is in pixels, convert to logical units
        nRetval = (-mnStepWidth * PIXEL_TO_LOGIC);
    }
    else if(mnStepWidth > 0)
    {
        // is in logical units
        nRetval = mnStepWidth;
    }

    if(0 == nRetval)
    {
        // step 1 pixel, canned value

        // with very high DPIs like in PDF export, this can
        // still get zero.  for that cases, set a default, too (taken
        // from ainfoscrolltext.cxx)
        nRetval = 100;
    }

    return nRetval;
}

void ActivityImpl::ImpForceScrollTextAnimNodes()
{
    if(!maVector.empty())
        return;

    // prepare values
    sal_uInt32 nLoopTime;
    double fZeroLogic, fOneLogic, fInitLogic, fDistanceLogic;
    double fZeroLogicAlternate = 0.0, fOneLogicAlternate = 0.0;
    double fZeroRelative, fOneRelative, fInitRelative;

    if(ScrollHorizontal())
    {
        if(DoAlternate())
        {
            if(maPaintRectangleLogic.GetWidth() >
               maScrollRectangleLogic.GetWidth())
            {
                fZeroLogicAlternate = maScrollRectangleLogic.Right() - maPaintRectangleLogic.GetWidth();
                fOneLogicAlternate = maScrollRectangleLogic.Left();
            }
            else
            {
                fZeroLogicAlternate = maScrollRectangleLogic.Left();
                fOneLogicAlternate = maScrollRectangleLogic.Right() - maPaintRectangleLogic.GetWidth();
            }
        }

        fZeroLogic = maScrollRectangleLogic.Left() - maPaintRectangleLogic.GetWidth();
        fOneLogic = maScrollRectangleLogic.Right();
        fInitLogic = maPaintRectangleLogic.Left();
    }
    else
    {
        if(DoAlternate())
        {
            if(maPaintRectangleLogic.GetHeight() > maScrollRectangleLogic.GetHeight())
            {
                fZeroLogicAlternate = maScrollRectangleLogic.Bottom() - maPaintRectangleLogic.GetHeight();
                fOneLogicAlternate = maScrollRectangleLogic.Top();
            }
            else
            {
                fZeroLogicAlternate = maScrollRectangleLogic.Top();
                fOneLogicAlternate = maScrollRectangleLogic.Bottom() - maPaintRectangleLogic.GetHeight();
            }
        }

        fZeroLogic = maScrollRectangleLogic.Top() - maPaintRectangleLogic.GetHeight();
        fOneLogic = maScrollRectangleLogic.Bottom();
        fInitLogic = maPaintRectangleLogic.Top();
    }

    fDistanceLogic = fOneLogic - fZeroLogic;
    fInitRelative = (fInitLogic - fZeroLogic) / fDistanceLogic;

    if(DoAlternate())
    {
        fZeroRelative =
            (fZeroLogicAlternate - fZeroLogic) / fDistanceLogic;
        fOneRelative =
            (fOneLogicAlternate - fZeroLogic) / fDistanceLogic;
    }
    else
    {
        fZeroRelative = 0.0;
        fOneRelative = 1.0;
    }

    if(mbVisibleWhenStarted)
    {
        double fRelativeStartValue, fRelativeEndValue,fRelativeDistance;

        if(DoScrollForward())
        {
            fRelativeStartValue = fInitRelative;
            fRelativeEndValue = fOneRelative;
            fRelativeDistance = fRelativeEndValue - fRelativeStartValue;
        }
        else
        {
            fRelativeStartValue = fInitRelative;
            fRelativeEndValue = fZeroRelative;
            fRelativeDistance = fRelativeStartValue - fRelativeEndValue;
        }

        const double fNumberSteps =
            (fRelativeDistance * fDistanceLogic) / GetStepWidthLogic();
        nLoopTime = FRound(fNumberSteps * mnFrequency);

        // init loop
        ScrollTextAnimNode aInitNode(
            nLoopTime, 1,
            fRelativeStartValue, fRelativeEndValue,
            mnFrequency, false);
        maVector.push_back(aInitNode);
    }

    // prepare main loop values
    {
        double fRelativeStartValue, fRelativeEndValue, fRelativeDistance;

        if(DoScrollForward())
        {
            fRelativeStartValue = fZeroRelative;
            fRelativeEndValue = fOneRelative;
            fRelativeDistance = fRelativeEndValue - fRelativeStartValue;
        }
        else
        {
            fRelativeStartValue = fOneRelative;
            fRelativeEndValue = fZeroRelative;
            fRelativeDistance = fRelativeStartValue - fRelativeEndValue;
        }

        const double fNumberSteps =
            (fRelativeDistance * fDistanceLogic) / GetStepWidthLogic();
        nLoopTime = FRound(fNumberSteps * mnFrequency);

        if(0 == mnRepeat)
        {
            if(!DoScrollIn())
            {
                // endless main loop
                ScrollTextAnimNode aMainNode(
                    nLoopTime, 0,
                    fRelativeStartValue, fRelativeEndValue,
                    mnFrequency, DoAlternate());
                maVector.push_back(aMainNode);
            }
        }
        else
        {
            sal_uInt32 nNumRepeat(mnRepeat);

            if(DoAlternate() && (nNumRepeat + 1) % 2L)
                nNumRepeat += 1;

            // ending main loop
            ScrollTextAnimNode aMainNode(
                nLoopTime, nNumRepeat,
                fRelativeStartValue, fRelativeEndValue,
                mnFrequency, DoAlternate());
            maVector.push_back(aMainNode);
        }
    }

    if(!mbVisibleWhenStopped)
        return;

    double fRelativeStartValue, fRelativeEndValue, fRelativeDistance;

    if(DoScrollForward())
    {
        fRelativeStartValue = fZeroRelative;
        fRelativeEndValue = fInitRelative;
        fRelativeDistance = fRelativeEndValue - fRelativeStartValue;
    }
    else
    {
        fRelativeStartValue = fOneRelative;
        fRelativeEndValue = fInitRelative;
        fRelativeDistance = fRelativeStartValue - fRelativeEndValue;
    }

    const double fNumberSteps =
        (fRelativeDistance * fDistanceLogic) / GetStepWidthLogic();
    nLoopTime = FRound(fNumberSteps * mnFrequency);

    // exit loop
    ScrollTextAnimNode aExitNode(
        nLoopTime, 1,
        fRelativeStartValue, fRelativeEndValue, mnFrequency, false);
    maVector.push_back(aExitNode);
}

ScrollTextAnimNode* ActivityImpl::ImpGetScrollTextAnimNode(
    sal_uInt32 nTime, sal_uInt32& rRelativeTime )
{
    ScrollTextAnimNode* pRetval = nullptr;
    ImpForceScrollTextAnimNodes();

    if(!maVector.empty())
    {
        rRelativeTime = nTime;

        for(ScrollTextAnimNode & rNode: maVector)
        {
            if(!rNode.GetRepeat())
            {
                // endless loop, use it
                pRetval = &rNode;
            }
            else if(rNode.GetFullTime() > rRelativeTime)
            {
                // ending node
                pRetval = &rNode;
            }
            else
            {
                // look at next
                rRelativeTime -= rNode.GetFullTime();
            }
        }
    }

    return pRetval;
}

sal_uInt32 ActivityImpl::ImpRegisterAgainScrollTextMixerState(sal_uInt32 nTime)
{
    sal_uInt32 nRetval(0);
    ImpForceScrollTextAnimNodes();

    if(!maVector.empty())
    {
        sal_uInt32 nRelativeTime;
        ScrollTextAnimNode* pNode = ImpGetScrollTextAnimNode(nTime, nRelativeTime);

        if(pNode)
        {
            // take register time
            nRetval = pNode->GetFrequency();
        }
    }
    else
    {
        // #i38135# not initialized, return default
        nRetval = mnFrequency;
    }

    return nRetval;
}

void ActivityImpl::updateShapeAttributes(
    double fTime, basegfx::B2DRectangle const& parentBounds )
{
    OSL_ASSERT( meAnimKind != drawing::TextAnimationKind_NONE );
    if( meAnimKind == drawing::TextAnimationKind_NONE )
        return;

    double const fMixerState = GetMixerState(
        static_cast<sal_uInt32>(fTime * 1000.0) );

    if( meAnimKind == drawing::TextAnimationKind_BLINK )
    {
        // show/hide text:
        maShapeAttrLayer.get()->setVisibility( fMixerState < 0.5 );
    }
    else if(mpMetaFile) // scroll mode:
    {

        // keep care: the below code is highly sensible to changes...


        // rectangle of the pure text:
        double const fPaintWidth = maPaintRectangleLogic.GetWidth();
        double const fPaintHeight = maPaintRectangleLogic.GetHeight();
        // rectangle where the scrolling takes place (-> clipping):
        double const fScrollWidth = maScrollRectangleLogic.GetWidth();
        double const fScrollHeight = maScrollRectangleLogic.GetHeight();

        basegfx::B2DPoint pos, clipPos;

        if(ScrollHorizontal())
        {
            double const fOneEquiv( fScrollWidth );
            double const fZeroEquiv( -fPaintWidth );

            pos.setX( fZeroEquiv + (fMixerState * (fOneEquiv - fZeroEquiv)) );

            clipPos.setX( -pos.getX() );
            clipPos.setY( -pos.getY() );

            // #i69844# Compensation for text-wider-than-shape case
            if( fPaintWidth > fScrollWidth )
                pos.setX( pos.getX() + (fPaintWidth-fScrollWidth) / 2.0 );
        }
        else
        {
            // scroll vertical:
            double const fOneEquiv( fScrollHeight );
            double const fZeroEquiv( -fPaintHeight );

            pos.setY( fZeroEquiv + (fMixerState * (fOneEquiv - fZeroEquiv)) );

            clipPos.setX( -pos.getX() );
            clipPos.setY( -pos.getY() );

            // #i69844# Compensation for text-higher-than-shape case
            if( fPaintHeight > fScrollHeight )
                pos.setY( pos.getY() + (fPaintHeight-fScrollHeight) / 2.0 );
        }

        basegfx::B2DPolygon clipPoly(
            basegfx::utils::createPolygonFromRect(
                basegfx::B2DRectangle( clipPos.getX(),
                                       clipPos.getY(),
                                       clipPos.getX() + fScrollWidth,
                                       clipPos.getY() + fScrollHeight ) ) );

        if( !::basegfx::fTools::equalZero( mfRotationAngle ))
        {
            maShapeAttrLayer.get()->setRotationAngle( mfRotationAngle );
            double const fRotate = basegfx::deg2rad(mfRotationAngle);
            basegfx::B2DHomMatrix aTransform;
            // position:
            aTransform.rotate( fRotate );
            pos *= aTransform;
        }

        pos += parentBounds.getCenter();
        maShapeAttrLayer.get()->setPosition( pos );
        maShapeAttrLayer.get()->setClip( basegfx::B2DPolyPolygon(clipPoly) );
    }
}

bool ActivityImpl::perform()
{
    if( !isActive() )
        return false;

    ENSURE_OR_RETURN_FALSE(
        mpDrawShape,
        "ActivityImpl::perform(): still active, but NULL draw shape" );

    DrawShapeSharedPtr const pParentDrawShape( mpParentDrawShape );
    if( !pParentDrawShape )
        return false; // parent has vanished

    if( pParentDrawShape->isVisible() )
    {
        if( !mbIsShapeAnimated )
        {
            mpDrawShape->setVisibility(true); // shape may be initially hidden
            maContext.mpSubsettableShapeManager->enterAnimationMode( mpDrawShape );
            maTimer.reset();
            mbIsShapeAnimated = true;
        }
        // update attributes related to current time:
        basegfx::B2DRectangle const parentBounds(
            pParentDrawShape->getBounds() );

        const double nCurrTime( maTimer.getElapsedTime() );
        updateShapeAttributes( nCurrTime, parentBounds );

        const sal_uInt32 nFrequency(
            ImpRegisterAgainScrollTextMixerState(
                static_cast<sal_uInt32>(nCurrTime * 1000.0)) );

        if(nFrequency)
        {
            mpWakeupEvent->start();
            mpWakeupEvent->setNextTimeout(
                std::max(0.1,nFrequency/1000.0) );
            maContext.mrEventQueue.addEvent( mpWakeupEvent );

            if( mpDrawShape->isContentChanged() )
                maContext.mpSubsettableShapeManager->notifyShapeUpdate( mpDrawShape );
        }
        // else: finished, not need to wake up again.
    }
    else
    {
        // busy-wait, until parent shape gets visible
        mpWakeupEvent->start();
        mpWakeupEvent->setNextTimeout( 2.0 );
    }

    // don't reinsert, WakeupEvent will perform that after the given timeout:
    return false;
}

ActivityImpl::ActivityImpl(
    SlideShowContext const& rContext,
    std::shared_ptr<WakeupEvent> const& pWakeupEvent,
    std::shared_ptr<DrawShape> const& pParentDrawShape )
    : maContext(rContext),
      mpWakeupEvent(pWakeupEvent),
      mpParentDrawShape(pParentDrawShape),
      mpListener( new IntrinsicAnimationListener(*this) ),
      maTimer(rContext.mrEventQueue.getTimer()),
      mfRotationAngle(0.0),
      mbIsShapeAnimated(false),
      mbIsDisposed(false),
      mbIsActive(true),
      meAnimKind(drawing::TextAnimationKind_NONE),
      mbVisibleWhenStopped(false),
      mbVisibleWhenStarted(false),
      mnStepWidth(0)
{
    // get doctreenode:
    sal_Int32 const nNodes = pParentDrawShape->getNumberOfTreeNodes(
        DocTreeNode::NodeType::LogicalParagraph );

    DocTreeNode scrollTextNode(
        pParentDrawShape->getTreeNode(
            0, DocTreeNode::NodeType::LogicalParagraph ));
    // xxx todo: remove this hack
    if( nNodes > 1 )
        scrollTextNode.setEndIndex(
            pParentDrawShape->getTreeNode(
                nNodes - 1,
                DocTreeNode::NodeType::LogicalParagraph ).getEndIndex());

    // TODO(Q3): Doing this manually, instead of using
    // ShapeSubset. This is because of lifetime issues (ShapeSubset
    // generates circular references to parent shape)
    mpDrawShape = std::dynamic_pointer_cast<DrawShape>(
        maContext.mpSubsettableShapeManager->getSubsetShape(
            pParentDrawShape,
            scrollTextNode ));

    mpMetaFile = mpDrawShape->forceScrollTextMetaFile();

    // make scroll text invisible for slide transition bitmaps
    mpDrawShape->setVisibility(false);

    basegfx::B2DRectangle aScrollRect, aPaintRect;
    ENSURE_OR_THROW( getRectanglesFromScrollMtf( aScrollRect,
                                                  aPaintRect,
                                                  mpMetaFile ),
                      "ActivityImpl::ActivityImpl(): Could not extract "
                      "scroll anim rectangles from mtf" );

    maScrollRectangleLogic = vcl::unotools::rectangleFromB2DRectangle(
        aScrollRect );
    maPaintRectangleLogic = vcl::unotools::rectangleFromB2DRectangle(
        aPaintRect );

    maShapeAttrLayer.createAttributeLayer(mpDrawShape);

    uno::Reference<drawing::XShape> const xShape( mpDrawShape->getXShape() );
    uno::Reference<beans::XPropertySet> const xProps( xShape, uno::UNO_QUERY_THROW );

    getPropertyValue( meAnimKind, xProps, "TextAnimationKind" );
    OSL_ASSERT( meAnimKind != drawing::TextAnimationKind_NONE );
    mbAlternate = (meAnimKind == drawing::TextAnimationKind_ALTERNATE);
    mbScrollIn = (meAnimKind == drawing::TextAnimationKind_SLIDE);

    // adopted from in AInfoBlinkText::ImplInit():
    sal_Int16 nRepeat(0);
    getPropertyValue( nRepeat, xProps, "TextAnimationCount" );
    mnRepeat = nRepeat;

    if(mbAlternate)
    {
        // force visible when started for scroll-forth-and-back, because
        // slide has been coming in with visible text in the middle:
        mbVisibleWhenStarted = true;
    }
    else
    {
        getPropertyValue( mbVisibleWhenStarted, xProps,
                          "TextAnimationStartInside" );
    }

    // set visible when stopped
    getPropertyValue( mbVisibleWhenStopped, xProps,
                      "TextAnimatiogonStopInside" );
    // rotation:
    getPropertyValue( mfRotationAngle, xProps,
                      "RotateAngle" );
    mfRotationAngle /= -100.0; // (switching direction)

    // set frequency
    sal_Int16 nDelay(0);
    getPropertyValue( nDelay, xProps, "TextAnimationDelay" );
    // set delay if not automatic
    mnFrequency = (nDelay ? nDelay :
                   // default:
                   meAnimKind == drawing::TextAnimationKind_BLINK
                   ? 250 : 50 );

    // adopted from in AInfoScrollText::ImplInit():

    // If it is a simple m_bScrollIn, reset some parameters
    if( DoScrollIn() )
    {
        // most parameters are set correctly from the dialog logic, but
        // eg VisibleWhenStopped is grayed out and needs to be corrected here.
        mbVisibleWhenStopped = true;
        mbVisibleWhenStarted = false;
        mnRepeat = 0;
    }

    // Get animation direction
    getPropertyValue( meDirection, xProps, "TextAnimationDirection" );

    // Get step width. Negative means pixel, positive logical units
    getPropertyValue( mnStepWidth, xProps, "TextAnimationAmount" );

    maContext.mpSubsettableShapeManager->addIntrinsicAnimationHandler(
        mpListener );
}

bool ActivityImpl::enableAnimations()
{
    mbIsActive = true;
    return maContext.mrActivitiesQueue.addActivity( std::dynamic_pointer_cast<Activity>(shared_from_this()) );
}

void ActivityImpl::dispose()
{
    if( mbIsDisposed )
        return;

    end();

    // only remove subset here, since end() is called on slide end
    // (and we must not spoil the slide preview bitmap with scroll
    // text)
    maShapeAttrLayer.reset();
    if( mpDrawShape )
    {
        // TODO(Q3): Doing this manually, instead of using
        // ShapeSubset. This is because of lifetime issues
        // (ShapeSubset generates circular references to parent
        // shape)
        DrawShapeSharedPtr pParent( mpParentDrawShape.lock() );
        if( pParent )
            maContext.mpSubsettableShapeManager->revokeSubset(
                pParent,
                mpDrawShape );
    }

    mpMetaFile.reset();
    mpDrawShape.reset();
    mpParentDrawShape.reset();
    mpWakeupEvent.reset();
    maContext.dispose();
    mbIsDisposed = true;

    maContext.mpSubsettableShapeManager->removeIntrinsicAnimationHandler(
        mpListener );
}

double ActivityImpl::calcTimeLag() const
{
    return 0.0;
}

bool ActivityImpl::isActive() const
{
    return mbIsActive;
}

void ActivityImpl::dequeued()
{
    // not used here
}

void ActivityImpl::end()
{
    // not used here
    mbIsActive = false;

    if( mbIsShapeAnimated )
    {
        maContext.mpSubsettableShapeManager->leaveAnimationMode( mpDrawShape );
        mbIsShapeAnimated = false;
    }
}

} // anon namespace

namespace slideshow {
namespace internal {

std::shared_ptr<Activity> createDrawingLayerAnimActivity(
    SlideShowContext const& rContext,
    std::shared_ptr<DrawShape> const& pDrawShape )
{
    std::shared_ptr<Activity> pActivity;

    try
    {
        std::shared_ptr<WakeupEvent> const pWakeupEvent(
            new WakeupEvent( rContext.mrEventQueue.getTimer(),
                             rContext.mrActivitiesQueue ) );
        pActivity.reset( new ActivityImpl( rContext, pWakeupEvent, pDrawShape ) );
        pWakeupEvent->setActivity( pActivity );
    }
    catch( uno::RuntimeException& )
    {
        throw;
    }
    catch( uno::Exception& )
    {
        // translate any error into empty factory product.
        SAL_WARN( "slideshow", exceptionToString( cppu::getCaughtException() ) );
    }

    return pActivity;
}

} // namespace internal
} // namespace presentation

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
