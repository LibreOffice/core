/*************************************************************************
 *
 *  $RCSfile: pdfexport.cxx,v $
 *
 *  $Revision: 1.23 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-25 17:57:51 $
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
#include "pdf.hrc"
#include "pdfexport.hxx"
#include <tools/urlobj.hxx>
#include <tools/fract.hxx>
#include <tools/multisel.hxx>
#include <vcl/pdfwriter.hxx>
#include <vcl/mapmod.hxx>
#include <vcl/virdev.hxx>
#include <vcl/metaact.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/jobset.hxx>
#include <vcl/poly.hxx>
#include <vcl/salbtype.hxx>
#include <so3/embobj.hxx>
#include <vcl/bmpacc.hxx>
#include <toolkit/awt/vclxdevice.hxx>
#include <unotools/localfilehelper.hxx>
#include <unotools/processfactory.hxx>
#include <svtools/FilterConfigItem.hxx>
#include <svtools/filter.hxx>
#include <svtools/solar.hrc>

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
#ifndef _COM_SUN_STAR_VIEW_XRENDERABLE_HPP_
#include <com/sun/star/view/XRenderable.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XSTATUSINDICATORSUPPLIER_HPP_
#include <com/sun/star/task/XStatusIndicatorSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XSTATUSINDICATORFACTORY_HPP_
#include <com/sun/star/task/XStatusIndicatorFactory.hpp>
#endif

using namespace ::rtl;
using namespace ::vcl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::view;

// -------------
// - PDFExport -
// -------------

PDFExport::PDFExport( const Reference< XComponent >& rxSrcDoc ) :
    mxSrcDoc( rxSrcDoc )
{
}

// -----------------------------------------------------------------------------

PDFExport::~PDFExport()
{
}

// -----------------------------------------------------------------------------

sal_Bool PDFExport::Export( const OUString& rFile, const Sequence< PropertyValue >& rFilterData )
{
    INetURLObject   aURL( rFile );
    OUString        aFile;
    sal_Bool        bRet = sal_False;

    if( aURL.GetProtocol() != INET_PROT_FILE )
    {
        String aTmp;

        if( ::utl::LocalFileHelper::ConvertPhysicalNameToURL( rFile, aTmp ) )
            aURL = aTmp;
    }

    if( aURL.GetProtocol() == INET_PROT_FILE )
    {
        Reference< XRenderable > xRenderable( mxSrcDoc, UNO_QUERY );

        if( xRenderable.is() )
        {
            Sequence< PropertyValue >   aRenderOptions( 1 );
            VCLXDevice*                 pXDevice = new VCLXDevice;
            OUString                    aPageRange;
            Any                         aSelection;
            sal_Int32                   nCompressMode = 0;
            sal_Int32                   nPageCount = 0;
            PDFWriter::Compression      eCompressMode;

            aRenderOptions[ 0 ].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "RenderDevice" ) );
            aRenderOptions[ 0 ].Value <<= Reference< awt::XDevice >( pXDevice );

            for( sal_Int32 nData = 0, nDataCount = rFilterData.getLength(); nData < nDataCount; ++nData )
            {
                if( rFilterData[ nData ].Name == OUString( RTL_CONSTASCII_USTRINGPARAM( "CompressMode" ) ) )
                    rFilterData[ nData ].Value >>= nCompressMode;
                else if( rFilterData[ nData ].Name == OUString( RTL_CONSTASCII_USTRINGPARAM( "PageRange" ) ) )
                    rFilterData[ nData ].Value >>= aPageRange;
                else if( rFilterData[ nData ].Name == OUString( RTL_CONSTASCII_USTRINGPARAM( "Selection" ) ) )
                    rFilterData[ nData ].Value >>= aSelection;
            }

            switch( nCompressMode )
            {
                case( 1 ): eCompressMode = PDFWriter::Print; break;
                case( 2 ): eCompressMode = PDFWriter::Press; break;

                default:
                    eCompressMode = PDFWriter::Screen;
                break;
            }

            PDFWriter*      pPDFWriter = new PDFWriter( aURL.GetMainURL(INetURLObject::DECODE_TO_IURI), PDFWriter::PDF_1_4, eCompressMode );
            OutputDevice*   pOut = pPDFWriter->GetReferenceDevice();

            DBG_ASSERT( pOut, "PDFExport::Export: no reference device" );
            pXDevice->SetOutputDevice( pOut );

            if( aPageRange.getLength() || !aSelection.hasValue() )
            {
                aSelection = Any();
                aSelection <<= mxSrcDoc;
            }

            try
            {
                nPageCount = xRenderable->getRendererCount( aSelection, aRenderOptions );

                if( nPageCount && pOut )
                {
                    const Range     aRange( 1, nPageCount );
                    MultiSelection  aSel;

                    try
                    {
                        if( !aPageRange.getLength() )
                        {
                            aSel.SetTotalRange( aRange );
                            aSel.Select( aRange );
                        }
                        else
                        {
                            aSel = MultiSelection( aPageRange );
                            aSel.SetTotalRange( aRange );
                        }
                        Reference< task::XStatusIndicator > xStatusIndicator;
                        if ( mxSrcDoc.is() )
                        {
                            Reference< frame::XModel > xModel( mxSrcDoc, UNO_QUERY );
                            if ( xModel.is() )
                            {
                                Reference< frame::XController > xController( xModel->getCurrentController());
                                if( xController.is() )
                                {
                                    Reference< frame::XFrame > xFrame( xController->getFrame());
                                    if( xFrame.is() )
                                    {
                                        Reference< task::XStatusIndicatorFactory > xFactory( xFrame, UNO_QUERY );
                                        if( xFactory.is() )
                                        {
                                            ByteString aResMgrName( "pdffilter" );
                                            aResMgrName.Append( ByteString::CreateFromInt32( SOLARUPD ) );
                                            ResMgr* pResMgr = ResMgr::CreateResMgr( aResMgrName.GetBuffer(), Application::GetSettings().GetUILanguage() );
                                            if ( pResMgr )
                                            {
                                                xStatusIndicator = xFactory->createStatusIndicator();
                                                xStatusIndicator->start( String( ResId( PDF_PROGRESS_BAR, pResMgr ) ), aSel.GetSelectCount() );
                                                delete pResMgr;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        sal_Int32 nSel, nProgressValue;
                        for( nSel = aSel.FirstSelected(), nProgressValue = 0; nSel != SFX_ENDOFSELECTION;
                                nSel = aSel.NextSelected(), nProgressValue++ )
                        {
                            Sequence< PropertyValue >   aRenderer( xRenderable->getRenderer( nSel - 1, aSelection, aRenderOptions ) );
                            awt::Size                   aPageSize;
                            sal_Bool                    bProcess = sal_True;

                            for( sal_Int32 nProperty = 0, nPropertyCount = aRenderer.getLength(); nProperty < nPropertyCount; ++nProperty )
                            {
                                if( aRenderer[ nProperty ].Name == OUString( RTL_CONSTASCII_USTRINGPARAM( "PageSize" ) ) )
                                    aRenderer[ nProperty].Value >>= aPageSize;
                            }

                            if( ( aPageSize.Width > 0 ) && ( aPageSize.Height > 0 ) )
                            {
                                GDIMetaFile                 aMtf;
                                const MapMode               aMapMode( MAP_100TH_MM );
                                const Size                  aMtfSize( aPageSize.Width, aPageSize.Height );

                                pOut->Push();
                                pOut->EnableOutput( FALSE );
                                pOut->SetMapMode( aMapMode );

                                aMtf.SetPrefSize( aMtfSize );
                                aMtf.SetPrefMapMode( aMapMode );
                                aMtf.Record( pOut );

                                xRenderable->render( nSel - 1, aSelection, aRenderOptions );

                                aMtf.Stop();
                                aMtf.WindStart();

                                if( aMtf.GetActionCount() )
                                    bRet = ImplExportPage( *pPDFWriter, aMtf, nCompressMode ) || bRet;

                                pOut->Pop();
                            }
                            if ( xStatusIndicator.is() )
                                xStatusIndicator->setValue( nProgressValue );
                        }
                        if ( xStatusIndicator.is() )
                            xStatusIndicator->end();
                    }
                    catch( UnknownPropertyException )
                    {
                    }

                    if( bRet )
                        pPDFWriter->Emit();
                }
            }
            catch( RuntimeException )
            {
            }

            delete pPDFWriter;
        }
    }

    return bRet;
}

// -----------------------------------------------------------------------------

sal_Bool PDFExport::ImplExportPage( PDFWriter& rWriter, const GDIMetaFile& rMtf, sal_Int32 nCompressMode )
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
    bRet = ImplWriteActions( rWriter, rMtf, aDummyVDev, nCompressMode );

    return bRet;
}

// -----------------------------------------------------------------------------

sal_Bool PDFExport::ImplWriteActions( PDFWriter& rWriter, const GDIMetaFile& rMtf, VirtualDevice& rDummyVDev, sal_Int32 nCompressMode )
{
    for( ULONG i = 0, nCount = rMtf.GetActionCount(); i < nCount; i++ )
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
                rWriter.DrawLine( pA->GetStartPoint(), pA->GetEndPoint() );
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
                rWriter.DrawPolyLine( pA->GetPolygon() );
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

                ImplWriteGradient( rWriter, aPolyPoly, pA->GetGradient(), rDummyVDev, nCompressMode );
            }
            break;

            case( META_GRADIENTEX_ACTION ):
            {
                const MetaGradientExAction* pA = (const MetaGradientExAction*) pAction;
                ImplWriteGradient( rWriter, pA->GetPolyPolygon(), pA->GetGradient(), rDummyVDev, nCompressMode );
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
                sal_Int32   nMaxBmpDPI = nCompressMode ? 300 : 72;
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
                        ImplWriteBitmapEx( rWriter, rDummyVDev, nCompressMode, rPos, rSize, BitmapEx( aPaint, aAlpha ) );
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
                ImplWriteActions( rWriter, aSubstitute, rDummyVDev, nCompressMode );
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
                        ImplWriteGradient( rWriter, pGradAction->GetPolyPolygon(), pGradAction->GetGradient(), rDummyVDev, nCompressMode );
                }
            }
            break;

            case( META_BMP_ACTION ):
            {
                const MetaBmpAction* pA = (const MetaBmpAction*) pAction;
                BitmapEx aBitmapEx( pA->GetBitmap() );
                Size aSize( OutputDevice::LogicToLogic( aBitmapEx.GetPrefSize(),
                        aBitmapEx.GetPrefMapMode(), rDummyVDev.GetMapMode() ) );
                ImplWriteBitmapEx( rWriter, rDummyVDev, nCompressMode, pA->GetPoint(), aSize, aBitmapEx );
            }
            break;

            case( META_BMPSCALE_ACTION ):
            {
                const MetaBmpScaleAction* pA = (const MetaBmpScaleAction*) pAction;
                ImplWriteBitmapEx( rWriter, rDummyVDev, nCompressMode , pA->GetPoint(), pA->GetSize(), BitmapEx( pA->GetBitmap() ) );
            }
            break;

            case( META_BMPSCALEPART_ACTION ):
            {
                const MetaBmpScalePartAction* pA = (const MetaBmpScalePartAction*) pAction;
                BitmapEx aBitmapEx( pA->GetBitmap() );
                aBitmapEx.Crop( Rectangle( pA->GetSrcPoint(), pA->GetSrcSize() ) );
                ImplWriteBitmapEx( rWriter, rDummyVDev, nCompressMode, pA->GetDestPoint(), pA->GetDestSize(), aBitmapEx );
            }
            break;

            case( META_BMPEX_ACTION ):
            {
                const MetaBmpExAction*  pA = (const MetaBmpExAction*) pAction;
                BitmapEx aBitmapEx( pA->GetBitmapEx() );
                Size aSize( OutputDevice::LogicToLogic( aBitmapEx.GetPrefSize(),
                        aBitmapEx.GetPrefMapMode(), rDummyVDev.GetMapMode() ) );
                ImplWriteBitmapEx( rWriter, rDummyVDev, nCompressMode, pA->GetPoint(), aSize, aBitmapEx );
            }
            break;

            case( META_BMPEXSCALE_ACTION ):
            {
                const MetaBmpExScaleAction* pA = (const MetaBmpExScaleAction*) pAction;
                ImplWriteBitmapEx( rWriter, rDummyVDev, nCompressMode, pA->GetPoint(), pA->GetSize(), pA->GetBitmapEx() );
            }
            break;

            case( META_BMPEXSCALEPART_ACTION ):
            {
                const MetaBmpExScalePartAction* pA = (const MetaBmpExScalePartAction*) pAction;
                BitmapEx aBitmapEx( pA->GetBitmapEx() );
                aBitmapEx.Crop( Rectangle( pA->GetSrcPoint(), pA->GetSrcSize() ) );
                ImplWriteBitmapEx( rWriter, rDummyVDev, nCompressMode, pA->GetDestPoint(), pA->GetDestSize(), aBitmapEx );
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
                const MetaMapModeAction* pA = (const MetaMapModeAction*) pAction;

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

            default:
                DBG_ERROR( "SVGActionWriter::ImplWriteActions: unsupported MetaAction #" );
            break;
        }
    }

    return sal_True;
}

// -----------------------------------------------------------------------------

void PDFExport::ImplWriteGradient( PDFWriter& rWriter, const PolyPolygon& rPolyPoly, const Gradient& rGradient, VirtualDevice& rDummyVDev, sal_Int32 nCompressMode )
{
    GDIMetaFile aTmpMtf;

    rDummyVDev.AddGradientActions( rPolyPoly.GetBoundRect(), rGradient, aTmpMtf );

    rWriter.Push();
    rWriter.IntersectClipRegion( rPolyPoly );
    ImplWriteActions( rWriter, aTmpMtf, rDummyVDev, nCompressMode );
    rWriter.Pop();
}

// -----------------------------------------------------------------------------

void PDFExport::ImplWriteBitmapEx( PDFWriter& rWriter, VirtualDevice& rDummyVDev, sal_Int32 nCompressMode,
                                  const Point& rPoint, const Size& rSize, const BitmapEx& rBitmapEx )
{
    if ( !rBitmapEx.IsEmpty() )
    {
        BitmapEx    aBitmapEx( rBitmapEx );
        sal_Bool    bIsBW = aBitmapEx.GetBitmap().GetBitCount() == 1;

        sal_Int32   nMaxBmpDPI = 300;
        sal_Int32   nQuality   = 75;
        sal_Int32   nColorMode = 0;

        switch( nCompressMode )
        {
            case 0 :
            {
                nMaxBmpDPI = bIsBW ? 300 : 72;
                nQuality = 30;
            }
            break;
            case 1 :
            {
                nMaxBmpDPI = bIsBW ? 1200 : 300;
                nQuality = 60;
            }
            break;
            case 2 :
            {
                nMaxBmpDPI = bIsBW ? 1200 : 300;
                nQuality = 80;
            }
            break;
        }

        // do downsampling if neccessary
        const Size      aDstSizeTwip( rDummyVDev.PixelToLogic( rDummyVDev.LogicToPixel( rSize ), MAP_TWIP ) );
        const Size      aBmpSize( aBitmapEx.GetSizePixel() );
        const double    fBmpPixelX = aBmpSize.Width();
        const double    fBmpPixelY = aBmpSize.Height();
        const double    fMaxPixelX = aDstSizeTwip.Width() * nMaxBmpDPI / 1440.0;
        const double    fMaxPixelY = aDstSizeTwip.Height() * nMaxBmpDPI / 1440.0;

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
        const Size aSizePixel( aBitmapEx.GetSizePixel() );
        if ( aSizePixel.Width() && aSizePixel.Height() )
        {
            sal_Bool bUseJPGCompression = sal_False;

            // use jpeg not for too small graphics and graphics with more than 256 colors
            if ( ( aSizePixel.Width() > 32 ) && ( aSizePixel.Height() > 32 ) && ( aBitmapEx.GetBitCount() > 8 ) )
                bUseJPGCompression = sal_True;
            if ( bUseJPGCompression )
            {
                Bitmap aMask;
                if ( aBitmapEx.IsTransparent() )
                {
                    if ( aBitmapEx.IsAlpha() )
                        aMask = aBitmapEx.GetAlpha().GetBitmap();
                    else
                        aMask = aBitmapEx.GetMask();
                }
                SvMemoryStream  aStrm;
                GraphicFilter   aGraphicFilter;
                Graphic         aGraphic( aBitmapEx.GetBitmap() );
                sal_uInt16      nFormatName = aGraphicFilter.GetExportFormatNumberForShortName( OUString( RTL_CONSTASCII_USTRINGPARAM( "JPG" ) ) );

                Sequence< PropertyValue > aFilterData( 2 );
                aFilterData[ 0 ].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "Quality" ) );
                aFilterData[ 0 ].Value <<= nQuality;
                aFilterData[ 1 ].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "ColorMode" ) );
                aFilterData[ 1 ].Value <<= nColorMode;

                sal_uInt16 nError = aGraphicFilter.ExportGraphic( aGraphic, String(), aStrm, nFormatName, sal_True, &aFilterData );
                rWriter.DrawJPGBitmap( aStrm, aSizePixel, Rectangle( rPoint, rSize ), aMask );
            }
            else
            {
                if ( aBitmapEx.IsTransparent() )
                    rWriter.DrawBitmapEx( rPoint, rSize, aBitmapEx );
                else
                    rWriter.DrawBitmap( rPoint, rSize, aBitmapEx.GetBitmap() );
            }
        }
    }
}
