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

namespace vcl {	class PDFWriter; }

// -------------
// - PDFExport -
// -------------

class PDFExport
{
private:

    Reference< XComponent >	mxSrcDoc;
    Reference< lang::XMultiServiceFactory > mxMSF;
    Reference< task::XStatusIndicator > mxStatusIndicator;

    sal_Bool				mbUseTaggedPDF;
    sal_Int32               mnPDFTypeSelection;
    sal_Bool				mbExportNotes;
    sal_Bool				mbExportNotesPages;
    sal_Bool	            mbEmbedStandardFonts;
    sal_Bool				mbUseTransitionEffects;
    sal_Bool                mbExportBookmarks;
    sal_Int32               mnOpenBookmarkLevels;

    sal_Bool				mbUseLosslessCompression;
    sal_Bool				mbReduceImageResolution;
    sal_Bool                mbSkipEmptyPages;
    sal_Bool                mbAddStream;
    sal_Int32				mnMaxImageResolution;
    sal_Int32				mnQuality;
    sal_Int32				mnFormsFormat;
    sal_Bool                mbExportFormFields;
    sal_Bool                mbAllowDuplicateFieldNames;
    sal_Int32				mnProgressValue;
    sal_Bool                mbRemoveTransparencies;
    
    sal_Bool                mbWatermark;
    uno::Any                maWatermark;

//these variable are here only to have a location in filter/pdf to set the default
//to be used by the macro (when the FilterData are set by the macro itself)
    sal_Bool				mbHideViewerToolbar;
    sal_Bool				mbHideViewerMenubar;
    sal_Bool				mbHideViewerWindowControls;
    sal_Bool				mbFitWindow;
    sal_Bool				mbCenterWindow;
    sal_Bool				mbOpenInFullScreenMode;
    sal_Bool				mbDisplayPDFDocumentTitle;
    sal_Int32				mnPDFDocumentMode;
    sal_Int32				mnPDFDocumentAction;
    sal_Int32               mnZoom;
    sal_Int32               mnInitialPage;
    sal_Int32				mnPDFPageLayout;
    sal_Bool				mbFirstPageLeft;

    sal_Bool				mbEncrypt;
    rtl::OUString			msOpenPassword;
    sal_Bool				mbRestrictPermissions;
    rtl::OUString			msPermissionPassword;
    sal_Int32				mnPrintAllowed;
    sal_Int32				mnChangesAllowed;
    sal_Bool				mbCanCopyOrExtract;
    sal_Bool				mbCanExtractForAccessibility;
    
    SvtGraphicFill          maCacheFill;
    sal_Int32               mnCachePatternId;

//--->i56629
    sal_Bool				mbExportRelativeFsysLinks;
    sal_Int32               mnDefaultLinkAction;
    sal_Bool				mbConvertOOoTargetToPDFTarget;
    sal_Bool				mbExportBmkToDest;
//<---
    sal_Bool                ImplExportPage( ::vcl::PDFWriter& rWriter, ::vcl::PDFExtOutDevData& rPDFExtOutDevData,
                                                const GDIMetaFile& rMtf );
    sal_Bool                ImplWriteActions( ::vcl::PDFWriter& rWriter, ::vcl::PDFExtOutDevData* pPDFExtOutDevData,
                                                const GDIMetaFile& rMtf, VirtualDevice& rDummyVDev );
    void                    ImplWriteGradient( ::vcl::PDFWriter& rWriter, const PolyPolygon& rPolyPoly,
                                                const Gradient& rGradient, VirtualDevice& rDummyVDev );
    void					ImplWriteBitmapEx( ::vcl::PDFWriter& rWriter, VirtualDevice& rDummyVDev,
                                                const Point& rPoint, const Size& rSize, const BitmapEx& rBitmap );

    void                    ImplWriteWatermark( ::vcl::PDFWriter& rWriter, const Size& rPageSize );
public:

                            PDFExport( const Reference< XComponent >& rxSrcDoc, Reference< task::XStatusIndicator >& xStatusIndicator, const Reference< lang::XMultiServiceFactory >& xFact );
                            ~PDFExport();

    sal_Bool                ExportSelection( vcl::PDFWriter& rPDFWriter, Reference< com::sun::star::view::XRenderable >& rRenderable, Any& rSelection,
                                MultiSelection aMultiSelection, Sequence< PropertyValue >& rRenderOptions, sal_Int32 nPageCount );

    sal_Bool                Export( const OUString& rFile, const Sequence< PropertyValue >& rFilterData );
    
    void                    showErrors( const std::set<vcl::PDFWriter::ErrorCode>& );
};

#endif
