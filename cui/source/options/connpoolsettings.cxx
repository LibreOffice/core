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

#include "connpoolsettings.hxx"


namespace offapp
{

    DriverPooling::DriverPooling( const OUString& _rName, bool _bEnabled, const sal_Int32 _nTimeout )
        :sName(_rName)
        ,bEnabled(_bEnabled)
        ,nTimeoutSeconds(_nTimeout)
    {
    }


    bool DriverPooling::operator == (const DriverPooling& _rR) const
    {
        return  (sName == _rR.sName)
            &&  (bEnabled == _rR.bEnabled)
            &&  (nTimeoutSeconds == _rR.nTimeoutSeconds);
    }

    DriverPoolingSettings::DriverPoolingSettings()
    {
    }

    TYPEINIT1( DriverPoolingSettingsItem, SfxPoolItem )

    DriverPoolingSettingsItem::DriverPoolingSettingsItem( sal_uInt16 _nId, const DriverPoolingSettings &_rSettings )
        :SfxPoolItem(_nId)
        ,m_aSettings(_rSettings)
    {
    }


    bool DriverPoolingSettingsItem::operator==( const SfxPoolItem& _rCompare ) const
    {
        const DriverPoolingSettingsItem* pItem = dynamic_cast<const DriverPoolingSettingsItem*>( &_rCompare );
        if (!pItem)
            return false;

        if (m_aSettings.size() != pItem->m_aSettings.size())
            return false;

        DriverPoolingSettings::const_iterator aOwn = m_aSettings.begin();
        DriverPoolingSettings::const_iterator aOwnEnd = m_aSettings.end();
        DriverPoolingSettings::const_iterator aForeign = pItem->m_aSettings.begin();
        while (aOwn < aOwnEnd)
        {
            if (*aOwn != *aForeign)
                return false;

            ++aForeign;
            ++aOwn;
        }

        return true;
    }


    SfxPoolItem* DriverPoolingSettingsItem::Clone( SfxItemPool * ) const
    {
        return new DriverPoolingSettingsItem(Which(), m_aSettings);
    }




}   // namespace offapp



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
