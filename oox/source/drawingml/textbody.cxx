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



#include "oox/drawingml/textbody.hxx"
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextCursor.hpp>
#include "oox/drawingml/textparagraph.hxx"

using ::rtl::OUString;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::frame;

namespace oox { namespace drawingml {


TextBody::TextBody()
{
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

    for( TextParagraphVector::const_iterator aBeg = maParagraphs.begin(), aIt = aBeg, aEnd = maParagraphs.end(); aIt != aEnd; ++aIt )
        (*aIt)->insertAt( rFilterBase, xText, xAt, rTextStyleProperties, aCombinedTextStyle, aIt == aBeg );
}


} }
