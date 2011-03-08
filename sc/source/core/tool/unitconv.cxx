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

#include "unitconv.hxx"
#include "global.hxx"
#include "viewopti.hxx"         //! move ScLinkConfigItem to separate header!

using namespace utl;
using namespace com::sun::star::uno;

using ::rtl::OUString;

// --------------------------------------------------------------------

const sal_Unicode cDelim = 0x01;        // Delimiter zwischen From und To


// --- ScUnitConverterData --------------------------------------------

ScUnitConverterData::ScUnitConverterData( const String& rFromUnit,
            const String& rToUnit, double fVal )
        :
        StrData( rFromUnit ),
        fValue( fVal )
{
    String aTmp;
    ScUnitConverterData::BuildIndexString( aTmp, rFromUnit, rToUnit );
    SetString( aTmp );
}


ScUnitConverterData::ScUnitConverterData( const ScUnitConverterData& r )
        :
        StrData( r ),
        fValue( r.fValue )
{
}


ScDataObject* ScUnitConverterData::Clone() const
{
    return new ScUnitConverterData( *this );
}


void ScUnitConverterData::BuildIndexString( String& rStr,
            const String& rFromUnit, const String& rToUnit )
{
#if 1
// case sensitive
    rStr = rFromUnit;
    rStr += cDelim;
    rStr += rToUnit;
#else
// not case sensitive
    rStr = rFromUnit;
    String aTo( rToUnit );
    ScGlobal::pCharClass->toUpper( rStr );
    ScGlobal::pCharClass->toUpper( aTo );
    rStr += cDelim;
    rStr += aTo;
#endif
}


// --- ScUnitConverter ------------------------------------------------

#define CFGPATH_UNIT        "Office.Calc/UnitConversion"
#define CFGSTR_UNIT_FROM    "FromUnit"
#define CFGSTR_UNIT_TO      "ToUnit"
#define CFGSTR_UNIT_FACTOR  "Factor"

ScUnitConverter::ScUnitConverter( USHORT nInit, USHORT nDeltaP ) :
        ScStrCollection( nInit, nDeltaP, FALSE )
{
    //  read from configuration - "convert.ini" is no longer used
    //! config item as member to allow change of values

    ScLinkConfigItem aConfigItem( OUString(RTL_CONSTASCII_USTRINGPARAM( CFGPATH_UNIT )) );

    // empty node name -> use the config item's path itself
    OUString aEmptyString;
    Sequence<OUString> aNodeNames = aConfigItem.GetNodeNames( aEmptyString );

    long nNodeCount = aNodeNames.getLength();
    if ( nNodeCount )
    {
        const OUString* pNodeArray = aNodeNames.getConstArray();
        Sequence<OUString> aValNames( nNodeCount * 3 );
        OUString* pValNameArray = aValNames.getArray();
        const OUString sSlash('/');

        long nIndex = 0;
        for (long i=0; i<nNodeCount; i++)
        {
            OUString sPrefix = pNodeArray[i];
            sPrefix += sSlash;

            pValNameArray[nIndex] = sPrefix;
            pValNameArray[nIndex++] += OUString(RTL_CONSTASCII_USTRINGPARAM( CFGSTR_UNIT_FROM ));
            pValNameArray[nIndex] = sPrefix;
            pValNameArray[nIndex++] += OUString(RTL_CONSTASCII_USTRINGPARAM( CFGSTR_UNIT_TO ));
            pValNameArray[nIndex] = sPrefix;
            pValNameArray[nIndex++] += OUString(RTL_CONSTASCII_USTRINGPARAM( CFGSTR_UNIT_FACTOR ));
        }

        Sequence<Any> aProperties = aConfigItem.GetProperties(aValNames);

        if (aProperties.getLength() == aValNames.getLength())
        {
            const Any* pProperties = aProperties.getConstArray();

            OUString sFromUnit;
            OUString sToUnit;
            double fFactor = 0;

            nIndex = 0;
            for (long i=0; i<nNodeCount; i++)
            {
                pProperties[nIndex++] >>= sFromUnit;
                pProperties[nIndex++] >>= sToUnit;
                pProperties[nIndex++] >>= fFactor;

                ScUnitConverterData* pNew = new ScUnitConverterData( sFromUnit, sToUnit, fFactor );
                if ( !Insert( pNew ) )
                    delete pNew;
            }
        }
    }
}

BOOL ScUnitConverter::GetValue( double& fValue, const String& rFromUnit,
                const String& rToUnit ) const
{
    ScUnitConverterData aSearch( rFromUnit, rToUnit );
    USHORT nIndex;
    if ( Search( &aSearch, nIndex ) )
    {
        fValue = ((const ScUnitConverterData*)(At( nIndex )))->GetValue();
        return TRUE;
    }
    fValue = 1.0;
    return FALSE;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
