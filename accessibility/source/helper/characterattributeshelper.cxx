/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: characterattributeshelper.cxx,v $
 * $Revision: 1.4 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_accessibility.hxx"
#include <accessibility/helper/characterattributeshelper.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;


// -----------------------------------------------------------------------------
// CharacterAttributesHelper
// -----------------------------------------------------------------------------

CharacterAttributesHelper::CharacterAttributesHelper( const Font& rFont, sal_Int32 nBackColor, sal_Int32 nColor )
{
    m_aAttributeMap.insert( AttributeMap::value_type( ::rtl::OUString::createFromAscii( "CharBackColor" ), makeAny( (sal_Int32) nBackColor ) ) );
    m_aAttributeMap.insert( AttributeMap::value_type( ::rtl::OUString::createFromAscii( "CharColor" ), makeAny( (sal_Int32) nColor ) ) );
    m_aAttributeMap.insert( AttributeMap::value_type( ::rtl::OUString::createFromAscii( "CharFontCharSet" ), makeAny( (sal_Int16) rFont.GetCharSet() ) ) );
    m_aAttributeMap.insert( AttributeMap::value_type( ::rtl::OUString::createFromAscii( "CharFontFamily" ), makeAny( (sal_Int16) rFont.GetFamily() ) ) );
    m_aAttributeMap.insert( AttributeMap::value_type( ::rtl::OUString::createFromAscii( "CharFontName" ), makeAny( (::rtl::OUString) rFont.GetName() ) ) );
    m_aAttributeMap.insert( AttributeMap::value_type( ::rtl::OUString::createFromAscii( "CharFontPitch" ), makeAny( (sal_Int16) rFont.GetPitch() ) ) );
    m_aAttributeMap.insert( AttributeMap::value_type( ::rtl::OUString::createFromAscii( "CharFontStyleName" ), makeAny( (::rtl::OUString) rFont.GetStyleName() ) ) );
    m_aAttributeMap.insert( AttributeMap::value_type( ::rtl::OUString::createFromAscii( "CharHeight" ), makeAny( (sal_Int16) rFont.GetSize().Height() ) ) );
    m_aAttributeMap.insert( AttributeMap::value_type( ::rtl::OUString::createFromAscii( "CharScaleWidth" ), makeAny( (sal_Int16) rFont.GetSize().Width() ) ) );
    m_aAttributeMap.insert( AttributeMap::value_type( ::rtl::OUString::createFromAscii( "CharStrikeout" ), makeAny( (sal_Int16) rFont.GetStrikeout() ) ) );
    m_aAttributeMap.insert( AttributeMap::value_type( ::rtl::OUString::createFromAscii( "CharUnderline" ), makeAny( (sal_Int16) rFont.GetUnderline() ) ) );
    m_aAttributeMap.insert( AttributeMap::value_type( ::rtl::OUString::createFromAscii( "CharWeight" ), makeAny( (float) rFont.GetWeight() ) ) );
}

// -----------------------------------------------------------------------------

CharacterAttributesHelper::~CharacterAttributesHelper()
{
    m_aAttributeMap.clear();
}

// -----------------------------------------------------------------------------

Sequence< PropertyValue > CharacterAttributesHelper::GetCharacterAttributes()
{
    Sequence< PropertyValue > aValues( m_aAttributeMap.size() );
    PropertyValue* pValues = aValues.getArray();

    for ( AttributeMap::iterator aIt = m_aAttributeMap.begin(); aIt != m_aAttributeMap.end(); ++aIt, ++pValues )
    {
        pValues->Name   = aIt->first;
        pValues->Handle = (sal_Int32) -1;
        pValues->Value  = aIt->second;
        pValues->State  = PropertyState_DIRECT_VALUE;
    }

    return aValues;
}

// -----------------------------------------------------------------------------

Sequence< PropertyValue > CharacterAttributesHelper::GetCharacterAttributes( const Sequence< ::rtl::OUString >& aRequestedAttributes )
{
    Sequence< PropertyValue > aValues;
    sal_Int32 nLength = aRequestedAttributes.getLength();

    if ( nLength != 0 )
    {
        const ::rtl::OUString* pNames = aRequestedAttributes.getConstArray();
        AttributeMap aAttributeMap;

        for ( sal_Int32 i = 0; i < nLength; ++i )
        {
            AttributeMap::iterator aFound = m_aAttributeMap.find( pNames[i] );
            if ( aFound != m_aAttributeMap.end() )
                aAttributeMap.insert( *aFound );
        }

        aValues.realloc( aAttributeMap.size() );
        PropertyValue* pValues = aValues.getArray();

        for ( AttributeMap::iterator aIt = aAttributeMap.begin(); aIt != aAttributeMap.end(); ++aIt, ++pValues )
        {
            pValues->Name   = aIt->first;
            pValues->Handle = (sal_Int32) -1;
            pValues->Value  = aIt->second;
            pValues->State  = PropertyState_DIRECT_VALUE;
        }
    }
    else
    {
        aValues = GetCharacterAttributes();
    }

    return aValues;
}

// -----------------------------------------------------------------------------
