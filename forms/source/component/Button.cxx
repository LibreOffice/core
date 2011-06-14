/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_forms.hxx"

#include "Button.hxx"

#include <com/sun/star/awt/XVclWindowPeer.hpp>

#include <comphelper/streamsection.hxx>
#include <comphelper/basicio.hxx>
#include <tools/diagnose_ex.h>
#include <tools/debug.hxx>
#include <tools/urlobj.hxx>
#include <vcl/svapp.hxx>
#include <osl/mutex.hxx>

//.........................................................................
namespace frm
{
//.........................................................................

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::form;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;
using ::com::sun::star::frame::XDispatchProviderInterceptor;

//==================================================================
//= OButtonModel
//==================================================================
DBG_NAME(OButtonModel)
//------------------------------------------------------------------
InterfaceRef SAL_CALL OButtonModel_CreateInstance(const Reference<XMultiServiceFactory>& _rxFactory)
{
    return *(new OButtonModel(_rxFactory));
}

//------------------------------------------------------------------
OButtonModel::OButtonModel(const Reference<XMultiServiceFactory>& _rxFactory)
    :OClickableImageBaseModel( _rxFactory, VCL_CONTROLMODEL_COMMANDBUTTON, FRM_SUN_CONTROL_COMMANDBUTTON )
                                    // use the old control name for compatibility reasons
    ,m_aResetHelper( *this, m_aMutex )
    ,m_eDefaultState( STATE_NOCHECK )
{
    DBG_CTOR( OButtonModel, NULL );
    m_nClassId = FormComponentType::COMMANDBUTTON;
}

//------------------------------------------------------------------
Any SAL_CALL OButtonModel::queryAggregation( const Type& _type ) throw(RuntimeException)
{
    Any aReturn = OClickableImageBaseModel::queryAggregation( _type );
    if ( !aReturn.hasValue() )
        aReturn = OButtonModel_Base::queryInterface( _type );
    return aReturn;
}

//------------------------------------------------------------------
Sequence< Type > OButtonModel::_getTypes()
{
    return ::comphelper::concatSequences(
        OClickableImageBaseModel::_getTypes(),
        OButtonModel_Base::getTypes()
    );
}

//------------------------------------------------------------------
OButtonModel::OButtonModel( const OButtonModel* _pOriginal, const Reference<XMultiServiceFactory>& _rxFactory )
    :OClickableImageBaseModel( _pOriginal, _rxFactory )
    ,m_aResetHelper( *this, m_aMutex )
    ,m_eDefaultState( _pOriginal->m_eDefaultState )
{
    DBG_CTOR( OButtonModel, NULL );
    m_nClassId = FormComponentType::COMMANDBUTTON;

    implInitializeImageURL();
}

//------------------------------------------------------------------------------
OButtonModel::~OButtonModel()
{
    DBG_DTOR(OButtonModel, NULL);
}

//------------------------------------------------------------------------------
void OButtonModel::describeFixedProperties( Sequence< Property >& _rProps ) const
{
    BEGIN_DESCRIBE_PROPERTIES( 6, OClickableImageBaseModel )
        DECL_PROP1( BUTTONTYPE,             FormButtonType,             BOUND );
        DECL_PROP1( DEFAULT_STATE,          sal_Int16,                  BOUND );
        DECL_PROP1( DISPATCHURLINTERNAL,    sal_Bool,                   BOUND );
        DECL_PROP1( TARGET_URL,             ::rtl::OUString,            BOUND );
        DECL_PROP1( TARGET_FRAME,           ::rtl::OUString,            BOUND );
        DECL_PROP1( TABINDEX,               sal_Int16,                  BOUND );
    END_DESCRIBE_PROPERTIES();
}

//------------------------------------------------------------------------------
IMPLEMENT_DEFAULT_CLONING( OButtonModel )

// XServiceInfo
//------------------------------------------------------------------------------
StringSequence  OButtonModel::getSupportedServiceNames() throw()
{
    StringSequence aSupported = OClickableImageBaseModel::getSupportedServiceNames();
    aSupported.realloc( aSupported.getLength() + 1 );

    ::rtl::OUString* pArray = aSupported.getArray();
    pArray[ aSupported.getLength() - 1 ] = FRM_SUN_COMPONENT_COMMANDBUTTON;

    return aSupported;
}

//------------------------------------------------------------------------------
::rtl::OUString OButtonModel::getServiceName() throw ( ::com::sun::star::uno::RuntimeException)
{
    return FRM_COMPONENT_COMMANDBUTTON; // old (non-sun) name for compatibility !
}

//------------------------------------------------------------------------------
void OButtonModel::write(const Reference<XObjectOutputStream>& _rxOutStream) throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException)
{
    OClickableImageBaseModel::write(_rxOutStream);

    _rxOutStream->writeShort(0x0003);   // Version

    {
        OStreamSection aSection( _rxOutStream.get() );
            // this will allow readers to skip unknown bytes in their dtor

        _rxOutStream->writeShort( (sal_uInt16)m_eButtonType );

        ::rtl::OUString sTmp = INetURLObject::decode( m_sTargetURL, '%', INetURLObject::DECODE_UNAMBIGUOUS);
        _rxOutStream << sTmp;
        _rxOutStream << m_sTargetFrame;
        writeHelpTextCompatibly(_rxOutStream);
        _rxOutStream << isDispatchUrlInternal();
    }
}

//------------------------------------------------------------------------------
void OButtonModel::read(const Reference<XObjectInputStream>& _rxInStream) throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException)
{
    OClickableImageBaseModel::read(_rxInStream);

    sal_uInt16 nVersion = _rxInStream->readShort();     // Version
    switch (nVersion)
    {
        case 0x0001:
        {
            m_eButtonType = (FormButtonType)_rxInStream->readShort();

            _rxInStream >> m_sTargetURL;
            _rxInStream >> m_sTargetFrame;
        }
        break;

        case 0x0002:
        {
            m_eButtonType = (FormButtonType)_rxInStream->readShort();

            _rxInStream >> m_sTargetURL;
            _rxInStream >> m_sTargetFrame;
            readHelpTextCompatibly(_rxInStream);
        }
        break;

        case 0x0003:
        {
            OStreamSection aSection( _rxInStream.get() );
                // this will skip any unknown bytes in it's dtor

            // button type
            m_eButtonType = (FormButtonType)_rxInStream->readShort();

            // URL
            _rxInStream >> m_sTargetURL;

            // target frame
            _rxInStream >> m_sTargetFrame;

            // help text
            readHelpTextCompatibly(_rxInStream);

            // DispatchInternal
            sal_Bool bDispath;
            _rxInStream >> bDispath;
            setDispatchUrlInternal(bDispath);
        }
        break;

        default:
            OSL_FAIL("OButtonModel::read : unknown version !");
            m_eButtonType = FormButtonType_PUSH;
            m_sTargetURL = ::rtl::OUString();
            m_sTargetFrame = ::rtl::OUString();
            break;
    }
}

//--------------------------------------------------------------------
void SAL_CALL OButtonModel::disposing()
{
    m_aResetHelper.disposing();
    OClickableImageBaseModel::disposing();
}

//--------------------------------------------------------------------
void SAL_CALL OButtonModel::reset() throw (RuntimeException)
{
    if ( !m_aResetHelper.approveReset() )
        return;

    impl_resetNoBroadcast_nothrow();

    m_aResetHelper.notifyResetted();
}

//--------------------------------------------------------------------
void SAL_CALL OButtonModel::addResetListener( const Reference< XResetListener >& _listener ) throw (RuntimeException)
{
    m_aResetHelper.addResetListener( _listener );
}

//--------------------------------------------------------------------
void SAL_CALL OButtonModel::removeResetListener( const Reference< XResetListener >& _listener ) throw (RuntimeException)
{
    m_aResetHelper.removeResetListener( _listener );
}

//--------------------------------------------------------------------
void SAL_CALL OButtonModel::getFastPropertyValue( Any& _rValue, sal_Int32 _nHandle ) const
{
    switch ( _nHandle )
    {
    case PROPERTY_ID_DEFAULT_STATE:
        _rValue <<= (sal_Int16)m_eDefaultState;
        break;

    default:
        OClickableImageBaseModel::getFastPropertyValue( _rValue, _nHandle );
        break;
    }
}

//--------------------------------------------------------------------
void SAL_CALL OButtonModel::setFastPropertyValue_NoBroadcast( sal_Int32 _nHandle, const Any& _rValue ) throw (Exception)
{
    switch ( _nHandle )
    {
    case PROPERTY_ID_DEFAULT_STATE:
    {
        sal_Int16 nDefaultState( (sal_Int16)STATE_NOCHECK );
        OSL_VERIFY( _rValue >>= nDefaultState );
        m_eDefaultState = (ToggleState)nDefaultState;
        impl_resetNoBroadcast_nothrow();
    }
    break;

    default:
        OClickableImageBaseModel::setFastPropertyValue_NoBroadcast( _nHandle, _rValue );
        break;
    }
}

//--------------------------------------------------------------------
sal_Bool SAL_CALL OButtonModel::convertFastPropertyValue( Any& _rConvertedValue, Any& _rOldValue, sal_Int32 _nHandle, const Any& _rValue ) throw (IllegalArgumentException)
{
    sal_Bool bModified = sal_False;
    switch ( _nHandle )
    {
    case PROPERTY_ID_DEFAULT_STATE:
        bModified = tryPropertyValue( _rConvertedValue, _rOldValue, _rValue, (sal_Int16)m_eDefaultState );
        break;

    default:
        bModified = OClickableImageBaseModel::convertFastPropertyValue( _rConvertedValue, _rOldValue, _nHandle, _rValue );
        break;
    }
    return bModified;
}

//--------------------------------------------------------------------
Any OButtonModel::getPropertyDefaultByHandle( sal_Int32 _nHandle ) const
{
    Any aDefault;
    switch ( _nHandle )
    {
    case PROPERTY_ID_DEFAULT_STATE:
        aDefault <<= (sal_Int16)STATE_NOCHECK;
        break;

    default:
        aDefault = OClickableImageBaseModel::getPropertyDefaultByHandle( _nHandle );
        break;
    }
    return aDefault;
}

//--------------------------------------------------------------------
void OButtonModel::impl_resetNoBroadcast_nothrow()
{
    try
    {
        setPropertyValue( PROPERTY_STATE, getPropertyValue( PROPERTY_DEFAULT_STATE ) );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

//==================================================================
// OButtonControl
//==================================================================
//------------------------------------------------------------------
InterfaceRef SAL_CALL OButtonControl_CreateInstance(const Reference<XMultiServiceFactory>& _rxFactory)
{
    return *(new OButtonControl(_rxFactory));
}

//------------------------------------------------------------------------------
Sequence<Type> OButtonControl::_getTypes()
{
    return ::comphelper::concatSequences(
        OButtonControl_BASE::getTypes(),
        OClickableImageBaseControl::_getTypes(),
        OFormNavigationHelper::getTypes()
    );
}

//------------------------------------------------------------------------------
StringSequence  OButtonControl::getSupportedServiceNames() throw()
{
    StringSequence aSupported = OClickableImageBaseControl::getSupportedServiceNames();
    aSupported.realloc(aSupported.getLength() + 1);

    ::rtl::OUString*pArray = aSupported.getArray();
    pArray[aSupported.getLength()-1] = FRM_SUN_CONTROL_COMMANDBUTTON;
    return aSupported;
}

//------------------------------------------------------------------------------
OButtonControl::OButtonControl(const Reference<XMultiServiceFactory>& _rxFactory)
                 :OClickableImageBaseControl(_rxFactory, VCL_CONTROL_COMMANDBUTTON)
                 ,OFormNavigationHelper( _rxFactory )
                 ,m_nClickEvent( 0 )
                 ,m_nTargetUrlFeatureId( -1 )
                 ,m_bEnabledByPropertyValue( sal_False )
{
    increment(m_refCount);
    {
        // als ActionListener anmelden
        Reference<XButton>  xButton;
        query_aggregation( m_xAggregate, xButton);
        if (xButton.is())
            xButton->addActionListener(this);
    }
    // Refcount bei 1 fuer Listener
    decrement(m_refCount);
}

//------------------------------------------------------------------------------
OButtonControl::~OButtonControl()
{
    if (m_nClickEvent)
        Application::RemoveUserEvent(m_nClickEvent);
}

// UNO Anbindung
//------------------------------------------------------------------------------
Any SAL_CALL OButtonControl::queryAggregation(const Type& _rType) throw (RuntimeException)
{
    // if asked for the XTypeProvider, don't let OButtonControl_BASE do this
    Any aReturn;
    if ( !_rType.equals( ::getCppuType( static_cast< Reference< XTypeProvider >* >( NULL ) ) ) )
        aReturn = OButtonControl_BASE::queryInterface( _rType );

    if ( !aReturn.hasValue() )
        aReturn = OClickableImageBaseControl::queryAggregation( _rType );

    if ( !aReturn.hasValue() )
        aReturn = OFormNavigationHelper::queryInterface( _rType );

    return aReturn;
}

//------------------------------------------------------------------------------
void SAL_CALL OButtonControl::disposing()
{
    startOrStopModelPropertyListening( false );

    OClickableImageBaseControl::disposing();
    OFormNavigationHelper::dispose();
}

//------------------------------------------------------------------------------
void SAL_CALL OButtonControl::disposing( const EventObject& _rSource ) throw( RuntimeException )
{
    OControl::disposing( _rSource );
    OFormNavigationHelper::disposing( _rSource );
}

// ActionListener
//------------------------------------------------------------------------------
void OButtonControl::actionPerformed(const ActionEvent& /*rEvent*/) throw ( ::com::sun::star::uno::RuntimeException)
{
    // Asynchron fuer starutil::URL-Button
    sal_uLong n = Application::PostUserEvent( LINK(this, OButtonControl,OnClick) );
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        m_nClickEvent = n;
    }
}

//------------------------------------------------------------------------------
IMPL_LINK( OButtonControl, OnClick, void*, EMPTYARG )
{
    ::osl::ClearableMutexGuard aGuard( m_aMutex );
    m_nClickEvent = 0;

    if (m_aApproveActionListeners.getLength())
    {
        // if there are listeners, start the action in an own thread, to not allow
        // them to block us here (we're in the application's main thread)
        getImageProducerThread()->addEvent();
    }
    else
    {
        // Sonst nicht. Dann darf man aber auf keinen Fal die Listener
        // benachrichtigen, auch dann nicht, wenn er spaeter hinzukommt.
        aGuard.clear();

        // recognize the button type
        Reference<XPropertySet>  xSet(getModel(), UNO_QUERY);
        if (!xSet.is())
            return 0L;

        if (FormButtonType_PUSH == *(FormButtonType*)xSet->getPropertyValue(PROPERTY_BUTTONTYPE).getValue())
        {
            // notify the action listeners for a push button
            ::cppu::OInterfaceIteratorHelper aIter(m_aActionListeners);
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
                    // silent this
                }
#endif
                catch( const Exception& )
                {
                    OSL_FAIL( "OButtonControl::OnClick: caught a exception other than RuntimeException!" );
                }
            }
        }
        else
            actionPerformed_Impl( sal_False, ::com::sun::star::awt::MouseEvent() );
    }
    return 0L;
}

//------------------------------------------------------------------------------
void OButtonControl::actionPerformed_Impl( sal_Bool _bNotifyListener, const ::com::sun::star::awt::MouseEvent& _rEvt )
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
//------------------------------------------------------------------------------
void OButtonControl::setLabel(const ::rtl::OUString& Label) throw( RuntimeException )
{
    Reference<XButton>  xButton;
    query_aggregation( m_xAggregate, xButton );
    if (xButton.is())
        xButton->setLabel(Label);
}

//------------------------------------------------------------------------------
void SAL_CALL OButtonControl::setActionCommand(const ::rtl::OUString& _rCommand) throw( RuntimeException )
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

//------------------------------------------------------------------------------
void SAL_CALL OButtonControl::addActionListener(const Reference<XActionListener>& _rxListener) throw( RuntimeException )
{
    m_aActionListeners.addInterface(_rxListener);
}

//------------------------------------------------------------------------------
void SAL_CALL OButtonControl::removeActionListener(const Reference<XActionListener>& _rxListener) throw( RuntimeException )
{
    m_aActionListeners.removeInterface(_rxListener);
}

//------------------------------------------------------------------------------
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

    void    handleListening( const ::rtl::OUString& _rPropertyName );
};

//..............................................................................
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

//..............................................................................
void DoPropertyListening::handleListening( const ::rtl::OUString& _rPropertyName )
{
    if ( m_xProps.is() )
    {
        if ( m_bStartListening )
            m_xProps->addPropertyChangeListener( _rPropertyName, m_xListener );
        else
            m_xProps->removePropertyChangeListener( _rPropertyName, m_xListener );
    }
}

//------------------------------------------------------------------------------
void OButtonControl::startOrStopModelPropertyListening( bool _bStart )
{
    DoPropertyListening aListeningHandler( getModel(), this, _bStart );
    aListeningHandler.handleListening( PROPERTY_TARGET_URL );
    aListeningHandler.handleListening( PROPERTY_BUTTONTYPE );
    aListeningHandler.handleListening( PROPERTY_ENABLED );
}

//------------------------------------------------------------------------------
sal_Bool SAL_CALL OButtonControl::setModel( const Reference< XControlModel >& _rxModel ) throw ( RuntimeException )
{
    startOrStopModelPropertyListening( false );
    sal_Bool bResult = OClickableImageBaseControl::setModel( _rxModel );
    startOrStopModelPropertyListening( true );

    m_bEnabledByPropertyValue = sal_True;
    Reference< XPropertySet > xModelProps( _rxModel, UNO_QUERY );
    if ( xModelProps.is() )
        xModelProps->getPropertyValue( PROPERTY_ENABLED ) >>= m_bEnabledByPropertyValue;

    modelFeatureUrlPotentiallyChanged( );

    return bResult;
}

//------------------------------------------------------------------------------
void OButtonControl::modelFeatureUrlPotentiallyChanged( )
{
    sal_Int16 nOldUrlFeatureId = m_nTargetUrlFeatureId;

    // doe we have another TargetURL now? If so, we need to update our dispatches
    m_nTargetUrlFeatureId = getModelUrlFeatureId( );
    if ( nOldUrlFeatureId != m_nTargetUrlFeatureId )
    {
        invalidateSupportedFeaturesSet();
        if ( !isDesignMode() )
            updateDispatches( );
    }
}

//------------------------------------------------------------------------------
void SAL_CALL OButtonControl::propertyChange( const PropertyChangeEvent& _rEvent ) throw ( RuntimeException )
{
    if  (   _rEvent.PropertyName.equals( PROPERTY_TARGET_URL )
        ||  _rEvent.PropertyName.equals( PROPERTY_BUTTONTYPE )
        )
    {
        modelFeatureUrlPotentiallyChanged( );
    }
    else if ( _rEvent.PropertyName.equals( PROPERTY_ENABLED ) )
    {
        _rEvent.NewValue >>= m_bEnabledByPropertyValue;
    }
}

//------------------------------------------------------------------------------
namespace
{
    bool isFormControllerURL( const ::rtl::OUString& _rURL )
    {
        const sal_Int32 nPrefixLen = URL_CONTROLLER_PREFIX.length;
        return  ( _rURL.getLength() > nPrefixLen )
            &&  ( _rURL.compareToAscii( URL_CONTROLLER_PREFIX, nPrefixLen ) == 0 );
    }
}

//------------------------------------------------------------------------------
sal_Int16 OButtonControl::getModelUrlFeatureId( ) const
{
    sal_Int16 nFeatureId = -1;

    // some URL related properties of the model
    ::rtl::OUString sUrl;
    FormButtonType eButtonType = FormButtonType_PUSH;

    Reference< XPropertySet > xModelProps( const_cast< OButtonControl* >( this )->getModel(), UNO_QUERY );
    if ( xModelProps.is() )
    {
        xModelProps->getPropertyValue( PROPERTY_TARGET_URL ) >>= sUrl;
        xModelProps->getPropertyValue( PROPERTY_BUTTONTYPE ) >>= eButtonType;
    }

    // are we an URL button?
    if ( eButtonType == FormButtonType_URL )
    {
        // is it a feature URL?
        if ( isFormControllerURL( sUrl ) )
        {
            OFormNavigationMapper aMapper( m_aContext.getLegacyServiceFactory() );
            nFeatureId = aMapper.getFeatureId( sUrl );
        }
    }

    return nFeatureId;
}

//------------------------------------------------------------------
void SAL_CALL OButtonControl::setDesignMode( sal_Bool _bOn ) throw( RuntimeException )
{
    OClickableImageBaseControl::setDesignMode( _bOn  );

    if ( _bOn )
        disconnectDispatchers();
    else
        connectDispatchers();
        // this will connect if not already connected and just update else
}

//------------------------------------------------------------------------------
void OButtonControl::getSupportedFeatures( ::std::vector< sal_Int16 >& /* [out] */ _rFeatureIds )
{
    if ( -1 != m_nTargetUrlFeatureId )
        _rFeatureIds.push_back( m_nTargetUrlFeatureId );
}

//------------------------------------------------------------------
void OButtonControl::featureStateChanged( sal_Int16 _nFeatureId, sal_Bool _bEnabled )
{
    if ( _nFeatureId == m_nTargetUrlFeatureId )
    {
        // enable or disable our peer, according to the new state
        Reference< XVclWindowPeer > xPeer( getPeer(), UNO_QUERY );
        if ( xPeer.is() )
            xPeer->setProperty( PROPERTY_ENABLED, makeAny( m_bEnabledByPropertyValue ? _bEnabled : sal_False ) );
            // if we're disabled according to our model's property, then
            // we don't care for the feature state, but *are* disabled.
            // If the model's property states that we're enabled, then we *do*
            // care for the feature state
    }

    // base class
    OFormNavigationHelper::featureStateChanged( _nFeatureId, _bEnabled );
}

//------------------------------------------------------------------
void OButtonControl::allFeatureStatesChanged( )
{
    if ( -1 != m_nTargetUrlFeatureId )
        // we have only one supported feature, so simulate it has changed ...
        featureStateChanged( m_nTargetUrlFeatureId, isEnabled( m_nTargetUrlFeatureId ) );

    // base class
    OFormNavigationHelper::allFeatureStatesChanged( );
}

//------------------------------------------------------------------
bool OButtonControl::isEnabled( sal_Int16 _nFeatureId ) const
{
    if ( const_cast< OButtonControl* >( this )->isDesignMode() )
        // TODO: the model property?
       return true;

    return OFormNavigationHelper::isEnabled( _nFeatureId );
}

//--------------------------------------------------------------------
void SAL_CALL OButtonControl::registerDispatchProviderInterceptor( const Reference< XDispatchProviderInterceptor >& _rxInterceptor ) throw (RuntimeException)
{
    OClickableImageBaseControl::registerDispatchProviderInterceptor( _rxInterceptor );
    OFormNavigationHelper::registerDispatchProviderInterceptor( _rxInterceptor );
}

//--------------------------------------------------------------------
void SAL_CALL OButtonControl::releaseDispatchProviderInterceptor( const Reference< XDispatchProviderInterceptor >& _rxInterceptor ) throw (RuntimeException)
{
    OClickableImageBaseControl::releaseDispatchProviderInterceptor( _rxInterceptor );
    OFormNavigationHelper::releaseDispatchProviderInterceptor( _rxInterceptor );
}

//.........................................................................
}   // namespace frm
//.........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
