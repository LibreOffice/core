/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pdfexport.hxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:16:59 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef PDFEXPORT_HXX
#define PDFEXPORT_HXX

#include "pdffilter.hxx"
#include <tools/multisel.hxx>
#include <vcl/pdfwriter.hxx>
#include <vcl/pdfextoutdevdata.hxx>
#ifndef _COM_SUN_STAR_VIEW_XRENDERABLE_HPP_
#include <com/sun/star/view/XRenderable.hpp>
#endif

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
    sal_Int32               mnProgressValue;

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
    rtl::OUString           msOpenPassword;
    sal_Bool                mbRestrictPermissions;
    rtl::OUString           msPermissionPassword;
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
    sal_Bool                ImplWriteActions( ::vcl::PDFWriter& rWriter, ::vcl::PDFExtOutDevData* pPDFExtOutDevData,
                                                const GDIMetaFile& rMtf, VirtualDevice& rDummyVDev );
    void                    ImplWriteGradient( ::vcl::PDFWriter& rWriter, const PolyPolygon& rPolyPoly,
                                                const Gradient& rGradient, VirtualDevice& rDummyVDev );
    void                    ImplWriteBitmapEx( ::vcl::PDFWriter& rWriter, VirtualDevice& rDummyVDev,
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
