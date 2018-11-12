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

#include <vcl/cvtgrf.hxx>
#include <vcl/GraphicObject.hxx>
#include <tools/urlobj.hxx>
#include <tools/stream.hxx>
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
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>

#include "drawshapesubsetting.hxx"
#include "drawshape.hxx"
#include "backgroundshape.hxx"
#include "mediashape.hxx"
#include "appletshape.hxx"
#include <shapeimporter.hxx>
#include <slideshowexceptions.hxx>
#include "gdimtftools.hxx"
#include <tools.hxx>
#include <slideshowcontext.hxx>

#include <memory>

using namespace com::sun::star;

namespace slideshow {
namespace internal {

namespace {

std::unique_ptr<GraphicObject> importShapeGraphic(uno::Reference<beans::XPropertySet> const& xPropSet)
{
    std::unique_ptr<GraphicObject> xRet;

    uno::Reference<graphic::XGraphic> xGraphic;
    if (!getPropertyValue(xGraphic, xPropSet, "Graphic") || !xGraphic.is())
    {
        // no or empty property - cannot import shape graphic
        return xRet;
    }

    Graphic aGraphic(xGraphic);
    xRet.reset(new GraphicObject(aGraphic));

    if (GraphicType::Default == xRet->GetType() || GraphicType::NONE == xRet->GetType())
    {
        xRet.reset();
    }
    return xRet;
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
    virtual uno::Reference<drawing::XShape> getXShape() const override;
    virtual void addViewLayer( ViewLayerSharedPtr const& pNewLayer,
                               bool                      bRedrawLayer ) override;
    virtual bool removeViewLayer( ViewLayerSharedPtr const& pNewLayer ) override;
    virtual void clearAllViewLayers() override;
    virtual bool update() const override;
    virtual bool render() const override;
    virtual bool isContentChanged() const override;
    virtual basegfx::B2DRectangle getBounds() const override;
    virtual basegfx::B2DRectangle getDomBounds() const override;
    virtual basegfx::B2DRectangle getUpdateArea() const override;
    virtual bool isVisible() const override;
    virtual double getPriority() const override;
    virtual bool isBackgroundDetached() const override;

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
    uno::Any const aTmpRect_( xPropSet->getPropertyValue( "BoundRect" ));
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

void ShapeOfGroup::clearAllViewLayers()
{
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
    double const posX = groupPosSize.getMinX() + maPosOffset.getX();
    double const posY = groupPosSize.getMinY() + maPosOffset.getY();
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
    OUString const& shapeType ) const
{
    if( shapeType == "com.sun.star.drawing.MediaShape" || shapeType == "com.sun.star.presentation.MediaShape" )
    {
        // Media shape (video etc.). This is a special object
        return createMediaShape(xCurrShape,
                                mnAscendingPrio,
                                mrContext);
    }
    else if( shapeType == "com.sun.star.drawing.AppletShape" )
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
                                  "com.sun.star.comp.sfx2.AppletObject",
                                  aPropertyValues,
                                  SAL_N_ELEMENTS(aPropertyValues),
                                  mrContext );
    }
    else if( shapeType == "com.sun.star.drawing.OLE2Shape" || shapeType == "com.sun.star.presentation.OLE2Shape" )
    {
        // #i46224# Mark OLE shapes as foreign content - scan them for
        // unsupported actions, and fallback to bitmap, if necessary
        return DrawShape::create( xCurrShape,
                                  mxPage,
                                  mnAscendingPrio,
                                  true,
                                  mrContext );
    }
    else if( shapeType == "com.sun.star.drawing.GraphicObjectShape" || shapeType == "com.sun.star.presentation.GraphicObjectShape" )
    {
        // to get hold of GIF animations, inspect Graphic
        // objects more thoroughly (the plain-jane shape
        // metafile of course would only contain the first
        // animation frame)
        std::unique_ptr<GraphicObject> xGraphicObject(importShapeGraphic(xPropSet));
        if (!xGraphicObject)
            return ShapeSharedPtr(); // error loading graphic -
                                     // no placeholders in
                                     // slideshow

        if (!xGraphicObject->IsAnimated())
        {
            // no animation - simply utilize plain draw shape import

            // import shape as bitmap - either it's a bitmap
            // anyway, or it's a metafile, which currently the
            // metafile renderer might not display correctly.
            return DrawShape::create( xCurrShape,
                                      mxPage,
                                      mnAscendingPrio,
                                      true,
                                      mrContext );
        }


        // now extract relevant shape attributes via API


        drawing::ColorMode eColorMode( drawing::ColorMode_STANDARD );
        sal_Int16 nLuminance(0);
        sal_Int16 nContrast(0);
        sal_Int16 nRed(0);
        sal_Int16 nGreen(0);
        sal_Int16 nBlue(0);
        double    nGamma(1.0);
        sal_Int16 nTransparency(0);
        sal_Int32 nRotation(0);

        getPropertyValue( eColorMode, xPropSet, "GraphicColorMode" );
        getPropertyValue( nLuminance, xPropSet, "AdjustLuminance" );
        getPropertyValue( nContrast, xPropSet, "AdjustContrast" );
        getPropertyValue( nRed, xPropSet, "AdjustRed" );
        getPropertyValue( nGreen, xPropSet, "AdjustGreen" );
        getPropertyValue( nBlue, xPropSet, "AdjustBlue" );
        getPropertyValue( nGamma, xPropSet, "Gamma" );
        getPropertyValue( nTransparency, xPropSet, "Transparency" );
        getPropertyValue( nRotation, xPropSet, "RotateAngle" );

        GraphicAttr aGraphAttrs;
        aGraphAttrs.SetDrawMode( static_cast<GraphicDrawMode>(eColorMode) );
        aGraphAttrs.SetLuminance( nLuminance );
        aGraphAttrs.SetContrast( nContrast );
        aGraphAttrs.SetChannelR( nRed );
        aGraphAttrs.SetChannelG( nGreen );
        aGraphAttrs.SetChannelB( nBlue );
        aGraphAttrs.SetGamma( nGamma );
        aGraphAttrs.SetTransparency( static_cast<sal_uInt8>(nTransparency) );
        aGraphAttrs.SetRotation( static_cast<sal_uInt16>(nRotation*10) );

        text::GraphicCrop aGraphCrop;
        if( getPropertyValue( aGraphCrop, xPropSet, "GraphicCrop" ))
        {
            aGraphAttrs.SetCrop( aGraphCrop.Left,
                                 aGraphCrop.Top,
                                 aGraphCrop.Right,
                                 aGraphCrop.Bottom );
        }

        // fetch readily transformed and color-modified
        // graphic


        Graphic aGraphic(
            xGraphicObject->GetTransformedGraphic(
                xGraphicObject->GetPrefSize(),
                xGraphicObject->GetPrefMapMode(),
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
    OUString const& shapeType,
    uno::Reference< drawing::XLayer> const& xLayer )
{
    // skip empty presentation objects:
    bool bEmpty = false;
    if( getPropertyValue( bEmpty,
                          xPropSet,
                          "IsEmptyPresentationObject") &&
        bEmpty )
    {
        return true;
    }

    //skip shapes which corresponds to annotations
    if(xLayer.is())
    {
        OUString layerName;
        uno::Reference<beans::XPropertySet> xPropLayerSet(
                                                          xLayer, uno::UNO_QUERY );
        const uno::Any& a(xPropLayerSet->getPropertyValue("Name") );
        bool const bRet = (a >>= layerName);
        if(bRet)
        {
            if( layerName == "DrawnInSlideshow" )
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
        if( shapeType == "com.sun.star.presentation.TitleTextShape" || shapeType == "com.sun.star.presentation.OutlinerShape" )
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
    getPropertyValue( aRetval, xPropSet, "PolyPolygon" );
    getPropertyValue( nLineColor, xPropSet, "LineColor" );
    getPropertyValue( fLineWidth, xPropSet, "LineWidth" );

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
    for( const auto& pView : mrContext.mrViewContainer )
    {
        ::cppcanvas::PolyPolygonSharedPtr pPolyPoly(
            ::cppcanvas::BaseGfxFactory::createPolyPolygon( pView->getCanvas(),
                                                            aPoly ) );
        if( pPolyPoly )
        {
                pPolyPoly->setRGBALineColor( unoColor2RGBColor( nLineColor ).getIntegerColor() );
                pPolyPoly->setStrokeWidth(fLineWidth);
                pPolyPoly->draw();
                maPolygons.push_back(pPolyPoly);
        }
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

            OUString const shapeType( xCurrShape->getShapeType());

            // is this shape presentation-invisible?
            if( !isSkip(xPropSet, shapeType, xDrawnInSlideshow) )
            {
                bIsGroupShape = shapeType == "com.sun.star.drawing.GroupShape";

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

const PolyPolygonVector& ShapeImporter::getPolygons()
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
