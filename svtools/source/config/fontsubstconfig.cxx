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

#include <svtools/fontsubstconfig.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <tools/debug.hxx>
#include <vcl/outdev.hxx>

#include <vector>

using namespace utl;
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::beans;


const sal_Char cReplacement[] = "Replacement";
const sal_Char cFontPairs[] = "FontPairs";

const sal_Char cReplaceFont[]   = "ReplaceFont";
const sal_Char cSubstituteFont[]= "SubstituteFont";
const sal_Char cOnScreenOnly[]  = "OnScreenOnly";
const sal_Char cAlways[]        = "Always";

typedef std::vector<SubstitutionStruct> SubstitutionStructArr;

struct SvtFontSubstConfig_Impl
{
    SubstitutionStructArr   aSubstArr;
};

SvtFontSubstConfig::SvtFontSubstConfig() :
    ConfigItem(OUString("Office.Common/Font/Substitution")),
    bIsEnabled(false),
    pImpl(new SvtFontSubstConfig_Impl)
{
    Sequence<OUString> aNames { cReplacement };
    Sequence<Any> aValues = GetProperties(aNames);
    DBG_ASSERT(aValues.getConstArray()[0].hasValue(), "no value available");
    if(aValues.getConstArray()[0].hasValue())
        bIsEnabled = *static_cast<sal_Bool const *>(aValues.getConstArray()[0].getValue());

    OUString sPropPrefix(cFontPairs);
    Sequence<OUString> aNodeNames = GetNodeNames(sPropPrefix, CONFIG_NAME_LOCAL_PATH);
    const OUString* pNodeNames = aNodeNames.getConstArray();
    Sequence<OUString> aPropNames(aNodeNames.getLength() * 4);
    OUString* pNames = aPropNames.getArray();
    sal_Int32 nName = 0;
    sPropPrefix += "/";
    sal_Int32 nNode;
    for(nNode = 0; nNode < aNodeNames.getLength(); nNode++)
    {
        OUString sStart = sPropPrefix + pNodeNames[nNode] + "/";
        pNames[nName] = sStart;     pNames[nName++] += cReplaceFont;
        pNames[nName] = sStart;     pNames[nName++] += cSubstituteFont;
        pNames[nName] = sStart;     pNames[nName++] += cAlways;
        pNames[nName] = sStart;     pNames[nName++] += cOnScreenOnly;
    }
    Sequence<Any> aNodeValues = GetProperties(aPropNames);
    const Any* pNodeValues = aNodeValues.getConstArray();
    nName = 0;
    for(nNode = 0; nNode < aNodeNames.getLength(); nNode++)
    {
        SubstitutionStruct aInsert;
        pNodeValues[nName++] >>= aInsert.sFont;
        pNodeValues[nName++] >>= aInsert.sReplaceBy;
        aInsert.bReplaceAlways = *static_cast<sal_Bool const *>(pNodeValues[nName++].getValue());
        aInsert.bReplaceOnScreenOnly = *static_cast<sal_Bool const *>(pNodeValues[nName++].getValue());
        pImpl->aSubstArr.push_back(aInsert);
    }
}

SvtFontSubstConfig::~SvtFontSubstConfig()
{
}

void SvtFontSubstConfig::Notify( const css::uno::Sequence< OUString >& )
{
}

void SvtFontSubstConfig::ImplCommit()
{
    Sequence<OUString> aNames { cReplacement };
    Sequence<Any> aValues(1);
    aValues.getArray()[0].setValue(&bIsEnabled, cppu::UnoType<bool>::get());
    PutProperties(aNames, aValues);

    OUString sNode(cFontPairs);
    if(pImpl->aSubstArr.empty())
        ClearNodeSet(sNode);
    else
    {
        Sequence<PropertyValue> aSetValues(4 * pImpl->aSubstArr.size());
        PropertyValue* pSetValues = aSetValues.getArray();
        sal_Int32 nSetValue = 0;

        const OUString sReplaceFont(cReplaceFont);
        const OUString sSubstituteFont(cSubstituteFont);
        const OUString sAlways(cAlways);
        const OUString sOnScreenOnly(cOnScreenOnly);

        const uno::Type& rBoolType = cppu::UnoType<bool>::get();
        for(size_t i = 0; i < pImpl->aSubstArr.size(); i++)
        {
            OUString sPrefix = sNode + "/_" + OUString::number(i) + "/";

            SubstitutionStruct& pSubst = pImpl->aSubstArr[i];
            pSetValues[nSetValue].Name = sPrefix; pSetValues[nSetValue].Name += sReplaceFont;
            pSetValues[nSetValue++].Value <<= pSubst.sFont;
            pSetValues[nSetValue].Name = sPrefix; pSetValues[nSetValue].Name += sSubstituteFont;
            pSetValues[nSetValue++].Value <<= pSubst.sReplaceBy;
            pSetValues[nSetValue].Name = sPrefix; pSetValues[nSetValue].Name += sAlways;
            pSetValues[nSetValue++].Value.setValue(&pSubst.bReplaceAlways, rBoolType);
            pSetValues[nSetValue].Name = sPrefix; pSetValues[nSetValue].Name += sOnScreenOnly;
            pSetValues[nSetValue++].Value.setValue(&pSubst.bReplaceOnScreenOnly, rBoolType);
        }
        ReplaceSetProperties(sNode, aSetValues);
    }
}

sal_Int32 SvtFontSubstConfig::SubstitutionCount() const
{
    return pImpl->aSubstArr.size();
}

void SvtFontSubstConfig::ClearSubstitutions()
{
    pImpl->aSubstArr.clear();
}

const SubstitutionStruct* SvtFontSubstConfig::GetSubstitution(sal_Int32 nPos)
{
    sal_Int32 nCount = static_cast<sal_Int32>(pImpl->aSubstArr.size());
    DBG_ASSERT(nPos >= 0 && nPos < nCount, "illegal array index");
    if(nPos >= 0 && nPos < nCount)
        return &pImpl->aSubstArr[nPos];
    return nullptr;
}

void SvtFontSubstConfig::AddSubstitution(const SubstitutionStruct& rToAdd)
{
    pImpl->aSubstArr.push_back(rToAdd);
}

void SvtFontSubstConfig::Apply()
{
    OutputDevice::BeginFontSubstitution();

    // remove old substitutions
    sal_uInt16 nOldCount = OutputDevice::GetFontSubstituteCount();

    while (nOldCount)
        OutputDevice::RemoveFontSubstitute(--nOldCount);

    // read new substitutions
    sal_Int32 nCount = IsEnabled() ? SubstitutionCount() : 0;

    for (sal_Int32  i = 0; i < nCount; i++)
    {
        AddFontSubstituteFlags nFlags = AddFontSubstituteFlags::NONE;
        const SubstitutionStruct* pSubs = GetSubstitution(i);
        if(pSubs->bReplaceAlways)
            nFlags |= AddFontSubstituteFlags::ALWAYS;
        if(pSubs->bReplaceOnScreenOnly)
            nFlags |= AddFontSubstituteFlags::ScreenOnly;
        OutputDevice::AddFontSubstitute( pSubs->sFont, pSubs->sReplaceBy, nFlags );
    }

    OutputDevice::EndFontSubstitution();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
