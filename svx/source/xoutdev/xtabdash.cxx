/*************************************************************************
 *
 *  $RCSfile: xtabdash.cxx,v $
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

#include <tools/urlobj.hxx>
#include <vcl/virdev.hxx>
#include <vcl/window.hxx>
#include <svtools/itemset.hxx>
#include <sfx2/docfile.hxx>
#include "dialogs.hrc"
#include "dialmgr.hxx"
#include "xtable.hxx"
#include "xiocomp.hxx"
#include "xpool.hxx"
#include "xoutx.hxx"

#ifndef _SVX_XLINEIT0_HXX //autogen
#include <xlineit0.hxx>
#endif

#ifndef _SVX_XLNCLIT_HXX //autogen
#include <xlnclit.hxx>
#endif

#ifndef _SVX_XLNWTIT_HXX //autogen
#include <xlnwtit.hxx>
#endif

#ifndef _SVX_XLNDSIT_HXX //autogen
#include <xlndsit.hxx>
#endif

#define GLOBALOVERFLOW

sal_Unicode const pszExtDash[]  = {'s','o','d'};
sal_Unicode const pszChckDash[] = {'S','O','D','L'};
// Neuer Key, damit alte Version (3.00) nicht bei dem
// Versuch abstuerzt, eine neue Tabelle zu laden.
sal_Unicode const pszChckDash0[]    = {'S','O','D','0'};

// -----------------
// class XDashTable
// -----------------

/*************************************************************************
|*
|* XDashTable::XDashTable()
|*
*************************************************************************/

XDashTable::XDashTable( const String& rPath,
                            XOutdevItemPool* pInPool,
                            USHORT nInitSize, USHORT nReSize ) :
                XPropertyTable( rPath, pInPool, nInitSize, nReSize)
{
    pBmpTable = new Table( nInitSize, nReSize );
}

/************************************************************************/

XDashTable::~XDashTable()
{
}

/************************************************************************/

XDashEntry* XDashTable::Replace(long nIndex, XDashEntry* pEntry )
{
    return (XDashEntry*) XPropertyTable::Replace(nIndex, pEntry);
}

/************************************************************************/

XDashEntry* XDashTable::Remove(long nIndex)
{
    return (XDashEntry*) XPropertyTable::Remove(nIndex, 0);
}

/************************************************************************/

XDashEntry* XDashTable::Get(long nIndex) const
{
    return (XDashEntry*) XPropertyTable::Get(nIndex, 0);
}

/************************************************************************/

BOOL XDashTable::Load()
{
    return( FALSE );
}

/************************************************************************/

BOOL XDashTable::Save()
{
    return( FALSE );
}

/************************************************************************/

BOOL XDashTable::Create()
{
    return( FALSE );
}

/************************************************************************/

BOOL XDashTable::CreateBitmapsForUI()
{
    return( FALSE );
}

/************************************************************************/

Bitmap* XDashTable::CreateBitmapForUI( long nIndex, BOOL bDelete )
{
    return( NULL );
}

/************************************************************************/

SvStream& XDashTable::ImpStore( SvStream& rOut )
{
    // Schreiben
    rOut.SetStreamCharSet( gsl_getSystemTextEncoding() );

    // Tabellentyp schreiben (0 = gesamte Tabelle)
    rOut << (long)0;

    // Anzahl der Eintraege
    rOut << (long)Count();

    // die Eintraege
    XDashEntry* pEntry = (XDashEntry*)aTable.First();
    for (long nIndex = 0; nIndex < Count(); nIndex++)
    {
        rOut << (long)aTable.GetCurKey();

        // UNICODE: rOut << pEntry->GetName();
        rOut.WriteByteString(pEntry->GetName());

        XDash& rDash = pEntry->GetDash();
        rOut << (long)rDash.GetDashStyle();
        rOut << (long)rDash.GetDots();
        rOut << rDash.GetDotLen();
        rOut << (long)rDash.GetDashes();
        rOut << rDash.GetDashLen();
        rOut << rDash.GetDistance();
        pEntry = (XDashEntry*)aTable.Next();
    }

    return rOut;
}

/************************************************************************/

SvStream& XDashTable::ImpRead( SvStream& rIn )
{
    // Lesen
    rIn.SetStreamCharSet( RTL_TEXTENCODING_IBM_850 );

    delete pBmpTable;
    pBmpTable = new Table( 16, 16 );

    XDashEntry* pEntry = NULL;
    long        nType;
    long        nCount;
    long        nIndex;
    XubString       aName;

    long        nStyle;
    long        nDots;
    ULONG       nDotLen;
    long        nDashes;
    ULONG       nDashLen;
    ULONG       nDistance;

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
            rIn >> nDots;
            rIn >> nDotLen;
            rIn >> nDashes;
            rIn >> nDashLen;
            rIn >> nDistance;
            XDash aDash((XDashStyle)nStyle, (BYTE)nDots, nDotLen,
                        (BYTE)nDashes, nDashLen, nDistance);
            pEntry = new XDashEntry (aDash, aName);
            Insert (nIndex, pEntry);
        }
    }
    return( rIn );
}


// ----------------
// class XDashList
// ----------------

/*************************************************************************
|*
|* XDashList::XDashList()
|*
*************************************************************************/

XDashList::XDashList( const String& rPath,
                            XOutdevItemPool* pInPool,
                            USHORT nInitSize, USHORT nReSize ) :
                XPropertyList   ( rPath, pInPool, nInitSize, nReSize),
                pVD             ( NULL ),
                pXOut           ( NULL ),
                pXFSet          ( NULL ),
                pXLSet          ( NULL )
{
    pBmpList = new List( nInitSize, nReSize );
}

/************************************************************************/

XDashList::~XDashList()
{
    if( pVD )    delete pVD;
    if( pXOut )  delete pXOut;
    if( pXFSet ) delete pXFSet;
    if( pXLSet ) delete pXLSet;
}

/************************************************************************/

XDashEntry* XDashList::Replace(XDashEntry* pEntry, long nIndex )
{
    return (XDashEntry*) XPropertyList::Replace(pEntry, nIndex);
}

/************************************************************************/

XDashEntry* XDashList::Remove(long nIndex)
{
    return (XDashEntry*) XPropertyList::Remove(nIndex, 0);
}

/************************************************************************/

XDashEntry* XDashList::Get(long nIndex) const
{
    return (XDashEntry*) XPropertyList::Get(nIndex, 0);
}

/************************************************************************/

BOOL XDashList::Load()
{
#ifndef SVX_LIGHT
    if( bListDirty )
    {
        bListDirty = FALSE;

        INetURLObject aURL;

        aURL.SetSmartURL( aPath );
        aURL.Append( aName );

        if( !aURL.getExtension().Len() )
            aURL.setExtension( String( pszExtDash, 3 ) );

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

        // Handelt es sich um die gew"unschte Tabelle?
        if( aCheck == String(pszChckDash, 4) ||
            aCheck == String(pszChckDash0, 4) )
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

BOOL XDashList::Save()
{
#ifndef SVX_LIGHT
    INetURLObject aURL;

    aURL.SetSmartURL( aPath );
    aURL.Append( aName );

    if( !aURL.getExtension().Len() )
        aURL.setExtension( String( pszExtDash, 3 ) );

    SfxMedium aMedium( aURL.PathToFileName(),
                STREAM_WRITE | STREAM_TRUNC,
                TRUE,                 // direkt
                FALSE );              // Upload
    aMedium.IsRemote();

    SvStream* pStream = aMedium.GetOutStream();
    if( !pStream )
        return( FALSE );

    // UNICODE: *pStream << String( pszChckDash0, 4 );
    pStream->WriteByteString(String( pszChckDash0, 4 ));

    ImpStore( *pStream );

    aMedium.Close();
    aMedium.Commit();

    return( aMedium.GetError() == 0 );
#else
    return FALSE;
#endif
}

/************************************************************************/

BOOL XDashList::Create()
{
    XubString aStr( SVX_RES( RID_SVXSTR_LINESTYLE ) );
    xub_StrLen nLen;

    aStr.AppendAscii(" 1");
    nLen = aStr.Len() - 1;
    Insert(new XDashEntry(XDash(XDASH_RECT,1, 50,1, 50, 50),aStr));
    aStr.SetChar(nLen, sal_Unicode('2'));
    Insert(new XDashEntry(XDash(XDASH_RECT,1,500,1,500,500),aStr));
    aStr.SetChar(nLen, sal_Unicode('3'));
    Insert(new XDashEntry(XDash(XDASH_RECT,2, 50,3,250,120),aStr));

    return( TRUE );
}

/************************************************************************/

BOOL XDashList::CreateBitmapsForUI()
{
    for( long i = 0; i < Count(); i++)
    {
        Bitmap* pBmp = CreateBitmapForUI( i, FALSE );
        DBG_ASSERT( pBmp, "XDashList: Bitmap(UI) konnte nicht erzeugt werden!" );

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

Bitmap* XDashList::CreateBitmapForUI( long nIndex, BOOL bDelete )
{
    Point   aZero;

    if( !pVD ) // und pXOut und pXFSet und pXLSet
    {
        pVD = new VirtualDevice;
        DBG_ASSERT( pVD, "XDashList: Konnte kein VirtualDevice erzeugen!" );
        pVD->SetMapMode( MAP_100TH_MM );
        pVD->SetOutputSize( pVD->PixelToLogic( Size( BITMAP_WIDTH * 2, BITMAP_HEIGHT ) ) );
        const StyleSettings& rStyleSettings = pVD->GetSettings().GetStyleSettings();
        pVD->SetFillColor( rStyleSettings.GetFieldColor() );
        pVD->SetLineColor( rStyleSettings.GetFieldColor() );

        pXOut = new XOutputDevice( pVD );
        DBG_ASSERT( pVD, "XDashList: Konnte kein XOutDevice erzeugen!" );

        pXFSet = new XFillAttrSetItem( pXPool );
        DBG_ASSERT( pVD, "XDashList: Konnte kein XFillAttrSetItem erzeugen!" );

        pXLSet = new XLineAttrSetItem( pXPool );
        DBG_ASSERT( pVD, "XDashList: Konnte kein XLineAttrSetItem erzeugen!" );
        pXLSet->GetItemSet().Put( XLineStyleItem( XLINE_DASH ) );
        pXLSet->GetItemSet().Put( XLineColorItem( String(), RGB_Color( COL_BLACK ) ) );
        pXLSet->GetItemSet().Put( XLineWidthItem( 30 ) );
    }

    Size aVDSize = pVD->GetOutputSize();
    pVD->DrawRect( Rectangle( aZero, aVDSize ) );
    pXLSet->GetItemSet().Put( XLineDashItem( String(), Get( nIndex )->GetDash() ) );
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

SvStream& XDashList::ImpStore( SvStream& rOut )
{
    // Schreiben
    rOut.SetStreamCharSet( gsl_getSystemTextEncoding() );

    // Version statt Anzahl, um auch alte Tabellen zu lesen
    rOut << (long) -1;

    // Anzahl der Eintraege
    rOut << (long)Count();

    // die Eintraege
    XDashEntry* pEntry = NULL;
    for (long nIndex = 0; nIndex < Count(); nIndex++)
    {
        // Versionsverwaltung: Version 0
        XIOCompat aIOC( rOut, STREAM_WRITE, 0 );

        pEntry = Get(nIndex);

        // UNICODE: rOut << pEntry->GetName();
        rOut.WriteByteString(pEntry->GetName());

        XDash& rDash = pEntry->GetDash();
        rOut << (long)rDash.GetDashStyle();
        rOut << (long)rDash.GetDots();
        rOut << rDash.GetDotLen();
        rOut << (long)rDash.GetDashes();
        rOut << rDash.GetDashLen();
        rOut << rDash.GetDistance();
    }
    return rOut;
}

/************************************************************************/

XubString& XDashList::ConvertName( XubString& rStrName )
{
    static USHORT __READONLY_DATA aDefResId[] =
    {
        RID_SVXSTR_DASH5_DEF,
        RID_SVXSTR_DASH0_DEF,
        RID_SVXSTR_DASH1_DEF,
        RID_SVXSTR_DASH2_DEF,
        RID_SVXSTR_DASH3_DEF,
        RID_SVXSTR_DASH4_DEF,
        RID_SVXSTR_DASH6_DEF,
        RID_SVXSTR_DASH7_DEF,
        RID_SVXSTR_DASH8_DEF,
        RID_SVXSTR_DASH9_DEF,
        RID_SVXSTR_DASH10_DEF
    };
    static USHORT __READONLY_DATA aResId[] =
    {
        RID_SVXSTR_DASH5,
        RID_SVXSTR_DASH0,
        RID_SVXSTR_DASH1,
        RID_SVXSTR_DASH2,
        RID_SVXSTR_DASH3,
        RID_SVXSTR_DASH4,
        RID_SVXSTR_DASH6,
        RID_SVXSTR_DASH7,
        RID_SVXSTR_DASH8,
        RID_SVXSTR_DASH9,
        RID_SVXSTR_DASH10
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

SvStream& XDashList::ImpRead( SvStream& rIn )
{
    // Lesen
    rIn.SetStreamCharSet( RTL_TEXTENCODING_IBM_850 );

    delete pBmpList;
    pBmpList = new List( 16, 16 );

    XDashEntry* pEntry = NULL;
    long        nCount;
    XubString   aName;

    long        nStyle;
    long        nDots;
    ULONG       nDotLen;
    long        nDashes;
    ULONG       nDashLen;
    ULONG       nDistance;

    rIn >> nCount;

    if( nCount >= 0 ) // Alte Tabellen (bis 3.00)
    {
        for (long nIndex = 0; nIndex < nCount; nIndex++)
        {
            // UNICODE: rIn >> aName;
            rIn.ReadByteString(aName);

            aName = ConvertName( aName );
            rIn >> nStyle;
            rIn >> nDots;
            rIn >> nDotLen;
            rIn >> nDashes;
            rIn >> nDashLen;
            rIn >> nDistance;
            XDash aDash((XDashStyle)nStyle, (BYTE)nDots, nDotLen,
                        (BYTE)nDashes, nDashLen, nDistance);
            pEntry = new XDashEntry (aDash, aName);
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
            rIn >> nDots;
            rIn >> nDotLen;
            rIn >> nDashes;
            rIn >> nDashLen;
            rIn >> nDistance;

            if (aIOC.GetVersion() > 0)
            {
                // lesen neuer Daten ...
            }

            XDash aDash((XDashStyle)nStyle, (BYTE)nDots, nDotLen,
                        (BYTE)nDashes, nDashLen, nDistance);
            pEntry = new XDashEntry (aDash, aName);
            Insert (pEntry, nIndex);
        }
    }
    return( rIn );
}



