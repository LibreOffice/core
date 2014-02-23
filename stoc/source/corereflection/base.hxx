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
//  #define TEST_LIST_CLASSES

#include <osl/diagnose.h>
#include <osl/mutex.hxx>
#include <uno/mapping.hxx>
#include <uno/dispatcher.h>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/component.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <rtl/ustring.hxx>

#include "lrucache.hxx"

#ifdef TEST_LIST_CLASSES
#include <list>
#include <algorithm>
#endif
#include <boost/unordered_map.hpp>

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>

#include <com/sun/star/reflection/XIdlClass.hpp>
#include <com/sun/star/reflection/XIdlReflection.hpp>
#include <com/sun/star/reflection/XIdlField.hpp>
#include <com/sun/star/reflection/XIdlField2.hpp>
#include <com/sun/star/reflection/XIdlMethod.hpp>

using namespace std;
using namespace osl;
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


Mutex & getMutexAccess();


inline bool td_equals( typelib_TypeDescription * pTD, typelib_TypeDescriptionReference * pType )
{
    return (pTD->pWeakRef == pType ||
            (pTD->pTypeName->length == pType->pTypeName->length &&
             rtl_ustr_compare( pTD->pTypeName->buffer, pType->pTypeName->buffer ) == 0));
}

inline typelib_TypeDescription * getTypeByName( const OUString & rName )
{
    typelib_TypeDescription * pTypeDescr = 0;
    typelib_typedescription_getByName( &pTypeDescr, rName.pData );
    if (! pTypeDescr->bComplete)
        typelib_typedescription_complete( &pTypeDescr );
    return pTypeDescr;
}

typedef boost::unordered_map< OUString, WeakReference< XIdlField >,
    FctHashOUString, equal_to< OUString > > OUString2Field;
typedef boost::unordered_map< OUString, WeakReference< XIdlMethod >,
    FctHashOUString, equal_to< OUString > > OUString2Method;


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

    Mapping                     _aCpp2Uno;
    Mapping                     _aUno2Cpp;

    inline Reference< XIdlClass > constructClass( typelib_TypeDescription * pTypeDescr );
public:
    Reference< XHierarchicalNameAccess > getTDMgr() const
        { return _xTDMgr; }
    Reference< XMultiServiceFactory > getSMgr() const
        { return _xMgr; }

    const Mapping & getCpp2Uno() throw(::com::sun::star::uno::RuntimeException);
    const Mapping & getUno2Cpp() throw(::com::sun::star::uno::RuntimeException);
    uno_Interface * mapToUno( const Any & rObj, typelib_InterfaceTypeDescription * pTo ) throw(::com::sun::star::uno::RuntimeException);

    // ctor/ dtor
    IdlReflectionServiceImpl( const Reference< XComponentContext > & xContext );
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


class InterfaceIdlClassImpl
    : public IdlClassImpl
{
    typedef pair< OUString, typelib_TypeDescription * > MemberInit;

    Sequence< Reference< XIdlClass > >      _xSuperClasses;

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


class IdlMemberImpl
    : public WeakImplHelper1< XIdlMember >
{
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

    // ctor/ dtor
    IdlMemberImpl( IdlReflectionServiceImpl * pReflection, const OUString & rName,
                   typelib_TypeDescription * pTypeDescr, typelib_TypeDescription * pDeclTypeDescr );
    virtual ~IdlMemberImpl();

    // XIdlMember
    virtual Reference< XIdlClass > SAL_CALL getDeclaringClass() throw(::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL getName() throw(::com::sun::star::uno::RuntimeException);
};


// coerces to type descr pTo else queries for it: the interface pointer is returned via rDest
// ## type to XidlClass coercion possible
inline sal_Bool extract(
    const Any & rObj, typelib_InterfaceTypeDescription * pTo,
    Reference< XInterface > & rDest,
    IdlReflectionServiceImpl * pRefl )
{
    rDest.clear();
    if (0 != pTo)
    {
        if (! rObj.hasValue())
            return sal_True;
        if (rObj.getValueTypeClass() == TypeClass_INTERFACE)
        {
            return ::uno_type_assignData(
                &rDest, ((typelib_TypeDescription *)pTo)->pWeakRef,
                const_cast< void * >( rObj.getValue() ), rObj.getValueTypeRef(),
                reinterpret_cast< uno_QueryInterfaceFunc >(cpp_queryInterface),
                reinterpret_cast< uno_AcquireFunc >(cpp_acquire),
                reinterpret_cast< uno_ReleaseFunc >(cpp_release) );
        }
        else if (rObj.getValueTypeClass() == TypeClass_TYPE)
        {
            rDest = pRefl->forType( reinterpret_cast< const Type * >( rObj.getValue() )->getTypeLibType() );
            return rDest.is();
        }
    }
    return sal_False;
}

inline sal_Bool coerce_assign(
    void * pDest, typelib_TypeDescription * pTD, const Any & rSource,
    IdlReflectionServiceImpl * pRefl )
{
    if (pTD->eTypeClass == typelib_TypeClass_INTERFACE)
    {
        Reference< XInterface > xVal;
        if (extract( rSource, (typelib_InterfaceTypeDescription *)pTD, xVal, pRefl ))
        {
            if (*(XInterface **)pDest)
                (*(XInterface **)pDest)->release();
            *(XInterface **)pDest = xVal.get();
            if (*(XInterface **)pDest)
                (*(XInterface **)pDest)->acquire();
            return sal_True;
        }
        return sal_False;
    }
    else if (pTD->eTypeClass == typelib_TypeClass_ANY)
    {
        return uno_assignData(
            pDest, pTD,
            (void *)&rSource, pTD,
            reinterpret_cast< uno_QueryInterfaceFunc >(cpp_queryInterface),
            reinterpret_cast< uno_AcquireFunc >(cpp_acquire),
            reinterpret_cast< uno_ReleaseFunc >(cpp_release) );
    }
    else
    {
        return uno_type_assignData(
            pDest, pTD->pWeakRef,
            (void *)rSource.getValue(), rSource.getValueTypeRef(),
            reinterpret_cast< uno_QueryInterfaceFunc >(cpp_queryInterface),
            reinterpret_cast< uno_AcquireFunc >(cpp_acquire),
            reinterpret_cast< uno_ReleaseFunc >(cpp_release) );
    }
}

}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
