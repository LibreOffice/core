/*************************************************************************
 *
 *  $RCSfile: xtabhtch.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: ka $ $Date: 2001-02-19 17:20:45 $
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

#ifndef _SVX_XFLHTIT_HXX //autogen
#include <xflhtit.hxx>
#endif

#ifndef _SVX_XFLCLIT_HXX //autogen
#include <xflclit.hxx>
#endif

#ifndef SVX_XFILLIT0_HXX //autogen
#include <xfillit0.hxx>
#endif

#define GLOBALOVERFLOW
#include <segmentc.hxx>

sal_Unicode const pszExtHatch[]  = {'s','o','h'};
sal_Unicode const pszChckHatch[] = {'S','O','H','L'};
// Neuer Key, damit alte Version (3.00) nicht bei dem
// Versuch abstuerzt, eine neue Tabelle zu laden.
sal_Unicode const pszChckHatch0[] = {'S','O','H','0'};

// ------------------
// class XHatchTable
// ------------------

/*************************************************************************
|*
|* XHatchTable::XHatchTable()
|*
*************************************************************************/

XHatchTable::XHatchTable( const String& rPath,
                            XOutdevItemPool* pInPool,
                            USHORT nInitSize, USHORT nReSize ) :
                XPropertyTable( rPath, pInPool, nInitSize, nReSize)
{
    pBmpTable = new Table( nInitSize, nReSize );
}

/************************************************************************/

XHatchTable::~XHatchTable()
{
}

/************************************************************************/

XHatchEntry* XHatchTable::Replace(long nIndex, XHatchEntry* pEntry )
{
    return (XHatchEntry*) XPropertyTable::Replace(nIndex, pEntry);
}

/************************************************************************/

XHatchEntry* XHatchTable::Remove(long nIndex)
{
    return (XHatchEntry*) XPropertyTable::Remove(nIndex, 0);
}

/************************************************************************/

XHatchEntry* XHatchTable::Get(long nIndex) const
{
    return (XHatchEntry*) XPropertyTable::Get(nIndex, 0);
}

/************************************************************************/

BOOL XHatchTable::Load()
{
    return( FALSE );
}

/************************************************************************/

BOOL XHatchTable::Save()
{
    return( FALSE );
}

/************************************************************************/

BOOL XHatchTable::Create()
{
    return( FALSE );
}

/************************************************************************/

BOOL XHatchTable::CreateBitmapsForUI()
{
    return( FALSE );
}

/************************************************************************/

Bitmap* XHatchTable::CreateBitmapForUI( long nIndex, BOOL bDelete )
{
    return( NULL );
}

/************************************************************************/

SvStream& XHatchTable::ImpStore( SvStream& rOut )
{
    // Schreiben
    rOut.SetStreamCharSet( gsl_getSystemTextEncoding() );

    // Tabellentyp schreiben (0 = gesamte Tabelle)
    rOut << (long)0;

    // Anzahl der Eintraege
    rOut << (long)Count();

    // die Eintraege
    XHatchEntry* pEntry = (XHatchEntry*)aTable.First();;
    for (long nIndex = 0; nIndex < Count(); nIndex++)
    {
        rOut << (long)aTable.GetCurKey();

        // UNICODE: rOut << pEntry->GetName();
        rOut.WriteByteString(pEntry->GetName());

        XHatch& rHatch = pEntry->GetHatch();
        rOut << (long)rHatch.GetHatchStyle();
        rOut << rHatch.GetColor().GetRed();
        rOut << rHatch.GetColor().GetGreen();
        rOut << rHatch.GetColor().GetBlue();
        rOut << rHatch.GetDistance();
        rOut << rHatch.GetAngle();
        pEntry = (XHatchEntry*)aTable.Next();
    }

    return rOut;
}

/************************************************************************/

SvStream& XHatchTable::ImpRead( SvStream& rIn )
{
    // Lesen
    rIn.SetStreamCharSet( RTL_TEXTENCODING_IBM_850 );

    delete pBmpTable;
    pBmpTable = new Table( 16, 16 );

    XHatchEntry* pEntry = NULL;
    long        nType;
    long        nCount;
    long        nIndex;
    XubString   aName;

    long        nStyle;
    USHORT      nRed;
    USHORT      nGreen;
    USHORT      nBlue;
    long        nDistance;
    long        nAngle;

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
            rIn >> nDistance;
            rIn >> nAngle;

            Color aColor ( (BYTE) nRed, (BYTE) nGreen, (BYTE) nBlue);
            XHatch aHatch(aColor, (XHatchStyle)nStyle, nDistance, nAngle);
            pEntry = new XHatchEntry (aHatch, aName);
            Insert (nIndex, pEntry);
        }
    }
    return( rIn );
}

// -----------------
// class XHatchList
// -----------------

/*************************************************************************
|*
|* XHatchList::XHatchList()
|*
*************************************************************************/

XHatchList::XHatchList( const String& rPath,
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

XHatchList::~XHatchList()
{
    if( pVD )    delete pVD;
    if( pXOut )  delete pXOut;
    if( pXFSet ) delete pXFSet;
}

/************************************************************************/

XHatchEntry* XHatchList::Replace(XHatchEntry* pEntry, long nIndex )
{
    return (XHatchEntry*) XPropertyList::Replace(pEntry, nIndex);
}

/************************************************************************/

XHatchEntry* XHatchList::Remove(long nIndex)
{
    return (XHatchEntry*) XPropertyList::Remove(nIndex, 0);
}

/************************************************************************/

XHatchEntry* XHatchList::Get(long nIndex) const
{
    return (XHatchEntry*) XPropertyList::Get(nIndex, 0);
}

/************************************************************************/

BOOL XHatchList::Load()
{
#ifndef SVX_LIGHT
    if( bListDirty )
    {
        bListDirty = FALSE;

        INetURLObject aURL( aPath );

        if( INET_PROT_NOT_VALID == aURL.GetProtocol() )
        {
            DBG_ASSERT( !aPath.Len(), "invalid URL" );
            return FALSE;
        }

        aURL.Append( aName );

        if( !aURL.getExtension().Len() )
            aURL.setExtension( String( pszExtHatch, 3 ) );

        SfxMedium aMedium( aURL.GetMainURL(), STREAM_READ | STREAM_NOCREATE, TRUE );
        SvStream* pStream = aMedium.GetInStream();
        if( !pStream )
            return( FALSE );

        String aCheck;
        // UNICODE: *pStream >> aCheck;
        pStream->ReadByteString(aCheck);

        // Handelt es sich um die gew"unschte Tabelle?
        if( aCheck == String(pszChckHatch, 4) ||
            aCheck == String(pszChckHatch0, 4) )
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

BOOL XHatchList::Save()
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
        aURL.setExtension( String( pszExtHatch, 3 ) );

    SfxMedium aMedium( aURL.GetMainURL(), STREAM_WRITE | STREAM_TRUNC, TRUE );
    aMedium.IsRemote();

    SvStream* pStream = aMedium.GetOutStream();
    if( !pStream )
        return( FALSE );

    // UNICODE: *pStream << String( pszChckHatch0, 4 );
    pStream->WriteByteString(String( pszChckHatch0, 4 ));

    ImpStore( *pStream );

    aMedium.Close();
    aMedium.Commit();

    return( aMedium.GetError() == 0 );
#else
    return FALSE;
#endif
}

/************************************************************************/

BOOL XHatchList::Create()
{
    XubString aStr( SVX_RES( RID_SVXSTR_HATCH ) );
    xub_StrLen nLen;

    aStr.AppendAscii(" 1");
    nLen = aStr.Len() - 1;
    Insert(new XHatchEntry(XHatch(RGB_Color(COL_BLACK),XHATCH_SINGLE,100,  0),aStr));
    aStr.SetChar(nLen, sal_Unicode('2'));
    Insert(new XHatchEntry(XHatch(RGB_Color(COL_RED  ),XHATCH_DOUBLE, 80,450),aStr));
    aStr.SetChar(nLen, sal_Unicode('3'));
    Insert(new XHatchEntry(XHatch(RGB_Color(COL_BLUE ),XHATCH_TRIPLE,120,  0),aStr));

    return( TRUE );
}

/************************************************************************/

BOOL XHatchList::CreateBitmapsForUI()
{
    for( long i = 0; i < Count(); i++)
    {
        Bitmap* pBmp = CreateBitmapForUI( i, FALSE );
        DBG_ASSERT( pBmp, "XHatchList: Bitmap(UI) konnte nicht erzeugt werden!" );

        if( pBmp )
            pBmpList->Insert( pBmp, i );
    }
    // Loeschen, da JOE den Pool vorm Dtor entfernt!
    if( pVD )   { delete pVD;   pVD = NULL;     }
    if( pXOut ) { delete pXOut; pXOut = NULL;   }
    if( pXFSet ){ delete pXFSet; pXFSet = NULL; }

    return( TRUE );
}

/************************************************************************/

Bitmap* XHatchList::CreateBitmapForUI( long nIndex, BOOL bDelete )
{
    Point   aZero;

    if( !pVD ) // und pXOut und pXFSet
    {
        pVD = new VirtualDevice;
        DBG_ASSERT( pVD, "XHatchList: Konnte kein VirtualDevice erzeugen!" );
        //pVD->SetMapMode( MAP_100TH_MM );
        //pVD->SetOutputSize( pVD->PixelToLogic( Size( BITMAP_WIDTH, BITMAP_HEIGHT ) ) );
        pVD->SetOutputSizePixel( Size( BITMAP_WIDTH, BITMAP_HEIGHT ) );

        pXOut = new XOutputDevice( pVD );
        DBG_ASSERT( pVD, "XHatchList: Konnte kein XOutDevice erzeugen!" );

        pXFSet = new XFillAttrSetItem( pXPool );
        DBG_ASSERT( pVD, "XHatchList: Konnte kein XFillAttrSetItem erzeugen!" );
    }

    // Damit die Schraffuren mit Rahmen angezeigt werden:
    // MapMode-Aenderungen (100th mm <--> Pixel)
    Size aPixelSize = pVD->GetOutputSizePixel();
    pVD->SetMapMode( MAP_PIXEL );

    pXFSet->GetItemSet().Put( XFillStyleItem( XFILL_SOLID ) );
    pXFSet->GetItemSet().Put( XFillColorItem( String(), RGB_Color( COL_WHITE ) ) );

//-/    pXOut->SetFillAttr( *pXFSet );
    pXOut->SetFillAttr( pXFSet->GetItemSet() );

    // #73550#
    pXOut->OverrideLineColor( Color( COL_BLACK ) );

    pXOut->DrawRect( Rectangle( aZero, aPixelSize ) );

    pVD->SetMapMode( MAP_100TH_MM );
    Size aVDSize = pVD->GetOutputSize();
    // 1 Pixel (Rahmen) abziehen
    aVDSize.Width() -= (long) ( aVDSize.Width() / aPixelSize.Width() + 1 );
    aVDSize.Height() -= (long) ( aVDSize.Height() / aPixelSize.Height() + 1 );

    pXFSet->GetItemSet().Put( XFillStyleItem( XFILL_HATCH ) );
    pXFSet->GetItemSet().Put( XFillHatchItem( String(), Get( nIndex )->GetHatch() ) );

//-/    pXOut->SetFillAttr( *pXFSet );
    pXOut->SetFillAttr( pXFSet->GetItemSet() );

    pXOut->DrawRect( Rectangle( aZero, aVDSize ) );

    Bitmap* pBitmap = new Bitmap( pVD->GetBitmap( aZero, pVD->GetOutputSize() ) );

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

SvStream& XHatchList::ImpStore( SvStream& rOut )
{
    // Schreiben
    rOut.SetStreamCharSet( gsl_getSystemTextEncoding() );

    // Version statt Anzahl, um auch alte Tabellen zu lesen
    rOut << (long) -1;

    // Anzahl der Eintraege
    rOut << (long)Count();

    // die Eintraege
    XHatchEntry* pEntry = NULL;
    for (long nIndex = 0; nIndex < Count(); nIndex++)
    {
        // Versionsverwaltung: Version 0
        XIOCompat aIOC( rOut, STREAM_WRITE, 0 );

        pEntry = Get(nIndex);

        // UNICODE: rOut << pEntry->GetName();
        rOut.WriteByteString(pEntry->GetName());

        XHatch& rHatch = pEntry->GetHatch();
        rOut << (long)rHatch.GetHatchStyle();
#ifdef VCL
        USHORT nCol = rHatch.GetColor().GetRed();
        nCol = nCol << 8;
        rOut << nCol;

        nCol = rHatch.GetColor().GetGreen();
        nCol = nCol << 8;
        rOut << nCol;

        nCol = rHatch.GetColor().GetBlue();
        nCol = nCol << 8;
        rOut << nCol;
#else
        rOut << rHatch.GetColor().GetRed();
        rOut << rHatch.GetColor().GetGreen();
        rOut << rHatch.GetColor().GetBlue();
#endif
        rOut << rHatch.GetDistance();
        rOut << rHatch.GetAngle();
    }

    return rOut;
}

/************************************************************************/

XubString& XHatchList::ConvertName( XubString& rStrName )
{
    BOOL bFound = FALSE;

    for( USHORT i=0; i<(RID_SVXSTR_HATCH_DEF_END-RID_SVXSTR_HATCH_DEF_START+1) && !bFound; i++ )
    {
        XubString aStrDefName = SVX_RESSTR( RID_SVXSTR_HATCH_DEF_START + i );
        if( rStrName.Search( aStrDefName ) == 0 )
        {
            rStrName.Replace( 0, aStrDefName.Len(), SVX_RESSTR( RID_SVXSTR_HATCH_START + i ) );
            bFound = TRUE;
        }
    }

    return rStrName;
}

/************************************************************************/

SvStream& XHatchList::ImpRead( SvStream& rIn )
{
    // Lesen
    rIn.SetStreamCharSet( RTL_TEXTENCODING_IBM_850 );

    delete pBmpList;
    pBmpList = new List( 16, 16 );

    XHatchEntry* pEntry = NULL;
    long        nCount;
    XubString       aName;

    long        nStyle;
    USHORT      nRed;
    USHORT      nGreen;
    USHORT      nBlue;
    long        nDistance;
    long        nAngle;
    Color       aColor;

    rIn >> nCount;

    if( nCount >= 0 ) // Alte Tabellen (bis 3.00)
    {
        for( long nIndex = 0; nIndex < nCount; nIndex++ )
        {
            // UNICODE:rIn >> aName;
            rIn.ReadByteString(aName);

            aName = ConvertName( aName );
            rIn >> nStyle;
            rIn >> nRed;
            rIn >> nGreen;
            rIn >> nBlue;
            rIn >> nDistance;
            rIn >> nAngle;

#ifdef VCL
            aColor = Color( (BYTE) ( nRed   >> 8 ),
                            (BYTE) ( nGreen >> 8 ),
                            (BYTE) ( nBlue  >> 8 ) );
#else
            aColor = Color( nRed, nGreen, nBlue );
#endif
            XHatch aHatch(aColor, (XHatchStyle)nStyle, nDistance, nAngle);
            pEntry = new XHatchEntry (aHatch, aName);
            Insert (pEntry, nIndex);
        }
    }
    else // ab 3.00a
    {
        rIn >> nCount;

        for( long nIndex = 0; nIndex < nCount; nIndex++ )
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
            rIn >> nDistance;
            rIn >> nAngle;

            if (aIOC.GetVersion() > 0)
            {
                // lesen neuer Daten ...
            }

#ifdef VCL
            aColor = Color( (BYTE) ( nRed   >> 8 ),
                            (BYTE) ( nGreen >> 8 ),
                            (BYTE) ( nBlue  >> 8 ) );
#else
            aColor = Color( nRed, nGreen, nBlue );
#endif
            XHatch aHatch(aColor, (XHatchStyle)nStyle, nDistance, nAngle);
            pEntry = new XHatchEntry (aHatch, aName);
            Insert (pEntry, nIndex);
        }
    }
    return( rIn );
}



