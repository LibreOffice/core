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
#include <miscuno.hxx>
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

SfxPoolItem* ScTpPrintItem::Clone( SfxItemPool * ) const
{
    return new ScTpPrintItem( *this );
}

#define CFGPATH_PRINT           "Office.Calc/Print"

#define SCPRINTOPT_EMPTYPAGES       0
#define SCPRINTOPT_ALLSHEETS        1
#define SCPRINTOPT_FORCEBREAKS      2

Sequence<OUString> ScPrintCfg::GetPropertyNames()
{
    return {"Page/EmptyPages",          // SCPRINTOPT_EMPTYPAGES
            "Other/AllSheets",          // SCPRINTOPT_ALLSHEETS
            "Page/ForceBreaks"};        // SCPRINTOPT_FORCEBREAKS;
}

ScPrintCfg::ScPrintCfg() :
    ConfigItem( CFGPATH_PRINT )
{
    Sequence<OUString> aNames = GetPropertyNames();
    Sequence<Any> aValues = GetProperties(aNames);
    const Any* pValues = aValues.getConstArray();
    OSL_ENSURE(aValues.getLength() == aNames.getLength(), "GetProperties failed");
    if(aValues.getLength() == aNames.getLength())
    {
        for(int nProp = 0; nProp < aNames.getLength(); nProp++)
        {
            OSL_ENSURE(pValues[nProp].hasValue(), "property value missing");
            if(pValues[nProp].hasValue())
            {
                switch(nProp)
                {
                    case SCPRINTOPT_EMPTYPAGES:
                        // reversed
                        SetSkipEmpty( !ScUnoHelpFunctions::GetBoolFromAny( pValues[nProp] ) );
                        break;
                    case SCPRINTOPT_ALLSHEETS:
                        SetAllSheets( ScUnoHelpFunctions::GetBoolFromAny( pValues[nProp] ) );
                        break;
                    case SCPRINTOPT_FORCEBREAKS:
                        SetForceBreaks( ScUnoHelpFunctions::GetBoolFromAny( pValues[nProp] ) );
                        break;
                }
            }
        }
    }
}

void ScPrintCfg::ImplCommit()
{
    Sequence<OUString> aNames = GetPropertyNames();
    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

    for(int nProp = 0; nProp < aNames.getLength(); nProp++)
    {
        switch(nProp)
        {
            case SCPRINTOPT_EMPTYPAGES:
                // reversed
                pValues[nProp] <<= !GetSkipEmpty();
                break;
            case SCPRINTOPT_ALLSHEETS:
                pValues[nProp] <<= GetAllSheets();
                break;
            case SCPRINTOPT_FORCEBREAKS:
                pValues[nProp] <<= GetForceBreaks();
                break;
        }
    }
    PutProperties(aNames, aValues);
}

void ScPrintCfg::SetOptions( const ScPrintOptions& rNew )
{
    *static_cast<ScPrintOptions*>(this) = rNew;
    SetModified();
}

void ScPrintCfg::Notify( const css::uno::Sequence< OUString >& ) {}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
