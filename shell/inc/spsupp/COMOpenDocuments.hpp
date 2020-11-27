/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
* This file is part of the LibreOffice project.
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef INCLUDED_SHELL_INC_SPSUPP_COMOPENDOCUMENTS_HPP
#define INCLUDED_SHELL_INC_SPSUPP_COMOPENDOCUMENTS_HPP

#if defined __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnon-virtual-dtor"
#endif
#include <spsupp_h.h>
#if defined __clang__
#pragma clang diagnostic pop
#endif
#include "COMRefCounted.hpp"
#include <Objsafe.h>
#include <assert.h>

class COMOpenDocuments : public COMRefCounted<IOWSNewDocument3, IObjectSafety>
{
public:

    class Error {
    public:
        Error(HRESULT syserr) : m_nErr(syserr) {}
        HRESULT val() const { return m_nErr; }
    private:
        HRESULT m_nErr;
    };

    COMOpenDocuments();
    virtual ~COMOpenDocuments() override;

    // IUnknown methods

    HRESULT STDMETHODCALLTYPE QueryInterface(
        REFIID riid,
        void **ppvObject) override;

    // IDispatch methods

    HRESULT STDMETHODCALLTYPE GetTypeInfoCount(
        UINT *pctinfo) override;

    HRESULT STDMETHODCALLTYPE GetTypeInfo(
        UINT iTInfo,
        LCID lcid,
        ITypeInfo **ppTInfo) override;

    HRESULT STDMETHODCALLTYPE GetIDsOfNames(
        REFIID riid,
        LPOLESTR *rgszNames,
        UINT cNames,
        LCID lcid,
        DISPID *rgDispId) override;

    HRESULT STDMETHODCALLTYPE Invoke(
        DISPID dispIdMember,
        REFIID riid,
        LCID lcid,
        WORD wFlags,
        DISPPARAMS *pDispParams,
        VARIANT *pVarResult,
        EXCEPINFO *pExcepInfo,
        UINT *puArgErr) override;

    // IOWSNewDocument methods https://msdn.microsoft.com/en-us/library/cc264316

    HRESULT STDMETHODCALLTYPE CreateNewDocument(
        BSTR bstrTemplateLocation,
        BSTR bstrDefaultSaveLocation,
        VARIANT_BOOL *pbResult) override;

    HRESULT STDMETHODCALLTYPE EditDocument(
        BSTR bstrDocumentLocation,
        VARIANT varProgID,
        VARIANT_BOOL *pbResult) override;

    // IOWSNewDocument2 methods

    HRESULT STDMETHODCALLTYPE ViewDocument(
        BSTR bstrDocumentLocation,
        VARIANT varProgID,
        VARIANT_BOOL *pbResult) override;

    HRESULT STDMETHODCALLTYPE ViewDocument2(
        IDispatch *pdisp,
        BSTR bstrDocumentLocation,
        VARIANT varProgID,
        VARIANT_BOOL *pbResult) override;

    HRESULT STDMETHODCALLTYPE EditDocument2(
        IDispatch *pdisp,
        BSTR bstrDocumentLocation,
        VARIANT varProgID,
        VARIANT_BOOL *pbResult) override;

    HRESULT STDMETHODCALLTYPE CreateNewDocument2(
        IDispatch *pdisp,
        BSTR bstrTemplateLocation,
        BSTR bstrDefaultSaveLocation,
        VARIANT_BOOL *pbResult) override;

    HRESULT STDMETHODCALLTYPE PromptedOnLastOpen(
        VARIANT_BOOL *pbResult) override;

    // IOWSNewDocument3 methods

    HRESULT STDMETHODCALLTYPE ViewDocument3(
        IDispatch *pdisp,
        BSTR bstrDocumentLocation,
        int OpenType,
        VARIANT varProgID,
        VARIANT_BOOL *pbResult) override;

    HRESULT STDMETHODCALLTYPE CheckinDocument(
        BSTR bstrDocumentLocation,
        int CheckinType,
        BSTR CheckinComment,
        VARIANT_BOOL bKeepCheckout,
        VARIANT_BOOL *pbResult) override;

    HRESULT STDMETHODCALLTYPE DiscardLocalCheckout(
        BSTR bstrDocumentLocationRaw,
        VARIANT_BOOL *pbResult) override;

    HRESULT STDMETHODCALLTYPE ViewInExcel(
        BSTR SiteUrl,
        BSTR FileName,
        BSTR SessionId,
        BSTR Cmd,
        BSTR Sheet,
        int Row,
        int Column,
        VARIANT varProgID) override;

    HRESULT STDMETHODCALLTYPE CheckoutDocumentPrompt(
        BSTR bstrDocumentLocationRaw,
        VARIANT_BOOL fEditAfterCheckout,
        VARIANT varProgID,
        VARIANT_BOOL *pbResult) override;

    HRESULT STDMETHODCALLTYPE EditDocument3(
        IDispatch *pdisp,
        BSTR bstrDocumentLocation,
        VARIANT_BOOL fUseLocalCopy,
        VARIANT varProgID,
        VARIANT_BOOL *pbResult) override;

    HRESULT STDMETHODCALLTYPE NewBlogPost(
        BSTR bstrProviderId,
        BSTR bstrBlogUrl,
        BSTR bstrBlogName) override;

    // IObjectSafety methods

    HRESULT STDMETHODCALLTYPE GetInterfaceSafetyOptions(
        REFIID riid,
        DWORD *pdwSupportedOptions,
        DWORD *pdwEnabledOptions) override;

    HRESULT STDMETHODCALLTYPE SetInterfaceSafetyOptions(
        REFIID riid,
        DWORD dwOptionSetMask,
        DWORD dwEnabledOptions) override;

    // Non-COM methods

    static LONG GetObjectCount();

private:
    static LONG m_nObjCount;
    static ITypeInfo* m_pTypeInfo;
    static constexpr DWORD iSupportedOptionsMask = INTERFACESAFE_FOR_UNTRUSTED_CALLER | INTERFACESAFE_FOR_UNTRUSTED_DATA;
    DWORD m_iEnabledOptions = 0;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
