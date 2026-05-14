/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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


//  File:       odffilter.cxx
//  Contents:   IFilter implementation for ODF documents, used by Windows Search
//              for full-text indexing.
//  Summary:    The ODF filter reads ODF files (including the legacy StarOffice
//              .sxw .sxi etc. variants) and extracts their content, author,
//              keywords, subject, comments and title for the search index.

#include <contentreader.hxx>
#include <metainforeader.hxx>
#include <fileextensions.hxx>


//  Include file    Purpose
//  windows.h       Win32 declarations
//  string.h        string wstring declarations
//  filter.h        IFilter interface declarations
//  filterr.h       FACILITY_ITF error definitions for IFilter
//  ntquery.h       Indexing Service declarations
//  assert.h        assertion function.
//  odffilter.hxx   ODF filter declarations
//  propspec.hxx    PROPSPEC


#if !defined WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <string.h>
#include <filter.h>
#include <filterr.h>
#include <ntquery.h>
#include <assert.h>
#include "odffilter.hxx"
#include <objidl.h>
#include <stdio.h>
#include "propspec.hxx"

#include <stream_helper.hxx>

//C-------------------------------------------------------------------------
//  Class:      COdfFilter
//  Summary:    Implements ODF filter class

//M-------------------------------------------------------------------------
//  Method:     COdfFilter::COdfFilter
//  Summary:    Class constructor
//  Arguments:  void
//  Purpose:    Manages global instance count

COdfFilter::COdfFilter() :
    m_lRefs(1),
    m_pContentReader(nullptr),
    m_pMetaInfoReader(nullptr),
    m_eState(FilterState::FilteringContent),
    m_ulUnicodeBufferLen(0),
    m_ulUnicodeCharsRead(0),
    m_ulPropertyNum(0),
    m_ulCurrentPropertyNum(0),
    m_ulChunkID(1),
    m_fContents(false),
    m_fEof(false),
    m_ChunkPosition(0),
    m_cAttributes(0),
    m_pAttributes(nullptr),
    m_pStream(nullptr)

{
    InterlockedIncrement( &g_lInstances );
}
//M-------------------------------------------------------------------------
//  Method:     COdfFilter::~COdfFilter
//  Summary:    Class destructor
//  Arguments:  void
//  Purpose:    Manages global instance count and file handle

COdfFilter::~COdfFilter()
{
    delete [] m_pAttributes;
    delete m_pContentReader;
    delete m_pMetaInfoReader;
    delete m_pStream;

    InterlockedDecrement( &g_lInstances );
}

//M-------------------------------------------------------------------------
//  Method:     COdfFilter::QueryInterface      (IUnknown::QueryInterface)
//  Summary:    Queries for requested interface //  Arguments:  riid
//              [in] Reference IID of requested interface
//              ppvObject
//              [out] Address that receives requested interface pointer
//  Returns:    S_OK
//              Interface is supported
//              E_NOINTERFACE
//              Interface is not supported

HRESULT STDMETHODCALLTYPE COdfFilter::QueryInterface(
    REFIID riid,
    void  ** ppvObject)
{
    IUnknown *pUnkTemp = nullptr;
    if ( IID_IFilter == riid )
        pUnkTemp = static_cast<IFilter *>(this);
    else if ( IID_IPersistFile == riid )
        pUnkTemp = static_cast<IPersistFile *>(this);
    else if ( IID_IPersist == riid )
        pUnkTemp = static_cast<IPersistFile *>(this);
    else if (IID_IPersistStream == riid)
        pUnkTemp = static_cast<IPersistStream *>(this);
    else if ( IID_IUnknown == riid )
        pUnkTemp = static_cast<IPersistFile *>(this);
    else
    {
        *ppvObject = nullptr;
        return E_NOINTERFACE;
    }
    *ppvObject = pUnkTemp;
    pUnkTemp->AddRef();
    return S_OK;
}
//M-------------------------------------------------------------------------
//  Method:     COdfFilter::AddRef              (IUnknown::AddRef)
//  Summary:    Increments interface refcount
//  Arguments:  void
//  Returns:    Value of incremented interface refcount

ULONG STDMETHODCALLTYPE COdfFilter::AddRef()
{
    return InterlockedIncrement( &m_lRefs );
}
//M-------------------------------------------------------------------------
//  Method:     COdfFilter::Release             (IUnknown::Release)
//  Summary:    Decrements interface refcount, deleting if unreferenced
//  Arguments:  void
//  Returns:    Value of decremented interface refcount

ULONG STDMETHODCALLTYPE COdfFilter::Release()
{
    ULONG ulTmp = InterlockedDecrement( &m_lRefs );

    if ( 0 == ulTmp )
        delete this;
    return ulTmp;
}
//M-------------------------------------------------------------------------
//  Method:     COdfFilter::Init                (IFilter::Init)
//  Summary:    Initializes ODF filter instance
//  Arguments:  grfFlags
//                  [in] Flags for filter behavior
//              cAttributes
//                  [in] Number attributes in array aAttributes
//              aAttributes
//                  [in] Array of requested attribute strings
//              pFlags
//                  [out] Pointer to return flags for additional properties
//  Returns:    S_OK
//                  Initialization succeeded
//              E_FAIL
//                  File not previously loaded
//              E_INVALIDARG
//                  Count and contents of attributes do not agree
//              FILTER_E_ACCESS
//                  Unable to access file to be filtered
//              FILTER_E_PASSWORD
//                  (not implemented)

const int COUNT_ATTRIBUTES = 5;

SCODE STDMETHODCALLTYPE COdfFilter::Init(
    ULONG grfFlags,
    ULONG cAttributes,
    FULLPROPSPEC const * aAttributes,
    ULONG * pFlags)
{
    // Enumerate OLE properties, since any NTFS file can have them
    *pFlags = IFILTER_FLAGS_OLE_PROPERTIES;
    try
    {
        m_fContents = false;
        m_ulPropertyNum = 0;
        m_ulCurrentPropertyNum = 0;
        if ( m_cAttributes > 0 )
        {
            delete[] m_pAttributes;
            m_pAttributes = nullptr;
            m_cAttributes = 0;
        }
        if( 0 < cAttributes )
        {
            // Filter properties specified in aAttributes
            if ( nullptr == aAttributes )
                return E_INVALIDARG;
            m_pAttributes = new CFullPropSpec[cAttributes];
            m_cAttributes = cAttributes;
            // Is caller want to filter contents?
            CFullPropSpec const *pAttrib = reinterpret_cast<CFullPropSpec const *>(aAttributes);
            ULONG ulNumAttr;
            for ( ulNumAttr = 0 ; ulNumAttr < cAttributes; ulNumAttr++ )
            {
                if ( pAttrib[ulNumAttr].IsPropertyPropid() &&
                     pAttrib[ulNumAttr].GetPropertyPropid() == PID_STG_CONTENTS &&
                     pAttrib[ulNumAttr].GetPropSet() == guidStorage )
                {
                    m_fContents = true;
                }
                // save the requested properties.
                m_pAttributes[ulNumAttr] = pAttrib[ulNumAttr];
            }
        }
        else if ( grfFlags & IFILTER_INIT_APPLY_INDEX_ATTRIBUTES )
        {
            // Filter contents and all pseudo-properties
            m_fContents = true;

            m_pAttributes = new CFullPropSpec[COUNT_ATTRIBUTES];
            m_cAttributes = COUNT_ATTRIBUTES;
            m_pAttributes[0].SetPropSet( FMTID_SummaryInformation );
            m_pAttributes[0].SetProperty( PIDSI_AUTHOR );
            m_pAttributes[1].SetPropSet( FMTID_SummaryInformation );
            m_pAttributes[1].SetProperty( PIDSI_TITLE );
            m_pAttributes[2].SetPropSet( FMTID_SummaryInformation );
            m_pAttributes[2].SetProperty( PIDSI_SUBJECT );
            m_pAttributes[3].SetPropSet( FMTID_SummaryInformation );
            m_pAttributes[3].SetProperty( PIDSI_KEYWORDS );
            m_pAttributes[4].SetPropSet( FMTID_SummaryInformation );
            m_pAttributes[4].SetProperty( PIDSI_COMMENTS );
        }
        else if ( 0 == grfFlags )
        {
            // Filter only contents
            m_fContents = true;
        }
        else
            m_fContents = false;
        // Re-initialize
        if ( m_fContents )
        {
            m_fEof = false;
            m_eState = FilterState::FilteringContent;
            m_ulUnicodeCharsRead = 0;
            m_ChunkPosition = 0;
        }
        else
        {
            m_fEof = true;
            m_eState = FilterState::FilteringProperty;
        }
        m_ulChunkID = 1;
    }
    catch (const std::exception&)
    {
        return E_FAIL;
    }

    return S_OK;
}
//M-------------------------------------------------------------------------
//  Method:     COdfFilter::GetChunk            (IFilter::GetChunk)
//  Summary:    Gets the next chunk
//  Arguments:  ppStat
//                  [out] Pointer to description of current chunk
//  Returns:    S_OK
//                  Chunk was successfully retrieved
//              E_FAIL
//                  Character conversion failed
//              FILTER_E_ACCESS
//                  General access failure occurred
//              FILTER_E_END_OF_CHUNKS
//                  Previous chunk was the last chunk
//              FILTER_E_EMBEDDING_UNAVAILABLE
//                  (not implemented)
//              FILTER_E_LINK_UNAVAILABLE
//                  (not implemented)
//              FILTER_E_PASSWORD
//                  (not implemented)

SCODE STDMETHODCALLTYPE COdfFilter::GetChunk(STAT_CHUNK * pStat)
{
    for(;;)
    {
        switch ( m_eState )
        {
        case FilterState::FilteringContent:
        {
            if( m_ChunkPosition == m_pContentReader ->getChunkBuffer().size() )
            {
                m_ulUnicodeBufferLen=0;
                m_fEof = true;
            }

            if ( !m_fContents || m_fEof )
            {
                m_eState = FilterState::FilteringProperty;
                continue;
            }
            m_pwsBuffer = m_pContentReader -> getChunkBuffer()[m_ChunkPosition].second;
            m_ulUnicodeBufferLen = static_cast<ULONG>(m_pwsBuffer.length());
            DWORD ChunkLCID = LocaleSetToLCID( m_pContentReader -> getChunkBuffer()[m_ChunkPosition].first );
            // Set chunk description
            pStat->idChunk   = m_ulChunkID;
            pStat->breakType = CHUNK_NO_BREAK;
            pStat->flags     = CHUNK_TEXT;
            pStat->locale    = ChunkLCID;
            pStat->attribute.guidPropSet       = guidStorage;
            pStat->attribute.psProperty.ulKind = PRSPEC_PROPID;
            pStat->attribute.psProperty.propid = PID_STG_CONTENTS;
            pStat->idChunkSource  = m_ulChunkID;
            pStat->cwcStartSource = 0;
            pStat->cwcLenSource   = 0;
            m_ulUnicodeCharsRead = 0;
            m_ulChunkID++;
            m_ChunkPosition++;
            return S_OK;
        }
        case FilterState::FilteringProperty:
        {
            if ( m_cAttributes ==  0 )
                return FILTER_E_END_OF_CHUNKS;
            while(  !( ( m_pAttributes[m_ulPropertyNum].IsPropertyPropid() ) &&
                       ( m_pAttributes[m_ulPropertyNum].GetPropSet() == FMTID_SummaryInformation ) )||
                     ( ( m_pAttributes[m_ulPropertyNum].GetPropertyPropid() != PIDSI_AUTHOR ) &&
                       ( m_pAttributes[m_ulPropertyNum].GetPropertyPropid() != PIDSI_TITLE ) &&
                       ( m_pAttributes[m_ulPropertyNum].GetPropertyPropid() != PIDSI_SUBJECT ) &&
                       ( m_pAttributes[m_ulPropertyNum].GetPropertyPropid() != PIDSI_KEYWORDS ) &&
                       ( m_pAttributes[m_ulPropertyNum].GetPropertyPropid() != PIDSI_COMMENTS ) ) )
            {
                if ( m_ulPropertyNum <  m_cAttributes )
                    m_ulPropertyNum++;
                else
                    break;
            }
            if ( m_ulPropertyNum ==  m_cAttributes)
                return FILTER_E_END_OF_CHUNKS;
            else
            {
                // Set chunk description
                pStat->idChunk = m_ulChunkID;
                pStat->breakType = CHUNK_EOS;
                pStat->flags = CHUNK_VALUE;
                pStat->locale = GetSystemDefaultLCID();
                pStat->attribute.guidPropSet = FMTID_SummaryInformation;
                pStat->attribute.psProperty.ulKind = PRSPEC_PROPID;
                pStat->attribute.psProperty.propid = m_pAttributes[m_ulPropertyNum].GetPropertyPropid();
                pStat->idChunkSource = m_ulChunkID;
                pStat->cwcStartSource = 0;
                pStat->cwcLenSource = 0;
                m_ulCurrentPropertyNum = m_ulPropertyNum;
                m_ulPropertyNum++;
                m_ulChunkID++;
                return S_OK;
            }
        }
        }//switch(...)
    }//for(;;)
}
//M-------------------------------------------------------------------------
//  Method:     COdfFilter::GetText             (IFilter::GetText)
//  Summary:    Retrieves UNICODE text for index
//  Arguments:  pcwcBuffer
//                  [in] Pointer to size of UNICODE buffer
//                  [out] Pointer to count of UNICODE characters returned
//              awcBuffer
//                  [out] Pointer to buffer to receive UNICODE text
//  Returns:    S_OK
//                  Text successfully retrieved, but text remains in chunk
//              FILTER_E_NO_MORE_TEXT
//                  All of the text in the current chunk has been returned
//              FILTER_S_LAST_TEXT
//                  Next call to GetText will return FILTER_E_NO_MORE_TEXT

SCODE STDMETHODCALLTYPE COdfFilter::GetText(ULONG * pcwcBuffer, WCHAR * awcBuffer)
{
    switch ( m_eState )
    {
    case FilterState::FilteringProperty:
        return FILTER_E_NO_TEXT;
    case FilterState::FilteringContent:
    {
        if ( !m_fContents || 0 == m_ulUnicodeBufferLen )
        {
            *pcwcBuffer = 0;
            return FILTER_E_NO_MORE_TEXT;
        }
        // Copy UNICODE characters in chunk buffer to output UNICODE buffer
        ULONG ulToCopy = min( *pcwcBuffer, m_ulUnicodeBufferLen - m_ulUnicodeCharsRead );
        wmemcpy( awcBuffer, m_pwsBuffer.c_str() + m_ulUnicodeCharsRead, ulToCopy );
        ZeroMemory(
             awcBuffer + ulToCopy, (*pcwcBuffer - ulToCopy) * sizeof (WCHAR));
        m_ulUnicodeCharsRead += ulToCopy;
        *pcwcBuffer = ulToCopy;
        if ( m_ulUnicodeBufferLen == m_ulUnicodeCharsRead )
        {
            m_ulUnicodeCharsRead = 0;
            m_ulUnicodeBufferLen = 0;
            return FILTER_S_LAST_TEXT;
        }
        return S_OK;
    }
    }
    return E_FAIL; // Should not happen!
}
//M-------------------------------------------------------------------------
//  Method:     GetMetaInfoNameFromPropertyId
//  Summary:    helper function to convert PropertyID into respective
//              MetaInfo names.
//  Arguments:  ulPropID
//                  [in] property ID
//  Returns:    corresponding metainfo names.


static ::std::wstring GetMetaInfoNameFromPropertyId( ULONG ulPropID )
{
    switch ( ulPropID )
    {
        case PIDSI_AUTHOR:   return META_INFO_AUTHOR;
        case PIDSI_TITLE:    return META_INFO_TITLE;
        case PIDSI_SUBJECT:  return META_INFO_SUBJECT;
        case PIDSI_KEYWORDS: return META_INFO_KEYWORDS;
        case PIDSI_COMMENTS: return META_INFO_DESCRIPTION;
        default:             return EMPTY_STRING;
    }
}
//M-------------------------------------------------------------------------
//  Method:     COdfFilter::GetValue            (IFilter::GetValue)
//  Summary:    Retrieves properties for index
//  Arguments:  ppPropValue
//                  [out] Address that receives pointer to property value
//  Returns:    FILTER_E_NO_VALUES
//                  Always
//              FILTER_E_NO_MORE_VALUES
//                  (not implemented)


SCODE STDMETHODCALLTYPE COdfFilter::GetValue(PROPVARIANT ** ppPropValue)
{
    if (m_eState == FilterState::FilteringContent)
        return FILTER_E_NO_VALUES;
    else // m_eState == FilteringProperty
    {
        if ( m_cAttributes == 0 || ( m_ulCurrentPropertyNum == m_ulPropertyNum ) )
            return FILTER_E_NO_MORE_VALUES;
        PROPVARIANT *pPropVar = static_cast<PROPVARIANT *>(CoTaskMemAlloc( sizeof (PROPVARIANT) ));
        if ( pPropVar == nullptr )
            return E_OUTOFMEMORY;
        ::std::wstring wsTagName= GetMetaInfoNameFromPropertyId( m_pAttributes[m_ulCurrentPropertyNum].GetPropertyPropid() );
        if ( wsTagName == EMPTY_STRING )
            return FILTER_E_NO_VALUES;
        ::std::wstring wsTagData = m_pMetaInfoReader->getTagData(wsTagName);
        pPropVar->vt = VT_LPWSTR;
        size_t cw = wsTagData.length() + 1; // reserve one for the '\0'
        pPropVar->pwszVal = static_cast<WCHAR*>( CoTaskMemAlloc(cw*sizeof(WCHAR)) );
        if (pPropVar->pwszVal == nullptr)
        {
            CoTaskMemFree(pPropVar);
            return E_OUTOFMEMORY;
        }
        wmemcpy(pPropVar->pwszVal, wsTagData.c_str(), cw);
        *ppPropValue = pPropVar;
        m_ulCurrentPropertyNum = m_ulPropertyNum;
        return S_OK;
    }
}
//M-------------------------------------------------------------------------
//  Method:     COdfFilter::BindRegion          (IFilter::BindRegion)
//  Summary:    Creates moniker or other interface for indicated text
//  Arguments:  origPos
//                  [in] Description of text location and extent
//              riid
//                  [in] Reference IID of specified interface
//              ppunk
//                  [out] Address that receives requested interface pointer
//  Returns:    E_NOTIMPL
//                  Always
//              FILTER_W_REGION_CLIPPED
//                  (not implemented)


SCODE STDMETHODCALLTYPE COdfFilter::BindRegion(
    FILTERREGION /*origPos*/,
    REFIID /*riid*/,
    void ** /*ppunk*/)
{
    // BindRegion is currently reserved for future use
    return E_NOTIMPL;
}
//M-------------------------------------------------------------------------
//  Method:     COdfFilter::GetClassID          (IPersist::GetClassID)
//  Summary:    Retrieves the class id of the filter class
//  Arguments:  pClassID
//                  [out] Pointer to the class ID of the filter
//  Returns:    S_OK
//                  Always
//              E_FAIL
//                  (not implemented)

HRESULT STDMETHODCALLTYPE COdfFilter::GetClassID(CLSID * pClassID)
{
    *pClassID = CLSID_COdfFilter;
    return S_OK;
}
//M-------------------------------------------------------------------------
//  Method:     COdfFilter::IsDirty             (IPersistFile::IsDirty)
//  Summary:    Checks whether file has changed since last save
//  Arguments:  void
//  Returns:    S_FALSE
//                  Always
//              S_OK
//                  (not implemented)

HRESULT STDMETHODCALLTYPE COdfFilter::IsDirty()
{
    // File is opened read-only and never changes
    return S_FALSE;
}
//M-------------------------------------------------------------------------
//  Method:     COdfFilter::Load                (IPersistFile::Load)
//  Summary:    Opens and initializes the specified file
//  Arguments:  pszFileName
//                  [in] Pointer to zero-terminated string
//                       of absolute path of file to open
//              dwMode
//                  [in] Access mode to open the file
//  Returns:    S_OK
//                  File was successfully loaded
//              E_OUTOFMEMORY
//                  File could not be loaded due to insufficient memory
//              E_FAIL
//                  (not implemented)

HRESULT STDMETHODCALLTYPE COdfFilter::Load(LPCOLESTR pszFileName, DWORD /*dwMode*/)
{
    // Load just sets the filename for GetChunk to read and ignores the mode
    m_pwszFileName = getShortPathName( pszFileName );

    // Open the file previously specified in call to IPersistFile::Load and get content.
    try
    {
        delete m_pMetaInfoReader;
        m_pMetaInfoReader = new CMetaInfoReader(m_pwszFileName);

        delete m_pContentReader;
        m_pContentReader = new CContentReader(m_pwszFileName, m_pMetaInfoReader->getDefaultLocale());
    }
    catch (const std::exception&)
    {
        return E_FAIL;
    }
    return S_OK;
}
//M-------------------------------------------------------------------------
//  Method:     COdfFilter::Save                (IPersistFile::Save)
//  Summary:    Saves a copy of the current file being filtered
//  Arguments:  pszFileName
//                  [in] Pointer to zero-terminated string of
//                       absolute path of where to save file
//              fRemember
//                  [in] Whether the saved copy is made the current file
//  Returns:    E_FAIL
//                  Always
//              S_OK
//                  (not implemented)

HRESULT STDMETHODCALLTYPE COdfFilter::Save(LPCOLESTR /*pszFileName*/, BOOL /*fRemember*/)
{
    // File is opened read-only; saving it is an error
    return E_FAIL;
}
//M-------------------------------------------------------------------------
//  Method:     COdfFilter::SaveCompleted      (IPersistFile::SaveCompleted)
//  Summary:    Determines whether a file save is completed
//  Arguments:  pszFileName
//                  [in] Pointer to zero-terminated string of
//                       absolute path where file was previously saved
//  Returns:    S_OK
//                  Always

HRESULT STDMETHODCALLTYPE COdfFilter::SaveCompleted(LPCOLESTR /*pszFileName*/)
{
    // File is opened read-only, so "save" is always finished
    return S_OK;
}

//M-------------------------------------------------------------------------
//  Method:     COdfFilter::Load      (IPersistStream::Load)
//  Summary:    Initializes an object from the stream where it was previously saved
//  Arguments:  pStm
//                  [in] Pointer to stream from which object should be loaded
//  Returns:    S_OK
//              E_OUTOFMEMORY
//              E_FAIL

HRESULT STDMETHODCALLTYPE COdfFilter::Load(IStream *pStm)
{
    m_pStream = new BufferStream(pStm);
    try
    {
        delete m_pMetaInfoReader;
        m_pMetaInfoReader = new CMetaInfoReader(m_pStream);

        delete m_pContentReader;
        m_pContentReader = new CContentReader(m_pStream, m_pMetaInfoReader->getDefaultLocale());
    }
    catch (const std::exception&)
    {
        return E_FAIL;
    }
    return S_OK;
}

//M-------------------------------------------------------------------------
//  Method:     COdfFilter::GetSizeMax      (IPersistStream::GetSizeMax)
//  Summary:    Returns the size in bytes of the stream needed to save the object.
//  Arguments:  pcbSize
//                  [out] Pointer to a 64 bit unsigned int indicating the size needed
//  Returns:    E_NOTIMPL

HRESULT STDMETHODCALLTYPE COdfFilter::GetSizeMax(ULARGE_INTEGER * /*pcbSize*/)
{
    return E_NOTIMPL;
}

//M-------------------------------------------------------------------------
//  Method:     COdfFilter::Save      (IPersistStream::Save)
//  Summary:    Save object to specified stream
//  Arguments:  pStm
//                  [in] Pointer to stream
//              fClearDirty
//                  [in] Indicates whether to clear dirty flag
//  Returns:    E_NOTIMPL

HRESULT STDMETHODCALLTYPE COdfFilter::Save(IStream * /*pStm*/, BOOL )
{
    return E_NOTIMPL;
}

//M-------------------------------------------------------------------------
//  Method:     COdfFilter::GetCurFile          (IPersistFile::GetCurFile)
//  Summary:    Returns a copy of the current file name
//  Arguments:  ppszFileName
//                  [out] Address to receive pointer to zero-terminated
//                        string for absolute path to current file
//  Returns:    S_OK
//                  A valid absolute path was successfully returned
//              S_FALSE
//                  (not implemented)
//              E_OUTOFMEMORY
//                  Operation failed due to insufficient memory
//              E_FAIL
//                  Operation failed due to some reason
//                  other than insufficient memory

HRESULT STDMETHODCALLTYPE COdfFilter::GetCurFile(LPOLESTR * ppszFileName)
{
    if ( EMPTY_STRING == m_pwszFileName )
        return E_FAIL;
    else
        *ppszFileName = const_cast<LPWSTR>(m_pwszFileName.c_str());
    return S_OK;
}

//M-------------------------------------------------------------------------
//  Method:     COdfFilterCF::COdfFilterCF
//  Summary:    Class factory constructor
//  Arguments:  void
//  Purpose:    Manages global instance count

COdfFilterCF::COdfFilterCF() :
    m_lRefs(1)
{
    InterlockedIncrement( &g_lInstances );
}
//M-------------------------------------------------------------------------
//  Method:     COdfFilterCF::~COdfFilterCF
//  Summary:    Class factory destructor
//  Arguments:  void
//  Purpose:    Manages global instance count

COdfFilterCF::~COdfFilterCF()
{
   InterlockedDecrement( &g_lInstances );
}
//M-------------------------------------------------------------------------
//  Method:     COdfFilterCF::QueryInterface    (IUnknown::QueryInterface)
//  Summary:    Queries for requested interface
//  Arguments:  riid
//                  [in] Reference IID of requested interface
//              ppvObject
//                  [out] Address that receives requested interface pointer
//  Returns:    S_OK
//                  Interface is supported
//              E_NOINTERFACE
//                  Interface is not supported

HRESULT STDMETHODCALLTYPE COdfFilterCF::QueryInterface(REFIID riid, void  ** ppvObject)
{
    IUnknown *pUnkTemp;

    if ( IID_IClassFactory == riid )
        pUnkTemp = this;
    else if ( IID_IUnknown == riid )
        pUnkTemp = this;
    else
    {
        *ppvObject = nullptr;
        return E_NOINTERFACE;
    }
    *ppvObject = pUnkTemp;
    pUnkTemp->AddRef();
    return S_OK;
}
//M-------------------------------------------------------------------------
//  Method:     COdfFilterCF::AddRef            (IUnknown::AddRef)
//  Summary:    Increments interface refcount
//  Arguments:  void
//  Returns:    Value of incremented interface refcount

ULONG STDMETHODCALLTYPE COdfFilterCF::AddRef()
{
   return InterlockedIncrement( &m_lRefs );
}
//M-------------------------------------------------------------------------
//  Method:     COdfFilterCF::Release           (IUnknown::Release)
//  Summary:    Decrements interface refcount, deleting if unreferenced
//  Arguments:  void
//  Returns:    Value of decremented refcount

ULONG STDMETHODCALLTYPE COdfFilterCF::Release()
{
    ULONG ulTmp = InterlockedDecrement( &m_lRefs );

    if ( 0 == ulTmp )
        delete this;
    return ulTmp;
}
//M-------------------------------------------------------------------------
//  Method:     COdfFilterCF::CreateInstance (IClassFactory::CreateInstance)
//  Summary:    Creates new ODF filter object
//  Arguments:  pUnkOuter
//                  [in] Pointer to IUnknown interface of aggregating object
//              riid
//                  [in] Reference IID of requested interface
//              ppvObject
//                  [out] Address that receives requested interface pointer
//  Returns:    S_OK
//                  ODF filter object was successfully created
//              CLASS_E_NOAGGREGATION
//                  pUnkOuter parameter was non-NULL
//              E_NOINTERFACE
//                  (not implemented)
//              E_OUTOFMEMORY
//                  ODF filter object could not be created
//                  due to insufficient memory
//              E_UNEXPECTED
//                  Unsuccessful due to an unexpected condition

HRESULT STDMETHODCALLTYPE COdfFilterCF::CreateInstance(
    IUnknown * pUnkOuter,
    REFIID riid,
    void  * * ppvObject)
{
    COdfFilter *pIUnk = nullptr;
    if ( nullptr != pUnkOuter )
        return CLASS_E_NOAGGREGATION;
    pIUnk = new COdfFilter();
    if ( SUCCEEDED( pIUnk->QueryInterface( riid , ppvObject ) ) )
    {
        // Release extra refcount from QueryInterface
        pIUnk->Release();
    }
    else
    {
        delete pIUnk;
        return E_UNEXPECTED;
    }
    return S_OK;
}

//M-------------------------------------------------------------------------
//  Method:     COdfFilterCF::LockServer        (IClassFactory::LockServer)
//  Summary:    Forces/allows filter class to remain loaded/be unloaded
//  Arguments:  fLock
//                  [in] TRUE to lock, FALSE to unlock
//  Returns:    S_OK
//                  Always
//              E_FAIL
//                  (not implemented)
//              E_OUTOFMEMORY
//                  (not implemented)
//              E_UNEXPECTED
//                  (not implemented)

HRESULT STDMETHODCALLTYPE COdfFilterCF::LockServer(BOOL fLock)
{
    if( fLock )
        InterlockedIncrement( &g_lInstances );
    else
        InterlockedDecrement( &g_lInstances );
    return S_OK;
}
//+-------------------------------------------------------------------------
//  DLL:        odffilter.dll
//  Summary:    Implements Dynamic Link Library functions for ODF filter

//F-------------------------------------------------------------------------
//  Function:   DllMain
//  Summary:    Called from C-Runtime on process/thread attach/detach
//  Arguments:  hInstance
//                  [in] Handle to the DLL
//              fdwReason
//                  [in] Reason for calling DLL entry point
//              lpReserve
//                  [in] Details of DLL initialization and cleanup
//  Returns:    TRUE
//                  Always

extern "C" BOOL WINAPI DllMain(
    HINSTANCE hInstance,
    DWORD     fdwReason,
    LPVOID    /*lpvReserved*/
)
{
    if ( DLL_PROCESS_ATTACH == fdwReason )
        DisableThreadLibraryCalls( hInstance );
    return TRUE;
}
//F-------------------------------------------------------------------------
//  Function:   DllGetClassObject
//  Summary:    Create ODF filter class factory object
//  Arguments:  cid
//                  [in] Class ID of class that class factory creates
//              iid
//                  [in] Reference IID of requested class factory interface
//              ppvObj
//                  [out] Address that receives requested interface pointer
//  Returns:    S_OK
//                  Class factory object was created successfully
//              CLASS_E_CLASSNOTAVAILABLE
//                  DLL does not support the requested class
//              E_INVALIDARG
//                  (not implemented
//              E_OUTOFMEMORY
//                  Insufficient memory to create the class factory object
//              E_UNEXPECTED
//                  Unsuccessful due to an unexpected condition

extern "C" HRESULT STDMETHODCALLTYPE DllGetClassObject(
    REFCLSID   cid,
    REFIID     iid,
    LPVOID *   ppvObj
)
{
    COdfFilterCF* pImpl = nullptr;
    IUnknown *pResult = nullptr;

    if ( CLSID_COdfFilter == cid )
    {
        pImpl = new COdfFilterCF;
        pResult = pImpl;
    }
    else
        return CLASS_E_CLASSNOTAVAILABLE;
    if( SUCCEEDED( pResult->QueryInterface( iid, ppvObj ) ) )
        // Release extra refcount from QueryInterface
        pResult->Release();
    else
    {
        delete pImpl;
        return E_UNEXPECTED;
    }
    return S_OK;
}
//F-------------------------------------------------------------------------
//  Function:   DllCanUnloadNow
//  Summary:    Indicates whether it is possible to unload DLL
//  Arguments:  void
//  Returns:    S_OK
//                  DLL can be unloaded now
//              S_FALSE
//                  DLL must remain loaded

extern "C" HRESULT STDMETHODCALLTYPE DllCanUnloadNow()
{
    if ( 0 >= g_lInstances )
        return S_OK;
    else
        return S_FALSE;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
