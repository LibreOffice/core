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

#pragma once

#include <com/sun/star/frame/XModuleManager2.hpp>

#include "cfg.hxx"

class SvxConfigPageHelper
{
public:
    static void RemoveEntry(SvxEntries* pEntries, SvxConfigEntry const* pChildEntry);

    static OUString replaceSaveInName(const OUString& rMessage, std::u16string_view rSaveInName);
    static OUString stripHotKey(const OUString& str);
    static OUString replaceSixteen(const OUString& str, sal_Int32 nReplacement);

    static sal_Int16 GetImageType();
    static void InitImageType();
    static css::uno::Reference<css::graphic::XGraphic>
    GetGraphic(const css::uno::Reference<css::ui::XImageManager>& xImageManager,
               const OUString& rCommandURL);

    static OUString generateCustomName(const OUString& prefix, SvxEntries* entries,
                                       sal_Int32 suffix = 1);
    static OUString generateCustomMenuURL(SvxEntries* entries, sal_Int32 suffix = 1);
    static sal_uInt32 generateRandomValue();
    /**
        Generates a custom resource URL for a new toolbar.
        Typically something like: private:resource/toolbar/custom_toolbar_########
        The last 8 letters are randomly generated alphanumeric characters.
    */
    static OUString generateCustomURL(SvxEntries* entries);

    static OUString GetModuleName(const OUString& aModuleId);
    static OUString
    GetUIModuleName(const OUString& aModuleId,
                    const css::uno::Reference<css::frame::XModuleManager2>& rModuleManager);

    static bool
    GetMenuItemData(const css::uno::Reference<css::container::XIndexAccess>& rItemContainer,
                    sal_Int32 nIndex, OUString& rCommandURL, OUString& rLabel, sal_uInt16& rType,
                    sal_Int32& rStyle, css::uno::Reference<css::container::XIndexAccess>& rSubMenu);
    static bool
    GetToolbarItemData(const css::uno::Reference<css::container::XIndexAccess>& rItemContainer,
                       sal_Int32 nIndex, OUString& rCommandURL, OUString& rLabel, sal_uInt16& rType,
                       bool& rIsVisible, sal_Int32& rStyle);

    static css::uno::Sequence<css::beans::PropertyValue>
    ConvertSvxConfigEntry(const SvxConfigEntry* pEntry);
    static css::uno::Sequence<css::beans::PropertyValue>
    ConvertToolbarEntry(const SvxConfigEntry* pEntry);

    static bool EntrySort(SvxConfigEntry const* a, SvxConfigEntry const* b);

    static bool SvxConfigEntryModified(SvxConfigEntry const* pEntry);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
