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

#include <svx/sdr/contact/viewobjectcontact.hxx>
#include <svx/sdr/contact/viewcontact.hxx>
#include <svx/sdr/contact/objectcontact.hxx>
#include <svx/sdr/contact/displayinfo.hxx>
#include <vcl/region.hxx>
#include <svx/sdr/animation/objectanimator.hxx>
#include <svx/sdr/animation/animationstate.hxx>
#include <svx/sdr/contact/viewobjectcontactredirector.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/color/bcolor.hxx>
#include <drawinglayer/primitive2d/modifiedcolorprimitive2d.hxx>
#include <basegfx/tools/canvastools.hxx>
#include <drawinglayer/primitive2d/animatedprimitive2d.hxx>
#include <drawinglayer/processor2d/baseprocessor2d.hxx>
#include <svx/sdr/primitive2d/svx_primitivetypes2d.hxx>

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace
{
    

    
    
    
    
    
    
    
    class AnimatedExtractingProcessor2D : public drawinglayer::processor2d::BaseProcessor2D
    {
    protected:
        
        drawinglayer::primitive2d::Primitive2DSequence  maPrimitive2DSequence;

        
        
        bool                                            mbTextAnimationAllowed : 1;

        
        bool                                            mbGraphicAnimationAllowed : 1;

        
        
        virtual void processBasePrimitive2D(const drawinglayer::primitive2d::BasePrimitive2D& rCandidate);

    public:
        AnimatedExtractingProcessor2D(
            const drawinglayer::geometry::ViewInformation2D& rViewInformation,
            bool bTextAnimationAllowed,
            bool bGraphicAnimationAllowed);
        virtual ~AnimatedExtractingProcessor2D();

        
        const drawinglayer::primitive2d::Primitive2DSequence& getPrimitive2DSequence() const { return maPrimitive2DSequence; }
        bool isTextAnimationAllowed() const { return mbTextAnimationAllowed; }
        bool isGraphicAnimationAllowed() const { return mbGraphicAnimationAllowed; }
    };

    AnimatedExtractingProcessor2D::AnimatedExtractingProcessor2D(
        const drawinglayer::geometry::ViewInformation2D& rViewInformation,
        bool bTextAnimationAllowed,
        bool bGraphicAnimationAllowed)
    :   drawinglayer::processor2d::BaseProcessor2D(rViewInformation),
        maPrimitive2DSequence(),
        mbTextAnimationAllowed(bTextAnimationAllowed),
        mbGraphicAnimationAllowed(bGraphicAnimationAllowed)
    {
    }

    AnimatedExtractingProcessor2D::~AnimatedExtractingProcessor2D()
    {
    }

    void AnimatedExtractingProcessor2D::processBasePrimitive2D(const drawinglayer::primitive2d::BasePrimitive2D& rCandidate)
    {
        
        switch(rCandidate.getPrimitive2DID())
        {
            
            case PRIMITIVE2D_ID_ANIMATEDSWITCHPRIMITIVE2D :
            case PRIMITIVE2D_ID_ANIMATEDBLINKPRIMITIVE2D :
            case PRIMITIVE2D_ID_ANIMATEDINTERPOLATEPRIMITIVE2D :
            {
                const drawinglayer::primitive2d::AnimatedSwitchPrimitive2D& rSwitchPrimitive = static_cast< const drawinglayer::primitive2d::AnimatedSwitchPrimitive2D& >(rCandidate);

                if((rSwitchPrimitive.isTextAnimation() && isTextAnimationAllowed())
                    || (rSwitchPrimitive.isGraphicAnimation() && isGraphicAnimationAllowed()))
                {
                    const drawinglayer::primitive2d::Primitive2DReference xReference(const_cast< drawinglayer::primitive2d::BasePrimitive2D* >(&rCandidate));
                    drawinglayer::primitive2d::appendPrimitive2DReferenceToPrimitive2DSequence(maPrimitive2DSequence, xReference);
                }
                break;
            }

            
            
            case PRIMITIVE2D_ID_SDRGRAFPRIMITIVE2D :
            case PRIMITIVE2D_ID_GRAPHICPRIMITIVE2D :

            
            case PRIMITIVE2D_ID_SDRCAPTIONPRIMITIVE2D :
            case PRIMITIVE2D_ID_SDRCONNECTORPRIMITIVE2D :
            case PRIMITIVE2D_ID_SDRCUSTOMSHAPEPRIMITIVE2D :
            case PRIMITIVE2D_ID_SDRELLIPSEPRIMITIVE2D :
            case PRIMITIVE2D_ID_SDRELLIPSESEGMENTPRIMITIVE2D :
            case PRIMITIVE2D_ID_SDRMEASUREPRIMITIVE2D :
            case PRIMITIVE2D_ID_SDRPATHPRIMITIVE2D :
            case PRIMITIVE2D_ID_SDRRECTANGLEPRIMITIVE2D :

            
            
            case PRIMITIVE2D_ID_POLYPOLYGONGRAPHICPRIMITIVE2D:
            case PRIMITIVE2D_ID_FILLGRAPHICPRIMITIVE2D:
            case PRIMITIVE2D_ID_TRANSFORMPRIMITIVE2D:

            
            
            case PRIMITIVE2D_ID_MASKPRIMITIVE2D :
            case PRIMITIVE2D_ID_GROUPPRIMITIVE2D :
            {
                process(rCandidate.get2DDecomposition(getViewInformation2D()));
                break;
            }

            default :
            {
                
                break;
            }
        }
    }
} 

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        ViewObjectContact::ViewObjectContact(ObjectContact& rObjectContact, ViewContact& rViewContact)
        :   mrObjectContact(rObjectContact),
            mrViewContact(rViewContact),
            maObjectRange(),
            mxPrimitive2DSequence(),
            mpPrimitiveAnimation(0),
            mbLazyInvalidate(false)
        {
            
            mrViewContact.AddViewObjectContact(*this);

            
            mrObjectContact.AddViewObjectContact(*this);
        }

        ViewObjectContact::~ViewObjectContact()
        {
            
            if(!maObjectRange.isEmpty())
            {
                GetObjectContact().InvalidatePartOfView(maObjectRange);
            }

            
            if(mpPrimitiveAnimation)
            {
                delete mpPrimitiveAnimation;
                mpPrimitiveAnimation = 0;
            }

            
            
            
            
            
            
            GetObjectContact().RemoveViewObjectContact(*this);

            
            GetViewContact().RemoveViewObjectContact(*this);
        }

        const basegfx::B2DRange& ViewObjectContact::getObjectRange() const
        {
            if(maObjectRange.isEmpty())
            {
                
                const DisplayInfo aDisplayInfo;
                const drawinglayer::primitive2d::Primitive2DSequence xSequence(getPrimitive2DSequence(aDisplayInfo));

                if(xSequence.hasElements())
                {
                    const drawinglayer::geometry::ViewInformation2D& rViewInformation2D(GetObjectContact().getViewInformation2D());
                    const_cast< ViewObjectContact* >(this)->maObjectRange =
                        drawinglayer::primitive2d::getB2DRangeFromPrimitive2DSequence(xSequence, rViewInformation2D);
                }
            }

            return maObjectRange;
        }

        void ViewObjectContact::ActionChanged()
        {
            if(!mbLazyInvalidate)
            {
                
                mbLazyInvalidate = true;

                
                getObjectRange();

                if(!maObjectRange.isEmpty())
                {
                    
                    GetObjectContact().InvalidatePartOfView(maObjectRange);

                    
                    maObjectRange.reset();
                }

                
                GetObjectContact().setLazyInvalidate(*this);
            }
        }

        void ViewObjectContact::triggerLazyInvalidate()
        {
            if(mbLazyInvalidate)
            {
                
                mbLazyInvalidate = false;

                
                getObjectRange();

                if(!maObjectRange.isEmpty())
                {
                    
                    GetObjectContact().InvalidatePartOfView(maObjectRange);
                }
            }
        }

        
        void ViewObjectContact::ActionChildInserted(ViewContact& rChild)
        {
            
            
            rChild.GetViewObjectContact(GetObjectContact()).ActionChanged();

            
            
            
        }

        void ViewObjectContact::checkForPrimitive2DAnimations()
        {
            
            if(mpPrimitiveAnimation)
            {
                delete mpPrimitiveAnimation;
                mpPrimitiveAnimation = 0;
            }

            
            if(mxPrimitive2DSequence.hasElements())
            {
                const bool bTextAnimationAllowed(GetObjectContact().IsTextAnimationAllowed());
                const bool bGraphicAnimationAllowed(GetObjectContact().IsGraphicAnimationAllowed());

                if(bTextAnimationAllowed || bGraphicAnimationAllowed)
                {
                    AnimatedExtractingProcessor2D aAnimatedExtractor(GetObjectContact().getViewInformation2D(),
                        bTextAnimationAllowed, bGraphicAnimationAllowed);
                    aAnimatedExtractor.process(mxPrimitive2DSequence);

                    if(aAnimatedExtractor.getPrimitive2DSequence().hasElements())
                    {
                        
                        mpPrimitiveAnimation =  new sdr::animation::PrimitiveAnimation(*this, aAnimatedExtractor.getPrimitive2DSequence());
                    }
                }
            }
        }

        drawinglayer::primitive2d::Primitive2DSequence ViewObjectContact::createPrimitive2DSequence(const DisplayInfo& rDisplayInfo) const
        {
            
            drawinglayer::primitive2d::Primitive2DSequence xRetval(GetViewContact().getViewIndependentPrimitive2DSequence());

            if(xRetval.hasElements())
            {
                
                if(!GetObjectContact().isOutputToPrinter() && GetObjectContact().AreGluePointsVisible())
                {
                    const drawinglayer::primitive2d::Primitive2DSequence xGlue(GetViewContact().createGluePointPrimitive2DSequence());

                    if(xGlue.hasElements())
                    {
                        drawinglayer::primitive2d::appendPrimitive2DSequenceToPrimitive2DSequence(xRetval, xGlue);
                    }
                }

                
                if(isPrimitiveGhosted(rDisplayInfo))
                {
                    const basegfx::BColor aRGBWhite(1.0, 1.0, 1.0);
                    const basegfx::BColorModifierSharedPtr aBColorModifier(
                        new basegfx::BColorModifier_interpolate(
                            aRGBWhite,
                            0.5));
                    const drawinglayer::primitive2d::Primitive2DReference xReference(
                        new drawinglayer::primitive2d::ModifiedColorPrimitive2D(
                            xRetval,
                            aBColorModifier));

                    xRetval = drawinglayer::primitive2d::Primitive2DSequence(&xReference, 1);
                }
            }

            return xRetval;
        }

        drawinglayer::primitive2d::Primitive2DSequence ViewObjectContact::getPrimitive2DSequence(const DisplayInfo& rDisplayInfo) const
        {
            drawinglayer::primitive2d::Primitive2DSequence xNewPrimitiveSequence;

            
            ViewObjectContactRedirector* pRedirector = GetObjectContact().GetViewObjectContactRedirector();

            if(pRedirector)
            {
                xNewPrimitiveSequence = pRedirector->createRedirectedPrimitive2DSequence(*this, rDisplayInfo);
            }
            else
            {
                xNewPrimitiveSequence = createPrimitive2DSequence(rDisplayInfo);
            }

            
            if(!drawinglayer::primitive2d::arePrimitive2DSequencesEqual(mxPrimitive2DSequence, xNewPrimitiveSequence))
            {
                
                const_cast< ViewObjectContact* >(this)->mxPrimitive2DSequence = xNewPrimitiveSequence;

                
                const_cast< ViewObjectContact* >(this)->checkForPrimitive2DAnimations();

                
                const drawinglayer::geometry::ViewInformation2D& rViewInformation2D(GetObjectContact().getViewInformation2D());
                const_cast< ViewObjectContact* >(this)->maObjectRange =
                    drawinglayer::primitive2d::getB2DRangeFromPrimitive2DSequence(mxPrimitive2DSequence, rViewInformation2D);
            }

            
            return mxPrimitive2DSequence;
        }

        bool ViewObjectContact::isPrimitiveVisible(const DisplayInfo& /*rDisplayInfo*/) const
        {
            
            return true;
        }

        bool ViewObjectContact::isPrimitiveGhosted(const DisplayInfo& rDisplayInfo) const
        {
            
            return (GetObjectContact().DoVisualizeEnteredGroup() && !GetObjectContact().isOutputToPrinter() && rDisplayInfo.IsGhostedDrawModeActive());
        }

        drawinglayer::primitive2d::Primitive2DSequence ViewObjectContact::getPrimitive2DSequenceHierarchy(DisplayInfo& rDisplayInfo) const
        {
            drawinglayer::primitive2d::Primitive2DSequence xRetval;

            
            if(isPrimitiveVisible(rDisplayInfo))
            {
                xRetval = getPrimitive2DSequence(rDisplayInfo);

                if(xRetval.hasElements())
                {
                    
                    const drawinglayer::geometry::ViewInformation2D& rViewInformation2D(GetObjectContact().getViewInformation2D());
                    const basegfx::B2DRange aObjectRange(drawinglayer::primitive2d::getB2DRangeFromPrimitive2DSequence(xRetval, rViewInformation2D));
                    const basegfx::B2DRange aViewRange(rViewInformation2D.getViewport());

                    
                    if(!aViewRange.isEmpty() && !aViewRange.overlaps(aObjectRange))
                    {
                        
                        xRetval.realloc(0);
                    }
                }
            }

            return xRetval;
        }

        drawinglayer::primitive2d::Primitive2DSequence ViewObjectContact::getPrimitive2DSequenceSubHierarchy(DisplayInfo& rDisplayInfo) const
        {
            const sal_uInt32 nSubHierarchyCount(GetViewContact().GetObjectCount());
            drawinglayer::primitive2d::Primitive2DSequence xSeqRetval;

            for(sal_uInt32 a(0); a < nSubHierarchyCount; a++)
            {
                const ViewObjectContact& rCandidate(GetViewContact().GetViewContact(a).GetViewObjectContact(GetObjectContact()));

                drawinglayer::primitive2d::appendPrimitive2DSequenceToPrimitive2DSequence(xSeqRetval, rCandidate.getPrimitive2DSequenceHierarchy(rDisplayInfo));
            }

            return xSeqRetval;
        }
    } 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
