/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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


#include <controls/animatedimages.hxx>
#include <helper/property.hxx>

#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/awt/VisualEffect.hpp>
#include <com/sun/star/awt/ImageScaleMode.hpp>
#include <com/sun/star/awt/XAnimation.hpp>
#include <com/sun/star/awt/XAnimatedImages.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/container/XContainerListener.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/util/XModifyListener.hpp>
#include <o3tl/safeint.hxx>
#include <toolkit/controls/unocontrolbase.hxx>
#include <toolkit/controls/unocontrolmodel.hxx>

#include <cppuhelper/implbase2.hxx>

#include <helper/unopropertyarrayhelper.hxx>

using namespace css::awt;
using namespace css::container;
using namespace css::lang;
using namespace css::uno;
using namespace cpo::uno;

namespace {

typedef ::cppu::AggImplInheritanceHelper2   <   UnoControlBase
                                            ,   css::awt::XAnimation
                                            ,   css::container::XContainerListener
                                            >   AnimatedImagesControl_Base;

class AnimatedImagesControl : public AnimatedImagesControl_Base
{
public:
    AnimatedImagesControl();
    OUString GetComponentServiceName() const override;

    // XAnimation
    virtual void startAnimation(  ) override;
    virtual void stopAnimation(  ) override;
    virtual bool isAnimationRunning(  ) override;

    // XServiceInfo
    OUString getImplementationName(  ) override;
    cpo::uno::Sequence< OUString > getSupportedServiceNames() override;

    // XControl
    bool setModel( const css::uno::Reference< css::awt::XControlModel >& i_rModel ) override;
    void createPeer( const css::uno::Reference< css::awt::XToolkit >& i_toolkit, const css::uno::Reference< css::awt::XWindowPeer >& i_parentPeer ) override;


    // XContainerListener
    virtual void elementInserted( const css::container::ContainerEvent& Event ) override;
    virtual void elementRemoved( const css::container::ContainerEvent& Event ) override;
    virtual void elementReplaced( const css::container::ContainerEvent& Event ) override;

    // XEventListener
    virtual void disposing( const css::lang::EventObject& i_event ) override;
};

    AnimatedImagesControl::AnimatedImagesControl()
    {
    }


    OUString AnimatedImagesControl::GetComponentServiceName() const
    {
        return u"AnimatedImages"_ustr;
    }


    void AnimatedImagesControl::startAnimation(  )
    {
        Reference< XAnimation > xAnimation( getPeer(), UNO_QUERY );
        if ( xAnimation.is() )
            xAnimation->startAnimation();
    }


    void AnimatedImagesControl::stopAnimation(  )
    {
        Reference< XAnimation > xAnimation( getPeer(), UNO_QUERY );
        if ( xAnimation.is() )
            xAnimation->stopAnimation();
    }


    bool AnimatedImagesControl::isAnimationRunning(  )
    {
        Reference< XAnimation > xAnimation( getPeer(), UNO_QUERY );
        if ( xAnimation.is() )
            return xAnimation->isAnimationRunning();
        return false;
    }


    OUString AnimatedImagesControl::getImplementationName(  )
    {
        return u"org.openoffice.comp.toolkit.AnimatedImagesControl"_ustr;
    }


    Sequence< OUString > AnimatedImagesControl::getSupportedServiceNames()
    {
        Sequence< OUString > aServices( AnimatedImagesControl_Base::getSupportedServiceNames() );
        aServices.realloc( aServices.getLength() + 1 );
        aServices.getArray()[ aServices.getLength() - 1 ] = u"com.sun.star.awt.AnimatedImagesControl"_ustr;
        return aServices;
    }

    void lcl_updatePeer( Reference< XWindowPeer > const& i_peer, Reference< XControlModel > const& i_model )
    {
        const Reference< css::util::XModifyListener > xPeerModify( i_peer, UNO_QUERY );
        if ( xPeerModify.is() )
        {
            EventObject aEvent;
            aEvent.Source = i_model;
            xPeerModify->modified( aEvent );
        }
    }

    bool AnimatedImagesControl::setModel( const Reference< XControlModel >& i_rModel )
    {
        const Reference< XAnimatedImages > xOldContainer( getModel(), UNO_QUERY );
        const Reference< XAnimatedImages > xNewContainer( i_rModel, UNO_QUERY );

        if ( !AnimatedImagesControl_Base::setModel( i_rModel ) )
            return false;

        if ( xOldContainer.is() )
            xOldContainer->removeContainerListener( this );

        if ( xNewContainer.is() )
            xNewContainer->addContainerListener( this );

        lcl_updatePeer( getPeer(), getModel() );

        return true;
    }


    void AnimatedImagesControl::createPeer( const Reference< XToolkit >& i_toolkit, const Reference< XWindowPeer >& i_parentPeer )
    {
        AnimatedImagesControl_Base::createPeer( i_toolkit, i_parentPeer );

        lcl_updatePeer( getPeer(), getModel() );
    }


    void AnimatedImagesControl::elementInserted( const ContainerEvent& i_event )
    {
        const Reference< XContainerListener > xPeerListener( getPeer(), UNO_QUERY );
        if ( xPeerListener.is() )
            xPeerListener->elementInserted( i_event );
    }


    void AnimatedImagesControl::elementRemoved( const ContainerEvent& i_event )
    {
        const Reference< XContainerListener > xPeerListener( getPeer(), UNO_QUERY );
        if ( xPeerListener.is() )
            xPeerListener->elementRemoved( i_event );
    }


    void AnimatedImagesControl::elementReplaced( const ContainerEvent& i_event )
    {
        const Reference< XContainerListener > xPeerListener( getPeer(), UNO_QUERY );
        if ( xPeerListener.is() )
            xPeerListener->elementReplaced( i_event );
    }


    void AnimatedImagesControl::disposing( const EventObject& i_event )
    {
        UnoControlBase::disposing( i_event );
    }

}

namespace toolkit {

    namespace
    {
        void lcl_checkIndex( const std::vector< cpo::uno::Sequence< OUString > > & rImageSets, const sal_Int32 i_index, const Reference< XInterface >& i_context,
            const bool i_forInsert = false )
        {
            if ( ( i_index < 0 ) || ( o3tl::make_unsigned( i_index ) > rImageSets.size() + ( i_forInsert ? 1 : 0 ) ) )
                throw IndexOutOfBoundsException( OUString(), i_context );
        }

        void lcl_notify( std::unique_lock<std::mutex>& i_guard, const comphelper::OInterfaceContainerHelper4<XContainerListener>& rContainer,
            void ( XContainerListener::*i_notificationMethod )( const ContainerEvent& ),
            const sal_Int32 i_accessor, const Sequence< OUString >& i_imageURLs, const Reference< XInterface >& i_context )
        {
            if ( !rContainer.getLength(i_guard) )
                return;

            ContainerEvent aEvent;
            aEvent.Source = i_context;
            aEvent.Accessor <<= i_accessor;
            aEvent.Element <<= i_imageURLs;

            rContainer.notifyEach( i_guard, i_notificationMethod, aEvent );
        }
    }


    AnimatedImagesControlModel::AnimatedImagesControlModel( Reference< css::uno::XComponentContext > const & i_factory )
        :AnimatedImagesControlModel_Base( i_factory )
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


    AnimatedImagesControlModel::AnimatedImagesControlModel( const AnimatedImagesControlModel& i_copySource )
        :AnimatedImagesControlModel_Base( i_copySource )
        ,maImageSets( i_copySource.maImageSets )
    {
    }


    AnimatedImagesControlModel::~AnimatedImagesControlModel()
    {
    }


    rtl::Reference<UnoControlModel> AnimatedImagesControlModel::Clone() const
    {
        std::unique_lock aGuard( m_aMutex );
        return new AnimatedImagesControlModel( *this );
    }


    Reference< css::beans::XPropertySetInfo > AnimatedImagesControlModel::getPropertySetInfo(  )
    {
        static Reference< css::beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
        return xInfo;
    }


    OUString AnimatedImagesControlModel::getServiceName()
    {
        return u"com.sun.star.awt.AnimatedImagesControlModel"_ustr;
    }


    OUString AnimatedImagesControlModel::getImplementationName(  )
    {
        return u"org.openoffice.comp.toolkit.AnimatedImagesControlModel"_ustr;
    }


    Sequence< OUString > AnimatedImagesControlModel::getSupportedServiceNames()
    {
        return { u"com.sun.star.awt.AnimatedImagesControlModel"_ustr, u"com.sun.star.awt.UnoControlModel"_ustr };
    }


    void AnimatedImagesControlModel::setFastPropertyValue_NoBroadcast( std::unique_lock<std::mutex>& rGuard, sal_Int32 i_handle, const Any& i_value )
    {
        switch ( i_handle )
        {
        case BASEPROPERTY_IMAGE_SCALE_MODE:
        {
            sal_Int16 nImageScaleMode( ImageScaleMode::ANISOTROPIC );
            OSL_VERIFY( i_value >>= nImageScaleMode );  // convertFastPropertyValue ensures that this has the proper type
            if  (   ( nImageScaleMode != ImageScaleMode::NONE )
                &&  ( nImageScaleMode != ImageScaleMode::ISOTROPIC )
                &&  ( nImageScaleMode != ImageScaleMode::ANISOTROPIC )
                )
                throw IllegalArgumentException( OUString(), *this, 1 );
        }
        break;
        }

        AnimatedImagesControlModel_Base::setFastPropertyValue_NoBroadcast( rGuard, i_handle, i_value );
    }


    Any AnimatedImagesControlModel::ImplGetDefaultValue( sal_uInt16 i_propertyId ) const
    {
        switch ( i_propertyId )
        {
        case BASEPROPERTY_DEFAULTCONTROL:
            return Any( u"com.sun.star.awt.AnimatedImagesControl"_ustr );

        case BASEPROPERTY_BORDER:
            return Any( css::awt::VisualEffect::NONE );

        case BASEPROPERTY_STEP_TIME:
            return Any( sal_Int32(100) );

        case BASEPROPERTY_AUTO_REPEAT:
            return Any( true );

        case BASEPROPERTY_IMAGE_SCALE_MODE:
            return Any( ImageScaleMode::NONE );

        default:
            return UnoControlModel::ImplGetDefaultValue( i_propertyId );
        }
    }


    ::cppu::IPropertyArrayHelper& AnimatedImagesControlModel::getInfoHelper()
    {
        static UnoPropertyArrayHelper aHelper( ImplGetPropertyIds() );
        return aHelper;
    }


    ::sal_Int32 AnimatedImagesControlModel::getStepTime()
    {
        sal_Int32 nStepTime( 100 );
        OSL_VERIFY( getPropertyValue( GetPropertyName( BASEPROPERTY_STEP_TIME ) ) >>= nStepTime );
        return nStepTime;
    }


    void AnimatedImagesControlModel::setStepTime( ::sal_Int32 i_stepTime )
    {
        setPropertyValue( GetPropertyName( BASEPROPERTY_STEP_TIME ), Any( i_stepTime ) );
    }


    bool AnimatedImagesControlModel::getAutoRepeat()
    {
        bool bAutoRepeat( true );
        OSL_VERIFY( getPropertyValue( GetPropertyName( BASEPROPERTY_AUTO_REPEAT ) ) >>= bAutoRepeat );
        return bAutoRepeat;
    }


    void AnimatedImagesControlModel::setAutoRepeat( bool i_autoRepeat )
    {
        setPropertyValue( GetPropertyName( BASEPROPERTY_AUTO_REPEAT ), Any( i_autoRepeat ) );
    }


    ::sal_Int16 AnimatedImagesControlModel::getScaleMode()
    {
        sal_Int16 nImageScaleMode( ImageScaleMode::ANISOTROPIC );
        OSL_VERIFY( getPropertyValue( GetPropertyName( BASEPROPERTY_IMAGE_SCALE_MODE ) ) >>= nImageScaleMode );
        return nImageScaleMode;
    }


    void AnimatedImagesControlModel::setScaleMode( ::sal_Int16 i_scaleMode )
    {
        setPropertyValue( GetPropertyName( BASEPROPERTY_IMAGE_SCALE_MODE ), Any( i_scaleMode ) );
    }


    ::sal_Int32 AnimatedImagesControlModel::getImageSetCount(  )
    {
        std::unique_lock aGuard( m_aMutex );
        if ( m_bDisposed )
            throw DisposedException();

        return maImageSets.size();
    }


    Sequence< OUString > AnimatedImagesControlModel::getImageSet( ::sal_Int32 i_index )
    {
        std::unique_lock aGuard( m_aMutex );
        if ( m_bDisposed )
            throw DisposedException();

        lcl_checkIndex( maImageSets, i_index, *this );

        return maImageSets[ i_index ];
    }


    void AnimatedImagesControlModel::insertImageSet( ::sal_Int32 i_index, const Sequence< OUString >& i_imageURLs )
    {
        std::unique_lock aGuard( m_aMutex );
        // sanity checks
        if ( m_bDisposed )
            throw DisposedException();

        lcl_checkIndex( maImageSets, i_index, *this, true );

        // actual insertion
        maImageSets.insert( maImageSets.begin() + i_index, i_imageURLs );

        // listener notification
        lcl_notify( aGuard, maContainerListeners, &XContainerListener::elementInserted, i_index, i_imageURLs, *this );
    }


    void AnimatedImagesControlModel::replaceImageSet( ::sal_Int32 i_index, const Sequence< OUString >& i_imageURLs )
    {
        std::unique_lock aGuard( m_aMutex );
        // sanity checks
        if ( m_bDisposed )
            throw DisposedException();

        lcl_checkIndex( maImageSets, i_index, *this );

        // actual insertion
        maImageSets[ i_index ] = i_imageURLs;

        // listener notification
        lcl_notify( aGuard, maContainerListeners, &XContainerListener::elementReplaced, i_index, i_imageURLs, *this );
    }


    void AnimatedImagesControlModel::removeImageSet( ::sal_Int32 i_index )
    {
        std::unique_lock aGuard( m_aMutex );
        // sanity checks
        if ( m_bDisposed )
            throw DisposedException();

        lcl_checkIndex( maImageSets, i_index, *this );

        // actual removal
        ::std::vector< Sequence< OUString > >::iterator removalPos = maImageSets.begin() + i_index;
        Sequence< OUString > aRemovedElement( *removalPos );
        maImageSets.erase( removalPos );

        // listener notification
        lcl_notify( aGuard, maContainerListeners, &XContainerListener::elementRemoved, i_index, aRemovedElement, *this );
    }


    void AnimatedImagesControlModel::addContainerListener( const Reference< XContainerListener >& i_listener )
    {
        std::unique_lock aGuard( m_aMutex );
        maContainerListeners.addInterface( aGuard, i_listener );
    }


    void AnimatedImagesControlModel::removeContainerListener( const Reference< XContainerListener >& i_listener )
    {
        std::unique_lock aGuard( m_aMutex );
        maContainerListeners.removeInterface( aGuard, i_listener );
    }

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
org_openoffice_comp_toolkit_AnimatedImagesControl_get_implementation(
    css::uno::XComponentContext *,
    cpo::uno::Sequence<cpo::uno::Any> const &)
{
    return cppu::acquire(new AnimatedImagesControl());
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
org_openoffice_comp_toolkit_AnimatedImagesControlModel_get_implementation(
    css::uno::XComponentContext *context,
    cpo::uno::Sequence<cpo::uno::Any> const &)
{
    return cppu::acquire(new toolkit::AnimatedImagesControlModel(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
