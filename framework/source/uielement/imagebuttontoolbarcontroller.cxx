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


#include "uielement/imagebuttontoolbarcontroller.hxx"

#include <framework/addonsoptions.hxx>

#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/frame/XControlNotificationListener.hpp>
#include "com/sun/star/util/theMacroExpander.hpp"
#include "com/sun/star/uno/XComponentContext.hpp"

#include <rtl/uri.hxx>
#include <osl/mutex.hxx>
#include <comphelper/processfactory.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <vcl/svapp.hxx>
#include <vcl/mnemonic.hxx>
#include <vcl/window.hxx>
#include <vcl/graph.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/graphicfilter.hxx>
#include <vcl/toolbox.hxx>
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
            uno::Reference< uno::XComponentContext > xContext(
                comphelper::getProcessComponentContext() );
            m_xMacroExpander =  util::theMacroExpander::get(xContext);
            xMacroExpander = m_xMacroExpander;
        }
    }

    return xMacroExpander;
}

static void SubstituteVariables( OUString& aURL )
{
    if ( aURL.startsWith( EXPAND_PROTOCOL ) )
    {
        uno::Reference< util::XMacroExpander > xMacroExpander = GetMacroExpander();

        // cut protocol
        OUString aMacro( aURL.copy( sizeof ( EXPAND_PROTOCOL ) -1 ) );
        // decode uric class chars
        aMacro = ::rtl::Uri::decode( aMacro, rtl_UriDecodeWithCharset, RTL_TEXTENCODING_UTF8 );
        // expand macro string
        aURL = xMacroExpander->expandMacros( aMacro );
    }
}

// ------------------------------------------------------------------

ImageButtonToolbarController::ImageButtonToolbarController(
    const Reference< XComponentContext >&    rxContext,
    const Reference< XFrame >&               rFrame,
    ToolBox*                                 pToolbar,
    sal_uInt16                                   nID,
    const OUString&                          aCommand ) :
    ComplexToolbarController( rxContext, rFrame, pToolbar, nID, aCommand )
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
                OUString aURL;
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
                    aInfo[0].Name  = OUString( "URL" );
                    aInfo[0].Value <<= aURL;
                    addNotifyInfo( OUString( "ImageChanged" ),
                                getDispatchFromCommand( m_aCommandURL ),
                                aInfo );
                    break;
                }
            }
        }
    }
}

sal_Bool ImageButtonToolbarController::ReadImageFromURL( sal_Bool bBigImage, const OUString& aImageURL, Image& aImage )
{
    SvStream* pStream = utl::UcbStreamHelper::CreateStream( aImageURL, STREAM_STD_READ );
    if ( pStream && ( pStream->GetErrorCode() == 0 ))
    {
        // Use graphic class to also support more graphic formats (bmp,png,...)
        Graphic aGraphic;

        GraphicFilter& rGF = GraphicFilter::GetGraphicFilter();
        rGF.ImportGraphic( aGraphic, OUString(), *pStream, GRFILTER_FORMAT_DONTKNOW );

        BitmapEx aBitmapEx = aGraphic.GetBitmapEx();

        const ::Size aSize = bBigImage ? aImageSizeBig : aImageSizeSmall; // Sizes used for toolbar images

        ::Size aBmpSize = aBitmapEx.GetSizePixel();
        if ( aBmpSize.Width() > 0 && aBmpSize.Height() > 0 )
        {
            ::Size aNoScaleSize( aBmpSize.Width(), aSize.Height() );
            if ( aBmpSize != aNoScaleSize )
                aBitmapEx.Scale( aNoScaleSize, BMP_SCALE_BESTQUALITY );
            aImage = Image( aBitmapEx );
            return sal_True;
        }
    }

    delete pStream;
    return sal_False;
}

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
