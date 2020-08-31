/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "VButton.hxx"

#include <ShapeFactory.hxx>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#include <com/sun/star/drawing/TextHorizontalAdjust.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <memory>

#include <CommonConverters.hxx>
#include <editeng/unoprnms.hxx>

namespace chart
{

using namespace css;

VButton::VButton()
    : m_bShowArrow(true)
    , m_nArrowColor(0x00000000)
    , m_nBGColor(0x00E6E6E6)
{
}

void VButton::init(const uno::Reference<drawing::XShapes>& xTargetPage,
                  const uno::Reference<lang::XMultiServiceFactory>& xFactory)
{
    m_xTarget = xTargetPage;
    m_xShapeFactory = xFactory;
}

uno::Reference<drawing::XShape> VButton::createTriangle(awt::Size aSize)
{
    uno::Reference<drawing::XShape> xShape;
    xShape.set(m_xShapeFactory->createInstance("com.sun.star.drawing.PolyPolygonShape"), uno::UNO_QUERY);

    if (!xShape.is())
        return xShape;

    uno::Reference<beans::XPropertySet> xproperties(xShape, uno::UNO_QUERY);

    drawing::PolyPolygonShape3D aPolyPolygon;
    aPolyPolygon.SequenceX.realloc(1);
    aPolyPolygon.SequenceY.realloc(1);
    aPolyPolygon.SequenceZ.realloc(1);

    drawing::DoubleSequence* pOuterSequenceX = aPolyPolygon.SequenceX.getArray();
    drawing::DoubleSequence* pOuterSequenceY = aPolyPolygon.SequenceY.getArray();
    drawing::DoubleSequence* pOuterSequenceZ = aPolyPolygon.SequenceZ.getArray();

    pOuterSequenceX->realloc(3);
    pOuterSequenceY->realloc(3);
    pOuterSequenceZ->realloc(3);

    double* pInnerSequenceX = pOuterSequenceX->getArray();
    double* pInnerSequenceY = pOuterSequenceY->getArray();
    double* pInnerSequenceZ = pOuterSequenceZ->getArray();

    pInnerSequenceX[0] = 0.0;
    pInnerSequenceY[0] = 0.0;
    pInnerSequenceZ[0] = 0.0;

    pInnerSequenceX[1] = aSize.Width / 2.0;
    pInnerSequenceY[1] = aSize.Height;
    pInnerSequenceZ[1] = 0.0;

    pInnerSequenceX[2] = aSize.Width;
    pInnerSequenceY[2] = 0.0;
    pInnerSequenceZ[2] = 0.0;

    xproperties->setPropertyValue("Name", uno::makeAny(m_sCID));
    xproperties->setPropertyValue(UNO_NAME_POLYPOLYGON, uno::Any(PolyToPointSequence(aPolyPolygon)));
    xproperties->setPropertyValue("LineStyle", uno::makeAny(drawing::LineStyle_NONE));
    xproperties->setPropertyValue("FillColor", uno::makeAny(m_nArrowColor));

    return xShape;
}

void VButton::createShapes(const uno::Reference<beans::XPropertySet>& xTextProp)
{
    ShapeFactory* pShapeFactory = ShapeFactory::getOrCreateShapeFactory(m_xShapeFactory);

    std::unique_ptr<tNameSequence> pPropNames(new tNameSequence);
    std::unique_ptr<tAnySequence> pPropValues(new tAnySequence);

    PropertyMapper::getTextLabelMultiPropertyLists(xTextProp, *pPropNames, *pPropValues);

    m_xShape.set(pShapeFactory->createGroup2D(m_xTarget, m_sCID), uno::UNO_QUERY);
    m_xShape->setPosition(m_aPosition);
    m_xShape->setSize(m_aSize);

    uno::Reference<drawing::XShapes> xContainer(m_xShape, uno::UNO_QUERY);
    if (!xContainer.is())
        return;

    tPropertyNameValueMap aTextValueMap;
    aTextValueMap["CharHeight"] <<= 10.0f;
    aTextValueMap["CharHeightAsian"] <<= 10.0f;
    aTextValueMap["CharHeightComplex"] <<= 10.0f;
    aTextValueMap["FillColor"] <<= m_nBGColor;
    aTextValueMap["FillStyle"] <<= drawing::FillStyle_SOLID;
    aTextValueMap["LineColor"] <<= sal_Int32(0xcccccc);
    aTextValueMap["LineStyle"] <<= drawing::LineStyle_SOLID;
    aTextValueMap["ParaAdjust"] <<= style::ParagraphAdjust_CENTER;
    aTextValueMap["TextHorizontalAdjust"] <<= drawing::TextHorizontalAdjust_LEFT;
    aTextValueMap["TextVerticalAdjust"] <<= drawing::TextVerticalAdjust_CENTER;
    aTextValueMap["ParaLeftMargin"] <<= sal_Int32(100);
    aTextValueMap["ParaRightMargin"] <<= sal_Int32(600);

    aTextValueMap["Name"] <<= m_sCID; //CID OUString

    PropertyMapper::getMultiPropertyListsFromValueMap(*pPropNames, *pPropValues, aTextValueMap);

    uno::Reference<drawing::XShape> xEntry = pShapeFactory->createText(
        xContainer, m_sLabel, *pPropNames, *pPropValues, uno::Any());

    if (xEntry.is())
    {
        xEntry->setPosition(m_aPosition);
        xEntry->setSize(m_aSize);
    }

    if (!m_bShowArrow)
        return;

    awt::Size aPolySize {280, 180};

    uno::Reference<drawing::XShape> xPoly = createTriangle(aPolySize);
    if (xPoly.is())
    {
        xPoly->setSize(aPolySize);
        xPoly->setPosition({ sal_Int32(m_aPosition.X + m_aSize.Width - aPolySize.Width - 100),
                             sal_Int32(m_aPosition.Y + (m_aSize.Height / 2.0) - (aPolySize.Height / 2.0)) });
        xContainer->add(xPoly);
    }
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
