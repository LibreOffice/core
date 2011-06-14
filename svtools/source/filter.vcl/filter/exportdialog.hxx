/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


#ifndef _EXPORTDIALOG_HXX_
#define _EXPORTDIALOG_HXX_

#include <svtools/fltcall.hxx>
#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/field.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/arrange.hxx>
#include <vcl/scrbar.hxx>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>

/*************************************************************************
|*
|* Dialog zum Einstellen von Filteroptionen bei Pixelformaten
|*
\************************************************************************/

class FilterConfigItem;
class ExportDialog : public ModalDialog
{
private:

    FltCallDialogParameter&
        mrFltCallPara;

    ResMgr*             mpMgr;

    const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >
        mxMgr;
    const com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >&
        mxSourceDocument;

    FixedLine           maFlExportSize;
    FixedText           maFtSizeX;
    MetricField         maMfSizeX;
    ListBox             maLbSizeX;
    FixedText           maFtSizeY;
    MetricField         maMfSizeY;
    ListBox             maLbSizeY;
    FixedText           maFtResolution;
    NumericField        maNfResolution;
    ListBox             maLbResolution;
    FixedLine           maFlColorDepth;
    ListBox             maLbColorDepth;
    FixedLine           maFlJPGQuality;
    FixedLine           maFlMode;
    FixedLine           maFlPBMOptions;
    ScrollBar           maSbCompression;
    NumericField        maNfCompression;
    FixedText           maFtJPGMin;
    FixedText           maFtJPGMax;
    FixedText           maFtPNGMin;
    FixedText           maFtPNGMax;
    CheckBox            maCbJPGPreview;
    CheckBox            maCbInterlaced;
    CheckBox            maCbRLEEncoding;
    FixedLine           maFlGIFDrawingObjects;
    CheckBox            maCbSaveTransparency;
    RadioButton         maRbBinary;
    RadioButton         maRbText;
    FixedLine           maFlEPSPreview;
    CheckBox            maCbEPSPreviewTIFF;
    CheckBox            maCbEPSPreviewEPSI;
    FixedLine           maFlEPSVersion;
    RadioButton         maRbEPSLevel1;
    RadioButton         maRbEPSLevel2;
    FixedLine           maFlEPSColorFormat;
    RadioButton         maRbEPSColorFormat1;
    RadioButton         maRbEPSColorFormat2;
    FixedLine           maFlCompression;
    RadioButton         maRbEPSCompressionLZW;
    RadioButton         maRbEPSCompressionNone;
    FixedLine           maFlEstimatedSize;
    FixedText           maFtEstimatedSize;
    String              msEstimatedSizePix1;
    String              msEstimatedSizePix2;
    String              msEstimatedSizeVec;
    FixedLine           maFlButtons;
    FixedBitmap         maFbJPGPreview;
    ScrollBar           maSbZoom;
    NumericField        maNfZoom;
    ScrollBar           maSbJPGPreviewHorz;
    ScrollBar           maSbJPGPreviewVert;
    OKButton            maBtnOK;
    CancelButton        maBtnCancel;
    HelpButton          maBtnHelp;

    String              ms1BitTreshold;
    String              ms1BitDithered;
    String              ms4BitGrayscale;
    String              ms4BitColorPalette;
    String              ms8BitGrayscale;
    String              ms8BitColorPalette;
    String              ms24BitColor;

    vcl::RowOrColumn    maLayout;
    Size                maDialogSize;

    FilterConfigItem*   mpOptionsItem;
    FilterConfigItem*   mpFilterOptionsItem;

    String              maExt;
    String              maEstimatedSizeText;
    sal_Int16           mnFormat;
    sal_Int32           mnMaxFilesizeForRealtimePreview;

    Rectangle           maRectFlButtons;
    Rectangle           maRectBtnHelp;
    Rectangle           maRectBtnOK;
    Rectangle           maRectBtnCancel;

    SvStream*           mpTempStream;
    Bitmap              maBitmap;

    com::sun::star::awt::Size
                        maOriginalSize;     // the original graphic size in 1/100mm
    com::sun::star::awt::Size
                        maSize;             // for vector graphics it always contains the logical size in 1/100mm

    sal_Bool            mbPreview;
    sal_Bool            mbIsPixelFormat;
    sal_Bool            mbExportSelection;
    sal_Bool            mbPreserveAspectRatio;

    sal_Int32           mnInitialResolutionUnit;

    // for pixel graphics it always contains the pixel count
    com::sun::star::awt::Size
                        maResolution;       // it always contains the number of pixels per meter

    com::sun::star::uno::Reference< com::sun::star::drawing::XShape >
                        mxShape;
    com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >
                        mxShapes;
    com::sun::star::uno::Reference< com::sun::star::drawing::XDrawPage >
                        mxPage;


                        DECL_LINK( UpdateHdl,void* p );
                        DECL_LINK( UpdateHdlMtfSizeX,void* p );
                        DECL_LINK( UpdateHdlMtfSizeY,void* p );
                        DECL_LINK( UpdateHdlNfResolution,void* p );
                        DECL_LINK( SbCompressionUpdateHdl,void* p );
                        DECL_LINK( NfCompressionUpdateHdlX,void* p );

                        DECL_LINK( OK, void* p );

                        void createSizeControls( vcl::RowOrColumn& );
                        void createColorDepthControls( vcl::RowOrColumn& );
                        void createFilterOptions( vcl::RowOrColumn& );
                        void createButtons( vcl::RowOrColumn& );
                        void createScrollBar( vcl::RowOrColumn& );
                        void setupLayout();
                        void updatePreview();
                        void updateControls();

                        void GetGraphicSource();
                        sal_Bool GetGraphicStream();
                        Bitmap GetGraphicBitmap( SvStream& rStream );
                        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >
                            GetFilterData( sal_Bool bUpdateConfig );

                        sal_uInt32 GetRawFileSize() const;
                        sal_Bool IsTempExportAvailable() const;

                        com::sun::star::awt::Size
                            GetOriginalSize();

                        sal_Int32 GetDefaultUnit();

public:
                        ExportDialog( FltCallDialogParameter& rPara,
                            const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory > rxMgr,
                                const com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& rxSourceDocument,
                                    sal_Bool bExportSelection, sal_Bool bIsExportVectorFormat );
                        ~ExportDialog();
};



#endif // _EXPORTDIALOG_HXX_

