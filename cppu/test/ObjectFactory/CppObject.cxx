/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: CppObject.cxx,v $
 * $Revision: 1.4 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_cppu.hxx"

#include "cppu/EnvDcp.hxx"
#include "cppuhelper/implbase1.hxx"

#include "com/sun/star/uno/XComponentContext.hpp"

#include "CppObject.hxx"


using namespace ::com::sun::star;


#ifdef LOG_LIFECYCLE
#define LOG_LIFECYCLE_CppObject
#endif

#define LOG_LIFECYCLE_CppObject
#ifdef LOG_LIFECYCLE_CppObject
#  include <iostream>
#  define LOG_LIFECYCLE_CppObject_emit(x) x

#else
#  define LOG_LIFECYCLE_CppObject_emit(x)

#endif

CppObject::CppObject(Callee * pCallee) SAL_THROW((uno::RuntimeException))
    : m_nRef   (1),
      m_pCallee(pCallee)
{
    LOG_LIFECYCLE_CppObject_emit(fprintf(stderr, "LIFE: %s -> %p\n", "CppObject::CppObject", this));
}

CppObject::~CppObject() SAL_THROW((uno::RuntimeException))
{
    LOG_LIFECYCLE_CppObject_emit(fprintf(stderr, "LIFE: %s -> %p\n", "CppObject::~CppObject", this));
}

void SAL_CALL CppObject::acquire() throw ()
{
    m_pCallee(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.uno.XInterface::acquire")).pData);

    osl_incrementInterlockedCount(&m_nRef);
}

void SAL_CALL CppObject::release() throw ()
{
    m_pCallee(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.uno.XInterface::release")).pData);

    if (osl_decrementInterlockedCount(&m_nRef) == 0)
        delete this;
}

uno::Any SAL_CALL CppObject::queryInterface(uno::Type const & rType ) throw (uno::RuntimeException)
{
    m_pCallee(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.uno.XInterface::queryInterface")).pData);

    void * bla = this;

    return uno::Any(&bla, rType);
}


uno::XInterface * CppObject::s_create(Callee * pCallee)
{
    return new CppObject(pCallee);
}

void CppObject::s_call(uno::XInterface * pXInterface)
{
    uno::Reference<uno::XInterface> tmp;

    pXInterface->queryInterface(::getCppuType(&tmp));
}
