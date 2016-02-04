/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <accessibility/helper/characterattributeshelper.hxx>
#include <tools/gen.hxx>
#include <comphelper/sequence.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;


CharacterAttributesHelper::CharacterAttributesHelper( const vcl::Font& rFont, sal_Int32 nBackColor, sal_Int32 nColor )
{
    m_aAttributeMap.insert( AttributeMap::value_type( OUString( "CharBackColor" ),     makeAny( (sal_Int32) nBackColor ) ) );
    m_aAttributeMap.insert( AttributeMap::value_type( OUString( "CharColor" ),         makeAny( (sal_Int32) nColor ) ) );
    m_aAttributeMap.insert( AttributeMap::value_type( OUString( "CharFontCharSet" ),   makeAny( (sal_Int16) rFont.GetCharSet() ) ) );
    m_aAttributeMap.insert( AttributeMap::value_type( OUString( "CharFontFamily" ),    makeAny( (sal_Int16) rFont.GetFamilyType() ) ) );
    m_aAttributeMap.insert( AttributeMap::value_type( OUString( "CharFontName" ),      makeAny( rFont.GetFamilyName() ) ) );
    m_aAttributeMap.insert( AttributeMap::value_type( OUString( "CharFontPitch" ),     makeAny( (sal_Int16) rFont.GetPitch() ) ) );
    m_aAttributeMap.insert( AttributeMap::value_type( OUString( "CharFontStyleName" ), makeAny( rFont.GetStyleName() ) ) );
    m_aAttributeMap.insert( AttributeMap::value_type( OUString( "CharHeight" ),        makeAny( (sal_Int16) rFont.GetFontSize().Height() ) ) );
    m_aAttributeMap.insert( AttributeMap::value_type( OUString( "CharScaleWidth" ),    makeAny( (sal_Int16) rFont.GetFontSize().Width() ) ) );
    m_aAttributeMap.insert( AttributeMap::value_type( OUString( "CharStrikeout" ),     makeAny( (sal_Int16) rFont.GetStrikeout() ) ) );
    m_aAttributeMap.insert( AttributeMap::value_type( OUString( "CharUnderline" ),     makeAny( (sal_Int16) rFont.GetUnderline() ) ) );
    m_aAttributeMap.insert( AttributeMap::value_type( OUString( "CharWeight" ),        makeAny( (float) rFont.GetWeight() ) ) );
    m_aAttributeMap.insert( AttributeMap::value_type( OUString( "CharPosture" ),       makeAny( (sal_Int16)rFont.GetItalic() ) ) );
}


CharacterAttributesHelper::~CharacterAttributesHelper()
{
    m_aAttributeMap.clear();
}


std::vector< PropertyValue > CharacterAttributesHelper::GetCharacterAttributes()
{
    std::vector< PropertyValue > aValues( m_aAttributeMap.size() );

    int i = 0;
    for ( AttributeMap::iterator aIt = m_aAttributeMap.begin(); aIt != m_aAttributeMap.end(); ++aIt, ++i )
    {
        aValues[i].Name   = aIt->first;
        aValues[i].Handle = (sal_Int32) -1;
        aValues[i].Value  = aIt->second;
        aValues[i].State  = PropertyState_DIRECT_VALUE;
    }

    return aValues;
}


Sequence< PropertyValue > CharacterAttributesHelper::GetCharacterAttributes( const css::uno::Sequence< OUString >& aRequestedAttributes )
{
    if ( aRequestedAttributes.getLength() == 0 )
        return comphelper::containerToSequence(GetCharacterAttributes());

    std::vector< PropertyValue > aValues;
    sal_Int32 nLength = aRequestedAttributes.getLength();

    AttributeMap aAttributeMap;

    for ( sal_Int32 i = 0; i < nLength; ++i )
    {
        AttributeMap::iterator aFound = m_aAttributeMap.find( aRequestedAttributes[i] );
        if ( aFound != m_aAttributeMap.end() )
            aAttributeMap.insert( *aFound );
    }

    aValues.reserve( aAttributeMap.size() );

    int i = 0;
    for ( AttributeMap::iterator aIt = aAttributeMap.begin(); aIt != aAttributeMap.end(); ++aIt, ++i )
    {
        aValues[i].Name   = aIt->first;
        aValues[i].Handle = (sal_Int32) -1;
        aValues[i].Value  = aIt->second;
        aValues[i].State  = PropertyState_DIRECT_VALUE;
    }
    return comphelper::containerToSequence(aValues);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
