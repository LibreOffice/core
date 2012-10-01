/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
        if( !mbExportAll )
        {
            uno::Reference< frame::XDesktop > xDesktop( mxMSF->createInstance( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.frame.Desktop" )) ),
                                                        uno::UNO_QUERY);
            if( xDesktop.is() )
            {
                uno::Reference< frame::XFrame > xFrame( xDesktop->getCurrentFrame() );

                if( xFrame.is() )
                {
                    uno::Reference< frame::XController > xController( xFrame->getController() );

                    if( xController.is() )
                    {
                        /*
                         *  Get the selection from the Slide Sorter Center Pane
                         */
                        if( !mSelectedPages.hasElements() )
                        {
                            uno::Reference< beans::XPropertySet > xControllerPropertySet( xController, uno::UNO_QUERY );
                            uno::Reference< drawing::XDrawSubController > xSubController;
                            xControllerPropertySet->getPropertyValue(::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "SubController" ) ) )
                                    >>= xSubController;

                            if( xSubController.is() )
                            {
                                uno::Any aSelection = xSubController->getSelection();
                                if( aSelection.hasValue() )
                                {
                                    ObjectSequence aSelectedPageSequence;
                                    aSelection >>= aSelectedPageSequence;
                                    mSelectedPages.realloc( aSelectedPageSequence.getLength() );
                                    for( sal_Int32 i = 0; i < mSelectedPages.getLength(); ++i )
                                    {
                                        uno::Reference< drawing::XDrawPage > xDrawPage( aSelectedPageSequence[i],  uno::UNO_QUERY );
                                        mSelectedPages[i] = xDrawPage;
                                    }
                                }
                            }
                        }
                    }
                }
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

            uno::Reference< drawing::XMasterPagesSupplier >        xMasterPagesSupplier( mxSrcDoc, uno::UNO_QUERY );
            uno::Reference< drawing::XDrawPagesSupplier >        xDrawPagesSupplier( mxSrcDoc, uno::UNO_QUERY );

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
