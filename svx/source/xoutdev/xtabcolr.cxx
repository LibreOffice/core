/*************************************************************************
 *
 *  $RCSfile: xtabcolr.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: ka $ $Date: 2001-02-19 17:19:57 $
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

#pragma hdrstop

#include <sfx2/docfile.hxx>
#include <tools/urlobj.hxx>
#include "dialogs.hrc"
#include "dialmgr.hxx"
#include "xtable.hxx"
#include "xiocomp.hxx"
#include "xpool.hxx"
#include "xoutx.hxx"

#define GLOBALOVERFLOW

sal_Unicode const pszExtColor[]  = {'s','o','c'};
sal_Unicode const pszChckColor[] = {'S','O','C','L'};
// Neuer Key, damit alte Version (3.00) nicht bei dem
// Versuch abstuerzt, eine neue Tabelle zu laden.
sal_Unicode const pszChckColor0[] = {'S','O','C','0'};

// ------------------
// class XColorTable
// ------------------

/*************************************************************************
|*
|* XColorTable::XColorTable()
|*
*************************************************************************/

XColorTable::XColorTable( const String& rPath,
                            XOutdevItemPool* pInPool,
                            USHORT nInitSize, USHORT nReSize ) :
                XPropertyTable( rPath, pInPool, nInitSize, nReSize)
{
    // ColorTable braucht keine eigene BmpTable
    // pBmpTable = new Table( nInitSize, nReSize );
}

/************************************************************************/

XColorTable::~XColorTable()
{
}

/************************************************************************/

XColorEntry* XColorTable::Replace(long nIndex, XColorEntry* pEntry )
{
    return (XColorEntry*) XPropertyTable::Replace(nIndex, pEntry);
}

/************************************************************************/

XColorEntry* XColorTable::Remove(long nIndex)
{
    return (XColorEntry*) XPropertyTable::Remove(nIndex, 0);
}

/************************************************************************/

XColorEntry* XColorTable::Get(long nIndex) const
{
    return (XColorEntry*) XPropertyTable::Get(nIndex, 0);
}

/************************************************************************/

BOOL XColorTable::Load()
{
#ifndef SVX_LIGHT
    if( bTableDirty )
    {
        bTableDirty = FALSE;

        INetURLObject aURL( aPath );

        if( INET_PROT_NOT_VALID == aURL.GetProtocol() )
        {
            DBG_ASSERT( !aPath.Len(), "invalid URL" );
            return FALSE;
        }

        aURL.Append( aName );

        if( !aURL.getExtension().Len() )
            aURL.setExtension( String( pszExtColor, 3 ) );

        SfxMedium aMedium( aURL.GetMainURL(), STREAM_READ | STREAM_NOCREATE, TRUE );
        SvStream* pStream = aMedium.GetInStream();
        if( !pStream )
            return( FALSE );

        String aCheck;

        // UNICODE: *pStream >> aCheck;
        pStream->ReadByteString(aCheck);

        // Handelt es sich um die gew"unschte Tabelle?
        if( aCheck == String(pszChckColor, 4) ||
            aCheck == String(pszChckColor0, 4)  )
        {
            ImpRead( *pStream );
        }
        else
            return( FALSE );

        return( pStream->GetError() == SVSTREAM_OK );
    }
#endif
    return( FALSE );
}

/************************************************************************/

BOOL XColorTable::Save()
{
#ifndef SVX_LIGHT
    INetURLObject aURL( aPath );

    if( INET_PROT_NOT_VALID == aURL.GetProtocol() )
    {
        DBG_ASSERT( !aPath.Len(), "invalid URL" );
        return FALSE;
    }

    aURL.Append( aName );

    if( !aURL.getExtension().Len() )
        aURL.setExtension( String( pszExtColor, 3 ) );

    SfxMedium aMedium( aURL.GetMainURL(), STREAM_WRITE | STREAM_TRUNC, TRUE );
    aMedium.IsRemote();

    SvStream* pStream = aMedium.GetOutStream();
    if( !pStream )
        return( FALSE );

    // UNICODE: *pStream << String( pszChckColor0, 4 );
    pStream->WriteByteString(String( pszChckColor0, 4 ));


    ImpStore( *pStream );

    aMedium.Close();
    aMedium.Commit();

    return( aMedium.GetError() == 0 );
#else
    return FALSE;
#endif
}

/************************************************************************/

BOOL XColorTable::Create()
{
    XubString aStr;
    xub_StrLen nLen;
    ResMgr* pRes = DIALOG_MGR();

    static USHORT __READONLY_DATA aResId[] =
    {
        RID_SVXSTR_BLACK,
        RID_SVXSTR_BLUE,
        RID_SVXSTR_GREEN,
        RID_SVXSTR_CYAN,
        RID_SVXSTR_RED,
        RID_SVXSTR_MAGENTA,
        RID_SVXSTR_BROWN,
        RID_SVXSTR_GREY,
        RID_SVXSTR_LIGHTGREY,
        RID_SVXSTR_LIGHTBLUE,
        RID_SVXSTR_LIGHTGREEN,
        RID_SVXSTR_LIGHTCYAN,
        RID_SVXSTR_LIGHTRED,
        RID_SVXSTR_LIGHTMAGENTA,
        RID_SVXSTR_YELLOW,
        RID_SVXSTR_WHITE
    };

    // MT: COL_XXX ist in VCL kein enum mehr!!!
    // COL_WHITE ist seeeehr gross! ( => Zugriff ueber das obige Array hinweg )
    // Mit der unteren Schleife gibt es keinen Absturtz, aber es ist
    // alles schwarz, weil alles kleine Werte.
    // Ausserdem ist die ganze Vorgehensweise laut MM sehr unperformant
    // => lieber gleich Stringlisten laden.

    // BM: ifndef VCL part removed (deprecated)

    static ColorData __READONLY_DATA aColTab[] =
    {
        COL_BLACK,
        COL_BLUE,
        COL_GREEN,
        COL_CYAN,
        COL_RED,
        COL_MAGENTA,
        COL_BROWN,
        COL_GRAY,
        COL_LIGHTGRAY,
        COL_LIGHTBLUE,
        COL_LIGHTGREEN,
        COL_LIGHTCYAN,
        COL_LIGHTRED,
        COL_LIGHTMAGENTA,
        COL_YELLOW,
        COL_WHITE
    };

    for( USHORT n = 0; n < 16; ++n )
    {
        Insert( n, new XColorEntry( Color( aColTab[n] ),
                                    String( ResId( aResId[ n ], pRes )) ) );
    }

    aStr = SVX_RESSTR( RID_SVXSTR_GREY );
    aStr.AppendAscii(" 80%");
    nLen = aStr.Len() - 3;
    Insert(16, new XColorEntry( Color( 51, 51, 51 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('7'));
    Insert(17, new XColorEntry( Color( 76, 76, 76 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('6'));
    Insert(18, new XColorEntry( Color(102,102,102 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('4'));
    Insert(19, new XColorEntry( Color(153,153,153 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('3'));
    Insert(20, new XColorEntry( Color(179,179,179 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('2'));
    Insert(21, new XColorEntry( Color(204,204,204 ), aStr ) );
    // BM: new 15%
    aStr.SetChar(nLen, sal_Unicode('1'));
    aStr.SetChar(nLen + 1, sal_Unicode('5'));
    Insert(22, new XColorEntry( Color(217,217,217 ), aStr ) );
    aStr.SetChar(nLen + 1, sal_Unicode('0'));
    Insert(23, new XColorEntry( Color(230,230,230 ), aStr ) );
    Insert(24, new XColorEntry( Color(230,230,255 ), SVX_RESSTR( RID_SVXSTR_BLUEGREY ) ) );

    aStr = SVX_RESSTR( RID_SVXSTR_RED );
    aStr.AppendAscii(" 1");
    nLen = aStr.Len() - 1;
    Insert(25, new XColorEntry( Color(255, 51,102 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('2'));
    Insert(26, new XColorEntry( Color(220, 35,  0 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('3'));
    Insert(27, new XColorEntry( Color(184, 71,  0 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('4'));
    Insert(28, new XColorEntry( Color(255, 51, 51 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('5'));
    Insert(29, new XColorEntry( Color(235, 97, 61 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('6'));
    Insert(30, new XColorEntry( Color(184, 71, 71 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('7'));
    Insert(31, new XColorEntry( Color(184,  0, 71 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('8'));
    Insert(32, new XColorEntry( Color(153, 40, 76 ), aStr ) );

    aStr = SVX_RESSTR( RID_SVXSTR_MAGENTA );
    aStr.AppendAscii(" 1");
    nLen = aStr.Len() - 1;
    Insert(33, new XColorEntry( Color(148,  0,107 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('2'));
    Insert(34, new XColorEntry( Color(148, 71,107 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('3'));
    Insert(35, new XColorEntry( Color(148, 71,148 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('4'));
    Insert(36, new XColorEntry( Color(153,102,204 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('5'));
    Insert(37, new XColorEntry( Color(107, 71,148 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('6'));
    Insert(38, new XColorEntry( Color(107, 35,148 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('7'));
    Insert(39, new XColorEntry( Color(107,  0,148 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('8'));
    Insert(40, new XColorEntry( Color( 94, 17,166 ), aStr ) );

    aStr = SVX_RESSTR( RID_SVXSTR_BLUE );
    aStr.AppendAscii(" 1");
    nLen = aStr.Len() - 1;
    Insert(41, new XColorEntry( Color( 40,  0,153 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('2'));
    Insert(42, new XColorEntry( Color( 71,  0,184 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('3'));
    Insert(43, new XColorEntry( Color( 35,  0,220 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('4'));
    Insert(44, new XColorEntry( Color( 35, 35,220 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('5'));
    Insert(45, new XColorEntry( Color(  0, 71,255 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('6'));
    Insert(46, new XColorEntry( Color(  0,153,255 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('7'));
    Insert(47, new XColorEntry( Color(  0,184,255 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('8'));
    Insert(48, new XColorEntry( Color(153,204,255 ), aStr ) );
    //Insert(48, new XColorEntry( Color( 46,215,255 ), aStr ) );

    aStr = SVX_RESSTR( RID_SVXSTR_CYAN );
    aStr.AppendAscii(" 1");
    nLen = aStr.Len() - 1;
    Insert(49, new XColorEntry( Color(  0,220,255 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('2'));
    Insert(50, new XColorEntry( Color(  0,204,204 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('3'));
    Insert(51, new XColorEntry( Color( 35,184,220 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('4'));
    Insert(52, new XColorEntry( Color( 71,184,184 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('5'));
    Insert(53, new XColorEntry( Color( 51,163,163 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('6'));
    Insert(54, new XColorEntry( Color( 25,138,138 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('7'));
    Insert(55, new XColorEntry( Color(  0,107,107 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('8'));
    Insert(56, new XColorEntry( Color(  0, 74, 74 ), aStr ) );

    aStr = SVX_RESSTR( RID_SVXSTR_GREEN );
    aStr.AppendAscii(" 1");
    nLen = aStr.Len() - 1;
    Insert(57, new XColorEntry( Color( 53, 94,  0 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('2'));
    Insert(58, new XColorEntry( Color( 92,133, 38 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('3'));
    Insert(59, new XColorEntry( Color(125,166, 71 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('4'));
    Insert(60, new XColorEntry( Color(148,189, 94 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('5'));
    Insert(61, new XColorEntry( Color(  0,174,  0 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('6'));
    Insert(62, new XColorEntry( Color( 51,204,102 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('7'));
    Insert(63, new XColorEntry( Color( 61,235, 61 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('8'));
    Insert(64, new XColorEntry( Color( 35,255, 35 ), aStr ) );

    aStr = SVX_RESSTR( RID_SVXSTR_YELLOW );
    aStr.AppendAscii(" 1");
    nLen = aStr.Len() - 1;
    Insert(65, new XColorEntry( Color(230,255,  0 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('2'));
    Insert(66, new XColorEntry( Color(255,255,153 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('3'));
    Insert(67, new XColorEntry( Color(255,255,102 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('4'));
    Insert(68, new XColorEntry( Color(230,230, 76 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('5'));
    Insert(69, new XColorEntry( Color(204,204,  0 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('6'));
    Insert(70, new XColorEntry( Color(179,179,  0 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('7'));
    Insert(71, new XColorEntry( Color(128,128, 25 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('8'));
    Insert(72, new XColorEntry( Color(102,102,  0 ), aStr ) );

    aStr = SVX_RESSTR( RID_SVXSTR_BROWN );
    aStr.AppendAscii(" 1");
    nLen = aStr.Len() - 1;
    Insert(73, new XColorEntry( Color( 76, 25,  0 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('2'));
    Insert(74, new XColorEntry( Color(102, 51,  0 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('3'));
    Insert(75, new XColorEntry( Color(128, 76, 25 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('4'));
    Insert(76, new XColorEntry( Color(153,102, 51 ), aStr ) );

    aStr = SVX_RESSTR( RID_SVXSTR_ORANGE );
    aStr.AppendAscii(" 1");
    nLen = aStr.Len() - 1;
    Insert(77, new XColorEntry( Color(204,102, 51 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('2'));
    Insert(78, new XColorEntry( Color(255,102, 51 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('3'));
    Insert(79, new XColorEntry( Color(255,153,102 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('4'));
    Insert(80, new XColorEntry( Color(255,204,153 ), aStr ) );

    // new chart colors
    aStr = SVX_RESSTR( RID_SVXSTR_VIOLET );
    Insert( 81, new XColorEntry( Color( 0x99, 0x99, 0xff ), aStr ) );

    aStr = SVX_RESSTR( RID_SVXSTR_BORDEAUX );
    Insert( 82, new XColorEntry( Color( 0x99, 0x33, 0x66 ), aStr ) );

    aStr = SVX_RESSTR( RID_SVXSTR_PALE_YELLOW );
    Insert( 83, new XColorEntry( Color( 0xff, 0xff, 0xcc ), aStr ) );

    aStr = SVX_RESSTR( RID_SVXSTR_PALE_GREEN );
    Insert( 84, new XColorEntry( Color( 0xcc, 0xff, 0xff ), aStr ) );

    aStr = SVX_RESSTR( RID_SVXSTR_DKVIOLET );
    Insert( 85, new XColorEntry( Color( 0x66, 0x00, 0x66 ), aStr ) );

    aStr = SVX_RESSTR( RID_SVXSTR_SALMON );
    Insert( 86, new XColorEntry( Color( 0xff, 0x80, 0x80 ), aStr ) );

    aStr = SVX_RESSTR( RID_SVXSTR_SEABLUE );
    Insert( 87, new XColorEntry( Color( 0x00, 0x66, 0xcc ), aStr ) );

    // Sun colors
    aStr = SVX_RESSTR( RID_SVXSTR_COLOR_SUN );
    aStr.AppendAscii(" 1");
    nLen = aStr.Len() - 1;
    Insert( 88, new XColorEntry( Color( 0x33, 0x33, 0x66 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('2'));
    Insert( 89, new XColorEntry( Color( 0x66, 0x66, 0x99 ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('3'));
    Insert( 90, new XColorEntry( Color( 0x99, 0x99, 0xcc ), aStr ) );
    aStr.SetChar(nLen, sal_Unicode('4'));
    Insert( 91, new XColorEntry( Color( 0xcc, 0xcc, 0xff ), aStr ) );

    return( Count() == 92 );
}

/************************************************************************/

BOOL XColorTable::CreateBitmapsForUI()
{
    return( FALSE );
}

/************************************************************************/

Bitmap* XColorTable::CreateBitmapForUI( long nIndex, BOOL bDelete )
{
    return( NULL );
}

/************************************************************************/

SvStream& XColorTable::ImpStore( SvStream& rOut )
{
    // Schreiben
    rOut.SetStreamCharSet( gsl_getSystemTextEncoding() );

    // Tabellentyp schreiben (0 = gesamte Tabelle)
    // Version statt Tabellentyp, um auch alte Tabellen zu lesen
    rOut << (long) -1;

    // Anzahl der Eintraege
    rOut << (long)Count();

    // die Farben
    XColorEntry* pEntry = (XColorEntry*)aTable.First();
    for (long nIndex = 0; nIndex < Count(); nIndex++)
    {
        // Versionsverwaltung: Version 0
        XIOCompat aIOC( rOut, STREAM_WRITE, 0 );

        rOut << (long)aTable.GetCurKey();

        // UNICODE:: rOut << pEntry->GetName();
        rOut.WriteByteString(pEntry->GetName());

#ifdef VCL
        USHORT nCol = pEntry->GetColor().GetRed();
        nCol = nCol << 8;
        rOut << nCol;

        nCol = pEntry->GetColor().GetGreen();
        nCol = nCol << 8;
        rOut << nCol;

        nCol = pEntry->GetColor().GetBlue();
        nCol = nCol << 8;
        rOut << nCol;
#else
        rOut << pEntry->GetColor().GetRed();
        rOut << pEntry->GetColor().GetGreen();
        rOut << pEntry->GetColor().GetBlue();
#endif
        pEntry = (XColorEntry*)aTable.Next();
    }
    return( rOut );
}

/************************************************************************/

XubString& XColorTable::ConvertName( XubString& rStrName )
{
    static USHORT __READONLY_DATA aDefResId[] =
    {
        RID_SVXSTR_BLUEGREY_DEF,
        RID_SVXSTR_BLACK_DEF,
        RID_SVXSTR_BLUE_DEF,
        RID_SVXSTR_GREEN_DEF,
        RID_SVXSTR_CYAN_DEF,
        RID_SVXSTR_RED_DEF,
        RID_SVXSTR_MAGENTA_DEF,
        RID_SVXSTR_BROWN_DEF,
        RID_SVXSTR_GREY_DEF,
        RID_SVXSTR_LIGHTGREY_DEF,
        RID_SVXSTR_LIGHTBLUE_DEF,
        RID_SVXSTR_LIGHTGREEN_DEF,
        RID_SVXSTR_LIGHTCYAN_DEF,
        RID_SVXSTR_LIGHTRED_DEF,
        RID_SVXSTR_LIGHTMAGENTA_DEF,
        RID_SVXSTR_YELLOW_DEF,
        RID_SVXSTR_WHITE_DEF,
        RID_SVXSTR_ORANGE_DEF,
        RID_SVXSTR_VIOLET_DEF,
        RID_SVXSTR_BORDEAUX_DEF,
        RID_SVXSTR_PALE_YELLOW_DEF,
        RID_SVXSTR_PALE_GREEN_DEF,
        RID_SVXSTR_DKVIOLET_DEF,
        RID_SVXSTR_SALMON_DEF,
        RID_SVXSTR_SEABLUE_DEF,
        RID_SVXSTR_COLOR_SUN_DEF
    };
    static USHORT __READONLY_DATA aResId[] =
    {
        RID_SVXSTR_BLUEGREY,
        RID_SVXSTR_BLACK,
        RID_SVXSTR_BLUE,
        RID_SVXSTR_GREEN,
        RID_SVXSTR_CYAN,
        RID_SVXSTR_RED,
        RID_SVXSTR_MAGENTA,
        RID_SVXSTR_BROWN,
        RID_SVXSTR_GREY,
        RID_SVXSTR_LIGHTGREY,
        RID_SVXSTR_LIGHTBLUE,
        RID_SVXSTR_LIGHTGREEN,
        RID_SVXSTR_LIGHTCYAN,
        RID_SVXSTR_LIGHTRED,
        RID_SVXSTR_LIGHTMAGENTA,
        RID_SVXSTR_YELLOW,
        RID_SVXSTR_WHITE,
        RID_SVXSTR_ORANGE,
        RID_SVXSTR_VIOLET,
        RID_SVXSTR_BORDEAUX,
        RID_SVXSTR_PALE_YELLOW,
        RID_SVXSTR_PALE_GREEN,
        RID_SVXSTR_DKVIOLET,
        RID_SVXSTR_SALMON,
        RID_SVXSTR_SEABLUE,
        RID_SVXSTR_COLOR_SUN
    };

    BOOL bFound = FALSE;

    for( int i=0; i<(sizeof(aDefResId) / sizeof(USHORT)) && !bFound; i++ )
    {
        XubString aStrDefName = SVX_RESSTR( aDefResId[i] );
        if( rStrName.Search( aStrDefName ) == 0 )
        {
            rStrName.Replace( 0, aStrDefName.Len(), SVX_RESSTR( aResId[i] ) );
            bFound = TRUE;
        }
    }

    return rStrName;
}

/************************************************************************/

SvStream& XColorTable::ImpRead( SvStream& rIn )
{
    // Lesen
    rIn.SetStreamCharSet( RTL_TEXTENCODING_IBM_850 );

    delete pBmpTable;
    pBmpTable = new Table( 16, 16 );

    XColorEntry* pEntry = NULL;
    long         nType;
    long         nCount;
    long         nIndex;
    USHORT       nRed;
    USHORT       nGreen;
    USHORT       nBlue;
    Color        aColor;
    XubString    aName;

    rIn >> nType;

    // gesamte Tabelle?
    if (nType == 0)
    {
        rIn >> nCount;
        for (long nI = 0; nI < nCount; nI++)
        {
            rIn >> nIndex;

            // UNICODE: rIn >> aName;
            rIn.ReadByteString(aName);
            aName = ConvertName( aName );

            rIn >> nRed;
            rIn >> nGreen;
            rIn >> nBlue;

#ifdef VCL
            aColor = Color( (BYTE) ( nRed   >> 8 ),
                            (BYTE) ( nGreen >> 8 ),
                            (BYTE) ( nBlue  >> 8 ) );
#else
            aColor = Color( nRed, nGreen, nBlue );
#endif
            pEntry = new XColorEntry( aColor, aName);
            Insert (nIndex, pEntry);
        }
    }
    else // Version ab 3.00a
    {
        rIn >> nCount;
        for (long nI = 0; nI < nCount; nI++)
        {
            // Versionsverwaltung
            XIOCompat aIOC( rIn, STREAM_READ );

            rIn >> nIndex;

            // UNICODE: rIn >> aName;
            rIn.ReadByteString(aName);
            aName = ConvertName( aName );

            if( aIOC.GetVersion() >= 0 )
            {
                rIn >> nRed;
                rIn >> nGreen;
                rIn >> nBlue;
#ifdef VCL
                aColor = Color( (BYTE) ( nRed   >> 8 ),
                                (BYTE) ( nGreen >> 8 ),
                                (BYTE) ( nBlue  >> 8 ) );
#else
                aColor = Color( nRed, nGreen, nBlue );
#endif
            }
            /*
            else if( aIOC.GetVersion() >= 1 )
            {
                // lesen neuer Daten ...
            }
            */

            pEntry = new XColorEntry( aColor, aName );
            Insert( nIndex, pEntry );
        }
    }
    return( rIn );
}

// --------------------
// class XColorList
// --------------------

/*************************************************************************
|*
|* XColorList::XColorList()
|*
*************************************************************************/

XColorList::XColorList( const String& rPath,
                            XOutdevItemPool* pInPool,
                            USHORT nInitSize, USHORT nReSize ) :
                XPropertyList( rPath, pInPool, nInitSize, nReSize)
{
    // pBmpList = new List( nInitSize, nReSize );
}

/************************************************************************/

XColorList::~XColorList()
{
}

/************************************************************************/

XColorEntry* XColorList::Replace(XColorEntry* pEntry, long nIndex )
{
    return (XColorEntry*) XPropertyList::Replace(pEntry, nIndex);
}

/************************************************************************/

XColorEntry* XColorList::Remove(long nIndex)
{
    return (XColorEntry*) XPropertyList::Remove(nIndex, 0);
}

/************************************************************************/

XColorEntry* XColorList::Get(long nIndex) const
{
    return (XColorEntry*) XPropertyList::Get(nIndex, 0);
}

/************************************************************************/

BOOL XColorList::Load()
{
    return( FALSE );
}

/************************************************************************/

BOOL XColorList::Save()
{
    return( FALSE );
}

/************************************************************************/

BOOL XColorList::Create()
{
    return( FALSE );
}

/************************************************************************/

BOOL XColorList::CreateBitmapsForUI()
{
    return( FALSE );
}

/************************************************************************/

Bitmap* XColorList::CreateBitmapForUI( long nIndex, BOOL bDelete )
{
    return( NULL );
}

/************************************************************************/

SvStream& XColorList::ImpStore( SvStream& rOut )
{
    // Erstmal von XColorTable uebernommen !!!

    // Schreiben
    rOut.SetStreamCharSet( gsl_getSystemTextEncoding() );

    // Tabellentyp schreiben (0 = gesamte Tabelle)
    rOut << (long)0;

    // Anzahl der Eintraege
    rOut << (long)Count();

    // die Farben
    XColorEntry* pEntry = (XColorEntry*)aList.First();
    for (long nIndex = 0; nIndex < Count(); nIndex++)
    {
        // rOut << (long)aList.GetCurKey();

        // UNICODE:rOut << pEntry->GetName();
        rOut.WriteByteString(pEntry->GetName());

        rOut << pEntry->GetColor().GetRed();
        rOut << pEntry->GetColor().GetGreen();
        rOut << pEntry->GetColor().GetBlue();
        pEntry = (XColorEntry*)aList.Next();
    }
    return rOut;
}

/************************************************************************/

SvStream& XColorList::ImpRead( SvStream& rIn )
{
    // Lesen
    rIn.SetStreamCharSet( RTL_TEXTENCODING_IBM_850 );

    // Erstmal von XColorTable uebernommen !!!
    delete pBmpList;
    pBmpList = new List( 16, 16 );


    XColorEntry* pEntry = NULL;
    long         nType;
    long         nCount;
    long         nIndex;
    USHORT       nRed;
    USHORT       nGreen;
    USHORT       nBlue;
    XubString    aName;

    rIn >> nType;

    // gesamte Tabelle?
    if (nType == 0)
    {
        rIn >> nCount;
        for (long nI = 0; nI < nCount; nI++)
        {
            rIn >> nIndex;

            // UNICODE: rIn >> aName;
            rIn.ReadByteString(aName);

            rIn >> nRed;
            rIn >> nGreen;
            rIn >> nBlue;

            pEntry = new XColorEntry (Color( (BYTE) nRed, (BYTE) nGreen, (BYTE) nBlue), aName);
            Insert (pEntry, nIndex);
        }
    }
    return( rIn );
}


