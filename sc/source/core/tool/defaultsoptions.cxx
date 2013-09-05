/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#include "defaultsoptions.hxx"
#include "miscuno.hxx"
#include "global.hxx"
#include "globstr.hrc"

using namespace utl;
using namespace com::sun::star::uno;

TYPEINIT1(ScTpDefaultsItem, SfxPoolItem);

ScDefaultsOptions::ScDefaultsOptions()
{
    SetDefaults();
}

ScDefaultsOptions::ScDefaultsOptions( const ScDefaultsOptions& rCpy ) :
    nInitTabCount( rCpy.nInitTabCount ),
    aInitTabPrefix( rCpy.aInitTabPrefix )
{
}

ScDefaultsOptions::~ScDefaultsOptions()
{
}

void ScDefaultsOptions::SetDefaults()
{
    nInitTabCount  = 1;
    aInitTabPrefix = ScGlobal::GetRscString(STR_TABLE_DEF); // Default Prefix "Sheet"
}

ScDefaultsOptions& ScDefaultsOptions::operator=( const ScDefaultsOptions& rCpy )
{
    nInitTabCount  = rCpy.nInitTabCount;
    aInitTabPrefix = rCpy.aInitTabPrefix;

    return *this;
}

bool ScDefaultsOptions::operator==( const ScDefaultsOptions& rOpt ) const
{
    return rOpt.nInitTabCount  == nInitTabCount
        && rOpt.aInitTabPrefix == aInitTabPrefix;
}

bool ScDefaultsOptions::operator!=( const ScDefaultsOptions& rOpt ) const
{
    return !(operator==(rOpt));
}

ScTpDefaultsItem::ScTpDefaultsItem( sal_uInt16 nWhichP, const ScDefaultsOptions& rOpt ) :
    SfxPoolItem ( nWhichP ),
    theOptions  ( rOpt )
{
}

ScTpDefaultsItem::ScTpDefaultsItem( const ScTpDefaultsItem& rItem ) :
    SfxPoolItem ( rItem ),
    theOptions  ( rItem.theOptions )
{
}

ScTpDefaultsItem::~ScTpDefaultsItem()
{
}

OUString ScTpDefaultsItem::GetValueText() const
{
    return OUString("ScTpDefaultsItem");
}

int ScTpDefaultsItem::operator==( const SfxPoolItem& rItem ) const
{
    OSL_ENSURE( SfxPoolItem::operator==( rItem ), "unequal Which or Type" );

    const ScTpDefaultsItem& rPItem = (const ScTpDefaultsItem&)rItem;
    return ( theOptions == rPItem.theOptions );
}

SfxPoolItem* ScTpDefaultsItem::Clone( SfxItemPool * ) const
{
    return new ScTpDefaultsItem( *this );
}

#define CFGPATH_FORMULA          "Office.Calc/Defaults"

#define SCDEFAULTSOPT_TAB_COUNT  0
#define SCDEFAULTSOPT_TAB_PREFIX 1
#define SCDEFAULTSOPT_COUNT      2

Sequence<OUString> ScDefaultsCfg::GetPropertyNames()
{
    static const char* aPropNames[] =
    {
        "Sheet/SheetCount", // SCDEFAULTSOPT_TAB_COUNT
        "Sheet/SheetPrefix" // SCDEFAULTSOPT_TAB_PREFIX
    };
    Sequence<OUString> aNames(SCDEFAULTSOPT_COUNT);
    OUString* pNames = aNames.getArray();
    for (int i = 0; i < SCDEFAULTSOPT_COUNT; ++i)
        pNames[i] = OUString::createFromAscii(aPropNames[i]);

    return aNames;
}

ScDefaultsCfg::ScDefaultsCfg() :
    ConfigItem( OUString( CFGPATH_FORMULA ) )
{
    OUString aPrefix;

    Sequence<OUString> aNames = GetPropertyNames();
    Sequence<Any> aValues = GetProperties(aNames);
    const Any* pValues = aValues.getConstArray();
    OSL_ENSURE(aValues.getLength() == aNames.getLength(), "GetProperties failed");
    if(aValues.getLength() == aNames.getLength())
    {
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
                }
            }
        }
    }
}

void ScDefaultsCfg::Commit()
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
        }
    }
    PutProperties(aNames, aValues);
}

void ScDefaultsCfg::SetOptions( const ScDefaultsOptions& rNew )
{
    *(ScDefaultsOptions*)this = rNew;
    SetModified();
}

void ScDefaultsCfg::Notify( const ::com::sun::star::uno::Sequence< OUString >& ) {}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
