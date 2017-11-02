/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
* This file is part of the LibreOffice project.
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include <sal/config.h>

#include <cstring>
#include <vector>

#include <COMOpenDocuments.hpp>
#include <spsuppServ.hpp>
#include <stdio.h>

// Display confirmation dialog, return false on negative answer
bool SecurityWarning(const wchar_t* sProgram, const wchar_t* sDocument)
{
    // TODO: change wording (currently taken from MS Office), use LO localization
    wchar_t sBuf[65536];
    swprintf(sBuf, sizeof(sBuf) / sizeof(sBuf[0]),
        L"Some files contain viruses that can be harmful to your computer. It is important to be certain that this file is from a trustworthy source.\n\n"
        L"Do you want to open this file ?\n\n"
        L"Program : %s\n\n"
        L"Address : %s", sProgram, sDocument);
    return (MessageBoxW(nullptr, sBuf, L"LibreOffice SharePoint integration", MB_YESNO | MB_ICONWARNING) == IDYES);
}

// Returns S_OK if successful
HRESULT LOStart(const wchar_t* sModeArg, const wchar_t* sFilePath, bool bDoSecurityWarning)
{
    const wchar_t* sProgram = GetLOPath();
    if (bDoSecurityWarning && !SecurityWarning(sProgram, sFilePath))
    {
        // Return success to avoid downloading in browser
        return S_OK;
    }

    STARTUPINFOW si;
    std::memset(&si, 0, sizeof si);
    si.cb = sizeof si;
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_SHOW;
    PROCESS_INFORMATION pi = {};
    const size_t cchCommandLine = 32768;
    wchar_t sCommandLine[cchCommandLine];
    swprintf(sCommandLine, cchCommandLine, L"\"%s\" %s \"%s\"", sProgram, sModeArg, sFilePath);
    if (CreateProcessW(nullptr, sCommandLine, nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi) == FALSE)
    {
        DWORD dwError = GetLastError();
        wchar_t* sMsgBuf = nullptr;
        FormatMessageW(
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr,
            dwError,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            reinterpret_cast<LPWSTR>(&sMsgBuf),
            0, nullptr);

        size_t nBufSize = wcslen(sMsgBuf) + 100;
        std::vector<wchar_t> sDisplayBuf(nBufSize);
        swprintf(sDisplayBuf.data(), nBufSize, L"Could not start LibreOffice. Error is 0x%08X:\n\n%s", dwError, sMsgBuf);
        HeapFree(GetProcessHeap(), 0, sMsgBuf);

        // Report the error to user and return error
        MessageBoxW(nullptr, sDisplayBuf.data(), nullptr, MB_ICONERROR);
        return HRESULT_FROM_WIN32(dwError);
    }
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return S_OK;
}

// IObjectSafety methods

void COMOpenDocuments::COMObjectSafety::SetMaskedOptions(DWORD iMask, DWORD iOptions)
{
    m_iEnabledOptions &= ~iMask;
    m_iEnabledOptions |= (iOptions & iMask);
}

void COMOpenDocuments::COMObjectSafety::SetSafe_forUntrustedCaller(bool bSafe)
{
    if (GetSafe_forUntrustedCaller() != bSafe)
    {
        SetMaskedOptions(INTERFACESAFE_FOR_UNTRUSTED_CALLER, bSafe ? 0xFFFFFFFF : 0);
    }
}

void COMOpenDocuments::COMObjectSafety::SetSafe_forUntrustedData(bool bSafe)
{
    if (GetSafe_forUntrustedData() != bSafe)
    {
        SetMaskedOptions(INTERFACESAFE_FOR_UNTRUSTED_DATA, bSafe ? 0xFFFFFFFF : 0);
    }
}

HRESULT STDMETHODCALLTYPE COMOpenDocuments::COMObjectSafety::GetInterfaceSafetyOptions(
    REFIID riid,
    DWORD *pdwSupportedOptions,
    DWORD *pdwEnabledOptions)
{
    void* ppvo;
    HRESULT hr = m_pOwner->QueryInterface(riid, &ppvo);
    if (FAILED(hr))
    {
        return hr;
    }

    // We know about it; release reference and return required information
    static_cast<IUnknown*>(ppvo)->Release();
    *pdwSupportedOptions = iSupportedOptionsMask;
    *pdwEnabledOptions = m_iEnabledOptions;
    return S_OK;
}

HRESULT STDMETHODCALLTYPE COMOpenDocuments::COMObjectSafety::SetInterfaceSafetyOptions(
    REFIID /*riid*/,
    DWORD dwOptionSetMask,
    DWORD dwEnabledOptions)
{
    // Are there unsupported options in mask?
    if (dwOptionSetMask & ~iSupportedOptionsMask)
        return E_FAIL;

    if (dwOptionSetMask & INTERFACESAFE_FOR_UNTRUSTED_CALLER)
    {
        SetSafe_forUntrustedCaller(dwEnabledOptions & INTERFACESAFE_FOR_UNTRUSTED_CALLER);
    }

    if (dwOptionSetMask & INTERFACESAFE_FOR_UNTRUSTED_DATA)
    {
        SetSafe_forUntrustedData((dwEnabledOptions & INTERFACESAFE_FOR_UNTRUSTED_DATA) != 0);
    }

    return S_OK;
}

long COMOpenDocuments::m_nObjCount = 0;
ITypeInfo* COMOpenDocuments::m_pTypeInfo = nullptr;

COMOpenDocuments::COMOpenDocuments()
    : m_aObjectSafety(this)
{
    ::InterlockedIncrement(&m_nObjCount);
    if (m_pTypeInfo == nullptr)
    {
        ITypeLib* pITypeLib = GetTypeLib();
        HRESULT hr = pITypeLib->GetTypeInfoOfGuid(__uuidof(IOWSNewDocument3), &m_pTypeInfo);
        if (FAILED(hr))
            throw Error(hr);
    }
}

COMOpenDocuments::~COMOpenDocuments()
{
    if (::InterlockedDecrement(&m_nObjCount) == 0 && m_pTypeInfo)
    {
        m_pTypeInfo->Release();
        m_pTypeInfo = nullptr;
    }
}

// IUnknown methods

STDMETHODIMP COMOpenDocuments::QueryInterface(REFIID riid, void **ppvObject)
{
    *ppvObject = nullptr;
    if (IsEqualIID(riid, __uuidof(IUnknown)) ||
        IsEqualIID(riid, __uuidof(IDispatch)) ||
        IsEqualIID(riid, __uuidof(IOWSNewDocument)) ||
        IsEqualIID(riid, __uuidof(IOWSNewDocument2)) ||
        IsEqualIID(riid, __uuidof(IOWSNewDocument3)))
    {
        *ppvObject = this;
    }
    else if (IsEqualIID(riid, __uuidof(IObjectSafety)))
    {
        *ppvObject = &m_aObjectSafety;
    }
    else
    {
        return E_NOINTERFACE;
    }

    static_cast<IUnknown*>(*ppvObject)->AddRef();
    return S_OK;
}

// IDispatch methods

STDMETHODIMP COMOpenDocuments::GetTypeInfoCount(UINT *pctinfo)
{
    if (pctinfo == nullptr)
        return E_INVALIDARG;

    *pctinfo = 1;
    return S_OK;
}

STDMETHODIMP COMOpenDocuments::GetTypeInfo(UINT iTInfo, LCID /*lcid*/, ITypeInfo **ppTInfo)
{
    if (ppTInfo == nullptr)
        return E_INVALIDARG;
    *ppTInfo = nullptr;

    if (iTInfo != 0)
        return DISP_E_BADINDEX;

    (*ppTInfo = m_pTypeInfo)->AddRef();
    return S_OK;
}

STDMETHODIMP COMOpenDocuments::GetIDsOfNames(
    REFIID /*riid*/,
    LPOLESTR *rgszNames,
    UINT cNames,
    LCID /*lcid*/,
    DISPID *rgDispId)
{
    return m_pTypeInfo->GetIDsOfNames(rgszNames, cNames, rgDispId);
}

STDMETHODIMP COMOpenDocuments::Invoke(
    DISPID dispIdMember,
    REFIID /*riid*/, // IID_NULL (see https://msdn.microsoft.com/en-us/library/windows/desktop/ms221479)
    LCID /*lcid*/,
    WORD wFlags,
    DISPPARAMS *pDispParams,
    VARIANT *pVarResult,
    EXCEPINFO *pExcepInfo,
    UINT *puArgErr)
{
//    ReportInvoke(m_pTypeInfo, dispIdMember, pDispParams);
    return DispInvoke(this, m_pTypeInfo, dispIdMember, wFlags, pDispParams, pVarResult, pExcepInfo, puArgErr);
}

// IOWSNewDocument methods

// Creates a document based on the specified document template
STDMETHODIMP COMOpenDocuments::CreateNewDocument(
    BSTR bstrTemplateLocation,    // A string that contains the URL of the document template from which the document is created, or the programmatic identifier (progID) of the application to invoke when creating the document
    BSTR bstrDefaultSaveLocation, // A string that contains the path that specifies a suggested default location for saving the new document
    VARIANT_BOOL *pbResult)       // true if the document creation succeeds; otherwise false
{
    return CreateNewDocument2(nullptr, bstrTemplateLocation, bstrDefaultSaveLocation, pbResult);
}

// Opens the specified document for editing with its associated application
// or with the specified editor
STDMETHODIMP COMOpenDocuments::EditDocument(
    BSTR bstrDocumentLocation, // A string that contains the URL of the document to open for editing
    VARIANT varProgID,         // An optional string that contains the ProgID of the application with which to edit the document. If this argument is omitted, the default editor for the document is used
    VARIANT_BOOL *pbResult)    // true if the document was successfully opened; otherwise false
{
    return EditDocument3(nullptr, bstrDocumentLocation, FALSE, varProgID, pbResult);
}

// IOWSNewDocument2 methods

// Opens the document for reading instead of editing, so that the document is not locked on the server
//
// Use the ViewDocument method to open a document in its appropriate application,
// instead of inside of an application instance embedded within the browser
STDMETHODIMP COMOpenDocuments::ViewDocument(
    BSTR bstrDocumentLocation, // A string that contains the URL of the document to open for reading
    VARIANT varProgID,         // An optional string that contains the ProgID of the application with which to open the document. If this argument is omitted, the default viewer for the document is used
    VARIANT_BOOL *pbResult)    // true if the document was successfully opened; otherwise false
{
    return ViewDocument3(nullptr, bstrDocumentLocation, 0, varProgID, pbResult);
}

// Opens the document for reading instead of editing, so that the document
// is not locked on the server and in a specified window
//
// Use the ViewDocument method to open a document in its appropriate application,
// instead of inside of an application instance embedded within the browser
STDMETHODIMP COMOpenDocuments::ViewDocument2(
    IDispatch *pdisp,          // An Object that represents the window from which the ViewDocument2 method is being activated
    BSTR bstrDocumentLocation, // A string that contains the URL of the document to open for reading
    VARIANT varProgID,         // An optional string that contains the ProgID of the application with which to open the document. If this argument is omitted, the default viewer for the document is used
    VARIANT_BOOL *pbResult)    // true if the document was successfully opened; otherwise false
{
    return ViewDocument3(pdisp, bstrDocumentLocation, 0, varProgID, pbResult);
}

// Opens the specified document for editing with its associated application
// or with the specified editor based on the specified window object
STDMETHODIMP COMOpenDocuments::EditDocument2(
    IDispatch *pdisp,          // An Object that represents the window from which the EditDocument2 method is being activated
    BSTR bstrDocumentLocation, // A string that contains the URL of the document to open for editing
    VARIANT varProgID,         // An optional string that contains the ProgID of the application with which to edit the document. If this argument is omitted, the default editor for the document is used
    VARIANT_BOOL *pbResult)    // true if the document was successfully opened; otherwise false
{
    return EditDocument3(pdisp, bstrDocumentLocation, FALSE, varProgID, pbResult);
}

// Creates a document based on the specified document template and window object
STDMETHODIMP COMOpenDocuments::CreateNewDocument2(
    IDispatch* /*pdisp*/,             // An Object that represents the window from which the CreateNewDocument2 method is being activated
    BSTR bstrTemplateLocation,        // A string that contains the URL of the document template from which the document is created, or the programmatic identifier (progID) of the application to invoke when creating the document
    BSTR /*bstrDefaultSaveLocation*/, // A string that contains the path that specifies a suggested default location for saving the new document
    VARIANT_BOOL* pbResult)           // true if the document creation succeeds; otherwise false
{
    // TODO: resolve the program from varProgID (nullptr -> default?)
    HRESULT hr = LOStart(L"-n", bstrTemplateLocation, m_aObjectSafety.GetSafe_forUntrustedCaller() || m_aObjectSafety.GetSafe_forUntrustedData());
    *pbResult = VARIANT_BOOL(SUCCEEDED(hr));
    return hr;
}

// Used with the OpenDocuments.CreateNewDocument2 Method method to determine
// whether the security dialog box that appears when a document is opened has already appeared
//
// If the PromptedOnLastOpen method returns true, the window containing the document library view
// refreshes itself the next time it receives focus. One refresh can occur after the new document
// is saved to the server
STDMETHODIMP COMOpenDocuments::PromptedOnLastOpen(
    VARIANT_BOOL* /*pbResult*/) // true if the security dialog box that appears when a document is opened has already appeared; otherwise false
{
    // TODO
    return E_NOTIMPL;
}

// IOWSNewDocument3 methods

// Opens the document for reading instead of editing, so that the document
// is not locked on the server in a specified window, and with a specified type
//
// The following table shows possible values for OpenType
//
// 0 When checked out, or when the document library does not require check out, the user can read or edit the document
// 1 When another user has checked it out, the user can only read the document
// 2 When the current user has checked it out, the user can only edit the document
// 3 When the document is not checked out and the document library requires that documents be checked out to be edited, the user can only read the document, or check it out and edit it
// 4 When the current user has checked it out, the user can only edit the local copy of the document
STDMETHODIMP COMOpenDocuments::ViewDocument3(
    IDispatch* /*pdisp*/,      // An Object that represents the window from which the ViewDocument3 method is being activated
    BSTR bstrDocumentLocation, // A string that contains the URL of the document to open for reading
    int /*OpenType*/,          // A Long integer that specifies the rights for opening the document
    VARIANT /*varProgID*/,     // An optional string that contains the ProgID of the application with which to open the document. If this argument is omitted, the default viewer for the document is used
    VARIANT_BOOL *pbResult)    // true if the document was successfully opened; otherwise false
{
    // TODO: resolve the program from varProgID (nullptr -> default?)
    HRESULT hr = LOStart(L"--view", bstrDocumentLocation, m_aObjectSafety.GetSafe_forUntrustedCaller() || m_aObjectSafety.GetSafe_forUntrustedData());
    *pbResult = VARIANT_BOOL(SUCCEEDED(hr));
    return hr;
}

// Checks in the specified document to a library
STDMETHODIMP COMOpenDocuments::CheckinDocument(
    BSTR /*bstrDocumentLocation*/,  // A string that contains the URL of the document to check in
    int /*CheckinType*/,            // A Long that specifies the type of check-in, where 0 = minor check-in, 1 = major check-in, and 2 = overwrite check-in
    BSTR /*CheckinComment*/,        // A string that contains a comment for checking in the document
    VARIANT_BOOL /*bKeepCheckout*/, // Optional. true to check in changes that have been made to the document yet keep the document checked out; otherwise, false. The default value is false
    VARIANT_BOOL* /*pbResult*/)     // true if the document is successfully checked in; otherwise, false
{
    // TODO
    return E_NOTIMPL;
}

// Discards the check out of a document to the client computer and deletes the local draft
STDMETHODIMP COMOpenDocuments::DiscardLocalCheckout(
    BSTR /*bstrDocumentLocationRaw*/, // A string that contains the URL of the document
    VARIANT_BOOL* /*pbResult*/)       // true if the operation to discard the local checkout of the document is successful; otherwise, false
{
    // TODO
    return E_NOTIMPL;
}

// Deprecated. Returns E_NOTIMPL
STDMETHODIMP COMOpenDocuments::ViewInExcel(
    BSTR /*SiteUrl*/,
    BSTR /*FileName*/,
    BSTR /*SessionId*/,
    BSTR /*Cmd*/,
    BSTR /*Sheet*/,
    int /*Row*/,
    int /*Column*/,
    VARIANT /*varProgID*/)
{
    return E_NOTIMPL;
}

// Checks out a document from a library
STDMETHODIMP COMOpenDocuments::CheckoutDocumentPrompt(
    BSTR /*bstrDocumentLocationRaw*/,    // A string that contains the URL of the document to check out
    VARIANT_BOOL /*fEditAfterCheckout*/, // true to open the document in an editing application; otherwise, false
    VARIANT /*varProgID*/,               // An optional string that contains the ProgID of the application that is used to work with the document. If this argument is omitted, the default application for the document is used
    VARIANT_BOOL* /*pbResult*/)          // true if the document is successfully checked out; otherwise, false
{
    // TODO
    return E_NOTIMPL;
}

// Opens the specified document for editing with its associated application
// or with the specified editor based on the specified window object,
// and specifies whether to use a local copy
STDMETHODIMP COMOpenDocuments::EditDocument3(
    IDispatch* /*pdisp*/,           // An Object that represents the window from which the EditDocument3 method is being activated
    BSTR bstrDocumentLocation,      // A string that contains the URL of the document to open for editing
    VARIANT_BOOL /*fUseLocalCopy*/, // true to use a local copy; otherwise false
    VARIANT /*varProgID*/,          // An optional string that contains the ProgID of the application with which to edit the document. If this argument is omitted, the default editor for the document is used
    VARIANT_BOOL *pbResult)     // true if the document was successfully opened; otherwise false
{
    // TODO: resolve the program from varProgID (nullptr -> default?)
    HRESULT hr = LOStart(L"-o", bstrDocumentLocation, m_aObjectSafety.GetSafe_forUntrustedCaller() || m_aObjectSafety.GetSafe_forUntrustedData());
    *pbResult = VARIANT_BOOL(SUCCEEDED(hr));
    return hr;
}

// Creates a new blog post in the editing application
STDMETHODIMP COMOpenDocuments::NewBlogPost(
    BSTR /*bstrProviderId*/, // A string that contains the GUID of the blog provider
    BSTR /*bstrBlogUrl*/,    // A string that contains the absolute URL of the blog site
    BSTR /*bstrBlogName*/)   // A string that contains the GUID of the blog site and the GUID of the post list separated by the pound sign (#)
{
    return E_NOTIMPL;
}

long COMOpenDocuments::GetObjectCount() { return m_nObjCount; }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
