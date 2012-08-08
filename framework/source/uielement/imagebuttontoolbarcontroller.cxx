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


#include "uielement/imagebuttontoolbarcontroller.hxx"

#include <framework/addonsoptions.hxx>
#include "uielement/toolbar.hxx"

#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/frame/XControlNotificationListener.hpp>
#include "com/sun/star/util/XMacroExpander.hpp"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/beans/XPropertySet.hpp"

#include <rtl/uri.hxx>
#include <osl/mutex.hxx>
#include <comphelper/processfactory.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <vcl/svapp.hxx>
#include <vcl/mnemonic.hxx>
#include <vcl/window.hxx>
#include <vcl/graph.hxx>
#include <vcl/bitmap.hxx>
#include <svtools/filter.hxx>
#include <svtools/miscopt.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::util;

#define EXPAND_PROTOCOL "vnd.sun.star.expand:"

const ::Size  aImageSizeSmall( 16, 16 );
const ::Size  aImageSizeBig( 26, 26 );

namespace framework
{

static uno::WeakReference< util::XMacroExpander > m_xMacroExpander;

// ------------------------------------------------------------------

uno::Reference< util::XMacroExpander > GetMacroExpander()
{
    uno::Reference< util::XMacroExpander > xMacroExpander( m_xMacroExpander );
    if ( !xMacroExpander.is() )
    {
        SolarMutexGuard aSolarMutexGuard;

        if ( !xMacroExpander.is() )
        {
            uno::Reference< uno::XComponentContext > xContext;
            uno::Reference< beans::XPropertySet > xProps( ::comphelper::getProcessServiceFactory(), UNO_QUERY );
            xProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DefaultContext" ))) >>= xContext;
            if ( xContext.is() )
            {
                m_xMacroExpander =  Reference< com::sun::star::util::XMacroExpander >( xContext->getValueByName(
                                        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "/singletons/com.sun.star.util.theMacroExpander"))),
                                        UNO_QUERY );
                xMacroExpander = m_xMacroExpander;
            }
        }
    }

    return xMacroExpander;
}

static void SubstituteVariables( ::rtl::OUString& aURL )
{
    if ( aURL.compareToAscii( RTL_CONSTASCII_STRINGPARAM( EXPAND_PROTOCOL )) == 0 )
    {
        uno::Reference< util::XMacroExpander > xMacroExpander = GetMacroExpander();

        // cut protocol
        rtl::OUString aMacro( aURL.copy( sizeof ( EXPAND_PROTOCOL ) -1 ) );
        // decode uric class chars
        aMacro = ::rtl::Uri::decode( aMacro, rtl_UriDecodeWithCharset, RTL_TEXTENCODING_UTF8 );
        // expand macro string
        aURL = xMacroExpander->expandMacros( aMacro );
    }
}

// ------------------------------------------------------------------

ImageButtonToolbarController::ImageButtonToolbarController(
    const Reference< XMultiServiceFactory >& rServiceManager,
    const Reference< XFrame >&               rFrame,
    ToolBox*                                 pToolbar,
    sal_uInt16                                   nID,
    const ::rtl::OUString&                          aCommand ) :
    ComplexToolbarController( rServiceManager, rFrame, pToolbar, nID, aCommand )
{
    sal_Bool bBigImages( SvtMiscOptions().AreCurrentSymbolsLarge() );

    Image aImage = AddonsOptions().GetImageFromURL( aCommand, bBigImages, sal_True );

    // Height will be controlled by scaling according to button height
    m_pToolbar->SetItemImage( m_nID, aImage );
}

// ------------------------------------------------------------------

ImageButtonToolbarController::~ImageButtonToolbarController()
{
}

// ------------------------------------------------------------------

void SAL_CALL ImageButtonToolbarController::dispose()
throw ( RuntimeException )
{
    SolarMutexGuard aSolarMutexGuard;
    ComplexToolbarController::dispose();
}

// ------------------------------------------------------------------

void ImageButtonToolbarController::executeControlCommand( const ::com::sun::star::frame::ControlCommand& rControlCommand )
{
    SolarMutexGuard aSolarMutexGuard;
    // i73486 to be downward compatible use old and "wrong" also!
    if (( rControlCommand.Command.equalsAsciiL( "SetImag", 7 )) ||
        ( rControlCommand.Command.equalsAsciiL( "SetImage", 8 )) )
    {
        for ( sal_Int32 i = 0; i < rControlCommand.Arguments.getLength(); i++ )
        {
            if ( rControlCommand.Arguments[i].Name.equalsAsciiL( "URL", 3 ))
            {
                rtl::OUString aURL;
                rControlCommand.Arguments[i].Value >>= aURL;

                SubstituteVariables( aURL );

                Image aImage;
                if ( ReadImageFromURL( SvtMiscOptions().AreCurrentSymbolsLarge(),
                                       aURL,
                                       aImage ))
                {
                    m_pToolbar->SetItemImage( m_nID, aImage );

                    // send notification
                    uno::Sequence< beans::NamedValue > aInfo( 1 );
                    aInfo[0].Name  = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "URL" ));
                    aInfo[0].Value <<= aURL;
                    addNotifyInfo( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ImageChanged" )),
                                getDispatchFromCommand( m_aCommandURL ),
                                aInfo );
                    break;
                }
            }
        }
    }
}

sal_Bool ImageButtonToolbarController::ReadImageFromURL( sal_Bool bBigImage, const ::rtl::OUString& aImageURL, Image& aImage )
{
    SvStream* pStream = utl::UcbStreamHelper::CreateStream( aImageURL, STREAM_STD_READ );
    if ( pStream && ( pStream->GetErrorCode() == 0 ))
    {
        // Use graphic class to also support more graphic formats (bmp,png,...)
        Graphic aGraphic;

        GraphicFilter& rGF = GraphicFilter::GetGraphicFilter();
        rGF.ImportGraphic( aGraphic, String(), *pStream, GRFILTER_FORMAT_DONTKNOW );

        BitmapEx aBitmapEx = aGraphic.GetBitmapEx();

        const ::Size aSize = bBigImage ? aImageSizeBig : aImageSizeSmall; // Sizes used for toolbar images

        ::Size aBmpSize = aBitmapEx.GetSizePixel();
        if ( aBmpSize.Width() > 0 && aBmpSize.Height() > 0 )
        {
            ::Size aNoScaleSize( aBmpSize.Width(), aSize.Height() );
            if ( aBmpSize != aNoScaleSize )
                aBitmapEx.Scale( aNoScaleSize, BMP_SCALE_BEST );
            aImage = Image( aBitmapEx );
            return sal_True;
        }
    }

    delete pStream;
    return sal_False;
}

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
