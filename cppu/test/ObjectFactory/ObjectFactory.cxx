/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ObjectFactory.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2007-05-09 13:44:08 $
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

