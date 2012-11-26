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
#include "precompiled_cui.hxx"

#include "connpoolsettings.hxx"

//........................................................................
namespace offapp
{
//........................................................................

    //====================================================================
    //= DriverPooling
    //====================================================================
    //--------------------------------------------------------------------
    DriverPooling::DriverPooling( const String& _rName, sal_Bool _bEnabled, const sal_Int32 _nTimeout )
        :sName(_rName)
        ,bEnabled(_bEnabled)
        ,nTimeoutSeconds(_nTimeout)
    {
    }

    //--------------------------------------------------------------------
    sal_Bool DriverPooling::operator == (const DriverPooling& _rR) const
    {
        return  (sName == _rR.sName)
            &&  (bEnabled == _rR.bEnabled)
            &&  (nTimeoutSeconds == _rR.nTimeoutSeconds);
    }

    //====================================================================
    //= DriverPoolingSettings
    //====================================================================
    //--------------------------------------------------------------------
    DriverPoolingSettings::DriverPoolingSettings()
    {
    }

    //====================================================================
    //= DriverPoolingSettingsItem
    //====================================================================
    DriverPoolingSettingsItem::DriverPoolingSettingsItem( sal_uInt16 _nId, const DriverPoolingSettings _rSettings )
        :SfxPoolItem(_nId)
        ,m_aSettings(_rSettings)
    {
    }

    //--------------------------------------------------------------------
    int DriverPoolingSettingsItem::operator==( const SfxPoolItem& _rCompare ) const
    {
        const DriverPoolingSettingsItem* pItem = dynamic_cast< const DriverPoolingSettingsItem* >( &_rCompare);
        if (!pItem)
            return sal_False;

        if (m_aSettings.size() != pItem->m_aSettings.size())
            return sal_False;

        DriverPoolingSettings::const_iterator aOwn = m_aSettings.begin();
        DriverPoolingSettings::const_iterator aOwnEnd = m_aSettings.end();
        DriverPoolingSettings::const_iterator aForeign = pItem->m_aSettings.begin();
        while (aOwn < aOwnEnd)
        {
            if (*aOwn != *aForeign)
                return sal_False;

            ++aForeign;
            ++aOwn;
        }

        return sal_True;
    }

    //--------------------------------------------------------------------
    SfxPoolItem* DriverPoolingSettingsItem::Clone( SfxItemPool * ) const
    {
        return new DriverPoolingSettingsItem(Which(), m_aSettings);
    }

    //--------------------------------------------------------------------

//........................................................................
}   // namespace offapp
//........................................................................


