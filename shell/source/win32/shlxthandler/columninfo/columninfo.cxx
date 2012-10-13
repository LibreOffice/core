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

#include "internal/global.hxx"
#include "internal/columninfo.hxx"
#include "internal/fileextensions.hxx"
#include "internal/metainforeader.hxx"
#include "internal/utilities.hxx"
#include "internal/config.hxx"

#include <sal/macros.h>
#include <malloc.h>

#if defined GCC
#pragma GCC diagnostic warning "-Wmissing-braces"
#endif

//----------------------------

namespace /* private */
{
    SHCOLUMNINFO ColumnInfoTable[] =
    {
        {{PSGUID_SUMMARYINFORMATION, PIDSI_TITLE},    VT_BSTR, LVCFMT_LEFT, 30, SHCOLSTATE_TYPE_STR, L"Title",    L"Title"},
        {{PSGUID_SUMMARYINFORMATION, PIDSI_AUTHOR},   VT_BSTR, LVCFMT_LEFT, 30, SHCOLSTATE_TYPE_STR, L"Author",   L"Author"},
        {{PSGUID_SUMMARYINFORMATION, PIDSI_SUBJECT},  VT_BSTR, LVCFMT_LEFT, 30, SHCOLSTATE_TYPE_STR, L"Subject",  L"Subject"},
        {{PSGUID_SUMMARYINFORMATION, PIDSI_KEYWORDS}, VT_BSTR, LVCFMT_LEFT, 30, SHCOLSTATE_TYPE_STR, L"Keywords", L"Keywords"},
        {{PSGUID_SUMMARYINFORMATION, PIDSI_COMMENTS}, VT_BSTR, LVCFMT_LEFT, 30, SHCOLSTATE_TYPE_STR, L"Comments", L"Comments"},
        {{PSGUID_SUMMARYINFORMATION, PIDSI_PAGECOUNT},VT_BSTR, LVCFMT_LEFT, 30, SHCOLSTATE_TYPE_STR, L"Pagecount", L"Pagecount"}
    };

    size_t ColumnInfoTableSize = sizeof(ColumnInfoTable)/sizeof(ColumnInfoTable[0]);
}

//----------------------------

CColumnInfo::CColumnInfo(long RefCnt) :
    m_RefCnt(RefCnt)
{
    InterlockedIncrement(&g_DllRefCnt);
}

//----------------------------

CColumnInfo::~CColumnInfo()
{
    InterlockedDecrement(&g_DllRefCnt);
}

//-----------------------------
// IUnknown methods
//-----------------------------

HRESULT STDMETHODCALLTYPE CColumnInfo::QueryInterface(REFIID riid, void __RPC_FAR *__RPC_FAR *ppvObject)
{
    *ppvObject = 0;

    if (IID_IUnknown == riid || IID_IColumnProvider == riid)
    {
        IUnknown* pUnk = static_cast<IColumnProvider*>(this);
        pUnk->AddRef();
        *ppvObject = pUnk;
        return S_OK;
    }

    return E_NOINTERFACE;
}

//----------------------------

ULONG STDMETHODCALLTYPE CColumnInfo::AddRef(void)
{
    return InterlockedIncrement(&m_RefCnt);
}

//----------------------------

ULONG STDMETHODCALLTYPE CColumnInfo::Release( void)
{
    long refcnt = InterlockedDecrement(&m_RefCnt);

    if (0 == m_RefCnt)
        delete this;

    return refcnt;
}

//-----------------------------
// IColumnProvider
//-----------------------------

HRESULT STDMETHODCALLTYPE CColumnInfo::Initialize(LPCSHCOLUMNINIT /*psci*/)
{
    return S_OK;
}

//-----------------------------
// Register all columns we support
//-----------------------------

HRESULT STDMETHODCALLTYPE CColumnInfo::GetColumnInfo(DWORD dwIndex, SHCOLUMNINFO *psci)
{
    if (dwIndex >= ColumnInfoTableSize)
        return S_FALSE;

    //  Return information on each column we support. Return S_FALSE
    //  to indicate that we have returned information on all our
    //  columns. GetColumnInfo will be called repeatedly until S_FALSE
    //  or an error is returned
    psci->scid.fmtid = ColumnInfoTable[dwIndex].scid.fmtid;
    psci->scid.pid   = ColumnInfoTable[dwIndex].scid.pid;
    ZeroMemory(psci->wszTitle, sizeof(psci->wszTitle));
    wcsncpy(psci->wszTitle, ColumnInfoTable[dwIndex].wszTitle, (sizeof(psci->wszTitle) - 1));


    return S_OK;
}

//-----------------------------

HRESULT STDMETHODCALLTYPE CColumnInfo::GetItemData(LPCSHCOLUMNID pscid, LPCSHCOLUMNDATA pscd, VARIANT *pvarData)
{
    if (IsOOFileExtension(pscd->pwszExt))
    {
        try
        {
            std::wstring fname = getShortPathName( std::wstring( pscd->wszFile ) );

            CMetaInfoReader meta_info_accessor(WStringToString(fname));

            VariantClear(pvarData);

            if (IsEqualGUID (pscid->fmtid, FMTID_SummaryInformation) && pscid->pid == PIDSI_TITLE)
            {
                pvarData->vt = VT_BSTR;
                pvarData->bstrVal = SysAllocString(meta_info_accessor.getTagData( META_INFO_TITLE ).c_str());

                return S_OK;
            }
            else if (IsEqualGUID (pscid->fmtid, FMTID_SummaryInformation) && pscid->pid == PIDSI_AUTHOR)
            {
                pvarData->vt = VT_BSTR;
                pvarData->bstrVal = SysAllocString(meta_info_accessor.getTagData( META_INFO_AUTHOR).c_str());

                return S_OK;
            }
            else if (IsEqualGUID (pscid->fmtid, FMTID_SummaryInformation) && pscid->pid == PIDSI_SUBJECT)
            {
                pvarData->vt = VT_BSTR;
                pvarData->bstrVal = SysAllocString(meta_info_accessor.getTagData( META_INFO_SUBJECT).c_str());

                return S_OK;
            }
            else if (IsEqualGUID (pscid->fmtid, FMTID_SummaryInformation) && pscid->pid == PIDSI_KEYWORDS)
            {
                pvarData->vt = VT_BSTR;
                pvarData->bstrVal = SysAllocString(meta_info_accessor.getTagData( META_INFO_KEYWORDS).c_str());

                return S_OK;
            }
            else if (IsEqualGUID (pscid->fmtid, FMTID_SummaryInformation) && pscid->pid == PIDSI_COMMENTS)
            {
                pvarData->vt = VT_BSTR;
                pvarData->bstrVal = SysAllocString(meta_info_accessor.getTagData( META_INFO_DESCRIPTION).c_str());

                return S_OK;
            }
            else if (IsEqualGUID (pscid->fmtid, FMTID_SummaryInformation) && pscid->pid == PIDSI_PAGECOUNT)
            {
                pvarData->vt = VT_BSTR;
                pvarData->bstrVal = SysAllocString(meta_info_accessor.getTagAttribute( META_INFO_DOCUMENT_STATISTIC, META_INFO_PAGES).c_str());

                return S_OK;
            }
        }
        catch (const std::exception&)
        {
            return S_FALSE;
        }
    }

    return S_FALSE;
}

//-----------------------------

bool CColumnInfo::IsOOFileExtension(wchar_t* Extension) const
{
    for (size_t i = 0; i < OOFileExtensionTableSize; i++)
    {
        if (0 == _wcsicmp(Extension, OOFileExtensionTable[i].ExtensionUnicode))
            return true;
    }

    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
