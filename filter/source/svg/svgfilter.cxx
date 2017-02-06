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

#include <comphelper/lok.hxx>
#include <comphelper/servicedecl.hxx>
#include <uno/environment.h>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XDrawView.hpp>
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
#include <comphelper/processfactory.hxx>

#include <unotools/mediadescriptor.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/streamwrap.hxx>
#include <tools/zcodec.hxx>

#include <osl/mutex.hxx>

#include "svgfilter.hxx"
#include "svgwriter.hxx"

#include <memory>

using namespace ::com::sun::star;

namespace
{
    static const char constFilterName[] = "svg_Scalable_Vector_Graphics";
}

SVGFilter::SVGFilter( const Reference< XComponentContext >& rxCtx ) :
    mxContext( rxCtx ),
    mpSVGDoc( nullptr ),
    mpSVGExport( nullptr ),
    mpSVGFontExport( nullptr ),
    mpSVGWriter( nullptr ),
    mpDefaultSdrPage( nullptr ),
    mpSdrModel( nullptr ),
    mbPresentation( false ),
    mbSinglePage( false ),
    mnVisiblePage( -1 ),
    mpObjects( nullptr ),
    mxSrcDoc(),
    mxDstDoc(),
    mxDefaultPage(),
    maFilterData(),
    maShapeSelection(),
    mbExportShapeSelection(false),
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
    SolarMutexGuard aGuard;
    vcl::Window*     pFocusWindow = Application::GetFocusWindow();
    bool    bRet;

    if( pFocusWindow )
        pFocusWindow->EnterWait();

    if( mxDstDoc.is() )
        bRet = implImport( rDescriptor );
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
                            ObjectSequence aSelectedPageSequence;
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
            ObjectSet::const_iterator aElem = aMasterPageTargetSet.begin();
            for( sal_Int32 i = 0; aElem != aMasterPageTargetSet.end(); ++aElem, ++i)
            {
                uno::Reference< drawing::XDrawPage > xMasterPage( *aElem,  uno::UNO_QUERY );
                mMasterPageTargets[i] = xMasterPage;
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

bool SVGFilter::isStreamGZip(const uno::Reference<io::XInputStream>& xInput)
{
    uno::Reference<io::XSeekable> xSeek(xInput, uno::UNO_QUERY);
    if(xSeek.is())
        xSeek->seek(0);

    uno::Sequence<sal_Int8> aBuffer(2);
    const sal_uInt64 nBytes = xInput->readBytes(aBuffer, 2);
    if (nBytes == 2)
    {
        const sal_Int8* pBuffer = aBuffer.getConstArray();
        if (pBuffer[0] == 0x1F && static_cast<sal_uInt8>(pBuffer[1]) == 0x8B)
            return true;
    }
    return false;
}

bool SVGFilter::isStreamSvg(const uno::Reference<io::XInputStream>& xInput)
{
    uno::Reference<io::XSeekable> xSeek(xInput, uno::UNO_QUERY);
    if(xSeek.is())
        xSeek->seek(0);

    const sal_Int32 nLookAhead = 1024;
    uno::Sequence<sal_Int8> aBuffer(nLookAhead);
    const sal_uInt64 nBytes = xInput->readBytes(aBuffer, nLookAhead);
    const sal_Int8* pBuffer = aBuffer.getConstArray();

    sal_Int8 aMagic1[] = {'<', 's', 'v', 'g'};
    sal_Int32 aMagic1Size = sizeof(aMagic1) / sizeof(*aMagic1);

    if (std::search(pBuffer, pBuffer + nBytes, aMagic1, aMagic1 + aMagic1Size) != pBuffer + nBytes )
        return true;

    sal_Int8 aMagic2[] = {'D', 'O', 'C', 'T', 'Y', 'P', 'E', ' ', 's', 'v', 'g'};
    sal_Int32 aMagic2Size = sizeof(aMagic2) / sizeof(*aMagic2);

    if (std::search(pBuffer, pBuffer + nBytes, aMagic2, aMagic2 + aMagic2Size) != pBuffer + nBytes)
        return true;

    return false;
}

OUString SAL_CALL SVGFilter::detect(Sequence<PropertyValue>& rDescriptor)
{
    utl::MediaDescriptor aMediaDescriptor(rDescriptor);
    uno::Reference<io::XInputStream> xInput(aMediaDescriptor[utl::MediaDescriptor::PROP_INPUTSTREAM()], UNO_QUERY);

    if (!xInput.is())
        return OUString();

    try {
        if (isStreamGZip(xInput))
        {
            std::unique_ptr<SvStream> aStream(utl::UcbStreamHelper::CreateStream(xInput, true ));
            if(!aStream.get())
                return OUString();

            SvStream* pMemoryStream = new SvMemoryStream;
            uno::Reference<io::XSeekable> xSeek(xInput, uno::UNO_QUERY);
            if (!xSeek.is())
                return OUString();
            xSeek->seek(0);

            ZCodec aCodec;
            aCodec.BeginCompression(ZCODEC_DEFAULT_COMPRESSION, false, true);
            aCodec.Decompress(*aStream.get(), *pMemoryStream);
            aCodec.EndCompression();
            pMemoryStream->Seek(STREAM_SEEK_TO_BEGIN);
            uno::Reference<io::XInputStream> xDecompressedInput(new utl::OSeekableInputStreamWrapper(pMemoryStream, true));

            if (xDecompressedInput.is() && isStreamSvg(xDecompressedInput))
                return OUString(constFilterName);
        }
        else
        {
            if (isStreamSvg(xInput))
                return OUString(constFilterName);
        }
    } catch (css::io::IOException & e) {
        SAL_WARN("filter.svg", "caught IOException " + e.Message);
    }
    return OUString();
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
extern "C" SAL_DLLPUBLIC_EXPORT void* SAL_CALL svgfilter_component_getFactory(
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
