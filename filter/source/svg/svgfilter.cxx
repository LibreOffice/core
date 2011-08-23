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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_filter.hxx"

#include <cstdio>

#include <comphelper/servicedecl.hxx>
#include <uno/environment.h>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XDrawView.hpp>
#include <com/sun/star/frame/XDesktop.hdl>
#include <com/sun/star/frame/XController.hdl>
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
    mbPresentation( sal_False )
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
    sal_Int16 	nCurrentPageNumber = -1;
    sal_Bool    bRet;

    if( pFocusWindow )
        pFocusWindow->EnterWait();

    if( mxDstDoc.is() )
        bRet = implImport( rDescriptor );
    else
    if( mxSrcDoc.is() )
    {
        uno::Reference< frame::XDesktop > xDesktop( mxMSF->createInstance( ::rtl::OUString::createFromAscii( "com.sun.star.frame.Desktop" ) ), 
                                                    uno::UNO_QUERY);
        if( xDesktop.is() )
        {
            uno::Reference< frame::XFrame > xFrame( xDesktop->getCurrentFrame() );
            
            if( xFrame.is() )
            {
                uno::Reference< frame::XController > xController( xFrame->getController() );
                
                if( xController.is() )
                {
                    uno::Reference< drawing::XDrawView > xDrawView( xController, uno::UNO_QUERY );
                    
                    if( xDrawView.is() )
                    {
                        uno::Reference< drawing::XDrawPage > xDrawPage( xDrawView->getCurrentPage() );
                        
                        if( xDrawPage.is() )
                        {
                            uno::Reference< beans::XPropertySet >( xDrawPage, uno::UNO_QUERY )->
                                getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Number" ) ) ) >>= nCurrentPageNumber;
                        }
                    }
                }
            }
        }
        
        Sequence< PropertyValue > aNewDescritor( rDescriptor );
        
        if( nCurrentPageNumber > 0 )
        {
            const sal_uInt32	nOldLength = rDescriptor.getLength();
            
            aNewDescritor.realloc( nOldLength + 1 );
            aNewDescritor[ nOldLength ].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PagePos" ) );
            aNewDescritor[ nOldLength ].Value <<= static_cast< sal_Int16 >( nCurrentPageNumber - 1 );
        }
        
        bRet = implExport( aNewDescritor );
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
    rtl::OUString aURL;

    const beans::PropertyValue* pAttribs = io_rDescriptor.getConstArray();
    const sal_Int32 nAttribs = io_rDescriptor.getLength();
    for( sal_Int32 i = 0; i < nAttribs; i++ )
    {
        if( pAttribs[i].Name.equalsAscii( "InputStream" ) )
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

class FilterConfigItem;
extern "C" SAL_DLLPUBLIC_EXPORT BOOL __LOADONCALLAPI GraphicImport(SvStream & rStream, Graphic & rGraphic, FilterConfigItem*, BOOL )
{
    BOOL bRet = FALSE;
    try
    {
        bRet = importSvg( rStream, rGraphic );
    }
    catch (const uno::Exception&) {
    }
    return bRet;
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
COMPHELPER_SERVICEDECL_EXPORTS1(svgFilter)

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
