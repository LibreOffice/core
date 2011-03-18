/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"



#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#include "printopt.hxx"
#include "miscuno.hxx"

using namespace utl;
using namespace com::sun::star::uno;

using ::rtl::OUString;

// -----------------------------------------------------------------------

TYPEINIT1(ScTpPrintItem, SfxPoolItem);

// -----------------------------------------------------------------------

ScPrintOptions::ScPrintOptions()
{
    SetDefaults();
}

ScPrintOptions::ScPrintOptions( const ScPrintOptions& rCpy ) :
    bSkipEmpty( rCpy.bSkipEmpty ),
    bAllSheets( rCpy.bAllSheets )
{
}

ScPrintOptions::~ScPrintOptions()
{
}

void ScPrintOptions::SetDefaults()
{
    bSkipEmpty = sal_True;
    bAllSheets = false;
}

const ScPrintOptions& ScPrintOptions::operator=( const ScPrintOptions& rCpy )
{
    bSkipEmpty = rCpy.bSkipEmpty;
    bAllSheets = rCpy.bAllSheets;
    return *this;
}

int ScPrintOptions::operator==( const ScPrintOptions& rOpt ) const
{
    return bSkipEmpty == rOpt.bSkipEmpty
        && bAllSheets == rOpt.bAllSheets;
}

int ScPrintOptions::operator!=( const ScPrintOptions& rOpt ) const
{
    return !(operator==(rOpt));
}

// -----------------------------------------------------------------------


ScTpPrintItem::ScTpPrintItem( sal_uInt16 nWhichP, const ScPrintOptions& rOpt ) :
    SfxPoolItem ( nWhichP ),
    theOptions  ( rOpt )
{
}

ScTpPrintItem::ScTpPrintItem( const ScTpPrintItem& rItem ) :
    SfxPoolItem ( rItem ),
    theOptions  ( rItem.theOptions )
{
}

ScTpPrintItem::~ScTpPrintItem()
{
}

String ScTpPrintItem::GetValueText() const
{
    return String::CreateFromAscii( "ScTpPrintItem" );
}

int ScTpPrintItem::operator==( const SfxPoolItem& rItem ) const
{
    DBG_ASSERT( SfxPoolItem::operator==( rItem ), "unequal Which or Type" );

    const ScTpPrintItem& rPItem = (const ScTpPrintItem&)rItem;
    return ( theOptions == rPItem.theOptions );
}

SfxPoolItem* ScTpPrintItem::Clone( SfxItemPool * ) const
{
    return new ScTpPrintItem( *this );
}

// -----------------------------------------------------------------------

#define CFGPATH_PRINT           "Office.Calc/Print"

#define SCPRINTOPT_EMPTYPAGES       0
#define SCPRINTOPT_ALLSHEETS        1
#define SCPRINTOPT_COUNT            2

Sequence<OUString> ScPrintCfg::GetPropertyNames()
{
    static const char* aPropNames[] =
    {
        "Page/EmptyPages",          // SCPRINTOPT_EMPTYPAGES
        "Other/AllSheets"           // SCPRINTOPT_ALLSHEETS
    };
    Sequence<OUString> aNames(SCPRINTOPT_COUNT);
    OUString* pNames = aNames.getArray();
    for(int i = 0; i < SCPRINTOPT_COUNT; i++)
        pNames[i] = OUString::createFromAscii(aPropNames[i]);

    return aNames;
}

ScPrintCfg::ScPrintCfg() :
    ConfigItem( OUString(RTL_CONSTASCII_USTRINGPARAM( CFGPATH_PRINT )) )
{
    Sequence<OUString> aNames = GetPropertyNames();
    Sequence<Any> aValues = GetProperties(aNames);
    const Any* pValues = aValues.getConstArray();
    DBG_ASSERT(aValues.getLength() == aNames.getLength(), "GetProperties failed");
    if(aValues.getLength() == aNames.getLength())
    {
        for(int nProp = 0; nProp < aNames.getLength(); nProp++)
        {
            DBG_ASSERT(pValues[nProp].hasValue(), "property value missing");
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
                }
            }
        }
    }
}


void ScPrintCfg::Commit()
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
                ScUnoHelpFunctions::SetBoolInAny( pValues[nProp], !GetSkipEmpty() );
                break;
            case SCPRINTOPT_ALLSHEETS:
                ScUnoHelpFunctions::SetBoolInAny( pValues[nProp], GetAllSheets() );
                break;
        }
    }
    PutProperties(aNames, aValues);
}

void ScPrintCfg::SetOptions( const ScPrintOptions& rNew )
{
    *(ScPrintOptions*)this = rNew;
    SetModified();
}

void ScPrintCfg::Notify( const ::com::sun::star::uno::Sequence< OUString >& ) {}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
