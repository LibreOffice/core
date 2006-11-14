/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xtabgrdt.cxx,v $
 *
 *  $Revision: 1.18 $
 *
 *  last change: $Author: ihi $ $Date: 2006-11-14 13:58:22 $
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

#include <tools/urlobj.hxx>
#include <vcl/virdev.hxx>
#include <svtools/itemset.hxx>
#include <sfx2/docfile.hxx>
#include "dialogs.hrc"
#include "dialmgr.hxx"
#include "xtable.hxx"
#include "xpool.hxx"
#include "xoutx.hxx"

#ifndef SVX_XFILLIT0_HXX //autogen
#include <xfillit0.hxx>
#endif

#ifndef _SVX_XFLGRIT_HXX //autogen
#include <xflgrit.hxx>
#endif

#define GLOBALOVERFLOW

using namespace com::sun::star;
using namespace rtl;

sal_Unicode const pszExtGradient[]  = {'s','o','g'};

char const aChckGradient[]  = { 0x04, 0x00, 'S','O','G','L'};   // < 5.2
char const aChckGradient0[] = { 0x04, 0x00, 'S','O','G','0'};   // = 5.2
char const aChckXML[]       = { '<', '?', 'x', 'm', 'l' };      // = 6.0

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

XGradientEntry* XGradientTable::GetGradient(long nIndex) const
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

Bitmap* XGradientTable::CreateBitmapForUI( long /*nIndex*/, BOOL /*bDelete*/)
{
    return( NULL );
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

XGradientEntry* XGradientList::GetGradient(long nIndex) const
{
    return( (XGradientEntry*) XPropertyList::Get( nIndex, 0 ) );
}

/************************************************************************/

BOOL XGradientList::Load()
{
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
            aURL.setExtension( rtl::OUString( pszExtGradient, 3 ) );

        uno::Reference< container::XNameContainer > xTable( SvxUnoXGradientTable_createInstance( this ), uno::UNO_QUERY );
        return SvxXMLXTableImport::load( aURL.GetMainURL( INetURLObject::NO_DECODE ), xTable );

    }
    return( FALSE );
}

/************************************************************************/

BOOL XGradientList::Save()
{
    INetURLObject aURL( aPath );

    if( INET_PROT_NOT_VALID == aURL.GetProtocol() )
    {
        DBG_ASSERT( !aPath.Len(), "invalid URL" );
        return FALSE;
    }

    aURL.Append( aName );

    if( !aURL.getExtension().getLength() )
        aURL.setExtension( rtl::OUString( pszExtGradient, 3 ) );

    uno::Reference< container::XNameContainer > xTable( SvxUnoXGradientTable_createInstance( this ), uno::UNO_QUERY );
    return SvxXMLXTableExportComponent::save( aURL.GetMainURL( INetURLObject::NO_DECODE ), xTable );
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
    XFillGradientItem( pXPool, GetGradient( nIndex )->GetGradient() ) );
    pXOut->SetFillAttr( pXFSet->GetItemSet() );

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

// eof
