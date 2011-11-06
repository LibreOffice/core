/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#include "oox/drawingml/textliststyle.hxx"

namespace oox { namespace drawingml {

TextListStyle::TextListStyle()
{
    for ( int i = 0; i < 9; i++ )
    {
        maListStyle.push_back( TextParagraphPropertiesPtr( new TextParagraphProperties() ) );
        maAggregationListStyle.push_back( TextParagraphPropertiesPtr( new TextParagraphProperties() ) );
    }
}

TextListStyle::~TextListStyle()
{
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
            aDestListStyleIter++;
        }
        else
            rDestListStyle.push_back( TextParagraphPropertiesPtr( new TextParagraphProperties( **aSourceListStyleIter ) ) );
        aSourceListStyleIter++;
    }
}

void TextListStyle::apply( const TextListStyle& rTextListStyle )
{
    applyStyleList( rTextListStyle.getAggregationListStyle(), getAggregationListStyle() );
    applyStyleList( rTextListStyle.getListStyle(), getListStyle() );
}

} }
