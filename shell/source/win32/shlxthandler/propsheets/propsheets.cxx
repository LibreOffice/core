/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include <config.hxx>
#include <global.hxx>

#include <propsheets.hxx>
#include <utilities.hxx>
#include <resource.h>
#include "listviewbuilder.hxx"

#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#include <shellapi.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif

#include <memory>
#include <string>
#include <vector>
#include <utility>
#include <strsafe.h>


/*---------------------------------------------
    INFO - INFO - INFO - INFO - INFO - INFO

    See MSDN "Using Windows XP Visual Styles"
    for hints how to enable the new common
    control library for our property sheet.

    INFO - INFO - INFO - INFO - INFO - INFO
----------------------------------------------*/


CPropertySheet::CPropertySheet(long RefCnt) :
    m_RefCnt(RefCnt)
{
    OutputDebugStringFormatW(L"CPropertySheet::CTor [%d], [%d]", m_RefCnt, g_DllRefCnt );
    InterlockedIncrement(&g_DllRefCnt);
}


CPropertySheet::~CPropertySheet()
{
    OutputDebugStringFormatW(L"CPropertySheet::DTor [%d], [%d]", m_RefCnt, g_DllRefCnt );
    InterlockedDecrement(&g_DllRefCnt);
}


// IUnknown methods


HRESULT STDMETHODCALLTYPE CPropertySheet::QueryInterface(
    REFIID riid, void __RPC_FAR *__RPC_FAR *ppvObject)
{
    *ppvObject = nullptr;

    IUnknown* pUnk = nullptr;
    if (IID_IUnknown == riid || IID_IShellExtInit == riid)
    {
        pUnk = static_cast<IShellExtInit*>(this);
        pUnk->AddRef();
        *ppvObject = pUnk;
        return S_OK;
    }
    else if (IID_IShellPropSheetExt == riid)
    {
        pUnk = static_cast<IShellPropSheetExt*>(this);
        pUnk->AddRef();
        *ppvObject = pUnk;
        return S_OK;
    }

    return E_NOINTERFACE;
}


ULONG STDMETHODCALLTYPE CPropertySheet::AddRef()
{
    OutputDebugStringFormatW(L"CPropertySheet::AddRef [%d]", m_RefCnt );
    return InterlockedIncrement(&m_RefCnt);
}


ULONG STDMETHODCALLTYPE CPropertySheet::Release()
{
    OutputDebugStringFormatW(L"CPropertySheet::Release [%d]", m_RefCnt );
    long refcnt = InterlockedDecrement(&m_RefCnt);

    if (0 == refcnt)
        delete this;

    return refcnt;
}


// IShellExtInit


HRESULT STDMETHODCALLTYPE CPropertySheet::Initialize(
    LPCITEMIDLIST /*pidlFolder*/, LPDATAOBJECT lpdobj, HKEY /*hkeyProgID*/)
{
    InitCommonControls();

    STGMEDIUM medium;
    FORMATETC fe = {CF_HDROP, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};

    HRESULT hr = lpdobj->GetData(&fe, &medium);

    // save the file name
    if (SUCCEEDED(hr) &&
        (1 == DragQueryFileW(
            static_cast<HDROP>(medium.hGlobal),
            0xFFFFFFFF,
            nullptr,
            0)))
    {
        UINT size = DragQueryFileW( static_cast<HDROP>(medium.hGlobal), 0, nullptr, 0 );
        if ( size != 0 )
        {
            auto buffer = std::unique_ptr<WCHAR[]>(new WCHAR[ size + 1 ]);
            UINT result_size = DragQueryFileW( static_cast<HDROP>(medium.hGlobal),
                                               0, buffer.get(), size + 1 );
            if ( result_size != 0 )
            {
                std::wstring fname = getShortPathName( buffer.get() );
                ZeroMemory( m_szFileName, sizeof( m_szFileName ) );
                wcsncpy( m_szFileName, fname.c_str(), ( SAL_N_ELEMENTS( m_szFileName ) - 1 ) );
                hr = S_OK;
            }
            else
                hr = E_INVALIDARG;
        }
        else
            hr = E_INVALIDARG;
    }
    else
        hr = E_INVALIDARG;

    ReleaseStgMedium(&medium);

    return hr;
}


// IShellPropSheetExt


HRESULT STDMETHODCALLTYPE CPropertySheet::AddPages(LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam)
{
    std::wstring proppage_header;

    PROPSHEETPAGE psp;
    ZeroMemory(&psp, sizeof(PROPSHEETPAGEA));

    // add the summary property page
    psp.dwSize      = sizeof(PROPSHEETPAGE);
    psp.dwFlags     = PSP_DEFAULT | PSP_USETITLE | PSP_USECALLBACK;
    psp.hInstance   = GetModuleHandleW(MODULE_NAME);
    psp.lParam      = reinterpret_cast<LPARAM>(this);
    psp.pfnCallback = reinterpret_cast<LPFNPSPCALLBACK>(CPropertySheet::PropPageSummaryCallback);

    HPROPSHEETPAGE hPage = nullptr;

    // add the statistics property page
    proppage_header = GetResString(IDS_PROPPAGE_STATISTICS_TITLE);

    psp.pszTemplate = MAKEINTRESOURCE(IDD_PROPPAGE_STATISTICS);
    psp.pszTitle    = proppage_header.c_str();
    psp.pfnDlgProc  = reinterpret_cast<DLGPROC>(CPropertySheet::PropPageStatisticsProc);

    hPage = CreatePropertySheetPage(&psp);

    if (hPage)
    {
        if (lpfnAddPage(hPage, lParam))
            AddRef();
        else
            DestroyPropertySheetPage(hPage);
    }

    // always return success else no property sheet will be displayed at all
    return NOERROR;
}


HRESULT STDMETHODCALLTYPE CPropertySheet::ReplacePage(
    UINT /*uPageID*/, LPFNADDPROPSHEETPAGE /*lpfnReplaceWith*/, LPARAM /*lParam*/)
{
    return E_NOTIMPL;
}


UINT CALLBACK CPropertySheet::PropPageSummaryCallback(
    HWND /*hwnd*/, UINT uMsg, LPPROPSHEETPAGE ppsp)
{
    CPropertySheet* pImpl =
        reinterpret_cast<CPropertySheet*>(ppsp->lParam);

    // release this instance, acquired in the AddPages method
    if (PSPCB_RELEASE == uMsg)
    {
        pImpl->Release();
    }

    return TRUE;
}


BOOL CALLBACK CPropertySheet::PropPageSummaryProc(HWND hwnd, UINT uiMsg, WPARAM /*wParam*/, LPARAM lParam)
{
    switch (uiMsg)
    {
    case WM_INITDIALOG:
        {
            LPPROPSHEETPAGE psp = reinterpret_cast<LPPROPSHEETPAGE>(lParam);
            CPropertySheet* pImpl = reinterpret_cast<CPropertySheet*>(psp->lParam);
            pImpl->InitPropPageSummary(hwnd, psp);
            return TRUE;
        }
    }

    return FALSE;
}


BOOL CALLBACK CPropertySheet::PropPageStatisticsProc(HWND hwnd, UINT uiMsg, WPARAM /*wParam*/, LPARAM lParam)
{
    switch (uiMsg)
    {
    case WM_INITDIALOG:
        {
            LPPROPSHEETPAGE psp = reinterpret_cast<LPPROPSHEETPAGE>(lParam);
            CPropertySheet* pImpl = reinterpret_cast<CPropertySheet*>(psp->lParam);
            pImpl->InitPropPageStatistics(hwnd, psp);
            return TRUE;
        }
    }

    return FALSE;
}

void CPropertySheet::InitPropPageSummary(HWND hwnd, LPPROPSHEETPAGE /*lppsp*/)
{
    try
    {
        CMetaInfoReader metaInfo(m_szFileName);

        SetWindowTextW(GetDlgItem(hwnd,IDC_TITLE),    metaInfo.getTagData( META_INFO_TITLE ).c_str() );
        SetWindowTextW(GetDlgItem(hwnd,IDC_AUTHOR),   metaInfo.getTagData( META_INFO_AUTHOR ).c_str() );
        SetWindowTextW(GetDlgItem(hwnd,IDC_SUBJECT),  metaInfo.getTagData( META_INFO_SUBJECT ).c_str() );
        SetWindowTextW(GetDlgItem(hwnd,IDC_KEYWORDS), metaInfo.getTagData( META_INFO_KEYWORDS ).c_str() );

        // comments read from meta.xml use "\n" for return, but this will not displayable in Edit control, add
        // "\r" before "\n" to form "\r\n" in order to display return in Edit control.
        std::wstring tempStr = metaInfo.getTagData( META_INFO_DESCRIPTION );
        std::wstring::size_type itor = tempStr.find ( L"\n" , 0 );
        while (itor != std::wstring::npos)
        {
            tempStr.insert(itor, L"\r");
            itor = tempStr.find(L"\n", itor + 2);
        }
        SetWindowTextW(GetDlgItem(hwnd,IDC_COMMENTS), tempStr.c_str());
    }
    catch (const std::exception&)
    {
    }
}


void CPropertySheet::InitPropPageStatistics(HWND hwnd, LPPROPSHEETPAGE /*lppsp*/)
{
    try
    {
        CMetaInfoReader metaInfo(m_szFileName);

        document_statistic_reader_ptr doc_stat_reader = create_document_statistic_reader(m_szFileName, &metaInfo);

        statistic_group_list_t sgl;
        doc_stat_reader->read(&sgl);

        list_view_builder_ptr lv_builder = create_list_view_builder(
            GetDlgItem(hwnd, IDC_STATISTICSLIST),
            GetResString(IDS_PROPERTY),
            GetResString(IDS_PROPERTY_VALUE));

        lv_builder->build(sgl);
    }
    catch (const std::exception&)
    {
    }
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
