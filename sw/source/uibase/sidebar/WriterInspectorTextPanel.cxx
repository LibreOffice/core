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

#include <doc.hxx>
#include <ndtxt.hxx>
#include <docsh.hxx>
#include <wrtsh.hxx>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/text/XTextCursor.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>

#include <unotextrange.hxx>

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

static void UpdateTree(SwDocShell* pDocSh, std::vector<svx::sidebar::TreeNode>& aStore)
{
    SwDoc* pDoc = pDocSh->GetDoc();
    SwPaM* pCursor = pDoc->GetEditShell()->GetCursor();
    svx::sidebar::TreeNode aDFNode;
    svx::sidebar::TreeNode aCharNode;
    svx::sidebar::TreeNode aParaNode;

    aDFNode.sNodeName = "Direct Formatting";
    aCharNode.sNodeName = "Character Styles";
    aParaNode.sNodeName = "Paragraph Styles";
    aDFNode.NodeType = svx::sidebar::TreeNode::Category;
    aCharNode.NodeType = svx::sidebar::TreeNode::Category;
    aParaNode.NodeType = svx::sidebar::TreeNode::Category;

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
            const uno::Any aAny = xPropertiesSet->getPropertyValue(rProperty.Name);
            aIsDefined[rProperty.Name] = true;
            svx::sidebar::TreeNode aTemp;
            aTemp.sNodeName = rProperty.Name;
            aTemp.aValue = aAny;
            aDFNode.children.push_back(aTemp);
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
        svx::sidebar::TreeNode aCurrentChild;
        aCurrentChild.sNodeName = sDisplayName;
        aCurrentChild.NodeType = svx::sidebar::TreeNode::ComplexProperty;

        for (const beans::Property& rProperty : std::as_const(aProperties))
        {
            OUString sPropName = rProperty.Name;
            if (xPropertiesState->getPropertyState(sPropName) == beans::PropertyState_DIRECT_VALUE)
            {
                const uno::Any aAny = xPropertiesSet->getPropertyValue(sPropName);
                svx::sidebar::TreeNode aTemp;
                if (aIsDefined[sPropName]) // Already defined in "Direct Formatting" ?
                    aTemp.isGrey = true;
                aIsDefined[sPropName] = true;
                aTemp.sNodeName = sPropName;
                aTemp.aValue = aAny;
                aCurrentChild.children.push_back(aTemp);
            }
        }

        aCharNode.children.push_back(aCurrentChild);
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
        svx::sidebar::TreeNode aCurrentChild;
        aCurrentChild.sNodeName = sDisplayName;
        aCurrentChild.NodeType = svx::sidebar::TreeNode::ComplexProperty;

        for (const beans::Property& rProperty : std::as_const(aProperties))
        {
            OUString sPropName = rProperty.Name;
            if (aParentParaStyle.isEmpty() // root of style hierarchy
                || xPropertiesState->getPropertyState(sPropName)
                       == beans::PropertyState_DIRECT_VALUE)
            {
                const uno::Any aAny = xPropertiesSet->getPropertyValue(sPropName);
                svx::sidebar::TreeNode aTemp;
                // Already defined in "Default Formatting" or "Character Styles" or any child Paragraph Style ?
                if (aIsDefined[sPropName])
                    aTemp.isGrey = true;
                aIsDefined[sPropName] = true;
                aTemp.sNodeName = sPropName;
                aTemp.aValue = aAny;
                aCurrentChild.children.push_back(aTemp);
            }
        }

        aParaNode.children.push_back(aCurrentChild);
        sCurrentParaStyle = aParentParaStyle;
    }

    std::reverse(aParaNode.children.begin(),
                 aParaNode.children.end()); // Parent style should be first then children

    /*
    Display Order :-
    PARAGRAPH STYLE
    CHARACTER STYLE
    DEFAULT FORMATTING
    */
    aStore.push_back(aParaNode);
    aStore.push_back(aCharNode);
    aStore.push_back(aDFNode);
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
