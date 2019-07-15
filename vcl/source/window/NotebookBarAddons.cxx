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

#include <vcl/NotebookBarAddons.hxx>
#include <svtools/imagemgr.hxx>
#include <svtools/miscopt.hxx>
#include <svtools/toolboxcontroller.hxx>
#include <vcl/commandinfoprovider.hxx>
#include <vcl/toolbox.hxx>
#include <rtl/ustring.hxx>

static const char MERGE_NOTEBOOKBAR_URL[] = "URL";
static const char MERGE_NOTEBOOKBAR_TITLE[] = "Title";
static const char MERGE_NOTEBOOKBAR_IMAGEID[] = "ImageIdentifier";
static const char MERGE_NOTEBOOKBAR_CONTEXT[] = "Context";
static const char MERGE_NOTEBOOKBAR_TARGET[] = "Target";
static const char MERGE_NOTEBOOKBAR_CONTROLTYPE[] = "ControlType";
static const char MERGE_NOTEBOOKBAR_WIDTH[] = "Width";
static const char MERGE_NOTEBOOKBAR_STYLE[] = "Style";

NotebookBarAddons::NotebookBarAddons(vcl::Window* pParent,
                                     const css::uno::Reference<css::frame::XFrame>& m_xFrame,
                                     AddonsNotebookBarItems& aAddonsNotebookBarItems)
{
    MergeNotebookBarAddons(pParent, m_xFrame, aAddonsNotebookBarItems);
}

void NotebookBarAddons::MergeNotebookBarAddons(
    vcl::Window* pParent, const css::uno::Reference<css::frame::XFrame>& m_xFrame,
    AddonsNotebookBarItems& aAddonsNotebookBarItems)
{
    css::uno::Sequence<css::uno::Sequence<css::beans::PropertyValue>> aExtension;
    for (unsigned int nIdx = 0; nIdx < aAddonsNotebookBarItems.size(); nIdx++)
    {
        aExtension = aAddonsNotebookBarItems[nIdx];

        for (int nSecIdx = 0; nSecIdx < aExtension.getLength(); nSecIdx++)
        {
            AddonNotebookBarItem aAddonNotebookBarItem;
            const css::uno::Sequence<css::beans::PropertyValue> pExtension = aExtension[nSecIdx];
            for (int nRes = 0; nRes < pExtension.getLength(); nRes++)
            {
                if (pExtension[nRes].Name == MERGE_NOTEBOOKBAR_URL)
                    pExtension[nRes].Value >>= aAddonNotebookBarItem.sCommandURL;
                else if (pExtension[nRes].Name == MERGE_NOTEBOOKBAR_TITLE)
                    pExtension[nRes].Value >>= aAddonNotebookBarItem.sLabel;
                else if (pExtension[nRes].Name == MERGE_NOTEBOOKBAR_IMAGEID)
                    pExtension[nRes].Value >>= aAddonNotebookBarItem.sImageIdentifier;
                else if (pExtension[nRes].Name == MERGE_NOTEBOOKBAR_CONTEXT)
                    pExtension[nRes].Value >>= aAddonNotebookBarItem.sContext;
                else if (pExtension[nRes].Name == MERGE_NOTEBOOKBAR_TARGET)
                    pExtension[nRes].Value >>= aAddonNotebookBarItem.sTarget;
                else if (pExtension[nRes].Name == MERGE_NOTEBOOKBAR_CONTROLTYPE)
                    pExtension[nRes].Value >>= aAddonNotebookBarItem.sControlType;
                else if (pExtension[nRes].Name == MERGE_NOTEBOOKBAR_WIDTH)
                    pExtension[nRes].Value >>= aAddonNotebookBarItem.nWidth;
                else if (pExtension[nRes].Name == MERGE_NOTEBOOKBAR_STYLE)
                    pExtension[nRes].Value >>= aAddonNotebookBarItem.sStyle;
            }

            sal_uInt16 nItemId = 0;
            ToolBox* pToolbar = dynamic_cast<ToolBox*>(pParent);
            if (pToolbar)
            {
                Size aSize(0, 0);
                pToolbar->InsertItem(aAddonNotebookBarItem.sCommandURL, m_xFrame,
                                     ToolBoxItemBits::NONE, aSize);
                nItemId = pToolbar->GetItemId(aAddonNotebookBarItem.sCommandURL);
                pToolbar->SetItemCommand(nItemId, aAddonNotebookBarItem.sCommandURL);
                pToolbar->SetQuickHelpText(nItemId, aAddonNotebookBarItem.sLabel);
                pToolbar->SetItemText(nItemId, aAddonNotebookBarItem.sLabel);

                Image aImage = vcl::CommandInfoProvider::GetImageForCommand(
                    aAddonNotebookBarItem.sImageIdentifier, m_xFrame, pToolbar->GetImageSize());
                if (!!aImage)
                    pToolbar->SetItemImage(nItemId, aImage);
            }
            pToolbar->InsertSeparator();
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */