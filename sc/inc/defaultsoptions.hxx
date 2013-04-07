/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *       Albert Thuswaldner <albert.thuswaldner@gmail.com>
 * Portions created by the Initial Developer are Copyright (C) 2012 the
 * Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#ifndef SC_DEFAULTSOPT_HXX
#define SC_DEFAULTSOPT_HXX

#include <svl/poolitem.hxx>
#include <unotools/configitem.hxx>
#include "formula/grammar.hxx"
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

//==================================================================
// item for the dialog / options page
//==================================================================

class SC_DLLPUBLIC ScTpDefaultsItem : public SfxPoolItem
{
public:
    TYPEINFO();
    ScTpDefaultsItem( sal_uInt16 nWhich,
                   const ScDefaultsOptions& rOpt );
    ScTpDefaultsItem( const ScTpDefaultsItem& rItem );
    ~ScTpDefaultsItem();

    virtual String          GetValueText() const;
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;

    const ScDefaultsOptions& GetDefaultsOptions() const { return theOptions; }

private:
    ScDefaultsOptions theOptions;
};

//==================================================================
// config item
//==================================================================

class ScDefaultsCfg : public ScDefaultsOptions, public utl::ConfigItem
{
    com::sun::star::uno::Sequence<OUString> GetPropertyNames();
public:
    ScDefaultsCfg();

    void SetOptions( const ScDefaultsOptions& rNew );

    virtual void Commit();
    virtual void Notify( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
