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

#include <SvxConfigPageHelper.hxx>

#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/ui/ImageType.hpp>
#include <com/sun/star/ui/ItemType.hpp>

#include <comphelper/propertyvalue.hxx>
#include <comphelper/random.hxx>
#include <svtools/imgdef.hxx>
#include <svtools/miscopt.hxx>

static sal_Int16 theImageType = css::ui::ImageType::COLOR_NORMAL | css::ui::ImageType::SIZE_DEFAULT;

void SvxConfigPageHelper::RemoveEntry(SvxEntries* pEntries, SvxConfigEntry const* pChildEntry)
{
    SvxEntries::iterator iter = pEntries->begin();

    while (iter != pEntries->end())
    {
        if (pChildEntry == *iter)
        {
            pEntries->erase(iter);
            break;
        }
        ++iter;
    }
}

OUString SvxConfigPageHelper::replaceSaveInName(const OUString& rMessage,
                                                std::u16string_view rSaveInName)
{
    OUString name = rMessage.replaceFirst("%SAVE IN SELECTION%", rSaveInName);

    return name;
}

OUString SvxConfigPageHelper::stripHotKey(const OUString& str) { return str.replaceFirst("~", ""); }

OUString SvxConfigPageHelper::replaceSixteen(const OUString& str, sal_Int32 nReplacement)
{
    return str.replaceAll(OUString::number(16), OUString::number(nReplacement));
}

sal_Int16 SvxConfigPageHelper::GetImageType() { return theImageType; }

void SvxConfigPageHelper::InitImageType()
{
    theImageType = css::ui::ImageType::COLOR_NORMAL | css::ui::ImageType::SIZE_DEFAULT;

    if (SvtMiscOptions::GetCurrentSymbolsSize() == SFX_SYMBOLS_SIZE_LARGE)
    {
        theImageType |= css::ui::ImageType::SIZE_LARGE;
    }
    else if (SvtMiscOptions::GetCurrentSymbolsSize() == SFX_SYMBOLS_SIZE_32)
    {
        theImageType |= css::ui::ImageType::SIZE_32;
    }
}

css::uno::Reference<css::graphic::XGraphic>
SvxConfigPageHelper::GetGraphic(const css::uno::Reference<css::ui::XImageManager>& xImageManager,
                                const OUString& rCommandURL)
{
    css::uno::Reference<css::graphic::XGraphic> result;

    if (xImageManager.is())
    {
        // TODO handle large graphics
        css::uno::Sequence<css::uno::Reference<css::graphic::XGraphic>> aGraphicSeq;

        css::uno::Sequence<OUString> aImageCmdSeq{ rCommandURL };

        try
        {
            aGraphicSeq = xImageManager->getImages(GetImageType(), aImageCmdSeq);

            if (aGraphicSeq.hasElements())
            {
                result = aGraphicSeq[0];
            }
        }
        catch (css::uno::Exception&)
        {
            // will return empty XGraphic
        }
    }

    return result;
}

OUString SvxConfigPageHelper::generateCustomName(const OUString& prefix, SvxEntries* entries,
                                                 sal_Int32 suffix /*= 1*/)
{
    OUString name;
    sal_Int32 pos = 0;

    // find and replace the %n placeholder in the prefix string
    name = prefix.replaceFirst("%n", OUString::number(suffix), &pos);

    if (pos == -1)
    {
        // no placeholder found so just append the suffix
        name += OUString::number(suffix);
    }

    if (!entries)
        return name;

    // now check if there is an already existing entry with this name
    bool bFoundEntry = false;
    for (auto const& entry : *entries)
    {
        if (name.equals(entry->GetName()))
        {
            bFoundEntry = true;
            break;
        }
    }

    if (bFoundEntry)
    {
        // name already exists so try the next number up
        return generateCustomName(prefix, entries, ++suffix);
    }

    return name;
}

OUString SvxConfigPageHelper::generateCustomMenuURL(SvxEntries* entries, sal_Int32 suffix /*= 1*/)
{
    OUString url = "vnd.openoffice.org:CustomMenu" + OUString::number(suffix);
    if (!entries)
        return url;

    // now check is there is an already existing entry with this url
    bool bFoundEntry = false;
    for (auto const& entry : *entries)
    {
        if (url.equals(entry->GetCommand()))
        {
            bFoundEntry = true;
            break;
        }
    }

    if (bFoundEntry)
    {
        // url already exists so try the next number up
        return generateCustomMenuURL(entries, ++suffix);
    }

    return url;
}

sal_uInt32 SvxConfigPageHelper::generateRandomValue()
{
    return comphelper::rng::uniform_uint_distribution(0, std::numeric_limits<unsigned int>::max());
}

OUString SvxConfigPageHelper::generateCustomURL(SvxEntries* entries)
{
    OUString url = OUString::Concat(ITEM_TOOLBAR_URL) + CUSTOM_TOOLBAR_STR +
                   // use a random number to minimize possible clash with existing custom toolbars
                   OUString::number(generateRandomValue(), 16);

    // now check is there is an already existing entry with this url
    bool bFoundEntry = false;
    for (auto const& entry : *entries)
    {
        if (url.equals(entry->GetCommand()))
        {
            bFoundEntry = true;
            break;
        }
    }

    if (bFoundEntry)
    {
        // url already exists so try the next number up
        return generateCustomURL(entries);
    }

    return url;
}

OUString SvxConfigPageHelper::GetModuleName(std::u16string_view aModuleId)
{
    if (aModuleId == u"com.sun.star.text.TextDocument"
        || aModuleId == u"com.sun.star.text.GlobalDocument")
        return u"Writer"_ustr;
    else if (aModuleId == u"com.sun.star.text.WebDocument")
        return u"Writer/Web"_ustr;
    else if (aModuleId == u"com.sun.star.drawing.DrawingDocument")
        return u"Draw"_ustr;
    else if (aModuleId == u"com.sun.star.presentation.PresentationDocument")
        return u"Impress"_ustr;
    else if (aModuleId == u"com.sun.star.sheet.SpreadsheetDocument")
        return u"Calc"_ustr;
    else if (aModuleId == u"com.sun.star.script.BasicIDE")
        return u"Basic"_ustr;
    else if (aModuleId == u"com.sun.star.formula.FormulaProperties")
        return u"Math"_ustr;
    else if (aModuleId == u"com.sun.star.sdb.RelationDesign")
        return u"Relation Design"_ustr;
    else if (aModuleId == u"com.sun.star.sdb.QueryDesign")
        return u"Query Design"_ustr;
    else if (aModuleId == u"com.sun.star.sdb.TableDesign")
        return u"Table Design"_ustr;
    else if (aModuleId == u"com.sun.star.sdb.DataSourceBrowser")
        return u"Data Source Browser"_ustr;
    else if (aModuleId == u"com.sun.star.sdb.DatabaseDocument")
        return u"Database"_ustr;

    return OUString();
}

OUString SvxConfigPageHelper::GetUIModuleName(
    const OUString& aModuleId,
    const css::uno::Reference<css::frame::XModuleManager2>& rModuleManager)
{
    assert(rModuleManager.is());

    OUString aModuleUIName;

    try
    {
        css::uno::Any a = rModuleManager->getByName(aModuleId);
        css::uno::Sequence<css::beans::PropertyValue> aSeq;

        if (a >>= aSeq)
        {
            for (css::beans::PropertyValue const& rProp : aSeq)
            {
                if (rProp.Name == "ooSetupFactoryUIName")
                {
                    rProp.Value >>= aModuleUIName;
                    break;
                }
            }
        }
    }
    catch (css::uno::RuntimeException&)
    {
        throw;
    }
    catch (css::uno::Exception&)
    {
    }

    if (aModuleUIName.isEmpty())
        aModuleUIName = GetModuleName(aModuleId);

    return aModuleUIName;
}

bool SvxConfigPageHelper::GetMenuItemData(
    const css::uno::Reference<css::container::XIndexAccess>& rItemContainer, sal_Int32 nIndex,
    OUString& rCommandURL, OUString& rLabel, sal_uInt16& rType, sal_Int32& rStyle,
    css::uno::Reference<css::container::XIndexAccess>& rSubMenu)
{
    try
    {
        css::uno::Sequence<css::beans::PropertyValue> aProps;
        if (rItemContainer->getByIndex(nIndex) >>= aProps)
        {
            for (css::beans::PropertyValue const& rProp : aProps)
            {
                if (rProp.Name == ITEM_DESCRIPTOR_COMMANDURL)
                {
                    rProp.Value >>= rCommandURL;
                }
                else if (rProp.Name == ITEM_DESCRIPTOR_CONTAINER)
                {
                    rProp.Value >>= rSubMenu;
                }
                else if (rProp.Name == ITEM_DESCRIPTOR_STYLE)
                {
                    rProp.Value >>= rStyle;
                }
                else if (rProp.Name == ITEM_DESCRIPTOR_LABEL)
                {
                    rProp.Value >>= rLabel;
                }
                else if (rProp.Name == ITEM_DESCRIPTOR_TYPE)
                {
                    rProp.Value >>= rType;
                }
            }

            return true;
        }
    }
    catch (css::lang::IndexOutOfBoundsException&)
    {
    }

    return false;
}

bool SvxConfigPageHelper::GetToolbarItemData(
    const css::uno::Reference<css::container::XIndexAccess>& rItemContainer, sal_Int32 nIndex,
    OUString& rCommandURL, OUString& rLabel, sal_uInt16& rType, bool& rIsVisible, sal_Int32& rStyle)
{
    try
    {
        css::uno::Sequence<css::beans::PropertyValue> aProps;
        if (rItemContainer->getByIndex(nIndex) >>= aProps)
        {
            for (css::beans::PropertyValue const& rProp : aProps)
            {
                if (rProp.Name == ITEM_DESCRIPTOR_COMMANDURL)
                {
                    rProp.Value >>= rCommandURL;
                }
                else if (rProp.Name == ITEM_DESCRIPTOR_STYLE)
                {
                    rProp.Value >>= rStyle;
                }
                else if (rProp.Name == ITEM_DESCRIPTOR_LABEL)
                {
                    rProp.Value >>= rLabel;
                }
                else if (rProp.Name == ITEM_DESCRIPTOR_TYPE)
                {
                    rProp.Value >>= rType;
                }
                else if (rProp.Name == ITEM_DESCRIPTOR_ISVISIBLE)
                {
                    rProp.Value >>= rIsVisible;
                }
            }

            return true;
        }
    }
    catch (css::lang::IndexOutOfBoundsException&)
    {
    }

    return false;
}

css::uno::Sequence<css::beans::PropertyValue>
SvxConfigPageHelper::ConvertSvxConfigEntry(const SvxConfigEntry* pEntry)
{
    // If the name has not been changed, then the label can be stored
    // as an empty string.
    // It will be initialised again later using the command to label map.
    OUString sLabel;
    if (pEntry->HasChangedName() || pEntry->GetCommand().isEmpty())
        sLabel = pEntry->GetName();

    css::uno::Sequence<css::beans::PropertyValue> aPropSeq{
        comphelper::makePropertyValue(ITEM_DESCRIPTOR_COMMANDURL, pEntry->GetCommand()),
        comphelper::makePropertyValue(ITEM_DESCRIPTOR_TYPE, css::ui::ItemType::DEFAULT),
        comphelper::makePropertyValue(ITEM_DESCRIPTOR_LABEL, sLabel),
        comphelper::makePropertyValue(ITEM_DESCRIPTOR_STYLE,
                                      static_cast<sal_Int16>(pEntry->GetStyle()))
    };

    return aPropSeq;
}

css::uno::Sequence<css::beans::PropertyValue>
SvxConfigPageHelper::ConvertToolbarEntry(const SvxConfigEntry* pEntry)
{
    // If the name has not been changed, then the label can be stored
    // as an empty string.
    // It will be initialised again later using the command to label map.
    OUString sLabel;
    if (pEntry->HasChangedName() || pEntry->GetCommand().isEmpty())
        sLabel = pEntry->GetName();

    css::uno::Sequence<css::beans::PropertyValue> aPropSeq{
        comphelper::makePropertyValue(ITEM_DESCRIPTOR_COMMANDURL, pEntry->GetCommand()),
        comphelper::makePropertyValue(ITEM_DESCRIPTOR_TYPE, css::ui::ItemType::DEFAULT),
        comphelper::makePropertyValue(ITEM_DESCRIPTOR_LABEL, sLabel),
        comphelper::makePropertyValue(ITEM_DESCRIPTOR_ISVISIBLE, pEntry->IsVisible()),
        comphelper::makePropertyValue(ITEM_DESCRIPTOR_STYLE,
                                      static_cast<sal_Int16>(pEntry->GetStyle()))
    };

    return aPropSeq;
}

bool SvxConfigPageHelper::EntrySort(SvxConfigEntry const* a, SvxConfigEntry const* b)
{
    return a->GetName().compareTo(b->GetName()) < 0;
}

bool SvxConfigPageHelper::SvxConfigEntryModified(SvxConfigEntry const* pEntry)
{
    SvxEntries* pEntries = pEntry->GetEntries();
    if (!pEntries)
        return false;

    for (const auto& entry : *pEntries)
    {
        if (entry->IsModified() || SvxConfigEntryModified(entry))
            return true;
    }
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
