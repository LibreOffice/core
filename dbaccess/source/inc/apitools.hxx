/*************************************************************************
 *
 *  $RCSfile: apitools.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: jl $ $Date: 2001-03-23 13:22:00 $
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

#ifndef _DBASHARED_APITOOLS_HXX_
#define _DBASHARED_APITOOLS_HXX_

#ifndef _CPPUHELPER_COMPONENT_HXX_
#include <cppuhelper/component.hxx>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _CPPUHELPER_INTERFACECONTAINER_HXX_
#include <cppuhelper/interfacecontainer.hxx>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif

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

public:
    OSubComponent(::osl::Mutex& _rMutex,
                  const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _xParent);

// ::com::sun::star::lang::XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes() throw (::com::sun::star::uno::RuntimeException);

// ::com::sun::star::uno::XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType )
            throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL acquire() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL release() throw(::com::sun::star::uno::RuntimeException);

    inline operator ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > () const
        { return (::com::sun::star::uno::XWeak *)this; }

protected:
    virtual void SAL_CALL disposing();
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
// declare service info methods - no getImplementationName, so the class is abstract
#define DECLARE_SERVICE_INFO_ABSTRACT() \
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException); \
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException) \

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
    IMPLEMENT_SERVICE_INFO_GETSUPPORTED2(classname, serviceasciiname1, serviceasciiname2)   \

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
    DECL_PROP_IMPL(varname, ::getCppuType((const type *)0)) ::com::sun::star::beans::PropertyAttribute::##attrib1)
//----------------------------------------------------------------------------------
#define DECL_PROP1_BOOL(varname, attrib1)   \
    DECL_PROP_IMPL(varname, ::getBooleanCppuType()) ::com::sun::star::beans::PropertyAttribute::##attrib1)
//----------------------------------------------------------------------------------
#define DECL_PROP1_IFACE(varname, iface, attrib1)   \
    DECL_PROP_IMPL(varname, ::getCppuType(static_cast< ::com::sun::star::uno::Reference< iface >* >(NULL))) ::com::sun::star::beans::PropertyAttribute::##attrib1)

//----------------------------------------------------------------------------------
#define DECL_PROP2(varname, type, attrib1, attrib2) \
    DECL_PROP_IMPL(varname, ::getCppuType((const type *)0)) ::com::sun::star::beans::PropertyAttribute::##attrib1 | ::com::sun::star::beans::PropertyAttribute::##attrib2)
//----------------------------------------------------------------------------------
#define DECL_PROP2_BOOL(varname, attrib1, attrib2)  \
    DECL_PROP_IMPL(varname, ::getBooleanCppuType()) ::com::sun::star::beans::PropertyAttribute::##attrib1 | ::com::sun::star::beans::PropertyAttribute::##attrib2)

//----------------------------------------------------------------------------------
#define DECL_PROP3(varname, type, attrib1, attrib2, attrib3)    \
    DECL_PROP_IMPL(varname, ::getCppuType((const type *)0)) ::com::sun::star::beans::PropertyAttribute::##attrib1 | ::com::sun::star::beans::PropertyAttribute::##attrib2 | ::com::sun::star::beans::PropertyAttribute::##attrib3)
//----------------------------------------------------------------------------------
#define DECL_PROP3_BOOL(varname, attrib1, attrib2, attrib3) \
    DECL_PROP_IMPL(varname, ::getBooleanCppuType()) ::com::sun::star::beans::PropertyAttribute::##attrib1 | ::com::sun::star::beans::PropertyAttribute::##attrib2 | ::com::sun::star::beans::PropertyAttribute::##attrib3)


//----------------------------------------------------------------------------------
#define END_PROPERTY_SEQUENCE()                             \
    OSL_ENSURE(nPos == aDescriptor.getLength(), "forgot to adjust the count ?");    \

//----------------------------------------------------------------------------------
#define END_PROPERTY_HELPER()                               \
    END_PROPERTY_SEQUENCE() \
    return new ::cppu::OPropertyArrayHelper(aDescriptor);


//==================================================================================
// miscellaneous
#define UNUSED(x)   x;

#endif // _DBASHARED_APITOOLS_HXX_

