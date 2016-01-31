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
#include <svx/compressgraphicdialog.hxx>
#include <sfx2/dispatch.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>

#include "compressgraphicdialog.hrc"

using namespace com::sun::star::uno;
using namespace com::sun::star::beans;

CompressGraphicsDialog::CompressGraphicsDialog( vcl::Window* pParent, SdrGrafObj* pGraphicObj, SfxBindings& rBindings ) :
    ModalDialog       ( pParent, "CompressGraphicDialog", "svx/ui/compressgraphicdialog.ui" ),
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

CompressGraphicsDialog::CompressGraphicsDialog( vcl::Window* pParent, Graphic& rGraphic, Size rViewSize100mm, Rectangle& rCropRectangle, SfxBindings& rBindings ) :
    ModalDialog       ( pParent, "CompressGraphicDialog", "svx/ui/compressgraphicdialog.ui" ),
    m_pGraphicObj     ( nullptr ),
    m_aGraphic        ( rGraphic ),
    m_aViewSize100mm  ( rViewSize100mm ),
    m_aCropRectangle  ( rCropRectangle ),
    m_rBindings       ( rBindings ),
    m_dResolution     ( 96.0 )
{
    Initialize();
}

CompressGraphicsDialog::~CompressGraphicsDialog()
{
    disposeOnce();
}

void CompressGraphicsDialog::dispose()
{
    m_pLabelGraphicType.clear();
    m_pFixedText2.clear();
    m_pFixedText3.clear();
    m_pFixedText5.clear();
    m_pFixedText6.clear();
    m_pReduceResolutionCB.clear();
    m_pMFNewWidth.clear();
    m_pMFNewHeight.clear();
    m_pResolutionLB.clear();
    m_pLosslessRB.clear();
    m_pJpegCompRB.clear();
    m_pCompressionMF.clear();
    m_pQualityMF.clear();
    m_pBtnCalculate.clear();
    m_pInterpolationCombo.clear();
    ModalDialog::dispose();
}

void CompressGraphicsDialog::Initialize()
{
    get(m_pLabelGraphicType,    "label-graphic-type");
    get(m_pFixedText2,          "label-original-size");
    get(m_pFixedText3,          "label-view-size");
    get(m_pFixedText5,          "label-image-capacity");
    get(m_pFixedText6,          "label-new-capacity");
    get(m_pJpegCompRB,          "radio-jpeg");
    get(m_pCompressionMF,       "spin-compression");
    get(m_pCompressionSlider,   "scale-compression");
    get(m_pLosslessRB,          "radio-lossless");
    get(m_pQualityMF,           "spin-quality");
    get(m_pQualitySlider,       "scale-quality");
    get(m_pReduceResolutionCB,  "checkbox-reduce-resolution");
    get(m_pMFNewWidth,          "spin-new-width");
    get(m_pMFNewHeight,         "spin-new-height");
    get(m_pResolutionLB,        "combo-resolution");
    get(m_pBtnCalculate,        "calculate");
    get(m_pInterpolationCombo,  "interpolation-method-combo");

    m_pInterpolationCombo->SelectEntry( "Lanczos" );

    m_pMFNewWidth->SetModifyHdl( LINK( this, CompressGraphicsDialog, NewWidthModifiedHdl ));
    m_pMFNewHeight->SetModifyHdl( LINK( this, CompressGraphicsDialog, NewHeightModifiedHdl ));

    m_pResolutionLB->SetModifyHdl( LINK( this, CompressGraphicsDialog, ResolutionModifiedHdl ));
    m_pBtnCalculate->SetClickHdl(  LINK( this, CompressGraphicsDialog, CalculateClickHdl ) );

    m_pLosslessRB->SetToggleHdl( LINK( this, CompressGraphicsDialog, ToggleCompressionRB ) );
    m_pJpegCompRB->SetToggleHdl( LINK( this, CompressGraphicsDialog, ToggleCompressionRB ) );

    m_pReduceResolutionCB->SetToggleHdl( LINK( this, CompressGraphicsDialog, ToggleReduceResolutionRB ) );

    m_pQualitySlider->SetLinkedField(m_pQualityMF);
    m_pCompressionSlider->SetLinkedField(m_pCompressionMF);

    m_pJpegCompRB->Check();
    m_pReduceResolutionCB->Check();

    UpdateNewWidthMF();
    UpdateNewHeightMF();
    UpdateResolutionLB();
    Update();
}

void CompressGraphicsDialog::Update()
{
    GfxLinkType aLinkType = m_aGraphic.GetLink().GetType();
    OUString aGraphicTypeString;
    switch(aLinkType)
    {
        case GFX_LINK_TYPE_NATIVE_GIF:
            aGraphicTypeString = SVX_RESSTR(STR_IMAGE_GIF);
            break;
        case GFX_LINK_TYPE_NATIVE_JPG:
            aGraphicTypeString = SVX_RESSTR(STR_IMAGE_JPEG);
            break;
        case GFX_LINK_TYPE_NATIVE_PNG:
            aGraphicTypeString = SVX_RESSTR(STR_IMAGE_PNG);
            break;
        case GFX_LINK_TYPE_NATIVE_TIF:
            aGraphicTypeString = SVX_RESSTR(STR_IMAGE_TIFF);
            break;
        case GFX_LINK_TYPE_NATIVE_WMF:
            aGraphicTypeString = SVX_RESSTR(STR_IMAGE_WMF);
            break;
        case GFX_LINK_TYPE_NATIVE_MET:
            aGraphicTypeString = SVX_RESSTR(STR_IMAGE_MET);
            break;
        case GFX_LINK_TYPE_NATIVE_PCT:
            aGraphicTypeString = SVX_RESSTR(STR_IMAGE_PCT);
            break;
        case GFX_LINK_TYPE_NATIVE_SVG:
            aGraphicTypeString = SVX_RESSTR(STR_IMAGE_SVG);
            break;
        case GFX_LINK_TYPE_NATIVE_BMP:
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
    Size aOriginalSize100mm( pDummyVDev->PixelToLogic( m_aGraphic.GetSizePixel(), MAP_100TH_MM ) );

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

    m_pFixedText6->SetText("??");
}

void CompressGraphicsDialog::UpdateNewWidthMF()
{
    int nPixelX = (sal_Int32)( GetViewWidthInch() * m_dResolution );
    m_pMFNewWidth->SetText( OUString::number( nPixelX ));
}

void CompressGraphicsDialog::UpdateNewHeightMF()
{
    int nPixelY = (sal_Int32)( GetViewHeightInch() * m_dResolution );
    m_pMFNewHeight->SetText( OUString::number( nPixelY ));
}

void CompressGraphicsDialog::UpdateResolutionLB()
{
    m_pResolutionLB->SetText( OUString::number( (sal_Int32) m_dResolution ) );
}

double CompressGraphicsDialog::GetViewWidthInch()
{
    return (double) MetricField::ConvertValue(m_aViewSize100mm.Width(),  2, MAP_100TH_MM, FUNIT_INCH) / 100.0;
}

double CompressGraphicsDialog::GetViewHeightInch()
{
    return (double) MetricField::ConvertValue(m_aViewSize100mm.Height(),  2, MAP_100TH_MM, FUNIT_INCH) / 100.0;
}

BmpScaleFlag CompressGraphicsDialog::GetSelectedInterpolationType()
{
    OUString aSelectionText = m_pInterpolationCombo->GetSelectEntry();

    if( aSelectionText == "Lanczos" ) {
        return BmpScaleFlag::Lanczos;
    } else if( aSelectionText == "Bilinear" ) {
        return BmpScaleFlag::BiLinear;
    } else if( aSelectionText == "Bicubic" ) {
        return BmpScaleFlag::BiCubic;
    } else if ( aSelectionText == "None" ) {
        return BmpScaleFlag::Fast;
    }
    return BmpScaleFlag::BestQuality;
}

void CompressGraphicsDialog::Compress(SvStream& aStream)
{
    BitmapEx aBitmap = m_aGraphic.GetBitmapEx();
    if ( m_pReduceResolutionCB->IsChecked() )
    {
        long nPixelX = (long)( GetViewWidthInch() * m_dResolution );
        long nPixelY = (long)( GetViewHeightInch() * m_dResolution );

        aBitmap.Scale( Size( nPixelX, nPixelY ), GetSelectedInterpolationType() );
    }
    Graphic aScaledGraphic( aBitmap );
    GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();

    Sequence< PropertyValue > aFilterData( 3 );
    aFilterData[ 0 ].Name = "Interlaced";
    aFilterData[ 0 ].Value <<= (sal_Int32) 0;
    aFilterData[ 1 ].Name = "Compression";
    aFilterData[ 1 ].Value <<= (sal_Int32) m_pCompressionMF->GetValue();
    aFilterData[ 2 ].Name = "Quality";
    aFilterData[ 2 ].Value <<= (sal_Int32) m_pQualityMF->GetValue();

    OUString aGraphicFormatName = m_pLosslessRB->IsChecked() ? OUString( "png" ) : OUString( "jpg" );

    sal_uInt16 nFilterFormat = rFilter.GetExportFormatNumberForShortName( aGraphicFormatName );
    rFilter.ExportGraphic( aScaledGraphic, "none", aStream, nFilterFormat, &aFilterData );
}

IMPL_LINK_NOARG_TYPED( CompressGraphicsDialog, NewWidthModifiedHdl, Edit&, void )
{
    m_dResolution =  m_pMFNewWidth->GetValue() / GetViewWidthInch();

    UpdateNewHeightMF();
    UpdateResolutionLB();
    Update();
}

IMPL_LINK_NOARG_TYPED( CompressGraphicsDialog, NewHeightModifiedHdl, Edit&, void )
{
    m_dResolution =  m_pMFNewHeight->GetValue() / GetViewHeightInch();

    UpdateNewWidthMF();
    UpdateResolutionLB();
    Update();
}

IMPL_LINK_NOARG_TYPED( CompressGraphicsDialog, ResolutionModifiedHdl, Edit&, void )
{
    m_dResolution = (double) m_pResolutionLB->GetText().toInt32();

    UpdateNewWidthMF();
    UpdateNewHeightMF();
    Update();
}

IMPL_LINK_NOARG_TYPED( CompressGraphicsDialog, ToggleCompressionRB, RadioButton&, void )
{
    bool choice = m_pLosslessRB->IsChecked();
    m_pCompressionMF->Enable(choice);
    m_pCompressionSlider->Enable(choice);
    m_pQualityMF->Enable(!choice);
    m_pQualitySlider->Enable(!choice);
}

IMPL_LINK_NOARG_TYPED( CompressGraphicsDialog, ToggleReduceResolutionRB, CheckBox&, void )
{
    bool choice = m_pReduceResolutionCB->IsChecked();
    m_pMFNewWidth->Enable(choice);
    m_pMFNewHeight->Enable(choice);
    m_pResolutionLB->Enable(choice);
    m_pInterpolationCombo->Enable(choice);
}

IMPL_LINK_NOARG_TYPED( CompressGraphicsDialog, CalculateClickHdl, Button*, void )
{
    sal_Int32 aSize = 0;

    if ( m_dResolution > 0.0  )
    {
        SvMemoryStream aMemStream;
        aMemStream.SetVersion( SOFFICE_FILEFORMAT_CURRENT );
        Compress( aMemStream );
        aMemStream.Seek( STREAM_SEEK_TO_END );
        aSize = aMemStream.Tell();
    }

    if ( aSize > 0 )
    {
        OUString aSizeAsString = OUString::number(aSize / 1024);

        OUString aNewSizeString = SVX_RESSTR(STR_IMAGE_CAPACITY);
        aNewSizeString = aNewSizeString.replaceAll("$(CAPACITY)", aSizeAsString);
        m_pFixedText6->SetText(aNewSizeString);
    }
}

Rectangle CompressGraphicsDialog::GetScaledCropRectangle()
{
    if ( m_pReduceResolutionCB->IsChecked() )
    {
        long nPixelX = (long)( GetViewWidthInch()  * m_dResolution );
        long nPixelY = (long)( GetViewHeightInch() * m_dResolution );
        Size aSize = m_aGraphic.GetBitmapEx().GetSizePixel();
        double aScaleX = nPixelX / (double) aSize.Width();
        double aScaleY = nPixelY / (double) aSize.Height();

        return Rectangle(
            m_aCropRectangle.Left()  * aScaleX,
            m_aCropRectangle.Top()   * aScaleY,
            m_aCropRectangle.Right() * aScaleX,
            m_aCropRectangle.Bottom()* aScaleY);
    }
    else
    {
        return m_aCropRectangle;
    }
}

Graphic CompressGraphicsDialog::GetCompressedGraphic()
{
    if ( m_dResolution > 0.0  )
    {
        SvMemoryStream aMemStream;
        aMemStream.SetVersion( SOFFICE_FILEFORMAT_CURRENT );
        Compress( aMemStream );
        aMemStream.Seek( STREAM_SEEK_TO_BEGIN );
        Graphic aResultGraphic;
        GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();
        rFilter.ImportGraphic( aResultGraphic, OUString("import"), aMemStream );

        return aResultGraphic;
    }
    return Graphic();
}

SdrGrafObj* CompressGraphicsDialog::GetCompressedSdrGrafObj()
{
    if ( m_dResolution > 0.0  )
    {
        SdrGrafObj* pNewObject = m_pGraphicObj->Clone();

        if ( m_pReduceResolutionCB->IsChecked() )
        {
            Rectangle aScaledCropedRectangle = GetScaledCropRectangle();
            SdrGrafCropItem aNewCrop(
                aScaledCropedRectangle.Left(),
                aScaledCropedRectangle.Top(),
                aScaledCropedRectangle.Right(),
                aScaledCropedRectangle.Bottom());

            pNewObject->SetMergedItem(aNewCrop);
        }
        pNewObject->SetGraphic( GetCompressedGraphic() );

        return pNewObject;
    }
    return nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
