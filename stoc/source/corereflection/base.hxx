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

#include <sal/config.h>

#include <o3tl/any.hxx>
#include <osl/mutex.hxx>
#include <uno/mapping.hxx>
#include <uno/dispatcher.h>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/component.hxx>
#include <rtl/ustring.hxx>
#include <rtl/ref.hxx>

#include "lrucache.hxx"

#ifdef TEST_LIST_CLASSES
#include <vector>
#include <algorithm>
#endif
#include <unordered_map>
#include <memory>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>

#include <com/sun/star/reflection/XIdlReflection.hpp>

namespace com::sun::star::uno { class XComponentContext; }
namespace com::sun::star::reflection { class XIdlClass; }
namespace com::sun::star::reflection { class XIdlField; }
namespace com::sun::star::reflection { class XIdlMethod; }

namespace stoc_corefl
{

#ifdef TEST_LIST_CLASSES
extern std::vector<OUString> g_aClassNames;
#endif


::osl::Mutex & getMutexAccess();


inline bool td_equals( typelib_TypeDescription * pTD, typelib_TypeDescriptionReference * pType )
{
    return (pTD->pWeakRef == pType ||
            (pTD->pTypeName->length == pType->pTypeName->length &&
             rtl_ustr_compare( pTD->pTypeName->buffer, pType->pTypeName->buffer ) == 0));
}

typedef std::unordered_map< OUString, css::uno::WeakReference< css::reflection::XIdlField > > OUString2Field;
typedef std::unordered_map< OUString, css::uno::WeakReference< css::reflection::XIdlMethod > > OUString2Method;


class IdlReflectionServiceImpl
    : public ::cppu::OComponentHelper
    , public css::reflection::XIdlReflection
    , public css::container::XHierarchicalNameAccess
    , public css::lang::XServiceInfo
{
    ::osl::Mutex                            _aComponentMutex;
    css::uno::Reference< css::container::XHierarchicalNameAccess >    _xTDMgr;

    // caching
    LRU_CacheAnyByOUString                  _aElements;

    css::uno::Mapping                     _aCpp2Uno;
    css::uno::Mapping                     _aUno2Cpp;

    inline css::uno::Reference< css::reflection::XIdlClass > constructClass( typelib_TypeDescription * pTypeDescr );

public:
    /// @throws css::uno::RuntimeException
    const css::uno::Mapping & getCpp2Uno();
    /// @throws css::uno::RuntimeException
    const css::uno::Mapping & getUno2Cpp();
    /// @throws css::uno::RuntimeException
    uno_Interface * mapToUno( const css::uno::Any & rObj, typelib_InterfaceTypeDescription * pTo );

    // ctor/ dtor
    explicit IdlReflectionServiceImpl( const css::uno::Reference< css::uno::XComponentContext > & xContext );
    virtual ~IdlReflectionServiceImpl() override;

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) override;
    virtual void SAL_CALL acquire() noexcept override;
    virtual void SAL_CALL release() noexcept override;

    // some XComponent part from OComponentHelper
    virtual void SAL_CALL dispose() override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString & rServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes() override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() override;

    // XIdlReflection
    virtual css::uno::Reference< css::reflection::XIdlClass > SAL_CALL forName( const OUString & rTypeName ) override;
    virtual css::uno::Reference< css::reflection::XIdlClass > SAL_CALL getType( const css::uno::Any & rObj ) override;

    // XHierarchicalNameAccess
    virtual css::uno::Any SAL_CALL getByHierarchicalName( const OUString & rName ) override;
    virtual sal_Bool SAL_CALL hasByHierarchicalName( const OUString & rName ) override;

    /// @throws css::uno::RuntimeException
    css::uno::Reference< css::reflection::XIdlClass > forType( typelib_TypeDescription * pTypeDescr );
    /// @throws css::uno::RuntimeException
    css::uno::Reference< css::reflection::XIdlClass > forType( typelib_TypeDescriptionReference * pRef );
};


class IdlClassImpl
    : public ::cppu::WeakImplHelper< css::reflection::XIdlClass >
{
    rtl::Reference<IdlReflectionServiceImpl>
                                 m_xReflection;

    OUString                    _aName;
    css::uno::TypeClass         _eTypeClass;

    typelib_TypeDescription *   _pTypeDescr;

public:
    typelib_TypeDescription *   getTypeDescr() const
        { return _pTypeDescr; }
    IdlReflectionServiceImpl *  getReflection() const
        { return m_xReflection.get(); }

    // Ctor
    IdlClassImpl( IdlReflectionServiceImpl * pReflection,
                  const OUString & rName, typelib_TypeClass eTypeClass,
                  typelib_TypeDescription * pTypeDescr );
    virtual ~IdlClassImpl() override;

    // XIdlClassImpl default implementation
    virtual css::uno::TypeClass SAL_CALL getTypeClass() override;
    virtual OUString SAL_CALL getName() override;
    virtual sal_Bool SAL_CALL equals( const css::uno::Reference< css::reflection::XIdlClass >& xType ) override;

    virtual sal_Bool SAL_CALL isAssignableFrom( const css::uno::Reference< css::reflection::XIdlClass > & xType ) override;
    virtual void SAL_CALL createObject( css::uno::Any & rObj ) override;

    // def impl ????
    virtual css::uno::Sequence< css::uno::Reference< css::reflection::XIdlClass > > SAL_CALL getClasses() override;
    virtual css::uno::Reference< css::reflection::XIdlClass > SAL_CALL getClass( const OUString & rName ) override;
    virtual css::uno::Sequence< css::uno::Reference< css::reflection::XIdlClass > > SAL_CALL getInterfaces() override;

    // structs, interfaces
    virtual css::uno::Sequence< css::uno::Reference< css::reflection::XIdlClass > > SAL_CALL getSuperclasses() override;
    // structs
    virtual css::uno::Reference< css::reflection::XIdlField > SAL_CALL getField( const OUString & rName ) override;
    virtual css::uno::Sequence< css::uno::Reference< css::reflection::XIdlField > > SAL_CALL getFields() override;
    // interfaces
    virtual css::uno::Uik SAL_CALL getUik() override;
    virtual css::uno::Reference< css::reflection::XIdlMethod > SAL_CALL getMethod( const OUString & rName ) override;
    virtual css::uno::Sequence< css::uno::Reference< css::reflection::XIdlMethod > > SAL_CALL getMethods() override;
    // array
    virtual css::uno::Reference< css::reflection::XIdlClass > SAL_CALL getComponentType() override;
    virtual css::uno::Reference< css::reflection::XIdlArray > SAL_CALL getArray() override;
};


class InterfaceIdlClassImpl
    : public IdlClassImpl
{
    typedef std::pair< OUString, typelib_TypeDescription * > MemberInit;

    css::uno::Sequence< css::uno::Reference< css::reflection::XIdlClass > >      _xSuperClasses;

    std::unique_ptr<MemberInit[]>           _pSortedMemberInit; // first methods, then attributes
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
        , _nMethods( 0 )
        , _nAttributes( 0 )
        {}
    virtual ~InterfaceIdlClassImpl() override;

    // IdlClassImpl modifications
    virtual sal_Bool SAL_CALL isAssignableFrom( const css::uno::Reference< css::reflection::XIdlClass > & xType ) override;
    virtual css::uno::Sequence< css::uno::Reference< css::reflection::XIdlClass > > SAL_CALL getSuperclasses() override;
    virtual css::uno::Uik SAL_CALL getUik() override;
    virtual css::uno::Reference< css::reflection::XIdlMethod > SAL_CALL getMethod( const OUString & rName ) override;
    virtual css::uno::Sequence< css::uno::Reference< css::reflection::XIdlMethod > > SAL_CALL getMethods() override;
    virtual css::uno::Reference< css::reflection::XIdlField > SAL_CALL getField( const OUString & rName ) override;
    virtual css::uno::Sequence< css::uno::Reference< css::reflection::XIdlField > > SAL_CALL getFields() override;
    virtual void SAL_CALL createObject( css::uno::Any & rObj ) override;
};


class CompoundIdlClassImpl
    : public IdlClassImpl
{
    css::uno::Reference< css::reflection::XIdlClass >
                                             _xSuperClass;
    std::unique_ptr< css::uno::Sequence< css::uno::Reference< css::reflection::XIdlField > > >
                                            _pFields;
    OUString2Field                          _aName2Field;

public:
    typelib_CompoundTypeDescription * getTypeDescr() const
        { return reinterpret_cast<typelib_CompoundTypeDescription *>(IdlClassImpl::getTypeDescr()); }

    // ctor/ dtor
    CompoundIdlClassImpl( IdlReflectionServiceImpl * pReflection,
                          const OUString & rName, typelib_TypeClass eTypeClass,
                          typelib_TypeDescription * pTypeDescr )
        : IdlClassImpl( pReflection, rName, eTypeClass, pTypeDescr )
        {}
    virtual ~CompoundIdlClassImpl() override;

    // IdlClassImpl modifications
    virtual sal_Bool SAL_CALL isAssignableFrom( const css::uno::Reference< css::reflection::XIdlClass > & xType ) override;
    virtual css::uno::Sequence< css::uno::Reference< css::reflection::XIdlClass > > SAL_CALL getSuperclasses() override;
    virtual css::uno::Reference< css::reflection::XIdlField > SAL_CALL getField( const OUString & rName ) override;
    virtual css::uno::Sequence< css::uno::Reference< css::reflection::XIdlField > > SAL_CALL getFields() override;
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

    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) override;
    virtual void SAL_CALL acquire() noexcept override;
    virtual void SAL_CALL release() noexcept override;

    // XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes() override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() override;

    // IdlClassImpl modifications
    virtual sal_Bool SAL_CALL isAssignableFrom( const css::uno::Reference< css::reflection::XIdlClass > & xType ) override;
    virtual css::uno::Reference< css::reflection::XIdlClass > SAL_CALL getComponentType() override;
    virtual css::uno::Reference< css::reflection::XIdlArray > SAL_CALL getArray() override;

    // XIdlArray
    virtual void SAL_CALL realloc( css::uno::Any & rArray, sal_Int32 nLen ) override;
    virtual sal_Int32 SAL_CALL getLen( const css::uno::Any & rArray ) override;
    virtual css::uno::Any SAL_CALL get( const css::uno::Any & rArray, sal_Int32 nIndex ) override;
    virtual void SAL_CALL set( css::uno::Any & rArray, sal_Int32 nIndex, const css::uno::Any & rNewValue ) override;
};


class EnumIdlClassImpl
    : public IdlClassImpl
{
    std::unique_ptr< css::uno::Sequence< css::uno::Reference< css::reflection::XIdlField > > > _pFields;
    OUString2Field                       _aName2Field;

public:
    typelib_EnumTypeDescription * getTypeDescr() const
        { return reinterpret_cast<typelib_EnumTypeDescription *>(IdlClassImpl::getTypeDescr()); }

    // ctor/ dtor
    EnumIdlClassImpl( IdlReflectionServiceImpl * pReflection,
                      const OUString & rName, typelib_TypeClass eTypeClass,
                      typelib_TypeDescription * pTypeDescr )
        : IdlClassImpl( pReflection, rName, eTypeClass, pTypeDescr )
        {}
    virtual ~EnumIdlClassImpl() override;

    // IdlClassImpl modifications
    virtual css::uno::Reference< css::reflection::XIdlField > SAL_CALL getField( const OUString & rName ) override;
    virtual css::uno::Sequence< css::uno::Reference< css::reflection::XIdlField > > SAL_CALL getFields() override;
    virtual void SAL_CALL createObject( css::uno::Any & rObj ) override;
};


class IdlMemberImpl
    : public ::cppu::WeakImplHelper< css::reflection::XIdlMember >
{
    rtl::Reference<IdlReflectionServiceImpl>
                                m_xReflection;
    OUString                    _aName;

    typelib_TypeDescription *   _pTypeDescr;
    typelib_TypeDescription *   _pDeclTypeDescr;

protected:
    css::uno::Reference< css::reflection::XIdlClass >      _xDeclClass;

public:
    IdlReflectionServiceImpl *  getReflection() const
        { return m_xReflection.get(); }
    typelib_TypeDescription *   getTypeDescr() const
        { return _pTypeDescr; }
    typelib_TypeDescription *   getDeclTypeDescr() const
        { return _pDeclTypeDescr; }

    // ctor/ dtor
    IdlMemberImpl( IdlReflectionServiceImpl * pReflection, const OUString & rName,
                   typelib_TypeDescription * pTypeDescr, typelib_TypeDescription * pDeclTypeDescr );
    virtual ~IdlMemberImpl() override;

    // XIdlMember
    virtual css::uno::Reference< css::reflection::XIdlClass > SAL_CALL getDeclaringClass() override;
    virtual OUString SAL_CALL getName() override;
};


// coerces to type descr pTo else queries for it: the interface pointer is returned via rDest
// ## type to XidlClass coercion possible
inline bool extract(
    const css::uno::Any & rObj, typelib_InterfaceTypeDescription * pTo,
    css::uno::Reference< css::uno::XInterface > & rDest,
    IdlReflectionServiceImpl * pRefl )
{
    rDest.clear();
    if (nullptr != pTo)
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
        else if (auto t = o3tl::tryAccess<css::uno::Type>(rObj))
        {
            rDest = pRefl->forType( t->getTypeLibType() );
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
