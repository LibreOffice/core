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
ClassNameVector g_aClassNames;
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
    : m_xReflection( pReflection )
    , _aName( rName )
    , _eTypeClass( static_cast<TypeClass>(eTypeClass) )
    , _pTypeDescr( pTypeDescr )
{
    if (_pTypeDescr)
    {
        typelib_typedescription_acquire( _pTypeDescr );
        if (! _pTypeDescr->bComplete)
            typelib_typedescription_complete( &_pTypeDescr );
    }

#ifdef TEST_LIST_CLASSES
    ClassNameVector::const_iterator iFind( std::find( g_aClassNames.begin(), g_aClassNames.end(), _aName ) );
    OSL_ENSURE( iFind == g_aClassNames.end(), "### idl class already exists!" );
    g_aClassNames.insert(g_aClassNames.begin(), _aName);
#endif
}

IdlClassImpl::~IdlClassImpl()
{
    if (_pTypeDescr)
        typelib_typedescription_release( _pTypeDescr );
    m_xReflection.clear();

#ifdef TEST_LIST_CLASSES
    ClassNameVector::iterator iFind( std::find( g_aClassNames.begin(), g_aClassNames.end(), _aName ) );
    OSL_ENSURE( iFind != g_aClassNames.end(), "### idl class does not exist!" );
    g_aClassNames.erase( iFind );
#endif
}

// XIdlClassImpl default implementation

TypeClass IdlClassImpl::getTypeClass()
{
    return _eTypeClass;
}

OUString IdlClassImpl::getName()
{
    return _aName;
}

sal_Bool IdlClassImpl::equals( const Reference< XIdlClass >& xType )
{
    return (xType.is() &&
            (xType->getTypeClass() == _eTypeClass) && (xType->getName() == _aName));
}

static const bool s_aAssignableFromTab[11][11] =
{
                         /* from  CH,    BO,    BY,    SH,    US,    LO,    UL,    HY,    UH,    FL,    DO */
/* TypeClass_CHAR */            { true,  false, false, false, false, false, false, false, false, false, false },
/* TypeClass_BOOLEAN */         { false, true,  false, false, false, false, false, false, false, false, false },
/* TypeClass_BYTE */            { false, false, true,  false, false, false, false, false, false, false, false },
/* TypeClass_SHORT */           { false, false, true,  true,  true,  false, false, false, false, false, false },
/* TypeClass_UNSIGNED_SHORT */  { false, false, true,  true,  true,  false, false, false, false, false, false },
/* TypeClass_LONG */            { false, false, true,  true,  true,  true,  true,  false, false, false, false },
/* TypeClass_UNSIGNED_LONG */   { false, false, true,  true,  true,  true,  true,  false, false, false, false },
/* TypeClass_HYPER */           { false, false, true,  true,  true,  true,  true,  true,  true,  false, false },
/* TypeClass_UNSIGNED_HYPER */  { false, false, true,  true,  true,  true,  true,  true,  true,  false, false },
/* TypeClass_FLOAT */           { false, false, true,  true,  true,  true,  true,  true,  true,  true,  false },
/* TypeClass_DOUBLE */          { false, false, true,  true,  true,  true,  true,  true,  true,  true,  true  }
};

sal_Bool IdlClassImpl::isAssignableFrom( const Reference< XIdlClass > & xType )
{
    TypeClass eAssign = getTypeClass();
    if (equals( xType ) || eAssign == TypeClass_ANY) // default shot
    {
        return true;
    }
    else
    {
        TypeClass eFrom   = xType->getTypeClass();
        if (eAssign > TypeClass_VOID && eAssign < TypeClass_STRING &&
            eFrom > TypeClass_VOID && eFrom < TypeClass_STRING)
        {
            return s_aAssignableFromTab[static_cast<int>(eAssign)-1][static_cast<int>(eFrom)-1];
        }
    }
    return false;
}

void IdlClassImpl::createObject( Any & rObj )
{
    rObj.clear();
    uno_any_destruct( &rObj, reinterpret_cast< uno_ReleaseFunc >(cpp_release) );
    uno_any_construct( &rObj, nullptr, getTypeDescr(), nullptr );
}

// what TODO ????

Sequence< Reference< XIdlClass > > IdlClassImpl::getClasses()
{
    OSL_FAIL( "### unexpected use!" );
    return Sequence< Reference< XIdlClass > >();
}

Reference< XIdlClass > IdlClassImpl::getClass( const OUString & )
{
    OSL_FAIL( "### unexpected use!" );
    return Reference< XIdlClass >();
}

Sequence< Reference< XIdlClass > > IdlClassImpl::getInterfaces()
{
//      OSL_FAIL( "### unexpected use!" );
    return Sequence< Reference< XIdlClass > >();
}

// structs, interfaces

Sequence< Reference< XIdlClass > > IdlClassImpl::getSuperclasses()
{
    return Sequence< Reference< XIdlClass > >();
}
// structs

Reference< XIdlField > IdlClassImpl::getField( const OUString & )
{
    return Reference< XIdlField >();
}

Sequence< Reference< XIdlField > > IdlClassImpl::getFields()
{
    return Sequence< Reference< XIdlField > >();
}
// interfaces

Uik IdlClassImpl::getUik()
{
    return Uik();
}

Reference< XIdlMethod > IdlClassImpl::getMethod( const OUString & )
{
    return Reference< XIdlMethod >();
}

Sequence< Reference< XIdlMethod > > IdlClassImpl::getMethods()
{
    return Sequence< Reference< XIdlMethod > >();
}
// array

Reference< XIdlClass > IdlClassImpl::getComponentType()
{
    return Reference< XIdlClass >();
}

Reference< XIdlArray > IdlClassImpl::getArray()
{
    return Reference< XIdlArray >();
}


IdlMemberImpl::IdlMemberImpl( IdlReflectionServiceImpl * pReflection, const OUString & rName,
                              typelib_TypeDescription * pTypeDescr,
                              typelib_TypeDescription * pDeclTypeDescr )
    : m_xReflection( pReflection )
    , _aName( rName )
    , _pTypeDescr( pTypeDescr )
    , _pDeclTypeDescr( pDeclTypeDescr )
{
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
}

// XIdlMember

Reference< XIdlClass > IdlMemberImpl::getDeclaringClass()
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
{
    return _aName;
}

}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
