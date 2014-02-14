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

#include "oox/drawingml/textbody.hxx"
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextCursor.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include "oox/drawingml/textparagraph.hxx"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::frame;

namespace oox { namespace drawingml {


TextBody::TextBody()
{
}

TextBody::TextBody( TextBodyPtr pBody )
{
    if( pBody.get() ) {
        maTextProperties = pBody->maTextProperties;
        maTextListStyle = pBody->maTextListStyle;
    }
}

TextBody::~TextBody()
{
}

TextParagraph& TextBody::addParagraph()
{
    TextParagraphPtr xPara( new TextParagraph );
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
    TextListStyle aCombinedTextStyle;
    aCombinedTextStyle.apply( *pMasterTextListStylePtr );
    aCombinedTextStyle.apply( maTextListStyle );

    Reference<css::beans::XPropertySet> xPropertySet(xAt, UNO_QUERY);
    float nCharHeight = xPropertySet->getPropertyValue("CharHeight").get<float>();
    for( TextParagraphVector::const_iterator aBeg = maParagraphs.begin(), aIt = aBeg, aEnd = maParagraphs.end(); aIt != aEnd; ++aIt )
        (*aIt)->insertAt( rFilterBase, xText, xAt, rTextStyleProperties, aCombinedTextStyle, aIt == aBeg, nCharHeight );
}

bool TextBody::isEmpty()
{
    if ( maParagraphs.size() <= 0 )
        return true;
    if ( maParagraphs.size() > 1 )
        return false;

    const TextRunVector aRuns = maParagraphs[0]->getRuns();
    if ( aRuns.size() <= 0 )
        return true;
    if ( aRuns.size() > 1 )
        return false;

    return aRuns[0]->getText().getLength() <= 0;
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
