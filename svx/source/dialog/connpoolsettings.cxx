/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: connpoolsettings.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 04:13:13 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#ifdef SVX_DLLIMPLEMENTATION
#undef SVX_DLLIMPLEMENTATION
#endif

#ifndef _OFFAPP_CONNPOOLSETTINGS_HXX_
#include "connpoolsettings.hxx"
#endif

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
    DriverPoolingSettingsItem::DriverPoolingSettingsItem( sal_uInt16 _nId, const DriverPoolingSettings _rSettings )
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


