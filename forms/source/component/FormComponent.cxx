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
#include <componenttools.hxx>
#include <FormComponent.hxx>
#include <strings.hrc>
#include <frm_resource.hxx>
#include <services.hxx>

#include <com/sun/star/awt/XTextComponent.hpp>
#include <com/sun/star/awt/XVclWindowPeer.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/form/XForm.hpp>
#include <com/sun/star/form/XLoadable.hpp>
#include <com/sun/star/form/binding/IncompatibleTypesException.hpp>
#include <com/sun/star/io/IOException.hpp>
#include <com/sun/star/io/XMarkableStream.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <com/sun/star/sdb/XRowSetChangeBroadcaster.hpp>
#include <com/sun/star/sdb/XRowSetSupplier.hpp>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/SQLException.hpp>
#include <com/sun/star/util/VetoException.hpp>
#include <com/sun/star/util/XModifyBroadcaster.hpp>

#include <comphelper/basicio.hxx>
#include <comphelper/guarding.hxx>
#include <comphelper/interfacecontainer2.hxx>
#include <comphelper/property.hxx>
#include <connectivity/dbtools.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <toolkit/helper/emptyfontdescriptor.hxx>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include <sal/log.hxx>

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

// FieldChangeNotifier
void ControlModelLock::impl_notifyAll_nothrow()
{
    m_rModel.firePropertyChanges( m_aHandles, m_aOldValues, m_aNewValues, OControlModel::LockAccess() );
}

void ControlModelLock::addPropertyNotification( const sal_Int32 _nHandle, const Any& _rOldValue, const Any& _rNewValue )
{
    assert( m_aHandles.size() == m_aOldValues.size() && m_aOldValues.size() == m_aNewValues.size() );

    m_aHandles.push_back( _nHandle );
    m_aOldValues.push_back( _rOldValue );
    m_aNewValues.push_back( _rNewValue );
}

class FieldChangeNotifier
{
public:
    explicit FieldChangeNotifier(ControlModelLock& _rLock)
        : m_rLock( _rLock )
        , m_rModel( dynamic_cast< OBoundControlModel& >( _rLock.getModel() ) )
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

// base class for form layer controls
OControl::OControl( const Reference< XComponentContext >& _rxContext, const OUString& _rAggregateService, const bool _bSetDelegator )
            :OComponentHelper(m_aMutex)
            ,m_xContext( _rxContext )
{
    // Aggregate VCL Control
    // Increment the RefCount for aggregates, because the aggregate by itself increments the RefCount in the setDelegator
    osl_atomic_increment( &m_refCount );
    {
        m_xAggregate.set(_rxContext->getServiceManager()->createInstanceWithContext(_rAggregateService, _rxContext), css::uno::UNO_QUERY);
        m_xControl.set(m_xAggregate, css::uno::UNO_QUERY);
    }
    osl_atomic_decrement( &m_refCount );

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
        m_xAggregate->setDelegator( nullptr );
}

void OControl::doSetDelegator()
{
    osl_atomic_increment( &m_refCount );
    if ( m_xAggregate.is() )
    {   // those brackets are important for some compilers, don't remove!
        // (they ensure that the temporary object created in the line below
        // is destroyed *before* the refcount-decrement)
        m_xAggregate->setDelegator( static_cast< XWeak* >( this ) );
    }
    osl_atomic_decrement( &m_refCount );
}

// UNO Binding
Any SAL_CALL OControl::queryAggregation( const Type& _rType )
{
    // ask the base class
    Any aReturn( OComponentHelper::queryAggregation(_rType) );
    // ask our own interfaces
    if (!aReturn.hasValue())
    {
        aReturn = OControl_BASE::queryInterface(_rType);
        // ask our aggregate
        if (!aReturn.hasValue() && m_xAggregate.is())
            aReturn = m_xAggregate->queryAggregation(_rType);
    }

    return aReturn;
}

Sequence<sal_Int8> SAL_CALL OControl::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

Sequence<Type> SAL_CALL OControl::getTypes()
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

// OComponentHelper
void OControl::disposing()
{
    OComponentHelper::disposing();

    m_aWindowStateGuard.attach( nullptr, nullptr );

    Reference< XComponent > xComp;
    if (query_aggregation(m_xAggregate, xComp))
        xComp->dispose();
}

// XServiceInfo
sal_Bool SAL_CALL OControl::supportsService(const OUString& _rsServiceName)
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

Sequence<OUString> SAL_CALL OControl::getSupportedServiceNames()
{
    // no own supported service names
    return getAggregateServiceNames();
}

// XEventListener
void SAL_CALL OControl::disposing(const css::lang::EventObject& _rEvent)
{
    Reference< XInterface > xAggAsIface;
    query_aggregation(m_xAggregate, xAggAsIface);

    // does the disposing come from the aggregate?
    if (xAggAsIface != Reference< XInterface >(_rEvent.Source, UNO_QUERY))
    {   // no -> forward it
        Reference<css::lang::XEventListener> xListener;
        if (query_aggregation(m_xAggregate, xListener))
            xListener->disposing(_rEvent);
    }
}

// XControl
void SAL_CALL OControl::setContext(const Reference< XInterface >& Context)
{
    if (m_xControl.is())
        m_xControl->setContext(Context);
}

Reference< XInterface > SAL_CALL OControl::getContext()
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
        DBG_UNHANDLED_EXCEPTION("forms.component");
    }
    m_aWindowStateGuard.attach( xWindow, xModel );
}

void SAL_CALL OControl::createPeer(const Reference<XToolkit>& _rxToolkit, const Reference<XWindowPeer>& _rxParent)
{
    if ( m_xControl.is() )
    {
        m_xControl->createPeer( _rxToolkit, _rxParent );
        impl_resetStateGuard_nothrow();
    }
}

Reference<XWindowPeer> SAL_CALL OControl::getPeer()
{
    return m_xControl.is() ? m_xControl->getPeer() : Reference<XWindowPeer>();
}

sal_Bool SAL_CALL OControl::setModel(const Reference<XControlModel>& Model)
{
    if ( !m_xControl.is() )
        return false;

    bool bSuccess = m_xControl->setModel( Model );
    impl_resetStateGuard_nothrow();
    return bSuccess;
}

Reference<XControlModel> SAL_CALL OControl::getModel()
{
    return m_xControl.is() ? m_xControl->getModel() : Reference<XControlModel>();
}

Reference<XView> SAL_CALL OControl::getView()
{
    return m_xControl.is() ? m_xControl->getView() : Reference<XView>();
}

void SAL_CALL OControl::setDesignMode(sal_Bool bOn)
{
    if (m_xControl.is())
        m_xControl->setDesignMode(bOn);
}

sal_Bool SAL_CALL OControl::isDesignMode()
{
    return !m_xControl.is() || m_xControl->isDesignMode();
}

sal_Bool SAL_CALL OControl::isTransparent()
{
    return !m_xControl.is() || m_xControl->isTransparent();
}

OBoundControl::OBoundControl( const Reference< XComponentContext >& _rxContext,
            const OUString& _rAggregateService, const bool _bSetDelegator )
    :OControl( _rxContext, _rAggregateService, _bSetDelegator )
    ,m_bLocked(false)
{
}

OBoundControl::~OBoundControl()
{
}

Sequence< Type> OBoundControl::_getTypes()
{
    return TypeBag( OControl::_getTypes(), OBoundControl_BASE::getTypes() ).getTypes();
}

Any SAL_CALL OBoundControl::queryAggregation(const Type& _rType)
{
    Any aReturn;

    // XTypeProvider first - don't ask the OBoundControl_BASE, it would deliver incomplete types
    if ( _rType.equals( cppu::UnoType<XTypeProvider>::get() ) )
        aReturn = OControl::queryAggregation( _rType );

    // ask our own interfaces
    // (do this first (except XTypeProvider ) - we want to "overwrite" XPropertiesChangeListener)
    if ( !aReturn.hasValue() )
        aReturn = OBoundControl_BASE::queryInterface( _rType );

    // ask the base class
    if ( !aReturn.hasValue() )
        aReturn = OControl::queryAggregation( _rType );

    return aReturn;
}

sal_Bool SAL_CALL OBoundControl::getLock()
{
    return m_bLocked;
}

void SAL_CALL OBoundControl::setLock(sal_Bool _bLock)
{
    if (m_bLocked == bool(_bLock))
        return;

    osl::MutexGuard aGuard(m_aMutex);
    _setLock(_bLock);
    m_bLocked = _bLock;
}

void OBoundControl::_setLock(bool _bLock)
{
    // try to set the text component to readonly
    Reference< XWindowPeer > xPeer = getPeer();
    Reference< XTextComponent > xText( xPeer, UNO_QUERY );

    if ( xText.is() )
        xText->setEditable( !_bLock );
    else
    {
        // disable the window
        Reference< XWindow > xComp( xPeer, UNO_QUERY );
        if ( xComp.is() )
            xComp->setEnable( !_bLock );
    }
}

sal_Bool SAL_CALL OBoundControl::setModel( const Reference< XControlModel >& _rxModel )
{
    return OControl::setModel( _rxModel );
}

void SAL_CALL OBoundControl::disposing(const EventObject& Source)
{
    // just disambiguate
    OControl::disposing(Source);
}

void OBoundControl::disposing()
{
    OControl::disposing();
}

// OControlModel
Sequence<sal_Int8> SAL_CALL OControlModel::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

Sequence<Type> SAL_CALL OControlModel::getTypes()
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

Any SAL_CALL OControlModel::queryAggregation(const Type& _rType)
{
    // base class 1
    Any aReturn(OComponentHelper::queryAggregation(_rType));

    // base class 2
    if (!aReturn.hasValue())
    {
        aReturn = OControlModel_BASE::queryInterface(_rType);

        // our own interfaces
        if (!aReturn.hasValue())
        {
            aReturn = OPropertySetAggregationHelper::queryInterface(_rType);
            // our aggregate
            if (!aReturn.hasValue() && m_xAggregate.is() && !_rType.equals(cppu::UnoType<XCloneable>::get()))
                aReturn = m_xAggregate->queryAggregation(_rType);
        }
    }
    return aReturn;
}

void OControlModel::readHelpTextCompatibly(const css::uno::Reference< css::io::XObjectInputStream >& _rxInStream)
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
        DBG_UNHANDLED_EXCEPTION("forms.component");
        SAL_WARN("forms.component", "OControlModel::readHelpTextCompatibly: could not forward the property value to the aggregate!");
    }
}

void OControlModel::writeHelpTextCompatibly(const css::uno::Reference< css::io::XObjectOutputStream >& _rxOutStream)
{
    OUString sHelpText;
    try
    {
        if (m_xAggregateSet.is())
            m_xAggregateSet->getPropertyValue(PROPERTY_HELPTEXT) >>= sHelpText;
    }
    catch(const Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("forms.component");
        SAL_WARN("forms.component", "OControlModel::writeHelpTextCompatibly: could not retrieve the property value from the aggregate!");
    }
    ::comphelper::operator<<( _rxOutStream, sHelpText);
}

OControlModel::OControlModel(
            const Reference<XComponentContext>& _rxContext,
            const OUString& _rUnoControlModelTypeName,
            const OUString& rDefault, const bool _bSetDelegator)
    :OComponentHelper(m_aMutex)
    ,OPropertySetAggregationHelper(OComponentHelper::rBHelper)
    ,m_xContext( _rxContext )
    ,m_lockCount( 0 )
    ,m_aPropertyBagHelper( *this )
    ,m_nTabIndex(FRM_DEFAULT_TABINDEX)
    ,m_nClassId(FormComponentType::CONTROL)
    ,m_bNativeLook( false )
    ,m_bGenerateVbEvents( false )
    ,m_nControlTypeinMSO(0) // 0 : default value is create from AOO
    ,m_nObjIDinMSO(INVALID_OBJ_ID_IN_MSO)
    // form controls are usually embedded into documents, not dialogs, and in documents
    // the native look is ugly...
    // #i37342#
{
    if (!_rUnoControlModelTypeName.isEmpty())  // the is a model we have to aggregate
    {
        osl_atomic_increment(&m_refCount);
        {
            m_xAggregate.set(m_xContext->getServiceManager()->createInstanceWithContext(_rUnoControlModelTypeName, m_xContext), UNO_QUERY);
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
                    DBG_UNHANDLED_EXCEPTION("forms.component");
                    SAL_WARN("forms.component",  "OControlModel::OControlModel: caught an exception!");
                }
            }
        }
        if (_bSetDelegator)
            doSetDelegator();

        // Refcount is at NULL again
        osl_atomic_decrement(&m_refCount);
    }
}

OControlModel::OControlModel( const OControlModel* _pOriginal, const Reference< XComponentContext>& _rxFactory, const bool _bCloneAggregate, const bool _bSetDelegator )
    :OComponentHelper( m_aMutex )
    ,OPropertySetAggregationHelper( OComponentHelper::rBHelper )
    ,m_xContext( _rxFactory )
    ,m_lockCount( 0 )
    ,m_aPropertyBagHelper( *this )
    ,m_nTabIndex( FRM_DEFAULT_TABINDEX )
    ,m_nClassId( FormComponentType::CONTROL )
{
    DBG_ASSERT( _pOriginal, "OControlModel::OControlModel: invalid original!" );

    // copy members
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
        // temporarily increment refcount because of temporary references to ourself in the following
        osl_atomic_increment( &m_refCount );
        {
            // transfer the (only, at the very moment!) ref count
            m_xAggregate = createAggregateClone( _pOriginal );

            // set aggregation (retrieve other direct interfaces of the aggregate)
            setAggregation( m_xAggregate );
        }

        // set the delegator, if allowed by our derived class
        if ( _bSetDelegator )
            doSetDelegator();

        // decrement ref count
        osl_atomic_decrement( &m_refCount );
    }
}

OControlModel::~OControlModel()
{
    // release the aggregate
    doResetDelegator( );
}

void OControlModel::clonedFrom( const OControlModel* /*_pOriginal*/ )
{
    // nothing to do in this base class
}

void OControlModel::doResetDelegator()
{
    if (m_xAggregate.is())
        m_xAggregate->setDelegator(nullptr);
}

void OControlModel::doSetDelegator()
{
    osl_atomic_increment(&m_refCount);
    if (m_xAggregate.is())
    {
        m_xAggregate->setDelegator(static_cast<XWeak*>(this));
    }
    osl_atomic_decrement(&m_refCount);
}

// XChild
Reference< XInterface > SAL_CALL OControlModel::getParent()
{
    return m_xParent;
}

void SAL_CALL OControlModel::setParent(const Reference< XInterface >& _rxParent)
{
    osl::MutexGuard aGuard(m_aMutex);

    Reference<XComponent> xComp(m_xParent, UNO_QUERY);
    if (xComp.is())
        xComp->removeEventListener(static_cast<XPropertiesChangeListener*>(this));

    m_xParent = _rxParent;
    xComp.set(m_xParent, css::uno::UNO_QUERY);

    if ( xComp.is() )
        xComp->addEventListener(static_cast<XPropertiesChangeListener*>(this));
}

// XNamed
OUString SAL_CALL OControlModel::getName()
{
    OUString aReturn;
    try
    {
        OPropertySetHelper::getFastPropertyValue(PROPERTY_ID_NAME) >>= aReturn;
    }
    catch (const css::beans::UnknownPropertyException&)
    {
        css::uno::Any a(cppu::getCaughtException());
        throw WrappedTargetRuntimeException(
            "OControlModel::getName",
            *this,
            a
        );
    }
    return aReturn;
}

void SAL_CALL OControlModel::setName(const OUString& _rName)
{
    try
    {
        setFastPropertyValue(PROPERTY_ID_NAME, makeAny(_rName));
    }
    catch (const css::beans::UnknownPropertyException&)
    {
        css::uno::Any a(cppu::getCaughtException());
        throw WrappedTargetRuntimeException(
            "OControlModel::setName",
            *this,
            a
        );
    }
}

// XServiceInfo
sal_Bool SAL_CALL OControlModel::supportsService(const OUString& _rServiceName)
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

Sequence<OUString> SAL_CALL OControlModel::getSupportedServiceNames()
{
    return ::comphelper::concatSequences(
        getAggregateServiceNames(),
        getSupportedServiceNames_Static()
    );
}

Sequence< OUString > OControlModel::getSupportedServiceNames_Static()
{
    Sequence< OUString > aServiceNames( 2 );
    aServiceNames[ 0 ] = FRM_SUN_FORMCOMPONENT;
    aServiceNames[ 1 ] = "com.sun.star.form.FormControlModel";
    return aServiceNames;
}

// XEventListener
void SAL_CALL OControlModel::disposing(const css::lang::EventObject& _rSource)
{
    // release the parent
    if (_rSource.Source == m_xParent)
    {
        osl::MutexGuard aGuard(m_aMutex);
        m_xParent = nullptr;
    }
    else
    {
        Reference<css::lang::XEventListener> xEvtLst;
        if (query_aggregation(m_xAggregate, xEvtLst))
        {
            osl::MutexGuard aGuard(m_aMutex);
            xEvtLst->disposing(_rSource);
        }
    }
}

// OComponentHelper
void OControlModel::disposing()
{
    OPropertySetAggregationHelper::disposing();

    Reference<css::lang::XComponent> xComp;
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

void SAL_CALL OControlModel::write(const Reference<css::io::XObjectOutputStream>& _rxOutStream)
{
    osl::MutexGuard aGuard(m_aMutex);

    // 1. writing the UnoControls
    Reference<css::io::XMarkableStream> xMark(_rxOutStream, UNO_QUERY);
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

    // determining the length
    nLen = xMark->offsetToMark(nMark) - 4;
    xMark->jumpToMark(nMark);
    _rxOutStream->writeLong(nLen);
    xMark->jumpToFurthest();
    xMark->deleteMark(nMark);

    // 2. writing a version number
    _rxOutStream->writeShort(0x0003);

    // 3. writing the general properties
    ::comphelper::operator<<( _rxOutStream, m_aName);
    _rxOutStream->writeShort(m_nTabIndex);
    ::comphelper::operator<<( _rxOutStream, m_aTag); // 3rd version

    // IMPORTANT NOTE!
    // don't write any new members here: this wouldn't be compatible with older versions, as OControlModel
    // is a base class which is called in derived classes "read" method. So if you increment the version
    // and write new stuff, older office versions will read this in the _derived_ classes, which may result
    // in anything from data loss to crash.
    // EOIN!
}

void OControlModel::read(const Reference<css::io::XObjectInputStream>& InStream)
{
    osl::MutexGuard aGuard(m_aMutex);

    Reference<css::io::XMarkableStream> xMark(InStream, UNO_QUERY);
    if ( !xMark.is() )
    {
        throw IOException(
            FRM_RES_STRING( RID_STR_INVALIDSTREAM ),
            static_cast< ::cppu::OWeakObject* >( this )
        );
    }

    // 1. reading the UnoControls
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
            DBG_UNHANDLED_EXCEPTION("forms.component");
        }

        xMark->jumpToMark(nMark);
        InStream->skipBytes(nLen);
        xMark->deleteMark(nMark);
    }

    // 2. reading the version number
    sal_uInt16 nVersion = InStream->readShort();

    // 3. reading the general properties
    ::comphelper::operator>>( InStream, m_aName);
    m_nTabIndex  = InStream->readShort();

    if (nVersion > 0x0002)
        ::comphelper::operator>>( InStream, m_aTag);

    // we had a version where we wrote the help text
    if (nVersion == 0x0004)
        readHelpTextCompatibly(InStream);

    DBG_ASSERT(nVersion < 5, "OControlModel::read : suspicious version number !");
    // 4 was the version where we wrote the help text
    // later versions shouldn't exist (see write for a detailed comment)
}

PropertyState OControlModel::getPropertyStateByHandle( sal_Int32 _nHandle )
{
    // simply compare the current and the default value
    Any aCurrentValue = getPropertyDefaultByHandle( _nHandle );
    Any aDefaultValue;  getFastPropertyValue( aDefaultValue, _nHandle );

    bool bEqual = aCurrentValue == aDefaultValue;
    return bEqual ? PropertyState_DEFAULT_VALUE : PropertyState_DIRECT_VALUE;
}

void OControlModel::setPropertyToDefaultByHandle( sal_Int32 _nHandle)
{
    Any aDefault = getPropertyDefaultByHandle( _nHandle );

    Any aConvertedValue, aOldValue;
    if ( convertFastPropertyValue( aConvertedValue, aOldValue, _nHandle, aDefault ) )
    {
        setFastPropertyValue_NoBroadcast( _nHandle, aConvertedValue );
        // TODO: fire the property change
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
            aReturn <<= sal_Int16(FormComponentType::CONTROL);
            break;
        case PROPERTY_ID_TABINDEX:
            aReturn <<= sal_Int16(FRM_DEFAULT_TABINDEX);
            break;
        case PROPERTY_ID_NATIVE_LOOK:
            aReturn <<= true;
            break;
        case PROPERTY_ID_GENERATEVBAEVENTS:
            aReturn <<= false;
            break;
        // added for exporting OCX control
        case PROPERTY_ID_CONTROL_TYPE_IN_MSO:
            aReturn <<= sal_Int16(0);
            break;
        case PROPERTY_ID_OBJ_ID_IN_MSO:
            aReturn <<= sal_uInt16(INVALID_OBJ_ID_IN_MSO);
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
            _rValue <<= m_bNativeLook;
            break;
        case PROPERTY_ID_GENERATEVBAEVENTS:
            _rValue <<= m_bGenerateVbEvents;
            break;
        // added for exporting OCX control
        case PROPERTY_ID_CONTROL_TYPE_IN_MSO:
            _rValue <<= m_nControlTypeinMSO;
            break;
        case PROPERTY_ID_OBJ_ID_IN_MSO:
            _rValue <<= m_nObjIDinMSO;
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
{
    bool bModified(false);
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
        // added for exporting OCX control
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
{
    switch (_nHandle)
    {
        case PROPERTY_ID_NAME:
            DBG_ASSERT(_rValue.getValueType() == cppu::UnoType<OUString>::get(),
                "OControlModel::setFastPropertyValue_NoBroadcast : invalid type" );
            _rValue >>= m_aName;
            break;
        case PROPERTY_ID_TAG:
            DBG_ASSERT(_rValue.getValueType() == cppu::UnoType<OUString>::get(),
                "OControlModel::setFastPropertyValue_NoBroadcast : invalid type" );
            _rValue >>= m_aTag;
            break;
        case PROPERTY_ID_TABINDEX:
            DBG_ASSERT(_rValue.getValueType() == cppu::UnoType<sal_Int16>::get(),
                "OControlModel::setFastPropertyValue_NoBroadcast : invalid type" );
            _rValue >>= m_nTabIndex;
            break;
        case PROPERTY_ID_NATIVE_LOOK:
            OSL_VERIFY( _rValue >>= m_bNativeLook );
            break;
        case PROPERTY_ID_GENERATEVBAEVENTS:
            OSL_VERIFY( _rValue >>= m_bGenerateVbEvents );
            break;
        // added for exporting OCX control
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
        DECL_PROP1      (OBJ_ID_IN_MSO,cppu::UnoUnsignedShortType,      BOUND);
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

Reference< XPropertySetInfo> SAL_CALL OControlModel::getPropertySetInfo()
{
    return createPropertySetInfo( getInfoHelper() );
}

::cppu::IPropertyArrayHelper& OControlModel::getInfoHelper()
{
    return m_aPropertyBagHelper.getInfoHelper();
}

void SAL_CALL OControlModel::addProperty( const OUString& _rName, ::sal_Int16 _nAttributes, const Any& _rInitialValue )
{
    m_aPropertyBagHelper.addProperty( _rName, _nAttributes, _rInitialValue );
}

void SAL_CALL OControlModel::removeProperty( const OUString& _rName )
{
    m_aPropertyBagHelper.removeProperty( _rName );
}

Sequence< PropertyValue > SAL_CALL OControlModel::getPropertyValues()
{
    return m_aPropertyBagHelper.getPropertyValues();
}

void SAL_CALL OControlModel::setPropertyValues( const Sequence< PropertyValue >& _rProps )
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

void OControlModel::firePropertyChanges( const std::vector< sal_Int32 >& _rHandles, const std::vector< Any >& _rOldValues,
                                        const std::vector< Any >& _rNewValues, LockAccess )
{
    OPropertySetHelper::fire(
        const_cast< std::vector< sal_Int32 >& >( _rHandles ).data(),
        _rNewValues.data(),
        _rOldValues.data(),
        _rHandles.size(),
        false
    );
}

// OBoundControlModel
Any SAL_CALL OBoundControlModel::queryAggregation( const Type& _rType )
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
        const bool _bCommitable, const bool _bSupportExternalBinding, const bool _bSupportsValidation )
    :OControlModel( _rxFactory, _rUnoControlModelTypeName, _rDefault, false )
    ,OPropertyChangeListener( m_aMutex )
    ,m_xField()
    ,m_xAmbientForm()
    ,m_nValuePropertyAggregateHandle( -1 )
    ,m_nFieldType( DataType::OTHER )
    ,m_bValuePropertyMayBeVoid( false )
    ,m_aResetHelper( *this, m_aMutex )
    ,m_aUpdateListeners(m_aMutex)
    ,m_aFormComponentListeners( m_aMutex )
    ,m_bInputRequired( false )
    ,m_pAggPropMultiplexer( nullptr )
    ,m_bFormListening( false )
    ,m_bLoaded(false)
    ,m_bRequired(false)
    ,m_bCommitable(_bCommitable)
    ,m_bSupportsExternalBinding( _bSupportExternalBinding )
    ,m_bSupportsValidation( _bSupportsValidation )
    ,m_bForwardValueChanges(true)
    ,m_bTransferingValue( false )
    ,m_bIsCurrentValueValid( true )
    ,m_bBindingControlsRO( false )
    ,m_bBindingControlsEnable( false )
    ,m_eControlValueChangeInstigator( eOther )
    ,m_aLabelServiceName(FRM_SUN_COMPONENT_FIXEDTEXT)
{
    // start property listening at the aggregate
    implInitAggMultiplexer( );
}

OBoundControlModel::OBoundControlModel(
        const OBoundControlModel* _pOriginal, const Reference< XComponentContext>& _rxFactory )
    :OControlModel( _pOriginal, _rxFactory, true, false )
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
    ,m_bInputRequired( false )
    ,m_pAggPropMultiplexer( nullptr )
    ,m_bFormListening( false )
    ,m_bLoaded( false )
    ,m_bRequired( false )
    ,m_bCommitable( _pOriginal->m_bCommitable )
    ,m_bSupportsExternalBinding( _pOriginal->m_bSupportsExternalBinding )
    ,m_bSupportsValidation( _pOriginal->m_bSupportsValidation )
    ,m_bForwardValueChanges( true )
    ,m_bTransferingValue( false )
    ,m_bIsCurrentValueValid( _pOriginal->m_bIsCurrentValueValid )
    ,m_bBindingControlsRO( false )
    ,m_bBindingControlsEnable( false )
    ,m_eControlValueChangeInstigator( eOther )
{
    // start property listening at the aggregate
    implInitAggMultiplexer( );
    m_aLabelServiceName = _pOriginal->m_aLabelServiceName;
    m_sValuePropertyName = _pOriginal->m_sValuePropertyName;
    m_nValuePropertyAggregateHandle = _pOriginal->m_nValuePropertyAggregateHandle;
    m_bValuePropertyMayBeVoid = _pOriginal->m_bValuePropertyMayBeVoid;
    m_aValuePropertyType = _pOriginal->m_aValuePropertyType;
    m_aControlSource = _pOriginal->m_aControlSource;
    m_bInputRequired = _pOriginal->m_bInputRequired;
    // m_xLabelControl, though being a property, is not to be cloned, not even the reference will be transferred.
    // (the former should be clear - a clone of the object we're only referencing does not make sense)
    // (the second would violate the restriction for label controls that they're part of the
    // same form component hierarchy - we ourself are no part, yet, so we can't have a label control)
    // start listening for changes at the value property
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
        m_pAggPropMultiplexer = nullptr;
    }
}

void OBoundControlModel::clonedFrom( const OControlModel* _pOriginal )
{
    const OBoundControlModel* pBoundOriginal = static_cast< const OBoundControlModel* >( _pOriginal );
    // the value binding can be handled as if somebody called setValueBinding here
    // By definition, bindings can be share between bindables
    if ( pBoundOriginal && pBoundOriginal->m_xExternalBinding.is() )
    {
        try
        {
            setValueBinding( pBoundOriginal->m_xExternalBinding );
        }

        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("forms.component");
        }

    }
}

void OBoundControlModel::implInitAggMultiplexer( )
{
    osl_atomic_increment( &m_refCount );
    if ( m_xAggregateSet.is() )
    {
        m_pAggPropMultiplexer = new OPropertyChangeMultiplexer( this, m_xAggregateSet, false );
        m_pAggPropMultiplexer->acquire();
    }

    osl_atomic_decrement( &m_refCount );
    doSetDelegator();
}

void OBoundControlModel::implInitValuePropertyListening( ) const
{
    // start listening for changes at the value property
    // There are three pre-requisites for this to be done:
    // 1. We support external value bindings. In this case, the changes in the control value need to
    //    be propagated to the external binding immediately when they happen
    // 2. We support external validation. In this case, we need to listen for changes in the value
    //    property, since we need to revalidate then.
    // 3. We are not committable. In this case, changes in the control value need to be propagated
    //    to the database column immediately when they happen.
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

    // start listening for changes at the value property
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

// OComponentHelper
void OBoundControlModel::disposing()
{
    OControlModel::disposing();

    ::osl::ClearableMutexGuard aGuard(m_aMutex);

    if ( m_pAggPropMultiplexer )
        m_pAggPropMultiplexer->dispose();

    // notify all our listeners
    css::lang::EventObject aEvt( static_cast< XWeak* >( this ) );
    m_aUpdateListeners.disposeAndClear( aEvt );
    m_aResetHelper.disposing();

    // disconnect from our database column
    // TODO: could we replace the following 5 lines with a call to impl_disconnectDatabaseColumn_noNotify?
    // The only more thing which it does is calling onDisconnectedDbColumn - could this
    // cause trouble? At least when we continue to call OControlModel::disposing before, it *may*.
    if ( hasField() )
    {
        getField()->removePropertyChangeListener( PROPERTY_VALUE, this );
        resetField();
    }

    m_xCursor = nullptr;
    Reference< XComponent > xComp( m_xLabelControl, UNO_QUERY );
    if ( xComp.is() )
        xComp->removeEventListener(static_cast< XEventListener* >( static_cast< XPropertyChangeListener* >( this ) ) );
    // disconnect from our external value binding
    if ( hasExternalValueBinding() )
        disconnectExternalValueBinding();
    // dito for the validator
    if ( hasValidator() )
        disconnectValidator( );
}

void OBoundControlModel::onValuePropertyChange( ControlModelLock& i_rControLock )
{
    if ( hasExternalValueBinding() )
    {
        // the control value changed, while we have an external value binding
        // -> forward the value to it
        if ( m_eControlValueChangeInstigator != eExternalBinding )
            transferControlValueToExternal( i_rControLock );
    }

    else if ( !m_bCommitable && m_xColumnUpdate.is() )
    {
        // the control value changed, while we are  bound to a database column,
        // but not committable (which means changes in the control have to be reflected to
        // the underlying database column immediately)
        // -> forward the value to the database column
        if ( m_eControlValueChangeInstigator != eDbColumnBinding )
            commitControlValueToDbColumn( false );
    }

    // validate the new value
    if ( m_bSupportsValidation )
        recheckValidity( true );
}

void OBoundControlModel::_propertyChanged( const PropertyChangeEvent& _rEvt )
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
        // if our parent does not directly support the XLoadable interface, then it might support the
        // XRowSetSupplier/XRowSetChangeBroadcaster interfaces. In this case we have to listen for changes
        // broadcasted by the latter.
        Reference< XRowSetChangeBroadcaster > xRowSetBroadcaster( getParent(), UNO_QUERY );
        if ( xRowSetBroadcaster.is() )
            _bStart ? xRowSetBroadcaster->addRowSetChangeListener( this ) : xRowSetBroadcaster->removeRowSetChangeListener( this );
    }

    m_bFormListening = _bStart && m_xAmbientForm.is();
}

// XChild
void SAL_CALL OBoundControlModel::setParent(const Reference<XInterface>& _rxParent)
{
    ControlModelLock aLock( *this );
    FieldChangeNotifier aBoundFieldNotifier( aLock );
    if ( getParent() == _rxParent )
        return;
    // disconnect from database column (which is controlled by parent, directly or indirectly)
    if ( hasField() )
        impl_disconnectDatabaseColumn_noNotify();
    // log off old listeners
    if ( isFormListening() )
        doFormListening( false );
    // actually set the new parent
    OControlModel::setParent( _rxParent );
    // a new parent means a new ambient form
    impl_determineAmbientForm_nothrow();
    if ( !hasExternalValueBinding() )
    {
        // log on new listeners
        doFormListening( true );
        // re-connect to database column of the new parent
        if ( m_xAmbientForm.is() && m_xAmbientForm->isLoaded() )
            impl_connectDatabaseColumn_noNotify( false );
    }
}

// XEventListener
void SAL_CALL OBoundControlModel::disposing(const css::lang::EventObject& _rEvent)
{
    ControlModelLock aLock( *this );
    if ( _rEvent.Source == getField() )
    {
        resetField();
    }

    else if ( _rEvent.Source == m_xLabelControl )
    {
        Reference<XPropertySet> xOldValue = m_xLabelControl;
        m_xLabelControl = nullptr;
        // fire a propertyChanged (when we leave aLock's scope)
        aLock.addPropertyNotification( PROPERTY_ID_CONTROLLABEL, makeAny( xOldValue ), makeAny( m_xLabelControl ) );
    }

    else if ( _rEvent.Source == m_xExternalBinding )
    {   // *first* check for the external binding
        disconnectExternalValueBinding( );
    }

    else if ( _rEvent.Source == m_xValidator )
    {   // *then* check for the validator. Reason is that bindings may also act as validator at the same
        // time, in this case, the validator is automatically revoked when the binding is revoked
        disconnectValidator( );
    }

    else
        OControlModel::disposing(_rEvent);
}

// XServiceInfo
css::uno::Sequence<OUString> SAL_CALL OBoundControlModel::getSupportedServiceNames()
{
    return ::comphelper::combineSequences(
        getAggregateServiceNames(),
        getSupportedServiceNames_Static()
    );
}

Sequence< OUString > OBoundControlModel::getSupportedServiceNames_Static()
{
    Sequence<OUString> aOwnServiceNames { "com.sun.star.form.DataAwareControlModel" };
    return ::comphelper::concatSequences(
        OControlModel::getSupportedServiceNames_Static(),
        aOwnServiceNames
    );
}

// XPersist
void SAL_CALL OBoundControlModel::write( const Reference<css::io::XObjectOutputStream>& _rxOutStream )
{
    OControlModel::write(_rxOutStream);
    osl::MutexGuard aGuard(m_aMutex);
    // Version
    _rxOutStream->writeShort(0x0002);
    // Controlsource
    ::comphelper::operator<<( _rxOutStream, m_aControlSource);
    // !!! IMPORTANT NOTE !!!
    // don't write any new members here: this wouldn't be compatible with older versions, as OBoundControlModel
    // is a base class which is called in derived classes "read" method. So if you increment the version
    // and write new stuff, older office versions will read this in the _derived_ classes, which may result
    // in anything from data loss to crash.
    // (use writeCommonProperties instead, this is called in derived classes write-method)
    // !!! EOIN !!!
}

void OBoundControlModel::defaultCommonProperties()
{
    Reference<css::lang::XComponent> xComp(m_xLabelControl, UNO_QUERY);
    if (xComp.is())
        xComp->removeEventListener(static_cast<css::lang::XEventListener*>(static_cast<XPropertyChangeListener*>(this)));
    m_xLabelControl = nullptr;
}

void OBoundControlModel::readCommonProperties(const Reference<css::io::XObjectInputStream>& _rxInStream)
{
    sal_Int32 nLen = _rxInStream->readLong();
    Reference<css::io::XMarkableStream> xMark(_rxInStream, UNO_QUERY);
    DBG_ASSERT(xMark.is(), "OBoundControlModel::readCommonProperties : can only work with markable streams !");
    sal_Int32 nMark = xMark->createMark();
    // read the reference to the label control
    Reference<css::io::XPersistObject> xPersist;
    sal_Int32 nUsedFlag;
    nUsedFlag = _rxInStream->readLong();
    if (nUsedFlag)
        xPersist = _rxInStream->readObject();
    m_xLabelControl.set(xPersist, css::uno::UNO_QUERY);
    Reference< XComponent > xComp( m_xLabelControl, UNO_QUERY );
    if (xComp.is())
        xComp->addEventListener(static_cast<css::lang::XEventListener*>(static_cast<XPropertyChangeListener*>(this)));
    // read any other new common properties here
    // skip the remaining bytes
    xMark->jumpToMark(nMark);
    _rxInStream->skipBytes(nLen);
    xMark->deleteMark(nMark);
}

void OBoundControlModel::writeCommonProperties(const Reference<css::io::XObjectOutputStream>& _rxOutStream)
{
    Reference<css::io::XMarkableStream> xMark(_rxOutStream, UNO_QUERY);
    DBG_ASSERT(xMark.is(), "OBoundControlModel::writeCommonProperties : can only work with markable streams !");
    sal_Int32 nMark = xMark->createMark();
    // a placeholder where we will write the overall length (later in this method)
    sal_Int32 nLen = 0;
    _rxOutStream->writeLong(nLen);
    // write the reference to the label control
    Reference<css::io::XPersistObject> xPersist(m_xLabelControl, UNO_QUERY);
    sal_Int32 nUsedFlag = 0;
    if (xPersist.is())
        nUsedFlag = 1;
    _rxOutStream->writeLong(nUsedFlag);
    if (xPersist.is())
        _rxOutStream->writeObject(xPersist);
    // write any other new common properties here
    // write the correct length at the beginning of the block
    nLen = xMark->offsetToMark(nMark) - sizeof(nLen);
    xMark->jumpToMark(nMark);
    _rxOutStream->writeLong(nLen);
    xMark->jumpToFurthest();
    xMark->deleteMark(nMark);
}

void SAL_CALL OBoundControlModel::read( const Reference< css::io::XObjectInputStream >& _rxInStream )
{
    OControlModel::read(_rxInStream);
    osl::MutexGuard aGuard(m_aMutex);
    _rxInStream->readShort(); // version;
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
{
    bool bModified(false);
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
            throw css::lang::IllegalArgumentException();
        case PROPERTY_ID_CONTROLLABEL:
            if (!_rValue.hasValue())
            {   // property set to void
                _rConvertedValue = Any();
                getFastPropertyValue(_rOldValue, _nHandle);
                bModified = m_xLabelControl.is();
            }

            else
            {
                bModified = tryPropertyValue(_rConvertedValue, _rOldValue, _rValue, m_xLabelControl);
                if (!m_xLabelControl.is())
                    // an empty interface is interpreted as VOID
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
            aDefault <<= false;
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

void OBoundControlModel::setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const Any& rValue )
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
            throw css::lang::IllegalArgumentException();
        case PROPERTY_ID_CONTROLLABEL:
        {
            if ( rValue.hasValue() && ( rValue.getValueTypeClass() != TypeClass_INTERFACE ) )
                throw css::lang::IllegalArgumentException();
            Reference< XInterface > xNewValue( rValue, UNO_QUERY );
            if ( !xNewValue.is() )
            {   // set property to "void"
                Reference< XComponent > xComp( m_xLabelControl, UNO_QUERY );
                if ( xComp.is() )
                    xComp->removeEventListener( static_cast< XPropertyChangeListener* >( this ) );
                m_xLabelControl = nullptr;
                break;
            }

            Reference< XControlModel >  xAsModel        ( xNewValue,        UNO_QUERY );
            Reference< XServiceInfo >   xAsServiceInfo  ( xAsModel,         UNO_QUERY );
            Reference< XPropertySet >   xAsPropSet      ( xAsServiceInfo,   UNO_QUERY );
            Reference< XChild >         xAsChild        ( xAsPropSet,       UNO_QUERY );
            if ( !xAsChild.is() || !xAsServiceInfo->supportsService( m_aLabelServiceName ) )
            {
                throw css::lang::IllegalArgumentException();
            }

            // Check if we and the given model have a common ancestor (up to the forms collection)
            Reference<XChild> xCont(
                static_cast<XWeak*>(this), css::uno::UNO_QUERY);
            Reference< XInterface > xMyTopLevel = xCont->getParent();
            while (xMyTopLevel.is())
            {
                Reference<XForm> xAsForm(xMyTopLevel, UNO_QUERY);
                if (!xAsForm.is())
                    // found my root
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
                // the both objects don't belong to the same forms collection -> not acceptable
                throw css::lang::IllegalArgumentException();
            }

            m_xLabelControl = xAsPropSet;
            Reference<css::lang::XComponent> xComp(m_xLabelControl, UNO_QUERY);
            if (xComp.is())
                xComp->addEventListener(static_cast<css::lang::XEventListener*>(static_cast<XPropertyChangeListener*>(this)));
        }

        break;
        default:
            OControlModel::setFastPropertyValue_NoBroadcast(nHandle, rValue );
    }
}

// XPropertyChangeListener
void SAL_CALL OBoundControlModel::propertyChange( const PropertyChangeEvent& evt )
{
    // if the DBColumn value changed, transfer it to the control
    if ( evt.PropertyName == PROPERTY_VALUE )
    {
        OSL_ENSURE( evt.Source == getField(), "OBoundControlModel::propertyChange: value changes from components other than our database column?" );
        osl::MutexGuard aGuard(m_aMutex);
        if ( m_bForwardValueChanges && m_xColumn.is() )
            transferDbValueToControl();
    }

    else
    {
        OSL_ENSURE( evt.Source == m_xExternalBinding, "OBoundControlModel::propertyChange: where did this come from?" );
        // our binding has properties which can control properties of ourself
        OUString sBindingControlledProperty;
        bool bForwardToLabelControl = false;
        if ( evt.PropertyName == PROPERTY_READONLY )
        {
            sBindingControlledProperty = PROPERTY_READONLY;
        }

        else if ( evt.PropertyName == PROPERTY_RELEVANT )
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
            DBG_UNHANDLED_EXCEPTION("forms.component");
            SAL_WARN("forms.component",  "OBoundControlModel::propertyChange: could not adjust my binding-controlled property!");
        }

    }
}

void SAL_CALL OBoundControlModel::onRowSetChanged( const EventObject& /*i_Event*/ )
{
    ControlModelLock aLock( *this );
    FieldChangeNotifier aBoundFieldNotifier( aLock );
    // disconnect from database column (which is controlled by parent, directly or indirectly)
    if ( hasField() )
        impl_disconnectDatabaseColumn_noNotify();
    // log off old listeners
    if ( isFormListening() )
        doFormListening( false );
    // determine the new ambient form
    impl_determineAmbientForm_nothrow();
    // log on new listeners
    doFormListening( true );
    // re-connect to database column if needed and possible
    if ( m_xAmbientForm.is() && m_xAmbientForm->isLoaded() )
        impl_connectDatabaseColumn_noNotify( false );
}

// XBoundComponent
void SAL_CALL OBoundControlModel::addUpdateListener(const Reference<XUpdateListener>& _rxListener)
{
    m_aUpdateListeners.addInterface(_rxListener);
}

void SAL_CALL OBoundControlModel::removeUpdateListener(const Reference< XUpdateListener>& _rxListener)
{
    m_aUpdateListeners.removeInterface(_rxListener);
}

sal_Bool SAL_CALL OBoundControlModel::commit()
{
    ControlModelLock aLock( *this );
    OSL_PRECOND( m_bCommitable, "OBoundControlModel::commit: invalid call (I'm not commitable !) " );
    if ( hasExternalValueBinding() )
    {
        // in most cases, no action is required: For most derivees, we know the value property of
        // our control (see initValueProperty), and when an external binding is active, we
        // instantly forward all changes in this property to the external binding.
        if ( m_sValuePropertyName.isEmpty() )
            // but for those derivees which did not use this feature, we need an
            // explicit transfer
            transferControlValueToExternal( aLock );
        return true;
    }

    OSL_ENSURE( !hasExternalValueBinding(), "OBoundControlModel::commit: control flow broken!" );
        // we reach this only if we're not working with an external binding
    if ( !hasField() )
        return true;
    ::comphelper::OInterfaceIteratorHelper2 aIter( m_aUpdateListeners );
    EventObject aEvent;
    aEvent.Source = static_cast< XWeak* >( this );
    bool bSuccess = true;
    aLock.release();
    // UNSAFE >
    while (aIter.hasMoreElements() && bSuccess)
        bSuccess = static_cast< XUpdateListener* >( aIter.next() )->approveUpdate( aEvent );
    // < UNSAFE
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
            bSuccess = false;
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

void OBoundControlModel::connectToField(const Reference<XRowSet>& rForm)
{
    OSL_PRECOND( !hasExternalValueBinding(), "OBoundControlModel::connectToField: invalid call (have an external binding)!" );
    // if there's a connection to the database
    if (rForm.is() && getConnection(rForm).is())
    {
        // determine field and PropertyChangeListener
        m_xCursor = rForm;
        Reference<XPropertySet> xFieldCandidate;
        if (m_xCursor.is())
        {
            Reference<XColumnsSupplier> xColumnsSupplier(m_xCursor, UNO_QUERY);
            DBG_ASSERT(xColumnsSupplier.is(), "OBoundControlModel::connectToField : the row set should support the css::sdb::ResultSet service !");
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
                impl_setField_noNotify( nullptr );
            if ( m_xField.is() )
            {
                if ( m_xField->getPropertySetInfo()->hasPropertyByName( PROPERTY_VALUE ) )
                {
                    m_nFieldType = nFieldType;
                    // listen to changing values
                    m_xField->addPropertyChangeListener( PROPERTY_VALUE, this );
                    m_xColumnUpdate.set( m_xField, UNO_QUERY );
                    m_xColumn.set( m_xField, UNO_QUERY );
                    sal_Int32 nNullableFlag = ColumnValue::NO_NULLS;
                    m_xField->getPropertyValue(PROPERTY_ISNULLABLE) >>= nNullableFlag;
                    m_bRequired = (ColumnValue::NO_NULLS == nNullableFlag);
                    // we're optimistic: in case of ColumnValue_NULLABLE_UNKNOWN we assume nullability...
                }
                else
                {
                    SAL_WARN("forms.component", "OBoundControlModel::connectToField: property " PROPERTY_VALUE " not supported!");
                    impl_setField_noNotify( nullptr );
                }

            }

        }

        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("forms.component");
            resetField();
        }

    }
    hasField();
}

void OBoundControlModel::initFromField( const Reference< XRowSet >& _rxRowSet )
{
    // but only if the rowset is positioned on a valid record
    if ( hasField() && _rxRowSet.is() )
    {
        bool shouldTransfer(!_rxRowSet->isBeforeFirst() && !_rxRowSet->isAfterLast());
        if (!shouldTransfer)
        {
            const Reference< XPropertySet > xPS(_rxRowSet, UNO_QUERY);
            if (xPS.is())
            {
                assert(!shouldTransfer);
                xPS->getPropertyValue("IsNew") >>= shouldTransfer;
            }
        }
        if ( shouldTransfer )
            transferDbValueToControl();
        else
            // reset the field if the row set is empty
            // #i30661#
            resetNoBroadcast();
    }
}

bool OBoundControlModel::approveDbColumnType(sal_Int32 _nColumnType)
{
    OSL_PRECOND( !hasExternalValueBinding(), "OBoundControlModel::approveDbColumnType: invalid call (have an external binding)!" );
    if ((_nColumnType == DataType::BINARY) || (_nColumnType == DataType::VARBINARY)
        || (_nColumnType == DataType::LONGVARBINARY) || (_nColumnType == DataType::OTHER)
        || (_nColumnType == DataType::OBJECT) || (_nColumnType == DataType::DISTINCT)
        || (_nColumnType == DataType::STRUCT) || (_nColumnType == DataType::ARRAY)
        || (_nColumnType == DataType::BLOB) /*|| (_nColumnType == DataType::CLOB)*/
        || (_nColumnType == DataType::REF) || (_nColumnType == DataType::SQLNULL))
        return false;
    return true;
}

void OBoundControlModel::impl_determineAmbientForm_nothrow()
{
    Reference< XInterface > xParent( getParent() );
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
    // consistency checks
    DBG_ASSERT( !( hasField() && !_bFromReload ),
        "OBoundControlModel::impl_connectDatabaseColumn_noNotify: the form is just *loaded*, but we already have a field!" );

    Reference< XRowSet > xRowSet( m_xAmbientForm, UNO_QUERY );
    OSL_ENSURE( xRowSet.is(), "OBoundControlModel::impl_connectDatabaseColumn_noNotify: no row set!" );
    if ( !xRowSet.is() )
        return;
    if ( !hasField() || _bFromReload )
    {
        // connect to the column
        connectToField( xRowSet );
    }

    // now that we're connected (more or less, even if we did not find a column),
    // we definitely want to forward any potentially occurring value changes
    m_bForwardValueChanges = true;
    // let derived classes react on this new connection
    m_bLoaded = true;
    onConnectedDbColumn( xRowSet );
    // initially transfer the db column value to the control, if we successfully connected to a database column
    if ( hasField() )
        initFromField( xRowSet );
}

void OBoundControlModel::impl_disconnectDatabaseColumn_noNotify()
{
    OSL_PRECOND( !hasExternalValueBinding(), "OBoundControlModel::impl_disconnectDatabaseColumn_noNotify: not to be called with an external value binding!" );
    // let derived classes react on this
    onDisconnectedDbColumn();
    if ( hasField() )
    {
        getField()->removePropertyChangeListener( PROPERTY_VALUE, this );
        resetField();
    }

    m_xCursor = nullptr;
    m_bLoaded = false;
}

// XLoadListener
void SAL_CALL OBoundControlModel::loaded( const EventObject& _rEvent )
{
    ControlModelLock aLock( *this );
    FieldChangeNotifier aBoundFieldNotifier( aLock );
    OSL_ENSURE( _rEvent.Source == m_xAmbientForm, "OBoundControlModel::loaded: where does this come from?" );
    OSL_PRECOND( !hasExternalValueBinding(), "OBoundControlModel::loaded: we should never reach this with an external value binding!" );
    if ( hasExternalValueBinding() )
        return;
    impl_connectDatabaseColumn_noNotify( false );
}

void SAL_CALL OBoundControlModel::unloaded( const css::lang::EventObject& /*aEvent*/ )
{
    OSL_PRECOND( !hasExternalValueBinding(), "OBoundControlModel::unloaded: we should never reach this with an external value binding!" );
}

void SAL_CALL OBoundControlModel::reloading( const css::lang::EventObject& /*aEvent*/ )
{
    OSL_PRECOND( !hasExternalValueBinding(), "OBoundControlModel::reloading: we should never reach this with an external value binding!" );
    if ( hasExternalValueBinding() )
        return;
    osl::MutexGuard aGuard(m_aMutex);
    m_bForwardValueChanges = false;
}

void SAL_CALL OBoundControlModel::unloading(const css::lang::EventObject& /*aEvent*/)
{
    ControlModelLock aLock( *this );
    FieldChangeNotifier aBoundFieldNotifier( aLock );
    OSL_PRECOND( !hasExternalValueBinding(), "OBoundControlModel::unloading: we should never reach this with an external value binding!" );
    if ( hasExternalValueBinding() )
        return;
    impl_disconnectDatabaseColumn_noNotify();
}

void SAL_CALL OBoundControlModel::reloaded( const EventObject& _rEvent )
{
    ControlModelLock aLock( *this );
    FieldChangeNotifier aBoundFieldNotifier( aLock );
    OSL_ENSURE( _rEvent.Source == m_xAmbientForm, "OBoundControlModel::reloaded: where does this come from?" );
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
        // release our mutex once (it's acquired in one of the calling methods), as setting aggregate properties
        // may cause any uno controls belonging to us to lock the solar mutex, which is potentially dangerous with
        // our own mutex locked
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
        DBG_UNHANDLED_EXCEPTION("forms.component");
        SAL_WARN("forms.component",  "OBoundControlModel::doSetControlValue: caught an exception!");
    }
}

void OBoundControlModel::onConnectedValidator( )
{
    try
    {
        // if we have an external validator, we do not want the control to force invalid
        // inputs to the default value. Instead, invalid inputs should be translated
        // to NaN (not a number)
        Reference< XPropertySetInfo > xAggregatePropertyInfo;
        if ( m_xAggregateSet.is() )
            xAggregatePropertyInfo = m_xAggregateSet->getPropertySetInfo();
        if ( xAggregatePropertyInfo.is() && xAggregatePropertyInfo->hasPropertyByName( PROPERTY_ENFORCE_FORMAT ) )
            m_xAggregateSet->setPropertyValue( PROPERTY_ENFORCE_FORMAT, makeAny( false ) );
    }

    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("forms.component");
        SAL_WARN("forms.component",  "OBoundControlModel::onConnectedValidator: caught an exception!");
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
            m_xAggregateSet->setPropertyValue( PROPERTY_ENFORCE_FORMAT, makeAny( true ) );
    }

    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("forms.component");
        SAL_WARN("forms.component",  "OBoundControlModel::onDisconnectedValidator: caught an exception!");
    }

    recheckValidity( false );
}

void OBoundControlModel::onConnectedExternalValue( )
{
    calculateExternalValueType();
}

void OBoundControlModel::onConnectedDbColumn( const Reference< XInterface >& /*_rxForm*/ )
{
    OSL_PRECOND( !hasExternalValueBinding(), "OBoundControlModel::onConnectedDbColumn: how this? There's an external value binding!" );
}

void OBoundControlModel::onDisconnectedDbColumn()
{
    OSL_PRECOND( !hasExternalValueBinding(), "OBoundControlModel::onDisconnectedDbColumn: how this? There's an external value binding!" );
}

// XReset
Any OBoundControlModel::getDefaultForReset() const
{
    return Any();
}

void OBoundControlModel::resetNoBroadcast()
{
    setControlValue( getDefaultForReset(), eOther );
}

void OBoundControlModel::addResetListener(const Reference<XResetListener>& l)
{
    m_aResetHelper.addResetListener( l );
}

void OBoundControlModel::removeResetListener(const Reference<XResetListener>& l)
{
    m_aResetHelper.removeResetListener( l );
}

void OBoundControlModel::reset()
{
    if ( !m_aResetHelper.approveReset() )
       return;
    ControlModelLock aLock( *this );
    // on a new record?
    bool bIsNewRecord = false;
    Reference<XPropertySet> xSet( m_xCursor, UNO_QUERY );
    if ( xSet.is() )
    {
        try
        {
            xSet->getPropertyValue( PROPERTY_ISNEW ) >>= bIsNewRecord;
        }

        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("forms.component");
        }

    }

    // cursor on an invalid row?
    bool bInvalidCursorPosition = true;
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
        DBG_UNHANDLED_EXCEPTION("forms.component");
        SAL_WARN("forms.component",  "OBoundControlModel::reset: caught an SQL exception!" );
    }

    // #i24495# - don't count the insert row as "invalid"
    bool bSimpleReset =
                        (   !m_xColumn.is()                     // no connection to a database column
                        ||  (   m_xCursor.is()                  // OR   we have an improperly positioned cursor
                            &&  bInvalidCursorPosition
                            )
                        ||  hasExternalValueBinding()           // OR we have an external value binding
                        );
    if ( !bSimpleReset )
    {
        // The default values will be set if and only if the current value of the field which we're bound
        // to is NULL.
        // Else, the current field value should be refreshed
        // This behaviour is not completely ... "matured": What should happen if the field as well as the
        // control have a default value?
        bool bIsNull = true;
        // we have to access the field content at least once to get a reliable result by XColumn::wasNull
        try
        {
            // normally, we'd do a getString here. However, this is extremely expensive in the case
            // of binary fields. Unfortunately, getString is the only method which is guaranteed
            // to *always* succeed, all other getXXX methods may fail if the column is asked for a
            // non-convertible type
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
            DBG_UNHANDLED_EXCEPTION("forms.component");
            SAL_WARN("forms.component", "OBoundControlModel::reset: this should have succeeded in all cases!");
        }

        bool bNeedValueTransfer = true;
        if ( bIsNull )
        {
            if ( bIsNewRecord )
            {
                // reset the control to its default
                resetNoBroadcast();
                // and immediately commit the changes to the DB column, to keep consistency
                commitControlValueToDbColumn( true );
                bNeedValueTransfer = false;
            }

        }

        if ( bNeedValueTransfer )
            transferDbValueToControl();
    }

    else
    {
        resetNoBroadcast();
        // transfer to the external binding, if necessary
        if ( hasExternalValueBinding() )
            transferControlValueToExternal( aLock );
    }

    // revalidate, if necessary
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

bool OBoundControlModel::impl_approveValueBinding_nolock( const Reference< XValueBinding >& _rxBinding )
{
    if ( !_rxBinding.is() )
        return false;
    Sequence< Type > aTypeCandidates;
    {
        // SYNCHRONIZED >
        ::osl::MutexGuard aGuard( m_aMutex );
        aTypeCandidates = getSupportedBindingTypes();
        // < SYNCHRONIZED
    }

    for ( auto const & type : aTypeCandidates )
    {
        if ( _rxBinding->supportsType( type ) )
            return true;
    }
    return false;
}

void OBoundControlModel::connectExternalValueBinding(
        const Reference< XValueBinding >& _rxBinding, ControlModelLock& _rInstanceLock )
{
    OSL_PRECOND( _rxBinding.is(), "OBoundControlModel::connectExternalValueBinding: invalid binding instance!" );
    OSL_PRECOND( !hasExternalValueBinding( ), "OBoundControlModel::connectExternalValueBinding: precond not met (currently have a binding)!" );
    // if we're connected to a database column, suspend this
    if ( hasField() )
        impl_disconnectDatabaseColumn_noNotify();
    // suspend listening for load-related events at out ambient form.
    // This is because an external value binding overrules a possible database binding.
    if ( isFormListening() )
        doFormListening( false );
    // remember this new binding
    m_xExternalBinding = _rxBinding;
    // tell the derivee
    onConnectedExternalValue();
    try
    {
        // add as value listener so we get notified when the value changes
        Reference< XModifyBroadcaster > xModifiable( m_xExternalBinding, UNO_QUERY );
        if ( xModifiable.is() )
            xModifiable->addModifyListener( this );
        // add as property change listener for some (possibly present) properties we're
        // interested in
        Reference< XPropertySet > xBindingProps( m_xExternalBinding, UNO_QUERY );
        Reference< XPropertySetInfo > xBindingPropsInfo( xBindingProps.is() ? xBindingProps->getPropertySetInfo() : Reference< XPropertySetInfo >() );
        if ( xBindingPropsInfo.is() )
        {
            if ( xBindingPropsInfo->hasPropertyByName( PROPERTY_READONLY ) )
            {
                xBindingProps->addPropertyChangeListener( PROPERTY_READONLY, this );
                m_bBindingControlsRO = true;
            }

            if ( xBindingPropsInfo->hasPropertyByName( PROPERTY_RELEVANT ) )
            {
                xBindingProps->addPropertyChangeListener( PROPERTY_RELEVANT, this );
                m_bBindingControlsEnable = true;
            }

        }

    }

    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("forms.component");
    }

    // propagate our new value
    transferExternalValueToControl( _rInstanceLock );
    // if the binding is also a validator, use it, too. This is a constraint of the
    // com.sun.star.form.binding.ValidatableBindableFormComponent service
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
            DBG_UNHANDLED_EXCEPTION("forms.component");
        }

    }
}

void OBoundControlModel::disconnectExternalValueBinding( )
{
    try
    {
        // not listening at the binding anymore
        Reference< XModifyBroadcaster > xModifiable( m_xExternalBinding, UNO_QUERY );
        if ( xModifiable.is() )
            xModifiable->removeModifyListener( this );
        // remove as property change listener
        Reference< XPropertySet > xBindingProps( m_xExternalBinding, UNO_QUERY );
        if ( m_bBindingControlsRO )
            xBindingProps->removePropertyChangeListener( PROPERTY_READONLY, this );
        if ( m_bBindingControlsEnable )
            xBindingProps->removePropertyChangeListener( PROPERTY_RELEVANT, this );
    }

    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("forms.component");
        SAL_WARN("forms.component",  "OBoundControlModel::disconnectExternalValueBinding: caught an exception!");
    }

    // if the binding also acts as our validator, disconnect the validator, too
    if ( ( m_xExternalBinding == m_xValidator ) && m_xValidator.is() )
        disconnectValidator( );
    // no binding anymore
    m_xExternalBinding.clear();
    // be a load listener at our form, again. This was suspended while we had
    // an external value binding in place.
    doFormListening( true );
    // re-connect to database column of the new parent
    if ( m_xAmbientForm.is() && m_xAmbientForm->isLoaded() )
        impl_connectDatabaseColumn_noNotify( false );
}

void SAL_CALL OBoundControlModel::setValueBinding( const Reference< XValueBinding >& _rxBinding )
{
    OSL_PRECOND( m_bSupportsExternalBinding, "OBoundControlModel::setValueBinding: How did you reach this method?" );
    // the interface for this method should not have been exposed if we do not
    // support binding to external data
    // allow reset
    if ( _rxBinding.is() && !impl_approveValueBinding_nolock( _rxBinding ) )
    {
        throw IncompatibleTypesException(
            FRM_RES_STRING( RID_STR_INCOMPATIBLE_TYPES ),
            *this
        );
    }

    ControlModelLock aLock( *this );
    // since a ValueBinding overrules any potentially active database binding, the change in a ValueBinding
    // might trigger a change in our BoundField.
    FieldChangeNotifier aBoundFieldNotifier( aLock );
    // disconnect from the old binding
    if ( hasExternalValueBinding() )
        disconnectExternalValueBinding( );
    // connect to the new binding
    if ( _rxBinding.is() )
        connectExternalValueBinding( _rxBinding, aLock );
}

Reference< XValueBinding > SAL_CALL OBoundControlModel::getValueBinding(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    OSL_PRECOND( m_bSupportsExternalBinding, "OBoundControlModel::getValueBinding: How did you reach this method?" );
    // the interface for this method should not have been exposed if we do not
    // support binding to external data
    return m_xExternalBinding;
}

void SAL_CALL OBoundControlModel::modified( const EventObject& _rEvent )
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
        DBG_UNHANDLED_EXCEPTION("forms.component");
    }
}

void OBoundControlModel::transferExternalValueToControl( ControlModelLock& _rInstanceLock )
{
        Reference< XValueBinding > xExternalBinding( m_xExternalBinding );
        Type aValueExchangeType( getExternalValueType() );
        _rInstanceLock.release();
        // UNSAFE >
        Any aExternalValue;
        try
        {
            aExternalValue = xExternalBinding->getValue( aValueExchangeType );
        }

        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("forms.component");
        }
        // < UNSAFE
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
        m_bTransferingValue = true;
        _rInstanceLock.release();
         // UNSAFE >
        try
        {
            m_xExternalBinding->setValue( aExternalValue );
        }

        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("forms.component");
        }

        // < UNSAFE
        _rInstanceLock.acquire();
        m_bTransferingValue = false;
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
    for ( auto const & typeCandidate : aTypeCandidates )
    {
        if ( m_xExternalBinding->supportsType( typeCandidate ) )
        {
            m_aExternalValueType = typeCandidate;
            break;
        }
    }
}

Any OBoundControlModel::translateExternalValueToControlValue( const Any& _rExternalValue ) const
{
    OSL_PRECOND( m_bSupportsExternalBinding && hasExternalValueBinding(),
        "OBoundControlModel::translateExternalValueToControlValue: precondition not met!" );
    Any aControlValue( _rExternalValue );
    // if the external value is VOID, and our value property is not allowed to be VOID,
    // then default-construct a value
    if ( !aControlValue.hasValue() && !m_bValuePropertyMayBeVoid )
        aControlValue.setValue( nullptr, m_aValuePropertyType );
    // out of here
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
    // determine the current control value
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

    // add as value listener so we get notified when the value changes
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

    // add as value listener so we get notified when the value changes
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

void SAL_CALL OBoundControlModel::setValidator( const Reference< XValidator >& _rxValidator )
{
    ::osl::ClearableMutexGuard aGuard( m_aMutex );
    OSL_PRECOND( m_bSupportsValidation, "OBoundControlModel::setValidator: How did you reach this method?" );
    // the interface for this method should not have been exposed if we do not
    // support validation

    // early out if the validator does not change
    if ( _rxValidator == m_xValidator )
        return;

    if ( m_xValidator.is() && ( m_xValidator == m_xExternalBinding ) )
        throw VetoException(
            FRM_RES_STRING( RID_STR_INVALID_VALIDATOR ),
            *this
        );

    // disconnect from the old validator
    if ( hasValidator() )
        disconnectValidator( );

    // connect to the new validator
    if ( _rxValidator.is() )
        connectValidator( _rxValidator );
}

Reference< XValidator > SAL_CALL OBoundControlModel::getValidator(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    OSL_PRECOND( m_bSupportsValidation, "OBoundControlModel::getValidator: How did you reach this method?" );
    // the interface for this method should not have been exposed if we do not
    // support validation

    return m_xValidator;
}

void SAL_CALL OBoundControlModel::validityConstraintChanged( const EventObject& /*Source*/ )
{
    ::osl::ClearableMutexGuard aGuard( m_aMutex );
    OSL_PRECOND( m_bSupportsValidation, "OBoundControlModel::validityConstraintChanged: How did you reach this method?" );
    // the interface for this method should not have been exposed if we do not
    // support validation

    recheckValidity( false );
}

sal_Bool SAL_CALL OBoundControlModel::isValid(  )
{
    return m_bIsCurrentValueValid;
}

css::uno::Any OBoundControlModel::getCurrentFormComponentValue() const
{
    if ( hasValidator() )
        return translateControlValueToValidatableValue();
    return getControlValue();
}

Any SAL_CALL OBoundControlModel::getCurrentValue(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    return getCurrentFormComponentValue();
}

void SAL_CALL OBoundControlModel::addFormComponentValidityListener( const Reference< validation::XFormComponentValidityListener >& Listener )
{
    if ( Listener.is() )
        m_aFormComponentListeners.addInterface( Listener );
}

void SAL_CALL OBoundControlModel::removeFormComponentValidityListener( const Reference< validation::XFormComponentValidityListener >& Listener )
{
    if ( Listener.is() )
        m_aFormComponentListeners.removeInterface( Listener );
}

void OBoundControlModel::recheckValidity( bool _bForceNotification )
{
    try
    {
        bool bIsCurrentlyValid = true;
        if ( hasValidator() )
            bIsCurrentlyValid = m_xValidator->isValid( translateControlValueToValidatableValue() );

        if ( ( bIsCurrentlyValid != m_bIsCurrentValueValid ) || _bForceNotification )
        {
            m_bIsCurrentValueValid = bIsCurrentlyValid;

            // release our mutex for the notifications
            MutexRelease aRelease( m_aMutex );
            m_aFormComponentListeners.notifyEach( &validation::XFormComponentValidityListener::componentValidityChanged, EventObject( *this ) );
        }

    }

    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("forms.component");
        SAL_WARN("forms.component",  "OBoundControlModel::recheckValidity: caught an exception!");
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
