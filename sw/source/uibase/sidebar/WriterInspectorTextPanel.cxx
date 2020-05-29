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
        throw lang::IllegalArgumentException("no SfxBindings given to WriterInspectorPanel::Create",
                                             nullptr, 2);

    return VclPtr<WriterInspectorTextPanel>::Create(pParent, rxFrame, pBindings);
}

WriterInspectorTextPanel::WriterInspectorTextPanel(vcl::Window* pParent,
                                                   const uno::Reference<frame::XFrame>& rxFrame,
                                                   SfxBindings* pBindings)
    : InspectorTextPanel(pParent, rxFrame)
    , maCharStyle(SID_STYLE_FAMILY1, *pBindings, *this)
{
}

void WriterInspectorTextPanel::NotifyItemUpdate(const sal_uInt16 nSId,
                                                const SfxItemState /*eState*/,
                                                const SfxPoolItem* /*pState*/)
{
    SwDocShell* pDocSh = static_cast<SwDocShell*>(SfxObjectShell::Current());
    std::vector<OUString> store;
    switch (nSId)
    {
        case SID_STYLE_FAMILY1:
        {
            if (pDocSh)
            {
                SwDoc* pDoc = pDocSh->GetDoc();
                SwPaM* pCursor = pDoc->GetEditShell()->GetCursor();

                uno::Reference<style::XStyleFamiliesSupplier> xStyleFamiliesSupplier(
                    pDocSh->GetBaseModel(), uno::UNO_QUERY);
                uno::Reference<container::XNameAccess> xStyleFamilies
                    = xStyleFamiliesSupplier->getStyleFamilies();
                uno::Reference<container::XNameAccess> xStyleFamily(
                    xStyleFamilies->getByName("CharacterStyles"), uno::UNO_QUERY);

                uno::Reference<text::XTextCursor> xCursor
                    = dynamic_cast<text::XTextCursor*>(pCursor);
                uno::Reference<text::XTextRange> xRange(
                    SwXTextRange::CreateXTextRange(*pDoc, *pCursor->GetPoint(), nullptr));
                uno::Reference<beans::XPropertySet> properties(xRange, uno::UNO_QUERY_THROW);

                OUString aCurrentCharStyle, aDisplayName;
                uno::fromAny(properties->getPropertyValue("CharStyleName"), &aCurrentCharStyle);
                if (aCurrentCharStyle.isEmpty()) // Default Style is always applied
                    aCurrentCharStyle = "Standard";

                const OUString aStyleName = aCurrentCharStyle;
                std::vector<OUString> aStyleNames;
                while (true)
                {
                    uno::Reference<style::XStyle> xProp1(xStyleFamily->getByName(aCurrentCharStyle),
                                                         uno::UNO_QUERY);
                    uno::Reference<beans::XPropertySet> xProp2(
                        xStyleFamily->getByName(aCurrentCharStyle), uno::UNO_QUERY);
                    OUString aParentCharStyle = xProp1->getParentStyle();
                    xProp2->getPropertyValue("DisplayName") >>= aDisplayName;
                    aStyleNames.push_back(aDisplayName);
                    if (aParentCharStyle.isEmpty())
                        break;
                    aCurrentCharStyle = aParentCharStyle;
                }
                // Top Parent goes first...current style goes last
                for (auto itr = aStyleNames.rbegin(); itr != aStyleNames.rend(); ++itr)
                {
                    store.push_back(*itr);
                }

                const uno::Reference<beans::XPropertySet> xStyleProps(
                    xStyleFamily->getByName(aStyleName), uno::UNO_QUERY);
                const uno::Sequence<beans::Property> xPropVal
                    = xStyleProps->getPropertySetInfo()->getProperties();
                for (const beans::Property& rProperty : xPropVal)
                {
                    OUString aPropertyValuePair;
                    const uno::Any aAny = xStyleProps->getPropertyValue(rProperty.Name);
                    WriterInspectorTextPanel::GetPropertyValues(rProperty, aAny,
                                                                aPropertyValuePair);
                    if (!aPropertyValuePair.isEmpty())
                        store.push_back("   " + aPropertyValuePair);
                }
            }
        }
        break;
    }
    InspectorTextPanel::updateEntries(store);
}

void WriterInspectorTextPanel::GetPropertyValues(const beans::Property rProperty,
                                                 const uno::Any& rAny, OUString& rString)
{
    OUString aValue;
    double fValue;
    bool bValue;
    short sValue;
    long lValue;
    awt::FontSlant iValue;

    rString = rProperty.Name + "     ";

    if ((rAny >>= aValue) && !(aValue.isEmpty()))
    {
        rString += aValue;
    }
    else if (rAny >>= iValue)
    {
        rString += ((iValue == awt::FontSlant_ITALIC) ? OUString("italic") : OUString("normal"));
    }
    else if ((rAny >>= fValue) || fValue)
    {
        if (rString == "CharWeight     ")
            rString += ((fValue > 100) ? OUString("bold") : OUString("normal"));
        else
            rString += OUString::number(fValue);
    }
    else if (rAny >>= bValue)
    {
        rString += OUString::boolean(bValue);
    }
    else if ((rAny >>= sValue) || sValue)
    {
        rString += OUString::number(sValue);
    }
    else if ((rAny >>= lValue) || lValue)
    {
        if (rString == "CharColor     ")
            rString += OUString::number(sal_Int16(lValue));
        else
            rString += OUString::number(lValue);
    }
}

} // end of namespace svx::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
