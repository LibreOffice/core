/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: textbody.cxx,v $
 * $Revision: 1.6 $
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

#include "oox/drawingml/textbody.hxx"

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
        const TextListStylePtr& pMasterTextListStylePtr )
{
#ifdef DEBUG
    if ( false )
    {
        const TextParagraphPropertiesVector& rListStyle( pMasterTextListStylePtr->getListStyle() );
        TextParagraphPropertiesVector::const_iterator aIter( rListStyle.begin() );
        while( aIter != rListStyle.end() )
        {
            (*aIter)->getTextParagraphPropertyMap().dump_debug("TextParagraph paragraph props");
            (*aIter)->getTextCharacterProperties()->getTextCharacterPropertyMap().dump_debug("TextParagraph paragraph props");
            aIter++;
        }
    }
#endif

    TextListStylePtr aCombinedTextStyle( new TextListStyle() );
    aCombinedTextStyle->apply( *pMasterTextListStylePtr );
    aCombinedTextStyle->apply( maTextListStyle );

    TextParagraphVector::iterator begin( maParagraphs.begin() );
    TextParagraphVector::iterator end( maParagraphs.end() );
    // apparently if there is no paragraph, it crashes. this is sort of the
    // expected behavior.
    while( begin != end )
    {
        (*begin)->insertAt( rFilterBase, xText, xAt, aCombinedTextStyle, begin == maParagraphs.begin() );
        begin++;
/*
            std::for_each( begin, end,
                                         boost::bind( &TextParagraph::insertAt, _1,
                                                                    rFilterBase, xText, xAt, xModel, aCombinedTextStyle,
                                                                    // determine whether it is the first paragraph of not
                                                                    boost::bind( std::equal_to<TextParagraphPtr>(), _1,
                                                                                             *maParagraphs.begin() ) ) );
*/
    }
}


} }
