/*************************************************************************
 *
 *  $RCSfile: xtabbtmp.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ka $ $Date: 2000-11-10 15:17:43 $
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

#include <tools/urlobj.hxx>
#include <vcl/virdev.hxx>
#include <svtools/itemset.hxx>
#include <sfx2/docfile.hxx>
#include "dialogs.hrc"
#include "dialmgr.hxx"
#include "xtable.hxx"
#include "xiocomp.hxx"
#include "xpool.hxx"
#include "xoutx.hxx"

#define GLOBALOVERFLOW

sal_Unicode const pszExtBitmap[]  = {'s','o','b'};
sal_Unicode const pszChckBitmap[] = {'S','O','B','L'};
// Neuer Key, damit alte Version (3.00) nicht bei dem
// Versuch abstuerzt, eine neue Tabelle zu laden.
sal_Unicode const pszChckBitmap0[] = {'S','O','B','0'};
sal_Unicode const pszChckBitmap1[] = {'S','O','B','1'};

// -------------------
// class XBitmapTable
// -------------------

/*************************************************************************
|*
|* XBitmapTable::XBitmapTable()
|*
*************************************************************************/

XBitmapTable::XBitmapTable( const String& rPath,
                            XOutdevItemPool* pInPool,
                            USHORT nInitSize, USHORT nReSize ) :
                XPropertyTable( rPath, pInPool, nInitSize, nReSize)
{
    pBmpTable = new Table( nInitSize, nReSize );
}

/************************************************************************/

XBitmapTable::~XBitmapTable()
{
}

/************************************************************************/

XBitmapEntry* XBitmapTable::Replace(long nIndex, XBitmapEntry* pEntry )
{
    return (XBitmapEntry*) XPropertyTable::Replace(nIndex, pEntry);
}

/************************************************************************/

XBitmapEntry* XBitmapTable::Remove(long nIndex)
{
    return (XBitmapEntry*) XPropertyTable::Remove(nIndex, 0);
}

/************************************************************************/

XBitmapEntry* XBitmapTable::Get(long nIndex) const
{
    return (XBitmapEntry*) XPropertyTable::Get(nIndex, 0);
}

/************************************************************************/

BOOL XBitmapTable::Load()
{
    return( FALSE );
}

/************************************************************************/

BOOL XBitmapTable::Save()
{
    return( FALSE );
}

/************************************************************************/

BOOL XBitmapTable::Create()
{
    return( FALSE );
}

/************************************************************************/

BOOL XBitmapTable::CreateBitmapsForUI()
{
    return( FALSE );
}

/************************************************************************/

Bitmap* XBitmapTable::CreateBitmapForUI( long nIndex, BOOL bDelete )
{
    return( NULL );
}

/************************************************************************/

SvStream& XBitmapTable::ImpStore( SvStream& rOut )
{
    // Schreiben
    rOut.SetStreamCharSet( gsl_getSystemTextEncoding() );

    // Tabellentyp schreiben (0 = gesamte Tabelle)
    // Version statt Tabellentyp, um auch alte Tabellen zu lesen
    rOut << (long) -1;

    // Anzahl der Eintraege
    rOut << (long)Count();

    // die Eintraege
    XBitmapEntry* pEntry = (XBitmapEntry*)aTable.First();

    for (long nIndex = 0; nIndex < Count(); nIndex++)
    {
        // Versionsverwaltung: Version 0
        XIOCompat aIOC( rOut, STREAM_WRITE, 0 );

        rOut << (long)aTable.GetCurKey();

        // UNICODE: rOut << pEntry->GetName();
        rOut.WriteByteString(pEntry->GetName());

        rOut << pEntry->GetXBitmap().GetBitmap();
        pEntry = (XBitmapEntry*)aTable.Next();
    }

    return rOut;
}

/************************************************************************/

SvStream& XBitmapTable::ImpRead( SvStream& rIn )
{
    // Lesen
    rIn.SetStreamCharSet( RTL_TEXTENCODING_IBM_850 );

    delete pBmpTable;
    pBmpTable = new Table( 16, 16 );

    XBitmapEntry* pEntry = NULL;
    long        nType;
    long        nCount;
    long        nIndex;
    String      aName;
    Bitmap      aBitmap;

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

            rIn >> aBitmap;

            pEntry = new XBitmapEntry (aBitmap, aName);
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

            rIn >> aBitmap;

            if (aIOC.GetVersion() > 0)
            {
                // lesen neuer Daten ...
            }

            pEntry = new XBitmapEntry (aBitmap, aName);
            Insert (nIndex, pEntry);
        }
    }
    return( rIn );
}

// ------------------
// class XBitmapList
// ------------------

/*************************************************************************
|*
|* XBitmapList::XBitmapList()
|*
*************************************************************************/

XBitmapList::XBitmapList( const String& rPath,
                            XOutdevItemPool* pInPool,
                            USHORT nInitSize, USHORT nReSize ) :
                XPropertyList( rPath, pInPool, nInitSize, nReSize)
{
    // pBmpList = new List( nInitSize, nReSize );
}

/************************************************************************/

XBitmapList::~XBitmapList()
{
}

/************************************************************************/

XBitmapEntry* XBitmapList::Replace(XBitmapEntry* pEntry, long nIndex )
{
    return (XBitmapEntry*) XPropertyList::Replace(pEntry, nIndex);
}

/************************************************************************/

XBitmapEntry* XBitmapList::Remove(long nIndex)
{
    return (XBitmapEntry*) XPropertyList::Remove(nIndex, 0);
}

/************************************************************************/

XBitmapEntry* XBitmapList::Get(long nIndex) const
{
    return (XBitmapEntry*) XPropertyList::Get(nIndex, 0);
}

/************************************************************************/

BOOL XBitmapList::Load()
{
#ifndef SVX_LIGHT
    if( bListDirty )
    {
        bListDirty = FALSE;

        INetURLObject aURL( aPath );

        if( INET_PROT_NOT_VALID == aURL.GetProtocol() )
        {
            DBG_ERROR( "invalid URL" );
            return FALSE;
        }

        aURL.Append( aName );

        if( !aURL.getExtension().Len() )
            aURL.setExtension( String( pszExtBitmap, 3 ) );

        SfxMedium aMedium( aURL.GetMainURL(), STREAM_READ | STREAM_NOCREATE, TRUE );

        SvStream* pStream = aMedium.GetInStream();
        if( !pStream )
            return( FALSE );

        String aCheck;
        // UNICODE: *pStream >> aCheck;
        pStream->ReadByteString(aCheck);

        // Handelt es sich um die gew"unschte Tabelle?
        if( aCheck == String(pszChckBitmap, 4) ||
            aCheck == String(pszChckBitmap0, 4) ||
            aCheck == String(pszChckBitmap1, 4) )
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

BOOL XBitmapList::Save()
{
#ifndef SVX_LIGHT

    INetURLObject aURL( aPath );

    if( INET_PROT_NOT_VALID == aURL.GetProtocol() )
    {
        DBG_ERROR( "invalid URL" );
        return FALSE;
    }

    aURL.Append( aName );

    if( !aURL.getExtension().Len() )
        aURL.setExtension( String( pszExtBitmap, 3 ) );

    SfxMedium aMedium( aURL.GetMainURL(), STREAM_WRITE | STREAM_TRUNC, TRUE );
    aMedium.IsRemote();

    SvStream* pStream = aMedium.GetOutStream();
    if( !pStream )
        return( FALSE );

    // UNICODE: *pStream << String( pszChckBitmap1, 4 );
    pStream->WriteByteString(String( pszChckBitmap1, 4 ));
    ImpStore( *pStream );

    aMedium.Close();
    aMedium.Commit();

    return( aMedium.GetError() == 0 );
#else
    return FALSE;
#endif
}

/************************************************************************/
// Umgestellt am 27.07.95 auf XBitmap

BOOL XBitmapList::Create()
{
    // Array der Bitmap
    //-----------------------
    // 00 01 02 03 04 05 06 07
    // 08 09 10 11 12 13 14 15
    // 16 17 18 19 20 21 22 23
    // 24 25 26 27 28 29 30 31
    // 32 33 34 35 36 37 38 39
    // 40 41 42 43 44 45 46 47
    // 48 49 50 51 52 53 54 55
    // 56 57 58 59 60 61 62 63

    String  aStr( SVX_RES( RID_SVXSTR_BITMAP ) );
    Color   aColWhite( RGB_Color( COL_WHITE ) );
    xub_StrLen nLen;
    USHORT  aArray[64];

    memset( aArray, 0, sizeof( aArray ) );
    aStr.AppendAscii(" 1");
    nLen = aStr.Len() - 1;
    Insert( new XBitmapEntry( XOBitmap( aArray, aColWhite, aColWhite ), aStr ) );

    aArray[ 0] = 1; aArray[ 9] = 1; aArray[18] = 1; aArray[27] = 1;
    aArray[36] = 1; aArray[45] = 1; aArray[54] = 1; aArray[63] = 1;
    aStr.SetChar(nLen, sal_Unicode('2'));
    Insert( new XBitmapEntry( XOBitmap( aArray, RGB_Color( COL_BLACK ), aColWhite ), aStr ) );

    aArray[ 7] = 1; aArray[14] = 1; aArray[21] = 1; aArray[28] = 1;
    aArray[35] = 1; aArray[42] = 1; aArray[49] = 1; aArray[56] = 1;
    aStr.SetChar(nLen, sal_Unicode('3'));
    Insert( new XBitmapEntry( XOBitmap( aArray, RGB_Color( COL_LIGHTRED ), aColWhite ), aStr ) );

    aArray[24] = 1; aArray[25] = 1; aArray[26] = 1;
    aArray[29] = 1; aArray[30] = 1; aArray[31] = 1;
    aStr.SetChar(nLen, sal_Unicode('4'));
    Insert( new XBitmapEntry( XOBitmap( aArray, RGB_Color( COL_LIGHTBLUE ), aColWhite ), aStr ) );

    return( TRUE );
}

/************************************************************************/

BOOL XBitmapList::CreateBitmapsForUI()
{
    return( FALSE );
}

/************************************************************************/

Bitmap* XBitmapList::CreateBitmapForUI( long nIndex, BOOL bDelete )
{
    return( NULL );
}

/************************************************************************/

SvStream& XBitmapList::ImpStore( SvStream& rOut )
{
    // Schreiben
    rOut.SetStreamCharSet( gsl_getSystemTextEncoding() );

    // Version !!!
    rOut << (long)-2;

    // Anzahl der Eintraege
    rOut << (long)Count();

    // die Eintraege
    XBitmapEntry* pEntry = NULL;

    for (long nIndex = 0; nIndex < Count(); nIndex++)
    {
        // Versionsverwaltung: Version 0
        XIOCompat aIOC( rOut, STREAM_WRITE, 0 );

        pEntry = Get(nIndex);
        XOBitmap aXOBitmap( pEntry->GetXBitmap() );

        // UNICODE: rOut << pEntry->GetName();
        rOut.WriteByteString(pEntry->GetName());

        rOut << (INT16) aXOBitmap.GetBitmapStyle();
        rOut << (INT16) aXOBitmap.GetBitmapType();

        if( aXOBitmap.GetBitmapType() == XBITMAP_IMPORT )
        {
            rOut << aXOBitmap.GetBitmap();
        }
        else if( aXOBitmap.GetBitmapType() == XBITMAP_8X8 )
        {
            USHORT* pArray = aXOBitmap.GetPixelArray();
            for( USHORT i = 0; i < 64; i++ )
                rOut << (USHORT) *( pArray + i );

            rOut << aXOBitmap.GetPixelColor();
            rOut << aXOBitmap.GetBackgroundColor();
        }
    }

    return rOut;
}

/************************************************************************/

XubString& XBitmapList::ConvertName( XubString& rStrName )
{
    BOOL bFound = FALSE;

    for( USHORT i=0; i<(RID_SVXSTR_BMP_DEF_END-RID_SVXSTR_BMP_DEF_START+1) && !bFound; i++ )
    {
        XubString aStrDefName = SVX_RESSTR( RID_SVXSTR_BMP_DEF_START + i );
        if( rStrName.Search( aStrDefName ) == 0 )
        {
            rStrName.Replace( 0, aStrDefName.Len(), SVX_RESSTR( RID_SVXSTR_BMP_START + i ) );
            bFound = TRUE;
        }
    }

    return rStrName;
}

/************************************************************************/

SvStream& XBitmapList::ImpRead( SvStream& rIn )
{
    // Lesen
    rIn.SetStreamCharSet( RTL_TEXTENCODING_IBM_850 );

    delete pBmpList;
    pBmpList = new List( 16, 16 );

    XBitmapEntry* pEntry = NULL;
    long        nCount;
    String      aName;

    rIn >> nCount; // Version oder Anzahl ?

    if( nCount >= 0 ) // Erste Version
    {
        for( long nIndex = 0; nIndex < nCount; nIndex++ )
        {
            // Behandlung der alten Bitmaps
            Bitmap  aBmp;
            XOBitmap    aXOBitmap;

            // UNICODE: rIn >> aName;
            rIn.ReadByteString(aName);

            aName = ConvertName( aName );
            rIn >> aBmp;

            aXOBitmap.SetBitmap( aBmp );
            aXOBitmap.SetBitmapStyle( XBITMAP_TILE );

            if( aBmp.GetSizePixel().Width() == 8 &&
                aBmp.GetSizePixel().Height() == 8 )
            {
                aXOBitmap.SetBitmapType( XBITMAP_8X8 );
                aXOBitmap.Bitmap2Array();
            }
            else
                aXOBitmap.SetBitmapType( XBITMAP_IMPORT );

            pEntry = new XBitmapEntry( aXOBitmap, aName );
            Insert( pEntry, nIndex );
        }
    }
    else if( nCount == -1 ) // Zweite Version
    {
        rIn >> nCount;
        for( long nIndex = 0; nIndex < nCount; nIndex++ )
        {
            // UNICODE: rIn >> aName;
            rIn.ReadByteString(aName);

            aName = ConvertName( aName );

            XOBitmap aXOBitmap;
            INT16   iTmp;

            rIn >> iTmp;
            aXOBitmap.SetBitmapStyle( (XBitmapStyle) iTmp );
            rIn >> iTmp;
            aXOBitmap.SetBitmapType( (XBitmapType) iTmp );

            if( aXOBitmap.GetBitmapType() == XBITMAP_IMPORT )
            {
                Bitmap aBmp;
                rIn >> aBmp;
                aXOBitmap.SetBitmap( aBmp );
            }
            else if( aXOBitmap.GetBitmapType() == XBITMAP_8X8 )
            {
                USHORT* pArray = new USHORT[ 64 ];
                Color   aColor;

                for( USHORT i = 0; i < 64; i++ )
                    rIn >> *( pArray + i );
                aXOBitmap.SetPixelArray( pArray );

                rIn >> aColor;
                aXOBitmap.SetPixelColor( aColor );
                rIn >> aColor;
                aXOBitmap.SetBackgroundColor( aColor );

                delete []pArray;
            }

            pEntry = new XBitmapEntry( aXOBitmap, aName );
            Insert( pEntry, nIndex );
        }
    }
    else // Version ab 3.00a
    {
        rIn >> nCount;
        for( long nIndex = 0; nIndex < nCount; nIndex++ )
        {
            // Versionsverwaltung
            XIOCompat aIOC( rIn, STREAM_READ );

            // UNICODE: rIn >> aName;
            rIn.ReadByteString(aName);

            aName = ConvertName( aName );

            XOBitmap aXOBitmap;
            INT16   iTmp;

            rIn >> iTmp;
            aXOBitmap.SetBitmapStyle( (XBitmapStyle) iTmp );
            rIn >> iTmp;
            aXOBitmap.SetBitmapType( (XBitmapType) iTmp );

            if( aXOBitmap.GetBitmapType() == XBITMAP_IMPORT )
            {
                Bitmap aBmp;
                rIn >> aBmp;
                aXOBitmap.SetBitmap( aBmp );
            }
            else if( aXOBitmap.GetBitmapType() == XBITMAP_8X8 )
            {
                USHORT* pArray = new USHORT[ 64 ];
                Color   aColor;

                for( USHORT i = 0; i < 64; i++ )
                    rIn >> *( pArray + i );
                aXOBitmap.SetPixelArray( pArray );

                rIn >> aColor;
                aXOBitmap.SetPixelColor( aColor );
                rIn >> aColor;
                aXOBitmap.SetBackgroundColor( aColor );

                delete []pArray;
            }

            if (aIOC.GetVersion() > 0)
            {
                // lesen neuer Daten ...
            }

            pEntry = new XBitmapEntry( aXOBitmap, aName );
            Insert( pEntry, nIndex );
        }
    }

    return( rIn );
}



