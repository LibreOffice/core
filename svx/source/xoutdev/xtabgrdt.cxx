/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

// include ---------------------------------------------------------------

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
#include <svx/xfillit0.hxx>
#include <svx/xflgrit.hxx>

#include <svx/svdorect.hxx>
#include <svx/svdmodel.hxx>
#include <svx/sdr/contact/objectcontactofobjlistpainter.hxx>
#include <svx/sdr/contact/displayinfo.hxx>
#include <vcl/svapp.hxx>
#include <svx/xlnclit.hxx>
#include <svx/xgrscit.hxx>

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
                            sal_uInt16 nInitSize, sal_uInt16 nReSize ) :
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

sal_Bool XGradientTable::Load()
{
    return( sal_False );
}

/************************************************************************/

sal_Bool XGradientTable::Save()
{
    return( sal_False );
}

/************************************************************************/

sal_Bool XGradientTable::Create()
{
    return( sal_False );
}

/************************************************************************/

sal_Bool XGradientTable::CreateBitmapsForUI()
{
    return( sal_False );
}

/************************************************************************/

Bitmap* XGradientTable::CreateBitmapForUI( long /*nIndex*/, sal_Bool /*bDelete*/)
{
    return( NULL );
}

// --------------------
// class XGradientList
// --------------------

class impXGradientList
{
private:
    VirtualDevice*          mpVirtualDevice;
    SdrModel*               mpSdrModel;
    SdrObject*              mpBackgroundObject;

public:
    impXGradientList(VirtualDevice* pV, SdrModel* pM, SdrObject* pB)
    :   mpVirtualDevice(pV),
        mpSdrModel(pM),
        mpBackgroundObject(pB)
    {}

    ~impXGradientList()
    {
        delete mpVirtualDevice;
        SdrObject::Free(mpBackgroundObject);
        delete mpSdrModel;
    }

    VirtualDevice* getVirtualDevice() const { return mpVirtualDevice; }
    SdrObject* getBackgroundObject() const { return mpBackgroundObject; }
};

void XGradientList::impCreate()
{
    if(!mpData)
    {
        const Point aZero(0, 0);
        const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();

        VirtualDevice* pVirDev = new VirtualDevice;
        OSL_ENSURE(0 != pVirDev, "XGradientList: no VirtualDevice created!" );
        pVirDev->SetMapMode(MAP_100TH_MM);
        const Size aSize(pVirDev->PixelToLogic(Size(BITMAP_WIDTH, BITMAP_HEIGHT)));
        pVirDev->SetOutputSize(aSize);
        pVirDev->SetDrawMode(rStyleSettings.GetHighContrastMode()
            ? DRAWMODE_SETTINGSLINE | DRAWMODE_SETTINGSFILL | DRAWMODE_SETTINGSTEXT | DRAWMODE_SETTINGSGRADIENT
            : DRAWMODE_DEFAULT);

        SdrModel* pSdrModel = new SdrModel();
        OSL_ENSURE(0 != pSdrModel, "XGradientList: no SdrModel created!" );
        pSdrModel->GetItemPool().FreezeIdRanges();

        const Size aSinglePixel(pVirDev->PixelToLogic(Size(1, 1)));
        const Rectangle aBackgroundSize(aZero, Size(aSize.getWidth() - aSinglePixel.getWidth(), aSize.getHeight() - aSinglePixel.getHeight()));
        SdrObject* pBackgroundObject = new SdrRectObj(aBackgroundSize);
        OSL_ENSURE(0 != pBackgroundObject, "XGradientList: no BackgroundObject created!" );
        pBackgroundObject->SetModel(pSdrModel);
        pBackgroundObject->SetMergedItem(XFillStyleItem(XFILL_GRADIENT));
        pBackgroundObject->SetMergedItem(XLineStyleItem(XLINE_SOLID));
        pBackgroundObject->SetMergedItem(XLineColorItem(String(), Color(COL_BLACK)));
        pBackgroundObject->SetMergedItem(XGradientStepCountItem(sal_uInt16((BITMAP_WIDTH + BITMAP_HEIGHT) / 3)));

        mpData = new impXGradientList(pVirDev, pSdrModel, pBackgroundObject);
        OSL_ENSURE(0 != mpData, "XGradientList: data creation went wrong!" );
    }
}

void XGradientList::impDestroy()
{
    if(mpData)
    {
        delete mpData;
        mpData = 0;
    }
}

XGradientList::XGradientList( const String& rPath, XOutdevItemPool* pInPool, sal_uInt16 nInitSize, sal_uInt16 nReSize)
:   XPropertyList(rPath, pInPool, nInitSize, nReSize),
    mpData(0)
{
    pBmpList = new List(nInitSize, nReSize);
}

XGradientList::~XGradientList()
{
    if(mpData)
    {
        delete mpData;
        mpData = 0;
    }
}

XGradientEntry* XGradientList::Replace(XGradientEntry* pEntry, long nIndex )
{
    return( (XGradientEntry*) XPropertyList::Replace( pEntry, nIndex ) );
}

XGradientEntry* XGradientList::Remove(long nIndex)
{
    return( (XGradientEntry*) XPropertyList::Remove( nIndex, 0 ) );
}

XGradientEntry* XGradientList::GetGradient(long nIndex) const
{
    return( (XGradientEntry*) XPropertyList::Get( nIndex, 0 ) );
}

sal_Bool XGradientList::Load()
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
            aURL.setExtension( rtl::OUString( pszExtGradient, 3 ) );

        uno::Reference< container::XNameContainer > xTable( SvxUnoXGradientTable_createInstance( this ), uno::UNO_QUERY );
        return SvxXMLXTableImport::load( aURL.GetMainURL( INetURLObject::NO_DECODE ), xTable );

    }
    return( sal_False );
}

sal_Bool XGradientList::Save()
{
    INetURLObject aURL( aPath );

    if( INET_PROT_NOT_VALID == aURL.GetProtocol() )
    {
        DBG_ASSERT( !aPath.Len(), "invalid URL" );
        return sal_False;
    }

    aURL.Append( aName );

    if( !aURL.getExtension().getLength() )
        aURL.setExtension( rtl::OUString( pszExtGradient, 3 ) );

    uno::Reference< container::XNameContainer > xTable( SvxUnoXGradientTable_createInstance( this ), uno::UNO_QUERY );
    return SvxXMLXTableExportComponent::save( aURL.GetMainURL( INetURLObject::NO_DECODE ), xTable );
}

sal_Bool XGradientList::Create()
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

    return( sal_True );
}

sal_Bool XGradientList::CreateBitmapsForUI()
{
    impCreate();

    for( long i = 0; i < Count(); i++)
    {
        Bitmap* pBmp = CreateBitmapForUI( i, sal_False );
        DBG_ASSERT( pBmp, "XGradientList: Bitmap(UI) konnte nicht erzeugt werden!" );

        if( pBmp )
            pBmpList->Insert( pBmp, i );
    }

    impDestroy();

    return( sal_False );
}

Bitmap* XGradientList::CreateBitmapForUI( long nIndex, sal_Bool bDelete )
{
    impCreate();
    VirtualDevice* pVD = mpData->getVirtualDevice();
    SdrObject* pBackgroundObject = mpData->getBackgroundObject();

    const SfxItemSet& rItemSet = pBackgroundObject->GetMergedItemSet();
    pBackgroundObject->SetMergedItem(XFillStyleItem(XFILL_GRADIENT));
    pBackgroundObject->SetMergedItem(XFillGradientItem(rItemSet.GetPool(), GetGradient(nIndex)->GetGradient()));

    sdr::contact::SdrObjectVector aObjectVector;
    aObjectVector.push_back(pBackgroundObject);
    sdr::contact::ObjectContactOfObjListPainter aPainter(*pVD, aObjectVector, 0);
    sdr::contact::DisplayInfo aDisplayInfo;

    aPainter.ProcessDisplay(aDisplayInfo);

    const Point aZero(0, 0);
    Bitmap* pBitmap = new Bitmap(pVD->GetBitmap(aZero, pVD->GetOutputSize()));

    if(bDelete)
    {
        impDestroy();
    }

    return pBitmap;
}

//////////////////////////////////////////////////////////////////////////////
// eof
