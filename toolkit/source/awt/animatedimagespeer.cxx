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


#include <awt/animatedimagespeer.hxx>
#include <helper/property.hxx>

#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/graphic/GraphicProvider.hpp>
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/awt/ImageScaleMode.hpp>

#include <comphelper/namedvaluecollection.hxx>
#include <comphelper/processfactory.hxx>
#include <o3tl/safeint.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <tools/urlobj.hxx>
#include <vcl/toolkit/throbber.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>

#include <limits>
#include <string_view>

namespace toolkit
{


    using ::com::sun::star::uno::XComponentContext;
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::lang::EventObject;
    using ::com::sun::star::container::ContainerEvent;
    using ::com::sun::star::awt::XAnimatedImages;
    using ::com::sun::star::awt::Size;
    using ::com::sun::star::graphic::XGraphicProvider;
    using ::com::sun::star::beans::XPropertySet;
    using ::com::sun::star::graphic::XGraphic;

    namespace ImageScaleMode = ::com::sun::star::awt::ImageScaleMode;

    //= helper

    namespace
    {

        OUString lcl_getHighContrastURL( OUString const& i_imageURL )
        {
            INetURLObject aURL( i_imageURL );
            if ( aURL.GetProtocol() != INetProtocol::PrivSoffice )
            {
                OSL_VERIFY( aURL.insertName( u"sifr", false, 0 ) );
                return aURL.GetMainURL( INetURLObject::DecodeMechanism::NONE );
            }
            // the private: scheme is not considered to be hierarchical by INetURLObject, so manually insert the
            // segment
            const sal_Int32 separatorPos = i_imageURL.indexOf( '/' );
            ENSURE_OR_RETURN( separatorPos != -1, "lcl_getHighContrastURL: unsupported URL scheme - cannot automatically determine HC version!", i_imageURL );

            OUString composer = OUString::Concat(i_imageURL.subView(0, separatorPos)) + "/sifr" +
                i_imageURL.subView(separatorPos);
            return composer;
        }


        bool lcl_ensureImage_throw( Reference< XGraphicProvider > const& i_graphicProvider, const bool i_isHighContrast, const AnimatedImagesPeer::CachedImage& i_cachedImage )
        {
            if ( !i_cachedImage.xGraphic.is() )
            {
                ::comphelper::NamedValueCollection aMediaProperties;
                if ( i_isHighContrast )
                {
                    // try (to find) the high-contrast version of the graphic first
                    aMediaProperties.put( u"URL"_ustr, lcl_getHighContrastURL( i_cachedImage.sImageURL ) );
                    i_cachedImage.xGraphic = i_graphicProvider->queryGraphic( aMediaProperties.getPropertyValues() );
                }
                if ( !i_cachedImage.xGraphic.is() )
                {
                    aMediaProperties.put( u"URL"_ustr, i_cachedImage.sImageURL );
                    i_cachedImage.xGraphic = i_graphicProvider->queryGraphic( aMediaProperties.getPropertyValues() );
                }
            }
            return i_cachedImage.xGraphic.is();
        }


        Size lcl_getGraphicSizePixel( Reference< XGraphic > const& i_graphic )
        {
            Size aSizePixel;
            try
            {
                if ( i_graphic.is() )
                {
                    const Reference< XPropertySet > xGraphicProps( i_graphic, UNO_QUERY_THROW );
                    OSL_VERIFY( xGraphicProps->getPropertyValue(u"SizePixel"_ustr) >>= aSizePixel );
                }
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION("toolkit");
            }
            return aSizePixel;
        }


        void lcl_init( Sequence< OUString > const& i_imageURLs, ::std::vector< AnimatedImagesPeer::CachedImage >& o_images )
        {
            o_images.resize(0);
            size_t count = size_t( i_imageURLs.getLength() );
            o_images.reserve( count );
            for ( const auto& rImageURL : i_imageURLs )
            {
                o_images.emplace_back( AnimatedImagesPeer::CachedImage{ rImageURL, nullptr } );
            }
        }


    }


    //= AnimatedImagesPeer


    AnimatedImagesPeer::AnimatedImagesPeer()
    {
    }


    AnimatedImagesPeer::~AnimatedImagesPeer()
    {
    }


    void SAL_CALL AnimatedImagesPeer::startAnimation()
    {
        SolarMutexGuard aGuard;
        VclPtr<Throbber> pThrobber = GetAsDynamic<Throbber>();
        if (pThrobber)
            pThrobber->start();
    }

    void SAL_CALL AnimatedImagesPeer::stopAnimation()
    {
        SolarMutexGuard aGuard;
        VclPtr<Throbber> pThrobber = GetAsDynamic<Throbber>();
        if (pThrobber)
            pThrobber->stop();
    }

    sal_Bool SAL_CALL AnimatedImagesPeer::isAnimationRunning()
    {
        SolarMutexGuard aGuard;
        VclPtr<Throbber> pThrobber = GetAsDynamic<Throbber>();
        if (pThrobber)
            return pThrobber->isRunning();
        return false;
    }

    void SAL_CALL AnimatedImagesPeer::setProperty( const OUString& i_propertyName, const Any& i_value )
    {
        SolarMutexGuard aGuard;

        VclPtr<Throbber> pThrobber = GetAsDynamic<Throbber>();
        if ( !pThrobber )
        {
            VCLXWindow::setProperty( i_propertyName, i_value );
            return;
        }

        const sal_uInt16 nPropertyId = GetPropertyId( i_propertyName );
        switch ( nPropertyId )
        {
            case BASEPROPERTY_STEP_TIME:
            {
                sal_Int32 nStepTime( 0 );
                if ( i_value >>= nStepTime )
                    pThrobber->setStepTime( nStepTime );
                break;
            }
            case BASEPROPERTY_AUTO_REPEAT:
            {
                bool bRepeat( true );
                if ( i_value >>= bRepeat )
                    pThrobber->setRepeat( bRepeat );
                break;
            }

            case BASEPROPERTY_IMAGE_SCALE_MODE:
            {
                sal_Int16 nScaleMode( ImageScaleMode::ANISOTROPIC );
                VclPtr<ImageControl> pImageControl = GetAsDynamic< ImageControl >();
                if ( pImageControl && ( i_value >>= nScaleMode ) )
                    pImageControl->SetScaleMode( nScaleMode );
            }
            break;

            default:
                AnimatedImagesPeer_Base::setProperty( i_propertyName, i_value );
                break;
        }
    }


    Any SAL_CALL AnimatedImagesPeer::getProperty( const OUString& i_propertyName )
    {
        SolarMutexGuard aGuard;

        Any aReturn;

        VclPtr<Throbber> pThrobber = GetAsDynamic<Throbber>();
        if ( !pThrobber )
            return VCLXWindow::getProperty( i_propertyName );

        const sal_uInt16 nPropertyId = GetPropertyId( i_propertyName );
        switch ( nPropertyId )
        {
        case BASEPROPERTY_STEP_TIME:
            aReturn <<= pThrobber->getStepTime();
            break;

        case BASEPROPERTY_AUTO_REPEAT:
            aReturn <<= pThrobber->getRepeat();
            break;

        case BASEPROPERTY_IMAGE_SCALE_MODE:
            {
                VclPtr<ImageControl> pImageControl = GetAsDynamic<ImageControl>();
                aReturn <<= ( pImageControl ? pImageControl->GetScaleMode() : ImageScaleMode::ANISOTROPIC );
            }
            break;

        default:
            aReturn = AnimatedImagesPeer_Base::getProperty( i_propertyName );
            break;
        }

        return aReturn;
    }


    void AnimatedImagesPeer::ProcessWindowEvent( const VclWindowEvent& i_windowEvent )
    {
        if ( i_windowEvent.GetId() == VclEventId::WindowResize )
        {
            updateImageList_nothrow();
        }

        AnimatedImagesPeer_Base::ProcessWindowEvent( i_windowEvent );
    }


    void AnimatedImagesPeer::impl_updateImages_nolck( const Reference< XInterface >& i_animatedImages )
    {
        SolarMutexGuard aGuard;

        updateImageList_nothrow( Reference< XAnimatedImages >( i_animatedImages, UNO_QUERY_THROW ) );
    }


    void SAL_CALL AnimatedImagesPeer::elementInserted( const ContainerEvent& i_event )
    {
        SolarMutexGuard aGuard;
        Reference< XAnimatedImages > xAnimatedImages( i_event.Source, UNO_QUERY_THROW );

        sal_Int32 nPosition(0);
        OSL_VERIFY( i_event.Accessor >>= nPosition );
        size_t position = size_t( nPosition );
        if ( position > maCachedImageSets.size() )
        {
            OSL_ENSURE( false, "AnimatedImagesPeer::elementInserted: illegal accessor/index!" );
            updateImageList_nothrow( xAnimatedImages );
        }

        Sequence< OUString > aImageURLs;
        OSL_VERIFY( i_event.Element >>= aImageURLs );
        ::std::vector< CachedImage > aImages;
        lcl_init( aImageURLs, aImages );
        maCachedImageSets.insert( maCachedImageSets.begin() + position, aImages );
        updateImageList_nothrow();
    }


    void SAL_CALL AnimatedImagesPeer::elementRemoved( const ContainerEvent& i_event )
    {
        SolarMutexGuard aGuard;
        Reference< XAnimatedImages > xAnimatedImages( i_event.Source, UNO_QUERY_THROW );

        sal_Int32 nPosition(0);
        OSL_VERIFY( i_event.Accessor >>= nPosition );
        size_t position = size_t( nPosition );
        if ( position >= maCachedImageSets.size() )
        {
            OSL_ENSURE( false, "AnimatedImagesPeer::elementRemoved: illegal accessor/index!" );
            updateImageList_nothrow( xAnimatedImages );
        }

        maCachedImageSets.erase( maCachedImageSets.begin() + position );
        updateImageList_nothrow();
    }


    void SAL_CALL AnimatedImagesPeer::elementReplaced( const ContainerEvent& i_event )
    {
        SolarMutexGuard aGuard;
        Reference< XAnimatedImages > xAnimatedImages( i_event.Source, UNO_QUERY_THROW );

        sal_Int32 nPosition(0);
        OSL_VERIFY( i_event.Accessor >>= nPosition );
        size_t position = size_t( nPosition );
        if ( position >= maCachedImageSets.size() )
        {
            OSL_ENSURE( false, "AnimatedImagesPeer::elementReplaced: illegal accessor/index!" );
            updateImageList_nothrow( xAnimatedImages );
        }

        Sequence< OUString > aImageURLs;
        OSL_VERIFY( i_event.Element >>= aImageURLs );
        ::std::vector< CachedImage > aImages;
        lcl_init( aImageURLs, aImages );
        maCachedImageSets[ position ] = aImages;
        updateImageList_nothrow();
    }


    void SAL_CALL AnimatedImagesPeer::disposing( const EventObject& i_event )
    {
        VCLXWindow::disposing( i_event );
    }


    void SAL_CALL AnimatedImagesPeer::modified( const EventObject& i_event )
    {
        impl_updateImages_nolck( i_event.Source );
    }


    void SAL_CALL AnimatedImagesPeer::dispose(  )
    {
        AnimatedImagesPeer_Base::dispose();
        SolarMutexGuard aGuard;
        maCachedImageSets.resize(0);
    }

        void AnimatedImagesPeer::updateImageList_nothrow()
        {
            VclPtr<Throbber> pThrobber = GetAsDynamic<Throbber>();
            if ( !pThrobber )
                return;

            try
            {
                // collect the image sizes of the different image sets
                const Reference< XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
                const Reference< XGraphicProvider > xGraphicProvider( css::graphic::GraphicProvider::create(xContext) );

                const bool isHighContrast = pThrobber->GetSettings().GetStyleSettings().GetHighContrastMode();

                sal_Int32 nPreferredSet = -1;
                const size_t nImageSetCount = maCachedImageSets.size();
                if ( nImageSetCount < 2 )
                {
                    nPreferredSet = sal_Int32( nImageSetCount ) - 1;
                }
                else
                {
                    ::std::vector< Size > aImageSizes( nImageSetCount );
                    for ( size_t nImageSet = 0; nImageSet < nImageSetCount; ++nImageSet )
                    {
                        ::std::vector< CachedImage > const& rImageSet( maCachedImageSets[ nImageSet ] );
                        if  (   ( rImageSet.empty() )
                            ||  ( !lcl_ensureImage_throw( xGraphicProvider, isHighContrast, rImageSet[0] ) )
                            )
                        {
                            aImageSizes[ nImageSet ] = Size( SAL_MAX_INT32, SAL_MAX_INT32 );
                        }
                        else
                        {
                            aImageSizes[ nImageSet ] = lcl_getGraphicSizePixel( rImageSet[0].xGraphic );
                        }
                    }

                    // find the set with the smallest difference between window size and image size
                    const ::Size aWindowSizePixel = pThrobber->GetSizePixel();
                    tools::Long nMinimalDistance = ::std::numeric_limits< tools::Long >::max();
                    for (   ::std::vector< Size >::const_iterator check = aImageSizes.begin();
                            check != aImageSizes.end();
                            ++check
                        )
                    {
                        if  (   ( check->Width > aWindowSizePixel.Width() )
                            ||  ( check->Height > aWindowSizePixel.Height() )
                            )
                            // do not use an image set which doesn't fit into the window
                            continue;

                        const sal_Int64 distance =
                                ( aWindowSizePixel.Width() - check->Width ) * ( aWindowSizePixel.Width() - check->Width )
                            +   ( aWindowSizePixel.Height() - check->Height ) * ( aWindowSizePixel.Height() - check->Height );
                        if ( distance < nMinimalDistance )
                        {
                            nMinimalDistance = distance;
                            nPreferredSet = check - aImageSizes.begin();
                        }
                    }
                }

                // found a set?
                std::vector< Image > aImages;
                if ( ( nPreferredSet >= 0 ) && ( o3tl::make_unsigned( nPreferredSet ) < nImageSetCount ) )
                {
                    // => set the images
                    ::std::vector< CachedImage > const& rImageSet( maCachedImageSets[ nPreferredSet ] );
                    aImages.resize( rImageSet.size() );
                    sal_Int32 imageIndex = 0;
                    for ( const auto& rCachedImage : rImageSet )
                    {
                        lcl_ensureImage_throw( xGraphicProvider, isHighContrast, rCachedImage );
                        aImages[ imageIndex++ ] = Image(rCachedImage.xGraphic);
                    }
                }
                pThrobber->setImageList( std::move(aImages) );
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION("toolkit");
            }
        }


        void AnimatedImagesPeer::updateImageList_nothrow( const Reference< XAnimatedImages >& i_images )
        {
            try
            {
                const sal_Int32 nImageSetCount = i_images->getImageSetCount();
                maCachedImageSets.resize(0);
                for ( sal_Int32 set = 0;  set < nImageSetCount; ++set )
                {
                    const Sequence< OUString > aImageURLs( i_images->getImageSet( set ) );
                    ::std::vector< CachedImage > aImages;
                    lcl_init( aImageURLs, aImages );
                    maCachedImageSets.push_back( aImages );
                }

                updateImageList_nothrow();
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION("toolkit");
            }
        }

} // namespace toolkit


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
