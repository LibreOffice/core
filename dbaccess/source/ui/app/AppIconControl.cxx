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

#include "AppIconControl.hxx"
#include <core_resource.hxx>
#include <strings.hrc>
#include <bitmaps.hlst>
#include <vcl/bitmapex.hxx>
#include <vcl/event.hxx>
#include <vcl/i18nhelp.hxx>
#include <vcl/mnemonic.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <callbacks.hxx>
#include <AppElementType.hxx>

using namespace ::dbaui;

OApplicationIconControl::OApplicationIconControl(std::unique_ptr<weld::ScrolledWindow> xScroll)
    : SfxThumbnailView(std::move(xScroll), nullptr)
//    , DropTargetHelper(nullptr)
    , m_pActionListener(nullptr)
    , m_nMaxWidth(0)
    , m_nMaxHeight(0)
{
}

void OApplicationIconControl::Fill()
{
    static const struct CategoryDescriptor
    {
        const char* pLabelResId;
        ElementType eType;
        const char* aImageResId;
    }   aCategories[] = {
        { RID_STR_TABLES_CONTAINER,     E_TABLE,    BMP_TABLEFOLDER_TREE_L  },
        { RID_STR_QUERIES_CONTAINER,    E_QUERY,    BMP_QUERYFOLDER_TREE_L  },
        { RID_STR_FORMS_CONTAINER,      E_FORM,     BMP_FORMFOLDER_TREE_L   },
        { RID_STR_REPORTS_CONTAINER,    E_REPORT,   BMP_REPORTFOLDER_TREE_L }
    };

    for (const CategoryDescriptor& aCategorie : aCategories)
    {
        std::unique_ptr<ThumbnailViewItem> xItem(new ThumbnailViewItem(*this, aCategorie.eType));
        xItem->mbBorder = false;
        xItem->maPreview1 = BitmapEx(OUString::createFromAscii(aCategorie.aImageResId));
        const Size& rSize = xItem->maPreview1.GetSizePixel();
        m_nMaxWidth = std::max(m_nMaxWidth, rSize.Width());
        m_nMaxHeight = std::max(m_nMaxHeight, rSize.Height());
        xItem->maTitle = DBA_RES(aCategorie.pLabelResId);
        AppendItem(std::move(xItem));
    }

    setItemDimensions(m_nMaxWidth, m_nMaxHeight, m_nMaxHeight, 0);

    set_size_request(m_nMaxWidth, -1);
}

ElementType OApplicationIconControl::GetSelectedItem() const
{
    for (const auto& rItem : mItemList)
    {
        if (!rItem->mbSelected)
            continue;
        return static_cast<ElementType>(rItem->mnId);
    }
    return E_NONE;
}

void OApplicationIconControl::createIconAutoMnemonics(MnemonicGenerator& rMnemonics)
{
    for (const auto& rItem : mItemList)
        rMnemonics.RegisterMnemonic(rItem->maTitle);

    // exchange texts with generated mnemonics
    for (auto& rItem : mItemList)
        rItem->maTitle = rMnemonics.CreateMnemonic(rItem->maTitle);
}

void OApplicationIconControl::Resize()
{
    setItemDimensions(GetOutputSizePixel().Width(), m_nMaxHeight, m_nMaxHeight, 0);
    SfxThumbnailView::Resize();
}

bool OApplicationIconControl::IsMnemonicChar(sal_Unicode cChar, ElementType& rType) const
{
    bool bRet = false;

    const vcl::I18nHelper& rI18nHelper = Application::GetSettings().GetUILocaleI18nHelper();
    for (const auto& rItem : mItemList)
    {
        if (rI18nHelper.MatchMnemonic(rItem->maTitle, cChar))
        {
            bRet = true;
            rType = static_cast<ElementType>(rItem->mnId);
            break;
        }
    }

    return bRet;
}

bool OApplicationIconControl::DoKeyShortCut(const KeyEvent& rKEvt)
{
    bool bMod2 = rKEvt.GetKeyCode().IsMod2();
    sal_Unicode cChar = rKEvt.GetCharCode();
    ElementType eType(E_NONE);
    if (bMod2 && cChar && IsMnemonicChar(cChar, eType))
    {
        // shortcut is clicked
        deselectItems();
        SelectItem(eType);
        return true;
    }

    return false;
}

bool OApplicationIconControl::KeyInput(const KeyEvent& rKEvt)
{
    return DoKeyShortCut(rKEvt) || SfxThumbnailView::KeyInput(rKEvt);
}

OApplicationIconControl::~OApplicationIconControl()
{
#if 0
    DropTargetHelper::dispose();
#endif
}

#if 0
sal_Int8 OApplicationIconControl::AcceptDrop( const AcceptDropEvent& _rEvt )
{
    sal_Int8 nDropOption = DND_ACTION_NONE;
    if ( m_pActionListener )
    {

        SvxIconChoiceCtrlEntry* pEntry = GetEntry(_rEvt.maPosPixel);
        if ( pEntry )
        {
            SetCursor(pEntry);
            nDropOption = m_pActionListener->queryDrop( _rEvt, GetDataFlavorExVector() );
        }
    }
    return nDropOption;
}

sal_Int8 OApplicationIconControl::ExecuteDrop( const ExecuteDropEvent& _rEvt )
{
    if ( m_pActionListener )
        return m_pActionListener->executeDrop( _rEvt );

    return DND_ACTION_NONE;
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
