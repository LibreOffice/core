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
        rtl::OUString                       maGraphicURL;
        rtl::OUString                       maGraphicStreamURL;
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
