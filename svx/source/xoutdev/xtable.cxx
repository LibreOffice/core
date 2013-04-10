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

#include <svx/xtable.hxx>
#include <svx/xpool.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdpool.hxx>

#define GLOBALOVERFLOW

// Vergleichsstrings
sal_Unicode __FAR_DATA pszStandard[] = { 's', 't', 'a', 'n', 'd', 'a', 'r', 'd', 0 };

// Konvertiert in echte RGB-Farben, damit in den Listboxen
// endlich mal richtig selektiert werden kann.
Color RGB_Color( ColorData nColorName )
{
    Color aColor( nColorName );
    Color aRGBColor( aColor.GetRed(), aColor.GetGreen(), aColor.GetBlue() );
    return aRGBColor;
}

// --------------------
// class XPropertyList
// --------------------

XPropertyList::XPropertyList( const String& rPath, XOutdevItemPool* pInPool ) :
            maName          ( pszStandard, 8 ),
            maPath          ( rPath ),
            mpXPool         ( pInPool ),
            maList          ( 16, 16 ),
            mbListDirty     (true)
{
    if( !mpXPool )
    {
        mpXPool = static_cast< XOutdevItemPool* >(&SdrObject::GetGlobalDrawObjectItemPool());
    }
}

/*************************************************************************
|*
|* XPropertyList::~XPropertyList()
|*
*************************************************************************/

XPropertyList::~XPropertyList()
{
    XPropertyEntry* pEntry = (XPropertyEntry*)maList.First();
    for( sal_uIntPtr nIndex = 0; nIndex < maList.Count(); nIndex++ )
    {
        delete pEntry;
        pEntry = (XPropertyEntry*)maList.Next();
    }
}

/*************************************************************************
|*
|* XPropertyList::Clear()
|*
*************************************************************************/

void XPropertyList::Clear()
{
    maList.Clear();
}

/************************************************************************/

long XPropertyList::Count() const
{
    if( mbListDirty )
    {
        // ( (XPropertyList*) this )->bListDirty = sal_False; <- im Load()
        if( !( (XPropertyList*) this )->Load() )
            ( (XPropertyList*) this )->Create();
    }
    return( maList.Count() );
}

/*************************************************************************
|*
|* XPropertyEntry* XPropertyList::Get()
|*
*************************************************************************/

XPropertyEntry* XPropertyList::Get( long nIndex, sal_uInt16 /*nDummy*/) const
{
    if( mbListDirty )
    {
        // ( (XPropertyList*) this )->bListDirty = sal_False; <- im Load()
        if( !( (XPropertyList*) this )->Load() )
            ( (XPropertyList*) this )->Create();
    }
    return (XPropertyEntry*) maList.GetObject( (sal_uIntPtr) nIndex );
}

/*************************************************************************
|*
|* XPropertyList::Get()
|*
*************************************************************************/

long XPropertyList::Get(const XubString& rName)
{
    if( mbListDirty )
    {
        //bListDirty = sal_False;
        if( !Load() )
            Create();
    }
    long nPos = 0;
    XPropertyEntry* pEntry = (XPropertyEntry*)maList.First();
    while (pEntry && pEntry->GetName() != rName)
    {
        nPos++;
        pEntry = (XPropertyEntry*)maList.Next();
    }
    if (!pEntry) nPos = -1;
    return nPos;
}

/*************************************************************************
|*
|* Bitmap* XPropertyList::GetBitmap()
|*
*************************************************************************/

Bitmap XPropertyList::GetUiBitmap( long nIndex ) const
{
    Bitmap aRetval;
    XPropertyEntry* pEntry = (XPropertyEntry*)maList.GetObject((sal_uIntPtr)nIndex);

    if(pEntry)
    {
        aRetval = pEntry->GetUiBitmap();

        if(aRetval.IsEmpty())
        {
            aRetval = const_cast< XPropertyList* >(this)->CreateBitmapForUI(nIndex);
            pEntry->SetUiBitmap(aRetval);
        }
    }

    return aRetval;
}

/*************************************************************************
|*
|* void XPropertyList::Insert()
|*
*************************************************************************/

void XPropertyList::Insert( XPropertyEntry* pEntry, long nIndex )
{
    maList.Insert( pEntry, (sal_uIntPtr) nIndex );
}

/*************************************************************************
|*
|* void XPropertyList::Replace()
|*
*************************************************************************/

XPropertyEntry* XPropertyList::Replace( XPropertyEntry* pEntry, long nIndex )
{
    return  (XPropertyEntry*) maList.Replace( pEntry, (sal_uIntPtr) nIndex );
}

/*************************************************************************
|*
|* void XPropertyList::Remove()
|*
*************************************************************************/

XPropertyEntry* XPropertyList::Remove( long nIndex, sal_uInt16 /*nDummy*/)
{
    return (XPropertyEntry*) maList.Remove( (sal_uIntPtr) nIndex );
}

/************************************************************************/

void XPropertyList::SetName( const String& rString )
{
    if(rString.Len())
    {
        maName = rString;
    }
}

sal_uInt32 XPropertyList::getUiBitmapWidth() const
{
    static sal_uInt32 nWidth = 32; // alternative: 42;

    return nWidth;
}

sal_uInt32 XPropertyList::getUiBitmapHeight() const
{
    static sal_uInt32 nHeight = 12; // alternative: 16;

    return nHeight;
}

sal_uInt32 XPropertyList::getUiBitmapLineWidth() const
{
    static sal_uInt32 nLineWidth = 1;

    return nLineWidth;
}

// eof
