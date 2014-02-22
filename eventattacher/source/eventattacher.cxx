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
#include <osl/diagnose.h>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/beans/Introspection.hpp>
#include <com/sun/star/beans/MethodConcept.hpp>
#include <com/sun/star/script/XEventAttacher2.hpp>
#include <com/sun/star/script/Converter.hpp>
#include <com/sun/star/script/XAllListener.hpp>
#include <com/sun/star/script/InvocationAdapterFactory.hpp>
#include <com/sun/star/reflection/theCoreReflection.hpp>
#include <com/sun/star/reflection/XIdlReflection.hpp>


#include <com/sun/star/script/XInvocation.hpp>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase3.hxx>
#include <cppuhelper/supportsservice.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::registry;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::script;
using namespace com::sun::star::reflection;
using namespace cppu;
using namespace osl;



#define SERVICENAME "com.sun.star.script.EventAttacher"
#define IMPLNAME    "com.sun.star.comp.EventAttacher"

namespace comp_EventAttacher {





class InvocationToAllListenerMapper : public WeakImplHelper1< XInvocation >
{
public:
    InvocationToAllListenerMapper( const Reference< XIdlClass >& ListenerType,
        const Reference< XAllListener >& AllListener, const Any& Helper );

    
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



Reference< XInterface > createAllListenerAdapter
(
    const Reference< XInvocationAdapterFactory2 >& xInvocationAdapterFactory,
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
        Sequence<Type> arg2(1);
        arg2[0] = aListenerType;
        xAdapter = xInvocationAdapterFactory->createAdapter( xInvocationToAllListenerMapper, arg2 );
    }
    return xAdapter;
}




InvocationToAllListenerMapper::InvocationToAllListenerMapper
    ( const Reference< XIdlClass >& ListenerType, const Reference< XAllListener >& AllListener, const Any& Helper )
        : m_xAllListener( AllListener )
        , m_xListenerType( ListenerType )
        , m_Helper( Helper )
{
}


Reference< XIntrospectionAccess > SAL_CALL InvocationToAllListenerMapper::getIntrospection(void)
    throw( RuntimeException )
{
    return Reference< XIntrospectionAccess >();
}


Any SAL_CALL InvocationToAllListenerMapper::invoke(const OUString& FunctionName, const Sequence< Any >& Params,
    Sequence< sal_Int16 >& , Sequence< Any >& )
        throw( IllegalArgumentException, CannotConvertException,
        InvocationTargetException, RuntimeException )
{
    Any aRet;

    
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


void SAL_CALL InvocationToAllListenerMapper::setValue(const OUString& , const Any& )
    throw( UnknownPropertyException, CannotConvertException,
           InvocationTargetException, RuntimeException )
{
}


Any SAL_CALL InvocationToAllListenerMapper::getValue(const OUString& )
    throw( UnknownPropertyException, RuntimeException )
{
    return Any();
}


sal_Bool SAL_CALL InvocationToAllListenerMapper::hasMethod(const OUString& Name)
    throw( RuntimeException )
{
    Reference< XIdlMethod > xMethod = m_xListenerType->getMethod( Name );
    return xMethod.is();
}


sal_Bool SAL_CALL InvocationToAllListenerMapper::hasProperty(const OUString& Name)
    throw( RuntimeException )
{
    Reference< XIdlField > xField = m_xListenerType->getField( Name );
    return xField.is();
}





class EventAttacherImpl : public WeakImplHelper3 < XEventAttacher2, XInitialization, XServiceInfo >
{
public:
    EventAttacherImpl( const Reference< XComponentContext >& );
    ~EventAttacherImpl();

    
    virtual OUString SAL_CALL getImplementationName(  ) throw(RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(RuntimeException);
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(RuntimeException);
    static OUString SAL_CALL getImplementationName_Static(  );
    static Sequence< OUString > SAL_CALL getSupportedServiceNames_Static(  );

    
    virtual void SAL_CALL initialize( const Sequence< Any >& aArguments )
        throw( Exception, RuntimeException);

    
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

    
    virtual Sequence< Reference<XEventListener> > SAL_CALL attachMultipleEventListeners(
        const Reference<XInterface>& xObject, const Sequence<com::sun::star::script::EventListener>& aListeners )
            throw( IllegalArgumentException, ServiceNotRegisteredException, CannotCreateAdapterException, IntrospectionException, RuntimeException );

    
    Reference< XTypeConverter > getConverter() throw( Exception );

    friend class FilterAllListenerImpl;

private:
    Reference<XEventListener> attachListenerForTarget(
        const Reference<XIntrospectionAccess>& xAccess,
        const Reference<XInvocationAdapterFactory2>& xInvocationAdapterFactory,
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
    Reference< XComponentContext >      m_xContext;

    
    Reference< XIntrospection >             m_xIntrospection;
    Reference< XIdlReflection >             m_xReflection;
    Reference< XTypeConverter >             m_xConverter;
    Reference< XInvocationAdapterFactory2 >  m_xInvocationAdapterFactory;

    
    Reference< XIntrospection >             getIntrospection() throw( Exception );
    Reference< XIdlReflection >             getReflection() throw( Exception );
    Reference< XInvocationAdapterFactory2 >  getInvocationAdapterService() throw( Exception );
};



EventAttacherImpl::EventAttacherImpl( const Reference< XComponentContext >& rxContext )
    : m_xContext( rxContext )
{
}


EventAttacherImpl::~EventAttacherImpl()
{
}


Reference< XInterface > SAL_CALL EventAttacherImpl_CreateInstance( const Reference< XMultiServiceFactory >& rSMgr ) throw( Exception )
{
    Reference< XInterface > xRet;
    XEventAttacher *pEventAttacher = (XEventAttacher*) new EventAttacherImpl( comphelper::getComponentContext(rSMgr) );

    if (pEventAttacher)
    {
        xRet = Reference<XInterface>::query(pEventAttacher);
    }

    return xRet;
}


OUString SAL_CALL EventAttacherImpl::getImplementationName(  )
    throw(RuntimeException)
{
    return OUString(  IMPLNAME  );
}

sal_Bool SAL_CALL EventAttacherImpl::supportsService( const OUString& ServiceName )
    throw(RuntimeException)
{
    return cppu::supportsService(this, ServiceName);
}

Sequence<OUString> SAL_CALL EventAttacherImpl::getSupportedServiceNames(  )
    throw(RuntimeException)
{
    return getSupportedServiceNames_Static();
}


Sequence<OUString> SAL_CALL EventAttacherImpl::getSupportedServiceNames_Static(  )
{
    OUString aStr(  SERVICENAME  );
    return Sequence< OUString >( &aStr, 1 );
}


void SAL_CALL EventAttacherImpl::initialize(const Sequence< Any >& Arguments) throw( Exception, RuntimeException )
{
    
    const Any * pArray = Arguments.getConstArray();
    for( sal_Int32 i = 0; i < Arguments.getLength(); i++ )
    {
        if( pArray[i].getValueType().getTypeClass() != TypeClass_INTERFACE )
            throw IllegalArgumentException();

        
        Reference< XInvocationAdapterFactory2 > xALAS;
        pArray[i] >>= xALAS;
        if( xALAS.is() )
        {
            Guard< Mutex > aGuard( m_aMutex );
            m_xInvocationAdapterFactory = xALAS;
        }
        
        Reference< XIntrospection > xI;
        pArray[i] >>= xI;
        if( xI.is() )
        {
            Guard< Mutex > aGuard( m_aMutex );
            m_xIntrospection = xI;
        }
        
        Reference< XIdlReflection > xIdlR;
        pArray[i] >>= xIdlR;
        if( xIdlR.is() )
        {
            Guard< Mutex > aGuard( m_aMutex );
            m_xReflection = xIdlR;
        }
        
        Reference< XTypeConverter > xC;
        pArray[i] >>= xC;
        if( xC.is() )
        {
            Guard< Mutex > aGuard( m_aMutex );
            m_xConverter = xC;
        }

        
        if( !xALAS.is() && !xI.is() && !xIdlR.is() && !xC.is() )
            throw IllegalArgumentException();
    }
}



Reference< XIntrospection > EventAttacherImpl::getIntrospection() throw( Exception )
{
    Guard< Mutex > aGuard( m_aMutex );
    if( !m_xIntrospection.is() )
    {
        m_xIntrospection = Introspection::create( m_xContext );
    }
    return m_xIntrospection;
}



Reference< XIdlReflection > EventAttacherImpl::getReflection() throw( Exception )
{
    Guard< Mutex > aGuard( m_aMutex );
    if( !m_xReflection.is() )
    {
        m_xReflection = theCoreReflection::get(m_xContext);
    }
    return m_xReflection;
}



Reference< XInvocationAdapterFactory2 > EventAttacherImpl::getInvocationAdapterService() throw( Exception )
{
    Guard< Mutex > aGuard( m_aMutex );
    if( !m_xInvocationAdapterFactory.is() )
    {
        m_xInvocationAdapterFactory = InvocationAdapterFactory::create(m_xContext);
    }
    return m_xInvocationAdapterFactory;
}




Reference< XTypeConverter > EventAttacherImpl::getConverter() throw( Exception )
{
    Guard< Mutex > aGuard( m_aMutex );
    if( !m_xConverter.is() )
    {
        m_xConverter = Converter::create(m_xContext);
    }
    return m_xConverter;
}






class FilterAllListenerImpl : public WeakImplHelper1< XAllListener  >
{
public:
    FilterAllListenerImpl( EventAttacherImpl * pEA_, const OUString& EventMethod_,
                           const Reference< XAllListener >& AllListener_ );

    
    virtual void SAL_CALL firing(const AllEventObject& Event) throw( RuntimeException );
    virtual Any SAL_CALL approveFiring(const AllEventObject& Event) throw( InvocationTargetException, RuntimeException );

    
    virtual void SAL_CALL disposing(const EventObject& Source) throw( RuntimeException );

private:
    
    void convertToEventReturn( Any & rRet, const Type& rRetType )
            throw( CannotConvertException );

    EventAttacherImpl *         m_pEA;
    Reference< XInterface >     m_xEAHold;
    OUString                    m_EventMethod;
    Reference< XAllListener >   m_AllListener;
};


FilterAllListenerImpl::FilterAllListenerImpl( EventAttacherImpl * pEA_, const OUString& EventMethod_,
                                              const Reference< XAllListener >& AllListener_ )
        : m_pEA( pEA_ )
        , m_xEAHold( *pEA_ )
        , m_EventMethod( EventMethod_ )
        , m_AllListener( AllListener_ )
{
}


void SAL_CALL FilterAllListenerImpl::firing(const AllEventObject& Event)
    throw( RuntimeException )
{
    if( Event.MethodName == m_EventMethod && m_AllListener.is() )
        m_AllListener->firing( Event );
}



void FilterAllListenerImpl::convertToEventReturn( Any & rRet, const Type & rRetType )
    throw( CannotConvertException )
{
    
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
            throw CannotConvertException(); 
    }
}


Any SAL_CALL FilterAllListenerImpl::approveFiring( const AllEventObject& Event )
    throw( InvocationTargetException, RuntimeException )
{
    Any aRet;

    if( Event.MethodName == m_EventMethod && m_AllListener.is() )
        aRet = m_AllListener->approveFiring( Event );
    else
    {
        
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


void FilterAllListenerImpl::disposing(const EventObject& )
    throw( RuntimeException )
{
    
}



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

    Reference< XInvocationAdapterFactory2 > xInvocationAdapterFactory = getInvocationAdapterService();
    if( !xInvocationAdapterFactory.is() )
        throw ServiceNotRegisteredException();

    Reference< XIdlReflection > xReflection = getReflection();
    if( !xReflection.is() )
        throw ServiceNotRegisteredException();

    
    
    
    
    Reference< XIntrospection > xIntrospection = getIntrospection();
    if( !xIntrospection.is() )
        return Reference<XEventListener>();

    
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
    const Reference<XInvocationAdapterFactory2>& xInvocationAdapterFactory,
    const Reference<XAllListener>& xAllListener,
    const Any& aObject,
    const Any& aHelper,
    const OUString& aListenerType,
    const OUString& aAddListenerParam)
{
    Reference< XEventListener > xRet = NULL;

    
    sal_Int32 nIndex = aListenerType.lastIndexOf('.');
    
    if( nIndex == -1 )
        
        nIndex = 0;
    else
        nIndex++;

    OUString aListenerName = (aListenerType[nIndex] == 'X') ? aListenerType.copy(nIndex+1) : aListenerType;
    OUString aAddListenerName = "add" + aListenerName;

    
    Sequence< Reference< XIdlMethod > > aMethodSeq = xAccess->getMethods( MethodConcept::LISTENER );
    const Reference< XIdlMethod >* pMethods = aMethodSeq.getConstArray();
    for (sal_Int32 i = 0, n = aMethodSeq.getLength(); i < n ; ++i)
    {
        const Reference< XIdlMethod >& rxMethod = pMethods[i];

        
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

        
        Reference< XInterface > xAdapter = createAllListenerAdapter(
            xInvocationAdapterFactory, xListenerType, xAllListener, aHelper );

        if( !xAdapter.is() )
            throw CannotCreateAdapterException();
        xRet = Reference< XEventListener >( xAdapter, UNO_QUERY );

        
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
        
        else if( nParamCount == 2 )
        {
            Sequence< Any > args( 2 );
            Any* pAnys = args.getArray();

            
            Reference< XIdlClass > xParamClass = params.getConstArray()[0];
            if( xParamClass->getTypeClass() == TypeClass_STRING )
            {
                pAnys[0] <<= aAddListenerParam;
            }

            
            pAnys[1] <<= xAdapter;

            
            
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
        
        throw RuntimeException();

    if (!xObject.is())
        throw IllegalArgumentException();

    Reference< XInvocationAdapterFactory2 > xInvocationAdapterFactory = getInvocationAdapterService();
    if( !xInvocationAdapterFactory.is() )
        throw ServiceNotRegisteredException();

    Reference< XIdlReflection > xReflection = getReflection();
    if( !xReflection.is() )
        throw ServiceNotRegisteredException();

    
    
    
    
    Reference< XIntrospection > xIntrospection = getIntrospection();
    if( !xIntrospection.is() )
        return Sequence< Reference<XEventListener> >();

    
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
    
    Reference< XAllListener > aFilterListener = (XAllListener*)
        new FilterAllListenerImpl( this, EventMethod, AllListener );
    return attachListener( xObject, aFilterListener, Helper, ListenerType, AddListenerParam);
}


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

    
    
    
    
    Reference< XIntrospection > xIntrospection = getIntrospection();
    if( !xIntrospection.is() )
        throw IntrospectionException();

    
    Any aObjAny( &xObject, ::getCppuType( (const Reference< XInterface > *)0) );
    Reference< XIntrospectionAccess > xAccess = xIntrospection->inspect( aObjAny );
    if( !xAccess.is() )
        throw IntrospectionException();

    
    OUString aRemoveListenerName;
    OUString aListenerName( ListenerType );
    sal_Int32 nIndex = aListenerName.lastIndexOf( '.' );
    
    if( nIndex == -1 )
        
        nIndex = 0;
    else
        nIndex++;
    if( aListenerName[nIndex] == 'X' )
        
        aListenerName = aListenerName.copy( nIndex +1 );
    aRemoveListenerName = "remove" + aListenerName;

    
    Sequence< Reference< XIdlMethod > > aMethodSeq = xAccess->getMethods( MethodConcept::LISTENER );
    sal_uInt32 i, nLen = aMethodSeq.getLength();
    const Reference< XIdlMethod >* pMethods = aMethodSeq.getConstArray();
    for( i = 0 ; i < nLen ; i++ )
    {
        
        const Reference< XIdlMethod >& rxMethod = pMethods[i];

        
        if( aRemoveListenerName == rxMethod->getName() )
        {
            Sequence< Reference< XIdlClass > > params = rxMethod->getParameterTypes();
            sal_uInt32 nParamCount = params.getLength();

            
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
            
            else if( nParamCount == 2 )
            {
                Sequence< Any > args( 2 );
                Any* pAnys = args.getArray();

                
                Reference< XIdlClass > xParamClass = params.getConstArray()[0];
                if( xParamClass->getTypeClass() == TypeClass_STRING )
                    pAnys[0] <<= AddListenerParam;

                
                pAnys[1] <<= aToRemoveListener;

                
                
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
