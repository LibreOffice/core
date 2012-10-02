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


#include "toolkit/awt/animatedimagespeer.hxx"
#include "toolkit/helper/property.hxx"

#include <com/sun/star/awt/XAnimatedImages.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/graphic/GraphicProvider.hpp>
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/awt/ImageScaleMode.hpp>

#include <comphelper/componentcontext.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <comphelper/processfactory.hxx>
#include <rtl/ustrbuf.hxx>
#include <tools/diagnose_ex.h>
#include <tools/urlobj.hxx>
#include <vcl/throbber.hxx>
#include <vcl/svapp.hxx>

#include <limits>

//......................................................................................................................
namespace toolkit
{
//......................................................................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::uno::XComponentContext;
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::UNO_SET_THROW;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::makeAny;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::Type;
    using ::com::sun::star::lang::EventObject;
    using ::com::sun::star::container::ContainerEvent;
    using ::com::sun::star::awt::XAnimatedImages;
    using ::com::sun::star::awt::Size;
    using ::com::sun::star::lang::XMultiServiceFactory;
    using ::com::sun::star::graphic::XGraphicProvider;
    using ::com::sun::star::beans::XPropertySet;
    using ::com::sun::star::graphic::XGraphic;
    /** === end UNO using === **/
    namespace ImageScaleMode = ::com::sun::star::awt::ImageScaleMode;

    //==================================================================================================================
    //= AnimatedImagesPeer_Data
    //==================================================================================================================
    struct CachedImage
    {
        ::rtl::OUString                 sImageURL;
        mutable Reference< XGraphic >   xGraphic;

        CachedImage()
            :sImageURL()
            ,xGraphic()
        {
        }

        CachedImage( ::rtl::OUString const& i_imageURL )
            :sImageURL( i_imageURL )
            ,xGraphic()
        {
        }
    };

    struct AnimatedImagesPeer_Data
    {
        AnimatedImagesPeer&                             rAntiImpl;
        ::std::vector< ::std::vector< CachedImage > >   aCachedImageSets;

        AnimatedImagesPeer_Data( AnimatedImagesPeer& i_antiImpl )
            :rAntiImpl( i_antiImpl )
            ,aCachedImageSets()
        {
        }
    };

    //==================================================================================================================
    //= helper
    //==================================================================================================================
    namespace
    {
        //--------------------------------------------------------------------------------------------------------------
        ::rtl::OUString lcl_getHighContrastURL( ::rtl::OUString const& i_imageURL )
        {
            INetURLObject aURL( i_imageURL );
            if ( aURL.GetProtocol() != INET_PROT_PRIV_SOFFICE )
            {
                OSL_VERIFY( aURL.insertName( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "hicontrast" ) ), false, 0 ) );
                return aURL.GetMainURL( INetURLObject::NO_DECODE );
            }
            // the private: scheme is not considered to be hierarchical by INetURLObject, so manually insert the
            // segment
            const sal_Int32 separatorPos = i_imageURL.indexOf( '/' );
            ENSURE_OR_RETURN( separatorPos != -1, "lcl_getHighContrastURL: unsipported URL scheme - cannot automatically determine HC version!", i_imageURL );

            ::rtl::OUStringBuffer composer;
            composer.append( i_imageURL.copy( 0, separatorPos ) );
            composer.appendAscii( "/hicontrast" );
            composer.append( i_imageURL.copy( separatorPos ) );
            return composer.makeStringAndClear();
        }

        //--------------------------------------------------------------------------------------------------------------
        bool lcl_ensureImage_throw( Reference< XGraphicProvider > const& i_graphicProvider, const bool i_isHighContrast, const CachedImage& i_cachedImage )
        {
            if ( !i_cachedImage.xGraphic.is() )
            {
                ::comphelper::NamedValueCollection aMediaProperties;
                if ( i_isHighContrast )
                {
                    // try (to find) the high-contrast version of the graphic first
                    aMediaProperties.put( "URL", lcl_getHighContrastURL( i_cachedImage.sImageURL ) );
                    i_cachedImage.xGraphic.set( i_graphicProvider->queryGraphic( aMediaProperties.getPropertyValues() ), UNO_QUERY );
                }
                if ( !i_cachedImage.xGraphic.is() )
                {
                    aMediaProperties.put( "URL", i_cachedImage.sImageURL );
                    i_cachedImage.xGraphic.set( i_graphicProvider->queryGraphic( aMediaProperties.getPropertyValues() ), UNO_QUERY );
                }
            }
            return i_cachedImage.xGraphic.is();
        }

        //--------------------------------------------------------------------------------------------------------------
        Size lcl_getGraphicSizePixel( Reference< XGraphic > const& i_graphic )
        {
            Size aSizePixel;
            try
            {
                if ( i_graphic.is() )
                {
                    const Reference< XPropertySet > xGraphicProps( i_graphic, UNO_QUERY_THROW );
                    OSL_VERIFY( xGraphicProps->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "SizePixel" ) ) ) >>= aSizePixel );
                }
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
            return aSizePixel;
        }

        //--------------------------------------------------------------------------------------------------------------
        void lcl_init( Sequence< ::rtl::OUString > const& i_imageURLs, ::std::vector< CachedImage >& o_images )
        {
            o_images.resize(0);
            size_t count = size_t( i_imageURLs.getLength() );
            o_images.reserve( count );
            for ( size_t i = 0; i < count; ++i )
            {
                o_images.push_back( CachedImage( i_imageURLs[i] ) );
            }
        }

        //--------------------------------------------------------------------------------------------------------------
        void lcl_updateImageList_nothrow( AnimatedImagesPeer_Data& i_data )
        {
            Throbber* pThrobber = dynamic_cast< Throbber* >( i_data.rAntiImpl.GetWindow() );
            if ( pThrobber == NULL )
                return;

            try
            {
                // collect the image sizes of the different image sets
                const Reference< XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
                const Reference< XGraphicProvider > xGraphicProvider( com::sun::star::graphic::GraphicProvider::create(xContext) );

                const bool isHighContrast = pThrobber->GetSettings().GetStyleSettings().GetHighContrastMode();

                sal_Int32 nPreferredSet = -1;
                const size_t nImageSetCount = i_data.aCachedImageSets.size();
                if ( nImageSetCount < 2 )
                {
                    nPreferredSet = sal_Int32( nImageSetCount ) - 1;
                }
                else
                {
                    ::std::vector< Size > aImageSizes( nImageSetCount );
                    for ( sal_Int32 nImageSet = 0; size_t( nImageSet ) < nImageSetCount; ++nImageSet )
                    {
                        ::std::vector< CachedImage > const& rImageSet( i_data.aCachedImageSets[ nImageSet ] );
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
                    long nMinimalDistance = ::std::numeric_limits< long >::max();
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
                Sequence< Reference< XGraphic > > aImages;
                if ( ( nPreferredSet >= 0 ) && ( size_t( nPreferredSet ) < nImageSetCount ) )
                {
                    // => set the images
                    ::std::vector< CachedImage > const& rImageSet( i_data.aCachedImageSets[ nPreferredSet ] );
                    aImages.realloc( rImageSet.size() );
                    sal_Int32 imageIndex = 0;
                    for (   ::std::vector< CachedImage >::const_iterator cachedImage = rImageSet.begin();
                            cachedImage != rImageSet.end();
                            ++cachedImage, ++imageIndex
                        )
                    {
                        lcl_ensureImage_throw( xGraphicProvider, isHighContrast, *cachedImage );
                        aImages[ imageIndex ] = cachedImage->xGraphic;
                    }
                }
                pThrobber->setImageList( aImages );
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
        }

        //--------------------------------------------------------------------------------------------------------------
        void lcl_updateImageList_nothrow( AnimatedImagesPeer_Data& i_data, const Reference< XAnimatedImages >& i_images )
        {
            try
            {
                const sal_Int32 nImageSetCount = i_images->getImageSetCount();
                i_data.aCachedImageSets.resize(0);
                for ( sal_Int32 set = 0;  set < nImageSetCount; ++set )
                {
                    const Sequence< ::rtl::OUString > aImageURLs( i_images->getImageSet( set ) );
                    ::std::vector< CachedImage > aImages;
                    lcl_init( aImageURLs, aImages );
                    i_data.aCachedImageSets.push_back( aImages );
                }

                lcl_updateImageList_nothrow( i_data );
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
        }
    }

    //==================================================================================================================
    //= AnimatedImagesPeer
    //==================================================================================================================
    //------------------------------------------------------------------------------------------------------------------
    AnimatedImagesPeer::AnimatedImagesPeer()
        :AnimatedImagesPeer_Base()
        ,m_pData( new AnimatedImagesPeer_Data( *this ) )
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    AnimatedImagesPeer::~AnimatedImagesPeer()
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL AnimatedImagesPeer::startAnimation(  ) throw (RuntimeException)
    {
        SolarMutexGuard aGuard;
        Throbber* pThrobber( dynamic_cast< Throbber* >( GetWindow() ) );
        if ( pThrobber != NULL)
            pThrobber->start();
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL AnimatedImagesPeer::stopAnimation(  ) throw (RuntimeException)
    {
        SolarMutexGuard aGuard;
        Throbber* pThrobber( dynamic_cast< Throbber* >( GetWindow() ) );
        if ( pThrobber != NULL)
            pThrobber->stop();
    }

    //------------------------------------------------------------------------------------------------------------------
    ::sal_Bool SAL_CALL AnimatedImagesPeer::isAnimationRunning(  ) throw (RuntimeException)
    {
        SolarMutexGuard aGuard;
        Throbber* pThrobber( dynamic_cast< Throbber* >( GetWindow() ) );
        if ( pThrobber != NULL)
            return pThrobber->isRunning();
        return sal_False;
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL AnimatedImagesPeer::setProperty( const ::rtl::OUString& i_propertyName, const Any& i_value ) throw(RuntimeException)
    {
        SolarMutexGuard aGuard;

        Throbber* pThrobber( dynamic_cast< Throbber* >( GetWindow() ) );
        if ( pThrobber == NULL )
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
                sal_Bool bRepeat( sal_True );
                if ( i_value >>= bRepeat )
                    pThrobber->setRepeat( bRepeat );
                break;
            }

            case BASEPROPERTY_IMAGE_SCALE_MODE:
            {
                sal_Int16 nScaleMode( ImageScaleMode::Anisotropic );
                ImageControl* pImageControl = dynamic_cast< ImageControl* >( GetWindow() );
                if ( pImageControl && ( i_value >>= nScaleMode ) )
                {
                    pImageControl->SetScaleMode( nScaleMode );
                }
            }
            break;

            default:
                AnimatedImagesPeer_Base::setProperty( i_propertyName, i_value );
                break;
        }
    }

    //------------------------------------------------------------------------------------------------------------------
    Any SAL_CALL AnimatedImagesPeer::getProperty( const ::rtl::OUString& i_propertyName ) throw(RuntimeException)
    {
        SolarMutexGuard aGuard;

        Any aReturn;

        Throbber* pThrobber( dynamic_cast< Throbber* >( GetWindow() ) );
        if ( pThrobber == NULL )
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
                ImageControl const* pImageControl = dynamic_cast< ImageControl* >( GetWindow() );
                aReturn <<= ( pImageControl ? pImageControl->GetScaleMode() : ImageScaleMode::Anisotropic );
            }
            break;

        default:
            aReturn = AnimatedImagesPeer_Base::getProperty( i_propertyName );
            break;
        }

        return aReturn;
    }

    //------------------------------------------------------------------------------------------------------------------
    void AnimatedImagesPeer::ProcessWindowEvent( const VclWindowEvent& i_windowEvent )
    {
        switch ( i_windowEvent.GetId() )
        {
        case VCLEVENT_WINDOW_RESIZE:
            lcl_updateImageList_nothrow( *m_pData );
            break;
        }

        AnimatedImagesPeer_Base::ProcessWindowEvent( i_windowEvent );
    }

    //------------------------------------------------------------------------------------------------------------------
    void AnimatedImagesPeer::impl_updateImages_nolck( const Reference< XInterface >& i_animatedImages )
    {
        SolarMutexGuard aGuard;

        lcl_updateImageList_nothrow( *m_pData, Reference< XAnimatedImages >( i_animatedImages, UNO_QUERY_THROW ) );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL AnimatedImagesPeer::elementInserted( const ContainerEvent& i_event ) throw (RuntimeException)
    {
        SolarMutexGuard aGuard;
        Reference< XAnimatedImages > xAnimatedImages( i_event.Source, UNO_QUERY_THROW );

        sal_Int32 nPosition(0);
        OSL_VERIFY( i_event.Accessor >>= nPosition );
        size_t position = size_t( nPosition );
        if ( position > m_pData->aCachedImageSets.size() )
        {
            OSL_ENSURE( false, "AnimatedImagesPeer::elementInserted: illegal accessor/index!" );
            lcl_updateImageList_nothrow( *m_pData, xAnimatedImages );
        }

        Sequence< ::rtl::OUString > aImageURLs;
        OSL_VERIFY( i_event.Element >>= aImageURLs );
        ::std::vector< CachedImage > aImages;
        lcl_init( aImageURLs, aImages );
        m_pData->aCachedImageSets.insert( m_pData->aCachedImageSets.begin() + position, aImages );
        lcl_updateImageList_nothrow( *m_pData );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL AnimatedImagesPeer::elementRemoved( const ContainerEvent& i_event ) throw (RuntimeException)
    {
        SolarMutexGuard aGuard;
        Reference< XAnimatedImages > xAnimatedImages( i_event.Source, UNO_QUERY_THROW );

        sal_Int32 nPosition(0);
        OSL_VERIFY( i_event.Accessor >>= nPosition );
        size_t position = size_t( nPosition );
        if ( position >= m_pData->aCachedImageSets.size() )
        {
            OSL_ENSURE( false, "AnimatedImagesPeer::elementRemoved: illegal accessor/index!" );
            lcl_updateImageList_nothrow( *m_pData, xAnimatedImages );
        }

        m_pData->aCachedImageSets.erase( m_pData->aCachedImageSets.begin() + position );
        lcl_updateImageList_nothrow( *m_pData );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL AnimatedImagesPeer::elementReplaced( const ContainerEvent& i_event ) throw (RuntimeException)
    {
        SolarMutexGuard aGuard;
        Reference< XAnimatedImages > xAnimatedImages( i_event.Source, UNO_QUERY_THROW );

        sal_Int32 nPosition(0);
        OSL_VERIFY( i_event.Accessor >>= nPosition );
        size_t position = size_t( nPosition );
        if ( position >= m_pData->aCachedImageSets.size() )
        {
            OSL_ENSURE( false, "AnimatedImagesPeer::elementReplaced: illegal accessor/index!" );
            lcl_updateImageList_nothrow( *m_pData, xAnimatedImages );
        }

        Sequence< ::rtl::OUString > aImageURLs;
        OSL_VERIFY( i_event.Element >>= aImageURLs );
        ::std::vector< CachedImage > aImages;
        lcl_init( aImageURLs, aImages );
        m_pData->aCachedImageSets[ position ] = aImages;
        lcl_updateImageList_nothrow( *m_pData );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL AnimatedImagesPeer::disposing( const EventObject& i_event ) throw (RuntimeException)
    {
        VCLXWindow::disposing( i_event );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL AnimatedImagesPeer::modified( const EventObject& i_event ) throw (RuntimeException)
    {
        impl_updateImages_nolck( i_event.Source );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL AnimatedImagesPeer::dispose(  ) throw(RuntimeException)
    {
        AnimatedImagesPeer_Base::dispose();
        SolarMutexGuard aGuard;
        m_pData->aCachedImageSets.resize(0);
    }

//......................................................................................................................
} // namespace toolkit
//......................................................................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
