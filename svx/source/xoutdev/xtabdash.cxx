/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xtabdash.cxx,v $
 *
 *  $Revision: 1.17 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 06:25:47 $
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
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#include <tools/urlobj.hxx>
#include <vcl/virdev.hxx>
#include <vcl/window.hxx>
#include <svtools/itemset.hxx>
#include <sfx2/docfile.hxx>
#include "dialogs.hrc"
#include "dialmgr.hxx"
#include "xtable.hxx"
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

using namespace com::sun::star;
using namespace rtl;

#define GLOBALOVERFLOW

sal_Unicode const pszExtDash[]  = {'s','o','d'};
char const aChckDash[]  = { 0x04, 0x00, 'S','O','D','L'};   // < 5.2
char const aChckDash0[] = { 0x04, 0x00, 'S','O','D','0'};   // = 5.2
char const aChckXML[]   = { '<', '?', 'x', 'm', 'l' };      // = 6.0

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

XDashEntry* XDashTable::GetDash(long nIndex) const
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

Bitmap* XDashTable::CreateBitmapForUI( long /*nIndex*/, BOOL /*bDelete*/)
{
    return( NULL );
}

/************************************************************************/

//BFS01SvStream& XDashTable::ImpStore( SvStream& rOut )
//BFS01{
//BFS01 // Schreiben
//BFS01 rOut.SetStreamCharSet( gsl_getSystemTextEncoding() );
//BFS01
//BFS01 // Tabellentyp schreiben (0 = gesamte Tabelle)
//BFS01 rOut << (long)0;
//BFS01
//BFS01 // Anzahl der Eintraege
//BFS01 rOut << (long)Count();
//BFS01
//BFS01 // die Eintraege
//BFS01 XDashEntry* pEntry = (XDashEntry*)aTable.First();
//BFS01 for (long nIndex = 0; nIndex < Count(); nIndex++)
//BFS01 {
//BFS01     rOut << (long)aTable.GetCurKey();
//BFS01
//BFS01     // UNICODE: rOut << pEntry->GetName();
//BFS01     rOut.WriteByteString(pEntry->GetName());
//BFS01
//BFS01     XDash& rDash = pEntry->GetDash();
//BFS01     rOut << (long)rDash.GetDashStyle();
//BFS01     rOut << (long)rDash.GetDots();
//BFS01     rOut << rDash.GetDotLen();
//BFS01     rOut << (long)rDash.GetDashes();
//BFS01     rOut << rDash.GetDashLen();
//BFS01     rOut << rDash.GetDistance();
//BFS01     pEntry = (XDashEntry*)aTable.Next();
//BFS01 }
//BFS01
//BFS01 return rOut;
//BFS01}

/************************************************************************/

//BFS01SvStream& XDashTable::ImpRead( SvStream& rIn )
//BFS01{
//BFS01 // Lesen
//BFS01 rIn.SetStreamCharSet( RTL_TEXTENCODING_IBM_850 );
//BFS01
//BFS01 delete pBmpTable;
//BFS01 pBmpTable = new Table( 16, 16 );
//BFS01
//BFS01 XDashEntry* pEntry = NULL;
//BFS01 long        nType;
//BFS01 long        nCount;
//BFS01 long        nIndex;
//BFS01 XubString       aName;
//BFS01
//BFS01 long        nStyle;
//BFS01 long        nDots;
//BFS01 ULONG       nDotLen;
//BFS01 long        nDashes;
//BFS01 ULONG       nDashLen;
//BFS01 ULONG       nDistance;
//BFS01
//BFS01 rIn >> nType;
//BFS01
//BFS01 // gesamte Tabelle?
//BFS01 if (nType == 0)
//BFS01 {
//BFS01     rIn >> nCount;
//BFS01     for (long nI = 0; nI < nCount; nI++)
//BFS01     {
//BFS01         rIn >> nIndex;
//BFS01
//BFS01         // UNICODE: rIn >> aName;
//BFS01         rIn.ReadByteString(aName);
//BFS01
//BFS01         rIn >> nStyle;
//BFS01         rIn >> nDots;
//BFS01         rIn >> nDotLen;
//BFS01         rIn >> nDashes;
//BFS01         rIn >> nDashLen;
//BFS01         rIn >> nDistance;
//BFS01         XDash aDash((XDashStyle)nStyle, (BYTE)nDots, nDotLen,
//BFS01                     (BYTE)nDashes, nDashLen, nDistance);
//BFS01         pEntry = new XDashEntry (aDash, aName);
//BFS01         Insert (nIndex, pEntry);
//BFS01     }
//BFS01 }
//BFS01 return( rIn );
//BFS01}


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

XDashEntry* XDashList::GetDash(long nIndex) const
{
    return (XDashEntry*) XPropertyList::Get(nIndex, 0);
}

/************************************************************************/

BOOL XDashList::Load()
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
            aURL.setExtension( rtl::OUString( pszExtDash, 3 ) );

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
//BFS01         // Handelt es sich um die gew"unschte Tabelle?
//BFS01         if( memcmp( aCheck, aChckDash, sizeof( aChckDash ) ) == 0 ||
//BFS01             memcmp( aCheck, aChckDash0, sizeof( aChckDash0 ) ) == 0 )
//BFS01         {
//BFS01             ImpRead( *pStream );
//BFS01             return( pStream->GetError() == SVSTREAM_OK );
//BFS01         }
//BFS01         else if( memcmp( aCheck, aChckXML, sizeof( aChckXML ) ) != 0 )
//BFS01         {
//BFS01             return FALSE;
//BFS01         }
//BFS01     }

        uno::Reference< container::XNameContainer > xTable( SvxUnoXDashTable_createInstance( this ), uno::UNO_QUERY );
        return SvxXMLXTableImport::load( aURL.GetMainURL( INetURLObject::NO_DECODE ), xTable );
    }
//BFS01#endif
    return( FALSE );
}

/************************************************************************/

BOOL XDashList::Save()
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
        aURL.setExtension( rtl::OUString( pszExtDash, 3 ) );

    uno::Reference< container::XNameContainer > xTable( SvxUnoXDashTable_createInstance( this ), uno::UNO_QUERY );
    return SvxXMLXTableExportComponent::save( aURL.GetMainURL( INetURLObject::NO_DECODE ), xTable );


/*


    SfxMedium aMedium( aURL.GetMainURL( INetURLObject::NO_DECODE ), STREAM_WRITE | STREAM_TRUNC, TRUE );
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
*/
//BFS01#else
//BFS01 return FALSE;
//BFS01#endif
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

        const StyleSettings& rStyles = Application::GetSettings().GetStyleSettings();
        pVD->SetFillColor( rStyles.GetFieldColor() );
        pVD->SetLineColor( rStyles.GetFieldColor() );

        pXOut = new XOutputDevice( pVD );
        DBG_ASSERT( pVD, "XDashList: Konnte kein XOutDevice erzeugen!" );

        pXFSet = new XFillAttrSetItem( pXPool );
        DBG_ASSERT( pVD, "XDashList: Konnte kein XFillAttrSetItem erzeugen!" );

        pXLSet = new XLineAttrSetItem( pXPool );
        DBG_ASSERT( pVD, "XDashList: Konnte kein XLineAttrSetItem erzeugen!" );
        pXLSet->GetItemSet().Put( XLineStyleItem( XLINE_DASH ) );
        pXLSet->GetItemSet().Put( XLineColorItem( String(), RGB_Color( rStyles.GetFieldTextColor().GetColor() ) ) );
        pXLSet->GetItemSet().Put( XLineWidthItem( 30 ) );
    }

    Size aVDSize = pVD->GetOutputSize();
    pVD->DrawRect( Rectangle( aZero, aVDSize ) );
    pXLSet->GetItemSet().Put( XLineDashItem( String(), GetDash( nIndex )->GetDash() ) );

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

//BFS01SvStream& XDashList::ImpStore( SvStream& rOut )
//BFS01{
//BFS01 // Schreiben
//BFS01 rOut.SetStreamCharSet( gsl_getSystemTextEncoding() );
//BFS01
//BFS01 // Version statt Anzahl, um auch alte Tabellen zu lesen
//BFS01 rOut << (long) -1;
//BFS01
//BFS01 // Anzahl der Eintraege
//BFS01 rOut << (long)Count();
//BFS01
//BFS01 // die Eintraege
//BFS01 XDashEntry* pEntry = NULL;
//BFS01 for (long nIndex = 0; nIndex < Count(); nIndex++)
//BFS01 {
//BFS01     // Versionsverwaltung: Version 0
//BFS01     XIOCompat aIOC( rOut, STREAM_WRITE, 0 );
//BFS01
//BFS01     pEntry = Get(nIndex);
//BFS01
//BFS01     // UNICODE: rOut << pEntry->GetName();
//BFS01     rOut.WriteByteString(pEntry->GetName());
//BFS01
//BFS01     XDash& rDash = pEntry->GetDash();
//BFS01     rOut << (long)rDash.GetDashStyle();
//BFS01     rOut << (long)rDash.GetDots();
//BFS01     rOut << rDash.GetDotLen();
//BFS01     rOut << (long)rDash.GetDashes();
//BFS01     rOut << rDash.GetDashLen();
//BFS01     rOut << rDash.GetDistance();
//BFS01 }
//BFS01 return rOut;
//BFS01}

/************************************************************************/

//BFS01XubString& XDashList::ConvertName( XubString& rStrName )
//BFS01{
//BFS01 static USHORT __READONLY_DATA aDefResId[] =
//BFS01 {
//BFS01     RID_SVXSTR_DASH5_DEF,
//BFS01     RID_SVXSTR_DASH0_DEF,
//BFS01     RID_SVXSTR_DASH1_DEF,
//BFS01     RID_SVXSTR_DASH2_DEF,
//BFS01     RID_SVXSTR_DASH3_DEF,
//BFS01     RID_SVXSTR_DASH4_DEF,
//BFS01     RID_SVXSTR_DASH6_DEF,
//BFS01     RID_SVXSTR_DASH7_DEF,
//BFS01     RID_SVXSTR_DASH8_DEF,
//BFS01     RID_SVXSTR_DASH9_DEF,
//BFS01     RID_SVXSTR_DASH10_DEF
//BFS01 };
//BFS01 static USHORT __READONLY_DATA aResId[] =
//BFS01 {
//BFS01     RID_SVXSTR_DASH5,
//BFS01     RID_SVXSTR_DASH0,
//BFS01     RID_SVXSTR_DASH1,
//BFS01     RID_SVXSTR_DASH2,
//BFS01     RID_SVXSTR_DASH3,
//BFS01     RID_SVXSTR_DASH4,
//BFS01     RID_SVXSTR_DASH6,
//BFS01     RID_SVXSTR_DASH7,
//BFS01     RID_SVXSTR_DASH8,
//BFS01     RID_SVXSTR_DASH9,
//BFS01     RID_SVXSTR_DASH10
//BFS01 };
//BFS01
//BFS01 BOOL bFound = FALSE;
//BFS01
//BFS01 for( int i=0; i<(sizeof(aDefResId) / sizeof(USHORT)) && !bFound; i++ )
//BFS01 {
//BFS01     XubString aStrDefName = SVX_RESSTR( aDefResId[i] );
//BFS01     if( rStrName.Search( aStrDefName ) == 0 )
//BFS01     {
//BFS01         rStrName.Replace( 0, aStrDefName.Len(), SVX_RESSTR( aResId[i] ) );
//BFS01         bFound = TRUE;
//BFS01     }
//BFS01 }
//BFS01
//BFS01 return rStrName;
//BFS01}

/************************************************************************/

//BFS01SvStream& XDashList::ImpRead( SvStream& rIn )
//BFS01{
//BFS01 // Lesen
//BFS01 rIn.SetStreamCharSet( RTL_TEXTENCODING_IBM_850 );
//BFS01
//BFS01 delete pBmpList;
//BFS01 pBmpList = new List( 16, 16 );
//BFS01
//BFS01 XDashEntry* pEntry = NULL;
//BFS01 long        nCount;
//BFS01 XubString   aName;
//BFS01
//BFS01 long        nStyle;
//BFS01 long        nDots;
//BFS01 ULONG       nDotLen;
//BFS01 long        nDashes;
//BFS01 ULONG       nDashLen;
//BFS01 ULONG       nDistance;
//BFS01
//BFS01 rIn >> nCount;
//BFS01
//BFS01 if( nCount >= 0 ) // Alte Tabellen (bis 3.00)
//BFS01 {
//BFS01     for (long nIndex = 0; nIndex < nCount; nIndex++)
//BFS01     {
//BFS01         // UNICODE: rIn >> aName;
//BFS01         rIn.ReadByteString(aName);
//BFS01
//BFS01         aName = ConvertName( aName );
//BFS01         rIn >> nStyle;
//BFS01         rIn >> nDots;
//BFS01         rIn >> nDotLen;
//BFS01         rIn >> nDashes;
//BFS01         rIn >> nDashLen;
//BFS01         rIn >> nDistance;
//BFS01         XDash aDash((XDashStyle)nStyle, (BYTE)nDots, nDotLen,
//BFS01                     (BYTE)nDashes, nDashLen, nDistance);
//BFS01         pEntry = new XDashEntry (aDash, aName);
//BFS01         Insert (pEntry, nIndex);
//BFS01     }
//BFS01 }
//BFS01 else // ab 3.00a
//BFS01 {
//BFS01     rIn >> nCount;
//BFS01
//BFS01     for (long nIndex = 0; nIndex < nCount; nIndex++)
//BFS01     {
//BFS01         // Versionsverwaltung
//BFS01         XIOCompat aIOC( rIn, STREAM_READ );
//BFS01
//BFS01         // UNICODE: rIn >> aName;
//BFS01         rIn.ReadByteString(aName);
//BFS01
//BFS01         aName = ConvertName( aName );
//BFS01         rIn >> nStyle;
//BFS01         rIn >> nDots;
//BFS01         rIn >> nDotLen;
//BFS01         rIn >> nDashes;
//BFS01         rIn >> nDashLen;
//BFS01         rIn >> nDistance;
//BFS01
//BFS01         if (aIOC.GetVersion() > 0)
//BFS01         {
//BFS01             // lesen neuer Daten ...
//BFS01         }
//BFS01
//BFS01         XDash aDash((XDashStyle)nStyle, (BYTE)nDots, nDotLen,
//BFS01                     (BYTE)nDashes, nDashLen, nDistance);
//BFS01         pEntry = new XDashEntry (aDash, aName);
//BFS01         Insert (pEntry, nIndex);
//BFS01     }
//BFS01 }
//BFS01 return( rIn );
//BFS01}


// eof
