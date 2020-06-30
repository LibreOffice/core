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

#include "WriterInspectorTextPanel.hxx"
#include <svx/svxids.hrc>
#include <doc.hxx>
#include <ndtxt.hxx>
#include <docsh.hxx>
#include <wrtsh.hxx>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/text/XTextCursor.hpp>
#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>

#include <unotextrange.hxx>
#include <svl/languageoptions.hxx>

namespace sw::sidebar
{
VclPtr<vcl::Window> WriterInspectorTextPanel::Create(vcl::Window* pParent,
                                                     const uno::Reference<frame::XFrame>& rxFrame)
{
    if (pParent == nullptr)
        throw lang::IllegalArgumentException(
            "no parent Window given to WriterInspectorTextPanel::Create", nullptr, 0);
    if (!rxFrame.is())
        throw lang::IllegalArgumentException("no XFrame given to WriterInspectorTextPanel::Create",
                                             nullptr, 1);

    return VclPtr<WriterInspectorTextPanel>::Create(pParent, rxFrame);
}

WriterInspectorTextPanel::WriterInspectorTextPanel(vcl::Window* pParent,
                                                   const uno::Reference<frame::XFrame>& rxFrame)
    : InspectorTextPanel(pParent, rxFrame)
{
    SwDocShell* pDocSh = static_cast<SwDocShell*>(SfxObjectShell::Current());
    SwWrtShell* pShell = pDocSh->GetWrtShell();
    if (pShell)
        pShell->SetChgLnk(LINK(this, WriterInspectorTextPanel, AttrChangedNotify));
}

static bool GetPropertyValues(const beans::Property rProperty, const uno::Any& rAny,
                              OUString& rString)
{
    // Hide Asian and Complex properties
    if (SvtLanguageOptions().IsCJKFontEnabled() && rProperty.Name.indexOf("Asian") != -1)
        return false;
    if (SvtLanguageOptions().IsCTLFontEnabled() && rProperty.Name.indexOf("Complex") != -1)
        return false;

    OUString aValue;
    double fValue;
    bool bValue;
    short sValue;
    long lValue;
    awt::FontSlant iValue;

    rString = rProperty.Name + "     ";

    if (rAny >>= bValue)
    {
        rString += OUString::boolean(bValue);
    }
    else if ((rAny >>= aValue) && !(aValue.isEmpty()))
    {
        rString += aValue;
    }
    else if (rAny >>= iValue)
    {
        rString += (iValue == awt::FontSlant_ITALIC) ? OUStringLiteral("italic")
                                                     : OUStringLiteral("normal");
    }
    else if ((rAny >>= lValue) && lValue)
    {
        if (rString.indexOf("Color") != -1)
            rString += "0x" + OUString::number(lValue, 16);
        else
            rString += OUString::number(lValue);
    }
    else if ((rAny >>= fValue) && fValue)
    {
        if (rString.indexOf("Weight") != -1)
            rString += (fValue > 100) ? OUStringLiteral("bold") : OUStringLiteral("normal");
        else
            rString += OUString::number((round(fValue * 100)) / 100.00);
    }
    else if ((rAny >>= sValue) && sValue)
    {
        rString += OUString::number(sValue);
    }
    else
        return false;
    return true;
}

static void UpdateTree(SwDocShell* pDocSh, std::vector<svx::sidebar::TreeNode>& aStore)
{
    SwDoc* pDoc = pDocSh->GetDoc();
    SwPaM* pCursor = pDoc->GetEditShell()->GetCursor();
    svx::sidebar::TreeNode pDFNode;
    svx::sidebar::TreeNode pCharNode;
    svx::sidebar::TreeNode pParaNode;

    pDFNode.sNodeName = "Direct Formatting";
    pCharNode.sNodeName = "Character Styles";
    pParaNode.sNodeName = "Paragraph Styles";

    uno::Reference<text::XTextRange> xRange(
        SwXTextRange::CreateXTextRange(*pDoc, *pCursor->GetPoint(), nullptr));

    uno::Reference<beans::XPropertySet> xPropertiesSet(xRange, uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertyState> xPropertiesState(xRange, uno::UNO_QUERY_THROW);

    uno::Sequence<beans::Property> aProperties
        = xPropertiesSet->getPropertySetInfo()->getProperties();

    std::unordered_map<OUString, bool> aIsDefined;
    for (const beans::Property& rProperty : std::as_const(aProperties))
    {
        if (xPropertiesState->getPropertyState(rProperty.Name) == beans::PropertyState_DIRECT_VALUE)
        {
            OUString aPropertyValuePair;
            const uno::Any aAny = xPropertiesSet->getPropertyValue(rProperty.Name);
            if (GetPropertyValues(rProperty, aAny, aPropertyValuePair))
            {
                aIsDefined[rProperty.Name] = true;
                svx::sidebar::TreeNode pTemp;
                pTemp.sNodeName = aPropertyValuePair;
                pDFNode.children.push_back(pTemp);
            }
        }
    }

    uno::Reference<style::XStyleFamiliesSupplier> xStyleFamiliesSupplier(pDocSh->GetBaseModel(),
                                                                         uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xStyleFamilies
        = xStyleFamiliesSupplier->getStyleFamilies();
    OUString sCurrentCharStyle, sCurrentParaStyle, sDisplayName;

    uno::Reference<container::XNameAccess> xStyleFamily(
        xStyleFamilies->getByName("CharacterStyles"), uno::UNO_QUERY_THROW);
    xPropertiesSet->getPropertyValue("CharStyleName") >>= sCurrentCharStyle;
    xPropertiesSet->getPropertyValue("ParaStyleName") >>= sCurrentParaStyle;

    if (!sCurrentCharStyle.isEmpty())
    {
        xPropertiesSet.set(xStyleFamily->getByName(sCurrentCharStyle), css::uno::UNO_QUERY_THROW);
        xPropertiesState.set(xPropertiesSet, css::uno::UNO_QUERY_THROW);
        xPropertiesSet->getPropertyValue("DisplayName") >>= sDisplayName;
        aProperties = xPropertiesSet->getPropertySetInfo()->getProperties();
        svx::sidebar::TreeNode pCurrentChild;
        pCurrentChild.sNodeName = sDisplayName;

        for (const beans::Property& rProperty : std::as_const(aProperties))
        {
            OUString sPropName = rProperty.Name;
            if (xPropertiesState->getPropertyState(sPropName) == beans::PropertyState_DIRECT_VALUE)
            {
                OUString aPropertyValuePair;
                const uno::Any aAny = xPropertiesSet->getPropertyValue(sPropName);
                if (GetPropertyValues(rProperty, aAny, aPropertyValuePair))
                {
                    if (aIsDefined[sPropName]) // Already defined in "Direct Formatting" ?
                        aPropertyValuePair = aPropertyValuePair + "  !!<GREY>!!";
                    else
                        aIsDefined[sPropName] = true;
                    svx::sidebar::TreeNode pTemp;
                    pTemp.sNodeName = aPropertyValuePair;
                    pCurrentChild.children.push_back(pTemp);
                }
            }
        }

        pCharNode.children.push_back(pCurrentChild);
    }

    xStyleFamily.set(xStyleFamilies->getByName("ParagraphStyles"), uno::UNO_QUERY_THROW);

    while (!sCurrentParaStyle.isEmpty())
    {
        uno::Reference<style::XStyle> xPropertiesStyle(xStyleFamily->getByName(sCurrentParaStyle),
                                                       uno::UNO_QUERY_THROW);
        xPropertiesSet.set(xPropertiesStyle, css::uno::UNO_QUERY_THROW);
        xPropertiesState.set(xPropertiesSet, css::uno::UNO_QUERY_THROW);
        xPropertiesSet->getPropertyValue("DisplayName") >>= sDisplayName;
        aProperties = xPropertiesSet->getPropertySetInfo()->getProperties();
        OUString aParentParaStyle = xPropertiesStyle->getParentStyle();
        svx::sidebar::TreeNode pCurrentChild;
        pCurrentChild.sNodeName = sDisplayName;

        for (const beans::Property& rProperty : std::as_const(aProperties))
        {
            OUString sPropName = rProperty.Name;
            if (aParentParaStyle.isEmpty() // root of style hierarchy
                || xPropertiesState->getPropertyState(sPropName)
                       == beans::PropertyState_DIRECT_VALUE)
            {
                OUString aPropertyValuePair;
                const uno::Any aAny = xPropertiesSet->getPropertyValue(sPropName);
                if (GetPropertyValues(rProperty, aAny, aPropertyValuePair))
                {
                    // Already defined in "Default Formatting" or "Character Styles" or any child Paragraph Style ?
                    if (aIsDefined[sPropName])
                        aPropertyValuePair = aPropertyValuePair + "    !!<GREY>!! ";
                    else
                        aIsDefined[sPropName] = true;
                    svx::sidebar::TreeNode pTemp;
                    pTemp.sNodeName = aPropertyValuePair;
                    pCurrentChild.children.push_back(pTemp);
                }
            }
        }

        pParaNode.children.push_back(pCurrentChild);
        sCurrentParaStyle = aParentParaStyle;
    }

    std::reverse(pParaNode.children.begin(),
                 pParaNode.children.end()); // Parent style should be first then children

    /*
    Display Order :-
    PARAGRAPH STYLE
    CHARACTER STYLE
    DEFAULT FORMATTING
    */
    aStore.push_back(pParaNode);
    aStore.push_back(pCharNode);
    aStore.push_back(pDFNode);
}

IMPL_LINK_NOARG(WriterInspectorTextPanel, AttrChangedNotify, LinkParamNone*, void)
{
    SwDocShell* pDocSh = static_cast<SwDocShell*>(SfxObjectShell::Current());
    std::vector<svx::sidebar::TreeNode> aStore;

    if (pDocSh && pDocSh->GetDoc()->GetEditShell()->GetCursor()->GetNode().GetTextNode())
        UpdateTree(pDocSh, aStore);

    updateEntries(aStore);
}

} // end of namespace svx::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
