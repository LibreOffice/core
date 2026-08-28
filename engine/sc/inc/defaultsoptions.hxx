/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <svl/poolitem.hxx>
#include <unotools/configitem.hxx>
#include <config_features.h>
#include "scdllapi.h"
#include "types.hxx"

class SC_DLLPUBLIC ScDefaultsOptions
{
private:
    SCTAB nInitTabCount;             // number of Tabs for new Spreadsheet doc
    OUString aInitTabPrefix;  // The Tab prefix name in new Spreadsheet doc
    bool     bJumboSheets;
    bool bInitTabPrefixChanged;

public:
    ScDefaultsOptions();

    void SetDefaults();

    SCTAB GetInitTabCount() const           { return nInitTabCount; }
    void   SetInitTabCount( SCTAB nTabs) { nInitTabCount = nTabs; }
    void SetInitTabPrefix(const OUString& aPrefix);
    OUString GetInitTabPrefix() const;
    bool   GetInitJumboSheets() const           { return bJumboSheets; }
#if HAVE_FEATURE_JUMBO_SHEETS
    void   SetInitJumboSheets( bool b) { bJumboSheets = b; }
#endif
    bool                operator== ( const ScDefaultsOptions& rOpt ) const;

};

// config item

class ScDefaultsCfg final : public ScDefaultsOptions, public utl::ConfigItem
{
private:
    static cpo::uno::Sequence<OUString> GetPropertyNames();
    virtual void ImplCommit() override;

public:
    ScDefaultsCfg();

    void SetOptions( const ScDefaultsOptions& rNew );

    virtual void Notify( const cpo::uno::Sequence< OUString >& aPropertyNames ) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
