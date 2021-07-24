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
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <o3tl/any.hxx>
#include <tools/debug.hxx>
#include <vcl/outdev.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/configitem.hxx>

#include <vector>

using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::beans;


constexpr OUStringLiteral cReplacement = u"Replacement";
constexpr OUStringLiteral cFontPairs = u"FontPairs";

constexpr OUStringLiteral cReplaceFont = u"ReplaceFont";
constexpr OUStringLiteral cSubstituteFont= u"SubstituteFont";
constexpr OUStringLiteral cOnScreenOnly = u"OnScreenOnly";
constexpr OUStringLiteral cAlways = u"Always";

namespace svtools
{

bool IsFontSubstitutionsEnabled()
{
    bool bIsEnabled = false;
    Reference<css::container::XHierarchicalNameAccess> xHierarchyAccess = utl::ConfigManager::acquireTree(u"Office.Common/Font/Substitution");
    Any aVal = xHierarchyAccess->getByHierarchicalName(cReplacement);

    DBG_ASSERT(aVal.hasValue(), "no value available");
    if(aVal.hasValue())
        bIsEnabled = *o3tl::doAccess<bool>(aVal);
    return bIsEnabled;
}

std::vector<SubstitutionStruct> GetFontSubstitutions()
{
    Reference<css::container::XHierarchicalNameAccess> xHierarchyAccess = utl::ConfigManager::acquireTree(u"Office.Common/Font/Substitution");

    const Sequence<OUString> aNodeNames = utl::ConfigItem::GetNodeNames(xHierarchyAccess, cFontPairs, utl::ConfigNameFormat::LocalPath);
    Sequence<OUString> aPropNames(aNodeNames.getLength() * 4);
    OUString* pNames = aPropNames.getArray();
    sal_Int32 nName = 0;
    for(const OUString& rNodeName : aNodeNames)
    {
        OUString sStart = cFontPairs + "/" + rNodeName + "/";
        pNames[nName++] = sStart + cReplaceFont;
        pNames[nName++] = sStart + cSubstituteFont;
        pNames[nName++] = sStart + cAlways;
        pNames[nName++] = sStart + cOnScreenOnly;
    }
    Sequence<Any> aNodeValues = utl::ConfigItem::GetProperties(xHierarchyAccess, aPropNames, /*bAllLocales*/false);
    const Any* pNodeValues = aNodeValues.getConstArray();
    nName = 0;
    std::vector<SubstitutionStruct> aSubstArr;
    for(sal_Int32 nNode = 0; nNode < aNodeNames.getLength(); nNode++)
    {
        SubstitutionStruct aInsert;
        pNodeValues[nName++] >>= aInsert.sFont;
        pNodeValues[nName++] >>= aInsert.sReplaceBy;
        aInsert.bReplaceAlways = *o3tl::doAccess<bool>(pNodeValues[nName++]);
        aInsert.bReplaceOnScreenOnly = *o3tl::doAccess<bool>(pNodeValues[nName++]);
        aSubstArr.push_back(aInsert);
    }
    return aSubstArr;
}

void SetFontSubstitutions(bool bIsEnabled, std::vector<SubstitutionStruct> const & aSubstArr)
{
    Reference<css::container::XHierarchicalNameAccess> xHierarchyAccess = utl::ConfigManager::acquireTree(u"Office.Common/Font/Substitution");
    utl::ConfigItem::PutProperties(xHierarchyAccess, {cReplacement}, {css::uno::Any(bIsEnabled)}, /*bAllLocales*/false);

    OUString sNode(cFontPairs);
    if(aSubstArr.empty())
    {
        utl::ConfigItem::ClearNodeSet(xHierarchyAccess, sNode);
        return;
    }

    Sequence<PropertyValue> aSetValues(4 * aSubstArr.size());
    PropertyValue* pSetValues = aSetValues.getArray();
    sal_Int32 nSetValue = 0;

    const OUString sReplaceFont(cReplaceFont);
    const OUString sSubstituteFont(cSubstituteFont);
    const OUString sAlways(cAlways);
    const OUString sOnScreenOnly(cOnScreenOnly);

    for(size_t i = 0; i < aSubstArr.size(); i++)
    {
        OUString sPrefix = sNode + "/_" + OUString::number(i) + "/";

        const SubstitutionStruct& rSubst = aSubstArr[i];
        pSetValues[nSetValue].Name = sPrefix; pSetValues[nSetValue].Name += sReplaceFont;
        pSetValues[nSetValue++].Value <<= rSubst.sFont;
        pSetValues[nSetValue].Name = sPrefix; pSetValues[nSetValue].Name += sSubstituteFont;
        pSetValues[nSetValue++].Value <<= rSubst.sReplaceBy;
        pSetValues[nSetValue].Name = sPrefix; pSetValues[nSetValue].Name += sAlways;
        pSetValues[nSetValue++].Value <<= rSubst.bReplaceAlways;
        pSetValues[nSetValue].Name = sPrefix; pSetValues[nSetValue].Name += sOnScreenOnly;
        pSetValues[nSetValue++].Value <<= rSubst.bReplaceOnScreenOnly;
    }
    utl::ConfigItem::ReplaceSetProperties(xHierarchyAccess, sNode, aSetValues, /*bAllLocales*/false);
}

void ApplyFontSubstitutionsToVcl()
{
    OutputDevice::BeginFontSubstitution();

    // remove old substitutions
    OutputDevice::RemoveFontsSubstitute();

    const bool bIsEnabled = IsFontSubstitutionsEnabled();
    std::vector<SubstitutionStruct> aSubst = GetFontSubstitutions();

    // read new substitutions
    if (bIsEnabled)
        for (const SubstitutionStruct & rSub : aSubst)
        {
            AddFontSubstituteFlags nFlags = AddFontSubstituteFlags::NONE;
            if(rSub.bReplaceAlways)
                nFlags |= AddFontSubstituteFlags::ALWAYS;
            if(rSub.bReplaceOnScreenOnly)
                nFlags |= AddFontSubstituteFlags::ScreenOnly;
            OutputDevice::AddFontSubstitute( rSub.sFont, rSub.sReplaceBy, nFlags );
        }

    OutputDevice::EndFontSubstitution();
}

} // namespace svtools

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
