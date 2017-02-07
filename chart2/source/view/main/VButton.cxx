/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "VButton.hxx"

#include "AbstractShapeFactory.hxx"
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#include <com/sun/star/drawing/TextHorizontalAdjust.hpp>

namespace chart
{

using namespace css;

VButton::VButton()
    : m_xShapeFactory(nullptr)
    , m_xTarget(nullptr)
    , m_xShape(nullptr)
    , m_rPosition(0, 0)
{
}

void VButton::init(const uno::Reference<drawing::XShapes>& xTargetPage,
                  const uno::Reference<lang::XMultiServiceFactory>& xFactory)
{
    m_xTarget = xTargetPage;
    m_xShapeFactory = xFactory;
}

void VButton::createShapes(const awt::Point& rPosition,
                           const awt::Size& rReferenceSize,
                           const uno::Reference<beans::XPropertySet>& xTextProp)
{
    AbstractShapeFactory* pShapeFactory = AbstractShapeFactory::getOrCreateShapeFactory(m_xShapeFactory);

    std::unique_ptr<tNameSequence> pPropNames(new tNameSequence);
    std::unique_ptr<tAnySequence> pPropValues(new tAnySequence);

    PropertyMapper::getTextLabelMultiPropertyLists(xTextProp, *pPropNames, *pPropValues);

    tPropertyNameValueMap aTextValueMap;
    aTextValueMap["CharHeight"] = uno::makeAny<float>(10.0f);
    aTextValueMap["FillColor"] = uno::makeAny<sal_Int32>(0xe6e6e6);
    aTextValueMap["FillStyle"] = uno::makeAny(drawing::FillStyle_SOLID);
    aTextValueMap["LineColor"] = uno::makeAny<sal_Int32>(0xcccccc);
    aTextValueMap["LineStyle"] = uno::makeAny(drawing::LineStyle_SOLID);
    aTextValueMap["ParaAdjust"] = uno::makeAny(style::ParagraphAdjust_CENTER);
    aTextValueMap["TextHorizontalAdjust"] = uno::makeAny(drawing::TextHorizontalAdjust_CENTER);
    aTextValueMap["TextVerticalAdjust"] = uno::makeAny(drawing::TextVerticalAdjust_CENTER);

    aTextValueMap["Name"] = uno::makeAny(OUString(m_sCID)); //CID OUString

    PropertyMapper::getMultiPropertyListsFromValueMap(*pPropNames, *pPropValues, aTextValueMap);

    uno::Reference<drawing::XShape> xEntry = pShapeFactory->createText(
        m_xTarget, m_sLabel, *pPropNames, *pPropValues, uno::Any());

    if (xEntry.is())
    {
        m_xShape = xEntry;
        m_xShape->setPosition(rPosition);
        m_xShape->setSize(rReferenceSize);
    }
}

void VButton::setWidth(sal_Int32 nWidth)
{
    awt::Size aSize = m_xShape->getSize();
    aSize.Width = nWidth;
    m_xShape->setSize(aSize);
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
