/*************************************************************************
 *
 *  $RCSfile: base.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:29:33 $
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

//  #define TEST_LIST_CLASSES
//  #define TRACE(x) OSL_TRACE(x)
#define TRACE(x)

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#ifndef _UNO_MAPPING_HXX_
#include <uno/mapping.hxx>
#endif
#ifndef _UNO_DISPATCHER_H_
#include <uno/dispatcher.h>
#endif

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif
#ifndef _CPPUHELPER_FACTORY_HXX_
#include <cppuhelper/factory.hxx>
#endif
#ifndef _CPPUHELPER_COMPONENT_HXX_
#include <cppuhelper/component.hxx>
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif

#include "lrucache.hxx"

#ifdef TEST_LIST_CLASSES
#include <stl/list>
#include <stl/algorithm>
#endif
#include <stl/hash_map>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>

#include <com/sun/star/reflection/XIdlClass.hpp>
#include <com/sun/star/reflection/XIdlReflection.hpp>
#include <com/sun/star/reflection/XIdlField.hpp>
#include <com/sun/star/reflection/XIdlMethod.hpp>

using namespace std;
using namespace osl;
using namespace rtl;
using namespace cppu;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::reflection;
using namespace com::sun::star::container;


namespace stoc_corefl
{

#ifdef TEST_LIST_CLASSES
typedef list< OUString > ClassNameList;
extern ClassNameList g_aClassNames;
#endif

//--------------------------------------------------------------------------------------------------
Mutex & getMutexAccess();

//--------------------------------------------------------------------------------------------------
inline td_equals( typelib_TypeDescription * pTD,
                  typelib_TypeDescriptionReference * pType )
{
    return (pTD->pWeakRef == pType ||
            (pTD->pTypeName->length == pType->pTypeName->length &&
             rtl_ustr_compare( pTD->pTypeName->buffer, pType->pTypeName->buffer ) == 0));
}
//--------------------------------------------------------------------------------------------------
inline typelib_TypeDescription * getTypeByName( const OUString & rName )
{
    typelib_TypeDescription * pTypeDescr = 0;
    typelib_typedescription_getByName( &pTypeDescr, rName.pData );
    if (! pTypeDescr->bComplete)
        typelib_typedescription_complete( &pTypeDescr );
    return pTypeDescr;
}

typedef std::hash_map< OUString, WeakReference< XIdlField >,
    FctHashOUString, equal_to< OUString > > OUString2Field;
typedef std::hash_map< OUString, WeakReference< XIdlMethod >,
    FctHashOUString, equal_to< OUString > > OUString2Method;

//==================================================================================================
class IdlReflectionServiceImpl
    : public OComponentHelper
    , public XIdlReflection
    , public XHierarchicalNameAccess
    , public XServiceInfo
{
    Mutex                                   _aComponentMutex;
    Reference< XMultiServiceFactory >       _xMgr;
    Reference< XHierarchicalNameAccess >    _xTDMgr;

    // caching
    LRU_CacheAnyByOUString                  _aElements;

    inline Reference< XIdlClass > constructClass( typelib_TypeDescription * pTypeDescr );
public:
    Reference< XHierarchicalNameAccess > getTDMgr() const
        { return _xTDMgr; }
    Reference< XMultiServiceFactory > getSMgr() const
        { return _xMgr; }

    // ctor/ dtor
    IdlReflectionServiceImpl( const Reference< XMultiServiceFactory > & xMgr );
    virtual ~IdlReflectionServiceImpl();

    // XInterface
    virtual Any SAL_CALL queryInterface( const Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL acquire() throw();
    virtual void SAL_CALL release() throw();

    // some XComponent part from OComponentHelper
    virtual void SAL_CALL dispose() throw(::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString & rServiceName ) throw(::com::sun::star::uno::RuntimeException);
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);

    // XTypeProvider
    virtual Sequence< Type > SAL_CALL getTypes() throw (::com::sun::star::uno::RuntimeException);
    virtual Sequence< sal_Int8 > SAL_CALL getImplementationId() throw (::com::sun::star::uno::RuntimeException);

    // XIdlReflection
    virtual Reference< XIdlClass > SAL_CALL forName( const OUString & rTypeName ) throw(::com::sun::star::uno::RuntimeException);
    virtual Reference< XIdlClass > SAL_CALL getType( const Any & rObj ) throw(::com::sun::star::uno::RuntimeException);

    // XHierarchicalNameAccess
    virtual Any SAL_CALL getByHierarchicalName( const OUString & rName ) throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasByHierarchicalName( const OUString & rName ) throw(::com::sun::star::uno::RuntimeException);

    Reference< XIdlClass > forType( typelib_TypeDescription * pTypeDescr ) throw(::com::sun::star::uno::RuntimeException);
    Reference< XIdlClass > forType( typelib_TypeDescriptionReference * pRef ) throw(::com::sun::star::uno::RuntimeException);
};

//==================================================================================================
class IdlClassImpl
    : public WeakImplHelper1< XIdlClass >
{
    IdlReflectionServiceImpl *  _pReflection;

    OUString                    _aName;
    TypeClass                   _eTypeClass;

    typelib_TypeDescription *   _pTypeDescr;

public:
    typelib_TypeDescription *   getTypeDescr() const
        { return _pTypeDescr; }
    IdlReflectionServiceImpl *  getReflection() const
        { return _pReflection; }
    Reference< XMultiServiceFactory > getSMgr() const
        { return _pReflection->getSMgr(); }
    Reference< XHierarchicalNameAccess > getTDMgr() const
        { return getReflection()->getTDMgr(); }

    // Ctor
    IdlClassImpl( IdlReflectionServiceImpl * pReflection,
                  const OUString & rName, typelib_TypeClass eTypeClass,
                  typelib_TypeDescription * pTypeDescr );
    virtual ~IdlClassImpl();

    // XIdlClassImpl default implementation
    virtual TypeClass SAL_CALL getTypeClass() throw(::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL getName() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL equals( const Reference< XIdlClass >& xType ) throw(::com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL isAssignableFrom( const Reference< XIdlClass > & xType ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL createObject( Any & rObj ) throw(::com::sun::star::uno::RuntimeException);

    // def impl ????
    virtual Sequence< Reference< XIdlClass > > SAL_CALL getClasses() throw(::com::sun::star::uno::RuntimeException);
    virtual Reference< XIdlClass > SAL_CALL getClass( const OUString & rName ) throw(::com::sun::star::uno::RuntimeException);
    virtual Sequence< Reference< XIdlClass > > SAL_CALL getInterfaces() throw(::com::sun::star::uno::RuntimeException);

    // structs, interfaces
    virtual Sequence< Reference< XIdlClass > > SAL_CALL getSuperclasses() throw(::com::sun::star::uno::RuntimeException);
    // structs
    virtual Reference< XIdlField > SAL_CALL getField( const OUString & rName ) throw(::com::sun::star::uno::RuntimeException);
    virtual Sequence< Reference< XIdlField > > SAL_CALL getFields() throw(::com::sun::star::uno::RuntimeException);
    // interfaces
    virtual Uik SAL_CALL getUik() throw(::com::sun::star::uno::RuntimeException);
    virtual Reference< XIdlMethod > SAL_CALL getMethod( const OUString & rName ) throw(::com::sun::star::uno::RuntimeException);
    virtual Sequence< Reference< XIdlMethod > > SAL_CALL getMethods() throw(::com::sun::star::uno::RuntimeException);
    // array
    virtual Reference< XIdlClass > SAL_CALL getComponentType() throw(::com::sun::star::uno::RuntimeException);
    virtual Reference< XIdlArray > SAL_CALL getArray() throw(::com::sun::star::uno::RuntimeException);
};

//==================================================================================================
class InterfaceIdlClassImpl
    : public IdlClassImpl
{
    typedef pair< OUString, typelib_TypeDescription * > MemberInit;

    Reference< XIdlClass >                  _xSuperClass;

    MemberInit *                            _pSortedMemberInit; // first methods, then attributes
    OUString2Field                          _aName2Field;
    OUString2Method                         _aName2Method;
    sal_Int32                               _nMethods;
    sal_Int32                               _nAttributes;

    void initMembers();

public:
    typelib_InterfaceTypeDescription * getTypeDescr() const
        { return (typelib_InterfaceTypeDescription *)IdlClassImpl::getTypeDescr(); }

    // ctor/ dtor
    InterfaceIdlClassImpl( IdlReflectionServiceImpl * pReflection,
                           const OUString & rName, typelib_TypeClass eTypeClass,
                           typelib_TypeDescription * pTypeDescr )
        : IdlClassImpl( pReflection, rName, eTypeClass, pTypeDescr )
        , _pSortedMemberInit( 0 )
        , _nMethods( 0 )
        , _nAttributes( 0 )
        {}
    virtual ~InterfaceIdlClassImpl();

    // IdlClassImpl modifications
    virtual sal_Bool SAL_CALL isAssignableFrom( const Reference< XIdlClass > & xType ) throw(::com::sun::star::uno::RuntimeException);
    virtual Sequence< Reference< XIdlClass > > SAL_CALL getSuperclasses() throw(::com::sun::star::uno::RuntimeException);
    virtual Uik SAL_CALL getUik() throw(::com::sun::star::uno::RuntimeException);
    virtual Reference< XIdlMethod > SAL_CALL getMethod( const OUString & rName ) throw(::com::sun::star::uno::RuntimeException);
    virtual Sequence< Reference< XIdlMethod > > SAL_CALL getMethods() throw(::com::sun::star::uno::RuntimeException);
    virtual Reference< XIdlField > SAL_CALL getField( const OUString & rName ) throw(::com::sun::star::uno::RuntimeException);
    virtual Sequence< Reference< XIdlField > > SAL_CALL getFields() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL createObject( Any & rObj ) throw(::com::sun::star::uno::RuntimeException);
};

//==================================================================================================
class CompoundIdlClassImpl
    : public IdlClassImpl
{
    Reference< XIdlClass >                  _xSuperClass;

    Sequence< Reference< XIdlField > > *    _pFields;
    OUString2Field                          _aName2Field;

public:
    typelib_CompoundTypeDescription * getTypeDescr() const
        { return (typelib_CompoundTypeDescription *)IdlClassImpl::getTypeDescr(); }

    // ctor/ dtor
    CompoundIdlClassImpl( IdlReflectionServiceImpl * pReflection,
                          const OUString & rName, typelib_TypeClass eTypeClass,
                          typelib_TypeDescription * pTypeDescr )
        : IdlClassImpl( pReflection, rName, eTypeClass, pTypeDescr )
        , _pFields( 0 )
        {}
    virtual ~CompoundIdlClassImpl();

    // IdlClassImpl modifications
    virtual sal_Bool SAL_CALL isAssignableFrom( const Reference< XIdlClass > & xType ) throw(::com::sun::star::uno::RuntimeException);
    virtual Sequence< Reference< XIdlClass > > SAL_CALL getSuperclasses() throw(::com::sun::star::uno::RuntimeException);
    virtual Reference< XIdlField > SAL_CALL getField( const OUString & rName ) throw(::com::sun::star::uno::RuntimeException);
    virtual Sequence< Reference< XIdlField > > SAL_CALL getFields() throw(::com::sun::star::uno::RuntimeException);
};

//==================================================================================================
class ArrayIdlClassImpl
    : public IdlClassImpl
    , public XIdlArray
{
public:
    typelib_IndirectTypeDescription * getTypeDescr() const
        { return (typelib_IndirectTypeDescription *)IdlClassImpl::getTypeDescr(); }

    // ctor
    ArrayIdlClassImpl( IdlReflectionServiceImpl * pReflection,
                       const OUString & rName, typelib_TypeClass eTypeClass,
                       typelib_TypeDescription * pTypeDescr )
        : IdlClassImpl( pReflection, rName, eTypeClass, pTypeDescr )
        {}

    virtual Any SAL_CALL queryInterface( const Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL acquire() throw();
    virtual void SAL_CALL release() throw();

    // XTypeProvider
    virtual Sequence< Type > SAL_CALL getTypes() throw (::com::sun::star::uno::RuntimeException);
    virtual Sequence< sal_Int8 > SAL_CALL getImplementationId() throw (::com::sun::star::uno::RuntimeException);

    // IdlClassImpl modifications
    virtual sal_Bool SAL_CALL isAssignableFrom( const Reference< XIdlClass > & xType ) throw(::com::sun::star::uno::RuntimeException);
    virtual Reference< XIdlClass > SAL_CALL getComponentType() throw(::com::sun::star::uno::RuntimeException);
    virtual Reference< XIdlArray > SAL_CALL getArray() throw(::com::sun::star::uno::RuntimeException);

    // XIdlArray
    virtual void SAL_CALL realloc( Any & rArray, sal_Int32 nLen ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getLen( const Any & rArray ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual Any SAL_CALL get( const Any & rArray, sal_Int32 nIndex ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::ArrayIndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL set( Any & rArray, sal_Int32 nIndex, const Any & rNewValue ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::ArrayIndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
};

//==================================================================================================
class EnumIdlClassImpl
    : public IdlClassImpl
{
    Sequence< Reference< XIdlField > > * _pFields;
    OUString2Field                       _aName2Field;

public:
    typelib_EnumTypeDescription * getTypeDescr() const
        { return (typelib_EnumTypeDescription *)IdlClassImpl::getTypeDescr(); }

    // ctor/ dtor
    EnumIdlClassImpl( IdlReflectionServiceImpl * pReflection,
                      const OUString & rName, typelib_TypeClass eTypeClass,
                      typelib_TypeDescription * pTypeDescr )
        : IdlClassImpl( pReflection, rName, eTypeClass, pTypeDescr )
        , _pFields( 0 )
        {}
    virtual ~EnumIdlClassImpl();

    // IdlClassImpl modifications
    virtual Reference< XIdlField > SAL_CALL getField( const OUString & rName ) throw(::com::sun::star::uno::RuntimeException);
    virtual Sequence< Reference< XIdlField > > SAL_CALL getFields() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL createObject( Any & rObj ) throw(::com::sun::star::uno::RuntimeException);
};

//==================================================================================================
class IdlMemberImpl
    : public WeakImplHelper1< XIdlMember >
{
    Mapping                     _aCpp2Uno;
    Mapping                     _aUno2Cpp;

    IdlReflectionServiceImpl *  _pReflection;
    OUString                    _aName;

    typelib_TypeDescription *   _pTypeDescr;
    typelib_TypeDescription *   _pDeclTypeDescr;

protected:
    Reference< XIdlClass >      _xDeclClass;

public:
    IdlReflectionServiceImpl *  getReflection() const
        { return _pReflection; }
    Reference< XMultiServiceFactory > getSMgr() const
        { return _pReflection->getSMgr(); }
    typelib_TypeDescription *   getTypeDescr() const
        { return _pTypeDescr; }
    typelib_TypeDescription *   getDeclTypeDescr() const
        { return _pDeclTypeDescr; }

    inline const Mapping & getCpp2Uno() throw(::com::sun::star::uno::RuntimeException);
    inline const Mapping & getUno2Cpp() throw(::com::sun::star::uno::RuntimeException);
    inline uno_Interface * mapToUno( const Any & rObj, typelib_InterfaceTypeDescription * pTo ) throw(::com::sun::star::uno::RuntimeException);

    // ctor/ dtor
    IdlMemberImpl( IdlReflectionServiceImpl * pReflection, const OUString & rName,
                   typelib_TypeDescription * pTypeDescr, typelib_TypeDescription * pDeclTypeDescr );
    virtual ~IdlMemberImpl();

    // XIdlMember
    virtual Reference< XIdlClass > SAL_CALL getDeclaringClass() throw(::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL getName() throw(::com::sun::star::uno::RuntimeException);
};
//__________________________________________________________________________________________________
inline const Mapping & IdlMemberImpl::getCpp2Uno()
    throw(::com::sun::star::uno::RuntimeException)
{
    if (! _aCpp2Uno.is())
    {
        MutexGuard aGuard( getMutexAccess() );
        if (! _aCpp2Uno.is())
        {
            _aCpp2Uno = Mapping( OUString( RTL_CONSTASCII_USTRINGPARAM(CPPU_CURRENT_LANGUAGE_BINDING_NAME) ),
                                 OUString( RTL_CONSTASCII_USTRINGPARAM(UNO_LB_UNO) ) );
            OSL_ENSHURE( _aCpp2Uno.is(), "### cannot c++ to uno mapping!" );
            if (! _aCpp2Uno.is())
            {
                throw RuntimeException(
                    OUString( RTL_CONSTASCII_USTRINGPARAM("cannot c++ to uno mapping!") ),
                    (XWeak *)(OWeakObject *)this );
            }
        }
    }
    return _aCpp2Uno;
}
//__________________________________________________________________________________________________
inline const Mapping & IdlMemberImpl::getUno2Cpp()
    throw(::com::sun::star::uno::RuntimeException)
{
    if (! _aUno2Cpp.is())
    {
        MutexGuard aGuard( getMutexAccess() );
        if (! _aUno2Cpp.is())
        {
            _aUno2Cpp = Mapping( OUString( RTL_CONSTASCII_USTRINGPARAM(UNO_LB_UNO) ),
                                 OUString( RTL_CONSTASCII_USTRINGPARAM(CPPU_CURRENT_LANGUAGE_BINDING_NAME) ) );
            OSL_ENSHURE( _aUno2Cpp.is(), "### cannot uno to c++ mapping!" );
            if (! _aUno2Cpp.is())
            {
                throw RuntimeException(
                    OUString( RTL_CONSTASCII_USTRINGPARAM("cannot uno TO c++ mapping!") ),
                    (XWeak *)(OWeakObject *)this );
            }
        }
    }
    return _aUno2Cpp;
}

//--------------------------------------------------------------------------------------------------
// coerces to type descr pTo else queries for it: the interface pointer is returned via rDest
// ## type to XidlClass coercion possible
inline sal_Bool extract(
    const Any & rObj, typelib_InterfaceTypeDescription * pTo,
    Reference< XInterface > & rDest,
    IdlReflectionServiceImpl * pRefl )
{
    rDest.clear();
    if (rObj.hasValue() && pTo)
    {
        if (rObj.getValueTypeClass() == TypeClass_INTERFACE)
        {
            return ::uno_type_assignData(
                &rDest, ((typelib_TypeDescription *)pTo)->pWeakRef,
                const_cast< void * >( rObj.getValue() ), rObj.getValueTypeRef(),
                cpp_queryInterface, cpp_acquire, cpp_release );
        }
        else if (rObj.getValueTypeClass() == TypeClass_TYPE)
        {
            rDest = pRefl->forType( reinterpret_cast< const Type * >( rObj.getValue() )->getTypeLibType() );
            return rDest.is();
        }
    }
    return sal_False;
}
//--------------------------------------------------------------------------------------------------
inline sal_Bool coerce_assign(
    void * pDest, typelib_TypeDescription * pTD, const Any & rSource,
    IdlReflectionServiceImpl * pRefl )
{
    if (rSource.hasValue())
    {
        if (pTD->eTypeClass == typelib_TypeClass_INTERFACE)
        {
            Reference< XInterface > xVal;
            if (extract( rSource, (typelib_InterfaceTypeDescription *)pTD, xVal, pRefl ))
            {
                if (*(XInterface **)pDest)
                    (*(XInterface **)pDest)->release();
                if (*(XInterface **)pDest = xVal.get())
                    (*(XInterface **)pDest)->acquire();
                return sal_True;
            }
        }
        else if (pTD->eTypeClass == typelib_TypeClass_ANY)
        {
            return uno_assignData(
                pDest, pTD,
                (void *)&rSource, pTD,
                cpp_queryInterface, cpp_acquire, cpp_release );
        }
        else
        {
            return uno_type_assignData(
                pDest, pTD->pWeakRef,
                (void *)rSource.getValue(), rSource.getValueTypeRef(),
                cpp_queryInterface, cpp_acquire, cpp_release );
        }
    }
    return sal_False;
}

//__________________________________________________________________________________________________
inline uno_Interface * IdlMemberImpl::mapToUno( const Any & rObj,
                                                typelib_InterfaceTypeDescription * pTo )
    throw(::com::sun::star::uno::RuntimeException)
{
    Reference< XInterface > xObj;
    if (extract( rObj, pTo, xObj, getReflection() ))
        return (uno_Interface *)getCpp2Uno().mapInterface( xObj.get(), pTo );

    throw RuntimeException(
        OUString( RTL_CONSTASCII_USTRINGPARAM("illegal object given!") ),
        (XWeak *)(OWeakObject *)this );
    return 0; // dummy
}

}


