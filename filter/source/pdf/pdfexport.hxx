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

#ifndef PDFEXPORT_HXX
#define PDFEXPORT_HXX

#include "pdffilter.hxx"
#include <tools/multisel.hxx>
#include <vcl/pdfwriter.hxx>
#include <vcl/pdfextoutdevdata.hxx>
#include <com/sun/star/view/XRenderable.hpp>

class GDIMetaFile;
class Size;

namespace vcl { class PDFWriter; }

// -------------
// - PDFExport -
// -------------

class PDFExport
{
private:

    Reference< XComponent > mxSrcDoc;
    Reference< lang::XMultiServiceFactory > mxMSF;
    Reference< task::XStatusIndicator > mxStatusIndicator;
    Reference< task::XInteractionHandler > mxIH;

    sal_Bool                mbUseTaggedPDF;
    sal_Int32               mnPDFTypeSelection;
    sal_Bool                mbExportNotes;
    sal_Bool                mbExportNotesPages;
    sal_Bool                mbEmbedStandardFonts;
    sal_Bool                mbUseTransitionEffects;
    sal_Bool                mbExportBookmarks;
    sal_Bool                mbExportHiddenSlides;
    sal_Int32               mnOpenBookmarkLevels;

    sal_Bool                mbUseLosslessCompression;
    sal_Bool                mbReduceImageResolution;
    sal_Bool                mbSkipEmptyPages;
    sal_Bool                mbAddStream;
    sal_Int32               mnMaxImageResolution;
    sal_Int32               mnQuality;
    sal_Int32               mnFormsFormat;
    sal_Bool                mbExportFormFields;
    sal_Bool                mbAllowDuplicateFieldNames;
    sal_Int32               mnProgressValue;
    sal_Bool                mbRemoveTransparencies;

    sal_Bool                mbWatermark;
    uno::Any                maWatermark;

//these variable are here only to have a location in filter/pdf to set the default
//to be used by the macro (when the FilterData are set by the macro itself)
    sal_Bool                mbHideViewerToolbar;
    sal_Bool                mbHideViewerMenubar;
    sal_Bool                mbHideViewerWindowControls;
    sal_Bool                mbFitWindow;
    sal_Bool                mbCenterWindow;
    sal_Bool                mbOpenInFullScreenMode;
    sal_Bool                mbDisplayPDFDocumentTitle;
    sal_Int32               mnPDFDocumentMode;
    sal_Int32               mnPDFDocumentAction;
    sal_Int32               mnZoom;
    sal_Int32               mnInitialPage;
    sal_Int32               mnPDFPageLayout;
    sal_Bool                mbFirstPageLeft;

    sal_Bool                mbEncrypt;
    sal_Bool                mbRestrictPermissions;
    sal_Int32               mnPrintAllowed;
    sal_Int32               mnChangesAllowed;
    sal_Bool                mbCanCopyOrExtract;
    sal_Bool                mbCanExtractForAccessibility;

    SvtGraphicFill          maCacheFill;

//--->i56629
    sal_Bool                mbExportRelativeFsysLinks;
    sal_Int32               mnDefaultLinkAction;
    sal_Bool                mbConvertOOoTargetToPDFTarget;
    sal_Bool                mbExportBmkToDest;
    sal_Bool                ImplExportPage( ::vcl::PDFWriter& rWriter, ::vcl::PDFExtOutDevData& rPDFExtOutDevData,
                                                const GDIMetaFile& rMtf );

    sal_Bool                mbSignPDF;
    OUString                msSignLocation;
    OUString                msSignContact;
    OUString                msSignReason;
    OUString                msSignPassword;
    Reference< security::XCertificate > maSignCertificate;

    void                    ImplWriteWatermark( ::vcl::PDFWriter& rWriter, const Size& rPageSize );
public:

                            PDFExport( const Reference< XComponent >& rxSrcDoc,
                                       const Reference< task::XStatusIndicator >& xStatusIndicator,
                                      const Reference< task::XInteractionHandler >& xIH,
                                       const Reference< lang::XMultiServiceFactory >& xFact );
                            ~PDFExport();

    sal_Bool                ExportSelection( vcl::PDFWriter& rPDFWriter,
                                Reference< com::sun::star::view::XRenderable >& rRenderable,
                                const Any& rSelection,
                                const StringRangeEnumerator& rRangeEnum,
                                Sequence< PropertyValue >& rRenderOptions,
                                sal_Int32 nPageCount );

    sal_Bool                Export( const OUString& rFile, const Sequence< PropertyValue >& rFilterData );

    void                    showErrors( const std::set<vcl::PDFWriter::ErrorCode>& );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
