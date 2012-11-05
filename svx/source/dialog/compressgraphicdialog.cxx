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

#include <svx/dialogs.hrc>
#include <svx/dialmgr.hxx>
#include <vcl/graph.hxx>
#include <compressgraphicdialog.hrc>
#include <svx/compressgraphicdialog.hxx>
#include <dlgunit.hxx>
#include <vcl/virdev.hxx>
#include <sfx2/dispatch.hxx>
#include <vcl/svapp.hxx>
#include <comphelper/processfactory.hxx>
#include <svtools/filter.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>

using namespace com::sun::star::uno;
using namespace com::sun::star::beans;

CompressGraphicsDialog::CompressGraphicsDialog( Window* pParent, const Graphic& rGraphic, const Size& rViewSize100mm, SfxBindings& rBindings ) :
    ModalDialog             ( pParent, SVX_RES( RID_SVXDLG_COMPRESSGRAPHICS ) ),
    m_aImageDetailsFL       ( this,    SVX_RES( FL_IMAGE_DETAILS ) ),
    m_aFixedText2X          ( this,    SVX_RES( FT_CG_2_X ) ),
    m_aFixedText2           ( this,    SVX_RES( FT_CG_2 ) ),
    m_aFixedText3X          ( this,    SVX_RES( FT_CG_3_X ) ),
    m_aFixedText3           ( this,    SVX_RES( FT_CG_3 ) ),
    m_aFixedText5X          ( this,    SVX_RES( FT_CG_5_X ) ),
    m_aFixedText5           ( this,    SVX_RES( FT_CG_5 ) ),
    m_aFixedText6X          ( this,    SVX_RES( FT_CG_6_X ) ),
    m_aFixedText6           ( this,    SVX_RES( FT_CG_6 ) ),
    m_aSettingsFL           ( this,    SVX_RES( FL_SETTINGS ) ),
    m_aReduceResolutionCB   ( this,    SVX_RES( CB_REDUCE_IMAGE_RESOLUTION ) ),
    m_aNewWidthFT           ( this,    SVX_RES( FT_NEW_WIDTH ) ),
    m_aMFNewWidth           ( this,    SVX_RES( MF_NEW_WIDTH ) ),
    m_aNewHeightFT          ( this,    SVX_RES( FT_NEW_HEIGHT ) ),
    m_aMFNewHeight          ( this,    SVX_RES( MF_NEW_HEIGHT ) ),
    m_aResolutionFT         ( this,    SVX_RES( FT_RESOLUTION ) ),
    m_aResolutionLB         ( this,    SVX_RES( LB_RESOLUTION ) ),
    m_aFixedTextDPI         ( this,    SVX_RES( FT_DPI ) ),
    m_aLosslessRB           ( this,    SVX_RES( RB_LOSSLESS_COMPRESSION ) ),
    m_aJpegCompRB           ( this,    SVX_RES( RB_JPEG_COMPRESSION ) ),
    m_aCompressionFT        ( this,    SVX_RES( FT_COMPRESSION ) ),
    m_aCompressionMF        ( this,    SVX_RES( MF_COMPRESSION_FACTOR ) ),
    m_aQualityFT            ( this,    SVX_RES( FT_QUALITY ) ),
    m_aQualityMF            ( this,    SVX_RES( MF_QUALITY_FACTOR ) ),
    m_aCropCB               ( this,    SVX_RES( CB_CROP ) ),
    m_aBtnOK                ( this,    SVX_RES( BUTTON_CG_OK ) ),
    m_aBtnCancel            ( this,    SVX_RES( BUTTON_CG_CANCEL ) ),
    m_aBtnHelp              ( this,    SVX_RES( BUTTON_CG_HELP ) ),
    m_aBtnCalculate         ( this,    SVX_RES( BUTTON_CG_CALCULATE ) ),
    m_aGraphic              ( rGraphic ),
    m_aViewSize100mm        ( rViewSize100mm ),
    m_rBindings             ( rBindings )
{
    FreeResource();

    SetStyle(GetStyle() | WB_CENTER | WB_VCENTER);

    m_aQualityMF.SetValue( 90L );
    m_aCompressionMF.SetValue( 9L );

    m_aMFNewWidth.SetModifyHdl( LINK( this, CompressGraphicsDialog, NewWidthModifiedHdl ));
    m_aMFNewHeight.SetModifyHdl( LINK( this, CompressGraphicsDialog, NewHeightModifiedHdl ));

    m_dResolution = 96.0;

    m_aResolutionLB.SetModifyHdl( LINK( this, CompressGraphicsDialog, ResolutionModifiedHdl ));
    m_aBtnCalculate.SetClickHdl(  LINK( this, CompressGraphicsDialog, CalculateClickHdl ) );

    m_aLosslessRB.SetToggleHdl( LINK( this, CompressGraphicsDialog, ToggleCompressionRB ) );
    m_aJpegCompRB.SetToggleHdl( LINK( this, CompressGraphicsDialog, ToggleCompressionRB ) );

    m_aReduceResolutionCB.SetToggleHdl( LINK( this, CompressGraphicsDialog, ToggleReduceResolutionRB ) );


    m_aLosslessRB.Check();
    m_aReduceResolutionCB.Check();

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
    LocaleDataWrapper aLocaleWrapper( Application::GetSettings().GetLocale() );
    sal_Unicode cSep = aLocaleWrapper.getNumDecimalSep()[0];

    VirtualDevice* pDummyVDev = new VirtualDevice();
    pDummyVDev->EnableOutput( false );
    pDummyVDev->SetMapMode( m_aGraphic.GetPrefMapMode() );

    Size aPixelSize = m_aGraphic.GetSizePixel();
    Size aOriginalSize100mm( pDummyVDev->PixelToLogic( m_aGraphic.GetSizePixel(), MAP_100TH_MM ) );

    String aBitmapSizeString;
    aBitmapSizeString += GetUnitString( aOriginalSize100mm.Width(), eFieldUnit, cSep );
    aBitmapSizeString += String( " x " ) ;
    aBitmapSizeString += GetUnitString( aOriginalSize100mm.Height(), eFieldUnit, cSep );
    aBitmapSizeString += String( " ( " ) ;
    aBitmapSizeString += UniString::CreateFromInt32(aPixelSize.Width());
    aBitmapSizeString += String( " x " ) ;
    aBitmapSizeString += UniString::CreateFromInt32(aPixelSize.Height());
    aBitmapSizeString += String( " px )" ) ;
    m_aFixedText2.SetText(aBitmapSizeString);

    String aViewSizeString;

    int aValX = (int) (aPixelSize.Width() / GetViewWidthInch());

    aViewSizeString += GetUnitString( m_aViewSize100mm.Width(), eFieldUnit, cSep );
    aViewSizeString += String( " x " ) ;
    aViewSizeString += GetUnitString( m_aViewSize100mm.Height(), eFieldUnit, cSep );
    aViewSizeString += ( " at " ) ;
    aViewSizeString += UniString::CreateFromInt32(aValX);
    aViewSizeString += ( " DPI" ) ;
    m_aFixedText3.SetText(aViewSizeString);

    SvMemoryStream aMemStream;
    aMemStream.SetVersion( SOFFICE_FILEFORMAT_CURRENT );
    m_aGraphic.ExportNative(aMemStream);
    aMemStream.Seek( STREAM_SEEK_TO_END );
    sal_Int32 aNativeSize = aMemStream.Tell();

    String aNativeSizeString;
    aNativeSizeString += UniString::CreateFromInt32(aNativeSize / 1024);
    aNativeSizeString += String( " kiB" ) ;

    m_aFixedText5.SetText(aNativeSizeString);
    m_aFixedText6.SetText(String("??"));
}

void CompressGraphicsDialog::UpdateNewWidthMF()
{
    int nPixelX = (sal_Int32)( GetViewWidthInch() * m_dResolution );
    m_aMFNewWidth.SetText( UniString::CreateFromInt32( nPixelX ) );
}

void CompressGraphicsDialog::UpdateNewHeightMF()
{
    int nPixelY = (sal_Int32)( GetViewHeightInch() * m_dResolution );
    m_aMFNewHeight.SetText( UniString::CreateFromInt32( nPixelY ) );
}

void CompressGraphicsDialog::UpdateResolutionLB()
{
    m_aResolutionLB.SetText( UniString::CreateFromInt32( (sal_Int32) m_dResolution ) );
}

double CompressGraphicsDialog::GetViewWidthInch()
{
    return (double) MetricField::ConvertValue(m_aViewSize100mm.Width(),  2, MAP_100TH_MM, FUNIT_INCH) / 100.0;
}

double CompressGraphicsDialog::GetViewHeightInch()
{
    return (double) MetricField::ConvertValue(m_aViewSize100mm.Height(),  2, MAP_100TH_MM, FUNIT_INCH) / 100.0;
}

void CompressGraphicsDialog::Compress(SvStream& aStream)
{
    long nPixelX = (long)( GetViewWidthInch() * m_dResolution );
    long nPixelY = (long)( GetViewHeightInch() * m_dResolution );

    BitmapEx bitmap = m_aGraphic.GetBitmapEx();
    if ( m_aReduceResolutionCB.IsChecked() )
    {
        bitmap.Scale( Size( nPixelX, nPixelY ), BMP_SCALE_BEST );
    }
    Graphic aScaledGraphic = Graphic( bitmap );
    GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();

    Sequence< PropertyValue > aFilterData( 3 );
    aFilterData[ 0 ].Name = "Interlaced";
    aFilterData[ 0 ].Value <<= (sal_Int32) 0;
    aFilterData[ 1 ].Name = "Compression";
    aFilterData[ 1 ].Value <<= (sal_Int32) m_aCompressionMF.GetValue();
    aFilterData[ 2 ].Name = "Quality";
    aFilterData[ 2 ].Value <<= (sal_Int32) m_aQualityMF.GetValue();

    String aGraphicFormatName = m_aLosslessRB.IsChecked() ? String( "png" ) : String( "jpg" );

    sal_uInt16 nFilterFormat = rFilter.GetExportFormatNumberForShortName( aGraphicFormatName );
    rFilter.ExportGraphic( aScaledGraphic, String( "test" ), aStream, nFilterFormat, &aFilterData );
}

IMPL_LINK_NOARG( CompressGraphicsDialog, NewWidthModifiedHdl )
{
    m_dResolution =  m_aMFNewWidth.GetValue() / GetViewWidthInch();

    UpdateNewHeightMF();
    UpdateResolutionLB();
    Update();

    return 0L;
}

IMPL_LINK_NOARG( CompressGraphicsDialog, NewHeightModifiedHdl )
{
    m_dResolution =  m_aMFNewHeight.GetValue() / GetViewHeightInch();

    UpdateNewWidthMF();
    UpdateResolutionLB();
    Update();

    return 0L;
}

IMPL_LINK_NOARG( CompressGraphicsDialog, ResolutionModifiedHdl )
{
    m_dResolution = (double) m_aResolutionLB.GetText().ToInt32();

    UpdateNewWidthMF();
    UpdateNewHeightMF();
    Update();

    return 0L;
}

IMPL_LINK_NOARG( CompressGraphicsDialog, ToggleCompressionRB )
{
    bool choice = m_aLosslessRB.IsChecked();
    m_aCompressionMF.Enable(choice);
    m_aQualityMF.Enable(!choice);

    return 0L;
}

IMPL_LINK_NOARG( CompressGraphicsDialog, ToggleReduceResolutionRB )
{
    bool choice = m_aReduceResolutionCB.IsChecked();
    m_aMFNewWidth.Enable(choice);
    m_aMFNewHeight.Enable(choice);
    m_aResolutionLB.Enable(choice);

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
        m_aFixedText6.SetText(aNewSizeString);
    }
    return 0L;
}

Graphic CompressGraphicsDialog::GetCompressedGraphic()
{
    if ( m_dResolution > 0  )
    {
        GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();
        Graphic aResultGraphic = Graphic ();
        SvMemoryStream aMemStream;
        aMemStream.SetVersion( SOFFICE_FILEFORMAT_CURRENT );
        Compress( aMemStream );
        aMemStream.Seek( STREAM_SEEK_TO_BEGIN );
        rFilter.ImportGraphic( aResultGraphic, String("test"), aMemStream );
        return aResultGraphic;
    }
    return m_aGraphic;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
