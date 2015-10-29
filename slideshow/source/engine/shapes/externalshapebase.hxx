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

#ifndef INCLUDED_SLIDESHOW_SOURCE_ENGINE_SHAPES_EXTERNALSHAPEBASE_HXX
#define INCLUDED_SLIDESHOW_SOURCE_ENGINE_SHAPES_EXTERNALSHAPEBASE_HXX

#include <vector>

#include "iexternalmediashapebase.hxx"
#include "unoview.hxx"
#include "subsettableshapemanager.hxx"
#include "slideshowexceptions.hxx"
#include "slideshowcontext.hxx"


namespace slideshow
{
    namespace internal
    {
        /** Base class for shapes rendered by external engines.

            Used as the common base for e.g. MediaShape or
            AppletShape, all of which are rendered by external
            components (and all employ distinct windows).

            Please note that this base class indeed assumes the shape
            does not interfere with the internal shapes in any way
            (including mutual overdraw). It therefore reports yes for
            the isBackgroundDetached() question.
         */
        class ExternalShapeBase : public IExternalMediaShapeBase
        {
        public:
            /** Create a shape for the given XShape for an external shape

                @param xShape
                The XShape to represent.

                @param nPrio
                Externally-determined shape priority (used e.g. for
                paint ordering). This number _must be_ unique!
             */
            ExternalShapeBase( const css::uno::Reference< css::drawing::XShape >&   xShape,
                               double                                       nPrio,
                               const SlideShowContext&                      rContext ); // throw ShapeLoadFailedException;
            virtual ~ExternalShapeBase();

            virtual css::uno::Reference< css::drawing::XShape > getXShape() const override;

            // animation methods


            virtual void play() override;
            virtual void stop() override;
            virtual void pause() override;
            virtual bool isPlaying() const override;
            virtual void setMediaTime(double) override;

            // render methods


            virtual bool update() const override;
            virtual bool render() const override;
            virtual bool isContentChanged() const override;


            // Shape attributes


            virtual ::basegfx::B2DRectangle getBounds() const override;
            virtual ::basegfx::B2DRectangle getDomBounds() const override;
            virtual ::basegfx::B2DRectangle getUpdateArea() const override;
            virtual bool   isVisible() const override;
            virtual double getPriority() const override;
            virtual bool   isBackgroundDetached() const override;

        protected:
            const css::uno::Reference<css::uno::XComponentContext>  mxComponentContext;

        private:
            class ExternalShapeBaseListener; friend class ExternalShapeBaseListener;

            /// override in derived class to render preview
            virtual bool implRender( const ::basegfx::B2DRange& rCurrBounds ) const = 0;

            /// override in derived class to resize
            virtual void implViewChanged( const UnoViewSharedPtr& rView ) = 0;
            /// override in derived class to resize
            virtual void implViewsChanged() = 0;

            /// override in derived class to start external viewer
            virtual bool implStartIntrinsicAnimation() = 0;
            /// override in derived class to stop external viewer
            virtual bool implEndIntrinsicAnimation() = 0;
            /// override in derived class to pause external viewer
            virtual bool implPauseIntrinsicAnimation() = 0;
            /// override in derived class to return status of animation
            virtual bool implIsIntrinsicAnimationPlaying() const = 0;
            /// override in derived class to set media time
            virtual void implSetIntrinsicAnimationTime(double) = 0;


            /// The associated XShape
            css::uno::Reference< css::drawing::XShape >                             mxShape;

            boost::shared_ptr<ExternalShapeBaseListener>                            mpListener;

            SubsettableShapeManagerSharedPtr                                        mpShapeManager;
            EventMultiplexer&                                                       mrEventMultiplexer;

            // The attributes of this Shape
            const double                                                            mnPriority;
            ::basegfx::B2DRectangle                                                 maBounds;
        };
    }
}

#endif // INCLUDED_SLIDESHOW_SOURCE_ENGINE_SHAPES_EXTERNALSHAPEBASE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
