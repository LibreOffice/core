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

#include <FltCallDialogParameter.hxx>
#include <vcl/weld.hxx>
#include <tools/stream.hxx>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <memory>

/*************************************************************************
|*
|* dialog to set filter options for pixel formats
|*
\************************************************************************/

class FilterConfigItem;
class ExportDialog : public weld::GenericDialogController
{
private:

    FltCallDialogParameter&    mrFltCallPara;

    const css::uno::Reference< css::uno::XComponentContext >
                               mxContext;
    const css::uno::Reference< css::lang::XComponent >&
                               mxSourceDocument;
    const css::uno::Reference< css::graphic::XGraphic >&
                               mxGraphic;

    OUString const            msEstimatedSizePix1;
    OUString const            msEstimatedSizePix2;
    OUString const            msEstimatedSizeVec;

    OUString const            ms1BitThreshold;
    OUString const            ms1BitDithered;
    OUString const            ms4BitGrayscale;
    OUString const            ms4BitColorPalette;
    OUString const            ms8BitGrayscale;
    OUString const            ms8BitColorPalette;
    OUString const            ms24BitColor;

    std::unique_ptr<FilterConfigItem> mpOptionsItem;
    std::unique_ptr<FilterConfigItem> mpFilterOptionsItem;

    OUString            maExt;
    sal_Int16           mnFormat;
    sal_Int32           mnMaxFilesizeForRealtimePreview;

    std::unique_ptr<SvMemoryStream> mpTempStream;

    css::awt::Size      maOriginalSize;     // the original graphic size in 1/100mm
    css::awt::Size      maSize;             // for vector graphics it always contains the logical size in 1/100mm

    bool const                mbIsPixelFormat;
    bool const                mbExportSelection;
    bool                mbGraphicsSource;   // whether source document is graphics (Draw, Impress) or not (Calc, Writer)

    sal_Int32           mnInitialResolutionUnit;

    // for pixel graphics it always contains the pixel count
    css::awt::Size      maResolution;       // it always contains the number of pixels per meter

    css::uno::Reference< css::drawing::XShape >
                        mxShape;
    css::uno::Reference< css::drawing::XShapes >
                        mxShapes;
    css::uno::Reference< css::drawing::XDrawPage >
                        mxPage;

    weld::Scale* mpSbCompression;
    weld::SpinButton* mpNfCompression;

    std::unique_ptr<weld::SpinButton> mxMfSizeX;
    std::unique_ptr<weld::ComboBox> mxLbSizeX;
    std::unique_ptr<weld::SpinButton> mxMfSizeY;
    std::unique_ptr<weld::Label> mxFtResolution;
    std::unique_ptr<weld::SpinButton> mxNfResolution;
    std::unique_ptr<weld::ComboBox> mxLbResolution;

    std::unique_ptr<weld::Widget> mxColorDepth;
    std::unique_ptr<weld::ComboBox> mxLbColorDepth;

    std::unique_ptr<weld::Widget> mxJPGQuality;
    std::unique_ptr<weld::Widget> mxPNGCompression;

    std::unique_ptr<weld::Scale> mxSbPngCompression;
    std::unique_ptr<weld::SpinButton> mxNfPngCompression;

    std::unique_ptr<weld::Scale> mxSbJpgCompression;
    std::unique_ptr<weld::SpinButton> mxNfJpgCompression;

    std::unique_ptr<weld::Widget> mxMode;
    std::unique_ptr<weld::CheckButton> mxCbInterlaced;

    std::unique_ptr<weld::Widget> mxBMPCompression;
    std::unique_ptr<weld::CheckButton> mxCbRLEEncoding;

    std::unique_ptr<weld::Widget> mxDrawingObjects;
    std::unique_ptr<weld::CheckButton> mxCbSaveTransparency;

    std::unique_ptr<weld::Widget> mxEncoding;
    std::unique_ptr<weld::RadioButton> mxRbBinary;
    std::unique_ptr<weld::RadioButton> mxRbText;

    std::unique_ptr<weld::Widget> mxEPSGrid;
    std::unique_ptr<weld::CheckButton> mxCbEPSPreviewTIFF;
    std::unique_ptr<weld::CheckButton> mxCbEPSPreviewEPSI;
    std::unique_ptr<weld::RadioButton> mxRbEPSLevel1;
    std::unique_ptr<weld::RadioButton> mxRbEPSLevel2;
    std::unique_ptr<weld::RadioButton> mxRbEPSColorFormat1;
    std::unique_ptr<weld::RadioButton> mxRbEPSColorFormat2;
    std::unique_ptr<weld::RadioButton> mxRbEPSCompressionLZW;
    std::unique_ptr<weld::RadioButton> mxRbEPSCompressionNone;

    std::unique_ptr<weld::Widget> mxInfo;
    std::unique_ptr<weld::Label> mxFtEstimatedSize;

    std::unique_ptr<weld::Button> mxBtnOK;

    DECL_LINK(UpdateHdl, weld::ToggleButton&, void);
    DECL_LINK(SelectListBoxHdl, weld::ComboBox&, void);
    DECL_LINK(SelectHdl, weld::SpinButton&, void);
    DECL_LINK(UpdateHdlMtfSizeX, weld::SpinButton&, void);
    DECL_LINK(UpdateHdlMtfSizeY, weld::SpinButton&, void);
    DECL_LINK(UpdateHdlNfResolution, weld::SpinButton&, void);
    DECL_LINK(SbCompressionUpdateHdl, weld::Scale&, void);

    DECL_LINK(OK, weld::Button&, void);

    void setupSizeControls();
    void createFilterOptions();
    void setupControls();
    void updateControls();

    void GetGraphicSource();
    void GetGraphicStream();
    css::uno::Sequence< css::beans::PropertyValue >
        GetFilterData( bool bUpdateConfig );

    sal_uInt32 GetRawFileSize() const;
    bool IsTempExportAvailable() const;

    css::awt::Size GetOriginalSize();

    sal_Int32 GetDefaultUnit();

public:
    ExportDialog( FltCallDialogParameter& rPara,
        const css::uno::Reference< css::uno::XComponentContext >& rxContext,
        const css::uno::Reference< css::lang::XComponent >& rxSourceDocument,
        bool bExportSelection, bool bIsExportVectorFormat, bool bGraphicsSource,
        const css::uno::Reference< css::graphic::XGraphic >& rxGraphic);
    virtual ~ExportDialog() override;
};


#endif // INCLUDED_SVTOOLS_SOURCE_FILTER_EXPORTDIALOG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
