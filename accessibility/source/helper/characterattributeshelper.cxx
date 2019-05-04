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

#include <helper/characterattributeshelper.hxx>
#include <tools/gen.hxx>
#include <comphelper/sequence.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;


CharacterAttributesHelper::CharacterAttributesHelper( const vcl::Font& rFont, sal_Int32 nBackColor, sal_Int32 nColor )
{
    m_aAttributeMap.emplace( OUString( "CharBackColor" ),     Any( nBackColor ) );
    m_aAttributeMap.emplace( OUString( "CharColor" ),         Any( nColor ) );
    m_aAttributeMap.emplace( OUString( "CharFontCharSet" ),   Any( static_cast<sal_Int16>(rFont.GetCharSet()) ) );
    m_aAttributeMap.emplace( OUString( "CharFontFamily" ),    Any( static_cast<sal_Int16>(rFont.GetFamilyType()) ) );
    m_aAttributeMap.emplace( OUString( "CharFontName" ),      Any( rFont.GetFamilyName() ) );
    m_aAttributeMap.emplace( OUString( "CharFontPitch" ),     Any( static_cast<sal_Int16>(rFont.GetPitch()) ) );
    m_aAttributeMap.emplace( OUString( "CharFontStyleName" ), Any( rFont.GetStyleName() ) );
    m_aAttributeMap.emplace( OUString( "CharHeight" ),        Any( static_cast<sal_Int16>(rFont.GetFontSize().Height()) ) );
    m_aAttributeMap.emplace( OUString( "CharScaleWidth" ),    Any( static_cast<sal_Int16>(rFont.GetFontSize().Width()) ) );
    m_aAttributeMap.emplace( OUString( "CharStrikeout" ),     Any( static_cast<sal_Int16>(rFont.GetStrikeout()) ) );
    m_aAttributeMap.emplace( OUString( "CharUnderline" ),     Any( static_cast<sal_Int16>(rFont.GetUnderline()) ) );
    m_aAttributeMap.emplace( OUString( "CharWeight" ),        Any( static_cast<float>(rFont.GetWeight()) ) );
    m_aAttributeMap.emplace( OUString( "CharPosture" ),       Any( static_cast<sal_Int16>(rFont.GetItalic()) ) );
}


std::vector< PropertyValue > CharacterAttributesHelper::GetCharacterAttributes()
{
    std::vector< PropertyValue > aValues;
    aValues.reserve( m_aAttributeMap.size() );

    for ( const auto& aIt : m_aAttributeMap)
    {
        aValues.emplace_back(aIt.first, sal_Int32(-1), aIt.second, PropertyState_DIRECT_VALUE);
    }

    return aValues;
}


Sequence< PropertyValue > CharacterAttributesHelper::GetCharacterAttributes( const css::uno::Sequence< OUString >& aRequestedAttributes )
{
    if ( !aRequestedAttributes.hasElements() )
        return comphelper::containerToSequence(GetCharacterAttributes());

    std::vector< PropertyValue > aValues;

    for ( const auto& aRequestedAttribute: aRequestedAttributes)
    {
        AttributeMap::iterator aFound = m_aAttributeMap.find( aRequestedAttribute );
        if ( aFound != m_aAttributeMap.end() )
            aValues.emplace_back(aFound->first, sal_Int32(-1), aFound->second, PropertyState_DIRECT_VALUE);
    }

    return comphelper::containerToSequence(aValues);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
