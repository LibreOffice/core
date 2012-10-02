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


#include "toolkit/controls/animatedimages.hxx"
#include "toolkit/helper/servicenames.hxx"
#include "toolkit/helper/property.hxx"
#include "toolkit/helper/unopropertyarrayhelper.hxx"

#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/awt/VisualEffect.hpp>
#include <com/sun/star/awt/ImageScaleMode.hpp>
#include <com/sun/star/util/XModifyListener.hpp>

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
    using ::com::sun::star::container::ContainerEvent;
    using ::com::sun::star::container::XContainerListener;
    using ::com::sun::star::beans::XPropertySetInfo;
    using ::com::sun::star::lang::DisposedException;
    using ::com::sun::star::lang::IndexOutOfBoundsException;
    using ::com::sun::star::lang::EventObject;
    using ::com::sun::star::awt::XControlModel;
    using ::com::sun::star::awt::XAnimatedImages;
    using ::com::sun::star::lang::IllegalArgumentException;
    using ::com::sun::star::awt::XWindowPeer;
    using ::com::sun::star::util::XModifyListener;
    using ::com::sun::star::awt::XToolkit;
    using ::com::sun::star::lang::XMultiServiceFactory;
    /** === end UNO using === **/
    namespace VisualEffect = ::com::sun::star::awt::VisualEffect;
    namespace ImageScaleMode = ::com::sun::star::awt::ImageScaleMode;

    //==================================================================================================================
    //= AnimatedImagesControl
    //==================================================================================================================
    //------------------------------------------------------------------------------------------------------------------
    AnimatedImagesControl::AnimatedImagesControl( Reference< XMultiServiceFactory > const & i_factory )
        :AnimatedImagesControl_Base( i_factory )
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    ::rtl::OUString AnimatedImagesControl::GetComponentServiceName()
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "AnimatedImages" ) );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL AnimatedImagesControl::startAnimation(  ) throw (RuntimeException)
    {
        Reference< XAnimation > xAnimation( getPeer(), UNO_QUERY );
        if ( xAnimation.is() )
            xAnimation->startAnimation();
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL AnimatedImagesControl::stopAnimation(  ) throw (RuntimeException)
    {
        Reference< XAnimation > xAnimation( getPeer(), UNO_QUERY );
        if ( xAnimation.is() )
            xAnimation->stopAnimation();
    }

    //------------------------------------------------------------------------------------------------------------------
    ::sal_Bool SAL_CALL AnimatedImagesControl::isAnimationRunning(  ) throw (RuntimeException)
    {
        Reference< XAnimation > xAnimation( getPeer(), UNO_QUERY );
        if ( xAnimation.is() )
            return xAnimation->isAnimationRunning();
        return sal_False;
    }

    //------------------------------------------------------------------------------------------------------------------
    ::rtl::OUString SAL_CALL AnimatedImagesControl::getImplementationName(  ) throw(RuntimeException)
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "org.openoffice.comp.toolkit.AnimatedImagesControl" ) );
    }

    //------------------------------------------------------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL AnimatedImagesControl::getSupportedServiceNames() throw(RuntimeException)
    {
        Sequence< ::rtl::OUString > aServices( AnimatedImagesControl_Base::getSupportedServiceNames() );
        aServices.realloc( aServices.getLength() + 1 );
        aServices[ aServices.getLength() - 1 ] = ::rtl::OUString::createFromAscii( szServiceName_AnimatedImagesControl );
        return aServices;
    }

    //------------------------------------------------------------------------------------------------------------------
    namespace
    {
        void lcl_updatePeer( Reference< XWindowPeer > const& i_peer, Reference< XControlModel > const& i_model )
        {
            const Reference< XModifyListener > xPeerModify( i_peer, UNO_QUERY );
            if ( xPeerModify.is() )
            {
                EventObject aEvent;
                aEvent.Source = i_model;
                xPeerModify->modified( aEvent );
            }
        }
    }

    //------------------------------------------------------------------------------------------------------------------
    sal_Bool SAL_CALL AnimatedImagesControl::setModel( const Reference< XControlModel >& i_rModel ) throw ( RuntimeException )
    {
        const Reference< XAnimatedImages > xOldContainer( getModel(), UNO_QUERY );
        const Reference< XAnimatedImages > xNewContainer( i_rModel, UNO_QUERY );

        if ( !AnimatedImagesControl_Base::setModel( i_rModel ) )
            return sal_False;

        if ( xOldContainer.is() )
            xOldContainer->removeContainerListener( this );

        if ( xNewContainer.is() )
            xNewContainer->addContainerListener( this );

        lcl_updatePeer( getPeer(), getModel() );

        return sal_True;
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL AnimatedImagesControl::createPeer( const Reference< XToolkit >& i_toolkit, const Reference< XWindowPeer >& i_parentPeer ) throw(RuntimeException)
    {
        AnimatedImagesControl_Base::createPeer( i_toolkit, i_parentPeer );

        lcl_updatePeer( getPeer(), getModel() );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL AnimatedImagesControl::elementInserted( const ContainerEvent& i_event ) throw (RuntimeException)
    {
        const Reference< XContainerListener > xPeerListener( getPeer(), UNO_QUERY );
        if ( xPeerListener.is() )
            xPeerListener->elementInserted( i_event );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL AnimatedImagesControl::elementRemoved( const ContainerEvent& i_event ) throw (RuntimeException)
    {
        const Reference< XContainerListener > xPeerListener( getPeer(), UNO_QUERY );
        if ( xPeerListener.is() )
            xPeerListener->elementRemoved( i_event );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL AnimatedImagesControl::elementReplaced( const ContainerEvent& i_event ) throw (RuntimeException)
    {
        const Reference< XContainerListener > xPeerListener( getPeer(), UNO_QUERY );
        if ( xPeerListener.is() )
            xPeerListener->elementReplaced( i_event );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL AnimatedImagesControl::disposing( const EventObject& i_event ) throw (RuntimeException)
    {
        UnoControlBase::disposing( i_event );
    }

    //==================================================================================================================
    //= AnimatedImagesControlModel_Data
    //==================================================================================================================
    struct AnimatedImagesControlModel_Data
    {
        ::std::vector< Sequence< ::rtl::OUString > >    aImageSets;
    };

    namespace
    {
        void lcl_checkIndex( const AnimatedImagesControlModel_Data& i_data, const sal_Int32 i_index, const Reference< XInterface >& i_context,
            const bool i_forInsert = false )
        {
            if ( ( i_index < 0 ) || ( size_t( i_index ) > i_data.aImageSets.size() + ( i_forInsert ? 1 : 0 ) ) )
                throw IndexOutOfBoundsException( ::rtl::OUString(), i_context );
        }

        void lcl_notify( ::osl::ClearableMutexGuard& i_guard, ::cppu::OBroadcastHelper& i_broadcaseHelper,
            void ( SAL_CALL XContainerListener::*i_notificationMethod )( const ContainerEvent& ),
            const sal_Int32 i_accessor, const Sequence< ::rtl::OUString >& i_imageURLs, const Reference< XInterface >& i_context )
        {
            ::cppu::OInterfaceContainerHelper* pContainerListeners = i_broadcaseHelper.getContainer( XContainerListener::static_type() );
            if ( pContainerListeners == NULL )
                return;

            ContainerEvent aEvent;
            aEvent.Source = i_context;
            aEvent.Accessor <<= i_accessor;
            aEvent.Element <<= i_imageURLs;

            i_guard.clear();
            pContainerListeners->notifyEach( i_notificationMethod, aEvent );
        }
    }

    //==================================================================================================================
    //= AnimatedImagesControlModel
    //==================================================================================================================
    //------------------------------------------------------------------------------------------------------------------
    AnimatedImagesControlModel::AnimatedImagesControlModel( Reference< XMultiServiceFactory > const & i_factory )
        :AnimatedImagesControlModel_Base( i_factory )
        ,m_pData( new AnimatedImagesControlModel_Data )
    {
        ImplRegisterProperty( BASEPROPERTY_AUTO_REPEAT );
        ImplRegisterProperty( BASEPROPERTY_BORDER );
        ImplRegisterProperty( BASEPROPERTY_BORDERCOLOR );
        ImplRegisterProperty( BASEPROPERTY_BACKGROUNDCOLOR );
        ImplRegisterProperty( BASEPROPERTY_DEFAULTCONTROL );
        ImplRegisterProperty( BASEPROPERTY_ENABLEVISIBLE );
        ImplRegisterProperty( BASEPROPERTY_HELPTEXT );
        ImplRegisterProperty( BASEPROPERTY_HELPURL );
        ImplRegisterProperty( BASEPROPERTY_IMAGE_SCALE_MODE );
        ImplRegisterProperty( BASEPROPERTY_STEP_TIME );
    }

    //------------------------------------------------------------------------------------------------------------------
    AnimatedImagesControlModel::AnimatedImagesControlModel( const AnimatedImagesControlModel& i_copySource )
        :AnimatedImagesControlModel_Base( i_copySource )
        ,m_pData( new AnimatedImagesControlModel_Data( *i_copySource.m_pData ) )
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    AnimatedImagesControlModel::~AnimatedImagesControlModel()
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    UnoControlModel* AnimatedImagesControlModel::Clone() const
    {
        return new AnimatedImagesControlModel( *this );
    }

    //------------------------------------------------------------------------------------------------------------------
    Reference< XPropertySetInfo > SAL_CALL AnimatedImagesControlModel::getPropertySetInfo(  ) throw(RuntimeException)
    {
        static Reference< XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
        return xInfo;
    }

    //------------------------------------------------------------------------------------------------------------------
    ::rtl::OUString SAL_CALL AnimatedImagesControlModel::getServiceName() throw(RuntimeException)
    {
        return ::rtl::OUString::createFromAscii( szServiceName_AnimatedImagesControlModel );
    }

    //------------------------------------------------------------------------------------------------------------------
    ::rtl::OUString SAL_CALL AnimatedImagesControlModel::getImplementationName(  ) throw(RuntimeException)
    {
        return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("org.openoffice.comp.toolkit.AnimatedImagesControlModel"));
    }

    //------------------------------------------------------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL AnimatedImagesControlModel::getSupportedServiceNames() throw(RuntimeException)
    {
        Sequence< ::rtl::OUString > aServiceNames(2);
        aServiceNames[0] = ::rtl::OUString::createFromAscii( szServiceName_AnimatedImagesControlModel );
        aServiceNames[1] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoControlModel"));
        return aServiceNames;
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL AnimatedImagesControlModel::setFastPropertyValue_NoBroadcast( sal_Int32 i_handle, const Any& i_value ) throw (Exception)
    {
        switch ( i_handle )
        {
        case BASEPROPERTY_IMAGE_SCALE_MODE:
        {
            sal_Int16 nImageScaleMode( ImageScaleMode::Anisotropic );
            OSL_VERIFY( i_value >>= nImageScaleMode );  // convertFastPropertyValue ensures that this has the proper type
            if  (   ( nImageScaleMode != ImageScaleMode::None )
                &&  ( nImageScaleMode != ImageScaleMode::Isotropic )
                &&  ( nImageScaleMode != ImageScaleMode::Anisotropic )
                )
                throw IllegalArgumentException( ::rtl::OUString(), *this, 1 );
        }
        break;
        }

        AnimatedImagesControlModel_Base::setFastPropertyValue_NoBroadcast( i_handle, i_value );
    }

    //------------------------------------------------------------------------------------------------------------------
    Any AnimatedImagesControlModel::ImplGetDefaultValue( sal_uInt16 i_propertyId ) const
    {
        switch ( i_propertyId )
        {
        case BASEPROPERTY_DEFAULTCONTROL:
            return makeAny( ::rtl::OUString::createFromAscii( szServiceName_AnimatedImagesControl ) );

        case BASEPROPERTY_BORDER:
            return makeAny( VisualEffect::NONE );

        case BASEPROPERTY_STEP_TIME:
            return makeAny( (sal_Int32) 100 );

        case BASEPROPERTY_AUTO_REPEAT:
            return makeAny( (sal_Bool)sal_True );

        case BASEPROPERTY_IMAGE_SCALE_MODE:
            return makeAny( ImageScaleMode::None );

        default:
            return UnoControlModel::ImplGetDefaultValue( i_propertyId );
        }
    }

    //------------------------------------------------------------------------------------------------------------------
    ::cppu::IPropertyArrayHelper& SAL_CALL AnimatedImagesControlModel::getInfoHelper()
    {
        static UnoPropertyArrayHelper* pHelper = NULL;
        if ( !pHelper )
        {
            Sequence< sal_Int32 > aIDs = ImplGetPropertyIds();
            pHelper = new UnoPropertyArrayHelper( aIDs );
        }
        return *pHelper;
    }

    //------------------------------------------------------------------------------------------------------------------
    ::sal_Int32 SAL_CALL AnimatedImagesControlModel::getStepTime() throw (RuntimeException)
    {
        sal_Int32 nStepTime( 100 );
        OSL_VERIFY( getPropertyValue( GetPropertyName( BASEPROPERTY_STEP_TIME ) ) >>= nStepTime );
        return nStepTime;
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL AnimatedImagesControlModel::setStepTime( ::sal_Int32 i_stepTime ) throw (RuntimeException)
    {
        setPropertyValue( GetPropertyName( BASEPROPERTY_STEP_TIME ), makeAny( i_stepTime ) );
    }

    //------------------------------------------------------------------------------------------------------------------
    ::sal_Bool SAL_CALL AnimatedImagesControlModel::getAutoRepeat() throw (RuntimeException)
    {
        sal_Bool bAutoRepeat( sal_True );
        OSL_VERIFY( getPropertyValue( GetPropertyName( BASEPROPERTY_AUTO_REPEAT ) ) >>= bAutoRepeat );
        return bAutoRepeat;
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL AnimatedImagesControlModel::setAutoRepeat( ::sal_Bool i_autoRepeat ) throw (RuntimeException)
    {
        setPropertyValue( GetPropertyName( BASEPROPERTY_AUTO_REPEAT ), makeAny( i_autoRepeat ) );
    }

    //------------------------------------------------------------------------------------------------------------------
    ::sal_Int16 SAL_CALL AnimatedImagesControlModel::getScaleMode() throw (RuntimeException)
    {
        sal_Int16 nImageScaleMode( ImageScaleMode::Anisotropic );
        OSL_VERIFY( getPropertyValue( GetPropertyName( BASEPROPERTY_IMAGE_SCALE_MODE ) ) >>= nImageScaleMode );
        return nImageScaleMode;
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL AnimatedImagesControlModel::setScaleMode( ::sal_Int16 i_scaleMode ) throw (IllegalArgumentException, RuntimeException)
    {
        setPropertyValue( GetPropertyName( BASEPROPERTY_IMAGE_SCALE_MODE ), makeAny( i_scaleMode ) );
    }

    //------------------------------------------------------------------------------------------------------------------
    ::sal_Int32 SAL_CALL AnimatedImagesControlModel::getImageSetCount(  ) throw (RuntimeException)
    {
        ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );
        if ( GetBroadcastHelper().bDisposed || GetBroadcastHelper().bInDispose )
            throw DisposedException();

        return m_pData->aImageSets.size();
    }

    //------------------------------------------------------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL AnimatedImagesControlModel::getImageSet( ::sal_Int32 i_index ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );
        if ( GetBroadcastHelper().bDisposed || GetBroadcastHelper().bInDispose )
            throw DisposedException();

        lcl_checkIndex( *m_pData, i_index, *this );

        return m_pData->aImageSets[ i_index ];
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL AnimatedImagesControlModel::insertImageSet( ::sal_Int32 i_index, const Sequence< ::rtl::OUString >& i_imageURLs ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        ::osl::ClearableMutexGuard aGuard( GetMutex() );
        // sanity checks
        if ( GetBroadcastHelper().bDisposed || GetBroadcastHelper().bInDispose )
            throw DisposedException();

        lcl_checkIndex( *m_pData, i_index, *this, true );

        // actaul insertion
        m_pData->aImageSets.insert( m_pData->aImageSets.begin() + i_index, i_imageURLs );

        // listener notification
        lcl_notify( aGuard, BrdcstHelper, &XContainerListener::elementInserted, i_index, i_imageURLs, *this );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL AnimatedImagesControlModel::replaceImageSet( ::sal_Int32 i_index, const Sequence< ::rtl::OUString >& i_imageURLs ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        ::osl::ClearableMutexGuard aGuard( GetMutex() );
        // sanity checks
        if ( GetBroadcastHelper().bDisposed || GetBroadcastHelper().bInDispose )
            throw DisposedException();

        lcl_checkIndex( *m_pData, i_index, *this );

        // actaul insertion
        m_pData->aImageSets[ i_index ] = i_imageURLs;

        // listener notification
        lcl_notify( aGuard, BrdcstHelper, &XContainerListener::elementReplaced, i_index, i_imageURLs, *this );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL AnimatedImagesControlModel::removeImageSet( ::sal_Int32 i_index ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        ::osl::ClearableMutexGuard aGuard( GetMutex() );
        // sanity checks
        if ( GetBroadcastHelper().bDisposed || GetBroadcastHelper().bInDispose )
            throw DisposedException();

        lcl_checkIndex( *m_pData, i_index, *this );

        // actual removal
        ::std::vector< Sequence< ::rtl::OUString > >::iterator removalPos = m_pData->aImageSets.begin() + i_index;
        Sequence< ::rtl::OUString > aRemovedElement( *removalPos );
        m_pData->aImageSets.erase( removalPos );

        // listener notification
        lcl_notify( aGuard, BrdcstHelper, &XContainerListener::elementRemoved, i_index, aRemovedElement, *this );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL AnimatedImagesControlModel::addContainerListener( const Reference< XContainerListener >& i_listener ) throw (RuntimeException)
    {
        BrdcstHelper.addListener( XContainerListener::static_type(), i_listener );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL AnimatedImagesControlModel::removeContainerListener( const Reference< XContainerListener >& i_listener ) throw (RuntimeException)
    {
        BrdcstHelper.removeListener( XContainerListener::static_type(), i_listener );
    }

//......................................................................................................................
} // namespace toolkit
//......................................................................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
