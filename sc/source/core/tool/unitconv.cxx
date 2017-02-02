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

#include <com/sun/star/uno/Sequence.hxx>

#include "unitconv.hxx"
#include "global.hxx"
#include "viewopti.hxx"

using namespace utl;
using namespace com::sun::star::uno;

const sal_Unicode cDelim = 0x01;        // Delimiter zwischen From und To

// ScUnitConverterData
ScUnitConverterData::ScUnitConverterData(
    const OUString& rFromUnit, const OUString& rToUnit, double fValue ) :
    maIndexString(BuildIndexString(rFromUnit, rToUnit)),
    mfValue(fValue) {}

ScUnitConverterData::ScUnitConverterData( const ScUnitConverterData& r ) :
    maIndexString(r.maIndexString),
    mfValue(r.mfValue) {}

ScUnitConverterData::~ScUnitConverterData() {}

OUString ScUnitConverterData::BuildIndexString(
    const OUString& rFromUnit, const OUString& rToUnit )
{
    OUStringBuffer aBuf(rFromUnit);
    aBuf.append(cDelim);
    aBuf.append(rToUnit);
    return aBuf.makeStringAndClear();
}

// ScUnitConverter
#define CFGPATH_UNIT        "Office.Calc/UnitConversion"
#define CFGSTR_UNIT_FROM    "FromUnit"
#define CFGSTR_UNIT_TO      "ToUnit"
#define CFGSTR_UNIT_FACTOR  "Factor"

ScUnitConverter::ScUnitConverter()
{
    // read from configuration - "convert.ini" is no longer used
    //TODO: config item as member to allow change of values

    ScLinkConfigItem aConfigItem( CFGPATH_UNIT );

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

            pValNameArray[nIndex++] = sPrefix + CFGSTR_UNIT_FROM;
            pValNameArray[nIndex++] = sPrefix + CFGSTR_UNIT_TO;
            pValNameArray[nIndex++] = sPrefix + CFGSTR_UNIT_FACTOR;
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

                ScUnitConverterData aNew(sFromUnit, sToUnit, fFactor);
                OUString const aIndex = aNew.GetIndexString();
                maData.insert(std::make_pair(aIndex, aNew));
            }
        }
    }
}

ScUnitConverter::~ScUnitConverter() {}

bool ScUnitConverter::GetValue(
    double& fValue, const OUString& rFromUnit, const OUString& rToUnit ) const
{
    OUString aIndex = ScUnitConverterData::BuildIndexString(rFromUnit, rToUnit);
    MapType::const_iterator it = maData.find(aIndex);
    if (it == maData.end())
    {
        fValue = 1.0;
        return false;
    }

    fValue = it->second.GetValue();
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
