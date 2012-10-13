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

#include <comphelper/servicedecl.hxx>
#include <uno/environment.h>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XDrawView.hpp>
#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/frame/XController.hpp>
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

#include <osl/mutex.hxx>

#include "svgfilter.hxx"

using ::rtl::OUString;
using namespace ::com::sun::star;

// -------------
// - SVGFilter -
// -------------

SVGFilter::SVGFilter( const Reference< XComponentContext >& rxCtx ) :
    mxMSF( rxCtx->getServiceManager(),
           uno::UNO_QUERY_THROW ),
    mpSVGDoc( NULL ),
    mpSVGExport( NULL ),
    mpSVGFontExport( NULL ),
    mpSVGWriter( NULL ),
    mpDefaultSdrPage( NULL ),
    mpSdrModel( NULL ),
    mbPresentation( sal_False ),
    mbExportAll( sal_False ),
    mpObjects( NULL )

{
}

// -----------------------------------------------------------------------------

SVGFilter::~SVGFilter()
{
    DBG_ASSERT( mpSVGDoc == NULL, "mpSVGDoc not destroyed" );
    DBG_ASSERT( mpSVGExport == NULL, "mpSVGExport not destroyed" );
    DBG_ASSERT( mpSVGFontExport == NULL, "mpSVGFontExport not destroyed" );
    DBG_ASSERT( mpSVGWriter == NULL, "mpSVGWriter not destroyed" );
    DBG_ASSERT( mpObjects == NULL, "mpObjects not destroyed" );
}

// -----------------------------------------------------------------------------

sal_Bool SAL_CALL SVGFilter::filter( const Sequence< PropertyValue >& rDescriptor )
    throw (RuntimeException)
{
    SolarMutexGuard aGuard;
    Window*     pFocusWindow = Application::GetFocusWindow();
    sal_Bool    bRet;

    if( pFocusWindow )
        pFocusWindow->EnterWait();

    if( mxDstDoc.is() )
        bRet = implImport( rDescriptor );
    else if( mxSrcDoc.is() )
    {
        if( !mbExportAll && !mSelectedPages.hasElements() )
        {
            uno::Reference< frame::XDesktop >                            xDesktop(mxMSF->createInstance( "com.sun.star.frame.Desktop" ),
                                                                                  uno::UNO_QUERY_THROW);
            uno::Reference< frame::XFrame >                              xFrame(xDesktop->getCurrentFrame(),
                                                                                uno::UNO_QUERY_THROW);
            uno::Reference<frame::XController >                          xController(xFrame->getController(),
                                                                                     uno::UNO_QUERY_THROW);
            uno::Reference<drawing::XDrawView >                          xDrawView(xController,
                                                                                   uno::UNO_QUERY_THROW);
            uno::Reference<drawing::framework::XControllerManager>       xManager(xController,
                                                                                  uno::UNO_QUERY_THROW);
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
                            mSelectedPages.realloc( aSelectedPageSequence.getLength() );
                            for( sal_Int32 j=0; j<mSelectedPages.getLength(); ++j )
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

            if( !mSelectedPages.hasElements() )
            {
                // apparently failed to glean selection - fallback to current page
                mSelectedPages.realloc( 1 );
                mSelectedPages[0] = xDrawView->getCurrentPage();
            }
        }

        /*
         * Export all slides, or requested "PagePos"
         */
        if( !mSelectedPages.hasElements() )
        {
            sal_Int32            nLength = rDescriptor.getLength();
            const PropertyValue* pValue = rDescriptor.getConstArray();
            sal_Int32            nPageToExport = -1;

            for ( sal_Int32 i = 0 ; i < nLength; ++i)
            {
                if ( pValue[ i ].Name == "PagePos" )
                {
                    pValue[ i ].Value >>= nPageToExport;
                }
            }

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

                    mSelectedPages.realloc( nPageToExport != -1 ? 1 : nDPCount );
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

        /*
         *  We get all master page that are targeted by at least one draw page.
         *  The master page are put in an unordered set.
         */
        ObjectSet aMasterPageTargetSet;
        for( sal_Int32 i = 0; i < mSelectedPages.getLength(); ++i )
        {
            uno::Reference< drawing::XMasterPageTarget > xMasterPageTarget( mSelectedPages[i], uno::UNO_QUERY );
            if( xMasterPageTarget.is() )
            {
                aMasterPageTargetSet.insert( xMasterPageTarget->getMasterPage() );
            }
        }
        // Later we move them to a uno::Sequence so we can get them by index
        mMasterPageTargets.realloc( aMasterPageTargetSet.size() );
        ObjectSet::const_iterator aElem = aMasterPageTargetSet.begin();
        for( sal_Int32 i = 0; aElem != aMasterPageTargetSet.end(); ++aElem, ++i)
        {
            uno::Reference< drawing::XDrawPage > xMasterPage( *aElem,  uno::UNO_QUERY );
            mMasterPageTargets[i] = xMasterPage;
        }

        bRet = implExport( rDescriptor );
    }
    else
        bRet = sal_False;

    if( pFocusWindow )
        pFocusWindow->LeaveWait();

    return bRet;
}

// -----------------------------------------------------------------------------

void SAL_CALL SVGFilter::cancel( ) throw (RuntimeException)
{
}

// -----------------------------------------------------------------------------

void SAL_CALL SVGFilter::setSourceDocument( const Reference< XComponent >& xDoc )
    throw (IllegalArgumentException, RuntimeException)
{
    mxSrcDoc = xDoc;
}

// -----------------------------------------------------------------------------

void SAL_CALL SVGFilter::setTargetDocument( const Reference< XComponent >& xDoc )
    throw (::com::sun::star::lang::IllegalArgumentException, RuntimeException)
{
    mxDstDoc = xDoc;
}

// -----------------------------------------------------------------------------

rtl::OUString SAL_CALL SVGFilter::detect( Sequence< PropertyValue >& io_rDescriptor ) throw (RuntimeException)
{
    uno::Reference< io::XInputStream > xInput;

    const beans::PropertyValue* pAttribs = io_rDescriptor.getConstArray();
    const sal_Int32 nAttribs = io_rDescriptor.getLength();
    for( sal_Int32 i = 0; i < nAttribs; i++ )
    {
        if ( pAttribs[i].Name == "InputStream" )
            pAttribs[i].Value >>= xInput;
    }

    if( !xInput.is() )
        return rtl::OUString();

    uno::Reference< io::XSeekable > xSeek( xInput, uno::UNO_QUERY );
    if( xSeek.is() )
        xSeek->seek( 0 );

    // read the first 1024 bytes & check a few magic string
    // constants (heuristically)
    const sal_Int32 nLookAhead = 1024;
    uno::Sequence< sal_Int8 > aBuf( nLookAhead );
    const sal_uInt64 nBytes=xInput->readBytes(aBuf, nLookAhead);
    const sal_Int8* const pBuf=aBuf.getConstArray();

    sal_Int8 aMagic1[] = {'<', 's', 'v', 'g'};
    if( std::search(pBuf, pBuf+nBytes,
                    aMagic1, aMagic1+sizeof(aMagic1)/sizeof(*aMagic1)) != pBuf+nBytes )
        return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("svg_Scalable_Vector_Graphics") );

    sal_Int8 aMagic2[] = {'D', 'O', 'C', 'T', 'Y', 'P', 'E', ' ', 's', 'v', 'g'};
    if( std::search(pBuf, pBuf+nBytes,
                    aMagic2, aMagic2+sizeof(aMagic2)/sizeof(*aMagic2)) != pBuf+nBytes )
        return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("svg_Scalable_Vector_Graphics") );

    return rtl::OUString();
}

// -----------------------------------------------------------------------------

namespace sdecl = comphelper::service_decl;
 sdecl::class_<SVGFilter> serviceImpl;
 const sdecl::ServiceDecl svgFilter(
     serviceImpl,
     "com.sun.star.comp.Draw.SVGFilter",
     "com.sun.star.document.ImportFilter;"
     "com.sun.star.document.ExportFilter;"
     "com.sun.star.document.ExtendedTypeDetection" );

// The C shared lib entry points
extern "C" SAL_DLLPUBLIC_EXPORT void* SAL_CALL svgfilter_component_getFactory(
    sal_Char const* pImplName,
    ::com::sun::star::lang::XMultiServiceFactory* pServiceManager,
    ::com::sun::star::registry::XRegistryKey* pRegistryKey )
{
    return component_getFactoryHelper( pImplName, pServiceManager,
        pRegistryKey, svgFilter );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
