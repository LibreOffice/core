/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: columninfo.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:59:34 $
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

#ifndef GLOBAL_HXX_INCLUDED
#include "internal/global.hxx"
#endif

#ifndef COLUMNINFO_HXX_INCLUDED
#include "internal/columninfo.hxx"
#endif

#ifndef FILEEXTENSIONS_HXX_INCLUDED
#include "internal/fileextensions.hxx"
#endif

#ifndef METAINFOREADER_HXX_INCLUDED
#include "internal/metainforeader.hxx"
#endif

#ifndef UTILITIES_HXX_INCLUDED
#include "internal/utilities.hxx"
#endif

#ifndef CONFIG_HXX_INCLUDED
#include "internal/config.hxx"
#endif

#include <malloc.h>

//----------------------------
//
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
//
//----------------------------

CColumnInfo::CColumnInfo(long RefCnt) :
    m_RefCnt(RefCnt)
{
    InterlockedIncrement(&g_DllRefCnt);
}

//----------------------------
//
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
//
//----------------------------

ULONG STDMETHODCALLTYPE CColumnInfo::AddRef(void)
{
    return InterlockedIncrement(&m_RefCnt);
}

//----------------------------
//
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

    //wcscpy(psci->wszTitle, ColumnInfoTable[dwIndex].wszTitle);

    return S_OK;
}

//-----------------------------
//
//-----------------------------

 HRESULT STDMETHODCALLTYPE CColumnInfo::GetItemData(LPCSHCOLUMNID pscid, LPCSHCOLUMNDATA pscd, VARIANT *pvarData)
{
    if (IsOOFileExtension(pscd->pwszExt))
    {
        try
        {

            CMetaInfoReader meta_info_accessor(WStringToString(pscd->wszFile));

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
//
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
