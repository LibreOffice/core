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


#ifndef INCLUDED_SVTOOLS_SOURCE_FILTER_EXPORTDIALOG_HXX
#define INCLUDED_SVTOOLS_SOURCE_FILTER_EXPORTDIALOG_HXX

#include <vcl/fltcall.hxx>
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
|* dialog to set filter options for pixel formats
|*
\************************************************************************/

class FilterConfigItem;
class ExportDialog : public ModalDialog
{
private:

    FltCallDialogParameter&    mrFltCallPara;

    const css::uno::Reference< css::uno::XComponentContext >
                               mxContext;
    const css::uno::Reference< css::lang::XComponent >&
                               mxSourceDocument;

    VclPtr<NumericField>       mpMfSizeX;
    VclPtr<ListBox>            mpLbSizeX;
    VclPtr<NumericField>       mpMfSizeY;
    VclPtr<FixedText>          mpFtResolution;
    VclPtr<NumericField>       mpNfResolution;
    VclPtr<ListBox>            mpLbResolution;

    VclPtr<VclContainer>       mpColorDepth;
    VclPtr<ListBox>            mpLbColorDepth;

    VclPtr<VclContainer>       mpJPGQuality;
    VclPtr<VclContainer>       mpPNGCompression;

    VclPtr<ScrollBar>          mpSbCompression;
    VclPtr<NumericField>       mpNfCompression;

    VclPtr<VclContainer>       mpMode;
    VclPtr<CheckBox>           mpCbInterlaced;

    VclPtr<VclContainer>       mpBMPCompression;
    VclPtr<CheckBox>           mpCbRLEEncoding;

    VclPtr<VclContainer>       mpDrawingObjects;
    VclPtr<CheckBox>           mpCbSaveTransparency;

    VclPtr<VclContainer>       mpEncoding;
    VclPtr<RadioButton>        mpRbBinary;
    VclPtr<RadioButton>        mpRbText;

    VclPtr<VclContainer>       mpEPSGrid;
    VclPtr<CheckBox>           mpCbEPSPreviewTIFF;
    VclPtr<CheckBox>           mpCbEPSPreviewEPSI;
    VclPtr<RadioButton>        mpRbEPSLevel1;
    VclPtr<RadioButton>        mpRbEPSLevel2;
    VclPtr<RadioButton>        mpRbEPSColorFormat1;
    VclPtr<RadioButton>        mpRbEPSColorFormat2;
    VclPtr<RadioButton>        mpRbEPSCompressionLZW;
    VclPtr<RadioButton>        mpRbEPSCompressionNone;

    VclPtr<VclContainer>       mpInfo;
    VclPtr<FixedText>          mpFtEstimatedSize;

    VclPtr<OKButton>           mpBtnOK;

    OUString            msEstimatedSizePix1;
    OUString            msEstimatedSizePix2;
    OUString            msEstimatedSizeVec;

    OUString            ms1BitTreshold;
    OUString            ms1BitDithered;
    OUString            ms4BitGrayscale;
    OUString            ms4BitColorPalette;
    OUString            ms8BitGrayscale;
    OUString            ms8BitColorPalette;
    OUString            ms24BitColor;

    FilterConfigItem*   mpOptionsItem;
    FilterConfigItem*   mpFilterOptionsItem;

    OUString            maExt;
    sal_Int16           mnFormat;
    sal_Int32           mnMaxFilesizeForRealtimePreview;

    SvStream*           mpTempStream;
    Bitmap              maBitmap;

    css::awt::Size      maOriginalSize;     // the original graphic size in 1/100mm
    css::awt::Size      maSize;             // for vector graphics it always contains the logical size in 1/100mm

    bool                mbIsPixelFormat;
    bool                mbExportSelection;
    bool                mbPreserveAspectRatio;

    sal_Int32           mnInitialResolutionUnit;

    // for pixel graphics it always contains the pixel count
    css::awt::Size      maResolution;       // it always contains the number of pixels per meter

    css::uno::Reference< css::drawing::XShape >
                        mxShape;
    css::uno::Reference< css::drawing::XShapes >
                        mxShapes;
    css::uno::Reference< css::drawing::XDrawPage >
                        mxPage;


                        DECL_LINK_TYPED( UpdateHdl, Button*, void );
                        DECL_LINK_TYPED( SelectListBoxHdl, ListBox&, void );
                        DECL_LINK_TYPED( SelectHdl, Edit&, void );
                        DECL_LINK_TYPED( UpdateHdlMtfSizeX, Edit&, void );
                        DECL_LINK_TYPED( UpdateHdlMtfSizeY, Edit&, void );
                        DECL_LINK_TYPED( UpdateHdlNfResolution, Edit&, void );
                        DECL_LINK_TYPED( SbCompressionUpdateHdl, ScrollBar*, void );

                        DECL_LINK_TYPED( OK, Button*, void );

                        void setupSizeControls();
                        void createFilterOptions();
                        void setupControls();
                        void updateControls();

                        void GetGraphicSource();
                        bool GetGraphicStream();
                        static Bitmap GetGraphicBitmap( SvStream& rStream );
                        css::uno::Sequence< css::beans::PropertyValue >
                            GetFilterData( bool bUpdateConfig );

                        sal_uInt32 GetRawFileSize() const;
                        bool IsTempExportAvailable() const;

                        css::awt::Size
                            GetOriginalSize();

                        sal_Int32 GetDefaultUnit();

public:
                        ExportDialog( FltCallDialogParameter& rPara,
                            const css::uno::Reference< css::uno::XComponentContext >& rxContext,
                            const css::uno::Reference< css::lang::XComponent >& rxSourceDocument,
                            bool bExportSelection, bool bIsExportVectorFormat );
                        virtual ~ExportDialog();
                        virtual void dispose() override;
};



#endif // INCLUDED_SVTOOLS_SOURCE_FILTER_EXPORTDIALOG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
