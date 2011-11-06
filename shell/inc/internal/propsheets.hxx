/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef PROPSHEETS_HXX_INCLUDED
#define PROPSHEETS_HXX_INCLUDED

#include "internal/metainforeader.hxx"

#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#include <shlobj.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif
#include <string>
#include <memory>

class CPropertySheet : public IShellExtInit, public IShellPropSheetExt
{
public:
    CPropertySheet(long RefCnt = 1);

    virtual ~CPropertySheet();

    //-----------------------------
    // IUnknown methods
    //-----------------------------

    virtual HRESULT STDMETHODCALLTYPE QueryInterface(
            REFIID riid,
            void __RPC_FAR *__RPC_FAR *ppvObject);

    virtual ULONG STDMETHODCALLTYPE AddRef( void);

    virtual ULONG STDMETHODCALLTYPE Release( void);

    //-----------------------------
    // IShellExtInit
    //-----------------------------

    virtual HRESULT STDMETHODCALLTYPE Initialize(
        LPCITEMIDLIST pidlFolder, LPDATAOBJECT lpdobj, HKEY hkeyProgID);

    //-----------------------------
    // IShellPropSheetExt
    //-----------------------------

    virtual HRESULT STDMETHODCALLTYPE AddPages(LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam);

    virtual HRESULT STDMETHODCALLTYPE ReplacePage(
        UINT uPageID, LPFNADDPROPSHEETPAGE lpfnReplaceWith, LPARAM lParam);

private:
    // Windows callback functions
    static UINT CALLBACK PropPageSummaryCallback(HWND hwnd, UINT uMsg, LPPROPSHEETPAGE ppsp);
    static BOOL CALLBACK PropPageSummaryProc(HWND hwnd, UINT uiMsg, WPARAM wParam, LPARAM lParam);
    static BOOL CALLBACK PropPageStatisticsProc(HWND hwnd, UINT uiMsg, WPARAM wParam, LPARAM lParam);


    // Helper functions
    void InitPropPageSummary(HWND hwnd, LPPROPSHEETPAGE lppsp);
    void InitPropPageStatistics(HWND hwnd, LPPROPSHEETPAGE lppsp);

private:
    long m_RefCnt;
    char m_szFileName[MAX_PATH];
};

#endif
