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
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/container/XChild.hpp>
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
#include <com/sun/star/uno/XComponentContext.hpp>

#include <tools/urlobj.hxx>
#include <comphelper/interaction.hxx>
#include <framework/interaction.hxx>
#include <com/sun/star/drawing/GraphicFilterRequest.hpp>
#include <com/sun/star/util/URL.hpp>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
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
#include <svx/xoutbmp.hxx>
#include <vcl/graphicfilter.hxx>
#include <svx/unoapi.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdmodel.hxx>
#include <svx/fmview.hxx>
#include <svx/fmmodel.hxx>
#include <svx/unopage.hxx>
#include <svx/pageitem.hxx>
#include <editeng/eeitem.hxx>
#include <svx/svdoutl.hxx>
#include <editeng/flditem.hxx>
#include "UnoGraphicExporter.hxx"
#include <memory>

#define MAX_EXT_PIX         2048

using namespace ::comphelper;
using namespace ::cppu;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::task;

// #i102251#
#include <editeng/editstat.hxx>

namespace {

    struct ExportSettings
    {
        OUString maFilterName;
        OUString maMediaType;
        URL maURL;
        css::uno::Reference< css::io::XOutputStream >         mxOutputStream;
        css::uno::Reference< css::graphic::XGraphicRenderer > mxGraphicRenderer;
        css::uno::Reference< css::task::XStatusIndicator >    mxStatusIndicator;
        css::uno::Reference< css::task::XInteractionHandler > mxInteractionHandler;

        sal_Int32 mnWidth;
        sal_Int32 mnHeight;
        bool mbExportOnlyBackground;
        bool mbScrollText;
        bool mbUseHighContrast;
        bool mbTranslucent;

        Sequence< PropertyValue >   maFilterData;

        Fraction    maScaleX;
        Fraction    maScaleY;

        TriState meAntiAliasing = TRISTATE_INDET;

        explicit ExportSettings(const SdrModel* pSdrModel);
    };

    ExportSettings::ExportSettings(const SdrModel* pSdrModel)
    :   mnWidth( 0 )
        ,mnHeight( 0 )
        ,mbExportOnlyBackground( false )
        ,mbScrollText( false )
        ,mbUseHighContrast( false )
        ,mbTranslucent( false )
        ,maScaleX( 1, 1 )
        ,maScaleY( 1, 1 )
    {
        if (pSdrModel)
        {
            maScaleX = pSdrModel->GetScaleFraction();
            maScaleY = pSdrModel->GetScaleFraction();
        }
    }

    /** implements a component to export shapes or pages to external graphic formats.

        @implements com.sun.star.drawing.GraphicExportFilter
    */
    class GraphicExporter : public WeakImplHelper< XGraphicExportFilter, XServiceInfo >
    {
    public:
        GraphicExporter();

        // XFilter
        virtual sal_Bool SAL_CALL filter( const Sequence< PropertyValue >& aDescriptor ) override;
        virtual void SAL_CALL cancel(  ) override;

        // XExporter
        virtual void SAL_CALL setSourceDocument( const Reference< XComponent >& xDoc ) override;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  ) override;
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
        virtual Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;

        // XMimeTypeInfo
        virtual sal_Bool SAL_CALL supportsMimeType( const OUString& MimeTypeName ) override;
        virtual Sequence< OUString > SAL_CALL getSupportedMimeTypeNames(  ) override;

        VclPtr<VirtualDevice> CreatePageVDev( SdrPage* pPage, sal_uIntPtr nWidthPixel, sal_uIntPtr nHeightPixel ) const;

        DECL_LINK( CalcFieldValueHdl, EditFieldInfo*, void );

        void ParseSettings( const Sequence< PropertyValue >& aDescriptor, ExportSettings& rSettings );
        bool GetGraphic( ExportSettings const & rSettings, Graphic& aGraphic, bool bVectorType );

    private:
        Reference< XShape >     mxShape;
        Reference< XDrawPage >  mxPage;
        Reference< XShapes >    mxShapes;
        Graphic maGraphic;

        SvxDrawPage*        mpUnoPage;

        Link<EditFieldInfo*,void> maOldCalcFieldValueHdl;
        sal_Int32           mnPageNumber;
        SdrPage*            mpCurrentPage;
        SdrModel*           mpDoc;
    };

    /** creates a bitmap that is optionally transparent from a metafile
    */
    BitmapEx GetBitmapFromMetaFile( const GDIMetaFile& rMtf, const Size* pSize )
    {
        // use new primitive conversion tooling
        basegfx::B2DRange aRange(basegfx::B2DPoint(0.0, 0.0));
        sal_uInt32 nMaximumQuadraticPixels(500000);

        if(pSize)
        {
            // use 100th mm for primitive bitmap converter tool, input is pixel
            // use a real OutDev to get the correct DPI, the static LogicToLogic assumes 72dpi which is wrong (!)
            const Size aSize100th(Application::GetDefaultDevice()->PixelToLogic(*pSize, MapMode(MapUnit::Map100thMM)));

            aRange.expand(basegfx::B2DPoint(aSize100th.Width(), aSize100th.Height()));

            // when explicitly pixels are requested from the GraphicExporter, use a *very* high limit
            // of 16gb (4096x4096 pixels), else use the default for the converters
            nMaximumQuadraticPixels = std::min(sal_uInt32(4096 * 4096), sal_uInt32(pSize->Width() * pSize->Height()));
        }
        else
        {
            // use 100th mm for primitive bitmap converter tool
            const Size aSize100th(OutputDevice::LogicToLogic(rMtf.GetPrefSize(), rMtf.GetPrefMapMode(), MapMode(MapUnit::Map100thMM)));

            aRange.expand(basegfx::B2DPoint(aSize100th.Width(), aSize100th.Height()));
        }

        // get hairline and full bound rect to evtl. correct logic size by the
        // equivalent of one pixel to make those visible at right and bottom
        tools::Rectangle aHairlineRect;
        const tools::Rectangle aRect(rMtf.GetBoundRect(*Application::GetDefaultDevice(), &aHairlineRect));

        if(!aRect.IsEmpty())
        {
            // tdf#105998 Correct the Metafile using information from it's real sizes measured
            // using rMtf.GetBoundRect above and a copy
            const Size aOnePixelInMtf(
                Application::GetDefaultDevice()->PixelToLogic(
                    Size(1, 1),
                    rMtf.GetPrefMapMode()));
            GDIMetaFile aMtf(rMtf);
            const Size aHalfPixelInMtf(
                (aOnePixelInMtf.getWidth() + 1) / 2,
                (aOnePixelInMtf.getHeight() + 1) / 2);
            const bool bHairlineBR(
                !aHairlineRect.IsEmpty() && (aRect.Right() == aHairlineRect.Right() || aRect.Bottom() == aHairlineRect.Bottom()));

            // Move the content to (0,0), usually TopLeft ist slightly
            // negative. For better visualization, add a half pixel, too
            aMtf.Move(
                aHalfPixelInMtf.getWidth() - aRect.Left(),
                aHalfPixelInMtf.getHeight() - aRect.Top());

            // Do not Scale, but set the PrefSize. Some levels deeper the
            // MetafilePrimitive will add a mapping to the decomposition
            // (and possibly a clipping) to map the graphic content to
            // a unit coordinate system.
            // Size is the measured size plus one pixel if needed (bHairlineBR)
            // and the moved half pixwel from above
            aMtf.SetPrefSize(
                Size(
                    aRect.getWidth() + (bHairlineBR ? aOnePixelInMtf.getWidth() : 0) + aHalfPixelInMtf.getWidth(),
                    aRect.getHeight() + (bHairlineBR ? aOnePixelInMtf.getHeight() : 0) + aHalfPixelInMtf.getHeight()));

            return convertMetafileToBitmapEx(aMtf, aRange, nMaximumQuadraticPixels);
        }

        return BitmapEx();
    }

    Size* CalcSize( sal_Int32 nWidth, sal_Int32 nHeight, const Size& aBoundSize, Size& aOutSize )
    {
        if( (nWidth == 0) && (nHeight == 0) )
            return nullptr;

        if( (nWidth == 0) && (nHeight != 0) && (aBoundSize.Height() != 0) )
        {
            nWidth = ( nHeight * aBoundSize.Width() ) / aBoundSize.Height();
        }
        else if( (nWidth != 0) && (nHeight == 0) && (aBoundSize.Width() != 0) )
        {
            nHeight = ( nWidth * aBoundSize.Height() ) / aBoundSize.Width();
        }

        aOutSize.setWidth( nWidth );
        aOutSize.setHeight( nHeight );

        return &aOutSize;
    }

class ImplExportCheckVisisbilityRedirector : public sdr::contact::ViewObjectContactRedirector
{
public:
    explicit ImplExportCheckVisisbilityRedirector( SdrPage* pCurrentPage );

    virtual drawinglayer::primitive2d::Primitive2DContainer createRedirectedPrimitive2DSequence(
        const sdr::contact::ViewObjectContact& rOriginal,
        const sdr::contact::DisplayInfo& rDisplayInfo) override;

private:
    SdrPage* const    mpCurrentPage;
};

ImplExportCheckVisisbilityRedirector::ImplExportCheckVisisbilityRedirector( SdrPage* pCurrentPage )
:   ViewObjectContactRedirector(), mpCurrentPage( pCurrentPage )
{
}

drawinglayer::primitive2d::Primitive2DContainer ImplExportCheckVisisbilityRedirector::createRedirectedPrimitive2DSequence(
    const sdr::contact::ViewObjectContact& rOriginal,
    const sdr::contact::DisplayInfo& rDisplayInfo)
{
    SdrObject* pObject = rOriginal.GetViewContact().TryToGetSdrObject();

    if(pObject)
    {
        SdrPage* pPage = mpCurrentPage;

        if(nullptr == pPage)
        {
            pPage = pObject->getSdrPageFromSdrObject();
        }

        if( (pPage == nullptr) || pPage->checkVisibility(rOriginal, rDisplayInfo, false) )
        {
            return sdr::contact::ViewObjectContactRedirector::createRedirectedPrimitive2DSequence(rOriginal, rDisplayInfo);
        }

        return drawinglayer::primitive2d::Primitive2DContainer();
    }
    else
    {
        // not an object, maybe a page
        return sdr::contact::ViewObjectContactRedirector::createRedirectedPrimitive2DSequence(rOriginal, rDisplayInfo);
    }
}

GraphicExporter::GraphicExporter()
: mpUnoPage( nullptr ), mnPageNumber(-1), mpCurrentPage(nullptr), mpDoc( nullptr )
{
}

IMPL_LINK(GraphicExporter, CalcFieldValueHdl, EditFieldInfo*, pInfo, void)
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
            if( dynamic_cast<const SvxPageField*>( pField) )
            {
                OUString aPageNumValue;
                bool bUpper = false;

                switch(mpDoc->GetPageNumType())
                {
                    case css::style::NumberingType::CHARS_UPPER_LETTER:
                        aPageNumValue += OUStringLiteral1( (mnPageNumber - 1) % 26 + 'A' );
                        break;
                    case css::style::NumberingType::CHARS_LOWER_LETTER:
                        aPageNumValue += OUStringLiteral1( (mnPageNumber - 1) % 26 + 'a' );
                        break;
                    case css::style::NumberingType::ROMAN_UPPER:
                        bUpper = true;
                        [[fallthrough]];
                    case css::style::NumberingType::ROMAN_LOWER:
                        aPageNumValue += SvxNumberFormat::CreateRomanString(mnPageNumber, bUpper);
                        break;
                    case css::style::NumberingType::NUMBER_NONE:
                        aPageNumValue = " ";
                        break;
                    default:
                        aPageNumValue += OUString::number( mnPageNumber );
                }

                pInfo->SetRepresentation( aPageNumValue );

                return;
            }
        }
    }

    maOldCalcFieldValueHdl.Call( pInfo );

    if( pInfo && mpCurrentPage )
        pInfo->SetSdrPage( nullptr );
}

/** creates an virtual device for the given page

    @return the returned VirtualDevice is owned by the caller
*/
VclPtr<VirtualDevice> GraphicExporter::CreatePageVDev( SdrPage* pPage, sal_uIntPtr nWidthPixel, sal_uIntPtr nHeightPixel ) const
{
    VclPtr<VirtualDevice>  pVDev = VclPtr<VirtualDevice>::Create();
    MapMode         aMM( MapUnit::Map100thMM );

    Point aPoint( 0, 0 );
    Size aPageSize(pPage->GetSize());

    // use scaling?
    if( nWidthPixel )
    {
        const Fraction aFrac( static_cast<long>(nWidthPixel), pVDev->LogicToPixel( aPageSize, aMM ).Width() );

        aMM.SetScaleX( aFrac );

        if( nHeightPixel == 0 )
            aMM.SetScaleY( aFrac );
    }

    if( nHeightPixel )
    {
        const Fraction aFrac( static_cast<long>(nHeightPixel), pVDev->LogicToPixel( aPageSize, aMM ).Height() );

        if( nWidthPixel == 0 )
            aMM.SetScaleX( aFrac );

        aMM.SetScaleY( aFrac );
    }

    pVDev->SetMapMode( aMM );
    bool bSuccess(false);

    // #i122820# If available, use pixel size directly
    if(nWidthPixel && nHeightPixel)
    {
        bSuccess = pVDev->SetOutputSizePixel(Size(nWidthPixel, nHeightPixel));
    }
    else
    {
        bSuccess = pVDev->SetOutputSize(aPageSize);
    }

    if(bSuccess)
    {
        std::unique_ptr<SdrView> pView(new SdrView(*mpDoc, pVDev));

        pView->SetPageVisible( false );
        pView->SetBordVisible( false );
        pView->SetGridVisible( false );
        pView->SetHlplVisible( false );
        pView->SetGlueVisible( false );
        pView->ShowSdrPage(pPage);

        vcl::Region aRegion (tools::Rectangle( aPoint, aPageSize ) );

        ImplExportCheckVisisbilityRedirector aRedirector( mpCurrentPage );

        pView->CompleteRedraw(pVDev, aRegion, &aRedirector);
    }
    else
    {
        OSL_ENSURE(false, "Could not get a VirtualDevice of requested size (!)");
    }

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
        else if( pValues->Name == "Width" )  // for compatibility reasons, deprecated
        {
            pValues->Value >>= rSettings.mnWidth;
        }
        else if( pValues->Name == "Height" ) // for compatibility reasons, deprecated
        {
            pValues->Value >>= rSettings.mnHeight;
        }
        else if( pValues->Name == "ExportOnlyBackground" )   // for compatibility reasons, deprecated
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
                else if( pDataValues->Name == "Width" )  // for compatibility reasons, deprecated
                {
                    pDataValues->Value >>= rSettings.mnWidth;
                    pDataValues->Name = "PixelWidth";
                }
                else if( pDataValues->Name == "Height" ) // for compatibility reasons, deprecated
                {
                    pDataValues->Value >>= rSettings.mnHeight;
                    pDataValues->Name = "PixelHeight";
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
                else if (pDataValues->Name == "AntiAliasing")
                {
                    bool bAntiAliasing;
                    if (pDataValues->Value >>= bAntiAliasing)
                        rSettings.meAntiAliasing = bAntiAliasing ? TRISTATE_TRUE : TRISTATE_FALSE;
                }

                pDataValues++;
            }
        }

        pValues++;
    }

    // putting the StatusIndicator that we got from the MediaDescriptor into our local FilterData copy
    if ( rSettings.mxStatusIndicator.is() )
    {
        int i = rSettings.maFilterData.getLength();
        rSettings.maFilterData.realloc( i + 1 );
        rSettings.maFilterData[ i ].Name = "StatusIndicator";
        rSettings.maFilterData[ i ].Value <<= rSettings.mxStatusIndicator;
    }
}

bool GraphicExporter::GetGraphic( ExportSettings const & rSettings, Graphic& aGraphic, bool bVectorType )
{
    if( !mpDoc || !mpUnoPage )
        return false;

    SdrPage* pPage = mpUnoPage->GetSdrPage();
    if( !pPage )
        return false;

    ScopedVclPtrInstance< VirtualDevice > aVDev;
    const MapMode       aMap( mpDoc->GetScaleUnit(), Point(), rSettings.maScaleX, rSettings.maScaleY );

    SdrOutliner& rOutl=mpDoc->GetDrawOutliner();
    maOldCalcFieldValueHdl = rOutl.GetCalcFieldValueHdl();
    rOutl.SetCalcFieldValueHdl( LINK(this, GraphicExporter, CalcFieldValueHdl) );
    rOutl.SetBackgroundColor( pPage->GetPageBackgroundColor() );

    // #i102251#
    const EEControlBits nOldCntrl(rOutl.GetControlWord());
    EEControlBits nCntrl = nOldCntrl & ~EEControlBits::ONLINESPELLING;
    rOutl.SetControlWord(nCntrl);

    SdrObject* pTempBackgroundShape = nullptr;
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
                pTempBackgroundShape = new SdrRectObj(
                    *mpDoc,
                    tools::Rectangle(Point(0,0), pPage->GetSize()));
                pTempBackgroundShape->SetMergedItemSet(pCorrectProperties->GetItemSet());
                pTempBackgroundShape->SetMergedItem(XLineStyleItem(drawing::LineStyle_NONE));
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

                        double fWidthDif = static_cast<double>(aSizePix.Width()) / nWidthPix;
                        double fHeightDif = static_cast<double>(aSizePix.Height()) / nHeightPix;

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

                std::unique_ptr<SdrView> xLocalView;

                if (FmFormModel* pFormModel = dynamic_cast<FmFormModel*>(mpDoc))
                {
                    xLocalView.reset(new FmFormView(*pFormModel, aVDev));
                }
                else
                {
                    xLocalView.reset(new SdrView(*mpDoc, aVDev));
                }

                ScopedVclPtr<VirtualDevice> pVDev(CreatePageVDev( pPage, nWidthPix, nHeightPix ));

                if( pVDev )
                {
                    aGraphic = pVDev->GetBitmapEx( Point(), pVDev->GetOutputSize() );
                    aGraphic.SetPrefMapMode( aMap );
                    aGraphic.SetPrefSize( aSize );
                }
            }
            // create a metafile to export a vector format
            else
            {
                GDIMetaFile aMtf;

                aVDev->SetMapMode( aMap );
                if( rSettings.mbUseHighContrast )
                    aVDev->SetDrawMode( aVDev->GetDrawMode() | DrawModeFlags::SettingsLine | DrawModeFlags::SettingsFill | DrawModeFlags::SettingsText | DrawModeFlags::SettingsGradient );
                aVDev->EnableOutput( false );
                aMtf.Record( aVDev );
                Size aNewSize;

                // create a view
                std::unique_ptr< SdrView > pView;

                if (FmFormModel *pFormModel = dynamic_cast<FmFormModel*>(mpDoc))
                {
                    pView.reset(new FmFormView(*pFormModel, aVDev));
                }
                else
                {
                    pView.reset(new SdrView(*mpDoc, aVDev));
                }

                pView->SetBordVisible( false );
                pView->SetPageVisible( false );
                pView->ShowSdrPage( pPage );

                // tdf#96922 completely deactivate EditView PageVisualization, including
                // PageBackground (formerly 'wiese').
                pView->SetPagePaintingAllowed(false);

                const Point aNewOrg( pPage->GetLeftBorder(), pPage->GetUpperBorder() );
                aNewSize = Size( aSize.Width() - pPage->GetLeftBorder() - pPage->GetRightBorder(),
                                 aSize.Height() - pPage->GetUpperBorder() - pPage->GetLowerBorder() );
                const tools::Rectangle aClipRect( aNewOrg, aNewSize );
                MapMode         aVMap( aMap );

                aVDev->Push();
                aVMap.SetOrigin( Point( -aNewOrg.X(), -aNewOrg.Y() ) );
                aVDev->SetRelativeMapMode( aVMap );
                aVDev->IntersectClipRegion( aClipRect );

                // Use new StandardCheckVisisbilityRedirector
                ImplExportCheckVisisbilityRedirector aRedirector( mpCurrentPage );

                pView->CompleteRedraw(aVDev, vcl::Region(tools::Rectangle(aNewOrg, aNewSize)), &aRedirector);

                aVDev->Pop();

                aMtf.Stop();
                aMtf.WindStart();
                aMtf.SetPrefMapMode( aMap );
                aMtf.SetPrefSize( aNewSize );

                // AW: Here the current version was filtering out the MetaActionType::CLIPREGIONs
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
                    aGraphic = GetBitmapFromMetaFile( aGraphic.GetGDIMetaFile(), CalcSize( rSettings.mnWidth, rSettings.mnHeight, aNewSize, aOutSize ) );
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
                if( dynamic_cast<const SdrGrafObj*>( pObj) && !static_cast<SdrGrafObj*>(pObj)->HasText() )
                {
                    aGraphic = static_cast<SdrGrafObj*>(pObj)->GetTransformedGraphic();
                    if ( aGraphic.GetType() == GraphicType::Bitmap )
                    {
                        Size aSizePixel( aGraphic.GetSizePixel() );
                        if( rSettings.mnWidth && rSettings.mnHeight &&
                            ( ( rSettings.mnWidth != aSizePixel.Width() ) ||
                              ( rSettings.mnHeight != aSizePixel.Height() ) ) )
                        {
                            BitmapEx aBmpEx( aGraphic.GetBitmapEx() );
                            // export: use highest quality
                            aBmpEx.Scale( Size( rSettings.mnWidth, rSettings.mnHeight ), BmpScaleFlag::Lanczos );
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
                if( dynamic_cast<const SdrTextObj*>( pObj)
                    && static_cast<SdrTextObj*>(pObj)->HasText() )
                {
                    tools::Rectangle aScrollRectangle;
                    tools::Rectangle aPaintRectangle;

                    const std::unique_ptr< GDIMetaFile > pMtf(
                        static_cast<SdrTextObj*>(pObj)->GetTextScrollMetaFileAndRectangle(
                           aScrollRectangle, aPaintRectangle ) );

                    // take the larger one of the two rectangles (that
                    // should be the bound rect of the retrieved
                    // metafile)
                    tools::Rectangle aTextRect;

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
                                         sizeof( tools::Rectangle ) ) );
                    pMtf->AddAction( new MetaCommentAction(
                                         "XTEXT_PAINTRECT", 0,
                                         reinterpret_cast<sal_uInt8 const*>(&aPaintRectangle),
                                         sizeof( tools::Rectangle ) ) );

                    aGraphic = Graphic( *pMtf );

                    bSingleGraphic = true;
                }
            }
        }

        if( !bSingleGraphic )
        {
            // create a metafile for all shapes
            ScopedVclPtrInstance< VirtualDevice > aOut;

            // calculate bound rect for all shapes
            tools::Rectangle aBound;

            {
                for( SdrObject* pObj : aShapes )
                {
                    tools::Rectangle aR1(pObj->GetCurrentBoundRect());
                    if (aBound.IsEmpty())
                        aBound=aR1;
                    else
                        aBound.Union(aR1);
                }
            }

            aOut->EnableOutput( false );
            aOut->SetMapMode( aMap );
            if( rSettings.mbUseHighContrast )
                aOut->SetDrawMode( aOut->GetDrawMode() | DrawModeFlags::SettingsLine | DrawModeFlags::SettingsFill | DrawModeFlags::SettingsText | DrawModeFlags::SettingsGradient );

            GDIMetaFile aMtf;
            aMtf.Clear();
            aMtf.Record( aOut );

            MapMode aOutMap( aMap );
            aOutMap.SetOrigin( Point( -aBound.TopLeft().X(), -aBound.TopLeft().Y() ) );
            aOut->SetRelativeMapMode( aOutMap );

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
                sdr::contact::ObjectContactOfObjListPainter aMultiObjectPainter(*aOut, aShapes, mpCurrentPage);
                ImplExportCheckVisisbilityRedirector aCheckVisibilityRedirector(mpCurrentPage);
                aMultiObjectPainter.SetViewObjectContactRedirector(&aCheckVisibilityRedirector);

                aMultiObjectPainter.ProcessDisplay(aDisplayInfo);
            }

            aMtf.Stop();
            aMtf.WindStart();

            const Size  aExtSize( aOut->PixelToLogic( Size( 0, 0  ) ) );
            Size        aBoundSize( aBound.GetWidth() + ( aExtSize.Width() ),
                                    aBound.GetHeight() + ( aExtSize.Height() ) );

            aMtf.SetPrefMapMode( aMap );
            aMtf.SetPrefSize( aBoundSize );

            if( !bVectorType )
            {
                Size aOutSize;
                aGraphic = GetBitmapFromMetaFile( aMtf, CalcSize( rSettings.mnWidth, rSettings.mnHeight, aBoundSize, aOutSize ) );
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
{
    ::SolarMutexGuard aGuard;

    if( !maGraphic && nullptr == mpUnoPage )
        return false;

    if( !maGraphic && ( nullptr == mpUnoPage->GetSdrPage() || nullptr == mpDoc ) )
        return false;

    GraphicFilter &rFilter = GraphicFilter::GetGraphicFilter();

    // get the arguments from the descriptor
    ExportSettings aSettings(mpDoc);
    ParseSettings(aDescriptor, aSettings);

    const sal_uInt16    nFilter = !aSettings.maMediaType.isEmpty()
                            ? rFilter.GetExportFormatNumberForMediaType( aSettings.maMediaType )
                            : rFilter.GetExportFormatNumberForShortName( aSettings.maFilterName );
    bool            bVectorType = !rFilter.IsExportPixelFormat( nFilter );

    // create the output stuff
    Graphic aGraphic = maGraphic;

    ErrCode nStatus = ERRCODE_NONE;
    if (!maGraphic)
    {
        SvtOptionsDrawinglayer aOptions;
        bool bAntiAliasing = aOptions.IsAntiAliasing();
        AllSettings aAllSettings = Application::GetSettings();
        StyleSettings aStyleSettings = aAllSettings.GetStyleSettings();
        bool bUseFontAAFromSystem = aStyleSettings.GetUseFontAAFromSystem();
        if (aSettings.meAntiAliasing != TRISTATE_INDET)
        {
            // This is safe to do globally as we own the solar mutex.
            aOptions.SetAntiAliasing(aSettings.meAntiAliasing == TRISTATE_TRUE);
            // Opt in to have AA affect font rendering as well.
            aStyleSettings.SetUseFontAAFromSystem(false);
            aAllSettings.SetStyleSettings(aStyleSettings);
            Application::SetSettings(aAllSettings);
        }
        nStatus = GetGraphic( aSettings, aGraphic, bVectorType ) ? ERRCODE_NONE : ERRCODE_GRFILTER_FILTERERROR;
        if (aSettings.meAntiAliasing != TRISTATE_INDET)
        {
            aOptions.SetAntiAliasing(bAntiAliasing);
            aStyleSettings.SetUseFontAAFromSystem(bUseFontAAFromSystem);
            aAllSettings.SetStyleSettings(aStyleSettings);
            Application::SetSettings(aAllSettings);
        }
    }

    if( nStatus == ERRCODE_NONE )
    {
        // export graphic only if it has a size
        const Size aGraphSize( aGraphic.GetPrefSize() );
        if ( ( aGraphSize.Width() == 0 ) || ( aGraphSize.Height() == 0 ) )
        {
            nStatus = ERRCODE_GRFILTER_FILTERERROR;
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

                nStatus = rFilter.ExportGraphic( aGraphic,"", aStream, nFilter, &aSettings.maFilterData );

                // copy temp stream to XOutputStream
                SvOutputStream aOutputStream( aSettings.mxOutputStream );
                aStream.Seek(0);
                aOutputStream.WriteStream( aStream );
            }
            else
            {
                INetURLObject aURLObject( aSettings.maURL.Complete );
                DBG_ASSERT( aURLObject.GetProtocol() != INetProtocol::NotValid, "invalid URL" );

                nStatus = XOutBitmap::ExportGraphic( aGraphic, aURLObject, rFilter, nFilter, &aSettings.maFilterData );
            }
        }
    }

    if ( aSettings.mxInteractionHandler.is() && ( nStatus != ERRCODE_NONE ) )
    {
        Any aInteraction;
        Sequence< css::uno::Reference< css::task::XInteractionContinuation > > lContinuations(1);
        ::comphelper::OInteractionApprove* pApprove = new ::comphelper::OInteractionApprove();
        lContinuations[0].set(static_cast< XInteractionContinuation* >(pApprove), UNO_QUERY);

        GraphicFilterRequest aErrorCode;
        aErrorCode.ErrCode = sal_uInt32(nStatus);
        aInteraction <<= aErrorCode;
        aSettings.mxInteractionHandler->handle( framework::InteractionRequest::CreateRequest( aInteraction, lContinuations ) );
    }
    return nStatus == ERRCODE_NONE;
}

void SAL_CALL GraphicExporter::cancel()
{
}

// XExporter

/** the source 'document' could be a XDrawPage, a XShape or a generic XShapes */
void SAL_CALL GraphicExporter::setSourceDocument( const Reference< lang::XComponent >& xComponent )
{
    ::SolarMutexGuard aGuard;

    mxShapes = nullptr;
    mpUnoPage = nullptr;

    try
    {
    // any break inside this one loop while will throw a IllegalArgumentException
    do
    {
        mxPage.set( xComponent, UNO_QUERY );
        mxShapes.set( xComponent, UNO_QUERY );
        mxShape.set( xComponent, UNO_QUERY );

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
            mxShapes = nullptr;
        }

        // Step 2: try a shape
        if( mxShape.is() )
        {
            if( nullptr == GetSdrObjectFromXShape( mxShape ) )
            {
                // This is not a Draw shape, let's see if it's a Writer one.
                uno::Reference<beans::XPropertySet> xPropertySet(mxShape, uno::UNO_QUERY);
                if (!xPropertySet.is())
                    break;
                uno::Reference<graphic::XGraphic> xGraphic(
                    xPropertySet->getPropertyValue("Graphic"), uno::UNO_QUERY);
                if (!xGraphic.is())
                    break;

                maGraphic = Graphic(xGraphic);
                if (maGraphic)
                    return;
                else
                    break;
            }

            // get page for this shape
            Reference< XChild > xChild( mxShape, UNO_QUERY );
            if( !xChild.is() )
                break;

            Reference< XInterface > xInt;
            do
            {
                xInt = xChild->getParent();
                mxPage.set( xInt, UNO_QUERY );
                if( !mxPage.is() )
                    xChild.set( xInt, UNO_QUERY );
            }
            while( !mxPage.is() && xChild.is() );

            if( !mxPage.is() )
                break;
        }

        // Step 3: check the page
        if( !mxPage.is() )
            break;

        mpUnoPage = SvxDrawPage::getImplementation( mxPage );

        if( nullptr == mpUnoPage || nullptr == mpUnoPage->GetSdrPage() )
            break;

        mpDoc = &mpUnoPage->GetSdrPage()->getSdrModelFromSdrPage();

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
                bOk = pObj && pObj->getSdrPageFromSdrObject() == pPage;
            }

            if( !bOk )
                break;
        }

        // no errors so far
        return;
    }
    while( false );
    }
    catch( Exception& )
    {
    }

    throw IllegalArgumentException();
}

// XServiceInfo
OUString SAL_CALL GraphicExporter::getImplementationName(  )
{
    return OUString( "com.sun.star.comp.Draw.GraphicExporter" );
}

sal_Bool SAL_CALL GraphicExporter::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

Sequence< OUString > SAL_CALL GraphicExporter::getSupportedServiceNames(  )
{
    Sequence< OUString > aSupportedServiceNames { "com.sun.star.drawing.GraphicExportFilter" };
    return aSupportedServiceNames;
}

// XMimeTypeInfo
sal_Bool SAL_CALL GraphicExporter::supportsMimeType( const OUString& rMimeTypeName )
{
    GraphicFilter &rFilter = GraphicFilter::GetGraphicFilter();
    sal_uInt16 nCount = rFilter.GetExportFormatCount();
    sal_uInt16 nFilter;
    for( nFilter = 0; nFilter < nCount; nFilter++ )
    {
        if( rMimeTypeName == rFilter.GetExportFormatMediaType( nFilter ) )
        {
            return true;
        }
    }

    return false;
}

Sequence< OUString > SAL_CALL GraphicExporter::getSupportedMimeTypeNames(  )
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

}

Graphic SvxGetGraphicForShape( SdrObject& rShape )
{
    Graphic aGraphic;
    try
    {
        rtl::Reference< GraphicExporter > xExporter( new GraphicExporter() );
        Reference< XComponent > xComp( rShape.getUnoShape(), UNO_QUERY_THROW );
        xExporter->setSourceDocument( xComp );
        ExportSettings aSettings(&rShape.getSdrModelFromSdrObject());
        xExporter->GetGraphic( aSettings, aGraphic, true/*bVector*/ );
    }
    catch( Exception& )
    {
        OSL_FAIL("SvxGetGraphicForShape(), exception caught!");
    }
    return aGraphic;
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_Draw_GraphicExporter_get_implementation(
    css::uno::XComponentContext *,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new GraphicExporter);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
