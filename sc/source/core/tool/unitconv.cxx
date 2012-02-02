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

ScUnitConverterData::ScUnitConverterData(
    const rtl::OUString& rFromUnit, const rtl::OUString& rToUnit, double fValue ) :
    maIndexString(BuildIndexString(rFromUnit, rToUnit)),
    mfValue(fValue) {}

ScUnitConverterData::ScUnitConverterData( const ScUnitConverterData& r ) :
    maIndexString(r.maIndexString),
    mfValue(r.mfValue) {}

ScUnitConverterData::~ScUnitConverterData() {}

double ScUnitConverterData::GetValue() const
{
    return mfValue;
}

const rtl::OUString& ScUnitConverterData::GetIndexString() const
{
    return maIndexString;
}

rtl::OUString ScUnitConverterData::BuildIndexString(
    const rtl::OUString& rFromUnit, const rtl::OUString& rToUnit )
{
    rtl::OUStringBuffer aBuf(rFromUnit);
    aBuf.append(cDelim);
    aBuf.append(rToUnit);
    return aBuf.makeStringAndClear();
}

// --- ScUnitConverter ------------------------------------------------

#define CFGPATH_UNIT        "Office.Calc/UnitConversion"
#define CFGSTR_UNIT_FROM    "FromUnit"
#define CFGSTR_UNIT_TO      "ToUnit"
#define CFGSTR_UNIT_FACTOR  "Factor"

ScUnitConverter::ScUnitConverter()
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
                rtl::OUString aIndex  = pNew->GetIndexString();
                maData.insert(aIndex, pNew);
            }
        }
    }
}

ScUnitConverter::~ScUnitConverter() {}

bool ScUnitConverter::GetValue(
    double& fValue, const rtl::OUString& rFromUnit, const rtl::OUString& rToUnit ) const
{
    rtl::OUString aIndex = ScUnitConverterData::BuildIndexString(rFromUnit, rToUnit);
    MapType::const_iterator it = maData.find(aIndex);
    if (it == maData.end())
    {
        fValue = 1.0;
        return false;
    }

    fValue = it->second->GetValue();
    return true;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
