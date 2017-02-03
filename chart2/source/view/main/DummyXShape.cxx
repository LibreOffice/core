/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "DummyXShape.hxx"

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
#include <cppuhelper/implbase.hxx>
#include <editeng/unoprnms.hxx>
#include <toolkit/helper/vclunohelper.hxx>

#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/matrix/b3dhommatrix.hxx>

#include <com/sun/star/beans/Property.hpp>

#include <com/sun/star/awt/XBitmap.hpp>

#define ENABLE_DEBUG_PROPERTIES 0

using namespace com::sun::star;

using namespace std;

namespace chart {

namespace dummy {

#if 0

std::ostream& operator<<(std::ostream& rStrm, const awt::Point& rPoint)
{
    rStrm << rPoint.X << "," << rPoint.Y;
    return rStrm;
}

std::ostream& operator<<(std::ostream& rStrm, const awt::Size& rSize)
{
    rStrm << rSize.Width << "," << rSize.Height;
    return rStrm;
}

#endif

bool TextCache::hasEntry(const TextCacheKey& rKey)
{
    return maCache.find(rKey) != maCache.end();
}

BitmapEx& TextCache::getBitmap(const TextCacheKey& rKey)
{
    return maCache.find(rKey)->second;
}

void TextCache::insertBitmap(const TextCacheKey& rKey, const BitmapEx& rBitmap)
{
    maCache.insert(std::pair<TextCacheKey, BitmapEx>(rKey, rBitmap));
}

class DummyPropertySetInfo : public cppu::WeakImplHelper<
                                css::beans::XPropertySetInfo >
{
public:
    explicit DummyPropertySetInfo(const std::map<OUString, uno::Any>& rProps ):
        mrProperties(rProps) {}

    virtual sal_Bool SAL_CALL hasPropertyByName( const OUString& rName ) override;

    virtual beans::Property SAL_CALL getPropertyByName( const OUString& rName ) override;

    virtual uno::Sequence< beans::Property > SAL_CALL getProperties() override;

private:
    const std::map<OUString, uno::Any>& mrProperties;
};

sal_Bool SAL_CALL DummyPropertySetInfo::hasPropertyByName( const OUString& rName )
{
    return mrProperties.find(rName) != mrProperties.end();
}

beans::Property SAL_CALL DummyPropertySetInfo::getPropertyByName( const OUString& rName )
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
        SAL_INFO("chart2.opengl.properties", "Property: " << rProp.first);
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
{
    return maName;
}

void SAL_CALL DummyXShape::setName( const OUString& rName )
{
    maName = rName;
}

awt::Point SAL_CALL DummyXShape::getPosition()
{
    return maPosition;
}

void SAL_CALL DummyXShape::setPosition( const awt::Point& rPoint )
{
    maPosition = rPoint;
}

awt::Size SAL_CALL DummyXShape::getSize()
{
    return maSize;
}

void SAL_CALL DummyXShape::setSize( const awt::Size& rSize )
{
    maSize = rSize;
}

OUString SAL_CALL DummyXShape::getShapeType()
{
    return OUString("dummy shape");
}

uno::Reference< beans::XPropertySetInfo > SAL_CALL DummyXShape::getPropertySetInfo()
{
    return new DummyPropertySetInfo(maProperties);
}

void SAL_CALL DummyXShape::setPropertyValue( const OUString& rName, const uno::Any& rValue)
{
    SAL_INFO("chart2", "DummyXShape::setProperty: " << rName << " Any");
    maProperties[rName] = rValue;
    if(rName == "Transformation")
    {
        SAL_INFO("chart2.opengl", "Transformation");
    }
}

uno::Any SAL_CALL DummyXShape::getPropertyValue( const OUString& rName )
{
    SAL_INFO("chart2.opengl", "DummyXShape::getPropertyValue: " << rName);
    std::map<OUString, uno::Any>::iterator itr = maProperties.find(rName);
    if(itr != maProperties.end())
        return itr->second;

    return uno::Any();
}

void SAL_CALL DummyXShape::addPropertyChangeListener( const OUString&, const uno::Reference< beans::XPropertyChangeListener >& )
{
}

void SAL_CALL DummyXShape::removePropertyChangeListener( const OUString&, const uno::Reference< beans::XPropertyChangeListener >& )
{
}

void SAL_CALL DummyXShape::addVetoableChangeListener( const OUString&, const uno::Reference< beans::XVetoableChangeListener >& )
{
}

void SAL_CALL DummyXShape::removeVetoableChangeListener( const OUString&, const uno::Reference< beans::XVetoableChangeListener >& )
{
}

void SAL_CALL DummyXShape::setPropertyValues( const uno::Sequence< OUString >& rNames,
        const uno::Sequence< uno::Any >& rValues)
{
    size_t n = std::min<size_t>(rNames.getLength(), rValues.getLength());
    for(size_t i = 0; i < n; ++i)
    {
        maProperties[rNames[i]] = rValues[i];
    }
}

uno::Sequence< uno::Any > SAL_CALL DummyXShape::getPropertyValues(
        const uno::Sequence< OUString >& rNames)
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

void SAL_CALL DummyXShape::addPropertiesChangeListener( const uno::Sequence< OUString >& , const uno::Reference< beans::XPropertiesChangeListener >& )
{
}

void SAL_CALL DummyXShape::removePropertiesChangeListener( const uno::Reference< beans::XPropertiesChangeListener >& )
{
}

void SAL_CALL DummyXShape::firePropertiesChangeEvent( const uno::Sequence< OUString >& ,
        const uno::Reference< beans::XPropertiesChangeListener >& )
{
}

OUString SAL_CALL DummyXShape::getImplementationName()
{
    return OUString("DummyXShape");
}

namespace {

uno::Sequence< OUString > const & listSupportedServices()
{
    static const uno::Sequence< OUString > aSupportedServices{
        "com.sun.star.drawing.Shape",
        "com.sun.star.container.Named",
        "com.sun.star.beans.PropertySet"};
    return aSupportedServices;
}

}

uno::Sequence< OUString > SAL_CALL DummyXShape::getSupportedServiceNames()
{
    return listSupportedServices();
}

sal_Bool SAL_CALL DummyXShape::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

uno::Reference< uno::XInterface > SAL_CALL DummyXShape::getParent()
{
    return mxParent;
}

void SAL_CALL DummyXShape::setParent( const uno::Reference< uno::XInterface >& xParent )
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
    SAL_INFO("chart2.opengl", "render DummyPieSegment2D");
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

DummyStripe::DummyStripe(const uno::Reference< beans::XPropertySet > & xPropSet,
        const tPropertyNameMap& rPropertyNameMap )
{
    setProperties(xPropSet, rPropertyNameMap, maProperties);
}

DummyArea2D::DummyArea2D(const drawing::PointSequenceSequence& rShape):
    maShapes(rShape)
{
}

void DummyArea2D::render()
{
    SAL_INFO("chart2.opengl", "render DummyArea2D");
    DummyChart* pChart = getRootShape();
    sal_Int32 nPointssCount = maShapes.getLength();
    for(sal_Int32 i = 0; i < nPointssCount; i++)
    {
        const css::uno::Sequence<css::awt::Point>& points = maShapes[i];
        sal_Int32 nPointsCount = points.getLength();
        for(sal_Int32 j = 0; j < nPointsCount; j++)
        {
            const css::awt::Point& p = points[j];
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
    SAL_INFO("chart2.opengl", "render DummyCircle");
    debugProperties(maProperties);
    DummyChart* pChart = getRootShape();

    sal_uInt8 nAlpha = 255;
    std::map<OUString, uno::Any>::const_iterator itr = maProperties.find("FillTransparence");
    if(itr != maProperties.end())
    {
        sal_Int32 nTrans = itr->second.get<sal_Int32>()/100.0*255;
        nAlpha = 255 - static_cast<sal_uInt8>(nTrans & 0xFF);

        if(nAlpha == 0)
            return;
    }

    itr = maProperties.find("FillColor");
    if(itr != maProperties.end())
    {
        sal_Int32 nColor = itr->second.get<sal_Int32>();
        pChart->m_GLRender.SetColor(nColor, nAlpha);
    }
    else
        SAL_WARN("chart2.opengl", "missing color");

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

DummyLine3D::DummyLine3D(const VLineProperties& rLineProperties)
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
    SAL_INFO("chart2.opengl", "rendering line 2D");
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

        SAL_INFO("chart2.opengl", "width = " << nWidth);
    }
    else
        SAL_WARN("chart2.opengl", "no line width set");

    sal_Int32 pointsscount = maPoints.getLength();
    for(sal_Int32 i = 0; i < pointsscount; i++)
    {
        css::uno::Sequence<css::awt::Point>& points = maPoints[i];
        sal_Int32 pointscount = points.getLength();
        for(sal_Int32 j = 0; j < pointscount; j++)
        {
            css::awt::Point& p = points[j];
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
    SAL_INFO("chart2.opengl", "render DummyRectangle");
    debugProperties(maProperties);
    DummyChart* pChart = getRootShape();
    std::map< OUString, uno::Any >::const_iterator itr = maProperties.find("Invisible");
    if(itr != maProperties.end())
    {
        return;
    }

    bool bFill = true;
    drawing::FillStyle eStyle = drawing::FillStyle_NONE;
    itr = maProperties.find("FillStyle");
    if(itr != maProperties.end())
    {
        eStyle = itr->second.get<drawing::FillStyle>();
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
        //here FillStyle works for background color and gradients
        pChart->m_GLRender.SetBackGroundColor(nColorValue, nColorValue, eStyle);
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
        if (aGradientValue.endsWith("1"))
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
    explicit FontAttribSetter(vcl::Font& rFont):
        mrFont(rFont) {}

    void operator()(const std::pair<OUString, uno::Any>& rProp)
    {
        const OUString& rPropName = rProp.first;
        if(rPropName == "CharFontName")
        {
            OUString aName = rProp.second.get<OUString>();
            mrFont.SetFamilyName(aName);
        }
        else if(rPropName == "CharColor")
        {
            sal_Int32 nColor = rProp.second.get<sal_Int32>();
            mrFont.SetFillColor(nColor);
        }
        else if(rPropName == "CharHeight")
        {
            float fHeight = rProp.second.get<float>();
            mrFont.SetFontSize(Size(0,(fHeight*127+36)/72)); //taken from the MCW implementation
        }
        else if(rPropName == "CharUnderline")
        {
            FontLineStyle eUnderline = static_cast<FontLineStyle>(rProp.second.get<sal_Int16>());
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
            mrFont.SetAverageFontWidth(eFontWidth);
        }
    }
private:
    vcl::Font& mrFont;
};

}

DummyText::DummyText(const OUString& rText, const tNameSequence& rNames,
        const tAnySequence& rValues, const uno::Any& rTrans, uno::Reference< drawing::XShapes > const & xTarget, double nRotation ):
    maText(rText),
    maTrans(rTrans),
    mnRotation(nRotation)
{
    setProperties(rNames, rValues, maProperties);

    xTarget->add(this);
    DummyChart* pChart = getRootShape();
    TextCache& rCache = pChart->getTextCache();
    TextCache::TextCacheKey aKey;
    aKey.maText = maText;
    aKey.maProperties = maProperties;
    int bmpWidth;
    int bmpHeight;
    if(rCache.hasEntry(aKey))
    {
        maBitmap = rCache.getBitmap(aKey);
        bmpWidth = maBitmap.GetSizePixel().Width();
        bmpHeight = maBitmap.GetSizePixel().Height();
    }
    else
    {
        vcl::Font aFont;
        std::for_each(maProperties.begin(), maProperties.end(), FontAttribSetter(aFont));
        ScopedVclPtrInstance< VirtualDevice > pDevice(*Application::GetDefaultDevice(),
                                                      DeviceFormat::DEFAULT, DeviceFormat::DEFAULT);
        pDevice->Erase();
        Rectangle aRect;
        pDevice->SetFont(aFont);
        pDevice->GetTextBoundRect(aRect, rText);
        int screenWidth = (aRect.BottomRight().X());
        int screenHeight = (aRect.BottomRight().Y());
        pDevice->SetOutputSizePixel(Size(screenWidth * 3, screenHeight));
        pDevice->SetBackground(Wallpaper(COL_TRANSPARENT));
        pDevice->DrawText(Point(0, 0), rText);
        bmpWidth = aRect.Right() - aRect.Left();
        bmpHeight = aRect.Bottom() - aRect.Top();
        maBitmap = BitmapEx(pDevice->GetBitmapEx(aRect.TopLeft(), Size(bmpWidth, bmpHeight)));
        rCache.insertBitmap(aKey, maBitmap);
    }

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
    SAL_INFO("chart2.opengl", "render DummyText");
    debugProperties(maProperties);

    DummyChart* pChart = getRootShape();

    drawing::HomogenMatrix3 aTransformation;
    std::map<OUString, uno::Any>::const_iterator itr =
        maProperties.find("Transformation");
    if(itr != maProperties.end())
    {
        SAL_INFO("chart2.opengl", "found a transformation");
        if(itr->second.hasValue())
        {
            aTransformation = itr->second.get<drawing::HomogenMatrix3>();
        }
    }
    else if(maTrans.hasValue())
    {
        aTransformation = maTrans.get<drawing::HomogenMatrix3>();
    }
    pChart->m_GLRender.CreateTextTexture(maBitmap, maPosition, maSize,
            mnRotation, aTransformation);
    pChart->m_GLRender.RenderTextShape();
}

void SAL_CALL DummyText::setPropertyValue( const OUString& rName, const uno::Any& rValue)
{
    SAL_INFO("chart2.opengl", "property value set after image has been created");
    DummyXShape::setPropertyValue(rName, rValue);
}

void SAL_CALL DummyText::setPosition(const awt::Point& rPosition )
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
    setPropertyValue("Transformation", uno::Any(rMatrix));
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
{
    for(std::vector<DummyXShape*>::const_iterator itr = maShapes.begin(),
            itrEnd = maShapes.end(); itr != itrEnd; ++itr)
    {
        const awt::Point& rOldPos = (*itr)->getPos();
        awt::Point aNewPos( rPos.X + rOldPos.X, rPos.Y + rOldPos.Y);
        (*itr)->setPosition(aNewPos);
    }
}

void SAL_CALL DummyGroup2D::setSize( const awt::Size& )
{
    SAL_WARN("chart2.opengl", "set size on group shape");
}

DummyGraphic2D::DummyGraphic2D(const drawing::Position3D& rPos, const drawing::Direction3D& rSize)
{
    setPosition(Position3DToAWTPoint(rPos));
    setSize(Direction3DToAWTSize(rSize));
}

DummyChart* DummyXShape::getRootShape()
{
    assert(mxParent.is());
    DummyXShape& rParent = dynamic_cast<DummyXShape&>(*mxParent.get());
    return rParent.getRootShape();
}

DummyChart* DummyChart::getRootShape()
{
    return this;
}

uno::Any SAL_CALL DummyXShapes::queryInterface( const uno::Type& rType )
{
    if( rType == cppu::UnoType<drawing::XShapes>::get() )
        return uno::Any(uno::Reference<drawing::XShapes>(this));
    if( rType == cppu::UnoType<container::XIndexAccess>::get() )
        return uno::Any(uno::Reference<container::XIndexAccess>(this));
    return DummyXShape::queryInterface(rType);
}

uno::Any SAL_CALL DummyXShapes::queryAggregation( const uno::Type & rType )
{
    if( rType == cppu::UnoType<drawing::XShapes>::get() )
        return uno::Any(uno::Reference< drawing::XShapes >(this));
    else
        return DummyXShape::queryAggregation( rType );
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
{
    DummyXShape& rChild = dynamic_cast<DummyXShape&>(*xShape.get());
    maUNOShapes.push_back(xShape);
    rChild.setParent(static_cast< ::cppu::OWeakObject* >( this ));
    maShapes.push_back(&rChild);
}

void SAL_CALL DummyXShapes::remove( const uno::Reference< drawing::XShape>& xShape )
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
{
    return cppu::UnoType<drawing::XShape>::get();
}

sal_Bool SAL_CALL DummyXShapes::hasElements()
{
    return !maUNOShapes.empty();
}

sal_Int32 SAL_CALL DummyXShapes::getCount()
{
    return maUNOShapes.size();
}

uno::Any SAL_CALL DummyXShapes::getByIndex(sal_Int32 nIndex)
{
    uno::Any aShape;
    aShape <<= maUNOShapes[nIndex];
    return aShape;
}

void DummyXShapes::render()
{
    SAL_INFO("chart2.opengl", "render DummyShapes");
    for(std::vector<DummyXShape*>::iterator itr = maShapes.begin(),
            itrEnd = maShapes.end(); itr != itrEnd; ++itr)
    {
        (*itr)->render();
    }
}

DummyChart::DummyChart():
    mbNotInit(true),
    m_GLRender()
{
    SAL_INFO("chart2.opengl", "DummyXShape::DummyChart()-----test: ");
    setName("com.sun.star.chart2.shapes");
}

void SAL_CALL DummyChart::setPosition( const awt::Point& aPosition )
{
    DummyXShape::setPosition(aPosition);
}

DummyChart::~DummyChart()
{
}

void SAL_CALL DummyChart::setSize( const awt::Size& aSize )
{
    SAL_INFO("chart2.opengl", "DummyChart::setSize()---aSize.Width = " << aSize.Width << ", aSize.Height = " << aSize.Height);
    int width = aSize.Width;
    int height = aSize.Height;
    DummyXShape::setSize(awt::Size(0,0));
    m_GLRender.SetSize(width, height);
    SAL_INFO("chart2.opengl", "DummyChart::GLRender.Width = " << width << ", GLRender.Height = " << height);
}

void DummyChart::render()
{
    if(mbNotInit)
    {
        m_GLRender.InitOpenGL();
        mbNotInit = false;
    }

    SAL_INFO("chart2.opengl", "render chart");
    m_GLRender.prepareToRender();
#if 0
    m_GLRender.renderDebug();
#else
    DummyXShapes::render();
#endif
}

void DummyChart::clear()
{
    maUNOShapes.clear();
    maShapes.clear();
}

}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
