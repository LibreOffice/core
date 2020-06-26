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
#include <docsh.hxx>
#include <wrtsh.hxx>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/text/XTextCursor.hpp>
#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>

#include <unotextrange.hxx>

namespace sw::sidebar
{
VclPtr<vcl::Window> WriterInspectorTextPanel::Create(vcl::Window* pParent,
                                                     const uno::Reference<frame::XFrame>& rxFrame,
                                                     SfxBindings* pBindings)
{
    if (pParent == nullptr)
        throw lang::IllegalArgumentException(
            "no parent Window given to WriterInspectorTextPanel::Create", nullptr, 0);
    if (!rxFrame.is())
        throw lang::IllegalArgumentException("no XFrame given to WriterInspectorTextPanel::Create",
                                             nullptr, 1);
    if (pBindings == nullptr)
        throw lang::IllegalArgumentException(
            "no SfxBindings given to WriterInspectorTextPanel::Create", nullptr, 2);

    return VclPtr<WriterInspectorTextPanel>::Create(pParent, rxFrame, pBindings);
}

WriterInspectorTextPanel::WriterInspectorTextPanel(vcl::Window* pParent,
                                                   const uno::Reference<frame::XFrame>& rxFrame,
                                                   SfxBindings* pBindings)
    : InspectorTextPanel(pParent, rxFrame)
    , maCharStyle(SID_STYLE_FAMILY1, *pBindings, *this)
    , maParaStyle(SID_STYLE_FAMILY2, *pBindings, *this)
{
}

void WriterInspectorTextPanel::mUpdateTree(SwDocShell* pDocSh, Mynode* xCurrentTree,
                                           std::unordered_map<OUString, bool>& maIsDefined,
                                           OUString sType)
{
    SwDoc* pDoc = pDocSh->GetDoc();
    SwPaM* pCursor = pDoc->GetEditShell()->GetCursor();

    uno::Reference<style::XStyleFamiliesSupplier> xStyleFamiliesSupplier(pDocSh->GetBaseModel(),
                                                                         uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xStyleFamilies
        = xStyleFamiliesSupplier->getStyleFamilies();
    uno::Reference<container::XNameAccess> xStyleFamily;

    if (sType == "CHARACTER STYLES")
        xStyleFamilies->getByName("CharacterStyles") >>= xStyleFamily;
    else
        xStyleFamilies->getByName("ParagraphStyles") >>= xStyleFamily;

    uno::Reference<text::XTextCursor> xCursor = dynamic_cast<text::XTextCursor*>(pCursor);
    uno::Reference<text::XTextRange> xRange(
        SwXTextRange::CreateXTextRange(*pDoc, *pCursor->GetPoint(), nullptr));
    uno::Reference<beans::XPropertySet> properties(xRange, uno::UNO_QUERY_THROW);

    OUString aCurrentStyleName, aDisplayName;
    if (sType == "CHARACTER STYLES")
        properties->getPropertyValue("CharStyleName") >>= aCurrentStyleName;
    else
        properties->getPropertyValue("ParaStyleName") >>= aCurrentStyleName;

    if (aCurrentStyleName.isEmpty())
        aCurrentStyleName = "Standard";

    while (true)
    {
        uno::Reference<style::XStyle> xProp1;
        uno::Reference<beans::XPropertySet> xProp1Set;
        uno::Reference<beans::XPropertyState> xProp1State;
        xStyleFamily->getByName(aCurrentStyleName) >>= xProp1;
        xStyleFamily->getByName(aCurrentStyleName) >>= xProp1Set;
        xStyleFamily->getByName(aCurrentStyleName) >>= xProp1State;
        OUString aParentCharStyle = xProp1->getParentStyle();
        xProp1Set->getPropertyValue("DisplayName") >>= aDisplayName;
        Mynode* xCurTree = new Mynode(aDisplayName);

        if (aParentCharStyle.isEmpty())
        {
            break; // when current style is "Standard" there is no parent
        }

        const uno::Sequence<beans::Property> xProp1SetInfo
            = xProp1Set->getPropertySetInfo()->getProperties();
        const uno::Reference<beans::XPropertySet> xProp2Set(
            xStyleFamily->getByName(aParentCharStyle), uno::UNO_QUERY);

        try
        {
            for (const beans::Property& rProperty : xProp1SetInfo)
            {
                OUString sPropName = rProperty.Name;
                if (xProp1Set->getPropertyValue(sPropName)
                    == xProp1State->getPropertyDefault(sPropName))
                    continue;
                if (maIsDefined[sPropName])
                    continue;
                if (xProp1Set->getPropertyValue(sPropName)
                    != xProp2Set->getPropertyValue(sPropName))
                {
                    maIsDefined[sPropName] = true;
                    OUString aPropertyValuePair;
                    const uno::Any aAny = xProp1Set->getPropertyValue(sPropName);
                    WriterInspectorTextPanel::GetPropertyValues(rProperty, aAny,
                                                                aPropertyValuePair);
                    if (!aPropertyValuePair.isEmpty())
                    {
                        Mynode* xtemP = new Mynode(aPropertyValuePair);
                        xCurTree->vec.emplace_back(xtemP);
                    }
                }
            }
        }
        catch (const uno::Exception&)
        {
            //do nothing
        }

        xCurrentTree->vec.emplace_back(xCurTree);
        aCurrentStyleName = aParentCharStyle;
    }

    uno::Reference<beans::XPropertySet> xProp1Set;
    uno::Reference<beans::XPropertyState> xProp1State;
    xStyleFamily->getByName(aCurrentStyleName) >>= xProp1Set;
    xStyleFamily->getByName(aCurrentStyleName) >>= xProp1State;

    const uno::Sequence<beans::Property> xPropVal
        = xProp1Set->getPropertySetInfo()->getProperties();
    Mynode* xCurTree = new Mynode(aDisplayName);

    for (const beans::Property& rProperty : xPropVal)
    {
        OUString aPropertyValuePair, sPropName = rProperty.Name;
        if (xProp1Set->getPropertyValue(sPropName) == xProp1State->getPropertyDefault(sPropName))
            continue;
        if (maIsDefined[sPropName])
            continue;
        maIsDefined[sPropName] = true;

        const uno::Any aAny = xProp1Set->getPropertyValue(sPropName);
        if (WriterInspectorTextPanel::GetPropertyValues(rProperty, aAny, aPropertyValuePair))
        {
            if (!aPropertyValuePair.isEmpty())
            {
                Mynode* xtemP = new Mynode(aPropertyValuePair);
                xCurTree->vec.emplace_back(xtemP);
            }
        }
    }

    xCurrentTree->vec.emplace_back(xCurTree);
    std::reverse(xCurrentTree->vec.begin(), xCurrentTree->vec.end());
}

void WriterInspectorTextPanel::NotifyItemUpdate(const sal_uInt16 nSId,
                                                const SfxItemState /*eState*/,
                                                const SfxPoolItem* /*pState*/)
{
    SwDocShell* pDocSh = static_cast<SwDocShell*>(SfxObjectShell::Current());
    xStore.clear();
    std::unordered_map<OUString, bool> maIsDefined;

    switch (nSId)
    {
        case SID_STYLE_FAMILY1:
        case SID_STYLE_FAMILY2:
        {
            if (pDocSh)
            {
                /*
                First check in the property set of Character Styles
                (as CS has higher priority over PS), then look into
                property set of Paragraph Styles;
                */
                Mynode* xTempChar = new Mynode("CHARACTER STYLES");
                mUpdateTree(pDocSh, xTempChar, maIsDefined, "CHARACTER STYLES");
                Mynode* xTempPara = new Mynode("PARAGRAPH STYLES");
                mUpdateTree(pDocSh, xTempPara, maIsDefined, "PARAGRAPH STYLES");

                /*
                First insert the properties extracted from PS
                then properties extracted from CS
                */
                xStore.emplace_back(xTempPara);
                xStore.emplace_back(xTempChar);
            }
        }
        break;
    }

    updateEntries();
}

bool WriterInspectorTextPanel::GetPropertyValues(const beans::Property rProperty,
                                                 const uno::Any& rAny, OUString& rString)
{
    // Hide Asian and Complex properties
    if (rProperty.Name.indexOf("Asian") != -1 || rProperty.Name.indexOf("Complex") != -1)
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

} // end of namespace svx::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
