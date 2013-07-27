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
#include <com/sun/star/container/XNameContainer.hpp>
#include "svx/XPropertyTable.hxx"
#include <unotools/ucbstreamhelper.hxx>
#include "xmlxtexp.hxx"
#include "xmlxtimp.hxx"
#include <vcl/svapp.hxx>
#include <tools/urlobj.hxx>
#include <vcl/virdev.hxx>
#include <svx/dialogs.hrc>
#include <svx/dialmgr.hxx>
#include <svx/xtable.hxx>
#include <drawinglayer/attribute/lineattribute.hxx>
#include <drawinglayer/attribute/strokeattribute.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <drawinglayer/processor2d/processor2dtools.hxx>

using namespace com::sun::star;
using namespace rtl;

#define GLOBALOVERFLOW

sal_Unicode const pszExtDash[]  = {'s','o','d'};
//char const aChckDash[]  = { 0x04, 0x00, 'S','O','D','L'}; // < 5.2
//char const aChckDash0[] = { 0x04, 0x00, 'S','O','D','0'}; // = 5.2
//char const aChckXML[]   = { '<', '?', 'x', 'm', 'l' };        // = 6.0

// ----------------
// class XDashList
// ----------------

XDashList::XDashList(const String& rPath )
:   XPropertyList(rPath),
    maBitmapSolidLine(),
    maStringSolidLine(),
    maStringNoLine()
{
}

XDashList::~XDashList()
{
}

XDashEntry* XDashList::Replace(XDashEntry* pEntry, long nIndex )
{
    return (XDashEntry*) XPropertyList::Replace(pEntry, nIndex);
}

XDashEntry* XDashList::Remove(long nIndex)
{
    return (XDashEntry*) XPropertyList::Remove(nIndex);
}

XDashEntry* XDashList::GetDash(long nIndex) const
{
    return (XDashEntry*) XPropertyList::Get(nIndex);
}

bool XDashList::Load()
{
    if( mbListDirty )
    {
        mbListDirty = false;

        INetURLObject aURL( maPath );

        if( INET_PROT_NOT_VALID == aURL.GetProtocol() )
        {
            OSL_ENSURE( !maPath.Len(), "invalid URL" );
            return false;
        }

        aURL.Append( maName );

        if( !aURL.getExtension().getLength() )
            aURL.setExtension( rtl::OUString( pszExtDash, 3 ) );

        uno::Reference< container::XNameContainer > xTable( SvxUnoXDashTable_createInstance( this ), uno::UNO_QUERY );
        return SvxXMLXTableImport::load( aURL.GetMainURL( INetURLObject::NO_DECODE ), xTable );
    }

    return false;
}

bool XDashList::Save()
{
    INetURLObject aURL( maPath );

    if( INET_PROT_NOT_VALID == aURL.GetProtocol() )
    {
        OSL_ENSURE( !maPath.Len(), "invalid URL" );
        return false;
    }

    aURL.Append( maName );

    if( !aURL.getExtension().getLength() )
        aURL.setExtension( rtl::OUString( pszExtDash, 3 ) );

    uno::Reference< container::XNameContainer > xTable( SvxUnoXDashTable_createInstance( this ), uno::UNO_QUERY );
    return SvxXMLXTableExportComponent::save( aURL.GetMainURL( INetURLObject::NO_DECODE ), xTable );
}

bool XDashList::Create()
{
    XubString aStr( SVX_RES( RID_SVXSTR_LINESTYLE ) );
    xub_StrLen nLen;

    aStr.AppendAscii(" 1");
    nLen = aStr.Len() - 1;
    Insert(new XDashEntry(XDash(XDASH_RECT,1, 50,1, 50, 50),aStr));
    aStr.SetChar(nLen, sal_Unicode('2'));
    Insert(new XDashEntry(XDash(XDASH_RECT,1,500,1,500,500),aStr));
    aStr.SetChar(nLen, sal_Unicode('3'));
    Insert(new XDashEntry(XDash(XDASH_RECT,2, 50,3,250,120),aStr));

    return true;
}

Bitmap XDashList::ImpCreateBitmapForXDash(const XDash* pDash)
{
    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    const Size& rSize = rStyleSettings.GetListBoxPreviewDefaultPixelSize();
    const sal_uInt32 nFactor(2);
    const Size aSize((rSize.Width() * 5 * 2) / 2, rSize.Height() * nFactor);

    // prepare polygon geometry for line
    basegfx::B2DPolygon aLine;

    aLine.append(basegfx::B2DPoint(0.0, aSize.Height() / 2.0));
    aLine.append(basegfx::B2DPoint(aSize.Width(), aSize.Height() / 2.0));

    // prepare LineAttribute
    const basegfx::BColor aLineColor(rStyleSettings.GetFieldTextColor().getBColor());
    const double fLineWidth(rStyleSettings.GetListBoxPreviewDefaultLineWidth() * (nFactor * 1.1));
    const drawinglayer::attribute::LineAttribute aLineAttribute(
        aLineColor,
        fLineWidth);

    // prepare StrokeAttribute
    ::std::vector< double > aDotDashArray;
    double fFullDotDashLen(0.0);

    if(pDash && (pDash->GetDots() || pDash->GetDashes()))
    {
        const basegfx::B2DHomMatrix aScaleMatrix(OutputDevice::LogicToLogic(MAP_100TH_MM, MAP_PIXEL));
        const basegfx::B2DVector aScaleVector(aScaleMatrix * basegfx::B2DVector(1.0, 0.0));
        const double fScaleValue(aScaleVector.getLength() * (nFactor * (1.4 / 2.0)));
        const double fLineWidthInUnits(fLineWidth / fScaleValue);

        fFullDotDashLen = pDash->CreateDotDashArray(aDotDashArray, fLineWidthInUnits);

        if(!aDotDashArray.empty())
        {
            for(sal_uInt32 a(0); a < aDotDashArray.size(); a++)
            {
                aDotDashArray[a] *= fScaleValue;
            }

            fFullDotDashLen *= fScaleValue;
        }
    }

    const drawinglayer::attribute::StrokeAttribute aStrokeAttribute(
        aDotDashArray,
        fFullDotDashLen);

    // cerate LinePrimitive
    const drawinglayer::primitive2d::Primitive2DReference aLinePrimitive(
        new drawinglayer::primitive2d::PolygonStrokePrimitive2D(
            aLine,
            aLineAttribute,
            aStrokeAttribute));

    // prepare VirtualDevice
    VirtualDevice aVirtualDevice;
    const drawinglayer::geometry::ViewInformation2D aNewViewInformation2D;

    aVirtualDevice.SetOutputSizePixel(aSize);
    aVirtualDevice.SetDrawMode(rStyleSettings.GetHighContrastMode()
        ? DRAWMODE_SETTINGSLINE | DRAWMODE_SETTINGSFILL | DRAWMODE_SETTINGSTEXT | DRAWMODE_SETTINGSGRADIENT
        : DRAWMODE_DEFAULT);

    if(rStyleSettings.GetPreviewUsesCheckeredBackground())
    {
        const Point aNull(0, 0);
        static const sal_uInt32 nLen(8 * nFactor);
        static const Color aW(COL_WHITE);
        static const Color aG(0xef, 0xef, 0xef);

        aVirtualDevice.DrawCheckered(aNull, aSize, nLen, aW, aG);
    }
    else
    {
        aVirtualDevice.SetBackground(rStyleSettings.GetFieldColor());
        aVirtualDevice.Erase();
    }

    // create processor and draw primitives
    drawinglayer::processor2d::BaseProcessor2D* pProcessor2D = drawinglayer::processor2d::createPixelProcessor2DFromOutputDevice(
        aVirtualDevice,
        aNewViewInformation2D);

    if(pProcessor2D)
    {
        const drawinglayer::primitive2d::Primitive2DSequence aSequence(&aLinePrimitive, 1);

        pProcessor2D->process(aSequence);
        delete pProcessor2D;
    }

    // get result bitmap and scale
    Bitmap aRetval(aVirtualDevice.GetBitmap(Point(0, 0), aVirtualDevice.GetOutputSizePixel()));

    if(1 != nFactor)
    {
        aRetval.Scale(Size((rSize.Width() * 5) / 2, rSize.Height()), BMP_SCALE_FASTESTINTERPOLATE);
    }

    return aRetval;
}

Bitmap XDashList::CreateBitmapForUI( long nIndex )
{
    Bitmap aRetval;
    OSL_ENSURE(nIndex < Count(), "OOps, global values missing (!)");

    if(nIndex < Count())
    {
        const XDash& rDash = GetDash(nIndex)->GetDash();

        aRetval = ImpCreateBitmapForXDash(&rDash);
    }

    return aRetval;
}

Bitmap XDashList::GetBitmapForUISolidLine() const
{
    if(maBitmapSolidLine.IsEmpty())
    {
        const_cast< XDashList* >(this)->maBitmapSolidLine = const_cast< XDashList* >(this)->ImpCreateBitmapForXDash(0);
    }

    return maBitmapSolidLine;
}

String XDashList::GetStringForUiSolidLine() const
{
    if(!maStringSolidLine.Len())
    {
        const_cast< XDashList* >(this)->maStringSolidLine = String(ResId(RID_SVXSTR_SOLID, DIALOG_MGR()));
    }

    return maStringSolidLine;
}

String XDashList::GetStringForUiNoLine() const
{
    if(!maStringNoLine.Len())
    {
        // formally was RID_SVXSTR_INVISIBLE, but tomake equal
        // everywhere, use RID_SVXSTR_NONE
        const_cast< XDashList* >(this)->maStringNoLine = String(ResId(RID_SVXSTR_NONE, DIALOG_MGR()));
    }

    return maStringNoLine;
}

//////////////////////////////////////////////////////////////////////////////
// eof
