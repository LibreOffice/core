/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unitconv.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-21 11:45:48 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
using namespace rtl;
using namespace com::sun::star::uno;

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


DataObject* ScUnitConverterData::Clone() const
{
    return new ScUnitConverterData( *this );
}


// static
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

ScUnitConverter::ScUnitConverter( USHORT nInit, USHORT nDelta ) :
        StrCollection( nInit, nDelta, FALSE )
{
    //  read from configuration - "convert.ini" is no longer used
    //! config item as member to allow change of values

    ScLinkConfigItem aConfigItem( OUString::createFromAscii( CFGPATH_UNIT ) );

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
            pValNameArray[nIndex++] += OUString::createFromAscii( CFGSTR_UNIT_FROM );
            pValNameArray[nIndex] = sPrefix;
            pValNameArray[nIndex++] += OUString::createFromAscii( CFGSTR_UNIT_TO );
            pValNameArray[nIndex] = sPrefix;
            pValNameArray[nIndex++] += OUString::createFromAscii( CFGSTR_UNIT_FACTOR );
        }

        Sequence<Any> aProperties = aConfigItem.GetProperties(aValNames);

        if (aProperties.getLength() == aValNames.getLength())
        {
            const Any* pProperties = aProperties.getConstArray();

            OUString sFromUnit;
            OUString sToUnit;
            double fFactor;

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


