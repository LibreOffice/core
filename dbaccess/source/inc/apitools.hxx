/*************************************************************************
 *
 *  $RCSfile: apitools.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: fs $ $Date: 2000-10-11 11:20:44 $
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

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_SQLEXCEPTION_HPP_
#include <com/sun/star/sdbc/SQLException.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XUNOTUNNEL_HPP_
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XDATABASEMETADATA_HPP_
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#endif
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
//= various helper functions
//==================================================================================
//----------------------------------------------------------------------------------
template <class TYPE>
inline void disposeComponent(const ::com::sun::star::uno::Reference< TYPE >& _rxObject)
{
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent > xComp(_rxObject, UNO_QUERY);
    if (xComp.is())
        xComp->dispose();
}

//----------------------------------------------------------------------------------
/** compose a complete table name from it's up to three parts, regarding to the database meta data composing rules
*/
void composeTableName(  const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData >& _rxMetaData,
                        const ::rtl::OUString& _rCatalog,
                        const ::rtl::OUString& _rSchema,
                        const ::rtl::OUString& _rName,
                        ::rtl::OUString& _rComposedName,
                        sal_Bool _bQuote);

//----------------------------------------------------------------------------------
/** throw a generic SQLException, i.e. one with an SQLState of S1000, an ErrorCode of 0 and no NextException
*/
inline void throwGenericSQLException(const ::rtl::OUString& _rMsg, const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxSource)
    throw (::com::sun::star::sdbc::SQLException)
{
    throw ::com::sun::star::sdbc::SQLException(_rMsg, _rxSource, ::rtl::OUString::createFromAscii("S1000"), 0, ::com::sun::star::uno::Any());
}

//----------------------------------------------------------------------------------
template <class TYPE>
sal_Bool getImplementation(TYPE*& _pObject, const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxIFace)
{
    _pObject = NULL;
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XUnoTunnel > xTunnel(_rxIFace, UNO_QUERY);
    if (xTunnel.is())
        _pObject = reinterpret_cast< TYPE* >(xTunnel->getSomething(TYPE::getUnoTunnelImplementationId()));

    return (_pObject != NULL);
}

//==================================================================================
//= OSubComponent - a class which olds a Mutex and a OBroadcastHelper;
//=                 needed because when deriving from OPropertySetHelper,
//=                 the OBroadcastHelper has to be initialized before
//=                 the OPropertySetHelper
//==================================================================================
class OMutexAndBroadcastHelper
{
protected:
    ::osl::Mutex                m_aMutex;
    ::cppu::OBroadcastHelper    m_aBHelper;

public:
    OMutexAndBroadcastHelper() : m_aBHelper( m_aMutex ) { }

    ::osl::Mutex&               GetMutex() { return m_aMutex; }
    ::cppu::OBroadcastHelper&   GetBroadcastHelper() { return m_aBHelper; }

};

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
//= StandardExceptions
//==================================================================================
class FunctionSequenceException : public ::com::sun::star::sdbc::SQLException
{
public:
    FunctionSequenceException(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _Context,
        const ::com::sun::star::uno::Any& _Next = ::com::sun::star::uno::Any());
};


//************************************************************
//  OIdPropertyArrayUsageHelper
//************************************************************
namespace cppu { class IPropertyArrayHelper; }

typedef std::map< sal_Int32, ::cppu::IPropertyArrayHelper*, std::less< sal_Int32 > > OIdPropertyArrayMap;
template <class TYPE>
class OIdPropertyArrayUsageHelper
{
protected:
    static sal_Int32                        s_nRefCount;
    static OIdPropertyArrayMap*             s_pMap;
    static ::osl::Mutex                     s_aMutex;

public:
    OIdPropertyArrayUsageHelper();
    virtual ~OIdPropertyArrayUsageHelper()
    {
        ::osl::MutexGuard aGuard(s_aMutex);
        OSL_ENSHURE(s_nRefCount > 0, "OIdPropertyArrayUsageHelper::~OIdPropertyArrayUsageHelper : suspicious call : have a refcount of 0 !");
        if (!--s_nRefCount)
        {
            // delete the element
            for (OIdPropertyArrayMap::iterator i = s_pMap->begin(); i != s_pMap->end(); ++i)
                delete (*i).second;
            delete s_pMap;
            s_pMap = NULL;
        }
    }

    /** call this in the getInfoHelper method of your derived class. The method returns the array helper of the
        class, which is created if neccessary.
    */
    ::cppu::IPropertyArrayHelper* getArrayHelper(sal_Int32 nId);

protected:
    /** used to implement the creation of the array helper which is shared amongst all instances of the class.
        This method needs to be implemented in derived classes.
        <BR>
        The method gets called with s_aMutex acquired.
        <BR>
        as long as IPropertyArrayHelper has no virtual destructor, the implementation of ~OPropertyArrayUsageHelper
        assumes that you created an ::cppu::OPropertyArrayHelper when deleting s_pProps.
        @return                         an pointer to the newly created array helper. Must not be NULL.
    */
    virtual ::cppu::IPropertyArrayHelper* createArrayHelper(sal_Int32 nId) const = 0;
};

//------------------------------------------------------------------
template<class TYPE>
sal_Int32                       OIdPropertyArrayUsageHelper< TYPE >::s_nRefCount    = 0;

template<class TYPE>
OIdPropertyArrayMap*            OIdPropertyArrayUsageHelper< TYPE >::s_pMap = NULL;

template<class TYPE>
::osl::Mutex                    OIdPropertyArrayUsageHelper< TYPE >::s_aMutex;

//------------------------------------------------------------------
template <class TYPE>
OIdPropertyArrayUsageHelper<TYPE>::OIdPropertyArrayUsageHelper()
{
    ::osl::MutexGuard aGuard(s_aMutex);
    // create the map if necessary
    if (s_pMap == NULL)
        s_pMap = new OIdPropertyArrayMap();
    ++s_nRefCount;
}

//------------------------------------------------------------------
template <class TYPE>
::cppu::IPropertyArrayHelper* OIdPropertyArrayUsageHelper<TYPE>::getArrayHelper(sal_Int32 nId)
{
    OSL_ENSHURE(s_nRefCount, "OIdPropertyArrayUsageHelper::getArrayHelper : suspicious call : have a refcount of 0 !");
    ::osl::MutexGuard aGuard(s_aMutex);
    // do we have the array already?
    if (! (*s_pMap)[nId] )
    {
        (*s_pMap)[nId] = createArrayHelper(nId);
        OSL_ENSHURE((*s_pMap)[nId], "OIdPropertyArrayUsageHelper::getArrayHelper : createArrayHelper returned nonsense !");
    }
    return (*s_pMap)[nId];
}


//==================================================================================
//= helper for implementing the XServiceInfo interface

//----------------------------------------------------------------------------------
// (internal - not to be used outside - usually)
#define IMPLEMENT_SERVICE_INFO_IMPLNAME(classname, implasciiname)   \
    ::rtl::OUString SAL_CALL classname::getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException)   \
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

//----------------------------------------------------------------------------------
#define IMPLEMENT_SERVICE_INFO1(classname, implasciiname, serviceasciiname) \
    IMPLEMENT_SERVICE_INFO_IMPLNAME(classname, implasciiname)   \
    IMPLEMENT_SERVICE_INFO_SUPPORTS(classname)  \
    IMPLEMENT_SERVICE_INFO_GETSUPPORTED1(classname, serviceasciiname)   \

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
    OSL_ENSHURE(nPos == aDescriptor.getLength(), "forgot to adjust the count ?");   \

//----------------------------------------------------------------------------------
#define END_PROPERTY_HELPER()                               \
    END_PROPERTY_SEQUENCE() \
    return new ::cppu::OPropertyArrayHelper(aDescriptor);


//==================================================================================
// miscellaneous
#define UNUSED(x)   x;

#endif // _DBASHARED_APITOOLS_HXX_

