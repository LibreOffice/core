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

#include <vcl/NotebookBarAddonsMerger.hxx>
#include <vcl/commandinfoprovider.hxx>
#include <vcl/vclenum.hxx>
#include <vcl/toolbox.hxx>
#include <osl/module.hxx>
#include <vcl/OptionalBox.hxx>
#include <sfx2/sidebar/SidebarToolBox.hxx>
#include <vcl/IPrioritable.hxx>

static const char STYLE_TEXT[] = "Text";
static const char STYLE_ICON[] = "Icon";

static const char MERGE_NOTEBOOKBAR_URL[] = "URL";
static const char MERGE_NOTEBOOKBAR_TITLE[] = "Title";
static const char MERGE_NOTEBOOKBAR_IMAGEID[] = "ImageIdentifier";
static const char MERGE_NOTEBOOKBAR_CONTEXT[] = "Context";
static const char MERGE_NOTEBOOKBAR_TARGET[] = "Target";
static const char MERGE_NOTEBOOKBAR_CONTROLTYPE[] = "ControlType";
static const char MERGE_NOTEBOOKBAR_WIDTH[] = "Width";
static const char MERGE_NOTEBOOKBAR_STYLE[] = "Style";

static void GetAddonNotebookBarItem(const css::uno::Sequence<css::beans::PropertyValue>& pExtension,
                                    AddonNotebookBarItem& aAddonNotebookBarItem)
{
    for (int nIdx = 0; nIdx < pExtension.getLength(); nIdx++)
    {
        if (pExtension[nIdx].Name == MERGE_NOTEBOOKBAR_URL)
            pExtension[nIdx].Value >>= aAddonNotebookBarItem.sCommandURL;
        else if (pExtension[nIdx].Name == MERGE_NOTEBOOKBAR_TITLE)
            pExtension[nIdx].Value >>= aAddonNotebookBarItem.sLabel;
        else if (pExtension[nIdx].Name == MERGE_NOTEBOOKBAR_IMAGEID)
            pExtension[nIdx].Value >>= aAddonNotebookBarItem.sImageIdentifier;
        else if (pExtension[nIdx].Name == MERGE_NOTEBOOKBAR_CONTEXT)
            pExtension[nIdx].Value >>= aAddonNotebookBarItem.sContext;
        else if (pExtension[nIdx].Name == MERGE_NOTEBOOKBAR_TARGET)
            pExtension[nIdx].Value >>= aAddonNotebookBarItem.sTarget;
        else if (pExtension[nIdx].Name == MERGE_NOTEBOOKBAR_CONTROLTYPE)
            pExtension[nIdx].Value >>= aAddonNotebookBarItem.sControlType;
        else if (pExtension[nIdx].Name == MERGE_NOTEBOOKBAR_WIDTH)
            pExtension[nIdx].Value >>= aAddonNotebookBarItem.nWidth;
        else if (pExtension[nIdx].Name == MERGE_NOTEBOOKBAR_STYLE)
            pExtension[nIdx].Value >>= aAddonNotebookBarItem.sStyle;
    }
}

NotebookBarAddonsMerger::NotebookBarAddonsMerger() {}

NotebookBarAddonsMerger::~NotebookBarAddonsMerger() {}

void NotebookBarAddonsMerger::MergeNotebookBarAddons(
    vcl::Window* pParent, const customMakeWidget& pFunction,
    const css::uno::Reference<css::frame::XFrame>& m_xFrame,
    const NotebookBarAddonsItem& aNotebookBarAddonsItem, NotebookBarAddonsMerger::stringmap rMap)
{
    std::vector<Image> aImageVec = aNotebookBarAddonsItem.aImageValues;
    unsigned long nIter = 0;
    sal_uInt16 nPriorityIdx = aImageVec.size(), nItemId = 0;

    css::uno::Sequence<css::uno::Sequence<css::beans::PropertyValue>> aExtension;
    for (unsigned long nIdx = 0; nIdx < aNotebookBarAddonsItem.aAddonValues.size(); nIdx++)
    {
        aExtension = aNotebookBarAddonsItem.aAddonValues[nIdx];

        for (int nSecIdx = 0; nSecIdx < aExtension.getLength(); nSecIdx++)
        {
            VclPtr<vcl::Window> pOptionalParent;
            pOptionalParent = VclPtr<OptionalBox>::Create(pParent);
            pOptionalParent->Show();

            vcl::IPrioritable* pPrioritable
                = dynamic_cast<vcl::IPrioritable*>(pOptionalParent.get());
            if (pPrioritable)
                pPrioritable->SetPriority(nPriorityIdx - nIter);

            VclPtr<vcl::Window> pNotebookbarToolBox;
            VclPtr<vcl::Window> pTempParent(pOptionalParent);
            pFunction(pNotebookbarToolBox, pTempParent, rMap);

            ToolBox* pToolbox = dynamic_cast<ToolBox*>(pNotebookbarToolBox.get());
            AddonNotebookBarItem aAddonNotebookBarItem;
            const css::uno::Sequence<css::beans::PropertyValue> pExtension = aExtension[nSecIdx];
            GetAddonNotebookBarItem(pExtension, aAddonNotebookBarItem);

            if (pToolbox)
            {
                Size aSize(0, 0);
                Image sImage;
                pToolbox->InsertItem(aAddonNotebookBarItem.sCommandURL, m_xFrame,
                                     ToolBoxItemBits::NONE, aSize);
                nItemId = pToolbox->GetItemId(aAddonNotebookBarItem.sCommandURL);
                pToolbox->SetItemCommand(nItemId, aAddonNotebookBarItem.sCommandURL);
                pToolbox->SetQuickHelpText(nItemId, aAddonNotebookBarItem.sLabel);

                if (nIter < aImageVec.size())
                {
                    sImage = aImageVec[nIter];
                    if (!sImage)
                    {
                        sImage = vcl::CommandInfoProvider::GetImageForCommand(
                            aAddonNotebookBarItem.sImageIdentifier, m_xFrame);
                    }
                    nIter++;
                }

                if (aAddonNotebookBarItem.sStyle == STYLE_TEXT)
                    pToolbox->SetItemText(nItemId, aAddonNotebookBarItem.sLabel);
                else if (aAddonNotebookBarItem.sStyle == STYLE_ICON)
                    pToolbox->SetItemImage(nItemId, sImage);
                else
                {
                    pToolbox->SetItemText(nItemId, aAddonNotebookBarItem.sLabel);
                    pToolbox->SetItemImage(nItemId, sImage);
                }
                pToolbox->Show();
            }
        }
    }
}

void NotebookBarAddonsMerger::MergeNotebookBarMenuAddons(
    PopupMenu* pPopupMenu, sal_Int16 nItemId, const OString& sItemIdName,
    NotebookBarAddonsItem& aNotebookBarAddonsItem)
{
    std::vector<Image> aImageVec = aNotebookBarAddonsItem.aImageValues;
    unsigned long nIter = 0;
    css::uno::Sequence<css::uno::Sequence<css::beans::PropertyValue>> aExtension;
    for (unsigned long nIdx = 0; nIdx < aNotebookBarAddonsItem.aAddonValues.size(); nIdx++)
    {
        aExtension = aNotebookBarAddonsItem.aAddonValues[nIdx];

        for (int nSecIdx = 0; nSecIdx < aExtension.getLength(); nSecIdx++)
        {
            AddonNotebookBarItem aAddonNotebookBarItem;
            Image sImage;
            MenuItemBits nBits = MenuItemBits::ICON;
            const css::uno::Sequence<css::beans::PropertyValue> pExtension = aExtension[nSecIdx];

            GetAddonNotebookBarItem(pExtension, aAddonNotebookBarItem);

            pPopupMenu->InsertItem(nItemId, aAddonNotebookBarItem.sLabel, nBits, sItemIdName);
            pPopupMenu->SetItemCommand(nItemId, aAddonNotebookBarItem.sCommandURL);

            if (nIter < aImageVec.size())
            {
                sImage = aImageVec[nIter];
                nIter++;
            }
            pPopupMenu->SetItemImage(nItemId, sImage);

            if (nSecIdx == aExtension.getLength() - 1)
                pPopupMenu->InsertSeparator();

            ++nItemId;
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */