/*************************************************************************
 *
 *  $RCSfile: stdmenu.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: th $ $Date: 2001-03-09 15:43:34 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <string.h>

#ifndef _INTN_HXX
#include <tools/intn.hxx>
#endif
#ifndef _APP_HXX
#include <vcl/svapp.hxx>
#endif

#include <ctrltool.hxx>
#include <stdmenu.hxx>

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
    XubString aTempName = maCurName;
    maCurName = GetItemText( GetCurItemId() );
    maHighlightHdl.Call( this );
    maCurName = aTempName;
}

// -----------------------------------------------------------------------

void FontNameMenu::Fill( const FontList* pList )
{
    // Menu loeschen
    Clear();

    // Fonts eintragen
    International   aIntn = Application::GetAppInternational();
    USHORT          nFontCount = pList->GetFontNameCount();
    for ( USHORT i = 0; i < nFontCount; i++ )
    {
        const XubString& rName = pList->GetFontName( i ).GetName();

        // Sortieren, nach der in der Applikation eingestellten
        // International-Klasse
        USHORT j = GetItemCount();
        while ( j )
        {
            XubString aText = GetItemText( GetItemId( j-1 ) );
            if ( aIntn.Compare( rName, aText ) == COMPARE_GREATER )
                break;
            j--;
        }

        InsertItem( i+1, rName, MIB_RADIOCHECK | MIB_AUTOCHECK, j );
    }

    SetCurName( maCurName );
}

// -----------------------------------------------------------------------

void FontNameMenu::SetCurName( const XubString& rName )
{
    maCurName = rName;

    // Menueintrag checken
    USHORT nChecked = 0;
    USHORT nItemCount = GetItemCount();
    for( USHORT i = 0; i < nItemCount; i++ )
    {
        USHORT nItemId = GetItemId( i );

        if ( IsItemChecked( nItemId ) )
            nChecked = nItemId;

        XubString aText = GetItemText( nItemId );
        if ( aText == maCurName )
        {
            CheckItem( nItemId, TRUE );
            return;
        }
    }

    if ( nChecked )
        CheckItem( nChecked, FALSE );
}

// ========================================================================

FontStyleMenu::FontStyleMenu()
{
    SetMenuFlags( GetMenuFlags() | MENU_FLAG_NOAUTOMNEMONICS );
}

// -----------------------------------------------------------------------

FontStyleMenu::~FontStyleMenu()
{
}

// -----------------------------------------------------------------------

void FontStyleMenu::Select()
{
    USHORT nCurId = GetCurItemId();

    if ( (nCurId >= FONTSTYLEMENU_FIRSTID) && (nCurId <= FONTSTYLEMENU_LASTID) )
    {
        maCurStyle = GetItemText( nCurId );
        maSelectHdl.Call( this );
    }
    else
        PopupMenu::Select();
}

// -----------------------------------------------------------------------

void FontStyleMenu::Highlight()
{
    USHORT nCurId = GetCurItemId();

    if ( (nCurId >= FONTSTYLEMENU_FIRSTID) && (nCurId <= FONTSTYLEMENU_LASTID) )
    {
        XubString aTempName = maCurStyle;
        maCurStyle = GetItemText( nCurId );
        maHighlightHdl.Call( this );
        maCurStyle = aTempName;
    }
    else
        PopupMenu::Highlight();
}

// -----------------------------------------------------------------------

BOOL FontStyleMenu::ImplIsAlreadyInserted( const XubString& rStyleName, USHORT nCount )
{
    for ( USHORT i = 0; i < nCount; i++ )
    {
        if ( GetItemText( i+FONTSTYLEMENU_FIRSTID ) == rStyleName )
            return TRUE;
    }

    return FALSE;
}

// -----------------------------------------------------------------------

void FontStyleMenu::Fill( const XubString& rName, const FontList* pList )
{
    USHORT nItemId = GetItemId( 0 );
    while ( (nItemId >= FONTSTYLEMENU_FIRSTID) &&
            (nItemId <= FONTSTYLEMENU_LASTID) )
    {
        RemoveItem( 0 );
        nItemId = GetItemId( 0 );
    }

    // Existiert ein Font mit diesem Namen
    sal_Handle hFontInfo = pList->GetFirstFontInfo( rName );
    if ( hFontInfo )
    {
        XubString   aStyleText;
        USHORT      nPos = 0;
        USHORT      nId = FONTSTYLEMENU_FIRSTID;
        FontWeight  eLastWeight = WEIGHT_DONTKNOW;
        FontItalic  eLastItalic = ITALIC_NONE;
        FontWidth   eLastWidth = WIDTH_DONTKNOW;
        BOOL        bNormal = FALSE;
        BOOL        bItalic = FALSE;
        BOOL        bBold = FALSE;
        BOOL        bBoldItalic = FALSE;
        BOOL        bInsert = FALSE;
        FontInfo    aInfo;
        while ( hFontInfo )
        {
            aInfo = pList->GetFontInfo( hFontInfo );

            FontWeight  eWeight = aInfo.GetWeight();
            FontItalic  eItalic = aInfo.GetItalic();
            FontWidth   eWidth = aInfo.GetWidthType();
            // Only if the attributes are different, we insert the
            // Font to avoid double Entries in different languages
            if ( (eWeight != eLastWeight) || (eItalic != eLastItalic) ||
                 (eWidth != eLastWidth) )
            {
                if ( bInsert )
                {
                    InsertItem( nId, aStyleText,
                                MIB_RADIOCHECK | MIB_AUTOCHECK, nPos );
                    nPos++;
                    nId++;
                }

                if ( eWeight <= WEIGHT_NORMAL )
                {
                    if ( eItalic != ITALIC_NONE )
                        bItalic = TRUE;
                    else
                        bNormal = TRUE;
                }
                else
                {
                    if ( eItalic != ITALIC_NONE )
                        bBoldItalic = TRUE;
                    else
                        bBold = TRUE;
                }

                // For wrong StyleNames we replace this with the correct once
                aStyleText = pList->GetStyleName( aInfo );
                bInsert = !ImplIsAlreadyInserted( aStyleText, nPos );
                if ( !bInsert )
                {
                    aStyleText = pList->GetStyleName( eWeight, eItalic );
                    bInsert = !ImplIsAlreadyInserted( aStyleText, nPos );
                }

                eLastWeight = eWeight;
                eLastItalic = eItalic;
                eLastWidth = eWidth;
            }
            else
            {
                if ( bInsert )
                {
                    // If we have two names for the same attributes
                    // we prefer the translated standard names
                    const XubString& rAttrStyleText = pList->GetStyleName( eWeight, eItalic );
                    if ( rAttrStyleText != aStyleText )
                    {
                        XubString aTempStyleText = pList->GetStyleName( aInfo );
                        if ( rAttrStyleText == aTempStyleText )
                            aStyleText = rAttrStyleText;
                        bInsert = !ImplIsAlreadyInserted( aStyleText, nPos );
                    }
                }
            }

            if ( !bItalic && (aStyleText == pList->GetItalicStr()) )
                bItalic = TRUE;
            else if ( !bBold && (aStyleText == pList->GetBoldStr()) )
                bBold = TRUE;
            else if ( !bBoldItalic && (aStyleText == pList->GetBoldItalicStr()) )
                bBoldItalic = TRUE;

            hFontInfo = pList->GetNextFontInfo( hFontInfo );
        }

        if ( bInsert )
        {
            InsertItem( nId, aStyleText,
                        MIB_RADIOCHECK | MIB_AUTOCHECK, nPos );
            nPos++;
            nId++;
        }

        // Bestimmte Styles als Nachbildung
        if ( bNormal )
        {
            if ( !bItalic )
            {
                InsertItem( nId, pList->GetItalicStr(),
                            MIB_RADIOCHECK | MIB_AUTOCHECK, nPos );
                nPos++;
                nId++;
            }
            if ( !bBold )
            {
                InsertItem( nId, pList->GetBoldStr(),
                            MIB_RADIOCHECK | MIB_AUTOCHECK, nPos );
                nPos++;
                nId++;
            }
        }
        if ( !bBoldItalic )
        {
            if ( bNormal || bItalic || bBold )
            {
                InsertItem( nId, pList->GetBoldItalicStr(),
                            MIB_RADIOCHECK | MIB_AUTOCHECK, nPos );
                nPos++;
                nId++;
            }
        }
    }
    else
    {
        // Wenn Font nicht, dann Standard-Styles einfuegen
        InsertItem( FONTSTYLEMENU_FIRSTID,   pList->GetNormalStr(),
                    MIB_RADIOCHECK | MIB_AUTOCHECK, 0 );
        InsertItem( FONTSTYLEMENU_FIRSTID+1, pList->GetItalicStr(),
                    MIB_RADIOCHECK | MIB_AUTOCHECK, 0 );
        InsertItem( FONTSTYLEMENU_FIRSTID+2, pList->GetBoldStr(),
                    MIB_RADIOCHECK | MIB_AUTOCHECK, 0 );
        InsertItem( FONTSTYLEMENU_FIRSTID+3, pList->GetBoldItalicStr(),
                    MIB_RADIOCHECK | MIB_AUTOCHECK, 0 );
    }

    SetCurStyle( maCurStyle );
}

// -----------------------------------------------------------------------

void FontStyleMenu::SetCurStyle( const XubString& rStyle )
{
    maCurStyle = rStyle;

    // Menueintrag checken
    USHORT nChecked = 0;
    USHORT nItemCount = GetItemCount();
    for( USHORT i = 0; i < nItemCount; i++ )
    {
        USHORT nItemId = GetItemId( i );

        if ( (nItemId < FONTSTYLEMENU_FIRSTID) ||
             (nItemId > FONTSTYLEMENU_LASTID) )
            break;

        if ( IsItemChecked( nItemId ) )
            nChecked = nItemId;

        XubString aText = GetItemText( nItemId );
        if ( aText == maCurStyle )
        {
            CheckItem( nItemId, TRUE );
            return;
        }
    }

    if ( nChecked )
        CheckItem( nChecked, FALSE );
}

// ========================================================================

FontSizeMenu::FontSizeMenu() :
    maIntn( Application::GetAppInternational() )
{
    mpHeightAry = NULL;

    maIntn.SetNumTrailingZeros( FALSE );
    maIntn.SetNumDigits( 1 );

    SetMenuFlags( GetMenuFlags() | MENU_FLAG_NOAUTOMNEMONICS );
}

// -----------------------------------------------------------------------

FontSizeMenu::~FontSizeMenu()
{
    if ( mpHeightAry )
        delete mpHeightAry;
}

// -----------------------------------------------------------------------

void FontSizeMenu::Select()
{
    const USHORT nCurItemId = GetCurItemId();
    mnCurHeight = mpHeightAry[ nCurItemId - 1 ];
    maSelectHdl.Call( this );
}

// -----------------------------------------------------------------------

void FontSizeMenu::Highlight()
{
    const long nTempHeight = mnCurHeight;
    const USHORT nCurItemId = GetCurItemId();
    if ( !nCurItemId )
        mnCurHeight = 0;
    else
    {
        sal_Int32 nValue = GetItemText( nCurItemId ).ToInt32();
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
        delete mpHeightAry;

    const long* pTempAry;
    const long* pAry = pList->GetSizeAry( rInfo );
    USHORT nSizeCount = 0;
    while ( pAry[nSizeCount] )
        nSizeCount++;

    USHORT nPos = 0;

    // first insert font size names (for simplified/traditional chinese)
    FontSizeNames aFontSizeNames( maIntn.GetLanguage() );
    mpHeightAry = new long[nSizeCount+aFontSizeNames.Count()];
    if ( !aFontSizeNames.IsEmpty() )
    {
        if ( pAry == pList->GetStdSizeAry() )
        {
            // for scalable fonts all font size names
            ULONG nCount = aFontSizeNames.Count();
            for( ULONG i = 0; i < nCount; i++ )
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
    pTempAry = pAry;
    while ( *pTempAry )
    {
        mpHeightAry[nPos] = *pTempAry;
        nPos++; // Id is nPos+1
        InsertItem( nPos, maIntn.GetNum( *pTempAry ), MIB_RADIOCHECK | MIB_AUTOCHECK );
        pTempAry++;
    }

    SetCurHeight( mnCurHeight );
}

// -----------------------------------------------------------------------

void FontSizeMenu::SetCurHeight( long nHeight )
{
    mnCurHeight = nHeight;

    // check menue item
    XubString   aHeight = maIntn.GetNum( nHeight );
    USHORT      nChecked = 0;
    USHORT      nItemCount = GetItemCount();
    for( USHORT i = 0; i < nItemCount; i++ )
    {
        USHORT nItemId = GetItemId( i );

        if ( mpHeightAry[i] == nHeight )
        {
            CheckItem( nItemId, TRUE );
            return;
        }

        if ( IsItemChecked( nItemId ) )
            nChecked = nItemId;
    }

    if ( nChecked )
        CheckItem( nChecked, FALSE );
}
