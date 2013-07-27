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

//////////////////////////////////////////////////////////////////////////////
// class XColorEntry

XColorEntry::XColorEntry(const Color& rColor, const String& rName)
:   XPropertyEntry(rName),
    aColor(rColor)
{
}

XColorEntry::XColorEntry(const XColorEntry& rOther)
:   XPropertyEntry(rOther),
aColor(rOther.aColor)
{
}

//////////////////////////////////////////////////////////////////////////////
// class XLineEndEntry

XLineEndEntry::XLineEndEntry(const basegfx::B2DPolyPolygon& rB2DPolyPolygon, const String& rName)
:   XPropertyEntry(rName),
    aB2DPolyPolygon(rB2DPolyPolygon)
{
}

XLineEndEntry::XLineEndEntry(const XLineEndEntry& rOther)
:   XPropertyEntry(rOther),
    aB2DPolyPolygon(rOther.aB2DPolyPolygon)
{
}

//////////////////////////////////////////////////////////////////////////////
// class XDashEntry

XDashEntry::XDashEntry(const XDash& rDash, const String& rName)
:   XPropertyEntry(rName),
    aDash(rDash)
{
}

XDashEntry::XDashEntry(const XDashEntry& rOther)
:   XPropertyEntry(rOther),
aDash(rOther.aDash)
{
}

//////////////////////////////////////////////////////////////////////////////
// class XHatchEntry

XHatchEntry::XHatchEntry(const XHatch& rHatch, const String& rName)
:   XPropertyEntry(rName),
    aHatch(rHatch)
{
}

XHatchEntry::XHatchEntry(const XHatchEntry& rOther)
:   XPropertyEntry(rOther),
    aHatch(rOther.aHatch)
{
}

//////////////////////////////////////////////////////////////////////////////
// class XGradientEntry

XGradientEntry::XGradientEntry(const XGradient& rGradient, const String& rName)
:   XPropertyEntry(rName),
    aGradient(rGradient)
{
}

XGradientEntry::XGradientEntry(const XGradientEntry& rOther)
:   XPropertyEntry(rOther),
    aGradient(rOther.aGradient)
{
}

//////////////////////////////////////////////////////////////////////////////
// class XBitmapEntry

XBitmapEntry::XBitmapEntry(const GraphicObject& rGraphicObject, const String& rName)
:   XPropertyEntry(rName),
    maGraphicObject(rGraphicObject)
{
}

XBitmapEntry::XBitmapEntry(const XBitmapEntry& rOther)
:   XPropertyEntry(rOther),
    maGraphicObject(rOther.maGraphicObject)
{
}

//////////////////////////////////////////////////////////////////////////////
// class XPropertyList

XPropertyList::XPropertyList( const String& rPath ) :
            maName          ( pszStandard, 8 ),
            maPath          ( rPath ),
            maContent(),
            mbListDirty     (true)
{
}

XPropertyList::~XPropertyList()
{
    while(!maContent.empty())
    {
        delete maContent.back();
        maContent.pop_back();
    }
}

void XPropertyList::Clear()
{
    while(!maContent.empty())
    {
        delete maContent.back();
        maContent.pop_back();
    }
}

long XPropertyList::Count() const
{
    if( mbListDirty )
    {
        if(!const_cast< XPropertyList* >(this)->Load())
        {
            const_cast< XPropertyList* >(this)->Create();
        }
    }

    return maContent.size();
}

XPropertyEntry* XPropertyList::Get( long nIndex ) const
{
    if( mbListDirty )
    {
        if(!const_cast< XPropertyList* >(this)->Load())
        {
            const_cast< XPropertyList* >(this)->Create();
        }
    }

    const long nObjectCount(maContent.size());

    if(nIndex >= nObjectCount)
    {
        return 0;
    }

    return maContent[nIndex];
}

long XPropertyList::GetIndex(const XubString& rName) const
{
    if( mbListDirty )
    {
        if(!const_cast< XPropertyList* >(this)->Load())
        {
            const_cast< XPropertyList* >(this)->Create();
        }
    }

    ::std::vector< XPropertyEntry* >::const_iterator aStart(maContent.begin());
    const ::std::vector< XPropertyEntry* >::const_iterator aEnd(maContent.end());

    for(long a(0); aStart != aEnd; a++, aStart++)
    {
        const XPropertyEntry* pEntry = *aStart;

        if(pEntry && pEntry->GetName() == rName)
        {
            return a;
        }
    }

    return -1;
}

Bitmap XPropertyList::GetUiBitmap( long nIndex ) const
{
    Bitmap aRetval;
    XPropertyEntry* pEntry = Get(nIndex);

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

void XPropertyList::Insert( XPropertyEntry* pEntry, long nIndex )
{
    if(pEntry)
    {
        const long nObjectCount(maContent.size());

        if(static_cast< long >(LIST_APPEND) == nIndex || nIndex >= nObjectCount)
        {
            maContent.push_back(pEntry);
        }
        else
        {
            maContent.insert(maContent.begin() + nIndex, pEntry);
        }
    }
}

XPropertyEntry* XPropertyList::Replace( XPropertyEntry* pEntry, long nIndex )
{
    XPropertyEntry* pRetval = 0;

    if(pEntry)
    {
        const long nObjectCount(maContent.size());

        if(nIndex < nObjectCount)
        {
            pRetval = maContent[nIndex];
            maContent[nIndex] = pEntry;
        }
    }

    return pRetval;
}

XPropertyEntry* XPropertyList::Remove( long nIndex )
{
    XPropertyEntry* pRetval = 0;
    const long nObjectCount(maContent.size());

    if(nIndex < nObjectCount)
    {
        if(nIndex + 1 == nObjectCount)
        {
            pRetval = maContent.back();
            maContent.pop_back();
        }
        else
        {
            pRetval = maContent[nIndex];
            maContent.erase(maContent.begin() + nIndex);
        }
    }

    return pRetval;
}

void XPropertyList::SetName( const String& rString )
{
    if(rString.Len())
    {
        maName = rString;
    }
}

//////////////////////////////////////////////////////////////////////////////

XColorListSharedPtr XPropertyListFactory::CreateSharedXColorList( const String& rPath )
{
    return XColorListSharedPtr(new XColorList(rPath));
}

XLineEndListSharedPtr XPropertyListFactory::CreateSharedXLineEndList( const String& rPath )
{
    return XLineEndListSharedPtr(new XLineEndList(rPath));
}

XDashListSharedPtr XPropertyListFactory::CreateSharedXDashList( const String& rPath )
{
    return XDashListSharedPtr(new XDashList(rPath));
}

XHatchListSharedPtr XPropertyListFactory::CreateSharedXHatchList( const String& rPath )
{
    return XHatchListSharedPtr(new XHatchList(rPath));
}

XGradientListSharedPtr XPropertyListFactory::CreateSharedXGradientList( const String& rPath )
{
    return XGradientListSharedPtr(new XGradientList(rPath));
}

XBitmapListSharedPtr XPropertyListFactory::CreateSharedXBitmapList( const String& rPath )
{
    return XBitmapListSharedPtr(new XBitmapList(rPath));
}

//////////////////////////////////////////////////////////////////////////////
// eof
