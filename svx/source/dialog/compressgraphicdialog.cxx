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
    ModalDialog       ( pParent, SVX_RES( RID_SVXDLG_COMPRESSGRAPHICS ) ),
    m_aFixedText2X    ( this,    SVX_RES( FT_CG_2_X ) ),
    m_aFixedText2     ( this,    SVX_RES( FT_CG_2 ) ),
    m_aFixedText3X    ( this,    SVX_RES( FT_CG_3_X ) ),
    m_aFixedText3     ( this,    SVX_RES( FT_CG_3 ) ),
    m_aFixedText4X    ( this,    SVX_RES( FT_CG_4_X ) ),
    m_aFixedText4     ( this,    SVX_RES( FT_CG_4 ) ),
    m_aFixedText5X    ( this,    SVX_RES( FT_CG_5_X ) ),
    m_aFixedText5     ( this,    SVX_RES( FT_CG_5 ) ),
    m_aFixedText6X    ( this,    SVX_RES( FT_CG_6_X ) ),
    m_aFixedText6     ( this,    SVX_RES( FT_CG_6 ) ),
    m_aResolutionLB   ( this,    SVX_RES( LB_CG_RESOLUTION ) ),
    m_aMFQuality      ( this,    SVX_RES( MF_CG_QUALITY ) ),
    m_aLossless       ( this,    SVX_RES( CB_CG_LOSSLESS ) ),
    m_aBtnOK          ( this,    SVX_RES( BUTTON_CG_OK ) ),
    m_aBtnCancel      ( this,    SVX_RES( BUTTON_CG_CANCEL ) ),
    m_aBtnHelp        ( this,    SVX_RES( BUTTON_CG_HELP ) ),
    m_aBtnCalculate   ( this,    SVX_RES( BUTTON_CG_CALCULATE ) ),
    m_aGraphic        ( rGraphic ),
    m_aViewSize100mm  ( rViewSize100mm ),
    m_rBindings       ( rBindings )
{
    FreeResource();

    SetStyle(GetStyle() | WB_CENTER | WB_VCENTER);
    m_aMFQuality.SetValue( 90L );
    m_aResolutionLB.SetText(String("96 DPI"));
    m_aResolutionLB.SetModifyHdl( LINK( this, CompressGraphicsDialog, ResolutionModifiedHdl ));
    m_aBtnCalculate.SetClickHdl(  LINK( this, CompressGraphicsDialog, ClickHdl ) );
    Update();
}

CompressGraphicsDialog::~CompressGraphicsDialog()
{
}

void CompressGraphicsDialog::Update()
{
    const FieldUnit eFieldUnit = m_rBindings.GetDispatcher()->GetModule()->GetFieldUnit();
    LocaleDataWrapper aLocaleWrapper( ::comphelper::getProcessServiceFactory(), Application::GetSettings().GetLocale() );
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

    int aValX = aPixelSize.Width()  * 100 / MetricField::ConvertValue(m_aViewSize100mm.Width(),  2, MAP_100TH_MM, FUNIT_INCH);
    //int aValY = aPixelSize.Height() * 100 / MetricField::ConvertValue(m_aViewSize100mm.Height(), 2, MAP_100TH_MM, FUNIT_INCH);

    aViewSizeString += GetUnitString( m_aViewSize100mm.Width(), eFieldUnit, cSep );
    aViewSizeString += String( " x " ) ;
    aViewSizeString += GetUnitString( m_aViewSize100mm.Height(), eFieldUnit, cSep );
    aViewSizeString += ( " at " ) ;
    aViewSizeString += UniString::CreateFromInt32(aValX);
    aViewSizeString += ( " DPI" ) ;
    m_aFixedText3.SetText(aViewSizeString);

    int aResolution = m_aResolutionLB.GetText().ToInt32();
    int nPixelX = (sal_Int32)((double)MetricField::ConvertValue(m_aViewSize100mm.Width(),   2, MAP_100TH_MM, FUNIT_INCH) / 100 * aResolution );
    int nPixelY = (sal_Int32)((double)MetricField::ConvertValue(m_aViewSize100mm.Height(),  2, MAP_100TH_MM, FUNIT_INCH) / 100 * aResolution );

    SvMemoryStream aMemStream;
    aMemStream.SetVersion( SOFFICE_FILEFORMAT_CURRENT );
    m_aGraphic.ExportNative(aMemStream);
    aMemStream.Seek( STREAM_SEEK_TO_END );
    sal_Int32 aNativeSize = aMemStream.Tell();

    String aCompressedSizeString;
    aCompressedSizeString += UniString::CreateFromInt32(nPixelX);
    aCompressedSizeString += String( " x " ) ;
    aCompressedSizeString += UniString::CreateFromInt32(nPixelY);
    aCompressedSizeString += ( " at " ) ;
    aCompressedSizeString += UniString::CreateFromInt32(aResolution);
    aCompressedSizeString += ( " DPI" ) ;
    m_aFixedText4.SetText(aCompressedSizeString);

    String aNativeSizeString;
    aNativeSizeString += UniString::CreateFromInt32(aNativeSize / 1024);
    aNativeSizeString += String( " kiB" ) ;

    m_aFixedText5.SetText(aNativeSizeString);
    m_aFixedText6.SetText(String("??"));
}

IMPL_LINK_NOARG( CompressGraphicsDialog, ResolutionModifiedHdl )
{
    Update();
    return 0L;
}

IMPL_LINK_NOARG( CompressGraphicsDialog, ClickHdl )
{
    int aResolution = m_aResolutionLB.GetText().ToInt32();
    sal_Int32 aSize = 0;

    if ( aResolution > 0  )
    {
        long nPixelX = (long)((double) MetricField::ConvertValue(m_aViewSize100mm.Width(),   2, MAP_100TH_MM, FUNIT_INCH) / 100.0 * (double) aResolution );
        long nPixelY = (long)((double) MetricField::ConvertValue(m_aViewSize100mm.Height(),  2, MAP_100TH_MM, FUNIT_INCH) / 100.0 * (double) aResolution );

        BitmapEx bitmap = m_aGraphic.GetBitmapEx();
        bitmap.Scale(Size(nPixelX, nPixelY), BMP_SCALE_BEST);
        Graphic aScaledGraphic = Graphic (bitmap);
        GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();

        Sequence< PropertyValue > aFilterData( 3 );
        aFilterData[ 0 ].Name = "Interlaced";
        aFilterData[ 0 ].Value <<= (sal_Int32) 0;
        aFilterData[ 1 ].Name = "Compression";
        aFilterData[ 1 ].Value <<= (sal_Int32) 9;
        aFilterData[ 2 ].Name = "Quality";
        aFilterData[ 2 ].Value <<= (sal_Int32) m_aMFQuality.GetValue();

        SvMemoryStream aMemStream;
        aMemStream.SetVersion( SOFFICE_FILEFORMAT_CURRENT );

        if ( m_aLossless.IsChecked() )
        {
            sal_uInt16 nFilterFormat = rFilter.GetExportFormatNumberForShortName( String( "png" ) );
            rFilter.ExportGraphic( aScaledGraphic, String("test.png"), aMemStream, nFilterFormat, &aFilterData );
            aMemStream.Seek( STREAM_SEEK_TO_END );
            aSize = aMemStream.Tell();
        }
        else
        {
            sal_uInt16 nFilterFormat = rFilter.GetExportFormatNumberForShortName( String( "jpg" ) );
            rFilter.ExportGraphic( aScaledGraphic, String("test.jpg"), aMemStream, nFilterFormat, &aFilterData );
            aMemStream.Seek( STREAM_SEEK_TO_END );
            aSize = aMemStream.Tell();
        }
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
    int aResolution = m_aResolutionLB.GetText().ToInt32();

    if ( aResolution > 0  )
    {
        long nPixelX = (long)((double) MetricField::ConvertValue( m_aViewSize100mm.Width(),   2, MAP_100TH_MM, FUNIT_INCH) / 100.0 * (double) aResolution );
        long nPixelY = (long)((double) MetricField::ConvertValue( m_aViewSize100mm.Height(),  2, MAP_100TH_MM, FUNIT_INCH) / 100.0 * (double) aResolution );

        BitmapEx bitmap = m_aGraphic.GetBitmapEx();
        bitmap.Scale(Size(nPixelX, nPixelY), BMP_SCALE_BEST);
        Graphic aScaledGraphic = Graphic (bitmap);
        GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();

        Sequence< PropertyValue > aFilterData( 3 );
        aFilterData[ 0 ].Name = "Interlaced";
        aFilterData[ 0 ].Value <<= (sal_Int32) 0;
        aFilterData[ 1 ].Name = "Compression";
        aFilterData[ 1 ].Value <<= (sal_Int32) 9;
        aFilterData[ 2 ].Name = "Quality";
        aFilterData[ 2 ].Value <<= (sal_Int32) m_aMFQuality.GetValue();

        SvMemoryStream aMemStream;
        aMemStream.SetVersion( SOFFICE_FILEFORMAT_CURRENT );
        Graphic aResultGraphic = Graphic ();

        if ( m_aLossless.IsChecked() )
        {
            sal_uInt16 nFilterFormat = rFilter.GetExportFormatNumberForShortName( String( "png" ) );
            rFilter.ExportGraphic( aScaledGraphic, String("test.png"), aMemStream, nFilterFormat, &aFilterData );
            aMemStream.Seek( STREAM_SEEK_TO_BEGIN );
            rFilter.ImportGraphic( aResultGraphic, String("test.png"), aMemStream );
        }
        else
        {
            sal_uInt16 nFilterFormat = rFilter.GetExportFormatNumberForShortName( String( "jpg" ) );
            rFilter.ExportGraphic( aScaledGraphic, String("test.jpg"), aMemStream, nFilterFormat, &aFilterData );
            aMemStream.Seek( STREAM_SEEK_TO_BEGIN );
            rFilter.ImportGraphic( aResultGraphic, String("test.jpg"), aMemStream );
        }
        return aResultGraphic;
    }
    return m_aGraphic;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
