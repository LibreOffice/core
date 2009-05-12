/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ole2uno.hxx,v $
 * $Revision: 1.13 $
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

#ifndef _OLE2UNO_HXX
#define _OLE2UNO_HXX


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define STRICT
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif

#define _WIN32_DCOM
#if OSL_DEBUG_LEVEL > 0
//#define _ATL_DEBUG_INTERFACES
#endif

#pragma warning (push,1)
#pragma warning (disable:4917)
#pragma warning (disable:4005)
#pragma warning (disable:4548)

#include <tools/prewin.h>
#include <tchar.h>
#if (_MSC_VER >= 1200) || defined(__MINGW32__)
#include <dispex.h>
#endif
#include <tools/postwin.h>

#include <tools/presys.h>
#include <list>
#include <tools/postsys.h>

#pragma warning (pop)
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/script/XInvocation.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/bridge/XBridgeSupplier2.hpp>
#include <com/sun/star/bridge/ModelDependent.hpp>
#include <com/sun/star/reflection/InvocationTargetException.hpp>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/beans/UnknownPropertyException.hpp>
#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <cppuhelper/factory.hxx>
#include <sal/types.h>
#include <typelib/typeclass.h>
#include <osl/diagnose.h>
#include <osl/mutex.hxx>
#include <com/sun/star/uno/Reference.h>
#include <rtl/process.h>
#include <rtl/uuid.h>

#define UNO_2_OLE_EXCEPTIONCODE 1001
#define OUSTR(x) ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(x) )
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::script;
using namespace com::sun::star::registry;
using namespace com::sun::star::reflection;
using namespace com::sun::star::beans;
using namespace osl;
using namespace rtl;
using namespace std;



namespace ole_adapter
{

const VARTYPE getVarType( const Any& val);
/* creates a Type object for a given type name.

    The function returns false if the name does not represent
    a valid type.
*/
bool getType( BSTR name, Type & type);
void o2u_attachCurrentThread();

struct equalOUString_Impl
{
  bool operator()(const OUString & s1, const OUString & s2) const
  {
    return s1 == s2;
  }
};

struct hashOUString_Impl
{
    size_t operator()(const OUString & rName) const
    {
        return rName.hashCode();
    }
};


class BridgeRuntimeError
{
public:
    BridgeRuntimeError(const OUString& sMessage)
    {
        message = sMessage;
    }
    OUString message;
};


Mutex* getBridgeMutex();

} // end namespace



#endif // _OLE2UNO_HXX

