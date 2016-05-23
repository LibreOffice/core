/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <dlgunit.hxx>
#include <vcl/graph.hxx>
#include <vcl/graphicfilter.hxx>
#include <vcl/virdev.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <svx/svdograf.hxx>
#include <svx/sdgcpitm.hxx>
#include <svx/dialmgr.hxx>
#include <svx/graphicinfodialog.hxx>
#include <sfx2/dispatch.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>

#include "compressgraphicdialog.hrc"

using namespace com::sun::star::uno;
using namespace com::sun::star::beans;


GraphicInfoDialog::GraphicInfoDialog( vcl::Window* pParent, SdrGrafObj* pGraphicObj, SfxBindings& rBindings ) :
    ModalDialog       ( pParent, "GraphicInfoDialog", "svx/ui/graphicinfodialog.ui" ),
    m_pGraphicObj     ( pGraphicObj ),
    m_aGraphic        ( pGraphicObj->GetGraphicObject().GetGraphic() ),
    m_aViewSize100mm  ( pGraphicObj->GetLogicRect().GetSize() ),
    m_rBindings       ( rBindings ),
    m_dResolution     ( 96.0 )
{
    const SdrGrafCropItem& rCrop = static_cast<const SdrGrafCropItem&>( m_pGraphicObj->GetMergedItem(SDRATTR_GRAFCROP) );
    m_aCropRectangle = Rectangle(rCrop.GetLeft(), rCrop.GetTop(), rCrop.GetRight(), rCrop.GetBottom());

    Initialize();
}

GraphicInfoDialog::GraphicInfoDialog( vcl::Window* pParent, Graphic& rGraphic, Size rViewSize100mm, Rectangle& rCropRectangle, SfxBindings& rBindings ) :
    ModalDialog       ( pParent, "GraphicInfoDialog", "svx/ui/graphicinfodialog.ui" ),
    m_pGraphicObj     ( nullptr ),
    m_aGraphic        ( rGraphic ),
    m_aViewSize100mm  ( rViewSize100mm ),
    m_aCropRectangle  ( rCropRectangle ),
    m_rBindings       ( rBindings ),
    m_dResolution     ( 96.0 )
{
    Initialize();
}

GraphicInfoDialog::~GraphicInfoDialog()
{
    disposeOnce();
}

void GraphicInfoDialog::dispose()
{
    m_pLabelGraphicType.clear();
    m_pFixedText2.clear();
    m_pFixedText3.clear();
    m_pFixedText5.clear();
    ModalDialog::dispose();
}

void GraphicInfoDialog::Initialize()
{
    get(m_pLabelGraphicType,    "label-graphic-type");
    get(m_pFixedText2,          "label-original-size");
    get(m_pFixedText3,          "label-view-size");
    get(m_pFixedText5,          "label-image-capacity");
    Update();
}

void GraphicInfoDialog::Update()
{
    GfxLinkType aLinkType = m_aGraphic.GetLink().GetType();
    OUString aGraphicTypeString;
    switch(aLinkType)
    {
        case GfxLinkType::NativeGif:
            aGraphicTypeString = SVX_RESSTR(STR_IMAGE_GIF);
            break;
        case GfxLinkType::NativeJpg:
            aGraphicTypeString = SVX_RESSTR(STR_IMAGE_JPEG);
            break;
        case GfxLinkType::NativePng:
            aGraphicTypeString = SVX_RESSTR(STR_IMAGE_PNG);
            break;
        case GfxLinkType::NativeTif:
            aGraphicTypeString = SVX_RESSTR(STR_IMAGE_TIFF);
            break;
        case GfxLinkType::NativeWmf:
            aGraphicTypeString = SVX_RESSTR(STR_IMAGE_WMF);
            break;
        case GfxLinkType::NativeMet:
            aGraphicTypeString = SVX_RESSTR(STR_IMAGE_MET);
            break;
        case GfxLinkType::NativePct:
            aGraphicTypeString = SVX_RESSTR(STR_IMAGE_PCT);
            break;
        case GfxLinkType::NativeSvg:
            aGraphicTypeString = SVX_RESSTR(STR_IMAGE_SVG);
            break;
        case GfxLinkType::NativeBmp:
            aGraphicTypeString = SVX_RESSTR(STR_IMAGE_BMP);
            break;
        default:
            aGraphicTypeString = SVX_RESSTR(STR_IMAGE_UNKNOWN);
            break;
    }
    m_pLabelGraphicType->SetText(aGraphicTypeString);

    const FieldUnit eFieldUnit = m_rBindings.GetDispatcher()->GetModule()->GetFieldUnit();
    const LocaleDataWrapper& rLocaleWrapper( Application::GetSettings().GetLocaleDataWrapper() );
    sal_Unicode cSeparator = rLocaleWrapper.getNumDecimalSep()[0];

    VclPtr<VirtualDevice> pDummyVDev = VclPtr<VirtualDevice>::Create();
    pDummyVDev->EnableOutput( false );
    pDummyVDev->SetMapMode( m_aGraphic.GetPrefMapMode() );

    Size aPixelSize = m_aGraphic.GetSizePixel();
    Size aOriginalSize100mm( pDummyVDev->PixelToLogic( aPixelSize, MAP_100TH_MM ) );

    OUString aBitmapSizeString = SVX_RESSTR(STR_IMAGE_ORIGINAL_SIZE);
    OUString aWidthString  = GetUnitString( aOriginalSize100mm.Width(),  eFieldUnit, cSeparator );
    OUString aHeightString = GetUnitString( aOriginalSize100mm.Height(), eFieldUnit, cSeparator );
    aBitmapSizeString = aBitmapSizeString.replaceAll("$(WIDTH)",  aWidthString);
    aBitmapSizeString = aBitmapSizeString.replaceAll("$(HEIGHT)", aHeightString);
    aBitmapSizeString = aBitmapSizeString.replaceAll("$(WIDTH_IN_PX)",  OUString::number(aPixelSize.Width()));
    aBitmapSizeString = aBitmapSizeString.replaceAll("$(HEIGHT_IN_PX)", OUString::number(aPixelSize.Height()));
    m_pFixedText2->SetText(aBitmapSizeString);

    int aValX = (int) (aPixelSize.Width() / GetViewWidthInch());

    OUString aViewSizeString = SVX_RESSTR(STR_IMAGE_VIEW_SIZE);

    aWidthString  = GetUnitString( m_aViewSize100mm.Width(),  eFieldUnit, cSeparator );
    aHeightString = GetUnitString( m_aViewSize100mm.Height(), eFieldUnit, cSeparator );
    aViewSizeString = aViewSizeString.replaceAll("$(WIDTH)",  aWidthString);
    aViewSizeString = aViewSizeString.replaceAll("$(HEIGHT)", aHeightString);
    aViewSizeString = aViewSizeString.replaceAll("$(DPI)", OUString::number(aValX));
    m_pFixedText3->SetText(aViewSizeString);

    SvMemoryStream aMemStream;
    aMemStream.SetVersion( SOFFICE_FILEFORMAT_CURRENT );
    m_aGraphic.ExportNative(aMemStream);
    aMemStream.Seek( STREAM_SEEK_TO_END );
    sal_Int32 aNativeSize = aMemStream.Tell();

    OUString aNativeSizeString = SVX_RESSTR(STR_IMAGE_CAPACITY);
    aNativeSizeString = aNativeSizeString.replaceAll("$(CAPACITY)",  OUString::number(aNativeSize / 1024));
    m_pFixedText5->SetText(aNativeSizeString);

}


double GraphicInfoDialog::GetViewWidthInch()
{
    return (double) MetricField::ConvertValue(m_aViewSize100mm.Width(),  2, MAP_100TH_MM, FUNIT_INCH) / 100.0;
}

double GraphicInfoDialog::GetViewHeightInch()
{
    return (double) MetricField::ConvertValue(m_aViewSize100mm.Height(),  2, MAP_100TH_MM, FUNIT_INCH) / 100.0;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */