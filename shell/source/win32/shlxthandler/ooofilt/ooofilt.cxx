/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */










//


#include "internal/contentreader.hxx"
#include "internal/metainforeader.hxx"
#include "internal/registry.hxx"
#include "internal/fileextensions.hxx"













#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#include <windows.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif
#include <string.h>
#include <filter.h>
#include <filterr.h>
#include <ntquery.h>
#include "assert.h"
#include "ooofilt.hxx"
#include <objidl.h>
#include <stdio.h>
#include "propspec.hxx"
#ifdef __MINGW32__
#include <algorithm>
using ::std::min;
#endif

#include "internal/stream_helper.hxx"











COooFilter::COooFilter() :
    m_lRefs(1),
    m_pContentReader(NULL),
    m_pMetaInfoReader(NULL),
    m_eState(FilteringContent),
    m_ulUnicodeBufferLen(0),
    m_ulUnicodeCharsRead(0),
    m_ulPropertyNum(0),
    m_ulCurrentPropertyNum(0),
    m_ulChunkID(1),
    m_fContents(FALSE),
    m_fEof(FALSE),
    m_ChunkPosition(0),
    m_cAttributes(0),
    m_pAttributes(0),
    m_pStream(NULL)

{
    InterlockedIncrement( &g_lInstances );
}






COooFilter::~COooFilter()
{
    delete [] m_pAttributes;

    if (m_pContentReader)
        delete m_pContentReader;
    if (m_pMetaInfoReader)
        delete m_pMetaInfoReader;
    if (m_pStream)
        delete m_pStream;

    InterlockedDecrement( &g_lInstances );
}













SCODE STDMETHODCALLTYPE COooFilter::QueryInterface(
    REFIID riid,
    void  ** ppvObject)
{
    IUnknown *pUnkTemp = 0;
    if ( IID_IFilter == riid )
        pUnkTemp = (IUnknown *)(IFilter *)this;
    else if ( IID_IPersistFile == riid )
        pUnkTemp = (IUnknown *)(IPersistFile *)this;
    else if ( IID_IPersist == riid )
        pUnkTemp = (IUnknown *)(IPersist *)(IPersistFile *)this;
    else if (IID_IPersistStream == riid)
        pUnkTemp = (IUnknown *)(IPersistStream *)this;
    else if ( IID_IUnknown == riid )
        pUnkTemp = (IUnknown *)(IPersist *)(IPersistFile *)this;
    else
    {
        *ppvObject = NULL;
        return E_NOINTERFACE;
    }
    *ppvObject = (void  *)pUnkTemp;
    pUnkTemp->AddRef();
    return S_OK;
}






ULONG STDMETHODCALLTYPE COooFilter::AddRef()
{
    return InterlockedIncrement( &m_lRefs );
}






ULONG STDMETHODCALLTYPE COooFilter::Release()
{
    ULONG ulTmp = InterlockedDecrement( &m_lRefs );

    if ( 0 == ulTmp )
        delete this;
    return ulTmp;
}






















const int COUNT_ATTRIBUTES = 5;

SCODE STDMETHODCALLTYPE COooFilter::Init(
    ULONG grfFlags,
    ULONG cAttributes,
    FULLPROPSPEC const * aAttributes,
    ULONG * pFlags)
{
    
    *pFlags = IFILTER_FLAGS_OLE_PROPERTIES;
    try
    {
        m_fContents = FALSE;
        m_ulPropertyNum = 0;
        m_ulCurrentPropertyNum = 0;
        if ( m_cAttributes > 0 )
        {
            delete[] m_pAttributes;
            m_pAttributes = 0;
            m_cAttributes = 0;
        }
        if( 0 < cAttributes )
        {
            
            if ( 0 == aAttributes )
                return E_INVALIDARG;
            m_pAttributes = new CFullPropSpec[cAttributes];
            m_cAttributes = cAttributes;
            
            CFullPropSpec *pAttrib = (CFullPropSpec *) aAttributes;
            ULONG ulNumAttr;
            for ( ulNumAttr = 0 ; ulNumAttr < cAttributes; ulNumAttr++ )
            {
                if ( pAttrib[ulNumAttr].IsPropertyPropid() &&
                     pAttrib[ulNumAttr].GetPropertyPropid() == PID_STG_CONTENTS &&
                     pAttrib[ulNumAttr].GetPropSet() == guidStorage )
                {
                    m_fContents = TRUE;
                }
                
                m_pAttributes[ulNumAttr] = pAttrib[ulNumAttr];
            }
        }
        else if ( grfFlags & IFILTER_INIT_APPLY_INDEX_ATTRIBUTES )
        {
            
            m_fContents = TRUE;

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
            
            m_fContents = TRUE;
        }
        else
            m_fContents = FALSE;
        
        if ( m_fContents )
        {
            m_fEof = FALSE;
            m_eState = FilteringContent;
            m_ulUnicodeCharsRead = 0;
            m_ChunkPosition = 0;
        }
        else
        {
            m_fEof = TRUE;
            m_eState = FilteringProperty;
        }
        m_ulChunkID = 1;
    }
    catch (const std::exception&)
    {
        return E_FAIL;
    }

    return S_OK;
}




















SCODE STDMETHODCALLTYPE COooFilter::GetChunk(STAT_CHUNK * pStat)
{
    for(;;)
    {
        switch ( m_eState )
        {
        case FilteringContent:
        {
            
            if( m_ChunkPosition == m_pContentReader ->getChunkBuffer().size() )
            {
                m_ulUnicodeBufferLen=0;
                m_fEof = TRUE;
            }

            if ( !m_fContents || m_fEof )
            {
                m_eState = FilteringProperty;
                continue;
            }
            m_pwsBuffer = m_pContentReader -> getChunkBuffer()[m_ChunkPosition].second;
            m_ulUnicodeBufferLen = static_cast<ULONG>(m_pwsBuffer.length());
            DWORD ChunkLCID = LocaleSetToLCID( m_pContentReader -> getChunkBuffer()[m_ChunkPosition].first );
            
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
        case FilteringProperty:
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
        default:
            return E_FAIL;
        }
    }
}















SCODE STDMETHODCALLTYPE COooFilter::GetText(ULONG * pcwcBuffer, WCHAR * awcBuffer)
{
    switch ( m_eState )
    {
    case FilteringProperty:
        return FILTER_E_NO_TEXT;
    case FilteringContent:
    {
        if ( !m_fContents || 0 == m_ulUnicodeBufferLen )
        {
            *pcwcBuffer = 0;
            return FILTER_E_NO_MORE_TEXT;
        }
        
        ULONG ulToCopy = min( *pcwcBuffer, m_ulUnicodeBufferLen - m_ulUnicodeCharsRead );
        ZeroMemory(awcBuffer, sizeof(awcBuffer));
        wmemcpy( awcBuffer, m_pwsBuffer.c_str() + m_ulUnicodeCharsRead, ulToCopy );
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
    default:
        return E_FAIL;
    }
}









::std::wstring GetMetaInfoNameFromPropertyId( ULONG ulPropID )
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











SCODE STDMETHODCALLTYPE COooFilter::GetValue(PROPVARIANT ** ppPropValue)
{
    if (m_eState == FilteringContent)
        return FILTER_E_NO_VALUES;
    else if (m_eState == FilteringProperty)
    {
        if ( m_cAttributes == 0 || ( m_ulCurrentPropertyNum == m_ulPropertyNum ) )
            return FILTER_E_NO_MORE_VALUES;
        PROPVARIANT *pPropVar = (PROPVARIANT *) CoTaskMemAlloc( sizeof (PROPVARIANT) );
        if ( pPropVar == 0 )
            return E_OUTOFMEMORY;
        ::std::wstring wsTagName= GetMetaInfoNameFromPropertyId( m_pAttributes[m_ulCurrentPropertyNum].GetPropertyPropid() );
        if ( wsTagName == EMPTY_STRING )
            return FILTER_E_NO_VALUES;
        ::std::wstring wsTagData = m_pMetaInfoReader->getTagData(wsTagName);
        pPropVar->vt = VT_LPWSTR;
        size_t cw = wsTagData.length() + 1; 
        pPropVar->pwszVal = static_cast<WCHAR*>( CoTaskMemAlloc(cw*sizeof(WCHAR)) );
        if (pPropVar->pwszVal == 0)
        {
            CoTaskMemFree(pPropVar);
            return E_OUTOFMEMORY;
        }
        wmemcpy(pPropVar->pwszVal, wsTagData.c_str(), cw);
        *ppPropValue = pPropVar;
        m_ulCurrentPropertyNum = m_ulPropertyNum;
        return S_OK;
    }
    else
        return E_FAIL;
}















SCODE STDMETHODCALLTYPE COooFilter::BindRegion(
    FILTERREGION /*origPos*/,
    REFIID /*riid*/,
    void ** /*ppunk*/)
{
    
    return E_NOTIMPL;
}










SCODE STDMETHODCALLTYPE COooFilter::GetClassID(CLSID * pClassID)
{
    *pClassID = CLSID_COooFilter;
    return S_OK;
}









SCODE STDMETHODCALLTYPE COooFilter::IsDirty()
{
    
    return S_FALSE;
}















SCODE STDMETHODCALLTYPE COooFilter::Load(LPCWSTR pszFileName, DWORD /*dwMode*/)
{
    
    m_pwszFileName = getShortPathName( pszFileName );

    
    try
    {
        if (m_pMetaInfoReader)
            delete m_pMetaInfoReader;
        m_pMetaInfoReader = new CMetaInfoReader(WStringToString(m_pwszFileName));

        if (m_pContentReader)
            delete m_pContentReader;
        m_pContentReader = new CContentReader(WStringToString(m_pwszFileName), m_pMetaInfoReader->getDefaultLocale());
    }
    catch (const std::exception&)
    {
        return E_FAIL;
    }
    return S_OK;
}













SCODE STDMETHODCALLTYPE COooFilter::Save(LPCWSTR /*pszFileName*/, BOOL /*fRemember*/)
{
    
    return E_FAIL;
}









SCODE STDMETHODCALLTYPE COooFilter::SaveCompleted(LPCWSTR /*pszFileName*/)
{
    
    return S_OK;
}










SCODE STDMETHODCALLTYPE COooFilter::Load(IStream *pStm)
{
    m_pStream = new BufferStream(pStm);
    try
    {
        if (m_pMetaInfoReader)
            delete m_pMetaInfoReader;
        m_pMetaInfoReader = new CMetaInfoReader(m_pStream);

        if (m_pContentReader)
            delete m_pContentReader;
        m_pContentReader = new CContentReader(m_pStream, m_pMetaInfoReader->getDefaultLocale());
    }
    catch (const std::exception&)
    {
        return E_FAIL;
    }
    return S_OK;
}








SCODE STDMETHODCALLTYPE COooFilter::GetSizeMax(ULARGE_INTEGER * /*pcbSize*/)
{
    return E_NOTIMPL;
}










SCODE STDMETHODCALLTYPE COooFilter::Save(IStream * /*pStm*/, BOOL )
{
    return E_NOTIMPL;
}

















SCODE STDMETHODCALLTYPE COooFilter::GetCurFile(LPWSTR * ppszFileName)
{
    if ( EMPTY_STRING == m_pwszFileName )
        return E_FAIL;
    else
        *ppszFileName = (LPWSTR)m_pwszFileName.c_str();
    return S_OK;
}







COooFilterCF::COooFilterCF() :
    m_lRefs(1)
{
    InterlockedIncrement( &g_lInstances );
}






COooFilterCF::~COooFilterCF()
{
   InterlockedDecrement( &g_lInstances );
}












SCODE STDMETHODCALLTYPE COooFilterCF::QueryInterface(REFIID riid, void  ** ppvObject)
{
    IUnknown *pUnkTemp;

    if ( IID_IClassFactory == riid )
        pUnkTemp = (IUnknown *)(IClassFactory *)this;
    else if ( IID_IUnknown == riid )
        pUnkTemp = (IUnknown *)this;
    else
    {
        *ppvObject = NULL;
        return E_NOINTERFACE;
    }
    *ppvObject = (void  *)pUnkTemp;
    pUnkTemp->AddRef();
    return S_OK;
}






ULONG STDMETHODCALLTYPE COooFilterCF::AddRef()
{
   return InterlockedIncrement( &m_lRefs );
}






ULONG STDMETHODCALLTYPE COooFilterCF::Release()
{
    ULONG ulTmp = InterlockedDecrement( &m_lRefs );

    if ( 0 == ulTmp )
        delete this;
    return ulTmp;
}





















SCODE STDMETHODCALLTYPE COooFilterCF::CreateInstance(
    IUnknown * pUnkOuter,
    REFIID riid,
    void  * * ppvObject)
{
    COooFilter *pIUnk = 0;
    if ( 0 != pUnkOuter )
        return CLASS_E_NOAGGREGATION;
    pIUnk = new COooFilter();
    if ( SUCCEEDED( pIUnk->QueryInterface( riid , ppvObject ) ) )
    {
        
        pIUnk->Release();
    }
    else
    {
        delete pIUnk;
        return E_UNEXPECTED;
    }
    return S_OK;
}















SCODE STDMETHODCALLTYPE COooFilterCF::LockServer(BOOL fLock)
{
    if( fLock )
        InterlockedIncrement( &g_lInstances );
    else
        InterlockedDecrement( &g_lInstances );
    return S_OK;
}
















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




















extern "C" SCODE STDMETHODCALLTYPE DllGetClassObject(
    REFCLSID   cid,
    REFIID     iid,
    void **    ppvObj
)
{
    COooFilterCF* pImpl = 0;
    IUnknown *pResult = 0;

    if ( CLSID_COooFilter == cid )
    {
        pImpl = new COooFilterCF;
        pResult = (IUnknown *) pImpl;
    }
    else
        return CLASS_E_CLASSNOTAVAILABLE;
    if( SUCCEEDED( pResult->QueryInterface( iid, ppvObj ) ) )
        
        pResult->Release();
    else
    {
        delete pImpl;
        return E_UNEXPECTED;
    }
    return S_OK;
}









extern "C" SCODE STDMETHODCALLTYPE DllCanUnloadNow()
{
    if ( 0 >= g_lInstances )
        return S_OK;
    else
        return S_FALSE;
}




































namespace /* private */
{
    const char* GUID_PLACEHOLDER         = "{GUID}";
    const char* GUID_PERSIST_PLACEHOLDER = "{GUIDPERSIST}";
    const char* EXTENSION_PLACEHOLDER    = "{EXT}";

    const char* CLSID_GUID_INPROC_ENTRY             = "CLSID\\{GUID}\\InProcServer32";
    const char* CLSID_GUID_ENTRY                    = "CLSID\\{GUID}";
    const char* CLSID_GUID_PERSIST_ADDIN_ENTRY      = "CLSID\\{GUID}\\PersistentAddinsRegistered\\{GUIDPERSIST}";
    const char* CLSID_PERSIST_ENTRY                 = "CLSID\\{GUID}\\PersistentHandler";
    const char* EXT_PERSIST_ENTRY                   = "{EXT}\\PersistentHandler";

    const char* INDEXING_FILTER_DLLSTOREGISTER      = "SYSTEM\\CurrentControlSet\\Control\\ContentIndex";

    
    
    
    

    void SubstitutePlaceholder(std::string& String, const std::string& Placeholder, const std::string& Replacement)
    {
        std::string::size_type idx = String.find(Placeholder);
        std::string::size_type len = Placeholder.length();

        while (std::string::npos != idx)
        {
            String.replace(idx, len, Replacement);
            idx = String.find(Placeholder);
        }
    }

    
    
    
    
    
    

    HRESULT RegisterFilterHandler(const char* FilePath, const CLSID& FilterGuid)
    {
        std::string ClsidEntry = CLSID_GUID_ENTRY;
        SubstitutePlaceholder(ClsidEntry, GUID_PLACEHOLDER, ClsidToString(FilterGuid));

        if (!SetRegistryKey(HKEY_CLASSES_ROOT, ClsidEntry.c_str(), "", "LibreOffice Filter"))
            return E_FAIL;

        ClsidEntry = CLSID_GUID_INPROC_ENTRY;
        SubstitutePlaceholder(ClsidEntry, GUID_PLACEHOLDER, ClsidToString(FilterGuid));

        if (!SetRegistryKey(HKEY_CLASSES_ROOT, ClsidEntry.c_str(), "", FilePath))
            return E_FAIL;

        if (!SetRegistryKey(HKEY_CLASSES_ROOT, ClsidEntry.c_str(), "ThreadingModel", "Both"))
            return E_FAIL;

        return S_OK;
    }

    
    
    
    
    
    

    HRESULT RegisterPersistentHandler(const CLSID& FilterGuid, const CLSID& PersistentGuid)
    {
        std::string ClsidEntry_Persist = CLSID_GUID_ENTRY;
        SubstitutePlaceholder(ClsidEntry_Persist, GUID_PLACEHOLDER, ClsidToString(PersistentGuid));


        if (!SetRegistryKey(HKEY_CLASSES_ROOT, ClsidEntry_Persist.c_str(), "", "LibreOffice Persistent Handler"))
            return E_FAIL;

        
        std::string ClsidEntry_Persist_Entry = CLSID_PERSIST_ENTRY;
        SubstitutePlaceholder(ClsidEntry_Persist_Entry,
                              GUID_PLACEHOLDER,
                              ClsidToString(PersistentGuid));

        if (!SetRegistryKey(HKEY_CLASSES_ROOT, ClsidEntry_Persist_Entry.c_str(), "", ClsidToString(PersistentGuid).c_str()))
            return E_FAIL;

        std::string ClsidEntry_Persist_Addin = CLSID_GUID_PERSIST_ADDIN_ENTRY;
        SubstitutePlaceholder(ClsidEntry_Persist_Addin,
                              GUID_PLACEHOLDER,
                              ClsidToString(PersistentGuid));
        SubstitutePlaceholder(ClsidEntry_Persist_Addin,
                              GUID_PERSIST_PLACEHOLDER,
                              ClsidToString(CLSID_PERSISTENT_HANDLER_ADDIN));

        if (!SetRegistryKey(HKEY_CLASSES_ROOT, ClsidEntry_Persist_Addin.c_str(), "", ClsidToString(FilterGuid).c_str() ))
            return E_FAIL;

        return S_OK;
    }

    
    
    

    HRESULT UnregisterHandler(const CLSID& Guid)
    {
        std::string tmp = "CLSID\\";
        tmp += ClsidToString(Guid);
        return DeleteRegistryKey(HKEY_CLASSES_ROOT, tmp.c_str()) ? S_OK : E_FAIL;
    }

    
    
    
    
    

    HRESULT RegisterSearchHandler(const char* ModuleFileName)
    {
        if (FAILED(RegisterFilterHandler(ModuleFileName, CLSID_FILTER_HANDLER)))
            return E_FAIL;

        if (FAILED(RegisterPersistentHandler(CLSID_FILTER_HANDLER, CLSID_PERSISTENT_HANDLER )))
            return E_FAIL;

        std::string sExtPersistEntry;

        for(size_t i = 0; i < OOFileExtensionTableSize; i++)
        {
            
            sExtPersistEntry = EXT_PERSIST_ENTRY;
            SubstitutePlaceholder(sExtPersistEntry, EXTENSION_PLACEHOLDER, OOFileExtensionTable[i].ExtensionAnsi);
            if (!SetRegistryKey(HKEY_CLASSES_ROOT,
                                sExtPersistEntry.c_str(),
                                "",
                                ClsidToString(CLSID_PERSISTENT_HANDLER).c_str()))
                return E_FAIL;

            
            char extClassName[MAX_PATH];
            if (QueryRegistryKey(HKEY_CLASSES_ROOT, OOFileExtensionTable[i].ExtensionAnsi, "", extClassName,MAX_PATH))
            {
                ::std::string extCLSIDName( extClassName );
                extCLSIDName += "\\CLSID";
                char extCLSID[MAX_PATH];

                if (QueryRegistryKey( HKEY_CLASSES_ROOT, extCLSIDName.c_str(), "", extCLSID, MAX_PATH))
                {
                    std::string ClsidEntry_CLSID_Persist = CLSID_PERSIST_ENTRY;
                    SubstitutePlaceholder(ClsidEntry_CLSID_Persist,
                                        GUID_PLACEHOLDER,
                                        extCLSID);

                    if (!SetRegistryKey(HKEY_CLASSES_ROOT,
                                        ClsidEntry_CLSID_Persist.c_str(),
                                        "",
                                        ClsidToString(CLSID_PERSISTENT_HANDLER).c_str() ))
                        return E_FAIL;
                }
            }
        }

        return S_OK;
    }

    
    HRESULT UnregisterSearchHandler()
    {
        std::string sExtPersistEntry;

        for (size_t i = 0; i < OOFileExtensionTableSize; i++)
        {
            
            sExtPersistEntry = EXT_PERSIST_ENTRY;
            SubstitutePlaceholder(sExtPersistEntry, EXTENSION_PLACEHOLDER, OOFileExtensionTable[i].ExtensionAnsi);
            DeleteRegistryKey(HKEY_CLASSES_ROOT, sExtPersistEntry.c_str());

            
            char extClassName[MAX_PATH];
            if (QueryRegistryKey(HKEY_CLASSES_ROOT, OOFileExtensionTable[i].ExtensionAnsi, "", extClassName,MAX_PATH))
            {
                ::std::string extCLSIDName( extClassName );
                extCLSIDName += "\\CLSID";
                char extCLSID[MAX_PATH];

                if (QueryRegistryKey( HKEY_CLASSES_ROOT, extCLSIDName.c_str(), "", extCLSID, MAX_PATH))
                {
                    std::string ClsidEntry_CLSID_Persist = CLSID_PERSIST_ENTRY;
                    SubstitutePlaceholder(ClsidEntry_CLSID_Persist,
                                        GUID_PLACEHOLDER,
                                        extCLSID);

                    DeleteRegistryKey(HKEY_CLASSES_ROOT, ClsidEntry_CLSID_Persist.c_str());
                }
            }
        }

        return ((UnregisterHandler(CLSID_FILTER_HANDLER)==S_OK) && (UnregisterHandler(CLSID_PERSISTENT_HANDLER)==S_OK))?S_OK:E_FAIL;
    }

    
    
    
    
    
    HRESULT AddOrRemoveDllsToRegisterList( const ::std::string & DllPath, bool isAdd )
    {
        char DllsToRegisterList[4096];
        if (QueryRegistryKey(HKEY_LOCAL_MACHINE,
                             INDEXING_FILTER_DLLSTOREGISTER,
                             "DLLsToRegister",
                             DllsToRegisterList,
                             4096))
        {
            char * pChar = DllsToRegisterList;
            for ( ; *pChar != '\0' || *(pChar +1) != '\0'; pChar++)
                if ( *pChar == '\0')
                    *pChar = ';';
            *pChar = ';';
            *(pChar+1) = '\0';

            ::std::string DllList(DllsToRegisterList);
            if ( ( isAdd )&&( DllList.find( DllPath ) == ::std::string::npos ) )
                DllList.append( DllPath );
            else if ( ( !isAdd )&&( DllList.find( DllPath ) != ::std::string::npos ) )
                DllList.erase( DllList.find( DllPath )-1, DllPath.length()+1 );
            else
                return S_OK;

            pChar = DllsToRegisterList;
            for ( size_t nChar = 0; nChar < DllList.length(); pChar++,nChar++)
            {
                if ( DllList[nChar] == ';')
                    *pChar = '\0';
                else
                    *pChar = DllList[nChar];
            }
            *pChar = *( pChar+1 ) ='\0';

            HKEY hSubKey;
            char dummy[] = "";
            int rc = RegCreateKeyExA(HKEY_LOCAL_MACHINE,
                                    INDEXING_FILTER_DLLSTOREGISTER,
                                    0,
                                    dummy,
                                    REG_OPTION_NON_VOLATILE,
                                    KEY_WRITE,
                                    0,
                                    &hSubKey,
                                    0);

            if (ERROR_SUCCESS == rc)
            {
                rc = RegSetValueExA( hSubKey,
                                    "DLLsToRegister",
                                    0,
                                    REG_MULTI_SZ,
                                    reinterpret_cast<const BYTE*>(DllsToRegisterList),
                                    static_cast<DWORD>(DllList.length() + 2));

                RegCloseKey(hSubKey);
            }

            return (ERROR_SUCCESS == rc)?S_OK:E_FAIL;
        }

        return S_OK;
    }

} 

STDAPI DllRegisterServer()
{
    return S_OK;
}



STDAPI DllUnregisterServer()
{
    return S_OK;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
