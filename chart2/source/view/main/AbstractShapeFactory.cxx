/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "macros.hxx"
#include "PropertyMapper.hxx"
#include "CommonConverters.hxx"

#include "AbstractShapeFactory.hxx"
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/CircleKind.hpp>
#include <com/sun/star/drawing/DoubleSequence.hpp>
#include <com/sun/star/drawing/FlagSequence.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/drawing/NormalsKind.hpp>
#include <com/sun/star/drawing/PointSequence.hpp>
#include <com/sun/star/drawing/PolygonKind.hpp>
#include <com/sun/star/drawing/PolyPolygonBezierCoords.hpp>
#include <com/sun/star/drawing/ProjectionMode.hpp>
#include <com/sun/star/drawing/ShadeMode.hpp>
#include <com/sun/star/drawing/TextFitToSizeType.hpp>
#include <com/sun/star/drawing/TextureProjectionMode.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/uno/Any.hxx>

#include <editeng/unoprnms.hxx>
#include <rtl/math.hxx>
#include <svx/svdocirc.hxx>
#include <svx/svdopath.hxx>
#include <vcl/svapp.hxx>

#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/matrix/b3dhommatrix.hxx>

#include <osl/module.hxx>

#include "OpenglShapeFactory.hxx"
#include "ShapeFactory.hxx"

#include <config_features.h>

using namespace com::sun::star;

namespace chart {

namespace {

typedef opengl::OpenglShapeFactory* (*getOpenglShapeFactory_)(void);

#if HAVE_FEATURE_UI

#ifndef DISABLE_DYNLOADING

void SAL_CALL thisModule() {}

osl::Module* getOpenGLModule()
{
    static osl::Module aModule;
    if (aModule.is())
        // Already loaded.
        return &aModule;

    OUString aLibName(SVLIBRARY("chartopengl"));
    bool bLoaded = aModule.loadRelative(&thisModule, aLibName);
    if (!bLoaded)
        bLoaded = aModule.load(aLibName);

    return bLoaded ? &aModule : nullptr;
}

#endif

#endif

}

#ifdef DISABLE_DYNLOADING
extern "C" opengl::OpenglShapeFactory* getOpenglShapeFactory();
#endif

AbstractShapeFactory* AbstractShapeFactory::getOrCreateShapeFactory(const uno::Reference< lang::XMultiServiceFactory>& xFactory)
{
    static AbstractShapeFactory* pShapeFactory = nullptr;

    if (pShapeFactory)
        return pShapeFactory;

#if HAVE_FEATURE_UI
    if(getenv("CHART_DUMMY_FACTORY") && !Application::IsHeadlessModeEnabled())
    {
#ifndef DISABLE_DYNLOADING
        osl::Module* pModule = getOpenGLModule();
        if(pModule)
        {
            oslGenericFunction fn = pModule->getFunctionSymbol("getOpenglShapeFactory");
            if(fn)
            {

                pShapeFactory = reinterpret_cast<getOpenglShapeFactory_>(fn)();
                pShapeFactory->m_xShapeFactory = xFactory;
            }
        }
#elif defined(IOS) || defined(ANDROID) // Library_chartopengl is not portable enough yet
        pShapeFactory = NULL;
#else
        pShapeFactory = getOpenglShapeFactory();
        pShapeFactory->m_xShapeFactory = xFactory;
#endif
    }
#endif

    if (!pShapeFactory)
        pShapeFactory = new ShapeFactory(xFactory);

    return pShapeFactory;
}

uno::Reference< drawing::XShapes > AbstractShapeFactory::getChartRootShape(
    const uno::Reference< drawing::XDrawPage>& xDrawPage )
{
    uno::Reference< drawing::XShapes > xRet;
    uno::Reference< drawing::XShapes > xShapes( xDrawPage, uno::UNO_QUERY );
    if( xShapes.is() )
    {
        sal_Int32 nCount = xShapes->getCount();
        uno::Reference< drawing::XShape > xShape;
        for( sal_Int32 nN = nCount; nN--; )
        {
            if( xShapes->getByIndex( nN ) >>= xShape )
            {
                if( AbstractShapeFactory::getShapeName( xShape ) == "com.sun.star.chart2.shapes" )
                {
                    xRet.set( xShape, uno::UNO_QUERY );
                    break;
                }
            }
        }
    }
    return xRet;
}

void AbstractShapeFactory::makeShapeInvisible( const uno::Reference< drawing::XShape >& xShape )
{
    uno::Reference< beans::XPropertySet > xShapeProp( xShape, uno::UNO_QUERY );
    OSL_ENSURE(xShapeProp.is(), "created shape offers no XPropertySet");
    if( xShapeProp.is())
    {
        try
        {
            xShapeProp->setPropertyValue( "LineStyle", uno::Any( drawing::LineStyle_NONE ));
            xShapeProp->setPropertyValue( "FillStyle", uno::Any( drawing::FillStyle_NONE ));
        }
        catch( const uno::Exception& e )
        {
            ASSERT_EXCEPTION( e );
        }
    }
}

// set a name/CID at a shape (is used for selection handling)

void AbstractShapeFactory::setShapeName( const uno::Reference< drawing::XShape >& xShape
                               , const OUString& rName )
{
    if(!xShape.is())
        return;
    uno::Reference< beans::XPropertySet > xProp( xShape, uno::UNO_QUERY );
    OSL_ENSURE(xProp.is(), "shape offers no XPropertySet");
    if( xProp.is())
    {
        try
        {
            xProp->setPropertyValue( UNO_NAME_MISC_OBJ_NAME
                , uno::Any( rName ) );
        }
        catch( const uno::Exception& e )
        {
            ASSERT_EXCEPTION( e );
        }
    }
}

OUString AbstractShapeFactory::getShapeName( const uno::Reference< drawing::XShape >& xShape )
{
    OUString aRet;

    uno::Reference< beans::XPropertySet > xProp( xShape, uno::UNO_QUERY );
    OSL_ENSURE(xProp.is(), "shape offers no XPropertySet");
    if( xProp.is())
    {
        try
        {
            xProp->getPropertyValue( UNO_NAME_MISC_OBJ_NAME ) >>= aRet;
        }
        catch( const uno::Exception& e )
        {
            ASSERT_EXCEPTION( e );
        }
    }

    return aRet;
}

uno::Any AbstractShapeFactory::makeTransformation( const awt::Point& rScreenPosition2D, double fRotationAnglePi )
{
    ::basegfx::B2DHomMatrix aM;
    //As autogrow is active the rectangle is automatically expanded to that side
    //to which the text is not adjusted.
    // aM.scale( 1, 1 ); Oops? A scale with this parameters is neutral, line commented out
    aM.rotate( fRotationAnglePi );
    aM.translate( rScreenPosition2D.X, rScreenPosition2D.Y );
    uno::Any aATransformation( B2DHomMatrixToHomogenMatrix3(aM) );
    return aATransformation;
}

OUString AbstractShapeFactory::getStackedString( const OUString& rString, bool bStacked )
{
    sal_Int32 nLen = rString.getLength();
    if(!bStacked || !nLen)
        return rString;

    OUStringBuffer aStackStr;

    //add a newline after each letter
    //as we do not no letters here add a newline after each char
    for( sal_Int32 nPosSrc=0; nPosSrc < nLen; nPosSrc++ )
    {
        if( nPosSrc )
            aStackStr.append( '\r' );
        aStackStr.append(rString[nPosSrc]);
    }
    return aStackStr.makeStringAndClear();
}

bool AbstractShapeFactory::hasPolygonAnyLines( drawing::PolyPolygonShape3D& rPoly)
{
    // #i67757# check all contained polygons, if at least one polygon contains 2 or more points, return true
    for( sal_Int32 nIdx = 0, nCount = rPoly.SequenceX.getLength(); nIdx < nCount; ++nIdx )
        if( rPoly.SequenceX[ nIdx ].getLength() > 1 )
            return true;
    return false;
}

bool AbstractShapeFactory::isPolygonEmptyOrSinglePoint( drawing::PolyPolygonShape3D& rPoly)
{
    // true, if empty polypolygon or one polygon with one point
    return (rPoly.SequenceX.getLength() == 0) ||
        ((rPoly.SequenceX.getLength() == 1) && (rPoly.SequenceX[0].getLength() <= 1));
}

void AbstractShapeFactory::closePolygon( drawing::PolyPolygonShape3D& rPoly)
{
    OSL_ENSURE( rPoly.SequenceX.getLength() <= 1, "AbstractShapeFactory::closePolygon - single polygon expected" );
    //add a last point == first point
    if(isPolygonEmptyOrSinglePoint(rPoly))
        return;
    drawing::Position3D aFirst(rPoly.SequenceX[0][0],rPoly.SequenceY[0][0],rPoly.SequenceZ[0][0]);
    AddPointToPoly( rPoly, aFirst );
}

awt::Size AbstractShapeFactory::calculateNewSizeRespectingAspectRatio(
         const awt::Size& rTargetSize
         , const awt::Size& rSourceSizeWithCorrectAspectRatio )
{
    awt::Size aNewSize;

    double fFactorWidth = double(rTargetSize.Width)/double(rSourceSizeWithCorrectAspectRatio.Width);
    double fFactorHeight = double(rTargetSize.Height)/double(rSourceSizeWithCorrectAspectRatio.Height);
    double fFactor = std::min(fFactorWidth,fFactorHeight);
    aNewSize.Width=static_cast<sal_Int32>(fFactor*rSourceSizeWithCorrectAspectRatio.Width);
    aNewSize.Height=static_cast<sal_Int32>(fFactor*rSourceSizeWithCorrectAspectRatio.Height);

    return aNewSize;
}

awt::Point AbstractShapeFactory::calculateTopLeftPositionToCenterObject(
           const awt::Point& rTargetAreaPosition
         , const awt::Size& rTargetAreaSize
         , const awt::Size& rObjectSize )
{
    awt::Point aNewPosition(rTargetAreaPosition);
    aNewPosition.X += static_cast<sal_Int32>(double(rTargetAreaSize.Width-rObjectSize.Width)/2.0);
    aNewPosition.Y += static_cast<sal_Int32>(double(rTargetAreaSize.Height-rObjectSize.Height)/2.0);
    return aNewPosition;
}

::basegfx::B2IRectangle AbstractShapeFactory::getRectangleOfShape(
        const uno::Reference< drawing::XShape >& xShape )
{
    ::basegfx::B2IRectangle aRet;

    if( xShape.is() )
    {
        awt::Point aPos = xShape->getPosition();
        awt::Size aSize = xShape->getSize();
        aRet = BaseGFXHelper::makeRectangle(aPos,aSize);
    }
    return aRet;

}

awt::Size AbstractShapeFactory::getSizeAfterRotation(
         const uno::Reference< drawing::XShape >& xShape, double fRotationAngleDegree )
{
    awt::Size aRet(0,0);
    if(xShape.is())
    {
        const awt::Size aSize( xShape->getSize() );

        if( fRotationAngleDegree == 0.0 )
            aRet = aSize;
        else
        {
            while(fRotationAngleDegree>=360.0)
                fRotationAngleDegree-=360.0;
            while(fRotationAngleDegree<0.0)
                fRotationAngleDegree+=360.0;
            if(fRotationAngleDegree>270.0)
                fRotationAngleDegree=360.0-fRotationAngleDegree;
            else if(fRotationAngleDegree>180.0)
                fRotationAngleDegree=fRotationAngleDegree-180.0;
            else if(fRotationAngleDegree>90.0)
                fRotationAngleDegree=180.0-fRotationAngleDegree;

            const double fAnglePi = fRotationAngleDegree*F_PI/180.0;

            aRet.Height = static_cast<sal_Int32>(
                aSize.Width*rtl::math::sin( fAnglePi )
                + aSize.Height*rtl::math::cos( fAnglePi ));
            aRet.Width = static_cast<sal_Int32>(
                aSize.Width*rtl::math::cos( fAnglePi )
                + aSize.Height*rtl::math::sin( fAnglePi ));
        }
    }
    return aRet;
}

void AbstractShapeFactory::removeSubShapes( const uno::Reference< drawing::XShapes >& xShapes )
{
    if( xShapes.is() )
    {
        sal_Int32 nSubCount = xShapes->getCount();
        uno::Reference< drawing::XShape > xShape;
        for( sal_Int32 nS = nSubCount; nS--; )
        {
            if( xShapes->getByIndex( nS ) >>= xShape )
                xShapes->remove( xShape );
        }
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
