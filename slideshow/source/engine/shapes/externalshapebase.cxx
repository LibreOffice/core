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


// must be first
#include <tools/diagnose_ex.h>
#include <sal/log.hxx>

#include <comphelper/anytostring.hxx>
#include <cppuhelper/exc_hlp.hxx>

#include "externalshapebase.hxx"
#include <eventmultiplexer.hxx>
#include <vieweventhandler.hxx>
#include <intrinsicanimationeventhandler.hxx>
#include <tools.hxx>


using namespace ::com::sun::star;


namespace slideshow
{
    namespace internal
    {
        class ExternalShapeBase::ExternalShapeBaseListener : public ViewEventHandler,
                                                             public IntrinsicAnimationEventHandler
        {
        public:
            explicit ExternalShapeBaseListener( ExternalShapeBase& rBase ) :
                mrBase( rBase )
            {}
            ExternalShapeBaseListener(const ExternalShapeBaseListener&) = delete;
            ExternalShapeBaseListener& operator=(const ExternalShapeBaseListener&) = delete;

        private:
            // ViewEventHandler


            virtual void viewAdded( const UnoViewSharedPtr& ) override {}
            virtual void viewRemoved( const UnoViewSharedPtr& ) override {}
            virtual void viewChanged( const UnoViewSharedPtr& rView ) override
            {
                mrBase.implViewChanged(rView);
            }
            virtual void viewsChanged() override
            {
                mrBase.implViewsChanged();
            }


            // IntrinsicAnimationEventHandler


            virtual bool enableAnimations() override
            {
                return mrBase.implStartIntrinsicAnimation();
            }
            virtual bool disableAnimations() override
            {
                return mrBase.implEndIntrinsicAnimation();
            }

            ExternalShapeBase& mrBase;
        };


        ExternalShapeBase::ExternalShapeBase( const uno::Reference< drawing::XShape >&  xShape,
                                              double                                    nPrio,
                                              const SlideShowContext&                   rContext ) :
            mxComponentContext( rContext.mxComponentContext ),
            mxShape( xShape ),
            mpListener( new ExternalShapeBaseListener(*this) ),
            mpShapeManager( rContext.mpSubsettableShapeManager ),
            mrEventMultiplexer( rContext.mrEventMultiplexer ),
            mnPriority( nPrio ), // TODO(F1): When ZOrder someday becomes usable: make this ( getAPIShapePrio( xShape ) ),
            maBounds( getAPIShapeBounds( xShape ) )
        {
            ENSURE_OR_THROW( mxShape.is(), "ExternalShapeBase::ExternalShapeBase(): Invalid XShape" );

            mpShapeManager->addIntrinsicAnimationHandler( mpListener );
            mrEventMultiplexer.addViewHandler( mpListener );
        }


        ExternalShapeBase::~ExternalShapeBase()
        {
            try
            {
                mrEventMultiplexer.removeViewHandler( mpListener );
                mpShapeManager->removeIntrinsicAnimationHandler( mpListener );
            }
            catch (uno::Exception &)
            {
                SAL_WARN( "slideshow", exceptionToString( cppu::getCaughtException() ) );
            }
        }


        uno::Reference< drawing::XShape > ExternalShapeBase::getXShape() const
        {
            return mxShape;
        }


        void ExternalShapeBase::play()
        {
            implStartIntrinsicAnimation();
        }


        void ExternalShapeBase::stop()
        {
            implEndIntrinsicAnimation();
        }


        void ExternalShapeBase::pause()
        {
            implPauseIntrinsicAnimation();
        }


        bool ExternalShapeBase::isPlaying() const
        {
            return implIsIntrinsicAnimationPlaying();
        }


        void ExternalShapeBase::setMediaTime(double fTime)
        {
            implSetIntrinsicAnimationTime(fTime);
        }


        bool ExternalShapeBase::update() const
        {
            return render();
        }


        bool ExternalShapeBase::render() const
        {
            if( maBounds.getRange().equalZero() )
            {
                // zero-sized shapes are effectively invisible,
                // thus, we save us the rendering...
                return true;
            }

            return implRender( maBounds );
        }


        bool ExternalShapeBase::isContentChanged() const
        {
            return true;
        }


        ::basegfx::B2DRectangle ExternalShapeBase::getBounds() const
        {
            return maBounds;
        }


        ::basegfx::B2DRectangle ExternalShapeBase::getDomBounds() const
        {
            return maBounds;
        }


        ::basegfx::B2DRectangle ExternalShapeBase::getUpdateArea() const
        {
            return maBounds;
        }


        bool ExternalShapeBase::isVisible() const
        {
            return true;
        }


        double ExternalShapeBase::getPriority() const
        {
            return mnPriority;
        }


        bool ExternalShapeBase::isBackgroundDetached() const
        {
            // external shapes always have their own window/surface
            return true;
        }

    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
