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

#include <cppuhelper/queryinterface.hxx>
#include <uno/any2.h>

#include "base.hxx"

using namespace css::reflection;
using namespace css::uno;

namespace stoc_corefl
{

#ifdef TEST_LIST_CLASSES
ClassNameList g_aClassNames;
#endif


::osl::Mutex & getMutexAccess()
{
    static ::osl::Mutex * s_pMutex = nullptr;
    if (! s_pMutex)
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        if (! s_pMutex)
        {
            static ::osl::Mutex s_aMutex;
            s_pMutex = &s_aMutex;
        }
    }
    return *s_pMutex;
}


IdlClassImpl::IdlClassImpl( IdlReflectionServiceImpl * pReflection,
                            const OUString & rName, typelib_TypeClass eTypeClass,
                            typelib_TypeDescription * pTypeDescr )
    : _pReflection( pReflection )
    , _aName( rName )
    , _eTypeClass( (TypeClass)eTypeClass )
    , _pTypeDescr( pTypeDescr )
{
    if (_pReflection)
        _pReflection->acquire();
    if (_pTypeDescr)
    {
        typelib_typedescription_acquire( _pTypeDescr );
        if (! _pTypeDescr->bComplete)
            typelib_typedescription_complete( &_pTypeDescr );
    }

#ifdef TEST_LIST_CLASSES
    ClassNameList::const_iterator iFind( std::find( g_aClassNames.begin(), g_aClassNames.end(), _aName ) );
    OSL_ENSURE( iFind == g_aClassNames.end(), "### idl class already exists!" );
    g_aClassNames.push_front( _aName );
#endif
}

IdlClassImpl::~IdlClassImpl()
{
    if (_pTypeDescr)
        typelib_typedescription_release( _pTypeDescr );
    if (_pReflection)
        _pReflection->release();

#ifdef TEST_LIST_CLASSES
    ClassNameList::iterator iFind( std::find( g_aClassNames.begin(), g_aClassNames.end(), _aName ) );
    OSL_ENSURE( iFind != g_aClassNames.end(), "### idl class does not exist!" );
    g_aClassNames.erase( iFind );
#endif
}

// XIdlClassImpl default implementation

TypeClass IdlClassImpl::getTypeClass()
    throw(css::uno::RuntimeException, std::exception)
{
    return _eTypeClass;
}

OUString IdlClassImpl::getName()
    throw(css::uno::RuntimeException, std::exception)
{
    return _aName;
}

sal_Bool IdlClassImpl::equals( const Reference< XIdlClass >& xType )
    throw(css::uno::RuntimeException, std::exception)
{
    return (xType.is() &&
            (xType->getTypeClass() == _eTypeClass) && (xType->getName() == _aName));
}

static const sal_Bool s_aAssignableFromTab[11][11] =
{
                         /* from CH,BO,BY,SH,US,LO,UL,HY,UH,FL,DO */
/* TypeClass_CHAR */            { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
/* TypeClass_BOOLEAN */         { 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
/* TypeClass_BYTE */            { 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 },
/* TypeClass_SHORT */           { 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0 },
/* TypeClass_UNSIGNED_SHORT */  { 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0 },
/* TypeClass_LONG */            { 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0 },
/* TypeClass_UNSIGNED_LONG */   { 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0 },
/* TypeClass_HYPER */           { 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0 },
/* TypeClass_UNSIGNED_HYPER */  { 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0 },
/* TypeClass_FLOAT */           { 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0 },
/* TypeClass_DOUBLE */          { 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1 }
};

sal_Bool IdlClassImpl::isAssignableFrom( const Reference< XIdlClass > & xType )
    throw(css::uno::RuntimeException, std::exception)
{
    TypeClass eAssign = getTypeClass();
    if (equals( xType ) || eAssign == TypeClass_ANY) // default shot
    {
        return sal_True;
    }
    else
    {
        TypeClass eFrom   = xType->getTypeClass();
        if (eAssign > TypeClass_VOID && eAssign < TypeClass_STRING &&
            eFrom > TypeClass_VOID && eFrom < TypeClass_STRING)
        {
            return s_aAssignableFromTab[eAssign-1][eFrom-1];
        }
    }
    return sal_False;
}

void IdlClassImpl::createObject( Any & rObj )
    throw(css::uno::RuntimeException, std::exception)
{
    rObj.clear();
    uno_any_destruct( &rObj, reinterpret_cast< uno_ReleaseFunc >(cpp_release) );
    uno_any_construct( &rObj, nullptr, getTypeDescr(), nullptr );
}

// what TODO ????

Sequence< Reference< XIdlClass > > IdlClassImpl::getClasses()
    throw(css::uno::RuntimeException, std::exception)
{
    OSL_FAIL( "### unexpected use!" );
    return Sequence< Reference< XIdlClass > >();
}

Reference< XIdlClass > IdlClassImpl::getClass( const OUString & )
    throw(css::uno::RuntimeException, std::exception)
{
    OSL_FAIL( "### unexpected use!" );
    return Reference< XIdlClass >();
}

Sequence< Reference< XIdlClass > > IdlClassImpl::getInterfaces()
    throw(css::uno::RuntimeException, std::exception)
{
//      OSL_FAIL( "### unexpected use!" );
    return Sequence< Reference< XIdlClass > >();
}

// structs, interfaces

Sequence< Reference< XIdlClass > > IdlClassImpl::getSuperclasses() throw(css::uno::RuntimeException, std::exception)
{
    return Sequence< Reference< XIdlClass > >();
}
// structs

Reference< XIdlField > IdlClassImpl::getField( const OUString & )
    throw(css::uno::RuntimeException, std::exception)
{
    return Reference< XIdlField >();
}

Sequence< Reference< XIdlField > > IdlClassImpl::getFields()
    throw(css::uno::RuntimeException, std::exception)
{
    return Sequence< Reference< XIdlField > >();
}
// interfaces

Uik IdlClassImpl::getUik()
    throw(css::uno::RuntimeException, std::exception)
{
    return Uik();
}

Reference< XIdlMethod > IdlClassImpl::getMethod( const OUString & )
    throw(css::uno::RuntimeException, std::exception)
{
    return Reference< XIdlMethod >();
}

Sequence< Reference< XIdlMethod > > IdlClassImpl::getMethods()
    throw(css::uno::RuntimeException, std::exception)
{
    return Sequence< Reference< XIdlMethod > >();
}
// array

Reference< XIdlClass > IdlClassImpl::getComponentType()
    throw(css::uno::RuntimeException, std::exception)
{
    return Reference< XIdlClass >();
}

Reference< XIdlArray > IdlClassImpl::getArray()
    throw(css::uno::RuntimeException, std::exception)
{
    return Reference< XIdlArray >();
}








IdlMemberImpl::IdlMemberImpl( IdlReflectionServiceImpl * pReflection, const OUString & rName,
                              typelib_TypeDescription * pTypeDescr,
                              typelib_TypeDescription * pDeclTypeDescr )
    : _pReflection( pReflection )
    , _aName( rName )
    , _pTypeDescr( pTypeDescr )
    , _pDeclTypeDescr( pDeclTypeDescr )
{
    _pReflection->acquire();
    typelib_typedescription_acquire( _pTypeDescr );
    if (! _pTypeDescr->bComplete)
        typelib_typedescription_complete( &_pTypeDescr );
    typelib_typedescription_acquire( _pDeclTypeDescr );
    if (! _pDeclTypeDescr->bComplete)
        typelib_typedescription_complete( &_pDeclTypeDescr );
}

IdlMemberImpl::~IdlMemberImpl()
{
    typelib_typedescription_release( _pDeclTypeDescr );
    typelib_typedescription_release( _pTypeDescr );
    _pReflection->release();
}

// XIdlMember

Reference< XIdlClass > IdlMemberImpl::getDeclaringClass()
    throw(css::uno::RuntimeException, std::exception)
{
    if (! _xDeclClass.is())
    {
        Reference< XIdlClass > xDeclClass( getReflection()->forType( getDeclTypeDescr() ) );
        ::osl::MutexGuard aGuard( getMutexAccess() );
        if (! _xDeclClass.is())
            _xDeclClass = xDeclClass;
    }
    return _xDeclClass;
}

OUString IdlMemberImpl::getName()
    throw(css::uno::RuntimeException, std::exception)
{
    return _aName;
}

}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
