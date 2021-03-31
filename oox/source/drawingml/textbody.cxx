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
{
}

TextBody::TextBody( const TextBodyPtr& pBody )
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

void TextBody::appendParagraph(std::shared_ptr<TextParagraph> pTextParagraph)
{
    maParagraphs.push_back(pTextParagraph);
}

void TextBody::insertAt(
        const ::oox::core::XmlFilterBase& rFilterBase,
        const Reference < XText > & xText,
        const Reference < XTextCursor > & xAt,
        const TextCharacterProperties& rTextStyleProperties,
        const TextListStylePtr& pMasterTextListStylePtr ) const
{
    TextListStyle aCombinedTextStyle;
    aCombinedTextStyle.apply( *pMasterTextListStylePtr );
    aCombinedTextStyle.apply( maTextListStyle );

    Reference<css::beans::XPropertySet> xPropertySet(xAt, UNO_QUERY);
    float nCharHeight = xPropertySet->getPropertyValue("CharHeight").get<float>();
    size_t nIndex = 0;
    for (auto const& paragraph : maParagraphs)
    {
        paragraph->insertAt( rFilterBase, xText, xAt, rTextStyleProperties, aCombinedTextStyle, (nIndex == 0), nCharHeight );
        ++nIndex;
    }
}

bool TextBody::isEmpty() const
{
    if (maParagraphs.empty())
        return true;
    if ( maParagraphs.size() > 1 )
        return false;

    const TextRunVector aRuns = maParagraphs[0]->getRuns();
    if ( aRuns.empty() )
        return true;
    if ( aRuns.size() > 1 )
        return false;

    return aRuns[0]->getText().isEmpty();
}

OUString TextBody::toString() const
{
    if (!isEmpty())
        return maParagraphs.front()->getRuns().front()->getText();
    else
        return OUString();
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
    TextListStyle aCombinedTextStyle;
    aCombinedTextStyle.apply( *pMasterTextListStylePtr );
    aCombinedTextStyle.apply( maTextListStyle );

    PropertySet aPropSet(xText);
    TextCharacterProperties aTextCharacterProps(maParagraphs[0]->getCharacterStyle(rTextStyleProperties, aCombinedTextStyle));
    aTextCharacterProps.pushToPropSet(aPropSet, rFilterBase);

    // Apply paragraph properties
    TextParagraphPropertiesPtr pTextParagraphStyle = maParagraphs[0]->getParagraphStyle(aCombinedTextStyle);
    if (pTextParagraphStyle)
    {
        Reference< XPropertySet > xProps(xText, UNO_QUERY);
        PropertyMap aioBulletList;
        aioBulletList.setProperty< sal_Int32 >(PROP_LeftMargin, 0); // Init bullets left margin to 0 (no bullets).
        float nCharHeight = xProps->getPropertyValue("CharHeight").get<float>();
        TextParagraphProperties aParaProp;
        aParaProp.apply(*pTextParagraphStyle);
        aParaProp.pushToPropSet(&rFilterBase, xProps, aioBulletList, &pTextParagraphStyle->getBulletList(), true, nCharHeight, true);
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
