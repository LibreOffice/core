/*************************************************************************
 *
 *  $RCSfile: ole2uno.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:16:40 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
#ifdef _DEBUG
//#define _ATL_DEBUG_INTERFACES
#endif

//#include <atlbase.h>
//You may derive a class from CComModule and use it if you want to override
//something, but do not change the name of _Module

#include <tools/prewin.h>
//#include <stdlib.h>
//#include <malloc.h>
//#include <ole2.h>
//#include <oleauto.h>
//#include <oaidl.h>
//#include <ocidl.h>
#include <tchar.h>
//#include <objbase.h>
//#include <atlbase.h>
//extern CComModule _Module;
//#include <atlcom.h>
#if _MSC_VER >= 1200
#include <dispex.h>
//#include <winbase.h>
#endif
#include <tools/postwin.h>

#include <tools/presys.h>
#include <list>
#include <hash_map>
#include <tools/postsys.h>

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_XTYPECONVERTER_HPP_
#include <com/sun/star/script/XTypeConverter.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_FINISHENGINEEVENT_HPP_
#include <com/sun/star/script/FinishEngineEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_INTERRUPTREASON_HPP_
#include <com/sun/star/script/InterruptReason.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_XENGINELISTENER_HPP_
#include <com/sun/star/script/XEngineListener.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_XDEBUGGING_HPP__
#include <com/sun/star/script/XDebugging.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_XINVOCATION_HPP_
#include <com/sun/star/script/XInvocation.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_CONTEXTINFORMATION_HPP_
#include <com/sun/star/script/ContextInformation.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_FINISHREASON_HPP_
#include <com/sun/star/script/FinishReason.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_XENGINE_HPP_
#include <com/sun/star/script/XEngine.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_INTERRUPTENGINEEVENT_HPP_
#include <com/sun/star/script/InterruptEngineEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_XLIBRARYACCESS_HXX_
#include <com/sun/star/script/XLibraryAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_REGISTRY_INVALIDVALUEEXCEPTION_HPP_
#include <com/sun/star/registry/InvalidValueException.hpp>
#endif
#ifndef _COM_SUN_STAR_REGISTRY_REGISTRYKEYTYPE_HPP_
#include <com/sun/star/registry/RegistryKeyType.hpp>
#endif
#ifndef _COM_SUN_STAR_REGISTRY_XREGISTRYKEY_HPP_
#include <com/sun/star/registry/XRegistryKey.hpp>
#endif
#ifndef _COM_SUN_STAR_REGISTRY_REGISTRYVALUETYPE_HPP_
#include <com/sun/star/registry/RegistryValueType.hpp>
#endif
#ifndef _COM_SUN_STAR_REGISTRY_INVALIDREGISTRYEXCEPTION_HPP_
#include <com/sun/star/registry/InvalidRegistryException.hpp>
#endif
#ifndef _COM_SUN_STAR_LOADER_XIMPLEMENTATIONLOADER_HPP_
#include <com/sun/star/loader/XImplementationLoader.hpp>
#endif
#ifndef _COM_SUN_STAR_LOADER_CANNOTACTIVATEFACTORYEXCEPTION_HPP_
#include <com/sun/star/loader/CannotActivateFactoryException.hpp>
#endif
#ifndef _COM_SUN_STAR_BRIDGE_XBRIDGESUPPLIER_HPP_
#include <com/sun/star/bridge/XBridgeSupplier.hpp>
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
#ifndef _COM_SUN_STAR_LANG_XUNOTUNNEL_HPP_
#include <com/sun/star/lang/XUnoTunnel.hpp>
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

#ifndef _UNO_DISPATCHER_H_
#include <uno/dispatcher.h>
#endif

#ifndef _UNO_MAPPING_H_
#include <uno/mapping.hxx>
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
// function shared in this component
Reference<XSingleServiceFactory>    o2u_getConverterProvider2(const Reference<XMultiServiceFactory>& xMan, const Reference<XRegistryKey>& xKey);
Reference<XSingleServiceFactory>    o2u_getConverterProviderVar1(const Reference<XMultiServiceFactory>& xMan, const Reference<XRegistryKey>& xKey);
Reference<XSingleServiceFactory>    o2u_getClientProvider(const Reference<XMultiServiceFactory>& xMan, const Reference<XRegistryKey>& xKey);
Reference<XSingleServiceFactory>    o2u_getServerProvider(const Reference<XMultiServiceFactory>& xMan, const Reference<XRegistryKey>& xKey);

Reference<XMultiServiceFactory> o2u_getMultiServiceFactory();
Reference<XRegistryKey>     o2u_getRegistryKey();
const VARTYPE getVarType( const Any& val);
Type getType( BSTR type);


Uik                 o2u_uikFromGUID(GUID* pGuid);
void                o2u_attachCurrentThread();

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




// ask the object for XBridgeSupplier2 and on success bridges
// the uno object to IDispatch.
template < class T >
sal_Bool convertSelfToIDispatch( T& unoInterface, IDispatch** ppDisp)
{
    OSL_ASSERT( ppDisp);
    *ppDisp= NULL;

    Reference< XInterface > xInt( unoInterface, UNO_QUERY);
    if( xInt.is())
    {
        Reference< XBridgeSupplier2 > xSupplier( xInt, UNO_QUERY);
        if( xSupplier.is())
        {
            sal_Int8 arId[16];
            rtl_getGlobalProcessId( (sal_uInt8*)arId);
            Sequence<sal_Int8> seqId( arId, 16);
            Any anySource;
            anySource <<= xInt;
            Any anyDisp=    xSupplier->createBridge( anySource, seqId, UNO, OLE);
            if( anyDisp.getValueTypeClass() == TypeClass_UNSIGNED_LONG)
            {
                VARIANT* pvar= *(VARIANT**)anyDisp.getValue();
                if( pvar->vt == VT_DISPATCH)
                {
                    *ppDisp= pvar->pdispVal;
                    (*ppDisp)->AddRef();
                }
                VariantClear( pvar);
                CoTaskMemFree( pvar);
            }
        }
    }
    return *ppDisp ? sal_True : sal_False;
}
// ----------------------------------------------------------------------
// see the overloaded getCppuType function in this file
//class IUnknownWrapper: public XInterface
//{
//public:
//
//    virtual Any invokeWithDispId(DISPID dispID, const Sequence< Any >& Params, Sequence< sal_Int16 >& OutParamIndex, Sequence< Any >& OutParam) throw ( IllegalArgumentException, CannotConvertException, InvocationTargetException, Exception ) = 0;
//    virtual void setValueWithDispId(DISPID dispID, const Any& Value) throw( UnknownPropertyException, CannotConvertException, InvocationTargetException, Exception)  = 0;
//    virtual Any getValueWithDispId(DISPID dispID) throw( UnknownPropertyException, Exception) = 0;
//};
//-----------------------------------------------------------------------



/* ref class to hold IUnknown pointer
*/
template <class UnknownClass> class OleRef
{
public:

    typedef OleRef<UnknownClass> self;

    OleRef()
        : m_pUnknown(NULL)
    {}

    OleRef(const self& copy)
        : m_pUnknown(copy.m_pUnknown)
    {
        if (m_pUnknown) m_pUnknown->AddRef();
    }

    OleRef(UnknownClass* pUnknown)
        : m_pUnknown(pUnknown)
    {
        if (m_pUnknown) m_pUnknown->AddRef();
    }

    ~OleRef()
    {
        if (m_pUnknown) m_pUnknown->Release();
    }

    self& operator = (const self& copy)
    {
        if (m_pUnknown) m_pUnknown->Release();
        m_pUnknown = copy.m_pUnknown;
        if (m_pUnknown) m_pUnknown->AddRef();

        return *this;
    }

    self& operator = (UnknownClass* pUnknown)
    {
        if (m_pUnknown) m_pUnknown->Release();
        m_pUnknown = pUnknown;
        if (m_pUnknown) m_pUnknown->AddRef();
        return *this;
    }

    UnknownClass* get() { return m_pUnknown; }

    UnknownClass* operator->() { return m_pUnknown; }

    const UnknownClass* operator->() const { return m_pUnknown; }

    Boolean is() { return (m_pUnknown != NULL); }

protected:

    UnknownClass* m_pUnknown;
};

inline sal_Bool operator == (const Uik & uik1, const Uik & uik2)
{
    if( ! memcmp( &uik1, &uik2, sizeof( Uik)))
        return sal_True;
    else
        return sal_False;
}

} // end namespace



#endif _OLE2UNO_HXX

