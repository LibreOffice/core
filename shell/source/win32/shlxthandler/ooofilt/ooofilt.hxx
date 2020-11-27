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

#ifndef INCLUDED_SHELL_SOURCE_WIN32_SHLXTHANDLER_OOOFILT_OOOFILT_HXX
#define INCLUDED_SHELL_SOURCE_WIN32_SHLXTHANDLER_OOOFILT_OOOFILT_HXX

#include <types.hxx>

//+-------------------------------------------------------------------------
//  Contents:   LibreOffice filter declarations
//  Platform:   Windows 2000, Windows XP

//+-------------------------------------------------------------------------

class CContentReader;
class CMetaInfoReader;
class CFullPropSpec;

LONG g_lInstances = 0;                        // Global count of COooFilter and COooFilterCF instances
GUID const guidStorage = PSGUID_STORAGE;      // GUID for storage property set

//C-------------------------------------------------------------------------
//  Class:      COooFilter
//  Purpose:    Implements interfaces of LibreOffice filter

// OooFilter Class ID
// {7BC0E710-5703-45be-A29D-5D46D8B39262}
GUID const CLSID_COooFilter =
{
    0x7bc0e710,
    0x5703,
    0x45be,
    { 0xa2, 0x9d, 0x5d, 0x46, 0xd8, 0xb3, 0x92, 0x62 }
};

// LibreOffice Persistent Handler Class ID
// {7BC0E713-5703-45be-A29D-5D46D8B39262}
const CLSID CLSID_PERSISTENT_HANDLER =
{0x7bc0e713, 0x5703, 0x45be, {0xa2, 0x9d, 0x5d, 0x46, 0xd8, 0xb3, 0x92, 0x62}};

// LibreOffice Persistent Handler Addin Registered Class ID
// {89BCB740-6119-101A-BCB7-00DD010655AF}
const CLSID CLSID_PERSISTENT_HANDLER_ADDIN =
{0x89bcb740, 0x6119, 0x101a, {0xbc, 0xb7, 0x00, 0xdd, 0x01, 0x06, 0x55, 0xaf}};

// LibreOffice Filter Handler Class ID
// {7BC0E710-5703-45be-A29D-5D46D8B39262}
const CLSID CLSID_FILTER_HANDLER =
{0x7bc0e710, 0x5703, 0x45be, {0xa2, 0x9d, 0x5d, 0x46, 0xd8, 0xb3, 0x92, 0x62}};

enum class FilterState
{
    FilteringContent,                           // Filtering the content property
    FilteringProperty                           // Filtering the pseudo property
};
class COooFilter : public IFilter, public IPersistFile, public IPersistStream
{
public:
    // From IUnknown
    virtual  HRESULT STDMETHODCALLTYPE  QueryInterface(
        REFIID riid,
        void  ** ppvObject) override;
    virtual  ULONG STDMETHODCALLTYPE  AddRef() override;
    virtual  ULONG STDMETHODCALLTYPE  Release() override;

    // From IFilter
    virtual  SCODE STDMETHODCALLTYPE  Init(
        ULONG grfFlags,
        ULONG cAttributes,
        FULLPROPSPEC const * aAttributes,
        ULONG * pFlags) override;
    virtual  SCODE STDMETHODCALLTYPE  GetChunk(
        STAT_CHUNK * pStat) override;
    virtual  SCODE STDMETHODCALLTYPE  GetText(
        ULONG * pcwcBuffer,
        WCHAR * awcBuffer) override;

    virtual  SCODE STDMETHODCALLTYPE  GetValue(
        PROPVARIANT ** ppPropValue) override;

    virtual  SCODE STDMETHODCALLTYPE  BindRegion(
        FILTERREGION origPos,
        REFIID riid,
        void ** ppunk) override;

    // From IPersistFile
    virtual  HRESULT STDMETHODCALLTYPE  GetClassID(
        CLSID * pClassID) override;
    virtual  HRESULT STDMETHODCALLTYPE  IsDirty() override;
    virtual  HRESULT STDMETHODCALLTYPE  Load(
        LPCOLESTR pszFileName,
        DWORD dwMode) override;
    virtual  HRESULT STDMETHODCALLTYPE  Save(
        LPCOLESTR pszFileName,
        BOOL fRemember) override;

    virtual  HRESULT STDMETHODCALLTYPE  SaveCompleted(
        LPCOLESTR pszFileName) override;

    virtual  HRESULT STDMETHODCALLTYPE  GetCurFile(
        LPOLESTR  * ppszFileName) override;

    // From IPersistStream
    virtual HRESULT STDMETHODCALLTYPE  Load(
        IStream *pStm) override;

    virtual HRESULT STDMETHODCALLTYPE Save(
        IStream *pStm,
        BOOL fClearDirty) override;

    virtual HRESULT STDMETHODCALLTYPE  GetSizeMax(
        ULARGE_INTEGER *pcbSize) override;


private:
    friend class COooFilterCF;

    COooFilter();
    virtual ~COooFilter();

    LONG                      m_lRefs;                  // Reference count
    CContentReader *          m_pContentReader;         // A content reader that retrieves document content.
    CMetaInfoReader *         m_pMetaInfoReader;        // A metainfo reader that retrieves document metainfo.
    FilterState               m_eState;                 // State of filtering
    ::std::wstring            m_pwszFileName;           // Name of input file to filter
    ULONG                     m_ulUnicodeBufferLen;     // UNICODE Characters read from file to chunk buffer
    ULONG                     m_ulUnicodeCharsRead;     // UNICODE Characters read from chunk buffer
    ULONG                     m_ulPropertyNum;          // Number of properties that has been processed
    ULONG                     m_ulCurrentPropertyNum;   // Current Property that is processing;
    ULONG                     m_ulChunkID;              // Current chunk id
    bool                      m_fContents;              // TRUE if contents requested
    bool                      m_fEof;                   // TRUE if end of file reached
    ::std::wstring            m_pwsBuffer;              // Buffer to save UNICODE content from ChunkBuffer.
    ULONG                     m_ChunkPosition;          // Chunk pointer to specify the current Chunk;
    ULONG                     m_cAttributes;            // Count of attributes
    CFullPropSpec *           m_pAttributes;            // Attributes to filter
    StreamInterface *         m_pStream;

};

//C-------------------------------------------------------------------------
//  Class:      COooFilterCF
//  Purpose:    Implements class factory for LibreOffice filter


class COooFilterCF : public IClassFactory
{
public:
    // From IUnknown
    virtual  HRESULT STDMETHODCALLTYPE  QueryInterface(
        REFIID riid,
        void  ** ppvObject) override;

    virtual  ULONG STDMETHODCALLTYPE  AddRef() override;
    virtual  ULONG STDMETHODCALLTYPE  Release() override;

    // From IClassFactory
    virtual  HRESULT STDMETHODCALLTYPE  CreateInstance(
        IUnknown * pUnkOuter,
        REFIID riid, void  ** ppvObject) override;

    virtual  HRESULT STDMETHODCALLTYPE  LockServer(
        BOOL fLock) override;

private:
    friend HRESULT STDMETHODCALLTYPE DllGetClassObject(
        REFCLSID   cid,
        REFIID     iid,
        LPVOID *   ppvObj);

    COooFilterCF();
    virtual  ~COooFilterCF();

    LONG m_lRefs;           // Reference count
};

#endif // INCLUDED_SHELL_SOURCE_WIN32_SHLXTHANDLER_OOOFILT_OOOFILT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
