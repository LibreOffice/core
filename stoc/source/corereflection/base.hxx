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

#ifndef INCLUDED_STOC_SOURCE_COREREFLECTION_BASE_HXX
#define INCLUDED_STOC_SOURCE_COREREFLECTION_BASE_HXX

#include <osl/diagnose.h>
#include <osl/mutex.hxx>
#include <uno/mapping.hxx>
#include <uno/dispatcher.h>
#include <cppuhelper/implbase.hxx>
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
#include <unordered_map>

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>

#include <com/sun/star/reflection/XIdlClass.hpp>
#include <com/sun/star/reflection/XIdlReflection.hpp>
#include <com/sun/star/reflection/XIdlField.hpp>
#include <com/sun/star/reflection/XIdlField2.hpp>
#include <com/sun/star/reflection/XIdlMethod.hpp>

namespace stoc_corefl
{

#ifdef TEST_LIST_CLASSES
typedef std::list< OUString > ClassNameList;
extern ClassNameList g_aClassNames;
#endif


::osl::Mutex & getMutexAccess();


inline bool td_equals( typelib_TypeDescription * pTD, typelib_TypeDescriptionReference * pType )
{
    return (pTD->pWeakRef == pType ||
            (pTD->pTypeName->length == pType->pTypeName->length &&
             rtl_ustr_compare( pTD->pTypeName->buffer, pType->pTypeName->buffer ) == 0));
}

typedef std::unordered_map< OUString, css::uno::WeakReference< css::reflection::XIdlField >,
    OUStringHash > OUString2Field;
typedef std::unordered_map< OUString, css::uno::WeakReference< css::reflection::XIdlMethod >,
    OUStringHash > OUString2Method;


class IdlReflectionServiceImpl
    : public ::cppu::OComponentHelper
    , public css::reflection::XIdlReflection
    , public css::container::XHierarchicalNameAccess
    , public css::lang::XServiceInfo
{
    ::osl::Mutex                            _aComponentMutex;
    css::uno::Reference< css::lang::XMultiServiceFactory >       _xMgr;
    css::uno::Reference< css::container::XHierarchicalNameAccess >    _xTDMgr;

    // caching
    LRU_CacheAnyByOUString                  _aElements;

    css::uno::Mapping                     _aCpp2Uno;
    css::uno::Mapping                     _aUno2Cpp;

    inline css::uno::Reference< css::reflection::XIdlClass > constructClass( typelib_TypeDescription * pTypeDescr );

public:
    const css::uno::Mapping & getCpp2Uno() throw(css::uno::RuntimeException);
    const css::uno::Mapping & getUno2Cpp() throw(css::uno::RuntimeException);
    uno_Interface * mapToUno( const css::uno::Any & rObj, typelib_InterfaceTypeDescription * pTo ) throw(css::uno::RuntimeException);

    // ctor/ dtor
    explicit IdlReflectionServiceImpl( const css::uno::Reference< css::uno::XComponentContext > & xContext );
    virtual ~IdlReflectionServiceImpl();

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL acquire() throw() SAL_OVERRIDE;
    virtual void SAL_CALL release() throw() SAL_OVERRIDE;

    // some XComponent part from OComponentHelper
    virtual void SAL_CALL dispose() throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL supportsService( const OUString & rServiceName ) throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XIdlReflection
    virtual css::uno::Reference< css::reflection::XIdlClass > SAL_CALL forName( const OUString & rTypeName ) throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual css::uno::Reference< css::reflection::XIdlClass > SAL_CALL getType( const css::uno::Any & rObj ) throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XHierarchicalNameAccess
    virtual css::uno::Any SAL_CALL getByHierarchicalName( const OUString & rName ) throw(css::container::NoSuchElementException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL hasByHierarchicalName( const OUString & rName ) throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    css::uno::Reference< css::reflection::XIdlClass > forType( typelib_TypeDescription * pTypeDescr ) throw(css::uno::RuntimeException);
    css::uno::Reference< css::reflection::XIdlClass > forType( typelib_TypeDescriptionReference * pRef ) throw(css::uno::RuntimeException);
};


class IdlClassImpl
    : public ::cppu::WeakImplHelper< css::reflection::XIdlClass >
{
    IdlReflectionServiceImpl *  _pReflection;

    OUString                    _aName;
    css::uno::TypeClass         _eTypeClass;

    typelib_TypeDescription *   _pTypeDescr;

public:
    typelib_TypeDescription *   getTypeDescr() const
        { return _pTypeDescr; }
    IdlReflectionServiceImpl *  getReflection() const
        { return _pReflection; }

    // Ctor
    IdlClassImpl( IdlReflectionServiceImpl * pReflection,
                  const OUString & rName, typelib_TypeClass eTypeClass,
                  typelib_TypeDescription * pTypeDescr );
    virtual ~IdlClassImpl();

    // XIdlClassImpl default implementation
    virtual css::uno::TypeClass SAL_CALL getTypeClass() throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getName() throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL equals( const css::uno::Reference< css::reflection::XIdlClass >& xType ) throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual sal_Bool SAL_CALL isAssignableFrom( const css::uno::Reference< css::reflection::XIdlClass > & xType ) throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL createObject( css::uno::Any & rObj ) throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // def impl ????
    virtual css::uno::Sequence< css::uno::Reference< css::reflection::XIdlClass > > SAL_CALL getClasses() throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual css::uno::Reference< css::reflection::XIdlClass > SAL_CALL getClass( const OUString & rName ) throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual css::uno::Sequence< css::uno::Reference< css::reflection::XIdlClass > > SAL_CALL getInterfaces() throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // structs, interfaces
    virtual css::uno::Sequence< css::uno::Reference< css::reflection::XIdlClass > > SAL_CALL getSuperclasses() throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    // structs
    virtual css::uno::Reference< css::reflection::XIdlField > SAL_CALL getField( const OUString & rName ) throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual css::uno::Sequence< css::uno::Reference< css::reflection::XIdlField > > SAL_CALL getFields() throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    // interfaces
    virtual css::uno::Uik SAL_CALL getUik() throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual css::uno::Reference< css::reflection::XIdlMethod > SAL_CALL getMethod( const OUString & rName ) throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual css::uno::Sequence< css::uno::Reference< css::reflection::XIdlMethod > > SAL_CALL getMethods() throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    // array
    virtual css::uno::Reference< css::reflection::XIdlClass > SAL_CALL getComponentType() throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual css::uno::Reference< css::reflection::XIdlArray > SAL_CALL getArray() throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};


class InterfaceIdlClassImpl
    : public IdlClassImpl
{
    typedef std::pair< OUString, typelib_TypeDescription * > MemberInit;

    css::uno::Sequence< css::uno::Reference< css::reflection::XIdlClass > >      _xSuperClasses;

    MemberInit *                            _pSortedMemberInit; // first methods, then attributes
    OUString2Field                          _aName2Field;
    OUString2Method                         _aName2Method;
    sal_Int32                               _nMethods;
    sal_Int32                               _nAttributes;

    void initMembers();

public:
    typelib_InterfaceTypeDescription * getTypeDescr() const
        { return reinterpret_cast<typelib_InterfaceTypeDescription *>(IdlClassImpl::getTypeDescr()); }

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
    virtual sal_Bool SAL_CALL isAssignableFrom( const css::uno::Reference< css::reflection::XIdlClass > & xType ) throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual css::uno::Sequence< css::uno::Reference< css::reflection::XIdlClass > > SAL_CALL getSuperclasses() throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual css::uno::Uik SAL_CALL getUik() throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual css::uno::Reference< css::reflection::XIdlMethod > SAL_CALL getMethod( const OUString & rName ) throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual css::uno::Sequence< css::uno::Reference< css::reflection::XIdlMethod > > SAL_CALL getMethods() throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual css::uno::Reference< css::reflection::XIdlField > SAL_CALL getField( const OUString & rName ) throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual css::uno::Sequence< css::uno::Reference< css::reflection::XIdlField > > SAL_CALL getFields() throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL createObject( css::uno::Any & rObj ) throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};


class CompoundIdlClassImpl
    : public IdlClassImpl
{
    css::uno::Reference< css::reflection::XIdlClass >                  _xSuperClass;

    css::uno::Sequence< css::uno::Reference< css::reflection::XIdlField > > *    _pFields;
    OUString2Field                          _aName2Field;

public:
    typelib_CompoundTypeDescription * getTypeDescr() const
        { return reinterpret_cast<typelib_CompoundTypeDescription *>(IdlClassImpl::getTypeDescr()); }

    // ctor/ dtor
    CompoundIdlClassImpl( IdlReflectionServiceImpl * pReflection,
                          const OUString & rName, typelib_TypeClass eTypeClass,
                          typelib_TypeDescription * pTypeDescr )
        : IdlClassImpl( pReflection, rName, eTypeClass, pTypeDescr )
        , _pFields( 0 )
        {}
    virtual ~CompoundIdlClassImpl();

    // IdlClassImpl modifications
    virtual sal_Bool SAL_CALL isAssignableFrom( const css::uno::Reference< css::reflection::XIdlClass > & xType ) throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual css::uno::Sequence< css::uno::Reference< css::reflection::XIdlClass > > SAL_CALL getSuperclasses() throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual css::uno::Reference< css::reflection::XIdlField > SAL_CALL getField( const OUString & rName ) throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual css::uno::Sequence< css::uno::Reference< css::reflection::XIdlField > > SAL_CALL getFields() throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};


class ArrayIdlClassImpl
    : public IdlClassImpl
    , public css::reflection::XIdlArray
{
public:
    typelib_IndirectTypeDescription * getTypeDescr() const
        { return reinterpret_cast<typelib_IndirectTypeDescription *>(IdlClassImpl::getTypeDescr()); }

    // ctor
    ArrayIdlClassImpl( IdlReflectionServiceImpl * pReflection,
                       const OUString & rName, typelib_TypeClass eTypeClass,
                       typelib_TypeDescription * pTypeDescr )
        : IdlClassImpl( pReflection, rName, eTypeClass, pTypeDescr )
        {}

    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL acquire() throw() SAL_OVERRIDE;
    virtual void SAL_CALL release() throw() SAL_OVERRIDE;

    // XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // IdlClassImpl modifications
    virtual sal_Bool SAL_CALL isAssignableFrom( const css::uno::Reference< css::reflection::XIdlClass > & xType ) throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual css::uno::Reference< css::reflection::XIdlClass > SAL_CALL getComponentType() throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual css::uno::Reference< css::reflection::XIdlArray > SAL_CALL getArray() throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XIdlArray
    virtual void SAL_CALL realloc( css::uno::Any & rArray, sal_Int32 nLen ) throw(css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Int32 SAL_CALL getLen( const css::uno::Any & rArray ) throw(css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual css::uno::Any SAL_CALL get( const css::uno::Any & rArray, sal_Int32 nIndex ) throw(css::lang::IllegalArgumentException, css::lang::ArrayIndexOutOfBoundsException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL set( css::uno::Any & rArray, sal_Int32 nIndex, const css::uno::Any & rNewValue ) throw(css::lang::IllegalArgumentException, css::lang::ArrayIndexOutOfBoundsException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};


class EnumIdlClassImpl
    : public IdlClassImpl
{
    css::uno::Sequence< css::uno::Reference< css::reflection::XIdlField > > * _pFields;
    OUString2Field                       _aName2Field;

public:
    typelib_EnumTypeDescription * getTypeDescr() const
        { return reinterpret_cast<typelib_EnumTypeDescription *>(IdlClassImpl::getTypeDescr()); }

    // ctor/ dtor
    EnumIdlClassImpl( IdlReflectionServiceImpl * pReflection,
                      const OUString & rName, typelib_TypeClass eTypeClass,
                      typelib_TypeDescription * pTypeDescr )
        : IdlClassImpl( pReflection, rName, eTypeClass, pTypeDescr )
        , _pFields( 0 )
        {}
    virtual ~EnumIdlClassImpl();

    // IdlClassImpl modifications
    virtual css::uno::Reference< css::reflection::XIdlField > SAL_CALL getField( const OUString & rName ) throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual css::uno::Sequence< css::uno::Reference< css::reflection::XIdlField > > SAL_CALL getFields() throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL createObject( css::uno::Any & rObj ) throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};


class IdlMemberImpl
    : public ::cppu::WeakImplHelper< css::reflection::XIdlMember >
{
    IdlReflectionServiceImpl *  _pReflection;
    OUString                    _aName;

    typelib_TypeDescription *   _pTypeDescr;
    typelib_TypeDescription *   _pDeclTypeDescr;

protected:
    css::uno::Reference< css::reflection::XIdlClass >      _xDeclClass;

public:
    IdlReflectionServiceImpl *  getReflection() const
        { return _pReflection; }
    typelib_TypeDescription *   getTypeDescr() const
        { return _pTypeDescr; }
    typelib_TypeDescription *   getDeclTypeDescr() const
        { return _pDeclTypeDescr; }

    // ctor/ dtor
    IdlMemberImpl( IdlReflectionServiceImpl * pReflection, const OUString & rName,
                   typelib_TypeDescription * pTypeDescr, typelib_TypeDescription * pDeclTypeDescr );
    virtual ~IdlMemberImpl();

    // XIdlMember
    virtual css::uno::Reference< css::reflection::XIdlClass > SAL_CALL getDeclaringClass() throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getName() throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};


// coerces to type descr pTo else queries for it: the interface pointer is returned via rDest
// ## type to XidlClass coercion possible
inline bool extract(
    const css::uno::Any & rObj, typelib_InterfaceTypeDescription * pTo,
    css::uno::Reference< css::uno::XInterface > & rDest,
    IdlReflectionServiceImpl * pRefl )
{
    rDest.clear();
    if (0 != pTo)
    {
        if (! rObj.hasValue())
            return true;
        if (rObj.getValueTypeClass() == css::uno::TypeClass_INTERFACE)
        {
            return ::uno_type_assignData(
                &rDest, pTo->aBase.pWeakRef,
                const_cast< void * >( rObj.getValue() ), rObj.getValueTypeRef(),
                reinterpret_cast< uno_QueryInterfaceFunc >(css::uno::cpp_queryInterface),
                reinterpret_cast< uno_AcquireFunc >(css::uno::cpp_acquire),
                reinterpret_cast< uno_ReleaseFunc >(css::uno::cpp_release) );
        }
        else if (rObj.getValueTypeClass() == css::uno::TypeClass_TYPE)
        {
            rDest = pRefl->forType( static_cast< const css::uno::Type * >( rObj.getValue() )->getTypeLibType() );
            return rDest.is();
        }
    }
    return false;
}

inline bool coerce_assign(
    void * pDest, typelib_TypeDescription * pTD, const css::uno::Any & rSource,
    IdlReflectionServiceImpl * pRefl )
{
    if (pTD->eTypeClass == typelib_TypeClass_INTERFACE)
    {
        css::uno::Reference< css::uno::XInterface > xVal;
        if (extract( rSource, reinterpret_cast<typelib_InterfaceTypeDescription *>(pTD), xVal, pRefl ))
        {
            if (*static_cast<css::uno::XInterface **>(pDest))
                (*static_cast<css::uno::XInterface **>(pDest))->release();
            *static_cast<css::uno::XInterface **>(pDest) = xVal.get();
            if (*static_cast<css::uno::XInterface **>(pDest))
                (*static_cast<css::uno::XInterface **>(pDest))->acquire();
            return true;
        }
        return false;
    }
    else if (pTD->eTypeClass == typelib_TypeClass_ANY)
    {
        return uno_assignData(
            pDest, pTD,
            const_cast<css::uno::Any *>(&rSource), pTD,
            reinterpret_cast< uno_QueryInterfaceFunc >(css::uno::cpp_queryInterface),
            reinterpret_cast< uno_AcquireFunc >(css::uno::cpp_acquire),
            reinterpret_cast< uno_ReleaseFunc >(css::uno::cpp_release) );
    }
    else
    {
        return uno_type_assignData(
            pDest, pTD->pWeakRef,
            const_cast<void *>(rSource.getValue()), rSource.getValueTypeRef(),
            reinterpret_cast< uno_QueryInterfaceFunc >(css::uno::cpp_queryInterface),
            reinterpret_cast< uno_AcquireFunc >(css::uno::cpp_acquire),
            reinterpret_cast< uno_ReleaseFunc >(css::uno::cpp_release) );
    }
}

}


#endif // INCLUDED_STOC_SOURCE_COREREFLECTION_BASE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
