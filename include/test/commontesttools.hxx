/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <comphelper/configuration.hxx>

// RAII-style management of officecfg settings. Sets the setting specified as template parameter,
// to a value passed in constructor. Stores the previous value of the setting, and restores it in
// destructor.
template <class OfficeCfg> class ScopedConfigValue
{
    using ConfigValue_t = decltype(OfficeCfg::get());

public:
    ScopedConfigValue(const ConfigValue_t& rNewVal)
        : maOrigValue(OfficeCfg::get())
    {
        auto pChanges(comphelper::ConfigurationChanges::create());
        OfficeCfg::set(rNewVal, pChanges);
        pChanges->commit();
    }
    ScopedConfigValue(const ScopedConfigValue&) = delete;
    ~ScopedConfigValue()
    {
        auto pChanges(comphelper::ConfigurationChanges::create());
        OfficeCfg::set(maOrigValue, pChanges);
        pChanges->commit();
    }

private:
    ConfigValue_t maOrigValue;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
