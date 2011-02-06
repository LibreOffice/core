/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
    DriverPooling::DriverPooling()
        :bEnabled(sal_False)
        ,nTimeoutSeconds(0)
    {
    }

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
    TYPEINIT1( DriverPoolingSettingsItem, SfxPoolItem )
    //--------------------------------------------------------------------
    DriverPoolingSettingsItem::DriverPoolingSettingsItem( sal_uInt16 _nId, const DriverPoolingSettings &_rSettings )
        :SfxPoolItem(_nId)
        ,m_aSettings(_rSettings)
    {
    }

    //--------------------------------------------------------------------
    int DriverPoolingSettingsItem::operator==( const SfxPoolItem& _rCompare ) const
    {
        const DriverPoolingSettingsItem* pItem = PTR_CAST(DriverPoolingSettingsItem, &_rCompare);
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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
