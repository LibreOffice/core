/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: propsheets.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:04:10 $
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

#ifndef CONFIG_HXX_INCLUDED
#include "internal/config.hxx"
#endif

#ifndef GLOBAL_HXX_INCLUDED
#include "internal/global.hxx"
#endif

#ifndef PROPSEETS_HXX_INCLUDED
#include "internal/propsheets.hxx"
#endif

#ifndef UTILITIES_HXX_INCLUDED
#include "internal/utilities.hxx"
#endif

#ifndef RESOURCE_H_INCLUDED
#include "internal/resource.h"
#endif

#ifndef LISTVIEWBUILDER_HXX_INCLUDED
#include "listviewbuilder.hxx"
#endif

#include <shellapi.h>

#include <string>
#include <vector>
#include <utility>


/*---------------------------------------------
    INFO - INFO - INFO - INFO - INFO - INFO

    See MSDN "Using Windows XP Visual Styles"
    for hints how to enable the new common
    control library for our property sheet.

    INFO - INFO - INFO - INFO - INFO - INFO
----------------------------------------------*/

//-----------------------------
//
//-----------------------------

CPropertySheet::CPropertySheet(long RefCnt) :
    m_RefCnt(RefCnt)
{
    InterlockedIncrement(&g_DllRefCnt);
}

//-----------------------------
//
//-----------------------------

CPropertySheet::~CPropertySheet()
{
    InterlockedDecrement(&g_DllRefCnt);
}

//-----------------------------
// IUnknown methods
//-----------------------------

HRESULT STDMETHODCALLTYPE CPropertySheet::QueryInterface(
    REFIID riid, void __RPC_FAR *__RPC_FAR *ppvObject)
{
    *ppvObject = 0;

    IUnknown* pUnk = 0;
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

//-----------------------------
//
//-----------------------------

ULONG STDMETHODCALLTYPE CPropertySheet::AddRef(void)
{
    return InterlockedIncrement(&m_RefCnt);
}

//-----------------------------
//
//-----------------------------

ULONG STDMETHODCALLTYPE CPropertySheet::Release(void)
{
    long refcnt = InterlockedDecrement(&m_RefCnt);

    if (0 == refcnt)
        delete this;

    return refcnt;
}

//-----------------------------
// IShellExtInit
//-----------------------------

HRESULT STDMETHODCALLTYPE CPropertySheet::Initialize(
    LPCITEMIDLIST /*pidlFolder*/, LPDATAOBJECT lpdobj, HKEY /*hkeyProgID*/)
{
    InitCommonControls();

    STGMEDIUM medium;
    FORMATETC fe = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};

    HRESULT hr = lpdobj->GetData(&fe, &medium);

    // save the file name
    if (SUCCEEDED(hr) &&
        (1 == DragQueryFileA(
            reinterpret_cast<HDROP>(medium.hGlobal),
            0xFFFFFFFF,
            NULL,
            0)))
    {
        DragQueryFileA(
            reinterpret_cast<HDROP>(medium.hGlobal),
            0,
            m_szFileName,
            sizeof(m_szFileName));

        hr = S_OK;
    }
    else
        hr = E_INVALIDARG;

    ReleaseStgMedium(&medium);

    return hr;
}

//-----------------------------
// IShellPropSheetExt
//-----------------------------

HRESULT STDMETHODCALLTYPE CPropertySheet::AddPages(LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam)
{
    try
    {
        m_pMetaInfo = std::auto_ptr<CMetaInfoReader>(new CMetaInfoReader(m_szFileName));
    }
    catch (const std::exception&)
    {
        return E_FAIL;
    }

    PROPSHEETPAGE psp;

    // add the summary property page

    ZeroMemory(&psp, sizeof(PROPSHEETPAGEA));

    std::wstring proppage_header = GetResString(IDS_PROPPAGE_SUMMARY_TITLE);

    psp.dwSize      = sizeof(PROPSHEETPAGE);
    psp.dwFlags     = PSP_DEFAULT | PSP_USETITLE | PSP_USECALLBACK;
    psp.hInstance   = GetModuleHandle(MODULE_NAME);
    psp.pszTemplate = MAKEINTRESOURCE(IDD_PROPPAGE_SUMMARY);
    psp.pszTitle    = proppage_header.c_str();
    psp.pfnDlgProc  = reinterpret_cast<DLGPROC>(CPropertySheet::PropPageSummaryProc);
    psp.lParam      = reinterpret_cast<LPARAM>(this);
    psp.pfnCallback = reinterpret_cast<LPFNPSPCALLBACK>(CPropertySheet::PropPageSummaryCallback);

    HPROPSHEETPAGE hPage = CreatePropertySheetPage(&psp);

    // keep this instance alive
    // will be released when the
    // the page is about to be
    // destroyed in the callback
    // function

    if (hPage)
    {
        if (lpfnAddPage(hPage, lParam))
            AddRef();
        else
            DestroyPropertySheetPage(hPage);
    }

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

    // always return success else
    // no property sheet will be
    // displayed at all
    return NOERROR;
}

//-----------------------------
//
//-----------------------------

HRESULT STDMETHODCALLTYPE CPropertySheet::ReplacePage(
    UINT /*uPageID*/, LPFNADDPROPSHEETPAGE /*lpfnReplaceWith*/, LPARAM /*lParam*/)
{
    return E_NOTIMPL;
}

//-----------------------------
//
//-----------------------------

UINT CALLBACK CPropertySheet::PropPageSummaryCallback(
    HWND /*hwnd*/, UINT uMsg, LPPROPSHEETPAGE ppsp)
{
    CPropertySheet* pImpl =
        reinterpret_cast<CPropertySheet*>(ppsp->lParam);

    // release this instance, acquired
    // in the AddPages method
    if (PSPCB_RELEASE == uMsg)
    {
        pImpl->Release();
    }

    return TRUE;
}


//-----------------------------
//
//-----------------------------

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

//-----------------------------
//
//-----------------------------

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

//##################################
void CPropertySheet::InitPropPageSummary(HWND hwnd, LPPROPSHEETPAGE /*lppsp*/)
{
    SetWindowText(GetDlgItem(hwnd,IDC_TITLE),    m_pMetaInfo->getTagData( META_INFO_TITLE ).c_str() );
    SetWindowText(GetDlgItem(hwnd,IDC_AUTHOR),   m_pMetaInfo->getTagData( META_INFO_AUTHOR ).c_str() );
    SetWindowText(GetDlgItem(hwnd,IDC_SUBJECT),  m_pMetaInfo->getTagData( META_INFO_SUBJECT ).c_str() );
    SetWindowText(GetDlgItem(hwnd,IDC_KEYWORDS), m_pMetaInfo->getTagData( META_INFO_KEYWORDS ).c_str() );

    // comments read from meta.xml use "\n" for return, but this will not displayable in Edit control, add
    // "\r" before "\n" to form "\r\n" in order to display return in Edit control.
    std::wstring tempStr = m_pMetaInfo->getTagData( META_INFO_DESCRIPTION ).c_str();
    std::wstring::size_type itor = tempStr.find ( L"\n" , 0 );
    while (itor != std::wstring::npos)
    {
        tempStr.insert(itor, L"\r");
        itor = tempStr.find(L"\n", itor + 2);
    }
    SetWindowText(GetDlgItem(hwnd,IDC_COMMENTS), tempStr.c_str());
}

//---------------------------------
/**
*/
void CPropertySheet::InitPropPageStatistics(HWND hwnd, LPPROPSHEETPAGE /*lppsp*/)
{
    document_statistic_reader_ptr doc_stat_reader = create_document_statistic_reader(m_szFileName, m_pMetaInfo.get());

    statistic_group_list_t sgl;
    doc_stat_reader->read(&sgl);

    list_view_builder_ptr lv_builder = create_list_view_builder(
        GetDlgItem(hwnd, IDC_STATISTICSLIST),
        GetResString(IDS_PROPERTY),
        GetResString(IDS_PROPERTY_VALUE));

    lv_builder->build(sgl);
}




