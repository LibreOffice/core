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

#include "internal/types.hxx"

//+-------------------------------------------------------------------------
//  Contents:   OpenOffice.org filter declarations
//  Platform:   Windows 2000, Windows XP
//--------------------------------------------------------------------------

#pragma once

//+-------------------------------------------------------------------------
//  forward declaration
//--------------------------------------------------------------------------
class CContentReader;
class CMetaInfoReader;
class CFullPropSpec;

//+-------------------------------------------------------------------------
//  Global definitions
//--------------------------------------------------------------------------

long g_lInstances = 0;                        // Global count of COooFilter and COooFilterCF instances
GUID const guidStorage = PSGUID_STORAGE;      // GUID for storage property set

//C-------------------------------------------------------------------------
//  Class:      COooFilter
//  Purpose:    Implements interfaces of OpenOffice.org filter
//--------------------------------------------------------------------------

// OooFilter Class ID
// {7BC0E710-5703-45be-A29D-5D46D8B39262}
GUID const CLSID_COooFilter =
{
    0x7bc0e710,
    0x5703,
    0x45be,
    { 0xa2, 0x9d, 0x5d, 0x46, 0xd8, 0xb3, 0x92, 0x62 }
};

// OpenOffice.org Persistent Handler Class ID
// {7BC0E713-5703-45be-A29D-5D46D8B39262}
const CLSID CLSID_PERSISTENT_HANDLER =
{0x7bc0e713, 0x5703, 0x45be, {0xa2, 0x9d, 0x5d, 0x46, 0xd8, 0xb3, 0x92, 0x62}};

// OpenOffice.org Persistent Handler Addin Registered Class ID
// {89BCB740-6119-101A-BCB7-00DD010655AF}
const CLSID CLSID_PERSISTENT_HANDLER_ADDIN =
{0x89bcb740, 0x6119, 0x101a, {0xbc, 0xb7, 0x00, 0xdd, 0x01, 0x06, 0x55, 0xaf}};

// OpenOffice.org Filter Handler Class ID
// {7BC0E710-5703-45be-A29D-5D46D8B39262}
const CLSID CLSID_FILTER_HANDLER =
{0x7bc0e710, 0x5703, 0x45be, {0xa2, 0x9d, 0x5d, 0x46, 0xd8, 0xb3, 0x92, 0x62}};

enum FilterState
{
    FilteringContent,                           // Filtering the content property
    FilteringProperty                           // Filtering the pseudo property
};
class COooFilter : public IFilter, public IPersistFile, public IPersistStream
{
public:
    // From IUnknown
    virtual  SCODE STDMETHODCALLTYPE  QueryInterface(
        REFIID riid,
        void  ** ppvObject);
    virtual  ULONG STDMETHODCALLTYPE  AddRef();
    virtual  ULONG STDMETHODCALLTYPE  Release();

    // From IFilter
    virtual  SCODE STDMETHODCALLTYPE  Init(
        ULONG grfFlags,
        ULONG cAttributes,
        FULLPROPSPEC const * aAttributes,
        ULONG * pFlags);
    virtual  SCODE STDMETHODCALLTYPE  GetChunk(
        STAT_CHUNK * pStat);
    virtual  SCODE STDMETHODCALLTYPE  GetText(
        ULONG * pcwcBuffer,
        WCHAR * awcBuffer);

    virtual  SCODE STDMETHODCALLTYPE  GetValue(
        PROPVARIANT ** ppPropValue);

    virtual  SCODE STDMETHODCALLTYPE  BindRegion(
        FILTERREGION origPos,
        REFIID riid,
        void ** ppunk);

    // From IPersistFile
    virtual  SCODE STDMETHODCALLTYPE  GetClassID(
        CLSID * pClassID);
    virtual  SCODE STDMETHODCALLTYPE  IsDirty();
    virtual  SCODE STDMETHODCALLTYPE  Load(
        LPCWSTR pszFileName,
        DWORD dwMode);
    virtual  SCODE STDMETHODCALLTYPE  Save(
        LPCWSTR pszFileName,
        BOOL fRemember);

    virtual  SCODE STDMETHODCALLTYPE  SaveCompleted(
        LPCWSTR pszFileName);

    virtual  SCODE STDMETHODCALLTYPE  GetCurFile(
        LPWSTR  * ppszFileName);

    // From IPersistStream
    virtual SCODE STDMETHODCALLTYPE  Load(
        IStream *pStm);

    virtual SCODE STDMETHODCALLTYPE Save(
        IStream *pStm,
        BOOL fClearDirty);

    virtual SCODE STDMETHODCALLTYPE  GetSizeMax(
        ULARGE_INTEGER *pcbSize);


private:
    friend class COooFilterCF;

    COooFilter();
    virtual ~COooFilter();

    long                      m_lRefs;                  // Reference count
    CContentReader *          m_pContentReader;         // A content reader that retrive document content.
    CMetaInfoReader *         m_pMetaInfoReader;        // A metainfo reader that retrive document metainfo.
    FilterState               m_eState;                 // State of filtering
    ::std::wstring            m_pwszFileName;           // Name of input file to filter
    ULONG                     m_ulUnicodeBufferLen;     // UNICODE Characters read from file to chunk buffer
    ULONG                     m_ulUnicodeCharsRead;     // UNICODE Characters read from chunk buffer
    ULONG                     m_ulPropertyNum;          // Number of properties that has been processed
    ULONG                     m_ulCurrentPropertyNum;   // Current Property that is processing;
    ULONG                     m_ulChunkID;              // Current chunk id
    BOOL                      m_fContents;              // TRUE if contents requested
    BOOL                      m_fEof;                   // TRUE if end of file reached
    ::std::wstring            m_pwsBuffer;              // Buffer to save UNICODE content from ChunkBuffer.
    ULONG                     m_ChunkPosition;          // Chunk pointer to specify the current Chunk;
    ULONG                     m_cAttributes;            // Count of attributes
    CFullPropSpec *           m_pAttributes;            // Attributes to filter
    IStream *                 m_pStream;

};

//C-------------------------------------------------------------------------
//  Class:      COooFilterCF
//  Purpose:    Implements class factory for OpenOffice.org filter
//--------------------------------------------------------------------------

class COooFilterCF : public IClassFactory
{
public:
    // From IUnknown
    virtual  SCODE STDMETHODCALLTYPE  QueryInterface(
        REFIID riid,
        void  ** ppvObject);

    virtual  ULONG STDMETHODCALLTYPE  AddRef();
    virtual  ULONG STDMETHODCALLTYPE  Release();

    // From IClassFactory
    virtual  SCODE STDMETHODCALLTYPE  CreateInstance(
        IUnknown * pUnkOuter,
        REFIID riid, void  ** ppvObject);

    virtual  SCODE STDMETHODCALLTYPE  LockServer(
        BOOL fLock);

private:
    friend SCODE STDMETHODCALLTYPE DllGetClassObject(
        REFCLSID   cid,
        REFIID     iid,
        void **    ppvObj);

    COooFilterCF();
    virtual  ~COooFilterCF();

    long m_lRefs;           // Reference count
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
