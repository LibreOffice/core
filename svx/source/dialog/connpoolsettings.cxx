/*************************************************************************
 *
 *  $RCSfile: connpoolsettings.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2005-01-21 16:30:10 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

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
    SfxPoolItem* DriverPoolingSettingsItem::Clone( SfxItemPool *pPool ) const
    {
        return new DriverPoolingSettingsItem(Which(), m_aSettings);
    }

    //--------------------------------------------------------------------

//........................................................................
}   // namespace offapp
//........................................................................


