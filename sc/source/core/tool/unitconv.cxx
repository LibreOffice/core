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

#include <unitconv.hxx>
#include <global.hxx>
#include <optutil.hxx>

using namespace utl;
using namespace com::sun::star::uno;

const sal_Unicode cDelim = 0x01;        // delimiter between From and To

// ScUnitConverterData
ScUnitConverterData::ScUnitConverterData(
    std::u16string_view rFromUnit, std::u16string_view rToUnit, double fValue ) :
    maIndexString(BuildIndexString(rFromUnit, rToUnit)),
    mfValue(fValue) {}

OUString ScUnitConverterData::BuildIndexString(
    std::u16string_view rFromUnit, std::u16string_view rToUnit )
{
    return rFromUnit + OUStringChar(cDelim) + rToUnit;
}

// ScUnitConverter
constexpr OUStringLiteral CFGPATH_UNIT = u"Office.Calc/UnitConversion";
constexpr OUStringLiteral CFGSTR_UNIT_FROM = u"FromUnit";
constexpr OUStringLiteral CFGSTR_UNIT_TO = u"ToUnit";
constexpr OUStringLiteral CFGSTR_UNIT_FACTOR = u"Factor";

ScUnitConverter::ScUnitConverter()
{
    // read from configuration - "convert.ini" is no longer used
    //TODO: config item as member to allow change of values

    ScLinkConfigItem aConfigItem( CFGPATH_UNIT );

    // empty node name -> use the config item's path itself
    const Sequence<OUString> aNodeNames = aConfigItem.GetNodeNames( u""_ustr );

    tools::Long nNodeCount = aNodeNames.getLength();
    if ( !nNodeCount )
        return;

    Sequence<OUString> aValNames( nNodeCount * 3 );
    OUString* pValNameArray = aValNames.getArray();
    const OUString sSlash('/');

    tools::Long nIndex = 0;
    for (const OUString& rNode : aNodeNames)
    {
        OUString sPrefix = rNode + sSlash;

        pValNameArray[nIndex++] = sPrefix + CFGSTR_UNIT_FROM;
        pValNameArray[nIndex++] = sPrefix + CFGSTR_UNIT_TO;
        pValNameArray[nIndex++] = sPrefix + CFGSTR_UNIT_FACTOR;
    }

    Sequence<Any> aProperties = aConfigItem.GetProperties(aValNames);

    if (aProperties.getLength() != aValNames.getLength())
        return;

    const Any* pProperties = aProperties.getConstArray();

    OUString sFromUnit;
    OUString sToUnit;
    double fFactor = 0;

    nIndex = 0;
    for (tools::Long i=0; i<nNodeCount; i++)
    {
        pProperties[nIndex++] >>= sFromUnit;
        pProperties[nIndex++] >>= sToUnit;
        pProperties[nIndex++] >>= fFactor;

        ScUnitConverterData aNew(sFromUnit, sToUnit, fFactor);
        OUString const aIndex = aNew.GetIndexString();
        maData.insert(std::make_pair(aIndex, aNew));
    }
}

ScUnitConverter::~ScUnitConverter() {}

bool ScUnitConverter::GetValue(
    double& fValue, std::u16string_view rFromUnit, std::u16string_view rToUnit ) const
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
