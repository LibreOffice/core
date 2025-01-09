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

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <osl/diagnose.h>

#include <printopt.hxx>
#include <sc.hrc>

using namespace utl;
using namespace com::sun::star::uno;


ScPrintOptions::ScPrintOptions()
{
    SetDefaults();
}

void ScPrintOptions::SetDefaults()
{
    bSkipEmpty = true;
    bAllSheets = false;
    bForceBreaks = false;
}

bool ScPrintOptions::operator==( const ScPrintOptions& rOpt ) const
{
    return bSkipEmpty == rOpt.bSkipEmpty
        && bAllSheets == rOpt.bAllSheets
        && bForceBreaks == rOpt.bForceBreaks;
}

ScTpPrintItem::ScTpPrintItem( const ScPrintOptions& rOpt ) :
    SfxPoolItem ( SID_SCPRINTOPTIONS ),
    theOptions  ( rOpt )
{
}

ScTpPrintItem::~ScTpPrintItem()
{
}

bool ScTpPrintItem::operator==( const SfxPoolItem& rItem ) const
{
    assert(SfxPoolItem::operator==(rItem));

    const ScTpPrintItem& rPItem = static_cast<const ScTpPrintItem&>(rItem);
    return ( theOptions == rPItem.theOptions );
}

ScTpPrintItem* ScTpPrintItem::Clone( SfxItemPool * ) const
{
    return new ScTpPrintItem( *this );
}

constexpr OUStringLiteral CFGPATH_PRINT = u"Office.Calc/Print";

#define SCPRINTOPT_EMPTYPAGES       0
#define SCPRINTOPT_ALLSHEETS        1
#define SCPRINTOPT_FORCEBREAKS      2

Sequence<OUString> ScPrintCfg::GetPropertyNames()
{
    return {u"Page/EmptyPages"_ustr,          // SCPRINTOPT_EMPTYPAGES
            u"Other/AllSheets"_ustr,          // SCPRINTOPT_ALLSHEETS
            u"Page/ForceBreaks"_ustr};        // SCPRINTOPT_FORCEBREAKS;
}

ScPrintCfg::ScPrintCfg() :
    ConfigItem( CFGPATH_PRINT )
{
    Sequence<OUString> aNames = GetPropertyNames();
    EnableNotification(aNames);
    ReadCfg();
}

void ScPrintCfg::ReadCfg()
{
    const Sequence<OUString> aNames = GetPropertyNames();
    const Sequence<Any> aValues = GetProperties(aNames);
    OSL_ENSURE(aValues.getLength() == aNames.getLength(), "GetProperties failed");
    if(aValues.getLength() != aNames.getLength())
        return;

    if (bool bVal; aValues[SCPRINTOPT_EMPTYPAGES] >>= bVal)
        SetSkipEmpty(!bVal); // reversed
    if (bool bVal; aValues[SCPRINTOPT_ALLSHEETS] >>= bVal)
        SetAllSheets(bVal);
    if (bool bVal; aValues[SCPRINTOPT_FORCEBREAKS] >>= bVal)
        SetForceBreaks(bVal);
}

void ScPrintCfg::ImplCommit()
{
    Sequence<OUString> aNames = GetPropertyNames();
    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

    pValues[SCPRINTOPT_EMPTYPAGES] <<= !GetSkipEmpty(); // reversed
    pValues[SCPRINTOPT_ALLSHEETS] <<= GetAllSheets();
    pValues[SCPRINTOPT_FORCEBREAKS] <<= GetForceBreaks();
    PutProperties(aNames, aValues);
}

void ScPrintCfg::SetOptions( const ScPrintOptions& rNew )
{
    *static_cast<ScPrintOptions*>(this) = rNew;
    SetModified();
    Commit();
}

void ScPrintCfg::Notify( const css::uno::Sequence< OUString >& ) { ReadCfg(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
