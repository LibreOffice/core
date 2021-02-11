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


#include "Button.hxx"
#include <property.hxx>
#include <services.hxx>

#include <com/sun/star/awt/XVclWindowPeer.hpp>
#include <com/sun/star/form/FormComponentType.hpp>

#include <comphelper/streamsection.hxx>
#include <comphelper/basicio.hxx>
#include <comphelper/property.hxx>
#include <o3tl/any.hxx>
#include <tools/diagnose_ex.h>
#include <tools/debug.hxx>
#include <tools/urlobj.hxx>
#include <vcl/svapp.hxx>
#include <osl/mutex.hxx>

namespace frm
{


using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::form;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;
using ::com::sun::star::frame::XDispatchProviderInterceptor;


//= OButtonModel


OButtonModel::OButtonModel(const Reference<XComponentContext>& _rxFactory)
    :OClickableImageBaseModel( _rxFactory, VCL_CONTROLMODEL_COMMANDBUTTON, FRM_SUN_CONTROL_COMMANDBUTTON )
                                    // use the old control name for compatibility reasons
    ,m_aResetHelper( *this, m_aMutex )
    ,m_eDefaultState( TRISTATE_FALSE )
{
    m_nClassId = FormComponentType::COMMANDBUTTON;
}


Any SAL_CALL OButtonModel::queryAggregation( const Type& _type )
{
    Any aReturn = OClickableImageBaseModel::queryAggregation( _type );
    if ( !aReturn.hasValue() )
        aReturn = OButtonModel_Base::queryInterface( _type );
    return aReturn;
}


Sequence< Type > OButtonModel::_getTypes()
{
    return ::comphelper::concatSequences(
        OClickableImageBaseModel::_getTypes(),
        OButtonModel_Base::getTypes()
    );
}


OButtonModel::OButtonModel( const OButtonModel* _pOriginal, const Reference<XComponentContext>& _rxFactory )
    :OClickableImageBaseModel( _pOriginal, _rxFactory )
    ,m_aResetHelper( *this, m_aMutex )
    ,m_eDefaultState( _pOriginal->m_eDefaultState )
{
    m_nClassId = FormComponentType::COMMANDBUTTON;

    implInitializeImageURL();
}


OButtonModel::~OButtonModel()
{
}


void OButtonModel::describeFixedProperties( Sequence< Property >& _rProps ) const
{
    BEGIN_DESCRIBE_PROPERTIES( 6, OClickableImageBaseModel )
        DECL_PROP1( BUTTONTYPE,             FormButtonType,             BOUND );
        DECL_PROP1( DEFAULT_STATE,          sal_Int16,                  BOUND );
        DECL_PROP1( DISPATCHURLINTERNAL,    sal_Bool,                   BOUND );
        DECL_PROP1( TARGET_URL,             OUString,            BOUND );
        DECL_PROP1( TARGET_FRAME,           OUString,            BOUND );
        DECL_PROP1( TABINDEX,               sal_Int16,                  BOUND );
    END_DESCRIBE_PROPERTIES();
}


IMPLEMENT_DEFAULT_CLONING( OButtonModel )

// XServiceInfo

css::uno::Sequence<OUString>  OButtonModel::getSupportedServiceNames()
{
    css::uno::Sequence<OUString> aSupported = OClickableImageBaseModel::getSupportedServiceNames();
    aSupported.realloc( aSupported.getLength() + 2 );

    OUString* pArray = aSupported.getArray();
    pArray[ aSupported.getLength() - 2 ] = FRM_SUN_COMPONENT_COMMANDBUTTON;
    pArray[ aSupported.getLength() - 1 ] = FRM_COMPONENT_COMMANDBUTTON;

    return aSupported;
}


OUString OButtonModel::getServiceName()
{
    return FRM_COMPONENT_COMMANDBUTTON; // old (non-sun) name for compatibility !
}


void OButtonModel::write(const Reference<XObjectOutputStream>& _rxOutStream)
{
    OClickableImageBaseModel::write(_rxOutStream);

    _rxOutStream->writeShort(0x0003);   // Version

    {
        OStreamSection aSection( _rxOutStream );
            // this will allow readers to skip unknown bytes in their dtor

        _rxOutStream->writeShort( static_cast<sal_uInt16>(m_eButtonType) );

        OUString sTmp = INetURLObject::decode( m_sTargetURL, INetURLObject::DecodeMechanism::Unambiguous);
        _rxOutStream << sTmp;
        _rxOutStream << m_sTargetFrame;
        writeHelpTextCompatibly(_rxOutStream);
        _rxOutStream << isDispatchUrlInternal();
    }
}


void OButtonModel::read(const Reference<XObjectInputStream>& _rxInStream)
{
    OClickableImageBaseModel::read(_rxInStream);

    sal_uInt16 nVersion = _rxInStream->readShort();     // Version
    switch (nVersion)
    {
        case 0x0001:
        {
            m_eButtonType = static_cast<FormButtonType>(_rxInStream->readShort());

            _rxInStream >> m_sTargetURL;
            _rxInStream >> m_sTargetFrame;
        }
        break;

        case 0x0002:
        {
            m_eButtonType = static_cast<FormButtonType>(_rxInStream->readShort());

            _rxInStream >> m_sTargetURL;
            _rxInStream >> m_sTargetFrame;
            readHelpTextCompatibly(_rxInStream);
        }
        break;

        case 0x0003:
        {
            OStreamSection aSection( _rxInStream );
            // this will skip any unknown bytes in its dtor

            // button type
            m_eButtonType = static_cast<FormButtonType>(_rxInStream->readShort());

            // URL
            _rxInStream >> m_sTargetURL;

            // target frame
            _rxInStream >> m_sTargetFrame;

            // help text
            readHelpTextCompatibly(_rxInStream);

            // DispatchInternal
            bool bDispatch;
            _rxInStream >> bDispatch;
            setDispatchUrlInternal(bDispatch);
        }
        break;

        default:
            OSL_FAIL("OButtonModel::read : unknown version !");
            m_eButtonType = FormButtonType_PUSH;
            m_sTargetURL.clear();
            m_sTargetFrame.clear();
            break;
    }
}


void SAL_CALL OButtonModel::disposing()
{
    m_aResetHelper.disposing();
    OClickableImageBaseModel::disposing();
}


void SAL_CALL OButtonModel::reset()
{
    if ( !m_aResetHelper.approveReset() )
        return;

    impl_resetNoBroadcast_nothrow();

    m_aResetHelper.notifyResetted();
}


void SAL_CALL OButtonModel::addResetListener( const Reference< XResetListener >& _listener )
{
    m_aResetHelper.addResetListener( _listener );
}


void SAL_CALL OButtonModel::removeResetListener( const Reference< XResetListener >& _listener )
{
    m_aResetHelper.removeResetListener( _listener );
}


void SAL_CALL OButtonModel::getFastPropertyValue( Any& _rValue, sal_Int32 _nHandle ) const
{
    switch ( _nHandle )
    {
    case PROPERTY_ID_DEFAULT_STATE:
        _rValue <<= static_cast<sal_Int16>(m_eDefaultState);
        break;

    default:
        OClickableImageBaseModel::getFastPropertyValue( _rValue, _nHandle );
        break;
    }
}


void SAL_CALL OButtonModel::setFastPropertyValue_NoBroadcast( sal_Int32 _nHandle, const Any& _rValue )
{
    switch ( _nHandle )
    {
    case PROPERTY_ID_DEFAULT_STATE:
    {
        sal_Int16 nDefaultState = sal_Int16(TRISTATE_FALSE);
        OSL_VERIFY( _rValue >>= nDefaultState );
        m_eDefaultState = static_cast<ToggleState>(nDefaultState);
        impl_resetNoBroadcast_nothrow();
    }
    break;

    default:
        OClickableImageBaseModel::setFastPropertyValue_NoBroadcast( _nHandle, _rValue );
        break;
    }
}


sal_Bool SAL_CALL OButtonModel::convertFastPropertyValue( Any& _rConvertedValue, Any& _rOldValue, sal_Int32 _nHandle, const Any& _rValue )
{
    bool bModified = false;
    switch ( _nHandle )
    {
    case PROPERTY_ID_DEFAULT_STATE:
        bModified = tryPropertyValue( _rConvertedValue, _rOldValue, _rValue, static_cast<sal_Int16>(m_eDefaultState) );
        break;

    default:
        bModified = OClickableImageBaseModel::convertFastPropertyValue( _rConvertedValue, _rOldValue, _nHandle, _rValue );
        break;
    }
    return bModified;
}


Any OButtonModel::getPropertyDefaultByHandle( sal_Int32 _nHandle ) const
{
    Any aDefault;
    switch ( _nHandle )
    {
    case PROPERTY_ID_DEFAULT_STATE:
        aDefault <<= sal_Int16(TRISTATE_FALSE);
        break;

    default:
        aDefault = OClickableImageBaseModel::getPropertyDefaultByHandle( _nHandle );
        break;
    }
    return aDefault;
}


void OButtonModel::impl_resetNoBroadcast_nothrow()
{
    try
    {
        setPropertyValue( PROPERTY_STATE, getPropertyValue( PROPERTY_DEFAULT_STATE ) );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("forms.component");
    }
}


// OButtonControl


Sequence<Type> OButtonControl::_getTypes()
{
    return ::comphelper::concatSequences(
        OButtonControl_BASE::getTypes(),
        OClickableImageBaseControl::_getTypes(),
        OFormNavigationHelper::getTypes()
    );
}


css::uno::Sequence<OUString>  OButtonControl::getSupportedServiceNames()
{
    css::uno::Sequence<OUString> aSupported = OClickableImageBaseControl::getSupportedServiceNames();
    aSupported.realloc(aSupported.getLength() + 2);

    OUString*pArray = aSupported.getArray();
    pArray[aSupported.getLength()-2] = FRM_SUN_CONTROL_COMMANDBUTTON;
    pArray[aSupported.getLength()-1] = STARDIV_ONE_FORM_CONTROL_COMMANDBUTTON;
    return aSupported;
}


OButtonControl::OButtonControl(const Reference<XComponentContext>& _rxFactory)
                 :OClickableImageBaseControl(_rxFactory, VCL_CONTROL_COMMANDBUTTON)
                 ,OFormNavigationHelper( _rxFactory )
                 ,m_nClickEvent( nullptr )
                 ,m_nTargetUrlFeatureId( -1 )
                 ,m_bEnabledByPropertyValue( false )
{
    osl_atomic_increment(&m_refCount);
    {
        // Register as ActionListener
        Reference<XButton>  xButton;
        query_aggregation( m_xAggregate, xButton);
        if (xButton.is())
            xButton->addActionListener(this);
    }
    // For Listener: refcount at one
    osl_atomic_decrement(&m_refCount);
}


OButtonControl::~OButtonControl()
{
    if (m_nClickEvent)
        Application::RemoveUserEvent(m_nClickEvent);
}

// UNO binding

Any SAL_CALL OButtonControl::queryAggregation(const Type& _rType)
{
    // if asked for the XTypeProvider, don't let OButtonControl_BASE do this
    Any aReturn;
    if ( !_rType.equals( cppu::UnoType<XTypeProvider>::get() ) )
        aReturn = OButtonControl_BASE::queryInterface( _rType );

    if ( !aReturn.hasValue() )
        aReturn = OClickableImageBaseControl::queryAggregation( _rType );

    if ( !aReturn.hasValue() )
        aReturn = OFormNavigationHelper::queryInterface( _rType );

    return aReturn;
}


void SAL_CALL OButtonControl::disposing()
{
    startOrStopModelPropertyListening( false );

    OClickableImageBaseControl::disposing();
    OFormNavigationHelper::dispose();
}


void SAL_CALL OButtonControl::disposing( const EventObject& _rSource )
{
    OControl::disposing( _rSource );
    OFormNavigationHelper::disposing( _rSource );
}

// ActionListener

void OButtonControl::actionPerformed(const ActionEvent& /*rEvent*/)
{
    // Asynchronous for css::util::URL-Button
    ImplSVEvent * n = Application::PostUserEvent( LINK(this, OButtonControl, OnClick) );
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        m_nClickEvent = n;
    }
}


IMPL_LINK_NOARG(OButtonControl, OnClick, void*, void)
{
    ::osl::ClearableMutexGuard aGuard( m_aMutex );
    m_nClickEvent = nullptr;

    if (m_aApproveActionListeners.getLength())
    {
        // if there are listeners, start the action in an own thread, to not allow
        // them to block us here (we're in the application's main thread)
        getImageProducerThread()->addEvent();
    }
    else
    {
        // Else, don't. We then must not notify the Listeners in any case,
        // not even if added later on.
        aGuard.clear();

        // recognize the button type
        Reference<XPropertySet>  xSet(getModel(), UNO_QUERY);
        if (!xSet.is())
            return;

        if (FormButtonType_PUSH == *o3tl::doAccess<FormButtonType>(xSet->getPropertyValue(PROPERTY_BUTTONTYPE)))
        {
            // notify the action listeners for a push button
            ::comphelper::OInterfaceIteratorHelper2 aIter(m_aActionListeners);
            ActionEvent aEvt(static_cast<XWeak*>(this), m_aActionCommand);
            while(aIter.hasMoreElements() )
            {
                // catch exceptions
                // and catch them on a per-listener basis - if one listener fails, the others still need
                // to get notified
                try
                {
                    static_cast< XActionListener* >( aIter.next() )->actionPerformed(aEvt);
                }
#ifdef DBG_UTIL
                catch( const RuntimeException& )
                {
                    // silence this
                }
#endif
                catch( const Exception& )
                {
                    TOOLS_WARN_EXCEPTION( "forms.component", "OButtonControl::OnClick: caught an exception other than RuntimeException!" );
                }
            }
        }
        else
            actionPerformed_Impl( false, css::awt::MouseEvent() );
    }
}


void OButtonControl::actionPerformed_Impl( bool _bNotifyListener, const css::awt::MouseEvent& _rEvt )
{
    {
        sal_Int16 nFeatureId = -1;
        {
            ::osl::MutexGuard aGuard( m_aMutex );
            nFeatureId = m_nTargetUrlFeatureId;
        }

        if ( nFeatureId != -1 )
        {
            if ( !approveAction() )
                return;

            SolarMutexGuard aGuard;
            dispatch( nFeatureId );
            return;
        }
    }

    OClickableImageBaseControl::actionPerformed_Impl( _bNotifyListener, _rEvt );
}

// XButton

void OButtonControl::setLabel(const OUString& Label)
{
    Reference<XButton>  xButton;
    query_aggregation( m_xAggregate, xButton );
    if (xButton.is())
        xButton->setLabel(Label);
}


void SAL_CALL OButtonControl::setActionCommand(const OUString& _rCommand)
{
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        m_aActionCommand = _rCommand;
    }

    Reference<XButton>  xButton;
    query_aggregation( m_xAggregate, xButton);
    if (xButton.is())
        xButton->setActionCommand(_rCommand);
}


void SAL_CALL OButtonControl::addActionListener(const Reference<XActionListener>& _rxListener)
{
    m_aActionListeners.addInterface(_rxListener);
}


void SAL_CALL OButtonControl::removeActionListener(const Reference<XActionListener>& _rxListener)
{
    m_aActionListeners.removeInterface(_rxListener);
}

namespace {

class DoPropertyListening
{
private:
    Reference< XPropertySet >               m_xProps;
    Reference< XPropertyChangeListener >    m_xListener;
    bool                                    m_bStartListening;

public:
    DoPropertyListening(
        const Reference< XInterface >& _rxComponent,
        const Reference< XPropertyChangeListener >& _rxListener,
        bool _bStart
    );

    void    handleListening( const OUString& _rPropertyName );
};

}

DoPropertyListening::DoPropertyListening(
        const Reference< XInterface >& _rxComponent, const Reference< XPropertyChangeListener >& _rxListener,
        bool _bStart )
    :m_xProps( _rxComponent, UNO_QUERY )
    ,m_xListener( _rxListener )
    ,m_bStartListening( _bStart )
{
    DBG_ASSERT( m_xProps.is() || !_rxComponent.is(), "DoPropertyListening::DoPropertyListening: valid component, but no property set!" );
    DBG_ASSERT( m_xListener.is(), "DoPropertyListening::DoPropertyListening: invalid listener!" );
}


void DoPropertyListening::handleListening( const OUString& _rPropertyName )
{
    if ( m_xProps.is() )
    {
        if ( m_bStartListening )
            m_xProps->addPropertyChangeListener( _rPropertyName, m_xListener );
        else
            m_xProps->removePropertyChangeListener( _rPropertyName, m_xListener );
    }
}


void OButtonControl::startOrStopModelPropertyListening( bool _bStart )
{
    DoPropertyListening aListeningHandler( getModel(), this, _bStart );
    aListeningHandler.handleListening( PROPERTY_TARGET_URL );
    aListeningHandler.handleListening( PROPERTY_BUTTONTYPE );
    aListeningHandler.handleListening( PROPERTY_ENABLED );
}


sal_Bool SAL_CALL OButtonControl::setModel( const Reference< XControlModel >& _rxModel )
{
    startOrStopModelPropertyListening( false );
    bool bResult = OClickableImageBaseControl::setModel( _rxModel );
    startOrStopModelPropertyListening( true );

    m_bEnabledByPropertyValue = true;
    Reference< XPropertySet > xModelProps( _rxModel, UNO_QUERY );
    if ( xModelProps.is() )
        xModelProps->getPropertyValue( PROPERTY_ENABLED ) >>= m_bEnabledByPropertyValue;

    modelFeatureUrlPotentiallyChanged( );

    return bResult;
}


void OButtonControl::modelFeatureUrlPotentiallyChanged( )
{
    sal_Int16 nOldUrlFeatureId = m_nTargetUrlFeatureId;

    // Do we have another TargetURL now? If so, we need to update our dispatches
    m_nTargetUrlFeatureId = getModelUrlFeatureId( );
    if ( nOldUrlFeatureId != m_nTargetUrlFeatureId )
    {
        invalidateSupportedFeaturesSet();
        if ( !isDesignMode() )
            updateDispatches( );
    }
}


void SAL_CALL OButtonControl::propertyChange( const PropertyChangeEvent& _rEvent )
{
    if  (   _rEvent.PropertyName == PROPERTY_TARGET_URL
        ||  _rEvent.PropertyName == PROPERTY_BUTTONTYPE
        )
    {
        modelFeatureUrlPotentiallyChanged( );
    }
    else if ( _rEvent.PropertyName == PROPERTY_ENABLED )
    {
        _rEvent.NewValue >>= m_bEnabledByPropertyValue;
    }
}


namespace
{
    bool isFormControllerURL( const OUString& _rURL )
    {
        return  ( _rURL.getLength() > RTL_CONSTASCII_LENGTH( ".uno:FormController/" ) )
            &&  ( _rURL.startsWith( ".uno:FormController/" ) );
    }
}


sal_Int16 OButtonControl::getModelUrlFeatureId( ) const
{
    sal_Int16 nFeatureId = -1;

    // some URL related properties of the model
    OUString sUrl;
    FormButtonType eButtonType = FormButtonType_PUSH;

    Reference< XPropertySet > xModelProps( const_cast< OButtonControl* >( this )->getModel(), UNO_QUERY );
    if ( xModelProps.is() )
    {
        xModelProps->getPropertyValue( PROPERTY_TARGET_URL ) >>= sUrl;
        xModelProps->getPropertyValue( PROPERTY_BUTTONTYPE ) >>= eButtonType;
    }

    // are we a URL button?
    if ( eButtonType == FormButtonType_URL )
    {
        // is it a feature URL?
        if ( isFormControllerURL( sUrl ) )
        {
            nFeatureId = OFormNavigationMapper::getFeatureId( sUrl );
        }
    }

    return nFeatureId;
}


void SAL_CALL OButtonControl::setDesignMode( sal_Bool _bOn )
{
    OClickableImageBaseControl::setDesignMode( _bOn  );

    if ( _bOn )
        disconnectDispatchers();
    else
        connectDispatchers();
        // this will connect if not already connected and just update else
}


void OButtonControl::getSupportedFeatures( ::std::vector< sal_Int16 >& /* [out] */ _rFeatureIds )
{
    if ( -1 != m_nTargetUrlFeatureId )
        _rFeatureIds.push_back( m_nTargetUrlFeatureId );
}


void OButtonControl::featureStateChanged( sal_Int16 _nFeatureId, bool _bEnabled )
{
    if ( _nFeatureId == m_nTargetUrlFeatureId )
    {
        // enable or disable our peer, according to the new state
        Reference< XVclWindowPeer > xPeer( getPeer(), UNO_QUERY );
        if ( xPeer.is() )
            xPeer->setProperty( PROPERTY_ENABLED, makeAny( m_bEnabledByPropertyValue && _bEnabled ) );
            // if we're disabled according to our model's property, then
            // we don't care for the feature state, but *are* disabled.
            // If the model's property states that we're enabled, then we *do*
            // care for the feature state
    }

    // base class
    OFormNavigationHelper::featureStateChanged( _nFeatureId, _bEnabled );
}


void OButtonControl::allFeatureStatesChanged( )
{
    if ( -1 != m_nTargetUrlFeatureId )
        // we have only one supported feature, so simulate it has changed ...
        featureStateChanged( m_nTargetUrlFeatureId, isEnabled( m_nTargetUrlFeatureId ) );

    // base class
    OFormNavigationHelper::allFeatureStatesChanged( );
}


bool OButtonControl::isEnabled( sal_Int16 _nFeatureId ) const
{
    if ( const_cast< OButtonControl* >( this )->isDesignMode() )
        // TODO: the model property?
       return true;

    return OFormNavigationHelper::isEnabled( _nFeatureId );
}


void SAL_CALL OButtonControl::registerDispatchProviderInterceptor( const Reference< XDispatchProviderInterceptor >& _rxInterceptor )
{
    OClickableImageBaseControl::registerDispatchProviderInterceptor( _rxInterceptor );
    OFormNavigationHelper::registerDispatchProviderInterceptor( _rxInterceptor );
}


void SAL_CALL OButtonControl::releaseDispatchProviderInterceptor( const Reference< XDispatchProviderInterceptor >& _rxInterceptor )
{
    OClickableImageBaseControl::releaseDispatchProviderInterceptor( _rxInterceptor );
    OFormNavigationHelper::releaseDispatchProviderInterceptor( _rxInterceptor );
}

}   // namespace frm

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_form_OButtonModel_get_implementation(css::uno::XComponentContext* component,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new frm::OButtonModel(component));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_form_OButtonControl_get_implementation(css::uno::XComponentContext* component,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new frm::OButtonControl(component));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
