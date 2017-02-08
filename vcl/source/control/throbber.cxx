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

#include <vcl/throbber.hxx>
#include <vcl/svapp.hxx>

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/graphic/GraphicProvider.hpp>
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#include <com/sun/star/awt/ImageScaleMode.hpp>

#include <comphelper/namedvaluecollection.hxx>
#include <comphelper/processfactory.hxx>
#include <rtl/ustrbuf.hxx>
#include <tools/diagnose_ex.h>
#include <tools/urlobj.hxx>

#include <limits>

using ::com::sun::star::uno::Reference;
using ::com::sun::star::graphic::XGraphic;
using ::com::sun::star::graphic::XGraphicProvider;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::Exception;
namespace ImageScaleMode = ::com::sun::star::awt::ImageScaleMode;

Throbber::Throbber( vcl::Window* i_parentWindow, WinBits i_style )
    :ImageControl( i_parentWindow, i_style )
    ,mbRepeat( true )
    ,mnStepTime( 100 )
    ,mnCurStep( 0 )
    ,meImageSet( ImageSet::Auto )
{
    maWaitTimer.SetTimeout( mnStepTime );
    maWaitTimer.SetInvokeHandler( LINK( this, Throbber, TimeOutHdl ) );

    SetScaleMode( ImageScaleMode::NONE );
    initImages();
}

Throbber::~Throbber()
{
    disposeOnce();
}

void Throbber::dispose()
{
    maWaitTimer.Stop();
    ImageControl::dispose();
}

namespace
{
    ::std::vector< Image > lcl_loadImageSet( const Throbber::ImageSet i_imageSet )
    {
        ::std::vector< Image > aImages;

        const Reference< css::uno::XComponentContext > aContext( ::comphelper::getProcessComponentContext() );
        const Reference< XGraphicProvider > xGraphicProvider( css::graphic::GraphicProvider::create(aContext) );

        ::std::vector< OUString > aImageURLs( Throbber::getDefaultImageURLs( i_imageSet ) );
        aImages.reserve( aImageURLs.size() );

        ::comphelper::NamedValueCollection aMediaProperties;
        for (   ::std::vector< OUString >::const_iterator imageURL = aImageURLs.begin();
                imageURL != aImageURLs.end();
                ++imageURL
            )
        {
            Reference< XGraphic > xGraphic;
            aMediaProperties.put( "URL", *imageURL );
            xGraphic.set( xGraphicProvider->queryGraphic( aMediaProperties.getPropertyValues() ), UNO_QUERY );
            aImages.push_back( Image( xGraphic ) );
        }

        return aImages;
    }
}

void Throbber::Resize()
{
    ImageControl::Resize();

    if ( meImageSet == ImageSet::Auto )
        initImages();
}

void Throbber::initImages()
{
    try
    {
        ::std::vector< ::std::vector< Image > > aImageSets;
        if ( meImageSet == ImageSet::Auto )
        {
            aImageSets.push_back( lcl_loadImageSet( ImageSet::N16px ) );
            aImageSets.push_back( lcl_loadImageSet( ImageSet::N32px ) );
            aImageSets.push_back( lcl_loadImageSet( ImageSet::N64px ) );
        }
        else
        {
            aImageSets.push_back( lcl_loadImageSet( meImageSet ) );
        }

        // find the best matching image set (size-wise)
        const ::Size aWindowSizePixel = GetSizePixel();
        size_t nPreferredSet = 0;
        if ( aImageSets.size() > 1 )
        {
            long nMinimalDistance = ::std::numeric_limits< long >::max();
            for (   ::std::vector< ::std::vector< Image > >::const_iterator check = aImageSets.begin();
                    check != aImageSets.end();
                    ++check
                )
            {
                if ( check->empty() )
                {
                    SAL_WARN( "vcl.control", "Throbber::initImages: illegal image!" );
                    continue;
                }

                const Size aImageSize = (*check)[0].GetSizePixel();

                if  (   ( aImageSize.Width() > aWindowSizePixel.Width() )
                    ||  ( aImageSize.Height() > aWindowSizePixel.Height() )
                    )
                    // do not use an image set which doesn't fit into the window
                    continue;

                const sal_Int64 distance =
                        ( aWindowSizePixel.Width() - aImageSize.Width() ) * ( aWindowSizePixel.Width() - aImageSize.Width() )
                    +   ( aWindowSizePixel.Height() - aImageSize.Height() ) * ( aWindowSizePixel.Height() - aImageSize.Height() );
                if ( distance < nMinimalDistance )
                {
                    nMinimalDistance = distance;
                    nPreferredSet = check - aImageSets.begin();
                }
            }
        }

        if ( nPreferredSet < aImageSets.size() )
            setImageList( aImageSets[nPreferredSet] );
    }
    catch( const Exception& )
    {
    }
}

void Throbber::start()
{
    maWaitTimer.Start();
}

void Throbber::stop()
{
    maWaitTimer.Stop();
}

bool Throbber::isRunning() const
{
    return maWaitTimer.IsActive();
}

void Throbber::setImageList( ::std::vector< Image > const& i_images )
{
    SAL_WARN_IF( i_images.size()>=SAL_MAX_INT32, "vcl.control", "Throbber::setImageList: too many images!" );

    maImageList = i_images;

    const Image aInitialImage( maImageList.size() ? maImageList[ 0 ] : Image() );
    SetImage( aInitialImage );
}

::std::vector< OUString > Throbber::getDefaultImageURLs( const ImageSet i_imageSet )
{
    ::std::vector< OUString > aImageURLs;

    sal_Char const* const pResolutions[] = { "16", "32", "64" };
    size_t const nImageCounts[] = { 6, 12, 12 };

    size_t index = 0;
    switch ( i_imageSet )
    {
    case ImageSet::N16px:  index = 0;  break;
    case ImageSet::N32px:  index = 1;  break;
    case ImageSet::N64px:  index = 2;  break;
    case ImageSet::Auto:
        OSL_ENSURE( false, "Throbber::getDefaultImageURLs: illegal image set!" );
        return aImageURLs;
    }

    aImageURLs.reserve( nImageCounts[index] );
    for ( size_t i=0; i<nImageCounts[index]; ++i )
    {
        OUStringBuffer aURL;
        aURL.append( "private:graphicrepository/vcl/res/spinner-" );
        aURL.appendAscii( pResolutions[index] );
        aURL.append( "-" );
        if ( i < 9 )
            aURL.append( "0" );
        aURL.append     ( sal_Int32( i + 1 ) );
        aURL.append( ".png" );

        aImageURLs.push_back( aURL.makeStringAndClear() );
    }

    return aImageURLs;
}

IMPL_LINK_NOARG(Throbber, TimeOutHdl, Timer *, void)
{
    SolarMutexGuard aGuard;
    if ( maImageList.empty() )
        return;

    if ( mnCurStep < static_cast<sal_Int32>(maImageList.size()-1) )
        ++mnCurStep;
    else
    {
        if ( mbRepeat )
        {
            // start over
            mnCurStep = 0;
        }
        else
        {
            stop();
        }
    }

    SetImage( maImageList[ mnCurStep ] );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
