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
//char const aChckGradient[]  = { 0x04, 0x00, 'S','O','G','L'}; // < 5.2
//char const aChckGradient0[] = { 0x04, 0x00, 'S','O','G','0'}; // = 5.2
//char const aChckXML[]       = { '<', '?', 'x', 'm', 'l' };        // = 6.0

// --------------------
// class XGradientList
// --------------------

XGradientList::XGradientList( const String& rPath )
:   XPropertyList(rPath ),
    mpBackgroundObject(0)
{
}

XGradientList::~XGradientList()
{
    SdrObject::Free(mpBackgroundObject);
}

XGradientEntry* XGradientList::Replace(XGradientEntry* pEntry, long nIndex )
{
    return( (XGradientEntry*) XPropertyList::Replace( pEntry, nIndex ) );
}

XGradientEntry* XGradientList::Remove(long nIndex)
{
    return( (XGradientEntry*) XPropertyList::Remove( nIndex ) );
}

XGradientEntry* XGradientList::GetGradient(long nIndex) const
{
    return( (XGradientEntry*) XPropertyList::Get( nIndex ) );
}

sal_Bool XGradientList::Load()
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
            aURL.setExtension( rtl::OUString( pszExtGradient, 3 ) );

        uno::Reference< container::XNameContainer > xTable( SvxUnoXGradientTable_createInstance( this ), uno::UNO_QUERY );
        return SvxXMLXTableImport::load( aURL.GetMainURL( INetURLObject::NO_DECODE ), xTable );

    }
    return( sal_False );
}

sal_Bool XGradientList::Save()
{
    INetURLObject aURL( maPath );

    if( INET_PROT_NOT_VALID == aURL.GetProtocol() )
    {
        DBG_ASSERT( !maPath.Len(), "invalid URL" );
        return sal_False;
    }

    aURL.Append( maName );

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

Bitmap XGradientList::CreateBitmapForUI( long nIndex )
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
        const Size& rSize = rStyleSettings.GetListBoxPreviewDefaultPixelSize();
        const Size aSize(rVirDev.PixelToLogic(rSize));

        rVirDev.SetOutputSize(aSize);
        rVirDev.SetDrawMode(rStyleSettings.GetHighContrastMode()
            ? DRAWMODE_SETTINGSLINE | DRAWMODE_SETTINGSFILL | DRAWMODE_SETTINGSTEXT | DRAWMODE_SETTINGSGRADIENT
            : DRAWMODE_DEFAULT);
        rVirDev.SetBackground(rStyleSettings.GetFieldColor());

        if(!mpBackgroundObject)
        {
            const Size aSinglePixel(rVirDev.PixelToLogic(Size(1, 1)));
            const Rectangle aBackgroundSize(aZero, Size(aSize.getWidth() - aSinglePixel.getWidth(), aSize.getHeight() - aSinglePixel.getHeight()));
            mpBackgroundObject = new SdrRectObj(aBackgroundSize);
            OSL_ENSURE(0 != mpBackgroundObject, "XGradientList: no BackgroundObject created!" );
            mpBackgroundObject->SetModel(&rModel);
            mpBackgroundObject->SetMergedItem(XFillStyleItem(XFILL_GRADIENT));
            mpBackgroundObject->SetMergedItem(XLineStyleItem(XLINE_SOLID));
            mpBackgroundObject->SetMergedItem(XLineColorItem(String(), Color(COL_BLACK)));
            mpBackgroundObject->SetMergedItem(XGradientStepCountItem(sal_uInt16((rSize.Width() + rSize.Height()) / 3)));
        }

        const SfxItemSet& rItemSet = mpBackgroundObject->GetMergedItemSet();

        mpBackgroundObject->SetMergedItem(XFillStyleItem(XFILL_GRADIENT));
        mpBackgroundObject->SetMergedItem(XFillGradientItem(rItemSet.GetPool(), GetGradient(nIndex)->GetGradient()));

        sdr::contact::SdrObjectVector aObjectVector;

        aObjectVector.push_back(mpBackgroundObject);

        sdr::contact::ObjectContactOfObjListPainter aPainter(rVirDev, aObjectVector, 0);
        sdr::contact::DisplayInfo aDisplayInfo;

        rVirDev.Erase();
        aPainter.ProcessDisplay(aDisplayInfo);

        return rVirDev.GetBitmap(aZero, rVirDev.GetOutputSize());

    }

    return aRetval;
}

//////////////////////////////////////////////////////////////////////////////
// eof
