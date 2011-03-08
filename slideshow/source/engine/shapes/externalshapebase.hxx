/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef INCLUDED_SLIDESHOW_EXTERNALSHAPEBASE_HXX
#define INCLUDED_SLIDESHOW_EXTERNALSHAPEBASE_HXX

#include <vector>

#include "externalmediashape.hxx"
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
        class ExternalShapeBase : public ExternalMediaShape
        {
        public:
            /** Create a shape for the given XShape for an external shape

                @param xShape
                The XShape to represent.

                @param nPrio
                Externally-determined shape priority (used e.g. for
                paint ordering). This number _must be_ unique!
             */
            ExternalShapeBase( const ::com::sun::star::uno::Reference<
                                     ::com::sun::star::drawing::XShape >&   xShape,
                               double                                       nPrio,
                               const SlideShowContext&                      rContext ); // throw ShapeLoadFailedException;
            virtual ~ExternalShapeBase();

            virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::drawing::XShape > getXShape() const;

            // animation methods
            //------------------------------------------------------------------

            virtual void play();
            virtual void stop();
            virtual void pause();
            virtual bool isPlaying() const;
            virtual void setMediaTime(double);

            // render methods
            //------------------------------------------------------------------

            virtual bool update() const;
            virtual bool render() const;
            virtual bool isContentChanged() const;


            // Shape attributes
            //------------------------------------------------------------------

            virtual ::basegfx::B2DRectangle getBounds() const;
            virtual ::basegfx::B2DRectangle getDomBounds() const;
            virtual ::basegfx::B2DRectangle getUpdateArea() const;
            virtual bool   isVisible() const;
            virtual double getPriority() const;
            virtual bool   isBackgroundDetached() const;

        protected:
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::uno::XComponentContext>  mxComponentContext;

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
            ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >   mxShape;

            boost::shared_ptr<ExternalShapeBaseListener>                            mpListener;

            SubsettableShapeManagerSharedPtr                                        mpShapeManager;
            EventMultiplexer&                                                       mrEventMultiplexer;

            // The attributes of this Shape
            const double                                                            mnPriority;
            ::basegfx::B2DRectangle                                                 maBounds;
        };
    }
}

#endif /* INCLUDED_SLIDESHOW_EXTERNALSHAPEBASE_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
