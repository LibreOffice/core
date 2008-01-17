/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: textliststyle.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-17 08:05:52 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include "oox/drawingml/textliststyle.hxx"

namespace oox { namespace drawingml {

TextListStyle::TextListStyle()
    : maListStyle()
    , maAggregationListStyle()
{
    int i;
    for ( i = 0; i < 9; i++ )
        maListStyle.push_back( TextParagraphPropertiesPtr( new TextParagraphProperties() ) );
    for ( i = 0; i < 9; i++ )
        maAggregationListStyle.push_back( TextParagraphPropertiesPtr( new TextParagraphProperties() ) );
}
TextListStyle::~TextListStyle()
{
}

void applyStyleList( const std::vector< ::oox::drawingml::TextParagraphPropertiesPtr >& rSourceListStyle,
        std::vector< ::oox::drawingml::TextParagraphPropertiesPtr >& rDestListStyle )
{
    std::vector< ::oox::drawingml::TextParagraphPropertiesPtr >::const_iterator aSourceListStyleIter( rSourceListStyle.begin() );
    std::vector< ::oox::drawingml::TextParagraphPropertiesPtr >::iterator aDestListStyleIter( rDestListStyle.begin() );
    while( aSourceListStyleIter != rSourceListStyle.end() )
    {
        if ( aDestListStyleIter != rDestListStyle.end() )
        {
            (*aDestListStyleIter)->apply( *aSourceListStyleIter );
            aDestListStyleIter++;
        }
        else
            rDestListStyle.push_back( TextParagraphPropertiesPtr( new oox::drawingml::TextParagraphProperties( *(*aSourceListStyleIter).get() ) ) );
        aSourceListStyleIter++;
    }
}

void TextListStyle::apply( const TextListStylePtr& rTextListStylePtr )
{
    applyStyleList( rTextListStylePtr->getAggregationListStyle(), getAggregationListStyle() );
    applyStyleList( rTextListStylePtr->getListStyle(), getListStyle() );
}

} }
