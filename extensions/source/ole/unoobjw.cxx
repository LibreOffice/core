/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

// Documentation pointers for recent work:
//
// https://www.codeproject.com/Articles/9014/Understanding-COM-Event-Handling
// https://blogs.msdn.microsoft.com/ericlippert/2005/02/15/why-does-wscript-connectobject-not-always-work/

#include "ole2uno.hxx"

#include <stdio.h>
#include <list>
#include <sstream>
#include <unordered_map>
#include <vector>

#if defined _MSC_VER && defined __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wall"
#pragma clang diagnostic ignored "-Wattributes"
#pragma clang diagnostic ignored "-Wdelete-incomplete"
#pragma clang diagnostic ignored "-Wdynamic-class-memaccess"
#pragma clang diagnostic ignored "-Wextra"
#pragma clang diagnostic ignored "-Wint-to-pointer-cast"
#pragma clang diagnostic ignored "-Winvalid-noreturn"
#pragma clang diagnostic ignored "-Wmicrosoft"
#pragma clang diagnostic ignored "-Wnon-pod-varargs"
#pragma clang diagnostic ignored "-Wnon-virtual-dtor"
#pragma clang diagnostic ignored "-Wnonportable-include-path"
#pragma clang diagnostic ignored "-Wsequence-point"
#pragma clang diagnostic ignored "-Wtypename-missing"
#endif
#include <atlbase.h>
#include <atlcom.h>
#if defined _MSC_VER && defined __clang__
#pragma clang diagnostic pop
#endif
#include <comdef.h>

#include <osl/diagnose.h>
#include <salhelper/simplereferenceobject.hxx>
#include <rtl/ustring.hxx>
#include <sal/log.hxx>
#include <com/sun/star/beans/MethodConcept.hpp>
#include <com/sun/star/beans/PropertyConcept.hpp>
#include <com/sun/star/lang/NoSuchMethodException.hpp>
#include <com/sun/star/script/CannotConvertException.hpp>
#include <com/sun/star/script/FailReason.hpp>
#include <com/sun/star/reflection/theCoreReflection.hpp>
#include <com/sun/star/reflection/ParamInfo.hpp>
#include <com/sun/star/beans/XExactName.hpp>
#include <com/sun/star/container/NoSuchElementException.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>

#include <com/sun/star/beans/XMaterialHolder.hpp>
#include <com/sun/star/script/XInvocation2.hpp>
#include <com/sun/star/script/MemberType.hpp>
#include <com/sun/star/reflection/XIdlReflection.hpp>
#include <ooo/vba/XCollection.hpp>
#include <ooo/vba/XConnectable.hpp>
#include <ooo/vba/XConnectionPoint.hpp>
#include <ooo/vba/XSink.hpp>
#include <ooo/vba/msforms/XCheckBox.hpp>
#include <osl/interlck.h>
#include <com/sun/star/uno/genfunc.h>
#include <comphelper/automationinvokedzone.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/profilezone.hxx>
#include <comphelper/windowsdebugoutput.hxx>
#include <comphelper/windowserrorstring.hxx>
#include <o3tl/char16_t2wchar_t.hxx>

#include "comifaces.hxx"
#include "jscriptclasses.hxx"
#include "unotypewrapper.hxx"
#include "oleobjw.hxx"
#include "unoobjw.hxx"
#include "servprov.hxx"

using namespace std;
using namespace osl;
using namespace cppu;
using namespace com::sun::star::uno;
using namespace com::sun::star::beans;
using namespace com::sun::star::container;
using namespace com::sun::star::script;
using namespace com::sun::star::lang;
using namespace com::sun::star::bridge::ModelDependent;
using namespace com::sun::star::reflection;

std::unordered_map<sal_uIntPtr, WeakReference<XInterface> > UnoObjToWrapperMap;
static bool writeBackOutParameter(VARIANTARG* pDest, VARIANT* pSource);
static bool writeBackOutParameter2( VARIANTARG* pDest, VARIANT* pSource);
static HRESULT mapCannotConvertException(const CannotConvertException &e, unsigned int * puArgErr);

/* Does not throw any exceptions.
   Param pInfo can be NULL.
 */
static void writeExcepinfo(EXCEPINFO * pInfo, const OUString& message)
{
    if (pInfo != nullptr)
    {
        pInfo->wCode = UNO_2_OLE_EXCEPTIONCODE;
        pInfo->bstrSource = SysAllocString(L"[automation bridge] ");
        pInfo->bstrDescription = SysAllocString(o3tl::toW(message.getStr()));
    }
}

InterfaceOleWrapper::InterfaceOleWrapper( Reference<XMultiServiceFactory> const & xFactory,
                                          sal_uInt8 unoWrapperClass, sal_uInt8 comWrapperClass):
        UnoConversionUtilities<InterfaceOleWrapper>( xFactory, unoWrapperClass, comWrapperClass),
        m_defaultValueType( 0)
{
}

InterfaceOleWrapper::~InterfaceOleWrapper()
{
    MutexGuard guard(getBridgeMutex());
    // remove entries in global map
    auto it = UnoObjToWrapperMap.find( reinterpret_cast<sal_uIntPtr>(m_xOrigin.get()));
    if(it != UnoObjToWrapperMap.end())
        UnoObjToWrapperMap.erase(it);
}

STDMETHODIMP InterfaceOleWrapper::QueryInterface(REFIID riid, LPVOID FAR * ppv)
{
    comphelper::Automation::AutomationInvokedZone aAutomationActive;

    SAL_INFO("extensions.olebridge", this << "@InterfaceOleWrapper::QueryInterface(" << riid << ")");

    HRESULT ret= S_OK;

    if( !ppv)
        return E_POINTER;

    if(IsEqualIID(riid, IID_IUnknown))
    {
        AddRef();
        *ppv = static_cast<IUnknown*>(static_cast<IDispatch*>(this));
        SAL_INFO("extensions.olebridge", "  " << *ppv);
    }
    else if (IsEqualIID(riid, IID_IDispatch))
    {
        AddRef();
        *ppv = static_cast<IDispatch*>(this);
        SAL_INFO("extensions.olebridge", "  " << *ppv);
    }
    else if (IsEqualIID(riid, IID_IProvideClassInfo))
    {
        Reference<ooo::vba::XConnectable> xConnectable(m_xOrigin, UNO_QUERY);
        if (!xConnectable.is())
            return E_NOINTERFACE;
        AddRef();
        *ppv = static_cast<IProvideClassInfo*>(this);
        SAL_INFO("extensions.olebridge", "  " << *ppv);
    }
    else if (IsEqualIID(riid, IID_IConnectionPointContainer))
    {
        Reference<ooo::vba::XConnectable> xConnectable(m_xOrigin, UNO_QUERY);
        if (!xConnectable.is())
            return E_NOINTERFACE;
        AddRef();
        *ppv = static_cast<IConnectionPointContainer*>(this);
        SAL_INFO("extensions.olebridge", "  " << *ppv);
    }
    else if( IsEqualIID( riid, __uuidof( IUnoObjectWrapper)))
    {
        AddRef();
        *ppv= static_cast<IUnoObjectWrapper*>(this);
        SAL_INFO("extensions.olebridge", "  " << *ppv);
    }
    else
        ret= E_NOINTERFACE;
    return ret;
}

STDMETHODIMP_(ULONG) InterfaceOleWrapper::AddRef()
{
    acquire();
    // does not need to guard because one should not rely on the return value of
    // AddRef anyway
    return m_refCount;
}

STDMETHODIMP_(ULONG) InterfaceOleWrapper::Release()
{
    ULONG n= m_refCount;
    release();
    return n - 1;
}

// IUnoObjectWrapper --------------------------------------------------------
STDMETHODIMP InterfaceOleWrapper::getWrapperXInterface( Reference<XInterface>* pXInt)
{
    pXInt->set( static_cast<XWeak*>( this), UNO_QUERY);
    return pXInt->is() ? S_OK : E_FAIL;
}
STDMETHODIMP InterfaceOleWrapper::getOriginalUnoObject( Reference<XInterface>* pXInt)
{
    *pXInt= m_xOrigin;
    return m_xOrigin.is() ? S_OK : E_FAIL;
}
STDMETHODIMP  InterfaceOleWrapper::getOriginalUnoStruct( Any * pStruct)
{
    comphelper::Automation::AutomationInvokedZone aAutomationActive;

    HRESULT ret= E_FAIL;
    if( !m_xOrigin.is())
    {
        Reference<XMaterialHolder> xMatHolder( m_xInvocation, UNO_QUERY);
        if( xMatHolder.is())
        {
            Any any = xMatHolder->getMaterial();
            if( any.getValueTypeClass() == TypeClass_STRUCT)
            {
                *pStruct= any;
                ret= S_OK;
            }
        }
    }
    return ret;
}

STDMETHODIMP InterfaceOleWrapper::GetTypeInfoCount( unsigned int *pctinfo )
{
    SAL_INFO("extensions.olebridge", this << "@InterfaceOleWrapper::GetTypeInfoCount");

    if (!pctinfo)
        return E_POINTER;

    *pctinfo = 1;

    return S_OK;
}

class CXTypeInfo : public ITypeInfo,
                   public CComObjectRoot
{
public:
    enum class Kind { COCLASS, MAIN, OUTGOING };

#if defined __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"
#endif
    BEGIN_COM_MAP(CXTypeInfo)
#if defined __clang__
#pragma clang diagnostic pop
#endif
        COM_INTERFACE_ENTRY(ITypeInfo)
#if defined __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winconsistent-missing-override"
#pragma clang diagnostic ignored "-Wunused-function"
#endif
    END_COM_MAP()
#if defined __clang__
#pragma clang diagnostic pop
#endif

    DECLARE_NOT_AGGREGATABLE(CXTypeInfo)

    virtual ~CXTypeInfo() {}

    void InitForCoclass(Reference<XInterface> xOrigin,
                        const OUString& sImplementationName,
                        const IID& rIID,
                        Reference<XMultiServiceFactory> xMSF);
    void InitForClassItself(Reference<XInterface> xOrigin,
                            const OUString& sImplementationName,
                            const IID& rIID,
                            Reference<XMultiServiceFactory> xMSF);
    void InitForOutgoing(Reference<XInterface> xOrigin,
                         const OUString& sInterfaceName,
                         const IID& rIID,
                         Reference<XMultiServiceFactory> xMSF,
                         Type aType);
    virtual HRESULT STDMETHODCALLTYPE GetTypeAttr(TYPEATTR **ppTypeAttr) override;
    virtual HRESULT STDMETHODCALLTYPE GetTypeComp(ITypeComp **ppTComp) override;
    virtual HRESULT STDMETHODCALLTYPE GetFuncDesc(UINT index,
                                                  FUNCDESC **ppFuncDesc) override;
    virtual HRESULT STDMETHODCALLTYPE GetVarDesc(UINT index,
                                                 VARDESC **ppVarDesc) override;
    virtual HRESULT STDMETHODCALLTYPE GetNames(MEMBERID memid,
                                               BSTR *rgBstrNames,
                                               UINT cMaxNames,
                                               UINT *pcNames) override;
    virtual HRESULT STDMETHODCALLTYPE GetRefTypeOfImplType(UINT index,
                                                           HREFTYPE *pRefType) override;
    virtual HRESULT STDMETHODCALLTYPE GetImplTypeFlags(UINT index,
                                                       INT *pImplTypeFlags) override;
    virtual HRESULT STDMETHODCALLTYPE GetIDsOfNames(LPOLESTR *rgszNames,
                                                    UINT cNames,
                                                    MEMBERID *pMemId) override;
    virtual HRESULT STDMETHODCALLTYPE Invoke(PVOID pvInstance,
                                             MEMBERID memid,
                                             WORD wFlags,
                                             DISPPARAMS *pDispParams,
                                             VARIANT *pVarResult,
                                             EXCEPINFO *pExcepInfo,
                                             UINT *puArgErr) override;
    virtual HRESULT STDMETHODCALLTYPE GetDocumentation(MEMBERID memid,
                                                       BSTR *pBstrName,
                                                       BSTR *pBstrDocString,
                                                       DWORD *pdwHelpContext,
                                                       BSTR *pBstrHelpFile) override;
    virtual HRESULT STDMETHODCALLTYPE GetDllEntry(MEMBERID memid,
                                                  INVOKEKIND invKind,
                                                  BSTR *pBstrDllName,
                                                  BSTR *pBstrName,
                                                  WORD *pwOrdinal) override;
    virtual HRESULT STDMETHODCALLTYPE GetRefTypeInfo(HREFTYPE hRefType,
                                                     ITypeInfo **ppTInfo) override;
    virtual HRESULT STDMETHODCALLTYPE AddressOfMember(MEMBERID memid,
                                                      INVOKEKIND invKind,
                                                      PVOID *ppv) override;
    virtual HRESULT STDMETHODCALLTYPE CreateInstance(IUnknown *pUnkOuter,
                                                     REFIID riid,
                                                     PVOID *ppvObj) override;
    virtual HRESULT STDMETHODCALLTYPE GetMops(MEMBERID memid,
                                              BSTR *pBstrMops) override;
    virtual HRESULT STDMETHODCALLTYPE GetContainingTypeLib(ITypeLib **ppTLib,
                                                           UINT *pIndex) override;
    virtual void STDMETHODCALLTYPE ReleaseTypeAttr(TYPEATTR *pTypeAttr) override;
    virtual void STDMETHODCALLTYPE ReleaseFuncDesc(FUNCDESC *pFuncDesc) override;
    virtual void STDMETHODCALLTYPE ReleaseVarDesc(VARDESC *pVarDesc) override;

private:
    Kind meKind;
    Reference<XInterface> mxOrigin;
    OUString msImplementationName;
    OUString msInterfaceName;
    IID maIID;
    Reference<XMultiServiceFactory> mxMSF;
    Type maType;
};

class CXTypeLib : public ITypeLib,
                  public CComObjectRoot
{
public:
#if defined __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"
#endif
    BEGIN_COM_MAP(CXTypeLib)
#if defined __clang__
#pragma clang diagnostic pop
#endif
        COM_INTERFACE_ENTRY(ITypeLib)
#if defined __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winconsistent-missing-override"
#pragma clang diagnostic ignored "-Wunused-function"
#endif
    END_COM_MAP()
#if defined __clang__
#pragma clang diagnostic pop
#endif

    DECLARE_NOT_AGGREGATABLE(CXTypeLib)

    virtual ~CXTypeLib() {}

    void Init(Reference<XInterface> xOrigin,
              const OUString& sImplementationName,
              Reference<XMultiServiceFactory> xMSF)
    {
        SAL_INFO("extensions.olebridge", this << "@CXTypeLib::Init for " << sImplementationName);
        mxOrigin = xOrigin;
        msImplementationName = sImplementationName;
        mxMSF = xMSF;
    }

    virtual UINT STDMETHODCALLTYPE GetTypeInfoCount() override
    {
        SAL_WARN("extensions.olebridge", this << "@CXTypeLib::GetTypeInfoCount");
        return 1;
    }

    virtual HRESULT STDMETHODCALLTYPE GetTypeInfo(UINT,
                                                  ITypeInfo **) override
    {
        SAL_WARN("extensions.olebridge", this << "@CXTypeLib::GetTypeInfo: E_NOTIMPL");
        return E_NOTIMPL;
    }

    virtual HRESULT STDMETHODCALLTYPE GetTypeInfoType(UINT,
                                                      TYPEKIND *) override
    {
        SAL_WARN("extensions.olebridge", this << "@CXTypeLib::GetTypeInfoType: E_NOTIMPL");
        return E_NOTIMPL;
    }

    virtual HRESULT STDMETHODCALLTYPE GetTypeInfoOfGuid(REFGUID guid,
                                                        ITypeInfo **ppTInfo) override
    {
        comphelper::Automation::AutomationInvokedZone aAutomationActive;

        SAL_INFO("extensions.olebridge", this << "@CXTypeLib::GetTypeInfoOfGuid(" << guid << ")");
        if (!ppTInfo)
            return E_POINTER;

        Reference<ooo::vba::XConnectable> xConnectable(mxOrigin, UNO_QUERY);
        if (!xConnectable.is())
            return TYPE_E_ELEMENTNOTFOUND;

        IID aIID;
        if (SUCCEEDED(IIDFromString(reinterpret_cast<LPOLESTR>(xConnectable->getIID().pData->buffer), &aIID)))
        {
            if (IsEqualIID(guid, aIID))
            {
                HRESULT ret;

                CComObject<CXTypeInfo>* pTypeInfo;

                ret = CComObject<CXTypeInfo>::CreateInstance(&pTypeInfo);
                if (FAILED(ret))
                    return ret;

                pTypeInfo->AddRef();

                pTypeInfo->InitForCoclass(mxOrigin, msImplementationName, aIID, mxMSF);

                *ppTInfo = pTypeInfo;

                return S_OK;
            }
        }

#if 0
        ooo::vba::TypeAndIID aTypeAndIID = xConnectable->GetConnectionPoint();

        IID aIID;
        if (SUCCEEDED(IIDFromString((LPOLESTR)aTypeAndIID.IID.pData->buffer, &aIID)))
        {
            HRESULT ret;

            CComObject<CXTypeInfo>* pTypeInfo;

            ret = CComObject<CXTypeInfo>::CreateInstance(&pTypeInfo);
            if (FAILED(ret))
                return ret;

            pTypeInfo->AddRef();

            pTypeInfo->InitForOutgoing(mxOrigin, msImplementationName, aIID, mxMSF);

            *ppTInfo = pTypeInfo;

            return S_OK;
        }
#else
        SAL_WARN("extensions.olebridge", "Not implemented: GetTypeInfoOfGuid(" << guid << ")");
#endif

        return TYPE_E_ELEMENTNOTFOUND;

    }

    virtual HRESULT STDMETHODCALLTYPE GetLibAttr(TLIBATTR **) override
    {
        SAL_WARN("extensions.olebridge", this << "@CXTypeLib::GetLibAttr: E_NOTIMPL");
        return E_NOTIMPL;
    }

    virtual HRESULT STDMETHODCALLTYPE GetTypeComp(ITypeComp **) override
    {
        SAL_WARN("extensions.olebridge", this << "@CXTypeLib::GetTypeComp: E_NOTIMPL");
        return E_NOTIMPL;
    }

    virtual HRESULT STDMETHODCALLTYPE GetDocumentation(INT,
                                                       BSTR *,
                                                       BSTR *,
                                                       DWORD *,
                                                       BSTR *) override
    {
        SAL_WARN("extensions.olebridge", this << "@CXTypeLib::GetDocumentation: E_NOTIMPL");
        return E_NOTIMPL;
    }

    virtual HRESULT STDMETHODCALLTYPE IsName(LPOLESTR,
                                             ULONG,
                                             BOOL *) override
    {
        SAL_WARN("extensions.olebridge", this << "@CXTypeLib:IsName: E_NOTIMPL");
        return E_NOTIMPL;
    }

    virtual HRESULT STDMETHODCALLTYPE FindName(LPOLESTR,
                                               ULONG,
                                               ITypeInfo **,
                                               MEMBERID *,
                                               USHORT *) override
    {
        SAL_WARN("extensions.olebridge", this << "@CXTypeLib::FindName: E_NOTIMPL");
        return E_NOTIMPL;
    }

    virtual void STDMETHODCALLTYPE ReleaseTLibAttr(TLIBATTR *) override
    {
        SAL_WARN("extensions.olebridge", this << "@CXTypeLib::ReleaseTLibAttr: E_NOTIMPL");
    }

private:
    Reference<XInterface> mxOrigin;
    OUString msImplementationName;
    Reference<XMultiServiceFactory> mxMSF;
};

void CXTypeInfo::InitForCoclass(Reference<XInterface> xOrigin,
                                const OUString& sImplementationName,
                                const IID& rIID,
                                Reference<XMultiServiceFactory> xMSF)
{
    SAL_INFO("extensions.olebridge", this << "@CXTypeInfo::InitForCoclass(" << sImplementationName << "," << rIID << ")");
    meKind = Kind::COCLASS;
    mxOrigin = xOrigin;
    msImplementationName = sImplementationName;
    maIID = rIID;
    mxMSF = xMSF;
}

void CXTypeInfo::InitForClassItself(Reference<XInterface> xOrigin,
                                    const OUString& sImplementationName,
                                    const IID& rIID,
                                    Reference<XMultiServiceFactory> xMSF)
{
    SAL_INFO("extensions.olebridge", this << "@CXTypeInfo::InitForClassItself(" << sImplementationName << "," << rIID << ")");
    meKind = Kind::MAIN;
    mxOrigin = xOrigin;
    msImplementationName = sImplementationName;
    maIID = rIID;
    mxMSF = xMSF;
}

void CXTypeInfo::InitForOutgoing(Reference<XInterface> xOrigin,
                                 const OUString& sInterfaceName,
                                 const IID& rIID,
                                 Reference<XMultiServiceFactory> xMSF,
                                 Type aType)
{
    SAL_INFO("extensions.olebridge", this << "@CXTypeInfo::InitForOutgoing(" << sInterfaceName << "," << rIID << ")");
    meKind = Kind::OUTGOING;
    mxOrigin = xOrigin;
    msInterfaceName = sInterfaceName;
    maIID = rIID;
    mxMSF = xMSF;
    maType = aType;
}

HRESULT STDMETHODCALLTYPE CXTypeInfo::GetTypeAttr(TYPEATTR **ppTypeAttr)
{
    comphelper::Automation::AutomationInvokedZone aAutomationActive;

    SAL_INFO("extensions.olebridge", this << "@CXTypeInfo::GetTypeAttr");

    if (!ppTypeAttr)
        return E_POINTER;

    assert(!IsEqualIID(maIID, IID_NULL));

    TYPEATTR *pTypeAttr = new TYPEATTR;
    memset(pTypeAttr, 0, sizeof(*pTypeAttr));

    pTypeAttr->guid = maIID;

    if (meKind == Kind::COCLASS)
    {
        pTypeAttr->typekind = TKIND_COCLASS;
        pTypeAttr->cFuncs = 0;
        pTypeAttr->cVars = 0;
        pTypeAttr->cImplTypes = 3;
        pTypeAttr->cbSizeVft = 0;
        pTypeAttr->cbAlignment = 8;
        pTypeAttr->wTypeFlags = TYPEFLAG_FCANCREATE;
    }
    else if (meKind == Kind::MAIN)
    {
        pTypeAttr->typekind = TKIND_DISPATCH;
        pTypeAttr->cFuncs = 10; // FIXME, dummy
        pTypeAttr->cVars = 0;
        pTypeAttr->cImplTypes = 1;
        // FIXME: I think this is always supposed to be as if just for the seven methods in
        // IDIspatch?
        pTypeAttr->cbSizeVft = 7 * sizeof(void*);
        pTypeAttr->cbAlignment = 8;
        pTypeAttr->wTypeFlags = TYPEFLAG_FHIDDEN|TYPEFLAG_FDISPATCHABLE;
    }
    else if (meKind == Kind::OUTGOING)
    {
        pTypeAttr->typekind = TKIND_DISPATCH;

        Reference<XIdlReflection> xRefl = theCoreReflection::get(comphelper::getComponentContext(mxMSF));
        assert(xRefl.is());

        Reference<XIdlClass> xClass = xRefl->forName(maType.getTypeName());
        assert(xClass.is());

        auto aMethods = xClass->getMethods();
        assert(xClass->getTypeClass() == TypeClass_INTERFACE &&
               aMethods.getLength() > 0);

        // Drop the three XInterface methods, add the three corresponding IUnknown ones plus the
        // four IDispatch ones on top of that.
        pTypeAttr->cFuncs = aMethods.getLength() - 3 + 3 + 4;
        pTypeAttr->cVars = 0;
        pTypeAttr->cImplTypes = 1;
        // FIXME: I think this, too, is always supposed to be as if just for the seven methods in
        // IDIspatch?
        pTypeAttr->cbSizeVft = 7 * sizeof(void*);
        pTypeAttr->cbAlignment = 8;
        pTypeAttr->wTypeFlags = TYPEFLAG_FHIDDEN|TYPEFLAG_FNONEXTENSIBLE|TYPEFLAG_FDISPATCHABLE;
    }
    else
        assert(false);

    pTypeAttr->lcid = LOCALE_USER_DEFAULT;
    pTypeAttr->memidConstructor = MEMBERID_NIL;
    pTypeAttr->memidDestructor = MEMBERID_NIL;
    // FIXME: Is this correct, just the vtable pointer, right?
    pTypeAttr->cbSizeInstance = sizeof(void*);
    pTypeAttr->wMajorVerNum = 0;
    pTypeAttr->wMinorVerNum = 0;
    pTypeAttr->idldescType.wIDLFlags = IDLFLAG_NONE;

    SAL_INFO("extensions.olebridge", this << "@CXTypeInfo::GetTypeAttr: " << pTypeAttr);

    *ppTypeAttr = pTypeAttr;

    return S_OK;
}

HRESULT STDMETHODCALLTYPE CXTypeInfo::GetTypeComp(ITypeComp **)
{
    SAL_WARN("extensions.olebridge", this << "@CXTypeInfo::GetTypeComp: E_NOTIMPL");
    return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CXTypeInfo::GetFuncDesc(UINT index,
                                                  FUNCDESC **ppFuncDesc)
{
    comphelper::Automation::AutomationInvokedZone aAutomationActive;

    if (!ppFuncDesc)
        return E_POINTER;

    if (meKind != Kind::OUTGOING)
        return E_NOTIMPL;

    if (index <= 6)
    {
        *ppFuncDesc = new FUNCDESC;
        (*ppFuncDesc)->memid = 0x60000000 + index;
        (*ppFuncDesc)->lprgscode = nullptr;
        (*ppFuncDesc)->lprgelemdescParam = nullptr;
        (*ppFuncDesc)->funckind = FUNC_DISPATCH;
        (*ppFuncDesc)->invkind = INVOKE_FUNC;
        (*ppFuncDesc)->callconv = CC_STDCALL;
        switch (index)
        {
        case 0: // QueryInterface
            (*ppFuncDesc)->cParams = 2;
            (*ppFuncDesc)->elemdescFunc.tdesc.lptdesc = nullptr;
            (*ppFuncDesc)->elemdescFunc.tdesc.vt = VT_VOID;
            break;
        case 1: // AddRef
            (*ppFuncDesc)->cParams = 0;
            (*ppFuncDesc)->elemdescFunc.tdesc.lptdesc = nullptr;
            (*ppFuncDesc)->elemdescFunc.tdesc.vt = VT_UI4;
            break;
        case 2: // Release
            (*ppFuncDesc)->cParams = 1;
            (*ppFuncDesc)->elemdescFunc.tdesc.lptdesc = nullptr;
            (*ppFuncDesc)->elemdescFunc.tdesc.vt = VT_UI4;
            break;
        case 3: // GetTypeInfoCount
            (*ppFuncDesc)->cParams = 1;
            (*ppFuncDesc)->elemdescFunc.tdesc.lptdesc = nullptr;
            (*ppFuncDesc)->elemdescFunc.tdesc.vt = VT_VOID;
            break;
        case 4: // GetTypeInfo
            (*ppFuncDesc)->cParams = 3;
            (*ppFuncDesc)->elemdescFunc.tdesc.lptdesc = nullptr;
            (*ppFuncDesc)->elemdescFunc.tdesc.vt = VT_VOID;
            break;
        case 5: // GetIDsOfNames
            (*ppFuncDesc)->cParams = 5;
            (*ppFuncDesc)->elemdescFunc.tdesc.lptdesc = nullptr;
            (*ppFuncDesc)->elemdescFunc.tdesc.vt = VT_VOID;
            break;
        case 6: // Invoke
            (*ppFuncDesc)->cParams = 8;
            (*ppFuncDesc)->elemdescFunc.tdesc.lptdesc = nullptr;
            (*ppFuncDesc)->elemdescFunc.tdesc.vt = VT_VOID;
            break;
        }
        (*ppFuncDesc)->cParamsOpt = 0;
        (*ppFuncDesc)->oVft = index * sizeof(void*);
        (*ppFuncDesc)->cScodes = 0;
        (*ppFuncDesc)->wFuncFlags = FUNCFLAG_FRESTRICTED;

        SAL_INFO("extensions.olebridge", this << "@CXTypeInfo::GetFuncDesc(" << index << "): S_OK: " << *ppFuncDesc);

        return S_OK;
    }

    Reference<XIdlReflection> xRefl = theCoreReflection::get(comphelper::getComponentContext(mxMSF));
    assert(xRefl.is());

    Reference<XIdlClass> xClass = xRefl->forName(maType.getTypeName());
    assert(xClass.is());

    auto aMethods = xClass->getMethods();
    assert(xClass->getTypeClass() == TypeClass_INTERFACE &&
           aMethods.getLength() > 0);

    if (index > static_cast<UINT>(aMethods.getLength() - 3 + 3 + 4))
        return E_INVALIDARG;

    *ppFuncDesc = new FUNCDESC;

    (*ppFuncDesc)->memid = index - 6;
    (*ppFuncDesc)->lprgscode = nullptr;
    (*ppFuncDesc)->lprgelemdescParam = nullptr;
    (*ppFuncDesc)->funckind = FUNC_DISPATCH;
    (*ppFuncDesc)->invkind = INVOKE_FUNC;
    (*ppFuncDesc)->callconv = CC_STDCALL;
    (*ppFuncDesc)->cParams = aMethods[index - 4]->getParameterInfos().getLength();
    (*ppFuncDesc)->cParamsOpt = 0;
    (*ppFuncDesc)->oVft = index * sizeof(void*);
    (*ppFuncDesc)->cScodes = 0;
    (*ppFuncDesc)->elemdescFunc.tdesc.lptdesc = nullptr; // ???
    (*ppFuncDesc)->elemdescFunc.tdesc.vt = VT_VOID; // ???
    (*ppFuncDesc)->wFuncFlags = 0;

    SAL_INFO("extensions.olebridge", this << "@CXTypeInfo::GetFuncDesc(" << index << "): S_OK: " << *ppFuncDesc);

    return S_OK;
}

HRESULT STDMETHODCALLTYPE CXTypeInfo::GetVarDesc(UINT,
                                                 VARDESC **)
{
    SAL_WARN("extensions.olebridge", this << "@CXTypeInfo::GetVarDesc: E_NOTIMPL");
    return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CXTypeInfo::GetNames(MEMBERID memid,
                                               BSTR *rgBstrNames,
                                               UINT cMaxNames,
                                               UINT *pcNames)
{
    comphelper::Automation::AutomationInvokedZone aAutomationActive;

    SAL_INFO("extensions.olebridge", this << "@CXTypeInfo::GetNames(" << memid << ")");
    assert(meKind != Kind::COCLASS);

    if (!rgBstrNames)
        return E_POINTER;

    if (!pcNames)
        return E_POINTER;

    if (memid < 1)
        return E_INVALIDARG;

    if (cMaxNames < 1)
        return E_INVALIDARG;

    if (meKind == Kind::MAIN)
    {
        SAL_WARN("extensions.olebridge", "GetNames() for MAIN not implemented");
        return E_NOTIMPL;
    }

    Reference<XIdlReflection> xRefl = theCoreReflection::get(comphelper::getComponentContext(mxMSF));
    assert(xRefl.is());

    Reference<XIdlClass> xClass = xRefl->forName(maType.getTypeName());
    assert(xClass.is());

    auto aMethods = xClass->getMethods();
    assert(xClass->getTypeClass() == TypeClass_INTERFACE &&
           aMethods.getLength() > 0);

    // Subtract the three XInterface methods. Memid for the first following method is 1.
    if (memid > aMethods.getLength() - 3)
        return E_INVALIDARG;

    SAL_INFO("extensions.olebridge", "..." << this << "@CXTypeInfo::GetNames(" << memid << "): " << aMethods[memid + 2]->getName());
    rgBstrNames[0] = SysAllocString(reinterpret_cast<LPOLESTR>(aMethods[memid + 2]->getName().pData->buffer));
    *pcNames = 1;

    return S_OK;
}

HRESULT STDMETHODCALLTYPE CXTypeInfo::GetRefTypeOfImplType(UINT index,
                                                           HREFTYPE *pRefType)
{
    SAL_INFO("extensions.olebridge", this << "@CXTypeInfo::GetRefTypeOfImplType(" << index << ")");

    if (!pRefType)
        return E_POINTER;

    assert(index == 0 || index == 1);

    *pRefType = 1000+index;

    return S_OK;
}

HRESULT STDMETHODCALLTYPE CXTypeInfo::GetImplTypeFlags(UINT index,
                                                       INT *pImplTypeFlags)
{
    comphelper::Automation::AutomationInvokedZone aAutomationActive;

    SAL_INFO("extensions.olebridge", this << "@CXTypeInfo::GetImplTypeFlags(" << index << ")");

    if (!pImplTypeFlags)
        return E_POINTER;

    assert(meKind == Kind::COCLASS);
    assert(index == 0 || index == 1);

    if (index == 0)
        *pImplTypeFlags = IMPLTYPEFLAG_FDEFAULT;
    else if (index == 1)
        *pImplTypeFlags = IMPLTYPEFLAG_FDEFAULT|IMPLTYPEFLAG_FSOURCE;

    return S_OK;
}

HRESULT STDMETHODCALLTYPE CXTypeInfo::GetIDsOfNames(LPOLESTR *,
                                                    UINT,
                                                    MEMBERID *)
{
    SAL_WARN("extensions.olebridge", this << "@CXTypeInfo::GetIDsOfNames: E_NOTIMPL");
    return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CXTypeInfo::Invoke(PVOID,
                                             MEMBERID,
                                             WORD,
                                             DISPPARAMS *,
                                             VARIANT *,
                                             EXCEPINFO *,
                                             UINT *)
{
    SAL_WARN("extensions.olebridge", this << "@CXTypeInfo::Invoke: E_NOTIMPL");
    return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CXTypeInfo::GetDocumentation(MEMBERID memid,
                                                       BSTR *pBstrName,
                                                       BSTR *pBstrDocString,
                                                       DWORD *pdwHelpContext,
                                                       BSTR *pBstrHelpFile)
{
    comphelper::Automation::AutomationInvokedZone aAutomationActive;

    SAL_INFO("extensions.olebridge", this << "@CXTypeInfo::GetDocumentation(" << memid << ")");

    if (pBstrName)
    {
        if (memid == MEMBERID_NIL)
        {
            *pBstrName = SysAllocString(o3tl::toW(msImplementationName.getStr()));
        }
        else if (memid == DISPID_VALUE)
        {
            // MEMBERIDs are the same as DISPIDs, apparently?
            *pBstrName = SysAllocString(L"Value");
        }
        else
        {
            *pBstrName = SysAllocString(o3tl::toW(OUString(OUString("UnknownNameOfMember#") + OUString::number(memid)).getStr()));
        }
    }
    if (pBstrDocString)
        *pBstrDocString = SysAllocString(L"");
    if (pdwHelpContext)
        *pdwHelpContext = 0;
    if (pBstrHelpFile)
        *pBstrHelpFile = nullptr;

    return S_OK;
}

HRESULT STDMETHODCALLTYPE CXTypeInfo::GetDllEntry(MEMBERID,
                                                  INVOKEKIND,
                                                  BSTR *,
                                                  BSTR *,
                                                  WORD *)
{
    SAL_WARN("extensions.olebridge", this << "@CXTypeInfo::GetDllEntry: E_NOTIMPL");
    return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CXTypeInfo::GetRefTypeInfo(HREFTYPE hRefType,
                                                     ITypeInfo **ppTInfo)
{
    comphelper::Automation::AutomationInvokedZone aAutomationActive;

    SAL_INFO("extensions.olebridge", this << "@CXTypeInfo::GetRefTypeInfo(" << hRefType << ")");

    if (!ppTInfo)
        return E_POINTER;

    // FIXME: Is it correct to assume that the only interfaces on which GetRefTypeInfo() would be
    // called are those that implement ooo::vba::XConnectable?

    Reference<ooo::vba::XConnectable> xConnectable(mxOrigin, UNO_QUERY);
    if (!xConnectable.is())
        return E_NOTIMPL;

    ooo::vba::TypeAndIID aTypeAndIID = xConnectable->GetConnectionPoint();

    IID aIID;
    if (!SUCCEEDED(IIDFromString(reinterpret_cast<LPOLESTR>(aTypeAndIID.IID.pData->buffer), &aIID)))
        return E_NOTIMPL;

    HRESULT ret;

    CComObject<CXTypeInfo>* pTypeInfo;

    ret = CComObject<CXTypeInfo>::CreateInstance(&pTypeInfo);
    if (FAILED(ret))
        return ret;

    pTypeInfo->AddRef();

    pTypeInfo->InitForOutgoing(mxOrigin, aTypeAndIID.Type.getTypeName(), aIID, mxMSF, aTypeAndIID.Type);

    *ppTInfo = pTypeInfo;

    return S_OK;
}

HRESULT STDMETHODCALLTYPE CXTypeInfo::AddressOfMember(MEMBERID,
                                                      INVOKEKIND,
                                                      PVOID *)
{
    SAL_WARN("extensions.olebridge", this << "@CXTypeInfo::AddressOfMember: E_NOTIMPL");
    return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CXTypeInfo::CreateInstance(IUnknown *,
                                                     REFIID,
                                                     PVOID *)
{
    SAL_WARN("extensions.olebridge", this << "@CXTypeInfo::CreateInstance: E_NOTIMPL");
    return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CXTypeInfo::GetMops(MEMBERID,
                                              BSTR *)
{
    SAL_WARN("extensions.olebridge", this << "@CXTypeInfo::GetMops: E_NOTIMPL");
    return E_NOTIMPL;
}

// This is not actually called any more by my vbscript test after I added the IProvideClassInfo
// thing... so all the CXTypeLib stuff is dead code at the moment.

HRESULT STDMETHODCALLTYPE CXTypeInfo::GetContainingTypeLib(ITypeLib **ppTLib,
                                                           UINT *pIndex)
{
    comphelper::Automation::AutomationInvokedZone aAutomationActive;

    SAL_INFO("extensions.olebridge", this << "@CXTypeInfo::GetContainingTypeLib");

    if (!ppTLib || !pIndex)
        return E_POINTER;

    HRESULT ret;

    CComObject<CXTypeLib>* pTypeLib;

    ret = CComObject<CXTypeLib>::CreateInstance(&pTypeLib);
    if (FAILED(ret))
        return ret;

    pTypeLib->AddRef();

    pTypeLib->Init(mxOrigin, msImplementationName, mxMSF);

    *ppTLib = pTypeLib;

    return S_OK;
}

void STDMETHODCALLTYPE CXTypeInfo::ReleaseTypeAttr(TYPEATTR *pTypeAttr)
{
    SAL_INFO("extensions.olebridge", this << "@CXTypeInfo::ReleaseTypeAttr(" << pTypeAttr << ")");

    delete pTypeAttr;
}

void STDMETHODCALLTYPE CXTypeInfo::ReleaseFuncDesc(FUNCDESC *pFuncDesc)
{
    SAL_WARN("extensions.olebridge", this << "@CXTypeInfo::ReleaseFuncDesc(" << pFuncDesc << ")");

    delete pFuncDesc;
}

void STDMETHODCALLTYPE CXTypeInfo::ReleaseVarDesc(VARDESC *)
{
    SAL_WARN("extensions.olebridge", this << "@CXTypeInfo::ReleaseVarDesc: E_NOTIMPL");
}

STDMETHODIMP InterfaceOleWrapper::GetTypeInfo(unsigned int iTInfo, LCID, ITypeInfo ** ppTInfo)
{
    comphelper::Automation::AutomationInvokedZone aAutomationActive;

    SAL_INFO("extensions.olebridge", this << "@InterfaceOleWrapper::GetTypeInfo(" << iTInfo << ")");

    if (!ppTInfo)
        return E_POINTER;

    if (iTInfo != 0)
        return E_NOTIMPL;

    // FIXME: This is surely incorrect. Why is being able to handle GetTypeInfo() here coupled to
    // being a source for outgoing events, i.e. implementing XConnectable? What would break if we
    // would use XInterfaceWithIID and its getIID instead?

    Reference<ooo::vba::XConnectable> xConnectable(m_xOrigin, UNO_QUERY);
    if (!xConnectable.is())
        return E_NOTIMPL;

    OUString sIID = xConnectable->GetIIDForClassItselfNotCoclass();
    IID aIID;
    if (!SUCCEEDED(IIDFromString(reinterpret_cast<LPOLESTR>(sIID.pData->buffer), &aIID)))
        return E_NOTIMPL;

    HRESULT ret;

    CComObject<CXTypeInfo>* pTypeInfo;

    ret = CComObject<CXTypeInfo>::CreateInstance(&pTypeInfo);
    if (FAILED(ret))
        return ret;

    pTypeInfo->AddRef();

    pTypeInfo->InitForClassItself(m_xOrigin, m_sImplementationName, aIID, m_smgr);

    *ppTInfo = pTypeInfo;

    return S_OK;
}

STDMETHODIMP InterfaceOleWrapper::GetIDsOfNames(REFIID /*riid*/,
                                                OLECHAR ** rgszNames,
                                                unsigned int cNames,
                                                LCID /*lcid*/,
                                                DISPID * rgdispid )
{
    comphelper::Automation::AutomationInvokedZone aAutomationActive;

    if( ! rgdispid)
        return E_POINTER;

    OUString sNames;
    sNames += "[";
    for (unsigned int i = 0; i < cNames; ++i)
    {
        // Initialise returned rgdispid values.
        rgdispid[i] = DISPID_UNKNOWN;
        if (i > 0)
            sNames += ",";
        sNames += "\"" + OUString(o3tl::toU(rgszNames[i])) + "\"";
    }
    sNames += "]";

    SAL_INFO("extensions.olebridge", this << "@InterfaceOleWrapper::GetIDsOfNames(" << sNames);

    HRESULT ret = DISP_E_UNKNOWNNAME;
    try
    {
        MutexGuard guard( getBridgeMutex());

        // FIXME: Handle the cNames > 1 case? Note that the rest of the names mean the names of *arguments*.

        if( ! _wcsicmp( *rgszNames, JSCRIPT_VALUE_FUNC) ||
            ! _wcsicmp( *rgszNames, BRIDGE_VALUE_FUNC))
        {
            *rgdispid= DISPID_JSCRIPT_VALUE_FUNC;
            return S_OK;
        }
        else if( ! _wcsicmp( *rgszNames, GET_STRUCT_FUNC) ||
                 ! _wcsicmp( *rgszNames, BRIDGE_GET_STRUCT_FUNC))
        {
            *rgdispid= DISPID_GET_STRUCT_FUNC;
            return S_OK;
        }
        else if( ! _wcsicmp( *rgszNames, BRIDGE_CREATE_TYPE_FUNC))
        {
            *rgdispid= DISPID_CREATE_TYPE_FUNC;
            return S_OK;
        }

        if (m_xInvocation.is() && (cNames > 0))
        {
            OUString name(o3tl::toU(rgszNames[0]));
            NameToIdMap::iterator iter = m_nameToDispIdMap.find(name);

            bool bIsMethod = false;

            OUString exactName = name;

            if (iter == m_nameToDispIdMap.end())
            {
                if (m_xExactName.is())
                {
                    exactName = m_xExactName->getExactName(name);
                    if (exactName.isEmpty())
                        exactName = name;
                }

                MemberInfo d(0, exactName);

                if (m_xInvocation->hasProperty(exactName))
                {
                    d.flags |= DISPATCH_PROPERTYGET;
                    d.flags |= DISPATCH_PROPERTYPUT;
                    d.flags |= DISPATCH_PROPERTYPUTREF;
                }

                if (m_xInvocation->hasMethod(exactName))
                {
                    d.flags |= DISPATCH_METHOD;
                    bIsMethod = true;
                }

                if (d.flags != 0)
                {
                    m_MemberInfos.push_back(d);
                    iter = m_nameToDispIdMap.emplace(exactName, static_cast<DISPID>(m_MemberInfos.size())).first;

                    if (exactName != name)
                    {
                        iter = m_nameToDispIdMap.emplace(name, static_cast<DISPID>(m_MemberInfos.size())).first;
                    }
                }
            }

            if (iter == m_nameToDispIdMap.end())
            {
                ret = DISP_E_UNKNOWNNAME;
                SAL_INFO("extensions.olebridge", "  " << name << ": UNKNOWN");
            }
            else
            {
                rgdispid[0] = (*iter).second;
                SAL_INFO("extensions.olebridge", "  " << name << ": " << rgdispid[0]);

                if (bIsMethod && cNames > 1)
                {
                    Reference<XIdlMethod> xIdlMethod;
                    Reference<XIntrospectionAccess> xIntrospectionAccess = m_xInvocation->getIntrospection();
                    try
                    {
                        if (xIntrospectionAccess.is())
                            xIdlMethod = xIntrospectionAccess->getMethod(exactName, MethodConcept::ALL);
                    }
                    catch (const NoSuchMethodException&)
                    {
                    }
                    if (xIdlMethod.is())
                    {
                        auto aParamInfos = xIdlMethod->getParameterInfos();
                        for (unsigned int i = 1; i < cNames; ++i)
                        {
                            bool bFound = false;
                            for (int j = 0; j < aParamInfos.getLength(); ++j)
                            {
                                if (aParamInfos[j].aName.equalsIgnoreAsciiCase(OUString(o3tl::toU(rgszNames[i]))))
                                {
                                    rgdispid[i] = j;
                                    bFound = true;
                                    SAL_INFO("extensions.olebridge", "  " << OUString(o3tl::toU(rgszNames[i])) << ": " << rgdispid[i]);
                                    break;
                                }
                            }
                            if (!bFound)
                                SAL_INFO("extensions.olebridge", "  " << OUString(o3tl::toU(rgszNames[i])) << ": NOT FOUND");
                        }
                    }
                }

                // Return value should be S_OK only if *all* the names were found.
                unsigned int i;
                for (i = 0; i < cNames; ++i)
                    if (rgdispid[i] == DISPID_UNKNOWN)
                        break;
                if (i == cNames)
                    ret = S_OK;
            }
        }
    }
    catch(const BridgeRuntimeError&)
    {
        OSL_ASSERT(false);
    }
    catch(const Exception&)
    {
        OSL_ASSERT(false);
    }
    catch(...)
    {
        OSL_ASSERT(false);
    }

    return ret;
}

// Note: What the comments here say about JScript possibly holds for Automation clients in general,
// like VBScript ones, too. Or not. Hard to say. What is the relevance of JScript nowadays anyway,
// and can LO really be used from JScript code on web pages any longer?

// "convertDispparamsArgs" converts VARIANTS to their respecting Any counterparts
// The parameters "id", "wFlags" and "pdispparams" equal those as used in
// IDispatch::Invoke. The function handles special JavaScript
// cases where a VARIANT of type VT_DISPATCH is ambiguous and could represent
// an object, array ( JavaScript Array object), out parameter and in/out ( JavaScript Array object)
//  parameter (JavaScript Array object)
// Because all those VT_DISPATCH objects need a different conversion
// we have to find out what the object is supposed to be. The function does this
// by either using type information or by help of a specialized ValueObject object.

// A. Type Information

// With the help of type information the kind of parameter can be exactly determined
// and an appropriate conversion can be chosen. A problem arises if a method expects
// an Any. Then the type info does not tell what the type of the value, that is kept
// by the any, should be. In this situation the decision whether the param is a
// sequence or an object is made upon the fact if the object has a property "0"
// ( see function "isJScriptArray"). Since this is unsafe it is recommended to use
// the JScript value objects within a JScript script on such an occasion.

// B. JavaScript Value Object ( class JScriptValue )

// A JScriptValue (ValueObject) object is a COM object in that it implements IDispatch and the
// IJScriptValue object interface. Such objects are provided by all UNO wrapper
// objects used within a JScript script. To obtain an instance one has to call
// "_GetValueObject() or Bridge_GetValueObject()" on an UNO wrapper object (class InterfaceOleWrapper).
// A value object is appropriately initialized within the script and passed as
// parameter to an UNO object method or property. The convertDispparamsArgs function
// can easily find out that a param is such an object by querying for the
// IJScriptValue interface. By this interface one the type and kind ( out, in/out)
// can be determined and the right conversion can be applied.
// Using ValueObjects we spare us the effort of acquiring and examining type information
// in order to figure out what the an IDispatch parameter is meant for.

// Normal JScript object parameter can be mixed with JScriptValue object. If an
// VARIANT contains an VT_DISPATCH that is no JScriptValue than the type information
// is used to find out about the required type.
void InterfaceOleWrapper::convertDispparamsArgs(DISPID id,
    unsigned short /*wFlags*/, DISPPARAMS* pdispparams, Sequence<Any>& rSeq)
{
    // Parameters come in in reverse order in pdispparams. There might be less parameters than
    // expected. In that case, assume they are "optional" (but can't be marked as such in UNO IDL),
    // and fill in the rest with empty Anys. There might also be more than expected. In that case,
    // assume the oovbaapi UNO IDL hasn't kept up with added optional parameters in MSO, and just
    // ignore the extra ones, as long as they are empty.

    // An example: incoming parameters: <12, 13, "foo/bar.tem">
    //
    // Expected parameters: (string filename, int something, int somethingElse, Any whatever, Any
    // whateverElse)
    //
    // Here the existing incoming parameters are placed in reverse order in the first three outgoing
    // parameters, and the rest of the outgoing parameters are kept as empty Anys.
    //
    // Another example: incoming parameters: <EMPTY, TRUE>
    //
    // Expected parameters: (bool flag)
    //
    // Here the TRUE is passed as the sole outgoing parameter, and the incoming EMPTY is ignored.
    //
    // Still an example: incoming parameters: <"foo.doc", TRUE>
    //
    // Expected parameters: (bool flag)
    //
    // This throws an error as the incoming string parameter presumably should do something important,
    // but there is no corresponding outgoing parameter.

    HRESULT hr = S_OK;
    const int countIncomingArgs = pdispparams->cArgs;

    //Get type information for the current call
    InvocationInfo info;
    if( ! getInvocationInfoForCall( id, info))
        throw BridgeRuntimeError(
                  "[automation bridge]InterfaceOleWrapper::convertDispparamsArgs \n"
                  "Could not obtain type information for current call.");

    // Size rSeq according to the number of expected parameters.
    const int expectedArgs = info.aParamTypes.getLength() + (info.eMemberType == MemberType_PROPERTY ? 1 : 0);
    rSeq.realloc( expectedArgs );
    Any* pParams = rSeq.getArray();

    Any anyParam;

    int outgoingArgIndex = 0;

    // Go through incoming parameters in reverse order, i.e. in the order as declared in IDL
    for (int i = std::max(countIncomingArgs, expectedArgs) - 1; i >= 0; i--)
    {
        // Ignore too many parameters if they are VT_EMPTY anyway
        if ( outgoingArgIndex >= expectedArgs && pdispparams->rgvarg[i].vt == VT_EMPTY )
            continue;

        // But otherwise too many parameters is an error
        if ( outgoingArgIndex >= expectedArgs )
            throw BridgeRuntimeError( "[automation bridge] Too many parameters" );

        if (info.eMemberType == MemberType_METHOD &&
            info.aParamModes[ outgoingArgIndex ] == ParamMode_OUT)
        {
            outgoingArgIndex++;
            continue;
        }

        if (i < countIncomingArgs)
        {
            // A missing (and hopefully optional) arg (in the middle of the argument list) is passed
            // as an empty Any.
            if (pdispparams->rgvarg[i].vt == VT_ERROR && pdispparams->rgvarg[i].scode == DISP_E_PARAMNOTFOUND)
            {
                Any aEmpty;
                pParams[ outgoingArgIndex ] = aEmpty;
                outgoingArgIndex++;
                continue;
            }

            if(convertValueObject( & pdispparams->rgvarg[i], anyParam))
            { //a param is a ValueObject and could be converted
                pParams[ outgoingArgIndex ] = anyParam;
                outgoingArgIndex++;
                continue;
            }
        }
        else
        {
            // A missing arg. Let's hope it is de facto optional (there is no way in UNO IDL to mark
            // a parameter as optional). The corresponding slot in pParams is already a void Any.
            // Here we don't increase outgoingArgIndex!
            continue;
        }

        // If the param is an out, in/out parameter in
        // JScript (Array object, with value at index 0) then we
        // extract Array[0] and put the value into varParam. At the end of the loop varParam
        // is converted if it contains a value otherwise the VARIANT from
        // DISPPARAMS is converted.
        CComVariant varParam;

        // Check for JScript out and in/out paramsobjects (VT_DISPATCH).
        // To find them out we use typeinformation of the function being called.

        // No idea how this stuff, originally written for JScript, works for other Automation
        // clients.

        if( pdispparams->rgvarg[i].vt == VT_DISPATCH )
        {
            if( info.eMemberType == MemberType_METHOD && info.aParamModes[ outgoingArgIndex ] == ParamMode_INOUT)
            {
                // INOUT-param
                // Index ( property) "0" contains the actual IN-param. The object is a JScript
                // Array object.
                // Get the IN-param at index "0"
                IDispatch* pdisp= pdispparams->rgvarg[i].pdispVal;

                OLECHAR const * sindex= L"0";
                DISPID id2;
                DISPPARAMS noParams= {nullptr,nullptr,0,0};
                if(SUCCEEDED( hr= pdisp->GetIDsOfNames( IID_NULL, const_cast<OLECHAR **>(&sindex), 1, LOCALE_USER_DEFAULT, &id2)))
                    hr= pdisp->Invoke( id2, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET,
                                       & noParams, & varParam, nullptr, nullptr);
                if( FAILED( hr))
                {
                    throw BridgeRuntimeError(
                        "[automation bridge] Could not determine "
                        "if the object has a member \"0\". Error: " +
                        OUString::number(hr));
                }
            }
        }

        if( varParam.vt == VT_EMPTY) // then it was no in/out parameter
            varParam= pdispparams->rgvarg[i];

        if(info.eMemberType == MemberType_METHOD)
            variantToAny( & varParam, anyParam,
                           info.aParamTypes[ outgoingArgIndex ]);
        else if(info.eMemberType == MemberType_PROPERTY)
            variantToAny( & varParam, anyParam, info.aType);
        else
            OSL_ASSERT(false);

        if (outgoingArgIndex < expectedArgs)
            pParams[ outgoingArgIndex ]= anyParam;
        outgoingArgIndex++;
    }// end for / iterating over all parameters
}

bool  InterfaceOleWrapper::getInvocationInfoForCall( DISPID id, InvocationInfo& info)
{
    bool bTypesAvailable= false;

    if( !m_xInvocation.is() )return false;
    Reference<XInvocation2> inv2( m_xInvocation, UNO_QUERY);
    if( inv2.is())
    {
        // We need the name of the property or method to get its type information.
        // The name can be identified through the param "id"
        // that is kept as value in the map m_nameToDispIdMap.
        // Problem: the Windows JScript engine sometimes changes small letters to capital
        // letters as happens in xidlclass_obj.createObject( var) // in JScript.
        // IDispatch::GetIdsOfNames is then called with "CreateObject" !!!
        // m_nameToDispIdMap can contain several names for one DISPID but only one is
        // the exact one. If there's no m_xExactName and therefore no exact name then
        // there's only one entry in the map.
        OUString sMemberName;

        auto ci1 = std::find_if(m_nameToDispIdMap.cbegin(), m_nameToDispIdMap.cend(),
            [&id](const NameToIdMap::value_type& nameToDispId) { return nameToDispId.second == id; }); // item is a pair<OUString, DISPID>
        if (ci1 != m_nameToDispIdMap.cend())
            sMemberName= (*ci1).first;
        // Get information for the current call ( property or method).
        // There could be similar names which only differ in the cases
        // of letters. First we assume that the name which was passed into
        // GetIDsOfNames is correct. If we won't get information with that
        // name then we have the invocation service use the XExactName interface.
        bool validInfo= true;
        InvocationInfo invInfo;
        try{
            invInfo= inv2->getInfoForName( sMemberName, false);
        }
        catch(const IllegalArgumentException&)
        {
            validInfo= false;
        }

        if( ! validInfo)
        {
            invInfo= inv2->getInfoForName( sMemberName, true);
        }
        if( invInfo.aName.pData)
        {
            bTypesAvailable= true;
            info= invInfo;
        }
    }
    return bTypesAvailable;
}

// XBridgeSupplier2 ---------------------------------------------------
// only bridges itself ( this instance of InterfaceOleWrapper)from UNO to IDispatch
// If sourceModelType is UNO than any UNO interface implemented by InterfaceOleWrapper
// can bridged to IDispatch ( if destModelType == OLE). The IDispatch is
// implemented by this class.
Any SAL_CALL InterfaceOleWrapper::createBridge(const Any& modelDepObject,
                                const Sequence<sal_Int8>& /*ProcessId*/,
                                sal_Int16 sourceModelType,
                                sal_Int16 destModelType)
{

    Any retAny;
    if( sourceModelType == UNO && destModelType == OLE &&
        modelDepObject.getValueTypeClass() == TypeClass_INTERFACE )
    {
        Reference<XInterface> xInt;
        if( modelDepObject >>= xInt )
        {
            if( xInt == Reference<XInterface>( static_cast<XWeak*>( this), UNO_QUERY))
            {
                VARIANT *pVar= static_cast<VARIANT*>(CoTaskMemAlloc( sizeof( VARIANT)));
                if( pVar)
                {
                    pVar->vt= VT_DISPATCH;
                    pVar->pdispVal= static_cast<IDispatch*>( this);
                    AddRef();

                    retAny<<= reinterpret_cast< sal_uIntPtr >( pVar);
                }
            }
        }
    }

    return retAny;
}

// XInitialization --------------------------------------------------
void SAL_CALL InterfaceOleWrapper::initialize( const Sequence< Any >& aArguments )
{
    switch( aArguments.getLength() )
    {
    case 2: // the object wraps an UNO struct
        aArguments[0] >>= m_xInvocation;
        aArguments[1] >>= m_defaultValueType;
        break;
    case 3: // the object wraps an UNO interface
        aArguments[0] >>= m_xInvocation;
        aArguments[1] >>= m_xOrigin;
        aArguments[2] >>= m_defaultValueType;

        Reference<XServiceInfo> xServiceInfo(m_xOrigin, UNO_QUERY);
        if (xServiceInfo.is())
            m_sImplementationName = xServiceInfo->getImplementationName();

        SAL_INFO("extensions.olebridge", this << "@InterfaceOleWrapper::initialize for "
                 << (m_sImplementationName.isEmpty()?"an unknown implementation":m_sImplementationName));
        break;
    }

    m_xExactName.set( m_xInvocation, UNO_QUERY);
}

Reference< XInterface > InterfaceOleWrapper::createUnoWrapperInstance()
{
    Reference<XWeak> xWeak= static_cast<XWeak*>( new InterfaceOleWrapper(
                            m_smgr, m_nUnoWrapperClass, m_nComWrapperClass));
    return Reference<XInterface>( xWeak, UNO_QUERY);
}

Reference<XInterface> InterfaceOleWrapper::createComWrapperInstance()
{
    Reference<XWeak> xWeak= static_cast<XWeak*>( new IUnknownWrapper(
                            m_smgr, m_nUnoWrapperClass, m_nComWrapperClass));
    return Reference<XInterface>( xWeak, UNO_QUERY);
}

// "getType" is used in convertValueObject to map the string denoting the type
// to an actual Type object.
bool getType( const BSTR name, Type & type)
{
    bool ret = false;
    typelib_TypeDescription * pDesc= nullptr;
    OUString str(o3tl::toU(name));
    typelib_typedescription_getByName( &pDesc, str.pData );
    if( pDesc)
    {
        type = Type( pDesc->pWeakRef );
        typelib_typedescription_release( pDesc);
        ret = true;
    }
    return ret;
}

static bool writeBackOutParameter2( VARIANTARG* pDest, VARIANT* pSource)
{
    bool ret = false;
    HRESULT hr;

    // Handle JScriptValue objects and JScript out params ( Array object )
    CComVariant varDest( *pDest);

    if( SUCCEEDED( varDest.ChangeType(VT_DISPATCH)))
    {
        CComPtr<IDispatch> spDispDest(varDest.pdispVal);

        // special Handling for a JScriptValue object
        CComQIPtr<IJScriptValueObject> spValueDest(spDispDest);
        if (spValueDest)
        {
            VARIANT_BOOL varBool= VARIANT_FALSE;
            if ((SUCCEEDED(hr = spValueDest->IsOutParam(&varBool))
                 && varBool == VARIANT_TRUE)
                || (SUCCEEDED(hr = spValueDest->IsInOutParam(&varBool))
                    && varBool == VARIANT_TRUE))
            {
                if( SUCCEEDED( spValueDest->Set( CComVariant(), *pSource)))
                    ret= true;
            }
        }
        else if (pDest->vt == VT_DISPATCH)// VT_DISPATCH -> JScript out param
        {
            // We use IDispatchEx because its GetDispID function causes the creation
            // of a property if it does not exist already. This is convenient for
            // out parameters in JScript. Then the user must not specify property "0"
            // explicitly
            CComQIPtr<IDispatchEx> spDispEx( spDispDest);
            if( spDispEx)
            {
                CComBSTR nullProp(L"0");
                DISPID dwDispID;
                if( SUCCEEDED( spDispEx->GetDispID( nullProp, fdexNameEnsure, &dwDispID)))
                {
                    DISPPARAMS dispparams = {nullptr, nullptr, 1, 1};
                    dispparams.rgvarg = pSource;
                    DISPID dispidPut = DISPID_PROPERTYPUT;
                    dispparams.rgdispidNamedArgs = &dispidPut;

                    if (pSource->vt == VT_UNKNOWN || pSource->vt == VT_DISPATCH ||
                        (pSource->vt & VT_ARRAY) || (pSource->vt & VT_BYREF))
                        hr = spDispEx->InvokeEx(dwDispID, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYPUTREF,
                                                &dispparams, nullptr, nullptr, nullptr);
                    else
                        hr= spDispEx->InvokeEx(dwDispID, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYPUT,
                                               &dispparams, nullptr, nullptr, nullptr);
                    if( SUCCEEDED(hr))
                        ret= true;
                }
            }
        }
        else
            ret= writeBackOutParameter( pDest, pSource);
    }
    else // The param can't be a JScript out-parameter ( an Array object), it could be a VBScript
    {   // param. The function checks itself for correct VBScript params
        ret= writeBackOutParameter( pDest, pSource);
    }
    return ret;
}

// VisualBasic Script passes arguments as VT_VARIANT | VT_BYREF be it in or out parameter.
// Thus we are in charge of freeing an eventual value contained by the inner VARIANT
// Please note: VariantCopy doesn't free a VT_BYREF value
// The out parameters are expected to have always a valid type
static bool writeBackOutParameter(VARIANTARG* pDest, VARIANT* pSource)
{
    HRESULT hr;
    bool ret = false;
    // Out parameter must be VT_BYREF
    if ((V_VT(pDest) & VT_BYREF) != 0 )
    {
        VARTYPE oleTypeFlags = V_VT(pSource);

        // if caller accept VARIANT as out parameter, any value must be converted
        if (V_VT(pDest) == (VT_VARIANT | VT_BYREF))
        {
            // When the user provides a VARIANT rather than a concrete type
            // we just copy the source to the out, in/out parameter
            // VT_DISPATCH, VT_UNKNOWN, VT_ARRAY, VT_BSTR in the VARIANT that
            // is contained in pDest are released by VariantCopy
            VariantCopy(V_VARIANTREF(pDest), pSource);
            ret = true;
        }
        else
        {
            // variantarg and variant must have same type
              if ((V_VT(pDest) & oleTypeFlags) == oleTypeFlags)
            {
                if ((oleTypeFlags & VT_ARRAY) != 0)
                {
                    // In / Out Param
                    if( *V_ARRAYREF(pDest) != nullptr)
                        hr= SafeArrayCopyData( V_ARRAY(pSource), *V_ARRAYREF(pDest));
                    else
                        // Out Param
                        hr= SafeArrayCopy(V_ARRAY(pSource), V_ARRAYREF(pDest));
                    if( SUCCEEDED( hr))
                        ret = true;
                }
                else
                {
                    // copy base type
                    switch (V_VT(pSource))
                    {
                    case VT_I2:
                    {
                        *V_I2REF(pDest) = V_I2(pSource);
                        ret = true;
                        break;
                    }
                    case VT_I4:
                        *V_I4REF(pDest) = V_I4(pSource);
                        ret = true;
                        break;
                    case VT_R4:
                        *V_R4REF(pDest) = V_R4(pSource);
                        ret = true;
                        break;
                    case VT_R8:
                        *V_R8REF(pDest) = V_R8(pSource);
                        ret = true;
                        break;
                    case VT_CY:
                        *V_CYREF(pDest) = V_CY(pSource);
                        ret = true;
                        break;
                    case VT_DATE:
                        *V_DATEREF(pDest) = V_DATE(pSource);
                        ret = true;
                        break;
                    case VT_BSTR:
                        SysFreeString( *pDest->pbstrVal);

                        *V_BSTRREF(pDest) = SysAllocString(V_BSTR(pSource));
                        ret = true;
                        break;
                    case VT_DISPATCH:
                        if (*V_DISPATCHREF(pDest) != nullptr)
                            (*V_DISPATCHREF(pDest))->Release();

                        *V_DISPATCHREF(pDest) = V_DISPATCH(pSource);

                        if (*V_DISPATCHREF(pDest) != nullptr)
                            (*V_DISPATCHREF(pDest))->AddRef();

                        ret = true;
                        break;
                    case VT_ERROR:
                        *V_ERRORREF(pDest) = V_ERROR(pSource);
                        ret = true;
                        break;
                    case VT_BOOL:
                        *V_BOOLREF(pDest) = V_BOOL(pSource);
                        ret = true;
                        break;
                    case VT_UNKNOWN:
                        if (*V_UNKNOWNREF(pDest) != nullptr)
                            (*V_UNKNOWNREF(pDest))->Release();

                        *V_UNKNOWNREF(pDest) = V_UNKNOWN(pSource);

                        if (*V_UNKNOWNREF(pDest) != nullptr)
                            (*V_UNKNOWNREF(pDest))->AddRef();

                        ret = true;
                        break;
                    case VT_I1:
                        *V_I1REF(pDest) = V_I1(pSource);
                        ret = true;
                        break;
                    case VT_UI1:
                        *V_UI1REF(pDest) = V_UI1(pSource);
                        ret = true;
                        break;
                    case VT_UI2:
                        *V_UI2REF(pDest) = V_UI2(pSource);
                        ret = true;
                        break;
                    case VT_UI4:
                        *V_UI4REF(pDest) = V_UI4(pSource);
                        ret = true;
                        break;
                    case VT_INT:
                        *V_INTREF(pDest) = V_INT(pSource);
                        ret = true;
                        break;
                    case VT_UINT:
                        *V_UINTREF(pDest) = V_UINT(pSource);
                        ret = true;
                        break;
                    case VT_DECIMAL:
                        memcpy(pDest->pdecVal, pSource, sizeof(DECIMAL));
                        ret = true;
                        break;
                    default:
                        break;
                    }
                }
            }
            else
            {
                // Handling of special cases
                // Destination and source types are different
                if( pDest->vt == (VT_BSTR | VT_BYREF)
                    && pSource->vt == VT_I2)
                {
                    // When the user provides a String as out our in/out parameter
                    // and the type is char (TypeClass_CHAR) then we convert to a BSTR
                    // instead of VT_I2 as is done otherwise
                    OLECHAR buff[]= {0,0};
                    buff[0]= pSource->iVal;

                    SysFreeString( *pDest->pbstrVal);
                    *pDest->pbstrVal= SysAllocString( buff);
                    ret = true;
                }
            }
        }
    }
    return ret;
}

STDMETHODIMP InterfaceOleWrapper::Invoke(DISPID dispidMember,
                                         REFIID /*riid*/,
                                         LCID /*lcid*/,
                                         unsigned short wFlags,
                                         DISPPARAMS * pdispparams,
                                         VARIANT * pvarResult,
                                         EXCEPINFO * pexcepinfo,
                                         unsigned int * puArgErr )
{
    comphelper::Automation::AutomationInvokedZone aAutomationActive;

    OUString sParams;
#if defined SAL_LOG_INFO
    sParams += "[";
    for (unsigned int i = 0; i < pdispparams->cArgs; ++i)
    {
        if (i > 0)
            sParams += ",";
        std::stringstream aStringStream;
        aStringStream << pdispparams->rgvarg[i];
        sParams += OUString::createFromAscii(aStringStream.str().c_str());
    }
    sParams += "]";
#endif
    SAL_INFO("extensions.olebridge", this << "@InterfaceOleWrapper::Invoke(" << dispidMember << "," << sParams << ")");

    comphelper::ProfileZone aZone("COM Bridge");
    HRESULT ret = S_OK;

    try
    {
        bool bHandled= false;
        ret= InvokeGeneral( dispidMember,  wFlags, pdispparams, pvarResult,  pexcepinfo,
                            puArgErr, bHandled);
        if( bHandled)
            return ret;

        if ((dispidMember > 0) && (static_cast<size_t>(dispidMember) <= m_MemberInfos.size()) && m_xInvocation.is())
        {
            MemberInfo d = m_MemberInfos[dispidMember - 1];
            DWORD flags = wFlags & d.flags;

            if (flags != 0)
            {
                if ((flags & DISPATCH_METHOD) != 0)
                {
                    std::unique_ptr<DISPPARAMS> pNewDispParams;
                    std::vector<VARIANTARG> vNewArgs;

                    if (pdispparams->cNamedArgs > 0)
                    {
                        // Convert named arguments to positional ones.

                        // An example:
                        //
                        // Function declaration (in pseudo-code):
                        // int foo(int A, int B, optional int C, optional int D, optional int E, optional int F, optional int G)
                        //
                        // Corresponding parameter numbers (DISPIDs):
                        //             0      1               2               3               4               5               6
                        //
                        // Actual call:
                        // foo(10, 20, E:=50, D:=40, F:=60)
                        //
                        // That is, A and B are passed positionally, D, E, and F as named arguments,
                        // and the optional C and G parameters are left out.
                        //
                        // Incoming DISPPARAMS:
                        //     cArgs=5, cNamedArgs=3
                        //     rgvarg: [60, 40, 50, 20, 10]
                        //     rgdispidNamedArgs: [5, 3, 4]
                        //
                        // We calculate nLowestNamedArgDispid = 3 and nHighestNamedArgDispid = 5.
                        //
                        // Result of conversion, no named args:
                        //     cArgs=6, cNamedArgs=0
                        //     rgvarg: [60, 50, 40, DISP_E_PARAMNOTFOUND, 20, 10]

                        // First find the lowest and highest DISPID of the named arguments.
                        DISPID nLowestNamedArgDispid = 1000000;
                        DISPID nHighestNamedArgDispid = -1;
                        for (unsigned int i = 0; i < pdispparams->cNamedArgs; ++i)
                        {
                            if (pdispparams->rgdispidNamedArgs[i] < nLowestNamedArgDispid)
                                nLowestNamedArgDispid = pdispparams->rgdispidNamedArgs[i];
                            if (pdispparams->rgdispidNamedArgs[i] > nHighestNamedArgDispid)
                                nHighestNamedArgDispid = pdispparams->rgdispidNamedArgs[i];
                        }

                        // Make sure named arguments don't overlap with positional ones. The lowest
                        // DISPID of the named arguments should be >= the number of positional
                        // arguments.
                        if (nLowestNamedArgDispid < static_cast<DISPID>(pdispparams->cArgs - pdispparams->cNamedArgs))
                            return DISP_E_NONAMEDARGS;

                        // Do the actual conversion.
                        pNewDispParams.reset(new DISPPARAMS);
                        vNewArgs.resize(nHighestNamedArgDispid + 1);
                        pNewDispParams->rgvarg = vNewArgs.data();
                        pNewDispParams->rgdispidNamedArgs = nullptr;
                        pNewDispParams->cArgs = nHighestNamedArgDispid + 1;
                        pNewDispParams->cNamedArgs = 0;

                        // Initialise all parameter slots as missing
                        for (int i = 0; i < nHighestNamedArgDispid; ++i)
                        {
                            pNewDispParams->rgvarg[i].vt = VT_ERROR;
                            pNewDispParams->rgvarg[i].scode = DISP_E_PARAMNOTFOUND;
                        }

                        // Then set the value of those actually present.
                        for (unsigned int i = 0; i < pdispparams->cNamedArgs; ++i)
                            pNewDispParams->rgvarg[nHighestNamedArgDispid - pdispparams->rgdispidNamedArgs[i]] = pdispparams->rgvarg[i];

                        const int nFirstUnnamedArg = pdispparams->cNamedArgs + (nLowestNamedArgDispid-(pdispparams->cArgs - pdispparams->cNamedArgs));

                        for (unsigned int i = pdispparams->cNamedArgs; i < pdispparams->cArgs; ++i)
                            pNewDispParams->rgvarg[nFirstUnnamedArg + (i-pdispparams->cNamedArgs)] = pdispparams->rgvarg[i];

                        pdispparams = pNewDispParams.get();
                    }

                    Sequence<Any> params;

                    convertDispparamsArgs(dispidMember, wFlags, pdispparams , params );

                    ret= doInvoke(pdispparams, pvarResult,
                                  pexcepinfo, puArgErr, d.name, params);
                }
                else if ((flags & DISPATCH_PROPERTYGET) != 0)
                {
                    ret=  doGetProperty( pdispparams, pvarResult,
                                         pexcepinfo, d.name);
                }
                else if ((flags & DISPATCH_PROPERTYPUT) != 0 || (flags & DISPATCH_PROPERTYPUTREF) != 0)
                {
                    if (pdispparams->cArgs != 1)
                        ret = DISP_E_BADPARAMCOUNT;
                    else
                    {
                        Sequence<Any> params;
                        convertDispparamsArgs(dispidMember, wFlags, pdispparams, params );
                        if(params.getLength() > 0)
                            ret= doSetProperty( pdispparams, pvarResult, pexcepinfo, puArgErr, d.name, params);
                        else
                            ret = DISP_E_BADVARTYPE;
                    }
                }
            }
            else
                ret= DISP_E_MEMBERNOTFOUND;
        }
        else
            ret = DISP_E_MEMBERNOTFOUND;
    }
    catch(const BridgeRuntimeError& e)
    {
        writeExcepinfo(pexcepinfo, e.message);
        ret = DISP_E_EXCEPTION;
    }
    catch(const Exception& e)
    {
        OUString message= "InterfaceOleWrapper::Invoke : \n" +
                                e.Message;
        writeExcepinfo(pexcepinfo, message);
        ret = DISP_E_EXCEPTION;
    }
    catch(...)
    {
        OUString message= "InterfaceOleWrapper::Invoke : \n"
                          "Unexpected exception";
        writeExcepinfo(pexcepinfo, message);
        ret = DISP_E_EXCEPTION;
    }

    return ret;
}

HRESULT InterfaceOleWrapper::doInvoke( DISPPARAMS * pdispparams, VARIANT * pvarResult,
                              EXCEPINFO * pexcepinfo, unsigned int * puArgErr, OUString& name, Sequence<Any>& params)
{


    HRESULT ret= S_OK;
    try
    {
        Sequence<sal_Int16>     outIndex;
        Sequence<Any>   outParams;
        Any                 returnValue;

        if (pdispparams->cNamedArgs > 0)
            return DISP_E_NONAMEDARGS;

        // invoke method and take care of exceptions
        returnValue = m_xInvocation->invoke(name,
                                            params,
                                            outIndex,
                                            outParams);

        // try to write back out parameter
        if (outIndex.getLength() > 0)
        {
            const sal_Int16* pOutIndex = outIndex.getConstArray();
            const Any* pOutParams = outParams.getConstArray();

            for (sal_Int32 i = 0; i < outIndex.getLength(); i++)
            {
                CComVariant variant;
                // Currently a Sequence is converted to an SafeArray of VARIANTs.
                anyToVariant( &variant, pOutParams[i]);

                // out parameter need special handling if they are VT_DISPATCH
                // and used in JScript
                int outindex= pOutIndex[i];
                writeBackOutParameter2(&(pdispparams->rgvarg[pdispparams->cArgs - 1 - outindex]),
                                       &variant );
            }
        }

        // write back return value
        if (pvarResult != nullptr)
            anyToVariant(pvarResult, returnValue);
    }
    catch(const IllegalArgumentException & e) //XInvocation::invoke
    {
        writeExcepinfo(pexcepinfo, e.Message);
        ret = DISP_E_TYPEMISMATCH;
    }
    catch(const CannotConvertException & e) //XInvocation::invoke
    {
        writeExcepinfo(pexcepinfo, e.Message);
        ret = mapCannotConvertException( e, puArgErr);
    }
    catch(const InvocationTargetException &  e) //XInvocation::invoke
    {
        const Any& org = e.TargetException;
        Exception excTarget;
        org >>= excTarget;
        OUString message=
            org.getValueType().getTypeName() + ": " + excTarget.Message;
        writeExcepinfo(pexcepinfo, message);
        ret = DISP_E_EXCEPTION;
    }
    catch(const NoSuchMethodException & e) //XInvocation::invoke
    {
        writeExcepinfo(pexcepinfo, e.Message);
        ret = DISP_E_MEMBERNOTFOUND;
    }
    catch(const BridgeRuntimeError & e)
    {
        writeExcepinfo(pexcepinfo, e.message);
        ret = DISP_E_EXCEPTION;
    }
    catch(const Exception & e)
    {
        OUString message= "InterfaceOleWrapper::doInvoke : \n" +
                                e.Message;
        writeExcepinfo(pexcepinfo, message);
        ret = DISP_E_EXCEPTION;
    }
    catch( ... )
     {
        OUString message= "InterfaceOleWrapper::doInvoke : \n"
                          "Unexpected exception";
        writeExcepinfo(pexcepinfo, message);
        ret = DISP_E_EXCEPTION;
     }
    return ret;
}

HRESULT InterfaceOleWrapper::doGetProperty( DISPPARAMS * /*pdispparams*/, VARIANT * pvarResult,
                                            EXCEPINFO * pexcepinfo, OUString& name)
{
    HRESULT ret= S_OK;

    try
    {
        Any returnValue = m_xInvocation->getValue( name);
        // write back return value
        if (pvarResult)
            anyToVariant(pvarResult, returnValue);
    }
    catch(const UnknownPropertyException& e) //XInvocation::getValue
    {
        writeExcepinfo(pexcepinfo, e.Message);
        ret = DISP_E_MEMBERNOTFOUND;
    }
    catch(const BridgeRuntimeError& e)
    {
        writeExcepinfo(pexcepinfo, e.message);
        ret = DISP_E_EXCEPTION;
    }
    catch(const Exception& e)
    {
        OUString message= "InterfaceOleWrapper::doGetProperty : \n" +
                                e.Message;
        writeExcepinfo(pexcepinfo, message);
    }
    catch( ... )
    {
        OUString message= "InterfaceOleWrapper::doInvoke : \n"
                          "Unexpected exception";
        writeExcepinfo(pexcepinfo, message);
        ret = DISP_E_EXCEPTION;
    }
    return  ret;
}

HRESULT InterfaceOleWrapper::doSetProperty( DISPPARAMS * /*pdispparams*/, VARIANT * /*pvarResult*/,
                                        EXCEPINFO * pexcepinfo, unsigned int * puArgErr, OUString& name, Sequence<Any> const & params)
{
    HRESULT ret= S_OK;

    try
    {
        m_xInvocation->setValue( name, params.getConstArray()[0]);
    }
    catch(const UnknownPropertyException &)
    {
        ret = DISP_E_MEMBERNOTFOUND;
    }
    catch(const CannotConvertException &e)
    {
        ret= mapCannotConvertException( e, puArgErr);
    }
    catch(const InvocationTargetException &e)
    {
        if (pexcepinfo != nullptr)
        {
            Any org = e.TargetException;

            pexcepinfo->wCode = UNO_2_OLE_EXCEPTIONCODE;
            pexcepinfo->bstrSource = SysAllocString(L"any ONE component");
            pexcepinfo->bstrDescription = SysAllocString(
                o3tl::toW(org.getValueType().getTypeName().getStr()));
        }
        ret = DISP_E_EXCEPTION;
    }
    catch( ... )
    {
        ret= DISP_E_EXCEPTION;
    }
    return ret;
}

class CXEnumVariant : public IEnumVARIANT,
                      public CComObjectRoot
{
public:
    CXEnumVariant()
        : mnIndex(1)            // ooo::vba::XCollection index starts at one
    {
    }

    virtual ~CXEnumVariant()
    {
    }

#if defined __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"
#endif
    BEGIN_COM_MAP(CXEnumVariant)
#if defined __clang__
#pragma clang diagnostic pop
#endif
        COM_INTERFACE_ENTRY(IEnumVARIANT)
#if defined __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winconsistent-missing-override"
#pragma clang diagnostic ignored "-Wunused-function"
#endif
    END_COM_MAP()
#if defined __clang__
#pragma clang diagnostic pop
#endif

    DECLARE_NOT_AGGREGATABLE(CXEnumVariant)

    // Creates and initializes the enumerator
    void Init(InterfaceOleWrapper* pInterfaceOleWrapper,
              const Reference<ooo::vba::XCollection > xCollection)
    {
        mpInterfaceOleWrapper = pInterfaceOleWrapper;
        mxCollection = xCollection;
    }

    // IEnumVARIANT
    virtual HRESULT STDMETHODCALLTYPE Clone(IEnumVARIANT **) override
    {
        SAL_INFO("extensions.olebridge", this << "@CXEnumVariant::Clone: E_NOTIMPL");
        return E_NOTIMPL;
    }

    virtual HRESULT STDMETHODCALLTYPE Next(ULONG const celt,
                                           VARIANT *rgVar,
                                           ULONG *pCeltFetched) override
    {
        comphelper::Automation::AutomationInvokedZone aAutomationActive;

        if (pCeltFetched)
            *pCeltFetched = 0;

        if (celt == 0)
        {
            SAL_INFO("extensions.olebridge", this << "@CXEnumVariant::Next(" << celt << "): E_INVALIDARG");
            return E_INVALIDARG;
        }

        if (rgVar == nullptr || (celt != 1 && pCeltFetched == nullptr))
        {
            SAL_INFO("extensions.olebridge", this << "@CXEnumVariant::Next(" << celt << "): E_FAIL");
            return E_FAIL;
        }

        for (ULONG i = 0; i < celt; i++)
            VariantInit(&rgVar[i]);

        ULONG nLeft = celt;
        ULONG nReturned = 0;
        while (nLeft > 0)
        {
            if (mnIndex > mxCollection->getCount())
            {
                SAL_INFO("extensions.olebridge", this << "@CXEnumVariant::Next(" << celt << "): got " << nReturned << ": S_FALSE");
                return S_FALSE;
            }
            Any aIndex;
            aIndex <<= mnIndex;
            Any aElement = mxCollection->Item(aIndex, Any());
            mpInterfaceOleWrapper->anyToVariant(rgVar, aElement);
            // rgVar->pdispVal->AddRef(); ??
            if (pCeltFetched)
                (*pCeltFetched)++;
            rgVar++;
            nReturned++;
            mnIndex++;
            nLeft--;
        }
        SAL_INFO("extensions.olebridge", this << "@CXEnumVariant::Next(" << celt << "): S_OK");
        return S_OK;
    }

    virtual HRESULT STDMETHODCALLTYPE Reset() override
    {
        SAL_INFO("extensions.olebridge", this << "@CXEnumVariant::Reset: S_OK");
        mnIndex = 1;
        return S_OK;
    }

    virtual HRESULT STDMETHODCALLTYPE STDMETHODCALLTYPE Skip(ULONG const celt) override
    {
        comphelper::Automation::AutomationInvokedZone aAutomationActive;

        ULONG nLeft = celt;
        ULONG nSkipped = 0;
        while (nLeft > 0)
        {
            if (mnIndex > mxCollection->getCount())
            {
                SAL_INFO("extensions.olebridge", this << "@CXEnumVariant::Skip(" << celt << "): skipped " << nSkipped << ": S_FALSE");
                return S_FALSE;
            }
            mnIndex++;
            nLeft--;
        }
        SAL_INFO("extensions.olebridge", this << "@CXEnumVariant::Skip(" << celt << "): S_OK");
        return S_OK;
    }

private:
    InterfaceOleWrapper* mpInterfaceOleWrapper;
    Reference<ooo::vba::XCollection> mxCollection;
    sal_Int32 mnIndex;
};

class Sink : public cppu::WeakImplHelper<ooo::vba::XSink>
{
public:
    Sink(IUnknown* pUnkSink,
         Reference<XMultiServiceFactory> xMSF,
         ooo::vba::TypeAndIID aTypeAndIID,
         InterfaceOleWrapper* pInterfaceOleWrapper);

    // XSink
    void SAL_CALL Call( const OUString& Method, Sequence< Any >& Arguments ) override;

private:
    IUnknown* mpUnkSink;
    Reference<XMultiServiceFactory> mxMSF;
    ooo::vba::TypeAndIID maTypeAndIID;
    InterfaceOleWrapper* mpInterfaceOleWrapper;
};

Sink::Sink(IUnknown* pUnkSink,
           Reference<XMultiServiceFactory> xMSF,
           ooo::vba::TypeAndIID aTypeAndIID,
           InterfaceOleWrapper* pInterfaceOleWrapper) :
    mpUnkSink(pUnkSink),
    mxMSF(xMSF),
    maTypeAndIID(aTypeAndIID),
    mpInterfaceOleWrapper(pInterfaceOleWrapper)
{
    mpUnkSink->AddRef();
}

void SAL_CALL
Sink::Call( const OUString& Method, Sequence< Any >& Arguments )
{
    SAL_INFO("extensions.olebridge", "Sink::Call(" << Method << ", " << Arguments.getLength() << " arguments)");

    IDispatch* pDispatch;
    HRESULT nResult = mpUnkSink->QueryInterface(IID_IDispatch, reinterpret_cast<void **>(&pDispatch));
    if (!SUCCEEDED(nResult))
    {
        SAL_WARN("extensions.olebridge", "Sink::Call: Not IDispatch: " << WindowsErrorStringFromHRESULT(nResult));
        return;
    }

    Reference<XIdlReflection> xRefl = theCoreReflection::get(comphelper::getComponentContext(mxMSF));
    assert(xRefl.is());

    Reference<XIdlClass> xClass = xRefl->forName(maTypeAndIID.Type.getTypeName());
    assert(xClass.is());

    auto aMethods = xClass->getMethods();
    assert(xClass->getTypeClass() == TypeClass_INTERFACE &&
           aMethods.getLength() > 0);

    int nMemId = 1;
    // Skip the three XInterface methods
    for (int i = 3; i < aMethods.getLength(); i++)
    {
        if (aMethods[i]->getName() == Method)
        {
            // FIXME: Handle mismatch in type of actual argument and parameter of the method.

            // FIXME: Handle mismatch in number of arguments passed and actual number of parameters
            // of the method.

            auto aParamInfos = aMethods[i]->getParameterInfos();

            assert(Arguments.getLength() == aParamInfos.getLength());

            DISPPARAMS aDispParams;
            aDispParams.rgdispidNamedArgs = nullptr;
            aDispParams.cArgs = Arguments.getLength();
            aDispParams.cNamedArgs = 0;
            aDispParams.rgvarg = new VARIANT[aDispParams.cArgs];
            for (unsigned j = 0; j < aDispParams.cArgs; j++)
            {
                VariantInit(aDispParams.rgvarg+j);
                // Note: Reverse order of arguments in Arguments and aDispParams.rgvarg!
                const unsigned nIncomingArgIndex = aDispParams.cArgs - j - 1;
                mpInterfaceOleWrapper->anyToVariant(aDispParams.rgvarg+j, Arguments[nIncomingArgIndex]);

                // Handle OUT and INOUT arguments. For instance, the second ('Cancel') parameter to
                // DocumentBeforeClose() should be a VT_BYREF|VT_BOOL parameter. Need to handle that
                // here.

                if (aParamInfos[nIncomingArgIndex].aMode == ParamMode_OUT ||
                    aParamInfos[nIncomingArgIndex].aMode == ParamMode_INOUT)
                {
                    switch (aDispParams.rgvarg[j].vt)
                    {
                    case VT_I2:
                        aDispParams.rgvarg[j].byref = new SHORT(aDispParams.rgvarg[j].iVal);
                        aDispParams.rgvarg[j].vt |= VT_BYREF;
                        break;
                    case VT_I4:
                        aDispParams.rgvarg[j].byref = new LONG(aDispParams.rgvarg[j].lVal);
                        aDispParams.rgvarg[j].vt |= VT_BYREF;
                        break;
                    case VT_BSTR:
                        aDispParams.rgvarg[j].byref = new BSTR(aDispParams.rgvarg[j].bstrVal);
                        aDispParams.rgvarg[j].vt |= VT_BYREF;
                        break;
                    case VT_BOOL:
                        // SAL_ DEBUG("===> VT_BOOL is initially " << (int)aDispParams.rgvarg[j].boolVal);
                        aDispParams.rgvarg[j].byref = new VARIANT_BOOL(aDispParams.rgvarg[j].boolVal);
                        // SAL_ DEBUG("     byref=" << aDispParams.rgvarg[j].byref);
                        aDispParams.rgvarg[j].vt |= VT_BYREF;
                        break;
                    default:
                        assert(false && "Not handled yet");
                        break;
                    }
                }
            }

            VARIANT aVarResult;
            VariantInit(&aVarResult);
            UINT uArgErr;

            // In the case of a VBScript client, which uses "late binding", calling Invoke on the
            // sink it provides will cause a callback to our CXTypeInfo::GetNames for the given
            // member id, and in that we will tell it the name of the corresponding method, and the
            // client will know what event handler to invoke based on that name.
            //
            // As the outgoing interfaces used (ooo::vba::word::XApplicationOutgoing and others) are
            // totally not stable and not published in any way, there can be no client that would
            // have done "compile-time binding" and where the sink would actually be an object with
            // a vtbl corresponding to the outgoing interface. Late binding clients that work like
            // VBScript is all we support.
            SAL_INFO("extensions.olebridge", "Sink::Call(" << Method << "): Calling Invoke(" << nMemId << ")");

            nResult = pDispatch->Invoke(nMemId, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &aDispParams, &aVarResult, nullptr, &uArgErr);
            SAL_INFO("extensions.olebridge", "Sink::Call(" << Method << "): Invoke() returned");

            SAL_WARN_IF(!SUCCEEDED(nResult), "extensions.olebridge", "Call to " << Method << " failed: " << WindowsErrorStringFromHRESULT(nResult));

            // Undo VT_BYREF magic done above. Copy out parameters back to the Anys in Arguments
            for (unsigned j = 0; j < aDispParams.cArgs; j++)
            {
                const unsigned nIncomingArgIndex = aDispParams.cArgs - j - 1;
                if (aParamInfos[nIncomingArgIndex].aMode == ParamMode_OUT ||
                    aParamInfos[nIncomingArgIndex].aMode == ParamMode_INOUT)
                {
                    switch (aDispParams.rgvarg[j].vt)
                    {
                    case VT_BYREF|VT_I2:
                        {
                            SHORT *pI = static_cast<SHORT*>(aDispParams.rgvarg[j].byref);
                            Arguments[nIncomingArgIndex] <<= static_cast<sal_Int16>(*pI);
                            delete pI;
                        }
                        break;
                    case VT_BYREF|VT_I4:
                        {
                            LONG *pL = static_cast<LONG*>(aDispParams.rgvarg[j].byref);
                            Arguments[nIncomingArgIndex] <<= static_cast<sal_Int32>(*pL);
                            delete pL;
                        }
                        break;
                    case VT_BYREF|VT_BSTR:
                        {
                            BSTR *pBstr = static_cast<BSTR*>(aDispParams.rgvarg[j].byref);
                            Arguments[nIncomingArgIndex] <<= OUString(o3tl::toU(*pBstr));
                            // Undo SysAllocString() done in anyToVariant()
                            SysFreeString(*pBstr);
                            delete pBstr;
                        }
                        break;
                    case VT_BYREF|VT_BOOL:
                        {
                            VARIANT_BOOL *pBool = static_cast<VARIANT_BOOL*>(aDispParams.rgvarg[j].byref);
                            // SAL_ DEBUG("===> VT_BOOL: byref is now " << aDispParams.rgvarg[j].byref << ", " << (int)*pBool);
                            Arguments[nIncomingArgIndex] <<= (*pBool != VARIANT_FALSE);
                            delete pBool;
                        }
                        break;
                    default:
                        assert(false && "Not handled yet");
                        break;
                    }
                }
                else
                {
                    switch (aDispParams.rgvarg[j].vt)
                    {
                    case VT_BSTR:
                        // Undo SysAllocString() done in anyToVariant()
                        SysFreeString(aDispParams.rgvarg[j].bstrVal);
                        break;
                    }
                }
            }

            delete[] aDispParams.rgvarg;
            return;
        }
        nMemId++;
    }
    SAL_WARN("extensions.olebridge", "Sink::Call: Unknown method '" << Method << "'");
}

class CXEnumConnections : public IEnumConnections,
                          public CComObjectRoot
{
public:
    CXEnumConnections()
    {
    }

    virtual ~CXEnumConnections()
    {
    }

#if defined __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"
#endif
    BEGIN_COM_MAP(CXEnumConnections)
#if defined __clang__
#pragma clang diagnostic pop
#endif
        COM_INTERFACE_ENTRY(IEnumConnections)
#if defined __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winconsistent-missing-override"
#pragma clang diagnostic ignored "-Wunused-function"
#endif
    END_COM_MAP()
#if defined __clang__
#pragma clang diagnostic pop
#endif

    DECLARE_NOT_AGGREGATABLE(CXEnumConnections)

    void Init(std::vector<IUnknown*>& rUnknowns, std::vector<DWORD>& rCookies)
    {
        SAL_INFO("extensions.olebridge", this << "@CXEnumConnections::Init");
        SAL_WARN_IF(rUnknowns.size() != rCookies.size(), "extensions.olebridge", "Vectors of different size");
        mvUnknowns = rUnknowns;
        mvCookies = rCookies;
        mnIndex = 0;
    }

    virtual HRESULT STDMETHODCALLTYPE Next(ULONG cConnections,
                                           LPCONNECTDATA rgcd,
                                           ULONG *pcFetched) override
    {
        comphelper::Automation::AutomationInvokedZone aAutomationActive;

        if (!rgcd)
        {
            SAL_INFO("extensions.olebridge", this << "@CXEnumConnections::Next(" << cConnections << "): E_POINTER");
            return E_POINTER;
        }

        if (pcFetched && cConnections != 1)
        {
            SAL_INFO("extensions.olebridge", this << "@CXEnumConnections::Next(" << cConnections << "): E_INVALIDARG");
            return E_INVALIDARG;
        }

        ULONG nFetched = 0;
        while (nFetched < cConnections && mnIndex < mvUnknowns.size())
        {
            rgcd[nFetched].pUnk = mvUnknowns[mnIndex];
            rgcd[nFetched].pUnk->AddRef();
            rgcd[nFetched].dwCookie = mvCookies[mnIndex];
            ++nFetched;
            ++mnIndex;
        }
        if (nFetched != cConnections)
        {
            SAL_INFO("extensions.olebridge", this << "@CXEnumConnections::Next(" << cConnections << "): S_FALSE");
            if (pcFetched)
                *pcFetched = nFetched;
            return S_FALSE;
        }
        SAL_INFO("extensions.olebridge", this << "@CXEnumConnections::Next(" << cConnections << "): S_OK");
        if (pcFetched)
            *pcFetched = nFetched;

        return S_OK;
    }

    virtual HRESULT STDMETHODCALLTYPE Skip(ULONG cConnections) override
    {
        SAL_INFO("extensions.olebridge", this << "@CXEnumConnections::Skip(" << cConnections << "): E_NOTIMPL");

        return E_NOTIMPL;
    }

    virtual HRESULT STDMETHODCALLTYPE Reset() override
    {
        SAL_INFO("extensions.olebridge", this << "@CXEnumConnections::Reset: E_NOTIMPL");

        return E_NOTIMPL;
    }

    virtual HRESULT STDMETHODCALLTYPE Clone(IEnumConnections** /* ppEnum */) override
    {
        SAL_INFO("extensions.olebridge", this << "@CXEnumConnections::Clone: E_NOTIMPL");

        return E_NOTIMPL;
    }

private:
    std::vector<IUnknown*> mvUnknowns;
    std::vector<DWORD> mvCookies;
    ULONG mnIndex;
};

class CXConnectionPoint : public IConnectionPoint,
                          public CComObjectRoot
{
public:
#if defined __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"
#endif
    BEGIN_COM_MAP(CXConnectionPoint)
#if defined __clang__
#pragma clang diagnostic pop
#endif
        COM_INTERFACE_ENTRY(IConnectionPoint)
#if defined __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winconsistent-missing-override"
#pragma clang diagnostic ignored "-Wunused-function"
#endif
    END_COM_MAP()
#if defined __clang__
#pragma clang diagnostic pop
#endif

    DECLARE_NOT_AGGREGATABLE(CXConnectionPoint)

    virtual ~CXConnectionPoint() {}

    void Init(InterfaceOleWrapper* pInterfaceOleWrapper,
              Reference<ooo::vba::XConnectionPoint>& xCP,
              Reference<XMultiServiceFactory>& xMSF,
              ooo::vba::TypeAndIID aTypeAndIID)
    {
        SAL_INFO("extensions.olebridge", this << "@CXConnectionPoint::Init for " << pInterfaceOleWrapper->getImplementationName());

        IUnknown *pUnknown;
        if (SUCCEEDED(QueryInterface(IID_IUnknown, reinterpret_cast<void **>(&pUnknown))))
        {
            // In case QI for IUnknown returns a different pointer, but nah, it doesn't
            SAL_INFO("extensions.olebridge", "  (IUnknown@" << pUnknown << ")");
        }

        mpInterfaceOleWrapper = pInterfaceOleWrapper;
        mxCP = xCP;
        mxMSF = xMSF;
        maTypeAndIID = aTypeAndIID;
    }

    virtual HRESULT STDMETHODCALLTYPE GetConnectionInterface(IID *pIID) override
    {
        SAL_WARN("extensions.olebridge", this << "@CXConnectionPoint::GetConnectionInterface(" << *pIID << "): E_NOTIMPL");

        // FIXME: Needed?

        return E_NOTIMPL;
    }

    virtual HRESULT STDMETHODCALLTYPE GetConnectionPointContainer(IConnectionPointContainer **) override
    {
        SAL_WARN("extensions.olebridge", this << "@CXConnectionPoint::GetConnectionInterface: E_NOTIMPL");

        // FIXME: Needed?

        return E_NOTIMPL;
    }

    virtual HRESULT STDMETHODCALLTYPE Advise(IUnknown *pUnkSink,
                                             DWORD *pdwCookie) override
    {
        comphelper::Automation::AutomationInvokedZone aAutomationActive;

        SAL_INFO("extensions.olebridge", this << "@CXConnectionPoint::Advise(" << pUnkSink << ")");

        if (!pdwCookie)
            return E_POINTER;

        Reference<ooo::vba::XSink> xSink(new Sink(pUnkSink, mxMSF, maTypeAndIID, mpInterfaceOleWrapper));

        mvISinks.push_back(pUnkSink);
        *pdwCookie = mvISinks.size();

        mvCookies.push_back(mxCP->Advise(xSink));

        mvXSinks.push_back(xSink);

        SAL_INFO("extensions.olebridge", "  *pdwCookie: " << *pdwCookie);

        return S_OK;
    }

    virtual HRESULT STDMETHODCALLTYPE Unadvise(DWORD dwCookie) override
    {
        comphelper::Automation::AutomationInvokedZone aAutomationActive;

        SAL_INFO("extensions.olebridge", this << "@CXConnectionPoint::Unadvise(" << dwCookie << ")");

        if (dwCookie == 0 || dwCookie > mvISinks.size())
            return E_POINTER;

        mvISinks[dwCookie-1] = nullptr;

        mxCP->Unadvise(mvCookies[dwCookie-1]);

        mvXSinks[dwCookie-1] = Reference<ooo::vba::XSink>();

        return S_OK;
    }

    virtual HRESULT STDMETHODCALLTYPE EnumConnections(IEnumConnections **ppEnum) override
    {
        comphelper::Automation::AutomationInvokedZone aAutomationActive;

        HRESULT nResult;

        SAL_INFO("extensions.olebridge", this << "@CXConnectionPoint::EnumConnections...");

        if (!ppEnum)
        {
            SAL_INFO("extensions.olebridge", "..." << this << "@CXConnectionPoint::EnumConnections: E_POINTER");
            return E_POINTER;
        }

        CComObject<CXEnumConnections>* pEnumConnections;

        nResult = CComObject<CXEnumConnections>::CreateInstance(&pEnumConnections);
        if (FAILED(nResult))
        {
            SAL_INFO("extensions.olebridge", "..." << this << "@CXConnectionPoint::EnumConnections: " << WindowsErrorStringFromHRESULT(nResult));
            return nResult;
        }

        pEnumConnections->AddRef();

        pEnumConnections->Init(mvISinks, mvCookies);
        *ppEnum = pEnumConnections;

        SAL_INFO("extensions.olebridge", "..." << this << "@CXConnectionPoint::EnumConnections: S_OK");

        return S_OK;
    }

    InterfaceOleWrapper* mpInterfaceOleWrapper;
    std::vector<IUnknown*> mvISinks;
    std::vector<Reference<ooo::vba::XSink>> mvXSinks;
    std::vector<DWORD> mvCookies;
    Reference<XMultiServiceFactory> mxMSF;
    Reference<ooo::vba::XConnectionPoint> mxCP;
    ooo::vba::TypeAndIID maTypeAndIID;
};

HRESULT InterfaceOleWrapper::InvokeGeneral( DISPID dispidMember, unsigned short wFlags,
                         DISPPARAMS * pdispparams, VARIANT * pvarResult, EXCEPINFO * pexcepinfo,
                         unsigned int * /*puArgErr*/, bool& bHandled)
{
    HRESULT ret= S_OK;
    try
    {
// DISPID_VALUE | The DEFAULT Value is required in JScript when the situation
// is that we put an object into an Array object ( out parameter). We have to return
// IDispatch otherwise the object cannot be accessed from the Script.
        if( dispidMember == DISPID_VALUE && (wFlags & DISPATCH_PROPERTYGET) != 0
            && m_defaultValueType != VT_EMPTY && pvarResult != nullptr)
        {
            // Special case hack: If it is a ScVbaCheckBox, return the boolean value
            Reference<ooo::vba::msforms::XCheckBox> xCheckBox(m_xOrigin, UNO_QUERY);
            if (xCheckBox.is())
            {
                bHandled = true;
                Any aValue = xCheckBox->getValue();
                anyToVariant(pvarResult, aValue);
                return S_OK;
            }

            bHandled= true;
            if( m_defaultValueType == VT_DISPATCH)
            {
                pvarResult->vt= VT_DISPATCH;
                pvarResult->pdispVal= static_cast<IDispatch*>( this);
                AddRef();
                ret= S_OK;
            }
        }

        // function: _GetValueObject
        else if( dispidMember == DISPID_JSCRIPT_VALUE_FUNC)
        {
            bHandled= true;
            if( !pvarResult)
                return E_POINTER;
            CComObject< JScriptValue>* pValue;
            if( SUCCEEDED( CComObject<JScriptValue>::CreateInstance( &pValue)))
            {
                pValue->AddRef();
                pvarResult->vt= VT_DISPATCH;
                pvarResult->pdispVal= CComQIPtr<IDispatch>(pValue->GetUnknown());
                ret= S_OK;
            }
            else
                ret= DISP_E_EXCEPTION;
        }
        else if( dispidMember == DISPID_GET_STRUCT_FUNC)
        {
            bHandled= true;
            bool bStruct= false;


            Reference<XIdlReflection> xRefl = theCoreReflection::get(comphelper::getComponentContext(m_smgr));
            // the first parameter is in DISPPARAMS rgvargs contains the name of the struct.
            CComVariant arg;
            if( pdispparams->cArgs == 1 && SUCCEEDED( arg.ChangeType( VT_BSTR, &pdispparams->rgvarg[0])) )
            {
                Reference<XIdlClass> classStruct= xRefl->forName(o3tl::toU(arg.bstrVal));
                if( classStruct.is())
                {
                    Any anyStruct;
                    classStruct->createObject( anyStruct);
                    CComVariant var;
                    anyToVariant( &var, anyStruct );

                    if( var.vt == VT_DISPATCH)
                    {
                        VariantCopy( pvarResult, & var);
                        bStruct= true;
                    }
                }
            }
            ret= bStruct ? S_OK : DISP_E_EXCEPTION;
        }
        else if (dispidMember == DISPID_CREATE_TYPE_FUNC)
        {
            bHandled= true;
            if( !pvarResult)
                return E_POINTER;
            // the first parameter is in DISPPARAMS rgvargs contains the name of the struct.
            CComVariant arg;
            if( pdispparams->cArgs != 1)
                return DISP_E_BADPARAMCOUNT;
            if (FAILED( arg.ChangeType( VT_BSTR, &pdispparams->rgvarg[0])))
                return DISP_E_BADVARTYPE;

            //check if the provided name represents a valid type
            Type type;
            if (!getType(arg.bstrVal, type))
            {
                writeExcepinfo(pexcepinfo, "[automation bridge] A UNO type with the name " +
                                           OUString(o3tl::toU(arg.bstrVal)) + " does not exist!");
                return DISP_E_EXCEPTION;
            }

            if (!createUnoTypeWrapper(arg.bstrVal, pvarResult))
            {
                writeExcepinfo(pexcepinfo, "[automation bridge] InterfaceOleWrapper::InvokeGeneral\n"
                                           "Could not initialize UnoTypeWrapper object!");
                return DISP_E_EXCEPTION;
            }
        }
        else if (dispidMember == DISPID_NEWENUM)
        {
            bHandled = true;
            if( !pvarResult)
                return E_POINTER;

            Reference< ooo::vba::XCollection> xCollection(m_xOrigin, UNO_QUERY);
            if (!xCollection.is())
                return DISP_E_MEMBERNOTFOUND;

            CComObject<CXEnumVariant>* pEnumVar;

            ret = CComObject<CXEnumVariant>::CreateInstance(&pEnumVar);
            if (FAILED(ret))
                return ret;

            pEnumVar->AddRef();

            pEnumVar->Init(this, xCollection);

            pvarResult->vt = VT_UNKNOWN;
            pvarResult->punkVal = nullptr;

            ret = pEnumVar->QueryInterface(IID_IUnknown, reinterpret_cast<void**>(&pvarResult->punkVal));
            if (FAILED(ret))
            {
                pEnumVar->Release();
                return ret;
            }
        }
    }
    catch(const BridgeRuntimeError & e)
    {
        writeExcepinfo(pexcepinfo, e.message);
        ret = DISP_E_EXCEPTION;
    }
    catch(const Exception & e)
    {
        OUString message= "InterfaceOleWrapper::InvokeGeneral : \n" +
                                e.Message;
        writeExcepinfo(pexcepinfo, message);
        ret = DISP_E_EXCEPTION;
    }
    catch( ... )
     {
        OUString message= "InterfaceOleWrapper::InvokeGeneral : \n"
                          "Unexpected exception";
        writeExcepinfo(pexcepinfo, message);
        ret = DISP_E_EXCEPTION;
     }
    return ret;
}

STDMETHODIMP InterfaceOleWrapper::GetDispID(BSTR /*bstrName*/, DWORD /*grfdex*/, DISPID __RPC_FAR* /*pid*/)
{
    return ResultFromScode(E_NOTIMPL);
}

STDMETHODIMP InterfaceOleWrapper::InvokeEx(
    /* [in] */ DISPID /*id*/,
    /* [in] */ LCID /*lcid*/,
    /* [in] */ WORD /*wFlags*/,
    /* [in] */ DISPPARAMS __RPC_FAR* /*pdp*/,
    /* [out] */ VARIANT __RPC_FAR* /*pvarRes*/,
    /* [out] */ EXCEPINFO __RPC_FAR* /*pei*/,
    /* [unique][in] */ IServiceProvider __RPC_FAR* /*pspCaller*/)
{
    return ResultFromScode(E_NOTIMPL);
}

STDMETHODIMP InterfaceOleWrapper::DeleteMemberByName(
    /* [in] */ BSTR /*bstr*/,
    /* [in] */ DWORD /*grfdex*/)
{
    return ResultFromScode(E_NOTIMPL);
}

STDMETHODIMP InterfaceOleWrapper::DeleteMemberByDispID(DISPID /*id*/)
{
    return ResultFromScode(E_NOTIMPL);
}

STDMETHODIMP InterfaceOleWrapper::GetMemberProperties(
    /* [in] */ DISPID /*id*/,
    /* [in] */ DWORD /*grfdexFetch*/,
    /* [out] */ DWORD __RPC_FAR* /*pgrfdex*/)
{
    return ResultFromScode(E_NOTIMPL);
}

STDMETHODIMP InterfaceOleWrapper::GetMemberName(
    /* [in] */ DISPID /*id*/,
    /* [out] */ BSTR __RPC_FAR* /*pbstrName*/)
{
    return ResultFromScode(E_NOTIMPL);
}

STDMETHODIMP InterfaceOleWrapper::GetNextDispID(
    /* [in] */ DWORD /*grfdex*/,
    /* [in] */ DISPID /*id*/,
    /* [out] */ DISPID __RPC_FAR* /*pid*/)
{
    return ResultFromScode(E_NOTIMPL);
}

STDMETHODIMP InterfaceOleWrapper::GetNameSpaceParent(
    /* [out] */ IUnknown __RPC_FAR *__RPC_FAR* /*ppunk*/)
{
    return ResultFromScode(E_NOTIMPL);
}

// IProvideClassInfo
HRESULT STDMETHODCALLTYPE InterfaceOleWrapper::GetClassInfo (
    /* [out] */ ITypeInfo **ppTI)
{
    comphelper::Automation::AutomationInvokedZone aAutomationActive;

    SAL_INFO("extensions.olebridge", this << "@InterfaceOleWrapper::GetClassInfo");

    if (!ppTI)
        return E_POINTER;

    Reference<ooo::vba::XInterfaceWithIID> xIID(m_xOrigin, UNO_QUERY);
    if (!xIID.is())
        return E_NOTIMPL;

    OUString sIID = xIID->getIID();
    IID aIID;
    if (!SUCCEEDED(IIDFromString(reinterpret_cast<LPOLESTR>(sIID.pData->buffer), &aIID)))
        return E_NOTIMPL;

    HRESULT ret;

    CComObject<CXTypeInfo>* pTypeInfo;

    ret = CComObject<CXTypeInfo>::CreateInstance(&pTypeInfo);
    if (FAILED(ret))
        return ret;

    pTypeInfo->AddRef();

    pTypeInfo->InitForCoclass(m_xOrigin, m_sImplementationName, aIID, m_smgr);

    *ppTI = pTypeInfo;

    return S_OK;
}

// IConnectionPointContainer
HRESULT STDMETHODCALLTYPE InterfaceOleWrapper::EnumConnectionPoints(
    /* [out] */ IEnumConnectionPoints **)
{
    SAL_INFO("extensions.olebridge", this << "@InterfaceOleWrapper::EnumConnectionPoints");
    return ResultFromScode(E_NOTIMPL);
}

HRESULT STDMETHODCALLTYPE InterfaceOleWrapper::FindConnectionPoint(
    /* [in] */ REFIID riid,
    /* [out] */ IConnectionPoint **ppCP)
{
    comphelper::Automation::AutomationInvokedZone aAutomationActive;

    SAL_INFO("extensions.olebridge", this << "@InterfaceOleWrapper::FindConnectionPoint(" << riid << ")");

    if (!ppCP)
        return E_POINTER;

    Reference<ooo::vba::XConnectable> xConnectable(m_xOrigin, UNO_QUERY);

    // We checked already
    assert(xConnectable.is());
    if (!xConnectable.is())
        return E_NOTIMPL;

    ooo::vba::TypeAndIID aTypeAndIID = xConnectable->GetConnectionPoint();

    IID aIID;
    if (!SUCCEEDED(IIDFromString(reinterpret_cast<LPOLESTR>(aTypeAndIID.IID.pData->buffer), &aIID)))
        return E_INVALIDARG;

    if (!IsEqualIID(riid, aIID))
        return E_INVALIDARG;

    Reference<ooo::vba::XConnectionPoint> xCP = xConnectable->FindConnectionPoint();
    if (!xCP.is())
        return E_INVALIDARG;

    HRESULT ret;

    CComObject<CXConnectionPoint>* pConnectionPoint;

    ret = CComObject<CXConnectionPoint>::CreateInstance(&pConnectionPoint);
    if (FAILED(ret))
        return ret;

    pConnectionPoint->AddRef();

    pConnectionPoint->Init(this, xCP, m_smgr, aTypeAndIID);

    *ppCP = pConnectionPoint;

    return S_OK;
}

// UnoObjectWrapperRemoteOpt ---------------------------------------------------

UnoObjectWrapperRemoteOpt::UnoObjectWrapperRemoteOpt( Reference<XMultiServiceFactory> const & aFactory,
                                                     sal_uInt8 unoWrapperClass, sal_uInt8 comWrapperClass):
InterfaceOleWrapper( aFactory, unoWrapperClass, comWrapperClass),
m_currentId(1)

{
}
UnoObjectWrapperRemoteOpt::~UnoObjectWrapperRemoteOpt()
{
}

// UnoConversionUtilities
Reference< XInterface > UnoObjectWrapperRemoteOpt::createUnoWrapperInstance()
{
    Reference<XWeak> xWeak= static_cast<XWeak*>( new UnoObjectWrapperRemoteOpt(
                                                 m_smgr, m_nUnoWrapperClass, m_nComWrapperClass));
    return Reference<XInterface>( xWeak, UNO_QUERY);
}

STDMETHODIMP  UnoObjectWrapperRemoteOpt::GetIDsOfNames ( REFIID /*riid*/, OLECHAR ** rgszNames, unsigned int cNames,
                                LCID /*lcid*/, DISPID * rgdispid )
{
    MutexGuard guard( getBridgeMutex());

    if( ! rgdispid)
        return E_POINTER;
    HRESULT ret = E_UNEXPECTED;

    // _GetValueObject
    if( ! wcscmp( *rgszNames, JSCRIPT_VALUE_FUNC))
    {
        *rgdispid= DISPID_JSCRIPT_VALUE_FUNC;
        return S_OK;
    }
    else if( ! wcscmp( *rgszNames, GET_STRUCT_FUNC))
    {
        *rgdispid= DISPID_GET_STRUCT_FUNC;
        return S_OK;
    }

    if (m_xInvocation.is() && (cNames > 0))
    {
        OUString name(o3tl::toU(rgszNames[0]));
        // has this name been determined as "bad"
        BadNameMap::iterator badIter= m_badNameMap.find( name);
        if( badIter == m_badNameMap.end() )
        {
            // name has not been bad before( member exists
            typedef NameToIdMap::iterator ITnames;
            pair< ITnames, bool > pair_id= m_nameToDispIdMap.emplace(name, m_currentId++);
            // new ID inserted ?
            if( pair_id.second )
            {// yes, now create MemberInfo and ad to IdToMemberInfoMap
                MemberInfo d(0, name);
                m_idToMemberInfoMap.emplace(m_currentId - 1, d);
            }

            *rgdispid = pair_id.first->second;
            ret = S_OK;
        }
        else
            ret= DISP_E_UNKNOWNNAME;
    }
    return ret;
}

STDMETHODIMP  UnoObjectWrapperRemoteOpt::Invoke ( DISPID dispidMember, REFIID /*riid*/, LCID /*lcid*/, unsigned short wFlags,
                         DISPPARAMS * pdispparams, VARIANT * pvarResult, EXCEPINFO * pexcepinfo,
                         unsigned int * puArgErr )
{
    comphelper::Automation::AutomationInvokedZone aAutomationActive;

    HRESULT ret = S_OK;
    try
    {
        bool bHandled= false;
        ret= InvokeGeneral( dispidMember,  wFlags, pdispparams, pvarResult,  pexcepinfo,
                            puArgErr, bHandled);
        if( bHandled)
            return ret;

        if ( dispidMember > 0  && m_xInvocation.is())
        {

            IdToMemberInfoMap::iterator it_MemberInfo= m_idToMemberInfoMap.find( dispidMember);
            if( it_MemberInfo != m_idToMemberInfoMap.end() )
            {
                MemberInfo& info= it_MemberInfo->second;

                Sequence<Any> params; // holds converted any s
                if( ! info.flags )
                { // DISPID called for the first time
                    if( wFlags == DISPATCH_METHOD )
                    {
                        convertDispparamsArgs(dispidMember, wFlags, pdispparams, params );

                        if( FAILED( ret= doInvoke( pdispparams, pvarResult,
                                                   pexcepinfo, puArgErr, info.name, params))
                            && ret == DISP_E_MEMBERNOTFOUND)
                        {
                            // try to get the exact name
                            OUString exactName;
                            if (m_xExactName.is())
                            {
                                exactName = m_xExactName->getExactName( info.name);
                                // invoke again
                                if( !exactName.isEmpty() )
                                {
                                    if( SUCCEEDED( ret= doInvoke( pdispparams, pvarResult,
                                                                  pexcepinfo, puArgErr, exactName, params)))
                                        info.name= exactName;
                                }
                            }
                        }
                        if( SUCCEEDED( ret ) )
                            info.flags= DISPATCH_METHOD;
                    }
                    else if( wFlags == DISPATCH_PROPERTYPUT || wFlags == DISPATCH_PROPERTYPUTREF)
                    {
                        convertDispparamsArgs(dispidMember, wFlags, pdispparams, params );
                        if( FAILED( ret= doSetProperty( pdispparams, pvarResult,
                                                        pexcepinfo, puArgErr, info.name, params))
                            && ret == DISP_E_MEMBERNOTFOUND)
                        {
                            // try to get the exact name
                            OUString exactName;
                            if (m_xExactName.is())
                            {
                                exactName = m_xExactName->getExactName( info.name);
                                // invoke again
                                if( !exactName.isEmpty() )
                                {
                                    if( SUCCEEDED( ret= doSetProperty( pdispparams, pvarResult,
                                                                       pexcepinfo, puArgErr, exactName, params)))
                                        info.name= exactName;
                                }
                            }
                        }
                        if( SUCCEEDED( ret ) )
                            info.flags= DISPATCH_PROPERTYPUT | DISPATCH_PROPERTYGET;
                    }
                    else if( wFlags == DISPATCH_PROPERTYGET)
                    {
                        if( FAILED( ret= doGetProperty( pdispparams, pvarResult,
                                                        pexcepinfo, info.name))
                            && ret == DISP_E_MEMBERNOTFOUND)
                        {
                            // try to get the exact name
                            OUString exactName;
                            if (m_xExactName.is())
                            {
                                exactName = m_xExactName->getExactName( info.name);
                                // invoke again
                                if( !exactName.isEmpty() )
                                {
                                    if( SUCCEEDED( ret= doGetProperty( pdispparams, pvarResult,
                                                                       pexcepinfo, exactName)))
                                        info.name= exactName;
                                }
                            }
                        }
                        if( SUCCEEDED( ret ) )
                            info.flags= DISPATCH_PROPERTYGET | DISPATCH_PROPERTYPUT;
                    }
                    else if( wFlags & DISPATCH_METHOD &&
                             (wFlags & DISPATCH_PROPERTYPUT || wFlags & DISPATCH_PROPERTYPUTREF))
                    {

                        OUString exactName;
                        // convert params for DISPATCH_METHOD or DISPATCH_PROPERTYPUT
                        convertDispparamsArgs(dispidMember, wFlags, pdispparams, params );
                        // try first as method
                        if( FAILED( ret= doInvoke( pdispparams, pvarResult,
                                                   pexcepinfo, puArgErr, info.name, params))
                            && ret == DISP_E_MEMBERNOTFOUND)
                        {
                            // try to get the exact name
                            if (m_xExactName.is())
                            {
                                exactName = m_xExactName->getExactName( info.name);
                                // invoke again
                                if( !exactName.isEmpty() )
                                {
                                    if( SUCCEEDED( ret= doInvoke( pdispparams, pvarResult,
                                                                  pexcepinfo, puArgErr, exactName, params)))
                                        info.name= exactName;
                                }
                            }
                        }
                        if( SUCCEEDED( ret ) )
                            info.flags= DISPATCH_METHOD;

                        // try as property
                        if( FAILED( ret) && pdispparams->cArgs == 1)
                        {
                            if( FAILED( ret= doSetProperty( pdispparams, pvarResult,
                                                            pexcepinfo, puArgErr, info.name, params))
                                && ret == DISP_E_MEMBERNOTFOUND)
                            {
                                // try to get the exact name
                                if( !exactName.isEmpty() )
                                {
                                    if( SUCCEEDED( ret= doSetProperty( pdispparams, pvarResult,
                                                                       pexcepinfo, puArgErr, exactName, params)))
                                        info.name= exactName;
                                }
                            }
                            if( SUCCEEDED( ret ) )
                                info.flags= DISPATCH_PROPERTYPUT | DISPATCH_PROPERTYGET;
                        }
                    }
                    else if( wFlags & DISPATCH_METHOD && wFlags & DISPATCH_PROPERTYGET)
                    {
                        OUString exactName;
                        convertDispparamsArgs(dispidMember, wFlags, pdispparams, params );

                        if( FAILED( ret= doInvoke( pdispparams, pvarResult,
                                                   pexcepinfo, puArgErr, info.name, params))
                            && ret == DISP_E_MEMBERNOTFOUND)
                        {
                            // try to get the exact name
                            if (m_xExactName.is())
                            {
                                exactName = m_xExactName->getExactName( info.name);
                                // invoke again
                                if( !exactName.isEmpty() )
                                {
                                    if( SUCCEEDED( ret= doInvoke( pdispparams, pvarResult,
                                                                  pexcepinfo, puArgErr, exactName, params)))
                                        info.name= exactName;
                                }
                            }
                        }
                        if( SUCCEEDED( ret ) )
                            info.flags= DISPATCH_METHOD;

                        // try as property
                        if( FAILED( ret) && pdispparams->cArgs == 1)
                        {
                            if( FAILED( ret= doGetProperty( pdispparams, pvarResult,
                                                            pexcepinfo, info.name))
                                && ret == DISP_E_MEMBERNOTFOUND)
                            {
                                if( !exactName.isEmpty() )
                                {
                                    if( SUCCEEDED( ret= doSetProperty( pdispparams, pvarResult,
                                                                       pexcepinfo, puArgErr, exactName, params)))
                                        info.name= exactName;
                                }
                            }
                            if( SUCCEEDED( ret ) )
                                info.flags= DISPATCH_PROPERTYGET;
                        }
                    }

                    // update information about this member
                    if( ret == DISP_E_MEMBERNOTFOUND)
                    {
                        // Remember the name as not existing
                        // and remove the MemberInfo
                        m_badNameMap[info.name]= false;
                        m_idToMemberInfoMap.erase( it_MemberInfo);
                    }
                } // if( ! info.flags )
                else // IdToMemberInfoMap contains a MemberInfo
                {
                    if( wFlags & DISPATCH_METHOD && info.flags == DISPATCH_METHOD)
                    {
                        convertDispparamsArgs(dispidMember, wFlags, pdispparams, params );
                        ret= doInvoke( pdispparams, pvarResult,
                                       pexcepinfo, puArgErr, info.name, params);
                    }
                    else if( (wFlags & DISPATCH_PROPERTYPUT || wFlags & DISPATCH_PROPERTYPUTREF )  &&
                             info.flags & DISPATCH_PROPERTYPUT)
                    {
                        convertDispparamsArgs(dispidMember, wFlags, pdispparams, params );
                        ret= doSetProperty( pdispparams, pvarResult,
                                            pexcepinfo, puArgErr, info.name, params);
                    }
                    else if( (wFlags & DISPATCH_PROPERTYGET) && ( info.flags & DISPATCH_PROPERTYGET))
                    {
                        ret= doGetProperty( pdispparams, pvarResult,
                                            pexcepinfo, info.name);
                    }
                    else
                    {
                        ret= DISP_E_MEMBERNOTFOUND;
                    }
                }
            }//     if( it_MemberInfo != m_idToMemberInfoMap.end() )
            else
                ret= DISP_E_MEMBERNOTFOUND;
        }
    }
    catch(const BridgeRuntimeError& e)
    {
        writeExcepinfo(pexcepinfo, e.message);
        ret = DISP_E_EXCEPTION;
    }
    catch(const Exception& e)
    {
        OUString message= "UnoObjectWrapperRemoteOpt::Invoke : \n" +
            e.Message;
        writeExcepinfo(pexcepinfo, message);
        ret = DISP_E_EXCEPTION;
    }
    catch(...)
    {
        OUString message= "UnoObjectWrapperRemoteOpt::Invoke : \n"
                          "Unexpected exception";
        writeExcepinfo(pexcepinfo, message);
        ret = DISP_E_EXCEPTION;
    }

    return ret;
}

HRESULT UnoObjectWrapperRemoteOpt::methodInvoke( DISPID /*dispidMember*/, DISPPARAMS * /*pdispparams*/, VARIANT * /*pvarResult*/,
                              EXCEPINFO * /*pexcepinfo*/, unsigned int * /*puArgErr*/, Sequence<Any> const &)
{
    return S_OK;
}

// The returned HRESULT is only appropriate for IDispatch::Invoke
static HRESULT mapCannotConvertException(const CannotConvertException &e, unsigned int * puArgErr)
{
    HRESULT ret;
    bool bWriteIndex= true;

    switch ( e.Reason)
    {
        case FailReason::OUT_OF_RANGE:
            ret = DISP_E_OVERFLOW;
            break;
        case FailReason::IS_NOT_NUMBER:
            ret = DISP_E_TYPEMISMATCH;
            break;
        case FailReason::IS_NOT_ENUM:
            ret = DISP_E_TYPEMISMATCH;
            break;
        case FailReason::IS_NOT_BOOL:
            ret = DISP_E_TYPEMISMATCH;
            break;
        case FailReason::NO_SUCH_INTERFACE:
            ret = DISP_E_TYPEMISMATCH;
            break;
        case FailReason::SOURCE_IS_NO_DERIVED_TYPE:
            ret = DISP_E_TYPEMISMATCH;
            break;
        case FailReason::TYPE_NOT_SUPPORTED:
            ret = DISP_E_TYPEMISMATCH;
            break;
        case FailReason::INVALID:
            ret = DISP_E_TYPEMISMATCH;
            break;
        case FailReason::NO_DEFAULT_AVAILABLE:
            ret = DISP_E_BADPARAMCOUNT;
            break;
        case FailReason::UNKNOWN:
            ret = E_UNEXPECTED;
            break;
        default:
            ret = E_UNEXPECTED;
            bWriteIndex= false;
            break;
    }

    if( bWriteIndex &&  puArgErr != nullptr)
        *puArgErr = e.ArgumentIndex;
    return ret;
}

// The function maps the TypeClass of the any to VARTYPE: If
// the Any contains STRUCT or INTERFACE then the return value
// is VT_DISPATCH. The function is used from o2u_createUnoObjectWrapper
// and the result is put into the constructor of the uno - wrapper
// object. If a client asks the object for DISPID_VALUE and this
// function returned VT_DISPATCH then the IDispatch of the same
// object is being returned.
// See InterfaceOleWrapper::Invoke, InterfaceOleWrapper::m_defaultValueType
VARTYPE getVarType( const Any& value)
{
    VARTYPE ret= VT_EMPTY;

    switch ( value.getValueTypeClass())
    {
    case TypeClass_STRUCT: ret= VT_DISPATCH; break;
    case TypeClass_INTERFACE: ret= VT_DISPATCH; break;
    default: break;
    }
    return ret;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
