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
#include <osl/diagnose.h>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/beans/XIntrospection.hpp>
#include <com/sun/star/beans/MethodConcept.hpp>
#include <com/sun/star/script/XEventAttacher.hpp>
#include <com/sun/star/script/XTypeConverter.hpp>
#include <com/sun/star/script/XAllListener.hpp>
#include <com/sun/star/script/XInvocationAdapterFactory.hpp>
#include <com/sun/star/reflection/XIdlReflection.hpp>

// InvocationToAllListenerMapper
#include <com/sun/star/script/XInvocation.hpp>
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
using namespace rtl;


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
class EventAttacherImpl : public WeakImplHelper3 < XEventAttacher, XInitialization, XServiceInfo >
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

    // used by FilterAllListener_Impl
    Reference< XTypeConverter > getConverter() throw( Exception );

    friend class FilterAllListenerImpl;
private:
    Mutex                               m_aMutex;
    Reference< XMultiServiceFactory >   m_xSMgr;

    // Services merken
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
    return OUString( RTL_CONSTASCII_USTRINGPARAM( IMPLNAME ) );
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
    OUString aStr( RTL_CONSTASCII_USTRINGPARAM( SERVICENAME ) );
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
//*** Private Hilfs-Methoden ***
Reference< XIntrospection > EventAttacherImpl::getIntrospection() throw( Exception )
{
    Guard< Mutex > aGuard( m_aMutex );
    // Haben wir den Service schon? Sonst anlegen
    if( !m_xIntrospection.is() )
    {
        Reference< XInterface > xIFace( m_xSMgr->createInstance( rtl::OUString::createFromAscii("com.sun.star.beans.Introspection") ) );
        m_xIntrospection = Reference< XIntrospection >( xIFace, UNO_QUERY );
    }
    return m_xIntrospection;
}

//*************************************************************************
//*** Private Hilfs-Methoden ***
Reference< XIdlReflection > EventAttacherImpl::getReflection() throw( Exception )
{
    Guard< Mutex > aGuard( m_aMutex );
    // Haben wir den Service schon? Sonst anlegen
    if( !m_xReflection.is() )
    {
        Reference< XInterface > xIFace( m_xSMgr->createInstance( rtl::OUString::createFromAscii("com.sun.star.reflection.CoreReflection") ) );
        m_xReflection = Reference< XIdlReflection >( xIFace, UNO_QUERY);
    }
    return m_xReflection;
}

//*************************************************************************
//*** Private Hilfs-Methoden ***
Reference< XInvocationAdapterFactory > EventAttacherImpl::getInvocationAdapterService() throw( Exception )
{
    Guard< Mutex > aGuard( m_aMutex );
    // Haben wir den Service schon? Sonst anlegen
    if( !m_xInvocationAdapterFactory.is() )
    {
        Reference< XInterface > xIFace( m_xSMgr->createInstance( rtl::OUString::createFromAscii("com.sun.star.script.InvocationAdapterFactory") ) );
        m_xInvocationAdapterFactory = Reference< XInvocationAdapterFactory >( xIFace, UNO_QUERY );
    }
    return m_xInvocationAdapterFactory;
}


//*************************************************************************
//*** Private Hilfs-Methoden ***
Reference< XTypeConverter > EventAttacherImpl::getConverter() throw( Exception )
{
    Guard< Mutex > aGuard( m_aMutex );
    // Haben wir den Service schon? Sonst anlegen
    if( !m_xConverter.is() )
    {
        Reference< XInterface > xIFace( m_xSMgr->createInstance( rtl::OUString::createFromAscii("com.sun.star.script.Converter") ) );
        m_xConverter = Reference< XTypeConverter >( xIFace, UNO_QUERY );
    }
    return m_xConverter;
}

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
// Implementation eines EventAttacher-bezogenen AllListeners, der
// nur einzelne Events an einen allgemeinen AllListener weiterleitet
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
    // Nur durchreichen, wenn es die richtige Methode ist
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

    // Nur durchreichen, wenn es die richtige Methode ist
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
        catch( CannotConvertException& e )
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

    Reference< XEventListener > xRet = NULL;

    // InvocationAdapterService holen
    Reference< XInvocationAdapterFactory > xInvocationAdapterFactory = getInvocationAdapterService();
    if( !xInvocationAdapterFactory.is() )
        throw ServiceNotRegisteredException();

    // Listener-Klasse per Reflection besorgen
    Reference< XIdlReflection > xReflection = getReflection();
    if( !xReflection.is() )
        throw ServiceNotRegisteredException();

    // Anmelden, dazu passende addListener-Methode aufrufen
    // Zunaechst ueber Introspection gehen, da die Methoden in der gleichen
    // Weise analysiert werden koennen. Fuer bessere Performance entweder
    // hier nochmal implementieren oder die Impl-Methode der Introspection
    // fuer diesen Zweck konfigurierbar machen.

    // Introspection-Service holen
    Reference< XIntrospection > xIntrospection = getIntrospection();
    if( !xIntrospection.is() )
        return xRet;

    // und unspecten
    Any aObjAny( &xObject, ::getCppuType( (const Reference< XInterface > *)0) );

    Reference< XIntrospectionAccess > xAccess = xIntrospection->inspect( aObjAny );
    if( !xAccess.is() )
        return xRet;

    // Name der addListener-Methode zusammenbasteln
    OUString aAddListenerName;
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
    aAddListenerName = OUString( RTL_CONSTASCII_USTRINGPARAM( "add" ) ) + aListenerName;

    // Methoden nach der passenden addListener-Methode durchsuchen
    Sequence< Reference< XIdlMethod > > aMethodSeq = xAccess->getMethods( MethodConcept::LISTENER );
    sal_uInt32 i, nLen = aMethodSeq.getLength();
    const Reference< XIdlMethod >* pMethods = aMethodSeq.getConstArray();

    for( i = 0 ; i < nLen ; i++ )
    {
        // Methode ansprechen
        const Reference< XIdlMethod >& rxMethod = pMethods[i];

        // Ist es die richtige Methode?
        OUString aMethName = rxMethod->getName();

        if( aAddListenerName == aMethName )
        {
            Sequence< Reference< XIdlClass > > params = rxMethod->getParameterTypes();
            sal_uInt32 nParamCount = params.getLength();

            Reference< XIdlClass > xListenerType;
            if( nParamCount == 1 )
                xListenerType = params.getConstArray()[0];
            else if( nParamCount == 2 )
                xListenerType = params.getConstArray()[1];

            // Adapter zum eigentlichen Listener-Typ anfordern
            Reference< XInterface > xAdapter = createAllListenerAdapter
                ( xInvocationAdapterFactory, xListenerType, AllListener, Helper );

            if( !xAdapter.is() )
                throw CannotCreateAdapterException();
            xRet = Reference< XEventListener >( xAdapter, UNO_QUERY );


            // Nur der Listener als Parameter?
            if( nParamCount == 1 )
            {
                Sequence< Any > args( 1 );
                args.getArray()[0] <<= xAdapter;
                try
                {
                    rxMethod->invoke( aObjAny, args );
                }
                catch( InvocationTargetException& )
                {
                    throw IntrospectionException();
                }
            }
            // Sonst den Zusatzparameter mit uebergeben
            else if( nParamCount == 2 )
            {
                Sequence< Any > args( 2 );
                Any* pAnys = args.getArray();

                // Typ des 1. Parameters pruefen
                Reference< XIdlClass > xParamClass = params.getConstArray()[0];
                if( xParamClass->getTypeClass() == TypeClass_STRING )
                {
                    pAnys[0] <<= AddListenerParam;
                }

                // 2. Parameter == Listener? TODO: Pruefen!
                pAnys[1] <<= xAdapter;

                // TODO: Konvertierung String -> ?
                // else
                try
                {
                    rxMethod->invoke( aObjAny, args );
                }
                catch( InvocationTargetException& )
                {
                    throw IntrospectionException();
                }
            }
            break;
            // else...
            // Alles andere wird nicht unterstuetzt
        }
    }

    return xRet;
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
    // FilterListener anmelden
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

    // Listener-Klasse per Reflection besorgen
    Reference< XIdlReflection > xReflection = getReflection();
    if( !xReflection.is() )
        throw IntrospectionException();

    // Abmelden, dazu passende removeListener-Methode aufrufen
    // Zunaechst ueber Introspection gehen, da die Methoden in der gleichen
    // Weise analysiert werden koennen. Fuer bessere Performance entweder
    // hier nochmal implementieren oder die Impl-Methode der Introspection
    // fuer diesen Zweck konfigurierbar machen.

    // Introspection-Service holen
    Reference< XIntrospection > xIntrospection = getIntrospection();
    if( !xIntrospection.is() )
        throw IntrospectionException();

    // und inspecten
    Any aObjAny( &xObject, ::getCppuType( (const Reference< XInterface > *)0) );
    Reference< XIntrospectionAccess > xAccess = xIntrospection->inspect( aObjAny );
    if( !xAccess.is() )
        throw IntrospectionException();

    // Name der removeListener-Methode zusammenbasteln
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
    aRemoveListenerName = OUString( RTL_CONSTASCII_USTRINGPARAM("remove") ) + aListenerName;

    // Methoden nach der passenden addListener-Methode durchsuchen
    Sequence< Reference< XIdlMethod > > aMethodSeq = xAccess->getMethods( MethodConcept::LISTENER );
    sal_uInt32 i, nLen = aMethodSeq.getLength();
    const Reference< XIdlMethod >* pMethods = aMethodSeq.getConstArray();
    for( i = 0 ; i < nLen ; i++ )
    {
        // Methode ansprechen
        const Reference< XIdlMethod >& rxMethod = pMethods[i];

        // Ist es die richtige Methode?
        if( aRemoveListenerName == rxMethod->getName() )
        {
            Sequence< Reference< XIdlClass > > params = rxMethod->getParameterTypes();
            sal_uInt32 nParamCount = params.getLength();

            // Nur der Listener als Parameter?
            if( nParamCount == 1 )
            {
                Sequence< Any > args( 1 );
                args.getArray()[0] <<= aToRemoveListener;
                try
                {
                    rxMethod->invoke( aObjAny, args );
                }
                catch( InvocationTargetException& )
                {
                    throw IntrospectionException();
                }
            }
            // Sonst den Zusatzparameter mit uebergeben
            else if( nParamCount == 2 )
            {
                Sequence< Any > args( 2 );
                Any* pAnys = args.getArray();

                // Typ des 1. Parameters pruefen
                Reference< XIdlClass > xParamClass = params.getConstArray()[0];
                if( xParamClass->getTypeClass() == TypeClass_STRING )
                    pAnys[0] <<= AddListenerParam;

                // 2. Parameter == Listener? TODO: Pruefen!
                pAnys[1] <<= aToRemoveListener;

                // TODO: Konvertierung String -> ?
                // else
                try
                {
                    rxMethod->invoke( aObjAny, args );
                }
                catch( InvocationTargetException& )
                {
                    throw IntrospectionException();
                }
            }
            break;
        }
    }
}

}

extern "C"
{
//==================================================================================================
void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}
//==================================================================================================
void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * )
{
    void * pRet = 0;

    if (pServiceManager && rtl_str_compare( pImplName, IMPLNAME ) == 0)
    {
        Reference< XSingleServiceFactory > xFactory( createOneInstanceFactory(
            reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
            OUString( RTL_CONSTASCII_USTRINGPARAM( IMPLNAME ) ),
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



