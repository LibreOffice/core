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


#include <toolkit/controls/animatedimages.hxx>
#include <toolkit/helper/property.hxx>

#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/awt/VisualEffect.hpp>
#include <com/sun/star/awt/ImageScaleMode.hpp>
#include <com/sun/star/awt/XAnimation.hpp>
#include <com/sun/star/awt/XAnimatedImages.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/container/XContainerListener.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/util/XModifyListener.hpp>

#include <toolkit/controls/unocontrolbase.hxx>
#include <toolkit/controls/unocontrolmodel.hxx>

#include <cppuhelper/implbase.hxx>

#include "helper/unopropertyarrayhelper.hxx"

using namespace css::awt;
using namespace css::container;
using namespace css::lang;
using namespace css::uno;

namespace {

typedef ::cppu::AggImplInheritanceHelper    <   UnoControlBase
                                            ,   css::awt::XAnimation
                                            ,   css::container::XContainerListener
                                            >   AnimatedImagesControl_Base;

class AnimatedImagesControl : public AnimatedImagesControl_Base
{
public:
    AnimatedImagesControl();
    OUString GetComponentServiceName() override;

    // XAnimation
    virtual void SAL_CALL startAnimation(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL stopAnimation(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL isAnimationRunning(  ) throw (css::uno::RuntimeException, std::exception) override;

    // XServiceInfo
    OUString SAL_CALL getImplementationName(  ) throw(css::uno::RuntimeException, std::exception) override;
    css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(css::uno::RuntimeException, std::exception) override;

    // XControl
    sal_Bool SAL_CALL setModel( const css::uno::Reference< css::awt::XControlModel >& i_rModel ) throw ( css::uno::RuntimeException, std::exception ) override;
    void SAL_CALL createPeer( const css::uno::Reference< css::awt::XToolkit >& i_toolkit, const css::uno::Reference< css::awt::XWindowPeer >& i_parentPeer ) throw(css::uno::RuntimeException, std::exception) override;


    // XContainerListener
    virtual void SAL_CALL elementInserted( const css::container::ContainerEvent& Event ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL elementRemoved( const css::container::ContainerEvent& Event ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL elementReplaced( const css::container::ContainerEvent& Event ) throw (css::uno::RuntimeException, std::exception) override;

    // XEventListener
    virtual void SAL_CALL disposing( const css::lang::EventObject& i_event ) throw (css::uno::RuntimeException, std::exception) override;
};

    AnimatedImagesControl::AnimatedImagesControl()
        :AnimatedImagesControl_Base()
    {
    }


    OUString AnimatedImagesControl::GetComponentServiceName()
    {
        return OUString( "AnimatedImages" );
    }


    void SAL_CALL AnimatedImagesControl::startAnimation(  ) throw (RuntimeException, std::exception)
    {
        Reference< XAnimation > xAnimation( getPeer(), UNO_QUERY );
        if ( xAnimation.is() )
            xAnimation->startAnimation();
    }


    void SAL_CALL AnimatedImagesControl::stopAnimation(  ) throw (RuntimeException, std::exception)
    {
        Reference< XAnimation > xAnimation( getPeer(), UNO_QUERY );
        if ( xAnimation.is() )
            xAnimation->stopAnimation();
    }


    sal_Bool SAL_CALL AnimatedImagesControl::isAnimationRunning(  ) throw (RuntimeException, std::exception)
    {
        Reference< XAnimation > xAnimation( getPeer(), UNO_QUERY );
        if ( xAnimation.is() )
            return xAnimation->isAnimationRunning();
        return false;
    }


    OUString SAL_CALL AnimatedImagesControl::getImplementationName(  ) throw(RuntimeException, std::exception)
    {
        return OUString( "org.openoffice.comp.toolkit.AnimatedImagesControl" );
    }


    Sequence< OUString > SAL_CALL AnimatedImagesControl::getSupportedServiceNames() throw(RuntimeException, std::exception)
    {
        Sequence< OUString > aServices( AnimatedImagesControl_Base::getSupportedServiceNames() );
        aServices.realloc( aServices.getLength() + 1 );
        aServices[ aServices.getLength() - 1 ] = "com.sun.star.awt.AnimatedImagesControl";
        return aServices;
    }


    namespace
    {
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
    }


    sal_Bool SAL_CALL AnimatedImagesControl::setModel( const Reference< XControlModel >& i_rModel ) throw ( RuntimeException, std::exception )
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


    void SAL_CALL AnimatedImagesControl::createPeer( const Reference< XToolkit >& i_toolkit, const Reference< XWindowPeer >& i_parentPeer ) throw(RuntimeException, std::exception)
    {
        AnimatedImagesControl_Base::createPeer( i_toolkit, i_parentPeer );

        lcl_updatePeer( getPeer(), getModel() );
    }


    void SAL_CALL AnimatedImagesControl::elementInserted( const ContainerEvent& i_event ) throw (RuntimeException, std::exception)
    {
        const Reference< XContainerListener > xPeerListener( getPeer(), UNO_QUERY );
        if ( xPeerListener.is() )
            xPeerListener->elementInserted( i_event );
    }


    void SAL_CALL AnimatedImagesControl::elementRemoved( const ContainerEvent& i_event ) throw (RuntimeException, std::exception)
    {
        const Reference< XContainerListener > xPeerListener( getPeer(), UNO_QUERY );
        if ( xPeerListener.is() )
            xPeerListener->elementRemoved( i_event );
    }


    void SAL_CALL AnimatedImagesControl::elementReplaced( const ContainerEvent& i_event ) throw (RuntimeException, std::exception)
    {
        const Reference< XContainerListener > xPeerListener( getPeer(), UNO_QUERY );
        if ( xPeerListener.is() )
            xPeerListener->elementReplaced( i_event );
    }


    void SAL_CALL AnimatedImagesControl::disposing( const EventObject& i_event ) throw (RuntimeException, std::exception)
    {
        UnoControlBase::disposing( i_event );
    }

}

namespace toolkit {

    struct AnimatedImagesControlModel_Data
    {
        ::std::vector< Sequence< OUString > >    aImageSets;
    };

    namespace
    {
        void lcl_checkIndex( const AnimatedImagesControlModel_Data& i_data, const sal_Int32 i_index, const Reference< XInterface >& i_context,
            const bool i_forInsert = false )
        {
            if ( ( i_index < 0 ) || ( size_t( i_index ) > i_data.aImageSets.size() + ( i_forInsert ? 1 : 0 ) ) )
                throw IndexOutOfBoundsException( OUString(), i_context );
        }

        void lcl_notify( ::osl::ClearableMutexGuard& i_guard, ::cppu::OBroadcastHelper& i_broadcaseHelper,
            void ( SAL_CALL XContainerListener::*i_notificationMethod )( const ContainerEvent& ),
            const sal_Int32 i_accessor, const Sequence< OUString >& i_imageURLs, const Reference< XInterface >& i_context )
        {
            ::cppu::OInterfaceContainerHelper* pContainerListeners = i_broadcaseHelper.getContainer( cppu::UnoType<XContainerListener>::get() );
            if ( pContainerListeners == nullptr )
                return;

            ContainerEvent aEvent;
            aEvent.Source = i_context;
            aEvent.Accessor <<= i_accessor;
            aEvent.Element <<= i_imageURLs;

            i_guard.clear();
            pContainerListeners->notifyEach( i_notificationMethod, aEvent );
        }
    }


    AnimatedImagesControlModel::AnimatedImagesControlModel( Reference< css::uno::XComponentContext > const & i_factory )
        :AnimatedImagesControlModel_Base( i_factory )
        ,m_xData( new AnimatedImagesControlModel_Data )
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
        ,m_xData( new AnimatedImagesControlModel_Data( *i_copySource.m_xData ) )
    {
    }


    AnimatedImagesControlModel::~AnimatedImagesControlModel()
    {
    }


    UnoControlModel* AnimatedImagesControlModel::Clone() const
    {
        return new AnimatedImagesControlModel( *this );
    }


    Reference< css::beans::XPropertySetInfo > SAL_CALL AnimatedImagesControlModel::getPropertySetInfo(  ) throw(RuntimeException, std::exception)
    {
        static Reference< css::beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
        return xInfo;
    }


    OUString SAL_CALL AnimatedImagesControlModel::getServiceName() throw(RuntimeException, std::exception)
    {
        return OUString("com.sun.star.awt.AnimatedImagesControlModel");
    }


    OUString SAL_CALL AnimatedImagesControlModel::getImplementationName(  ) throw(RuntimeException, std::exception)
    {
        return OUString("org.openoffice.comp.toolkit.AnimatedImagesControlModel");
    }


    Sequence< OUString > SAL_CALL AnimatedImagesControlModel::getSupportedServiceNames() throw(RuntimeException, std::exception)
    {
        Sequence< OUString > aServiceNames(2);
        aServiceNames[0] = "com.sun.star.awt.AnimatedImagesControlModel";
        aServiceNames[1] = "com.sun.star.awt.UnoControlModel";
        return aServiceNames;
    }


    void SAL_CALL AnimatedImagesControlModel::setFastPropertyValue_NoBroadcast( sal_Int32 i_handle, const Any& i_value ) throw (Exception, std::exception)
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

        AnimatedImagesControlModel_Base::setFastPropertyValue_NoBroadcast( i_handle, i_value );
    }


    Any AnimatedImagesControlModel::ImplGetDefaultValue( sal_uInt16 i_propertyId ) const
    {
        switch ( i_propertyId )
        {
        case BASEPROPERTY_DEFAULTCONTROL:
            return makeAny( OUString("com.sun.star.awt.AnimatedImagesControl") );

        case BASEPROPERTY_BORDER:
            return makeAny( css::awt::VisualEffect::NONE );

        case BASEPROPERTY_STEP_TIME:
            return makeAny( (sal_Int32) 100 );

        case BASEPROPERTY_AUTO_REPEAT:
            return makeAny( true );

        case BASEPROPERTY_IMAGE_SCALE_MODE:
            return makeAny( ImageScaleMode::NONE );

        default:
            return UnoControlModel::ImplGetDefaultValue( i_propertyId );
        }
    }


    ::cppu::IPropertyArrayHelper& SAL_CALL AnimatedImagesControlModel::getInfoHelper()
    {
        static UnoPropertyArrayHelper* pHelper = nullptr;
        if ( !pHelper )
        {
            Sequence< sal_Int32 > aIDs = ImplGetPropertyIds();
            pHelper = new UnoPropertyArrayHelper( aIDs );
        }
        return *pHelper;
    }


    ::sal_Int32 SAL_CALL AnimatedImagesControlModel::getStepTime() throw (RuntimeException, std::exception)
    {
        sal_Int32 nStepTime( 100 );
        OSL_VERIFY( getPropertyValue( GetPropertyName( BASEPROPERTY_STEP_TIME ) ) >>= nStepTime );
        return nStepTime;
    }


    void SAL_CALL AnimatedImagesControlModel::setStepTime( ::sal_Int32 i_stepTime ) throw (RuntimeException, std::exception)
    {
        setPropertyValue( GetPropertyName( BASEPROPERTY_STEP_TIME ), makeAny( i_stepTime ) );
    }


    sal_Bool SAL_CALL AnimatedImagesControlModel::getAutoRepeat() throw (RuntimeException, std::exception)
    {
        bool bAutoRepeat( true );
        OSL_VERIFY( getPropertyValue( GetPropertyName( BASEPROPERTY_AUTO_REPEAT ) ) >>= bAutoRepeat );
        return bAutoRepeat;
    }


    void SAL_CALL AnimatedImagesControlModel::setAutoRepeat( sal_Bool i_autoRepeat ) throw (RuntimeException, std::exception)
    {
        setPropertyValue( GetPropertyName( BASEPROPERTY_AUTO_REPEAT ), makeAny( i_autoRepeat ) );
    }


    ::sal_Int16 SAL_CALL AnimatedImagesControlModel::getScaleMode() throw (RuntimeException, std::exception)
    {
        sal_Int16 nImageScaleMode( ImageScaleMode::ANISOTROPIC );
        OSL_VERIFY( getPropertyValue( GetPropertyName( BASEPROPERTY_IMAGE_SCALE_MODE ) ) >>= nImageScaleMode );
        return nImageScaleMode;
    }


    void SAL_CALL AnimatedImagesControlModel::setScaleMode( ::sal_Int16 i_scaleMode ) throw (IllegalArgumentException, RuntimeException, std::exception)
    {
        setPropertyValue( GetPropertyName( BASEPROPERTY_IMAGE_SCALE_MODE ), makeAny( i_scaleMode ) );
    }


    ::sal_Int32 SAL_CALL AnimatedImagesControlModel::getImageSetCount(  ) throw (RuntimeException, std::exception)
    {
        ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );
        if ( GetBroadcastHelper().bDisposed || GetBroadcastHelper().bInDispose )
            throw DisposedException();

        return m_xData->aImageSets.size();
    }


    Sequence< OUString > SAL_CALL AnimatedImagesControlModel::getImageSet( ::sal_Int32 i_index ) throw (IndexOutOfBoundsException, RuntimeException, std::exception)
    {
        ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );
        if ( GetBroadcastHelper().bDisposed || GetBroadcastHelper().bInDispose )
            throw DisposedException();

        lcl_checkIndex( *m_xData, i_index, *this );

        return m_xData->aImageSets[ i_index ];
    }


    void SAL_CALL AnimatedImagesControlModel::insertImageSet( ::sal_Int32 i_index, const Sequence< OUString >& i_imageURLs ) throw (IndexOutOfBoundsException, RuntimeException, std::exception)
    {
        ::osl::ClearableMutexGuard aGuard( GetMutex() );
        // sanity checks
        if ( GetBroadcastHelper().bDisposed || GetBroadcastHelper().bInDispose )
            throw DisposedException();

        lcl_checkIndex( *m_xData, i_index, *this, true );

        // actual insertion
        m_xData->aImageSets.insert( m_xData->aImageSets.begin() + i_index, i_imageURLs );

        // listener notification
        lcl_notify( aGuard, BrdcstHelper, &XContainerListener::elementInserted, i_index, i_imageURLs, *this );
    }


    void SAL_CALL AnimatedImagesControlModel::replaceImageSet( ::sal_Int32 i_index, const Sequence< OUString >& i_imageURLs ) throw (IndexOutOfBoundsException, RuntimeException, std::exception)
    {
        ::osl::ClearableMutexGuard aGuard( GetMutex() );
        // sanity checks
        if ( GetBroadcastHelper().bDisposed || GetBroadcastHelper().bInDispose )
            throw DisposedException();

        lcl_checkIndex( *m_xData, i_index, *this );

        // actual insertion
        m_xData->aImageSets[ i_index ] = i_imageURLs;

        // listener notification
        lcl_notify( aGuard, BrdcstHelper, &XContainerListener::elementReplaced, i_index, i_imageURLs, *this );
    }


    void SAL_CALL AnimatedImagesControlModel::removeImageSet( ::sal_Int32 i_index ) throw (IndexOutOfBoundsException, RuntimeException, std::exception)
    {
        ::osl::ClearableMutexGuard aGuard( GetMutex() );
        // sanity checks
        if ( GetBroadcastHelper().bDisposed || GetBroadcastHelper().bInDispose )
            throw DisposedException();

        lcl_checkIndex( *m_xData, i_index, *this );

        // actual removal
        ::std::vector< Sequence< OUString > >::iterator removalPos = m_xData->aImageSets.begin() + i_index;
        Sequence< OUString > aRemovedElement( *removalPos );
        m_xData->aImageSets.erase( removalPos );

        // listener notification
        lcl_notify( aGuard, BrdcstHelper, &XContainerListener::elementRemoved, i_index, aRemovedElement, *this );
    }


    void SAL_CALL AnimatedImagesControlModel::addContainerListener( const Reference< XContainerListener >& i_listener ) throw (RuntimeException, std::exception)
    {
        BrdcstHelper.addListener( cppu::UnoType<XContainerListener>::get(), i_listener );
    }


    void SAL_CALL AnimatedImagesControlModel::removeContainerListener( const Reference< XContainerListener >& i_listener ) throw (RuntimeException, std::exception)
    {
        BrdcstHelper.removeListener( cppu::UnoType<XContainerListener>::get(), i_listener );
    }

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
org_openoffice_comp_toolkit_AnimatedImagesControl_get_implementation(
    css::uno::XComponentContext *,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new AnimatedImagesControl());
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
org_openoffice_comp_toolkit_AnimatedImagesControlModel_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new toolkit::AnimatedImagesControlModel(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
