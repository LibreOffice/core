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

#include <rsc/rsc-vcl-shared-types.hxx>
#include <vcl/image.hxx>
#include <vcl/keycod.hxx>
#include <vcl/menu.hxx>

#include <com/sun/star/i18n/XCharacterClassification.hpp>

#include <vector>

class SalMenuItem;

struct MenuItemData
{
    sal_uInt16      nId;                    // SV Id
    MenuItemType    eType;                  // MenuItem-Type
    MenuItemBits    nBits;                  // MenuItem-Bits
    VclPtr<Menu>    pSubMenu;               // Pointer to SubMenu
    VclPtr<Menu>    pAutoSubMenu;           // Pointer to SubMenu from Resource
    OUString        aText;                  // Menu-Text
    OUString        aHelpText;              // Help-String
    OUString        aTipHelpText;           // TipHelp-String (eg, expanded filenames)
    OUString        aCommandStr;            // CommandString
    OUString        aHelpCommandStr;        // Help command string (to reference external help)
    OString         sIdent;
    OString         aHelpId;                // Help-Id
    sal_uLong       nUserValue;             // User value
    MenuUserDataReleaseFunction aUserValueReleaseFunc;   // called when MenuItemData is destroyed
    Image           aImage;                 // Image
    vcl::KeyCode    aAccelKey;              // Accelerator-Key
    bool            bChecked;               // Checked
    bool            bEnabled;               // Enabled
    bool            bVisible;               // Visible (note: this flag will not override MenuFlags::HideDisabledEntries when true)
    bool            bIsTemporary;           // Temporary inserted ('No selection possible')
    Size            aSz;                    // only temporarily valid
    OUString        aAccessibleName;        // accessible name

    SalMenuItem*    pSalMenuItem;           // access to native menu

    MenuItemData()
        : nId(0)
        , eType(MenuItemType::DONTKNOW)
        , nBits(MenuItemBits::NONE)
        , pSubMenu(nullptr)
        , pAutoSubMenu(nullptr)
        , nUserValue(0)
        , aUserValueReleaseFunc(nullptr)
        , bChecked(false)
        , bEnabled(false)
        , bVisible(false)
        , bIsTemporary(false)
        , pSalMenuItem(nullptr)
    {
    }
    MenuItemData( const OUString& rStr, const Image& rImage )
        : nId(0)
        , eType(MenuItemType::DONTKNOW)
        , nBits(MenuItemBits::NONE)
        , pSubMenu(nullptr)
        , pAutoSubMenu(nullptr)
        , aText(rStr)
        , nUserValue(0)
        , aUserValueReleaseFunc(nullptr)
        , aImage(rImage)
        , bChecked(false)
        , bEnabled(false)
        , bVisible(false)
        , bIsTemporary(false)
        , pSalMenuItem(nullptr)
    {
    }
    ~MenuItemData();
    bool HasCheck() const
    {
        return bChecked || ( nBits & ( MenuItemBits::RADIOCHECK | MenuItemBits::CHECKABLE | MenuItemBits::AUTOCHECK ) );
    }
};

class MenuItemList
{
private:
    ::std::vector< MenuItemData* > maItemList;

public:
                    MenuItemList() {}
                    ~MenuItemList();

    MenuItemData*   InsertMenuItem(
                        sal_uInt16 nId,
                        MenuItemType eType,
                        MenuItemBits nBits,
                        const OUString& rStr,
                        const Image& rImage,
                        Menu* pMenu,
                        size_t nPos,
                        const OString &rIdent
                    );
    MenuItemData*   InsertSeparator(const OString &rIdent, size_t nPos);
    void            RemoveMenuItem( size_t nPos );
    bool            PeelSeparators( bool bLastOneToo = false );

    MenuItemData*   GetData( sal_uInt16 nSVId, size_t& rPos ) const;
    MenuItemData*   GetData( sal_uInt16 nSVId ) const
                    {
                        size_t nTemp;
                        return GetData( nSVId, nTemp );
                    }
    MenuItemData*   GetDataFromPos( size_t nPos ) const
                    {
                        return ( nPos < maItemList.size() ) ? maItemList[ nPos ] : nullptr;
                    }

    MenuItemData*   SearchItem(
                        sal_Unicode cSelectChar,
                        vcl::KeyCode aKeyCode,
                        sal_uInt16& rPos,
                        sal_uInt16& nDuplicates,
                        sal_uInt16 nCurrentPos
                    ) const;
    size_t          GetItemCount( sal_Unicode cSelectChar ) const;
    size_t          GetItemCount( vcl::KeyCode aKeyCode ) const;
    size_t          size()
                    {
                        return maItemList.size();
                    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
