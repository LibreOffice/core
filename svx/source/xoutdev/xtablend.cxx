/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xtablend.cxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 06:26:49 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

// include ---------------------------------------------------------------

#ifndef SVX_LIGHT

#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _SVX_XPROPERTYTABLE_HXX
#include "XPropertyTable.hxx"
#endif
#ifndef _UNTOOLS_UCBSTREAMHELPER_HXX
#include <unotools/ucbstreamhelper.hxx>
#endif

#include "xmlxtexp.hxx"
#include "xmlxtimp.hxx"

#endif

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

using namespace com::sun::star;
using namespace rtl;

sal_Unicode const pszExtLineEnd[]   = {'s','o','e'};

static char const aChckLEnd[]  = { 0x04, 0x00, 'S','O','E','L'};    // < 5.2
static char const aChckLEnd0[] = { 0x04, 0x00, 'S','O','E','0'};    // = 5.2
static char const aChckXML[]   = { '<', '?', 'x', 'm', 'l' };       // = 6.0

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

XLineEndEntry* XLineEndTable::GetLineEnd(long nIndex) const
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

Bitmap* XLineEndTable::CreateBitmapForUI( long /*nIndex*/, BOOL /*bDelete*/)
{
    return( NULL );
}

/************************************************************************/

BOOL XLineEndTable::CreateBitmapsForUI()
{
    return( FALSE );
}

/************************************************************************/

//BFS01SvStream& XLineEndTable::ImpStore( SvStream& rOut )
//BFS01{
//BFS01 // Schreiben
//BFS01 rOut.SetStreamCharSet( gsl_getSystemTextEncoding() );
//BFS01
//BFS01 // 2. Version
//BFS01 rOut << (long)-1;
//BFS01
//BFS01 // Tabellentyp schreiben (0 = gesamte Tabelle)
//BFS01 rOut << (long)0;
//BFS01
//BFS01 // Anzahl der Eintraege
//BFS01 rOut << (long)Count();
//BFS01
//BFS01 // die Polygone
//BFS01 XLineEndEntry* pEntry = (XLineEndEntry*)aTable.First();
//BFS01 for (long nIndex = 0; nIndex < Count(); nIndex++)
//BFS01 {
//BFS01     rOut << (long)aTable.GetCurKey();
//BFS01
//BFS01     // UNICODE: rOut << pEntry->GetName();
//BFS01     rOut.WriteByteString(pEntry->GetName());
//BFS01
//BFS01     XPolygon& rXPoly = pEntry->GetLineEnd();
//BFS01     rOut << rXPoly;
//BFS01
//BFS01     pEntry = (XLineEndEntry*)aTable.Next();
//BFS01 }
//BFS01 return rOut;
//BFS01}

/************************************************************************/

//BFS01SvStream& XLineEndTable::ImpRead( SvStream& rIn )
//BFS01{
//BFS01 // Lesen
//BFS01 rIn.SetStreamCharSet( RTL_TEXTENCODING_IBM_850 );
//BFS01
//BFS01 delete pBmpTable;
//BFS01 pBmpTable = new Table( 16, 16 );
//BFS01
//BFS01 XLineEndEntry* pEntry = NULL;
//BFS01 long    nVersion;
//BFS01 long    nType;
//BFS01 long    nCount;
//BFS01 long    nIndex;
//BFS01 XubString   aName;
//BFS01 long    nFlags;
//BFS01
//BFS01 rIn >> nVersion;
//BFS01
//BFS01 if( nVersion == -1L ) // 2. Version
//BFS01 {
//BFS01     rIn >> nType;
//BFS01
//BFS01     // gesamte Tabelle?
//BFS01     if (nType == 0)
//BFS01     {
//BFS01         rIn >> nCount;
//BFS01         for (long nI = 0; nI < nCount; nI++)
//BFS01         {
//BFS01             rIn >> nIndex;
//BFS01
//BFS01             // UNICODE: rIn >> aName;
//BFS01             rIn.ReadByteString(aName);
//BFS01
//BFS01             USHORT nPoints;
//BFS01             ULONG  nTemp;
//BFS01             Point  aPoint;
//BFS01             rIn >> nTemp; nPoints = (USHORT)nTemp;
//BFS01             XPolygon* pXPoly = new XPolygon(nPoints);
//BFS01             for (USHORT nPoint = 0; nPoint < nPoints; nPoint++)
//BFS01             {
//BFS01                 rIn >> aPoint.X();
//BFS01                 rIn >> aPoint.Y();
//BFS01                 rIn >> nFlags;
//BFS01                 pXPoly->Insert(nPoint, aPoint, (XPolyFlags)nFlags);
//BFS01             }
//BFS01
//BFS01             pEntry = new XLineEndEntry (*pXPoly, aName);
//BFS01             Insert (nIndex, pEntry);
//BFS01         }
//BFS01     }
//BFS01 }
//BFS01 else // 1. Version
//BFS01 {
//BFS01     nType = nVersion;
//BFS01
//BFS01     // gesamte Tabelle?
//BFS01     if (nType == 0)
//BFS01     {
//BFS01         XPolygon aXPoly;
//BFS01
//BFS01         rIn >> nCount;
//BFS01         for (long nI = 0; nI < nCount; nI++)
//BFS01         {
//BFS01             rIn >> nIndex;
//BFS01
//BFS01             // UNICODE: rIn >> aName;
//BFS01             rIn.ReadByteString(aName);
//BFS01
//BFS01             rIn >> aXPoly;
//BFS01             XPolygon* pXPoly = new XPolygon( aXPoly );
//BFS01
//BFS01             pEntry = new XLineEndEntry( *pXPoly, aName );
//BFS01             Insert( nIndex, pEntry );
//BFS01         }
//BFS01     }
//BFS01 }
//BFS01 return( rIn );
//BFS01}

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

XLineEndEntry* XLineEndList::GetLineEnd(long nIndex) const
{
    return (XLineEndEntry*) XPropertyList::Get(nIndex, 0);
}

/************************************************************************/

BOOL XLineEndList::Load()
{
//BFS01#ifndef SVX_LIGHT
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

        if( !aURL.getExtension().getLength() )
            aURL.setExtension( rtl::OUString( pszExtLineEnd, 3 ) );

//BFS01     // check if file exists, SfxMedium shows an errorbox else
//BFS01     {
//BFS01         com::sun::star::uno::Reference < com::sun::star::task::XInteractionHandler > xHandler;
//BFS01         SvStream* pIStm = ::utl::UcbStreamHelper::CreateStream( aURL.GetMainURL( INetURLObject::NO_DECODE ), STREAM_READ, xHandler );
//BFS01
//BFS01         sal_Bool bOk = pIStm && ( pIStm->GetError() == 0);
//BFS01
//BFS01         if( pIStm )
//BFS01             delete pIStm;
//BFS01
//BFS01         if( !bOk )
//BFS01             return sal_False;
//BFS01     }

//BFS01     {
//BFS01         SfxMedium aMedium( aURL.GetMainURL( INetURLObject::NO_DECODE ), STREAM_READ | STREAM_NOCREATE, TRUE );
//BFS01         SvStream* pStream = aMedium.GetInStream();
//BFS01         if( !pStream )
//BFS01             return( FALSE );
//BFS01
//BFS01         char aCheck[6];
//BFS01         pStream->Read( aCheck, 6 );
//BFS01
//BFS01         // Handelt es sich um die gew'unschte Tabelle?
//BFS01         if( memcmp( aCheck, aChckLEnd, sizeof( aChckLEnd ) ) == 0 ||
//BFS01             memcmp( aCheck, aChckLEnd0, sizeof( aChckLEnd0 ) ) == 0 )
//BFS01         {
//BFS01             ImpRead( *pStream );
//BFS01             return( pStream->GetError() == SVSTREAM_OK );
//BFS01         }
//BFS01         else if( memcmp( aCheck, aChckXML, sizeof( aChckXML ) ) != 0 )
//BFS01         {
//BFS01             return FALSE;
//BFS01         }
//BFS01     }

        uno::Reference< container::XNameContainer > xTable( SvxUnoXLineEndTable_createInstance( this ), uno::UNO_QUERY );
        return SvxXMLXTableImport::load( aURL.GetMainURL( INetURLObject::NO_DECODE ), xTable );
    }
//BFS01#endif
    return( FALSE );
}

/************************************************************************/

BOOL XLineEndList::Save()
{
//BFS01#ifndef SVX_LIGHT
    INetURLObject aURL( aPath );

    if( INET_PROT_NOT_VALID == aURL.GetProtocol() )
    {
        DBG_ASSERT( !aPath.Len(), "invalid URL" );
        return FALSE;
    }

    aURL.Append( aName );

    if( !aURL.getExtension().getLength() )
        aURL.setExtension( rtl::OUString( pszExtLineEnd, 3 ) );

    uno::Reference< container::XNameContainer > xTable( SvxUnoXLineEndTable_createInstance( this ), uno::UNO_QUERY );
    return SvxXMLXTableExportComponent::save( aURL.GetMainURL( INetURLObject::NO_DECODE ), xTable );

/*
    SfxMedium aMedium( aURL.GetMainURL( INetURLObject::NO_DECODE ), STREAM_WRITE | STREAM_TRUNC, TRUE );
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
*/
//BFS01#else
//BFS01 return FALSE;
//BFS01#endif
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

        const StyleSettings& rStyles = Application::GetSettings().GetStyleSettings();
        pVD->SetFillColor( rStyles.GetFieldColor() );
        pVD->SetLineColor( rStyles.GetFieldColor() );

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
        pXLSet->GetItemSet().Put( XLineColorItem( String(), RGB_Color( rStyles.GetFieldTextColor().GetColor() ) ) );

    }
    else
        aVDSize = pVD->GetOutputSize();

    pXLSet->GetItemSet().Put( XLineStyleItem( XLINE_NONE ) );
//-/    pXOut->SetLineAttr( *pXLSet );

    pXOut->SetLineAttr( pXLSet->GetItemSet() );
//-/    pXOut->SetFillAttr( *pXFSet );

    pXOut->SetFillAttr( pXFSet->GetItemSet() );
    pXOut->DrawRect( Rectangle( aZero, aVDSize ) );

    pXLSet->GetItemSet().Put( XLineStyleItem( XLINE_SOLID ) );
    pXLSet->GetItemSet().Put( XLineStartItem( String(), GetLineEnd( nIndex )->GetLineEnd() ) );
    pXLSet->GetItemSet().Put( XLineEndItem( String(), GetLineEnd( nIndex )->GetLineEnd() ) );

//-/    pXOut->SetLineAttr( *pXLSet );
    pXOut->SetLineAttr( pXLSet->GetItemSet() );

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

//BFS01SvStream& XLineEndList::ImpStore( SvStream& rOut )
//BFS01{
//BFS01 // Schreiben
//BFS01 rOut.SetStreamCharSet( gsl_getSystemTextEncoding() );
//BFS01
//BFS01 // 3. Version
//BFS01 rOut << (long) -2;
//BFS01
//BFS01 // Anzahl der Eintraege
//BFS01 rOut << (long)Count();
//BFS01
//BFS01 // die Polygone
//BFS01 XLineEndEntry* pEntry = NULL;
//BFS01 for( long nIndex = 0; nIndex < Count(); nIndex++ )
//BFS01 {
//BFS01     // Versionsverwaltung (auch abwaertskompatibel): Version 0
//BFS01     XIOCompat aIOC( rOut, STREAM_WRITE, 0 );
//BFS01
//BFS01     pEntry = Get( nIndex );
//BFS01     // UNICODE: rOut << pEntry->GetName();
//BFS01     rOut.WriteByteString(pEntry->GetName());
//BFS01
//BFS01     XPolygon& rXPoly = pEntry->GetLineEnd();
//BFS01     rOut << rXPoly;
//BFS01 }
//BFS01 return rOut;
//BFS01}

/************************************************************************/

//BFS01XubString& XLineEndList::ConvertName( XubString& rStrName )
//BFS01{
//BFS01 BOOL bFound = FALSE;
//BFS01
//BFS01 for( USHORT i=0; i<(RID_SVXSTR_LEND_DEF_END-RID_SVXSTR_LEND_DEF_START+1) && !bFound; i++ )
//BFS01 {
//BFS01     XubString aStrDefName = SVX_RESSTR( RID_SVXSTR_LEND_DEF_START + i );
//BFS01     if( rStrName.Search( aStrDefName ) == 0 )
//BFS01     {
//BFS01         rStrName.Replace( 0, aStrDefName.Len(), SVX_RESSTR( RID_SVXSTR_LEND_START + i ) );
//BFS01         bFound = TRUE;
//BFS01     }
//BFS01 }
//BFS01
//BFS01 return rStrName;
//BFS01}

/************************************************************************/

//BFS01SvStream& XLineEndList::ImpRead( SvStream& rIn )
//BFS01{
//BFS01 // Lesen
//BFS01 rIn.SetStreamCharSet( RTL_TEXTENCODING_IBM_850 );
//BFS01
//BFS01 delete pBmpList;
//BFS01 pBmpList = new List( 16, 16 );
//BFS01
//BFS01 XLineEndEntry* pEntry = NULL;
//BFS01 long    nVersion;
//BFS01 long    nCount;
//BFS01 XubString   aName;
//BFS01 long    nFlags;
//BFS01
//BFS01 rIn >> nVersion;
//BFS01
//BFS01 if( nVersion >= 0 ) // 1. Version
//BFS01 {
//BFS01     nCount = nVersion;
//BFS01     for( long nI = 0; nI < nCount; nI++ )
//BFS01     {
//BFS01         // UNICODE: rIn >> aName;
//BFS01         rIn.ReadByteString(aName);
//BFS01
//BFS01         aName = ConvertName( aName );
//BFS01         USHORT nPoints;
//BFS01         ULONG  nTemp;
//BFS01         Point  aPoint;
//BFS01         rIn >> nTemp; nPoints = (USHORT)nTemp;
//BFS01         XPolygon* pXPoly = new XPolygon(nPoints);
//BFS01         for (USHORT nPoint = 0; nPoint < nPoints; nPoint++)
//BFS01         {
//BFS01             rIn >> aPoint.X();
//BFS01             rIn >> aPoint.Y();
//BFS01             rIn >> nFlags;
//BFS01             pXPoly->Insert(nPoint, aPoint, (XPolyFlags)nFlags);
//BFS01         }
//BFS01
//BFS01         pEntry = new XLineEndEntry( *pXPoly, aName );
//BFS01         Insert( pEntry, nI );
//BFS01     }
//BFS01 }
//BFS01 else if( nVersion == -1L ) // 2. Version
//BFS01 {
//BFS01     rIn >> nCount;
//BFS01     for( long nI = 0; nI < nCount; nI++ )
//BFS01     {
//BFS01         // UNICODE: rIn >> aName;
//BFS01         rIn.ReadByteString(aName);
//BFS01         aName = ConvertName( aName );
//BFS01
//BFS01         XPolygon* pXPoly = new XPolygon;
//BFS01         rIn >> *pXPoly;
//BFS01
//BFS01         pEntry = new XLineEndEntry( *pXPoly, aName );
//BFS01         Insert( pEntry, nI );
//BFS01     }
//BFS01 }
//BFS01 else // ab 3.00a
//BFS01 {
//BFS01     rIn >> nCount;
//BFS01     for( long nI = 0; nI < nCount; nI++ )
//BFS01     {
//BFS01         // Versionsverwaltung
//BFS01         XIOCompat aIOC( rIn, STREAM_READ );
//BFS01
//BFS01         // UNICODE: rIn >> aName;
//BFS01         rIn.ReadByteString(aName);
//BFS01         aName = ConvertName( aName );
//BFS01
//BFS01         XPolygon aXPoly;
//BFS01         rIn >> aXPoly;
//BFS01
//BFS01         if (aIOC.GetVersion() > 0)
//BFS01         {
//BFS01             // lesen neuer Daten ...
//BFS01         }
//BFS01
//BFS01         pEntry = new XLineEndEntry( aXPoly, aName );
//BFS01         Insert( pEntry, nI );
//BFS01     }
//BFS01 }
//BFS01 return( rIn );
//BFS01}

// eof
