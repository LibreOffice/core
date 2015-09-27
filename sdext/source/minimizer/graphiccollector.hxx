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

#ifndef INCLUDED_SDEXT_SOURCE_MINIMIZER_GRAPHICCOLLECTOR_HXX
#define INCLUDED_SDEXT_SOURCE_MINIMIZER_GRAPHICCOLLECTOR_HXX

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/awt/DeviceInfo.hpp>
#include <com/sun/star/text/GraphicCrop.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <vector>


struct GraphicSettings
{
    bool    mbJPEGCompression;
    sal_Int32   mnJPEGQuality;
    bool    mbRemoveCropArea;
    sal_Int32   mnImageResolution;
    bool    mbEmbedLinkedGraphics;

    GraphicSettings( bool bJPEGCompression, sal_Int32 nJPEGQuality, bool bRemoveCropArea,
                        sal_Int32 nImageResolution, bool bEmbedLinkedGraphics )
    : mbJPEGCompression( bJPEGCompression )
    , mnJPEGQuality( nJPEGQuality )
    , mbRemoveCropArea( bRemoveCropArea )
    , mnImageResolution( nImageResolution )
    , mbEmbedLinkedGraphics( bEmbedLinkedGraphics ) {};
};

class GraphicCollector
{
    public:

    struct GraphicUser
    {
        css::uno::Reference< css::drawing::XShape >       mxShape;            // if mbFillBitmap is false the xShape has
        css::uno::Reference< css::beans::XPropertySet >   mxPropertySet;      // to be used otherwise the PropertySet
        css::uno::Reference< css::beans::XPropertySet >   mxPagePropertySet;
        OUString                       maGraphicURL;
        OUString                       maGraphicStreamURL;
        css::text::GraphicCrop         maGraphicCropLogic;
        css::awt::Size                 maLogicalSize;
        bool                            mbFillBitmap;

        GraphicUser() : mxShape(), maGraphicCropLogic( 0, 0, 0, 0 ), mbFillBitmap( false ) {};
    };

    struct GraphicEntity
    {
        css::awt::Size                              maLogicalSize;                          // the biggest logical size the graphic will be displayed
        bool                                        mbRemoveCropArea;
        css::text::GraphicCrop                      maGraphicCropLogic;
        std::vector< GraphicUser >                      maUser;

        explicit GraphicEntity( const GraphicUser& rUser )
            : maLogicalSize( rUser.maLogicalSize ), mbRemoveCropArea( false ), maGraphicCropLogic( 0, 0, 0, 0 ) { maUser.push_back( rUser ); };
    };

    static const css::awt::DeviceInfo& GetDeviceInfo( const css::uno::Reference< css::uno::XComponentContext >& rxFact );
    static css::awt::Size GetOriginalSize( const css::uno::Reference< css::uno::XComponentContext >& rxMSF,
                const css::uno::Reference< css::graphic::XGraphic >& rxGraphic );

    // collecting graphic instances, the downside of this method is that every graphic is swapped in
    static void CollectGraphics( const css::uno::Reference< css::uno::XComponentContext >& rxMSF, const css::uno::Reference< css::frame::XModel >& rxModel,
        const GraphicSettings& rGraphicSettings, std::vector< GraphicEntity >& io_rGraphicList );
    // counting graphics without swapping in graphics
    static void CountGraphics( const css::uno::Reference< css::uno::XComponentContext >& rxMSF, const css::uno::Reference< css::frame::XModel >& rxModel,
        const GraphicSettings& rGraphicSettings, sal_Int32& rGraphics );
};


// - GRAPHICCOLLECTOR -



#endif // INCLUDED_SDEXT_SOURCE_MINIMIZER_GRAPHICCOLLECTOR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
