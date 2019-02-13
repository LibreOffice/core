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

#ifndef INCLUDED_FILTER_SOURCE_PDF_PDFEXPORT_HXX
#define INCLUDED_FILTER_SOURCE_PDF_PDFEXPORT_HXX

#include "pdffilter.hxx"
#include <tools/multisel.hxx>
#include <vcl/pdfwriter.hxx>
#include <vcl/pdfextoutdevdata.hxx>
#include <com/sun/star/view/XRenderable.hpp>

class GDIMetaFile;
class Size;

namespace vcl { class PDFWriter; }


class PDFExport
{
private:

    Reference< XComponent > mxSrcDoc;
    Reference< uno::XComponentContext > mxContext;
    Reference< task::XStatusIndicator > mxStatusIndicator;
    Reference< task::XInteractionHandler > mxIH;

    bool                mbUseTaggedPDF;
    sal_Int32           mnPDFTypeSelection;
    bool                mbExportNotes;
    bool                mbExportPlaceholders;
    bool                mbUseReferenceXObject;
    bool                mbExportNotesPages;
    bool                mbExportOnlyNotesPages;
    bool                mbUseTransitionEffects;
    bool                mbExportBookmarks;
    bool                mbExportHiddenSlides;
    sal_Int32           mnOpenBookmarkLevels;

    bool                mbUseLosslessCompression;
    bool                mbReduceImageResolution;
    bool                mbSkipEmptyPages;
    bool                mbAddStream;
    sal_Int32           mnMaxImageResolution;
    sal_Int32           mnQuality;
    sal_Int32           mnFormsFormat;
    bool                mbExportFormFields;
    bool                mbAllowDuplicateFieldNames;
    sal_Int32           mnProgressValue;
    bool                mbRemoveTransparencies;

    bool                mbIsRedactMode;

    OUString            msWatermark;
    OUString            msTiledWatermark;

    // these variable are here only to have a location in filter/pdf to set the default
    // to be used by the macro (when the FilterData are set by the macro itself)
    bool                mbHideViewerToolbar;
    bool                mbHideViewerMenubar;
    bool                mbHideViewerWindowControls;
    bool                mbFitWindow;
    bool                mbCenterWindow;
    bool                mbOpenInFullScreenMode;
    bool                mbDisplayPDFDocumentTitle;
    sal_Int32           mnPDFDocumentMode;
    sal_Int32           mnPDFDocumentAction;
    sal_Int32           mnZoom;
    sal_Int32           mnInitialPage;
    sal_Int32           mnPDFPageLayout;

    bool                mbEncrypt;
    bool                mbRestrictPermissions;
    sal_Int32           mnPrintAllowed;
    sal_Int32           mnChangesAllowed;
    bool                mbCanCopyOrExtract;
    bool                mbCanExtractForAccessibility;

    // #i56629
    bool                mbExportRelativeFsysLinks;
    sal_Int32           mnDefaultLinkAction;
    bool                mbConvertOOoTargetToPDFTarget;
    bool                mbExportBmkToDest;
    void                ImplExportPage( vcl::PDFWriter& rWriter, vcl::PDFExtOutDevData& rPDFExtOutDevData,
                                        const GDIMetaFile& rMtf );

    bool                mbSignPDF;
    OUString            msSignLocation;
    OUString            msSignContact;
    OUString            msSignReason;
    OUString            msSignPassword;
    Reference< security::XCertificate > maSignCertificate;
    OUString            msSignTSA;

    void                ImplWriteWatermark( vcl::PDFWriter& rWriter, const Size& rPageSize );
    void                ImplWriteTiledWatermark( vcl::PDFWriter& rWriter, const Size& rPageSize );


public:

                        PDFExport(  const Reference< XComponent >& rxSrcDoc,
                                    const Reference< task::XStatusIndicator >& xStatusIndicator,
                                    const Reference< task::XInteractionHandler >& xIH,
                                    const Reference< uno::XComponentContext >& xFact );
                        ~PDFExport();

    bool                ExportSelection( vcl::PDFWriter& rPDFWriter,
                                    Reference< css::view::XRenderable > const & rRenderable,
                                    const Any& rSelection,
                                    const StringRangeEnumerator& rRangeEnum,
                                    Sequence< PropertyValue >& rRenderOptions,
                                    sal_Int32 nPageCount );

    bool                Export( const OUString& rFile, const Sequence< PropertyValue >& rFilterData );

    void                showErrors( const std::set<vcl::PDFWriter::ErrorCode>& );
};

#endif // INCLUDED_FILTER_SOURCE_PDF_PDFEXPORT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
