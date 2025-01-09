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

#pragma once

#include <sal/config.h>

#include <vector>

#include <rtl/ustring.hxx>
#include <svl/poolitem.hxx>


namespace offapp
{

    struct DriverPooling
    {
        OUString            sName;
        bool                bEnabled;
        sal_Int32           nTimeoutSeconds;

        explicit DriverPooling( OUString  _aName );

        bool operator == (const DriverPooling& _rR) const;
        bool operator != (const DriverPooling& _rR) const { return !operator ==(_rR); }
    };

    class DriverPoolingSettings final
    {
        typedef std::vector<DriverPooling> DriverSettings;
        DriverSettings      m_aDrivers;

    public:
        typedef DriverSettings::const_iterator const_iterator;
        typedef DriverSettings::iterator iterator;

        DriverPoolingSettings();

        size_t size() const { return m_aDrivers.size(); }
        DriverPooling& operator[]( size_t nPos ) { return m_aDrivers[nPos]; }
        bool empty() const { return m_aDrivers.empty(); }

        const_iterator  begin() const   { return m_aDrivers.begin(); }
        const_iterator  end() const     { return m_aDrivers.end(); }

        iterator        begin()     { return m_aDrivers.begin(); }
        iterator        end()       { return m_aDrivers.end(); }

        void push_back(const DriverPooling& _rElement) { m_aDrivers.push_back(_rElement); }
    };

    class DriverPoolingSettingsItem final : public SfxPoolItem
    {
        DriverPoolingSettings   m_aSettings;

    public:
        DECLARE_ITEM_TYPE_FUNCTION(DriverPoolingSettingsItem)
        DriverPoolingSettingsItem( sal_uInt16 _nId, DriverPoolingSettings _aSettings );

        virtual bool             operator==( const SfxPoolItem& ) const override;
        virtual DriverPoolingSettingsItem* Clone( SfxItemPool *pPool = nullptr ) const override;

        const DriverPoolingSettings& getSettings() const    { return m_aSettings; }
    };


}   // namespace offapp

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
