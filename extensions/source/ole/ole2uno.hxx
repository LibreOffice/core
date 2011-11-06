/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

