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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_slideshow.hxx"

// must be first
#include <canvas/debug.hxx>
#include <tools/diagnose_ex.h>
#include <canvas/verbosetrace.hxx>

#include <comphelper/anytostring.hxx>
#include <cppuhelper/exc_hlp.hxx>

#include "externalshapebase.hxx"
#include "eventmultiplexer.hxx"
#include "vieweventhandler.hxx"
#include "intrinsicanimationeventhandler.hxx"
#include "tools.hxx"

#include <boost/noncopyable.hpp>


using namespace ::com::sun::star;


namespace slideshow
{
    namespace internal
    {
        class ExternalShapeBase::ExternalShapeBaseListener : public ViewEventHandler,
                                                             public IntrinsicAnimationEventHandler,
                                                             private boost::noncopyable
        {
        public:
            explicit ExternalShapeBaseListener( ExternalShapeBase& rBase ) :
                mrBase( rBase )
            {}


        private:
            // ViewEventHandler
            // -------------------------------------------------

            virtual void viewAdded( const UnoViewSharedPtr& ) {}
            virtual void viewRemoved( const UnoViewSharedPtr& ) {}
            virtual void viewChanged( const UnoViewSharedPtr& rView )
            {
                mrBase.implViewChanged(rView);
            }
            virtual void viewsChanged()
            {
                mrBase.implViewsChanged();
            }


            // IntrinsicAnimationEventHandler
            // -------------------------------------------------

            virtual bool enableAnimations()
            {
                return mrBase.implStartIntrinsicAnimation();
            }
            virtual bool disableAnimations()
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

        // ---------------------------------------------------------------------

        ExternalShapeBase::~ExternalShapeBase()
        {
            try
            {
                mrEventMultiplexer.removeViewHandler( mpListener );
                mpShapeManager->removeIntrinsicAnimationHandler( mpListener );
            }
            catch (uno::Exception &)
            {
                OSL_FAIL( rtl::OUStringToOString(
                                comphelper::anyToString(
                                    cppu::getCaughtException() ),
                                RTL_TEXTENCODING_UTF8 ).getStr() );
            }
        }

        // ---------------------------------------------------------------------

        uno::Reference< drawing::XShape > ExternalShapeBase::getXShape() const
        {
            return mxShape;
        }

        // ---------------------------------------------------------------------

        void ExternalShapeBase::play()
        {
            implStartIntrinsicAnimation();
        }

        // ---------------------------------------------------------------------

        void ExternalShapeBase::stop()
        {
            implEndIntrinsicAnimation();
        }

        // ---------------------------------------------------------------------

        void ExternalShapeBase::pause()
        {
            implPauseIntrinsicAnimation();
        }

        // ---------------------------------------------------------------------

        bool ExternalShapeBase::isPlaying() const
        {
            return implIsIntrinsicAnimationPlaying();
        }

        // ---------------------------------------------------------------------

        void ExternalShapeBase::setMediaTime(double fTime)
        {
            implSetIntrinsicAnimationTime(fTime);
        }

        // ---------------------------------------------------------------------

        bool ExternalShapeBase::update() const
        {
            return render();
        }

        // ---------------------------------------------------------------------

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

        // ---------------------------------------------------------------------

        bool ExternalShapeBase::isContentChanged() const
        {
            return true;
        }

        // ---------------------------------------------------------------------

        ::basegfx::B2DRectangle ExternalShapeBase::getBounds() const
        {
            return maBounds;
        }

        // ---------------------------------------------------------------------

        ::basegfx::B2DRectangle ExternalShapeBase::getDomBounds() const
        {
            return maBounds;
        }

        // ---------------------------------------------------------------------

        ::basegfx::B2DRectangle ExternalShapeBase::getUpdateArea() const
        {
            return maBounds;
        }

        // ---------------------------------------------------------------------

        bool ExternalShapeBase::isVisible() const
        {
            return true;
        }

        // ---------------------------------------------------------------------

        double ExternalShapeBase::getPriority() const
        {
            return mnPriority;
        }

        // ---------------------------------------------------------------------

        bool ExternalShapeBase::isBackgroundDetached() const
        {
            // external shapes always have their own window/surface
            return true;
        }

    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
