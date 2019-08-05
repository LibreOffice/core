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

#ifndef INCLUDED_VCL_NOTEBOOKBARADDONSMERGER_HXX
#define INCLUDED_VCL_NOTEBOOKBARADDONSMERGER_HXX

#include <vcl/dllapi.h>
#include <vcl/window.hxx>
#include <vcl/vclptr.hxx>
#include <vcl/image.hxx>
#include <vcl/menu.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/uno/Sequence.h>
#include <memory>
#include <vector>

class PopupMenu;

struct ExtensionPackingData
{
    bool m_bVerticalOrient;
    sal_Int32 m_nPosition;
    ExtensionPackingData(bool bVerticalOrient = false)
        : m_bVerticalOrient(bVerticalOrient)
        , m_nPosition(-1)
    {
    }
};

struct ExtensionWinAndId
{
    OString m_sID;
    VclPtr<vcl::Window> m_pWindow;
    ExtensionPackingData m_aPackingData;
    ExtensionWinAndId(const OString& rId, vcl::Window* pWindow, bool bVertical)
        : m_sID(rId)
        , m_pWindow(pWindow)
        , m_aPackingData(bVertical)
    {
    }
};

struct NotebookBarAddonsItem
{
    Image aImage;
    std::vector<Image> aImageValues;
    std::vector<css::uno::Sequence<css::uno::Sequence<css::beans::PropertyValue>>> aAddonValues;
};

struct AddonsParams
{
    OUString sImageId;
    OUString sControlType;
    sal_uInt16 nWidth;
};

struct AddonNotebookBarItem
{
    OUString sCommandURL;
    OUString sLabel;
    OUString sImageIdentifier;
    OUString sTarget;
    OUString sContext;
    OUString sControlType;
    sal_uInt16 nWidth;
    OUString sStyle;
};

class NotebookBarAddonsMerger
{
public:
    NotebookBarAddonsMerger();
    ~NotebookBarAddonsMerger();
    static std::vector<ExtensionWinAndId>
    MergeNotebookBarAddons(vcl::Window* pParent,
                           const css::uno::Reference<css::frame::XFrame>& rFrame,
                           const NotebookBarAddonsItem& aNotebookBarAddonsItem);
    static void MergeNotebookBarMenuAddons(PopupMenu* pPopupMenu, sal_Int16 nItemId,
                                           const OString& sItemIdName,
                                           NotebookBarAddonsItem& aNotebookBarAddonsItem);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
