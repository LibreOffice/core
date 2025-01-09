/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/uno/Sequence.hxx>
#include <osl/diagnose.h>

#include <defaultsoptions.hxx>
#include <globstr.hrc>
#include <scresid.hxx>
#include <sc.hrc>
#include <utility>

using namespace utl;
using namespace com::sun::star::uno;


ScDefaultsOptions::ScDefaultsOptions()
{
    SetDefaults();
}

void ScDefaultsOptions::SetDefaults()
{
    nInitTabCount  = 1;
    aInitTabPrefix = ScResId(STR_TABLE_DEF); // Default Prefix "Sheet"
    bJumboSheets = false;
}

bool ScDefaultsOptions::operator==( const ScDefaultsOptions& rOpt ) const
{
    return rOpt.nInitTabCount  == nInitTabCount
        && rOpt.aInitTabPrefix == aInitTabPrefix
        && rOpt.bJumboSheets == bJumboSheets;
}

ScTpDefaultsItem::ScTpDefaultsItem( ScDefaultsOptions aOpt ) :
    SfxPoolItem ( SID_SCDEFAULTSOPTIONS ),
    theOptions  (std::move( aOpt ))
{
}

ScTpDefaultsItem::~ScTpDefaultsItem()
{
}

bool ScTpDefaultsItem::operator==( const SfxPoolItem& rItem ) const
{
    assert(SfxPoolItem::operator==(rItem));

    const ScTpDefaultsItem& rPItem = static_cast<const ScTpDefaultsItem&>(rItem);
    return ( theOptions == rPItem.theOptions );
}

ScTpDefaultsItem* ScTpDefaultsItem::Clone( SfxItemPool * ) const
{
    return new ScTpDefaultsItem( *this );
}

constexpr OUStringLiteral CFGPATH_FORMULA = u"Office.Calc/Defaults";

#define SCDEFAULTSOPT_TAB_COUNT  0
#define SCDEFAULTSOPT_TAB_PREFIX 1
#define SCDEFAULTSOPT_JUMBO_SHEETS 2

Sequence<OUString> ScDefaultsCfg::GetPropertyNames()
{
    return {u"Sheet/SheetCount"_ustr,   // SCDEFAULTSOPT_TAB_COUNT
            u"Sheet/SheetPrefix"_ustr,  // SCDEFAULTSOPT_TAB_PREFIX
            u"Sheet/JumboSheets"_ustr};  // SCDEFAULTSOPT_JUMBO_SHEETS

}

ScDefaultsCfg::ScDefaultsCfg() :
    ConfigItem( CFGPATH_FORMULA )
{
    OUString aPrefix;

    Sequence<OUString> aNames = GetPropertyNames();
    Sequence<Any> aValues = GetProperties(aNames);
    const Any* pValues = aValues.getConstArray();
    OSL_ENSURE(aValues.getLength() == aNames.getLength(), "GetProperties failed");
    if(aValues.getLength() != aNames.getLength())
        return;

    sal_Int32 nIntVal = 0;
    for(int nProp = 0; nProp < aNames.getLength(); nProp++)
    {
        if(pValues[nProp].hasValue())
        {
            switch (nProp)
            {
            case SCDEFAULTSOPT_TAB_COUNT:
                if (pValues[nProp] >>= nIntVal)
                    SetInitTabCount( static_cast<SCTAB>(nIntVal) );
                break;
            case SCDEFAULTSOPT_TAB_PREFIX:
                if (pValues[nProp] >>= aPrefix)
                    SetInitTabPrefix(aPrefix);
                break;
            case SCDEFAULTSOPT_JUMBO_SHEETS:
#if HAVE_FEATURE_JUMBO_SHEETS
            {
                bool bValue;
                if (pValues[nProp] >>= bValue)
                    SetInitJumboSheets(bValue);
            }
#endif
                break;
            }
        }
    }
}

void ScDefaultsCfg::ImplCommit()
{
    Sequence<OUString> aNames = GetPropertyNames();
    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

    for (int nProp = 0; nProp < aNames.getLength(); ++nProp)
    {
        switch(nProp)
        {
        case SCDEFAULTSOPT_TAB_COUNT:
            pValues[nProp] <<= static_cast<sal_Int32>(GetInitTabCount());
        break;
        case SCDEFAULTSOPT_TAB_PREFIX:
            pValues[nProp] <<= GetInitTabPrefix();
        break;
        case SCDEFAULTSOPT_JUMBO_SHEETS:
            pValues[nProp] <<= GetInitJumboSheets();
        break;
        }
    }
    PutProperties(aNames, aValues);
}

void ScDefaultsCfg::SetOptions( const ScDefaultsOptions& rNew )
{
    *static_cast<ScDefaultsOptions*>(this) = rNew;
    SetModified();
}

void ScDefaultsCfg::Notify( const css::uno::Sequence< OUString >& ) {}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
