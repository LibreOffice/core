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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#include <vcl/configsettings.hxx>

#include <svdata.hxx>

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>

using namespace rtl;
using namespace utl;
using namespace vcl;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::container;

#define SETTINGS_CONFIGNODE "VCL/Settings"

/*
 *  SettingsConfigItem::get
 */

SettingsConfigItem* SettingsConfigItem::get()
{
    ImplSVData* pSVData = ImplGetSVData();
    if( ! pSVData->mpSettingsConfigItem )
        pSVData->mpSettingsConfigItem = new SettingsConfigItem();
    return pSVData->mpSettingsConfigItem;
}

/*
 *  SettignsConfigItem constructor
 */

SettingsConfigItem::SettingsConfigItem()
        :
        ConfigItem( OUString( RTL_CONSTASCII_USTRINGPARAM( SETTINGS_CONFIGNODE ) ),
                    CONFIG_MODE_DELAYED_UPDATE ),
    m_aSettings( 0 )
{
    getValues();
}

/*
 *  SettingsConfigItem destructor
 */

SettingsConfigItem::~SettingsConfigItem()
{
    if( IsModified() )
        Commit();
}

/*
 *  SettingsConfigItem::Commit
 */

void SettingsConfigItem::Commit()
{
    if( ! IsValidConfigMgr() )
        return;

    std::hash_map< OUString, SmallOUStrMap, rtl::OUStringHash >::const_iterator group;

    for( group = m_aSettings.begin(); group != m_aSettings.end(); ++group )
    {
        String aKeyName( group->first );
        /*sal_Bool bAdded =*/ AddNode( OUString(), aKeyName );
        Sequence< PropertyValue > aValues( group->second.size() );
        PropertyValue* pValues = aValues.getArray();
        int nIndex = 0;
        SmallOUStrMap::const_iterator it;
        for( it = group->second.begin(); it != group->second.end(); ++it )
        {
            String aName( aKeyName );
            aName.Append( '/' );
            aName.Append( String( it->first ) );
            pValues[nIndex].Name    = aName;
            pValues[nIndex].Handle  = 0;
            pValues[nIndex].Value <<= it->second;
            pValues[nIndex].State   = PropertyState_DIRECT_VALUE;
            nIndex++;
        }
        ReplaceSetProperties( aKeyName, aValues );
    }
}

/*
 *  SettingsConfigItem::Notify
 */

void SettingsConfigItem::Notify( const Sequence< OUString >& )
{
    getValues();
}

/*
 *  SettingsConfigItem::getValues
 */
void SettingsConfigItem::getValues()
{
    if( ! IsValidConfigMgr() )
        return;

    m_aSettings.clear();

    Sequence< OUString > aNames( GetNodeNames( OUString() ) );
    m_aSettings.rehash( aNames.getLength() );

    for( int j = 0; j < aNames.getLength(); j++ )
    {
#if OSL_DEBUG_LEVEL > 2
        fprintf( stderr, "found settings data for \"%s\"\n",
                 OUStringToOString( aNames.getConstArray()[j], RTL_TEXTENCODING_ASCII_US ).getStr()
                 );
#endif
        String aKeyName( aNames.getConstArray()[j] );
        Sequence< OUString > aKeys( GetNodeNames( aKeyName ) );
        Sequence< OUString > aSettingsKeys( aKeys.getLength() );
        const OUString* pFrom = aKeys.getConstArray();
        OUString* pTo = aSettingsKeys.getArray();
        for( int m = 0; m < aKeys.getLength(); m++ )
        {
            String aName( aKeyName );
            aName.Append( '/' );
            aName.Append( String( pFrom[m] ) );
            pTo[m] = aName;
        }
        Sequence< Any > aValues( GetProperties( aSettingsKeys ) );
        const Any* pValue = aValues.getConstArray();
        for( int i = 0; i < aValues.getLength(); i++, pValue++ )
        {
            if( pValue->getValueTypeClass() == TypeClass_STRING )
            {
                const OUString* pLine = (const OUString*)pValue->getValue();
                if( pLine->getLength() )
                    m_aSettings[ aKeyName ][ pFrom[i] ] = *pLine;
#if OSL_DEBUG_LEVEL > 2
                fprintf( stderr, "   \"%s\"=\"%.30s\"\n",
                         OUStringToOString( aKeys.getConstArray()[i], RTL_TEXTENCODING_ASCII_US ).getStr(),
                         OUStringToOString( *pLine, RTL_TEXTENCODING_ASCII_US ).getStr()
                         );
#endif
            }
        }
    }
}

/*
 *  SettingsConfigItem::getDefaultFont
 */

const OUString& SettingsConfigItem::getValue( const OUString& rGroup, const OUString& rKey ) const
{
    ::std::hash_map< OUString, SmallOUStrMap, rtl::OUStringHash >::const_iterator group = m_aSettings.find( rGroup );
    if( group == m_aSettings.end() || group->second.find( rKey ) == group->second.end() )
    {
        static OUString aEmpty;
        return aEmpty;
    }
    return group->second.find(rKey)->second;
}

/*
 *  SettingsConfigItem::setDefaultFont
 */

void SettingsConfigItem::setValue( const OUString& rGroup, const OUString& rKey, const OUString& rValue )
{
    bool bModified = m_aSettings[ rGroup ][ rKey ] != rValue;
    if( bModified )
    {
        m_aSettings[ rGroup ][ rKey ] = rValue;
        SetModified();
    }
}

