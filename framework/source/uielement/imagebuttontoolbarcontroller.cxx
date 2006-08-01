/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: imagebuttontoolbarcontroller.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2006-08-01 09:38:55 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef __FRAMEWORK_UIELEMENT_IMAGEBUTTONTOOLBARCONTROLLER_HXX
#include "uielement/imagebuttontoolbarcontroller.hxx"
#endif

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_CLASSES_ADDONSOPTIONS_HXX_
#include <classes/addonsoptions.hxx>
#endif
#ifndef __FRAMEWORK_TOOLBAR_HXX_
#include "uielement/toolbar.hxx"
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_UTIL_XURLTRANSFORMER_HPP_
#include <com/sun/star/util/XURLTransformer.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDER_HPP_
#include <com/sun/star/frame/XDispatchProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XCONTROLNOTIFICATIONLISTENER_HPP_
#include <com/sun/star/frame/XControlNotificationListener.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XMACROEXPANDER_HPP_
#include "com/sun/star/util/XMacroExpander.hpp"
#endif
#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include "com/sun/star/uno/XComponentContext.hpp"
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include "com/sun/star/beans/XPropertySet.hpp"
#endif

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#include <rtl/uri.hxx>
#include <vos/mutex.hxx>
#include <comphelper/processfactory.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <tools/urlobj.hxx>
#include <vcl/svapp.hxx>
#include <vcl/mnemonic.hxx>
#include <vcl/window.hxx>
#include <vcl/graph.hxx>
#include <vcl/bitmap.hxx>
#include <svtools/filter.hxx>
#include <svtools/miscopt.hxx>

using namespace ::rtl;
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
        vos::OGuard aSolarMutexGuard( Application::GetSolarMutex() );

        if ( !xMacroExpander.is() )
        {
            uno::Reference< uno::XComponentContext > xContext;
            uno::Reference< beans::XPropertySet > xProps( ::comphelper::getProcessServiceFactory(), UNO_QUERY );
            xProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DefaultContext" ))) >>= xContext;
            if ( xContext.is() )
            {
                m_xMacroExpander =  Reference< com::sun::star::util::XMacroExpander >( xContext->getValueByName(
                                        OUString( RTL_CONSTASCII_USTRINGPARAM( "/singletons/com.sun.star.util.theMacroExpander"))),
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
        aMacro = Uri::decode( aMacro, rtl_UriDecodeWithCharset, RTL_TEXTENCODING_UTF8 );
        // expand macro string
        aURL = xMacroExpander->expandMacros( aMacro );
    }
}

// ------------------------------------------------------------------

ImageButtonToolbarController::ImageButtonToolbarController(
    const Reference< XMultiServiceFactory >& rServiceManager,
    const Reference< XFrame >&               rFrame,
    ToolBar*                                 pToolbar,
    USHORT                                   nID,
    const OUString&                          aCommand ) :
    ComplexToolbarController( rServiceManager, rFrame, pToolbar, nID, aCommand )
{
    sal_Bool bBigImages( SvtMiscOptions().AreCurrentSymbolsLarge() );
    sal_Bool bHiContrast( pToolbar->GetDisplayBackground().GetColor().IsDark() );

    Image aImage = AddonsOptions().GetImageFromURL( aCommand, bBigImages, bHiContrast, sal_True );

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
    vos::OGuard aSolarMutexGuard( Application::GetSolarMutex() );
    ComplexToolbarController::dispose();
}

// ------------------------------------------------------------------

void SAL_CALL ImageButtonToolbarController::execute( sal_Int16 KeyModifier )
throw ( RuntimeException )
{
    ComplexToolbarController::execute( KeyModifier );
}

// ------------------------------------------------------------------

void ImageButtonToolbarController::executeControlCommand( const ::com::sun::star::frame::ControlCommand& rControlCommand )
{
    vos::OGuard aSolarMutexGuard( Application::GetSolarMutex() );
    if ( rControlCommand.Command.equalsAsciiL( "SetImage", 7 ))
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

sal_Bool ImageButtonToolbarController::ReadImageFromURL( sal_Bool bBigImage, const OUString& aImageURL, Image& aImage )
{
    SvStream* pStream = utl::UcbStreamHelper::CreateStream( aImageURL, STREAM_STD_READ );
    if ( pStream && ( pStream->GetErrorCode() == 0 ))
    {
        // Use graphic class to also support more graphic formats (bmp,png,...)
        Graphic aGraphic;

        GraphicFilter* pGF = GraphicFilter::GetGraphicFilter();
        pGF->ImportGraphic( aGraphic, String(), *pStream, GRFILTER_FORMAT_DONTKNOW );

        BitmapEx aBitmapEx = aGraphic.GetBitmapEx();

        const ::Size aSize = bBigImage ? aImageSizeBig : aImageSizeSmall; // Sizes used for toolbar images

        ::Size aBmpSize = aBitmapEx.GetSizePixel();
        if ( aBmpSize.Width() > 0 && aBmpSize.Height() > 0 )
        {
            ::Size aNoScaleSize( aBmpSize.Width(), aSize.Height() );
            if ( aBmpSize != aNoScaleSize )
                aBitmapEx.Scale( aNoScaleSize, BMP_SCALE_INTERPOLATE );
            aImage = Image( aBitmapEx );
            return sal_True;
        }
    }

    delete pStream;
    return sal_False;
}

} // namespace
