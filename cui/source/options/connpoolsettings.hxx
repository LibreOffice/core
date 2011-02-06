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

#ifndef _OFFAPP_CONNPOOLSETTINGS_HXX_
#define _OFFAPP_CONNPOOLSETTINGS_HXX_

#include <tools/string.hxx>
#include <comphelper/stl_types.hxx>
#include <svl/poolitem.hxx>

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

        DriverPoolingSettingsItem( sal_uInt16 _nId, const DriverPoolingSettings &_rSettings );

        virtual int              operator==( const SfxPoolItem& ) const;
        virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const;

        const DriverPoolingSettings& getSettings() const    { return m_aSettings; }
    };

//........................................................................
}   // namespace offapp
//........................................................................

#endif // _OFFAPP_CONNPOOLSETTINGS_HXX_


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
