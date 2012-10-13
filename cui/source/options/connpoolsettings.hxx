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
