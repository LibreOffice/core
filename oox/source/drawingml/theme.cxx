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

#include "oox/drawingml/theme.hxx"

using ::rtl::OUString;

namespace oox {
namespace drawingml {

// ============================================================================

Theme::Theme()
{
}

Theme::~Theme()
{
}

namespace {

template< typename Type >
const Type* lclGetStyleElement( const RefVector< Type >& rVector, sal_Int32 nIndex )
{
    return (rVector.empty() || (nIndex < 1)) ? 0 :
        rVector.get( ::std::min( static_cast< sal_Int32 >( nIndex - 1 ), static_cast< sal_Int32 >( rVector.size() - 1 ) ) ).get();
}

} // namespace

const FillProperties* Theme::getFillStyle( sal_Int32 nIndex ) const
{
    return (nIndex >= 1000) ?
        lclGetStyleElement( maBgFillStyleList, nIndex - 1000 ) :
        lclGetStyleElement( maFillStyleList, nIndex );
}

const LineProperties* Theme::getLineStyle( sal_Int32 nIndex ) const
{
     return lclGetStyleElement( maLineStyleList, nIndex );
}

const PropertyMap* Theme::getEffectStyle( sal_Int32 nIndex ) const
{
    return lclGetStyleElement( maEffectStyleList, nIndex );
}

const TextCharacterProperties* Theme::getFontStyle( sal_Int32 nSchemeType ) const
{
    return maFontScheme.get( nSchemeType ).get();
}

const TextFont* Theme::resolveFont( const OUString& rName ) const
{
    /*  Resolves the following names:
        +mj-lt, +mj-ea, +mj-cs  --  major Latin, Asian, Complex font
        +mn-lt, +mn-ea, +mn-cs  --  minor Latin, Asian, Complex font
     */
    if( (rName.getLength() == 6) && (rName[ 0 ] == '+') && (rName[ 3 ] == '-') )
    {
        const TextCharacterProperties* pCharProps = 0;
        if( (rName[ 1 ] == 'm') && (rName[ 2 ] == 'j') )
            pCharProps = maFontScheme.get( XML_major ).get();
        else if( (rName[ 1 ] == 'm') && (rName[ 2 ] == 'n') )
            pCharProps = maFontScheme.get( XML_minor ).get();
        if( pCharProps )
        {
            if( (rName[ 4 ] == 'l') && (rName[ 5 ] == 't') )
                return &pCharProps->maLatinFont;
            if( (rName[ 4 ] == 'e') && (rName[ 5 ] == 'a') )
                return &pCharProps->maAsianFont;
            if( (rName[ 4 ] == 'c') && (rName[ 5 ] == 's') )
                return &pCharProps->maComplexFont;
        }
    }
    return 0;
}

// ============================================================================

} // namespace drawingml
} // namespace oox

