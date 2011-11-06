/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#ifndef SVX_LIGHT

#include <com/sun/star/container/XNameContainer.hpp>
#include "svx/XPropertyTable.hxx"
#include <unotools/ucbstreamhelper.hxx>

#include "xmlxtexp.hxx"
#include "xmlxtimp.hxx"

#endif

#include <tools/urlobj.hxx>
#include <vcl/virdev.hxx>
#include <svl/itemset.hxx>
#include <sfx2/docfile.hxx>
#include <svx/dialogs.hrc>
#include <svx/dialmgr.hxx>
#include <svx/xtable.hxx>
#include <svx/xpool.hxx>

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
                            sal_uInt16 nInitSize, sal_uInt16 nReSize ) :
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

sal_Bool XBitmapTable::Load()
{
    return( sal_False );
}

/************************************************************************/

sal_Bool XBitmapTable::Save()
{
    return( sal_False );
}

/************************************************************************/

sal_Bool XBitmapTable::Create()
{
    return( sal_False );
}

/************************************************************************/

sal_Bool XBitmapTable::CreateBitmapsForUI()
{
    return( sal_False );
}

/************************************************************************/

Bitmap* XBitmapTable::CreateBitmapForUI( long /*nIndex*/, sal_Bool /*bDelete*/)
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
                            sal_uInt16 nInitSize, sal_uInt16 nReSize ) :
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

sal_Bool XBitmapList::Load()
{
    if( bListDirty )
    {
        bListDirty = sal_False;

        INetURLObject aURL( aPath );

        if( INET_PROT_NOT_VALID == aURL.GetProtocol() )
        {
            DBG_ASSERT( !aPath.Len(), "invalid URL" );
            return sal_False;
        }

        aURL.Append( aName );

        if( !aURL.getExtension().getLength() )
            aURL.setExtension( rtl::OUString( pszExtBitmap, 3 ) );

        uno::Reference< container::XNameContainer > xTable( SvxUnoXBitmapTable_createInstance( this ), uno::UNO_QUERY );
        return SvxXMLXTableImport::load( aURL.GetMainURL( INetURLObject::NO_DECODE ), xTable );
    }
    return( sal_False );
}

/************************************************************************/

sal_Bool XBitmapList::Save()
{
    INetURLObject aURL( aPath );

    if( INET_PROT_NOT_VALID == aURL.GetProtocol() )
    {
        DBG_ASSERT( !aPath.Len(), "invalid URL" );
        return sal_False;
    }

    aURL.Append( aName );

    if( !aURL.getExtension().getLength() )
        aURL.setExtension( rtl::OUString( pszExtBitmap, 3 ) );

    uno::Reference< container::XNameContainer > xTable( SvxUnoXBitmapTable_createInstance( this ), uno::UNO_QUERY );
    return SvxXMLXTableExportComponent::save( aURL.GetMainURL( INetURLObject::NO_DECODE ), xTable );
}

/************************************************************************/
// Umgestellt am 27.07.95 auf XBitmap

sal_Bool XBitmapList::Create()
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
    sal_uInt16  aArray[64];

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

    return( sal_True );
}

/************************************************************************/

sal_Bool XBitmapList::CreateBitmapsForUI()
{
    return( sal_False );
}

/************************************************************************/

Bitmap* XBitmapList::CreateBitmapForUI( long /*nIndex*/, sal_Bool /*bDelete*/)
{
    return( NULL );
}

// eof
