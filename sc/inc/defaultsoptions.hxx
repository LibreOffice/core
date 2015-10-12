/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_INC_DEFAULTSOPTIONS_HXX
#define INCLUDED_SC_INC_DEFAULTSOPTIONS_HXX

#include <svl/poolitem.hxx>
#include <unotools/configitem.hxx>
#include <formula/grammar.hxx>
#include "scdllapi.h"
#include "global.hxx"

class SC_DLLPUBLIC ScDefaultsOptions
{
private:
    SCTAB nInitTabCount;             // number of Tabs for new Spreadsheet doc
    OUString aInitTabPrefix;  // The Tab prefix name in new Spreadsheet doc

public:
    ScDefaultsOptions();
    ScDefaultsOptions( const ScDefaultsOptions& rCpy );
    ~ScDefaultsOptions();

    void SetDefaults();

    SCTAB GetInitTabCount() const           { return nInitTabCount; }
    void   SetInitTabCount( SCTAB nTabs) { nInitTabCount = nTabs; }
    void   SetInitTabPrefix(const OUString& aPrefix) { aInitTabPrefix = aPrefix; }
    OUString GetInitTabPrefix() const { return aInitTabPrefix; }

    ScDefaultsOptions&  operator=  ( const ScDefaultsOptions& rCpy );
    bool                operator== ( const ScDefaultsOptions& rOpt ) const;
    bool                operator!= ( const ScDefaultsOptions& rOpt ) const;

};

// item for the dialog / options page

class SC_DLLPUBLIC ScTpDefaultsItem : public SfxPoolItem
{
public:
    TYPEINFO_OVERRIDE();
    ScTpDefaultsItem( sal_uInt16 nWhich,
                   const ScDefaultsOptions& rOpt );
    ScTpDefaultsItem( const ScTpDefaultsItem& rItem );
    virtual ~ScTpDefaultsItem();

    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const override;

    const ScDefaultsOptions& GetDefaultsOptions() const { return theOptions; }

private:
    ScDefaultsOptions theOptions;
};

// config item

class ScDefaultsCfg : public ScDefaultsOptions, public utl::ConfigItem
{
private:
    static com::sun::star::uno::Sequence<OUString> GetPropertyNames();
    virtual void ImplCommit() override;

public:
    ScDefaultsCfg();

    void SetOptions( const ScDefaultsOptions& rNew );

    virtual void Notify( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames ) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
