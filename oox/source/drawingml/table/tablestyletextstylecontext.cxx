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



#include <osl/diagnose.h>

#include "oox/drawingml/table/tablestyletextstylecontext.hxx"
#include "oox/drawingml/colorchoicecontext.hxx"
#include "oox/helper/attributelist.hxx"

using namespace ::oox::core;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using ::rtl::OUString;

namespace oox { namespace drawingml { namespace table {

TableStyleTextStyleContext::TableStyleTextStyleContext( ContextHandler& rParent,
    const Reference< XFastAttributeList >& xAttribs, TableStylePart& rTableStylePart )
: ContextHandler( rParent )
, mrTableStylePart( rTableStylePart )
{
    sal_Int32 nB = xAttribs->getOptionalValueToken( XML_b, XML_def );
    if ( nB == XML_on )
        mrTableStylePart.getTextBoldStyle() = ::boost::optional< sal_Bool >( sal_True );
    else if ( nB == XML_off )
        mrTableStylePart.getTextBoldStyle() = ::boost::optional< sal_Bool >( sal_False );

    sal_Int32 nI = xAttribs->getOptionalValueToken( XML_i, XML_def );
    if ( nI == XML_on )
        mrTableStylePart.getTextItalicStyle() = ::boost::optional< sal_Bool >( sal_True );
    else if ( nI == XML_off )
        mrTableStylePart.getTextItalicStyle() = ::boost::optional< sal_Bool >( sal_False );
}

TableStyleTextStyleContext::~TableStyleTextStyleContext()
{
}

// CT_TableStyleTextStyle
uno::Reference< xml::sax::XFastContextHandler > SAL_CALL
TableStyleTextStyleContext::createFastChildContext( ::sal_Int32 aElementToken, const uno::Reference< xml::sax::XFastAttributeList >& xAttribs )
    throw ( xml::sax::SAXException, uno::RuntimeException)
{
    uno::Reference< xml::sax::XFastContextHandler > xRet;
    AttributeList aAttribs( xAttribs );

    switch( aElementToken )
    {
        // EG_ThemeableFontStyles (choice)
        case A_TOKEN( font ):       // CT_FontCollection
            xRet.set( this );
            break;
        case A_TOKEN( ea ):             // CT_TextFont
            mrTableStylePart.getAsianFont().setAttributes( aAttribs );
            return 0;
        case A_TOKEN( cs ):             // CT_TextFont
            mrTableStylePart.getComplexFont().setAttributes( aAttribs );
            return 0;
        case A_TOKEN( sym ):            // CT_TextFont
            mrTableStylePart.getSymbolFont().setAttributes( aAttribs );
            return 0;
        case A_TOKEN( latin ):          // CT_TextFont
            mrTableStylePart.getLatinFont().setAttributes( aAttribs );
            return 0;

        case A_TOKEN( fontRef ):    // CT_FontReference
            {
                ShapeStyleRef& rFontStyle = mrTableStylePart.getStyleRefs()[ XML_fontRef ];
                rFontStyle.mnThemedIdx = aAttribs.getToken( XML_idx, XML_none );
                xRet.set( new ColorContext( *this, rFontStyle.maPhClr ) );
            }
            break;

        case A_TOKEN( extLst ):     // CT_OfficeArtExtensionList
            break;
    }
    if( !xRet.is() )
        xRet.set( new ColorValueContext( *this, mrTableStylePart.getTextColor() ) );

    return xRet;
}

} } }
