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


#include <string.h>
#include <vcl/svapp.hxx>
#include <vcl/i18nhelp.hxx>
#include <svtools/ctrltool.hxx>
#include <svtools/stdmenu.hxx>

// ========================================================================

FontNameMenu::FontNameMenu()
{
    SetMenuFlags( GetMenuFlags() | MENU_FLAG_NOAUTOMNEMONICS );
}

// -----------------------------------------------------------------------

FontNameMenu::~FontNameMenu()
{
}

// -----------------------------------------------------------------------

void FontNameMenu::Select()
{
    maCurName = GetItemText( GetCurItemId() );
    maSelectHdl.Call( this );
}

// -----------------------------------------------------------------------

void FontNameMenu::Highlight()
{
    OUString aTempName = maCurName;
    maCurName = GetItemText( GetCurItemId() );
    maHighlightHdl.Call( this );
    maCurName = aTempName;
}

// -----------------------------------------------------------------------

void FontNameMenu::Fill( const FontList* pList )
{
    // clear menu
    Clear();

    // add fonts
    const vcl::I18nHelper& rI18nHelper = Application::GetSettings().GetUILocaleI18nHelper();
    // more than 100 fonts reduces the speed of opening the menu.
    // So only the first 100 fonts will be displayed.
    sal_uInt16 nFontCount = ::std::min( pList->GetFontNameCount(), static_cast< sal_uInt16 >(100) );
    for (sal_uInt16 i = 0; i < nFontCount; ++i)
    {
        const OUString& rName = pList->GetFontName( i ).GetName();

        // sort with the I18nHelper
        sal_uInt16 j = GetItemCount();
        while ( j )
        {
            OUString aText = GetItemText( GetItemId( j-1 ) );
            if ( rI18nHelper.CompareString( rName, aText ) > 0 )
                break;
            j--;
        }
        InsertItem( i+1, rName, MIB_RADIOCHECK | MIB_AUTOCHECK, OString(), j );
    }

    SetCurName( maCurName );
}

// -----------------------------------------------------------------------

void FontNameMenu::SetCurName(const OUString& rName)
{
    maCurName = rName;

    // Menueintrag checken
    sal_uInt16 nChecked = 0;
    sal_uInt16 nItemCount = GetItemCount();
    for( sal_uInt16 i = 0; i < nItemCount; i++ )
    {
        sal_uInt16 nItemId = GetItemId( i );

        if ( IsItemChecked( nItemId ) )
            nChecked = nItemId;

        OUString aText = GetItemText( nItemId );
        if ( aText == maCurName )
        {
            CheckItem( nItemId, sal_True );
            return;
        }
    }

    if ( nChecked )
        CheckItem( nChecked, sal_False );
}

// ========================================================================

FontSizeMenu::FontSizeMenu()
:    mpHeightAry( NULL )
,    mnCurHeight( 100 )
{
    SetMenuFlags( GetMenuFlags() | MENU_FLAG_NOAUTOMNEMONICS );
}

// -----------------------------------------------------------------------

FontSizeMenu::~FontSizeMenu()
{
    if ( mpHeightAry )
        delete[] mpHeightAry;
}

// -----------------------------------------------------------------------

void FontSizeMenu::Select()
{
    const sal_uInt16 nCurItemId = GetCurItemId();
    mnCurHeight = mpHeightAry[ nCurItemId - 1 ];
    maSelectHdl.Call( this );
}

// -----------------------------------------------------------------------

void FontSizeMenu::Highlight()
{
    const long nTempHeight = mnCurHeight;
    const sal_uInt16 nCurItemId = GetCurItemId();
    if ( !nCurItemId )
        mnCurHeight = 0;
    else
    {
        //sal_Int32 nValue = GetItemText( nCurItemId ).ToInt32();
        mnCurHeight = mpHeightAry[ nCurItemId - 1 ];
    }
    maHighlightHdl.Call( this );
    mnCurHeight = nTempHeight;
}

// -----------------------------------------------------------------------

void FontSizeMenu::Fill( const FontInfo& rInfo, const FontList* pList )
{
    Clear();

    // setup font size array
    if ( mpHeightAry )
        delete[] mpHeightAry;

    const sal_IntPtr* pTempAry;
    const sal_IntPtr* pAry = pList->GetSizeAry( rInfo );
    sal_uInt16 nSizeCount = 0;
    while ( pAry[nSizeCount] )
        nSizeCount++;

    sal_uInt16 nPos = 0;

    // first insert font size names (for simplified/traditional chinese)
    FontSizeNames aFontSizeNames( Application::GetSettings().GetUILanguageTag().getLanguageType() );
    mpHeightAry = new long[nSizeCount+aFontSizeNames.Count()];
    if ( !aFontSizeNames.IsEmpty() )
    {
        if ( pAry == pList->GetStdSizeAry() )
        {
            // for scalable fonts all font size names
            sal_uLong nCount = aFontSizeNames.Count();
            for( sal_uLong i = 0; i < nCount; i++ )
            {
                String  aSizeName = aFontSizeNames.GetIndexName( i );
                long    nSize = aFontSizeNames.GetIndexSize( i );
                mpHeightAry[nPos] = nSize;
                nPos++; // Id is nPos+1
                InsertItem( nPos, aSizeName, MIB_RADIOCHECK | MIB_AUTOCHECK );
            }
        }
        else
        {
            // for fixed size fonts only selectable font size names
            pTempAry = pAry;
            while ( *pTempAry )
            {
                String aSizeName = aFontSizeNames.Size2Name( *pTempAry );
                if ( aSizeName.Len() )
                {
                    mpHeightAry[nPos] = *pTempAry;
                    nPos++; // Id is nPos+1
                    InsertItem( nPos, aSizeName, MIB_RADIOCHECK | MIB_AUTOCHECK );
                }
                pTempAry++;
            }
        }
    }

    // then insert numerical font size values
    const vcl::I18nHelper& rI18nHelper = Application::GetSettings().GetUILocaleI18nHelper();
    pTempAry = pAry;
    while ( *pTempAry )
    {
        mpHeightAry[nPos] = *pTempAry;
        nPos++; // Id is nPos+1
        InsertItem( nPos, rI18nHelper.GetNum( *pTempAry, 1, sal_True, sal_False ), MIB_RADIOCHECK | MIB_AUTOCHECK );
        pTempAry++;
    }

    SetCurHeight( mnCurHeight );
}

// -----------------------------------------------------------------------

void FontSizeMenu::SetCurHeight( long nHeight )
{
    mnCurHeight = nHeight;

    // check menu item
    sal_uInt16      nChecked = 0;
    sal_uInt16      nItemCount = GetItemCount();
    for( sal_uInt16 i = 0; i < nItemCount; i++ )
    {
        sal_uInt16 nItemId = GetItemId( i );

        if ( mpHeightAry[i] == nHeight )
        {
            CheckItem( nItemId, sal_True );
            return;
        }

        if ( IsItemChecked( nItemId ) )
            nChecked = nItemId;
    }

    if ( nChecked )
        CheckItem( nChecked, sal_False );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
