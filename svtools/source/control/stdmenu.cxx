/*************************************************************************
 *
 *  $RCSfile: stdmenu.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:58:57 $
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
        USHORT      nPos = 0;
        USHORT      nId = FONTSTYLEMENU_FIRSTID;
        BOOL        bNormal = FALSE;
        BOOL        bItalic = FALSE;
        BOOL        bBold = FALSE;
        BOOL        bBoldItalic = FALSE;
        FontInfo    aInfo;
        while ( hFontInfo )
        {
            aInfo = pList->GetFontInfo( hFontInfo );

            XubString aStyleText = pList->GetStyleName( aInfo );
            FontWeight eWeight = aInfo.GetWeight();
            FontItalic eItalic = aInfo.GetItalic();
            if ( eWeight <= WEIGHT_NORMAL )
            {
                bNormal = TRUE;
                if ( eItalic != ITALIC_NONE )
                    bItalic = TRUE;
            }
            else
            {
                if ( eItalic != ITALIC_NONE )
                    bBoldItalic = TRUE;
                else
                    bBold = TRUE;
            }
            if ( aStyleText == pList->GetItalicStr() )
                bItalic = TRUE;
            else if ( aStyleText == pList->GetBoldStr() )
                bBold = TRUE;
            else if ( aStyleText == pList->GetBoldItalicStr() )
                bBoldItalic = TRUE;

            InsertItem( nId, aStyleText,
                        MIB_RADIOCHECK | MIB_AUTOCHECK, nPos );
            nPos++;
            nId++;

            hFontInfo = pList->GetNextFontInfo( hFontInfo );
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
    mnCurHeight = mpHeightAry[GetCurItemId()-1];
    maSelectHdl.Call( this );
}

// -----------------------------------------------------------------------

void FontSizeMenu::Highlight()
{
    long nTempHeight = mnCurHeight;
    USHORT nCurItemId = GetCurItemId();
    if ( !nCurItemId )
        mnCurHeight = 0;
    else
        mnCurHeight = mpHeightAry[nCurItemId-1];
    maHighlightHdl.Call( this );
    mnCurHeight = nTempHeight;
}

// -----------------------------------------------------------------------

void FontSizeMenu::Fill( const FontInfo& rInfo, const FontList* pList )
{
    // Menu loeschen
    Clear();

    // Groessen ermitteln
    const long* pAry = pList->GetSizeAry( rInfo );

    // Array kopieren
    if ( mpHeightAry )
        delete mpHeightAry;
    USHORT n = 0;
    while ( pAry[n] )
        n++;
    mpHeightAry = new long[n];
    memcpy( mpHeightAry, pAry, n*sizeof(long) );

    // Groessen einfuegen
    USHORT nId = 1;
    while ( *pAry )
    {
        InsertItem( nId, maIntn.GetNum( *pAry ),
                    MIB_RADIOCHECK | MIB_AUTOCHECK );
        nId++;
        pAry++;
    }

    SetCurHeight( mnCurHeight );
}

// -----------------------------------------------------------------------

void FontSizeMenu::SetCurHeight( long nHeight )
{
    mnCurHeight = nHeight;

    // Menueintrag checken
    XubString   aHeight = maIntn.GetNum( nHeight );
    USHORT      nChecked = 0;
    USHORT      nItemCount = GetItemCount();
    for( USHORT i = 0; i < nItemCount; i++ )
    {
        USHORT nItemId = GetItemId( i );

        if ( IsItemChecked( nItemId ) )
            nChecked = nItemId;

        XubString aText = GetItemText( nItemId );
        if ( aText == aHeight )
        {
            CheckItem( nItemId, TRUE );
            return;
        }
    }

    if ( nChecked )
        CheckItem( nChecked, FALSE );
}
