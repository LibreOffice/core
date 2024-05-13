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


#include <comphelper/diagnose_ex.hxx>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/range/b2irange.hxx>
#include <basegfx/utils/canvastools.hxx>

#include <cppcanvas/canvas.hxx>
#include <canvas/canvastools.hxx>

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/rendering/XCanvas.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/awt/WindowDescriptor.hpp>
#include <com/sun/star/awt/Toolkit.hpp>
#include <com/sun/star/awt/XWindow2.hpp>
#include <com/sun/star/awt/XWindowPeer.hpp>
#include <com/sun/star/awt/WindowAttribute.hpp>
#include <com/sun/star/awt/VclWindowPeerAttribute.hpp>
#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/frame/Frame.hpp>
#include <com/sun/star/frame/XSynchronousFrameLoader.hpp>

#include "viewappletshape.hxx"
#include <tools.hxx>
#include <utility>


using namespace ::com::sun::star;

namespace slideshow::internal
{
        ViewAppletShape::ViewAppletShape( ViewLayerSharedPtr                              xViewLayer,
                                          const uno::Reference< drawing::XShape >&        rxShape,
                                          const OUString&                          rServiceName,
                                          const char**                                    pPropCopyTable,
                                          std::size_t                                     nNumPropEntries,
                                          uno::Reference< uno::XComponentContext >        xContext ) :
            mpViewLayer(std::move( xViewLayer )),
            mxViewer(),
            mxFrame(),
            mxComponentContext(std::move( xContext ))
        {
            ENSURE_OR_THROW( rxShape.is(), "ViewAppletShape::ViewAppletShape(): Invalid Shape" );
            ENSURE_OR_THROW( mpViewLayer, "ViewAppletShape::ViewAppletShape(): Invalid View" );
            ENSURE_OR_THROW( mpViewLayer->getCanvas(), "ViewAppletShape::ViewAppletShape(): Invalid ViewLayer canvas" );
            ENSURE_OR_THROW( mxComponentContext.is(), "ViewAppletShape::ViewAppletShape(): Invalid component context" );

            uno::Reference<lang::XMultiComponentFactory> xFactory(
                mxComponentContext->getServiceManager(),
                uno::UNO_SET_THROW );

            mxViewer.set( xFactory->createInstanceWithContext( rServiceName,
                                                               mxComponentContext),
                          uno::UNO_QUERY_THROW );

            uno::Reference< beans::XPropertySet > xShapePropSet( rxShape,
                                                                 uno::UNO_QUERY_THROW );
            uno::Reference< beans::XPropertySet > xViewerPropSet( mxViewer,
                                                                   uno::UNO_QUERY_THROW );

            // copy shape properties to applet viewer
            OUString aPropName;
            for( std::size_t i=0; i<nNumPropEntries; ++i )
            {
                aPropName = OUString::createFromAscii( pPropCopyTable[i] );
                xViewerPropSet->setPropertyValue( aPropName,
                                                  xShapePropSet->getPropertyValue(
                                                  aPropName ));
            }
        }

        ViewAppletShape::~ViewAppletShape()
        {
            try
            {
                endApplet();
            }
            catch (const uno::Exception &)
            {
                TOOLS_WARN_EXCEPTION("slideshow", "");
            }
        }

        const ViewLayerSharedPtr& ViewAppletShape::getViewLayer() const
        {
            return mpViewLayer;
        }

        void ViewAppletShape::startApplet( const ::basegfx::B2DRectangle& rBounds )
        {
            ENSURE_OR_RETURN_VOID( mpViewLayer && mpViewLayer->getCanvas() && mpViewLayer->getCanvas()->getUNOCanvas().is(),
                               "ViewAppletShape::startApplet(): Invalid or disposed view" );
            try
            {
                ::cppcanvas::CanvasSharedPtr pCanvas = mpViewLayer->getCanvas();

                uno::Reference< beans::XPropertySet > xPropSet( pCanvas->getUNOCanvas()->getDevice(),
                                                                uno::UNO_QUERY_THROW );

                uno::Reference< awt::XWindow2 > xParentWindow(
                    xPropSet->getPropertyValue(u"Window"_ustr),
                    uno::UNO_QUERY_THROW );

                uno::Reference<lang::XMultiComponentFactory> xFactory(
                    mxComponentContext->getServiceManager() );

                if( xFactory.is() )
                {
                    // create an awt window to contain the applet
                    // ==========================================

                    uno::Reference< awt::XToolkit2 > xToolkit = awt::Toolkit::create(mxComponentContext);

                    awt::WindowDescriptor aOwnWinDescriptor( awt::WindowClass_SIMPLE,
                                                             OUString(),
                                                             uno::Reference< awt::XWindowPeer >(xParentWindow,
                                                                                                uno::UNO_QUERY_THROW),
                                                             0,
                                                             awt::Rectangle(),
                                                             awt::WindowAttribute::SHOW
                                                             | awt::VclWindowPeerAttribute::CLIPCHILDREN );

                    uno::Reference< awt::XWindowPeer > xNewWinPeer(
                        xToolkit->createWindow( aOwnWinDescriptor ));
                    uno::Reference< awt::XWindow > xOwnWindow( xNewWinPeer,
                                                               uno::UNO_QUERY_THROW );


                    // create a frame, and load the applet into it
                    // ===========================================

                    mxFrame = frame::Frame::create( mxComponentContext );
                    mxFrame->initialize( xOwnWindow );

                    uno::Reference < frame::XSynchronousFrameLoader > xLoader( mxViewer,
                                                                               uno::UNO_SET_THROW );
                    xLoader->load( uno::Sequence < beans::PropertyValue >(),
                                   uno::Reference<frame::XFrame>(mxFrame, uno::UNO_QUERY_THROW) );


                    // resize surrounding window and applet to current shape size
                    // ==========================================================

                    ::basegfx::B2DRange aTmpRange;
                    ::canvas::tools::calcTransformedRectBounds( aTmpRange,
                                                                rBounds,
                                                                mpViewLayer->getTransformation() );
                    const ::basegfx::B2IRange& rPixelBounds(
                        ::basegfx::unotools::b2ISurroundingRangeFromB2DRange( aTmpRange ));

                    uno::Reference< awt::XWindow > xSurroundingWindow( mxFrame->getContainerWindow() );
                    if( xSurroundingWindow.is() )
                        xSurroundingWindow->setPosSize( rPixelBounds.getMinX(),
                                                        rPixelBounds.getMinY(),
                                                        static_cast<sal_Int32>(rPixelBounds.getWidth()),
                                                        static_cast<sal_Int32>(rPixelBounds.getHeight()),
                                                        awt::PosSize::POSSIZE );

                    uno::Reference< awt::XWindow > xAppletWindow( mxFrame->getComponentWindow() );
                    if( xAppletWindow.is() )
                        xAppletWindow->setPosSize( 0, 0,
                                                   static_cast<sal_Int32>(rPixelBounds.getWidth()),
                                                   static_cast<sal_Int32>(rPixelBounds.getHeight()),
                                                   awt::PosSize::POSSIZE );
                }
            }
            catch (uno::Exception &)
            {
            }
        }


        void ViewAppletShape::endApplet()
        {
            uno::Reference<util::XCloseable> xCloseable(
                mxFrame,
                uno::UNO_QUERY );

            if( xCloseable.is() )
            {
                xCloseable->close( true );
                mxFrame.clear();
            }
        }


        bool ViewAppletShape::render( const ::basegfx::B2DRectangle& rBounds ) const
        {
            ::cppcanvas::CanvasSharedPtr pCanvas = mpViewLayer->getCanvas();

            if( !pCanvas )
                return false;

            if( !mxFrame.is() )
            {
                // fill the shape background with black
                fillRect( pCanvas,
                          rBounds,
                          0xFFFFFFFFU );
            }

            return true;
        }

        bool ViewAppletShape::resize( const ::basegfx::B2DRectangle& rBounds ) const
        {
            if( !mxFrame.is() )
                return false;

            ::basegfx::B2DRange aTmpRange;
            ::canvas::tools::calcTransformedRectBounds( aTmpRange,
                                                        rBounds,
                                                        mpViewLayer->getTransformation() );
            const ::basegfx::B2IRange& rPixelBounds(
                ::basegfx::unotools::b2ISurroundingRangeFromB2DRange( aTmpRange ));

            uno::Reference< awt::XWindow > xFrameWindow( mxFrame->getContainerWindow() );
            if( xFrameWindow.is() )
                xFrameWindow->setPosSize( rPixelBounds.getMinX(),
                                          rPixelBounds.getMinY(),
                                          static_cast<sal_Int32>(rPixelBounds.getWidth()),
                                          static_cast<sal_Int32>(rPixelBounds.getHeight()),
                                          awt::PosSize::POSSIZE );

            uno::Reference< awt::XWindow > xAppletWindow( mxFrame->getComponentWindow() );
            if( xAppletWindow.is() )
                xAppletWindow->setPosSize( 0, 0,
                                           static_cast<sal_Int32>(rPixelBounds.getWidth()),
                                           static_cast<sal_Int32>(rPixelBounds.getHeight()),
                                           awt::PosSize::POSSIZE );

            return true;
        }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
