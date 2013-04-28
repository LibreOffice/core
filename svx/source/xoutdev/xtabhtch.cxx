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

// include ---------------------------------------------------------------

#ifndef SVX_LIGHT

#include <com/sun/star/container/XNameContainer.hpp>
#include "svx/XPropertyTable.hxx"
#include <unotools/ucbstreamhelper.hxx>
#include <vcl/svapp.hxx>

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
#include "svx/dlgutil.hxx"
#include <svx/xflhtit.hxx>
#include <svx/xflclit.hxx>
#include <svx/xfillit0.hxx>

#include <svx/svdorect.hxx>
#include <svx/svdmodel.hxx>
#include <svx/sdr/contact/objectcontactofobjlistpainter.hxx>
#include <svx/sdr/contact/displayinfo.hxx>
#include <svx/xlnclit.hxx>

using namespace ::com::sun::star;
using namespace ::rtl;

sal_Unicode const pszExtHatch[]  = {'s','o','h'};
//char const aChckHatch[]  = { 0x04, 0x00, 'S','O','H','L'};    // < 5.2
//char const aChckHatch0[] = { 0x04, 0x00, 'S','O','H','0'};    // = 5.2
//char const aChckXML[]    = { '<', '?', 'x', 'm', 'l' };       // = 6.0

// -----------------
// class XHatchList
// -----------------

XHatchList::XHatchList(const String& rPath )
:   XPropertyList(rPath),
    mpBackgroundObject(0),
    mpHatchObject(0)
{
}

XHatchList::~XHatchList()
{
    SdrObject::Free(mpBackgroundObject);
    SdrObject::Free(mpHatchObject);
}

XHatchEntry* XHatchList::Replace(XHatchEntry* pEntry, long nIndex )
{
    return (XHatchEntry*) XPropertyList::Replace(pEntry, nIndex);
}

XHatchEntry* XHatchList::Remove(long nIndex)
{
    return (XHatchEntry*) XPropertyList::Remove(nIndex);
}

XHatchEntry* XHatchList::GetHatch(long nIndex) const
{
    return (XHatchEntry*) XPropertyList::Get(nIndex);
}

sal_Bool XHatchList::Load()
{
    if( mbListDirty )
    {
        mbListDirty = false;

        INetURLObject aURL( maPath );

        if( INET_PROT_NOT_VALID == aURL.GetProtocol() )
        {
            DBG_ASSERT( !maPath.Len(), "invalid URL" );
            return sal_False;
        }

        aURL.Append( maName );

        if( !aURL.getExtension().getLength() )
            aURL.setExtension( rtl::OUString( pszExtHatch, 3 ) );

        uno::Reference< container::XNameContainer > xTable( SvxUnoXHatchTable_createInstance( this ), uno::UNO_QUERY );
        return SvxXMLXTableImport::load( aURL.GetMainURL( INetURLObject::NO_DECODE ), xTable );
    }
    return( sal_False );
}

sal_Bool XHatchList::Save()
{
    INetURLObject aURL( maPath );

    if( INET_PROT_NOT_VALID == aURL.GetProtocol() )
    {
        DBG_ASSERT( !maPath.Len(), "invalid URL" );
        return sal_False;
    }

    aURL.Append( maName );

    if( !aURL.getExtension().getLength() )
        aURL.setExtension( rtl::OUString( pszExtHatch, 3 ) );

    uno::Reference< container::XNameContainer > xTable( SvxUnoXHatchTable_createInstance( this ), uno::UNO_QUERY );
    return SvxXMLXTableExportComponent::save( aURL.GetMainURL( INetURLObject::NO_DECODE ), xTable );
}

sal_Bool XHatchList::Create()
{
    XubString aStr( SVX_RES( RID_SVXSTR_HATCH ) );
    xub_StrLen nLen;

    aStr.AppendAscii(" 1");
    nLen = aStr.Len() - 1;
    Insert(new XHatchEntry(XHatch(RGB_Color(COL_BLACK),XHATCH_SINGLE,100,  0),aStr));
    aStr.SetChar(nLen, sal_Unicode('2'));
    Insert(new XHatchEntry(XHatch(RGB_Color(COL_RED  ),XHATCH_DOUBLE, 80,450),aStr));
    aStr.SetChar(nLen, sal_Unicode('3'));
    Insert(new XHatchEntry(XHatch(RGB_Color(COL_BLUE ),XHATCH_TRIPLE,120,  0),aStr));

    return( sal_True );
}

Bitmap XHatchList::CreateBitmapForUI( long nIndex )
{
    Bitmap aRetval;
    OSL_ENSURE(pGlobalsharedModelAndVDev, "OOps, global values missing (!)");
    OSL_ENSURE(nIndex < Count(), "OOps, global values missing (!)");

    if(pGlobalsharedModelAndVDev && nIndex < Count())
    {
        SdrModel& rModel = pGlobalsharedModelAndVDev->getSharedSdrModel();
        VirtualDevice& rVirDev = pGlobalsharedModelAndVDev->getSharedVirtualDevice();
        const Point aZero(0, 0);
        const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
        const Size aSize(rVirDev.PixelToLogic(rStyleSettings.GetListBoxPreviewDefaultPixelSize()));

        rVirDev.SetOutputSize(aSize);
        rVirDev.SetDrawMode(rStyleSettings.GetHighContrastMode()
            ? DRAWMODE_SETTINGSLINE | DRAWMODE_SETTINGSFILL | DRAWMODE_SETTINGSTEXT | DRAWMODE_SETTINGSGRADIENT
            : DRAWMODE_DEFAULT);
        rVirDev.SetBackground(rStyleSettings.GetFieldColor());

        const Size aSinglePixel(rVirDev.PixelToLogic(Size(1, 1)));
        const Rectangle aBackgroundSize(aZero, Size(aSize.getWidth() - aSinglePixel.getWidth(), aSize.getHeight() - aSinglePixel.getHeight()));

        if(!mpBackgroundObject)
        {
            mpBackgroundObject = new SdrRectObj(aBackgroundSize);
            OSL_ENSURE(0 != mpBackgroundObject, "XDashList: no BackgroundObject created!" );
            mpBackgroundObject->SetModel(&rModel);
            mpBackgroundObject->SetMergedItem(XFillStyleItem(XFILL_SOLID));
            mpBackgroundObject->SetMergedItem(XFillColorItem(String(), rStyleSettings.GetFieldColor()));
            mpBackgroundObject->SetMergedItem(XLineStyleItem(XLINE_SOLID));
            mpBackgroundObject->SetMergedItem(XLineColorItem(String(), Color(COL_BLACK)));
        }

        if(!mpHatchObject)
        {
            mpHatchObject = new SdrRectObj(aBackgroundSize);
            OSL_ENSURE(0 != mpHatchObject, "XDashList: no HatchObject created!" );
            mpHatchObject->SetModel(&rModel);
            mpHatchObject->SetMergedItem(XFillStyleItem(XFILL_HATCH));
            mpHatchObject->SetMergedItem(XLineStyleItem(XLINE_NONE));
        }

        mpHatchObject->SetMergedItem(XFillStyleItem(XFILL_HATCH));
        mpHatchObject->SetMergedItem(XFillHatchItem(String(), GetHatch(nIndex)->GetHatch()));

        sdr::contact::SdrObjectVector aObjectVector;

        aObjectVector.push_back(mpBackgroundObject);
        aObjectVector.push_back(mpHatchObject);

        sdr::contact::ObjectContactOfObjListPainter aPainter(rVirDev, aObjectVector, 0);
        sdr::contact::DisplayInfo aDisplayInfo;

        rVirDev.Erase();
        aPainter.ProcessDisplay(aDisplayInfo);

        aRetval = rVirDev.GetBitmap(aZero, rVirDev.GetOutputSize());
    }

    return aRetval;
}

//////////////////////////////////////////////////////////////////////////////
// eof
