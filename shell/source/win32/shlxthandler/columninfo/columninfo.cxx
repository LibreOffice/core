/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_shell.hxx"
#include "internal/global.hxx"
#include "internal/columninfo.hxx"
#include "internal/fileextensions.hxx"
#include "internal/metainforeader.hxx"
#include "internal/utilities.hxx"
#include "internal/config.hxx"

#include <sal/macros.h>
#include <malloc.h>

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

    size_t ColumnInfoTableSize = SAL_N_ELEMENTS(ColumnInfoTable);
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
