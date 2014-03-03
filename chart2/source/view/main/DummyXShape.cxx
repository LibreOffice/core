/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "DummyXShape.hxx"

#include <stdio.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <stdlib.h>
#include <string.h>


#include "CommonConverters.hxx"

#include <rtl/ustring.hxx>

#include <vcl/window.hxx>
#include <vcl/virdev.hxx>
#include <vcl/svapp.hxx>
#include <tools/gen.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/implbase1.hxx>
#include <editeng/unoprnms.hxx>
#include <toolkit/helper/vclunohelper.hxx>

#include <algorithm>

#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/matrix/b3dhommatrix.hxx>

#include <com/sun/star/beans/Property.hpp>

#define ENABLE_DEBUG_PROPERTIES 0

using namespace com::sun::star;

using namespace std;

namespace chart {

namespace dummy {

class DummyPropertySetInfo : public cppu::WeakImplHelper1<
                                com::sun::star::beans::XPropertySetInfo >
{
public:
    DummyPropertySetInfo(const std::map<OUString, uno::Any>& rProps ):
        mrProperties(rProps) {}

    virtual sal_Bool SAL_CALL hasPropertyByName( const OUString& rName )
        throw(uno::RuntimeException, std::exception);

    virtual beans::Property SAL_CALL getPropertyByName( const OUString& rName )
        throw(uno::RuntimeException, beans::UnknownPropertyException, std::exception);

    virtual uno::Sequence< beans::Property > SAL_CALL getProperties()
        throw(uno::RuntimeException, std::exception);

private:
    const std::map<OUString, uno::Any>& mrProperties;
};

sal_Bool SAL_CALL DummyPropertySetInfo::hasPropertyByName( const OUString& rName )
    throw(uno::RuntimeException, std::exception)
{
    return mrProperties.find(rName) != mrProperties.end();
}

beans::Property SAL_CALL DummyPropertySetInfo::getPropertyByName( const OUString& rName )
    throw(uno::RuntimeException, beans::UnknownPropertyException, std::exception)
{
    beans::Property aRet;
    if(mrProperties.find(rName) == mrProperties.end())
        throw beans::UnknownPropertyException();

    std::map<OUString, uno::Any>::const_iterator itr = mrProperties.find(rName);
    aRet.Name = rName;
    aRet.Type = itr->second.getValueType();

    return aRet;
}

uno::Sequence< beans::Property > SAL_CALL DummyPropertySetInfo::getProperties()
    throw(uno::RuntimeException, std::exception)
{
    uno::Sequence< beans::Property > aRet(mrProperties.size());

    size_t i = 0;
    for(std::map<OUString, uno::Any>::const_iterator itr = mrProperties.begin(),
            itrEnd = mrProperties.end(); itr != itrEnd; ++itr, ++i)
    {
        beans::Property aProp;

        aProp.Name = itr->first;
        aProp.Type = itr->second.getValueType();
        aRet[i] = aProp;
    }
    return aRet;
}

namespace {

struct PrintProperties
{
#if ENABLE_DEBUG_PROPERTIES
    void operator()(const std::pair<OUString, uno::Any>& rProp)
    {
        SAL_WARN("chart2.opengl", "Property: " << rProp.first);
    }
#else
    void operator()(const std::pair<OUString, uno::Any>&)
    {
    }
#endif
};

void debugProperties(std::map<OUString, uno::Any>& rProperties)
{
    for_each(rProperties.begin(), rProperties.end(), PrintProperties());
}

}

DummyXShape::DummyXShape()
{
}

OUString SAL_CALL DummyXShape::getName()
    throw(uno::RuntimeException, std::exception)
{
    return maName;
}

void SAL_CALL DummyXShape::setName( const OUString& rName )
    throw(uno::RuntimeException, std::exception)
{
    maName = rName;
}

awt::Point SAL_CALL DummyXShape::getPosition()
    throw(uno::RuntimeException, std::exception)
{
    return maPosition;
}

void SAL_CALL DummyXShape::setPosition( const awt::Point& rPoint )
    throw(uno::RuntimeException, std::exception)
{
    maPosition = rPoint;
}

awt::Size SAL_CALL DummyXShape::getSize()
    throw(uno::RuntimeException, std::exception)
{
    return maSize;
}

void SAL_CALL DummyXShape::setSize( const awt::Size& rSize )
    throw(beans::PropertyVetoException, uno::RuntimeException, std::exception)
{
    maSize = rSize;
}

OUString SAL_CALL DummyXShape::getShapeType()
    throw(uno::RuntimeException, std::exception)
{
    return OUString("dummy shape");
}

uno::Reference< beans::XPropertySetInfo > SAL_CALL DummyXShape::getPropertySetInfo()
    throw(uno::RuntimeException, std::exception)
{
    return new DummyPropertySetInfo(maProperties);
}

void SAL_CALL DummyXShape::setPropertyValue( const OUString& rName, const uno::Any& rValue)
    throw(beans::UnknownPropertyException, beans::PropertyVetoException,
            lang::IllegalArgumentException, lang::WrappedTargetException,
            uno::RuntimeException, std::exception)
{
    SAL_WARN("chart2", "DummyXShape::setProperty: " << rName << " " << "Any");
    maProperties[rName] = rValue;
    if(rName == "Transformation")
    {
        SAL_WARN("chart2.opengl", "Transformation");
    }
}

uno::Any SAL_CALL DummyXShape::getPropertyValue( const OUString& rName )
    throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SAL_WARN("chart2.opengl", "DummyXShape::getPropertyValue: " << rName);
    std::map<OUString, uno::Any>::iterator itr = maProperties.find(rName);
    if(itr != maProperties.end())
        return itr->second;

    return uno::Any();
}

void SAL_CALL DummyXShape::addPropertyChangeListener( const OUString&, const uno::Reference< beans::XPropertyChangeListener >& )
    throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
}

void SAL_CALL DummyXShape::removePropertyChangeListener( const OUString&, const uno::Reference< beans::XPropertyChangeListener >& )
    throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
}

void SAL_CALL DummyXShape::addVetoableChangeListener( const OUString&, const uno::Reference< beans::XVetoableChangeListener >& )
    throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
}

void SAL_CALL DummyXShape::removeVetoableChangeListener( const OUString&, const uno::Reference< beans::XVetoableChangeListener >& )
    throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
}

void SAL_CALL DummyXShape::setPropertyValues( const uno::Sequence< OUString >& rNames,
        const uno::Sequence< uno::Any >& rValues)
throw (beans::PropertyVetoException, lang::IllegalArgumentException,
        lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    size_t n = std::min<size_t>(rNames.getLength(), rValues.getLength());
    for(size_t i = 0; i < n; ++i)
    {
        maProperties[rNames[i]] = rValues[i];
    }
}

uno::Sequence< uno::Any > SAL_CALL DummyXShape::getPropertyValues(
        const uno::Sequence< OUString >& rNames)
    throw (uno::RuntimeException, std::exception)
{
    uno::Sequence< uno::Any > aValues(rNames.getLength());
    for(sal_Int32 i = 0; i < rNames.getLength(); ++i)
    {
        OUString aName = rNames[i];

        std::map<OUString, uno::Any>::iterator itr = maProperties.find(aName);
        if(itr != maProperties.end())
            aValues[i] = itr->second;
    }
    return aValues;
}

void SAL_CALL DummyXShape::addPropertiesChangeListener( const uno::Sequence< OUString >& , const uno::Reference< beans::XPropertiesChangeListener >& ) throw (uno::RuntimeException, std::exception)
{
}

void SAL_CALL DummyXShape::removePropertiesChangeListener( const uno::Reference< beans::XPropertiesChangeListener >& ) throw (uno::RuntimeException, std::exception)
{
}

void SAL_CALL DummyXShape::firePropertiesChangeEvent( const uno::Sequence< OUString >& ,
        const uno::Reference< beans::XPropertiesChangeListener >& )
    throw (uno::RuntimeException, std::exception)
{
}

OUString SAL_CALL DummyXShape::getImplementationName()
    throw(uno::RuntimeException, std::exception)
{
    return OUString("DummyXShape");
}

namespace {

uno::Sequence< OUString > listSupportedServices()
{
    static uno::Sequence< OUString > aSupportedServices;
    if(aSupportedServices.getLength() == 0)
    {
        aSupportedServices.realloc(3);
        aSupportedServices[0] = "com.sun.star.drawing.Shape";
        aSupportedServices[1] = "com.sun.star.container.Named";
        aSupportedServices[2] = "com.sun.star.beans.PropertySet";
    }

    return aSupportedServices;
}

}

uno::Sequence< OUString > SAL_CALL DummyXShape::getSupportedServiceNames()
    throw(uno::RuntimeException, std::exception)
{
    return listSupportedServices();
}

sal_Bool SAL_CALL DummyXShape::supportsService( const OUString& rServiceName )
    throw(uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Reference< uno::XInterface > SAL_CALL DummyXShape::getParent()
    throw(uno::RuntimeException, std::exception)
{
    return mxParent;
}

void SAL_CALL DummyXShape::setParent( const uno::Reference< uno::XInterface >& xParent )
    throw(lang::NoSupportException, uno::RuntimeException, std::exception)
{
    mxParent = xParent;
}

void DummyXShape::render()
{
    SAL_WARN("chart2.opengl", "maybe a missing implementation in a subclass?");
}

namespace {

void setProperties( const uno::Reference< beans::XPropertySet > & xPropSet, const tPropertyNameMap& rPropertyNameMap,
        std::map<OUString, uno::Any>& rTargetMap)
{
    tNameSequence aNames;
    tAnySequence aValues;
    PropertyMapper::getMultiPropertyLists( aNames, aValues,
            xPropSet, rPropertyNameMap );

    sal_Int32 nSize = std::min(aNames.getLength(), aValues.getLength());
    for(sal_Int32 i = 0; i < nSize; ++i)
    {
        rTargetMap[aNames[i]] = aValues[i];
    }
}

void setProperties( const tNameSequence& rNames, const tAnySequence& rValues,
        std::map<OUString, uno::Any>& rTargetMap)
{
    sal_Int32 nSize = std::min(rNames.getLength(), rValues.getLength());
    for(sal_Int32 i = 0; i < nSize; ++i)
    {
        rTargetMap[rNames[i]] = rValues[i];
    }
}

}

DummyCube::DummyCube(const drawing::Position3D &rPos, const drawing::Direction3D& rSize,
        const uno::Reference< beans::XPropertySet > & xPropSet,
        const tPropertyNameMap& rPropertyNameMap )
{
    setPosition(Position3DToAWTPoint(rPos));
    setSize(Direction3DToAWTSize(rSize));
    setProperties(xPropSet, rPropertyNameMap, maProperties);
}

DummyCylinder::DummyCylinder(const drawing::Position3D& rPos, const drawing::Direction3D& rSize )
{
    setPosition(Position3DToAWTPoint(rPos));
    setSize(Direction3DToAWTSize(rSize));
}

DummyPyramid::DummyPyramid(const drawing::Position3D& rPos, const drawing::Direction3D& rSize,
        const uno::Reference< beans::XPropertySet > & xPropSet,
        const tPropertyNameMap& rPropertyNameMap)
{
    setPosition(Position3DToAWTPoint(rPos));
    setSize(Direction3DToAWTSize(rSize));
    setProperties(xPropSet, rPropertyNameMap, maProperties);
}

DummyCone::DummyCone(const drawing::Position3D& rPos, const drawing::Direction3D& rSize)
{
    setPosition(Position3DToAWTPoint(rPos));
    setSize(Direction3DToAWTSize(rSize));
}

DummyPieSegment2D::DummyPieSegment2D(double fUnitCircleStartAngleDegree, double fUnitCircleWidthAngleDegree,
        double fUnitCircleInnerRadius, double fUnitCircleOuterRadius,
        const drawing::Direction3D& rOffset, const drawing::HomogenMatrix& rUnitCircleToScene):
    mfUnitCircleStartAngleDegree(fUnitCircleStartAngleDegree),
    mfUnitCircleWidthAngleDegree(fUnitCircleWidthAngleDegree),
    mfUnitCircleInnerRadius(fUnitCircleInnerRadius),
    mfUnitCircleOuterRadius(fUnitCircleOuterRadius),
    maOffset(rOffset),
    maUnitCircleToScene(rUnitCircleToScene)
{
}

void DummyPieSegment2D::render()
{
    SAL_WARN("chart2.opengl", "render DummyPieSegment2D");
    DummyChart* pChart = getRootShape();

    while(mfUnitCircleWidthAngleDegree>360)
        mfUnitCircleWidthAngleDegree -= 360.0;
    while(mfUnitCircleWidthAngleDegree<0)
        mfUnitCircleWidthAngleDegree += 360.0;

    pChart->m_GLRender.GeneratePieSegment2D(mfUnitCircleInnerRadius, mfUnitCircleOuterRadius,
            mfUnitCircleStartAngleDegree, mfUnitCircleWidthAngleDegree);

    sal_uInt8 nAlpha = 255;
    std::map<OUString, uno::Any>::const_iterator itr = maProperties.find(UNO_NAME_FILL_TRANSPARENCE);
    if(itr != maProperties.end())
    {
        nAlpha = 255 - itr->second.get<sal_Int32>();
    }

    itr = maProperties.find(UNO_NAME_FILLCOLOR);
    if(itr != maProperties.end())
    {
        sal_Int32 nColor = itr->second.get<sal_Int32>();
        pChart->m_GLRender.SetColor(nColor, nAlpha);
    }

    float nSize = std::max<float>(maUnitCircleToScene.Line1.Column1, maUnitCircleToScene.Line2.Column2);
    pChart->m_GLRender.RenderPieSegment2DShape(nSize, maUnitCircleToScene.Line1.Column4 + maOffset.DirectionX,
            maUnitCircleToScene.Line2.Column4 + maOffset.DirectionY);

}

DummyPieSegment::DummyPieSegment(
        const drawing::Direction3D& rOffset, const drawing::HomogenMatrix& rUnitCircleToScene ):
    maOffset(rOffset),
    maUnitCircleToScene(rUnitCircleToScene)
{
}

DummyStripe::DummyStripe(const Stripe& rStripe, const uno::Reference< beans::XPropertySet > & xPropSet,
        const tPropertyNameMap& rPropertyNameMap ):
    maStripe(rStripe)
{
    setProperties(xPropSet, rPropertyNameMap, maProperties);
}

DummyArea3D::DummyArea3D(const drawing::PolyPolygonShape3D& rShape):
    maShapes(rShape)
{
}

DummyArea2D::DummyArea2D(const drawing::PointSequenceSequence& rShape):
    maShapes(rShape)
{
}

void DummyArea2D::render()
{
    SAL_WARN("chart2.opengl", "render DummyArea2D");
    DummyChart* pChart = getRootShape();
    sal_Int32 nPointssCount = maShapes.getLength();
    for(sal_Int32 i = 0; i < nPointssCount; i++)
    {
        const com::sun::star::uno::Sequence<com::sun::star::awt::Point>& points = maShapes[i];
        sal_Int32 nPointsCount = points.getLength();
        for(sal_Int32 j = 0; j < nPointsCount; j++)
        {
            const com::sun::star::awt::Point& p = points[j];
            pChart->m_GLRender.SetArea2DShapePoint((float)p.X, (float)p.Y, nPointsCount);
        }
    }

    std::map<OUString, uno::Any>::const_iterator itr = maProperties.find(UNO_NAME_FILLCOLOR);
    if(itr != maProperties.end())
    {
        sal_Int32 nColor = itr->second.get<sal_Int32>();
        pChart->m_GLRender.SetColor(nColor, 255);
    }

    pChart->m_GLRender.RenderArea2DShape();
}

DummySymbol2D::DummySymbol2D(const drawing::Position3D& rPos, const drawing::Direction3D& rSize,
        sal_Int32 nStandardSymbol, sal_Int32 nFillColor):
    mrPosition(rPos),
    mrSize(rSize),
    mnStandardSymbol(nStandardSymbol),
    mnFillColor(nFillColor)
{
    setPosition(Position3DToAWTPoint(rPos));
    setSize(Direction3DToAWTSize(rSize));
}

void DummySymbol2D::render()
{
    DummyChart* pChart = getRootShape();

    pChart->m_GLRender.SetColor(mnFillColor, 255);

    pChart->m_GLRender.RenderSymbol2DShape(maPosition.X, maPosition.Y, maSize.Width, maSize.Height, mnStandardSymbol);
}

DummyCircle::DummyCircle(const awt::Point& rPos, const awt::Size& rSize)
{
    setPosition(rPos);
    setSize(rSize);
}

void DummyCircle::render()
{
    SAL_WARN("chart2.opengl", "render DummyCircle");
    debugProperties(maProperties);
    DummyChart* pChart = getRootShape();
    std::map<OUString, uno::Any>::const_iterator itr = maProperties.find("FillColor");
    if(itr != maProperties.end())
    {
        sal_Int32 nColor = itr->second.get<sal_Int32>();
        pChart->m_GLRender.SetColor(nColor, 255);
    }
    else
        SAL_WARN("chart2.opengl", "missing color");

    itr = maProperties.find("FillTransparence");
    if(itr != maProperties.end())
    {
        sal_Int32 nTrans = itr->second.get<sal_Int32>();
        pChart->m_GLRender.SetTransparency(nTrans&0xFF);
    }

    pChart->m_GLRender.Bubble2DShapePoint(maPosition.X, maPosition.Y,
                                          maSize.Width, maSize.Height);
    pChart->m_GLRender.RenderBubble2FBO(GL_TRUE);
}

namespace {

void setProperties( const VLineProperties& rLineProperties, std::map<OUString, uno::Any>& rTargetProps )
{
    //Transparency
    if(rLineProperties.Transparence.hasValue())
        rTargetProps.insert(std::pair<OUString, uno::Any>(
                    UNO_NAME_LINETRANSPARENCE, rLineProperties.Transparence));

    //LineStyle
    if(rLineProperties.LineStyle.hasValue())
        rTargetProps.insert(std::pair<OUString, uno::Any>(
                    UNO_NAME_LINESTYLE, rLineProperties.LineStyle));

    //LineWidth
    if(rLineProperties.Width.hasValue())
        rTargetProps.insert(std::pair<OUString, uno::Any>(
                    UNO_NAME_LINEWIDTH, rLineProperties.Width));

    //LineColor
    if(rLineProperties.Color.hasValue())
        rTargetProps.insert(std::pair<OUString, uno::Any>(
                    UNO_NAME_LINECOLOR, rLineProperties.Color));

    //LineDashName
    if(rLineProperties.DashName.hasValue())
        rTargetProps.insert(std::pair<OUString, uno::Any>(
                    "LineDashName", rLineProperties.DashName));
}

}

DummyLine3D::DummyLine3D(const drawing::PolyPolygonShape3D& rPoints, const VLineProperties& rLineProperties):
    maPoints(rPoints)
{
    setProperties(rLineProperties, maProperties);
}

DummyLine2D::DummyLine2D(const drawing::PointSequenceSequence& rPoints, const VLineProperties* pLineProperties):
    maPoints(rPoints)
{
    if(pLineProperties)
        setProperties(*pLineProperties, maProperties);
}

DummyLine2D::DummyLine2D(const awt::Size& rSize, const awt::Point& rPosition)
{
    setPosition(rPosition);
    setSize(rSize);
}

void DummyLine2D::render()
{
    SAL_WARN("chart2.opengl", "rendering line 2D");
    debugProperties(maProperties);
    DummyChart* pChart = getRootShape();

    //add style and transparency
    std::map< OUString, uno::Any >::const_iterator itr = maProperties.find(UNO_NAME_LINESTYLE);
    if (itr != maProperties.end())
    {
        uno::Any cow = itr->second;
        drawing::LineStyle nStyle = cow.get<drawing::LineStyle>();
        if (drawing::LineStyle_NONE == nStyle)
        {
            // nothing to render
            return;
        }
    }

    sal_uInt8 nAlpha = 255;
    itr = maProperties.find("LineTransparence");
    if(itr != maProperties.end())
    {
        uno::Any al = itr->second;
        nAlpha = 255 - al.get<sal_Int32>();
    }

    itr = maProperties.find(UNO_NAME_LINECOLOR);
    if(itr != maProperties.end())
    {
        //set line color
        uno::Any co =  itr->second;
        sal_Int32 nColorValue = co.get<sal_Int32>();
        SAL_INFO("chart2.opengl", "line colorvalue = " << nColorValue);
        sal_uInt8 R = (nColorValue & 0x00FF0000) >> 16;
        sal_uInt8 G = (nColorValue & 0x0000FF00) >> 8;
        sal_uInt8 B = (nColorValue & 0x000000FF);
        pChart->m_GLRender.SetLine2DColor(R, G, B, nAlpha);

        SAL_INFO("chart2.opengl", "line colorvalue = " << nColorValue << ", R = " << (int)R << ", G = " << (int)G << ", B = " << (int)B);
    }
    else
        SAL_WARN("chart2.opengl", "no line color set");

    //set line width
    itr = maProperties.find(UNO_NAME_LINEWIDTH);
    if(itr != maProperties.end())
    {
        uno::Any cow = itr->second;
        sal_Int32 nWidth = cow.get<sal_Int32>();
        pChart->m_GLRender.SetLine2DWidth(nWidth);

        SAL_WARN("chart2.opengl", "width = " << nWidth);
    }
    else
        SAL_WARN("chart2.opengl", "no line width set");

    sal_Int32 pointsscount = maPoints.getLength();
    for(sal_Int32 i = 0; i < pointsscount; i++)
    {
        com::sun::star::uno::Sequence<com::sun::star::awt::Point>& points = maPoints[i];
        sal_Int32 pointscount = points.getLength();
        for(sal_Int32 j = 0; j < pointscount; j++)
        {
            com::sun::star::awt::Point& p = points[j];
            pChart->m_GLRender.SetLine2DShapePoint((float)p.X, (float)p.Y, pointscount);
        }

    }
    pChart->m_GLRender.RenderLine2FBO(GL_TRUE);

}

DummyRectangle::DummyRectangle()
{
}

DummyRectangle::DummyRectangle(const awt::Size& rSize)
{
    setSize(rSize);
}

DummyRectangle::DummyRectangle(const awt::Size& rSize, const awt::Point& rPoint, const tNameSequence& rNames,
        const tAnySequence& rValues)
{
    setSize(rSize);
    setPosition(rPoint);
    setProperties(rNames, rValues, maProperties);
}

void DummyRectangle::render()
{
    SAL_WARN("chart2.opengl", "render DummyRectangle");
    debugProperties(maProperties);
    DummyChart* pChart = getRootShape();
    std::map< OUString, uno::Any >::const_iterator itr = maProperties.find("Invisible");
    if(itr != maProperties.end())
    {
        return;
    }

    bool bFill = true;
    itr = maProperties.find("FillStyle");
    if(itr != maProperties.end())
    {
        drawing::FillStyle eStyle = itr->second.get<drawing::FillStyle>();
        if(eStyle == drawing::FillStyle_NONE)
        {
            bFill = false;
        }
    }

    itr = maProperties.find("FillColor");
    if(itr != maProperties.end())
    {
        uno::Any co =  itr->second;
        sal_Int32 nColorValue = co.get<sal_Int32>();

        itr = maProperties.find("FillTransparence");
        sal_uInt8 nAlpha = 255;
        if(itr != maProperties.end())
        {
            uno::Any al = itr->second;
            nAlpha = al.get<sal_Int32>();
        }
        pChart->m_GLRender.SetBackGroundColor(nColorValue, nColorValue, nAlpha);
    }

    bool bBorder = true;
    itr =  maProperties.find(UNO_NAME_LINESTYLE);
    if (itr != maProperties.end())
    {
        uno::Any cow = itr->second;
        drawing::LineStyle nStyle = cow.get<drawing::LineStyle>();
        if (drawing::LineStyle_NONE == nStyle)
        {
            bBorder = false;
        }
    }

    //TODO: moggi: correct handling of gradients
    itr =  maProperties.find("FillTransparenceGradientName");
    if (itr != maProperties.end())
    {
        uno::Any co = itr->second;
        rtl::OUString aGradientValue = co.get<rtl::OUString>();
        if (aGradientValue.endsWithAsciiL("1", 1))
        {
            pChart->m_GLRender.SetChartTransparencyGradient(1);
        }
    }
    pChart->m_GLRender.RectangleShapePoint(maPosition.X, maPosition.Y, maSize.Width, maSize.Height);
    pChart->m_GLRender.RenderRectangleShape(bBorder, bFill);
}

namespace {

struct FontAttribSetter
{
    FontAttribSetter(Font& rFont):
        mrFont(rFont) {}

    void operator()(const std::pair<OUString, uno::Any>& rProp)
    {
        const OUString& rPropName = rProp.first;
        if(rPropName == "CharFontName")
        {
            OUString aName = rProp.second.get<OUString>();
            mrFont.SetName(aName);
        }
        else if(rPropName == "CharColor")
        {
            sal_Int32 nColor = rProp.second.get<sal_Int32>();
            mrFont.SetFillColor(nColor);
        }
        else if(rPropName == "CharHeight")
        {
            float fHeight = rProp.second.get<float>();
            mrFont.SetSize(Size(0,(fHeight*127+36)/72)); //taken from the MCW implementation
        }
        else if(rPropName == "CharUnderline")
        {
            FontUnderline eUnderline = static_cast<FontUnderline>(rProp.second.get<sal_Int16>());
            mrFont.SetUnderline(eUnderline);
        }
        else if(rPropName == "CharWeight")
        {
            float fWeight = rProp.second.get<float>();
            FontWeight eFontWeight = VCLUnoHelper::ConvertFontWeight(fWeight);
            mrFont.SetWeight(eFontWeight);
        }
        else if(rPropName == "ChartWidth")
        {
            float fWidth = rProp.second.get<float>();
            FontWidth eFontWidth = VCLUnoHelper::ConvertFontWidth(fWidth);
            mrFont.SetWidth(eFontWidth);
        }
    }
private:
    Font& mrFont;
};

}

DummyText::DummyText(const OUString& rText, const tNameSequence& rNames,
        const tAnySequence& rValues, const uno::Any& rTrans, uno::Reference< drawing::XShapes > xTarget ):
    maText(rText),
    maTrans(rTrans)
{
    setProperties(rNames, rValues, maProperties);

    Font aFont;
    std::for_each(maProperties.begin(), maProperties.end(), FontAttribSetter(aFont));

    VirtualDevice aDevice(*Application::GetDefaultDevice(), 0, 0);
    aDevice.Erase();
    Rectangle aRect;
    aDevice.SetFont(aFont);
    aDevice.GetTextBoundRect(aRect, rText);
    int screenWidth = (aRect.BottomRight().X() + 3) & ~3;
    int screenHeight = (aRect.BottomRight().Y() + 3) & ~3;
    aDevice.SetOutputSizePixel(Size(screenWidth * 3, screenHeight));
    aDevice.SetBackground(Wallpaper(COL_TRANSPARENT));
    aDevice.DrawText(Point(0, 0), rText);
    int bmpWidth = (aRect.Right() - aRect.Left() + 3) & ~3;
    int bmpHeight = (aRect.Bottom() - aRect.Top() + 3) & ~3;
    maBitmap = BitmapEx(aDevice.GetBitmapEx(aRect.TopLeft(), Size(bmpWidth, bmpHeight)));

    if(rTrans.hasValue())
    {
        drawing::HomogenMatrix3 aTrans = rTrans.get<drawing::HomogenMatrix3>();
        setSize(awt::Size(20*bmpWidth, 20*bmpHeight));
        setPosition(awt::Point(aTrans.Line1.Column3, aTrans.Line2.Column3));
        aTrans.Line1.Column1 = 20 * bmpWidth;
        aTrans.Line2.Column2 = 20 * bmpHeight;
        setTransformatAsProperty(aTrans);
    }
    else
    {
        setSize(awt::Size(20*bmpWidth, 20*bmpHeight));
        uno::Reference< drawing::XShape > xTargetShape(xTarget, uno::UNO_QUERY);
        drawing::HomogenMatrix3 aTrans;
        aTrans.Line1.Column1 = 20 * bmpWidth;
        aTrans.Line2.Column2 = 20 * bmpHeight;
        aTrans.Line3.Column3 = 1;
        if(xTargetShape.is())
        {
            const awt::Point rPoint = xTargetShape->getPosition();
            setPosition(rPoint);
            aTrans.Line1.Column3 = rPoint.X;
            aTrans.Line2.Column3 = rPoint.Y;
        }
        setTransformatAsProperty(aTrans);
    }
}

void DummyText::render()
{
    SAL_WARN("chart2.opengl", "render DummyText");
    debugProperties(maProperties);

    DummyChart* pChart = getRootShape();

    drawing::HomogenMatrix3 aTransformation;
    std::map<OUString, uno::Any>::const_iterator itr =
        maProperties.find("Transformation");
    if(itr != maProperties.end())
    {
        SAL_WARN("chart2.opengl", "found a transformation");
        if(itr->second.hasValue())
        {
            aTransformation = itr->second.get<drawing::HomogenMatrix3>();
        }
    }
    else if(maTrans.hasValue())
    {
        aTransformation = maTrans.get<drawing::HomogenMatrix3>();
    }
    pChart->m_GLRender.CreateTextTexture(maBitmap, maPosition, maSize, 0,
            aTransformation);
    pChart->m_GLRender.RenderTextShape();
}

void SAL_CALL DummyText::setPropertyValue( const OUString& rName, const uno::Any& rValue)
    throw(beans::UnknownPropertyException, beans::PropertyVetoException,
            lang::IllegalArgumentException, lang::WrappedTargetException,
            uno::RuntimeException, std::exception)
{
    SAL_WARN("chart2.opengl", "property value set after image has been created");
    SAL_WARN("chart2.opengl", rName);
    DummyXShape::setPropertyValue(rName, rValue);
}

void SAL_CALL DummyText::setPosition(const awt::Point& rPosition )
    throw(uno::RuntimeException, std::exception)
{
    DummyXShape::setPosition(rPosition);
    if(maTrans.hasValue())
        return;

    std::map<OUString, uno::Any>::const_iterator itr =
        maProperties.find("Transformation");
    if(itr != maProperties.end())
    {
        if(itr->second.hasValue())
        {
            drawing::HomogenMatrix3 aTrans = itr->second.get<drawing::HomogenMatrix3>();
            aTrans.Line1.Column3 = rPosition.X;
            aTrans.Line2.Column3 = rPosition.Y;
            setTransformatAsProperty(aTrans);
        }
    }
}

void DummyText::setTransformatAsProperty(const drawing::HomogenMatrix3& rMatrix)
{
    uno::Any aNewTrans;
    aNewTrans <<= rMatrix;
    setPropertyValue("Transformation", aNewTrans);
}

DummyFormattedText::DummyFormattedText(uno::Sequence< uno::Reference<
        chart2::XFormattedString > >& rFormattedString):
    maFormattedString(rFormattedString)
{
}

DummyGroup3D::DummyGroup3D(const OUString& rName)
{
    setName(rName);
}

DummyGroup2D::DummyGroup2D(const OUString& rName)
{
    setName(rName);
}

awt::Point SAL_CALL DummyGroup2D::getPosition()
    throw(uno::RuntimeException, std::exception)
{
    long nTop = std::numeric_limits<long>::max();
    long nLeft = std::numeric_limits<long>::max();
    for(std::vector<DummyXShape*>::iterator itr = maShapes.begin(),
            itrEnd = maShapes.end(); itr != itrEnd; ++itr)
    {
        awt::Point aPoint = (*itr)->getPosition();
        if(aPoint.X >= 0 && aPoint.Y >= 0)
        {
            nLeft = std::min<long>(nLeft, aPoint.X);
            nTop = std::min<long>(nTop, aPoint.Y);
        }
    }

    return awt::Point(nLeft, nTop);
}

awt::Size SAL_CALL DummyGroup2D::getSize()
    throw(uno::RuntimeException, std::exception)
{
    long nTop = std::numeric_limits<long>::max();
    long nLeft = std::numeric_limits<long>::max();
    long nBottom = 0;
    long nRight = 0;
    for(std::vector<DummyXShape*>::iterator itr = maShapes.begin(),
            itrEnd = maShapes.end(); itr != itrEnd; ++itr)
    {
        awt::Point aPoint = (*itr)->getPosition();
        nLeft = std::min<long>(nLeft, aPoint.X);
        nTop = std::min<long>(nTop, aPoint.Y);
        awt::Size aSize = (*itr)->getSize();
        nRight = std::max<long>(nRight, aPoint.X + aSize.Width);
        nBottom = std::max<long>(nBottom, aPoint.Y + aSize.Height);
    }

    return awt::Size(nRight - nLeft, nBottom - nTop);
}

void SAL_CALL DummyGroup2D::setPosition( const awt::Point& rPos )
    throw(uno::RuntimeException, std::exception)
{
    for(std::vector<DummyXShape*>::const_iterator itr = maShapes.begin(),
            itrEnd = maShapes.end(); itr != itrEnd; ++itr)
    {
        awt::Point aPos = (*itr)->getPosition();
        awt::Point aNewPos( rPos.X + aPos.X, rPos.Y + aPos.Y);
        (*itr)->setPosition(aNewPos);
    }
}

void SAL_CALL DummyGroup2D::setSize( const awt::Size& )
    throw( beans::PropertyVetoException, uno::RuntimeException, std::exception )
{
    SAL_WARN("chart2.opengl", "set size on group shape");
}

DummyGraphic2D::DummyGraphic2D(const drawing::Position3D& rPos, const drawing::Direction3D& rSize,
        const uno::Reference< graphic::XGraphic > xGraphic ):
    mxGraphic(xGraphic)
{
    setPosition(Position3DToAWTPoint(rPos));
    setSize(Direction3DToAWTSize(rSize));
}

DummyChart* DummyXShape::getRootShape()
{
    assert(mxParent.is());
    DummyXShape* pParent = dynamic_cast<DummyXShape*>(mxParent.get());
    assert(pParent);
    return pParent->getRootShape();
}

DummyChart* DummyChart::getRootShape()
{
    return this;
}

#define QUERYINT( xint ) \
    if( rType == ::getCppuType((const uno::Reference< xint >*)0) ) \
        aAny <<= uno::Reference< xint >(this)

#define QUERY_INTERFACE( xint ) \
    if( rType == ::getCppuType((const uno::Reference< xint >*)0 ) ) \
        return uno::makeAny(uno::Reference<xint>(this));

uno::Any SAL_CALL DummyXShapes::queryInterface( const uno::Type& rType )
    throw(uno::RuntimeException, std::exception)
{
    QUERY_INTERFACE( drawing::XShapes );
    QUERY_INTERFACE( container::XIndexAccess );
    return DummyXShape::queryInterface(rType);
}

uno::Any SAL_CALL DummyXShapes::queryAggregation( const uno::Type & rType )
    throw(uno::RuntimeException, std::exception)
{
    uno::Any aAny;

    //QUERYINT( drawing::XShapeGroup );
    QUERYINT( drawing::XShapes );
    else
        return DummyXShape::queryAggregation( rType );

    return aAny;
}

void SAL_CALL DummyXShapes::acquire()
    throw()
{
    DummyXShape::acquire();
}

void DummyXShapes::release()
    throw()
{
    DummyXShape::release();
}

void SAL_CALL DummyXShapes::add( const uno::Reference< drawing::XShape>& xShape )
    throw(uno::RuntimeException, std::exception)
{
    DummyXShape* pChild = dynamic_cast<DummyXShape*>(xShape.get());
    assert(pChild);
    maUNOShapes.push_back(xShape);
    pChild->setParent(static_cast< ::cppu::OWeakObject* >( this ));
    maShapes.push_back(pChild);
}

void SAL_CALL DummyXShapes::remove( const uno::Reference< drawing::XShape>& xShape )
    throw(uno::RuntimeException, std::exception)
{
    std::vector< uno::Reference<drawing::XShape> >::iterator itr = std::find(maUNOShapes.begin(), maUNOShapes.end(), xShape);
    if(itr != maUNOShapes.end())
    {
        DummyXShape* pChild = dynamic_cast<DummyXShape*>((*itr).get());
        std::vector< DummyXShape* >::iterator itrShape = std::find(maShapes.begin(), maShapes.end(), pChild);
        if(itrShape != maShapes.end())
            maShapes.erase(itrShape);

        maUNOShapes.erase(itr);
    }
}

uno::Type SAL_CALL DummyXShapes::getElementType()
    throw(uno::RuntimeException, std::exception)
{
    return ::getCppuType(( const uno::Reference< drawing::XShape >*)0);
}

sal_Bool SAL_CALL SAL_CALL DummyXShapes::hasElements()
    throw(uno::RuntimeException, std::exception)
{
    return !maUNOShapes.empty();
}

sal_Int32 SAL_CALL DummyXShapes::getCount()
    throw(uno::RuntimeException, std::exception)
{
    return maUNOShapes.size();
}

uno::Any SAL_CALL DummyXShapes::getByIndex(sal_Int32 nIndex)
    throw(lang::IndexOutOfBoundsException, lang::WrappedTargetException,
            uno::RuntimeException, std::exception)
{
    uno::Any aShape;
    aShape <<= maUNOShapes[nIndex];
    return aShape;
}

void DummyXShapes::render()
{
    SAL_WARN("chart2.opengl", "render DummyShapes");
    for(std::vector<DummyXShape*>::iterator itr = maShapes.begin(),
            itrEnd = maShapes.end(); itr != itrEnd; ++itr)
    {
        (*itr)->render();
    }
}

#if defined( WNT )

bool DummyChart::initWindow()
{
    const SystemEnvData* sysData(mpWindow->GetSystemData());
    GLWin.hWnd = sysData->hWnd;
    SystemWindowData winData;
    winData.nSize = sizeof(winData);
    pWindow.reset(new SystemChildWindow(mpWindow.get(), 0, &winData, sal_False));


    if( pWindow )
    {
        pWindow->SetMouseTransparent( sal_True );
        pWindow->SetParentClipMode( PARENTCLIPMODE_NOCLIP );
        pWindow->EnableEraseBackground( sal_False );
        pWindow->SetControlForeground();
        pWindow->SetControlBackground();
        pWindow->EnablePaint(sal_False);
        GLWin.hWnd = sysData->hWnd;
    }

    return true;
}

#elif defined( MACOSX )

bool DummyChart::initWindow()
{
    return false;
}

#elif defined( UNX )

namespace {

// we need them before glew can initialize them
// glew needs an OpenGL context so we need to get the address manually
void initOpenGLFunctionPointers()
{
    glXChooseFBConfig = (GLXFBConfig*(*)(Display *dpy, int screen, const int *attrib_list, int *nelements))glXGetProcAddressARB((GLubyte*)"glXChooseFBConfig");
    glXGetVisualFromFBConfig = (XVisualInfo*(*)(Display *dpy, GLXFBConfig config))glXGetProcAddressARB((GLubyte*)"glXGetVisualFromFBConfig");    // try to find a visual for the current set of attributes
    glXGetFBConfigAttrib = (int(*)(Display *dpy, GLXFBConfig config, int attribute, int* value))glXGetProcAddressARB((GLubyte*)"glXGetFBConfigAttrib");

}

}

bool DummyChart::initWindow()
{
    const SystemEnvData* sysData(mpWindow->GetSystemData());

    GLWin.dpy = reinterpret_cast<Display*>(sysData->pDisplay);

    if( !glXQueryExtension( GLWin.dpy, NULL, NULL ) )
        return false;

    GLWin.win = sysData->aWindow;

    OSL_TRACE("parent window: %d", GLWin.win);

    XWindowAttributes xattr;
    XGetWindowAttributes( GLWin.dpy, GLWin.win, &xattr );

    GLWin.screen = XScreenNumberOfScreen( xattr.screen );

    static int visual_attribs[] =
    {
        GLX_RED_SIZE,           8,
        GLX_GREEN_SIZE,         8,
        GLX_BLUE_SIZE,          8,
        GLX_ALPHA_SIZE,         8,
        GLX_DEPTH_SIZE,         24,
        GLX_X_VISUAL_TYPE,      GLX_TRUE_COLOR,
        None
    };

    const SystemEnvData* pChildSysData = NULL;
    pWindow.reset();

    initOpenGLFunctionPointers();

    int fbCount = 0;
    GLXFBConfig* pFBC = glXChooseFBConfig( GLWin.dpy,
            GLWin.screen,
            visual_attribs, &fbCount );

    if(!pFBC)
    {
        SAL_WARN("chart2.opengl", "no suitable fb format found");
        return false;
    }

    int best_fbc = -1, best_num_samp = -1;
    for(int i = 0; i < fbCount; ++i)
    {
        XVisualInfo* pVi = glXGetVisualFromFBConfig( GLWin.dpy, pFBC[i] );
        if(pVi)
        {
            // pick the one with the most samples per pixel
            int nSampleBuf = 0;
            int nSamples = 0;
            glXGetFBConfigAttrib( GLWin.dpy, pFBC[i], GLX_SAMPLE_BUFFERS, &nSampleBuf );
            glXGetFBConfigAttrib( GLWin.dpy, pFBC[i], GLX_SAMPLES       , &nSamples  );

            if ( best_fbc < 0 || (nSampleBuf && ( nSamples > best_num_samp )) )
            {
                best_fbc = i;
                best_num_samp = nSamples;
            }
        }
        XFree( pVi );
    }

    XVisualInfo* vi = glXGetVisualFromFBConfig( GLWin.dpy, pFBC[best_fbc] );
    if( vi )
    {
        SystemWindowData winData;
        winData.nSize = sizeof(winData);
        OSL_TRACE("using VisualID %08X", vi->visualid);
        winData.pVisual = (void*)(vi->visual);
        pWindow.reset(new SystemChildWindow(mpWindow.get(), 0, &winData, false));
        pChildSysData = pWindow->GetSystemData();
    }

    if (!pWindow || !pChildSysData)
        return false;

    pWindow->SetMouseTransparent( true );
    pWindow->SetParentClipMode( PARENTCLIPMODE_NOCLIP );
    pWindow->EnableEraseBackground( false );
    pWindow->SetControlForeground();
    pWindow->SetControlBackground();

    GLWin.dpy = reinterpret_cast<Display*>(pChildSysData->pDisplay);
    GLWin.win = pChildSysData->aWindow;
    GLWin.vi = vi;
    GLWin.GLXExtensions = glXQueryExtensionsString( GLWin.dpy, GLWin.screen );
    OSL_TRACE("available GLX extensions: %s", GLWin.GLXExtensions);

    return true;
}

namespace {

static bool errorTriggered;
int oglErrorHandler( Display* /*dpy*/, XErrorEvent* /*evnt*/ )
{
    errorTriggered = true;

    return 0;
}

}

#endif

#ifdef DBG_UTIL

namespace {

const char* getSeverityString(GLenum severity)
{
    switch(severity)
    {
        case GL_DEBUG_SEVERITY_LOW:
            return "low";
        case GL_DEBUG_SEVERITY_MEDIUM:
            return "medium";
        case GL_DEBUG_SEVERITY_HIGH:
            return "high";
        default:
            ;
    }

    return "unknown";
}

const char* getSourceString(GLenum source)
{
    switch(source)
    {
        case GL_DEBUG_SOURCE_API:
            return "API";
        case GL_DEBUG_SOURCE_SHADER_COMPILER:
            return "shader compiler";
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
            return "window system";
        case GL_DEBUG_SOURCE_THIRD_PARTY:
            return "third party";
        case GL_DEBUG_SOURCE_APPLICATION:
            return "Libreoffice";
        case GL_DEBUG_SOURCE_OTHER:
            return "unknown";
        default:
            ;
    }

    return "unknown";
}

const char* getTypeString(GLenum type)
{
    switch(type)
    {
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
            return "deprecated behavior";
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
            return "undefined behavior";
        case GL_DEBUG_TYPE_PERFORMANCE:
            return "performance";
        case GL_DEBUG_TYPE_PORTABILITY:
            return "portability";
        case GL_DEBUG_TYPE_MARKER:
            return "marker";
        case GL_DEBUG_TYPE_PUSH_GROUP:
            return "push group";
        case GL_DEBUG_TYPE_POP_GROUP:
            return "pop group";
        case GL_DEBUG_TYPE_OTHER:
            return "other";
        default:
            ;
    }

    return "unkown";
}

extern "C" void
#if defined _WIN32
APIENTRY
#endif
debug_callback(GLenum source, GLenum type, GLuint id,
        GLenum severity, GLsizei , const GLchar* message, GLvoid* )
{
    SAL_WARN("chart2.opengl", "OpenGL debug message: source: " << getSourceString(source) << ", type: "
            << getTypeString(type) << ", id: " << id << ", severity: " << getSeverityString(severity) << " with message: " << message);
}

}

#endif

bool DummyChart::initOpengl()
{
    SAL_WARN("chart2.opengl", "DummyChart::initOpengl----start");
    initWindow();
    mpWindow->setPosSizePixel(0,0,0,0);
    GLWin.Width = 0;
    GLWin.Height = 0;

#if defined( WNT )
    GLWin.hDC = GetDC(GLWin.hWnd);
#elif defined( MACOSX )

#elif defined( UNX )
    GLWin.ctx = glXCreateContext(GLWin.dpy,
                                 GLWin.vi,
                                 0,
                                 GL_TRUE);
    if( GLWin.ctx == NULL )
    {
        OSL_TRACE("unable to create GLX context");
        return false;
    }
#endif

#if defined( WNT )
    PIXELFORMATDESCRIPTOR PixelFormatFront =                    // PixelFormat Tells Windows How We Want Things To Be
    {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,                              // Version Number
        PFD_DRAW_TO_WINDOW |
        PFD_SUPPORT_OPENGL |
        PFD_DOUBLEBUFFER,
        PFD_TYPE_RGBA,                  // Request An RGBA Format
        (BYTE)32,                       // Select Our Color Depth
        0, 0, 0, 0, 0, 0,               // Color Bits Ignored
        0,                              // No Alpha Buffer
        0,                              // Shift Bit Ignored
        0,                              // No Accumulation Buffer
        0, 0, 0, 0,                     // Accumulation Bits Ignored
        64,                             // 32 bit Z-BUFFER
        0,                              // 0 bit stencil buffer
        0,                              // No Auxiliary Buffer
        0,                              // now ignored
        0,                              // Reserved
        0, 0, 0                         // Layer Masks Ignored
    };

    //  we must check whether can set the MSAA
    int WindowPix;
    m_GLRender.InitMultisample(PixelFormatFront);
    if (m_GLRender.GetMSAASupport())
    {
        WindowPix = m_GLRender.GetMSAAFormat();
    }
    else
    {
        WindowPix = ChoosePixelFormat(GLWin.hDC,&PixelFormatFront);
    }
    SetPixelFormat(GLWin.hDC,WindowPix,&PixelFormatFront);
    GLWin.hRC  = wglCreateContext(GLWin.hDC);
    wglMakeCurrent(GLWin.hDC,GLWin.hRC);

#elif defined( MACOSX )

#elif defined( UNX )
    if( !glXMakeCurrent( GLWin.dpy, GLWin.win, GLWin.ctx ) )
    {
        OSL_TRACE("unable to select current GLX context");
        return false;
    }

    int glxMinor, glxMajor;
    double nGLXVersion = 0;
    if( glXQueryVersion( GLWin.dpy, &glxMajor, &glxMinor ) )
      nGLXVersion = glxMajor + 0.1*glxMinor;
    OSL_TRACE("available GLX version: %f", nGLXVersion);

    GLWin.GLExtensions = glGetString( GL_EXTENSIONS );
    OSL_TRACE("available GL  extensions: %s", GLWin.GLExtensions);

    if( GLWin.HasGLXExtension("GLX_SGI_swap_control" ) )
    {
        // enable vsync
        typedef GLint (*glXSwapIntervalProc)(GLint);
        glXSwapIntervalProc glXSwapInterval = (glXSwapIntervalProc) glXGetProcAddress( (const GLubyte*) "glXSwapIntervalSGI" );
        if( glXSwapInterval ) {
        int (*oldHandler)(Display* /*dpy*/, XErrorEvent* /*evnt*/);

        // replace error handler temporarily
        oldHandler = XSetErrorHandler( oglErrorHandler );

        errorTriggered = false;

        glXSwapInterval( 1 );

        // sync so that we possibly get an XError
        glXWaitGL();
        XSync(GLWin.dpy, false);

        if( errorTriggered )
            OSL_TRACE("error when trying to set swap interval, NVIDIA or Mesa bug?");
        else
            OSL_TRACE("set swap interval to 1 (enable vsync)");

        // restore the error handler
        XSetErrorHandler( oldHandler );
        }
    }

#endif

    m_GLRender.InitOpenGL(GLWin);

#ifdef DBG_UTIL
    // only enable debug output in dbgutil build
    if( GLEW_ARB_debug_output )
    {
        glEnable(GL_DEBUG_OUTPUT);
        glDebugMessageCallback(&debug_callback, NULL);
    }

#endif

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);

#if defined( WNT )
    SwapBuffers(GLWin.hDC);
    glFlush();
#elif defined( MACOSX )

#elif defined( UNX )
    glXSwapBuffers(GLWin.dpy, GLWin.win);
#endif
    glEnable(GL_LIGHTING);
    GLfloat light_direction[] = { 0.0 , 0.0 , 1.0 };
    GLfloat materialDiffuse[] = { 1.0 , 1.0 , 1.0 , 1.0};
    glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, light_direction);
    glMaterialfv(GL_FRONT,GL_DIFFUSE,materialDiffuse);
    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);
    SAL_WARN("chart2.opengl", "DummyChart::initOpengl----end");
//    mpWindow->Show(1, 1);
    return true;
}


DummyChart::DummyChart(uno::Reference< drawing::XShape > xTarget):
    mpWindow(new Window(0, WB_NOBORDER|WB_NODIALOGCONTROL)),
    m_GLRender(xTarget)
{
    SAL_WARN("chart2.opengl", "DummyXShape::DummyChart()-----test: ");
    setName("com.sun.star.chart2.shapes");
    createGLContext();
}

void DummyChart::createGLContext()
{
    initOpengl();
}

void SAL_CALL DummyChart::setPosition( const awt::Point& aPosition )
    throw( uno::RuntimeException, std::exception )
{
    DummyXShape::setPosition(aPosition);
}

DummyChart::~DummyChart()
{
}

void SAL_CALL DummyChart::setSize( const awt::Size& aSize )
    throw( beans::PropertyVetoException, uno::RuntimeException, std::exception )
{
    SAL_INFO("chart2.opengl", "DummyChart::setSize()---aSize.Width = " << aSize.Width << ", aSize.Height = " << aSize.Height);
    int width = aSize.Width / OPENGL_SCALE_VALUE;
    int height = aSize.Height / OPENGL_SCALE_VALUE;
    mpWindow->SetSizePixel(Size(width, height));
    pWindow->SetSizePixel(Size(width, height));
    DummyXShape::setSize(awt::Size(0,0));
    m_GLRender.SetSize(width, height);
    SAL_INFO("chart2.opengl", "DummyChart::GLRender.Width = " << width << ", GLRender.Height = " << height);
}

void DummyChart::render()
{
    SAL_WARN("chart2.opengl", "render chart");
    m_GLRender.prepareToRender();
#if 0
    m_GLRender.renderDebug();
#else
    DummyXShapes::render();
#endif
    m_GLRender.renderToBitmap();
}

void DummyChart::clear()
{
    maUNOShapes.clear();
    maShapes.clear();
}

}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
