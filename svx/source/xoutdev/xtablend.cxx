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

#ifndef _SV_APP_HXX
#include <vcl/svapp.hxx>
#endif
#include <svl/itemset.hxx>
#include <sfx2/docfile.hxx>

#include <svx/dialogs.hrc>
#include <svx/dialmgr.hxx>

#include <svx/xtable.hxx>
#include <svx/xpool.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xflclit.hxx>
#include <svx/xlnstwit.hxx>
#include <svx/xlnedwit.hxx>
#include <svx/xlnclit.hxx>
#include <svx/xlineit0.hxx>
#include <svx/xlnstit.hxx>
#include <svx/xlnedit.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>

#include <svx/svdorect.hxx>
#include <svx/svdopath.hxx>
#include <svx/svdmodel.hxx>
#include <svx/sdr/contact/objectcontactofobjlistpainter.hxx>
#include <svx/sdr/contact/displayinfo.hxx>
#include <svx/xlnwtit.hxx>

#define GLOBALOVERFLOW

using namespace com::sun::star;
using namespace rtl;

sal_Unicode const pszExtLineEnd[]   = {'s','o','e'};
//static char const aChckLEnd[]  = { 0x04, 0x00, 'S','O','E','L'};  // < 5.2
//static char const aChckLEnd0[] = { 0x04, 0x00, 'S','O','E','0'};  // = 5.2
//static char const aChckXML[]   = { '<', '?', 'x', 'm', 'l' };     // = 6.0

// --------------------
// class XLineEndList
// --------------------

XLineEndList::XLineEndList(const String& rPath)
:   XPropertyList(rPath),
    mpBackgroundObject(0),
    mpLineObject(0)
{
}

XLineEndList::~XLineEndList()
{
    SdrObject::Free(mpBackgroundObject);
    SdrObject::Free(mpLineObject);
}

XLineEndEntry* XLineEndList::Replace(XLineEndEntry* pEntry, long nIndex )
{
    return (XLineEndEntry*) XPropertyList::Replace(pEntry, nIndex);
}

XLineEndEntry* XLineEndList::Remove(long nIndex)
{
    return (XLineEndEntry*) XPropertyList::Remove(nIndex);
}

XLineEndEntry* XLineEndList::GetLineEnd(long nIndex) const
{
    return (XLineEndEntry*) XPropertyList::Get(nIndex);
}

sal_Bool XLineEndList::Load()
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
            aURL.setExtension( rtl::OUString( pszExtLineEnd, 3 ) );

        uno::Reference< container::XNameContainer > xTable( SvxUnoXLineEndTable_createInstance( this ), uno::UNO_QUERY );
        return SvxXMLXTableImport::load( aURL.GetMainURL( INetURLObject::NO_DECODE ), xTable );
    }
    return( sal_False );
}

sal_Bool XLineEndList::Save()
{
    INetURLObject aURL( maPath );

    if( INET_PROT_NOT_VALID == aURL.GetProtocol() )
    {
        DBG_ASSERT( !maPath.Len(), "invalid URL" );
        return sal_False;
    }

    aURL.Append( maName );

    if( !aURL.getExtension().getLength() )
        aURL.setExtension( rtl::OUString( pszExtLineEnd, 3 ) );

    uno::Reference< container::XNameContainer > xTable( SvxUnoXLineEndTable_createInstance( this ), uno::UNO_QUERY );
    return SvxXMLXTableExportComponent::save( aURL.GetMainURL( INetURLObject::NO_DECODE ), xTable );
}

sal_Bool XLineEndList::Create()
{
    basegfx::B2DPolygon aTriangle;
    aTriangle.append(basegfx::B2DPoint(10.0, 0.0));
    aTriangle.append(basegfx::B2DPoint(0.0, 30.0));
    aTriangle.append(basegfx::B2DPoint(20.0, 30.0));
    aTriangle.setClosed(true);
    Insert( new XLineEndEntry( basegfx::B2DPolyPolygon(aTriangle), SVX_RESSTR( RID_SVXSTR_ARROW ) ) );

    basegfx::B2DPolygon aSquare;
    aSquare.append(basegfx::B2DPoint(0.0, 0.0));
    aSquare.append(basegfx::B2DPoint(10.0, 0.0));
    aSquare.append(basegfx::B2DPoint(10.0, 10.0));
    aSquare.append(basegfx::B2DPoint(0.0, 10.0));
    aSquare.setClosed(true);
    Insert( new XLineEndEntry( basegfx::B2DPolyPolygon(aSquare), SVX_RESSTR( RID_SVXSTR_SQUARE ) ) );

    basegfx::B2DPolygon aCircle(basegfx::tools::createPolygonFromCircle(basegfx::B2DPoint(0.0, 0.0), 100.0));
    Insert( new XLineEndEntry( basegfx::B2DPolyPolygon(aCircle), SVX_RESSTR( RID_SVXSTR_CIRCLE ) ) );

    return( sal_True );
}

Bitmap XLineEndList::CreateBitmapForUI( long nIndex )
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
        const Size aSize(rVirDev.PixelToLogic(Size(rSize.Width() * 2, rSize.Height())));

        rVirDev.SetOutputSize(aSize);
        rVirDev.SetDrawMode(rStyleSettings.GetHighContrastMode()
            ? DRAWMODE_SETTINGSLINE | DRAWMODE_SETTINGSFILL | DRAWMODE_SETTINGSTEXT | DRAWMODE_SETTINGSGRADIENT
            : DRAWMODE_DEFAULT);
        rVirDev.SetBackground(rStyleSettings.GetFieldColor());

        if(!mpBackgroundObject)
        {
            const Rectangle aBackgroundSize(aZero, aSize);
            mpBackgroundObject = new SdrRectObj(aBackgroundSize);
            OSL_ENSURE(0 != mpBackgroundObject, "XLineEndList: no BackgroundObject created!" );
            mpBackgroundObject->SetModel(&rModel);
            mpBackgroundObject->SetMergedItem(XFillStyleItem(XFILL_SOLID));
            mpBackgroundObject->SetMergedItem(XLineStyleItem(XLINE_NONE));
            mpBackgroundObject->SetMergedItem(XFillColorItem(String(), rStyleSettings.GetFieldColor()));
        }

        if(!mpLineObject)
        {
            const basegfx::B2DPoint aStart(0, aSize.Height() / 2);
            const basegfx::B2DPoint aEnd(aSize.Width(), aSize.Height() / 2);
            basegfx::B2DPolygon aPolygon;
            aPolygon.append(aStart);
            aPolygon.append(aEnd);
            mpLineObject = new SdrPathObj(OBJ_LINE, basegfx::B2DPolyPolygon(aPolygon));
            OSL_ENSURE(0 != mpLineObject, "XLineEndList: no LineObject created!" );
            mpLineObject->SetModel(&rModel);
            const Size aLineWidth(rVirDev.PixelToLogic(Size(rStyleSettings.GetListBoxPreviewDefaultLineWidth(), 0)));
            mpLineObject->SetMergedItem(XLineWidthItem(aLineWidth.getWidth()));
            const sal_uInt32 nArrowHeight((aSize.Height() * 8) / 10);
            mpLineObject->SetMergedItem(XLineStartWidthItem(nArrowHeight));
            mpLineObject->SetMergedItem(XLineEndWidthItem(nArrowHeight));
            mpLineObject->SetMergedItem(XLineColorItem(String(), rStyleSettings.GetFieldTextColor()));
        }

        mpLineObject->SetMergedItem(XLineStyleItem(XLINE_SOLID));
        mpLineObject->SetMergedItem(XLineStartItem(String(), GetLineEnd(nIndex)->GetLineEnd()));
        mpLineObject->SetMergedItem(XLineEndItem(String(), GetLineEnd(nIndex)->GetLineEnd()));

        sdr::contact::SdrObjectVector aObjectVector;

        aObjectVector.push_back(mpBackgroundObject);
        aObjectVector.push_back(mpLineObject);

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
