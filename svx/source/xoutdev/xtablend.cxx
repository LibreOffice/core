/*************************************************************************
 *
 *  $RCSfile: xtablend.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:28 $
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

#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif

#ifndef _SV_VIRDEV_HXX
#include <vcl/virdev.hxx>
#endif

#ifndef _SV_APP_HXX
#include <vcl/svapp.hxx>
#endif

#ifndef _SFXITEMSET_HXX
#include <svtools/itemset.hxx>
#endif

#ifndef _SFXDOCFILE_HXX
#include <sfx2/docfile.hxx>
#endif

#include "dialogs.hrc"
#include "dialmgr.hxx"

#include "xtable.hxx"
#include "xiocomp.hxx"
#include "xpool.hxx"
#include "xoutx.hxx"

#ifndef SVX_XFILLIT0_HXX //autogen
#include <xfillit0.hxx>
#endif

#ifndef _SVX_XFLCLIT_HXX //autogen
#include <xflclit.hxx>
#endif

#ifndef _SVX_XLNSTWIT_HXX //autogen
#include <xlnstwit.hxx>
#endif

#ifndef _SVX_XLNEDWIT_HXX //autogen
#include <xlnedwit.hxx>
#endif

#ifndef _SVX_XLNCLIT_HXX //autogen
#include <xlnclit.hxx>
#endif

#ifndef _SVX_XLINEIT0_HXX //autogen
#include <xlineit0.hxx>
#endif

#ifndef _SVX_XLNSTIT_HXX //autogen
#include <xlnstit.hxx>
#endif

#ifndef _SVX_XLNEDIT_HXX //autogen
#include <xlnedit.hxx>
#endif

#define GLOBALOVERFLOW

sal_Unicode const pszExtLineEnd[]   = {'s','o','e'};
sal_Unicode const pszChckLineEnd[]  = {'S','O','E','L'};
// Neuer Key, damit alte Version (3.00) nicht bei dem
// Versuch abstuerzt, eine neue Tabelle zu laden.
sal_Unicode const pszChckLineEnd0[] = {'S','O','E','0'};

// --------------------
// class XLineEndTable
// --------------------

/*************************************************************************
|*
|* XLineEndTable::XLineEndTable()
|*
*************************************************************************/

XLineEndTable::XLineEndTable( const String& rPath,
                            XOutdevItemPool* pInPool,
                            USHORT nInitSize, USHORT nReSize ) :
                XPropertyTable( rPath, pInPool, nInitSize, nReSize)
{
    pBmpTable = new Table( nInitSize, nReSize );
}

/************************************************************************/

XLineEndTable::~XLineEndTable()
{
}

/************************************************************************/

XLineEndEntry* XLineEndTable::Replace(long nIndex, XLineEndEntry* pEntry )
{
    return (XLineEndEntry*) XPropertyTable::Replace(nIndex, pEntry);
}

/************************************************************************/

XLineEndEntry* XLineEndTable::Remove(long nIndex)
{
    return (XLineEndEntry*) XPropertyTable::Remove(nIndex, 0);
}

/************************************************************************/

XLineEndEntry* XLineEndTable::Get(long nIndex) const
{
    return (XLineEndEntry*) XPropertyTable::Get(nIndex, 0);
}

/************************************************************************/

BOOL XLineEndTable::Load()
{
    return( FALSE );
}

/************************************************************************/

BOOL XLineEndTable::Save()
{
    return( FALSE );
}

/************************************************************************/

BOOL XLineEndTable::Create()
{
    return( FALSE );
}

/************************************************************************/

Bitmap* XLineEndTable::CreateBitmapForUI( long nIndex, BOOL bDelete )
{
    return( NULL );
}

/************************************************************************/

BOOL XLineEndTable::CreateBitmapsForUI()
{
    return( FALSE );
}

/************************************************************************/

SvStream& XLineEndTable::ImpStore( SvStream& rOut )
{
    // Schreiben
    rOut.SetStreamCharSet( gsl_getSystemTextEncoding() );

    // 2. Version
    rOut << (long)-1;

    // Tabellentyp schreiben (0 = gesamte Tabelle)
    rOut << (long)0;

    // Anzahl der Eintraege
    rOut << (long)Count();

    // die Polygone
    XLineEndEntry* pEntry = (XLineEndEntry*)aTable.First();
    for (long nIndex = 0; nIndex < Count(); nIndex++)
    {
        rOut << (long)aTable.GetCurKey();

        // UNICODE: rOut << pEntry->GetName();
        rOut.WriteByteString(pEntry->GetName());

        XPolygon& rXPoly = pEntry->GetLineEnd();
        rOut << rXPoly;

        pEntry = (XLineEndEntry*)aTable.Next();
    }
    return rOut;
}

/************************************************************************/

SvStream& XLineEndTable::ImpRead( SvStream& rIn )
{
    // Lesen
    rIn.SetStreamCharSet( RTL_TEXTENCODING_IBM_850 );

    delete pBmpTable;
    pBmpTable = new Table( 16, 16 );

    XLineEndEntry* pEntry = NULL;
    long    nVersion;
    long    nType;
    long    nCount;
    long    nIndex;
    XubString   aName;
    long    nFlags;

    rIn >> nVersion;

    if( nVersion == -1L ) // 2. Version
    {
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

                USHORT nPoints;
                ULONG  nTemp;
                Point  aPoint;
                rIn >> nTemp; nPoints = (USHORT)nTemp;
                XPolygon* pXPoly = new XPolygon(nPoints);
                for (USHORT nPoint = 0; nPoint < nPoints; nPoint++)
                {
                    rIn >> aPoint.X();
                    rIn >> aPoint.Y();
                    rIn >> nFlags;
                    pXPoly->Insert(nPoint, aPoint, (XPolyFlags)nFlags);
                }

                pEntry = new XLineEndEntry (*pXPoly, aName);
                Insert (nIndex, pEntry);
            }
        }
    }
    else // 1. Version
    {
        nType = nVersion;

        // gesamte Tabelle?
        if (nType == 0)
        {
            XPolygon aXPoly;

            rIn >> nCount;
            for (long nI = 0; nI < nCount; nI++)
            {
                rIn >> nIndex;

                // UNICODE: rIn >> aName;
                rIn.ReadByteString(aName);

                rIn >> aXPoly;
                XPolygon* pXPoly = new XPolygon( aXPoly );

                pEntry = new XLineEndEntry( *pXPoly, aName );
                Insert( nIndex, pEntry );
            }
        }
    }
    return( rIn );
}

// --------------------
// class XLineEndList
// --------------------

/*************************************************************************
|*
|* XLineEndList::XLineEndList()
|*
*************************************************************************/

XLineEndList::XLineEndList( const String& rPath,
                            XOutdevItemPool* pInPool,
                            USHORT nInitSize, USHORT nReSize ) :
                XPropertyList( rPath, pInPool, nInitSize, nReSize),
                pVD             ( NULL ),
                pXOut           ( NULL ),
                pXFSet          ( NULL ),
                pXLSet          ( NULL )
{
    pBmpList = new List( nInitSize, nReSize );
}

/************************************************************************/

XLineEndList::~XLineEndList()
{
    if( pVD )    delete pVD;
    if( pXOut )  delete pXOut;
    if( pXFSet ) delete pXFSet;
    if( pXLSet ) delete pXLSet;
}

/************************************************************************/

XLineEndEntry* XLineEndList::Replace(XLineEndEntry* pEntry, long nIndex )
{
    return (XLineEndEntry*) XPropertyList::Replace(pEntry, nIndex);
}

/************************************************************************/

XLineEndEntry* XLineEndList::Remove(long nIndex)
{
    return (XLineEndEntry*) XPropertyList::Remove(nIndex, 0);
}

/************************************************************************/

XLineEndEntry* XLineEndList::Get(long nIndex) const
{
    return (XLineEndEntry*) XPropertyList::Get(nIndex, 0);
}

/************************************************************************/

BOOL XLineEndList::Load()
{
#ifndef SVX_LIGHT
    if( bListDirty )
    {
        bListDirty = FALSE;

        INetURLObject aURL;

        aURL.SetSmartURL( aPath );
        aURL.Append( aName );

        if( !aURL.getExtension().Len() )
            aURL.setExtension( String( pszExtLineEnd, 3 ) );

        SfxMedium aMedium( aURL.PathToFileName(),
                    STREAM_READ | STREAM_NOCREATE,
                    TRUE,                 // direkt
                    TRUE );               // Download

        SvStream* pStream = aMedium.GetInStream();
        if( !pStream )
            return( FALSE );

        String aCheck;

        // UNICODE: *pStream >> aCheck;
        pStream->ReadByteString(aCheck);

        // Handelt es sich um die gew'unschte Tabelle?
        if( aCheck == String(pszChckLineEnd, 4) ||
            aCheck == String(pszChckLineEnd0, 4) )
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

BOOL XLineEndList::Save()
{
#ifndef SVX_LIGHT
    INetURLObject aURL;

    aURL.SetSmartURL( aPath );
    aURL.Append( aName );

    if( !aURL.getExtension().Len() )
        aURL.setExtension( String( pszExtLineEnd, 3 ) );

    SfxMedium aMedium( aURL.PathToFileName(),
                STREAM_WRITE | STREAM_TRUNC,
                TRUE,                 // direkt
                FALSE );              // Upload
    aMedium.IsRemote();

    SvStream* pStream = aMedium.GetOutStream();
    if( !pStream )
        return( FALSE );

    // UNICODE: *pStream << String( pszChckLineEnd0, 4 );
    pStream->WriteByteString(String( pszChckLineEnd0, 4 ));

    ImpStore( *pStream );

    aMedium.Close();
    aMedium.Commit();

    return( aMedium.GetError() == 0 );
#else
    return FALSE;
#endif
}

/************************************************************************/

BOOL XLineEndList::Create()
{
    XPolygon aTriangle(3);
    aTriangle[0].X()=10; aTriangle[0].Y()= 0;
    aTriangle[1].X()= 0; aTriangle[1].Y()=30;
    aTriangle[2].X()=20; aTriangle[2].Y()=30;
    Insert( new XLineEndEntry( aTriangle, SVX_RESSTR( RID_SVXSTR_ARROW ) ) );

    XPolygon aSquare(4);
    aSquare[0].X()= 0; aSquare[0].Y()= 0;
    aSquare[1].X()=10; aSquare[1].Y()= 0;
    aSquare[2].X()=10; aSquare[2].Y()=10;
    aSquare[3].X()= 0; aSquare[3].Y()=10;
    Insert( new XLineEndEntry( aSquare, SVX_RESSTR( RID_SVXSTR_SQUARE ) ) );

    XPolygon aCircle(Point(0,0),100,100);
    Insert( new XLineEndEntry( aCircle, SVX_RESSTR( RID_SVXSTR_CIRCLE ) ) );

    return( TRUE );
}

/************************************************************************/

BOOL XLineEndList::CreateBitmapsForUI()
{
    for( long i = 0; i < Count(); i++)
    {
        Bitmap* pBmp = CreateBitmapForUI( i, FALSE );
        DBG_ASSERT( pBmp, "XLineEndList: Bitmap(UI) konnte nicht erzeugt werden!" );

        if( pBmp )
            pBmpList->Insert( pBmp, i );
    }
    // Loeschen, da JOE den Pool vorm Dtor entfernt!
    if( pVD )   { delete pVD;   pVD = NULL;     }
    if( pXOut ) { delete pXOut; pXOut = NULL;   }
    if( pXFSet ){ delete pXFSet; pXFSet = NULL; }
    if( pXLSet ){ delete pXLSet; pXLSet = NULL; }

    return( TRUE );
}

/************************************************************************/

Bitmap* XLineEndList::CreateBitmapForUI( long nIndex, BOOL bDelete )
{
    Point   aZero;
    Size    aVDSize;

    if( !pVD ) // und pXOut und pXFSet und pXLSet
    {
        pVD = new VirtualDevice;
        DBG_ASSERT( pVD, "XLineEndList: Konnte kein VirtualDevice erzeugen!" );
        pVD->SetMapMode( MAP_100TH_MM );
        aVDSize = pVD->PixelToLogic( Size( BITMAP_WIDTH * 2, BITMAP_HEIGHT ) );
        pVD->SetOutputSize( aVDSize );

        pXOut = new XOutputDevice( pVD );
        DBG_ASSERT( pVD, "XLineEndList: Konnte kein XOutDevice erzeugen!" );

        pXFSet = new XFillAttrSetItem( pXPool );
        DBG_ASSERT( pVD, "XLineEndList: Konnte kein XFillAttrSetItem erzeugen!" );
        pXFSet->GetItemSet().Put( XFillStyleItem( XFILL_SOLID ) );
        const StyleSettings& rStyleSettings = pVD->GetSettings().GetStyleSettings();
        pXFSet->GetItemSet().Put( XFillColorItem( String(), rStyleSettings.GetFieldColor() ) );

        pXLSet = new XLineAttrSetItem( pXPool );
        DBG_ASSERT( pVD, "XLineEndList: Konnte kein XLineAttrSetItem erzeugen!" );
        pXLSet->GetItemSet().Put( XLineStartWidthItem( aVDSize.Height() ) );
        pXLSet->GetItemSet().Put( XLineEndWidthItem( aVDSize.Height() ) );
        pXLSet->GetItemSet().Put( XLineColorItem( String(), RGB_Color( COL_BLACK ) ) );

    }
    else
        aVDSize = pVD->GetOutputSize();

    pXLSet->GetItemSet().Put( XLineStyleItem( XLINE_NONE ) );
    pXOut->SetLineAttr( *pXLSet );
    pXOut->SetFillAttr( *pXFSet );
    pXOut->DrawRect( Rectangle( aZero, aVDSize ) );

    pXLSet->GetItemSet().Put( XLineStyleItem( XLINE_SOLID ) );
    pXLSet->GetItemSet().Put( XLineStartItem( String(), Get( nIndex )->GetLineEnd() ) );
    pXLSet->GetItemSet().Put( XLineEndItem( String(), Get( nIndex )->GetLineEnd() ) );
    pXOut->SetLineAttr( *pXLSet );
    pXOut->DrawLine( Point( 0, aVDSize.Height() / 2 ),
                     Point( aVDSize.Width(), aVDSize.Height() / 2 ) );

    Bitmap* pBitmap = new Bitmap( pVD->GetBitmap( aZero, aVDSize ) );

    // Loeschen, da JOE den Pool vorm Dtor entfernt!
    if( bDelete )
    {
        if( pVD )   { delete pVD;   pVD = NULL;     }
        if( pXOut ) { delete pXOut; pXOut = NULL;   }
        if( pXFSet ){ delete pXFSet; pXFSet = NULL; }
        if( pXLSet ){ delete pXLSet; pXLSet = NULL; }
    }

    return( pBitmap );
}

/************************************************************************/

SvStream& XLineEndList::ImpStore( SvStream& rOut )
{
    // Schreiben
    rOut.SetStreamCharSet( gsl_getSystemTextEncoding() );

    // 3. Version
    rOut << (long) -2;

    // Anzahl der Eintraege
    rOut << (long)Count();

    // die Polygone
    XLineEndEntry* pEntry = NULL;
    for( long nIndex = 0; nIndex < Count(); nIndex++ )
    {
        // Versionsverwaltung (auch abwaertskompatibel): Version 0
        XIOCompat aIOC( rOut, STREAM_WRITE, 0 );

        pEntry = Get( nIndex );
        // UNICODE: rOut << pEntry->GetName();
        rOut.WriteByteString(pEntry->GetName());

        XPolygon& rXPoly = pEntry->GetLineEnd();
        rOut << rXPoly;
    }
    return rOut;
}

/************************************************************************/

XubString& XLineEndList::ConvertName( XubString& rStrName )
{
    BOOL bFound = FALSE;

    for( USHORT i=0; i<(RID_SVXSTR_LEND_DEF_END-RID_SVXSTR_LEND_DEF_START+1) && !bFound; i++ )
    {
        XubString aStrDefName = SVX_RESSTR( RID_SVXSTR_LEND_DEF_START + i );
        if( rStrName.Search( aStrDefName ) == 0 )
        {
            rStrName.Replace( 0, aStrDefName.Len(), SVX_RESSTR( RID_SVXSTR_LEND_START + i ) );
            bFound = TRUE;
        }
    }

    return rStrName;
}

/************************************************************************/

SvStream& XLineEndList::ImpRead( SvStream& rIn )
{
    // Lesen
    rIn.SetStreamCharSet( RTL_TEXTENCODING_IBM_850 );

    delete pBmpList;
    pBmpList = new List( 16, 16 );

    XLineEndEntry* pEntry = NULL;
    long    nVersion;
    long    nCount;
    XubString   aName;
    long    nFlags;

    rIn >> nVersion;

    if( nVersion >= 0 ) // 1. Version
    {
        nCount = nVersion;
        for( long nI = 0; nI < nCount; nI++ )
        {
            // UNICODE: rIn >> aName;
            rIn.ReadByteString(aName);

            aName = ConvertName( aName );
            USHORT nPoints;
            ULONG  nTemp;
            Point  aPoint;
            rIn >> nTemp; nPoints = (USHORT)nTemp;
            XPolygon* pXPoly = new XPolygon(nPoints);
            for (USHORT nPoint = 0; nPoint < nPoints; nPoint++)
            {
                rIn >> aPoint.X();
                rIn >> aPoint.Y();
                rIn >> nFlags;
                pXPoly->Insert(nPoint, aPoint, (XPolyFlags)nFlags);
            }

            pEntry = new XLineEndEntry( *pXPoly, aName );
            Insert( pEntry, nI );
        }
    }
    else if( nVersion == -1L ) // 2. Version
    {
        rIn >> nCount;
        for( long nI = 0; nI < nCount; nI++ )
        {
            // UNICODE: rIn >> aName;
            rIn.ReadByteString(aName);
            aName = ConvertName( aName );

            XPolygon* pXPoly = new XPolygon;
            rIn >> *pXPoly;

            pEntry = new XLineEndEntry( *pXPoly, aName );
            Insert( pEntry, nI );
        }
    }
    else // ab 3.00a
    {
        rIn >> nCount;
        for( long nI = 0; nI < nCount; nI++ )
        {
            // Versionsverwaltung
            XIOCompat aIOC( rIn, STREAM_READ );

            // UNICODE: rIn >> aName;
            rIn.ReadByteString(aName);
            aName = ConvertName( aName );

            XPolygon aXPoly;
            rIn >> aXPoly;

            if (aIOC.GetVersion() > 0)
            {
                // lesen neuer Daten ...
            }

            pEntry = new XLineEndEntry( aXPoly, aName );
            Insert( pEntry, nI );
        }
    }
    return( rIn );
}



