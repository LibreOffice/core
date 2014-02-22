/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */
#include "componenttools.hxx"
#include "FormComponent.hxx"
#include "frm_resource.hrc"
#include "frm_resource.hxx"
#include "property.hrc"
#include "services.hxx"

#include <com/sun/star/awt/XTextComponent.hpp>
#include <com/sun/star/awt/XVclWindowPeer.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/form/XForm.hpp>
#include <com/sun/star/form/XLoadable.hpp>
#include <com/sun/star/io/XMarkableStream.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/sdb/XRowSetChangeBroadcaster.hpp>
#include <com/sun/star/sdb/XRowSetSupplier.hpp>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/util/XModifyBroadcaster.hpp>

#include <comphelper/basicio.hxx>
#include <comphelper/guarding.hxx>
#include <comphelper/listenernotification.hxx>
#include <comphelper/property.hxx>
#include <connectivity/dbtools.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <toolkit/helper/emptyfontdescriptor.hxx>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>

#include <functional>
#include <algorithm>

namespace frm
{
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
using namespace ::com::sun::star::form::binding;
using namespace ::com::sun::star::form::validation;
using namespace ::dbtools;
using namespace ::comphelper;


void ControlModelLock::impl_notifyAll_nothrow()
{
    m_rModel.firePropertyChanges( m_aHandles, m_aOldValues, m_aNewValues, OControlModel::LockAccess() );
}

void ControlModelLock::addPropertyNotification( const sal_Int32 _nHandle, const Any& _rOldValue, const Any& _rNewValue )
{
    sal_Int32 nOldLength = m_aHandles.getLength();
    if  (   ( nOldLength != m_aOldValues.getLength() )
        ||  ( nOldLength != m_aNewValues.getLength() )
        )
        throw RuntimeException( OUString(), m_rModel );

    m_aHandles.realloc( nOldLength + 1 );
    m_aHandles[ nOldLength ] = _nHandle;
    m_aOldValues.realloc( nOldLength + 1 );
    m_aOldValues[ nOldLength ] = _rOldValue;
    m_aNewValues.realloc( nOldLength + 1 );
    m_aNewValues[ nOldLength ] = _rNewValue;
}

class FieldChangeNotifier
{
public:
    FieldChangeNotifier( ControlModelLock& _rLock )
        :m_rLock( _rLock )
        ,m_rModel( dynamic_cast< OBoundControlModel& >( _rLock.getModel() ) )
    {
        m_xOldField = m_rModel.getField();
    }

    ~FieldChangeNotifier()
    {
        Reference< XPropertySet > xNewField( m_rModel.getField() );
        if ( m_xOldField != xNewField )
            m_rLock.addPropertyNotification( PROPERTY_ID_BOUNDFIELD, makeAny( m_xOldField ), makeAny( xNewField ) );
    }

private:
    ControlModelLock&           m_rLock;
    OBoundControlModel&         m_rModel;
    Reference< XPropertySet >   m_xOldField;
};


OControl::OControl( const Reference< XComponentContext >& _rxContext, const OUString& _rAggregateService, const sal_Bool _bSetDelegator )
            :OComponentHelper(m_aMutex)
            ,m_xContext( _rxContext )
{
    
    
    
    increment( m_refCount );
    {
        m_xAggregate = m_xAggregate.query( _rxContext->getServiceManager()->createInstanceWithContext(_rAggregateService, _rxContext) );
        m_xControl = m_xControl.query( m_xAggregate );
    }
    decrement( m_refCount );

    if ( _bSetDelegator )
        doSetDelegator();
}

OControl::~OControl()
{
    doResetDelegator();
}

void OControl::doResetDelegator()
{
    if ( m_xAggregate.is() )
        m_xAggregate->setDelegator( NULL );
}

void OControl::doSetDelegator()
{
    increment( m_refCount );
    if ( m_xAggregate.is() )
    {   
        
        
        m_xAggregate->setDelegator( static_cast< XWeak* >( this ) );
    }
    decrement( m_refCount );
}


Any SAL_CALL OControl::queryAggregation( const Type& _rType ) throw(RuntimeException)
{
    
    Any aReturn( OComponentHelper::queryAggregation(_rType) );
    
    if (!aReturn.hasValue())
    {
        aReturn = OControl_BASE::queryInterface(_rType);
        
        if (!aReturn.hasValue() && m_xAggregate.is())
            aReturn = m_xAggregate->queryAggregation(_rType);
    }

    return aReturn;
}

Sequence<sal_Int8> SAL_CALL OControl::getImplementationId() throw(RuntimeException)
{
    return OImplementationIds::getImplementationId(getTypes());
}

Sequence<Type> SAL_CALL OControl::getTypes() throw(RuntimeException)
{
    TypeBag aTypes( _getTypes() );

    Reference< XTypeProvider > xProv;
    if ( query_aggregation( m_xAggregate, xProv ) )
        aTypes.addTypes( xProv->getTypes() );

    return aTypes.getTypes();
}

Sequence<Type> OControl::_getTypes()
{
    return TypeBag( OComponentHelper::getTypes(), OControl_BASE::getTypes() ).getTypes();
}

void OControl::initFormControlPeer( const Reference< XWindowPeer >& /*_rxPeer*/ )
{
    
}


void OControl::disposing()
{
    OComponentHelper::disposing();

    m_aWindowStateGuard.attach( NULL, NULL );

    Reference< XComponent > xComp;
    if (query_aggregation(m_xAggregate, xComp))
        xComp->dispose();
}


sal_Bool SAL_CALL OControl::supportsService(const OUString& _rsServiceName) throw ( RuntimeException)
{
    return cppu::supportsService(this, _rsServiceName);
}

Sequence< OUString > OControl::getAggregateServiceNames()
{
    Sequence< OUString > aAggServices;
    Reference< XServiceInfo > xInfo;
    if ( query_aggregation( m_xAggregate, xInfo ) )
        aAggServices = xInfo->getSupportedServiceNames();

    return aAggServices;
}

Sequence<OUString> SAL_CALL OControl::getSupportedServiceNames() throw(RuntimeException)
{
    return ::comphelper::concatSequences(
        getAggregateServiceNames(),
        getSupportedServiceNames_Static()
   );
}

Sequence< OUString > SAL_CALL OControl::getSupportedServiceNames_Static() throw( RuntimeException )
{
    
    return Sequence< OUString >();
}


void SAL_CALL OControl::disposing(const com::sun::star::lang::EventObject& _rEvent) throw (RuntimeException)
{
    Reference< XInterface > xAggAsIface;
    query_aggregation(m_xAggregate, xAggAsIface);

    
    if (xAggAsIface != Reference< XInterface >(_rEvent.Source, UNO_QUERY))
    {   
                Reference<com::sun::star::lang::XEventListener> xListener;
        if (query_aggregation(m_xAggregate, xListener))
            xListener->disposing(_rEvent);
    }
}


void SAL_CALL OControl::setContext(const Reference< XInterface >& Context) throw (RuntimeException)
{
    if (m_xControl.is())
        m_xControl->setContext(Context);
}

Reference< XInterface > SAL_CALL OControl::getContext() throw (RuntimeException)
{
    return m_xControl.is() ? m_xControl->getContext() : Reference< XInterface >();
}

void OControl::impl_resetStateGuard_nothrow()
{
    Reference< XWindow2 > xWindow;
    Reference< XControlModel > xModel;
    try
    {
        xWindow.set( getPeer(), UNO_QUERY );
        xModel.set( getModel(), UNO_QUERY );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
    m_aWindowStateGuard.attach( xWindow, xModel );
}

void SAL_CALL OControl::createPeer(const Reference<XToolkit>& _rxToolkit, const Reference<XWindowPeer>& _rxParent) throw (RuntimeException)
{
    if ( m_xControl.is() )
    {
        m_xControl->createPeer( _rxToolkit, _rxParent );
        initFormControlPeer( getPeer() );
        impl_resetStateGuard_nothrow();
    }
}

Reference<XWindowPeer> SAL_CALL OControl::getPeer() throw ( RuntimeException)
{
    return m_xControl.is() ? m_xControl->getPeer() : Reference<XWindowPeer>();
}

sal_Bool SAL_CALL OControl::setModel(const Reference<XControlModel>& Model) throw ( RuntimeException)
{
    if ( !m_xControl.is() )
        return sal_False;

    sal_Bool bSuccess = m_xControl->setModel( Model );
    impl_resetStateGuard_nothrow();
    return bSuccess;
}

Reference<XControlModel> SAL_CALL OControl::getModel() throw ( RuntimeException)
{
    return m_xControl.is() ? m_xControl->getModel() : Reference<XControlModel>();
}

Reference<XView> SAL_CALL OControl::getView() throw ( RuntimeException)
{
    return m_xControl.is() ? m_xControl->getView() : Reference<XView>();
}

void SAL_CALL OControl::setDesignMode(sal_Bool bOn) throw ( RuntimeException)
{
    if (m_xControl.is())
        m_xControl->setDesignMode(bOn);
}

sal_Bool SAL_CALL OControl::isDesignMode() throw ( RuntimeException)
{
    return m_xControl.is() ? m_xControl->isDesignMode() : sal_True;
}

sal_Bool SAL_CALL OControl::isTransparent() throw ( RuntimeException)
{
    return m_xControl.is() ? m_xControl->isTransparent() : sal_True;
}


OBoundControl::OBoundControl( const Reference< XComponentContext >& _rxContext,
            const OUString& _rAggregateService, const sal_Bool _bSetDelegator )
    :OControl( _rxContext, _rAggregateService, _bSetDelegator )
    ,m_bLocked(sal_False)
    ,m_aOriginalFont( EmptyFontDescriptor() )
    ,m_nOriginalTextLineColor( 0 )
{
}

OBoundControl::~OBoundControl()
{
}

Sequence< Type> OBoundControl::_getTypes()
{
    return TypeBag( OControl::_getTypes(), OBoundControl_BASE::getTypes() ).getTypes();
}

Any SAL_CALL OBoundControl::queryAggregation(const Type& _rType) throw(RuntimeException)
{
    Any aReturn;

    
    if ( _rType.equals( ::getCppuType( static_cast< Reference< XTypeProvider >* >( NULL ) ) ) )
        aReturn = OControl::queryAggregation( _rType );

    
    
    if ( !aReturn.hasValue() )
        aReturn = OBoundControl_BASE::queryInterface( _rType );

    
    if ( !aReturn.hasValue() )
        aReturn = OControl::queryAggregation( _rType );

    return aReturn;
}

sal_Bool SAL_CALL OBoundControl::getLock() throw(RuntimeException)
{
    return m_bLocked;
}

void SAL_CALL OBoundControl::setLock(sal_Bool _bLock) throw(RuntimeException)
{
    if (m_bLocked == _bLock)
        return;

    osl::MutexGuard aGuard(m_aMutex);
    _setLock(_bLock);
    m_bLocked = _bLock;
}

void OBoundControl::_setLock(sal_Bool _bLock)
{
    
    Reference< XWindowPeer > xPeer = getPeer();
    Reference< XTextComponent > xText( xPeer, UNO_QUERY );

    if ( xText.is() )
        xText->setEditable( !_bLock );
    else
    {
        
        Reference< XWindow > xComp( xPeer, UNO_QUERY );
        if ( xComp.is() )
            xComp->setEnable( !_bLock );
    }
}

sal_Bool SAL_CALL OBoundControl::setModel( const Reference< XControlModel >& _rxModel ) throw (RuntimeException)
{
    return OControl::setModel( _rxModel );
}

void SAL_CALL OBoundControl::disposing(const EventObject& Source) throw (RuntimeException)
{
    
    OControl::disposing(Source);
}

void OBoundControl::disposing()
{
    OControl::disposing();
}


Sequence<sal_Int8> SAL_CALL OControlModel::getImplementationId() throw(RuntimeException)
{
    return OImplementationIds::getImplementationId(getTypes());
}

Sequence<Type> SAL_CALL OControlModel::getTypes() throw(RuntimeException)
{
    TypeBag aTypes( _getTypes() );

    Reference< XTypeProvider > xProv;

    if ( query_aggregation( m_xAggregate, xProv ) )
        aTypes.addTypes( xProv->getTypes() );

    return aTypes.getTypes();
}

Sequence<Type> OControlModel::_getTypes()
{
    return TypeBag( OComponentHelper::getTypes(),
        OPropertySetAggregationHelper::getTypes(),
        OControlModel_BASE::getTypes()
    ).getTypes();
}

Any SAL_CALL OControlModel::queryAggregation(const Type& _rType) throw (RuntimeException)
{
    
    Any aReturn(OComponentHelper::queryAggregation(_rType));

    
    if (!aReturn.hasValue())
    {
        aReturn = OControlModel_BASE::queryInterface(_rType);

        
        if (!aReturn.hasValue())
        {
            aReturn = OPropertySetAggregationHelper::queryInterface(_rType);
            
            if (!aReturn.hasValue() && m_xAggregate.is() && !_rType.equals(::getCppuType(static_cast< Reference< XCloneable>* >(NULL))))
                aReturn = m_xAggregate->queryAggregation(_rType);
        }
    }
    return aReturn;
}

void OControlModel::readHelpTextCompatibly(const staruno::Reference< stario::XObjectInputStream >& _rxInStream)
{
    OUString sHelpText;
    ::comphelper::operator>>( _rxInStream, sHelpText);
    try
    {
        if (m_xAggregateSet.is())
            m_xAggregateSet->setPropertyValue(PROPERTY_HELPTEXT, makeAny(sHelpText));
    }
    catch(const Exception&)
    {
        SAL_WARN("forms.component", "OControlModel::readHelpTextCompatibly: could not forward the property value to the aggregate!");
        DBG_UNHANDLED_EXCEPTION();
    }
}

void OControlModel::writeHelpTextCompatibly(const staruno::Reference< stario::XObjectOutputStream >& _rxOutStream)
{
    OUString sHelpText;
    try
    {
        if (m_xAggregateSet.is())
            m_xAggregateSet->getPropertyValue(PROPERTY_HELPTEXT) >>= sHelpText;
    }
    catch(const Exception&)
    {
        SAL_WARN("forms.component", "OControlModel::writeHelpTextCompatibly: could not retrieve the property value from the aggregate!");
        DBG_UNHANDLED_EXCEPTION();
    }
    ::comphelper::operator<<( _rxOutStream, sHelpText);
}

OControlModel::OControlModel(
            const Reference<XComponentContext>& _rxContext,
            const OUString& _rUnoControlModelTypeName,
            const OUString& rDefault, const sal_Bool _bSetDelegator)
    :OComponentHelper(m_aMutex)
    ,OPropertySetAggregationHelper(OComponentHelper::rBHelper)
    ,m_xContext( _rxContext )
    ,m_lockCount( 0 )
    ,m_aPropertyBagHelper( *this )
    ,m_nTabIndex(FRM_DEFAULT_TABINDEX)
    ,m_nClassId(FormComponentType::CONTROL)
    ,m_bNativeLook( sal_False )
    ,m_bGenerateVbEvents( sal_False )
    ,m_nControlTypeinMSO(0) 
    ,m_nObjIDinMSO(INVALID_OBJ_ID_IN_MSO)
    
    
    
{
    if (!_rUnoControlModelTypeName.isEmpty())  
    {
        increment(m_refCount);
        {
            m_xAggregate = Reference<XAggregation>(m_xContext->getServiceManager()->createInstanceWithContext(_rUnoControlModelTypeName, m_xContext), UNO_QUERY);
            setAggregation(m_xAggregate);

            if ( m_xAggregateSet.is() )
            {
                try
                {
                    if ( !rDefault.isEmpty() )
                        m_xAggregateSet->setPropertyValue( PROPERTY_DEFAULTCONTROL, makeAny( rDefault ) );
                }
                catch( const Exception& )
                {
                    SAL_WARN("forms.component",  "OControlModel::OControlModel: caught an exception!");
                    DBG_UNHANDLED_EXCEPTION();
                }
            }
        }
        if (_bSetDelegator)
            doSetDelegator();

        
        decrement(m_refCount);
    }
}

OControlModel::OControlModel( const OControlModel* _pOriginal, const Reference< XComponentContext>& _rxFactory, const sal_Bool _bCloneAggregate, const sal_Bool _bSetDelegator )
    :OComponentHelper( m_aMutex )
    ,OPropertySetAggregationHelper( OComponentHelper::rBHelper )
    ,m_xContext( _rxFactory )
    ,m_lockCount( 0 )
    ,m_aPropertyBagHelper( *this )
    ,m_nTabIndex( FRM_DEFAULT_TABINDEX )
    ,m_nClassId( FormComponentType::CONTROL )
{
    DBG_ASSERT( _pOriginal, "OControlModel::OControlModel: invalid original!" );

    
    m_aName = _pOriginal->m_aName;
    m_aTag = _pOriginal->m_aTag;
    m_nTabIndex = _pOriginal->m_nTabIndex;
    m_nClassId = _pOriginal->m_nClassId;
    m_bNativeLook = _pOriginal->m_bNativeLook;
    m_bGenerateVbEvents = _pOriginal->m_bGenerateVbEvents;
    m_nControlTypeinMSO = _pOriginal->m_nControlTypeinMSO;
    m_nObjIDinMSO = _pOriginal->m_nObjIDinMSO;

    if ( _bCloneAggregate )
    {
        
        increment( m_refCount );
        {
            
            m_xAggregate = createAggregateClone( _pOriginal );

            
            setAggregation( m_xAggregate );
        }

        
        if ( _bSetDelegator )
            doSetDelegator();

        
        decrement( m_refCount );
    }
}

OControlModel::~OControlModel()
{
    
    doResetDelegator( );
}

void OControlModel::clonedFrom( const OControlModel* /*_pOriginal*/ )
{
    
}

void OControlModel::doResetDelegator()
{
    if (m_xAggregate.is())
        m_xAggregate->setDelegator(NULL);
}

void OControlModel::doSetDelegator()
{
    increment(m_refCount);
    if (m_xAggregate.is())
    {
        m_xAggregate->setDelegator(static_cast<XWeak*>(this));
    }
    decrement(m_refCount);
}


Reference< XInterface > SAL_CALL OControlModel::getParent() throw(RuntimeException)
{
    return m_xParent;
}

void SAL_CALL OControlModel::setParent(const Reference< XInterface >& _rxParent) throw(com::sun::star::lang::NoSupportException, RuntimeException)
{
    osl::MutexGuard aGuard(m_aMutex);

    Reference<XComponent> xComp(m_xParent, UNO_QUERY);
    if (xComp.is())
        xComp->removeEventListener(static_cast<XPropertiesChangeListener*>(this));

    m_xParent = _rxParent;
    xComp = xComp.query( m_xParent );

    if ( xComp.is() )
        xComp->addEventListener(static_cast<XPropertiesChangeListener*>(this));
}


OUString SAL_CALL OControlModel::getName() throw(RuntimeException)
{
    OUString aReturn;
    OPropertySetHelper::getFastPropertyValue(PROPERTY_ID_NAME) >>= aReturn;
    return aReturn;
}

void SAL_CALL OControlModel::setName(const OUString& _rName) throw(RuntimeException)
{
        setFastPropertyValue(PROPERTY_ID_NAME, makeAny(_rName));
}


sal_Bool SAL_CALL OControlModel::supportsService(const OUString& _rServiceName) throw ( RuntimeException)
{
    return cppu::supportsService(this, _rServiceName);
}

Sequence< OUString > OControlModel::getAggregateServiceNames()
{
    Sequence< OUString > aAggServices;
    Reference< XServiceInfo > xInfo;
    if ( query_aggregation( m_xAggregate, xInfo ) )
        aAggServices = xInfo->getSupportedServiceNames();
    return aAggServices;
}

Sequence<OUString> SAL_CALL OControlModel::getSupportedServiceNames() throw(RuntimeException)
{
    return ::comphelper::concatSequences(
        getAggregateServiceNames(),
        getSupportedServiceNames_Static()
    );
}

Sequence< OUString > SAL_CALL OControlModel::getSupportedServiceNames_Static() throw( RuntimeException )
{
    Sequence< OUString > aServiceNames( 2 );
    aServiceNames[ 0 ] = FRM_SUN_FORMCOMPONENT;
    aServiceNames[ 1 ] = "com.sun.star.form.FormControlModel";
    return aServiceNames;
}


void SAL_CALL OControlModel::disposing(const com::sun::star::lang::EventObject& _rSource) throw (RuntimeException)
{
    
    if (_rSource.Source == m_xParent)
    {
        osl::MutexGuard aGuard(m_aMutex);
        m_xParent = NULL;
    }
    else
    {
        Reference<com::sun::star::lang::XEventListener> xEvtLst;
        if (query_aggregation(m_xAggregate, xEvtLst))
        {
            osl::MutexGuard aGuard(m_aMutex);
            xEvtLst->disposing(_rSource);
        }
    }
}


void OControlModel::disposing()
{
    OPropertySetAggregationHelper::disposing();

    Reference<com::sun::star::lang::XComponent> xComp;
    if (query_aggregation(m_xAggregate, xComp))
        xComp->dispose();

    setParent(Reference<XFormComponent>());

    m_aPropertyBagHelper.dispose();
}

void OControlModel::writeAggregate( const Reference< XObjectOutputStream >& _rxOutStream ) const
{
    Reference< XPersistObject > xPersist;
    if ( query_aggregation( m_xAggregate, xPersist ) )
        xPersist->write( _rxOutStream );
}

void OControlModel::readAggregate( const Reference< XObjectInputStream >& _rxInStream )
{
    Reference< XPersistObject > xPersist;
    if ( query_aggregation( m_xAggregate, xPersist ) )
        xPersist->read( _rxInStream );
}

void SAL_CALL OControlModel::write(const Reference<stario::XObjectOutputStream>& _rxOutStream)
                        throw(stario::IOException, RuntimeException)
{
    osl::MutexGuard aGuard(m_aMutex);

    
    Reference<stario::XMarkableStream> xMark(_rxOutStream, UNO_QUERY);
    if ( !xMark.is() )
    {
        throw IOException(
            FRM_RES_STRING( RID_STR_INVALIDSTREAM ),
            static_cast< ::cppu::OWeakObject* >( this )
        );
    }

    sal_Int32 nMark = xMark->createMark();
    sal_Int32 nLen = 0;

    _rxOutStream->writeLong(nLen);

    writeAggregate( _rxOutStream );

    
    nLen = xMark->offsetToMark(nMark) - 4;
    xMark->jumpToMark(nMark);
    _rxOutStream->writeLong(nLen);
    xMark->jumpToFurthest();
    xMark->deleteMark(nMark);

    
    _rxOutStream->writeShort(0x0003);

    
    ::comphelper::operator<<( _rxOutStream, m_aName);
    _rxOutStream->writeShort(m_nTabIndex);
    ::comphelper::operator<<( _rxOutStream, m_aTag); 

    
    
    
    
    
    
}

void OControlModel::read(const Reference<stario::XObjectInputStream>& InStream) throw (::com::sun::star::io::IOException, RuntimeException)
{
    osl::MutexGuard aGuard(m_aMutex);

    Reference<stario::XMarkableStream> xMark(InStream, UNO_QUERY);
    if ( !xMark.is() )
    {
        throw IOException(
            FRM_RES_STRING( RID_STR_INVALIDSTREAM ),
            static_cast< ::cppu::OWeakObject* >( this )
        );
    }

    
    sal_Int32 nLen = InStream->readLong();
    if (nLen)
    {
        sal_Int32 nMark = xMark->createMark();

        try
        {
            readAggregate( InStream );
        }

        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }

        xMark->jumpToMark(nMark);
        InStream->skipBytes(nLen);
        xMark->deleteMark(nMark);
    }

    
    sal_uInt16 nVersion = InStream->readShort();

    
    ::comphelper::operator>>( InStream, m_aName);
    m_nTabIndex  = InStream->readShort();

    if (nVersion > 0x0002)
        ::comphelper::operator>>( InStream, m_aTag);

    
    if (nVersion == 0x0004)
        readHelpTextCompatibly(InStream);

    DBG_ASSERT(nVersion < 5, "OControlModel::read : suspicious version number !");
    
    
}

PropertyState OControlModel::getPropertyStateByHandle( sal_Int32 _nHandle )
{
    
    Any aCurrentValue = getPropertyDefaultByHandle( _nHandle );
    Any aDefaultValue;  getFastPropertyValue( aDefaultValue, _nHandle );

    sal_Bool bEqual = uno_type_equalData(
            const_cast< void* >( aCurrentValue.getValue() ), aCurrentValue.getValueType().getTypeLibType(),
            const_cast< void* >( aDefaultValue.getValue() ), aDefaultValue.getValueType().getTypeLibType(),
            reinterpret_cast< uno_QueryInterfaceFunc >(cpp_queryInterface),
            reinterpret_cast< uno_ReleaseFunc >(cpp_release)
        );
    return bEqual ? PropertyState_DEFAULT_VALUE : PropertyState_DIRECT_VALUE;
}

void OControlModel::setPropertyToDefaultByHandle( sal_Int32 _nHandle)
{
    Any aDefault = getPropertyDefaultByHandle( _nHandle );

    Any aConvertedValue, aOldValue;
    if ( convertFastPropertyValue( aConvertedValue, aOldValue, _nHandle, aDefault ) )
    {
        setFastPropertyValue_NoBroadcast( _nHandle, aConvertedValue );
        
    }
}

Any OControlModel::getPropertyDefaultByHandle( sal_Int32 _nHandle ) const
{
    Any aReturn;
    switch ( _nHandle )
    {
        case PROPERTY_ID_NAME:
        case PROPERTY_ID_TAG:
            aReturn <<= OUString();
            break;
        case PROPERTY_ID_CLASSID:
            aReturn <<= (sal_Int16)FormComponentType::CONTROL;
            break;
        case PROPERTY_ID_TABINDEX:
            aReturn <<= (sal_Int16)FRM_DEFAULT_TABINDEX;
            break;
        case PROPERTY_ID_NATIVE_LOOK:
            aReturn <<= (sal_Bool)sal_True;
            break;
        case PROPERTY_ID_GENERATEVBAEVENTS:
            aReturn <<= (sal_Bool)sal_False;
            break;
        
        case PROPERTY_ID_CONTROL_TYPE_IN_MSO:
            aReturn <<= (sal_Int16)0;
            break;
        case PROPERTY_ID_OBJ_ID_IN_MSO:
            aReturn <<= (sal_uInt16)INVALID_OBJ_ID_IN_MSO;
            break;
        default:
            if ( m_aPropertyBagHelper.hasDynamicPropertyByHandle( _nHandle ) )
                m_aPropertyBagHelper.getDynamicPropertyDefaultByHandle( _nHandle, aReturn );
            else
                SAL_WARN("forms.component",  "OControlModel::convertFastPropertyValue: unknown handle " << _nHandle);
    }
    return aReturn;
}

void OControlModel::getFastPropertyValue( Any& _rValue, sal_Int32 _nHandle ) const
{
    switch ( _nHandle )
    {
        case PROPERTY_ID_NAME:
            _rValue <<= m_aName;
            break;
        case PROPERTY_ID_TAG:
            _rValue <<= m_aTag;
            break;
        case PROPERTY_ID_CLASSID:
            _rValue <<= m_nClassId;
            break;
        case PROPERTY_ID_TABINDEX:
            _rValue <<= m_nTabIndex;
            break;
        case PROPERTY_ID_NATIVE_LOOK:
            _rValue <<= (sal_Bool)m_bNativeLook;
            break;
        case PROPERTY_ID_GENERATEVBAEVENTS:
            _rValue <<= (sal_Bool)m_bGenerateVbEvents;
        
        case PROPERTY_ID_CONTROL_TYPE_IN_MSO:
            _rValue <<= (sal_Int16)m_nControlTypeinMSO;
            break;
        case PROPERTY_ID_OBJ_ID_IN_MSO:
            _rValue <<= (sal_uInt16)m_nObjIDinMSO;
            break;
        default:
            if ( m_aPropertyBagHelper.hasDynamicPropertyByHandle( _nHandle ) )
                m_aPropertyBagHelper.getDynamicFastPropertyValue( _nHandle, _rValue );
            else
                OPropertySetAggregationHelper::getFastPropertyValue( _rValue, _nHandle );
            break;
    }
}

sal_Bool OControlModel::convertFastPropertyValue(
                        Any& _rConvertedValue, Any& _rOldValue, sal_Int32 _nHandle, const Any& _rValue)
                        throw (com::sun::star::lang::IllegalArgumentException)
{
    sal_Bool bModified(sal_False);
    switch (_nHandle)
    {
        case PROPERTY_ID_NAME:
            bModified = tryPropertyValue(_rConvertedValue, _rOldValue, _rValue, m_aName);
            break;
        case PROPERTY_ID_TAG:
            bModified = tryPropertyValue(_rConvertedValue, _rOldValue, _rValue, m_aTag);
            break;
        case PROPERTY_ID_TABINDEX:
            bModified = tryPropertyValue(_rConvertedValue, _rOldValue, _rValue, m_nTabIndex);
            break;
        case PROPERTY_ID_NATIVE_LOOK:
            bModified = tryPropertyValue(_rConvertedValue, _rOldValue, _rValue, m_bNativeLook);
            break;
        case PROPERTY_ID_GENERATEVBAEVENTS:
            bModified = tryPropertyValue(_rConvertedValue, _rOldValue, _rValue, m_bGenerateVbEvents);
            break;
        
        case PROPERTY_ID_CONTROL_TYPE_IN_MSO:
            bModified = tryPropertyValue(_rConvertedValue, _rOldValue, _rValue, m_nControlTypeinMSO);
            break;
        case PROPERTY_ID_OBJ_ID_IN_MSO:
            bModified = tryPropertyValue(_rConvertedValue, _rOldValue, _rValue, m_nObjIDinMSO);
            break;
        default:
            if ( m_aPropertyBagHelper.hasDynamicPropertyByHandle( _nHandle ) )
                bModified = m_aPropertyBagHelper.convertDynamicFastPropertyValue( _nHandle, _rValue, _rConvertedValue, _rOldValue );
            else
                SAL_WARN("forms.component",  "OControlModel::convertFastPropertyValue: unknown handle " << _nHandle);
            break;
    }
    return bModified;
}

void OControlModel::setFastPropertyValue_NoBroadcast(sal_Int32 _nHandle, const Any& _rValue)
                        throw (Exception)
{
    switch (_nHandle)
    {
        case PROPERTY_ID_NAME:
            DBG_ASSERT(_rValue.getValueType() == getCppuType((const OUString*)NULL),
                "OControlModel::setFastPropertyValue_NoBroadcast : invalid type" );
            _rValue >>= m_aName;
            break;
        case PROPERTY_ID_TAG:
            DBG_ASSERT(_rValue.getValueType() == getCppuType((const OUString*)NULL),
                "OControlModel::setFastPropertyValue_NoBroadcast : invalid type" );
            _rValue >>= m_aTag;
            break;
        case PROPERTY_ID_TABINDEX:
            DBG_ASSERT(_rValue.getValueType() == getCppuType((const sal_Int16*)NULL),
                "OControlModel::setFastPropertyValue_NoBroadcast : invalid type" );
            _rValue >>= m_nTabIndex;
            break;
        case PROPERTY_ID_NATIVE_LOOK:
            OSL_VERIFY( _rValue >>= m_bNativeLook );
            break;
        case PROPERTY_ID_GENERATEVBAEVENTS:
            OSL_VERIFY( _rValue >>= m_bGenerateVbEvents );
            break;
        
        case PROPERTY_ID_CONTROL_TYPE_IN_MSO:
            OSL_VERIFY( _rValue >>= m_nControlTypeinMSO );
            break;
        case PROPERTY_ID_OBJ_ID_IN_MSO:
            OSL_VERIFY( _rValue >>= m_nObjIDinMSO );
            break;
        default:
            if ( m_aPropertyBagHelper.hasDynamicPropertyByHandle( _nHandle ) )
                m_aPropertyBagHelper.setDynamicFastPropertyValue( _nHandle, _rValue );
            else
                SAL_WARN("forms.component",  "OControlModel::setFastPropertyValue_NoBroadcast: unknown handle " << _nHandle );
            break;
    }
}

void OControlModel::describeFixedProperties( Sequence< Property >& _rProps ) const
{
    BEGIN_DESCRIBE_BASE_PROPERTIES( 7 )
        DECL_PROP2      (CLASSID,     sal_Int16,        READONLY, TRANSIENT);
        DECL_PROP1      (NAME,        OUString,  BOUND);
        DECL_BOOL_PROP2 (NATIVE_LOOK,                   BOUND, TRANSIENT);
        DECL_PROP1      (TAG,         OUString,  BOUND);
        DECL_PROP1      (GENERATEVBAEVENTS,         sal_Bool,  TRANSIENT);
        DECL_PROP1      (CONTROL_TYPE_IN_MSO,sal_Int16,     BOUND);
        DECL_PROP1      (OBJ_ID_IN_MSO,sal_uInt16,      BOUND);
    END_DESCRIBE_PROPERTIES()
}

void OControlModel::describeAggregateProperties( Sequence< Property >& /* [out] */ _rAggregateProps ) const
{
    if ( m_xAggregateSet.is() )
    {
        Reference< XPropertySetInfo > xPSI( m_xAggregateSet->getPropertySetInfo() );
        if ( xPSI.is() )
            _rAggregateProps = xPSI->getProperties();
    }
}

::osl::Mutex& OControlModel::getMutex()
{
    return m_aMutex;
}

void OControlModel::describeFixedAndAggregateProperties( Sequence< Property >& _out_rFixedProperties, Sequence< Property >& _out_rAggregateProperties ) const
{
    describeFixedProperties( _out_rFixedProperties );
    describeAggregateProperties( _out_rAggregateProperties );
}

Reference< XMultiPropertySet > OControlModel::getPropertiesInterface()
{
    return Reference< XMultiPropertySet >( *this, UNO_QUERY );
}

Reference< XPropertySetInfo> SAL_CALL OControlModel::getPropertySetInfo() throw( RuntimeException)
{
    return createPropertySetInfo( getInfoHelper() );
}

::cppu::IPropertyArrayHelper& OControlModel::getInfoHelper()
{
    return m_aPropertyBagHelper.getInfoHelper();
}

void SAL_CALL OControlModel::addProperty( const OUString& _rName, ::sal_Int16 _nAttributes, const Any& _rInitialValue ) throw (PropertyExistException, IllegalTypeException, IllegalArgumentException, RuntimeException)
{
    m_aPropertyBagHelper.addProperty( _rName, _nAttributes, _rInitialValue );
}

void SAL_CALL OControlModel::removeProperty( const OUString& _rName ) throw (UnknownPropertyException, NotRemoveableException, RuntimeException)
{
    m_aPropertyBagHelper.removeProperty( _rName );
}

Sequence< PropertyValue > SAL_CALL OControlModel::getPropertyValues() throw (RuntimeException)
{
    return m_aPropertyBagHelper.getPropertyValues();
}

void SAL_CALL OControlModel::setPropertyValues( const Sequence< PropertyValue >& _rProps ) throw (UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException)
{
    m_aPropertyBagHelper.setPropertyValues( _rProps );
}

void OControlModel::lockInstance( LockAccess )
{
    m_aMutex.acquire();
    osl_atomic_increment( &m_lockCount );
}

oslInterlockedCount OControlModel::unlockInstance( LockAccess )
{
    OSL_ENSURE( m_lockCount > 0, "OControlModel::unlockInstance: not locked!" );
    oslInterlockedCount lockCount = osl_atomic_decrement( &m_lockCount );
    m_aMutex.release();
    return lockCount;
}

void OControlModel::firePropertyChanges( const Sequence< sal_Int32 >& _rHandles, const Sequence< Any >& _rOldValues,
                                        const Sequence< Any >& _rNewValues, LockAccess )
{
    OPropertySetHelper::fire(
        const_cast< Sequence< sal_Int32 >& >( _rHandles ).getArray(),
        _rNewValues.getConstArray(),
        _rOldValues.getConstArray(),
        _rHandles.getLength(),
        sal_False
    );
}


Any SAL_CALL OBoundControlModel::queryAggregation( const Type& _rType ) throw (RuntimeException)
{
    Any aReturn( OControlModel::queryAggregation(_rType) );
    if (!aReturn.hasValue())
    {
        aReturn = OBoundControlModel_BASE1::queryInterface(_rType);

        if ( !aReturn.hasValue() && m_bCommitable )
            aReturn = OBoundControlModel_COMMITTING::queryInterface( _rType );

        if ( !aReturn.hasValue() && m_bSupportsExternalBinding )
            aReturn = OBoundControlModel_BINDING::queryInterface( _rType );

        if ( !aReturn.hasValue() && m_bSupportsValidation )
            aReturn = OBoundControlModel_VALIDATION::queryInterface( _rType );
    }
    return aReturn;
}

OBoundControlModel::OBoundControlModel(
        const Reference< XComponentContext>& _rxFactory,
        const OUString& _rUnoControlModelTypeName, const OUString& _rDefault,
        const sal_Bool _bCommitable, const sal_Bool _bSupportExternalBinding, const sal_Bool _bSupportsValidation )
    :OControlModel( _rxFactory, _rUnoControlModelTypeName, _rDefault, sal_False )
    ,OPropertyChangeListener( m_aMutex )
    ,m_xField()
    ,m_xAmbientForm()
    ,m_nValuePropertyAggregateHandle( -1 )
    ,m_nFieldType( DataType::OTHER )
    ,m_bValuePropertyMayBeVoid( false )
    ,m_aResetHelper( *this, m_aMutex )
    ,m_aUpdateListeners(m_aMutex)
    ,m_aFormComponentListeners( m_aMutex )
    ,m_bInputRequired( sal_True )
    ,m_pAggPropMultiplexer( NULL )
    ,m_bFormListening( false )
    ,m_bLoaded(sal_False)
    ,m_bRequired(sal_False)
    ,m_bCommitable(_bCommitable)
    ,m_bSupportsExternalBinding( _bSupportExternalBinding )
    ,m_bSupportsValidation( _bSupportsValidation )
    ,m_bForwardValueChanges(sal_True)
    ,m_bTransferingValue( sal_False )
    ,m_bIsCurrentValueValid( sal_True )
    ,m_bBindingControlsRO( sal_False )
    ,m_bBindingControlsEnable( sal_False )
    ,m_eControlValueChangeInstigator( eOther )
    ,m_aLabelServiceName(FRM_SUN_COMPONENT_FIXEDTEXT)
{
    
    implInitAggMultiplexer( );
}

OBoundControlModel::OBoundControlModel(
        const OBoundControlModel* _pOriginal, const Reference< XComponentContext>& _rxFactory )
    :OControlModel( _pOriginal, _rxFactory, sal_True, sal_False )
    ,OPropertyChangeListener( m_aMutex )
    ,m_xField()
    ,m_xAmbientForm()
    ,m_nValuePropertyAggregateHandle( _pOriginal->m_nValuePropertyAggregateHandle )
    ,m_nFieldType( DataType::OTHER )
    ,m_bValuePropertyMayBeVoid( _pOriginal->m_bValuePropertyMayBeVoid )
    ,m_aResetHelper( *this, m_aMutex )
    ,m_aUpdateListeners( m_aMutex )
    ,m_aFormComponentListeners( m_aMutex )
    ,m_xValidator( _pOriginal->m_xValidator )
    ,m_bInputRequired( sal_True )
    ,m_pAggPropMultiplexer( NULL )
    ,m_bFormListening( false )
    ,m_bLoaded( sal_False )
    ,m_bRequired( sal_False )
    ,m_bCommitable( _pOriginal->m_bCommitable )
    ,m_bSupportsExternalBinding( _pOriginal->m_bSupportsExternalBinding )
    ,m_bSupportsValidation( _pOriginal->m_bSupportsValidation )
    ,m_bForwardValueChanges( sal_True )
    ,m_bTransferingValue( sal_False )
    ,m_bIsCurrentValueValid( _pOriginal->m_bIsCurrentValueValid )
    ,m_bBindingControlsRO( sal_False )
    ,m_bBindingControlsEnable( sal_False )
    ,m_eControlValueChangeInstigator( eOther )
{
    
    implInitAggMultiplexer( );
    m_aLabelServiceName = _pOriginal->m_aLabelServiceName;
    m_sValuePropertyName = _pOriginal->m_sValuePropertyName;
    m_nValuePropertyAggregateHandle = _pOriginal->m_nValuePropertyAggregateHandle;
    m_bValuePropertyMayBeVoid = _pOriginal->m_bValuePropertyMayBeVoid;
    m_aValuePropertyType = _pOriginal->m_aValuePropertyType;
    m_aControlSource = _pOriginal->m_aControlSource;
    m_bInputRequired = _pOriginal->m_bInputRequired;
    
    
    
    
    
    implInitValuePropertyListening( );
}

OBoundControlModel::~OBoundControlModel()
{
    if ( !OComponentHelper::rBHelper.bDisposed )
    {
        acquire();
        dispose();
    }

    doResetDelegator( );
    OSL_ENSURE( m_pAggPropMultiplexer, "OBoundControlModel::~OBoundControlModel: what about my property multiplexer?" );
    if ( m_pAggPropMultiplexer )
    {
        m_pAggPropMultiplexer->dispose();
        m_pAggPropMultiplexer->release();
        m_pAggPropMultiplexer = NULL;
    }
}

void OBoundControlModel::clonedFrom( const OControlModel* _pOriginal )
{
    const OBoundControlModel* pBoundOriginal = static_cast< const OBoundControlModel* >( _pOriginal );
    
    
    if ( pBoundOriginal && pBoundOriginal->m_xExternalBinding.is() )
    {
        try
        {
            setValueBinding( pBoundOriginal->m_xExternalBinding );
        }

        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }

    }
}

void OBoundControlModel::implInitAggMultiplexer( )
{
    increment( m_refCount );
    if ( m_xAggregateSet.is() )
    {
        m_pAggPropMultiplexer = new OPropertyChangeMultiplexer( this, m_xAggregateSet, false );
        m_pAggPropMultiplexer->acquire();
    }

    decrement( m_refCount );
    doSetDelegator();
}

void OBoundControlModel::implInitValuePropertyListening( ) const
{
    
    
    
    
    
    
    
    
    if ( m_bSupportsExternalBinding || m_bSupportsValidation || !m_bCommitable )
    {
        OSL_ENSURE( m_pAggPropMultiplexer, "OBoundControlModel::implInitValuePropertyListening: no multiplexer!" );
        if ( m_pAggPropMultiplexer && !m_sValuePropertyName.isEmpty() )
            m_pAggPropMultiplexer->addProperty( m_sValuePropertyName );
    }
}

void OBoundControlModel::initOwnValueProperty( const OUString& i_rValuePropertyName )
{
    OSL_PRECOND( m_sValuePropertyName.isEmpty() && -1 == m_nValuePropertyAggregateHandle,
        "OBoundControlModel::initOwnValueProperty: value property is already initialized!" );
    OSL_ENSURE( !i_rValuePropertyName.isEmpty(), "OBoundControlModel::initOwnValueProperty: invalid property name!" );
    m_sValuePropertyName = i_rValuePropertyName;
}

void OBoundControlModel::initValueProperty( const OUString& _rValuePropertyName, sal_Int32 _nValuePropertyExternalHandle )
{
    OSL_PRECOND( m_sValuePropertyName.isEmpty() && -1 == m_nValuePropertyAggregateHandle,
        "OBoundControlModel::initValueProperty: value property is already initialized!" );
    OSL_ENSURE( !_rValuePropertyName.isEmpty(), "OBoundControlModel::initValueProperty: invalid property name!" );
    OSL_ENSURE( _nValuePropertyExternalHandle != -1, "OBoundControlModel::initValueProperty: invalid property handle!" );
    
    m_sValuePropertyName = _rValuePropertyName;
    m_nValuePropertyAggregateHandle = getOriginalHandle( _nValuePropertyExternalHandle );
    OSL_ENSURE( m_nValuePropertyAggregateHandle != -1, "OBoundControlModel::initValueProperty: unable to find the original handle!" );
    
    if ( m_nValuePropertyAggregateHandle != -1 )
    {
        Reference< XPropertySetInfo > xPropInfo( m_xAggregateSet->getPropertySetInfo(), UNO_SET_THROW );
        Property aValuePropDesc = xPropInfo->getPropertyByName( m_sValuePropertyName );
        m_aValuePropertyType = aValuePropDesc.Type;
        m_bValuePropertyMayBeVoid = ( aValuePropDesc.Attributes & PropertyAttribute::MAYBEVOID ) != 0;
    }

    
    implInitValuePropertyListening( );
}

void OBoundControlModel::suspendValueListening( )
{
    OSL_PRECOND( !m_sValuePropertyName.isEmpty(), "OBoundControlModel::suspendValueListening: don't have a value property!" );
    OSL_PRECOND( m_pAggPropMultiplexer, "OBoundControlModel::suspendValueListening: I *am* not listening!" );
    
if ( m_pAggPropMultiplexer )
        m_pAggPropMultiplexer->lock();
}

void OBoundControlModel::resumeValueListening( )
{
    OSL_PRECOND( !m_sValuePropertyName.isEmpty(), "OBoundControlModel::resumeValueListening: don't have a value property!" );
    OSL_PRECOND( m_pAggPropMultiplexer, "OBoundControlModel::resumeValueListening: I *am* not listening at all!" );
    OSL_PRECOND( !m_pAggPropMultiplexer || m_pAggPropMultiplexer->locked(), "OBoundControlModel::resumeValueListening: listening not suspended currently!" );
    if ( m_pAggPropMultiplexer )
        m_pAggPropMultiplexer->unlock();
}

Sequence< Type > OBoundControlModel::_getTypes()
{
    TypeBag aTypes(
        OControlModel::_getTypes(),
        OBoundControlModel_BASE1::getTypes()
    );

    if ( m_bCommitable )
        aTypes.addTypes( OBoundControlModel_COMMITTING::getTypes() );

    if ( m_bSupportsExternalBinding )
        aTypes.addTypes( OBoundControlModel_BINDING::getTypes() );

    if ( m_bSupportsValidation )
        aTypes.addTypes( OBoundControlModel_VALIDATION::getTypes() );
    return aTypes.getTypes();
}


void OBoundControlModel::disposing()
{
    OControlModel::disposing();

    ::osl::ClearableMutexGuard aGuard(m_aMutex);

    if ( m_pAggPropMultiplexer )
        m_pAggPropMultiplexer->dispose();

    
    com::sun::star::lang::EventObject aEvt( static_cast< XWeak* >( this ) );
    m_aUpdateListeners.disposeAndClear( aEvt );
    m_aResetHelper.disposing();

    
    
    
    
    if ( hasField() )
    {
        getField()->removePropertyChangeListener( PROPERTY_VALUE, this );
        resetField();
    }

    m_xCursor = NULL;
    Reference< XComponent > xComp( m_xLabelControl, UNO_QUERY );
    if ( xComp.is() )
        xComp->removeEventListener(static_cast< XEventListener* >( static_cast< XPropertyChangeListener* >( this ) ) );
    
    if ( hasExternalValueBinding() )
        disconnectExternalValueBinding();
    
    if ( hasValidator() )
        disconnectValidator( );
}

void OBoundControlModel::onValuePropertyChange( ControlModelLock& i_rControLock )
{
    if ( hasExternalValueBinding() )
    {   
        
        
        if ( m_eControlValueChangeInstigator != eExternalBinding )
            transferControlValueToExternal( i_rControLock );
    }

    else if ( !m_bCommitable && m_xColumnUpdate.is() )
    {   
        
        
        
        
        if ( m_eControlValueChangeInstigator != eDbColumnBinding )
            commitControlValueToDbColumn( false );
    }

    
    if ( m_bSupportsValidation )
        recheckValidity( true );
}

void OBoundControlModel::_propertyChanged( const PropertyChangeEvent& _rEvt ) throw ( RuntimeException )
{
    ControlModelLock aLock( *this );
    OSL_ENSURE( _rEvt.PropertyName == m_sValuePropertyName,
        "OBoundControlModel::_propertyChanged: where did this come from (1)?" );
    OSL_ENSURE( m_pAggPropMultiplexer && !m_pAggPropMultiplexer->locked(),
        "OBoundControlModel::_propertyChanged: where did this come from (2)?" );
    if ( _rEvt.PropertyName == m_sValuePropertyName )
    {
        onValuePropertyChange( aLock );
    }
}

void OBoundControlModel::startAggregatePropertyListening( const OUString& _rPropertyName )
{
    OSL_PRECOND( m_pAggPropMultiplexer, "OBoundControlModel::startAggregatePropertyListening: no multiplexer!" );
    OSL_ENSURE( !_rPropertyName.isEmpty(), "OBoundControlModel::startAggregatePropertyListening: invalid property name!" );
    if ( m_pAggPropMultiplexer && !_rPropertyName.isEmpty() )
    {
        m_pAggPropMultiplexer->addProperty( _rPropertyName );
    }
}

void OBoundControlModel::doFormListening( const bool _bStart )
{
    OSL_PRECOND( !hasExternalValueBinding(), "OBoundControlModel::doFormListening: external value binding should overrule the database binding!" );
    if ( isFormListening() == _bStart )
        return;
    if ( m_xAmbientForm.is() )
        _bStart ? m_xAmbientForm->addLoadListener( this ) : m_xAmbientForm->removeLoadListener( this );
    Reference< XLoadable > xParentLoadable( getParent(), UNO_QUERY );
    if ( getParent().is() && !xParentLoadable.is() )
    {
        
        
        
        Reference< XRowSetChangeBroadcaster > xRowSetBroadcaster( getParent(), UNO_QUERY );
        if ( xRowSetBroadcaster.is() )
            _bStart ? xRowSetBroadcaster->addRowSetChangeListener( this ) : xRowSetBroadcaster->removeRowSetChangeListener( this );
    }

    m_bFormListening = _bStart && m_xAmbientForm.is();
}


void SAL_CALL OBoundControlModel::setParent(const Reference<XInterface>& _rxParent) throw(com::sun::star::lang::NoSupportException, RuntimeException)
{
    ControlModelLock aLock( *this );
    FieldChangeNotifier aBoundFieldNotifier( aLock );
    if ( getParent() == _rxParent )
        return;
    
    if ( hasField() )
        impl_disconnectDatabaseColumn_noNotify();
    
    if ( isFormListening() )
        doFormListening( false );
    
    OControlModel::setParent( _rxParent );
    
    impl_determineAmbientForm_nothrow();
    if ( !hasExternalValueBinding() )
    {
        
        doFormListening( true );
        
        if ( m_xAmbientForm.is() && m_xAmbientForm->isLoaded() )
            impl_connectDatabaseColumn_noNotify( false );
    }
}


void SAL_CALL OBoundControlModel::disposing(const com::sun::star::lang::EventObject& _rEvent) throw (RuntimeException)
{
    ControlModelLock aLock( *this );
    if ( _rEvent.Source == getField() )
    {
        resetField();
    }

    else if ( _rEvent.Source == m_xLabelControl )
    {
        Reference<XPropertySet> xOldValue = m_xLabelControl;
        m_xLabelControl = NULL;
        
        aLock.addPropertyNotification( PROPERTY_ID_CONTROLLABEL, makeAny( xOldValue ), makeAny( m_xLabelControl ) );
    }

    else if ( _rEvent.Source == m_xExternalBinding )
    {   
        disconnectExternalValueBinding( );
    }

    else if ( _rEvent.Source == m_xValidator )
    {   
        
        disconnectValidator( );
    }

    else
        OControlModel::disposing(_rEvent);
}


StringSequence SAL_CALL OBoundControlModel::getSupportedServiceNames() throw(RuntimeException)
{
    return ::comphelper::concatSequences(
        getAggregateServiceNames(),
        getSupportedServiceNames_Static()
    );
}

Sequence< OUString > SAL_CALL OBoundControlModel::getSupportedServiceNames_Static() throw( RuntimeException )
{
    Sequence< OUString > aOwnServiceNames( 1 );
    aOwnServiceNames[ 0 ] = "com.sun.star.form.DataAwareControlModel";
    return ::comphelper::concatSequences(
        OControlModel::getSupportedServiceNames_Static(),
        aOwnServiceNames
    );
}


void SAL_CALL OBoundControlModel::write( const Reference<stario::XObjectOutputStream>& _rxOutStream ) throw(stario::IOException, RuntimeException)
{
    OControlModel::write(_rxOutStream);
    osl::MutexGuard aGuard(m_aMutex);
    
    _rxOutStream->writeShort(0x0002);
    
    ::comphelper::operator<<( _rxOutStream, m_aControlSource);
    
    
    
    
    
    
    
}

void OBoundControlModel::defaultCommonProperties()
{
    Reference<com::sun::star::lang::XComponent> xComp(m_xLabelControl, UNO_QUERY);
    if (xComp.is())
        xComp->removeEventListener(static_cast<com::sun::star::lang::XEventListener*>(static_cast<XPropertyChangeListener*>(this)));
    m_xLabelControl = NULL;
}

void OBoundControlModel::readCommonProperties(const Reference<stario::XObjectInputStream>& _rxInStream)
{
    sal_Int32 nLen = _rxInStream->readLong();
    Reference<stario::XMarkableStream> xMark(_rxInStream, UNO_QUERY);
    DBG_ASSERT(xMark.is(), "OBoundControlModel::readCommonProperties : can only work with markable streams !");
    sal_Int32 nMark = xMark->createMark();
    
    Reference<stario::XPersistObject> xPersist;
    sal_Int32 nUsedFlag;
    nUsedFlag = _rxInStream->readLong();
    if (nUsedFlag)
        xPersist = _rxInStream->readObject();
    m_xLabelControl = m_xLabelControl.query( xPersist );
    Reference< XComponent > xComp( m_xLabelControl, UNO_QUERY );
    if (xComp.is())
        xComp->addEventListener(static_cast<com::sun::star::lang::XEventListener*>(static_cast<XPropertyChangeListener*>(this)));
    
    
    xMark->jumpToMark(nMark);
    _rxInStream->skipBytes(nLen);
    xMark->deleteMark(nMark);
}

void OBoundControlModel::writeCommonProperties(const Reference<stario::XObjectOutputStream>& _rxOutStream)
{
    Reference<stario::XMarkableStream> xMark(_rxOutStream, UNO_QUERY);
    DBG_ASSERT(xMark.is(), "OBoundControlModel::writeCommonProperties : can only work with markable streams !");
    sal_Int32 nMark = xMark->createMark();
    
    sal_Int32 nLen = 0;
    _rxOutStream->writeLong(nLen);
    
    Reference<stario::XPersistObject> xPersist(m_xLabelControl, UNO_QUERY);
    sal_Int32 nUsedFlag = 0;
    if (xPersist.is())
        nUsedFlag = 1;
    _rxOutStream->writeLong(nUsedFlag);
    if (xPersist.is())
        _rxOutStream->writeObject(xPersist);
    
    
    nLen = xMark->offsetToMark(nMark) - sizeof(nLen);
    xMark->jumpToMark(nMark);
    _rxOutStream->writeLong(nLen);
    xMark->jumpToFurthest();
    xMark->deleteMark(nMark);
}

void SAL_CALL OBoundControlModel::read( const Reference< stario::XObjectInputStream >& _rxInStream ) throw(stario::IOException, RuntimeException)
{
    OControlModel::read(_rxInStream);
    osl::MutexGuard aGuard(m_aMutex);
    sal_uInt16 nVersion = _rxInStream->readShort(); (void)nVersion;
    ::comphelper::operator>>( _rxInStream, m_aControlSource);
}

void OBoundControlModel::getFastPropertyValue(Any& rValue, sal_Int32 nHandle) const
{
    switch (nHandle)
    {
        case PROPERTY_ID_INPUT_REQUIRED:
            rValue <<= m_bInputRequired;
            break;
        case PROPERTY_ID_CONTROLSOURCEPROPERTY:
            rValue <<= m_sValuePropertyName;
            break;
        case PROPERTY_ID_CONTROLSOURCE:
            rValue <<= m_aControlSource;
            break;
        case PROPERTY_ID_BOUNDFIELD:
            rValue <<= getField();
            break;
        case PROPERTY_ID_CONTROLLABEL:
            if (!m_xLabelControl.is())
                rValue.clear();
            else
                rValue <<= m_xLabelControl;
            break;
        default:
            OControlModel::getFastPropertyValue(rValue, nHandle);
    }
}

sal_Bool OBoundControlModel::convertFastPropertyValue(
                                Any& _rConvertedValue, Any& _rOldValue,
                sal_Int32 _nHandle,
                                const Any& _rValue)
                throw (com::sun::star::lang::IllegalArgumentException)
{
    sal_Bool bModified(sal_False);
    switch (_nHandle)
    {
        case PROPERTY_ID_INPUT_REQUIRED:
            bModified = tryPropertyValue( _rConvertedValue, _rOldValue, _rValue, m_bInputRequired );
            break;
        case PROPERTY_ID_CONTROLSOURCE:
            bModified = tryPropertyValue(_rConvertedValue, _rOldValue, _rValue, m_aControlSource);
            break;
        case PROPERTY_ID_BOUNDFIELD:
            SAL_WARN("forms.component",  "OBoundControlModel::convertFastPropertyValue: BoundField should be a read-only property !" );
            throw com::sun::star::lang::IllegalArgumentException();
        case PROPERTY_ID_CONTROLLABEL:
            if (!_rValue.hasValue())
            {   
                _rConvertedValue = Any();
                getFastPropertyValue(_rOldValue, _nHandle);
                bModified = m_xLabelControl.is();
            }

            else
            {
                bModified = tryPropertyValue(_rConvertedValue, _rOldValue, _rValue, m_xLabelControl);
                if (!m_xLabelControl.is())
                    
                    _rOldValue.clear();
            }

            break;
        default:
            bModified = OControlModel::convertFastPropertyValue(_rConvertedValue, _rOldValue, _nHandle, _rValue);
    }
    return bModified;
}

Any OBoundControlModel::getPropertyDefaultByHandle( sal_Int32 _nHandle ) const
{
    Any aDefault;
    switch ( _nHandle )
    {
        case PROPERTY_ID_INPUT_REQUIRED:
            aDefault <<= sal_Bool( sal_True );
            break;
        case PROPERTY_ID_CONTROLSOURCE:
            aDefault <<= OUString();
            break;
        case PROPERTY_ID_CONTROLLABEL:
            aDefault <<= Reference< XPropertySet >();
            break;
    }
    return aDefault;
}

void OBoundControlModel::setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const Any& rValue ) throw (Exception)
{
    switch (nHandle)
    {
        case PROPERTY_ID_INPUT_REQUIRED:
            OSL_VERIFY( rValue >>= m_bInputRequired );
            break;
        case PROPERTY_ID_CONTROLSOURCE:
            OSL_VERIFY( rValue >>= m_aControlSource );
            break;
        case PROPERTY_ID_BOUNDFIELD:
            SAL_WARN("forms.component", "OBoundControlModel::setFastPropertyValue_NoBroadcast : BoundField should be a read-only property !");
            throw com::sun::star::lang::IllegalArgumentException();
        case PROPERTY_ID_CONTROLLABEL:
        {
            if ( rValue.hasValue() && ( rValue.getValueTypeClass() != TypeClass_INTERFACE ) )
                throw com::sun::star::lang::IllegalArgumentException();
            Reference< XInterface > xNewValue( rValue, UNO_QUERY );
            if ( !xNewValue.is() )
            {   
                Reference< XComponent > xComp( m_xLabelControl, UNO_QUERY );
                if ( xComp.is() )
                    xComp->removeEventListener( static_cast< XPropertyChangeListener* >( this ) );
                m_xLabelControl = NULL;
                break;
            }

            Reference< XControlModel >  xAsModel        ( xNewValue,        UNO_QUERY );
            Reference< XServiceInfo >   xAsServiceInfo  ( xAsModel,         UNO_QUERY );
            Reference< XPropertySet >   xAsPropSet      ( xAsServiceInfo,   UNO_QUERY );
            Reference< XChild >         xAsChild        ( xAsPropSet,       UNO_QUERY );
            if ( !xAsChild.is() || !xAsServiceInfo->supportsService( m_aLabelServiceName ) )
            {
                throw com::sun::star::lang::IllegalArgumentException();
            }

            
            Reference<XChild> xCont;
            query_interface(static_cast<XWeak*>(this), xCont);
            Reference< XInterface > xMyTopLevel = xCont->getParent();
            while (xMyTopLevel.is())
            {
                Reference<XForm> xAsForm(xMyTopLevel, UNO_QUERY);
                if (!xAsForm.is())
                    
                    break;
                Reference<XChild> xLoopAsChild(xMyTopLevel, UNO_QUERY);
                xMyTopLevel = xLoopAsChild.is() ? xLoopAsChild->getParent() : Reference< XInterface >();
            }

            Reference< XInterface > xNewTopLevel = xAsChild->getParent();
            while (xNewTopLevel.is())
            {
                Reference<XForm> xAsForm(xNewTopLevel, UNO_QUERY);
                if (!xAsForm.is())
                    break;
                Reference<XChild> xLoopAsChild(xNewTopLevel, UNO_QUERY);
                xNewTopLevel = xLoopAsChild.is() ? xLoopAsChild->getParent() : Reference< XInterface >();
            }

            if (xNewTopLevel != xMyTopLevel)
            {
                
                throw com::sun::star::lang::IllegalArgumentException();
            }

            m_xLabelControl = xAsPropSet;
            Reference<com::sun::star::lang::XComponent> xComp(m_xLabelControl, UNO_QUERY);
            if (xComp.is())
                xComp->addEventListener(static_cast<com::sun::star::lang::XEventListener*>(static_cast<XPropertyChangeListener*>(this)));
        }

        break;
        default:
            OControlModel::setFastPropertyValue_NoBroadcast(nHandle, rValue );
    }
}


void SAL_CALL OBoundControlModel::propertyChange( const PropertyChangeEvent& evt ) throw(RuntimeException)
{
    
    if ( evt.PropertyName.equals( PROPERTY_VALUE ) )
    {
        OSL_ENSURE( evt.Source == getField(), "OBoundControlModel::propertyChange: value changes from components other than our database column?" );
        osl::MutexGuard aGuard(m_aMutex);
        if ( m_bForwardValueChanges && m_xColumn.is() )
            transferDbValueToControl();
    }

    else
    {
        OSL_ENSURE( evt.Source == m_xExternalBinding, "OBoundControlModel::propertyChange: where did this come from?" );
        
        OUString sBindingControlledProperty;
        bool bForwardToLabelControl = false;
        if ( evt.PropertyName.equals( PROPERTY_READONLY ) )
        {
            sBindingControlledProperty = PROPERTY_READONLY;
        }

        else if ( evt.PropertyName.equals( PROPERTY_RELEVANT ) )
        {
            sBindingControlledProperty = PROPERTY_ENABLED;
            bForwardToLabelControl = true;
        }

        else
            return;
        try
        {
            setPropertyValue( sBindingControlledProperty, evt.NewValue );
            if ( bForwardToLabelControl && m_xLabelControl.is() )
                m_xLabelControl->setPropertyValue( sBindingControlledProperty, evt.NewValue );
        }

        catch( const Exception& )
        {
            SAL_WARN("forms.component",  "OBoundControlModel::propertyChange: could not adjust my binding-controlled property!");
            DBG_UNHANDLED_EXCEPTION();
        }

    }
}

void SAL_CALL OBoundControlModel::onRowSetChanged( const EventObject& /*i_Event*/ ) throw (RuntimeException)
{
    ControlModelLock aLock( *this );
    FieldChangeNotifier aBoundFieldNotifier( aLock );
    
    if ( hasField() )
        impl_disconnectDatabaseColumn_noNotify();
    
    if ( isFormListening() )
        doFormListening( false );
    
    impl_determineAmbientForm_nothrow();
    
    doFormListening( true );
    
    if ( m_xAmbientForm.is() && m_xAmbientForm->isLoaded() )
        impl_connectDatabaseColumn_noNotify( false );
}


void SAL_CALL OBoundControlModel::addUpdateListener(const Reference<XUpdateListener>& _rxListener) throw(RuntimeException)
{
    m_aUpdateListeners.addInterface(_rxListener);
}

void SAL_CALL OBoundControlModel::removeUpdateListener(const Reference< XUpdateListener>& _rxListener) throw(RuntimeException)
{
    m_aUpdateListeners.removeInterface(_rxListener);
}

sal_Bool SAL_CALL OBoundControlModel::commit() throw(RuntimeException)
{
    ControlModelLock aLock( *this );
    OSL_PRECOND( m_bCommitable, "OBoundControlModel::commit: invalid call (I'm not commitable !) " );
    if ( hasExternalValueBinding() )
    {
        
        
        
        if ( m_sValuePropertyName.isEmpty() )
            
            
            transferControlValueToExternal( aLock );
        return sal_True;
    }

    OSL_ENSURE( !hasExternalValueBinding(), "OBoundControlModel::commit: control flow broken!" );
        
    if ( !hasField() )
        return sal_True;
    ::cppu::OInterfaceIteratorHelper aIter( m_aUpdateListeners );
    EventObject aEvent;
    aEvent.Source = static_cast< XWeak* >( this );
    sal_Bool bSuccess = sal_True;
    aLock.release();
    
    while (aIter.hasMoreElements() && bSuccess)
        bSuccess = static_cast< XUpdateListener* >( aIter.next() )->approveUpdate( aEvent );
    
    aLock.acquire();
    if ( bSuccess )
    {
        try
        {
            if ( m_xColumnUpdate.is() )
                bSuccess = commitControlValueToDbColumn( false );
        }

        catch(const Exception&)
        {
            bSuccess = sal_False;
        }

    }

    if ( bSuccess )
    {
        aLock.release();
        m_aUpdateListeners.notifyEach( &XUpdateListener::updated, aEvent );
    }
    return bSuccess;
}

void OBoundControlModel::resetField()
{
    m_xColumnUpdate.clear();
    m_xColumn.clear();
    m_xField.clear();
    m_nFieldType = DataType::OTHER;
}

sal_Bool OBoundControlModel::connectToField(const Reference<XRowSet>& rForm)
{
    OSL_PRECOND( !hasExternalValueBinding(), "OBoundControlModel::connectToField: invalid call (have an external binding)!" );
    
    if (rForm.is() && getConnection(rForm).is())
    {
        
        m_xCursor = rForm;
        Reference<XPropertySet> xFieldCandidate;
        if (m_xCursor.is())
        {
            Reference<XColumnsSupplier> xColumnsSupplier(m_xCursor, UNO_QUERY);
            DBG_ASSERT(xColumnsSupplier.is(), "OBoundControlModel::connectToField : the row set should support the com::sun::star::sdb::ResultSet service !");
            if (xColumnsSupplier.is())
            {
                Reference<XNameAccess> xColumns(xColumnsSupplier->getColumns(), UNO_QUERY);
                if (xColumns.is() && xColumns->hasByName(m_aControlSource))
                {
                    OSL_VERIFY( xColumns->getByName(m_aControlSource) >>= xFieldCandidate );
                }

            }

        }

        try
        {
            sal_Int32 nFieldType = DataType::OTHER;
            if ( xFieldCandidate.is() )
            {
                xFieldCandidate->getPropertyValue( PROPERTY_FIELDTYPE ) >>= nFieldType;
                if ( approveDbColumnType( nFieldType ) )
                    impl_setField_noNotify( xFieldCandidate );
            }

            else
                impl_setField_noNotify( NULL );
            if ( m_xField.is() )
            {
                if ( m_xField->getPropertySetInfo()->hasPropertyByName( PROPERTY_VALUE ) )
                {
                    m_nFieldType = nFieldType;
                    
                    m_xField->addPropertyChangeListener( PROPERTY_VALUE, this );
                    m_xColumnUpdate = Reference< XColumnUpdate >( m_xField, UNO_QUERY );
                    m_xColumn = Reference< XColumn >( m_xField, UNO_QUERY );
                    sal_Int32 nNullableFlag = ColumnValue::NO_NULLS;
                    m_xField->getPropertyValue(PROPERTY_ISNULLABLE) >>= nNullableFlag;
                    m_bRequired = (ColumnValue::NO_NULLS == nNullableFlag);
                        
                }

                else
                {
                    SAL_WARN("forms.component", "OBoundControlModel::connectToField: property " << PROPERTY_VALUE << " not supported!");
                    impl_setField_noNotify( NULL );
                }

            }

        }

        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
            resetField();
        }

    }
    return hasField();
}

void OBoundControlModel::initFromField( const Reference< XRowSet >& _rxRowSet )
{
    
    if ( hasField() && _rxRowSet.is() )
    {
        if ( !_rxRowSet->isBeforeFirst() && !_rxRowSet->isAfterLast() )
            transferDbValueToControl();
        else
            
            
            resetNoBroadcast();
    }
}

sal_Bool OBoundControlModel::approveDbColumnType(sal_Int32 _nColumnType)
{
    OSL_PRECOND( !hasExternalValueBinding(), "OBoundControlModel::approveDbColumnType: invalid call (have an external binding)!" );
    if ((_nColumnType == DataType::BINARY) || (_nColumnType == DataType::VARBINARY)
        || (_nColumnType == DataType::LONGVARBINARY) || (_nColumnType == DataType::OTHER)
        || (_nColumnType == DataType::OBJECT) || (_nColumnType == DataType::DISTINCT)
        || (_nColumnType == DataType::STRUCT) || (_nColumnType == DataType::ARRAY)
        || (_nColumnType == DataType::BLOB) /*|| (_nColumnType == DataType::CLOB)*/
        || (_nColumnType == DataType::REF) || (_nColumnType == DataType::SQLNULL))
        return sal_False;
    return sal_True;
}

void OBoundControlModel::impl_determineAmbientForm_nothrow()
{
    Reference< XInterface > xParent( const_cast< OBoundControlModel* >( this )->getParent() );
    m_xAmbientForm.set( xParent, UNO_QUERY );
    if ( !m_xAmbientForm.is() )
    {
        Reference< XRowSetSupplier > xSupRowSet( xParent, UNO_QUERY );
        if ( xSupRowSet.is() )
            m_xAmbientForm.set( xSupRowSet->getRowSet(), UNO_QUERY );
    }
}

void OBoundControlModel::impl_connectDatabaseColumn_noNotify( bool _bFromReload )
{
    OSL_PRECOND( !hasExternalValueBinding(), "OBoundControlModel::impl_connectDatabaseColumn_noNotify: not to be called with an external value binding!" );
    
    DBG_ASSERT( !( hasField() && !_bFromReload ),
        "OBoundControlModel::impl_connectDatabaseColumn_noNotify: the form is just *loaded*, but we already have a field!" );
    (void)_bFromReload;
    Reference< XRowSet > xRowSet( m_xAmbientForm, UNO_QUERY );
    OSL_ENSURE( xRowSet.is(), "OBoundControlModel::impl_connectDatabaseColumn_noNotify: no row set!" );
    if ( !xRowSet.is() )
        return;
    if ( !hasField() )
    {
        
        connectToField( xRowSet );
    }

    
    
    m_bForwardValueChanges = sal_True;
    
    m_bLoaded = sal_True;
    onConnectedDbColumn( xRowSet );
    
    if ( hasField() )
        initFromField( xRowSet );
}

void OBoundControlModel::impl_disconnectDatabaseColumn_noNotify()
{
    OSL_PRECOND( !hasExternalValueBinding(), "OBoundControlModel::impl_disconnectDatabaseColumn_noNotify: not to be called with an external value binding!" );
    
    onDisconnectedDbColumn();
    if ( hasField() )
    {
        getField()->removePropertyChangeListener( PROPERTY_VALUE, this );
        resetField();
    }

    m_xCursor = NULL;
    m_bLoaded = sal_False;
}


void SAL_CALL OBoundControlModel::loaded( const EventObject& _rEvent ) throw(RuntimeException)
{
    ControlModelLock aLock( *this );
    FieldChangeNotifier aBoundFieldNotifier( aLock );
    OSL_ENSURE( _rEvent.Source == m_xAmbientForm, "OBoundControlModel::loaded: where does this come from?" );
    (void)_rEvent;
    OSL_PRECOND( !hasExternalValueBinding(), "OBoundControlModel::loaded: we should never reach this with an external value binding!" );
    if ( hasExternalValueBinding() )
        return;
    impl_connectDatabaseColumn_noNotify( false );
}

void SAL_CALL OBoundControlModel::unloaded( const com::sun::star::lang::EventObject& /*aEvent*/ ) throw(RuntimeException)
{
    OSL_PRECOND( !hasExternalValueBinding(), "OBoundControlModel::unloaded: we should never reach this with an external value binding!" );
}

void SAL_CALL OBoundControlModel::reloading( const com::sun::star::lang::EventObject& /*aEvent*/ ) throw(RuntimeException)
{
    OSL_PRECOND( !hasExternalValueBinding(), "OBoundControlModel::reloading: we should never reach this with an external value binding!" );
    if ( hasExternalValueBinding() )
        return;
    osl::MutexGuard aGuard(m_aMutex);
    m_bForwardValueChanges = sal_False;
}

void SAL_CALL OBoundControlModel::unloading(const com::sun::star::lang::EventObject& /*aEvent*/) throw(RuntimeException)
{
    ControlModelLock aLock( *this );
    FieldChangeNotifier aBoundFieldNotifier( aLock );
    OSL_PRECOND( !hasExternalValueBinding(), "OBoundControlModel::unloading: we should never reach this with an external value binding!" );
    if ( hasExternalValueBinding() )
        return;
    impl_disconnectDatabaseColumn_noNotify();
}

void SAL_CALL OBoundControlModel::reloaded( const EventObject& _rEvent ) throw(RuntimeException)
{
    ControlModelLock aLock( *this );
    FieldChangeNotifier aBoundFieldNotifier( aLock );
    OSL_ENSURE( _rEvent.Source == m_xAmbientForm, "OBoundControlModel::reloaded: where does this come from?" );
    (void)_rEvent;
    OSL_PRECOND( !hasExternalValueBinding(), "OBoundControlModel::reloaded: we should never reach this with an external value binding!" );
    if ( hasExternalValueBinding() )
        return;
    impl_connectDatabaseColumn_noNotify( true );
}

void OBoundControlModel::setControlValue( const Any& _rValue, ValueChangeInstigator _eInstigator )
{
    m_eControlValueChangeInstigator = _eInstigator;
    doSetControlValue( _rValue );
    m_eControlValueChangeInstigator = eOther;
}

void OBoundControlModel::doSetControlValue( const Any& _rValue )
{
    OSL_PRECOND( m_xAggregateFastSet.is() && m_xAggregateSet.is(),
        "OBoundControlModel::doSetControlValue: invalid aggregate !" );
    OSL_PRECOND( !m_sValuePropertyName.isEmpty() || ( m_nValuePropertyAggregateHandle != -1 ),
        "OBoundControlModel::doSetControlValue: please override if you have own value property handling!" );
    try
    {
        
        
        
        MutexRelease aRelease( m_aMutex );
        if ( ( m_nValuePropertyAggregateHandle != -1 ) && m_xAggregateFastSet.is() )
        {
            m_xAggregateFastSet->setFastPropertyValue( m_nValuePropertyAggregateHandle, _rValue );
        }

        else if ( !m_sValuePropertyName.isEmpty() && m_xAggregateSet.is() )
        {
            m_xAggregateSet->setPropertyValue( m_sValuePropertyName, _rValue );
        }

    }

    catch( const Exception& )
    {
        SAL_WARN("forms.component",  "OBoundControlModel::doSetControlValue: caught an exception!");
        DBG_UNHANDLED_EXCEPTION();
    }
}

void OBoundControlModel::onConnectedValidator( )
{
    try
    {
        
        
        
        Reference< XPropertySetInfo > xAggregatePropertyInfo;
        if ( m_xAggregateSet.is() )
            xAggregatePropertyInfo = m_xAggregateSet->getPropertySetInfo();
        if ( xAggregatePropertyInfo.is() && xAggregatePropertyInfo->hasPropertyByName( PROPERTY_ENFORCE_FORMAT ) )
            m_xAggregateSet->setPropertyValue( PROPERTY_ENFORCE_FORMAT, makeAny( (sal_Bool)sal_False ) );
    }

    catch( const Exception& )
    {
        SAL_WARN("forms.component",  "OBoundControlModel::onConnectedValidator: caught an exception!");
        DBG_UNHANDLED_EXCEPTION();
    }

    recheckValidity( false );
}

void OBoundControlModel::onDisconnectedValidator( )
{
    try
    {
        Reference< XPropertySetInfo > xAggregatePropertyInfo;
        if ( m_xAggregateSet.is() )
            xAggregatePropertyInfo = m_xAggregateSet->getPropertySetInfo();
        if ( xAggregatePropertyInfo.is() && xAggregatePropertyInfo->hasPropertyByName( PROPERTY_ENFORCE_FORMAT ) )
            m_xAggregateSet->setPropertyValue( PROPERTY_ENFORCE_FORMAT, makeAny( (sal_Bool)sal_True ) );
    }

    catch( const Exception& )
    {
        SAL_WARN("forms.component",  "OBoundControlModel::onDisconnectedValidator: caught an exception!");
        DBG_UNHANDLED_EXCEPTION();
    }

    recheckValidity( false );
}

void OBoundControlModel::onConnectedExternalValue( )
{
    calculateExternalValueType();
}

void OBoundControlModel::onDisconnectedExternalValue( )
{
}

void OBoundControlModel::onConnectedDbColumn( const Reference< XInterface >& /*_rxForm*/ )
{
    OSL_PRECOND( !hasExternalValueBinding(), "OBoundControlModel::onConnectedDbColumn: how this? There's an external value binding!" );
}

void OBoundControlModel::onDisconnectedDbColumn()
{
    OSL_PRECOND( !hasExternalValueBinding(), "OBoundControlModel::onDisconnectedDbColumn: how this? There's an external value binding!" );
}


Any OBoundControlModel::getDefaultForReset() const
{
    return Any();
}

void OBoundControlModel::resetNoBroadcast()
{
    setControlValue( getDefaultForReset(), eOther );
}

void OBoundControlModel::addResetListener(const Reference<XResetListener>& l) throw (RuntimeException)
{
    m_aResetHelper.addResetListener( l );
}

void OBoundControlModel::removeResetListener(const Reference<XResetListener>& l) throw (RuntimeException)
{
    m_aResetHelper.removeResetListener( l );
}

void OBoundControlModel::reset() throw (RuntimeException)
{
    if ( !m_aResetHelper.approveReset() )
       return;
    ControlModelLock aLock( *this );
    
    sal_Bool bIsNewRecord = sal_False;
    Reference<XPropertySet> xSet( m_xCursor, UNO_QUERY );
    if ( xSet.is() )
    {
        try
        {
            xSet->getPropertyValue( PROPERTY_ISNEW ) >>= bIsNewRecord;
        }

        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }

    }

    
    sal_Bool bInvalidCursorPosition = sal_True;
    try
    {
        bInvalidCursorPosition =    m_xCursor.is()
                                &&  (  m_xCursor->isAfterLast()
                                    || m_xCursor->isBeforeFirst()
                                    )
                                &&  !bIsNewRecord;
    }

    catch( const SQLException& )
    {
        SAL_WARN("forms.component",  "OBoundControlModel::reset: caught an SQL exception!" );
        DBG_UNHANDLED_EXCEPTION();
    }

    
    sal_Bool bSimpleReset =
                        (   !m_xColumn.is()                     
                        ||  (   m_xCursor.is()                  
                            &&  bInvalidCursorPosition
                            )
                        ||  hasExternalValueBinding()           
                        );
    if ( !bSimpleReset )
    {
        
        
        
        
        
        sal_Bool bIsNull = sal_True;
        
        try
        {
            
            
            
            
            sal_Int32 nFieldType = DataType::OBJECT;
            getField()->getPropertyValue( PROPERTY_FIELDTYPE ) >>= nFieldType;
            if  (  ( nFieldType == DataType::BINARY        )
                || ( nFieldType == DataType::VARBINARY     )
                || ( nFieldType == DataType::LONGVARBINARY )
                || ( nFieldType == DataType::OBJECT        )
                /*|| ( nFieldType == DataType::CLOB          )*/
                )
                m_xColumn->getBinaryStream();
            else if ( nFieldType == DataType::BLOB          )
                m_xColumn->getBlob();
            else
                m_xColumn->getString();
            bIsNull = m_xColumn->wasNull();
        }

        catch(const Exception&)
        {
            SAL_WARN("forms.component", "OBoundControlModel::reset: this should have succeeded in all cases!");
            DBG_UNHANDLED_EXCEPTION();
        }

        sal_Bool bNeedValueTransfer = sal_True;
        if ( bIsNull )
        {
            if ( bIsNewRecord )
            {
                
                resetNoBroadcast();
                
                commitControlValueToDbColumn( true );
                bNeedValueTransfer = sal_False;
            }

        }

        if ( bNeedValueTransfer )
            transferDbValueToControl();
    }

    else
    {
        resetNoBroadcast();
        
        if ( hasExternalValueBinding() )
            transferControlValueToExternal( aLock );
    }

    
    if ( hasValidator() )
        recheckValidity( true );
    aLock.release();
    m_aResetHelper.notifyResetted();
}

void OBoundControlModel::impl_setField_noNotify( const Reference< XPropertySet>& _rxField )
{
    DBG_ASSERT( !hasExternalValueBinding(), "OBoundControlModel::impl_setField_noNotify: We have an external value binding!" );
    m_xField = _rxField;
}

sal_Bool OBoundControlModel::impl_approveValueBinding_nolock( const Reference< XValueBinding >& _rxBinding )
{
    if ( !_rxBinding.is() )
        return sal_False;
    Sequence< Type > aTypeCandidates;
    {
        
        ::osl::MutexGuard aGuard( m_aMutex );
        aTypeCandidates = getSupportedBindingTypes();
        
    }

    for (   const Type* pType = aTypeCandidates.getConstArray();
            pType != aTypeCandidates.getConstArray() + aTypeCandidates.getLength();
            ++pType
        )
    {
        if ( _rxBinding->supportsType( *pType ) )
            return sal_True;
    }
    return sal_False;
}

void OBoundControlModel::connectExternalValueBinding(
        const Reference< XValueBinding >& _rxBinding, ControlModelLock& _rInstanceLock )
{
    OSL_PRECOND( _rxBinding.is(), "OBoundControlModel::connectExternalValueBinding: invalid binding instance!" );
    OSL_PRECOND( !hasExternalValueBinding( ), "OBoundControlModel::connectExternalValueBinding: precond not met (currently have a binding)!" );
    
    if ( hasField() )
        impl_disconnectDatabaseColumn_noNotify();
    
    
    if ( isFormListening() )
        doFormListening( false );
    
    m_xExternalBinding = _rxBinding;
    
    onConnectedExternalValue();
    try
    {
        
        Reference< XModifyBroadcaster > xModifiable( m_xExternalBinding, UNO_QUERY );
        if ( xModifiable.is() )
            xModifiable->addModifyListener( this );
        
        
        Reference< XPropertySet > xBindingProps( m_xExternalBinding, UNO_QUERY );
        Reference< XPropertySetInfo > xBindingPropsInfo( xBindingProps.is() ? xBindingProps->getPropertySetInfo() : Reference< XPropertySetInfo >() );
        if ( xBindingPropsInfo.is() )
        {
            if ( xBindingPropsInfo->hasPropertyByName( PROPERTY_READONLY ) )
            {
                xBindingProps->addPropertyChangeListener( PROPERTY_READONLY, this );
                m_bBindingControlsRO = sal_True;
            }

            if ( xBindingPropsInfo->hasPropertyByName( PROPERTY_RELEVANT ) )
            {
                xBindingProps->addPropertyChangeListener( PROPERTY_RELEVANT, this );
                m_bBindingControlsEnable = sal_True;
            }

        }

    }

    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    
    transferExternalValueToControl( _rInstanceLock );
    
    
    if ( m_bSupportsValidation )
    {
        try
        {
            Reference< XValidator > xAsValidator( _rxBinding, UNO_QUERY );
            if ( xAsValidator.is() )
                setValidator( xAsValidator );
        }

        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }

    }
}

void OBoundControlModel::disconnectExternalValueBinding( )
{
    try
    {
        
        Reference< XModifyBroadcaster > xModifiable( m_xExternalBinding, UNO_QUERY );
        if ( xModifiable.is() )
            xModifiable->removeModifyListener( this );
        
        Reference< XPropertySet > xBindingProps( m_xExternalBinding, UNO_QUERY );
        if ( m_bBindingControlsRO )
            xBindingProps->removePropertyChangeListener( PROPERTY_READONLY, this );
        if ( m_bBindingControlsEnable )
            xBindingProps->removePropertyChangeListener( PROPERTY_RELEVANT, this );
    }

    catch( const Exception& )
    {
        SAL_WARN("forms.component",  "OBoundControlModel::disconnectExternalValueBinding: caught an exception!");
        DBG_UNHANDLED_EXCEPTION();
    }

    
    if ( ( m_xExternalBinding == m_xValidator ) && m_xValidator.is() )
        disconnectValidator( );
    
    m_xExternalBinding.clear();
    
    
    doFormListening( true );
    
    if ( m_xAmbientForm.is() && m_xAmbientForm->isLoaded() )
        impl_connectDatabaseColumn_noNotify( false );
    
    onDisconnectedExternalValue();
}

void SAL_CALL OBoundControlModel::setValueBinding( const Reference< XValueBinding >& _rxBinding ) throw (IncompatibleTypesException, RuntimeException)
{
    OSL_PRECOND( m_bSupportsExternalBinding, "OBoundControlModel::setValueBinding: How did you reach this method?" );
        
        
    
    if ( _rxBinding.is() && !impl_approveValueBinding_nolock( _rxBinding ) )
    {
        throw IncompatibleTypesException(
            FRM_RES_STRING( RID_STR_INCOMPATIBLE_TYPES ),
            *this
        );
    }

    ControlModelLock aLock( *this );
    
    
    FieldChangeNotifier aBoundFieldNotifier( aLock );
    
    if ( hasExternalValueBinding() )
        disconnectExternalValueBinding( );
    
    if ( _rxBinding.is() )
        connectExternalValueBinding( _rxBinding, aLock );
}

Reference< XValueBinding > SAL_CALL OBoundControlModel::getValueBinding(  ) throw (RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    OSL_PRECOND( m_bSupportsExternalBinding, "OBoundControlModel::getValueBinding: How did you reach this method?" );
        
        
    return m_xExternalBinding;
}

void SAL_CALL OBoundControlModel::modified( const EventObject& _rEvent ) throw ( RuntimeException )
{
    ControlModelLock aLock( *this );
    OSL_PRECOND( hasExternalValueBinding(), "OBoundControlModel::modified: Where did this come from?" );
    if ( !m_bTransferingValue && ( m_xExternalBinding == _rEvent.Source ) && m_xExternalBinding.is() )
    {
        transferExternalValueToControl( aLock );
    }
}

void OBoundControlModel::transferDbValueToControl( )
{
    try
    {
        setControlValue( translateDbColumnToControlValue(), eDbColumnBinding );
    }

    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

void OBoundControlModel::transferExternalValueToControl( ControlModelLock& _rInstanceLock )
{
        Reference< XValueBinding > xExternalBinding( m_xExternalBinding );
        Type aValueExchangeType( getExternalValueType() );
        _rInstanceLock.release();
        
        Any aExternalValue;
        try
        {
            aExternalValue = xExternalBinding->getValue( aValueExchangeType );
        }

        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        
        _rInstanceLock.acquire();
        setControlValue( translateExternalValueToControlValue( aExternalValue ), eExternalBinding );
}

void OBoundControlModel::transferControlValueToExternal( ControlModelLock& _rInstanceLock )
{
    OSL_PRECOND( m_bSupportsExternalBinding && hasExternalValueBinding(),
        "OBoundControlModel::transferControlValueToExternal: precondition not met!" );
    if ( m_xExternalBinding.is() )
    {
        Any aExternalValue( translateControlValueToExternalValue() );
        m_bTransferingValue = sal_True;
        _rInstanceLock.release();
         
        try
        {
            m_xExternalBinding->setValue( aExternalValue );
        }

        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }

        
        _rInstanceLock.acquire();
        m_bTransferingValue = sal_False;
    }
}

Sequence< Type > OBoundControlModel::getSupportedBindingTypes()
{
    return Sequence< Type >( &m_aValuePropertyType, 1 );
}

void OBoundControlModel::calculateExternalValueType()
{
    m_aExternalValueType = Type();
    if ( !m_xExternalBinding.is() )
        return;
    Sequence< Type > aTypeCandidates( getSupportedBindingTypes() );
    for (   const Type* pTypeCandidate = aTypeCandidates.getConstArray();
            pTypeCandidate != aTypeCandidates.getConstArray() + aTypeCandidates.getLength();
            ++pTypeCandidate
        )
    {
        if ( m_xExternalBinding->supportsType( *pTypeCandidate ) )
        {
            m_aExternalValueType = *pTypeCandidate;
            break;
        }
    }
}

Any OBoundControlModel::translateExternalValueToControlValue( const Any& _rExternalValue ) const
{
    OSL_PRECOND( m_bSupportsExternalBinding && hasExternalValueBinding(),
        "OBoundControlModel::translateExternalValueToControlValue: precondition not met!" );
    Any aControlValue( _rExternalValue );
    
    
    if ( !aControlValue.hasValue() && !m_bValuePropertyMayBeVoid )
        aControlValue.setValue( NULL, m_aValuePropertyType );
    
    return aControlValue;
}

Any OBoundControlModel::translateControlValueToExternalValue( ) const
{
    return getControlValue( );
}

Any OBoundControlModel::translateControlValueToValidatableValue( ) const
{
    OSL_PRECOND( m_xValidator.is(), "OBoundControlModel::translateControlValueToValidatableValue: no validator, so why should I?" );
    if ( ( m_xValidator == m_xExternalBinding ) && m_xValidator.is() )
        return translateControlValueToExternalValue();
    return getControlValue();
}

Any OBoundControlModel::getControlValue( ) const
{
    OSL_PRECOND( m_xAggregateFastSet.is() && m_xAggregateSet.is(),
        "OBoundControlModel::getControlValue: invalid aggregate !" );
    OSL_PRECOND( !m_sValuePropertyName.isEmpty() || ( m_nValuePropertyAggregateHandle != -1 ),
        "OBoundControlModel::getControlValue: please override if you have own value property handling!" );
    
    Any aControlValue;
    if ( ( m_nValuePropertyAggregateHandle != -1 ) && m_xAggregateFastSet.is() )
    {
        aControlValue = m_xAggregateFastSet->getFastPropertyValue( m_nValuePropertyAggregateHandle );
    }

    else if ( !m_sValuePropertyName.isEmpty() && m_xAggregateSet.is() )
    {
        aControlValue = m_xAggregateSet->getPropertyValue( m_sValuePropertyName );
    }
    return aControlValue;
}

void OBoundControlModel::connectValidator( const Reference< XValidator >& _rxValidator )
{
    OSL_PRECOND( _rxValidator.is(), "OBoundControlModel::connectValidator: invalid validator instance!" );
    OSL_PRECOND( !hasValidator( ), "OBoundControlModel::connectValidator: precond not met (have a validator currently)!" );
    m_xValidator = _rxValidator;

    
    if ( m_xValidator.is() )
    {
        try
        {
            m_xValidator->addValidityConstraintListener( this );
        }

        catch( const RuntimeException& )
        {
        }
    }
    onConnectedValidator( );
}

void OBoundControlModel::disconnectValidator( )
{
    OSL_PRECOND( hasValidator( ), "OBoundControlModel::connectValidator: precond not met (don't have a validator currently)!" );

    
    if ( m_xValidator.is() )
    {
        try
        {
            m_xValidator->removeValidityConstraintListener( this );
        }

        catch( const RuntimeException& )
        {
        }
    }

    m_xValidator.clear();

    onDisconnectedValidator( );
}

void SAL_CALL OBoundControlModel::setValidator( const Reference< XValidator >& _rxValidator ) throw (VetoException,RuntimeException)
{
    ::osl::ClearableMutexGuard aGuard( m_aMutex );
    OSL_PRECOND( m_bSupportsValidation, "OBoundControlModel::setValidator: How did you reach this method?" );
    
    

    
    if ( _rxValidator == m_xValidator )
        return;

    if ( m_xValidator.is() && ( m_xValidator == m_xExternalBinding ) )
        throw VetoException(
            FRM_RES_STRING( RID_STR_INVALID_VALIDATOR ),
            *this
        );

    
    if ( hasValidator() )
        disconnectValidator( );

    
    if ( _rxValidator.is() )
        connectValidator( _rxValidator );
}

Reference< XValidator > SAL_CALL OBoundControlModel::getValidator(  ) throw (RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    OSL_PRECOND( m_bSupportsValidation, "OBoundControlModel::getValidator: How did you reach this method?" );
        
        

    return m_xValidator;
}

void SAL_CALL OBoundControlModel::validityConstraintChanged( const EventObject& /*Source*/ ) throw (RuntimeException)
{
    ::osl::ClearableMutexGuard aGuard( m_aMutex );
    OSL_PRECOND( m_bSupportsValidation, "OBoundControlModel::validityConstraintChanged: How did you reach this method?" );
        
        

    recheckValidity( false );
}

sal_Bool SAL_CALL OBoundControlModel::isValid(  ) throw (RuntimeException)
{
    return m_bIsCurrentValueValid;
}

::com::sun::star::uno::Any OBoundControlModel::getCurrentFormComponentValue() const
{
    if ( hasValidator() )
        return translateControlValueToValidatableValue();
    return getControlValue();
}

Any SAL_CALL OBoundControlModel::getCurrentValue(  ) throw (RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    return getCurrentFormComponentValue();
}

void SAL_CALL OBoundControlModel::addFormComponentValidityListener( const Reference< validation::XFormComponentValidityListener >& Listener ) throw (NullPointerException, RuntimeException)
{
    if ( Listener.is() )
        m_aFormComponentListeners.addInterface( Listener );
}

void SAL_CALL OBoundControlModel::removeFormComponentValidityListener( const Reference< validation::XFormComponentValidityListener >& Listener ) throw (NullPointerException, RuntimeException)
{
    if ( Listener.is() )
        m_aFormComponentListeners.removeInterface( Listener );
}

void OBoundControlModel::recheckValidity( bool _bForceNotification )
{
    try
    {
        sal_Bool bIsCurrentlyValid = sal_True;
        if ( hasValidator() )
            bIsCurrentlyValid = m_xValidator->isValid( translateControlValueToValidatableValue() );

        if ( ( bIsCurrentlyValid != m_bIsCurrentValueValid ) || _bForceNotification )
        {
            m_bIsCurrentValueValid = bIsCurrentlyValid;

            
            MutexRelease aRelease( m_aMutex );
            m_aFormComponentListeners.notifyEach( &validation::XFormComponentValidityListener::componentValidityChanged, EventObject( *this ) );
        }

    }

    catch( const Exception& )
    {
        SAL_WARN("forms.component",  "OBoundControlModel::recheckValidity: caught an exception!");
        DBG_UNHANDLED_EXCEPTION();
    }
}

void OBoundControlModel::describeFixedProperties( Sequence< Property >& _rProps ) const
{
    BEGIN_DESCRIBE_PROPERTIES( 5, OControlModel )
        DECL_PROP1      ( CONTROLSOURCE,           OUString,     BOUND );
        DECL_IFACE_PROP3( BOUNDFIELD,               XPropertySet,       BOUND, READONLY, TRANSIENT );
        DECL_IFACE_PROP2( CONTROLLABEL,             XPropertySet,       BOUND, MAYBEVOID );
        DECL_PROP2      ( CONTROLSOURCEPROPERTY,    OUString,    READONLY, TRANSIENT );
        DECL_BOOL_PROP1 ( INPUT_REQUIRED,                               BOUND );
    END_DESCRIBE_PROPERTIES()
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
