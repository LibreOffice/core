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
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#include <com/sun/star/drawing/TextHorizontalAdjust.hpp>

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

void VButton::init(const rtl::Reference<SvxShapeGroupAnyD>& xTargetPage)
{
    m_xTarget = xTargetPage;
}

rtl::Reference<SvxShapePolyPolygon> VButton::createTriangle(awt::Size aSize)
{
    rtl::Reference<SvxShapePolyPolygon> xShape = new SvxShapePolyPolygon(nullptr);
    xShape->setShapeKind(SdrObjKind::Polygon);

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

    xShape->SvxShape::setPropertyValue(u"Name"_ustr, uno::Any(m_sCID));
    xShape->SvxShape::setPropertyValue(UNO_NAME_POLYPOLYGON,
                                       uno::Any(PolyToPointSequence(aPolyPolygon)));
    xShape->SvxShape::setPropertyValue(u"LineStyle"_ustr, uno::Any(drawing::LineStyle_NONE));
    xShape->SvxShape::setPropertyValue(u"FillColor"_ustr, uno::Any(m_nArrowColor));

    return xShape;
}

void VButton::createShapes(const uno::Reference<beans::XPropertySet>& xTextProp)
{
    tNameSequence aPropNames;
    tAnySequence aPropValues;

    PropertyMapper::getTextLabelMultiPropertyLists(xTextProp, aPropNames, aPropValues);

    m_xShape = ShapeFactory::createGroup2D(m_xTarget, m_sCID);
    m_xShape->setPosition(m_aPosition);
    m_xShape->setSize(m_aSize);

    rtl::Reference<SvxShapeGroupAnyD> xContainer = m_xShape;

    tPropertyNameValueMap aTextValueMap;
    aTextValueMap[u"CharHeight"_ustr] <<= 10.0f;
    aTextValueMap[u"CharHeightAsian"_ustr] <<= 10.0f;
    aTextValueMap[u"CharHeightComplex"_ustr] <<= 10.0f;
    aTextValueMap[u"FillColor"_ustr] <<= m_nBGColor;
    aTextValueMap[u"FillStyle"_ustr] <<= drawing::FillStyle_SOLID;
    aTextValueMap[u"LineColor"_ustr] <<= sal_Int32(0xcccccc);
    aTextValueMap[u"LineStyle"_ustr] <<= drawing::LineStyle_SOLID;
    aTextValueMap[u"ParaAdjust"_ustr] <<= style::ParagraphAdjust_CENTER;
    aTextValueMap[u"TextHorizontalAdjust"_ustr] <<= drawing::TextHorizontalAdjust_LEFT;
    aTextValueMap[u"TextVerticalAdjust"_ustr] <<= drawing::TextVerticalAdjust_CENTER;
    aTextValueMap[u"ParaLeftMargin"_ustr] <<= sal_Int32(100);
    aTextValueMap[u"ParaRightMargin"_ustr] <<= sal_Int32(600);

    aTextValueMap[u"Name"_ustr] <<= m_sCID; //CID OUString

    PropertyMapper::getMultiPropertyListsFromValueMap(aPropNames, aPropValues, aTextValueMap);

    rtl::Reference<SvxShapeText> xEntry
        = ShapeFactory::createText(xContainer, m_sLabel, aPropNames, aPropValues, uno::Any());

    if (xEntry.is())
    {
        xEntry->setPosition(m_aPosition);
        xEntry->setSize(m_aSize);
    }

    if (!m_bShowArrow)
        return;

    awt::Size aPolySize{ 280, 180 };

    rtl::Reference<SvxShapePolyPolygon> xPoly = createTriangle(aPolySize);
    xPoly->setSize(aPolySize);
    xPoly->setPosition(
        { sal_Int32(m_aPosition.X + m_aSize.Width - aPolySize.Width - 100),
          sal_Int32(m_aPosition.Y + (m_aSize.Height / 2.0) - (aPolySize.Height / 2.0)) });
    xContainer->add(xPoly);
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
