/*************************************************************************
 *
 *  $RCSfile: eventattacher.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:15:25 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_REGISTRY_XREGISTRYKEY_HPP_
#include <com/sun/star/registry/XRegistryKey.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XINTROSPECTION_HPP_
#include <com/sun/star/beans/XIntrospection.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_METHODCONCEPT_HPP_
#include <com/sun/star/beans/MethodConcept.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_XEVENTATTACHER_HPP_
#include <com/sun/star/script/XEventAttacher.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_XTYPECONVERTER_HPP_
#include <com/sun/star/script/XTypeConverter.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_XALLLISTENER_HPP_
#include <com/sun/star/script/XAllListener.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_XINVOCATIONADAPTERFACTORY_HPP_
#include <com/sun/star/script/XInvocationAdapterFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_REFLECTION_XIDLREFLECTION_HPP_
#include <com/sun/star/reflection/XIdlReflection.hpp>
#endif

// InvocationToAllListenerMapper
#ifndef _COM_SUN_STAR_SCRIPT_XINVOCATION_HPP_
#include <com/sun/star/script/XInvocation.hpp>
#endif

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif
#ifndef _CPPUHELPER_FACTORY_HXX_
#include <cppuhelper/factory.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE3_HXX_
#include <cppuhelper/implbase3.hxx>
#endif

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
        , m_Helper( Helper )
        , m_xListenerType( ListenerType )
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
    Sequence< sal_Int16 >& OutParamIndex, Sequence< Any >& OutParam)
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
void SAL_CALL InvocationToAllListenerMapper::setValue(const OUString& PropertyName, const Any& Value)
    throw( UnknownPropertyException, CannotConvertException,
           InvocationTargetException, RuntimeException )
{
}

//*************************************************************************
Any SAL_CALL InvocationToAllListenerMapper::getValue(const OUString& PropertyName)
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
        throw( IllegalArgumentException, ServiceNotRegisteredException, IntrospectionException, RuntimeException );

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
void FilterAllListenerImpl::disposing(const EventObject& Source)
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
    throw( IllegalArgumentException, ServiceNotRegisteredException, IntrospectionException, RuntimeException )
{
    if( !xObject.is() || !aToRemoveListener.is() )
        throw IllegalArgumentException();

    // Listener-Klasse per Reflection besorgen
    Reference< XIdlReflection > xReflection = getReflection();
    if( !xReflection.is() )
        throw ServiceNotRegisteredException();

    // Abmelden, dazu passende removeListener-Methode aufrufen
    // Zunaechst ueber Introspection gehen, da die Methoden in der gleichen
    // Weise analysiert werden koennen. Fuer bessere Performance entweder
    // hier nochmal implementieren oder die Impl-Methode der Introspection
    // fuer diesen Zweck konfigurierbar machen.

    // Introspection-Service holen
    Reference< XIntrospection > xIntrospection = getIntrospection();
    if( !xIntrospection.is() )
        throw ServiceNotRegisteredException();

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
    const sal_Char ** ppEnvTypeName, uno_Environment ** ppEnv )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}
//==================================================================================================
sal_Bool SAL_CALL component_writeInfo(
    void * pServiceManager, void * pRegistryKey )
{
    if (pRegistryKey)
    {
        try
        {
            // DefaultRegistry
            Reference< XRegistryKey > xNewKey(
                reinterpret_cast< XRegistryKey * >( pRegistryKey )->createKey(
                    OUString( RTL_CONSTASCII_USTRINGPARAM( "/" IMPLNAME "/UNO/SERVICES") )));

            Sequence< OUString > & rSNL =
                ::comp_EventAttacher::EventAttacherImpl::getSupportedServiceNames_Static();
            const OUString * pArray = rSNL.getConstArray();
            for ( sal_Int32 nPos = rSNL.getLength(); nPos--; )
                xNewKey->createKey( pArray[nPos] );

            return sal_True;
        }
        catch (InvalidRegistryException &)
        {
            OSL_ENSHURE( sal_False, "### InvalidRegistryException!" );
        }
    }
    return sal_False;
}
//==================================================================================================
void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    void * pRet = 0;

    if (pServiceManager && rtl_str_compare( pImplName, IMPLNAME ) == 0)
    {
        Reference< XSingleServiceFactory > xFactory( createOneInstanceFactory(
            reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
            OUString( RTL_CONSTASCII_USTRINGPARAM( pImplName ) ),
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



