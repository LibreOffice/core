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
#include <vcl/cvtgrf.hxx>
#include <tools/urlobj.hxx>
#include <tools/stream.hxx>
#include <svtools/grfmgr.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/streamwrap.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <cppcanvas/basegfxfactory.hxx>
#include <cppcanvas/polypolygon.hxx>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/drawing/ColorMode.hpp>
#include <com/sun/star/text/GraphicCrop.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/drawing/PointSequenceSequence.hpp>
#include <com/sun/star/drawing/PointSequence.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/drawing/XLayerSupplier.hpp>
#include <com/sun/star/drawing/XLayerManager.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>

#include "drawshapesubsetting.hxx"
#include "drawshape.hxx"
#include "backgroundshape.hxx"
#include "mediashape.hxx"
#include "appletshape.hxx"
#include "shapeimporter.hxx"
#include "slideshowexceptions.hxx"
#include "gdimtftools.hxx"
#include "tools.hxx"
#include "slideshowcontext.hxx"

#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>

using namespace com::sun::star;
using namespace ::comphelper;

namespace slideshow {
namespace internal {

namespace {

bool importShapeGraphic(
    GraphicObject & o_rGraphic,
    uno::Reference<beans::XPropertySet> const& xPropSet )
{
    rtl::OUString aURL;
    if( !getPropertyValue( aURL, xPropSet, OUSTR("GraphicURL")) ||
        aURL.getLength() == 0 )
    {
        // no or empty property - cannot import shape graphic
        return false;
    }

    rtl::OUString const aVndUrl(
        RTL_CONSTASCII_USTRINGPARAM( "vnd.sun.star.GraphicObject:" ) );
    sal_Int32 nIndex( aURL.indexOf( aVndUrl ) );

    if(nIndex != -1)
    {
        // skip past the end of the "vnd..." prefix
        nIndex += aVndUrl.getLength();

        if(nIndex >= aURL.getLength())
        {
            OSL_ENSURE( false, "ShapeImporter::importShape(): "
                        "embedded graphic has no graphic ID" );
            return false;
        }

        // unique ID string found in URL, extract
        // to separate string
        rtl::OUString const aUniqueId(
            aURL.copy( nIndex, aURL.getLength() - nIndex ) );

        // TODO(T2): Creating a GraphicObject is not
        // thread safe (internally calls VCL, and has
        // unguarded internal singleton mpGlobalMgr)

        // fetch already loaded graphic from graphic manager.
        ByteString const aOldString( static_cast<String>(aUniqueId),
                                     RTL_TEXTENCODING_UTF8 );
        o_rGraphic = GraphicObject( aOldString );


        if( GRAPHIC_DEFAULT == o_rGraphic.GetType()
            || GRAPHIC_NONE == o_rGraphic.GetType() )
        {
            // even the GrfMgr does not seem to know this graphic
            return false;
        }
    }
    else
    {
        // no special string found, graphic must be
        // external. Load via GraphicIm porter
        INetURLObject aTmp( aURL );
        boost::scoped_ptr<SvStream> pGraphicStream(
            utl::UcbStreamHelper::CreateStream(
                aTmp.GetMainURL( INetURLObject::NO_DECODE ),
                STREAM_READ ) );
        if( !pGraphicStream )
        {
            OSL_ENSURE( false, "ShapeImporter::importShape(): "
                        "cannot create input stream for graphic" );
            return false;
        }

        Graphic aTmpGraphic;
        if( GraphicConverter::Import(
                *pGraphicStream, aTmpGraphic ) != ERRCODE_NONE )
        {
            OSL_ENSURE( false, "ShapeImporter::importShape(): "
                        "Failed to import shape graphic from given URL" );
            return false;
        }

        o_rGraphic = GraphicObject( aTmpGraphic );
    }
    return true;
}

/** This shape implementation just acts as a dummy for the layermanager.
    Its sole role is for hit test detection of group shapes.
*/
class ShapeOfGroup : public Shape
{
public:
    ShapeOfGroup( ShapeSharedPtr const&                      pGroupShape,
                  uno::Reference<drawing::XShape> const&     xShape,
                  uno::Reference<beans::XPropertySet> const& xPropSet,
                  double                                     nPrio );

    // Shape:
    virtual uno::Reference<drawing::XShape> getXShape() const;
    virtual void addViewLayer( ViewLayerSharedPtr const& pNewLayer,
                               bool                      bRedrawLayer );
    virtual bool removeViewLayer( ViewLayerSharedPtr const& pNewLayer );
    virtual bool clearAllViewLayers();
    virtual bool update() const;
    virtual bool render() const;
    virtual bool isContentChanged() const;
    virtual basegfx::B2DRectangle getBounds() const;
    virtual basegfx::B2DRectangle getDomBounds() const;
    virtual basegfx::B2DRectangle getUpdateArea() const;
    virtual bool isVisible() const;
    virtual double getPriority() const;
    virtual bool isBackgroundDetached() const;

private:
    ShapeSharedPtr const                  mpGroupShape;
    uno::Reference<drawing::XShape> const mxShape;
    double const                          mnPrio;
    basegfx::B2DPoint                     maPosOffset;
    double                                mnWidth;
    double                                mnHeight;
};

ShapeOfGroup::ShapeOfGroup( ShapeSharedPtr const&                      pGroupShape,
                            uno::Reference<drawing::XShape> const&     xShape,
                            uno::Reference<beans::XPropertySet> const& xPropSet,
                            double                                     nPrio ) :
    mpGroupShape(pGroupShape),
    mxShape(xShape),
    mnPrio(nPrio)
{
    // read bound rect
    uno::Any const aTmpRect_( xPropSet->getPropertyValue( OUSTR("BoundRect") ));
    awt::Rectangle const aTmpRect( aTmpRect_.get<awt::Rectangle>() );
    basegfx::B2DRectangle const groupPosSize( pGroupShape->getBounds() );
    maPosOffset = basegfx::B2DPoint( aTmpRect.X - groupPosSize.getMinX(),
                                     aTmpRect.Y - groupPosSize.getMinY() );
    mnWidth = aTmpRect.Width;
    mnHeight = aTmpRect.Height;
}

uno::Reference<drawing::XShape> ShapeOfGroup::getXShape() const
{
    return mxShape;
}

void ShapeOfGroup::addViewLayer( ViewLayerSharedPtr const& /*pNewLayer*/,
                                 bool                      /*bRedrawLayer*/ )
{
}

bool ShapeOfGroup::removeViewLayer( ViewLayerSharedPtr const& /*pNewLayer*/ )
{
    return true;
}

bool ShapeOfGroup::clearAllViewLayers()
{
    return true;
}

bool ShapeOfGroup::update() const
{
    return true;
}

bool ShapeOfGroup::render() const
{
    return true;
}

bool ShapeOfGroup::isContentChanged() const
{
    return false;
}

basegfx::B2DRectangle ShapeOfGroup::getBounds() const
{
    basegfx::B2DRectangle const groupPosSize( mpGroupShape->getBounds() );
    double const posX = (groupPosSize.getMinX() + maPosOffset.getX());
    double const posY = (groupPosSize.getMinY() + maPosOffset.getY());
    return basegfx::B2DRectangle( posX, posY, posX + mnWidth, posY + mnHeight );
}

basegfx::B2DRectangle ShapeOfGroup::getDomBounds() const
{
    return getBounds();
}

basegfx::B2DRectangle ShapeOfGroup::getUpdateArea() const
{
    return getBounds();
}

bool ShapeOfGroup::isVisible() const
{
    return mpGroupShape->isVisible();
}

double ShapeOfGroup::getPriority() const
{
    return mnPrio;
}

bool ShapeOfGroup::isBackgroundDetached() const
{
    return false;
}

} // anon namespace

ShapeSharedPtr ShapeImporter::createShape(
    uno::Reference<drawing::XShape> const& xCurrShape,
    uno::Reference<beans::XPropertySet> const& xPropSet,
    rtl::OUString const& shapeType ) const
{
    if( shapeType.equalsAsciiL(
            RTL_CONSTASCII_STRINGPARAM("com.sun.star.drawing.MediaShape") ) ||
        shapeType.equalsAsciiL(
            RTL_CONSTASCII_STRINGPARAM("com.sun.star.presentation.MediaShape") ) )
    {
        // Media shape (video etc.). This is a special object
        return createMediaShape(xCurrShape,
                                mnAscendingPrio,
                                mrContext);
    }
    else if( shapeType.equalsAsciiL(
                 RTL_CONSTASCII_STRINGPARAM("com.sun.star.drawing.PluginShape") ))
    {
        // PropertyValues to copy from XShape to plugin
        static const char* aPropertyValues[] =
            {
                "PluginURL",
                "PluginMimeType",
                "PluginCommands"
            };

        // (Netscape)Plugin shape. This is a special object
        return createAppletShape( xCurrShape,
                                  mnAscendingPrio,
                                  ::rtl::OUString(
                                      RTL_CONSTASCII_USTRINGPARAM(
                                          "com.sun.star.comp.sfx2.PluginObject" )),
                                  aPropertyValues,
                                  sizeof(aPropertyValues)/sizeof(*aPropertyValues),
                                  mrContext );
    }
    else if( shapeType.equalsAsciiL(
                 RTL_CONSTASCII_STRINGPARAM("com.sun.star.drawing.AppletShape") ))
    {
        // PropertyValues to copy from XShape to applet
        static const char* aPropertyValues[] =
            {
                "AppletCodeBase",
                "AppletName",
                "AppletCode",
                "AppletCommands",
                "AppletIsScript"
            };

        // (Java)Applet shape. This is a special object
        return createAppletShape( xCurrShape,
                                  mnAscendingPrio,
                                  ::rtl::OUString(
                                      RTL_CONSTASCII_USTRINGPARAM(
                                          "com.sun.star.comp.sfx2.AppletObject" )),
                                  aPropertyValues,
                                  sizeof(aPropertyValues)/sizeof(*aPropertyValues),
                                  mrContext );
    }
    else if( shapeType.equalsAsciiL(
                 RTL_CONSTASCII_STRINGPARAM("com.sun.star.drawing.OLE2Shape") ) ||
             shapeType.equalsAsciiL(
                RTL_CONSTASCII_STRINGPARAM("com.sun.star.presentation.OLE2Shape") ) )
    {
        // #i46224# Mark OLE shapes as foreign content - scan them for
        // unsupported actions, and fallback to bitmap, if necessary
        return DrawShape::create( xCurrShape,
                                  mxPage,
                                  mnAscendingPrio,
                                  true,
                                  mrContext );
    }
    else if( shapeType.equalsAsciiL(
                 RTL_CONSTASCII_STRINGPARAM(
                     "com.sun.star.drawing.GraphicObjectShape") ) ||
             shapeType.equalsAsciiL(
                 RTL_CONSTASCII_STRINGPARAM(
                     "com.sun.star.presentation.GraphicObjectShape") ) )
    {
        GraphicObject aGraphicObject;

        // to get hold of GIF animations, inspect Graphic
        // objects more thoroughly (the plain-jane shape
        // metafile of course would only contain the first
        // animation frame)
        if( !importShapeGraphic( aGraphicObject, xPropSet ) )
            return ShapeSharedPtr(); // error loading graphic -
                                     // no placeholders in
                                     // slideshow

        if( !aGraphicObject.IsAnimated() )
        {
            // no animation - simply utilize plain draw shape import

            // import shape as bitmap - either its a bitmap
            // anyway, or its a metafile, which currently the
            // metafile renderer might not display correctly.
            return DrawShape::create( xCurrShape,
                                      mxPage,
                                      mnAscendingPrio,
                                      true,
                                      mrContext );
        }


        // now extract relevant shape attributes via API
        // ---------------------------------------------

        drawing::ColorMode eColorMode( drawing::ColorMode_STANDARD );
        sal_Int16 nLuminance(0);
        sal_Int16 nContrast(0);
        sal_Int16 nRed(0);
        sal_Int16 nGreen(0);
        sal_Int16 nBlue(0);
        double    nGamma(1.0);
        sal_Int16 nTransparency(0);
        sal_Int32 nRotation(0);

        getPropertyValue( eColorMode, xPropSet, OUSTR("GraphicColorMode") );
        getPropertyValue( nLuminance, xPropSet, OUSTR("AdjustLuminance") );
        getPropertyValue( nContrast, xPropSet, OUSTR("AdjustContrast") );
        getPropertyValue( nRed, xPropSet, OUSTR("AdjustRed") );
        getPropertyValue( nGreen, xPropSet, OUSTR("AdjustGreen") );
        getPropertyValue( nBlue, xPropSet, OUSTR("AdjustBlue") );
        getPropertyValue( nGamma, xPropSet, OUSTR("Gamma") );
        getPropertyValue( nTransparency, xPropSet, OUSTR("Transparency") );
        getPropertyValue( nRotation, xPropSet, OUSTR("RotateAngle") );

        GraphicAttr aGraphAttrs;
        aGraphAttrs.SetDrawMode( (GraphicDrawMode)eColorMode );
        aGraphAttrs.SetLuminance( nLuminance );
        aGraphAttrs.SetContrast( nContrast );
        aGraphAttrs.SetChannelR( nRed );
        aGraphAttrs.SetChannelG( nGreen );
        aGraphAttrs.SetChannelB( nBlue );
        aGraphAttrs.SetGamma( nGamma );
        aGraphAttrs.SetTransparency( static_cast<sal_uInt8>(nTransparency) );
        aGraphAttrs.SetRotation( static_cast<sal_uInt16>(nRotation*10) );

        text::GraphicCrop aGraphCrop;
        if( getPropertyValue( aGraphCrop, xPropSet, OUSTR("GraphicCrop") ))
        {
            aGraphAttrs.SetCrop( aGraphCrop.Left,
                                 aGraphCrop.Top,
                                 aGraphCrop.Right,
                                 aGraphCrop.Bottom );
        }

        // fetch readily transformed and color-modified
        // graphic
        // ---------------------------------------------

        Graphic aGraphic(
            aGraphicObject.GetTransformedGraphic(
                aGraphicObject.GetPrefSize(),
                aGraphicObject.GetPrefMapMode(),
                aGraphAttrs ) );

        return DrawShape::create( xCurrShape,
                                  mxPage,
                                  mnAscendingPrio,
                                  aGraphic,
                                  mrContext );
    }
    else
    {
        return DrawShape::create( xCurrShape,
                                  mxPage,
                                  mnAscendingPrio,
                                  false,
                                  mrContext );
    }
}

bool ShapeImporter::isSkip(
    uno::Reference<beans::XPropertySet> const& xPropSet,
    rtl::OUString const& shapeType,
    uno::Reference< drawing::XLayer> const& xLayer )
{
    // skip empty presentation objects:
    bool bEmpty = false;
    if( getPropertyValue( bEmpty,
                          xPropSet,
                          OUSTR("IsEmptyPresentationObject")) &&
        bEmpty )
    {
        return true;
    }

    //skip shapes which corresponds to annotations
    if(xLayer.is())
    {
        rtl::OUString layerName;
        uno::Reference<beans::XPropertySet> xPropLayerSet(
                                                          xLayer, uno::UNO_QUERY );
        const uno::Any& a(xPropLayerSet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Name"))) );
        bool const bRet = (a >>= layerName);
        if(bRet)
        {
            if( layerName.equals(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DrawnInSlideshow"))))
            {
                //Transform shapes into PolyPolygons
                importPolygons(xPropSet);

                return true;
            }
        }
    }

    // don't export presentation placeholders on masterpage
    // they can be non empty when user edits the default texts
    if(mbConvertingMasterPage)
    {
        if(shapeType.equalsAsciiL(
                RTL_CONSTASCII_STRINGPARAM("com.sun.star.presentation."
                                           "TitleTextShape") ) ||
            shapeType.equalsAsciiL(
                RTL_CONSTASCII_STRINGPARAM("com.sun.star.presentation."
                                           "OutlinerShape") ))
        {
            return true;
        }
    }
    return false;
}


void ShapeImporter::importPolygons(uno::Reference<beans::XPropertySet> const& xPropSet) {

    drawing::PointSequenceSequence aRetval;
    sal_Int32           nLineColor=0;
    double              fLineWidth;
    getPropertyValue( aRetval, xPropSet, OUSTR("PolyPolygon") );
    getPropertyValue( nLineColor, xPropSet, OUSTR("LineColor") );
    getPropertyValue( fLineWidth, xPropSet, OUSTR("LineWidth") );

    drawing::PointSequence* pOuterSequence = aRetval.getArray();
    awt::Point* pInnerSequence = pOuterSequence->getArray();

    ::basegfx::B2DPolygon aPoly;
    basegfx::B2DPoint aPoint;
    for( sal_Int32 nCurrPoly=0; nCurrPoly<pOuterSequence->getLength(); ++nCurrPoly, ++pInnerSequence )
    {
        aPoint.setX((*pInnerSequence).X);
        aPoint.setY((*pInnerSequence).Y);
        aPoly.append( aPoint );
    }
    UnoViewVector::const_iterator aIter=(mrContext.mrViewContainer).begin();
    UnoViewVector::const_iterator aEnd=(mrContext.mrViewContainer).end();
    while(aIter != aEnd)
    {
        ::cppcanvas::PolyPolygonSharedPtr pPolyPoly(
            ::cppcanvas::BaseGfxFactory::getInstance().createPolyPolygon( (*aIter)->getCanvas(),
                                                                          aPoly ) );
        if( pPolyPoly )
        {
                pPolyPoly->setRGBALineColor( unoColor2RGBColor( nLineColor ).getIntegerColor() );
                pPolyPoly->setStrokeWidth(fLineWidth);
                pPolyPoly->draw();
                maPolygons.push_back(pPolyPoly);
        }
        ++aIter;
    }
}

ShapeSharedPtr ShapeImporter::importBackgroundShape() // throw (ShapeLoadFailedException)
{
    if( maShapesStack.empty() )
        throw ShapeLoadFailedException();

    XShapesEntry& rTop = maShapesStack.top();
    ShapeSharedPtr pBgShape(
        createBackgroundShape(mxPage,
                              uno::Reference<drawing::XDrawPage>(
                                  rTop.mxShapes,
                                  uno::UNO_QUERY_THROW),
                              mrContext) );
    mnAscendingPrio += 1.0;

    return pBgShape;
}

ShapeSharedPtr ShapeImporter::importShape() // throw (ShapeLoadFailedException)
{
    ShapeSharedPtr pRet;
    bool bIsGroupShape = false;

    while( !maShapesStack.empty() && !pRet )
    {
        XShapesEntry& rTop = maShapesStack.top();
        if( rTop.mnPos < rTop.mnCount )
        {
            uno::Reference<drawing::XShape> const xCurrShape(
                rTop.mxShapes->getByIndex( rTop.mnPos ), uno::UNO_QUERY );
            ++rTop.mnPos;
            uno::Reference<beans::XPropertySet> xPropSet(
                xCurrShape, uno::UNO_QUERY );
            if( !xPropSet.is() )
            {
                // we definitely need the properties of
                // the shape here. This will also fail,
                // if getByIndex did not return a valid
                // shape
                throw ShapeLoadFailedException();
            }

            //Retrieve the layer for the current shape
            uno::Reference< drawing::XLayer > xDrawnInSlideshow;

            uno::Reference< drawing::XLayerSupplier > xLayerSupplier(mxPagesSupplier, uno::UNO_QUERY);
            if(xLayerSupplier.is())
            {
                uno::Reference< container::XNameAccess > xNameAccess = xLayerSupplier->getLayerManager();

                uno::Reference< drawing::XLayerManager > xLayerManager(xNameAccess, uno::UNO_QUERY);

                   xDrawnInSlideshow = xLayerManager->getLayerForShape(xCurrShape);
            }

            rtl::OUString const shapeType( xCurrShape->getShapeType());

            // is this shape presentation-invisible?
            if( !isSkip(xPropSet, shapeType, xDrawnInSlideshow) )
            {
                bIsGroupShape = shapeType.equalsAsciiL(
                    RTL_CONSTASCII_STRINGPARAM(
                        "com.sun.star.drawing.GroupShape") );

                if( rTop.mpGroupShape ) // in group particle mode?
                {
                    pRet.reset( new ShapeOfGroup(
                                    rTop.mpGroupShape /* container shape */,
                                    xCurrShape, xPropSet,
                                    mnAscendingPrio ) );
                }
                else
                {
                    pRet = createShape( xCurrShape, xPropSet, shapeType );
                }
                mnAscendingPrio += 1.0;
            }
        }
        if( rTop.mnPos >= rTop.mnCount )
        {
            // group or top-level shapes finished:
            maShapesStack.pop();
        }
        if( bIsGroupShape && pRet )
        {
            // push new group on the stack: group traversal
            maShapesStack.push( XShapesEntry( pRet ) );
        }
    }

    return pRet;
}

bool ShapeImporter::isImportDone() const
{
    return maShapesStack.empty();
}

PolyPolygonVector ShapeImporter::getPolygons()
{
    return maPolygons;
}

ShapeImporter::ShapeImporter( uno::Reference<drawing::XDrawPage> const&          xPage,
                              uno::Reference<drawing::XDrawPage> const&          xActualPage,
                              uno::Reference<drawing::XDrawPagesSupplier> const& xPagesSupplier,
                              const SlideShowContext&                            rContext,
                              sal_Int32                                          nOrdNumStart,
                              bool                                               bConvertingMasterPage ) :
    mxPage( xActualPage ),
    mxPagesSupplier( xPagesSupplier ),
    mrContext( rContext ),
    maPolygons(),
    maShapesStack(),
    mnAscendingPrio( nOrdNumStart ),
    mbConvertingMasterPage( bConvertingMasterPage )
{
    uno::Reference<drawing::XShapes> const xShapes(
        xPage, uno::UNO_QUERY_THROW );
    maShapesStack.push( XShapesEntry(xShapes) );
}

} // namespace internal
} // namespace presentation

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
