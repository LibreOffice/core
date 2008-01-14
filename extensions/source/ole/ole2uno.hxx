/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ole2uno.hxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: ihi $ $Date: 2008-01-14 14:46:30 $
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

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_XINVOCATION_HPP_
#include <com/sun/star/script/XInvocation.hpp>
#endif
#ifndef _COM_SUN_STAR_REGISTRY_XREGISTRYKEY_HPP_
#include <com/sun/star/registry/XRegistryKey.hpp>
#endif
#ifndef _COM_SUN_STAR_BRIDGE_XBRIDGESUPPLIER2_HPP_
#include <com/sun/star/bridge/XBridgeSupplier2.hpp>
#endif
#ifndef _COM_SUN_STAR_BRIDGE_MODELDEPENDENT_HPP_
#include <com/sun/star/bridge/ModelDependent.hpp>
#endif
#ifndef _COM_SUN_STAR_REFLECTION_INVOCATIONTARGETEXCEPTION_HPP_
#include <com/sun/star/reflection/InvocationTargetException.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_EXCEPTION_HPP_
#include <com/sun/star/uno/Exception.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_UNKNOWNPROPERTYEXCEPTION_HPP_
#include <com/sun/star/beans/UnknownPropertyException.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _CPPUHELPER_FACTORY_HXX_
#include <cppuhelper/factory.hxx>
#endif

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _TYPELIB_TYPECLASS_H_
#include <typelib/typeclass.h>
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif

#ifndef _RTL_PROCESS_H_
#include <rtl/process.h>
#endif

#ifndef _RTL_UUID_H_
#include <rtl/uuid.h>
#endif

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

