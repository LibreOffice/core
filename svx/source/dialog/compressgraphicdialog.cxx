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
#include <vcl/virdev.hxx>
#include <vcl/svapp.hxx>
#include <svx/svdograf.hxx>
#include <svx/sdgcpitm.hxx>
#include <svx/dialmgr.hxx>
#include <svx/compressgraphicdialog.hxx>
#include <svtools/filter.hxx>
#include <sfx2/dispatch.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>

using namespace com::sun::star::uno;
using namespace com::sun::star::beans;

#include <stdio.h>

CompressGraphicsDialog::CompressGraphicsDialog( Window* pParent, SdrGrafObj* pGraphicObj, SfxBindings& rBindings ) :
    ModalDialog       ( pParent, "CompressGraphicDialog", "svx/ui/compressgraphicdialog.ui" ),
    m_pGraphicObj     ( pGraphicObj ),
    m_aGraphic        ( pGraphicObj->GetGraphicObject().GetGraphic() ),
    m_aViewSize100mm  ( pGraphicObj->GetLogicRect().GetSize() ),
    m_aCropRectangle  ( Rectangle() ),
    m_rBindings       ( rBindings ),
    m_dResolution     ( 96.0 )
{
    Initialize();
}

CompressGraphicsDialog::CompressGraphicsDialog( Window* pParent, const Graphic& rGraphic, Size& rViewSize100mm, Rectangle& rCropRectangle, SfxBindings& rBindings ) :
    ModalDialog       ( pParent, "CompressGraphicDialog", "svx/ui/compressgraphicdialog.ui" ),
    m_pGraphicObj     ( NULL ),
    m_aGraphic        ( rGraphic ),
    m_aViewSize100mm  ( rViewSize100mm ),
    m_aCropRectangle  ( rCropRectangle ),
    m_rBindings       ( rBindings ),
    m_dResolution     ( 96.0 )
{
    Initialize();
}


void CompressGraphicsDialog::Initialize()
{
    get(m_aFixedText2,          "label-original-size");
    get(m_aFixedText3,          "label-view-size");
    get(m_aFixedText5,          "label-image-capacity");
    get(m_aFixedText6,          "label-new-capacity");
    get(m_aJpegCompRB,          "radio-jpeg");
    get(m_aCompressionMF,       "spin-compression");
    get(m_aLosslessRB,          "radio-lossless");
    get(m_aQualityMF,           "spin-quality");
    get(m_aReduceResolutionCB,  "checkbox-reduce-resolution");
    get(m_aMFNewWidth,          "spin-new-width");
    get(m_aMFNewHeight,         "spin-new-height");
    get(m_aResolutionLB,        "combo-resolution");
    get(m_aBtnCalculate,        "calculate");
    get(m_aInterpolationCombo,  "interpolation-method-combo");

    m_aResolutionLB->InsertEntry(OUString("50"));
    m_aResolutionLB->InsertEntry(OUString("75"));
    m_aResolutionLB->InsertEntry(OUString("96"));
    m_aResolutionLB->InsertEntry(OUString("150"));
    m_aResolutionLB->InsertEntry(OUString("200"));
    m_aResolutionLB->InsertEntry(OUString("300"));
    m_aResolutionLB->InsertEntry(OUString("600"));

    m_aInterpolationCombo->SelectEntry( OUString("Lanczos") );

    m_aMFNewWidth->SetModifyHdl( LINK( this, CompressGraphicsDialog, NewWidthModifiedHdl ));
    m_aMFNewHeight->SetModifyHdl( LINK( this, CompressGraphicsDialog, NewHeightModifiedHdl ));

    m_aResolutionLB->SetModifyHdl( LINK( this, CompressGraphicsDialog, ResolutionModifiedHdl ));
    m_aBtnCalculate->SetClickHdl(  LINK( this, CompressGraphicsDialog, CalculateClickHdl ) );

    m_aLosslessRB->SetToggleHdl( LINK( this, CompressGraphicsDialog, ToggleCompressionRB ) );
    m_aJpegCompRB->SetToggleHdl( LINK( this, CompressGraphicsDialog, ToggleCompressionRB ) );

    m_aReduceResolutionCB->SetToggleHdl( LINK( this, CompressGraphicsDialog, ToggleReduceResolutionRB ) );

    m_aJpegCompRB->Check();
    m_aReduceResolutionCB->Check();

    UpdateNewWidthMF();
    UpdateNewHeightMF();
    UpdateResolutionLB();
    Update();
}

CompressGraphicsDialog::~CompressGraphicsDialog()
{
}

void CompressGraphicsDialog::Update()
{
    const FieldUnit eFieldUnit = m_rBindings.GetDispatcher()->GetModule()->GetFieldUnit();
    const LocaleDataWrapper& rLocaleWrapper( Application::GetSettings().GetLocaleDataWrapper() );
    sal_Unicode cSeparator = rLocaleWrapper.getNumDecimalSep()[0];

    VirtualDevice* pDummyVDev = new VirtualDevice();
    pDummyVDev->EnableOutput( false );
    pDummyVDev->SetMapMode( m_aGraphic.GetPrefMapMode() );

    Size aPixelSize = m_aGraphic.GetSizePixel();
    Size aOriginalSize100mm( pDummyVDev->PixelToLogic( m_aGraphic.GetSizePixel(), MAP_100TH_MM ) );

    String aBitmapSizeString;
    aBitmapSizeString += GetUnitString( aOriginalSize100mm.Width(), eFieldUnit, cSeparator );
    aBitmapSizeString += String( " x " ) ;
    aBitmapSizeString += GetUnitString( aOriginalSize100mm.Height(), eFieldUnit, cSeparator );
    aBitmapSizeString += String( " ( " ) ;
    aBitmapSizeString += UniString::CreateFromInt32(aPixelSize.Width());
    aBitmapSizeString += String( " x " ) ;
    aBitmapSizeString += UniString::CreateFromInt32(aPixelSize.Height());
    aBitmapSizeString += String( " px )" ) ;
    m_aFixedText2->SetText(aBitmapSizeString);

    String aViewSizeString;

    int aValX = (int) (aPixelSize.Width() / GetViewWidthInch());

    aViewSizeString += GetUnitString( m_aViewSize100mm.Width(), eFieldUnit, cSeparator );
    aViewSizeString += String( " x " ) ;
    aViewSizeString += GetUnitString( m_aViewSize100mm.Height(), eFieldUnit, cSeparator );
    aViewSizeString += ( " at " ) ;
    aViewSizeString += UniString::CreateFromInt32(aValX);
    aViewSizeString += ( " DPI" ) ;
    m_aFixedText3->SetText(aViewSizeString);

    SvMemoryStream aMemStream;
    aMemStream.SetVersion( SOFFICE_FILEFORMAT_CURRENT );
    m_aGraphic.ExportNative(aMemStream);
    aMemStream.Seek( STREAM_SEEK_TO_END );
    sal_Int32 aNativeSize = aMemStream.Tell();

    String aNativeSizeString;
    aNativeSizeString += UniString::CreateFromInt32(aNativeSize / 1024);
    aNativeSizeString += String( " kiB" ) ;

    m_aFixedText5->SetText(aNativeSizeString);
    m_aFixedText6->SetText(String("??"));
}

void CompressGraphicsDialog::UpdateNewWidthMF()
{
    int nPixelX = (sal_Int32)( GetViewWidthInch() * m_dResolution );
    m_aMFNewWidth->SetText( UniString::CreateFromInt32( nPixelX ) );
}

void CompressGraphicsDialog::UpdateNewHeightMF()
{
    int nPixelY = (sal_Int32)( GetViewHeightInch() * m_dResolution );
    m_aMFNewHeight->SetText( UniString::CreateFromInt32( nPixelY ) );
}

void CompressGraphicsDialog::UpdateResolutionLB()
{
    m_aResolutionLB->SetText( UniString::CreateFromInt32( (sal_Int32) m_dResolution ) );
}

double CompressGraphicsDialog::GetViewWidthInch()
{
    return (double) MetricField::ConvertValue(m_aViewSize100mm.Width(),  2, MAP_100TH_MM, FUNIT_INCH) / 100.0;
}

double CompressGraphicsDialog::GetViewHeightInch()
{
    return (double) MetricField::ConvertValue(m_aViewSize100mm.Height(),  2, MAP_100TH_MM, FUNIT_INCH) / 100.0;
}

sal_uLong CompressGraphicsDialog::GetSelectedInterpolationType()
{
    OUString aSelectionText = OUString( m_aInterpolationCombo->GetSelectEntry() );

    if( aSelectionText == "Lanczos" ) {
        return BMP_SCALE_LANCZOS;
    } else if( aSelectionText == "Bilinear" ) {
        return BMP_SCALE_BILINEAR;
    } else if( aSelectionText == "Bicubic" ) {
        return BMP_SCALE_BICUBIC;
    } else if ( aSelectionText == "None" ) {
        return BMP_SCALE_FAST;
    }
    return BMP_SCALE_BEST;
}

void CompressGraphicsDialog::Compress(SvStream& aStream)
{
    BitmapEx bitmap = m_aGraphic.GetBitmapEx();
    if ( m_aReduceResolutionCB->IsChecked() )
    {
        long nPixelX = (long)( GetViewWidthInch() * m_dResolution );
        long nPixelY = (long)( GetViewHeightInch() * m_dResolution );

        bitmap.Scale( Size( nPixelX, nPixelY ), GetSelectedInterpolationType() );
    }
    Graphic aScaledGraphic = Graphic( bitmap );
    GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();

    Sequence< PropertyValue > aFilterData( 3 );
    aFilterData[ 0 ].Name = "Interlaced";
    aFilterData[ 0 ].Value <<= (sal_Int32) 0;
    aFilterData[ 1 ].Name = "Compression";
    aFilterData[ 1 ].Value <<= (sal_Int32) m_aCompressionMF->GetValue();
    aFilterData[ 2 ].Name = "Quality";
    aFilterData[ 2 ].Value <<= (sal_Int32) m_aQualityMF->GetValue();

    String aGraphicFormatName = m_aLosslessRB->IsChecked() ? String( "png" ) : String( "jpg" );

    sal_uInt16 nFilterFormat = rFilter.GetExportFormatNumberForShortName( aGraphicFormatName );
    rFilter.ExportGraphic( aScaledGraphic, String( "test" ), aStream, nFilterFormat, &aFilterData );
}

IMPL_LINK_NOARG( CompressGraphicsDialog, NewWidthModifiedHdl )
{
    m_dResolution =  m_aMFNewWidth->GetValue() / GetViewWidthInch();

    UpdateNewHeightMF();
    UpdateResolutionLB();
    Update();

    return 0L;
}

IMPL_LINK_NOARG( CompressGraphicsDialog, NewHeightModifiedHdl )
{
    m_dResolution =  m_aMFNewHeight->GetValue() / GetViewHeightInch();

    UpdateNewWidthMF();
    UpdateResolutionLB();
    Update();

    return 0L;
}

IMPL_LINK_NOARG( CompressGraphicsDialog, ResolutionModifiedHdl )
{
    m_dResolution = (double) m_aResolutionLB->GetText().ToInt32();

    UpdateNewWidthMF();
    UpdateNewHeightMF();
    Update();

    return 0L;
}

IMPL_LINK_NOARG( CompressGraphicsDialog, ToggleCompressionRB )
{
    bool choice = m_aLosslessRB->IsChecked();
    m_aCompressionMF->Enable(choice);
    m_aQualityMF->Enable(!choice);

    return 0L;
}

IMPL_LINK_NOARG( CompressGraphicsDialog, ToggleReduceResolutionRB )
{
    bool choice = m_aReduceResolutionCB->IsChecked();
    m_aMFNewWidth->Enable(choice);
    m_aMFNewHeight->Enable(choice);
    m_aResolutionLB->Enable(choice);
    m_aInterpolationCombo->Enable(choice);

    return 0L;
}

IMPL_LINK_NOARG( CompressGraphicsDialog, CalculateClickHdl )
{
    sal_Int32 aSize = 0;

    if ( m_dResolution > 0  )
    {
        SvMemoryStream aMemStream;
        aMemStream.SetVersion( SOFFICE_FILEFORMAT_CURRENT );
        Compress( aMemStream );
        aMemStream.Seek( STREAM_SEEK_TO_END );
        aSize = aMemStream.Tell();
    }

    if ( aSize > 0 )
    {
        String aNewSizeString;
        aNewSizeString += UniString::CreateFromInt32(aSize / 1024);
        aNewSizeString += String( " kiB" ) ;
        m_aFixedText6->SetText(aNewSizeString);
    }
    return 0L;
}

SdrGrafObj* CompressGraphicsDialog::GetCompressedSdrGrafObj()
{
    if ( m_dResolution > 0  )
    {
        SdrGrafObj* pNewObject = (SdrGrafObj*) m_pGraphicObj->Clone();

        if ( m_aReduceResolutionCB->IsChecked() )
        {
            const SdrGrafCropItem& rCrop = (const SdrGrafCropItem&) m_pGraphicObj->GetMergedItem(SDRATTR_GRAFCROP);
            long nPixelX = (long)( GetViewWidthInch()  * m_dResolution );
            long nPixelY = (long)( GetViewHeightInch() * m_dResolution );
            Size size = m_aGraphic.GetBitmapEx().GetSizePixel();
            double aScaleX = nPixelX / (double) size.Width();
            double aScaleY = nPixelY / (double) size.Height();

            SdrGrafCropItem aNewCrop(
                rCrop.GetLeft()  * aScaleX,
                rCrop.GetTop()   * aScaleY,
                rCrop.GetRight() * aScaleX,
                rCrop.GetBottom()* aScaleY);
            pNewObject->SetMergedItem(aNewCrop);
        }

        SvMemoryStream aMemStream;
        aMemStream.SetVersion( SOFFICE_FILEFORMAT_CURRENT );
        Compress( aMemStream );
        aMemStream.Seek( STREAM_SEEK_TO_BEGIN );
        Graphic aResultGraphic;
        GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();
        rFilter.ImportGraphic( aResultGraphic, String("import"), aMemStream );

        pNewObject->SetGraphic( aResultGraphic );

        return pNewObject;
    }
    return NULL;
}

Graphic CompressGraphicsDialog::GetCompressedGraphic()
{
    if ( m_dResolution > 0  )
    {
        SvMemoryStream aMemStream;
        aMemStream.SetVersion( SOFFICE_FILEFORMAT_CURRENT );
        Compress( aMemStream );
        aMemStream.Seek( STREAM_SEEK_TO_BEGIN );
        Graphic aResultGraphic;
        GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();
        rFilter.ImportGraphic( aResultGraphic, String("import"), aMemStream );
        return aResultGraphic;
    }
    return m_aGraphic;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
