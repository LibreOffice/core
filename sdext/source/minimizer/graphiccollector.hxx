/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: graphiccollector.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: sj $ $Date: 2007-05-11 13:50:44 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef GRAPHICCOLLECTOR_HXX
#define GRAPHICCOLLECTOR_HXX

#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_DEVICEINFO_HPP_
#include <com/sun/star/awt/DeviceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_GRAPHICCROP_HPP_
#include <com/sun/star/text/GraphicCrop.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XSHAPE_HPP_
#include <com/sun/star/drawing/XShape.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_SIZE_HPP_
#include <com/sun/star/awt/Size.hpp>
#endif
#ifndef _COM_SUN_STAR_GRAPHIC_XGRAPHIC_HPP_
#include <com/sun/star/graphic/XGraphic.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
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
        com::sun::star::uno::Reference< com::sun::star::graphic::XGraphic > mxGraphic;          // the corresponding XGraphic of the Shape
        com::sun::star::awt::Size                       maLogicalSize;                          // the biggest logical size the graphic will be displayed
        sal_Bool                                        mbRemoveCropArea;                       //
        com::sun::star::text::GraphicCrop               maGraphicCropLogic;
        std::vector< GraphicUser >                      maUser;

        GraphicEntity( const com::sun::star::uno::Reference< com::sun::star::graphic::XGraphic >& xGraphic, const GraphicUser& rUser )
            : mxGraphic( xGraphic ), maLogicalSize( rUser.maLogicalSize ), mbRemoveCropArea( sal_False ), maGraphicCropLogic( 0, 0, 0, 0 ) { maUser.push_back( rUser ); };
    };

    static const com::sun::star::awt::DeviceInfo& GetDeviceInfo( const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& rxFact );
    static com::sun::star::awt::Size GetOriginalSize( const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& rxMSF,
                const com::sun::star::uno::Reference< com::sun::star::graphic::XGraphic >& rxGraphic );
    static void CollectGraphics( const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& rxMSF, const com::sun::star::uno::Reference< com::sun::star::frame::XModel >& rxModel,
        const GraphicSettings& rGraphicSettings, std::vector< GraphicEntity >& io_rGraphicList );

};

// --------------------
// - GRAPHICCOLLECTOR -
// --------------------


#endif // GRAPHICCOLLECTOR_HXX
