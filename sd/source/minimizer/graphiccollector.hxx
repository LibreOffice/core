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

#ifndef GRAPHICCOLLECTOR_HXX
#define GRAPHICCOLLECTOR_HXX

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
    sal_Bool    mbJPEGCompression;
    sal_Int32   mnJPEGQuality;
    sal_Bool    mbRemoveCropArea;
    sal_Int32   mnImageResolution;
    sal_Bool    mbEmbedLinkedGraphics;

    GraphicSettings( sal_Bool bJPEGCompression, sal_Int32 nJPEGQuality, sal_Bool bRemoveCropArea,
                        sal_Int32 nImageResolution, sal_Bool bEmbedLinkedGraphics )
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
        com::sun::star::uno::Reference< com::sun::star::drawing::XShape >       mxShape;            // if mbFillBitmap is false the xShape has
        com::sun::star::uno::Reference< com::sun::star::beans::XPropertySet >   mxPropertySet;      // to be used otherwise the PropertySet
        com::sun::star::uno::Reference< com::sun::star::beans::XPropertySet >   mxPagePropertySet;
        OUString                       maGraphicURL;
        OUString                       maGraphicStreamURL;
        com::sun::star::text::GraphicCrop   maGraphicCropLogic;
        com::sun::star::awt::Size           maLogicalSize;
        sal_Bool                            mbFillBitmap;

        GraphicUser() : mxShape(), maGraphicCropLogic( 0, 0, 0, 0 ), mbFillBitmap( sal_False ) {};
    };

    struct GraphicEntity
    {
        com::sun::star::awt::Size                       maLogicalSize;                          // the biggest logical size the graphic will be displayed
        sal_Bool                                        mbRemoveCropArea;                       //
        com::sun::star::text::GraphicCrop               maGraphicCropLogic;
        std::vector< GraphicUser >                      maUser;

        GraphicEntity( const GraphicUser& rUser )
            : maLogicalSize( rUser.maLogicalSize ), mbRemoveCropArea( sal_False ), maGraphicCropLogic( 0, 0, 0, 0 ) { maUser.push_back( rUser ); };
    };

    static const com::sun::star::awt::DeviceInfo& GetDeviceInfo( const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& rxFact );
    static com::sun::star::awt::Size GetOriginalSize( const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& rxMSF,
                const com::sun::star::uno::Reference< com::sun::star::graphic::XGraphic >& rxGraphic );

    // collecting graphic instances, the downside of this method is that every graphic is swapped in
    static void CollectGraphics( const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& rxMSF, const com::sun::star::uno::Reference< com::sun::star::frame::XModel >& rxModel,
        const GraphicSettings& rGraphicSettings, std::vector< GraphicEntity >& io_rGraphicList );
    // counting graphics without swapping in graphics
    static void CountGraphics( const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& rxMSF, const com::sun::star::uno::Reference< com::sun::star::frame::XModel >& rxModel,
        const GraphicSettings& rGraphicSettings, sal_Int32& rGraphics );
};

// --------------------
// - GRAPHICCOLLECTOR -
// --------------------


#endif // GRAPHICCOLLECTOR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
