/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <svl/poolitem.hxx>
#include <unotools/configitem.hxx>
#include "scdllapi.h"
#include "types.hxx"

class SC_DLLPUBLIC ScDefaultsOptions
{
private:
    SCTAB nInitTabCount;             // number of Tabs for new Spreadsheet doc
    OUString aInitTabPrefix;  // The Tab prefix name in new Spreadsheet doc
    bool     bJumboSheets;

public:
    ScDefaultsOptions();

    void SetDefaults();

    SCTAB GetInitTabCount() const           { return nInitTabCount; }
    void   SetInitTabCount( SCTAB nTabs) { nInitTabCount = nTabs; }
    void   SetInitTabPrefix(const OUString& aPrefix) { aInitTabPrefix = aPrefix; }
    const OUString& GetInitTabPrefix() const { return aInitTabPrefix; }
    bool   GetInitJumboSheets() const           { return bJumboSheets; }
    void   SetInitJumboSheets( bool b) { bJumboSheets = b; }

    bool                operator== ( const ScDefaultsOptions& rOpt ) const;

};

// item for the dialog / options page

class SC_DLLPUBLIC ScTpDefaultsItem final : public SfxPoolItem
{
public:
    ScTpDefaultsItem( const ScDefaultsOptions& rOpt );
    virtual ~ScTpDefaultsItem() override;

    ScTpDefaultsItem(ScTpDefaultsItem const &) = default;
    ScTpDefaultsItem(ScTpDefaultsItem &&) = default;
    ScTpDefaultsItem & operator =(ScTpDefaultsItem const &) = delete; // due to SfxPoolItem
    ScTpDefaultsItem & operator =(ScTpDefaultsItem &&) = delete; // due to SfxPoolItem

    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual ScTpDefaultsItem* Clone( SfxItemPool *pPool = nullptr ) const override;

    const ScDefaultsOptions& GetDefaultsOptions() const { return theOptions; }

private:
    ScDefaultsOptions theOptions;
};

// config item

class ScDefaultsCfg final : public ScDefaultsOptions, public utl::ConfigItem
{
private:
    static css::uno::Sequence<OUString> GetPropertyNames();
    virtual void ImplCommit() override;

public:
    ScDefaultsCfg();

    void SetOptions( const ScDefaultsOptions& rNew );

    virtual void Notify( const css::uno::Sequence< OUString >& aPropertyNames ) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
