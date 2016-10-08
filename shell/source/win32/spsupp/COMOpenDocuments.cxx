/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
* This file is part of the LibreOffice project.
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "COMOpenDocuments.hpp"
#include "spsuppServ.hpp"
#include "stdio.h"

const wchar_t* GUID2Str(REFGUID rGUID)
{
    static wchar_t sGUID[40];
    if (::StringFromGUID2(rGUID, sGUID, sizeof(sGUID) / sizeof(sGUID[0])) == 0)
        return nullptr;
    return sGUID;
}

const wchar_t* DWORD2Str(DWORD nVal)
{
    static wchar_t sDWORD[12];
    if (swprintf(sDWORD, sizeof(sDWORD) / sizeof(sDWORD[0]), L"0x%08X", nVal) == -1)
        return nullptr;
    return sDWORD;
}

const wchar_t* VARIANT2Str(const VARIANT& aVal)
{
    static wchar_t sBuf[65536] = {0};
    switch (aVal.vt)
    {
    case VT_EMPTY:
        return L"VT_EMPTY";
    case VT_NULL:
        return L"VT_NULL";
    case VT_I2:
        swprintf(sBuf, sizeof(sBuf) / sizeof(sBuf[0]), L"VT_I2:0x%04X", aVal.iVal);
        break;
    case VT_I4:
        swprintf(sBuf, sizeof(sBuf) / sizeof(sBuf[0]), L"VT_I4:0x%08X", aVal.lVal);
        break;
    case VT_R4:
        swprintf(sBuf, sizeof(sBuf) / sizeof(sBuf[0]), L"VT_R4:%f", aVal.fltVal);
        break;
    case VT_R8:
        swprintf(sBuf, sizeof(sBuf) / sizeof(sBuf[0]), L"VT_R8:%f", aVal.dblVal);
        break;
    case VT_CY:
        return L"VT_CY";
    case VT_DATE:
        return L"VT_DATE";
    case VT_BSTR:
        swprintf(sBuf, sizeof(sBuf) / sizeof(sBuf[0]), L"VT_BSTR:%s", aVal.bstrVal);
        break;
    case VT_DISPATCH:
        return L"VT_DISPATCH";
    case VT_ERROR:
        return L"VT_ERROR";
    case VT_BOOL:
        swprintf(sBuf, sizeof(sBuf) / sizeof(sBuf[0]), L"VT_BOOL:%s", aVal.boolVal ? L"TRUE" : L"FALSE");
        break;
    case VT_VARIANT:
        return L"VT_VARIANT";
    case VT_UNKNOWN:
        return L"VT_UNKNOWN";
    case VT_BYREF | VT_DECIMAL:
        return L"VT_BYREF | VT_DECIMAL";
    case VT_I1:
        swprintf(sBuf, sizeof(sBuf) / sizeof(sBuf[0]), L"VT_I1:0x%02X", aVal.cVal);
        break;
    case VT_BYREF | VT_I1:
        swprintf(sBuf, sizeof(sBuf) / sizeof(sBuf[0]), L"VT_BYREF | VT_I1:0x%02X", *aVal.pcVal);
        break;
    case VT_UI1:
        swprintf(sBuf, sizeof(sBuf) / sizeof(sBuf[0]), L"VT_UI1:0x%02X", aVal.bVal);
        break;
    case VT_UI2:
        swprintf(sBuf, sizeof(sBuf) / sizeof(sBuf[0]), L"VT_UI2:0x%04X", aVal.uiVal);
        break;
    case VT_UI4:
        swprintf(sBuf, sizeof(sBuf) / sizeof(sBuf[0]), L"VT_UI4:0x%08X", aVal.ulVal);
        break;
    case VT_I8:
        swprintf(sBuf, sizeof(sBuf) / sizeof(sBuf[0]), L"VT_I8:0x%016llX", aVal.llVal);
        break;
    case VT_UI8:
        swprintf(sBuf, sizeof(sBuf) / sizeof(sBuf[0]), L"VT_UI8:0x%016llX", aVal.ullVal);
        break;
    case VT_INT:
        swprintf(sBuf, sizeof(sBuf) / sizeof(sBuf[0]), L"VT_INT:0x%08X", aVal.intVal);
        break;
    case VT_UINT:
        swprintf(sBuf, sizeof(sBuf) / sizeof(sBuf[0]), L"VT_UINT:0x%08X", aVal.uintVal);
        break;
    case VT_VOID:
        return L"VT_VOID";
    case VT_HRESULT:
        swprintf(sBuf, sizeof(sBuf) / sizeof(sBuf[0]), L"VT_HRESULT:0x%08X", aVal.lVal);
        break;
    case VT_PTR:
        swprintf(sBuf, sizeof(sBuf) / sizeof(sBuf[0]), L"VT_PTR:0x%08X", aVal.lVal);
        break;
    case VT_SAFEARRAY:
        return L"VT_SAFEARRAY";
    case VT_CARRAY:
        return L"VT_CARRAY";
    case VT_USERDEFINED:
        return L"VT_USERDEFINED";
    case VT_LPSTR:
        swprintf(sBuf, sizeof(sBuf) / sizeof(sBuf[0]), L"VT_LPSTR:%S", aVal.pcVal);
        break;
    case VT_LPWSTR:
        swprintf(sBuf, sizeof(sBuf) / sizeof(sBuf[0]), L"VT_LPWSTR:%s", aVal.bstrVal);
        break;
    case VT_RECORD:
        return L"VT_RECORD";
    case VT_INT_PTR:
        return L"VT_INT_PTR";
    case VT_UINT_PTR:
        return L"VT_UINT_PTR";
    case VT_FILETIME:
        return L"VT_FILETIME";
    case VT_BLOB:
        return L"VT_BLOB";
    case VT_STREAM:
        return L"VT_STREAM";
    case VT_STORAGE:
        return L"VT_STORAGE";
    case VT_STREAMED_OBJECT:
        return L"VT_STREAMED_OBJECT";
    case VT_STORED_OBJECT:
        return L"VT_STORED_OBJECT";
    case VT_BLOB_OBJECT:
        return L"VT_BLOB_OBJECT";
    case VT_CF:
        return L"VT_CF";
    case VT_CLSID:
        return L"VT_CLSID";
    case VT_VERSIONED_STREAM:
        return L"VT_VERSIONED_STREAM";
    case VT_BSTR_BLOB:
        return L"VT_BSTR_BLOB";
    case VT_VECTOR:
        return L"VT_VECTOR";
    case VT_ARRAY:
        return L"VT_ARRAY";
    case VT_BYREF:
        return L"VT_BYREF";
    case VT_RESERVED:
        return L"VT_RESERVED";
    case VT_ILLEGAL:
        return L"VT_ILLEGAL";
    default:
        swprintf(sBuf, sizeof(sBuf) / sizeof(sBuf[0]), L"Unknown (0x%08X)", aVal.vt);
        break;
    }
    return sBuf;
}

#pragma warning (push)
#pragma warning (disable : 4996)
void ReportInvoke(ITypeInfo* pTypeInfo, DISPID dispIdMember, DISPPARAMS *pDispParams)
{
    BSTR bstrNames[16] = { nullptr };
    UINT cNames;
    pTypeInfo->GetNames(dispIdMember, bstrNames, sizeof(bstrNames) / sizeof(bstrNames[0]), &cNames);
    wchar_t sBuf[65536];
    wcscpy(sBuf, bstrNames[0]);
    ::SysFreeString(bstrNames[0]);
    wcscat(sBuf, L"(");
    for (UINT i = 1; i < cNames; ++i)
    {
        wcscat(sBuf, bstrNames[i]);
        ::SysFreeString(bstrNames[i]);
        if (i<cNames - 1)
            wcscat(sBuf, L", ");
    }
    wcscat(sBuf, L")\n");
    if (pDispParams->cArgs > 0)
    {
        wcscat(sBuf, L"\nUnnamed args:\n");
        for (UINT i = 0; i < pDispParams->cArgs; ++i)
        {
            wcscat(sBuf, VARIANT2Str(pDispParams->rgvarg[i]));
            wcscat(sBuf, L"\n");
        }
    }
    if (pDispParams->cNamedArgs > 0)
    {
        wcscat(sBuf, L"\nNamed args:\n");
        for (UINT i = 0; i < pDispParams->cNamedArgs; ++i)
        {
            pTypeInfo->GetNames(pDispParams->rgdispidNamedArgs[i], bstrNames, sizeof(bstrNames) / sizeof(bstrNames[0]), &cNames);
            wcscat(sBuf, bstrNames[0]);
            wcscat(sBuf, L"\n");
            for (UINT j = 0; j < cNames; ++j)
                ::SysFreeString(bstrNames[j]);
        }
    }
    MessageBoxW(nullptr, sBuf, L"COMOpenDocuments::Invoke", MB_ICONINFORMATION);
}
#pragma warning(pop)

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
HRESULT LOStart(wchar_t* sCommandLine)
{
    STARTUPINFOW si = { sizeof(si) };
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_SHOW;
    PROCESS_INFORMATION pi = {};
    if (CreateProcessW(nullptr, sCommandLine, nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi) == FALSE)
    {
        DWORD dwError = GetLastError();
        wchar_t* sMsgBuf;
        FormatMessageW(
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr,
            dwError,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPWSTR)&sMsgBuf,
            0, nullptr);

        size_t nBufSize = wcslen(sMsgBuf) + 100;
        wchar_t* sDisplayBuf = new wchar_t[nBufSize];
        swprintf(sDisplayBuf, nBufSize, L"Could not start LibreOffice. Error is 0x%08X:\n\n%s", dwError, sMsgBuf);
        LocalFree(sMsgBuf);

        // Report the error to user and return error
        MessageBoxW(nullptr, sDisplayBuf, nullptr, MB_ICONERROR);
        delete[](sDisplayBuf);
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

bool COMOpenDocuments::COMObjectSafety::SetSafe_forUntrustedCaller(bool bSafe)
{
    if (GetSafe_forUntrustedCaller() != bSafe)
    {
        SetMaskedOptions(INTERFACESAFE_FOR_UNTRUSTED_CALLER, bSafe ? 0xFFFFFFFF : 0);
    }
    return true;
}

bool COMOpenDocuments::COMObjectSafety::SetSafe_forUntrustedData(bool bSafe)
{
    if (GetSafe_forUntrustedData() != bSafe)
    {
        SetMaskedOptions(INTERFACESAFE_FOR_UNTRUSTED_DATA, bSafe ? 0xFFFFFFFF : 0);
    }
    return true;
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
    reinterpret_cast<IUnknown*>(ppvo)->Release();
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
        if (!SetSafe_forUntrustedCaller(dwEnabledOptions & INTERFACESAFE_FOR_UNTRUSTED_CALLER))
            return E_FAIL;
    }

    if (dwOptionSetMask & INTERFACESAFE_FOR_UNTRUSTED_DATA)
    {
        if (!SetSafe_forUntrustedData((dwEnabledOptions & INTERFACESAFE_FOR_UNTRUSTED_DATA) != 0))
            return E_FAIL;
    }

    return S_OK;
}

long COMOpenDocuments::m_nObjCount = 0;
ITypeInfo* COMOpenDocuments::m_pTypeInfo = nullptr;
wchar_t COMOpenDocuments::m_szLOPath[MAX_PATH] = {0};

COMOpenDocuments::COMOpenDocuments()
    : m_aObjectSafety(this)
{
    ::InterlockedIncrement(&m_nObjCount);
    if (m_pTypeInfo == nullptr)
    {
        ITypeLib* pITypeLib = GetTypeLib();
        if (FAILED(pITypeLib->GetTypeInfoOfGuid(__uuidof(IOWSNewDocument3), &m_pTypeInfo)))
            throw 0; // TODO: throw sensible object
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

    reinterpret_cast<IUnknown*>(*ppvObject)->AddRef();
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
    BSTR /*bstrTemplateLocation*/,    // A string that contains the URL of the document template from which the document is created, or the programmatic identifier (progID) of the application to invoke when creating the document
    BSTR /*bstrDefaultSaveLocation*/, // A string that contains the path that specifies a suggested default location for saving the new document
    VARIANT_BOOL* /*pbResult*/)       // true if the document creation succeeds; otherwise false
{
    MessageBoxW(nullptr, L"COMOpenDocuments::CreateNewDocument2", L"Information", MB_ICONINFORMATION);
    // TODO
    return E_NOTIMPL;
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
    MessageBoxW(nullptr, L"COMOpenDocuments::PromptedOnLastOpen", L"Information", MB_ICONINFORMATION);
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
    IDispatch* /*pdisp*/,      // An Object that represents the window from which the ViewDocmument3 method is being activated
    BSTR bstrDocumentLocation, // A string that contains the URL of the document to open for reading
    int /*OpenType*/,          // A Long integer that specifies the rights for opening the document
    VARIANT /*varProgID*/,     // An optional string that contains the ProgID of the application with which to open the document. If this argument is omitted, the default viewer for the document is used
    VARIANT_BOOL *pbResult)    // true if the document was successfully opened; otherwise false
{
    // TODO: resolve the program from varProgID (nullptr -> default?)
    const wchar_t* sProgram = GetLOPath();
    if (m_aObjectSafety.GetSafe_forUntrustedCaller() || m_aObjectSafety.GetSafe_forUntrustedData())
    {
        if (!SecurityWarning(sProgram, bstrDocumentLocation))
        {
            // Set result to true and return success to avoid downloading in browser
            *pbResult = TRUE;
            return S_OK;
        }
    }
    wchar_t sCommandLine[32768];
    swprintf(sCommandLine, sizeof(sCommandLine) / sizeof(*sCommandLine), L"\"%s\" --view \"%s\"", sProgram, bstrDocumentLocation);
    HRESULT hr = LOStart(sCommandLine);
    *pbResult = SUCCEEDED(hr);
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
    MessageBoxW(nullptr, L"COMOpenDocuments::CheckinDocument", L"Information", MB_ICONINFORMATION);
    // TODO
    return E_NOTIMPL;
}

// Discards the check out of a document to the client computer and deletes the local draft
STDMETHODIMP COMOpenDocuments::DiscardLocalCheckout(
    BSTR /*bstrDocumentLocationRaw*/, // A string that contains the URL of the document
    VARIANT_BOOL* /*pbResult*/)       // true if the operation to discard the local checkout of the document is successful; otherwise, false
{
    MessageBoxW(nullptr, L"COMOpenDocuments::DiscardLocalCheckout", L"Information", MB_ICONINFORMATION);
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
    MessageBoxW(nullptr, L"COMOpenDocuments::CheckoutDocumentPrompt", L"Information", MB_ICONINFORMATION);
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
    const wchar_t* sProgram = GetLOPath();
    if (m_aObjectSafety.GetSafe_forUntrustedCaller() || m_aObjectSafety.GetSafe_forUntrustedData())
    {
        if (!SecurityWarning(sProgram, bstrDocumentLocation))
        {
            // Set result to true and return success to avoid downloading in browser
            *pbResult = TRUE;
            return S_OK;
        }
    }
    wchar_t sCommandLine[32768];
    swprintf(sCommandLine, sizeof(sCommandLine) / sizeof(*sCommandLine), L"\"%s\" -o \"%s\"", sProgram, bstrDocumentLocation);
    HRESULT hr = LOStart(sCommandLine);
    *pbResult = SUCCEEDED(hr);
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
