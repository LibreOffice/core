/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include "oox/drawingml/textparagraph.hxx"
#include "oox/drawingml/drawingmltypes.hxx"

#include <rtl/ustring.hxx>
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

        for( TextRunVector::const_iterator aIt = maRuns.begin(), aEnd = maRuns.end(); aIt != aEnd; ++aIt )
        {
            (*aIt)->insertAt( rFilterBase, xText, xAt, aTextCharacterStyle );
            nParagraphSize += (*aIt)->getText().getLength();
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

