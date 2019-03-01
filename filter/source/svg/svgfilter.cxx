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


#include <cstdio>

#include <sal/log.hxx>
#include <comphelper/lok.hxx>
#include <comphelper/servicedecl.hxx>
#include <uno/environment.h>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XDrawView.hpp>
#include <com/sun/star/drawing/XMasterPagesSupplier.hpp>
#include <com/sun/star/drawing/XMasterPageTarget.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/io/IOException.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/drawing/XDrawSubController.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/drawing/framework/XControllerManager.hpp>
#include <com/sun/star/drawing/framework/XConfigurationController.hpp>
#include <com/sun/star/drawing/framework/XConfiguration.hpp>
#include <com/sun/star/drawing/framework/AnchorBindingMode.hpp>
#include <com/sun/star/drawing/framework/XResourceId.hpp>
#include <com/sun/star/drawing/framework/XResource.hpp>
#include <com/sun/star/drawing/framework/XView.hpp>
#include <com/sun/star/drawing/framework/ResourceId.hpp>

#include <unotools/mediadescriptor.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/streamwrap.hxx>
#include <tools/zcodec.hxx>

#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>

#include "svgfilter.hxx"
#include "svgwriter.hxx"

#include <svx/unopage.hxx>
#include <vcl/graphicfilter.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdograf.hxx>
#include <svl/itempool.hxx>

#include <memory>

using namespace ::com::sun::star;

namespace
{
    static const char constFilterNameDraw[] = "svg_Scalable_Vector_Graphics_Draw";
    static const char constFilterName[] = "svg_Scalable_Vector_Graphics";
}

SVGFilter::SVGFilter( const Reference< XComponentContext >& rxCtx ) :
    mxContext( rxCtx ),
    mpSVGDoc( nullptr ),
    mpSVGExport( nullptr ),
    mpSVGFontExport( nullptr ),
    mpSVGWriter( nullptr ),
    mbSinglePage( false ),
    mnVisiblePage( -1 ),
    mpObjects( nullptr ),
    mxSrcDoc(),
    mxDstDoc(),
    maShapeSelection(),
    mbExportShapeSelection(false),
    maFilterData(),
    mxDefaultPage(),
    mbWriterFilter(false),
    mbCalcFilter(false),
    mbImpressFilter(false),
    mpDefaultSdrPage( nullptr ),
    mbPresentation( false ),
    maOldFieldHdl()
{
}

SVGFilter::~SVGFilter()
{
    DBG_ASSERT( mpSVGDoc == nullptr, "mpSVGDoc not destroyed" );
    DBG_ASSERT( mpSVGExport == nullptr, "mpSVGExport not destroyed" );
    DBG_ASSERT( mpSVGFontExport == nullptr, "mpSVGFontExport not destroyed" );
    DBG_ASSERT( mpSVGWriter == nullptr, "mpSVGWriter not destroyed" );
    DBG_ASSERT( mpObjects == nullptr, "mpObjects not destroyed" );
}

sal_Bool SAL_CALL SVGFilter::filter( const Sequence< PropertyValue >& rDescriptor )
{
    mbWriterFilter = false;
    mbCalcFilter = false;
    mbImpressFilter = false;

    if(mxDstDoc.is()) // Import works for Impress / draw only
        return filterImpressOrDraw(rDescriptor);

    if(mxSrcDoc.is())
    {
        for (sal_Int32 nInd = 0; nInd < rDescriptor.getLength(); nInd++)
        {
            if (rDescriptor[nInd].Name == "FilterName")
            {
                OUString sFilterName;
                rDescriptor[nInd].Value >>= sFilterName;
                if(sFilterName == "impress_svg_Export")
                {
                    mbImpressFilter = true;
                    return filterImpressOrDraw(rDescriptor);
                }
                else if(sFilterName == "writer_svg_Export")
                {
                    mbWriterFilter = true;
                    return filterWriterOrCalc(rDescriptor);
                }
                else if(sFilterName == "calc_svg_Export")
                {
                    mbCalcFilter = true;
                    return filterWriterOrCalc(rDescriptor);
                }
                break;
            }
        }
        return filterImpressOrDraw(rDescriptor);
    }
    return false;
}

bool SVGFilter::filterImpressOrDraw( const Sequence< PropertyValue >& rDescriptor )
{
    SolarMutexGuard aGuard;
    vcl::Window* pFocusWindow(Application::GetFocusWindow());
    bool bRet(false);

    if(pFocusWindow)
    {
        pFocusWindow->EnterWait();
    }

    if(mxDstDoc.is())
    {
        // Import. Use an endless loop to have easy exits for error handling
        while(true)
        {
            // use MediaDescriptor to get needed data out of Sequence< PropertyValue >
            utl::MediaDescriptor aMediaDescriptor(rDescriptor);
            uno::Reference<io::XInputStream> xInputStream;

            xInputStream.set(aMediaDescriptor[utl::MediaDescriptor::PROP_INPUTSTREAM()], UNO_QUERY);

            if(!xInputStream.is())
            {
                // we need the InputStream
                break;
            }

            // get the DrawPagesSupplier
            uno::Reference< drawing::XDrawPagesSupplier > xDrawPagesSupplier( mxDstDoc, uno::UNO_QUERY );

            if(!xDrawPagesSupplier.is())
            {
                // we need the DrawPagesSupplier
                break;
            }

            // get the DrawPages
            uno::Reference< drawing::XDrawPages > xDrawPages( xDrawPagesSupplier->getDrawPages(), uno::UNO_QUERY );

            if(!xDrawPages.is())
            {
                // we need the DrawPages
                break;
            }

            // check DrawPageCount (there should be one by default)
            sal_Int32 nDrawPageCount(xDrawPages->getCount());

            if(0 == nDrawPageCount)
            {
                // at least one DrawPage should be there - we need that
                break;
            }

            // get that DrawPage
            uno::Reference< drawing::XDrawPage > xDrawPage( xDrawPages->getByIndex(0), uno::UNO_QUERY );

            if(!xDrawPage.is())
            {
                // we need that DrawPage
                break;
            }

            // get that DrawPage's UNO API implementation
            SvxDrawPage* pSvxDrawPage(SvxDrawPage::getImplementation(xDrawPage));

            if(nullptr == pSvxDrawPage || nullptr == pSvxDrawPage->GetSdrPage())
            {
                // we need a SvxDrawPage
                break;
            }

            // get the SvStream to work with
            std::unique_ptr< SvStream > aStream(utl::UcbStreamHelper::CreateStream(xInputStream, true));

            if (!aStream)
            {
                // we need the SvStream
                break;
            }

            // create a GraphicFilter and load the SVG (format already known, thus *could*
            // be handed over to ImportGraphic - but detection is fast).
            // As a bonus, zipped data is already detected and handled there
            GraphicFilter aGraphicFilter;
            Graphic aGraphic;
            const ErrCode nGraphicFilterErrorCode(
                aGraphicFilter.ImportGraphic(aGraphic, OUString(), *aStream));

            if(ERRCODE_NONE != nGraphicFilterErrorCode)
            {
                // SVG import error, cannot continue
                break;
            }

            // get the GraphicPrefSize early to check if we have any content
            // (the SVG may contain nothing and/or just <g visibility="hidden"> stuff...)
            const Size aGraphicPrefSize(aGraphic.GetPrefSize());

            if(0 == aGraphicPrefSize.Width() || 0 == aGraphicPrefSize.Height())
            {
                // SVG has no displayable content, stop import.
                // Also possible would be to get the sequence< Primitives >
                // from aGraphic and check if it is empty.
                // Possibility to set some error message here to tell
                // the user what/why loading went wrong, but I do not
                // know how this could be done here
                break;
            }

            // tdf#118232 Get the sequence of primitives and check if geometry is completely
            // hidden. If so, there is no need to add a SdrObject at all
            const VectorGraphicDataPtr& rVectorGraphicData(aGraphic.getVectorGraphicData());
            bool bContainsNoGeometry(false);

            if(bool(rVectorGraphicData) && VectorGraphicDataType::Svg == rVectorGraphicData->getVectorGraphicDataType())
            {
                const drawinglayer::primitive2d::Primitive2DContainer aContainer(rVectorGraphicData->getPrimitive2DSequence());

                if(!aContainer.empty())
                {
                    bool bAllAreHiddenGeometry(true);

                    for(const auto& rCandidate : aContainer)
                    {
                        if(rCandidate.is())
                        {
                            // try to cast to BasePrimitive2D implementation
                            const drawinglayer::primitive2d::BasePrimitive2D* pBasePrimitive(
                                dynamic_cast< const drawinglayer::primitive2d::BasePrimitive2D* >(rCandidate.get()));

                            if(pBasePrimitive && PRIMITIVE2D_ID_HIDDENGEOMETRYPRIMITIVE2D != pBasePrimitive->getPrimitive2DID())
                            {
                                bAllAreHiddenGeometry = false;
                                break;
                            }
                        }
                    }

                    if(bAllAreHiddenGeometry)
                    {
                        bContainsNoGeometry = true;
                    }
                }
            }

            // create a SdrModel-GraphicObject to insert to page
            SdrPage* pTargetSdrPage(pSvxDrawPage->GetSdrPage());
            std::unique_ptr< SdrGrafObj, SdrObjectFreeOp > aNewSdrGrafObj;

            // tdf#118232 only add an SdrGrafObj when we have Geometry
            if(!bContainsNoGeometry)
            {
                aNewSdrGrafObj.reset(
                    new SdrGrafObj(
                        pTargetSdrPage->getSdrModelFromSdrPage(),
                        aGraphic));
            }

            // Evtl. adapt the GraphicPrefSize to target-MapMode of target-Model
            // (should be 100thmm here, but just stay safe by doing the conversion)
            const MapMode aGraphicPrefMapMode(aGraphic.GetPrefMapMode());
            const MapUnit eDestUnit(pTargetSdrPage->getSdrModelFromSdrPage().GetItemPool().GetMetric(0));
            const MapUnit eSrcUnit(aGraphicPrefMapMode.GetMapUnit());
            Size aGraphicSize(aGraphicPrefSize);

            if (eDestUnit != eSrcUnit)
            {
                aGraphicSize = Size(
                    OutputDevice::LogicToLogic(aGraphicSize.Width(), eSrcUnit, eDestUnit),
                    OutputDevice::LogicToLogic(aGraphicSize.Height(), eSrcUnit, eDestUnit));
            }

            // Based on GraphicSize, set size of Page. Do not forget to adapt PageBorders,
            // but interpret them relative to PageSize so that they do not 'explode/shrink'
            // in comparison. Use a common scaling factor for hor/ver to not get
            // asynchronous border distances, though. All in all this will adapt borders
            // nicely and is based on office-defaults for standard-page-border-sizes.
            const Size aPageSize(pTargetSdrPage->GetSize());
            const double fBorderRelation((
                static_cast< double >(pTargetSdrPage->GetLeftBorder()) / aPageSize.Width() +
                static_cast< double >(pTargetSdrPage->GetRightBorder()) / aPageSize.Width() +
                static_cast< double >(pTargetSdrPage->GetUpperBorder()) / aPageSize.Height() +
                static_cast< double >(pTargetSdrPage->GetLowerBorder()) / aPageSize.Height()) / 4.0);
            const long nAllBorder(basegfx::fround((aGraphicSize.Width() + aGraphicSize.Height()) * fBorderRelation * 0.5));

            // Adapt PageSize and Border stuff. To get all MasterPages and PresObjs
            // correctly adapted, do not just use
            //      pTargetSdrPage->SetBorder(...) and
            //      pTargetSdrPage->SetSize(...),
            // but ::adaptSizeAndBorderForAllPages
            // Do use original Size and borders to get as close to original
            // as possible for better turn-arounds.
            pTargetSdrPage->getSdrModelFromSdrPage().adaptSizeAndBorderForAllPages(
                Size(
                    aGraphicSize.Width(),
                    aGraphicSize.Height()),
                nAllBorder,
                nAllBorder,
                nAllBorder,
                nAllBorder);

            // tdf#118232 set pos/size at SdrGraphicObj - use zero position for
            // better turn-around results
            if(!bContainsNoGeometry)
            {
                aNewSdrGrafObj->SetSnapRect(
                    tools::Rectangle(
                        Point(0, 0),
                        aGraphicSize));

                // insert to page (owner change of SdrGrafObj)
                pTargetSdrPage->InsertObject(aNewSdrGrafObj.release());
            }

            // done - set positive result now
            bRet = true;

            // always leave helper endless loop
            break;
        };
    }
    else if( mxSrcDoc.is() )
    {
        // #i124608# detect selection
        bool bSelectionOnly = false;
        bool bGotSelection = false;

        // when using LibreOfficeKit, default to exporting everything (-1)
        bool bPageProvided = comphelper::LibreOfficeKit::isActive();
        sal_Int32 nPageToExport = -1;

        for (sal_Int32 nInd = 0; nInd < rDescriptor.getLength(); nInd++)
        {
            if (rDescriptor[nInd].Name == "SelectionOnly")
            {
                // #i124608# extract single selection wanted from dialog return values
                rDescriptor[nInd].Value >>= bSelectionOnly;
                bPageProvided = false;
            }
            else if (rDescriptor[nInd].Name == "PagePos")
            {
                rDescriptor[nInd].Value >>= nPageToExport;
                bPageProvided = true;
            }
        }

        uno::Reference<frame::XDesktop2> xDesktop(frame::Desktop::create(mxContext));
        uno::Reference<frame::XController > xController;
        if (xDesktop->getCurrentFrame().is() && !bPageProvided) // Manage headless case
        {
            uno::Reference<frame::XFrame> xFrame(xDesktop->getCurrentFrame(), uno::UNO_QUERY_THROW);
            xController.set(xFrame->getController(), uno::UNO_QUERY_THROW);
            uno::Reference<drawing::XDrawView> xDrawView(xController, uno::UNO_QUERY_THROW);
            uno::Reference<drawing::framework::XControllerManager> xManager(xController, uno::UNO_QUERY_THROW);
            uno::Reference<drawing::framework::XConfigurationController> xConfigController(xManager->getConfigurationController());

            // which view configuration are we in?
            //
            // * traverse Impress resources to find slide preview pane, grab selection from there
            // * otherwise, fallback to current slide
            //
            uno::Sequence<uno::Reference<drawing::framework::XResourceId> > aResIds(
                xConfigController->getCurrentConfiguration()->getResources(
                    uno::Reference<drawing::framework::XResourceId>(),
                    "",
                    drawing::framework::AnchorBindingMode_INDIRECT));

            for( sal_Int32 i=0; i<aResIds.getLength(); ++i )
            {
                // can we somehow obtain the slidesorter from the Impress framework?
                if( aResIds[i]->getResourceURL() == "private:resource/view/SlideSorter" )
                {
                    // got it, grab current selection from there
                    uno::Reference<drawing::framework::XResource> xRes(
                        xConfigController->getResource(aResIds[i]));

                    uno::Reference< view::XSelectionSupplier > xSelectionSupplier(
                        xRes,
                        uno::UNO_QUERY );
                    if( xSelectionSupplier.is() )
                    {
                        uno::Any aSelection = xSelectionSupplier->getSelection();
                        if( aSelection.hasValue() )
                        {
                            Sequence< Reference< XInterface > > aSelectedPageSequence;
                            aSelection >>= aSelectedPageSequence;
                            mSelectedPages.resize( aSelectedPageSequence.getLength() );
                            for( size_t j=0; j<mSelectedPages.size(); ++j )
                            {
                                uno::Reference< drawing::XDrawPage > xDrawPage( aSelectedPageSequence[j],
                                                                                uno::UNO_QUERY );
                                mSelectedPages[j] = xDrawPage;
                            }

                            // and stop looping. it is likely not getting better
                            break;
                        }
                    }
                }
            }

            if( mSelectedPages.empty() )
            {
                // apparently failed to clean selection - fallback to current page
                mSelectedPages.resize( 1 );
                mSelectedPages[0] = xDrawView->getCurrentPage();
            }
        }

        /*
         * Export all slides, or requested "PagePos"
         */
        if( mSelectedPages.empty() )
        {
            uno::Reference< drawing::XMasterPagesSupplier > xMasterPagesSupplier( mxSrcDoc, uno::UNO_QUERY );
            uno::Reference< drawing::XDrawPagesSupplier >   xDrawPagesSupplier( mxSrcDoc, uno::UNO_QUERY );

            if( xMasterPagesSupplier.is() && xDrawPagesSupplier.is() )
            {
                uno::Reference< drawing::XDrawPages >   xMasterPages( xMasterPagesSupplier->getMasterPages(), uno::UNO_QUERY );
                uno::Reference< drawing::XDrawPages >   xDrawPages( xDrawPagesSupplier->getDrawPages(), uno::UNO_QUERY );
                if( xMasterPages.is() && xMasterPages->getCount() &&
                    xDrawPages.is() && xDrawPages->getCount() )
                {
                    sal_Int32 nDPCount = xDrawPages->getCount();

                    mSelectedPages.resize( nPageToExport != -1 ? 1 : nDPCount );
                    sal_Int32 i;
                    for( i = 0; i < nDPCount; ++i )
                    {
                        if( nPageToExport != -1 && nPageToExport == i )
                        {
                            uno::Reference< drawing::XDrawPage > xDrawPage( xDrawPages->getByIndex( i ), uno::UNO_QUERY );
                            mSelectedPages[0] = xDrawPage;
                        }
                        else
                        {
                            uno::Reference< drawing::XDrawPage > xDrawPage( xDrawPages->getByIndex( i ), uno::UNO_QUERY );
                            mSelectedPages[i] = xDrawPage;
                        }
                    }
                }
            }
        }

        if (bSelectionOnly)
        {
            // #i124608# when selection only is wanted, get the current object selection
            // from the DrawView
            Reference< view::XSelectionSupplier > xSelection (xController, UNO_QUERY);

            if (xSelection.is())
            {
                bGotSelection
                    = ( xSelection->getSelection() >>= maShapeSelection );
            }
        }

        if(bSelectionOnly && bGotSelection && 0 == maShapeSelection->getCount())
        {
            // #i124608# export selection, got maShapeSelection but no shape selected -> nothing
            // to export, we are done (maybe return true, but a hint that nothing was done
            // may be useful; it may have happened by error)
            bRet = false;
        }
        else
        {
            /*
             *  We get all master page that are targeted by at least one draw page.
             *  The master page are put in an unordered set.
             */
            ObjectSet aMasterPageTargetSet;
            for(uno::Reference<drawing::XDrawPage> & mSelectedPage : mSelectedPages)
            {
                uno::Reference< drawing::XMasterPageTarget > xMasterPageTarget( mSelectedPage, uno::UNO_QUERY );
                if( xMasterPageTarget.is() )
                {
                    aMasterPageTargetSet.insert( xMasterPageTarget->getMasterPage() );
                }
            }
            // Later we move them to a uno::Sequence so we can get them by index
            mMasterPageTargets.resize( aMasterPageTargetSet.size() );
            sal_Int32 i = 0;
            for (auto const& masterPageTarget : aMasterPageTargetSet)
            {
                uno::Reference< drawing::XDrawPage > xMasterPage( masterPageTarget,  uno::UNO_QUERY );
                mMasterPageTargets[i++] = xMasterPage;
            }

            bRet = implExport( rDescriptor );
        }
    }
    else
        bRet = false;

    if( pFocusWindow )
        pFocusWindow->LeaveWait();

    return bRet;
}

bool SVGFilter::filterWriterOrCalc( const Sequence< PropertyValue >& rDescriptor )
{
    bool bSelectionOnly = false;

    for (sal_Int32 nInd = 0; nInd < rDescriptor.getLength(); nInd++)
    {
        if (rDescriptor[nInd].Name == "SelectionOnly")
        {
            rDescriptor[nInd].Value >>= bSelectionOnly;
            break;
        }
    }

    if(!bSelectionOnly) // For Writer only the selection-only mode is supported
        return false;

    uno::Reference<frame::XDesktop2> xDesktop(frame::Desktop::create(mxContext));
    uno::Reference<frame::XController > xController;
    if (xDesktop->getCurrentFrame().is())
    {
        uno::Reference<frame::XFrame> xFrame(xDesktop->getCurrentFrame(), uno::UNO_QUERY_THROW);
        xController.set(xFrame->getController(), uno::UNO_QUERY_THROW);
    }

    Reference< view::XSelectionSupplier > xSelection (xController, UNO_QUERY);
    if (!xSelection.is())
        return false;

    // Select only one draw page
    uno::Reference< drawing::XDrawPagesSupplier > xDrawPagesSupplier( mxSrcDoc, uno::UNO_QUERY );
    uno::Reference<drawing::XDrawPages> xDrawPages = xDrawPagesSupplier->getDrawPages();
    uno::Reference< drawing::XDrawPage > xDrawPage( xDrawPages->getByIndex(0), uno::UNO_QUERY );
    mSelectedPages.resize( 1 );
    mSelectedPages[0] = xDrawPage;

    bool bGotSelection = xSelection->getSelection() >>= maShapeSelection;

    if (!bGotSelection)
    {
        if (mbWriterFilter)
        {
            // For Writer we might have a non-shape graphic
            bGotSelection = implExportWriterTextGraphic(xSelection);
        }
        if (!bGotSelection)
            return false;
    }

    return implExport( rDescriptor );
}

void SAL_CALL SVGFilter::cancel( )
{
}

void SAL_CALL SVGFilter::setSourceDocument( const Reference< XComponent >& xDoc )
{
    mxSrcDoc = xDoc;
}

void SAL_CALL SVGFilter::setTargetDocument( const Reference< XComponent >& xDoc )
{
    mxDstDoc = xDoc;
}

// There is already another SVG-Type_Detector, see
// vcl/source/filter/graphicfilter.cxx ("DOCTYPE svg"),
// but since these start from different preconditions it is not
// easy to unify these. For now, use this local helper.
class SVGFileInfo
{
private:
    const uno::Reference<io::XInputStream>&     mxInput;
    uno::Sequence< sal_Int8 >                   mnFirstBytes;
    sal_Int32                                   mnFirstBytesSize;
    sal_Int32                                   mnFirstRead;
    bool                                        mbProcessed;
    bool                                        mbIsSVG;

    bool impCheckForMagic(
        const sal_Int8* pMagic,
        const sal_Int32 nMagicSize)
    {
        const sal_Int8* pBuffer(mnFirstBytes.getConstArray());
        return std::search(
            pBuffer,
            pBuffer + mnFirstRead,
            pMagic,
            pMagic + nMagicSize) != pBuffer + mnFirstRead;
    }

    void impEnsureProcessed()
    {
        if(mbProcessed)
        {
            return;
        }

        mbProcessed = true;

        if(!mxInput.is())
        {
            return;
        }

        if(0 == mnFirstBytesSize)
        {
            return;
        }

        mnFirstBytes.realloc(mnFirstBytesSize);

        if(mnFirstBytesSize != mnFirstBytes.getLength())
        {
            return;
        }

        std::unique_ptr< SvStream > aStream(utl::UcbStreamHelper::CreateStream(mxInput, true));

        if (!aStream)
        {
            return;
        }

        const sal_uLong nStreamLen(aStream->remainingSize());

        if(aStream->GetError())
        {
            return;
        }

        mnFirstRead = aStream->ReadBytes(
            &mnFirstBytes[0],
            std::min(nStreamLen, sal_uLong(mnFirstBytesSize)));

        if(aStream->GetError())
        {
            return;
        }

        // check if it is gzipped -> svgz
        if (mnFirstBytes[0] == 0x1F && static_cast<sal_uInt8>(mnFirstBytes[1]) == 0x8B)
        {
            ZCodec aCodec;

            aCodec.BeginCompression(
                ZCODEC_DEFAULT_COMPRESSION,
                false,
                true);
            mnFirstRead = aCodec.Read(
                *aStream,
                reinterpret_cast< sal_uInt8* >(mnFirstBytes.getArray()),
                mnFirstBytesSize);
            aCodec.EndCompression();

            if (mnFirstRead < 0)
                return;
        }

        if(!mbIsSVG)
        {
            const sal_Int8 aMagic[] = {'<', 's', 'v', 'g'};
            const sal_Int32 nMagicSize(SAL_N_ELEMENTS(aMagic));

            mbIsSVG = impCheckForMagic(aMagic, nMagicSize);
        }

        if(!mbIsSVG)
        {
            const sal_Int8 aMagic[] = {'D', 'O', 'C', 'T', 'Y', 'P', 'E', ' ', 's', 'v', 'g'};
            const sal_Int32 nMagicSize(SAL_N_ELEMENTS(aMagic));

            mbIsSVG = impCheckForMagic(aMagic, nMagicSize);
        }

        return;
    }

public:
    SVGFileInfo(
        const uno::Reference<io::XInputStream>& xInput)
    :   mxInput(xInput),
        mnFirstBytes(),
        mnFirstBytesSize(2048),
        mnFirstRead(0),
        mbProcessed(false),
        mbIsSVG(false)
    {
        // For the default buffer size: Use not too big
        // (not more than 16K), but also not too small
        // (not less than 1/2K), see comments at
        // ImpPeekGraphicFormat, SVG section.
        // I remember these cases and it *can* happen
        // that SVGs have quite massive comments in their
        // headings (!)
        // Limit to plausible sizes, also for security reasons
        mnFirstBytesSize = std::min(sal_Int32(512), mnFirstBytesSize);
        mnFirstBytesSize = std::max(sal_Int32(16384), mnFirstBytesSize);
    }

    bool isSVG()
    {
        impEnsureProcessed();

        return mbIsSVG;
    }

    bool isOwnFormat()
    {
        impEnsureProcessed();

        if(mbIsSVG)
        {
            // xmlns:ooo
            const sal_Int8 aMagic[] = {'x', 'm', 'l', 'n', 's', ':', 'o', 'o', 'o'};
            const sal_Int32 nMagicSize(SAL_N_ELEMENTS(aMagic));

            return impCheckForMagic(aMagic, nMagicSize);
        }

        return false;
    }

    bool isImpress()
    {
        impEnsureProcessed();

        if(mbIsSVG)
        {
            // ooo:meta_slides
            const sal_Int8 aMagic[] = {'o', 'o', 'o', ':', 'm', 'e', 't', 'a', '_', 's', 'l', 'i', 'd', 'e', 's'};
            const sal_Int32 nMagicSize(SAL_N_ELEMENTS(aMagic));

            return impCheckForMagic(aMagic, nMagicSize);
        }

        return false;
    }
};

OUString SAL_CALL SVGFilter::detect(Sequence<PropertyValue>& rDescriptor)
{
    utl::MediaDescriptor aMediaDescriptor(rDescriptor);
    uno::Reference<io::XInputStream> xInput(aMediaDescriptor[utl::MediaDescriptor::PROP_INPUTSTREAM()], UNO_QUERY);
    OUString aRetval;

    if (!xInput.is())
    {
        return aRetval;
    }

    try
    {
        SVGFileInfo aSVGFileInfo(xInput);

        if(aSVGFileInfo.isSVG())
        {
            // We have SVG - set default document format to Draw
            aRetval = OUString(constFilterNameDraw);

            if(aSVGFileInfo.isOwnFormat())
            {
                // it's a file that was written/exported by LO
                if(aSVGFileInfo.isImpress())
                {
                    // it was written by Impress export. Set document
                    // format for import to Impress
                    aRetval = OUString(constFilterName);
                }
            }
        }
    }
    catch (css::io::IOException & e)
    {
        SAL_WARN("filter.svg", "caught " << e);
    }

    return aRetval;
}

#define SVG_FILTER_IMPL_NAME "com.sun.star.comp.Draw.SVGFilter"
#define SVG_WRITER_IMPL_NAME "com.sun.star.comp.Draw.SVGWriter"

namespace sdecl = comphelper::service_decl;
 sdecl::class_<SVGFilter> const serviceFilterImpl;
 const sdecl::ServiceDecl svgFilter(
     serviceFilterImpl,
     SVG_FILTER_IMPL_NAME,
     "com.sun.star.document.ImportFilter;"
     "com.sun.star.document.ExportFilter;"
     "com.sun.star.document.ExtendedTypeDetection" );

 sdecl::class_<SVGWriter, sdecl::with_args<true> > const serviceWriterImpl;
 const sdecl::ServiceDecl svgWriter(
     serviceWriterImpl,
     SVG_WRITER_IMPL_NAME,
     "com.sun.star.svg.SVGWriter" );

// The C shared lib entry points
extern "C" SAL_DLLPUBLIC_EXPORT void* svgfilter_component_getFactory(
    sal_Char const* pImplName, void*, void*)
{
    if ( rtl_str_compare (pImplName, SVG_FILTER_IMPL_NAME) == 0 )
    {
        return sdecl::component_getFactoryHelper( pImplName, {&svgFilter} );
    }
    else if ( rtl_str_compare (pImplName, SVG_WRITER_IMPL_NAME) == 0 )
    {
        return sdecl::component_getFactoryHelper( pImplName, {&svgWriter} );
    }
    return nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
