/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef PDFEXPORT_HXX
#define PDFEXPORT_HXX

#include "pdffilter.hxx"
#include <tools/multisel.hxx>
#include <vcl/pdfwriter.hxx>
#include <vcl/pdfextoutdevdata.hxx>
#include <com/sun/star/view/XRenderable.hpp>

class SvEmbeddedObject;
class GDIMetaFile;
class VirtualDevice;
class PolyPolygon;
class Gradient;
class BitmapEx;
class Point;
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
    sal_Int32               mnCachePatternId;

//--->i56629
    sal_Bool                mbExportRelativeFsysLinks;
    sal_Int32               mnDefaultLinkAction;
    sal_Bool                mbConvertOOoTargetToPDFTarget;
    sal_Bool                mbExportBmkToDest;
//<---
    sal_Bool                ImplExportPage( ::vcl::PDFWriter& rWriter, ::vcl::PDFExtOutDevData& rPDFExtOutDevData,
                                                const GDIMetaFile& rMtf );
    void                    ImplWriteWatermark( ::vcl::PDFWriter& rWriter, const Size& rPageSize );
public:

                            PDFExport( const Reference< XComponent >& rxSrcDoc,
                                       const Reference< task::XStatusIndicator >& xStatusIndicator,
                                      const Reference< task::XInteractionHandler >& xIH,
                                       const Reference< lang::XMultiServiceFactory >& xFact );
                            ~PDFExport();

    sal_Bool                ExportSelection( vcl::PDFWriter& rPDFWriter, Reference< com::sun::star::view::XRenderable >& rRenderable, Any& rSelection,
                                MultiSelection aMultiSelection, Sequence< PropertyValue >& rRenderOptions, sal_Int32 nPageCount );

    sal_Bool                Export( const OUString& rFile, const Sequence< PropertyValue >& rFilterData );

    void                    showErrors( const std::set<vcl::PDFWriter::ErrorCode>& );
};

#endif
