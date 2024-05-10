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

#include <drawingml/textbody.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <drawingml/textparagraph.hxx>
#include <oox/helper/propertyset.hxx>
#include <oox/token/properties.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::beans;

namespace oox::drawingml {

TextBody::TextBody()
    : mbHasNoninheritedBodyProperties( false )
{
}

TextBody::TextBody( const TextBodyPtr& pBody )
    : mbHasNoninheritedBodyProperties( false )
{
    if( pBody ) {
        maTextProperties = pBody->maTextProperties;
        maTextListStyle = pBody->maTextListStyle;
    }
}

TextParagraph& TextBody::addParagraph()
{
    auto xPara = std::make_shared<TextParagraph>();
    maParagraphs.push_back( xPara );
    return *xPara;
}

void TextBody::insertAt(
        const ::oox::core::XmlFilterBase& rFilterBase,
        const Reference < XText > & xText,
        const Reference < XTextCursor > & xAt,
        const TextCharacterProperties& rTextStyleProperties,
        const TextListStylePtr& pMasterTextListStylePtr ) const
{
    TextListStyle aMasterTextStyle(*pMasterTextListStylePtr);

    Reference<css::beans::XPropertySet> xPropertySet(xAt, UNO_QUERY);
    float nCharHeight = xPropertySet->getPropertyValue(u"CharHeight"_ustr).get<float>();
    size_t nIndex = 0;
    for (auto const& paragraph : maParagraphs)
    {
        paragraph->insertAt(rFilterBase, xText, xAt, rTextStyleProperties, aMasterTextStyle,
                            maTextListStyle, (nIndex == 0), nCharHeight);
        ++nIndex;
    }
}

bool TextBody::isEmpty() const
{
    if (maParagraphs.empty())
        return true;
    if ( maParagraphs.size() > 1 )
        return false;

    const TextRunVector& rRuns = maParagraphs[0]->getRuns();
    if ( rRuns.empty() )
        return true;
    if ( rRuns.size() > 1 )
        return false;

    return rRuns[0]->getText().isEmpty();
}

OUString TextBody::toString() const
{
    if (!isEmpty())
    {
        const TextRunVector& rRuns = maParagraphs.front()->getRuns();
        if(!rRuns.empty())
            return rRuns.front()->getText();
    }
    return OUString();
}

bool TextBody::hasVisualRunProperties() const
{
    for ( auto& pTextParagraph : getParagraphs() )
    {
        if ( pTextParagraph->hasVisualRunProperties() )
            return true;
    }
    return false;
}

bool TextBody::hasParagraphProperties() const
{
    for ( auto& pTextParagraph : getParagraphs() )
    {
        if ( pTextParagraph->hasProperties() )
            return true;
    }
    return false;
}

void TextBody::ApplyStyleEmpty(
    const ::oox::core::XmlFilterBase& rFilterBase,
    const Reference < XText > & xText,
    const TextCharacterProperties& rTextStyleProperties,
    const TextListStylePtr& pMasterTextListStylePtr) const
{
    assert(isEmpty());

    if (maParagraphs.empty())
        return;

    // Apply character properties
    PropertySet aPropSet(xText);
    TextCharacterProperties aTextCharacterProps(maParagraphs[0]->getCharacterStyle(
        rTextStyleProperties, *pMasterTextListStylePtr, maTextListStyle));
    aTextCharacterProps.pushToPropSet(aPropSet, rFilterBase);

    // Apply paragraph properties
    TextListStyle aCombinedTextStyle;
    aCombinedTextStyle.apply(*pMasterTextListStylePtr);
    aCombinedTextStyle.apply(maTextListStyle);

    TextParagraphProperties* pTextParagraphStyle = maParagraphs[0]->getParagraphStyle(aCombinedTextStyle);
    if (pTextParagraphStyle)
    {
        Reference< XPropertySet > xProps(xText, UNO_QUERY);
        PropertyMap aioBulletList;
        aioBulletList.setProperty< sal_Int32 >(PROP_LeftMargin, 0); // Init bullets left margin to 0 (no bullets).
        float nCharHeight = xProps->getPropertyValue(u"CharHeight"_ustr).get<float>();
        TextParagraphProperties aParaProp;
        aParaProp.apply(*pTextParagraphStyle);
        aParaProp.pushToPropSet(&rFilterBase, xProps, aioBulletList, &pTextParagraphStyle->getBulletList(),
                                true, nCharHeight, true);
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
