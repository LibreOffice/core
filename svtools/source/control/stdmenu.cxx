/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


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
    for ( sal_uInt16 i = 0; i < nFontCount; i++ )
    {
        const XubString& rName = pList->GetFontName( i ).GetName();

        // sort with the I18nHelper
        sal_uInt16 j = GetItemCount();
        while ( j )
        {
            OUString aText = GetItemText( GetItemId( j-1 ) );
            if ( rI18nHelper.CompareString( rName, aText ) > 0 )
                break;
            j--;
        }
        InsertItem( i+1, rName, MIB_RADIOCHECK | MIB_AUTOCHECK, j );
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

    const long* pTempAry;
    const long* pAry = pList->GetSizeAry( rInfo );
    sal_uInt16 nSizeCount = 0;
    while ( pAry[nSizeCount] )
        nSizeCount++;

    sal_uInt16 nPos = 0;

    // first insert font size names (for simplified/traditional chinese)
    FontSizeNames aFontSizeNames( Application::GetSettings().GetUILanguage() );
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
    OUString aHeight = Application::GetSettings().GetUILocaleI18nHelper().GetNum( nHeight, 1, sal_True, sal_False  );
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
