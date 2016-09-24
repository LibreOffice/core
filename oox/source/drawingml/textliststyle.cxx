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

#include "drawingml/textliststyle.hxx"
#include <osl/diagnose.h>

namespace oox { namespace drawingml {

TextListStyle::TextListStyle()
{
    for ( int i = 0; i < 9; i++ )
    {
        maListStyle.push_back( std::make_shared<TextParagraphProperties>( ) );
        maAggregationListStyle.push_back( std::make_shared<TextParagraphProperties>( ) );
    }
}

TextListStyle::~TextListStyle()
{
}

TextListStyle::TextListStyle(const TextListStyle& rStyle)
{
    assert(rStyle.maListStyle.size() == 9);
    assert(rStyle.maAggregationListStyle.size() == 9);
    for ( size_t i = 0; i < 9; i++ )
    {
        maListStyle.push_back( std::make_shared<TextParagraphProperties>( *rStyle.maListStyle[i] ) );
        maAggregationListStyle.push_back( std::make_shared<TextParagraphProperties>( *rStyle.maAggregationListStyle[i] ) );
    }
}

TextListStyle& TextListStyle::operator=(const TextListStyle& rStyle)
{
    if(this != &rStyle)
    {
        assert(rStyle.maListStyle.size() == 9);
        assert(rStyle.maAggregationListStyle.size() == 9);
        assert(maListStyle.size() == 9);
        assert(maAggregationListStyle.size() == 9);
        for ( size_t i = 0; i < 9; i++ )
        {
            *maListStyle[i] = *rStyle.maListStyle[i];
            *maAggregationListStyle[i] = *rStyle.maAggregationListStyle[i];
        }
    }
    return *this;
}

void applyStyleList( const TextParagraphPropertiesVector& rSourceListStyle, TextParagraphPropertiesVector& rDestListStyle )
{
    TextParagraphPropertiesVector::const_iterator aSourceListStyleIter( rSourceListStyle.begin() );
    TextParagraphPropertiesVector::iterator aDestListStyleIter( rDestListStyle.begin() );
    while( aSourceListStyleIter != rSourceListStyle.end() )
    {
        if ( aDestListStyleIter != rDestListStyle.end() )
        {
            (*aDestListStyleIter)->apply( **aSourceListStyleIter );
            ++aDestListStyleIter;
        }
        else
            rDestListStyle.push_back( std::make_shared<TextParagraphProperties>( **aSourceListStyleIter ) );
        ++aSourceListStyleIter;
    }
}

void TextListStyle::apply( const TextListStyle& rTextListStyle )
{
    applyStyleList( rTextListStyle.getAggregationListStyle(), getAggregationListStyle() );
    applyStyleList( rTextListStyle.getListStyle(), getListStyle() );
}

#ifdef DBG_UTIL
void TextListStyle::dump() const
{
    for ( int i = 0; i < 9; i++ )
    {
        SAL_INFO("oox.drawingml", "text list style level: " << i);
        maListStyle[i]->dump();
    }
}
#endif
} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
