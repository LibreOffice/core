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

#include "drawshapesubsetting.hxx"
#include "subsettableshapemanager.hxx"
#include "eventqueue.hxx"
#include "eventmultiplexer.hxx"
#include "intrinsicanimationactivity.hxx"
#include "intrinsicanimationeventhandler.hxx"

#include <boost/noncopyable.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/weak_ptr.hpp>

namespace slideshow
{
    namespace internal
    {
        /** Activity for intrinsic shape animations

            This is an Activity interface implementation for intrinsic
            shape animations. Intrinsic shape animations are
            animations directly within a shape, e.g. drawing layer
            animations, or GIF animations.
         */
        class IntrinsicAnimationActivity : public Activity,
                                           public boost::enable_shared_from_this<IntrinsicAnimationActivity>,
                                           private boost::noncopyable
        {
        public:
            /** Create an IntrinsicAnimationActivity.

                @param rContext
                Common slideshow objects

                @param rDrawShape
                Shape to control the intrinsic animation for

                @param rWakeupEvent
                Externally generated wakeup event, to set this
                activity to sleep during inter-frame intervals. Must
                come from the outside, since wakeup event and this
                object have mutual references to each other.

                @param rTimeouts
                Vector of timeout values, to wait before the next
                frame is shown.
             */
            IntrinsicAnimationActivity( const SlideShowContext&         rContext,
                                        const DrawShapeSharedPtr&       rDrawShape,
                                        const WakeupEventSharedPtr&     rWakeupEvent,
                                        const ::std::vector<double>&    rTimeouts,
                                        ::std::size_t                   nNumLoops,
                                        CycleMode                       eCycleMode );

            virtual void dispose() override;
            virtual double calcTimeLag() const override;
            virtual bool perform() override;
            virtual bool isActive() const override;
            virtual void dequeued() override;
            virtual void end() override;

            bool enableAnimations();

        private:
            SlideShowContext                        maContext;
            boost::weak_ptr<DrawShape>              mpDrawShape;
            WakeupEventSharedPtr                    mpWakeupEvent;
            IntrinsicAnimationEventHandlerSharedPtr mpListener;
            ::std::vector<double>                   maTimeouts;
            CycleMode                               meCycleMode;
            ::std::size_t                           mnCurrIndex;
            ::std::size_t                           mnNumLoops;
            ::std::size_t                           mnLoopCount;
            bool                                    mbIsActive;
        };



        class IntrinsicAnimationListener : public IntrinsicAnimationEventHandler,
                                           private boost::noncopyable
        {
        public:
            explicit IntrinsicAnimationListener( IntrinsicAnimationActivity& rActivity ) :
                mrActivity( rActivity )
            {}

        private:

            virtual bool enableAnimations() override { return mrActivity.enableAnimations(); }
            virtual bool disableAnimations() override { mrActivity.end(); return true; }

            IntrinsicAnimationActivity& mrActivity;
        };



        IntrinsicAnimationActivity::IntrinsicAnimationActivity( const SlideShowContext&         rContext,
                                                                const DrawShapeSharedPtr&       rDrawShape,
                                                                const WakeupEventSharedPtr&     rWakeupEvent,
                                                                const ::std::vector<double>&    rTimeouts,
                                                                ::std::size_t                   nNumLoops,
                                                                CycleMode                       eCycleMode ) :
            maContext( rContext ),
            mpDrawShape( rDrawShape ),
            mpWakeupEvent( rWakeupEvent ),
            mpListener( new IntrinsicAnimationListener(*this) ),
            maTimeouts( rTimeouts ),
            meCycleMode( eCycleMode ),
            mnCurrIndex(0),
            mnNumLoops(nNumLoops),
            mnLoopCount(0),
            mbIsActive(false)
        {
            ENSURE_OR_THROW( rContext.mpSubsettableShapeManager,
                              "IntrinsicAnimationActivity::IntrinsicAnimationActivity(): Invalid shape manager" );
            ENSURE_OR_THROW( rDrawShape,
                              "IntrinsicAnimationActivity::IntrinsicAnimationActivity(): Invalid draw shape" );
            ENSURE_OR_THROW( rWakeupEvent,
                              "IntrinsicAnimationActivity::IntrinsicAnimationActivity(): Invalid wakeup event" );
            ENSURE_OR_THROW( !rTimeouts.empty(),
                              "IntrinsicAnimationActivity::IntrinsicAnimationActivity(): Empty timeout vector" );

            maContext.mpSubsettableShapeManager->addIntrinsicAnimationHandler(
                mpListener );
        }

        void IntrinsicAnimationActivity::dispose()
        {
            end();

            if( mpWakeupEvent )
                mpWakeupEvent->dispose();

            maContext.dispose();
            mpDrawShape.reset();
            mpWakeupEvent.reset();
            maTimeouts.clear();
            mnCurrIndex = 0;

            maContext.mpSubsettableShapeManager->removeIntrinsicAnimationHandler(
                mpListener );
        }

        double IntrinsicAnimationActivity::calcTimeLag() const
        {
            return 0.0;
        }

        bool IntrinsicAnimationActivity::perform()
        {
            if( !isActive() )
                return false;

            DrawShapeSharedPtr pDrawShape( mpDrawShape.lock() );
            if( !pDrawShape || !mpWakeupEvent )
            {
                // event or draw shape vanished, no sense living on ->
                // commit suicide.
                dispose();
                return false;
            }

            // mnNumLoops == 0 means infinite looping
            if( mnNumLoops != 0 &&
                mnLoopCount >= mnNumLoops )
            {
                // #i55294# After finishing the loops, display the first frame
                pDrawShape->setIntrinsicAnimationFrame( 0 );
                maContext.mpSubsettableShapeManager->notifyShapeUpdate( pDrawShape );

                end();

                return false;
            }

            ::std::size_t       nNewIndex = 0;
            const ::std::size_t nNumFrames(maTimeouts.size());
            switch( meCycleMode )
            {
                case CYCLE_LOOP:
                {
                    pDrawShape->setIntrinsicAnimationFrame( mnCurrIndex );

                    mpWakeupEvent->start();
                    mpWakeupEvent->setNextTimeout( maTimeouts[mnCurrIndex] );

                    mnLoopCount += (mnCurrIndex + 1) / nNumFrames;
                    nNewIndex = (mnCurrIndex + 1) % nNumFrames;
                    break;
                }

                case CYCLE_PINGPONGLOOP:
                {
                    ::std::size_t nTrueIndex( mnCurrIndex < nNumFrames ?
                                              mnCurrIndex :
                                              2*nNumFrames - mnCurrIndex - 1 );
                    pDrawShape->setIntrinsicAnimationFrame( nTrueIndex );

                    mpWakeupEvent->start();
                    mpWakeupEvent->setNextTimeout( maTimeouts[nTrueIndex] );

                    mnLoopCount += (mnCurrIndex + 1) / (2*nNumFrames);
                    nNewIndex = (mnCurrIndex + 1) % 2*nNumFrames;
                    break;
                }
            }

            maContext.mrEventQueue.addEvent( mpWakeupEvent );
            maContext.mpSubsettableShapeManager->notifyShapeUpdate( pDrawShape );
            mnCurrIndex = nNewIndex;

            return false; // don't reinsert, WakeupEvent will perform
                          // that after the given timeout
        }

        bool IntrinsicAnimationActivity::isActive() const
        {
            return mbIsActive;
        }

        void IntrinsicAnimationActivity::dequeued()
        {
            // not used here
        }

        void IntrinsicAnimationActivity::end()
        {
            // there is no dedicated end state, just become inactive:
            mbIsActive = false;
        }

        bool IntrinsicAnimationActivity::enableAnimations()
        {
            mbIsActive = true;
            return maContext.mrActivitiesQueue.addActivity(
                shared_from_this() );
        }



        ActivitySharedPtr createIntrinsicAnimationActivity(
            const SlideShowContext&         rContext,
            const DrawShapeSharedPtr&       rDrawShape,
            const WakeupEventSharedPtr&     rWakeupEvent,
            const ::std::vector<double>&    rTimeouts,
            ::std::size_t                   nNumLoops,
            CycleMode                       eCycleMode )
        {
            return ActivitySharedPtr(
                new IntrinsicAnimationActivity(rContext,
                                               rDrawShape,
                                               rWakeupEvent,
                                               rTimeouts,
                                               nNumLoops,
                                               eCycleMode) );
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
