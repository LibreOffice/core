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

#include "dlgunit.hxx"
#include <vcl/fieldvalues.hxx>
#include <vcl/graph.hxx>
#include <vcl/graphicfilter.hxx>
#include <vcl/virdev.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <vcl/weld.hxx>
#include <svx/strings.hrc>
#include <svx/svdograf.hxx>
#include <svx/sdgcpitm.hxx>
#include <svx/dialmgr.hxx>
#include <svx/graphichelper.hxx>
#include <svx/compressgraphicdialog.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/module.hxx>
#include <comphelper/fileformat.h>
#include <com/sun/star/uno/Sequence.hxx>
#include <tools/stream.hxx>
#include <unotools/localedatawrapper.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::beans;

CompressGraphicsDialog::CompressGraphicsDialog( weld::Window* pParent, SdrGrafObj* pGraphicObj, SfxBindings& rBindings ) :
    GenericDialogController( pParent, "svx/ui/compressgraphicdialog.ui", "CompressGraphicDialog" ),
    m_xGraphicObj     ( pGraphicObj ),
    m_aGraphic        ( pGraphicObj->GetGraphicObject().GetGraphic() ),
    m_aViewSize100mm  ( pGraphicObj->GetLogicRect().GetSize() ),
    m_rBindings       ( rBindings ),
    m_dResolution     ( 300 )
{
    const SdrGrafCropItem& rCrop = m_xGraphicObj->GetMergedItem(SDRATTR_GRAFCROP);
    m_aCropRectangle = tools::Rectangle(rCrop.GetLeft(), rCrop.GetTop(), rCrop.GetRight(), rCrop.GetBottom());

    Initialize();
}

CompressGraphicsDialog::CompressGraphicsDialog( weld::Window* pParent, Graphic const & rGraphic, Size rViewSize100mm, tools::Rectangle const & rCropRectangle, SfxBindings& rBindings ) :
    GenericDialogController( pParent, "svx/ui/compressgraphicdialog.ui", "CompressGraphicDialog" ),
    m_xGraphicObj     ( nullptr ),
    m_aGraphic        ( rGraphic ),
    m_aViewSize100mm  ( rViewSize100mm ),
    m_aCropRectangle  ( rCropRectangle ),
    m_rBindings       ( rBindings ),
    m_dResolution     ( 300 )
{
    Initialize();
}

CompressGraphicsDialog::~CompressGraphicsDialog()
{
}

void CompressGraphicsDialog::Initialize()
{
    m_xLabelGraphicType = m_xBuilder->weld_label("label-graphic-type");
    m_xFixedText2 = m_xBuilder->weld_label("label-original-size");
    m_xFixedText3 = m_xBuilder->weld_label("label-view-size");
    m_xFixedText5 = m_xBuilder->weld_label("label-image-capacity");
    m_xFixedText6 = m_xBuilder->weld_label("label-new-capacity");
    m_xJpegCompRB = m_xBuilder->weld_radio_button("radio-jpeg");
    m_xCompressionMF = m_xBuilder->weld_spin_button("spin-compression");
    m_xCompressionSlider = m_xBuilder->weld_scale("scale-compression");
    m_xLosslessRB = m_xBuilder->weld_radio_button("radio-lossless");
    m_xQualityMF = m_xBuilder->weld_spin_button("spin-quality");
    m_xQualitySlider = m_xBuilder->weld_scale("scale-quality");
    m_xReduceResolutionCB = m_xBuilder->weld_check_button("checkbox-reduce-resolution");
    m_xMFNewWidth = m_xBuilder->weld_spin_button("spin-new-width");
    m_xMFNewHeight = m_xBuilder->weld_spin_button("spin-new-height");
    m_xResolutionLB = m_xBuilder->weld_combo_box("combo-resolution");
    m_xBtnCalculate = m_xBuilder->weld_button("calculate");
    m_xInterpolationCombo = m_xBuilder->weld_combo_box("interpolation-method-combo");

    m_xInterpolationCombo->set_active_text("Lanczos");

    m_xInterpolationCombo->connect_changed(LINK(this, CompressGraphicsDialog, NewInterpolationModifiedHdl));

    m_xMFNewWidth->connect_changed( LINK( this, CompressGraphicsDialog, NewWidthModifiedHdl ));
    m_xMFNewHeight->connect_changed( LINK( this, CompressGraphicsDialog, NewHeightModifiedHdl ));

    m_xResolutionLB->connect_changed( LINK( this, CompressGraphicsDialog, ResolutionModifiedHdl ));
    m_xBtnCalculate->connect_clicked(  LINK( this, CompressGraphicsDialog, CalculateClickHdl ) );

    m_xLosslessRB->connect_toggled( LINK( this, CompressGraphicsDialog, ToggleCompressionRB ) );
    m_xJpegCompRB->connect_toggled( LINK( this, CompressGraphicsDialog, ToggleCompressionRB ) );

    m_xReduceResolutionCB->connect_toggled( LINK( this, CompressGraphicsDialog, ToggleReduceResolutionRB ) );

    m_xQualitySlider->connect_value_changed( LINK( this, CompressGraphicsDialog, SlideHdl ));
    m_xCompressionSlider->connect_value_changed( LINK( this, CompressGraphicsDialog, SlideHdl ));
    m_xQualityMF->connect_changed( LINK( this, CompressGraphicsDialog, NewQualityModifiedHdl ));
    m_xCompressionMF->connect_changed( LINK( this, CompressGraphicsDialog, NewCompressionModifiedHdl ));

    m_xJpegCompRB->set_active(true);
    m_xReduceResolutionCB->set_active(true);

    UpdateNewWidthMF();
    UpdateNewHeightMF();
    UpdateResolutionLB();
    Update();
}

void CompressGraphicsDialog::Update()
{
    auto pGfxLink = m_aGraphic.GetSharedGfxLink();

    m_xLabelGraphicType->set_label(GraphicHelper::GetImageType(m_aGraphic));

    const FieldUnit eFieldUnit = m_rBindings.GetDispatcher()->GetModule()->GetFieldUnit();
    const LocaleDataWrapper& rLocaleWrapper( Application::GetSettings().GetLocaleDataWrapper() );
    sal_Unicode cSeparator = rLocaleWrapper.getNumDecimalSep()[0];

    ScopedVclPtrInstance<VirtualDevice> pDummyVDev;
    pDummyVDev->EnableOutput( false );
    pDummyVDev->SetMapMode( m_aGraphic.GetPrefMapMode() );

    Size aPixelSize = m_aGraphic.GetSizePixel();
    Size aOriginalSize100mm(pDummyVDev->PixelToLogic(m_aGraphic.GetSizePixel(), MapMode(MapUnit::Map100thMM)));

    OUString aBitmapSizeString = SvxResId(STR_IMAGE_ORIGINAL_SIZE);
    OUString aWidthString  = GetUnitString( aOriginalSize100mm.Width(),  eFieldUnit, cSeparator );
    OUString aHeightString = GetUnitString( aOriginalSize100mm.Height(), eFieldUnit, cSeparator );
    aBitmapSizeString = aBitmapSizeString.replaceAll("$(WIDTH)",  aWidthString);
    aBitmapSizeString = aBitmapSizeString.replaceAll("$(HEIGHT)", aHeightString);
    aBitmapSizeString = aBitmapSizeString.replaceAll("$(WIDTH_IN_PX)",  OUString::number(aPixelSize.Width()));
    aBitmapSizeString = aBitmapSizeString.replaceAll("$(HEIGHT_IN_PX)", OUString::number(aPixelSize.Height()));
    m_xFixedText2->set_label(aBitmapSizeString);

    int aValX = static_cast<int>(aPixelSize.Width() / GetViewWidthInch());

    OUString aViewSizeString = SvxResId(STR_IMAGE_VIEW_SIZE);

    aWidthString  = GetUnitString( m_aViewSize100mm.Width(),  eFieldUnit, cSeparator );
    aHeightString = GetUnitString( m_aViewSize100mm.Height(), eFieldUnit, cSeparator );
    aViewSizeString = aViewSizeString.replaceAll("$(WIDTH)",  aWidthString);
    aViewSizeString = aViewSizeString.replaceAll("$(HEIGHT)", aHeightString);
    aViewSizeString = aViewSizeString.replaceAll("$(DPI)", OUString::number(aValX));
    m_xFixedText3->set_label(aViewSizeString);

    m_aNativeSize = pGfxLink ? pGfxLink->GetDataSize() : 0;

    OUString aNativeSizeString = SvxResId(STR_IMAGE_CAPACITY);
    aNativeSizeString = aNativeSizeString.replaceAll("$(CAPACITY)",  OUString::number( m_aNativeSize  / 1024 ));
    m_xFixedText5->set_label(aNativeSizeString);

    m_xFixedText6->set_label("??");
}

void CompressGraphicsDialog::UpdateNewWidthMF()
{
    int nPixelX = static_cast<sal_Int32>( GetViewWidthInch() * m_dResolution );
    m_xMFNewWidth->set_value(nPixelX);
}

void CompressGraphicsDialog::UpdateNewHeightMF()
{
    int nPixelY = static_cast<sal_Int32>( GetViewHeightInch() * m_dResolution );
    m_xMFNewHeight->set_value(nPixelY);
}

void CompressGraphicsDialog::UpdateResolutionLB()
{
    m_xResolutionLB->set_entry_text( OUString::number( static_cast<sal_Int32>(m_dResolution) ) );
}

double CompressGraphicsDialog::GetViewWidthInch() const
{
    return static_cast<double>(vcl::ConvertValue(m_aViewSize100mm.Width(),  2, MapUnit::Map100thMM, FieldUnit::INCH)) / 100.0;
}

double CompressGraphicsDialog::GetViewHeightInch() const
{
    return static_cast<double>(vcl::ConvertValue(m_aViewSize100mm.Height(),  2, MapUnit::Map100thMM, FieldUnit::INCH)) / 100.0;
}

BmpScaleFlag CompressGraphicsDialog::GetSelectedInterpolationType() const
{
    OUString aSelectionText = m_xInterpolationCombo->get_active_text();

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
    if ( m_xReduceResolutionCB->get_active() )
    {
        tools::Long nPixelX = static_cast<tools::Long>( GetViewWidthInch() * m_dResolution );
        tools::Long nPixelY = static_cast<tools::Long>( GetViewHeightInch() * m_dResolution );

        aBitmap.Scale( Size( nPixelX, nPixelY ), GetSelectedInterpolationType() );
    }
    Graphic aScaledGraphic( aBitmap );
    GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();

    Sequence< PropertyValue > aFilterData( 3 );
    aFilterData[ 0 ].Name = "Interlaced";
    aFilterData[ 0 ].Value <<= sal_Int32(0);
    aFilterData[ 1 ].Name = "Compression";
    aFilterData[ 1 ].Value <<= static_cast<sal_Int32>(m_xCompressionMF->get_value());
    aFilterData[ 2 ].Name = "Quality";
    aFilterData[ 2 ].Value <<= static_cast<sal_Int32>(m_xQualityMF->get_value());

    OUString aGraphicFormatName = m_xLosslessRB->get_active() ? OUString( "png" ) : OUString( "jpg" );

    sal_uInt16 nFilterFormat = rFilter.GetExportFormatNumberForShortName( aGraphicFormatName );
    rFilter.ExportGraphic( aScaledGraphic, "none", aStream, nFilterFormat, &aFilterData );
}

IMPL_LINK_NOARG( CompressGraphicsDialog, NewWidthModifiedHdl, weld::Entry&, void )
{
    m_dResolution =  m_xMFNewWidth->get_value() / GetViewWidthInch();

    UpdateNewHeightMF();
    UpdateResolutionLB();
    Update();
}

IMPL_LINK( CompressGraphicsDialog, SlideHdl, weld::Scale&, rScale, void )
{
    if (&rScale == m_xQualitySlider.get())
        m_xQualityMF->set_value(m_xQualitySlider->get_value());
    else
        m_xCompressionMF->set_value(m_xCompressionSlider->get_value());
    Update();
}

IMPL_LINK_NOARG( CompressGraphicsDialog, NewInterpolationModifiedHdl, weld::ComboBox&, void )
{
    Update();
}

IMPL_LINK_NOARG( CompressGraphicsDialog, NewQualityModifiedHdl, weld::Entry&, void )
{
    m_xQualitySlider->set_value(m_xQualityMF->get_value());
    Update();
}

IMPL_LINK_NOARG( CompressGraphicsDialog, NewCompressionModifiedHdl, weld::Entry&, void )
{
    m_xCompressionSlider->set_value(m_xCompressionMF->get_value());
    Update();
}

IMPL_LINK_NOARG( CompressGraphicsDialog, NewHeightModifiedHdl, weld::Entry&, void )
{
    m_dResolution =  m_xMFNewHeight->get_value() / GetViewHeightInch();

    UpdateNewWidthMF();
    UpdateResolutionLB();
    Update();
}

IMPL_LINK_NOARG( CompressGraphicsDialog, ResolutionModifiedHdl, weld::ComboBox&, void )
{
    m_dResolution = static_cast<double>(m_xResolutionLB->get_active_text().toInt32());

    UpdateNewWidthMF();
    UpdateNewHeightMF();
    Update();
}

IMPL_LINK_NOARG( CompressGraphicsDialog, ToggleCompressionRB, weld::ToggleButton&, void )
{
    bool choice = m_xLosslessRB->get_active();
    m_xCompressionMF->set_sensitive(choice);
    m_xCompressionSlider->set_sensitive(choice);
    m_xQualityMF->set_sensitive(!choice);
    m_xQualitySlider->set_sensitive(!choice);
    Update();
}

IMPL_LINK_NOARG( CompressGraphicsDialog, ToggleReduceResolutionRB, weld::ToggleButton&, void )
{
    bool choice = m_xReduceResolutionCB->get_active();
    m_xMFNewWidth->set_sensitive(choice);
    m_xMFNewHeight->set_sensitive(choice);
    m_xResolutionLB->set_sensitive(choice);
    m_xInterpolationCombo->set_sensitive(choice);
    Update();
}

IMPL_LINK_NOARG( CompressGraphicsDialog, CalculateClickHdl, weld::Button&, void )
{
    sal_Int32 aSize = 0;

    if ( m_dResolution > 0.0  )
    {
        SvMemoryStream aMemStream;
        aMemStream.SetVersion( SOFFICE_FILEFORMAT_CURRENT );
        Compress( aMemStream );
        aSize = aMemStream.TellEnd();
    }

    if ( aSize > 0 )
    {
        OUString aSizeAsString = OUString::number(aSize / 1024);

        OUString aReductionSizeAsString = OUString::number( m_aNativeSize > 0 ? (m_aNativeSize - aSize) * 100 / m_aNativeSize : 0 );

        OUString aNewSizeString = SvxResId(STR_IMAGE_CAPACITY_WITH_REDUCTION);
        aNewSizeString = aNewSizeString.replaceAll("$(CAPACITY)", aSizeAsString);
        aNewSizeString = aNewSizeString.replaceAll("$(REDUCTION)", aReductionSizeAsString);
        m_xFixedText6->set_label(aNewSizeString);
    }
}

tools::Rectangle CompressGraphicsDialog::GetScaledCropRectangle() const
{
    if ( m_xReduceResolutionCB->get_active() )
    {
        tools::Long nPixelX = static_cast<tools::Long>( GetViewWidthInch()  * m_dResolution );
        tools::Long nPixelY = static_cast<tools::Long>( GetViewHeightInch() * m_dResolution );
        Size aSize = m_aGraphic.GetBitmapEx().GetSizePixel();
        double aScaleX = nPixelX / static_cast<double>(aSize.Width());
        double aScaleY = nPixelY / static_cast<double>(aSize.Height());

        return tools::Rectangle(
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
        SdrGrafObj* pNewObject(m_xGraphicObj->CloneSdrObject(m_xGraphicObj->getSdrModelFromSdrObject()));

        if ( m_xReduceResolutionCB->get_active() )
        {
            tools::Rectangle aScaledCropedRectangle = GetScaledCropRectangle();
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
