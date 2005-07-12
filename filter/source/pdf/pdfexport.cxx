/*************************************************************************
 *
 *  $RCSfile: pdfexport.cxx,v $
 *
 *  $Revision: 1.38 $
 *
 *  last change: $Author: kz $ $Date: 2005-07-12 13:47:26 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "pdfexport.hxx"

#include "pdf.hrc"
#include <tools/urlobj.hxx>
#include <tools/fract.hxx>
#include <tools/poly.hxx>
#include <vcl/mapmod.hxx>
#include <vcl/virdev.hxx>
#include <vcl/metaact.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/jobset.hxx>
#include <vcl/salbtype.hxx>
#include <vcl/bmpacc.hxx>
#ifndef _SV_SVAPP_HXX
#include "vcl/svapp.hxx"
#endif
#include <toolkit/awt/vclxdevice.hxx>
#include <unotools/localfilehelper.hxx>
#include <unotools/processfactory.hxx>
#include <svtools/FilterConfigItem.hxx>
#include <svtools/filter.hxx>
#include <svtools/solar.hrc>
#include <svtools/graphictools.hxx>
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_RECTANGLE_HPP_
#include <com/sun/star/awt/Rectangle.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XDEVICE_HPP_
#include <com/sun/star/awt/XDevice.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_MEASUREUNIT_HPP_
#include <com/sun/star/util/MeasureUnit.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XDOCUMENTINFO_HPP_
#include <com/sun/star/document/XDocumentInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XDOCUMENTINFOSUPPLIER_HPP_
#include <com/sun/star/document/XDocumentInfoSupplier.hpp>
#endif
#ifndef _UTL_CONFIGMGR_HXX_
#include <unotools/configmgr.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XSHAPES_HPP_
#include <com/sun/star/drawing/XShapes.hpp>
#endif

using namespace ::rtl;
using namespace ::vcl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::view;

sal_Bool GetPropertyValue( Any& rAny, const Reference< XPropertySet > & rXPropSet, const sal_Char* pName )
{
    sal_Bool bRetValue = sal_True;
    try
    {
        rAny = rXPropSet->getPropertyValue( String::CreateFromAscii( pName ) );
        if ( !rAny.hasValue() )
            bRetValue = sal_False;
    }
    catch( ::com::sun::star::uno::Exception& )
    {
        bRetValue = sal_False;
    }
    return bRetValue;
}

OUString GetProperty( const Reference< XPropertySet > & rXPropSet, const sal_Char* pName )
{
    OUString aRetValue;
    Any aAny;
    if ( GetPropertyValue( aAny, rXPropSet, pName ) )
        aAny >>= aRetValue;
    return aRetValue;
}

// -------------
// - PDFExport -
// -------------

PDFExport::PDFExport( const Reference< XComponent >& rxSrcDoc, Reference< task::XStatusIndicator >& rxStatusIndicator ) :
    mxSrcDoc                ( rxSrcDoc ),
    mxStatusIndicator       ( rxStatusIndicator ),
    mbUseTaggedPDF          ( sal_False ),
    mbExportNotes           ( sal_True ),
    mbExportNotesPages      ( sal_False ),
    mbUseTransitionEffects  ( sal_True ),
    mbUseLosslessCompression( sal_False ),
    mbReduceImageResolution ( sal_False ),
    mbSkipEmptyPages        ( sal_False ),
    mnMaxImageResolution    ( 300 ),
    mnQuality               ( 90 ),
    mnFormsFormat           ( 0 ),
    mnProgressValue         ( 0 )
{
}

// -----------------------------------------------------------------------------

PDFExport::~PDFExport()
{
}

// -----------------------------------------------------------------------------

sal_Bool PDFExport::ExportSelection( vcl::PDFWriter& rPDFWriter, Reference< com::sun::star::view::XRenderable >& rRenderable, Any& rSelection,
    MultiSelection aMultiSelection, Sequence< PropertyValue >& rRenderOptions )
{
    sal_Bool        bRet = sal_False;
    try
    {
        Any* pFirstPage = NULL;
        Any* pLastPage = NULL;

        for( sal_Int32 nData = 0, nDataCount = rRenderOptions.getLength(); nData < nDataCount; ++nData )
        {
            if( rRenderOptions[ nData ].Name == OUString( RTL_CONSTASCII_USTRINGPARAM( "FirstPage" ) ) )
                pFirstPage = &rRenderOptions[ nData ].Value;
            else if( rRenderOptions[ nData ].Name == OUString( RTL_CONSTASCII_USTRINGPARAM( "LastPage" ) ) )
                pLastPage = &rRenderOptions[ nData ].Value;
        }

        OutputDevice* pOut = rPDFWriter.GetReferenceDevice();
        sal_Int32 nPageCount = rRenderable->getRendererCount( rSelection, rRenderOptions );

        if( pOut )
        {
            vcl::PDFExtOutDevData* pPDFExtOutDevData = PTR_CAST( vcl::PDFExtOutDevData, pOut->GetExtOutDevData() );
            if ( nPageCount )
            {
                sal_Int32 nSel = aMultiSelection.FirstSelected();
                while ( nSel != sal_Int32(SFX_ENDOFSELECTION) )
                {
                    Sequence< PropertyValue >   aRenderer( rRenderable->getRenderer( nSel - 1, rSelection, rRenderOptions ) );
                    awt::Size                   aPageSize;

                    for( sal_Int32 nProperty = 0, nPropertyCount = aRenderer.getLength(); nProperty < nPropertyCount; ++nProperty )
                    {
                        if( aRenderer[ nProperty ].Name == OUString( RTL_CONSTASCII_USTRINGPARAM( "PageSize" ) ) )
                            aRenderer[ nProperty].Value >>= aPageSize;
                    }

                    pPDFExtOutDevData->SetCurrentPageNumber( nSel - 1 );

                    GDIMetaFile                 aMtf;
                    const MapMode               aMapMode( MAP_100TH_MM );
                    const Size                  aMtfSize( aPageSize.Width, aPageSize.Height );

                    pOut->Push();
                    pOut->EnableOutput( FALSE );
                    pOut->SetMapMode( aMapMode );

                    aMtf.SetPrefSize( aMtfSize );
                    aMtf.SetPrefMapMode( aMapMode );
                    aMtf.Record( pOut );

                    // --> FME 2004-10-08 #i35176#
                    // LastPage property.
                    const sal_Int32 nCurrentRenderer = nSel - 1;
                    nSel = aMultiSelection.NextSelected();
                    if ( pLastPage && sal_Int32(SFX_ENDOFSELECTION) == nSel )
                        *pLastPage <<= sal_True;
                    // <--

                    rRenderable->render( nCurrentRenderer, rSelection, rRenderOptions );

                    aMtf.Stop();
                    aMtf.WindStart();

                    if( aMtf.GetActionCount() &&
                             ( !mbSkipEmptyPages || aPageSize.Width || aPageSize.Height ) )
                        bRet = ImplExportPage( rPDFWriter, *pPDFExtOutDevData, aMtf ) || bRet;

                    pOut->Pop();

                    if ( mxStatusIndicator.is() )
                        mxStatusIndicator->setValue( mnProgressValue );
                    if ( pFirstPage )
                        *pFirstPage <<= sal_False;

                    ++mnProgressValue;
                }
            }
            else
            {
                bRet = sal_True;                        // #i18334# SJ: nPageCount == 0,
                rPDFWriter.NewPage( 10000, 10000 );     // creating dummy page
                rPDFWriter.SetMapMode( MAP_100TH_MM );  //
            }
        }
    }
    catch( RuntimeException )
    {
    }
    return bRet;
}

sal_Bool PDFExport::Export( const OUString& rFile, const Sequence< PropertyValue >& rFilterData )
{
    INetURLObject   aURL( rFile );
    OUString        aFile;
    sal_Bool        bRet = sal_False;

    if( aURL.GetProtocol() != INET_PROT_FILE )
    {
        String aTmp;

        if( ::utl::LocalFileHelper::ConvertPhysicalNameToURL( rFile, aTmp ) )
            aURL = INetURLObject(aTmp);
    }

    if( aURL.GetProtocol() == INET_PROT_FILE )
    {
        Reference< XRenderable > xRenderable( mxSrcDoc, UNO_QUERY );

        if( xRenderable.is() )
        {
            VCLXDevice*                 pXDevice = new VCLXDevice;
            OUString                    aPageRange;
            Any                         aSelection;

            // getting the string for the creator
            String aCreator;
            Reference< XServiceInfo > xInfo( mxSrcDoc, UNO_QUERY );
            if ( xInfo.is() )
            {
                if ( xInfo->supportsService( rtl::OUString::createFromAscii( "com.sun.star.presentation.PresentationDocument" ) ) )
                    aCreator.AppendAscii( "Impress" );
                else if ( xInfo->supportsService( rtl::OUString::createFromAscii( "com.sun.star.drawing.DrawingDocument" ) ) )
                    aCreator.AppendAscii( "Draw" );
                else if ( xInfo->supportsService( rtl::OUString::createFromAscii( "com.sun.star.text.TextDocument" ) ) )
                    aCreator.AppendAscii( "Writer" );
                else if ( xInfo->supportsService( rtl::OUString::createFromAscii( "com.sun.star.sheet.SpreadsheetDocument" ) ) )
                    aCreator.AppendAscii( "Calc" );
                else if ( xInfo->supportsService( rtl::OUString::createFromAscii( "com.sun.star.formula.FormulaProperties" ) ) )
                    aCreator.AppendAscii( "Math" );
            }

            for( sal_Int32 nData = 0, nDataCount = rFilterData.getLength(); nData < nDataCount; ++nData )
            {
                if( rFilterData[ nData ].Name == OUString( RTL_CONSTASCII_USTRINGPARAM( "PageRange" ) ) )
                    rFilterData[ nData ].Value >>= aPageRange;
                else if( rFilterData[ nData ].Name == OUString( RTL_CONSTASCII_USTRINGPARAM( "Selection" ) ) )
                    rFilterData[ nData ].Value >>= aSelection;
                else if ( rFilterData[ nData ].Name == OUString( RTL_CONSTASCII_USTRINGPARAM( "UseLosslessCompression" ) ) )
                    rFilterData[ nData ].Value >>= mbUseLosslessCompression;
                else if ( rFilterData[ nData ].Name == OUString( RTL_CONSTASCII_USTRINGPARAM( "Quality" ) ) )
                    rFilterData[ nData ].Value >>= mnQuality;
                else if ( rFilterData[ nData ].Name == OUString( RTL_CONSTASCII_USTRINGPARAM( "ReduceImageResolution" ) ) )
                    rFilterData[ nData ].Value >>= mbReduceImageResolution;
                else if ( rFilterData[ nData ].Name == OUString( RTL_CONSTASCII_USTRINGPARAM( "SkipEmptyPages" ) ) )
                    rFilterData[ nData ].Value >>= mbSkipEmptyPages;
                else if ( rFilterData[ nData ].Name == OUString( RTL_CONSTASCII_USTRINGPARAM( "MaxImageResolution" ) ) )
                    rFilterData[ nData ].Value >>= mnMaxImageResolution;
                else if ( rFilterData[ nData ].Name == OUString( RTL_CONSTASCII_USTRINGPARAM( "UseTaggedPDF" ) ) )
                    rFilterData[ nData ].Value >>= mbUseTaggedPDF;
                else if ( rFilterData[ nData ].Name == OUString( RTL_CONSTASCII_USTRINGPARAM( "ExportNotes" ) ) )
                    rFilterData[ nData ].Value >>= mbExportNotes;
                else if ( rFilterData[ nData ].Name == OUString( RTL_CONSTASCII_USTRINGPARAM( "ExportNotesPages" ) ) )
                    rFilterData[ nData ].Value >>= mbExportNotesPages;
                else if ( rFilterData[ nData ].Name == OUString( RTL_CONSTASCII_USTRINGPARAM( "UseTransitionEffects" ) ) )
                    rFilterData[ nData ].Value >>= mbUseTransitionEffects;
                else if ( rFilterData[ nData ].Name == OUString( RTL_CONSTASCII_USTRINGPARAM( "FormsFormat" ) ) )
                    rFilterData[ nData ].Value >>= mnFormsFormat;
            }
            PDFWriter::PDFWriterContext aContext;
            aContext.URL        = aURL.GetMainURL(INetURLObject::DECODE_TO_IURI);
            aContext.Version    = PDFWriter::PDF_1_4;
            aContext.Tagged     = mbUseTaggedPDF;
            PDFWriter*          pPDFWriter = new PDFWriter( aContext );
            OutputDevice*       pOut = pPDFWriter->GetReferenceDevice();
            vcl::PDFExtOutDevData* pPDFExtOutDevData = NULL;

            DBG_ASSERT( pOut, "PDFExport::Export: no reference device" );
            pXDevice->SetOutputDevice( pOut );

            if ( pOut )
            {
                DBG_ASSERT( pOut->GetExtOutDevData() == NULL, "PDFExport: ExtOutDevData already set!!!" );
                pPDFExtOutDevData = new vcl::PDFExtOutDevData( *pOut );
                pOut->SetExtOutDevData( pPDFExtOutDevData );
                pPDFExtOutDevData->SetIsExportNotes( mbExportNotes );
                pPDFExtOutDevData->SetIsExportTaggedPDF( mbUseTaggedPDF );
                pPDFExtOutDevData->SetIsExportTransitionEffects( mbUseTransitionEffects );
                pPDFExtOutDevData->SetFormsFormat( mnFormsFormat );
                pPDFExtOutDevData->SetIsLosslessCompression( mbUseLosslessCompression );
                pPDFExtOutDevData->SetIsReduceImageResolution( mbReduceImageResolution );

                Sequence< PropertyValue > aRenderOptions( 6 );
                aRenderOptions[ 0 ].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "RenderDevice" ) );
                aRenderOptions[ 0 ].Value <<= Reference< awt::XDevice >( pXDevice );
                aRenderOptions[ 1 ].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "ExportNotesPages" ) );
                aRenderOptions[ 1 ].Value <<= sal_False;
                Any& rExportNotesValue = aRenderOptions[ 1 ].Value;
                aRenderOptions[ 2 ].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "FirstPage" ) );
                aRenderOptions[ 2 ].Value <<= sal_True;
                aRenderOptions[ 3 ].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "LastPage" ) );
                aRenderOptions[ 3 ].Value <<= sal_False;
                aRenderOptions[ 4 ].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "PageRange" ) );
                aRenderOptions[ 4 ].Value <<= aPageRange;
                aRenderOptions[ 5 ].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "SkipEmptyPages" ) );
                aRenderOptions[ 5 ].Value <<= mbSkipEmptyPages;

                if( aPageRange.getLength() || !aSelection.hasValue() )
                {
                    aSelection = Any();
                    aSelection <<= mxSrcDoc;
                }
                sal_Bool        bSecondPassForImpressNotes = sal_False;
                sal_Int32       nPageCount = xRenderable->getRendererCount( aSelection, aRenderOptions );
                const Range     aRange( 1, nPageCount );
                MultiSelection  aMultiSelection;

                if ( mbExportNotesPages && aCreator.EqualsAscii( "Impress" ) )
                {
                    uno::Reference< drawing::XShapes > xShapes;     // sj: do not allow to export notes when
                    if ( ! ( aSelection >>= xShapes ) )             // exporting a selection -> todo: in the dialog
                        bSecondPassForImpressNotes = sal_True;      // the export notes checkbox needs to be disabled
                }

                if( !aPageRange.getLength() )
                {
                    aMultiSelection.SetTotalRange( aRange );
                    aMultiSelection.Select( aRange );
                }
                else
                {
                    aMultiSelection = MultiSelection( aPageRange );
                    aMultiSelection.SetTotalRange( aRange );
                }
                if ( mxStatusIndicator.is() )
                {
                    ByteString aResMgrName( "pdffilter" );
                    aResMgrName.Append( ByteString::CreateFromInt32( SOLARUPD ) );
                    ResMgr* pResMgr = ResMgr::CreateResMgr( aResMgrName.GetBuffer(), Application::GetSettings().GetUILocale() );
                    if ( pResMgr )
                    {
                        sal_Int32 nTotalPageCount = aMultiSelection.GetSelectCount();
                        if ( bSecondPassForImpressNotes )
                            nTotalPageCount *= 2;
                        mxStatusIndicator->start( String( ResId( PDF_PROGRESS_BAR, pResMgr ) ), nTotalPageCount );
                        delete pResMgr;
                    }
                }
                bRet = ExportSelection( *pPDFWriter, xRenderable, aSelection, aMultiSelection, aRenderOptions );
                if ( bRet && bSecondPassForImpressNotes )
                {
                    rExportNotesValue <<= sal_True;
                    bRet = ExportSelection( *pPDFWriter, xRenderable, aSelection, aMultiSelection, aRenderOptions );
                }
                if ( mxStatusIndicator.is() )
                    mxStatusIndicator->end();

                if( bRet )
                {
                    PDFDocInfo aDocInfo;
                    Reference< document::XDocumentInfoSupplier > xDocumentInfoSupplier( mxSrcDoc, UNO_QUERY );
                    if ( xDocumentInfoSupplier.is() )
                    {
                        Reference< document::XDocumentInfo > xDocumentInfo( xDocumentInfoSupplier->getDocumentInfo() );
                        if ( xDocumentInfo.is() )
                        {
                            Reference< XPropertySet > xPropSet( xDocumentInfo, UNO_QUERY );
                            if ( xPropSet.is() )
                            {
                                aDocInfo.Title = GetProperty( xPropSet, "Title" );
                                aDocInfo.Author = GetProperty( xPropSet, "Author" );
                                aDocInfo.Subject = GetProperty( xPropSet, "Subject" );
                                aDocInfo.Keywords = GetProperty( xPropSet, "Keywords" );
                            }
                        }
                    }
                    // getting the string for the producer
                    String aProducer;
                    ::utl::ConfigManager* pMgr = ::utl::ConfigManager::GetConfigManager();
                    if ( pMgr )
                    {
                        Any aProductName = pMgr->GetDirectConfigProperty( ::utl::ConfigManager::PRODUCTNAME );
                        ::rtl::OUString sProductName;
                        aProductName >>= sProductName;
                        aProducer = sProductName;
                        aProductName = pMgr->GetDirectConfigProperty( ::utl::ConfigManager::PRODUCTVERSION );
                        aProductName >>= sProductName;
                        aProducer.AppendAscii(" ");
                        aProducer += String( sProductName );
                    }
                    aDocInfo.Producer = aProducer;
                    aDocInfo.Creator = aCreator;

                    pPDFExtOutDevData->PlayGlobalActions( *pPDFWriter );
                    pPDFWriter->SetDocInfo( aDocInfo );
                    pPDFWriter->Emit();
                }
                pOut->SetExtOutDevData( NULL );
            }
            delete pPDFExtOutDevData;
            delete pPDFWriter;
        }
    }
    return bRet;
}

// -----------------------------------------------------------------------------

sal_Bool PDFExport::ImplExportPage( PDFWriter& rWriter, PDFExtOutDevData& rPDFExtOutDevData, const GDIMetaFile& rMtf )
{
    VirtualDevice   aDummyVDev;
    const Size      aSizePDF( OutputDevice::LogicToLogic( rMtf.GetPrefSize(), rMtf.GetPrefMapMode(), MAP_POINT ) );
    Point           aOrigin;
    Rectangle       aPageRect( aOrigin, rMtf.GetPrefSize() );
    sal_Bool        bRet = sal_False;

    aDummyVDev.EnableOutput( sal_False );
    aDummyVDev.SetMapMode( rMtf.GetPrefMapMode() );

    rWriter.NewPage( aSizePDF.Width(), aSizePDF.Height() );
    rWriter.SetMapMode( rMtf.GetPrefMapMode() );

    rWriter.SetClipRegion( aPageRect );
    bRet = ImplWriteActions( rWriter, &rPDFExtOutDevData, rMtf, aDummyVDev );
    rPDFExtOutDevData.ResetSyncData();
    return bRet;
}

// -----------------------------------------------------------------------------

sal_Bool PDFExport::ImplWriteActions( PDFWriter& rWriter, PDFExtOutDevData* pPDFExtOutDevData,
                                        const GDIMetaFile& rMtf, VirtualDevice& rDummyVDev )
{
    bool bAssertionFired( false );

    for( ULONG i = 0, nCount = rMtf.GetActionCount(); i < nCount; )
    {
        if ( !pPDFExtOutDevData || !pPDFExtOutDevData->PlaySyncPageAct( rWriter, i ) )
        {
            const MetaAction*   pAction = rMtf.GetAction( i );
            const USHORT        nType = pAction->GetType();

            switch( nType )
            {
                case( META_PIXEL_ACTION ):
                {
                    const MetaPixelAction* pA = (const MetaPixelAction*) pAction;
                    rWriter.DrawPixel( pA->GetPoint(), pA->GetColor() );
                }
                break;

                case( META_POINT_ACTION ):
                {
                    const MetaPointAction* pA = (const MetaPointAction*) pAction;
                    rWriter.DrawPixel( pA->GetPoint() );
                }
                break;

                case( META_LINE_ACTION ):
                {
                    const MetaLineAction* pA = (const MetaLineAction*) pAction;
                    if ( pA->GetLineInfo().IsDefault() )
                        rWriter.DrawLine( pA->GetStartPoint(), pA->GetEndPoint() );
                    else
                        rWriter.DrawLine( pA->GetStartPoint(), pA->GetEndPoint(), pA->GetLineInfo() );
                }
                break;

                case( META_RECT_ACTION ):
                {
                    const MetaRectAction* pA = (const MetaRectAction*) pAction;
                    rWriter.DrawRect( pA->GetRect() );
                }
                break;

                case( META_ROUNDRECT_ACTION ):
                {
                    const MetaRoundRectAction* pA = (const MetaRoundRectAction*) pAction;
                    rWriter.DrawRect( pA->GetRect(), pA->GetHorzRound(), pA->GetVertRound() );
                }
                break;

                case( META_ELLIPSE_ACTION ):
                {
                    const MetaEllipseAction* pA = (const MetaEllipseAction*) pAction;
                    rWriter.DrawEllipse( pA->GetRect() );
                }
                break;

                case( META_ARC_ACTION ):
                {
                    const MetaArcAction* pA = (const MetaArcAction*) pAction;
                    rWriter.DrawArc( pA->GetRect(), pA->GetStartPoint(), pA->GetEndPoint() );
                }
                break;

                case( META_PIE_ACTION ):
                {
                    const MetaArcAction* pA = (const MetaArcAction*) pAction;
                    rWriter.DrawPie( pA->GetRect(), pA->GetStartPoint(), pA->GetEndPoint() );
                }
                break;

                case( META_CHORD_ACTION ):
                {
                    const MetaChordAction* pA = (const MetaChordAction*) pAction;
                    rWriter.DrawChord( pA->GetRect(), pA->GetStartPoint(), pA->GetEndPoint() );
                }
                break;

                case( META_POLYGON_ACTION ):
                {
                    const MetaPolygonAction* pA = (const MetaPolygonAction*) pAction;
                    rWriter.DrawPolygon( pA->GetPolygon() );
                }
                break;

                case( META_POLYLINE_ACTION ):
                {
                    const MetaPolyLineAction* pA = (const MetaPolyLineAction*) pAction;
                    if ( pA->GetLineInfo().IsDefault() )
                        rWriter.DrawPolyLine( pA->GetPolygon() );
                    else
                        rWriter.DrawPolyLine( pA->GetPolygon(), pA->GetLineInfo() );
                }
                break;

                case( META_POLYPOLYGON_ACTION ):
                {
                    const MetaPolyPolygonAction* pA = (const MetaPolyPolygonAction*) pAction;
                    rWriter.DrawPolyPolygon( pA->GetPolyPolygon() );
                }
                break;

                case( META_GRADIENT_ACTION ):
                {
                    const MetaGradientAction* pA = (const MetaGradientAction*) pAction;
                    const PolyPolygon         aPolyPoly( pA->GetRect() );

                    ImplWriteGradient( rWriter, aPolyPoly, pA->GetGradient(), rDummyVDev );
                }
                break;

                case( META_GRADIENTEX_ACTION ):
                {
                    const MetaGradientExAction* pA = (const MetaGradientExAction*) pAction;
                    ImplWriteGradient( rWriter, pA->GetPolyPolygon(), pA->GetGradient(), rDummyVDev );
                }
                break;

                case META_HATCH_ACTION:
                {
                    const MetaHatchAction*  pA = (const MetaHatchAction*) pAction;
                    rWriter.DrawHatch( pA->GetPolyPolygon(), pA->GetHatch() );
                }
                break;

                case( META_TRANSPARENT_ACTION ):
                {
                    const MetaTransparentAction* pA = (const MetaTransparentAction*) pAction;
                    rWriter.DrawTransparent( pA->GetPolyPolygon(), pA->GetTransparence() );
                }
                break;

                case( META_FLOATTRANSPARENT_ACTION ):
                {
                    const MetaFloatTransparentAction* pA = (const MetaFloatTransparentAction*) pAction;

                    GDIMetaFile     aTmpMtf( pA->GetGDIMetaFile() );
                    const Point&    rPos = pA->GetPoint();
                    const Size&     rSize= pA->GetSize();
                    const Gradient& rTransparenceGradient = pA->GetGradient();

                    const Size  aDstSizeTwip( rDummyVDev.PixelToLogic( rDummyVDev.LogicToPixel( rSize ), MAP_TWIP ) );
                    sal_Int32   nMaxBmpDPI = mbUseLosslessCompression ? 300 : 72;
                    if ( mbReduceImageResolution )
                    {
                        if ( nMaxBmpDPI > mnMaxImageResolution )
                            nMaxBmpDPI = mnMaxImageResolution;
                    }
                    const sal_Int32 nPixelX = (sal_Int32)((double)aDstSizeTwip.Width() * (double)nMaxBmpDPI / 1440.0);
                    const sal_Int32 nPixelY = (sal_Int32)((double)aDstSizeTwip.Height() * (double)nMaxBmpDPI / 1440.0);
                    if ( nPixelX && nPixelY )
                    {
                        Size aDstSizePixel( nPixelX, nPixelY );
                        VirtualDevice* pVDev = new VirtualDevice;
                        if( pVDev->SetOutputSizePixel( aDstSizePixel ) )
                        {
                            Bitmap          aPaint, aMask;
                            AlphaMask       aAlpha;
                            Point           aPoint;

                            MapMode aMapMode( rDummyVDev.GetMapMode() );
                            aMapMode.SetOrigin( aPoint );
                            pVDev->SetMapMode( aMapMode );
                            Size aDstSize( pVDev->PixelToLogic( aDstSizePixel ) );

                            Point   aMtfOrigin( aTmpMtf.GetPrefMapMode().GetOrigin() );
                            if ( aMtfOrigin.X() || aMtfOrigin.Y() )
                                aTmpMtf.Move( -aMtfOrigin.X(), -aMtfOrigin.Y() );
                            double  fScaleX = (double)aDstSize.Width() / (double)aTmpMtf.GetPrefSize().Width();
                            double  fScaleY = (double)aDstSize.Height() / (double)aTmpMtf.GetPrefSize().Height();
                            if( fScaleX != 1.0 || fScaleY != 1.0 )
                                aTmpMtf.Scale( fScaleX, fScaleY );
                            aTmpMtf.SetPrefMapMode( aMapMode );

                            // create paint bitmap
                            aTmpMtf.WindStart();
                            aTmpMtf.Play( pVDev, aPoint, aDstSize );
                            aTmpMtf.WindStart();

                            pVDev->EnableMapMode( FALSE );
                            aPaint = pVDev->GetBitmap( aPoint, aDstSizePixel );
                            pVDev->EnableMapMode( TRUE );

                            // create mask bitmap
                            pVDev->SetLineColor( COL_BLACK );
                            pVDev->SetFillColor( COL_BLACK );
                            pVDev->DrawRect( Rectangle( aPoint, aDstSize ) );
                            pVDev->SetDrawMode( DRAWMODE_WHITELINE | DRAWMODE_WHITEFILL | DRAWMODE_WHITETEXT |
                                                DRAWMODE_WHITEBITMAP | DRAWMODE_WHITEGRADIENT );
                            aTmpMtf.WindStart();
                            aTmpMtf.Play( pVDev, aPoint, aDstSize );
                            aTmpMtf.WindStart();
                            pVDev->EnableMapMode( FALSE );
                            aMask = pVDev->GetBitmap( aPoint, aDstSizePixel );
                            pVDev->EnableMapMode( TRUE );

                            // create alpha mask from gradient
                            pVDev->SetDrawMode( DRAWMODE_GRAYGRADIENT );
                            pVDev->DrawGradient( Rectangle( aPoint, aDstSize ), rTransparenceGradient );
                            pVDev->SetDrawMode( DRAWMODE_DEFAULT );
                            pVDev->EnableMapMode( FALSE );
                            pVDev->DrawMask( aPoint, aDstSizePixel, aMask, Color( COL_WHITE ) );
                            aAlpha = pVDev->GetBitmap( aPoint, aDstSizePixel );
                            ImplWriteBitmapEx( rWriter, rDummyVDev, rPos, rSize, BitmapEx( aPaint, aAlpha ) );
                        }
                        delete pVDev;
                    }
                }
                break;

                case( META_EPS_ACTION ):
                {
                    const MetaEPSAction*    pA = (const MetaEPSAction*) pAction;
                    const GDIMetaFile       aSubstitute( pA->GetSubstitute() );

                    rWriter.Push();
                    rDummyVDev.Push();

                    MapMode aMapMode( aSubstitute.GetPrefMapMode() );
                    Size aOutSize( rDummyVDev.LogicToLogic( pA->GetSize(), rDummyVDev.GetMapMode(), aMapMode ) );
                    aMapMode.SetScaleX( Fraction( aOutSize.Width(), aSubstitute.GetPrefSize().Width() ) );
                    aMapMode.SetScaleY( Fraction( aOutSize.Height(), aSubstitute.GetPrefSize().Height() ) );
                    aMapMode.SetOrigin( rDummyVDev.LogicToLogic( pA->GetPoint(), rDummyVDev.GetMapMode(), aMapMode ) );

                    rWriter.SetMapMode( aMapMode );
                    rDummyVDev.SetMapMode( aMapMode );
                    ImplWriteActions( rWriter, NULL, aSubstitute, rDummyVDev );
                    rDummyVDev.Pop();
                    rWriter.Pop();
                }
                break;

                case( META_COMMENT_ACTION ):
                {
                    const MetaCommentAction*    pA = (const MetaCommentAction*) pAction;
                    String                      aSkipComment;

                    if( pA->GetComment().CompareIgnoreCaseToAscii( "XGRAD_SEQ_BEGIN" ) == COMPARE_EQUAL )
                    {
                        const MetaGradientExAction* pGradAction = NULL;
                        sal_Bool                    bDone = sal_False;

                        while( !bDone && ( ++i < nCount ) )
                        {
                            pAction = rMtf.GetAction( i );

                            if( pAction->GetType() == META_GRADIENTEX_ACTION )
                                pGradAction = (const MetaGradientExAction*) pAction;
                            else if( ( pAction->GetType() == META_COMMENT_ACTION ) &&
                                    ( ( (const MetaCommentAction*) pAction )->GetComment().CompareIgnoreCaseToAscii( "XGRAD_SEQ_END" ) == COMPARE_EQUAL ) )
                            {
                                bDone = sal_True;
                            }
                        }

                        if( pGradAction )
                            ImplWriteGradient( rWriter, pGradAction->GetPolyPolygon(), pGradAction->GetGradient(), rDummyVDev );
                    }
                    else
                    {
                        const BYTE* pData = pA->GetData();
                        if ( pData )
                        {
                            SvMemoryStream  aMemStm( (void*)pData, pA->GetDataSize(), STREAM_READ );
                            sal_Bool        bSkipSequence = sal_False;
                            ByteString      sSeqEnd;

                            if( pA->GetComment().Equals( "XPATHSTROKE_SEQ_BEGIN" ) )
                            {
                                sSeqEnd = ByteString( "XPATHSTROKE_SEQ_END" );
                                SvtGraphicStroke aStroke;
                                aMemStm >> aStroke;

                                Polygon aPath;
                                aStroke.getPath( aPath );

                                PolyPolygon aStartArrow;
                                PolyPolygon aEndArrow;
                                double fTransparency( aStroke.getTransparency() );
                                double fStrokeWidth( aStroke.getStrokeWidth() );
                                SvtGraphicStroke::JoinType eJT( aStroke.getJoinType() );
                                SvtGraphicStroke::DashArray aDashArray;

                                aStroke.getStartArrow( aStartArrow );
                                aStroke.getEndArrow( aEndArrow );
                                aStroke.getDashArray( aDashArray );

                                bSkipSequence = sal_True;
                                if ( aStartArrow.Count() || aEndArrow.Count() )
                                    bSkipSequence = sal_False;
                                if ( (sal_uInt32)eJT > 2 )
                                    bSkipSequence = sal_False;
                                if ( aDashArray.size() && ( fStrokeWidth != 0.0 ) )
                                    bSkipSequence = sal_False;
                                if ( bSkipSequence )
                                {
                                    PDFWriter::ExtLineInfo aInfo;
                                    aInfo.m_fLineWidth      = fStrokeWidth;
                                    aInfo.m_fTransparency   = fTransparency;
                                    aInfo.m_fMiterLimit     = aStroke.getMiterLimit();
                                    switch( aStroke.getCapType() )
                                    {
                                        default:
                                        case SvtGraphicStroke::capButt:   aInfo.m_eCap = PDFWriter::capButt;break;
                                        case SvtGraphicStroke::capRound:  aInfo.m_eCap = PDFWriter::capRound;break;
                                        case SvtGraphicStroke::capSquare: aInfo.m_eCap = PDFWriter::capSquare;break;
                                    }
                                    switch( aStroke.getJoinType() )
                                    {
                                        default:
                                        case SvtGraphicStroke::joinMiter: aInfo.m_eJoin = PDFWriter::joinMiter;break;
                                        case SvtGraphicStroke::joinRound: aInfo.m_eJoin = PDFWriter::joinRound;break;
                                        case SvtGraphicStroke::joinBevel: aInfo.m_eJoin = PDFWriter::joinBevel;break;
                                        case SvtGraphicStroke::joinNone:
                                            aInfo.m_eJoin = PDFWriter::joinMiter;
                                            aInfo.m_fMiterLimit = 0.0;
                                            break;
                                    }
                                    aInfo.m_aDashArray = aDashArray;
                                    rWriter.DrawPolyLine( aPath, aInfo );
                                }
                            }
                            if ( bSkipSequence )
                            {
                                while( ++i < nCount )
                                {
                                    pAction = rMtf.GetAction( i );
                                    if ( pAction->GetType() == META_COMMENT_ACTION )
                                    {
                                        ByteString sComment( ((MetaCommentAction*)pAction)->GetComment() );
                                        if ( sComment.Equals( sSeqEnd ) )
                                            break;
                                    }
                                }
                            }
                        }
                    }
                }
                break;

                case( META_BMP_ACTION ):
                {
                    const MetaBmpAction* pA = (const MetaBmpAction*) pAction;
                    BitmapEx aBitmapEx( pA->GetBitmap() );
                    Size aSize( OutputDevice::LogicToLogic( aBitmapEx.GetPrefSize(),
                            aBitmapEx.GetPrefMapMode(), rDummyVDev.GetMapMode() ) );
                    ImplWriteBitmapEx( rWriter, rDummyVDev, pA->GetPoint(), aSize, aBitmapEx );
                }
                break;

                case( META_BMPSCALE_ACTION ):
                {
                    const MetaBmpScaleAction* pA = (const MetaBmpScaleAction*) pAction;
                    ImplWriteBitmapEx( rWriter, rDummyVDev, pA->GetPoint(), pA->GetSize(), BitmapEx( pA->GetBitmap() ) );
                }
                break;

                case( META_BMPSCALEPART_ACTION ):
                {
                    const MetaBmpScalePartAction* pA = (const MetaBmpScalePartAction*) pAction;
                    BitmapEx aBitmapEx( pA->GetBitmap() );
                    aBitmapEx.Crop( Rectangle( pA->GetSrcPoint(), pA->GetSrcSize() ) );
                    ImplWriteBitmapEx( rWriter, rDummyVDev, pA->GetDestPoint(), pA->GetDestSize(), aBitmapEx );
                }
                break;

                case( META_BMPEX_ACTION ):
                {
                    const MetaBmpExAction*  pA = (const MetaBmpExAction*) pAction;
                    BitmapEx aBitmapEx( pA->GetBitmapEx() );
                    Size aSize( OutputDevice::LogicToLogic( aBitmapEx.GetPrefSize(),
                            aBitmapEx.GetPrefMapMode(), rDummyVDev.GetMapMode() ) );
                    ImplWriteBitmapEx( rWriter, rDummyVDev, pA->GetPoint(), aSize, aBitmapEx );
                }
                break;

                case( META_BMPEXSCALE_ACTION ):
                {
                    const MetaBmpExScaleAction* pA = (const MetaBmpExScaleAction*) pAction;
                    ImplWriteBitmapEx( rWriter, rDummyVDev, pA->GetPoint(), pA->GetSize(), pA->GetBitmapEx() );
                }
                break;

                case( META_BMPEXSCALEPART_ACTION ):
                {
                    const MetaBmpExScalePartAction* pA = (const MetaBmpExScalePartAction*) pAction;
                    BitmapEx aBitmapEx( pA->GetBitmapEx() );
                    aBitmapEx.Crop( Rectangle( pA->GetSrcPoint(), pA->GetSrcSize() ) );
                    ImplWriteBitmapEx( rWriter, rDummyVDev, pA->GetDestPoint(), pA->GetDestSize(), aBitmapEx );
                }
                break;

                case( META_MASK_ACTION ):
                case( META_MASKSCALE_ACTION ):
                case( META_MASKSCALEPART_ACTION ):
                {
                    DBG_ERROR( "MetaMask...Action not supported yet" );
                }
                break;

                case( META_TEXT_ACTION ):
                {
                    const MetaTextAction* pA = (const MetaTextAction*) pAction;
                    rWriter.DrawText( pA->GetPoint(), String( pA->GetText(), pA->GetIndex(), pA->GetLen() ) );
                }
                break;

                case( META_TEXTRECT_ACTION ):
                {
                    const MetaTextRectAction* pA = (const MetaTextRectAction*) pAction;
                    rWriter.DrawText( pA->GetRect(), String( pA->GetText() ), pA->GetStyle() );
                }
                break;

                case( META_TEXTARRAY_ACTION ):
                {
                    const MetaTextArrayAction* pA = (const MetaTextArrayAction*) pAction;
                    rWriter.DrawTextArray( pA->GetPoint(), pA->GetText(), pA->GetDXArray(), pA->GetIndex(), pA->GetLen() );
                }
                break;

                case( META_STRETCHTEXT_ACTION ):
                {
                    const MetaStretchTextAction* pA = (const MetaStretchTextAction*) pAction;
                    rWriter.DrawStretchText( pA->GetPoint(), pA->GetWidth(), pA->GetText(), pA->GetIndex(), pA->GetLen() );
                }
                break;


                case( META_TEXTLINE_ACTION ):
                {
                    const MetaTextLineAction* pA = (const MetaTextLineAction*) pAction;
                    rWriter.DrawTextLine( pA->GetStartPoint(), pA->GetWidth(), pA->GetStrikeout(), pA->GetUnderline() );

                }
                break;

                case( META_CLIPREGION_ACTION ):
                {
                    const MetaClipRegionAction* pA = (const MetaClipRegionAction*) pAction;

                    if( pA->IsClipping() )
                        rWriter.SetClipRegion( pA->GetRegion() );
                    else
                        rWriter.SetClipRegion();
                }
                break;

                case( META_ISECTRECTCLIPREGION_ACTION ):
                {
                    const MetaISectRectClipRegionAction* pA = (const MetaISectRectClipRegionAction*) pAction;
                    rWriter.IntersectClipRegion( pA->GetRect() );
                }
                break;

                case( META_ISECTREGIONCLIPREGION_ACTION ):
                {
                const MetaISectRegionClipRegionAction* pA = (const MetaISectRegionClipRegionAction*) pAction;
                rWriter.IntersectClipRegion( pA->GetRegion() );
                }
                break;

                case( META_MOVECLIPREGION_ACTION ):
                {
                    const MetaMoveClipRegionAction* pA = (const MetaMoveClipRegionAction*) pAction;
                    rWriter.MoveClipRegion( pA->GetHorzMove(), pA->GetVertMove() );
                }
                break;

                case( META_MAPMODE_ACTION ):
                {
                    const_cast< MetaAction* >( pAction )->Execute( &rDummyVDev );
                    rWriter.SetMapMode( rDummyVDev.GetMapMode() );
                }
                break;

                case( META_LINECOLOR_ACTION ):
                {
                    const MetaLineColorAction* pA = (const MetaLineColorAction*) pAction;

                    if( pA->IsSetting() )
                        rWriter.SetLineColor( pA->GetColor() );
                    else
                        rWriter.SetLineColor();
                }
                break;

                case( META_FILLCOLOR_ACTION ):
                {
                    const MetaFillColorAction* pA = (const MetaFillColorAction*) pAction;

                    if( pA->IsSetting() )
                        rWriter.SetFillColor( pA->GetColor() );
                    else
                        rWriter.SetFillColor();
                }
                break;

                case( META_TEXTLINECOLOR_ACTION ):
                {
                    const MetaTextLineColorAction* pA = (const MetaTextLineColorAction*) pAction;

                    if( pA->IsSetting() )
                        rWriter.SetTextLineColor( pA->GetColor() );
                    else
                        rWriter.SetTextLineColor();
                }
                break;

                case( META_TEXTFILLCOLOR_ACTION ):
                {
                    const MetaTextFillColorAction* pA = (const MetaTextFillColorAction*) pAction;

                    if( pA->IsSetting() )
                        rWriter.SetTextFillColor( pA->GetColor() );
                    else
                        rWriter.SetTextFillColor();
                }
                break;

                case( META_TEXTCOLOR_ACTION ):
                {
                    const MetaTextColorAction* pA = (const MetaTextColorAction*) pAction;
                    rWriter.SetTextColor( pA->GetColor() );
                }
                break;

                case( META_TEXTALIGN_ACTION ):
                {
                    const MetaTextAlignAction* pA = (const MetaTextAlignAction*) pAction;
                    rWriter.SetTextAlign( pA->GetTextAlign() );
                }
                break;

                case( META_FONT_ACTION ):
                {
                    const MetaFontAction* pA = (const MetaFontAction*) pAction;
                    rWriter.SetFont( pA->GetFont() );
                }
                break;

                case( META_PUSH_ACTION ):
                {
                    const MetaPushAction* pA = (const MetaPushAction*) pAction;

                    rDummyVDev.Push( pA->GetFlags() );
                    rWriter.Push( pA->GetFlags() );
                }
                break;

                case( META_POP_ACTION ):
                {
                    rDummyVDev.Pop();
                    rWriter.Pop();
                }
                break;

                case( META_LAYOUTMODE_ACTION ):
                {
                    const MetaLayoutModeAction* pA = (const MetaLayoutModeAction*) pAction;
                    rWriter.SetLayoutMode( pA->GetLayoutMode() );
                }
                break;

                case( META_WALLPAPER_ACTION ):
                {
                    const MetaWallpaperAction* pA = (const MetaWallpaperAction*) pAction;
                    rWriter.DrawWallpaper( pA->GetRect(), pA->GetWallpaper() );
                }
                break;

                case( META_RASTEROP_ACTION ):
                {
                    // !!! >>> we don't want to support this actions
                }
                break;

                case( META_REFPOINT_ACTION ):
                {
                    // !!! >>> we don't want to support this actions
                }
                break;

                case META_TEXTLANGUAGE_ACTION:
                {
                    // TODO: Implement me for tagged PDF
                }
                break;

                default:
                    // #i24604# Made assertion fire only once per
                    // metafile. The asserted actions here are all
                    // deprecated
                    if( !bAssertionFired )
                    {
                        bAssertionFired = true;
                        DBG_ERROR( "PDFExport::ImplWriteActions: deprecated and unsupported MetaAction encountered" );
                    }
                break;
            }
            i++;
        }
    }

    return sal_True;
}

// -----------------------------------------------------------------------------

void PDFExport::ImplWriteGradient( PDFWriter& rWriter, const PolyPolygon& rPolyPoly, const Gradient& rGradient, VirtualDevice& rDummyVDev )
{
    GDIMetaFile aTmpMtf;

    rDummyVDev.AddGradientActions( rPolyPoly.GetBoundRect(), rGradient, aTmpMtf );

    rWriter.Push();
    rWriter.IntersectClipRegion( rPolyPoly );
    ImplWriteActions( rWriter, NULL, aTmpMtf, rDummyVDev );
    rWriter.Pop();
}

// -----------------------------------------------------------------------------

void PDFExport::ImplWriteBitmapEx( PDFWriter& rWriter, VirtualDevice& rDummyVDev,
                                  const Point& rPoint, const Size& rSize, const BitmapEx& rBitmapEx )
{
    if ( !rBitmapEx.IsEmpty() && rSize.Width() && rSize.Height() )
    {
        BitmapEx        aBitmapEx( rBitmapEx );
        Point           aPoint( rPoint );
        Size            aSize( rSize );

        // #i19065# Negative sizes have mirror semantics on
        // OutputDevice. BitmapEx and co. have no idea about that, so
        // perform that _before_ doing anything with aBitmapEx.
        ULONG nMirrorFlags(BMP_MIRROR_NONE);
        if( aSize.Width() < 0 )
        {
            aSize.Width() *= -1;
            aPoint.X() -= aSize.Width();
            nMirrorFlags |= BMP_MIRROR_HORZ;
        }
        if( aSize.Height() < 0 )
        {
            aSize.Height() *= -1;
            aPoint.Y() -= aSize.Height();
            nMirrorFlags |= BMP_MIRROR_VERT;
        }

        if( nMirrorFlags != BMP_MIRROR_NONE )
        {
            aBitmapEx.Mirror( nMirrorFlags );
        }
        if ( mbReduceImageResolution )
        {
            // do downsampling if neccessary
            const Size      aDstSizeTwip( rDummyVDev.PixelToLogic( rDummyVDev.LogicToPixel( aSize ), MAP_TWIP ) );
            const Size      aBmpSize( aBitmapEx.GetSizePixel() );
            const double    fBmpPixelX = aBmpSize.Width();
            const double    fBmpPixelY = aBmpSize.Height();
            const double    fMaxPixelX = aDstSizeTwip.Width() * mnMaxImageResolution / 1440.0;
            const double    fMaxPixelY = aDstSizeTwip.Height() * mnMaxImageResolution / 1440.0;

            // check, if the bitmap DPI exceeds the maximum DPI (allow 4 pixel rounding tolerance)
            if( ( ( fBmpPixelX > ( fMaxPixelX + 4 ) ) ||
                ( fBmpPixelY > ( fMaxPixelY + 4 ) ) ) &&
                ( fBmpPixelY > 0.0 ) && ( fMaxPixelY > 0.0 ) )
            {
                // do scaling
                Size            aNewBmpSize;
                const double    fBmpWH = fBmpPixelX / fBmpPixelY;
                const double    fMaxWH = fMaxPixelX / fMaxPixelY;

                if( fBmpWH < fMaxWH )
                {
                    aNewBmpSize.Width() = FRound( fMaxPixelY * fBmpWH );
                    aNewBmpSize.Height() = FRound( fMaxPixelY );
                }
                else if( fBmpWH > 0.0 )
                {
                    aNewBmpSize.Width() = FRound( fMaxPixelX );
                    aNewBmpSize.Height() = FRound( fMaxPixelX / fBmpWH);
                }
                if( aNewBmpSize.Width() && aNewBmpSize.Height() )
                    aBitmapEx.Scale( aNewBmpSize );
                else
                    aBitmapEx.SetEmpty();
            }
        }

        const Size aSizePixel( aBitmapEx.GetSizePixel() );
        if ( aSizePixel.Width() && aSizePixel.Height() )
        {
            sal_Bool bUseJPGCompression = !mbUseLosslessCompression;
            if ( ( aSizePixel.Width() < 32 ) || ( aSizePixel.Height() < 32 ) )
                bUseJPGCompression = sal_False;

            SvMemoryStream  aStrm;
            Bitmap          aMask;

            if ( bUseJPGCompression )
            {
                sal_uInt32 nZippedFileSize;     // sj: we will calculate the filesize of a zipped bitmap
                {                               // to determine if jpeg compression is usefull
                    SvMemoryStream aTemp;
                    aTemp.SetCompressMode( aTemp.GetCompressMode() | COMPRESSMODE_ZBITMAP );
                    aTemp.SetVersion( SOFFICE_FILEFORMAT_40 );  // sj: up from version 40 our bitmap stream operator
                    aTemp << aBitmapEx;                         // is capable of zlib stream compression
                    aTemp.Seek( STREAM_SEEK_TO_END );
                    nZippedFileSize = aTemp.Tell();
                }
                if ( aBitmapEx.IsTransparent() )
                {
                    if ( aBitmapEx.IsAlpha() )
                        aMask = aBitmapEx.GetAlpha().GetBitmap();
                    else
                        aMask = aBitmapEx.GetMask();
                }
                GraphicFilter   aGraphicFilter;
                Graphic         aGraphic( aBitmapEx.GetBitmap() );
                sal_uInt16      nFormatName = aGraphicFilter.GetExportFormatNumberForShortName( OUString( RTL_CONSTASCII_USTRINGPARAM( "JPG" ) ) );
                sal_Int32       nColorMode = 0;

                Sequence< PropertyValue > aFilterData( 2 );
                aFilterData[ 0 ].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "Quality" ) );
                aFilterData[ 0 ].Value <<= mnQuality;
                aFilterData[ 1 ].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "ColorMode" ) );
                aFilterData[ 1 ].Value <<= nColorMode;

                /*sal_uInt16 nError =*/ aGraphicFilter.ExportGraphic( aGraphic, String(), aStrm, nFormatName, sal_True, &aFilterData );
                aStrm.Seek( STREAM_SEEK_TO_END );
                if ( aStrm.Tell() > nZippedFileSize )
                    bUseJPGCompression = sal_False;
            }
            if ( bUseJPGCompression )
                rWriter.DrawJPGBitmap( aStrm, aSizePixel, Rectangle( aPoint, aSize ), aMask );
            else if ( aBitmapEx.IsTransparent() )
                rWriter.DrawBitmapEx( aPoint, aSize, aBitmapEx );
            else
                rWriter.DrawBitmap( aPoint, aSize, aBitmapEx.GetBitmap() );
        }
    }
}
