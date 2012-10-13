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


#ifndef _EXPORTDIALOG_HXX_
#define _EXPORTDIALOG_HXX_

#include <svtools/fltcall.hxx>
#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/field.hxx>
#include <vcl/layout.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/msgbox.hxx>
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

    FltCallDialogParameter& mrFltCallPara;

    const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >
        mxMgr;
    const com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >&
        mxSourceDocument;

    NumericField*       mpMfSizeX;
    ListBox*            mpLbSizeX;
    NumericField*       mpMfSizeY;
    FixedText*          mpFtResolution;
    NumericField*       mpNfResolution;
    ListBox*            mpLbResolution;

    VclContainer*       mpColorDepth;
    ListBox*            mpLbColorDepth;

    VclContainer*       mpJPGQuality;
    VclContainer*       mpPNGCompression;

    ScrollBar*          mpSbCompression;
    NumericField*       mpNfCompression;

    VclContainer*       mpMode;
    CheckBox*           mpCbInterlaced;

    VclContainer*       mpBMPCompression;
    CheckBox*           mpCbRLEEncoding;

    VclContainer*       mpDrawingObjects;
    CheckBox*           mpCbSaveTransparency;

    VclContainer*       mpEncoding;
    RadioButton*        mpRbBinary;
    RadioButton*        mpRbText;

    VclContainer*       mpEPSGrid;
    CheckBox*           mpCbEPSPreviewTIFF;
    CheckBox*           mpCbEPSPreviewEPSI;
    RadioButton*        mpRbEPSLevel1;
    RadioButton*        mpRbEPSLevel2;
    RadioButton*        mpRbEPSColorFormat1;
    RadioButton*        mpRbEPSColorFormat2;
    RadioButton*        mpRbEPSCompressionLZW;
    RadioButton*        mpRbEPSCompressionNone;

    VclContainer*       mpInfo;
    FixedText*          mpFtEstimatedSize;

    OKButton*           mpBtnOK;

    String              msEstimatedSizePix1;
    String              msEstimatedSizePix2;
    String              msEstimatedSizeVec;

    String              ms1BitTreshold;
    String              ms1BitDithered;
    String              ms4BitGrayscale;
    String              ms4BitColorPalette;
    String              ms8BitGrayscale;
    String              ms8BitColorPalette;
    String              ms24BitColor;

    Size                maDialogSize;

    FilterConfigItem*   mpOptionsItem;
    FilterConfigItem*   mpFilterOptionsItem;

    String              maExt;
    String              maEstimatedSizeText;
    sal_Int16           mnFormat;
    sal_Int32           mnMaxFilesizeForRealtimePreview;

    SvStream*           mpTempStream;
    Bitmap              maBitmap;

    com::sun::star::awt::Size
                        maOriginalSize;     // the original graphic size in 1/100mm
    com::sun::star::awt::Size
                        maSize;             // for vector graphics it always contains the logical size in 1/100mm

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

                        void setupSizeControls();
                        void createFilterOptions();
                        void setupControls();
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
