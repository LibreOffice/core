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



#include "oox/drawingml/textliststylecontext.hxx"
#include "oox/drawingml/textparagraphpropertiescontext.hxx"
#include "oox/helper/attributelist.hxx"

using ::rtl::OUString;
using namespace ::oox::core;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

namespace oox { namespace drawingml {

// --------------------------------------------------------------------

// CT_TextListStyle
TextListStyleContext::TextListStyleContext( ContextHandler& rParent, TextListStyle& rTextListStyle )
: ContextHandler( rParent )
, mrTextListStyle( rTextListStyle )
{
}

TextListStyleContext::~TextListStyleContext()
{
}

// --------------------------------------------------------------------

void TextListStyleContext::endFastElement( sal_Int32 ) throw (SAXException, RuntimeException)
{
}

// --------------------------------------------------------------------

Reference< XFastContextHandler > TextListStyleContext::createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& rxAttributes ) throw (SAXException, RuntimeException)
{
    Reference< XFastContextHandler > xRet;
    switch( aElementToken )
    {
        case A_TOKEN( defPPr ):     // CT_TextParagraphProperties
            xRet.set( new TextParagraphPropertiesContext( *this, rxAttributes, *mrTextListStyle.getListStyle()[ 0 ] ) );
            break;
        case A_TOKEN( outline1pPr ):
            xRet.set( new TextParagraphPropertiesContext( *this, rxAttributes, *mrTextListStyle.getAggregationListStyle()[ 0 ] ) );
            break;
        case A_TOKEN( outline2pPr ):
            xRet.set( new TextParagraphPropertiesContext( *this, rxAttributes, *mrTextListStyle.getAggregationListStyle()[ 1 ] ) );
            break;
        case A_TOKEN( lvl1pPr ):
            xRet.set( new TextParagraphPropertiesContext( *this, rxAttributes, *mrTextListStyle.getListStyle()[ 0 ] ) );
            break;
        case A_TOKEN( lvl2pPr ):
            xRet.set( new TextParagraphPropertiesContext( *this, rxAttributes, *mrTextListStyle.getListStyle()[ 1 ] ) );
            break;
        case A_TOKEN( lvl3pPr ):
            xRet.set( new TextParagraphPropertiesContext( *this, rxAttributes, *mrTextListStyle.getListStyle()[ 2 ] ) );
            break;
        case A_TOKEN( lvl4pPr ):
            xRet.set( new TextParagraphPropertiesContext( *this, rxAttributes, *mrTextListStyle.getListStyle()[ 3 ] ) );
            break;
        case A_TOKEN( lvl5pPr ):
            xRet.set( new TextParagraphPropertiesContext( *this, rxAttributes, *mrTextListStyle.getListStyle()[ 4 ] ) );
            break;
        case A_TOKEN( lvl6pPr ):
            xRet.set( new TextParagraphPropertiesContext( *this, rxAttributes, *mrTextListStyle.getListStyle()[ 5 ] ) );
            break;
        case A_TOKEN( lvl7pPr ):
            xRet.set( new TextParagraphPropertiesContext( *this, rxAttributes, *mrTextListStyle.getListStyle()[ 6 ] ) );
            break;
        case A_TOKEN( lvl8pPr ):
            xRet.set( new TextParagraphPropertiesContext( *this, rxAttributes, *mrTextListStyle.getListStyle()[ 7 ] ) );
            break;
        case A_TOKEN( lvl9pPr ):
            xRet.set( new TextParagraphPropertiesContext( *this, rxAttributes, *mrTextListStyle.getListStyle()[ 8 ] ) );
            break;
    }
    if ( !xRet.is() )
        xRet.set( this );
    return xRet;
}

// --------------------------------------------------------------------

} }

