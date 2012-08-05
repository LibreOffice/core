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

#ifndef _DBASHARED_APITOOLS_HXX_
#define _DBASHARED_APITOOLS_HXX_

#include <cppuhelper/component.hxx>
#include <osl/mutex.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <osl/diagnose.h>
#include <comphelper/stl_types.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <comphelper/sequence.hxx>

//==================================================================================
//= various typedefs
//==================================================================================
DECLARE_STL_VECTOR(::com::sun::star::uno::WeakReferenceHelper, OWeakRefArray);

//==================================================================================
//= OSubComponent - a component which holds a hard ref to it's parent
//=                 and is been hold itself (by the parent) with a weak ref
//==================================================================================
class OSubComponent : public ::cppu::OComponentHelper
{
protected:
    // the parent must support the tunnel implementation
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > m_xParent;
    virtual ~OSubComponent();

public:
    OSubComponent(::osl::Mutex& _rMutex,
                  const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _xParent);

// ::com::sun::star::lang::XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes() throw (::com::sun::star::uno::RuntimeException);

// ::com::sun::star::uno::XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType )
            throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL acquire() throw();
    virtual void SAL_CALL release() throw();

    inline operator ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > () const
        { return (::com::sun::star::uno::XWeak *)this; }

};

//==================================================================================
//= helper for implementing the XServiceInfo interface

//----------------------------------------------------------------------------------
// (internal - not to be used outside - usually)
#define IMPLEMENT_SERVICE_INFO_IMPLNAME(classname, implasciiname)   \
    ::rtl::OUString SAL_CALL classname::getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException)   \
    {   \
        return ::rtl::OUString::createFromAscii(implasciiname); \
    }   \

#define IMPLEMENT_SERVICE_INFO_IMPLNAME_STATIC(classname, implasciiname)    \
    ::rtl::OUString SAL_CALL classname::getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException)   \
    {   \
        return getImplementationName_Static();  \
    }   \
    ::rtl::OUString SAL_CALL classname::getImplementationName_Static(  ) throw (::com::sun::star::uno::RuntimeException)    \
    {   \
        return ::rtl::OUString::createFromAscii(implasciiname); \
    }   \

#define IMPLEMENT_SERVICE_INFO_SUPPORTS(classname)  \
    sal_Bool SAL_CALL classname::supportsService( const ::rtl::OUString& _rServiceName ) throw(::com::sun::star::uno::RuntimeException) \
    {   \
        ::com::sun::star::uno::Sequence< ::rtl::OUString > aSupported(getSupportedServiceNames());  \
        const ::rtl::OUString* pSupported = aSupported.getConstArray(); \
        for (sal_Int32 i=0; i<aSupported.getLength(); ++i, ++pSupported)    \
            if (pSupported->equals(_rServiceName))  \
                return sal_True;    \
    \
        return sal_False;   \
    }   \

#define IMPLEMENT_SERVICE_INFO_GETSUPPORTED1(classname, serviceasciiname)   \
    ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL classname::getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException)  \
    {   \
        ::com::sun::star::uno::Sequence< ::rtl::OUString > aSupported(1);   \
        aSupported[0] = ::rtl::OUString::createFromAscii(serviceasciiname); \
        return aSupported;  \
    }   \

#define IMPLEMENT_SERVICE_INFO_GETSUPPORTED1_STATIC(classname, serviceasciiname)    \
    ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL classname::getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException)  \
    {   \
        return getSupportedServiceNames_Static();   \
    }   \
    ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL classname::getSupportedServiceNames_Static(  ) throw(::com::sun::star::uno::RuntimeException)   \
    {   \
        ::com::sun::star::uno::Sequence< ::rtl::OUString > aSupported(1);   \
        aSupported[0] = ::rtl::OUString::createFromAscii(serviceasciiname); \
        return aSupported;  \
    }   \

#define IMPLEMENT_SERVICE_INFO_GETSUPPORTED2_STATIC(classname, serviceasciiname1, serviceasciiname2)    \
    ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL classname::getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException)  \
    {   \
        return getSupportedServiceNames_Static();   \
    }   \
    ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL classname::getSupportedServiceNames_Static(  ) throw(::com::sun::star::uno::RuntimeException)   \
    {   \
        ::com::sun::star::uno::Sequence< ::rtl::OUString > aSupported(2);   \
        aSupported[0] = ::rtl::OUString::createFromAscii(serviceasciiname1);    \
        aSupported[1] = ::rtl::OUString::createFromAscii(serviceasciiname2);    \
        return aSupported;  \
    }   \

#define IMPLEMENT_SERVICE_INFO_GETSUPPORTED2(classname, serviceasciiname1, serviceasciiname2)   \
    ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL classname::getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException)  \
    {   \
        ::com::sun::star::uno::Sequence< ::rtl::OUString > aSupported(2);   \
        aSupported[0] = ::rtl::OUString::createFromAscii(serviceasciiname1);    \
        aSupported[1] = ::rtl::OUString::createFromAscii(serviceasciiname2);    \
        return aSupported;  \
    }   \

#define IMPLEMENT_SERVICE_INFO_GETSUPPORTED3(classname, serviceasciiname1, serviceasciiname2, serviceasciiname3)    \
    ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL classname::getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException)  \
    {   \
        ::com::sun::star::uno::Sequence< ::rtl::OUString > aSupported(3);   \
        aSupported[0] = ::rtl::OUString::createFromAscii(serviceasciiname1);    \
        aSupported[1] = ::rtl::OUString::createFromAscii(serviceasciiname2);    \
        aSupported[2] = ::rtl::OUString::createFromAscii(serviceasciiname3);    \
        return aSupported;  \
    }   \

#define IMPLEMENT_SERVICE_INFO_CREATE_STATIC(classname) \
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >   \
        SAL_CALL classname::Create(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB)  \
    {   \
        return static_cast< XServiceInfo* >(new classname(_rxORB)); \
    }   \

//----------------------------------------------------------------------------------
// declare service info methods
#define DECLARE_SERVICE_INFO()  \
    virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException); \
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException); \
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException) \

#define DECLARE_SERVICE_INFO_STATIC()   \
    DECLARE_SERVICE_INFO(); \
    static ::rtl::OUString SAL_CALL getImplementationName_Static(  ) throw (::com::sun::star::uno::RuntimeException);   \
    static ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames_Static(  ) throw(::com::sun::star::uno::RuntimeException);  \
    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >    \
        SAL_CALL Create(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&)    \

//----------------------------------------------------------------------------------
#define IMPLEMENT_SERVICE_INFO1(classname, implasciiname, serviceasciiname) \
    IMPLEMENT_SERVICE_INFO_IMPLNAME(classname, implasciiname)   \
    IMPLEMENT_SERVICE_INFO_SUPPORTS(classname)  \
    IMPLEMENT_SERVICE_INFO_GETSUPPORTED1(classname, serviceasciiname)   \

//----------------------------------------------------------------------------------
#define IMPLEMENT_SERVICE_INFO1_STATIC(classname, implasciiname, serviceasciiname)  \
    IMPLEMENT_SERVICE_INFO_IMPLNAME_STATIC(classname, implasciiname)    \
    IMPLEMENT_SERVICE_INFO_SUPPORTS(classname)  \
    IMPLEMENT_SERVICE_INFO_GETSUPPORTED1_STATIC(classname, serviceasciiname)    \
    IMPLEMENT_SERVICE_INFO_CREATE_STATIC(classname) \

//----------------------------------------------------------------------------------
#define IMPLEMENT_SERVICE_INFO2(classname, implasciiname, serviceasciiname1, serviceasciiname2) \
    IMPLEMENT_SERVICE_INFO_IMPLNAME(classname, implasciiname)   \
    IMPLEMENT_SERVICE_INFO_SUPPORTS(classname)  \
    IMPLEMENT_SERVICE_INFO_GETSUPPORTED2(classname, serviceasciiname1, serviceasciiname2)

//----------------------------------------------------------------------------------
#define IMPLEMENT_SERVICE_INFO2_STATIC(classname, implasciiname, serviceasciiname1, serviceasciiname2)  \
    IMPLEMENT_SERVICE_INFO_IMPLNAME_STATIC(classname, implasciiname)    \
    IMPLEMENT_SERVICE_INFO_SUPPORTS(classname)  \
    IMPLEMENT_SERVICE_INFO_GETSUPPORTED2_STATIC(classname, serviceasciiname1,serviceasciiname2) \
    IMPLEMENT_SERVICE_INFO_CREATE_STATIC(classname) \

//----------------------------------------------------------------------------------
#define IMPLEMENT_SERVICE_INFO3(classname, implasciiname, serviceasciiname1, serviceasciiname2, serviceasciiname3)  \
    IMPLEMENT_SERVICE_INFO_IMPLNAME(classname, implasciiname)   \
    IMPLEMENT_SERVICE_INFO_SUPPORTS(classname)  \
    IMPLEMENT_SERVICE_INFO_GETSUPPORTED3(classname, serviceasciiname1, serviceasciiname2, serviceasciiname3)    \

//----------------------------------------------------------------------------------
#define IMPLEMENT_SERVICE_INFO1_ABSTRACT(classname, serviceasciiname)   \
    IMPLEMENT_SERVICE_INFO_SUPPORTS(classname)  \
    IMPLEMENT_SERVICE_INFO_GETSUPPORTED1(classname, serviceasciiname)   \

//----------------------------------------------------------------------------------
#define IMPLEMENT_SERVICE_INFO2_ABSTRACT(classname, serviceasciiname1, serviceasciiname2)   \
    IMPLEMENT_SERVICE_INFO_SUPPORTS(classname)  \
    IMPLEMENT_SERVICE_INFO_GETSUPPORTED2(classname, serviceasciiname1, serviceasciiname2)   \

//==================================================================================
//= XTypeProvider helpers

//--------------------------------------------------------------------------
#define DECLARE_IMPLEMENTATION_ID( )    \
    virtual ::com::sun::star::uno::Sequence<sal_Int8> SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException);  \
    static ::com::sun::star::uno::Sequence< sal_Int8 >  getUnoTunnelImplementationId() \

//--------------------------------------------------------------------------
#define DECLARE_GETTYPES( ) \
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw (::com::sun::star::uno::RuntimeException);

//--------------------------------------------------------------------------
#define DECLARE_TYPEPROVIDER( ) \
    DECLARE_GETTYPES( ) \
    DECLARE_IMPLEMENTATION_ID( )

//--------------------------------------------------------------------------
#define IMPLEMENT_IMPLEMENTATION_ID( classname )    \
::com::sun::star::uno::Sequence< sal_Int8 > classname::getUnoTunnelImplementationId() \
{   \
    static ::cppu::OImplementationId* pId = 0;  \
    if ( !pId ) \
    {   \
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() ); \
        if ( !pId ) \
        {   \
            static ::cppu::OImplementationId aId;   \
            pId = &aId; \
        }   \
    }   \
    return pId->getImplementationId();  \
} \
::com::sun::star::uno::Sequence< sal_Int8 > classname::getImplementationId() throw (::com::sun::star::uno::RuntimeException)    \
{   \
    return classname::getUnoTunnelImplementationId(); \
}

//--------------------------------------------------------------------------
#define IMPLEMENT_GETTYPES2( classname, baseclass1, baseclass2 )    \
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > classname::getTypes() throw (::com::sun::star::uno::RuntimeException)    \
    {   \
        return  ::comphelper::concatSequences(  \
            baseclass1::getTypes( ),    \
            baseclass2::getTypes( ) \
        );  \
    }

//--------------------------------------------------------------------------
#define IMPLEMENT_GETTYPES3( classname, baseclass1, baseclass2, baseclass3 )    \
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > classname::getTypes() throw (::com::sun::star::uno::RuntimeException)    \
    {   \
        return  ::comphelper::concatSequences(  \
            baseclass1::getTypes( ),    \
            baseclass2::getTypes( ),    \
            baseclass3::getTypes( ) \
        );  \
    }

//--------------------------------------------------------------------------
#define IMPLEMENT_TYPEPROVIDER2( classname, baseclass1, baseclass2 )    \
    IMPLEMENT_IMPLEMENTATION_ID( classname) \
    IMPLEMENT_GETTYPES2( classname, baseclass1, baseclass2 )

//--------------------------------------------------------------------------
#define IMPLEMENT_TYPEPROVIDER3( classname, baseclass1, baseclass2, baseclass3 )    \
    IMPLEMENT_IMPLEMENTATION_ID( classname) \
    IMPLEMENT_GETTYPES3(classname, baseclass1, baseclass2, baseclass3 )

//==================================================================================
//= helper for declaring/implementing classes based on the OPropertyContainer and an OPropertyArrayUsageHelper
//----------------------------------------------------------------------------------
#define DECLARE_PROPERTYCONTAINER_DEFAULTS( )   \
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo>  SAL_CALL getPropertySetInfo() throw(::com::sun::star::uno::RuntimeException); \
    virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper(); \
    virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const

//----------------------------------------------------------------------------------
#define IMPLEMENT_PROPERTYCONTAINER_DEFAULTS2( classname , baseclass1)  \
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >  SAL_CALL classname::getPropertySetInfo() throw(::com::sun::star::uno::RuntimeException)  \
    {   \
        Reference< XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );    \
        return xInfo;   \
    }   \
    ::cppu::IPropertyArrayHelper& classname::getInfoHelper()    \
    {   \
    return *baseclass1::getArrayHelper();   \
    }   \
    ::cppu::IPropertyArrayHelper* classname::createArrayHelper( ) const \
    {   \
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property > aProps;    \
        describeProperties(aProps); \
        return new ::cppu::OPropertyArrayHelper(aProps);    \
    }
//----------------------------------------------------------------------------------
#define IMPLEMENT_PROPERTYCONTAINER_DEFAULTS( classname )   \
    IMPLEMENT_PROPERTYCONTAINER_DEFAULTS2( classname, classname )

//==================================================================================
//= helper for implementing the createArrayHelper
//----------------------------------------------------------------------------------
#define BEGIN_PROPERTY_SEQUENCE(count)  \
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property> aDescriptor(count); \
    ::com::sun::star::beans::Property* pDesc = aDescriptor.getArray();                  \
    sal_Int32 nPos = 0;                                             \

//----------------------------------------------------------------------------------
#define BEGIN_PROPERTY_HELPER(count)                            \
    BEGIN_PROPERTY_SEQUENCE(count)

//----------------------------------------------------------------------------------
#define DECL_PROP_IMPL(varname, cpputype) \
    pDesc[nPos++] = ::com::sun::star::beans::Property(PROPERTY_##varname, PROPERTY_ID_##varname, cpputype,

//----------------------------------------------------------------------------------
#define DECL_PROP0(varname, type)   \
    DECL_PROP_IMPL(varname, ::getCppuType((const type *)0)) 0)
//----------------------------------------------------------------------------------
#define DECL_PROP0_BOOL(varname)    \
    DECL_PROP_IMPL(varname, ::getBooleanCppuType()) 0)
//----------------------------------------------------------------------------------
#define DECL_PROP0_IFACE(varname, iface)    \
    DECL_PROP_IMPL(varname, ::getCppuType(static_cast< ::com::sun::star::uno::Reference< iface >* >(NULL))) 0)

//----------------------------------------------------------------------------------
#define DECL_PROP1(varname, type, attrib1)  \
    DECL_PROP_IMPL(varname, ::getCppuType((const type *)0)) ::com::sun::star::beans::PropertyAttribute::attrib1)
//----------------------------------------------------------------------------------
#define DECL_PROP1_BOOL(varname, attrib1)   \
    DECL_PROP_IMPL(varname, ::getBooleanCppuType()) ::com::sun::star::beans::PropertyAttribute::attrib1)
//----------------------------------------------------------------------------------
#define DECL_PROP1_IFACE(varname, iface, attrib1)   \
    DECL_PROP_IMPL(varname, ::getCppuType(static_cast< ::com::sun::star::uno::Reference< iface >* >(NULL))) ::com::sun::star::beans::PropertyAttribute::attrib1)
//----------------------------------------------------------------------------------
#define DECL_PROP2_IFACE(varname, iface, attrib1, attrib2)  \
    DECL_PROP_IMPL(varname, ::getCppuType(static_cast< ::com::sun::star::uno::Reference< iface >* >(NULL))) ::com::sun::star::beans::PropertyAttribute::attrib1 | ::com::sun::star::beans::PropertyAttribute::attrib2)

//----------------------------------------------------------------------------------
#define DECL_PROP2(varname, type, attrib1, attrib2) \
    DECL_PROP_IMPL(varname, ::getCppuType((const type *)0)) ::com::sun::star::beans::PropertyAttribute::attrib1 | ::com::sun::star::beans::PropertyAttribute::attrib2)
//----------------------------------------------------------------------------------
#define DECL_PROP2_BOOL(varname, attrib1, attrib2)  \
    DECL_PROP_IMPL(varname, ::getBooleanCppuType()) ::com::sun::star::beans::PropertyAttribute::attrib1 | ::com::sun::star::beans::PropertyAttribute::attrib2)

//----------------------------------------------------------------------------------
#define DECL_PROP3(varname, type, attrib1, attrib2, attrib3)    \
    DECL_PROP_IMPL(varname, ::getCppuType((const type *)0)) ::com::sun::star::beans::PropertyAttribute::attrib1 | ::com::sun::star::beans::PropertyAttribute::attrib2 | ::com::sun::star::beans::PropertyAttribute::attrib3)
//----------------------------------------------------------------------------------
#define DECL_PROP3_BOOL(varname, attrib1, attrib2, attrib3) \
    DECL_PROP_IMPL(varname, ::getBooleanCppuType()) ::com::sun::star::beans::PropertyAttribute::attrib1 | ::com::sun::star::beans::PropertyAttribute::attrib2 | ::com::sun::star::beans::PropertyAttribute::attrib3)


//----------------------------------------------------------------------------------
#define END_PROPERTY_SEQUENCE()                             \
    OSL_ENSURE(nPos == aDescriptor.getLength(), "forgot to adjust the count ?");    \

//----------------------------------------------------------------------------------
#define END_PROPERTY_HELPER()                               \
    END_PROPERTY_SEQUENCE() \
    return new ::cppu::OPropertyArrayHelper(aDescriptor);


#define NOTIFY_LISTERNERS(_rListeners,T,method)                                   \
    Sequence< Reference< XInterface > > aListenerSeq = _rListeners.getElements(); \
                                                                                  \
    const Reference< XInterface >* pxIntBegin = aListenerSeq.getConstArray();     \
    const Reference< XInterface >* pxInt = pxIntBegin + aListenerSeq.getLength(); \
                                                                                  \
    _rGuard.clear();                                                              \
    while( pxInt > pxIntBegin )                                                   \
    {                                                                             \
        try                                                                       \
        {                                                                         \
            while( pxInt > pxIntBegin )                                           \
            {                                                                     \
                --pxInt;                                                          \
                static_cast< T* >( pxInt->get() )->method(aEvt);                  \
            }                                                                     \
        }                                                                         \
        catch( RuntimeException& )                                                \
        {                                                                         \
        }                                                                         \
    }                                                                             \
    _rGuard.reset();

#endif // _DBASHARED_APITOOLS_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
