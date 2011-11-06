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



#include "oox/drawingml/textparagraph.hxx"
#include "oox/drawingml/drawingmltypes.hxx"

#include <rtl/ustring.hxx>
#include "oox/helper/propertyset.hxx"
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextCursor.hpp>
#include <com/sun/star/text/ControlCharacter.hpp>

using ::rtl::OUString;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;

namespace oox { namespace drawingml {

TextParagraph::TextParagraph()
{
}

TextParagraph::~TextParagraph()
{
}

void TextParagraph::insertAt(
        const ::oox::core::XmlFilterBase& rFilterBase,
        const Reference < XText > &xText,
        const Reference < XTextCursor > &xAt,
        const TextCharacterProperties& rTextStyleProperties,
        const TextListStyle& rTextListStyle, bool bFirst) const
{
    try {
        sal_Int32 nParagraphSize = 0;
        Reference< XTextRange > xStart( xAt, UNO_QUERY );

        sal_Int16 nLevel = maProperties.getLevel();
        const TextParagraphPropertiesVector& rListStyle = rTextListStyle.getListStyle();
        if ( nLevel >= static_cast< sal_Int16 >( rListStyle.size() ) )
            nLevel = 0;
        TextParagraphPropertiesPtr pTextParagraphStyle;
        if ( rListStyle.size() )
            pTextParagraphStyle = rListStyle[ nLevel ];

        TextCharacterProperties aTextCharacterStyle( rTextStyleProperties );
        if ( pTextParagraphStyle.get() )
            aTextCharacterStyle.assignUsed( pTextParagraphStyle->getTextCharacterProperties() );
        aTextCharacterStyle.assignUsed( maProperties.getTextCharacterProperties() );

        if( !bFirst )
        {
            xText->insertControlCharacter( xStart, ControlCharacter::APPEND_PARAGRAPH, sal_False );
            xAt->gotoEnd( sal_True );
        }
        if ( maRuns.begin() == maRuns.end() )
        {
            PropertySet aPropSet( xStart );

            TextCharacterProperties aTextCharacterProps( aTextCharacterStyle );
            aTextCharacterProps.assignUsed( maEndProperties );
            aTextCharacterProps.pushToPropSet( aPropSet, rFilterBase );
        }
        else
        {
            for( TextRunVector::const_iterator aIt = maRuns.begin(), aEnd = maRuns.end(); aIt != aEnd; ++aIt )
            {
                (*aIt)->insertAt( rFilterBase, xText, xAt, aTextCharacterStyle );
                nParagraphSize += (*aIt)->getText().getLength();
            }
        }
        xAt->gotoEnd( sal_True );

        PropertyMap aioBulletList;
        Reference< XPropertySet > xProps( xStart, UNO_QUERY);
        float fCharacterSize = 18;
        if ( pTextParagraphStyle.get() )
        {
            pTextParagraphStyle->pushToPropSet( rFilterBase, xProps, aioBulletList, NULL, sal_False, fCharacterSize );
            fCharacterSize = pTextParagraphStyle->getCharHeightPoints( 18 );
        }
        maProperties.pushToPropSet( rFilterBase, xProps, aioBulletList, &pTextParagraphStyle->getBulletList(), sal_True, fCharacterSize );

        // empty paragraphs do not have bullets in ppt
        if ( !nParagraphSize )
        {
            const OUString sNumberingLevel( CREATE_OUSTRING( "NumberingLevel" ) );
            xProps->setPropertyValue( sNumberingLevel, Any( static_cast< sal_Int16 >( -1 ) ) );
        }

// FIXME this is causing a lot of dispruption (ie does not work). I wonder what to do -- Hub
//          Reference< XTextRange > xEnd( xAt, UNO_QUERY );
//      Reference< XPropertySet > xProps2( xEnd, UNO_QUERY );
//          mpEndProperties->pushToPropSet( xProps2 );
    }
    catch( Exception & )
    {
        OSL_TRACE("OOX: exception in TextParagraph::insertAt");
    }
}


} }

