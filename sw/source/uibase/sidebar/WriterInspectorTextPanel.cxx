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

using namespace css;

namespace sw::sidebar
{
VclPtr<vcl::Window>
WriterInspectorTextPanel::Create(vcl::Window* pParent,
                                 const css::uno::Reference<css::frame::XFrame>& rxFrame,
                                 SfxBindings* pBindings)
{
    if (pParent == nullptr)
        throw lang::IllegalArgumentException(
            "no parent Window given to WriterInspectorTextPanel::Create", nullptr, 0);
    if (!rxFrame.is())
        throw lang::IllegalArgumentException("no XFrame given to WriterInspectorTextPanel::Create",
                                             nullptr, 1);
    if (pBindings == nullptr)
        throw ::com::sun::star::lang::IllegalArgumentException(
            "no SfxBindings given to WriterInspectorPanel::Create", nullptr, 0);

    return VclPtr<WriterInspectorTextPanel>::Create(pParent, rxFrame, pBindings);
}

WriterInspectorTextPanel::WriterInspectorTextPanel(
    vcl::Window* pParent, const css::uno::Reference<css::frame::XFrame>& rxFrame,
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
                css::uno::fromAny(properties->getPropertyValue("CharStyleName"),
                                  &aCurrentCharStyle);
                if (aCurrentCharStyle.isEmpty()) // Default Style is always applied
                    aCurrentCharStyle = "Standard";

                const OUString style_name = aCurrentCharStyle;
                std::vector<OUString> pStyles;
                while (true)
                {
                    uno::Reference<style::XStyle> xProp(xStyleFamily->getByName(aCurrentCharStyle),
                                                        uno::UNO_QUERY);
                    uno::Reference<beans::XPropertySet> xProps(
                        xStyleFamily->getByName(aCurrentCharStyle), uno::UNO_QUERY);
                    OUString aParentCharStyle = xProp->getParentStyle();
                    xProps->getPropertyValue("DisplayName") >>= aDisplayName;
                    pStyles.push_back(aDisplayName);
                    if (aParentCharStyle.isEmpty())
                        break;
                    aCurrentCharStyle = aParentCharStyle;
                }
                for (auto itr = pStyles.rbegin(); itr != pStyles.rend(); ++itr)
                {
                    store.push_back(*itr); // Top Parent goes first...current style goes last
                }
                uno::Reference<beans::XPropertySet> xPropm(xStyleFamily->getByName(style_name),
                                                           uno::UNO_QUERY);
                uno::Any aAny;

                if (WriterInspectorTextPanel::GetPropertyValues(aAny, xPropm, "CharFontName"))
                {
                    OUString aFontName;
                    if (aAny >>= aFontName)
                    {
                        store.push_back("  FONT NAME            " + aFontName);
                    }
                }
                if (WriterInspectorTextPanel::GetPropertyValues(aAny, xPropm, "CharHeight"))
                {
                    float aCharHeight;
                    if (aAny >>= aCharHeight)
                    {
                        store.push_back("  FONT HEIGHT          " + OUString::number(aCharHeight));
                    }
                }
                if (WriterInspectorTextPanel::GetPropertyValues(aAny, xPropm, "CharWeight"))
                {
                    float aCharWeight;
                    if (aAny >>= aCharWeight)
                    {
                        if (aCharWeight > 100)
                            store.push_back("  FONT WEIGHT           Bold");
                        else
                            store.push_back("  FONT WEIGHT           normal");
                    }
                }
                if (WriterInspectorTextPanel::GetPropertyValues(aAny, xPropm, "CharPosture"))
                {
                    awt::FontSlant aCharPosture;
                    if (aAny >>= aCharPosture)
                    {
                        if (aCharPosture == awt::FontSlant_ITALIC)
                            store.push_back("  FONT POSTURE           Italic");
                        else
                            store.push_back("  FONT POSTURE           normal");
                    }
                }
                if (WriterInspectorTextPanel::GetPropertyValues(aAny, xPropm, "CharFontPitch"))
                {
                    short aCharPitch;
                    if (aAny >>= aCharPitch)
                    {
                        store.push_back("  FONT PITCH           " + OUString::number(aCharPitch));
                    }
                }
                if (WriterInspectorTextPanel::GetPropertyValues(aAny, xPropm, "CharColor"))
                {
                    long aColor;
                    if (aAny >>= aColor)
                    {
                        // Is there a method to change hex value to string colour names ?
                        store.push_back("  FONT COLOR           " + OUString::number(aColor));
                    }
                }
            }
        }
        break;
    }
    InspectorTextPanel::updateEntries(store);
}

bool WriterInspectorTextPanel::GetPropertyValues(
    uno::Any& rAny, const uno::Reference<beans::XPropertySet>& rXPropSet, const OUString& rString)
{
    bool bRetValue = true;
    try
    {
        rAny = rXPropSet->getPropertyValue(rString);
        if (!rAny.hasValue())
            bRetValue = false;
    }
    catch (const uno::Exception&)
    {
        bRetValue = false;
    }
    return bRetValue;
}

} // end of namespace svx::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
