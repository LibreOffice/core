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

#include <drawingml/textliststyle.hxx>
#include <sal/log.hxx>

namespace oox::drawingml {

TextListStyle::TextListStyle()
{
    for ( int i = 0; i < NUM_TEXT_LIST_STYLE_ENTRIES; i++ )
    {
        maListStyle[i] = std::make_shared<TextParagraphProperties>( );
        maAggregationListStyle[i] = std::make_shared<TextParagraphProperties>( );
    }
}

TextListStyle::~TextListStyle()
{
}

TextListStyle::TextListStyle(const TextListStyle& rStyle)
{
    for ( size_t i = 0; i < NUM_TEXT_LIST_STYLE_ENTRIES; i++ )
    {
        maListStyle[i] = std::make_shared<TextParagraphProperties>( *rStyle.maListStyle[i] );
        maAggregationListStyle[i] = std::make_shared<TextParagraphProperties>( *rStyle.maAggregationListStyle[i] );
    }
}

TextListStyle& TextListStyle::operator=(const TextListStyle& rStyle)
{
    if(this != &rStyle)
    {
        for ( size_t i = 0; i < NUM_TEXT_LIST_STYLE_ENTRIES; i++ )
        {
            *maListStyle[i] = *rStyle.maListStyle[i];
            *maAggregationListStyle[i] = *rStyle.maAggregationListStyle[i];
        }
    }
    return *this;
}

static void applyStyleList( const TextParagraphPropertiesArray& rSourceListStyle, TextParagraphPropertiesArray& rDestListStyle )
{
    for ( size_t i = 0; i < NUM_TEXT_LIST_STYLE_ENTRIES; i++ )
        rDestListStyle[i]->apply(*rSourceListStyle[i]);
}

void TextListStyle::apply( const TextListStyle& rTextListStyle )
{
    applyStyleList( rTextListStyle.getAggregationListStyle(), getAggregationListStyle() );
    applyStyleList( rTextListStyle.getListStyle(), getListStyle() );
}

#ifdef DBG_UTIL
void TextListStyle::dump() const
{
    for ( int i = 0; i < NUM_TEXT_LIST_STYLE_ENTRIES; i++ )
    {
        SAL_INFO("oox.drawingml", "text list style level: " << i);
        maListStyle[i]->dump();
    }
}
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
