/*************************************************************************
 *
 *  $RCSfile: xtabgrdt.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ka $ $Date: 2000-10-17 13:24:03 $
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

// include ---------------------------------------------------------------

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

#ifndef SVX_XFILLIT0_HXX //autogen
#include <xfillit0.hxx>
#endif

#ifndef _SVX_XFLGRIT_HXX //autogen
#include <xflgrit.hxx>
#endif

#define GLOBALOVERFLOW

sal_Unicode const pszExtGradient[]  = {'s','o','g'};
sal_Unicode const pszChckGradient[] = {'S','O','G','L'};
// Neuer Key, damit alte Version (3.00) nicht bei dem
// Versuch abstuerzt, eine neue Tabelle zu laden.
sal_Unicode const pszChckGradient0[]    = {'S','O','G','0'};

// ---------------------
// class XGradientTable
// ---------------------

/*************************************************************************
|*
|* XGradientTable::XGradientTable()
|*
*************************************************************************/

XGradientTable::XGradientTable( const String& rPath,
                            XOutdevItemPool* pInPool,
                            USHORT nInitSize, USHORT nReSize ) :
                XPropertyTable( rPath, pInPool, nInitSize, nReSize)
{
    pBmpTable = new Table( nInitSize, nReSize );
}

/************************************************************************/

XGradientTable::~XGradientTable()
{
}

/************************************************************************/

XGradientEntry* XGradientTable::Replace(long nIndex, XGradientEntry* pEntry )
{
    return (XGradientEntry*) XPropertyTable::Replace(nIndex, pEntry);
}

/************************************************************************/

XGradientEntry* XGradientTable::Remove(long nIndex)
{
    return (XGradientEntry*) XPropertyTable::Remove(nIndex, 0);
}

/************************************************************************/

XGradientEntry* XGradientTable::Get(long nIndex) const
{
    return (XGradientEntry*) XPropertyTable::Get(nIndex, 0);
}

/************************************************************************/

BOOL XGradientTable::Load()
{
    return( FALSE );
}

/************************************************************************/

BOOL XGradientTable::Save()
{
    return( FALSE );
}

/************************************************************************/

BOOL XGradientTable::Create()
{
    return( FALSE );
}

/************************************************************************/

BOOL XGradientTable::CreateBitmapsForUI()
{
    return( FALSE );
}

/************************************************************************/

Bitmap* XGradientTable::CreateBitmapForUI( long nIndex, BOOL bDelete )
{
    return( NULL );
}

/************************************************************************/

SvStream& XGradientTable::ImpStore( SvStream& rOut )
{
    // Schreiben
    rOut.SetStreamCharSet( gsl_getSystemTextEncoding() );

    // Tabellentyp schreiben (0 = gesamte Tabelle)
    rOut << (long)0;

    // Anzahl der Eintraege
    rOut << (long)Count();

    // die Eintraege
    XGradientEntry* pEntry = (XGradientEntry*)aTable.First();

    for (long nIndex = 0; nIndex < Count(); nIndex++)
    {
        rOut << (long)aTable.GetCurKey();

        // UNICODE: rOut << pEntry->GetName();
        rOut.WriteByteString(pEntry->GetName());

        XGradient& rGradient = pEntry->GetGradient();
        rOut << (long)rGradient.GetGradientStyle();
        rOut << rGradient.GetStartColor().GetRed();
        rOut << rGradient.GetStartColor().GetGreen();
        rOut << rGradient.GetStartColor().GetBlue();
        rOut << rGradient.GetEndColor().GetRed();
        rOut << rGradient.GetEndColor().GetGreen();
        rOut << rGradient.GetEndColor().GetBlue();
        rOut << rGradient.GetAngle();
        rOut << (ULONG)rGradient.GetBorder();
        rOut << (ULONG)rGradient.GetXOffset();
        rOut << (ULONG)rGradient.GetYOffset();
        pEntry = (XGradientEntry*)aTable.Next();
    }

    return rOut;
}

/************************************************************************/

SvStream& XGradientTable::ImpRead( SvStream& rIn )
{
    // Lesen
    rIn.SetStreamCharSet( RTL_TEXTENCODING_IBM_850 );

    delete pBmpTable;
    pBmpTable = new Table( 16, 16 );

    XGradientEntry* pEntry = NULL;
    long        nType;
    long        nCount;
    long        nIndex;
    XubString       aName;

    long    nStyle;
    USHORT  nRed;
    USHORT  nGreen;
    USHORT  nBlue;
    Color   aStart;
    Color   aEnd;
    long    nAngle;
    ULONG   nBorder;
    ULONG   nXOfs;
    ULONG   nYOfs;

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

            rIn >> nStyle;
            rIn >> nRed;
            rIn >> nGreen;
            rIn >> nBlue;

#ifdef VCL
            aStart = Color( (BYTE) ( nRed   >> 8 ),
                            (BYTE) ( nGreen >> 8 ),
                            (BYTE) ( nBlue  >> 8 ) );
#else
            aStart = Color( (USHORT)nRed, (USHORT)nGreen, (USHORT)nBlue );
#endif
            rIn >> nRed;
            rIn >> nGreen;
            rIn >> nBlue;

#ifdef VCL
            aEnd = Color( (BYTE) ( nRed   >> 8 ),
                            (BYTE) ( nGreen >> 8 ),
                            (BYTE) ( nBlue  >> 8 ) );
#else
            aEnd = Color( (USHORT)nRed, (USHORT)nGreen, (USHORT)nBlue );
#endif

            rIn >> nAngle;
            rIn >> nBorder;
            rIn >> nXOfs;
            rIn >> nYOfs;

            XGradient aGradient(aStart, aEnd, (XGradientStyle)nStyle, nAngle,
                                (USHORT)nXOfs, (USHORT)nYOfs, (USHORT)nBorder);
            pEntry = new XGradientEntry (aGradient, aName);
            Insert (nIndex, pEntry);
        }
    }
    return( rIn );
}

// --------------------
// class XGradientList
// --------------------

/*************************************************************************
|*
|* XGradientList::XGradientList()
|*
*************************************************************************/

XGradientList::XGradientList( const String& rPath,
                            XOutdevItemPool* pInPool,
                            USHORT nInitSize, USHORT nReSize ) :
                XPropertyList   ( rPath, pInPool, nInitSize, nReSize),
                pVD             ( NULL ),
                pXOut           ( NULL ),
                pXFSet          ( NULL )
{
    pBmpList = new List( nInitSize, nReSize );
}

/************************************************************************/

XGradientList::~XGradientList()
{
    if( pVD )    delete pVD;
    if( pXOut )  delete pXOut;
    if( pXFSet ) delete pXFSet;
}

/************************************************************************/

XGradientEntry* XGradientList::Replace(XGradientEntry* pEntry, long nIndex )
{
    return( (XGradientEntry*) XPropertyList::Replace( pEntry, nIndex ) );
}

/************************************************************************/

XGradientEntry* XGradientList::Remove(long nIndex)
{
    return( (XGradientEntry*) XPropertyList::Remove( nIndex, 0 ) );
}

/************************************************************************/

XGradientEntry* XGradientList::Get(long nIndex) const
{
    return( (XGradientEntry*) XPropertyList::Get( nIndex, 0 ) );
}

/************************************************************************/

BOOL XGradientList::Load()
{
#ifndef SVX_LIGHT
    if( bListDirty )
    {
        bListDirty = FALSE;

        INetURLObject aURL;

        aURL.SetSmartURL( aPath );
        aURL.Append( aName );

        if( !aURL.getExtension().Len() )
            aURL.setExtension( String( pszExtGradient, 3 ) );

        SfxMedium aMedium( aURL.PathToFileName(), STREAM_READ | STREAM_NOCREATE, TRUE );
        SvStream* pStream = aMedium.GetInStream();
        if( !pStream )
            return( FALSE );

        String aCheck;
        // UNICODE: *pStream >> aCheck;
        pStream->ReadByteString(aCheck);

        // Handelt es sich um die gew"unschte Tabelle?
        if( aCheck == String(pszChckGradient, 4) ||
            aCheck == String(pszChckGradient0, 4) )
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

BOOL XGradientList::Save()
{
#ifndef SVX_LIGHT
    INetURLObject aURL;

    aURL.SetSmartURL( aPath );
    aURL.Append( aName );

    if( !aURL.getExtension().Len() )
        aURL.setExtension( String( pszExtGradient, 3 ) );

    SfxMedium aMedium( aURL.PathToFileName(), STREAM_WRITE | STREAM_TRUNC, TRUE );
    aMedium.IsRemote();

    SvStream* pStream = aMedium.GetOutStream();
    if( !pStream )
        return( FALSE );

    // UNICODE: *pStream << String( pszChckGradient0, 4 );
    pStream->WriteByteString(String( pszChckGradient0, 4 ));

    ImpStore( *pStream );

    aMedium.Close();
    aMedium.Commit();

    return( aMedium.GetError() == 0 );
#else
    return FALSE;
#endif
}

/************************************************************************/

BOOL XGradientList::Create()
{
    XubString aStr( SVX_RES( RID_SVXSTR_GRADIENT ) );
    xub_StrLen nLen;

    aStr.AppendAscii(" 1");
    nLen = aStr.Len() - 1;
    Insert(new XGradientEntry(XGradient(RGB_Color(COL_BLACK  ),RGB_Color(COL_WHITE  ),XGRAD_LINEAR    ,    0,10,10, 0,100,100),aStr));
    aStr.SetChar(nLen, sal_Unicode('2'));
    Insert(new XGradientEntry(XGradient(RGB_Color(COL_BLUE   ),RGB_Color(COL_RED    ),XGRAD_AXIAL     ,  300,20,20,10,100,100),aStr));
    aStr.SetChar(nLen, sal_Unicode('3'));
    Insert(new XGradientEntry(XGradient(RGB_Color(COL_RED    ),RGB_Color(COL_YELLOW ),XGRAD_RADIAL    ,  600,30,30,20,100,100),aStr));
    aStr.SetChar(nLen, sal_Unicode('4'));
    Insert(new XGradientEntry(XGradient(RGB_Color(COL_YELLOW ),RGB_Color(COL_GREEN  ),XGRAD_ELLIPTICAL,  900,40,40,30,100,100),aStr));
    aStr.SetChar(nLen, sal_Unicode('5'));
    Insert(new XGradientEntry(XGradient(RGB_Color(COL_GREEN  ),RGB_Color(COL_MAGENTA),XGRAD_SQUARE    , 1200,50,50,40,100,100),aStr));
    aStr.SetChar(nLen, sal_Unicode('6'));
    Insert(new XGradientEntry(XGradient(RGB_Color(COL_MAGENTA),RGB_Color(COL_YELLOW ),XGRAD_RECT      , 1900,60,60,50,100,100),aStr));

    return( TRUE );
}

/************************************************************************/

BOOL XGradientList::CreateBitmapsForUI()
{
    for( long i = 0; i < Count(); i++)
    {
        Bitmap* pBmp = CreateBitmapForUI( i, FALSE );
        DBG_ASSERT( pBmp, "XGradientList: Bitmap(UI) konnte nicht erzeugt werden!" );

        if( pBmp )
            pBmpList->Insert( pBmp, i );
    }
    // Loeschen, da JOE den Pool vorm Dtor entfernt!
    if( pVD )   { delete pVD;   pVD = NULL;     }
    if( pXOut ) { delete pXOut; pXOut = NULL;   }
    if( pXFSet ){ delete pXFSet; pXFSet = NULL; }

    return( FALSE );
}

/************************************************************************/

Bitmap* XGradientList::CreateBitmapForUI( long nIndex, BOOL bDelete )
{
    if( !pVD ) // und pXOut und pXFSet
    {
        pVD = new VirtualDevice;
        DBG_ASSERT( pVD, "XGradientList: Konnte kein VirtualDevice erzeugen!" );
        pVD->SetOutputSizePixel( Size( BITMAP_WIDTH, BITMAP_HEIGHT ) );

        pXOut = new XOutputDevice( pVD );
        DBG_ASSERT( pVD, "XGradientList: Konnte kein XOutDevice erzeugen!" );

        pXFSet = new XFillAttrSetItem( pXPool );
        DBG_ASSERT( pVD, "XGradientList: Konnte kein XFillAttrSetItem erzeugen!" );
        pXFSet->GetItemSet().Put( XFillStyleItem( XFILL_GRADIENT ) );
    }

    pXFSet->GetItemSet().Put(
        XFillGradientItem( pXPool, Get( nIndex )->GetGradient() ) );
    pXOut->SetFillAttr( *pXFSet );

    // #73550#
    pXOut->OverrideLineColor( Color( COL_BLACK ) );

    Size aVDSize = pVD->GetOutputSizePixel();
    pXOut->DrawRect( Rectangle( Point(), aVDSize ) );
    Bitmap* pBitmap = new Bitmap( pVD->GetBitmap( Point(), aVDSize ) );

    // Loeschen, da JOE den Pool vorm Dtor entfernt!
    if( bDelete )
    {
        if( pVD )   { delete pVD;   pVD = NULL;     }
        if( pXOut ) { delete pXOut; pXOut = NULL;   }
        if( pXFSet ){ delete pXFSet; pXFSet = NULL; }
    }
    return( pBitmap );
}

/************************************************************************/

SvStream& XGradientList::ImpStore( SvStream& rOut )
{
    // Schreiben
    rOut.SetStreamCharSet( gsl_getSystemTextEncoding() );

    XGradientEntry* pEntry = NULL;

    // Kennung
    rOut << (long) -2;

    // Anzahl der Eintraege
    rOut << (long)Count();

    for (long nIndex = 0; nIndex < Count(); nIndex++)
    {
        // Versionsverwaltung: Version 0
        XIOCompat aIOC( rOut, STREAM_WRITE, 0 );

        pEntry = Get(nIndex);

        // UNICODE: rOut << pEntry->GetName();
        rOut.WriteByteString(pEntry->GetName());

        XGradient& rGradient = pEntry->GetGradient();
        rOut << (long)rGradient.GetGradientStyle();

#ifdef VCL
        USHORT nCol = rGradient.GetStartColor().GetRed();
        nCol = nCol << 8;
        rOut << nCol;
        nCol = rGradient.GetStartColor().GetGreen();
        nCol = nCol << 8;
        rOut << nCol;
        nCol = rGradient.GetStartColor().GetBlue();
        nCol = nCol << 8;
        rOut << nCol;

        nCol = rGradient.GetEndColor().GetRed();
        nCol = nCol << 8;
        rOut << nCol;
        nCol = rGradient.GetEndColor().GetGreen();
        nCol = nCol << 8;
        rOut << nCol;
        nCol = rGradient.GetEndColor().GetBlue();
        nCol = nCol << 8;
        rOut << nCol;
#else
        rOut << rGradient.GetStartColor().GetRed();
        rOut << rGradient.GetStartColor().GetGreen();
        rOut << rGradient.GetStartColor().GetBlue();

        rOut << rGradient.GetEndColor().GetRed();
        rOut << rGradient.GetEndColor().GetGreen();
        rOut << rGradient.GetEndColor().GetBlue();
#endif
        rOut << rGradient.GetAngle();
        rOut << (ULONG)rGradient.GetBorder();
        rOut << (ULONG)rGradient.GetXOffset();
        rOut << (ULONG)rGradient.GetYOffset();
        rOut << (ULONG)rGradient.GetStartIntens();
        rOut << (ULONG)rGradient.GetEndIntens();
    }

    return rOut;
}

/************************************************************************/

XubString& XGradientList::ConvertName( XubString& rStrName )
{
    BOOL bFound = FALSE;

    for( USHORT i=0; i<(RID_SVXSTR_GRDT_DEF_END-RID_SVXSTR_GRDT_DEF_START+1) && !bFound; i++ )
    {
        XubString aStrDefName = SVX_RESSTR( RID_SVXSTR_GRDT_DEF_START + i );
        if( rStrName.Search( aStrDefName ) == 0 )
        {
            rStrName.Replace( 0, aStrDefName.Len(), SVX_RESSTR( RID_SVXSTR_GRDT_START + i ) );
            bFound = TRUE;
        }
    }

    return rStrName;
}

/************************************************************************/

SvStream& XGradientList::ImpRead( SvStream& rIn )
{
    // Lesen
    rIn.SetStreamCharSet( RTL_TEXTENCODING_IBM_850 );

    delete pBmpList;
    pBmpList = new List( 16, 16 );

    XGradientEntry* pEntry = NULL;
    long        nCheck;
    long        nCount;
    XubString       aName;

    long    nStyle;
    USHORT  nRed;
    USHORT  nGreen;
    USHORT  nBlue;
    Color   aStart;
    Color   aEnd;
    long    nAngle;
    ULONG   nBorder;
    ULONG   nXOfs;
    ULONG   nYOfs;
    ULONG   nStartIntens;
    ULONG   nEndIntens;

    // Kennung oder Anzahl
    rIn >> nCheck;

    if( nCheck >= 0 )
    {
        nCount = nCheck;

        for (long nIndex = 0; nIndex < nCount; nIndex++)
        {
            // UNICODE: rIn >> aName;
            rIn.ReadByteString(aName);

            aName = ConvertName( aName );
            rIn >> nStyle;
            rIn >> nRed;
            rIn >> nGreen;
            rIn >> nBlue;
#ifdef VCL
            aStart = Color( (BYTE) ( nRed   >> 8 ),
                            (BYTE) ( nGreen >> 8 ),
                            (BYTE) ( nBlue  >> 8 ) );
#else
            aStart = Color( (USHORT)nRed, (USHORT)nGreen, (USHORT)nBlue );
#endif
            rIn >> nRed;
            rIn >> nGreen;
            rIn >> nBlue;
#ifdef VCL
            aEnd = Color( (BYTE) ( nRed   >> 8 ),
                            (BYTE) ( nGreen >> 8 ),
                            (BYTE) ( nBlue  >> 8 ) );
#else
            aEnd = Color( (USHORT)nRed, (USHORT)nGreen, (USHORT)nBlue );
#endif

            rIn >> nAngle;
            rIn >> nBorder;
            rIn >> nXOfs;
            rIn >> nYOfs;

            nStartIntens = 100L;
            nEndIntens = 100L;

            XGradient aGradient( aStart, aEnd, (XGradientStyle)nStyle, nAngle,
                                 (USHORT) nXOfs, (USHORT) nYOfs, (USHORT) nBorder,
                                 (USHORT) nStartIntens, (USHORT) nEndIntens );
            pEntry = new XGradientEntry (aGradient, aName);
            Insert (pEntry, nIndex);
        }
    }
    else if( nCheck == -1L )
    {
        rIn >> nCount;
        for (long nIndex = 0; nIndex < nCount; nIndex++)
        {
            // UNICODE: rIn >> aName;
            rIn.ReadByteString(aName);

            aName = ConvertName( aName );
            rIn >> nStyle;
            rIn >> nRed;
            rIn >> nGreen;
            rIn >> nBlue;
#ifdef VCL
            aStart = Color( (BYTE) ( nRed   >> 8 ),
                            (BYTE) ( nGreen >> 8 ),
                            (BYTE) ( nBlue  >> 8 ) );
#else
            aStart = Color( (USHORT)nRed, (USHORT)nGreen, (USHORT)nBlue );
#endif
            rIn >> nRed;
            rIn >> nGreen;
            rIn >> nBlue;
#ifdef VCL
            aEnd = Color( (BYTE) ( nRed   >> 8 ),
                            (BYTE) ( nGreen >> 8 ),
                            (BYTE) ( nBlue  >> 8 ) );
#else
            aEnd = Color( (USHORT)nRed, (USHORT)nGreen, (USHORT)nBlue );
#endif

            rIn >> nAngle;
            rIn >> nBorder;
            rIn >> nXOfs;
            rIn >> nYOfs;

            rIn >> nStartIntens;
            rIn >> nEndIntens;

            XGradient aGradient( aStart, aEnd, (XGradientStyle)nStyle, nAngle,
                                 (USHORT) nXOfs, (USHORT) nYOfs, (USHORT) nBorder,
                                 (USHORT) nStartIntens, (USHORT) nEndIntens );
            pEntry = new XGradientEntry (aGradient, aName);
            Insert (pEntry, nIndex);
        }
    }
    else // ab 3.00a
    {
        rIn >> nCount;
        for (long nIndex = 0; nIndex < nCount; nIndex++)
        {
            // Versionsverwaltung
            XIOCompat aIOC( rIn, STREAM_READ );

            // UNICODE: rIn >> aName;
            rIn.ReadByteString(aName);

            aName = ConvertName( aName );
            rIn >> nStyle;
            rIn >> nRed;
            rIn >> nGreen;
            rIn >> nBlue;
#ifdef VCL
            aStart = Color( (BYTE) ( nRed   >> 8 ),
                            (BYTE) ( nGreen >> 8 ),
                            (BYTE) ( nBlue  >> 8 ) );
#else
            aStart = Color( (USHORT)nRed, (USHORT)nGreen, (USHORT)nBlue );
#endif
            rIn >> nRed;
            rIn >> nGreen;
            rIn >> nBlue;
#ifdef VCL
            aEnd = Color( (BYTE) ( nRed   >> 8 ),
                            (BYTE) ( nGreen >> 8 ),
                            (BYTE) ( nBlue  >> 8 ) );
#else
            aEnd = Color( (USHORT)nRed, (USHORT)nGreen, (USHORT)nBlue );
#endif

            rIn >> nAngle;
            rIn >> nBorder;
            rIn >> nXOfs;
            rIn >> nYOfs;

            rIn >> nStartIntens;
            rIn >> nEndIntens;

            if (aIOC.GetVersion() > 0)
            {
                // lesen neuer Daten ...
            }

            XGradient aGradient( aStart, aEnd, (XGradientStyle)nStyle, nAngle,
                                 (USHORT) nXOfs, (USHORT) nYOfs, (USHORT) nBorder,
                                 (USHORT) nStartIntens, (USHORT) nEndIntens );
            pEntry = new XGradientEntry (aGradient, aName);
            Insert (pEntry, nIndex);
        }
    }
    return( rIn );
}


