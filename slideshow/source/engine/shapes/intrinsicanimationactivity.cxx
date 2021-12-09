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

#include <subsettableshapemanager.hxx>
#include <eventqueue.hxx>
#include "intrinsicanimationactivity.hxx"
#include <intrinsicanimationeventhandler.hxx>

#include <memory>

namespace slideshow::internal
{
        namespace {

        /** Activity for intrinsic shape animations

            This is an Activity interface implementation for intrinsic
            shape animations. Intrinsic shape animations are
            animations directly within a shape, e.g. drawing layer
            animations, or GIF animations.
         */
        class IntrinsicAnimationActivity : public Activity
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
                                        ::std::vector<double>&&         rTimeouts,
                                        ::std::size_t                   nNumLoops );
            IntrinsicAnimationActivity(const IntrinsicAnimationActivity&) = delete;
            IntrinsicAnimationActivity& operator=(const IntrinsicAnimationActivity&) = delete;

            virtual void dispose() override;
            virtual double calcTimeLag() const override;
            virtual bool perform() override;
            virtual bool isActive() const override;
            virtual void dequeued() override;
            virtual void end() override;

            bool enableAnimations();

        private:
            SlideShowContext                        maContext;
            std::weak_ptr<DrawShape>              mpDrawShape;
            WakeupEventSharedPtr                    mpWakeupEvent;
            IntrinsicAnimationEventHandlerSharedPtr mpListener;
            ::std::vector<double>                   maTimeouts;
            ::std::size_t                           mnCurrIndex;
            ::std::size_t                           mnNumLoops;
            ::std::size_t                           mnLoopCount;
            bool                                    mbIsActive;
        };


        class IntrinsicAnimationListener : public IntrinsicAnimationEventHandler
        {
        public:
            explicit IntrinsicAnimationListener( IntrinsicAnimationActivity& rActivity ) :
                mrActivity( rActivity )
            {}
            IntrinsicAnimationListener(const IntrinsicAnimationListener&) = delete;
            IntrinsicAnimationListener& operator=(const IntrinsicAnimationListener&) = delete;

        private:

            virtual bool enableAnimations() override { return mrActivity.enableAnimations(); }
            virtual bool disableAnimations() override { mrActivity.end(); return true; }

            IntrinsicAnimationActivity& mrActivity;
        };

        }

        IntrinsicAnimationActivity::IntrinsicAnimationActivity( const SlideShowContext&         rContext,
                                                                const DrawShapeSharedPtr&       rDrawShape,
                                                                const WakeupEventSharedPtr&     rWakeupEvent,
                                                                ::std::vector<double>&&         rTimeouts,
                                                                ::std::size_t                   nNumLoops ) :
            maContext( rContext ),
            mpDrawShape( rDrawShape ),
            mpWakeupEvent( rWakeupEvent ),
            mpListener( std::make_shared<IntrinsicAnimationListener>(*this) ),
            maTimeouts( std::move(rTimeouts) ),
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
            ENSURE_OR_THROW( !maTimeouts.empty(),
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

            const ::std::size_t nNumFrames(maTimeouts.size());

            // mnNumLoops == 0 means infinite looping
            if( mnNumLoops != 0 &&
                mnLoopCount >= mnNumLoops )
            {
                // #i55294# After finishing the loops, display the last frame
                // powerpoint 2013 and firefox etc show the last frame when
                // the animation ends
                pDrawShape->setIntrinsicAnimationFrame(nNumFrames - 1);
                maContext.mpSubsettableShapeManager->notifyShapeUpdate( pDrawShape );

                end();

                return false;
            }

            ::std::size_t       nNewIndex = 0;

            pDrawShape->setIntrinsicAnimationFrame( mnCurrIndex );

            mpWakeupEvent->start();
            mpWakeupEvent->setNextTimeout( maTimeouts[mnCurrIndex] );

            mnLoopCount += (mnCurrIndex + 1) / nNumFrames;
            nNewIndex = (mnCurrIndex + 1) % nNumFrames;

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
            return maContext.mrActivitiesQueue.addActivity( std::dynamic_pointer_cast<Activity>(shared_from_this()) );

        }


        ActivitySharedPtr createIntrinsicAnimationActivity(
            const SlideShowContext&         rContext,
            const DrawShapeSharedPtr&       rDrawShape,
            const WakeupEventSharedPtr&     rWakeupEvent,
            ::std::vector<double>&&         rTimeouts,
            sal_uInt32                      nNumLoops)
        {
            return std::make_shared<IntrinsicAnimationActivity>(rContext,
                                               rDrawShape,
                                               rWakeupEvent,
                                               std::move(rTimeouts),
                                               nNumLoops);
        }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
