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
#include <vcl/virdev.hxx>
#include <svx/svdmodel.hxx>

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

sharedModelAndVDev::sharedModelAndVDev()
:   mnUseCount(0),
    mpVirtualDevice(0),
    mpSdrModel(0)
{
}

sharedModelAndVDev::~sharedModelAndVDev()
{
    delete mpVirtualDevice;
    delete mpSdrModel;
}

void sharedModelAndVDev::increaseUseCount()
{
    mnUseCount++;
}

bool sharedModelAndVDev::decreaseUseCount()
{
    if(mnUseCount)
    {
        mnUseCount--;
    }

    return 0 == mnUseCount;
}

SdrModel& sharedModelAndVDev::getSharedSdrModel()
{
    if(!mpSdrModel)
    {
        mpSdrModel = new SdrModel();
        OSL_ENSURE(0 != mpSdrModel, "XPropertyList sharedModelAndVDev: no SdrModel created!" );
        mpSdrModel->GetItemPool().FreezeIdRanges();
    }

    return *mpSdrModel;
}

VirtualDevice& sharedModelAndVDev::getSharedVirtualDevice()
{
    if(!mpVirtualDevice)
    {
        mpVirtualDevice = new VirtualDevice;
        OSL_ENSURE(0 != mpVirtualDevice, "XPropertyList sharedModelAndVDev: no VirtualDevice created!" );
        mpVirtualDevice->SetMapMode(MAP_100TH_MM);
    }

    return *mpVirtualDevice;
}

sharedModelAndVDev* XPropertyList::pGlobalsharedModelAndVDev = 0;

// --------------------
// class XPropertyList
// --------------------

XPropertyList::XPropertyList( const String& rPath ) :
            maName          ( pszStandard, 8 ),
            maPath          ( rPath ),
            maContent(),
            mbListDirty     (true)
{
    if(!pGlobalsharedModelAndVDev)
    {
        pGlobalsharedModelAndVDev = new sharedModelAndVDev();
    }

    pGlobalsharedModelAndVDev->increaseUseCount();
}

/*************************************************************************
|*
|* XPropertyList::~XPropertyList()
|*
*************************************************************************/

XPropertyList::~XPropertyList()
{
    while(!maContent.empty())
    {
        delete maContent.back();
        maContent.pop_back();
    }

    if(pGlobalsharedModelAndVDev && pGlobalsharedModelAndVDev->decreaseUseCount())
    {
        delete pGlobalsharedModelAndVDev;
        pGlobalsharedModelAndVDev = 0;
    }
}

/*************************************************************************
|*
|* XPropertyList::Clear()
|*
*************************************************************************/

void XPropertyList::Clear()
{
    while(!maContent.empty())
    {
        delete maContent.back();
        maContent.pop_back();
    }
}

/************************************************************************/

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

/*************************************************************************
|*
|* XPropertyEntry* XPropertyList::Get()
|*
*************************************************************************/

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

/*************************************************************************
|*
|* XPropertyList::Get()
|*
*************************************************************************/

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

/*************************************************************************
|*
|* Bitmap* XPropertyList::GetBitmap()
|*
*************************************************************************/

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

/*************************************************************************
|*
|* void XPropertyList::Insert()
|*
*************************************************************************/

void XPropertyList::Insert( XPropertyEntry* pEntry, long nIndex )
{
    if(pEntry)
    {
        const long nObjectCount(maContent.size());

        if(LIST_APPEND == nIndex || nIndex >= nObjectCount)
        {
            maContent.push_back(pEntry);
        }
        else
        {
            maContent.insert(maContent.begin() + nIndex, pEntry);
        }
    }
}

/*************************************************************************
|*
|* void XPropertyList::Replace()
|*
*************************************************************************/

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

/*************************************************************************
|*
|* void XPropertyList::Remove()
|*
*************************************************************************/

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

/************************************************************************/

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
