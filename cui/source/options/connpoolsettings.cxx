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

#include <utility>

#include "connpoolsettings.hxx"


namespace offapp
{

    DriverPooling::DriverPooling( OUString _aName )
        :sName(std::move(_aName))
        ,bEnabled(false)
        ,nTimeoutSeconds(120)
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


    DriverPoolingSettingsItem::DriverPoolingSettingsItem( sal_uInt16 _nId, DriverPoolingSettings _aSettings )
        :SfxPoolItem(_nId )
        ,m_aSettings(std::move(_aSettings))
    {
    }


    bool DriverPoolingSettingsItem::operator==( const SfxPoolItem& _rCompare ) const
    {
        assert(SfxPoolItem::operator==(_rCompare));
        const DriverPoolingSettingsItem* pItem = static_cast<const DriverPoolingSettingsItem*>( &_rCompare );

        if (m_aSettings.size() != pItem->m_aSettings.size())
            return false;

        DriverPoolingSettings::const_iterator aForeign = pItem->m_aSettings.begin();
        for (auto const& ownSetting : m_aSettings)
        {
            if (ownSetting != *aForeign)
                return false;

            ++aForeign;
        }

        return true;
    }

    DriverPoolingSettingsItem* DriverPoolingSettingsItem::Clone( SfxItemPool * ) const
    {
        return new DriverPoolingSettingsItem(*this);
    }


}   // namespace offapp


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
