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


#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/awt/FontWeight.hpp>

#include <vcl/metaact.hxx>
#include <vcl/gdimtf.hxx>

#include <basegfx/numeric/ftools.hxx>
#include <sal/log.hxx>

#include <cmath>
#include <algorithm>
#include <functional>
#include <limits>

#include "backgroundshape.hxx"
#include <slideshowexceptions.hxx>
#include <slideshowcontext.hxx>
#include "gdimtftools.hxx"
#include <shape.hxx>
#include "viewbackgroundshape.hxx"


using namespace ::com::sun::star;


namespace slideshow
{
    namespace internal
    {
        /** Representation of a draw document's background shape.

            This class implements the Shape interface for the
            background shape. Since the background shape is neither
            animatable nor attributable, those more specialized
            derivations of the Shape interface are not implemented
            here.

            @attention this class is to be treated 'final', i.e. one
            should not derive from it.
         */
        class BackgroundShape : public Shape
        {
        public:
            /** Create the background shape.

                This method creates a shape that handles the
                peculiarities of the draw API regarding background
                content.
             */
            BackgroundShape( const css::uno::Reference< css::drawing::XDrawPage >& xDrawPage,
                             const css::uno::Reference< css::drawing::XDrawPage >& xMasterPage,
                             const SlideShowContext&                    rContext ); // throw ShapeLoadFailedException;

            virtual css::uno::Reference<
                css::drawing::XShape > getXShape() const override;

            // View layer methods


            virtual void addViewLayer( const ViewLayerSharedPtr&    rNewLayer,
                                       bool                         bRedrawLayer ) override;
            virtual bool removeViewLayer( const ViewLayerSharedPtr& rNewLayer ) override;
            virtual void clearAllViewLayers() override;


            // attribute methods


            virtual ::basegfx::B2DRectangle getBounds() const override;
            virtual ::basegfx::B2DRectangle getDomBounds() const override;
            virtual ::basegfx::B2DRectangle getUpdateArea() const override;
            virtual bool isVisible() const override;
            virtual double getPriority() const override;
            virtual bool isBackgroundDetached() const override;


            // render methods


            virtual bool update() const override;
            virtual bool render() const override;
            virtual bool isContentChanged() const override;

        private:
            /// The metafile actually representing the Shape
            GDIMetaFileSharedPtr        mpMtf;

            // The attributes of this Shape
            ::basegfx::B2DRectangle     maBounds; // always needed for rendering

            /// the list of active view shapes (one for each registered view layer)
            typedef ::std::vector< ViewBackgroundShapeSharedPtr > ViewBackgroundShapeVector;
            ViewBackgroundShapeVector   maViewShapes;
        };


        BackgroundShape::BackgroundShape( const uno::Reference< drawing::XDrawPage >& xDrawPage,
                                          const uno::Reference< drawing::XDrawPage >& xMasterPage,
                                          const SlideShowContext&                     rContext ) :
            mpMtf(),
            maBounds(),
            maViewShapes()
        {
            uno::Reference< beans::XPropertySet > xPropSet( xDrawPage,
                                                            uno::UNO_QUERY_THROW );
            // first try the page background (overrides
            // masterpage background), then try masterpage
            GDIMetaFileSharedPtr xMtf = getMetaFile(uno::Reference<lang::XComponent>(xDrawPage, uno::UNO_QUERY),
                                                    xDrawPage, MTF_LOAD_BACKGROUND_ONLY,
                                                    rContext.mxComponentContext);

            if (!xMtf)
            {
                xMtf = getMetaFile( uno::Reference<lang::XComponent>(xMasterPage, uno::UNO_QUERY),
                                    xDrawPage, MTF_LOAD_BACKGROUND_ONLY,
                                    rContext.mxComponentContext );
            }

            if (!xMtf)
            {
                throw ShapeLoadFailedException();
            }

            // there is a special background shape, add it
            // as the first one

            sal_Int32 nDocWidth=0;
            sal_Int32 nDocHeight=0;
            xPropSet->getPropertyValue("Width") >>= nDocWidth;
            xPropSet->getPropertyValue("Height") >>= nDocHeight;

            mpMtf = xMtf;
            maBounds = ::basegfx::B2DRectangle( 0,0,nDocWidth, nDocHeight );
        }

        uno::Reference< drawing::XShape > BackgroundShape::getXShape() const
        {
            // no real XShape representative
            return uno::Reference< drawing::XShape >();
        }

        void BackgroundShape::addViewLayer( const ViewLayerSharedPtr&   rNewLayer,
                                            bool                        bRedrawLayer )
        {
            // already added?
            if( ::std::any_of( maViewShapes.begin(),
                               maViewShapes.end(),
                               [&rNewLayer]( const ViewBackgroundShapeSharedPtr& pBgShape )
                               { return pBgShape->getViewLayer() == rNewLayer; } ) )
            {
                // yes, nothing to do
                return;
            }

            maViewShapes.push_back(
                std::make_shared<ViewBackgroundShape>(
                    rNewLayer, maBounds ) );

            // render the Shape on the newly added ViewLayer
            if( bRedrawLayer )
                maViewShapes.back()->render( mpMtf );
        }

        bool BackgroundShape::removeViewLayer( const ViewLayerSharedPtr& rLayer )
        {
            const ViewBackgroundShapeVector::iterator aEnd( maViewShapes.end() );

            OSL_ENSURE( ::std::count_if(maViewShapes.begin(),
                                        aEnd,
                                        [&rLayer]( const ViewBackgroundShapeSharedPtr& pBgShape )
                                        { return pBgShape->getViewLayer() == rLayer; } ) < 2,
                        "BackgroundShape::removeViewLayer(): Duplicate ViewLayer entries!" );

            ViewBackgroundShapeVector::iterator aIter;

            if( (aIter=::std::remove_if( maViewShapes.begin(),
                                         aEnd,
                                         [&rLayer]( const ViewBackgroundShapeSharedPtr& pBgShape )
                                         { return pBgShape->getViewLayer() == rLayer; } )) == aEnd )
            {
                // view layer seemingly was not added, failed
                return false;
            }

            // actually erase from container
            maViewShapes.erase( aIter, aEnd );

            return true;
        }

        void BackgroundShape::clearAllViewLayers()
        {
            maViewShapes.clear();
        }

        ::basegfx::B2DRectangle BackgroundShape::getBounds() const
        {
            return maBounds;
        }

        ::basegfx::B2DRectangle BackgroundShape::getDomBounds() const
        {
            return maBounds;
        }

        ::basegfx::B2DRectangle BackgroundShape::getUpdateArea() const
        {
            // TODO(F1): Need to expand background, too, when
            // antialiasing?

            // no transformation etc. possible for background shape
            return maBounds;
        }

        bool BackgroundShape::isVisible() const
        {
            return true;
        }

        double BackgroundShape::getPriority() const
        {
            return 0.0; // lowest prio, we're the background
        }

        bool BackgroundShape::update() const
        {
            return render();
        }

        bool BackgroundShape::render() const
        {
            SAL_INFO( "slideshow", "::presentation::internal::BackgroundShape::render()" );
            SAL_INFO( "slideshow", "::presentation::internal::BackgroundShape: 0x" << std::hex << this );

            // gcc again...
            const ::basegfx::B2DRectangle& rCurrBounds( BackgroundShape::getBounds() );

            if( rCurrBounds.getRange().equalZero() )
            {
                // zero-sized shapes are effectively invisible,
                // thus, we save us the rendering...
                return true;
            }

            // redraw all view shapes, by calling their render() method
            if( ::std::count_if( maViewShapes.begin(),
                                 maViewShapes.end(),
                                 [this]( const ViewBackgroundShapeSharedPtr& pBgShape )
                                 { return pBgShape->render( this->mpMtf ); } )
                != static_cast<ViewBackgroundShapeVector::difference_type>(maViewShapes.size()) )
            {
                // at least one of the ViewBackgroundShape::render() calls did return
                // false - update failed on at least one ViewLayer
                return false;
            }

            return true;
        }

        bool BackgroundShape::isContentChanged() const
        {
            return false;
        }

        bool BackgroundShape::isBackgroundDetached() const
        {
            return false; // we're not animatable
        }


       ShapeSharedPtr createBackgroundShape(
            const uno::Reference< drawing::XDrawPage >& xDrawPage,
            const uno::Reference< drawing::XDrawPage >& xMasterPage,
            const SlideShowContext&                     rContext )
        {
            return ShapeSharedPtr(
                new BackgroundShape(
                    xDrawPage,
                    xMasterPage,
                    rContext ));
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
