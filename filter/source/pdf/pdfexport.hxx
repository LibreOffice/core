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

#pragma once

#include <tools/multisel.hxx>
#include <vcl/pdfwriter.hxx>
#include <vcl/pdfextoutdevdata.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/view/XRenderable.hpp>

class GDIMetaFile;
class Size;

namespace vcl { class PDFWriter; }

class PDFExport
{
private:

    css::uno::Reference< css::lang::XComponent > mxSrcDoc;
    css::uno::Reference< css::uno::XComponentContext > mxContext;
    css::uno::Reference< css::task::XStatusIndicator > mxStatusIndicator;
    css::uno::Reference< css::task::XInteractionHandler > mxIH;

    bool                mbUseTaggedPDF;
    sal_Int32           mnPDFTypeSelection;
    bool                mbPDFUACompliance;
    bool                mbExportNotes;
    bool                mbExportPlaceholders;
    bool                mbUseReferenceXObject;
    bool                mbExportNotesPages;
    bool                mbExportOnlyNotesPages;
    bool                mbUseTransitionEffects;
    bool                mbExportBookmarks;
    bool                mbExportHiddenSlides;
    bool                mbSinglePageSheets;
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
    css::uno::Reference< css::security::XCertificate > maSignCertificate;
    OUString            msSignTSA;

    void                ImplWriteWatermark( vcl::PDFWriter& rWriter, const Size& rPageSize );
    void                ImplWriteTiledWatermark( vcl::PDFWriter& rWriter, const Size& rPageSize );
    css::uno::Reference<css::security::XCertificate> GetCertificateFromSubjectName(const std::u16string_view& rSubjectName) const;


public:

                        PDFExport(  const css::uno::Reference< css::lang::XComponent >& rxSrcDoc,
                                    const css::uno::Reference< css::task::XStatusIndicator >& xStatusIndicator,
                                    const css::uno::Reference< css::task::XInteractionHandler >& xIH,
                                    const css::uno::Reference< css::uno::XComponentContext >& xFact );
                        ~PDFExport();

    bool                ExportSelection( vcl::PDFWriter& rPDFWriter,
                                    css::uno::Reference< css::view::XRenderable > const & rRenderable,
                                    const css::uno::Any& rSelection,
                                    const StringRangeEnumerator& rRangeEnum,
                                    css::uno::Sequence< css::beans::PropertyValue >& rRenderOptions,
                                    sal_Int32 nPageCount );

    bool                Export( const OUString& rFile, const css::uno::Sequence< css::beans::PropertyValue >& rFilterData );

    void                showErrors( const std::set<vcl::PDFWriter::ErrorCode>& );
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
