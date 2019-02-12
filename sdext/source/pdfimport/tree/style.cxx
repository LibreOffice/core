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


#include "style.hxx"
#include <genericelements.hxx>
#include <xmlemitter.hxx>
#include <pdfiprocessor.hxx>
#include <rtl/ustrbuf.hxx>

#include <algorithm>
#include <string_view>

using namespace pdfi;


StyleContainer::StyleContainer() :
    m_nNextId( 1 )
{
}

sal_Int32 StyleContainer::impl_getStyleId( const Style& rStyle, bool bSubStyle )
{
    sal_Int32 nRet = -1;

    // construct HashedStyle to find or insert
    HashedStyle aSearchStyle;
    aSearchStyle.Name                   = rStyle.Name;
    aSearchStyle.Properties             = rStyle.Properties;
    aSearchStyle.Contents               = rStyle.Contents;
    aSearchStyle.ContainedElement       = rStyle.ContainedElement;
    for(Style* pSubStyle : rStyle.SubStyles)
        aSearchStyle.SubStyles.push_back( impl_getStyleId( *pSubStyle, true ) );

    std::unordered_map< HashedStyle, sal_Int32, StyleHash >::iterator it =
        m_aStyleToId.find( aSearchStyle );

    if( it != m_aStyleToId.end() )
    {
        nRet = it->second;
        RefCountedHashedStyle& rFound = m_aIdToStyle[ nRet ];
        // increase refcount on this style
        rFound.RefCount++;
        if( ! bSubStyle )
            rFound.style.IsSubStyle = false;
    }
    else
    {
        nRet = m_nNextId++;
        // create new style
        RefCountedHashedStyle& rNew = m_aIdToStyle[ nRet ];
        rNew.style = aSearchStyle;
        rNew.RefCount           = 1;
        rNew.style.IsSubStyle         = bSubStyle;
        // fill the style hash to find the id
        m_aStyleToId[ rNew.style ] = nRet;
    }
    return nRet;
}

sal_Int32 StyleContainer::getStandardStyleId( const OString& rName )
{
    PropertyMap aProps;
    aProps[ "style:family" ] = OStringToOUString( rName, RTL_TEXTENCODING_UTF8 );
    aProps[ "style:name" ] = "standard";

    Style aStyle( "style:style", aProps );
    return getStyleId( aStyle );
}

const PropertyMap* StyleContainer::getProperties( sal_Int32 nStyleId ) const
{
    std::unordered_map< sal_Int32, RefCountedHashedStyle >::const_iterator it =
        m_aIdToStyle.find( nStyleId );
    return it != m_aIdToStyle.end() ? &(it->second.style.Properties) : nullptr;
}

sal_Int32 StyleContainer::setProperties( sal_Int32 nStyleId, const PropertyMap& rNewProps )
{
    sal_Int32 nRet = -1;
    std::unordered_map< sal_Int32, RefCountedHashedStyle >::iterator it =
        m_aIdToStyle.find( nStyleId );
    if( it != m_aIdToStyle.end() )
    {
        if( it->second.RefCount == 1 )
        {
            nRet = it->first;
            // erase old hash to id mapping
            m_aStyleToId.erase( it->second.style );
            // change properties
            it->second.style.Properties = rNewProps;
            // fill in new hash to id mapping
            m_aStyleToId[ it->second.style ] = nRet;
        }
        else
        {
            // decrease refcount on old instance
            it->second.RefCount--;
            // acquire new HashedStyle
            HashedStyle aSearchStyle;
            aSearchStyle.Name                   = it->second.style.Name;
            aSearchStyle.Properties             = rNewProps;
            aSearchStyle.Contents               = it->second.style.Contents;
            aSearchStyle.ContainedElement       = it->second.style.ContainedElement;
            aSearchStyle.SubStyles              = it->second.style.SubStyles;
            aSearchStyle.IsSubStyle             = it->second.style.IsSubStyle;

            // find out whether this new style already exists
            std::unordered_map< HashedStyle, sal_Int32, StyleHash >::iterator new_it =
                m_aStyleToId.find( aSearchStyle );
            if( new_it != m_aStyleToId.end() )
            {
                nRet = new_it->second;
                m_aIdToStyle[ nRet ].RefCount++;
            }
            else
            {
                nRet = m_nNextId++;
                // create new style with new id
                RefCountedHashedStyle& rNew = m_aIdToStyle[ nRet ];
                rNew.style = aSearchStyle;
                rNew.RefCount = 1;
                // fill style to id hash
                m_aStyleToId[ aSearchStyle ] = nRet;
            }
        }
    }
    return nRet;
}

OUString StyleContainer::getStyleName( sal_Int32 nStyle ) const
{
    OUStringBuffer aRet( 64 );

    std::unordered_map< sal_Int32, RefCountedHashedStyle >::const_iterator style_it =
        m_aIdToStyle.find( nStyle );
    if( style_it != m_aIdToStyle.end() )
    {
        const HashedStyle& rStyle = style_it->second.style;

        PropertyMap::const_iterator name_it = rStyle.Properties.find( "style:name" );
        if( name_it != rStyle.Properties.end() )
            aRet.append( name_it->second );
        else
        {
            PropertyMap::const_iterator fam_it = rStyle.Properties.find( "style:family" );
            OUString aStyleName;
            if( fam_it != rStyle.Properties.end() )
            {
                aStyleName = fam_it->second;
            }
            else
                aStyleName = OStringToOUString( rStyle.Name, RTL_TEXTENCODING_ASCII_US );
            sal_Int32 nIndex = aStyleName.lastIndexOf( ':' );
            aRet.append( std::u16string_view(aStyleName).substr(nIndex+1) );
            aRet.append( nStyle );
        }
    }
    else
    {
        aRet.append( "invalid style id " );
        aRet.append( nStyle );
    }

    return aRet.makeStringAndClear();
}

void StyleContainer::impl_emitStyle( sal_Int32           nStyleId,
                                     EmitContext&        rContext,
                                     ElementTreeVisitor& rContainedElemVisitor )
{
    std::unordered_map< sal_Int32, RefCountedHashedStyle >::const_iterator it = m_aIdToStyle.find( nStyleId );
    if( it == m_aIdToStyle.end() )
        return;

    const HashedStyle& rStyle = it->second.style;
    PropertyMap aProps( rStyle.Properties );
    if( !rStyle.IsSubStyle )
        aProps[ "style:name" ] = getStyleName( nStyleId );
    if (rStyle.Name == "draw:stroke-dash")
        aProps[ "draw:name" ] = aProps[ "style:name" ];
    rContext.rEmitter.beginTag( rStyle.Name.getStr(), aProps );

    for(sal_Int32 nSubStyle : rStyle.SubStyles)
        impl_emitStyle( nSubStyle, rContext, rContainedElemVisitor );
    if( !rStyle.Contents.isEmpty() )
        rContext.rEmitter.write( rStyle.Contents );
    if( rStyle.ContainedElement )
        rStyle.ContainedElement->visitedBy( rContainedElemVisitor,
                                            std::list<std::unique_ptr<Element>>::iterator() );
    rContext.rEmitter.endTag( rStyle.Name.getStr() );
}

void StyleContainer::emit( EmitContext&        rContext,
                           ElementTreeVisitor& rContainedElemVisitor )
{
    std::vector< sal_Int32 > aMasterPageSection, aAutomaticStyleSection, aOfficeStyleSection;
    for( const auto& rEntry : m_aIdToStyle )
    {
        if( ! rEntry.second.style.IsSubStyle )
        {
            if( rEntry.second.style.Name == "style:master-page" )
                aMasterPageSection.push_back( rEntry.first );
            else if( getStyleName( rEntry.first ) == "standard" )
                aOfficeStyleSection.push_back( rEntry.first );
            else
                aAutomaticStyleSection.push_back( rEntry.first );
        }
    }

    if( ! aMasterPageSection.empty() )
        std::stable_sort( aMasterPageSection.begin(), aMasterPageSection.end(), StyleIdNameSort(&m_aIdToStyle) );
    if( ! aAutomaticStyleSection.empty() )
        std::stable_sort( aAutomaticStyleSection.begin(), aAutomaticStyleSection.end(), StyleIdNameSort(&m_aIdToStyle) );
    if( ! aOfficeStyleSection.empty() )
        std::stable_sort( aOfficeStyleSection.begin(), aOfficeStyleSection.end(), StyleIdNameSort(&m_aIdToStyle) );

    int n = 0, nElements = 0;
    rContext.rEmitter.beginTag( "office:styles", PropertyMap() );
    for( n = 0, nElements = aOfficeStyleSection.size(); n < nElements; n++ )
        impl_emitStyle( aOfficeStyleSection[n], rContext, rContainedElemVisitor );
    rContext.rEmitter.endTag( "office:styles" );
    rContext.rEmitter.beginTag( "office:automatic-styles", PropertyMap() );
    for( n = 0, nElements = aAutomaticStyleSection.size(); n < nElements; n++ )
        impl_emitStyle( aAutomaticStyleSection[n], rContext, rContainedElemVisitor );
    rContext.rEmitter.endTag( "office:automatic-styles" );
    rContext.rEmitter.beginTag( "office:master-styles", PropertyMap() );
    for( n = 0, nElements = aMasterPageSection.size(); n < nElements; n++ )
        impl_emitStyle( aMasterPageSection[n], rContext, rContainedElemVisitor );
    rContext.rEmitter.endTag( "office:master-styles" );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
