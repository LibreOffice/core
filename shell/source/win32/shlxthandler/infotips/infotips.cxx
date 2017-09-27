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

#include "global.hxx"
#include "infotips.hxx"
#include "shlxthdl.hxx"
#include "metainforeader.hxx"
#include "contentreader.hxx"
#include "utilities.hxx"
#include "registry.hxx"
#include "fileextensions.hxx"
#include "iso8601_converter.hxx"
#include "config.hxx"

#include "resource.h"
#include <stdio.h>
#include <utility>
#include <stdlib.h>


#define MAX_STRING 80
#define KB 1024.0
const std::wstring WSPACE = std::wstring(SPACE);


CInfoTip::CInfoTip(long RefCnt) :
    m_RefCnt(RefCnt)
{
    ZeroMemory(m_szFileName, sizeof(m_szFileName));
    InterlockedIncrement(&g_DllRefCnt);
}


CInfoTip::~CInfoTip()
{
    InterlockedDecrement(&g_DllRefCnt);
}


// IUnknown methods


HRESULT STDMETHODCALLTYPE CInfoTip::QueryInterface(REFIID riid, void __RPC_FAR *__RPC_FAR *ppvObject)
{
    *ppvObject = nullptr;

    IUnknown* pUnk = nullptr;

    if (IID_IUnknown == riid || IID_IQueryInfo == riid)
    {
        pUnk = static_cast<IQueryInfo*>(this);
        pUnk->AddRef();
        *ppvObject = pUnk;
        return S_OK;
    }
    else if (IID_IPersistFile == riid)
    {
        pUnk = static_cast<IPersistFile*>(this);
        pUnk->AddRef();
        *ppvObject = pUnk;
        return S_OK;
    }

    return E_NOINTERFACE;
}


ULONG STDMETHODCALLTYPE CInfoTip::AddRef()
{
    return InterlockedIncrement(&m_RefCnt);
}


ULONG STDMETHODCALLTYPE CInfoTip::Release()
{
    long refcnt = InterlockedDecrement(&m_RefCnt);

    if (0 == m_RefCnt)
        delete this;

    return refcnt;
}

//********************helper functions for GetInfoTip functions**********************

/** get file type information from registry.
*/
std::wstring getFileTypeInfo(const std::wstring& file_extension)
{
    wchar_t extKeyValue[MAX_STRING];
    wchar_t typeKeyValue[MAX_STRING];
    ::std::wstring sDot(L".");
    if (QueryRegistryKey(HKEY_CLASSES_ROOT, (sDot.append(file_extension)).c_str(), L"", extKeyValue, MAX_STRING))
        if (QueryRegistryKey( HKEY_CLASSES_ROOT, extKeyValue, L"",typeKeyValue, MAX_STRING))
            return typeKeyValue;

    return EMPTY_STRING;
}

/** get file size.
*/
DWORD getSizeOfFile( wchar_t const * FileName )
{
    HANDLE hFile = CreateFileW(FileName,                                    // open file
                        GENERIC_READ,                                       // open for reading
                        FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE, // share for all operations
                        nullptr,                                            // no security
                        OPEN_EXISTING,                                      // existing file only
                        FILE_ATTRIBUTE_NORMAL,                              // normal file
                        nullptr);                                           // no attr. template

    if (hFile != INVALID_HANDLE_VALUE)
    {
        DWORD dwSize = GetFileSize( hFile, nullptr );
        CloseHandle( hFile );
        return dwSize;
    }

    return INVALID_FILE_SIZE;
}

/** format file size in to be more readable.
*/
std::wstring formatSizeOfFile( DWORD dwSize )
{
    if ( dwSize < 1000 )
    {
        char buffer[3];
        int dFileSize = dwSize;

        _itoa( dFileSize, buffer, 10 );
        return StringToWString( buffer ).append(StringToWString("B"));
    }

    char *buffer=nullptr;
    int  decimal, sign;
    double dFileSize = (double)dwSize/KB;

    buffer = _fcvt( dFileSize, 1, &decimal, &sign );

    ::std::wstring wsTemp = StringToWString( buffer );
    int  pos=decimal % 3;
    ::std::wstring wsBuffer = wsTemp.substr( 0,pos);

    if ( decimal )
        for (;decimal - pos > 2;pos += 3)
        {
            if (pos)
                wsBuffer.append(StringToWString(","));
            wsBuffer.append( wsTemp.substr( pos, 3) );
        }
    else
        wsBuffer.append(StringToWString("0"));

    wsBuffer.append(StringToWString("."));
    wsBuffer.append(wsTemp.substr( decimal, wsTemp.size()-decimal ));
    wsBuffer.append(StringToWString("KB"));

    return wsBuffer;
}


/** get file size information.
*/
std::wstring getFileSizeInfo(wchar_t const * FileName)
{
    DWORD dwSize=getSizeOfFile(FileName);
    if (dwSize != INVALID_FILE_SIZE)
        return formatSizeOfFile( dwSize );

    return EMPTY_STRING;
}


// IQueryInfo methods


HRESULT STDMETHODCALLTYPE CInfoTip::GetInfoTip(DWORD /*dwFlags*/, wchar_t** ppwszTip)
{
    std::wstring msg;
    const std::wstring CONST_SPACE(SPACE);

    //display File Type, no matter other info is loaded successfully or not.
    std::wstring tmpTypeStr = getFileTypeInfo( get_file_name_extension(m_szFileName) );
    if ( tmpTypeStr != EMPTY_STRING )
    {
        msg += GetResString(IDS_TYPE_COLON) + CONST_SPACE;
        msg += tmpTypeStr;
    }

    try
    {
        CMetaInfoReader meta_info_accessor(m_szFileName);

        //display document title;
        if ( meta_info_accessor.getTagData( META_INFO_TITLE ).length() > 0)
        {
            if ( msg != EMPTY_STRING )
                msg += L"\n";
            msg += GetResString(IDS_TITLE_COLON) + CONST_SPACE;
            msg += meta_info_accessor.getTagData( META_INFO_TITLE );
        }
        else
        {
            if ( msg != EMPTY_STRING )
                msg += L"\n";
            msg += GetResString(IDS_TITLE_COLON) + CONST_SPACE;
            msg += m_FileNameOnly;
        }

        //display document author;
        if ( meta_info_accessor.getTagData( META_INFO_AUTHOR ).length() > 0)
        {
            if ( msg != EMPTY_STRING )
                msg += L"\n";
            msg += GetResString( IDS_AUTHOR_COLON ) + CONST_SPACE;
            msg += meta_info_accessor.getTagData( META_INFO_AUTHOR );
        }

        //display document subject;
        if ( meta_info_accessor.getTagData( META_INFO_SUBJECT ).length() > 0)
        {
            if ( msg != EMPTY_STRING )
                msg += L"\n";
            msg += GetResString(IDS_SUBJECT_COLON) + CONST_SPACE;
            msg += meta_info_accessor.getTagData( META_INFO_SUBJECT );
        }

        //display document description;
        if ( meta_info_accessor.getTagData( META_INFO_DESCRIPTION ).length() > 0)
        {
            if ( msg != EMPTY_STRING )
                msg += L"\n";
            msg += GetResString( IDS_COMMENTS_COLON ) + CONST_SPACE;
            msg += meta_info_accessor.getTagData( META_INFO_DESCRIPTION );
        }

        //display modified time formatted into locale representation.
        if ( iso8601_date_to_local_date(meta_info_accessor.getTagData(META_INFO_MODIFIED )).length() > 0)
        {
            if ( msg != EMPTY_STRING )
                msg += L"\n";
            msg += GetResString( IDS_MODIFIED_COLON ) + CONST_SPACE;
            msg += iso8601_date_to_local_date(meta_info_accessor.getTagData(META_INFO_MODIFIED ));
        }
    }
    catch (const std::exception&)
    {
    }

    //display file size, no matter other information is loaded successfully or not.
    std::wstring tmpSizeStr = getFileSizeInfo( m_szFileName );
    if ( tmpSizeStr != EMPTY_STRING )
    {
        msg += L"\n";
        msg += GetResString( IDS_SIZE_COLON ) + CONST_SPACE;
        msg += tmpSizeStr;
    }


    //finalize and assign the string.
    LPMALLOC lpMalloc;
    HRESULT hr = SHGetMalloc(&lpMalloc);

    if (SUCCEEDED(hr))
    {
        size_t len = sizeof(wchar_t) * msg.length() + sizeof(wchar_t);
        wchar_t* pMem = static_cast<wchar_t*>(lpMalloc->Alloc(len));

        ZeroMemory(pMem, len);

        wcscpy_s(pMem, msg.length()+1, msg.c_str());

        *ppwszTip = pMem;
        lpMalloc->Release();

        return S_OK;
    }

    return E_FAIL;
}


HRESULT STDMETHODCALLTYPE CInfoTip::GetInfoFlags(DWORD * /*pdwFlags*/ )
{
    return E_NOTIMPL;
}


// IPersist methods


HRESULT STDMETHODCALLTYPE CInfoTip::GetClassID(CLSID* pClassID)
{
    pClassID = const_cast<CLSID*>(&CLSID_INFOTIP_HANDLER);
    return S_OK;
}


// IPersistFile methods


HRESULT STDMETHODCALLTYPE CInfoTip::Load(LPCOLESTR pszFileName, DWORD /*dwMode*/)
{
    std::wstring fname = pszFileName;

    // there must be a '\' and there must even be an
    // extension, else we would not have been called
    std::wstring::iterator begin = fname.begin() + fname.find_last_of(L"\\") + 1;
    std::wstring::iterator end   = fname.end();

    m_FileNameOnly = std::wstring(begin, end);

    fname = getShortPathName( fname );

    // ZeroMemory because strncpy doesn't '\0'-terminates the destination
    // string; reserve the last place in the buffer for the final '\0'
    // that's why '(sizeof(m_szFileName) - 1)'
    ZeroMemory(m_szFileName, sizeof(m_szFileName));
    wcsncpy(m_szFileName, fname.c_str(), (sizeof(m_szFileName)/sizeof(*m_szFileName) - 1));

    return S_OK;
}


HRESULT STDMETHODCALLTYPE CInfoTip::IsDirty()
{
    return E_NOTIMPL;
}


HRESULT STDMETHODCALLTYPE CInfoTip::Save(LPCOLESTR /*pszFileName*/, BOOL /*fRemember*/)
{
    return E_NOTIMPL;
}


HRESULT STDMETHODCALLTYPE CInfoTip::SaveCompleted(LPCOLESTR /*pszFileName*/)
{
    return E_NOTIMPL;
}


HRESULT STDMETHODCALLTYPE CInfoTip::GetCurFile(LPOLESTR __RPC_FAR * /*ppszFileName*/)
{
    return E_NOTIMPL;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
