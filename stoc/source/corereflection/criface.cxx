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

#include <sal/config.h>

#include <cassert>
#include <cstddef>
#include <limits>

#ifdef SAL_UNX
#include <sal/alloca.h>
#endif
#include <o3tl/any.hxx>
#include <typelib/typedescription.hxx>
#include <uno/data.h>

#include "base.hxx"

#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <com/sun/star/reflection/XIdlField2.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <cppuhelper/typeprovider.hxx>

using namespace css::lang;
using namespace css::reflection;
using namespace css::uno;

namespace {

std::size_t multipleOf16(std::size_t n) {
    assert(n <= std::numeric_limits<std::size_t>::max() - 15);
    return (n + 15) & ~std::size_t(15);
}

}

namespace stoc_corefl
{

namespace {

typedef cppu::ImplInheritanceHelper<IdlMemberImpl, XIdlField, XIdlField2> IdlAttributeFieldImpl_Base;
class IdlAttributeFieldImpl : public IdlAttributeFieldImpl_Base
{
public:
    typelib_InterfaceAttributeTypeDescription * getAttributeTypeDescr() const
        { return reinterpret_cast<typelib_InterfaceAttributeTypeDescription *>(getTypeDescr()); }

    IdlAttributeFieldImpl( IdlReflectionServiceImpl * pReflection, const OUString & rName,
                           typelib_TypeDescription * pTypeDescr, typelib_TypeDescription * pDeclTypeDescr )
        : IdlAttributeFieldImpl_Base( pReflection, rName, pTypeDescr, pDeclTypeDescr )
        {}

    // XIdlMember
    virtual Reference< XIdlClass > SAL_CALL getDeclaringClass() override;
    virtual OUString SAL_CALL getName() override;
    // XIdlField
    virtual Reference< XIdlClass > SAL_CALL getType() override;
    virtual FieldAccessMode SAL_CALL getAccessMode() override;
    virtual Any SAL_CALL get( const Any & rObj ) override;
    virtual void SAL_CALL set( const Any & rObj, const Any & rValue ) override;
    // XIdlField2: getType, getAccessMode and get are equal to XIdlField
    virtual void SAL_CALL set( Any & rObj, const Any & rValue ) override;

private:
    void checkException(
        uno_Any * exception, Reference< XInterface > const & context) const;
};

}

// XIdlMember

Reference< XIdlClass > IdlAttributeFieldImpl::getDeclaringClass()
{
    if (! _xDeclClass.is())
    {
        ::osl::MutexGuard aGuard( getMutexAccess() );
        if (! _xDeclClass.is())
        {
            OUString aName(getAttributeTypeDescr()->aBase.aBase.pTypeName);
            sal_Int32 i = aName.indexOf(':');
            OSL_ASSERT(i >= 0);
            _xDeclClass = getReflection()->forName(aName.copy(0, i));
        }
    }
    return _xDeclClass;
}

OUString IdlAttributeFieldImpl::getName()
{
    return IdlMemberImpl::getName();
}

// XIdlField

Reference< XIdlClass > IdlAttributeFieldImpl::getType()
{
    return getReflection()->forType(
        getAttributeTypeDescr()->pAttributeTypeRef );
}

FieldAccessMode IdlAttributeFieldImpl::getAccessMode()
{
    return (getAttributeTypeDescr()->bReadOnly
            ? FieldAccessMode_READONLY : FieldAccessMode_READWRITE);
}

Any IdlAttributeFieldImpl::get( const Any & rObj )
{
    uno_Interface * pUnoI = getReflection()->mapToUno(
        rObj, reinterpret_cast<typelib_InterfaceTypeDescription *>(getDeclTypeDescr()) );
    OSL_ENSURE( pUnoI, "### illegal destination object given!" );
    if (pUnoI)
    {
        TypeDescription aTD( getAttributeTypeDescr()->pAttributeTypeRef );
        typelib_TypeDescription * pTD = aTD.get();

        uno_Any aExc;
        uno_Any * pExc = &aExc;
        void * pReturn = alloca( pTD->nSize );

        (*pUnoI->pDispatcher)( pUnoI, getTypeDescr(), pReturn, nullptr, &pExc );
        (*pUnoI->release)( pUnoI );

        checkException(pExc, *o3tl::doAccess<Reference<XInterface>>(rObj));
        Any aRet;
        uno_any_destruct(
            &aRet, reinterpret_cast< uno_ReleaseFunc >(cpp_release) );
        uno_any_constructAndConvert( &aRet, pReturn, pTD, getReflection()->getUno2Cpp().get() );
        uno_destructData( pReturn, pTD, nullptr );
        return aRet;
    }
    throw IllegalArgumentException(
        "illegal object given!",
        getXWeak(), 0 );
}

void IdlAttributeFieldImpl::set( Any & rObj, const Any & rValue )
{
    if (getAttributeTypeDescr()->bReadOnly)
    {
        throw IllegalAccessException(
            "cannot set readonly attribute!",
            getXWeak() );
    }

    uno_Interface * pUnoI = getReflection()->mapToUno(
        rObj, reinterpret_cast<typelib_InterfaceTypeDescription *>(getDeclTypeDescr()) );
    OSL_ENSURE( pUnoI, "### illegal destination object given!" );
    if (pUnoI)
    {
        TypeDescription aTD( getAttributeTypeDescr()->pAttributeTypeRef );
        typelib_TypeDescription * pTD = aTD.get();

        // construct uno value to be set
        void * pArgs[1];
        void * pArg = pArgs[0] = alloca( pTD->nSize );

        bool bAssign;
        if (pTD->eTypeClass == typelib_TypeClass_ANY)
        {
            uno_copyAndConvertData( pArg, const_cast< Any * >(&rValue),
                                    pTD, getReflection()->getCpp2Uno().get() );
            bAssign = true;
        }
        else if (typelib_typedescriptionreference_equals( rValue.getValueTypeRef(), pTD->pWeakRef ))
        {
            uno_copyAndConvertData( pArg, const_cast< void * >(rValue.getValue()),
                                    pTD, getReflection()->getCpp2Uno().get() );
            bAssign = true;
        }
        else if (pTD->eTypeClass == typelib_TypeClass_INTERFACE)
        {
            Reference< XInterface > xObj;
            bAssign = extract(
                rValue, reinterpret_cast<typelib_InterfaceTypeDescription *>(pTD), xObj,
                getReflection() );
            if (bAssign)
            {
                *static_cast<void **>(pArg) = getReflection()->getCpp2Uno().mapInterface(
                    xObj.get(), reinterpret_cast<typelib_InterfaceTypeDescription *>(pTD) );
            }
        }
        else
        {
            typelib_TypeDescription * pValueTD = nullptr;
            TYPELIB_DANGER_GET( &pValueTD, rValue.getValueTypeRef() );
            // construct temp uno val to do proper assignment: todo opt
            void * pTemp = alloca( pValueTD->nSize );
            uno_copyAndConvertData(
                pTemp, const_cast<void *>(rValue.getValue()), pValueTD, getReflection()->getCpp2Uno().get() );
            uno_constructData(
                pArg, pTD );
            // assignment does simple conversion
            bAssign = uno_assignData(
                pArg, pTD, pTemp, pValueTD, nullptr, nullptr, nullptr );
            uno_destructData(
                pTemp, pValueTD, nullptr );
            TYPELIB_DANGER_RELEASE( pValueTD );
        }

        if (bAssign)
        {
            uno_Any aExc;
            uno_Any * pExc = &aExc;
            (*pUnoI->pDispatcher)( pUnoI, getTypeDescr(), nullptr, pArgs, &pExc );
            (*pUnoI->release)( pUnoI );

            uno_destructData( pArg, pTD, nullptr );
            checkException(pExc, *o3tl::doAccess<Reference<XInterface>>(rObj));
            return;
        }
        (*pUnoI->release)( pUnoI );

        throw IllegalArgumentException(
            "illegal value given!",
            *o3tl::doAccess<Reference<XInterface>>(rObj), 1 );
    }
    throw IllegalArgumentException(
        "illegal destination object given!",
        getXWeak(), 0 );
}

void IdlAttributeFieldImpl::set( const Any & rObj, const Any & rValue )
{
    IdlAttributeFieldImpl::set( const_cast< Any & >( rObj ), rValue );
}

void IdlAttributeFieldImpl::checkException(
    uno_Any * exception, Reference< XInterface > const & context) const
{
    if (exception == nullptr)
        return;

    Any e;
    uno_any_destruct(&e, reinterpret_cast< uno_ReleaseFunc >(cpp_release));
    uno_type_any_constructAndConvert(
        &e, exception->pData, exception->pType,
        getReflection()->getUno2Cpp().get());
    uno_any_destruct(exception, nullptr);
    if (!e.isExtractableTo(
            cppu::UnoType<RuntimeException>::get()))
    {
        throw WrappedTargetRuntimeException(
            "non-RuntimeException occurred when accessing an"
            " interface type attribute",
            context, e);
    }
    cppu::throwException(e);
}

namespace {

typedef cppu::ImplInheritanceHelper<IdlMemberImpl, XIdlMethod> IdlInterfaceMethodImpl_Base;
class IdlInterfaceMethodImpl : public IdlInterfaceMethodImpl_Base
{
    std::optional<Sequence< Reference< XIdlClass > >>   m_xExceptionTypes;
    std::optional<Sequence< Reference< XIdlClass > >>   m_xParamTypes;
    std::optional<Sequence< ParamInfo >>                m_xParamInfos;

public:
    typelib_InterfaceMethodTypeDescription * getMethodTypeDescr() const
        { return reinterpret_cast<typelib_InterfaceMethodTypeDescription *>(getTypeDescr()); }

    IdlInterfaceMethodImpl( IdlReflectionServiceImpl * pReflection, const OUString & rName,
                            typelib_TypeDescription * pTypeDescr, typelib_TypeDescription * pDeclTypeDescr )
        : IdlInterfaceMethodImpl_Base( pReflection, rName, pTypeDescr, pDeclTypeDescr )
        {}

    // XTypeProvider
    virtual Sequence< sal_Int8 > SAL_CALL getImplementationId() override;

    // XIdlMember
    virtual Reference< XIdlClass > SAL_CALL getDeclaringClass() override;
    virtual OUString SAL_CALL getName() override;
    // XIdlMethod
    virtual Reference< XIdlClass > SAL_CALL getReturnType() override;
    virtual Sequence< Reference< XIdlClass > > SAL_CALL getParameterTypes() override;
    virtual Sequence< ParamInfo > SAL_CALL getParameterInfos() override;
    virtual Sequence< Reference< XIdlClass > > SAL_CALL getExceptionTypes() override;
    virtual MethodMode SAL_CALL getMode() override;
    virtual Any SAL_CALL invoke( const Any & rObj, Sequence< Any > & rArgs ) override;
};

}

// XTypeProvider

Sequence< sal_Int8 > IdlInterfaceMethodImpl::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

// XIdlMember

Reference< XIdlClass > IdlInterfaceMethodImpl::getDeclaringClass()
{
    if (! _xDeclClass.is())
    {
        ::osl::MutexGuard aGuard( getMutexAccess() );
        if (! _xDeclClass.is())
        {
            OUString aName(getMethodTypeDescr()->aBase.aBase.pTypeName);
            sal_Int32 i = aName.indexOf(':');
            OSL_ASSERT(i >= 0);
            _xDeclClass = getReflection()->forName(aName.copy(0, i));
        }
    }
    return _xDeclClass;
}

OUString IdlInterfaceMethodImpl::getName()
{
    return IdlMemberImpl::getName();
}

// XIdlMethod

Reference< XIdlClass > SAL_CALL IdlInterfaceMethodImpl::getReturnType()
{
    return getReflection()->forType( getMethodTypeDescr()->pReturnTypeRef );
}

Sequence< Reference< XIdlClass > > IdlInterfaceMethodImpl::getExceptionTypes()
{
    if (! m_xExceptionTypes)
    {
        ::osl::MutexGuard aGuard( getMutexAccess() );
        if (! m_xExceptionTypes)
        {
            sal_Int32 nExc = getMethodTypeDescr()->nExceptions;
            Sequence< Reference< XIdlClass > > aTempExceptionTypes( nExc );
            Reference< XIdlClass > * pExceptionTypes = aTempExceptionTypes.getArray();

            typelib_TypeDescriptionReference ** ppExc =
                getMethodTypeDescr()->ppExceptions;
            IdlReflectionServiceImpl * pRefl = getReflection();

            while (nExc--)
                pExceptionTypes[nExc] = pRefl->forType( ppExc[nExc] );

            m_xExceptionTypes = std::move(aTempExceptionTypes);
        }
    }
    return *m_xExceptionTypes;
}

Sequence< Reference< XIdlClass > > IdlInterfaceMethodImpl::getParameterTypes()
{
    if (! m_xParamTypes)
    {
        ::osl::MutexGuard aGuard( getMutexAccess() );
        if (! m_xParamTypes)
        {
            sal_Int32 nParams = getMethodTypeDescr()->nParams;
            Sequence< Reference< XIdlClass > > aTempParamTypes( nParams );
            Reference< XIdlClass > * pParamTypes = aTempParamTypes.getArray();

            typelib_MethodParameter * pTypelibParams =
                getMethodTypeDescr()->pParams;
            IdlReflectionServiceImpl * pRefl = getReflection();

            while (nParams--)
                pParamTypes[nParams] = pRefl->forType( pTypelibParams[nParams].pTypeRef );

            m_xParamTypes = std::move(aTempParamTypes);
        }
    }
    return *m_xParamTypes;
}

Sequence< ParamInfo > IdlInterfaceMethodImpl::getParameterInfos()
{
    if (! m_xParamInfos)
    {
        ::osl::MutexGuard aGuard( getMutexAccess() );
        if (! m_xParamInfos)
        {
            sal_Int32 nParams = getMethodTypeDescr()->nParams;
            Sequence< ParamInfo > aTempParamInfos( nParams );
            ParamInfo * pParamInfos = aTempParamInfos.getArray();

            typelib_MethodParameter * pTypelibParams =
                getMethodTypeDescr()->pParams;

            if (m_xParamTypes) // use param types
            {
                const Reference< XIdlClass > * pParamTypes = m_xParamTypes->getConstArray();

                while (nParams--)
                {
                    const typelib_MethodParameter & rParam = pTypelibParams[nParams];
                    ParamInfo & rInfo = pParamInfos[nParams];
                    rInfo.aName = rParam.pName;
                    if (rParam.bIn)
                        rInfo.aMode = (rParam.bOut ? ParamMode_INOUT : ParamMode_IN);
                    else
                        rInfo.aMode = ParamMode_OUT;
                    rInfo.aType = pParamTypes[nParams];
                }
            }
            else // make also param types sequence if not already initialized
            {
                Sequence< Reference< XIdlClass > > aTempParamTypes( nParams );
                Reference< XIdlClass > * pParamTypes = aTempParamTypes.getArray();

                IdlReflectionServiceImpl * pRefl = getReflection();

                while (nParams--)
                {
                    const typelib_MethodParameter & rParam = pTypelibParams[nParams];
                    ParamInfo & rInfo = pParamInfos[nParams];
                    rInfo.aName = rParam.pName;
                    if (rParam.bIn)
                        rInfo.aMode = (rParam.bOut ? ParamMode_INOUT : ParamMode_IN);
                    else
                        rInfo.aMode = ParamMode_OUT;
                    rInfo.aType = pParamTypes[nParams] = pRefl->forType( rParam.pTypeRef );
                }

                m_xParamTypes = std::move(aTempParamTypes);
            }

            m_xParamInfos = std::move(aTempParamInfos);
        }
    }
    return *m_xParamInfos;
}

MethodMode SAL_CALL IdlInterfaceMethodImpl::getMode()
{
    return
        getMethodTypeDescr()->bOneWay ? MethodMode_ONEWAY : MethodMode_TWOWAY;
}

Any SAL_CALL IdlInterfaceMethodImpl::invoke( const Any & rObj, Sequence< Any > & rArgs )
{
    if (auto ifc = o3tl::tryAccess<css::uno::Reference<css::uno::XInterface>>(
            rObj))
    {
        // acquire()/ release()
        if (rtl_ustr_ascii_compare( getTypeDescr()->pTypeName->buffer,
                                    "com.sun.star.uno.XInterface::acquire" ) == 0)
        {
            (*ifc)->acquire();
            return Any();
        }
        else if (rtl_ustr_ascii_compare( getTypeDescr()->pTypeName->buffer,
                                         "com.sun.star.uno.XInterface::release" ) == 0)
        {
            (*ifc)->release();
            return Any();
        }
    }

    uno_Interface * pUnoI = getReflection()->mapToUno(
        rObj, reinterpret_cast<typelib_InterfaceTypeDescription *>(getDeclTypeDescr()) );
    OSL_ENSURE( pUnoI, "### illegal destination object given!" );
    if (pUnoI)
    {
        sal_Int32 nParams = getMethodTypeDescr()->nParams;
        if (rArgs.getLength() != nParams)
        {
            (*pUnoI->release)( pUnoI );
            throw IllegalArgumentException(
                "expected " + OUString::number(nParams) +
                 " arguments, got " + OUString::number(rArgs.getLength()),
                *o3tl::doAccess<Reference<XInterface>>(rObj), 1 );
        }

        Any * pCppArgs = rArgs.getArray();
        typelib_MethodParameter * pParams = getMethodTypeDescr()->pParams;
        typelib_TypeDescription * pReturnType = nullptr;
        TYPELIB_DANGER_GET(
            &pReturnType, getMethodTypeDescr()->pReturnTypeRef );

        // C/C++ ABIs typically assume that structs are padded at the end, and
        // that those padding bytes may be written to (e.g., to write into the
        // end of a "short" struct by writing the full contents of a "long"
        // register); so create enough space here (assuming that no ABI requires
        // padding larger than 16 byte boundaries):
        void * pUnoReturn = (pReturnType->nSize == 0) ? nullptr : alloca( multipleOf16(pReturnType->nSize) );
        void ** ppUnoArgs = static_cast<void **>(alloca( sizeof(void *) * nParams *2 ));
        typelib_TypeDescription ** ppParamTypes = reinterpret_cast<typelib_TypeDescription **>(ppUnoArgs + nParams);

        // convert arguments
        for ( sal_Int32 nPos = 0; nPos < nParams; ++nPos )
        {
            ppParamTypes[nPos] = nullptr;
            TYPELIB_DANGER_GET( ppParamTypes + nPos, pParams[nPos].pTypeRef );
            typelib_TypeDescription * pTD = ppParamTypes[nPos];
            assert(pTD);

            ppUnoArgs[nPos] = alloca( pTD->nSize );
            if (pParams[nPos].bIn)
            {
                bool bAssign;
                if (typelib_typedescriptionreference_equals(
                        pCppArgs[nPos].getValueTypeRef(), pTD->pWeakRef ))
                {
                    uno_type_copyAndConvertData(
                        ppUnoArgs[nPos], const_cast<void *>(pCppArgs[nPos].getValue()),
                        pCppArgs[nPos].getValueTypeRef(), getReflection()->getCpp2Uno().get() );
                    bAssign = true;
                }
                else if (pTD->eTypeClass == typelib_TypeClass_ANY)
                {
                    uno_type_any_constructAndConvert(
                        static_cast<uno_Any *>(ppUnoArgs[nPos]), const_cast<void *>(pCppArgs[nPos].getValue()),
                        pCppArgs[nPos].getValueTypeRef(), getReflection()->getCpp2Uno().get() );
                    bAssign = true;
                }
                else if (pTD->eTypeClass == typelib_TypeClass_INTERFACE)
                {
                    Reference< XInterface > xDest;
                    bAssign = extract(
                        pCppArgs[nPos], reinterpret_cast<typelib_InterfaceTypeDescription *>(pTD),
                        xDest, getReflection() );
                    if (bAssign)
                    {
                        *static_cast<void **>(ppUnoArgs[nPos]) = getReflection()->getCpp2Uno().mapInterface(
                            xDest.get(), reinterpret_cast<typelib_InterfaceTypeDescription *>(pTD) );
                    }
                }
                else
                {
                    typelib_TypeDescription * pValueTD = nullptr;
                    TYPELIB_DANGER_GET( &pValueTD, pCppArgs[nPos].getValueTypeRef() );
                    // construct temp uno val to do proper assignment: todo opt
                    void * pTemp = alloca( pValueTD->nSize );
                    uno_copyAndConvertData(
                        pTemp, const_cast<void *>(pCppArgs[nPos].getValue()), pValueTD,
                        getReflection()->getCpp2Uno().get() );
                    uno_constructData(
                        ppUnoArgs[nPos], pTD );
                    // assignment does simple conversion
                    bAssign = uno_assignData(
                        ppUnoArgs[nPos], pTD, pTemp, pValueTD, nullptr, nullptr, nullptr );
                    uno_destructData(
                        pTemp, pValueTD, nullptr );
                    TYPELIB_DANGER_RELEASE( pValueTD );
                }

                if (! bAssign)
                {
                    IllegalArgumentException aExc(
                        "cannot coerce argument type during corereflection call:"
                        "\narg no.: " + OUString::number(nPos)
                        + " expected: \"" + OUString::unacquired(&pTD->pTypeName)
                        + "\" actual: \"" + OUString::unacquired(&pCppArgs[nPos].getValueTypeRef()->pTypeName)
                        + "\"",
                        *o3tl::doAccess<Reference<XInterface>>(rObj), static_cast<sal_Int16>(nPos) );

                    // cleanup
                    while (nPos--)
                    {
                        if (pParams[nPos].bIn)
                            uno_destructData( ppUnoArgs[nPos], ppParamTypes[nPos], nullptr );
                        TYPELIB_DANGER_RELEASE( ppParamTypes[nPos] );
                    }
                    TYPELIB_DANGER_RELEASE( pReturnType );
                    (*pUnoI->release)( pUnoI );

                    throw aExc;
                }
            }
        }

        uno_Any aUnoExc;
        uno_Any * pUnoExc = &aUnoExc;

        (*pUnoI->pDispatcher)(
            pUnoI, getTypeDescr(), pUnoReturn, ppUnoArgs, &pUnoExc );
        (*pUnoI->release)( pUnoI );

        Any aRet;
        if (pUnoExc)
        {
            // cleanup
            while (nParams--)
            {
                if (pParams[nParams].bIn)
                    uno_destructData( ppUnoArgs[nParams], ppParamTypes[nParams], nullptr );
                TYPELIB_DANGER_RELEASE( ppParamTypes[nParams] );
            }
            TYPELIB_DANGER_RELEASE( pReturnType );

            uno_any_destruct(&aRet,
                reinterpret_cast< uno_ReleaseFunc >(cpp_release) );
            uno_type_copyAndConvertData(&aRet, pUnoExc, cppu::UnoType<Any>::get().getTypeLibType(),
                getReflection()->getUno2Cpp().get() );
            uno_any_destruct( pUnoExc, nullptr );
            throw InvocationTargetException("exception occurred during invocation!",
                                            *o3tl::doAccess<Reference<XInterface>>(rObj), aRet);
        }
        else
        {
            // reconvert arguments and cleanup
            while (nParams--)
            {
                if (pParams[nParams].bOut) // write back
                {
                    uno_any_destruct(
                        &pCppArgs[nParams],
                        reinterpret_cast< uno_ReleaseFunc >(cpp_release) );
                    uno_any_constructAndConvert(
                        &pCppArgs[nParams], ppUnoArgs[nParams], ppParamTypes[nParams],
                        getReflection()->getUno2Cpp().get() );
                }
                uno_destructData( ppUnoArgs[nParams], ppParamTypes[nParams], nullptr );
                TYPELIB_DANGER_RELEASE( ppParamTypes[nParams] );
            }
            uno_any_destruct(
                &aRet, reinterpret_cast< uno_ReleaseFunc >(cpp_release) );
            uno_any_constructAndConvert(
                &aRet, pUnoReturn, pReturnType,
                getReflection()->getUno2Cpp().get() );
            uno_destructData( pUnoReturn, pReturnType, nullptr );
            TYPELIB_DANGER_RELEASE( pReturnType );
        }
        return aRet;
    }
    throw IllegalArgumentException(
        "illegal destination object given!",
        getXWeak(), 0 );
}


InterfaceIdlClassImpl::~InterfaceIdlClassImpl()
{
    for ( sal_Int32 nPos = _nMethods + _nAttributes; nPos--; )
        typelib_typedescription_release( _pSortedMemberInit[nPos].second );
}


Sequence< Reference< XIdlClass > > InterfaceIdlClassImpl::getSuperclasses()
{
    ::osl::MutexGuard aGuard(getMutexAccess());
    if (!_xSuperClasses.hasElements()) {
        typelib_InterfaceTypeDescription * pType = getTypeDescr();
        _xSuperClasses.realloc(pType->nBaseTypes);
        auto pSuperClasses = _xSuperClasses.getArray();
        for (sal_Int32 i = 0; i < pType->nBaseTypes; ++i) {
            pSuperClasses[i] = getReflection()->forType(
                &pType->ppBaseTypes[i]->aBase);
            OSL_ASSERT(_xSuperClasses[i].is());
        }
    }
    return _xSuperClasses;
}

void InterfaceIdlClassImpl::initMembers()
{
    sal_Int32 nAll = getTypeDescr()->nAllMembers;
    std::unique_ptr<MemberInit[]> pSortedMemberInit(new MemberInit[nAll]);
    typelib_TypeDescriptionReference ** ppAllMembers = getTypeDescr()->ppAllMembers;

    for ( sal_Int32 nPos = 0; nPos < nAll; ++nPos )
    {
        sal_Int32 nIndex;
        if (ppAllMembers[nPos]->eTypeClass == typelib_TypeClass_INTERFACE_METHOD)
        {
            // methods to front
            nIndex = _nMethods;
            ++_nMethods;
        }
        else
        {
            ++_nAttributes;
            nIndex = (nAll - _nAttributes);
            // attributes at the back
        }

        typelib_TypeDescription * pTD = nullptr;
        typelib_typedescriptionreference_getDescription( &pTD, ppAllMembers[nPos] );
        assert(pTD && "### cannot get type description!");
        pSortedMemberInit[nIndex].first = reinterpret_cast<typelib_InterfaceMemberTypeDescription *>(pTD)->pMemberName;
        pSortedMemberInit[nIndex].second = pTD;
    }

    _pSortedMemberInit = std::move(pSortedMemberInit);
}

sal_Bool InterfaceIdlClassImpl::isAssignableFrom( const Reference< XIdlClass > & xType )
{
    if (xType.is() && xType->getTypeClass() == TypeClass_INTERFACE)
    {
        if (equals( xType ))
            return true;
        else
        {
            const Sequence< Reference< XIdlClass > > & rSeq = xType->getSuperclasses();
            if (std::any_of(rSeq.begin(), rSeq.end(),
                    [this](const Reference<XIdlClass>& rType){ return isAssignableFrom(rType); }))
                return true;
        }
    }
    return false;
}

Uik InterfaceIdlClassImpl::getUik()
{
    return Uik(0, 0, 0, 0, 0);
        // Uiks are deprecated and this function must not be called
}

Sequence< Reference< XIdlMethod > > InterfaceIdlClassImpl::getMethods()
{
    ::osl::MutexGuard aGuard( getMutexAccess() );
    if (! _pSortedMemberInit)
        initMembers();

    // create methods sequence
    Sequence< Reference< XIdlMethod > > aRet( _nMethods );
    Reference< XIdlMethod > * pRet = aRet.getArray();
    for ( sal_Int32 nPos = _nMethods; nPos--; )
    {

        /*_aName2Method[_pSortedMemberInit[nPos].first] = */pRet[nPos] = new IdlInterfaceMethodImpl(
            getReflection(), _pSortedMemberInit[nPos].first,
            _pSortedMemberInit[nPos].second, IdlClassImpl::getTypeDescr() );
    }
    return aRet;
}

Sequence< Reference< XIdlField > > InterfaceIdlClassImpl::getFields()
{
    ::osl::MutexGuard aGuard( getMutexAccess() );
    if (! _pSortedMemberInit)
        initMembers();

    // create fields sequence
    Sequence< Reference< XIdlField > > aRet( _nAttributes );
    Reference< XIdlField > * pRet = aRet.getArray();
    for ( sal_Int32 nPos = _nAttributes; nPos--; )
    {
        /*_aName2Field[_pSortedMemberInit[_nMethods+nPos].first] = */pRet[_nAttributes-nPos-1] =
            new IdlAttributeFieldImpl(
                getReflection(), _pSortedMemberInit[_nMethods+nPos].first,
                _pSortedMemberInit[_nMethods+nPos].second, IdlClassImpl::getTypeDescr() );
    }
    return aRet;
}

Reference< XIdlMethod > InterfaceIdlClassImpl::getMethod( const OUString & rName )
{
    ::osl::MutexGuard aGuard( getMutexAccess() );
    if (! _pSortedMemberInit)
        initMembers();

    Reference< XIdlMethod > xRet;

    // try weak map
    const OUString2Method::const_iterator iFind( _aName2Method.find( rName ) );
    if (iFind != _aName2Method.end())
        xRet = (*iFind).second; // harden ref

    if (! xRet.is())
    {
        for ( sal_Int32 nPos = _nMethods; nPos--; )
        {
            if (_pSortedMemberInit[nPos].first == rName)
            {
                _aName2Method[rName] = xRet = new IdlInterfaceMethodImpl(
                    getReflection(), rName,
                    _pSortedMemberInit[nPos].second, IdlClassImpl::getTypeDescr() );
                break;
            }
        }
    }
    return xRet;
}

Reference< XIdlField > InterfaceIdlClassImpl::getField( const OUString & rName )
{
    ::osl::MutexGuard aGuard( getMutexAccess() );
    if (! _pSortedMemberInit)
        initMembers();

    Reference< XIdlField > xRet;

    // try weak map
    const OUString2Field::const_iterator iFind( _aName2Field.find( rName ) );
    if (iFind != _aName2Field.end())
        xRet = (*iFind).second; // harden ref

    if (! xRet.is())
    {
        for ( sal_Int32 nPos = _nAttributes; nPos--; )
        {
            if (_pSortedMemberInit[_nMethods+nPos].first == rName)
            {
                _aName2Field[rName] = xRet = new IdlAttributeFieldImpl(
                    getReflection(), rName,
                    _pSortedMemberInit[_nMethods+nPos].second, IdlClassImpl::getTypeDescr() );
                break;
            }
        }
    }
    return xRet;
}

void InterfaceIdlClassImpl::createObject( Any & rObj )
{
    // interfaces cannot be constructed
    rObj.clear();
}

}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
