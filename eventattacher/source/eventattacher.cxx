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
#include <osl/diagnose.h>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/beans/XIntrospection.hpp>
#include <com/sun/star/beans/MethodConcept.hpp>
#include <com/sun/star/script/XEventAttacher2.hpp>
#include <com/sun/star/script/Converter.hpp>
#include <com/sun/star/script/XAllListener.hpp>
#include <com/sun/star/script/XInvocationAdapterFactory.hpp>
#include <com/sun/star/reflection/XIdlReflection.hpp>

// InvocationToAllListenerMapper
#include <com/sun/star/script/XInvocation.hpp>
#include <comphelper/componentcontext.hxx>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase3.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::registry;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::script;
using namespace com::sun::star::reflection;
using namespace cppu;
using namespace osl;

using ::rtl::OUString;


#define SERVICENAME "com.sun.star.script.EventAttacher"
#define IMPLNAME    "com.sun.star.comp.EventAttacher"

namespace comp_EventAttacher {

//*************************************************************************
//  class InvocationToAllListenerMapper
//  helper class to map XInvocation to XAllListener
//*************************************************************************
class InvocationToAllListenerMapper : public WeakImplHelper1< XInvocation >
{
public:
    InvocationToAllListenerMapper( const Reference< XIdlClass >& ListenerType,
        const Reference< XAllListener >& AllListener, const Any& Helper );

    // XInvocation
    virtual Reference< XIntrospectionAccess > SAL_CALL getIntrospection(void) throw( RuntimeException );
    virtual Any SAL_CALL invoke(const OUString& FunctionName, const Sequence< Any >& Params, Sequence< sal_Int16 >& OutParamIndex, Sequence< Any >& OutParam)
        throw( IllegalArgumentException, CannotConvertException, InvocationTargetException, RuntimeException );
    virtual void SAL_CALL setValue(const OUString& PropertyName, const Any& Value)
        throw( UnknownPropertyException, CannotConvertException, InvocationTargetException, RuntimeException );
    virtual Any SAL_CALL getValue(const OUString& PropertyName) throw( UnknownPropertyException, RuntimeException );
    virtual sal_Bool SAL_CALL hasMethod(const OUString& Name) throw( RuntimeException );
    virtual sal_Bool SAL_CALL hasProperty(const OUString& Name) throw( RuntimeException );

private:
    Reference< XIdlReflection >  m_xCoreReflection;
    Reference< XAllListener >    m_xAllListener;
    Reference< XIdlClass >       m_xListenerType;
    Any                          m_Helper;
};


// Function to replace AllListenerAdapterService::createAllListerAdapter
Reference< XInterface > createAllListenerAdapter
(
    const Reference< XInvocationAdapterFactory >& xInvocationAdapterFactory,
    const Reference< XIdlClass >& xListenerType,
    const Reference< XAllListener >& xListener,
    const Any& Helper
)
{
    Reference< XInterface > xAdapter;
    if( xInvocationAdapterFactory.is() && xListenerType.is() && xListener.is() )
    {
       Reference< XInvocation > xInvocationToAllListenerMapper =
            (XInvocation*)new InvocationToAllListenerMapper( xListenerType, xListener, Helper );
        Type aListenerType( xListenerType->getTypeClass(), xListenerType->getName());
        xAdapter = xInvocationAdapterFactory->createAdapter( xInvocationToAllListenerMapper, aListenerType );
    }
    return xAdapter;
}


//--------------------------------------------------------------------------------------------------
// InvocationToAllListenerMapper
InvocationToAllListenerMapper::InvocationToAllListenerMapper
    ( const Reference< XIdlClass >& ListenerType, const Reference< XAllListener >& AllListener, const Any& Helper )
        : m_xAllListener( AllListener )
        , m_xListenerType( ListenerType )
        , m_Helper( Helper )
{
}

//*************************************************************************
Reference< XIntrospectionAccess > SAL_CALL InvocationToAllListenerMapper::getIntrospection(void)
    throw( RuntimeException )
{
    return Reference< XIntrospectionAccess >();
}

//*************************************************************************
Any SAL_CALL InvocationToAllListenerMapper::invoke(const OUString& FunctionName, const Sequence< Any >& Params,
    Sequence< sal_Int16 >& , Sequence< Any >& )
        throw( IllegalArgumentException, CannotConvertException,
        InvocationTargetException, RuntimeException )
{
    Any aRet;

    // Check if to firing or approveFiring has to be called
    Reference< XIdlMethod > xMethod = m_xListenerType->getMethod( FunctionName );
    sal_Bool bApproveFiring = sal_False;
    if( !xMethod.is() )
        return aRet;
    Reference< XIdlClass > xReturnType = xMethod->getReturnType();
    Sequence< Reference< XIdlClass > > aExceptionSeq = xMethod->getExceptionTypes();
    if( ( xReturnType.is() && xReturnType->getTypeClass() != TypeClass_VOID ) ||
        aExceptionSeq.getLength() > 0 )
    {
        bApproveFiring = sal_True;
    }
    else
    {
        Sequence< ParamInfo > aParamSeq = xMethod->getParameterInfos();
        sal_uInt32 nParamCount = aParamSeq.getLength();
        if( nParamCount > 1 )
        {
            const ParamInfo* pInfos = aParamSeq.getConstArray();
            for( sal_uInt32 i = 0 ; i < nParamCount ; i++ )
            {
                if( pInfos[ i ].aMode != ParamMode_IN )
                {
                    bApproveFiring = sal_True;
                    break;
                }
            }
        }
    }

    AllEventObject aAllEvent;
    aAllEvent.Source = (OWeakObject*) this;
    aAllEvent.Helper = m_Helper;
    aAllEvent.ListenerType = Type(m_xListenerType->getTypeClass(), m_xListenerType->getName());
    aAllEvent.MethodName = FunctionName;
    aAllEvent.Arguments = Params;
    if( bApproveFiring )
        aRet = m_xAllListener->approveFiring( aAllEvent );
    else
        m_xAllListener->firing( aAllEvent );
    return aRet;
}

//*************************************************************************
void SAL_CALL InvocationToAllListenerMapper::setValue(const OUString& , const Any& )
    throw( UnknownPropertyException, CannotConvertException,
           InvocationTargetException, RuntimeException )
{
}

//*************************************************************************
Any SAL_CALL InvocationToAllListenerMapper::getValue(const OUString& )
    throw( UnknownPropertyException, RuntimeException )
{
    return Any();
}

//*************************************************************************
sal_Bool SAL_CALL InvocationToAllListenerMapper::hasMethod(const OUString& Name)
    throw( RuntimeException )
{
    Reference< XIdlMethod > xMethod = m_xListenerType->getMethod( Name );
    return xMethod.is();
}

//*************************************************************************
sal_Bool SAL_CALL InvocationToAllListenerMapper::hasProperty(const OUString& Name)
    throw( RuntimeException )
{
    Reference< XIdlField > xField = m_xListenerType->getField( Name );
    return xField.is();
}

//*************************************************************************
//  class EventAttacherImpl
//  represents an implementation of the EventAttacher service
//*************************************************************************
class EventAttacherImpl : public WeakImplHelper3 < XEventAttacher2, XInitialization, XServiceInfo >
{
public:
    EventAttacherImpl( const Reference< XMultiServiceFactory >& );
    ~EventAttacherImpl();

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw(RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(RuntimeException);
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(RuntimeException);
    static OUString SAL_CALL getImplementationName_Static(  );
    static Sequence< OUString > SAL_CALL getSupportedServiceNames_Static(  );

    // XInitialization
    virtual void SAL_CALL initialize( const Sequence< Any >& aArguments )
        throw( Exception, RuntimeException);

    // Methoden von XEventAttacher
    virtual Reference< XEventListener > SAL_CALL attachListener(const Reference< XInterface >& xObject,
            const Reference< XAllListener >& AllListener, const Any& Helper,
            const OUString& ListenerType, const OUString& AddListenerParam)
        throw( IllegalArgumentException, ServiceNotRegisteredException, CannotCreateAdapterException, IntrospectionException, RuntimeException );
    virtual Reference< XEventListener > SAL_CALL attachSingleEventListener(const Reference< XInterface >& xObject,
            const Reference< XAllListener >& AllListener, const Any& Helper,
            const OUString& ListenerType, const OUString& AddListenerParam,
            const OUString& EventMethod)
        throw( IllegalArgumentException, ServiceNotRegisteredException, CannotCreateAdapterException, IntrospectionException, RuntimeException );
    virtual void SAL_CALL removeListener(const Reference< XInterface >& xObject,
            const OUString& ListenerType, const OUString& AddListenerParam,
            const Reference< XEventListener >& aToRemoveListener)
        throw( IllegalArgumentException, IntrospectionException, RuntimeException );

    // XEventAttacher2
    virtual Sequence< Reference<XEventListener> > SAL_CALL attachMultipleEventListeners(
        const Reference<XInterface>& xObject, const Sequence<com::sun::star::script::EventListener>& aListeners )
            throw( IllegalArgumentException, ServiceNotRegisteredException, CannotCreateAdapterException, IntrospectionException, RuntimeException );

    // used by FilterAllListener_Impl
    Reference< XTypeConverter > getConverter() throw( Exception );

    friend class FilterAllListenerImpl;

private:
    Reference<XEventListener> attachListenerForTarget(
        const Reference<XIntrospectionAccess>& xAccess,
        const Reference<XInvocationAdapterFactory>& xInvocationAdapterFactory,
        const Reference<XAllListener>& xAllListener,
        const Any& aObject,
        const Any& aHelper,
        const OUString& aListenerType,
        const OUString& aAddListenerParam );

    Sequence< Reference<XEventListener> > attachListeners(
        const Reference<XInterface>& xObject,
        const Sequence< Reference<XAllListener> >& AllListeners,
        const Sequence<com::sun::star::script::EventListener>& aListeners );

private:
    Mutex                               m_aMutex;
    Reference< XMultiServiceFactory >   m_xSMgr;

    // Save Services
    Reference< XIntrospection >             m_xIntrospection;
    Reference< XIdlReflection >             m_xReflection;
    Reference< XTypeConverter >             m_xConverter;
    Reference< XInvocationAdapterFactory >  m_xInvocationAdapterFactory;

    // needed services
    Reference< XIntrospection >             getIntrospection() throw( Exception );
    Reference< XIdlReflection >             getReflection() throw( Exception );
    Reference< XInvocationAdapterFactory >  getInvocationAdapterService() throw( Exception );
};


//*************************************************************************
EventAttacherImpl::EventAttacherImpl( const Reference< XMultiServiceFactory >& rSMgr )
    : m_xSMgr( rSMgr )
{
}

//*************************************************************************
EventAttacherImpl::~EventAttacherImpl()
{
}

//*************************************************************************
Reference< XInterface > SAL_CALL EventAttacherImpl_CreateInstance( const Reference< XMultiServiceFactory >& rSMgr ) throw( Exception )
{
    Reference< XInterface > xRet;
    XEventAttacher *pEventAttacher = (XEventAttacher*) new EventAttacherImpl(rSMgr);

    if (pEventAttacher)
    {
        xRet = Reference<XInterface>::query(pEventAttacher);
    }

    return xRet;
}

//*************************************************************************
OUString SAL_CALL EventAttacherImpl::getImplementationName(  )
    throw(RuntimeException)
{
    return OUString(  IMPLNAME  );
}

//*************************************************************************
sal_Bool SAL_CALL EventAttacherImpl::supportsService( const OUString& ServiceName )
    throw(RuntimeException)
{
    Sequence< OUString > aSNL = getSupportedServiceNames();
    const OUString * pArray = aSNL.getArray();
    for( sal_Int32 i = 0; i < aSNL.getLength(); i++ )
        if( pArray[i] == ServiceName )
            return sal_True;
    return sal_False;
}

//*************************************************************************
Sequence<OUString> SAL_CALL EventAttacherImpl::getSupportedServiceNames(  )
    throw(RuntimeException)
{
    return getSupportedServiceNames_Static();
}

//*************************************************************************
Sequence<OUString> SAL_CALL EventAttacherImpl::getSupportedServiceNames_Static(  )
{
    OUString aStr(  SERVICENAME  );
    return Sequence< OUString >( &aStr, 1 );
}

//*************************************************************************
void SAL_CALL EventAttacherImpl::initialize(const Sequence< Any >& Arguments) throw( Exception, RuntimeException )
{
    // get services from the argument list
    const Any * pArray = Arguments.getConstArray();
    for( sal_Int32 i = 0; i < Arguments.getLength(); i++ )
    {
        if( pArray[i].getValueType().getTypeClass() != TypeClass_INTERFACE )
            throw IllegalArgumentException();

        // InvocationAdapter service ?
        Reference< XInvocationAdapterFactory > xALAS;
        pArray[i] >>= xALAS;
        if( xALAS.is() )
        {
            Guard< Mutex > aGuard( m_aMutex );
            m_xInvocationAdapterFactory = xALAS;
        }
        // Introspection service ?
        Reference< XIntrospection > xI;
        pArray[i] >>= xI;
        if( xI.is() )
        {
            Guard< Mutex > aGuard( m_aMutex );
            m_xIntrospection = xI;
        }
        // Reflection service ?
        Reference< XIdlReflection > xIdlR;
        pArray[i] >>= xIdlR;
        if( xIdlR.is() )
        {
            Guard< Mutex > aGuard( m_aMutex );
            m_xReflection = xIdlR;
        }
        // Converter Service ?
        Reference< XTypeConverter > xC;
        pArray[i] >>= xC;
        if( xC.is() )
        {
            Guard< Mutex > aGuard( m_aMutex );
            m_xConverter = xC;
        }

        // no right interface
        if( !xALAS.is() && !xI.is() && !xIdlR.is() && !xC.is() )
            throw IllegalArgumentException();
    }
}

//*************************************************************************
//*** Private helper methods ***
Reference< XIntrospection > EventAttacherImpl::getIntrospection() throw( Exception )
{
    Guard< Mutex > aGuard( m_aMutex );
    if( !m_xIntrospection.is() )
    {
        Reference< XInterface > xIFace( m_xSMgr->createInstance( rtl::OUString("com.sun.star.beans.Introspection") ) );
        m_xIntrospection = Reference< XIntrospection >( xIFace, UNO_QUERY );
    }
    return m_xIntrospection;
}

//*************************************************************************
//*** Private helper methods ***
Reference< XIdlReflection > EventAttacherImpl::getReflection() throw( Exception )
{
    Guard< Mutex > aGuard( m_aMutex );
    if( !m_xReflection.is() )
    {
        Reference< XInterface > xIFace( m_xSMgr->createInstance( rtl::OUString("com.sun.star.reflection.CoreReflection") ) );
        m_xReflection = Reference< XIdlReflection >( xIFace, UNO_QUERY);
    }
    return m_xReflection;
}

//*************************************************************************
//*** Private helper methods ***
Reference< XInvocationAdapterFactory > EventAttacherImpl::getInvocationAdapterService() throw( Exception )
{
    Guard< Mutex > aGuard( m_aMutex );
    if( !m_xInvocationAdapterFactory.is() )
    {
        Reference< XInterface > xIFace( m_xSMgr->createInstance( rtl::OUString("com.sun.star.script.InvocationAdapterFactory") ) );
        m_xInvocationAdapterFactory = Reference< XInvocationAdapterFactory >( xIFace, UNO_QUERY );
    }
    return m_xInvocationAdapterFactory;
}


//*************************************************************************
//*** Private helper methods ***
Reference< XTypeConverter > EventAttacherImpl::getConverter() throw( Exception )
{
    Guard< Mutex > aGuard( m_aMutex );
    if( !m_xConverter.is() )
    {
        m_xConverter = Converter::create(comphelper::ComponentContext(m_xSMgr).getUNOContext());
    }
    return m_xConverter;
}

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
// Implementation of an EventAttacher-related AllListeners, which brings
// a few Events to a general AllListener
class FilterAllListenerImpl : public WeakImplHelper1< XAllListener  >
{
public:
    FilterAllListenerImpl( EventAttacherImpl * pEA_, const OUString& EventMethod_,
                           const Reference< XAllListener >& AllListener_ );

    // XAllListener
    virtual void SAL_CALL firing(const AllEventObject& Event) throw( RuntimeException );
    virtual Any SAL_CALL approveFiring(const AllEventObject& Event) throw( InvocationTargetException, RuntimeException );

    // XEventListener
    virtual void SAL_CALL disposing(const EventObject& Source) throw( RuntimeException );

private:
    // convert
    void convertToEventReturn( Any & rRet, const Type& rRetType )
            throw( CannotConvertException );

    EventAttacherImpl *         m_pEA;
    Reference< XInterface >     m_xEAHold;
    OUString                    m_EventMethod;
    Reference< XAllListener >   m_AllListener;
};

//*************************************************************************
FilterAllListenerImpl::FilterAllListenerImpl( EventAttacherImpl * pEA_, const OUString& EventMethod_,
                                              const Reference< XAllListener >& AllListener_ )
        : m_pEA( pEA_ )
        , m_xEAHold( *pEA_ )
        , m_EventMethod( EventMethod_ )
        , m_AllListener( AllListener_ )
{
}

//*************************************************************************
void SAL_CALL FilterAllListenerImpl::firing(const AllEventObject& Event)
    throw( RuntimeException )
{
    if( Event.MethodName == m_EventMethod && m_AllListener.is() )
        m_AllListener->firing( Event );
}

//*************************************************************************
// Convert to the standard event return
void FilterAllListenerImpl::convertToEventReturn( Any & rRet, const Type & rRetType )
    throw( CannotConvertException )
{
    // no return value? Set to the specified values
    if( rRet.getValueType().getTypeClass() == TypeClass_VOID )
    {
        switch( rRetType.getTypeClass()  )
        {
            case TypeClass_INTERFACE:
                {
                rRet <<= Reference< XInterface >();
                }
                break;

            case TypeClass_BOOLEAN:
                rRet <<= sal_True;
                break;

            case TypeClass_STRING:
                rRet <<= OUString();
                break;

            case TypeClass_FLOAT:           rRet <<= float(0);  break;
            case TypeClass_DOUBLE:          rRet <<= double(0.0);   break;
            case TypeClass_BYTE:            rRet <<= sal_uInt8( 0 );    break;
            case TypeClass_SHORT:           rRet <<= sal_Int16( 0 );    break;
            case TypeClass_LONG:            rRet <<= sal_Int32( 0 );    break;
            case TypeClass_UNSIGNED_SHORT:  rRet <<= sal_uInt16( 0 );   break;
            case TypeClass_UNSIGNED_LONG:   rRet <<= sal_uInt32( 0 );   break;
                     default:
            break;
        }
    }
    else if( !rRet.getValueType().equals( rRetType ) )
    {
        Reference< XTypeConverter > xConverter = m_pEA->getConverter();
        if( xConverter.is() )
            rRet = xConverter->convertTo( rRet, rRetType );
        else
            throw CannotConvertException(); // TODO TypeConversionException
    }
}

//*************************************************************************
Any SAL_CALL FilterAllListenerImpl::approveFiring( const AllEventObject& Event )
    throw( InvocationTargetException, RuntimeException )
{
    Any aRet;

    if( Event.MethodName == m_EventMethod && m_AllListener.is() )
        aRet = m_AllListener->approveFiring( Event );
    else
    {
        // Convert to the standard event return
        try
        {
            Reference< XIdlClass > xListenerType = m_pEA->getReflection()->
                        forName( Event.ListenerType.getTypeName() );
            Reference< XIdlMethod > xMeth = xListenerType->getMethod( Event.MethodName );
            if( xMeth.is() )
            {
                Reference< XIdlClass > xRetType = xMeth->getReturnType();
                Type aRetType( xRetType->getTypeClass(), xRetType->getName() );
                convertToEventReturn( aRet, aRetType );
            }
        }
        catch( const CannotConvertException& e )
        {
            throw InvocationTargetException( OUString(), Reference< XInterface >(), Any(&e, ::getCppuType( (CannotConvertException*)0)) );
        }
    }
    return aRet;
}

//*************************************************************************
void FilterAllListenerImpl::disposing(const EventObject& )
    throw( RuntimeException )
{
    // TODO: ???
}


//*************************************************************************
Reference< XEventListener > EventAttacherImpl::attachListener
(
    const Reference< XInterface >& xObject,
    const Reference< XAllListener >& AllListener,
    const Any& Helper,
    const OUString& ListenerType,
    const OUString& AddListenerParam
)
    throw( IllegalArgumentException, ServiceNotRegisteredException, CannotCreateAdapterException, IntrospectionException, RuntimeException )
{
    if( !xObject.is() || !AllListener.is() )
        throw IllegalArgumentException();

    Reference< XInvocationAdapterFactory > xInvocationAdapterFactory = getInvocationAdapterService();
    if( !xInvocationAdapterFactory.is() )
        throw ServiceNotRegisteredException();

    Reference< XIdlReflection > xReflection = getReflection();
    if( !xReflection.is() )
        throw ServiceNotRegisteredException();

    // Sign in, Call the fitting addListener method
    // First Introspection, as the Methods can be analyzed in the same way
    // For better perfomance it is implemented here again or make the Impl-Method
    // of the Introspection configurable for this purpose.
    Reference< XIntrospection > xIntrospection = getIntrospection();
    if( !xIntrospection.is() )
        return Reference<XEventListener>();

    // Inspect Introspection
    Any aObjAny( &xObject, ::getCppuType( (const Reference< XInterface > *)0) );

    Reference< XIntrospectionAccess > xAccess = xIntrospection->inspect( aObjAny );
    if( !xAccess.is() )
        return Reference<XEventListener>();

    return attachListenerForTarget(
        xAccess, xInvocationAdapterFactory, AllListener, aObjAny, Helper,
        ListenerType, AddListenerParam);
}

Reference<XEventListener> EventAttacherImpl::attachListenerForTarget(
    const Reference<XIntrospectionAccess>& xAccess,
    const Reference<XInvocationAdapterFactory>& xInvocationAdapterFactory,
    const Reference<XAllListener>& xAllListener,
    const Any& aObject,
    const Any& aHelper,
    const OUString& aListenerType,
    const OUString& aAddListenerParam)
{
    Reference< XEventListener > xRet = NULL;

    // Construct the name of the addListener-Method.
    sal_Int32 nIndex = aListenerType.lastIndexOf('.');
    // set index to the interface name without package name
    if( nIndex == -1 )
        // not found
        nIndex = 0;
    else
        nIndex++;

    OUString aListenerName = (aListenerType[nIndex] == 'X') ? aListenerType.copy(nIndex+1) : aListenerType;
    OUString aAddListenerName = OUString("add") + aListenerName;

    // Send Methods to the correct addListener-Method
    Sequence< Reference< XIdlMethod > > aMethodSeq = xAccess->getMethods( MethodConcept::LISTENER );
    const Reference< XIdlMethod >* pMethods = aMethodSeq.getConstArray();
    for (sal_Int32 i = 0, n = aMethodSeq.getLength(); i < n ; ++i)
    {
        const Reference< XIdlMethod >& rxMethod = pMethods[i];

        // Is it the correct method?
        OUString aMethName = rxMethod->getName();

        if (aAddListenerName != aMethName)
            continue;

        Sequence< Reference< XIdlClass > > params = rxMethod->getParameterTypes();
        sal_uInt32 nParamCount = params.getLength();

        Reference< XIdlClass > xListenerType;
        if( nParamCount == 1 )
            xListenerType = params.getConstArray()[0];
        else if( nParamCount == 2 )
            xListenerType = params.getConstArray()[1];

        // Request Adapter for the actual Listener type
        Reference< XInterface > xAdapter = createAllListenerAdapter(
            xInvocationAdapterFactory, xListenerType, xAllListener, aHelper );

        if( !xAdapter.is() )
            throw CannotCreateAdapterException();
        xRet = Reference< XEventListener >( xAdapter, UNO_QUERY );

        // Just the Listener as parameter?
        if( nParamCount == 1 )
        {
            Sequence< Any > args( 1 );
            args.getArray()[0] <<= xAdapter;
            try
            {
                rxMethod->invoke( aObject, args );
            }
            catch( const InvocationTargetException& )
            {
                throw IntrospectionException();
            }
        }
        // Else, pass the other parameter now
        else if( nParamCount == 2 )
        {
            Sequence< Any > args( 2 );
            Any* pAnys = args.getArray();

            // Check the type of the 1st parameter
            Reference< XIdlClass > xParamClass = params.getConstArray()[0];
            if( xParamClass->getTypeClass() == TypeClass_STRING )
            {
                pAnys[0] <<= aAddListenerParam;
            }

            // 2nd Parameter == Listener? TODO: Test!
            pAnys[1] <<= xAdapter;

            // TODO: Convert String -> ?
            // else
            try
            {
                rxMethod->invoke( aObject, args );
            }
            catch( const InvocationTargetException& )
            {
                throw IntrospectionException();
            }
        }
        break;
        // else...
        // Anything else is not supported
    }

    return xRet;
}

Sequence< Reference<XEventListener> > EventAttacherImpl::attachListeners(
    const Reference<XInterface>& xObject,
    const Sequence< Reference<XAllListener> >& AllListeners,
    const Sequence<com::sun::star::script::EventListener>& aListeners )
{
    sal_Int32 nCount = aListeners.getLength();
    if (nCount != AllListeners.getLength())
        // This is a prerequisite!
        throw RuntimeException();

    if (!xObject.is())
        throw IllegalArgumentException();

    Reference< XInvocationAdapterFactory > xInvocationAdapterFactory = getInvocationAdapterService();
    if( !xInvocationAdapterFactory.is() )
        throw ServiceNotRegisteredException();

    Reference< XIdlReflection > xReflection = getReflection();
    if( !xReflection.is() )
        throw ServiceNotRegisteredException();

    // Sign in, Call the fitting addListener method
    // First Introspection, as the Methods can be analyzed in the same way
    // For better perfomance it is implemented here again or make the Impl-Method
    // of the Introspection configurable for this purpose.
    Reference< XIntrospection > xIntrospection = getIntrospection();
    if( !xIntrospection.is() )
        return Sequence< Reference<XEventListener> >();

    // Inspect Introspection
    Any aObjAny( &xObject, ::getCppuType(static_cast<const Reference<XInterface>*>(0)) );

    Reference<XIntrospectionAccess> xAccess = xIntrospection->inspect(aObjAny);
    if (!xAccess.is())
        return Sequence< Reference<XEventListener> >();

    Sequence< Reference<XEventListener> > aRet(nCount);
    Reference<XEventListener>* pArray = aRet.getArray();

    for (sal_Int32 i = 0; i < nCount; ++i)
    {
        pArray[i] = attachListenerForTarget(
            xAccess, xInvocationAdapterFactory, AllListeners[ i ],
            aObjAny, aListeners[i].Helper, aListeners[i].ListenerType, aListeners[i].AddListenerParam);
    }

    return aRet;
}

// XEventAttacher
Reference< XEventListener > EventAttacherImpl::attachSingleEventListener
(
    const Reference< XInterface >& xObject,
    const Reference< XAllListener >& AllListener,
    const Any& Helper,
    const OUString& ListenerType,
    const OUString& AddListenerParam,
    const OUString& EventMethod
)
    throw( IllegalArgumentException, ServiceNotRegisteredException, CannotCreateAdapterException, IntrospectionException, RuntimeException )
{
    // Subscribe FilterListener
    Reference< XAllListener > aFilterListener = (XAllListener*)
        new FilterAllListenerImpl( this, EventMethod, AllListener );
    return attachListener( xObject, aFilterListener, Helper, ListenerType, AddListenerParam);
}

// XEventAttacher
void EventAttacherImpl::removeListener
(
    const Reference< XInterface >& xObject,
    const OUString& ListenerType,
    const OUString& AddListenerParam,
    const Reference< XEventListener >& aToRemoveListener
)
    throw( IllegalArgumentException, IntrospectionException, RuntimeException )
{
    if( !xObject.is() || !aToRemoveListener.is() )
        throw IllegalArgumentException();

    Reference< XIdlReflection > xReflection = getReflection();
    if( !xReflection.is() )
        throw IntrospectionException();

    // Sign off, Call the fitting removeListener method
    // First Introspection, as the Methods can be analyzed in the same way
    // For better perfomance it is implemented here again or make the Impl-Method
    // of the Introspection configurable for this purpose.
    Reference< XIntrospection > xIntrospection = getIntrospection();
    if( !xIntrospection.is() )
        throw IntrospectionException();

    //Inspect Introspection
    Any aObjAny( &xObject, ::getCppuType( (const Reference< XInterface > *)0) );
    Reference< XIntrospectionAccess > xAccess = xIntrospection->inspect( aObjAny );
    if( !xAccess.is() )
        throw IntrospectionException();

    // Create name of the removeListener-Method
    OUString aRemoveListenerName;
    OUString aListenerName( ListenerType );
    sal_Int32 nIndex = aListenerName.lastIndexOf( '.' );
    // set index to the interface name without package name
    if( nIndex == -1 )
        // not found
        nIndex = 0;
    else
        nIndex++;
    if( aListenerName[nIndex] == 'X' )
        // erase X from the interface name
        aListenerName = aListenerName.copy( nIndex +1 );
    aRemoveListenerName = OUString( "remove" ) + aListenerName;

    // Search methods for the correct removeListener method
    Sequence< Reference< XIdlMethod > > aMethodSeq = xAccess->getMethods( MethodConcept::LISTENER );
    sal_uInt32 i, nLen = aMethodSeq.getLength();
    const Reference< XIdlMethod >* pMethods = aMethodSeq.getConstArray();
    for( i = 0 ; i < nLen ; i++ )
    {
        // Call Methode
        const Reference< XIdlMethod >& rxMethod = pMethods[i];

        // Is it the right method?
        if( aRemoveListenerName == rxMethod->getName() )
        {
            Sequence< Reference< XIdlClass > > params = rxMethod->getParameterTypes();
            sal_uInt32 nParamCount = params.getLength();

            // Just the Listener as parameter?
            if( nParamCount == 1 )
            {
                Sequence< Any > args( 1 );
                args.getArray()[0] <<= aToRemoveListener;
                try
                {
                    rxMethod->invoke( aObjAny, args );
                }
                catch( const InvocationTargetException& )
                {
                    throw IntrospectionException();
                }
            }
            // Else pass the other parameter
            else if( nParamCount == 2 )
            {
                Sequence< Any > args( 2 );
                Any* pAnys = args.getArray();

                // Check the type of the 1st parameter
                Reference< XIdlClass > xParamClass = params.getConstArray()[0];
                if( xParamClass->getTypeClass() == TypeClass_STRING )
                    pAnys[0] <<= AddListenerParam;

                // 2nd parameter == Listener? TODO: Test!
                pAnys[1] <<= aToRemoveListener;

                // TODO: Convert String -> ?
                // else
                try
                {
                    rxMethod->invoke( aObjAny, args );
                }
                catch( const InvocationTargetException& )
                {
                    throw IntrospectionException();
                }
            }
            break;
        }
    }
}

Sequence< Reference<XEventListener> > EventAttacherImpl::attachMultipleEventListeners(
    const Reference<XInterface>& xObject, const Sequence<com::sun::star::script::EventListener>& aListeners )
        throw( IllegalArgumentException, ServiceNotRegisteredException, CannotCreateAdapterException, IntrospectionException, RuntimeException )
{
    sal_Int32 nCount = aListeners.getLength();
    Sequence< Reference<XAllListener> > aFilterListeners(nCount);
    for (sal_Int32 i = 0; i < nCount; ++i)
    {
        aFilterListeners[i] = (XAllListener*)
            new FilterAllListenerImpl(this, aListeners[i].EventMethod, aListeners[i].AllListener);
    }

    return attachListeners(xObject, aFilterListeners, aListeners);
}

}

extern "C"
{
SAL_DLLPUBLIC_EXPORT void * SAL_CALL evtatt_component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * )
{
    void * pRet = 0;

    if (pServiceManager && rtl_str_compare( pImplName, IMPLNAME ) == 0)
    {
        Reference< XSingleServiceFactory > xFactory( createOneInstanceFactory(
            reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
            OUString(  IMPLNAME  ),
            ::comp_EventAttacher::EventAttacherImpl_CreateInstance,
            ::comp_EventAttacher::EventAttacherImpl::getSupportedServiceNames_Static() ) );

        if (xFactory.is())
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }

    return pRet;
}
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
