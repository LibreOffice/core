/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: connpoolsettings.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 20:46:53 $
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

#ifndef _OFFAPP_CONNPOOLSETTINGS_HXX_
#define _OFFAPP_CONNPOOLSETTINGS_HXX_

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif
#ifndef _SFXPOOLITEM_HXX
#include <svtools/poolitem.hxx>
#endif

//........................................................................
namespace offapp
{
//........................................................................

    //====================================================================
    //= DriverPooling
    //====================================================================
    struct DriverPooling
    {
        String              sName;
        sal_Bool            bEnabled;
        sal_Int32           nTimeoutSeconds;

        DriverPooling();
        DriverPooling( const String& _rName, sal_Bool _bEnabled, const sal_Int32 _nTimeout );

        sal_Bool operator == (const DriverPooling& _rR) const;
        sal_Bool operator != (const DriverPooling& _rR) const { return !operator ==(_rR); }
    };

    //====================================================================
    //= DriverPoolingSettings
    //====================================================================
    class DriverPoolingSettings
    {
    protected:
        DECLARE_STL_VECTOR( DriverPooling, DriverSettings );
        DriverSettings      m_aDrivers;

    public:
        typedef ConstDriverSettingsIterator const_iterator;
        typedef DriverSettingsIterator      iterator;

    public:
        DriverPoolingSettings();

        sal_Int32 size() const { return m_aDrivers.size(); }

        const_iterator  begin() const   { return m_aDrivers.begin(); }
        const_iterator  end() const     { return m_aDrivers.end(); }

        iterator        begin()     { return m_aDrivers.begin(); }
        iterator        end()       { return m_aDrivers.end(); }

        void push_back(const DriverPooling& _rElement) { m_aDrivers.push_back(_rElement); }
    };

    //====================================================================
    //= DriverPoolingSettingsItem
    //====================================================================
    class DriverPoolingSettingsItem : public SfxPoolItem
    {
    protected:
        DriverPoolingSettings   m_aSettings;

    public:
        TYPEINFO();

        DriverPoolingSettingsItem( sal_uInt16 _nId, const DriverPoolingSettings _rSettings );

        virtual int              operator==( const SfxPoolItem& ) const;
        virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const;

        const DriverPoolingSettings& getSettings() const    { return m_aSettings; }
    };

//........................................................................
}   // namespace offapp
//........................................................................

#endif // _OFFAPP_CONNPOOLSETTINGS_HXX_


