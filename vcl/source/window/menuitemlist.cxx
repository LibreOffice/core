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

#include "menuitemlist.hxx"

#include <salframe.hxx>
#include <salinst.hxx>
#include <salmenu.hxx>
#include <svdata.hxx>
#include <vcl/i18nhelp.hxx>
#include <vcl/settings.hxx>
#include <vcl/window.hxx>

using namespace css;
using namespace vcl;

MenuItemData::~MenuItemData()
{
    if (aUserValueReleaseFunc)
        aUserValueReleaseFunc(nUserValue);
    pSalMenuItem.reset();
    pSubMenu.disposeAndClear();
}

MenuItemList::~MenuItemList()
{
}

MenuItemData* MenuItemList::Insert(
    sal_uInt16 nId,
    MenuItemType eType,
    MenuItemBits nBits,
    const OUString& rStr,
    Menu* pMenu,
    size_t nPos,
    const OString &rIdent
)
{
    MenuItemData* pData     = new MenuItemData( rStr );
    pData->nId              = nId;
    pData->sIdent           = rIdent;
    pData->eType            = eType;
    pData->nBits            = nBits;
    pData->pSubMenu         = nullptr;
    pData->nUserValue       = nullptr;
    pData->bChecked         = false;
    pData->bEnabled         = true;
    pData->bVisible         = true;
    pData->bIsTemporary     = false;

    SalItemParams aSalMIData;
    aSalMIData.nId = nId;
    aSalMIData.eType = eType;
    aSalMIData.nBits = nBits;
    aSalMIData.pMenu = pMenu;
    aSalMIData.aText = rStr;

    // Native-support: returns NULL if not supported
    pData->pSalMenuItem = ImplGetSVData()->mpDefInst->CreateMenuItem( aSalMIData );

    if( nPos < maItemList.size() ) {
        maItemList.insert( maItemList.begin() + nPos, std::unique_ptr<MenuItemData>(pData) );
    } else {
        maItemList.emplace_back( pData );
    }
    return pData;
}

void MenuItemList::InsertSeparator(const OString &rIdent, size_t nPos)
{
    MenuItemData* pData     = new MenuItemData;
    pData->nId              = 0;
    pData->sIdent           = rIdent;
    pData->eType            = MenuItemType::SEPARATOR;
    pData->nBits            = MenuItemBits::NONE;
    pData->pSubMenu         = nullptr;
    pData->nUserValue       = nullptr;
    pData->bChecked         = false;
    pData->bEnabled         = true;
    pData->bVisible         = true;
    pData->bIsTemporary     = false;

    SalItemParams aSalMIData;
    aSalMIData.nId = 0;
    aSalMIData.eType = MenuItemType::SEPARATOR;
    aSalMIData.nBits = MenuItemBits::NONE;
    aSalMIData.pMenu = nullptr;
    aSalMIData.aText.clear();
    aSalMIData.aImage = Image();

    // Native-support: returns NULL if not supported
    pData->pSalMenuItem = ImplGetSVData()->mpDefInst->CreateMenuItem( aSalMIData );

    if( nPos < maItemList.size() ) {
        maItemList.insert( maItemList.begin() + nPos, std::unique_ptr<MenuItemData>(pData) );
    } else {
        maItemList.emplace_back( pData );
    }
}

void MenuItemList::Remove( size_t nPos )
{
    if( nPos < maItemList.size() )
    {
        maItemList.erase( maItemList.begin() + nPos );
    }
}

void MenuItemList::Clear()
{
    maItemList.clear();
}

MenuItemData* MenuItemList::GetData( sal_uInt16 nSVId, size_t& rPos ) const
{
    for( size_t i = 0, n = maItemList.size(); i < n; ++i )
    {
        if ( maItemList[ i ]->nId == nSVId )
        {
            rPos = i;
            return maItemList[ i ].get();
        }
    }
    return nullptr;
}

MenuItemData* MenuItemList::GetDataFromSubMenu(sal_uInt16 nSVId) const
{
    for ( size_t i = 0, n = maItemList.size(); i < n; ++i )
    {
        if ( maItemList[i]->pSubMenu
            && maItemList[i]->pSubMenu->GetCurItemId() != 0 ) // if something is selected
            return maItemList[i].get()->pSubMenu->GetItemList()->GetDataFromPos(nSVId - 1);
    }
    return nullptr;
}

MenuItemData* MenuItemList::SearchItem(
    sal_Unicode cSelectChar,
    KeyCode aKeyCode,
    size_t& rPos,
    size_t& nDuplicates,
    size_t nCurrentPos
) const
{
    const vcl::I18nHelper& rI18nHelper = Application::GetSettings().GetUILocaleI18nHelper();

    size_t nListCount = maItemList.size();

    // try character code first
    nDuplicates = GetItemCount( cSelectChar );  // return number of duplicates
    if( nDuplicates )
    {
        for ( rPos = 0; rPos < nListCount; rPos++)
        {
            MenuItemData* pData = maItemList[ rPos ].get();
            if ( pData->bEnabled && rI18nHelper.MatchMnemonic( pData->aText, cSelectChar ) )
            {
                if( nDuplicates > 1 && rPos == nCurrentPos )
                    continue;   // select next entry with the same mnemonic
                else
                    return pData;
            }
        }
    }

    // nothing found, try keycode instead
    nDuplicates = GetItemCount( aKeyCode ); // return number of duplicates

    if( nDuplicates )
    {
        char ascii = 0;
        if( aKeyCode.GetCode() >= KEY_A && aKeyCode.GetCode() <= KEY_Z )
            ascii = sal::static_int_cast<char>('A' + (aKeyCode.GetCode() - KEY_A));

        for ( rPos = 0; rPos < nListCount; rPos++)
        {
            MenuItemData* pData = maItemList[ rPos ].get();
            if ( pData->bEnabled )
            {
                sal_Int32 n = pData->aText.indexOf('~');
                if ( n != -1 )
                {
                    KeyCode nKeyCode;
                    sal_Unicode nUnicode = pData->aText[n+1];
                    vcl::Window* pDefWindow = ImplGetDefaultWindow();
                    if(  (  pDefWindow
                         && pDefWindow->ImplGetFrame()->MapUnicodeToKeyCode( nUnicode,
                             Application::GetSettings().GetUILanguageTag().getLanguageType(), nKeyCode )
                         && aKeyCode.GetCode() == nKeyCode.GetCode()
                         )
                      || (  ascii
                         && rI18nHelper.MatchMnemonic( pData->aText, ascii )
                         )
                      )
                    {
                        if( nDuplicates > 1 && rPos == nCurrentPos )
                            continue;   // select next entry with the same mnemonic
                        else
                            return pData;
                    }
                }
            }
        }
    }

    return nullptr;
}

size_t MenuItemList::GetItemCount( sal_Unicode cSelectChar ) const
{
    // returns number of entries with same mnemonic
    const vcl::I18nHelper& rI18nHelper = Application::GetSettings().GetUILocaleI18nHelper();

    size_t nItems = 0;
    for ( size_t nPos = maItemList.size(); nPos; )
    {
        MenuItemData* pData = maItemList[ --nPos ].get();
        if ( pData->bEnabled && rI18nHelper.MatchMnemonic( pData->aText, cSelectChar ) )
            nItems++;
    }

    return nItems;
}

size_t MenuItemList::GetItemCount( KeyCode aKeyCode ) const
{
    // returns number of entries with same mnemonic
    // uses key codes instead of character codes
    const vcl::I18nHelper& rI18nHelper = Application::GetSettings().GetUILocaleI18nHelper();
    char ascii = 0;
    if( aKeyCode.GetCode() >= KEY_A && aKeyCode.GetCode() <= KEY_Z )
        ascii = sal::static_int_cast<char>('A' + (aKeyCode.GetCode() - KEY_A));

    size_t nItems = 0;
    for ( size_t nPos = maItemList.size(); nPos; )
    {
        MenuItemData* pData = maItemList[ --nPos ].get();
        if ( pData->bEnabled )
        {
            sal_Int32 n = pData->aText.indexOf('~');
            if (n != -1)
            {
                KeyCode nKeyCode;
                // if MapUnicodeToKeyCode fails or is unsupported we try the pure ascii mapping of the keycodes
                // so we have working shortcuts when ascii mnemonics are used
                vcl::Window* pDefWindow = ImplGetDefaultWindow();
                if(  (  pDefWindow
                     && pDefWindow->ImplGetFrame()->MapUnicodeToKeyCode( pData->aText[n+1],
                         Application::GetSettings().GetUILanguageTag().getLanguageType(), nKeyCode )
                     && aKeyCode.GetCode() == nKeyCode.GetCode()
                     )
                  || (  ascii
                     && rI18nHelper.MatchMnemonic( pData->aText, ascii )
                     )
                  )
                    nItems++;
            }
        }
    }

    return nItems;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
