/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: textcharacterproperties.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2008-01-17 14:23:40 $
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

#include "oox/drawingml/textcharacterproperties.hxx"

#include "oox/helper/propertyset.hxx"
#include "oox/core/namespaces.hxx"
#include "tokens.hxx"

using rtl::OUString;
using namespace ::oox::core;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;

namespace oox { namespace drawingml {

TextCharacterProperties::TextCharacterProperties()
: maCharColorPtr( new Color() )
, maUnderlineColorPtr( new Color() )
, maHighlightColorPtr( new Color() )
{
}
TextCharacterProperties::~TextCharacterProperties()
{
}
void TextCharacterProperties::apply( const TextCharacterPropertiesPtr& rSourceTextCharacterPropertiesPtr )
{
    maTextCharacterPropertyMap.insert( rSourceTextCharacterPropertiesPtr->maTextCharacterPropertyMap.begin(), rSourceTextCharacterPropertiesPtr->maTextCharacterPropertyMap.end() );
    maHyperlinkPropertyMap.insert( rSourceTextCharacterPropertiesPtr->maHyperlinkPropertyMap.begin(), rSourceTextCharacterPropertiesPtr->maHyperlinkPropertyMap.end() );
    ColorPtr rSourceCharColor( rSourceTextCharacterPropertiesPtr->getCharColor() );
    if ( rSourceCharColor->isUsed() )
        maCharColorPtr = rSourceCharColor;
    ColorPtr rSourceHighlightColor( rSourceTextCharacterPropertiesPtr->getHighlightColor() );
    if ( rSourceHighlightColor->isUsed() )
        maHighlightColorPtr = rSourceHighlightColor;
    ColorPtr rSourceUnderlineColor( rSourceTextCharacterPropertiesPtr->getUnderlineColor() );
    if ( rSourceUnderlineColor->isUsed() )
        maUnderlineColorPtr = rSourceUnderlineColor;
    Any& rHasUnderline = rSourceTextCharacterPropertiesPtr->getHasUnderline();
    if ( rHasUnderline.hasValue() )
        maHasUnderline = rHasUnderline;
    Any& rUnderlineLineFollowText = rSourceTextCharacterPropertiesPtr->getUnderlineLineFollowText();
    if ( rUnderlineLineFollowText.hasValue() )
        maUnderlineLineFollowText = rUnderlineLineFollowText;
    Any& rUnderlineFillFollowText = rSourceTextCharacterPropertiesPtr->getUnderlineFillFollowText();
    if ( rUnderlineFillFollowText.hasValue() )
        maUnderlineFillFollowText = rUnderlineFillFollowText;
}
void TextCharacterProperties::pushToPropSet( const ::oox::core::XmlFilterBase& rFilterBase, const Reference < XPropertySet > & xPropSet ) const
{
    PropertySet aPropSet( xPropSet );
    Sequence< OUString > aNames;
    Sequence< Any > aValues;

//  maTextCharacterPropertyMap.dump_debug("TextCharacter props");
    maTextCharacterPropertyMap.makeSequence( aNames, aValues );
    aPropSet.setProperties( aNames, aValues );
    if ( maCharColorPtr->isUsed() )
    {
        const rtl::OUString sCharColor( CREATE_OUSTRING( "CharColor" ) );
        aPropSet.setProperty( sCharColor, maCharColorPtr->getColor( rFilterBase ) );

    }

    sal_Bool bHasUnderline = sal_False;
    sal_Bool bUnderlineFillFollowText = sal_False;
    maHasUnderline >>= bHasUnderline;
    maUnderlineFillFollowText >>= bUnderlineFillFollowText;
    if( bHasUnderline )
    {
        if( maUnderlineColorPtr.get() && !bUnderlineFillFollowText )
        {
            const rtl::OUString sCharUnderlineColor( CREATE_OUSTRING( "CharUnderlineColor" ) );
            aPropSet.setProperty( sCharUnderlineColor, maUnderlineColorPtr->getColor( rFilterBase ) );
            const rtl::OUString sCharUnderlineHasColor( CREATE_OUSTRING( "CharUnderlineHasColor" ) );
            aPropSet.setProperty( sCharUnderlineHasColor, Any( sal_True ) );
        }
    }
}

void TextCharacterProperties::pushToUrlFieldPropSet( const Reference < XPropertySet > & xPropSet ) const
{
    PropertySet aPropSet( xPropSet );
    Sequence< OUString > aNames;
    Sequence< Any > aValues;

    maHyperlinkPropertyMap.makeSequence( aNames, aValues );
    aPropSet.setProperties( aNames, aValues );
}

float TextCharacterProperties::getCharacterSize( float fDefault ) const
{
    const rtl::OUString sCharHeight( CREATE_OUSTRING( "CharHeight" ) );
    float fCharHeight = 0;
    const Any* pAny = maTextCharacterPropertyMap.getPropertyValue( sCharHeight );
    if ( pAny && ( *pAny >>= fCharHeight ) )
        return fCharHeight;
    else
        return fDefault;
}

} }
