/*************************************************************************
 *
 *  $RCSfile: unoobjw.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jl $ $Date: 2000-10-12 13:07:57 $
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

#ifndef _UNOOBJW_HXX
#define _UNOOBJW_HXX

#include <com/sun/star/bridge/XBridgeSupplier2.hpp>
#include <com/sun/star/beans/XExactName.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <vos/refernce.hxx>

#include <tools/presys.h>
#include "stdafx.h"
#include "comifaces.hxx"
#include <tools/postsys.h>

#include "ole2uno.hxx"
#include "unoconversionutilities.hxx"


using namespace std;
using namespace cppu;
using namespace com::sun::star::bridge;
namespace ole_adapter
{



struct hash_IUnknown_Impl
{
    size_t operator()(const IUnknown* p) const
    {
        return (size_t)p;
    }
};

struct equal_to_IUnknown_Impl
{
    bool operator()(const IUnknown* s1, const IUnknown* s2) const
    {
        return s1 == s2;
    }
};



struct MemberInfo
{
    MemberInfo() : flags(0), name() {}
    MemberInfo(WORD f, const OUString& n) : flags(f), name(n) {}

    WORD    flags;
    OUString name;
};

typedef hash_map
<
    OUString,
    DISPID,
    hashOUString_Impl,
    equalOUString_Impl
> NameToIdMap;

typedef hash_map
<
    OUString,
    sal_Bool,
    hashOUString_Impl,
    equalOUString_Impl
> BadNameMap;

typedef hash_map
<
    DISPID,
    MemberInfo
> IdToMemberInfoMap;

/*****************************************************************************

    class declaration: InterfaceOleWrapper_Impl

*****************************************************************************/

class InterfaceOleWrapper_Impl : public WeakImplHelper2<XBridgeSupplier2, XInitialization>,
                                 public IDispatchEx,
                                 public UnoConversionUtilities<InterfaceOleWrapper_Impl>,
                                 public IUnoObjectWrapper
{
public:


    InterfaceOleWrapper_Impl(Reference<XMultiServiceFactory>& xFactory, sal_uInt8 unoWrapperClass, sal_uInt8 comWrapperClass);
    ~InterfaceOleWrapper_Impl();

    /* IUnknown methods */
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID FAR * ppvObj);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

    /* IDispatch methods */
    STDMETHOD( GetTypeInfoCount )( unsigned int * pctinfo );
    STDMETHOD( GetTypeInfo )( unsigned int itinfo, LCID lcid, ITypeInfo ** pptinfo );
    STDMETHOD( GetIDsOfNames )( REFIID riid, OLECHAR ** rgszNames, unsigned int cNames,
                                LCID lcid, DISPID * rgdispid );
    STDMETHOD( Invoke )( DISPID dispidMember, REFIID riid, LCID lcid, unsigned short wFlags,
                         DISPPARAMS * pdispparams, VARIANT * pvarResult, EXCEPINFO * pexcepinfo,
                         unsigned int * puArgErr );

    /* IDispatchEx methods */

    virtual HRESULT STDMETHODCALLTYPE GetDispID(
        /* [in] */ BSTR bstrName,
        /* [in] */ DWORD grfdex,
        /* [out] */ DISPID __RPC_FAR *pid);

    virtual /* [local] */ HRESULT STDMETHODCALLTYPE InvokeEx(
        /* [in] */ DISPID id,
        /* [in] */ LCID lcid,
        /* [in] */ WORD wFlags,
        /* [in] */ DISPPARAMS __RPC_FAR *pdp,
        /* [out] */ VARIANT __RPC_FAR *pvarRes,
        /* [out] */ EXCEPINFO __RPC_FAR *pei,
        /* [unique][in] */ IServiceProvider __RPC_FAR *pspCaller);

    virtual HRESULT STDMETHODCALLTYPE DeleteMemberByName(
        /* [in] */ BSTR bstr,
        /* [in] */ DWORD grfdex);

    virtual HRESULT STDMETHODCALLTYPE DeleteMemberByDispID(
        /* [in] */ DISPID id);

    virtual HRESULT STDMETHODCALLTYPE GetMemberProperties(
        /* [in] */ DISPID id,
        /* [in] */ DWORD grfdexFetch,
        /* [out] */ DWORD __RPC_FAR *pgrfdex);

    virtual HRESULT STDMETHODCALLTYPE GetMemberName(
        /* [in] */ DISPID id,
        /* [out] */ BSTR __RPC_FAR *pbstrName);

    virtual HRESULT STDMETHODCALLTYPE GetNextDispID(
        /* [in] */ DWORD grfdex,
        /* [in] */ DISPID id,
        /* [out] */ DISPID __RPC_FAR *pid);

    virtual HRESULT STDMETHODCALLTYPE GetNameSpaceParent(
        /* [out] */ IUnknown __RPC_FAR *__RPC_FAR *ppunk);

    //XInterface ----------------------------------------------------------
    void SAL_CALL InterfaceOleWrapper_Impl::acquire(  ) throw();
    void SAL_CALL InterfaceOleWrapper_Impl::release(  ) throw();

    // XBridgeSupplier2 ---------------------------------------------------
    virtual Any SAL_CALL createBridge(const Any& modelDepObject,
                                const Sequence<sal_Int8>& ProcessId,
                                sal_Int16 sourceModelType,
                                sal_Int16 destModelType)
            throw (IllegalArgumentException, RuntimeException);

    //XInitialization -----------------------------------------------------
    virtual void SAL_CALL initialize( const Sequence< Any >& aArguments ) throw(Exception, RuntimeException);

    // IUnoObjectWrapper
    STDMETHOD( getWrapperXInterface)( Reference<XInterface>* pXInt);
    STDMETHOD( getOriginalUnoObject)( Reference<XInterface>* pXInt);
    STDMETHOD( getOriginalUnoStruct)( Any * pStruct);

    // UnoConversionUtility
    virtual Reference< XInterface > createUnoWrapperInstance();
    virtual Reference< XInterface > createComWrapperInstance();


protected:
    virtual HRESULT doInvoke( DISPPARAMS * pdispparams, VARIANT * pvarResult,
                              EXCEPINFO * pexcepinfo, unsigned int * puArgErr, OUString & name, Sequence<Any>& params);

    virtual HRESULT doGetProperty( DISPPARAMS * pdispparams, VARIANT * pvarResult,
                                        EXCEPINFO * pexcepinfo, OUString & name );

    virtual HRESULT doSetProperty( DISPPARAMS * pdispparams, VARIANT * pvarResult,
                                        EXCEPINFO * pexcepinfo, unsigned int * puArgErr, OUString & name, Sequence<Any> params);

    virtual HRESULT InvokeGeneral( DISPID dispidMember, unsigned short wFlags,
                         DISPPARAMS * pdispparams, VARIANT * pvarResult, EXCEPINFO * pexcepinfo,
                         unsigned int * puArgErr, sal_Bool& bHandled);

    sal_Bool convertVARIANTARGS( DISPID id, unsigned short wFlags, DISPPARAMS* pdispparams,
                            Sequence<Any>& rSeq);

    vos::ORefCount                      m_refCount;
    Reference<XInvocation>                  m_xInvocation;
    Reference<XExactName>                   m_xExactName;
    Reference<XInterface>                   m_xOrigin;
    NameToIdMap                     m_nameToDispIdMap;
    vector<MemberInfo>              m_MemberInfos;
    // This member is used to determine the default value
    // denoted by DISPID_VALUE (0). For proper results in JavaScript
    // we have to return the default value when we write an object
    // as out parameter. That is, we get an JScript Array as parameter
    // and put a wrapped object on index null. The array object tries
    // to detect the default value. The wrapped object must then return
    // its own IDispatch* otherwise we cannot access it within the script.
    // see InterfaceOleWrapper_Impl::Invoke
    VARTYPE                         m_defaultValueType;

};

/*****************************************************************************

    class declaration: UnoObjectWrapperRemoteOpt
    ( Uno Object Wrapper Remote Optimized)
    This is the UNO wrapper used in the service com.sun.star.bridge.OleBridgeSupplierVar1.
    Key features:
    DISPIDs are passed out blindly. That is in GetIDsOfNames is no name checking carried out.
    Only if Invoke fails the name is being checked. Moreover Invoke tries to figure out
    if a call is made to a property or method if the flags are DISPATCH_METHOD | DISPATCH_PROPERTYPUT.
    If something has been found out about a property or member than it is saved
    in a MemberInfo structure hold by a IdToMemberInfoMap stl map.

*****************************************************************************/
class UnoObjectWrapperRemoteOpt: public InterfaceOleWrapper_Impl
{
public:
    UnoObjectWrapperRemoteOpt( Reference<XMultiServiceFactory>& aFactory, sal_uInt8 unoWrapperClass, sal_uInt8 comWrapperClass);
    ~UnoObjectWrapperRemoteOpt();

    STDMETHOD( GetIDsOfNames )( REFIID riid, OLECHAR ** rgszNames, unsigned int cNames,
                                LCID lcid, DISPID * rgdispid );
    STDMETHOD( Invoke )( DISPID dispidMember, REFIID riid, LCID lcid, unsigned short wFlags,
                         DISPPARAMS * pdispparams, VARIANT * pvarResult, EXCEPINFO * pexcepinfo,
                         unsigned int * puArgErr );

    // UnoConversionUtility
    // If UNO interfaces are converted in methods of this class then
    // they are always wrapped with instances of this class
    virtual Reference< XInterface > createUnoWrapperInstance();

protected:

        HRESULT methodInvoke( DISPID dispidMember, DISPPARAMS * pdispparams, VARIANT * pvarResult,
                              EXCEPINFO * pexcepinfo, unsigned int * puArgErr, Sequence<Any> params);
    // In GetIDsOfNames are blindly passed out, that is without verifying
    // the name. If two names are passed in during different calls to
    // GetIDsOfNames and the names differ only in their cases then different
    // id's are passed out ( e.g. "doSomethingMethod" or "dosomethingmethod").
    // In Invoke the DISPID is remapped to the name passed to GetIDsOfNames
    // and the name is used as parameter for XInvocation::invoke. If invoke
    // fails because of a wrong name, then m_xExactName ( XExactName) is used
    // to verify the name. The correct name is then inserted to m_MemberInfos
    // ( vector<MemberInfo> ). During the next call to Invoke the right name
    // is used.         .


    BadNameMap m_badNameMap;

    IdToMemberInfoMap m_idToMemberInfoMap;

};



} // end namespace

#endif
