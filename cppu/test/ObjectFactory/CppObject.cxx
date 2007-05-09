/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: CppObject.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2007-05-09 13:43:47 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

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
