/*************************************************************************
 *
 *  $RCSfile: printopt.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: nn $ $Date: 2001-05-29 19:33:50 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PCH
#include "core_pch.hxx"
#endif

#pragma hdrstop

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#include "printopt.hxx"
#include "miscuno.hxx"

using namespace utl;
using namespace rtl;
using namespace com::sun::star::uno;

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
    bSkipEmpty = FALSE;
    bAllSheets = TRUE;
}

const ScPrintOptions& ScPrintOptions::operator=( const ScPrintOptions& rCpy )
{
    bSkipEmpty = rCpy.bSkipEmpty;
    bAllSheets = rCpy.bAllSheets;
    return *this;
}

inline int ScPrintOptions::operator==( const ScPrintOptions& rOpt ) const
{
    return bSkipEmpty == rOpt.bSkipEmpty
        && bAllSheets == rOpt.bAllSheets;
}

inline int ScPrintOptions::operator!=( const ScPrintOptions& rOpt ) const
{
    return !(operator==(rOpt));
}

// -----------------------------------------------------------------------

ScTpPrintItem::ScTpPrintItem( USHORT nWhich ) : SfxPoolItem( nWhich )
{
}

ScTpPrintItem::ScTpPrintItem( USHORT nWhich, const ScPrintOptions& rOpt ) :
    SfxPoolItem ( nWhich ),
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
    ConfigItem( OUString::createFromAscii( CFGPATH_PRINT ) )
{
    Sequence<OUString> aNames = GetPropertyNames();
    Sequence<Any> aValues = GetProperties(aNames);
//  EnableNotification(aNames);
    const Any* pValues = aValues.getConstArray();
    DBG_ASSERT(aValues.getLength() == aNames.getLength(), "GetProperties failed")
    if(aValues.getLength() == aNames.getLength())
    {
        for(int nProp = 0; nProp < aNames.getLength(); nProp++)
        {
            DBG_ASSERT(pValues[nProp].hasValue(), "property value missing")
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
    OUString* pNames = aNames.getArray();
    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

    const Type& rType = ::getBooleanCppuType();
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

void ScPrintCfg::OptionsChanged()
{
    SetModified();
}


