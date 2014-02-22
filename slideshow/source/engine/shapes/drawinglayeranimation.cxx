/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include <canvas/debug.hxx>
#include <tools/diagnose_ex.h>
#include <tools/helpers.hxx>
#include <canvas/elapsedtime.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>

#include <comphelper/anytostring.hxx>
#include <cppuhelper/exc_hlp.hxx>

#include <rtl/math.hxx>
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

#include "activity.hxx"
#include "wakeupevent.hxx"
#include "eventqueue.hxx"
#include "drawshapesubsetting.hxx"
#include "drawshape.hxx"
#include "shapesubset.hxx"
#include "shapeattributelayerholder.hxx"
#include "slideshowcontext.hxx"
#include "tools.hxx"
#include "gdimtftools.hxx"
#include "eventmultiplexer.hxx"
#include "intrinsicanimationactivity.hxx"
#include "intrinsicanimationeventhandler.hxx"

#include <boost/weak_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>
#include <vector>

using namespace com::sun::star;
using namespace ::slideshow::internal;

namespace {

class ScrollTextAnimNode
{
    sal_uInt32  mnDuration; 
    sal_uInt32  mnRepeat; 
    double      mfStart;
    double      mfStop;
    sal_uInt32  mnFrequency; 
    
    bool        mbAlternate;

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
    
    if( mnDuration == 0 )
        return mfStop;

    if(mnRepeat)
    {
        
        const sal_uInt32 nRepeatCount(nRelativeTime / mnDuration);
        sal_uInt32 nFrameTime(nRelativeTime - (nRepeatCount * mnDuration));

        if(DoAlternate() && (nRepeatCount + 1L) % 2L)
            nFrameTime = mnDuration - nFrameTime;

        return mfStart + ((mfStop - mfStart) *
                          (double(nFrameTime) / mnDuration));
    }
    else
    {
        
        sal_uInt32 nFrameTime(nRelativeTime % mnDuration);

        if(DoAlternate())
        {
            const sal_uInt32 nRepeatCount(nRelativeTime / mnDuration);

            if((nRepeatCount + 1L) % 2L)
                nFrameTime = mnDuration - nFrameTime;
        }

        return mfStart + ((mfStop - mfStart) * (double(nFrameTime) / mnDuration));
    }
}

class ActivityImpl : public Activity,
                     public boost::enable_shared_from_this<ActivityImpl>,
                     private boost::noncopyable
{
public:
    virtual ~ActivityImpl();

    ActivityImpl(
        SlideShowContext const& rContext,
        boost::shared_ptr<WakeupEvent> const& pWakeupEvent,
        boost::shared_ptr<DrawShape> const& pDrawShape );

    bool enableAnimations();

    
    virtual void dispose();
    
    virtual double calcTimeLag() const;
    virtual bool perform();
    virtual bool isActive() const;
    virtual void dequeued();
    virtual void end();

private:
    void updateShapeAttributes( double fTime,
                                basegfx::B2DRectangle const& parentBounds );

    
    sal_Bool IsVisibleWhenStarted() const { return mbVisibleWhenStarted; }
    sal_Bool IsVisibleWhenStopped() const { return mbVisibleWhenStopped; }

    
    bool ScrollHorizontal() const {
        return (drawing::TextAnimationDirection_LEFT == meDirection ||
                drawing::TextAnimationDirection_RIGHT == meDirection);
    }

    
    sal_uInt32 GetStepWidthLogic() const;

    
    bool DoScrollForward() const {
        return (drawing::TextAnimationDirection_RIGHT == meDirection ||
                drawing::TextAnimationDirection_DOWN == meDirection);
    }

    
    bool DoAlternate() const { return mbAlternate; }

    
    bool DoScrollIn() const { return mbScrollIn; }

    
    void ImpForceScrollTextAnimNodes();
    ScrollTextAnimNode* ImpGetScrollTextAnimNode(
        sal_uInt32 nTime, sal_uInt32& rRelativeTime );
    sal_uInt32 ImpRegisterAgainScrollTextMixerState(
        sal_uInt32 nTime);

    
    double GetMixerState(sal_uInt32 nTime);

    

    SlideShowContext                            maContext;
    boost::shared_ptr<WakeupEvent>              mpWakeupEvent;
    boost::weak_ptr<DrawShape>                  mpParentDrawShape;
    DrawShapeSharedPtr                          mpDrawShape;
    ShapeAttributeLayerHolder                   maShapeAttrLayer;
    GDIMetaFileSharedPtr                        mpMetaFile;
    IntrinsicAnimationEventHandlerSharedPtr     mpListener;
    canvas::tools::ElapsedTime                  maTimer;
    double                                      mfRotationAngle;
    bool                                        mbIsShapeAnimated;
    bool                                        mbIsDisposed;
    bool                                        mbIsActive;
    drawing::TextAnimationKind                  meAnimKind;

    
    sal_uInt32                                  mnFrequency;

    
    sal_uInt32                                  mnRepeat;

    
    bool                                        mbVisibleWhenStopped;
    bool                                        mbVisibleWhenStarted;

    
    bool                                        mbAlternate;

    
    bool                                        mbScrollIn;

    
    sal_uInt32                                  mnStartTime;

    
    drawing::TextAnimationDirection             meDirection;

    
    sal_Int32                                   mnStepWidth;

    
    std::vector< ScrollTextAnimNode >           maVector;

    
    Rectangle                                   maScrollRectangleLogic;

    
    Rectangle                                   maPaintRectangleLogic;
};



class IntrinsicAnimationListener : public IntrinsicAnimationEventHandler,
                                   private boost::noncopyable
{
public:
    explicit IntrinsicAnimationListener( ActivityImpl& rActivity ) :
        mrActivity( rActivity )
    {}

private:

    virtual bool enableAnimations() { return mrActivity.enableAnimations(); }
    virtual bool disableAnimations() { mrActivity.end(); return true; }

    ActivityImpl& mrActivity;
};



double ActivityImpl::GetMixerState( sal_uInt32 nTime )
{
    if( meAnimKind == drawing::TextAnimationKind_BLINK )
    {
        
        double fRetval(0.0);
        sal_Bool bDone(sal_False);
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

                bDone = sal_True;
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
        
        double fRetval(0.0);
        ImpForceScrollTextAnimNodes();

        if(!maVector.empty())
        {
            sal_uInt32 nRelativeTime;
            ScrollTextAnimNode* pNode =
                ImpGetScrollTextAnimNode(nTime, nRelativeTime);

            if(pNode)
            {
                
                fRetval = pNode->GetStateAtRelativeTime(nRelativeTime);
            }
            else
            {
                
                fRetval = maVector[maVector.size() - 1L].GetStop();
            }
        }

        return fRetval;
    }
}


sal_uInt32 ActivityImpl::GetStepWidthLogic() const
{
    
    sal_uInt32 const PIXEL_TO_LOGIC = 30;

    sal_uInt32 nRetval(0L);

    if(mnStepWidth < 0L)
    {
        
        nRetval = (-mnStepWidth * PIXEL_TO_LOGIC);
    }
    else if(mnStepWidth > 0L)
    {
        
        nRetval = mnStepWidth;
    }

    if(0L == nRetval)
    {
        

        
        
        
        nRetval = 100L;
    }

    return nRetval;
}

void ActivityImpl::ImpForceScrollTextAnimNodes()
{
    if(maVector.empty())
    {
        
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

        if(mnStartTime)
        {
            
            ScrollTextAnimNode aStartNode(
                mnStartTime, 1L, 0.0, 0.0, mnStartTime, false);
            maVector.push_back(aStartNode);
        }

        if(IsVisibleWhenStarted())
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

            
            ScrollTextAnimNode aInitNode(
                nLoopTime, 1L,
                fRelativeStartValue, fRelativeEndValue,
                mnFrequency, false);
            maVector.push_back(aInitNode);
        }

        
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

            if(0L == mnRepeat)
            {
                if(!DoScrollIn())
                {
                    
                    ScrollTextAnimNode aMainNode(
                        nLoopTime, 0L,
                        fRelativeStartValue, fRelativeEndValue,
                        mnFrequency, DoAlternate());
                    maVector.push_back(aMainNode);
                }
            }
            else
            {
                sal_uInt32 nNumRepeat(mnRepeat);

                if(DoAlternate() && (nNumRepeat + 1L) % 2L)
                    nNumRepeat += 1L;

                
                ScrollTextAnimNode aMainNode(
                    nLoopTime, nNumRepeat,
                    fRelativeStartValue, fRelativeEndValue,
                    mnFrequency, DoAlternate());
                maVector.push_back(aMainNode);
            }
        }

        if(IsVisibleWhenStopped())
        {
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

            
            ScrollTextAnimNode aExitNode(
                nLoopTime, 1L,
                fRelativeStartValue, fRelativeEndValue, mnFrequency, false);
            maVector.push_back(aExitNode);
        }
    }
}

ScrollTextAnimNode* ActivityImpl::ImpGetScrollTextAnimNode(
    sal_uInt32 nTime, sal_uInt32& rRelativeTime )
{
    ScrollTextAnimNode* pRetval = 0L;
    ImpForceScrollTextAnimNodes();

    if(!maVector.empty())
    {
        rRelativeTime = nTime;

        for(sal_uInt32 a(0L); !pRetval && a < maVector.size(); a++)
        {
            ScrollTextAnimNode & rNode = maVector[a];
            if(!rNode.GetRepeat())
            {
                
                pRetval = &rNode;
            }
            else if(rNode.GetFullTime() > rRelativeTime)
            {
                
                pRetval = &rNode;
            }
            else
            {
                
                rRelativeTime -= rNode.GetFullTime();
            }
        }
    }

    return pRetval;
}

sal_uInt32 ActivityImpl::ImpRegisterAgainScrollTextMixerState(sal_uInt32 nTime)
{
    sal_uInt32 nRetval(0L);
    ImpForceScrollTextAnimNodes();

    if(!maVector.empty())
    {
        sal_uInt32 nRelativeTime;
        ScrollTextAnimNode* pNode = ImpGetScrollTextAnimNode(nTime, nRelativeTime);

        if(pNode)
        {
            
            nRetval = pNode->GetFrequency();
        }
    }
    else
    {
        
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
        
        maShapeAttrLayer.get()->setVisibility( fMixerState < 0.5 );
    }
    else if(mpMetaFile) 
    {
        //
        
        //

        
        double const fPaintWidth = maPaintRectangleLogic.GetWidth();
        double const fPaintHeight = maPaintRectangleLogic.GetHeight();
        
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

            
            if( fPaintWidth > fScrollWidth )
                pos.setX( pos.getX() + (fPaintWidth-fScrollWidth) / 2.0 );
        }
        else
        {
            
            double const fOneEquiv( fScrollHeight );
            double const fZeroEquiv( -fPaintHeight );

            pos.setY( fZeroEquiv + (fMixerState * (fOneEquiv - fZeroEquiv)) );

            clipPos.setX( -pos.getX() );
            clipPos.setY( -pos.getY() );

            
            if( fPaintHeight > fScrollHeight )
                pos.setY( pos.getY() + (fPaintHeight-fScrollHeight) / 2.0 );
        }

        basegfx::B2DPolygon clipPoly(
            basegfx::tools::createPolygonFromRect(
                basegfx::B2DRectangle( clipPos.getX(),
                                       clipPos.getY(),
                                       clipPos.getX() + fScrollWidth,
                                       clipPos.getY() + fScrollHeight ) ) );

        if( !::basegfx::fTools::equalZero( mfRotationAngle ))
        {
            maShapeAttrLayer.get()->setRotationAngle( mfRotationAngle );
            double const fRotate = (mfRotationAngle * M_PI / 180.0);
            basegfx::B2DHomMatrix aTransform;
            
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
        return false; 

    if( pParentDrawShape->isVisible() )
    {
        if( !mbIsShapeAnimated )
        {
            mpDrawShape->setVisibility(true); 
            maContext.mpSubsettableShapeManager->enterAnimationMode( mpDrawShape );
            maTimer.reset();
            mbIsShapeAnimated = true;
        }
        
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
        
    }
    else
    {
        
        mpWakeupEvent->start();
        mpWakeupEvent->setNextTimeout( 2.0 );
    }

    
    return false;
}

ActivityImpl::ActivityImpl(
    SlideShowContext const& rContext,
    boost::shared_ptr<WakeupEvent> const& pWakeupEvent,
    boost::shared_ptr<DrawShape> const& pParentDrawShape )
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
      mnStartTime(0L)
{
    
    sal_Int32 const nNodes = pParentDrawShape->getNumberOfTreeNodes(
        DocTreeNode::NODETYPE_LOGICAL_PARAGRAPH );

    DocTreeNode scrollTextNode(
        pParentDrawShape->getTreeNode(
            0, DocTreeNode::NODETYPE_LOGICAL_PARAGRAPH ));
    
    if( nNodes > 1 )
        scrollTextNode.setEndIndex(
            pParentDrawShape->getTreeNode(
                nNodes - 1,
                DocTreeNode::NODETYPE_LOGICAL_PARAGRAPH ).getEndIndex());

    
    
    
    mpDrawShape = boost::dynamic_pointer_cast<DrawShape>(
        maContext.mpSubsettableShapeManager->getSubsetShape(
            pParentDrawShape,
            scrollTextNode ));

    mpMetaFile = mpDrawShape->forceScrollTextMetaFile();

    
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

    
    sal_Int16 nRepeat(0);
    getPropertyValue( nRepeat, xProps, "TextAnimationCount" );
    mnRepeat = nRepeat;

    if(mbAlternate)
    {
        
        
        mbVisibleWhenStarted = true;
    }
    else
    {
        getPropertyValue( mbVisibleWhenStarted, xProps,
                          "TextAnimationStartInside" );
    }

    
    getPropertyValue( mbVisibleWhenStopped, xProps,
                      "TextAnimatiogonStopInside" );
    
    getPropertyValue( mfRotationAngle, xProps,
                      "RotateAngle" );
    mfRotationAngle /= -100.0; 

    
    sal_Int16 nDelay(0);
    getPropertyValue( nDelay, xProps, "TextAnimationDelay" );
    
    mnFrequency = (nDelay ? nDelay :
                   
                   meAnimKind == drawing::TextAnimationKind_BLINK
                   ? 250L : 50L );

    

    
    if( DoScrollIn() )
    {
        
        
        mbVisibleWhenStopped = true;
        mbVisibleWhenStarted = false;
        mnRepeat = 0L;
    }

    
    getPropertyValue( meDirection, xProps, "TextAnimationDirection" );

    
    getPropertyValue( mnStepWidth, xProps, "TextAnimationAmount" );

    maContext.mpSubsettableShapeManager->addIntrinsicAnimationHandler(
        mpListener );
}

bool ActivityImpl::enableAnimations()
{
    mbIsActive = true;
    return maContext.mrActivitiesQueue.addActivity(
        shared_from_this() );
}

ActivityImpl::~ActivityImpl()
{
}

void ActivityImpl::dispose()
{
    if( !mbIsDisposed )
    {
        end();

        
        
        
        maShapeAttrLayer.reset();
        if( mpDrawShape )
        {
            
            
            
            
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
    
}

void ActivityImpl::end()
{
    
    mbIsActive = false;

    if( mbIsShapeAnimated )
    {
        maContext.mpSubsettableShapeManager->leaveAnimationMode( mpDrawShape );
        mbIsShapeAnimated = false;
    }
}

} 

namespace slideshow {
namespace internal {

boost::shared_ptr<Activity> createDrawingLayerAnimActivity(
    SlideShowContext const& rContext,
    boost::shared_ptr<DrawShape> const& pDrawShape )
{
    boost::shared_ptr<Activity> pActivity;

    try
    {
        boost::shared_ptr<WakeupEvent> const pWakeupEvent(
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
        
        OSL_FAIL( OUStringToOString(
                        comphelper::anyToString( cppu::getCaughtException() ),
                        RTL_TEXTENCODING_UTF8 ).getStr() );
    }

    return pActivity;
}

} 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
