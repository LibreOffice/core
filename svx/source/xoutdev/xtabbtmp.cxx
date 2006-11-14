/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xtabbtmp.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: ihi $ $Date: 2006-11-14 13:57:33 $
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

#define GLOBALOVERFLOW

using namespace com::sun::star;
using namespace rtl;

sal_Unicode const pszExtBitmap[]  = {'s','o','b'};

static char const aChckBitmap[]  = { 0x04, 0x00, 'S','O','B','L'};  // very old
static char const aChckBitmap0[] = { 0x04, 0x00, 'S','O','B','0'};  // old
static char const aChckBitmap1[] = { 0x04, 0x00, 'S','O','B','1'};  // = 5.2
static char const aChckXML[]     = { 'P', 'K', 0x03, 0x04 };        // = 6.0

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

XBitmapEntry* XBitmapTable::GetBitmap(long nIndex) const
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

Bitmap* XBitmapTable::CreateBitmapForUI( long /*nIndex*/, BOOL /*bDelete*/)
{
    return( NULL );
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

XBitmapEntry* XBitmapList::GetBitmap(long nIndex) const
{
    return (XBitmapEntry*) XPropertyList::Get(nIndex, 0);
}

/************************************************************************/

BOOL XBitmapList::Load()
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
            aURL.setExtension( rtl::OUString( pszExtBitmap, 3 ) );

        uno::Reference< container::XNameContainer > xTable( SvxUnoXBitmapTable_createInstance( this ), uno::UNO_QUERY );
        return SvxXMLXTableImport::load( aURL.GetMainURL( INetURLObject::NO_DECODE ), xTable );
    }
    return( FALSE );
}

/************************************************************************/

BOOL XBitmapList::Save()
{
    INetURLObject aURL( aPath );

    if( INET_PROT_NOT_VALID == aURL.GetProtocol() )
    {
        DBG_ASSERT( !aPath.Len(), "invalid URL" );
        return FALSE;
    }

    aURL.Append( aName );

    if( !aURL.getExtension().getLength() )
        aURL.setExtension( rtl::OUString( pszExtBitmap, 3 ) );

    uno::Reference< container::XNameContainer > xTable( SvxUnoXBitmapTable_createInstance( this ), uno::UNO_QUERY );
    return SvxXMLXTableExportComponent::save( aURL.GetMainURL( INetURLObject::NO_DECODE ), xTable );
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

Bitmap* XBitmapList::CreateBitmapForUI( long /*nIndex*/, BOOL /*bDelete*/)
{
    return( NULL );
}

// eof
