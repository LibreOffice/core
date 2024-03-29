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

#include <configsettings.hxx>

#include <svdata.hxx>

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <o3tl/any.hxx>
#include <sal/log.hxx>

using namespace utl;
using namespace vcl;
using namespace com::sun::star::uno;
using namespace com::sun::star::beans;
using namespace com::sun::star::container;

constexpr OUStringLiteral SETTINGS_CONFIGNODE = u"VCL/Settings";

SettingsConfigItem* SettingsConfigItem::get()
{
    ImplSVData* pSVData = ImplGetSVData();
    if( ! pSVData->mpSettingsConfigItem )
        pSVData->mpSettingsConfigItem.reset( new SettingsConfigItem() );
    return pSVData->mpSettingsConfigItem.get();
}

SettingsConfigItem::SettingsConfigItem()
 :  ConfigItem( SETTINGS_CONFIGNODE, ConfigItemMode::NONE ),
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
    for (auto const& setting : m_aSettings)
    {
        OUString aKeyName( setting.first );
        /*bool bAdded =*/ AddNode( OUString(), aKeyName );
        Sequence< PropertyValue > aValues( setting.second.size() );
        PropertyValue* pValues = aValues.getArray();
        int nIndex = 0;
        for (auto const& elem : setting.second)
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

    const Sequence< OUString > aNames( GetNodeNames( OUString() ) );

    for( const auto& aKeyName : aNames )
    {
#if OSL_DEBUG_LEVEL > 2
        SAL_INFO( "vcl", "found settings data for " << aKeyName );
#endif
        const Sequence< OUString > aKeys( GetNodeNames( aKeyName ) );
        Sequence< OUString > aSettingsKeys( aKeys.getLength() );
        std::transform(aKeys.begin(), aKeys.end(), aSettingsKeys.getArray(),
            [&aKeyName](const OUString& rKey) -> OUString { return aKeyName + "/" + rKey; });
        const Sequence< Any > aValues( GetProperties( aSettingsKeys ) );
        for( int i = 0; i < aValues.getLength(); i++ )
        {
            if( auto pLine = o3tl::tryAccess<OUString>(aValues[i]) )
            {
                if( !pLine->isEmpty() )
                    m_aSettings[ aKeyName ][ aKeys[i] ] = *pLine;
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
