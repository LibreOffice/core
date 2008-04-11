/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ObjectFactory.cxx,v $
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

#define CPPU_test_ObjectFactory_IMPL

#include "UnoObject.hxx"
#include "CppObject.hxx"
#include "ObjectFactory.hxx"


using namespace com::sun::star;



CPPU_test_ObjectFactory_EXPORT void * createObject(rtl::OUString const & envDcp, Callee * pCallee)
{
    void * result;

    if (envDcp.match(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(CPPU_STRINGIFY(CPPU_ENV)))))
        result = CppObject::s_create(pCallee);

    else if (envDcp.match(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(UNO_LB_UNO))))
        result = UnoObject_create(pCallee);

    else
        abort();

    return result;
}

CPPU_test_ObjectFactory_EXPORT void callObject(rtl::OUString const & envDcp, void * pObject)
{
    if (envDcp.match(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(CPPU_STRINGIFY(CPPU_ENV)))))
        CppObject::s_call(reinterpret_cast<uno::XInterface *>(pObject));

    else if (envDcp.match(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(UNO_LB_UNO))))
        UnoObject_call(reinterpret_cast<uno_Interface *>(pObject));

    else
        abort();
}

