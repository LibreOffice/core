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

#include <vcl/configsettings.hxx>

#include <svdata.hxx>

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <o3tl/any.hxx>

using namespace utl;
using namespace vcl;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::container;

#define SETTINGS_CONFIGNODE "VCL/Settings"

SettingsConfigItem* SettingsConfigItem::get()
{
    ImplSVData* pSVData = ImplGetSVData();
    if( ! pSVData->mpSettingsConfigItem )
        pSVData->mpSettingsConfigItem = new SettingsConfigItem();
    return pSVData->mpSettingsConfigItem;
}

SettingsConfigItem::SettingsConfigItem()
 :  ConfigItem( SETTINGS_CONFIGNODE, ConfigItemMode::DelayedUpdate ),
    m_aSettings( 0 )
{
    getValues();
}

SettingsConfigItem::~SettingsConfigItem()
{
    assert(!IsModified()); // should have been committed
}

void SettingsConfigItem::ImplCommit()
{
    std::unordered_map< OUString, SmallOUStrMap >::const_iterator group;

    for (auto const& setting : m_aSettings)
    {
        OUString aKeyName( setting.first );
        /*bool bAdded =*/ AddNode( OUString(), aKeyName );
        Sequence< PropertyValue > aValues( group->second.size() );
        PropertyValue* pValues = aValues.getArray();
        int nIndex = 0;
        for (auto const& elem : group->second)
        {
            pValues[nIndex].Name    = aKeyName + "/" + elem.first;
            pValues[nIndex].Handle  = 0;
            pValues[nIndex].Value <<= elem.second;
            pValues[nIndex].State   = PropertyState_DIRECT_VALUE;
            nIndex++;
        }
        ReplaceSetProperties( aKeyName, aValues );
    }
}

void SettingsConfigItem::Notify( const Sequence< OUString >& )
{
    getValues();
}

void SettingsConfigItem::getValues()
{
    m_aSettings.clear();

    Sequence< OUString > aNames( GetNodeNames( OUString() ) );

    for( int j = 0; j < aNames.getLength(); j++ )
    {
#if OSL_DEBUG_LEVEL > 2
        SAL_INFO( "vcl", "found settings data for " << aNames.getConstArray()[j] );
#endif
        OUString aKeyName( aNames.getConstArray()[j] );
        Sequence< OUString > aKeys( GetNodeNames( aKeyName ) );
        Sequence< OUString > aSettingsKeys( aKeys.getLength() );
        const OUString* pFrom = aKeys.getConstArray();
        OUString* pTo = aSettingsKeys.getArray();
        for( int m = 0; m < aKeys.getLength(); m++ )
        {
            pTo[m] = aKeyName + "/" + pFrom[m];
        }
        Sequence< Any > aValues( GetProperties( aSettingsKeys ) );
        const Any* pValue = aValues.getConstArray();
        for( int i = 0; i < aValues.getLength(); i++, pValue++ )
        {
            if( auto pLine = o3tl::tryAccess<OUString>(*pValue) )
            {
                if( !pLine->isEmpty() )
                    m_aSettings[ aKeyName ][ pFrom[i] ] = *pLine;
#if OSL_DEBUG_LEVEL > 2
                SAL_INFO( "vcl", "   \"" << aKeys.getConstArray()[i] << "\"=\"" << *pLine << "\"" );
#endif
            }
        }
    }
}

OUString SettingsConfigItem::getValue( const OUString& rGroup, const OUString& rKey ) const
{
    std::unordered_map< OUString, SmallOUStrMap >::const_iterator group = m_aSettings.find( rGroup );
    if( group == m_aSettings.end() || group->second.find( rKey ) == group->second.end() )
    {
        return OUString();
    }
    return group->second.find(rKey)->second;
}

void SettingsConfigItem::setValue( const OUString& rGroup, const OUString& rKey, const OUString& rValue )
{
    bool bModified = m_aSettings[ rGroup ][ rKey ] != rValue;
    if( bModified )
    {
        m_aSettings[ rGroup ][ rKey ] = rValue;
        SetModified();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
