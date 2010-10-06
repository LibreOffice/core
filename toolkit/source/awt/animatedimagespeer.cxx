/*************************************************************************
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

#include "precompiled_toolkit.hxx"

#include "toolkit/awt/animatedimagespeer.hxx"
#include "toolkit/helper/property.hxx"
#include "toolkit/helper/throbberimpl.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/awt/XAnimatedImages.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/awt/ImageScaleMode.hpp>
/** === end UNO includes === **/

#include <comphelper/componentcontext.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <comphelper/processfactory.hxx>
#include <tools/diagnose_ex.h>
#include <vcl/imgctrl.hxx>

#include <stl/limits>

//......................................................................................................................
namespace toolkit
{
//......................................................................................................................

    /** === begin UNO using === **/
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
    struct AnimatedImagesPeer_Data
    {
        Throbber_Impl                                   aThrobber;
        ::std::vector< Sequence< ::rtl::OUString > >    aCachedImageSets;

        AnimatedImagesPeer_Data( VCLXWindow& i_window )
            :aThrobber( i_window )
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
        Reference< XGraphic > lcl_getGraphic_throw( const Reference< XGraphicProvider >& i_graphicProvider, const ::rtl::OUString& i_imageURL )
        {
            ::comphelper::NamedValueCollection aMediaProperties;
            aMediaProperties.put( "URL", i_imageURL );
            return Reference< XGraphic >( i_graphicProvider->queryGraphic( aMediaProperties.getPropertyValues() ), UNO_QUERY );
        }

        //--------------------------------------------------------------------------------------------------------------
        Size lcl_getGraphicSizePixel( const Reference< XGraphicProvider >& i_graphicProvider, const ::rtl::OUString& i_imageURL )
        {
            Size aSizePixel;
            try
            {
                ::comphelper::NamedValueCollection aMediaProperties;
                aMediaProperties.put( "URL", i_imageURL );
                const Reference< XPropertySet > xGraphicProps( lcl_getGraphic_throw( i_graphicProvider, i_imageURL ), UNO_QUERY_THROW );
                OSL_VERIFY( xGraphicProps->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "SizePixel" ) ) ) >>= aSizePixel );
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
            return aSizePixel;
        }

        //--------------------------------------------------------------------------------------------------------------
        void lcl_updateImageList_nothrow( AnimatedImagesPeer_Data& i_data )
        {
            const Window* pWindow = i_data.aThrobber.getWindow();
            if ( pWindow == NULL )
                return;

            try
            {
                // collect the image sizes of the different image sets
                const ::comphelper::ComponentContext aContext( ::comphelper::getProcessServiceFactory() );
                const Reference< XGraphicProvider > xGraphicProvider( aContext.createComponent( "com.sun.star.graphic.GraphicProvider" ), UNO_QUERY_THROW );

                const size_t nImageSetCount = i_data.aCachedImageSets.size();
                ::std::vector< Size > aImageSizes( nImageSetCount );
                for ( sal_Int32 nImageSet = 0; nImageSet < nImageSetCount; ++nImageSet )
                {
                    Sequence< ::rtl::OUString > const& rImageSet( i_data.aCachedImageSets[ nImageSet ] );
                    if ( rImageSet.getLength() )
                        aImageSizes[ nImageSet ] = lcl_getGraphicSizePixel( xGraphicProvider, rImageSet[0] );
                    else
                        aImageSizes[ nImageSet ] = Size( ::std::numeric_limits< long >::max(), ::std::numeric_limits< long >::max() );
                }

                // find the set with the smallest difference between window size and image size
                const ::Size aWindowSizePixel = pWindow->GetSizePixel();
                sal_Int32 nPreferredSet = -1;
                long nMinimalDistance = ::std::numeric_limits< long >::max();
                for (   ::std::vector< Size >::const_iterator check = aImageSizes.begin();
                        check != aImageSizes.end();
                        ++check
                    )
                {
                    const sal_Int64 distance =
                            ( aWindowSizePixel.Width() - check->Width ) * ( aWindowSizePixel.Width() - check->Width )
                        +   ( aWindowSizePixel.Height() - check->Height ) * ( aWindowSizePixel.Height() - check->Height );
                    if ( distance < nMinimalDistance )
                    {
                        nMinimalDistance = distance;
                        nPreferredSet = check - aImageSizes.begin();
                    }
                }

                // found a set?
                Sequence< Reference< XGraphic > > aImages;
                if ( ( nPreferredSet >= 0 ) && ( nPreferredSet < nImageSetCount ) )
                {
                    // => set the images
                    Sequence< ::rtl::OUString > const& rImageSet( i_data.aCachedImageSets[ nPreferredSet ] );
                    aImages.realloc( rImageSet.getLength() );
                    sal_Int32 imageIndex = 0;
                    for (   ::rtl::OUString const* pImageURL = rImageSet.getConstArray();
                            pImageURL != rImageSet.getConstArray() + rImageSet.getLength();
                            ++pImageURL, ++imageIndex
                        )
                    {
                        aImages[ imageIndex ] = lcl_getGraphic_throw( xGraphicProvider, *pImageURL );
                    }
                }
                i_data.aThrobber.setImageList( aImages );
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
                    i_data.aCachedImageSets.push_back( i_images->getImageSet( set ) );

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
        ::vos::OGuard aGuard( GetMutex() );
        m_pData->aThrobber.start();
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL AnimatedImagesPeer::stopAnimation(  ) throw (RuntimeException)
    {
        ::vos::OGuard aGuard( GetMutex() );
        m_pData->aThrobber.stop();
    }

    //------------------------------------------------------------------------------------------------------------------
    ::sal_Bool SAL_CALL AnimatedImagesPeer::isAnimationRunning(  ) throw (RuntimeException)
    {
        ::vos::OGuard aGuard( GetMutex() );
        return m_pData->aThrobber.isRunning();
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL AnimatedImagesPeer::setProperty( const ::rtl::OUString& i_propertyName, const Any& i_value ) throw(RuntimeException)
    {
        ::vos::OGuard aGuard( GetMutex() );

        const sal_uInt16 nPropertyId = GetPropertyId( i_propertyName );
        switch ( nPropertyId )
        {
            case BASEPROPERTY_STEP_TIME:
            {
                sal_Int32 nStepTime( 0 );
                if ( i_value >>= nStepTime )
                    m_pData->aThrobber.setStepTime( nStepTime );
                break;
            }
            case BASEPROPERTY_AUTO_REPEAT:
            {
                sal_Bool bRepeat( sal_True );
                if ( i_value >>= bRepeat )
                    m_pData->aThrobber.setRepeat( bRepeat );
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
        ::vos::OGuard aGuard( GetMutex() );

        Any aReturn;

        const sal_uInt16 nPropertyId = GetPropertyId( i_propertyName );
        switch ( nPropertyId )
        {
        case BASEPROPERTY_STEP_TIME:
            aReturn <<= m_pData->aThrobber.getStepTime();
            break;

        case BASEPROPERTY_AUTO_REPEAT:
            aReturn <<= m_pData->aThrobber.getRepeat();
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
        ::vos::OGuard aGuard( GetMutex() );

        lcl_updateImageList_nothrow( *m_pData, Reference< XAnimatedImages >( i_animatedImages, UNO_QUERY_THROW ) );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL AnimatedImagesPeer::elementInserted( const ContainerEvent& i_event ) throw (RuntimeException)
    {
        impl_updateImages_nolck( i_event.Source );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL AnimatedImagesPeer::elementRemoved( const ContainerEvent& i_event ) throw (RuntimeException)
    {
        impl_updateImages_nolck( i_event.Source );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL AnimatedImagesPeer::elementReplaced( const ContainerEvent& i_event ) throw (RuntimeException)
    {
        impl_updateImages_nolck( i_event.Source );
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

//......................................................................................................................
} // namespace toolkit
//......................................................................................................................
