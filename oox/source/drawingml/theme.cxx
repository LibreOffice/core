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

