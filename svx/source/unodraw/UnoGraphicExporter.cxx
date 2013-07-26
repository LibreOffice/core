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

#include <vector>
#include <osl/mutex.hxx>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XGraphicExportFilter.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/graphic/XGraphicRenderer.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/task/XInteractionContinuation.hpp>

#include <comphelper/interaction.hxx>
#include <framework/interaction.hxx>
#include <com/sun/star/drawing/GraphicFilterRequest.hpp>
#include <com/sun/star/util/URL.hpp>
#include <cppuhelper/implbase4.hxx>
#include <osl/diagnose.h>
#include <vcl/metaact.hxx>
#include <vcl/svapp.hxx>
#include <vcl/virdev.hxx>
#include <vcl/FilterConfigItem.hxx>
#include <svl/outstrm.hxx>
#include <svx/sdr/contact/objectcontactofobjlistpainter.hxx>
#include <svx/sdr/contact/viewobjectcontact.hxx>
#include <svx/sdr/contact/viewcontact.hxx>
#include <svx/sdr/contact/displayinfo.hxx>
#include <svx/sdr/contact/viewcontactofsdrobj.hxx>
#include <editeng/numitem.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdograf.hxx>
#include "svx/xoutbmp.hxx"
#include <vcl/graphicfilter.hxx>
#include "svx/unoapi.hxx"
#include <svx/svdpage.hxx>
#include <svx/svdmodel.hxx>
#include <svx/fmview.hxx>
#include <svx/fmmodel.hxx>
#include <svx/unopage.hxx>
#include <svx/pageitem.hxx>
#include <editeng/eeitem.hxx>
#include <svx/svdoutl.hxx>
#include <editeng/flditem.hxx>

#include "boost/scoped_ptr.hpp"

#define MAX_EXT_PIX         2048

using namespace ::comphelper;
using namespace ::osl;
using namespace ::cppu;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::task;
#include <svx/sdr/contact/viewobjectcontactredirector.hxx>

// #i102251#
#include <editeng/editstat.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace svx
{
    struct ExportSettings
    {
        OUString maFilterName;
        OUString maMediaType;
        URL maURL;
        com::sun::star::uno::Reference< com::sun::star::io::XOutputStream > mxOutputStream;
        com::sun::star::uno::Reference< com::sun::star::graphic::XGraphicRenderer > mxGraphicRenderer;
        com::sun::star::uno::Reference< com::sun::star::task::XStatusIndicator >    mxStatusIndicator;
        com::sun::star::uno::Reference< com::sun::star::task::XInteractionHandler > mxInteractionHandler;

        sal_Int32 mnWidth;
        sal_Int32 mnHeight;
        sal_Bool mbExportOnlyBackground;
        sal_Bool mbScrollText;
        sal_Bool mbUseHighContrast;
        sal_Bool mbTranslucent;

        Sequence< PropertyValue >   maFilterData;

        Fraction    maScaleX;
        Fraction    maScaleY;

        ExportSettings( SdrModel* pDoc );
    };

    ExportSettings::ExportSettings( SdrModel* pDoc )
    : mnWidth( 0 )
    , mnHeight( 0 )
    , mbExportOnlyBackground( false )
    , mbScrollText( false )
    , mbUseHighContrast( false )
    , mbTranslucent( sal_False )
    , maScaleX( 1, 1 )
    , maScaleY( 1, 1 )
    {
        if( pDoc )
        {
            maScaleX = pDoc->GetScaleFraction();
            maScaleY = pDoc->GetScaleFraction();
        }
    }

    /** implements a component to export shapes or pages to external graphic formats.

        @implements com.sun.star.drawing.GraphicExportFilter
    */
    class GraphicExporter : public WeakImplHelper2< XGraphicExportFilter, XServiceInfo >
    {
    public:
        GraphicExporter();
        virtual ~GraphicExporter();

        // XFilter
        virtual sal_Bool SAL_CALL filter( const Sequence< PropertyValue >& aDescriptor ) throw(RuntimeException);
        virtual void SAL_CALL cancel(  ) throw(RuntimeException);

        // XExporter
        virtual void SAL_CALL setSourceDocument( const Reference< XComponent >& xDoc ) throw(IllegalArgumentException, RuntimeException);

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  ) throw(RuntimeException);
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(RuntimeException);
        virtual Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(RuntimeException);

        // XMimeTypeInfo
        virtual sal_Bool SAL_CALL supportsMimeType( const OUString& MimeTypeName ) throw (RuntimeException);
        virtual Sequence< OUString > SAL_CALL getSupportedMimeTypeNames(  ) throw (RuntimeException);

        VirtualDevice* CreatePageVDev( SdrPage* pPage, sal_uIntPtr nWidthPixel, sal_uIntPtr nHeightPixel ) const;

        DECL_LINK( CalcFieldValueHdl, EditFieldInfo* );

        void ParseSettings( const Sequence< PropertyValue >& aDescriptor, ExportSettings& rSettings );
        bool GetGraphic( ExportSettings& rSettings, Graphic& aGraphic, sal_Bool bVectorType );

    private:
        Reference< XShape >     mxShape;
        Reference< XDrawPage >  mxPage;
        Reference< XShapes >    mxShapes;

        SvxDrawPage*        mpUnoPage;

        Link                maOldCalcFieldValueHdl;
        sal_Int32           mnPageNumber;
        SdrPage*            mpCurrentPage;
        SdrModel*           mpDoc;
    };

    SVX_DLLPUBLIC Reference< XInterface > SAL_CALL GraphicExporter_createInstance(const Reference< XMultiServiceFactory > & )
        throw( Exception )
    {
        return (XWeak*)new GraphicExporter();
    }

    SVX_DLLPUBLIC Sequence< OUString > SAL_CALL GraphicExporter_getSupportedServiceNames()
        throw()
    {
        Sequence< OUString > aSupportedServiceNames( 1 );
        aSupportedServiceNames[0] = OUString( "com.sun.star.drawing.GraphicExportFilter" );
        return aSupportedServiceNames;
    }

    SVX_DLLPUBLIC OUString SAL_CALL GraphicExporter_getImplementationName()
        throw()
    {
        return OUString( "com.sun.star.comp.Draw.GraphicExporter" );
    }

    /** creates a bitmap that is optionaly transparent from a metafile
    */
    BitmapEx GetBitmapFromMetaFile( const GDIMetaFile& rMtf, sal_Bool bTransparent, const Size* pSize )
    {
        BitmapEx aBmpEx;

        if(bTransparent)
        {
            // use new primitive conversion tooling
            basegfx::B2DRange aRange(basegfx::B2DPoint(0.0, 0.0));

            if(pSize)
            {
                // use 100th mm for primitive bitmap converter tool, input is pixel
                // use a real OutDev to get the correct DPI, the static LogicToLogic assumes 72dpi which is wrong (!)
                const Size aSize100th(Application::GetDefaultDevice()->PixelToLogic(*pSize, MapMode(MAP_100TH_MM)));

                aRange.expand(basegfx::B2DPoint(aSize100th.Width(), aSize100th.Height()));
            }
            else
            {
                // use 100th mm for primitive bitmap converter tool
                const Size aSize100th(Application::GetDefaultDevice()->LogicToLogic(rMtf.GetPrefSize(), rMtf.GetPrefMapMode(), MapMode(MAP_100TH_MM)));

                aRange.expand(basegfx::B2DPoint(aSize100th.Width(), aSize100th.Height()));
            }

            aBmpEx = convertMetafileToBitmapEx(rMtf, aRange);
        }
        else
        {
            const SvtOptionsDrawinglayer aDrawinglayerOpt;
            const GraphicConversionParameters aParameters(
                pSize ? *pSize : Size(0, 0),
                true, // allow unlimited size
                aDrawinglayerOpt.IsAntiAliasing(),
                aDrawinglayerOpt.IsSnapHorVerLinesToDiscrete());
            const Graphic aGraphic(rMtf);

            aBmpEx = BitmapEx(aGraphic.GetBitmap(aParameters));
            aBmpEx.SetPrefMapMode( rMtf.GetPrefMapMode() );
            aBmpEx.SetPrefSize( rMtf.GetPrefSize() );
        }

        return aBmpEx;
    }

    Size* CalcSize( sal_Int32 nWidth, sal_Int32 nHeight, const Size& aBoundSize, Size& aOutSize )
    {
        if( (nWidth == 0) && (nHeight == 0) )
            return NULL;

        if( (nWidth == 0) && (nHeight != 0) && (aBoundSize.Height() != 0) )
        {
            nWidth = ( nHeight * aBoundSize.Width() ) / aBoundSize.Height();
        }
        else if( (nWidth != 0) && (nHeight == 0) && (aBoundSize.Width() != 0) )
        {
            nHeight = ( nWidth * aBoundSize.Height() ) / aBoundSize.Width();
        }

        aOutSize.Width() = nWidth;
        aOutSize.Height() = nHeight;

        return &aOutSize;
    }
}

class ImplExportCheckVisisbilityRedirector : public ::sdr::contact::ViewObjectContactRedirector
{
public:
    ImplExportCheckVisisbilityRedirector( SdrPage* pCurrentPage );
    virtual ~ImplExportCheckVisisbilityRedirector();

    virtual drawinglayer::primitive2d::Primitive2DSequence createRedirectedPrimitive2DSequence(
        const sdr::contact::ViewObjectContact& rOriginal,
        const sdr::contact::DisplayInfo& rDisplayInfo);

private:
    SdrPage*    mpCurrentPage;
};

ImplExportCheckVisisbilityRedirector::ImplExportCheckVisisbilityRedirector( SdrPage* pCurrentPage )
:   ViewObjectContactRedirector(), mpCurrentPage( pCurrentPage )
{
}

ImplExportCheckVisisbilityRedirector::~ImplExportCheckVisisbilityRedirector()
{
}

drawinglayer::primitive2d::Primitive2DSequence ImplExportCheckVisisbilityRedirector::createRedirectedPrimitive2DSequence(
    const sdr::contact::ViewObjectContact& rOriginal,
    const sdr::contact::DisplayInfo& rDisplayInfo)
{
    SdrObject* pObject = rOriginal.GetViewContact().TryToGetSdrObject();

    if(pObject)
    {
        SdrPage* pPage = mpCurrentPage;
        if( pPage == 0 )
            pPage = pObject->GetPage();

        if( (pPage == 0) || pPage->checkVisibility(rOriginal, rDisplayInfo, false) )
        {
            return ::sdr::contact::ViewObjectContactRedirector::createRedirectedPrimitive2DSequence(rOriginal, rDisplayInfo);
        }

        return drawinglayer::primitive2d::Primitive2DSequence();
    }
    else
    {
        // not an object, maybe a page
        return ::sdr::contact::ViewObjectContactRedirector::createRedirectedPrimitive2DSequence(rOriginal, rDisplayInfo);
    }
}

using namespace ::svx;

GraphicExporter::GraphicExporter()
: mpUnoPage( NULL ), mnPageNumber(-1), mpCurrentPage(0), mpDoc( NULL )
{
}

GraphicExporter::~GraphicExporter()
{
}

IMPL_LINK(GraphicExporter, CalcFieldValueHdl, EditFieldInfo*, pInfo)
{
    if( pInfo )
    {
        if( mpCurrentPage )
        {
            pInfo->SetSdrPage( mpCurrentPage );
        }
        else if( mnPageNumber != -1 )
        {
            const SvxFieldData* pField = pInfo->GetField().GetField();
            if( pField && pField->ISA( SvxPageField ) )
            {
                String aPageNumValue;
                sal_Bool bUpper = sal_False;

                switch(mpDoc->GetPageNumType())
                {
                    case SVX_CHARS_UPPER_LETTER:
                        aPageNumValue += (sal_Unicode)(char)((mnPageNumber - 1) % 26 + 'A');
                        break;
                    case SVX_CHARS_LOWER_LETTER:
                        aPageNumValue += (sal_Unicode)(char)((mnPageNumber - 1) % 26 + 'a');
                        break;
                    case SVX_ROMAN_UPPER:
                        bUpper = sal_True;
                    case SVX_ROMAN_LOWER:
                        aPageNumValue += SvxNumberFormat::CreateRomanString(mnPageNumber, bUpper);
                        break;
                    case SVX_NUMBER_NONE:
                        aPageNumValue.Erase();
                        aPageNumValue += sal_Unicode(' ');
                        break;
                    default:
                        aPageNumValue += OUString::number( (sal_Int32)mnPageNumber );
                }

                pInfo->SetRepresentation( aPageNumValue );

                return(0);
            }
        }
    }

    long nRet = maOldCalcFieldValueHdl.Call( pInfo );

    if( pInfo && mpCurrentPage )
        pInfo->SetSdrPage( 0 );

    return nRet;
}

/** creates an virtual device for the given page

    @return the returned VirtualDevice is owned by the caller
*/
VirtualDevice* GraphicExporter::CreatePageVDev( SdrPage* pPage, sal_uIntPtr nWidthPixel, sal_uIntPtr nHeightPixel ) const
{
    VirtualDevice*  pVDev = new VirtualDevice();
    MapMode         aMM( MAP_100TH_MM );

    Point aPoint( 0, 0 );
    Size aPageSize(pPage->GetSize());

    // use scaling?
    if( nWidthPixel )
    {
        const Fraction aFrac( (long) nWidthPixel, pVDev->LogicToPixel( aPageSize, aMM ).Width() );

        aMM.SetScaleX( aFrac );

        if( nHeightPixel == 0 )
            aMM.SetScaleY( aFrac );
    }

    if( nHeightPixel )
    {
        const Fraction aFrac( (long) nHeightPixel, pVDev->LogicToPixel( aPageSize, aMM ).Height() );

        if( nWidthPixel == 0 )
            aMM.SetScaleX( aFrac );

        aMM.SetScaleY( aFrac );
    }

    pVDev->SetMapMode( aMM );
#ifdef DBG_UTIL
    bool bAbort = !
#endif
        pVDev->SetOutputSize(aPageSize);
    DBG_ASSERT(!bAbort, "virt. Device nicht korrekt erzeugt");

    SdrView* pView = new SdrView(mpDoc, pVDev);
    pView->SetPageVisible( sal_False );
    pView->SetBordVisible( sal_False );
    pView->SetGridVisible( sal_False );
    pView->SetHlplVisible( sal_False );
    pView->SetGlueVisible( sal_False );
    pView->ShowSdrPage(pPage);
    Region aRegion (Rectangle( aPoint, aPageSize ) );

    ImplExportCheckVisisbilityRedirector aRedirector( mpCurrentPage );

    pView->CompleteRedraw(pVDev, aRegion, &aRedirector);

    delete pView;
    return pVDev;
}

void GraphicExporter::ParseSettings( const Sequence< PropertyValue >& aDescriptor, ExportSettings& rSettings )
{
    sal_Int32 nArgs = aDescriptor.getLength();
    const PropertyValue* pValues = aDescriptor.getConstArray();
    while( nArgs-- )
    {
        if ( pValues->Name == "FilterName" )
        {
            pValues->Value >>= rSettings.maFilterName;
        }
        else if ( pValues->Name == "MediaType" )
        {
            pValues->Value >>= rSettings.maMediaType;
        }
        else if ( pValues->Name == "URL" )
        {
            if( !( pValues->Value >>= rSettings.maURL ) )
            {
                pValues->Value >>= rSettings.maURL.Complete;
            }
        }
        else if ( pValues->Name == "OutputStream" )
        {
            pValues->Value >>= rSettings.mxOutputStream;
        }
        else if ( pValues->Name == "GraphicRenderer" )
        {
            pValues->Value >>= rSettings.mxGraphicRenderer;
        }
        else if ( pValues->Name == "StatusIndicator" )
        {
            pValues->Value >>= rSettings.mxStatusIndicator;
        }
        else if ( pValues->Name == "InteractionHandler" )
        {
            pValues->Value >>= rSettings.mxInteractionHandler;
        }
        else if( pValues->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "Width" ) ) )  // for compatibility reasons, deprecated
        {
            pValues->Value >>= rSettings.mnWidth;
        }
        else if( pValues->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "Height" ) ) ) // for compatibility reasons, deprecated
        {
            pValues->Value >>= rSettings.mnHeight;
        }
        else if( pValues->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "ExportOnlyBackground" ) ) )   // for compatibility reasons, deprecated
        {
            pValues->Value >>= rSettings.mbExportOnlyBackground;
        }
        else if ( pValues->Name == "FilterData" )
        {
            pValues->Value >>= rSettings.maFilterData;

            sal_Int32 nFilterArgs = rSettings.maFilterData.getLength();
            PropertyValue* pDataValues = rSettings.maFilterData.getArray();
            while( nFilterArgs-- )
            {
                if ( pDataValues->Name == "Translucent" )
                {
                    if ( !( pDataValues->Value >>= rSettings.mbTranslucent ) )  // SJ: TODO: The GIF Transparency is stored as int32 in
                    {                                               // configuration files, this has to be changed to boolean
                        sal_Int32 nTranslucent = 0;
                        if ( pDataValues->Value >>= nTranslucent )
                            rSettings.mbTranslucent = nTranslucent != 0;
                    }
                }
                else if ( pDataValues->Name == "PixelWidth" )
                {
                    pDataValues->Value >>= rSettings.mnWidth;
                }
                else if ( pDataValues->Name == "PixelHeight" )
                {
                    pDataValues->Value >>= rSettings.mnHeight;
                }
                else if( pDataValues->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "Width" ) ) )  // for compatibility reasons, deprecated
                {
                    pDataValues->Value >>= rSettings.mnWidth;
                    pDataValues->Name = OUString( "PixelWidth" );
                }
                else if( pDataValues->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "Height" ) ) ) // for compatibility reasons, deprecated
                {
                    pDataValues->Value >>= rSettings.mnHeight;
                    pDataValues->Name = OUString( "PixelHeight" );
                }
                else if ( pDataValues->Name == "ExportOnlyBackground" )
                {
                    pDataValues->Value >>= rSettings.mbExportOnlyBackground;
                }
                else if ( pDataValues->Name == "HighContrast" )
                {
                    pDataValues->Value >>= rSettings.mbUseHighContrast;
                }
                else if ( pDataValues->Name == "PageNumber" )
                {
                    pDataValues->Value >>= mnPageNumber;
                }
                else if ( pDataValues->Name == "ScrollText" )
                {
                    // #110496# Read flag solitary scroll text metafile
                    pDataValues->Value >>= rSettings.mbScrollText;
                }
                else if ( pDataValues->Name == "CurrentPage" )
                {
                    Reference< XDrawPage >  xPage;
                    pDataValues->Value >>= xPage;
                    if( xPage.is() )
                    {
                        SvxDrawPage* pUnoPage = SvxDrawPage::getImplementation( xPage );
                        if( pUnoPage && pUnoPage->GetSdrPage() )
                            mpCurrentPage = pUnoPage->GetSdrPage();
                    }
                }
                else if ( pDataValues->Name == "ScaleXNumerator" )
                {
                    sal_Int32 nVal = 1;
                    if( pDataValues->Value >>= nVal )
                        rSettings.maScaleX = Fraction( nVal, rSettings.maScaleX.GetDenominator() );
                }
                else if ( pDataValues->Name == "ScaleXDenominator" )
                {
                    sal_Int32 nVal = 1;
                    if( pDataValues->Value >>= nVal )
                        rSettings.maScaleX = Fraction( rSettings.maScaleX.GetNumerator(), nVal );
                }
                else if ( pDataValues->Name == "ScaleYNumerator" )
                {
                    sal_Int32 nVal = 1;
                    if( pDataValues->Value >>= nVal )
                        rSettings.maScaleY = Fraction( nVal, rSettings.maScaleY.GetDenominator() );
                }
                else if ( pDataValues->Name == "ScaleYDenominator" )
                {
                    sal_Int32 nVal = 1;
                    if( pDataValues->Value >>= nVal )
                        rSettings.maScaleY = Fraction( rSettings.maScaleY.GetNumerator(), nVal );
                }

                pDataValues++;
            }
        }

        pValues++;
    }

    // putting the StatusIndicator that we got from the MediaDescriptor into our local FilterData copy
    if ( rSettings.mxStatusIndicator.is() )
    {
        OUString sStatusIndicator( "StatusIndicator" );
        int i = rSettings.maFilterData.getLength();
        rSettings.maFilterData.realloc( i + 1 );
        rSettings.maFilterData[ i ].Name = sStatusIndicator;
        rSettings.maFilterData[ i ].Value <<= rSettings.mxStatusIndicator;
    }
}

bool GraphicExporter::GetGraphic( ExportSettings& rSettings, Graphic& aGraphic, sal_Bool bVectorType )
{
    if( !mpDoc || !mpUnoPage )
        return false;

    SdrPage* pPage = mpUnoPage->GetSdrPage();
    if( !pPage )
        return false;

    VirtualDevice       aVDev;
    const MapMode       aMap( mpDoc->GetScaleUnit(), Point(), rSettings.maScaleX, rSettings.maScaleY );

    SdrOutliner& rOutl=mpDoc->GetDrawOutliner(NULL);
    maOldCalcFieldValueHdl = rOutl.GetCalcFieldValueHdl();
    rOutl.SetCalcFieldValueHdl( LINK(this, GraphicExporter, CalcFieldValueHdl) );
    rOutl.SetBackgroundColor( pPage->GetPageBackgroundColor() );

    // #i102251#
    const sal_uInt32 nOldCntrl(rOutl.GetControlWord());
    sal_uInt32 nCntrl = nOldCntrl & ~EE_CNTRL_ONLINESPELLING;
    rOutl.SetControlWord(nCntrl);

    SdrObject* pTempBackgroundShape = 0;
    std::vector< SdrObject* > aShapes;
    bool bRet = true;

    // export complete page?
    if ( !mxShape.is() )
    {
        if( rSettings.mbExportOnlyBackground )
        {
            const SdrPageProperties* pCorrectProperties = pPage->getCorrectSdrPageProperties();

            if(pCorrectProperties)
            {
                pTempBackgroundShape = new SdrRectObj(Rectangle(Point(0,0), pPage->GetSize()));
                pTempBackgroundShape->SetMergedItemSet(pCorrectProperties->GetItemSet());
                pTempBackgroundShape->SetMergedItem(XLineStyleItem(XLINE_NONE));
                pTempBackgroundShape->NbcSetStyleSheet(pCorrectProperties->GetStyleSheet(), true);
                aShapes.push_back(pTempBackgroundShape);
            }
        }
        else
        {
            const Size aSize( pPage->GetSize() );

            // generate a bitmap to convert it to a pixel format.
            // For gif pictures there can also be a vector format used (bTranslucent)
            if ( !bVectorType && !rSettings.mbTranslucent )
            {
                long nWidthPix = 0;
                long nHeightPix = 0;
                if ( rSettings.mnWidth > 0 && rSettings.mnHeight > 0 )
                {
                    nWidthPix = rSettings.mnWidth;
                    nHeightPix = rSettings.mnHeight;
                }
                else
                {
                    const Size aSizePix( Application::GetDefaultDevice()->LogicToPixel( aSize, aMap ) );
                    if (aSizePix.Width() > MAX_EXT_PIX || aSizePix.Height() > MAX_EXT_PIX)
                    {
                        if (aSizePix.Width() > MAX_EXT_PIX)
                            nWidthPix = MAX_EXT_PIX;
                        else
                            nWidthPix = aSizePix.Width();
                        if (aSizePix.Height() > MAX_EXT_PIX)
                            nHeightPix = MAX_EXT_PIX;
                        else
                            nHeightPix = aSizePix.Height();

                        double fWidthDif = aSizePix.Width() / nWidthPix;
                        double fHeightDif = aSizePix.Height() / nHeightPix;

                        if (fWidthDif > fHeightDif)
                            nHeightPix = static_cast<long>(aSizePix.Height() / fWidthDif);
                        else
                            nWidthPix = static_cast<long>(aSizePix.Width() / fHeightDif);
                    }
                    else
                    {
                        nWidthPix = aSizePix.Width();
                        nHeightPix = aSizePix.Height();
                    }
                }

                boost::scoped_ptr< SdrView > pLocalView;
                if( PTR_CAST( FmFormModel, mpDoc ) )
                {
                    pLocalView.reset( new FmFormView( PTR_CAST( FmFormModel, mpDoc ), &aVDev ) );
                }
                else
                {
                    pLocalView.reset( new SdrView( mpDoc, &aVDev ) );
                }


                VirtualDevice*  pVDev = CreatePageVDev( pPage, nWidthPix, nHeightPix );

                if( pVDev )
                {
                    aGraphic = pVDev->GetBitmap( Point(), pVDev->GetOutputSize() );
                    aGraphic.SetPrefMapMode( aMap );
                    aGraphic.SetPrefSize( aSize );
                    delete pVDev;
                }
            }
            // create a metafile to export a vector format
            else
            {
                GDIMetaFile aMtf;

                aVDev.SetMapMode( aMap );
                if( rSettings.mbUseHighContrast )
                    aVDev.SetDrawMode( aVDev.GetDrawMode() | DRAWMODE_SETTINGSLINE | DRAWMODE_SETTINGSFILL | DRAWMODE_SETTINGSTEXT | DRAWMODE_SETTINGSGRADIENT );
                aVDev.EnableOutput( sal_False );
                aMtf.Record( &aVDev );
                Size aNewSize;

                // create a view
                boost::scoped_ptr< SdrView > pView;
                if( PTR_CAST( FmFormModel, mpDoc ) )
                {
                    pView.reset(new FmFormView( PTR_CAST( FmFormModel, mpDoc ), &aVDev ));
                }
                else
                {
                    pView.reset(new SdrView( mpDoc, &aVDev ));
                }

                pView->SetBordVisible( sal_False );
                pView->SetPageVisible( sal_False );
                pView->ShowSdrPage( pPage );

                const Point aNewOrg( pPage->GetLftBorder(), pPage->GetUppBorder() );
                aNewSize = Size( aSize.Width() - pPage->GetLftBorder() - pPage->GetRgtBorder(),
                                 aSize.Height() - pPage->GetUppBorder() - pPage->GetLwrBorder() );
                const Rectangle aClipRect( aNewOrg, aNewSize );
                MapMode         aVMap( aMap );

                aVDev.Push();
                aVMap.SetOrigin( Point( -aNewOrg.X(), -aNewOrg.Y() ) );
                aVDev.SetRelativeMapMode( aVMap );
                aVDev.IntersectClipRegion( aClipRect );

                // Use new StandardCheckVisisbilityRedirector
                ImplExportCheckVisisbilityRedirector aRedirector( mpCurrentPage );

                pView->CompleteRedraw(&aVDev, Region(Rectangle(Point(), aNewSize)), &aRedirector);

                aVDev.Pop();

                aMtf.Stop();
                aMtf.WindStart();
                aMtf.SetPrefMapMode( aMap );
                aMtf.SetPrefSize( aNewSize );

                // AW: Here the current version was filtering out the META_CLIPREGION_ACTIONs
                // from the metafile. I asked some other developers why this was done, but no
                // one knew a direct reason. Since it's in for long time, it may be an old
                // piece of code. MetaFiles save and load ClipRegions with polygons with preserving
                // the polygons, so a resolution-indepent roundtrip is supported. Removed this
                // code since it destroys some MetaFiles where ClipRegions are used. Anyways,
                // just filtering them out is a hack, at least the encapsulated content would need
                // to be clipped geometrically.
                aGraphic = Graphic(aMtf);

                pView->HideSdrPage();

                if( rSettings.mbTranslucent )
                {
                    Size aOutSize;
                    aGraphic = GetBitmapFromMetaFile( aGraphic.GetGDIMetaFile(), sal_True, CalcSize( rSettings.mnWidth, rSettings.mnHeight, aNewSize, aOutSize ) );
                }
            }
        }
    }

    // export only single shape or shape collection
    else
    {
        // build list of SdrObject
        if( mxShapes.is() )
        {
            Reference< XShape > xShape;
            const sal_Int32 nCount = mxShapes->getCount();

            for( sal_Int32 nIndex = 0; nIndex < nCount; nIndex++ )
            {
                mxShapes->getByIndex( nIndex ) >>= xShape;
                SdrObject* pObj = GetSdrObjectFromXShape( xShape );
                if( pObj )
                    aShapes.push_back( pObj );
            }
        }
        else
        {
            // only one shape
            SdrObject* pObj = GetSdrObjectFromXShape( mxShape );
            if( pObj )
                aShapes.push_back( pObj );
        }

        if( aShapes.empty() )
            bRet = false;
    }

    if( bRet && !aShapes.empty() )
    {
        // special treatment for only one SdrGrafObj that has text
        bool bSingleGraphic = false;

        if( 1 == aShapes.size() )
        {
            if( !bVectorType )
            {
                SdrObject* pObj = aShapes.front();
                if( pObj && pObj->ISA( SdrGrafObj ) && !( (SdrGrafObj*) pObj )->HasText() )
                {
                    aGraphic = ( (SdrGrafObj*) pObj )->GetTransformedGraphic();
                    if ( aGraphic.GetType() == GRAPHIC_BITMAP )
                    {
                        Size aSizePixel( aGraphic.GetSizePixel() );
                        if( rSettings.mnWidth && rSettings.mnHeight &&
                            ( ( rSettings.mnWidth != aSizePixel.Width() ) ||
                              ( rSettings.mnHeight != aSizePixel.Height() ) ) )
                        {
                            BitmapEx aBmpEx( aGraphic.GetBitmapEx() );
                            // export: use highest quality
                            aBmpEx.Scale( Size( rSettings.mnWidth, rSettings.mnHeight ), BMP_SCALE_LANCZOS );
                            aGraphic = aBmpEx;
                        }

                        // #118804# only accept for bitmap graphics, else the
                        // conversion to bitmap will happen anywhere without size control
                        // as evtl. defined in rSettings.mnWidth/mnHeight
                        bSingleGraphic = true;
                    }
                }
            }
            else if( rSettings.mbScrollText )
            {
                SdrObject* pObj = aShapes.front();
                if( pObj && pObj->ISA( SdrTextObj )
                    && ( (SdrTextObj*) pObj )->HasText() )
                {
                    Rectangle aScrollRectangle;
                    Rectangle aPaintRectangle;

                    const boost::scoped_ptr< GDIMetaFile > pMtf(
                        ( (SdrTextObj*) pObj )->GetTextScrollMetaFileAndRectangle(
                        aScrollRectangle, aPaintRectangle ) );

                    // take the larger one of the two rectangles (that
                    // should be the bound rect of the retrieved
                    // metafile)
                    Rectangle aTextRect;

                    if( aScrollRectangle.IsInside( aPaintRectangle ) )
                        aTextRect = aScrollRectangle;
                    else
                        aTextRect = aPaintRectangle;

                    // setup pref size and mapmode
                    pMtf->SetPrefSize( aTextRect.GetSize() );

                    // set actual origin (mtf is at actual shape
                    // output position)
                    MapMode aLocalMapMode( aMap );
                    aLocalMapMode.SetOrigin(
                        Point( -aPaintRectangle.Left(),
                               -aPaintRectangle.Top() ) );
                    pMtf->SetPrefMapMode( aLocalMapMode );

                    pMtf->AddAction( new MetaCommentAction(
                                         "XTEXT_SCROLLRECT", 0,
                                         reinterpret_cast<sal_uInt8 const*>(&aScrollRectangle),
                                         sizeof( Rectangle ) ) );
                    pMtf->AddAction( new MetaCommentAction(
                                         "XTEXT_PAINTRECT", 0,
                                         reinterpret_cast<sal_uInt8 const*>(&aPaintRectangle),
                                         sizeof( Rectangle ) ) );

                    aGraphic = Graphic( *pMtf );

                    bSingleGraphic = true;
                }
            }
        }

        if( !bSingleGraphic )
        {
            // create a metafile for all shapes
            VirtualDevice   aOut;

            // calculate bound rect for all shapes
            Rectangle aBound;

            {
                std::vector< SdrObject* >::iterator aIter = aShapes.begin();
                const std::vector< SdrObject* >::iterator aEnd = aShapes.end();

                while( aIter != aEnd )
                {
                    SdrObject* pObj = (*aIter++);
                    Rectangle aR1(pObj->GetCurrentBoundRect());
                    if (aBound.IsEmpty())
                        aBound=aR1;
                    else
                        aBound.Union(aR1);
                }
            }

            aOut.EnableOutput( sal_False );
            aOut.SetMapMode( aMap );
            if( rSettings.mbUseHighContrast )
                aOut.SetDrawMode( aVDev.GetDrawMode() | DRAWMODE_SETTINGSLINE | DRAWMODE_SETTINGSFILL | DRAWMODE_SETTINGSTEXT | DRAWMODE_SETTINGSGRADIENT );

            GDIMetaFile aMtf;
            aMtf.Clear();
            aMtf.Record( &aOut );

            MapMode aOutMap( aMap );
            aOutMap.SetOrigin( Point( -aBound.TopLeft().X(), -aBound.TopLeft().Y() ) );
            aOut.SetRelativeMapMode( aOutMap );

            sdr::contact::DisplayInfo aDisplayInfo;

            if(mpCurrentPage)
            {
                if(mpCurrentPage->TRG_HasMasterPage() && pPage->IsMasterPage())
                {
                    // MasterPage is processed as another page's SubContent
                    aDisplayInfo.SetProcessLayers(mpCurrentPage->TRG_GetMasterPageVisibleLayers());
                    aDisplayInfo.SetSubContentActive(true);
                }
            }

            if(!aShapes.empty())
            {
                // more effective way to paint a vector of SdrObjects. Hand over the processed page
                // to have it in the
                sdr::contact::ObjectContactOfObjListPainter aMultiObjectPainter(aOut, aShapes, mpCurrentPage);
                ImplExportCheckVisisbilityRedirector aCheckVisibilityRedirector(mpCurrentPage);
                aMultiObjectPainter.SetViewObjectContactRedirector(&aCheckVisibilityRedirector);

                aMultiObjectPainter.ProcessDisplay(aDisplayInfo);
            }

            aMtf.Stop();
            aMtf.WindStart();

            const Size  aExtSize( aOut.PixelToLogic( Size( 0, 0  ) ) );
            Size        aBoundSize( aBound.GetWidth() + ( aExtSize.Width() ),
                                    aBound.GetHeight() + ( aExtSize.Height() ) );

            aMtf.SetPrefMapMode( aMap );
            aMtf.SetPrefSize( aBoundSize );

            if( !bVectorType )
            {
                Size aOutSize;
                aGraphic = GetBitmapFromMetaFile( aMtf, rSettings.mbTranslucent, CalcSize( rSettings.mnWidth, rSettings.mnHeight, aBoundSize, aOutSize ) );
            }
            else
            {
                aGraphic = aMtf;
            }
        }
    }

    if(pTempBackgroundShape)
    {
        SdrObject::Free(pTempBackgroundShape);
    }

    rOutl.SetCalcFieldValueHdl( maOldCalcFieldValueHdl );

    // #i102251#
    rOutl.SetControlWord(nOldCntrl);

    return bRet;

}

// XFilter
sal_Bool SAL_CALL GraphicExporter::filter( const Sequence< PropertyValue >& aDescriptor )
    throw(RuntimeException)
{
    ::SolarMutexGuard aGuard;

    if( NULL == mpUnoPage )
        return sal_False;

    if( NULL == mpUnoPage->GetSdrPage() || NULL == mpDoc )
        return sal_False;

    GraphicFilter &rFilter = GraphicFilter::GetGraphicFilter();

    // get the arguments from the descriptor
    ExportSettings aSettings( mpDoc );
    ParseSettings( aDescriptor, aSettings );

    const sal_uInt16    nFilter = !aSettings.maMediaType.isEmpty()
                            ? rFilter.GetExportFormatNumberForMediaType( aSettings.maMediaType )
                            : rFilter.GetExportFormatNumberForShortName( aSettings.maFilterName );
    sal_Bool            bVectorType = !rFilter.IsExportPixelFormat( nFilter );

    // create the output stuff
    Graphic aGraphic;

    sal_uInt16 nStatus = GetGraphic( aSettings, aGraphic, bVectorType ) ? GRFILTER_OK : GRFILTER_FILTERERROR;

    if( nStatus == GRFILTER_OK )
    {
        // export graphic only if it has a size
        const Size aGraphSize( aGraphic.GetPrefSize() );
        if ( ( aGraphSize.Width() == 0 ) || ( aGraphSize.Height() == 0 ) )
        {
            nStatus = GRFILTER_FILTERERROR;
        }
        else
        {
            // now we have a graphic, so export it
            if( aSettings.mxGraphicRenderer.is() )
            {
                // render graphic directly into given renderer
                aSettings.mxGraphicRenderer->render( aGraphic.GetXGraphic() );
            }
            else if( aSettings.mxOutputStream.is() )
            {
                // TODO: Either utilize optional XSeekable functionality for the
                // SvOutputStream, or adapt the graphic filter to not seek anymore.
                SvMemoryStream aStream( 1024, 1024 );

                nStatus = rFilter.ExportGraphic( aGraphic, String(), aStream, nFilter, &aSettings.maFilterData );

                // copy temp stream to XOutputStream
                SvOutputStream aOutputStream( aSettings.mxOutputStream );
                aStream.Seek(0);
                aOutputStream << aStream;
            }
            else
            {
                INetURLObject aURLObject( aSettings.maURL.Complete );
                DBG_ASSERT( aURLObject.GetProtocol() != INET_PROT_NOT_VALID, "invalid URL" );

                nStatus = XOutBitmap::ExportGraphic( aGraphic, aURLObject, rFilter, nFilter, &aSettings.maFilterData );
            }
        }
    }

    if ( aSettings.mxInteractionHandler.is() && ( nStatus != GRFILTER_OK ) )
    {
        Any aInteraction;
        Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionContinuation > > lContinuations(1);
        ::comphelper::OInteractionApprove* pApprove = new ::comphelper::OInteractionApprove();
        lContinuations[0] = Reference< XInteractionContinuation >(static_cast< XInteractionContinuation* >(pApprove), UNO_QUERY);

        GraphicFilterRequest aErrorCode;
        aErrorCode.ErrCode = nStatus;
        aInteraction <<= aErrorCode;
        aSettings.mxInteractionHandler->handle( framework::InteractionRequest::CreateRequest( aInteraction, lContinuations ) );
    }
    return nStatus == GRFILTER_OK;
}

void SAL_CALL GraphicExporter::cancel()
    throw(RuntimeException)
{
}

// XExporter

/** the source 'document' could be a XDrawPage, a XShape or a generic XShapes */
void SAL_CALL GraphicExporter::setSourceDocument( const Reference< lang::XComponent >& xComponent )
    throw(IllegalArgumentException, RuntimeException)
{
    ::SolarMutexGuard aGuard;

    mxShapes = NULL;
    mpUnoPage = NULL;

    try
    {
    // any break inside this one loop while will throw a IllegalArgumentException
    do
    {
        mxPage = Reference< XDrawPage >::query( xComponent );
        mxShapes = Reference< XShapes >::query( xComponent );
        mxShape = Reference< XShape >::query( xComponent );

        // Step 1: try a generic XShapes
        if( !mxPage.is() && !mxShape.is() && mxShapes.is() )
        {
            // we do not support empty shape collections
            if( 0 == mxShapes->getCount() )
                break;

            // get first shape to detect corresponding page and model
            mxShapes->getByIndex(0) >>= mxShape;
        }
        else
        {
            mxShapes = NULL;
        }

        // Step 2: try a shape
        if( mxShape.is() )
        {
            if( NULL == GetSdrObjectFromXShape( mxShape ) )
                break;

            // get page for this shape
            Reference< XChild > xChild( mxShape, UNO_QUERY );
            if( !xChild.is() )
                break;

            Reference< XInterface > xInt;
            do
            {
                xInt = xChild->getParent();
                mxPage = Reference< XDrawPage >::query( xInt );
                if( !mxPage.is() )
                    xChild = Reference< XChild >::query( xInt );
            }
            while( !mxPage.is() && xChild.is() );

            if( !mxPage.is() )
                break;
        }

        // Step 3: check the page
        if( !mxPage.is() )
            break;

        mpUnoPage = SvxDrawPage::getImplementation( mxPage );

        if( NULL == mpUnoPage || NULL == mpUnoPage->GetSdrPage() )
            break;

        mpDoc = mpUnoPage->GetSdrPage()->GetModel();

        // Step 4:  If we got a generic XShapes test all contained shapes
        //          if they belong to the same XDrawPage

        if( mxShapes.is() )
        {
            SdrPage* pPage = mpUnoPage->GetSdrPage();
            SdrObject* pObj;
            Reference< XShape > xShape;

            bool bOk = true;

            const sal_Int32 nCount = mxShapes->getCount();

            // test all but the first shape if they have the same page than
            // the first shape
            for( sal_Int32 nIndex = 1; bOk && ( nIndex < nCount ); nIndex++ )
            {
                mxShapes->getByIndex( nIndex ) >>= xShape;
                pObj = GetSdrObjectFromXShape( xShape );
                bOk = pObj && pObj->GetPage() == pPage;
            }

            if( !bOk )
                break;
        }

        // no errors so far
        return;
    }
    while( 0 );
    }
    catch( Exception& )
    {
    }

    throw IllegalArgumentException();
}

// XServiceInfo
OUString SAL_CALL GraphicExporter::getImplementationName(  )
    throw(RuntimeException)
{
    return GraphicExporter_getImplementationName();
}

sal_Bool SAL_CALL GraphicExporter::supportsService( const OUString& ServiceName )
    throw(RuntimeException)
{
    Sequence< OUString > aSeq( GraphicExporter_getSupportedServiceNames() );
    sal_Int32 nArgs = aSeq.getLength();
    const OUString* pService = aSeq.getConstArray();
    while( nArgs-- )
        if( *pService++ == ServiceName )
            return sal_True;

    return sal_False;
}

Sequence< OUString > SAL_CALL GraphicExporter::getSupportedServiceNames(  )
    throw(RuntimeException)
{
    return GraphicExporter_getSupportedServiceNames();
}

// XMimeTypeInfo
sal_Bool SAL_CALL GraphicExporter::supportsMimeType( const OUString& MimeTypeName ) throw (RuntimeException)
{
    const String aMimeTypeName( MimeTypeName );

    GraphicFilter &rFilter = GraphicFilter::GetGraphicFilter();
    sal_uInt16 nCount = rFilter.GetExportFormatCount();
    sal_uInt16 nFilter;
    for( nFilter = 0; nFilter < nCount; nFilter++ )
    {
        if( aMimeTypeName.Equals( rFilter.GetExportFormatMediaType( nFilter ) ) )
        {
            return sal_True;
        }
    }

    return sal_False;
}

Sequence< OUString > SAL_CALL GraphicExporter::getSupportedMimeTypeNames(  ) throw (RuntimeException)
{
    GraphicFilter &rFilter = GraphicFilter::GetGraphicFilter();
    sal_uInt16 nCount = rFilter.GetExportFormatCount();
    sal_uInt16 nFilter;
    sal_uInt16 nFound = 0;

    Sequence< OUString > aSeq( nCount );
    OUString* pStr = aSeq.getArray();

    for( nFilter = 0; nFilter < nCount; nFilter++ )
    {
        OUString aMimeType( rFilter.GetExportFormatMediaType( nFilter ) );
        if( !aMimeType.isEmpty() )
        {
            *pStr++ = aMimeType;
            nFound++;
        }
    }

    if( nFound < nCount )
        aSeq.realloc( nFound );

    return aSeq;
}

Graphic SvxGetGraphicForShape( SdrObject& rShape, bool bVector )
{
    Graphic aGraphic;
    try
    {
        rtl::Reference< GraphicExporter > xExporter( new GraphicExporter() );
        Reference< XComponent > xComp( rShape.getUnoShape(), UNO_QUERY_THROW );
        xExporter->setSourceDocument( xComp );
        ExportSettings aSettings( rShape.GetModel() );
        xExporter->GetGraphic( aSettings, aGraphic, bVector );
    }
    catch( Exception& )
    {
        OSL_FAIL("SvxGetGraphicForShape(), exception caught!");
    }
    return aGraphic;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
