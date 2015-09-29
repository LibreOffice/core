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

#ifndef INCLUDED_EMBEDSERV_SOURCE_INC_EMBEDDOC_HXX
#define INCLUDED_EMBEDSERV_SOURCE_INC_EMBEDDOC_HXX
#ifdef _MSC_VER
#pragma warning(disable : 4917 4555)
#endif

#include "common.h"
#include <oleidl.h>
#include <objidl.h>

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/SEQUENCE.h>
#include <unordered_map>

#include "embeddocaccess.hxx"
#include "docholder.hxx"

typedef std::unordered_map< DWORD, IAdviseSink* > AdviseSinkHashMap;
typedef std::unordered_map< DWORD, IAdviseSink* >::iterator AdviseSinkHashMapIterator;

class GDIMetaFile;
class CIIAObj;

class EmbedDocument_Impl
    : public IPersistStorage,
      public IDataObject,
      public IOleObject,
      public IOleInPlaceObject,
      public IPersistFile,
      public IDispatch,
      public IExternalConnection
{
protected:
    css::uno::Sequence< css::beans::PropertyValue >
                fillArgsForLoading_Impl( css::uno::Reference< css::io::XInputStream > xStream,
                                         DWORD nStreamMode,
                                         LPCOLESTR pFilePath = NULL );
    css::uno::Sequence< css::beans::PropertyValue >
                fillArgsForStoring_Impl( css::uno::Reference< css::io::XOutputStream > xStream );

    HRESULT SaveTo_Impl( IStorage* pStg );

    sal_uInt64 getMetaFileHandle_Impl( sal_Bool isEnhMeta );

public:
    EmbedDocument_Impl( const css::uno::Reference< css::lang::XMultiServiceFactory >& smgr,
                        const GUID* guid );
    virtual ~EmbedDocument_Impl();

    /* IUnknown methods */
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID FAR * ppvObj);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

    /* IPersistMethod */
    STDMETHOD(GetClassID)(CLSID *pClassID);

    /* IPersistStorage methods */
    STDMETHOD(IsDirty) ();
    STDMETHOD(InitNew) ( IStorage *pStg );
    STDMETHOD(Load) ( IStorage* pStr );
    STDMETHOD(Save) ( IStorage *pStgSave, BOOL fSameAsLoad );
    STDMETHOD(SaveCompleted) ( IStorage *pStgNew );
    STDMETHOD(HandsOffStorage) (void);

    /* IDataObject methods */
    STDMETHOD(GetData) ( FORMATETC * pFormatetc, STGMEDIUM * pMedium );
    STDMETHOD(GetDataHere) ( FORMATETC * pFormatetc, STGMEDIUM * pMedium );
    STDMETHOD(QueryGetData) ( FORMATETC * pFormatetc );
    STDMETHOD(GetCanonicalFormatEtc) ( FORMATETC * pFormatetcIn, FORMATETC * pFormatetcOut );
    STDMETHOD(SetData) ( FORMATETC * pFormatetc, STGMEDIUM * pMedium, BOOL fRelease );
    STDMETHOD(EnumFormatEtc) ( DWORD dwDirection, IEnumFORMATETC ** ppFormatetc );
    STDMETHOD(DAdvise) ( FORMATETC * pFormatetc, DWORD advf, IAdviseSink * pAdvSink, DWORD * pdwConnection );
    STDMETHOD(DUnadvise) ( DWORD dwConnection );
    STDMETHOD(EnumDAdvise) ( IEnumSTATDATA ** ppenumAdvise );

    /* IOleObject methods */
    STDMETHOD(SetClientSite) ( IOleClientSite* pSite );
    STDMETHOD(GetClientSite) ( IOleClientSite** pSite );
    STDMETHOD(SetHostNames) ( LPCOLESTR szContainerApp, LPCOLESTR szContainerObj );
    STDMETHOD(Close) ( DWORD dwSaveOption);
    STDMETHOD(SetMoniker) ( DWORD dwWhichMoniker, IMoniker *pmk );
    STDMETHOD(GetMoniker) ( DWORD dwAssign, DWORD dwWhichMoniker, IMoniker **ppmk );
    STDMETHOD(InitFromData) ( IDataObject *pDataObject, BOOL fCreation, DWORD dwReserved );
    STDMETHOD(GetClipboardData) ( DWORD dwReserved, IDataObject **ppDataObject );
    STDMETHOD(DoVerb) ( LONG iVerb, LPMSG lpmsg, IOleClientSite *pActiveSite, LONG lindex, HWND hwndParent, LPCRECT lprcPosRect );
    STDMETHOD(EnumVerbs) ( IEnumOLEVERB **ppEnumOleVerb );
    STDMETHOD(Update) ();
    STDMETHOD(IsUpToDate) ();
    STDMETHOD(GetUserClassID) ( CLSID *pClsid );
    STDMETHOD(GetUserType) ( DWORD dwFormOfType, LPOLESTR *pszUserType );
    STDMETHOD(SetExtent) ( DWORD dwDrawAspect, SIZEL *psizel );
    STDMETHOD(GetExtent) ( DWORD dwDrawAspect, SIZEL *psizel );
    STDMETHOD(Advise) ( IAdviseSink *pAdvSink, DWORD *pdwConnection );
    STDMETHOD(Unadvise) ( DWORD dwConnection );
    STDMETHOD(EnumAdvise) ( IEnumSTATDATA **ppenumAdvise );
    STDMETHOD(GetMiscStatus) ( DWORD dwAspect, DWORD *pdwStatus );
    STDMETHOD(SetColorScheme) ( LOGPALETTE *pLogpal );

    /* IOleInPlaceObject methods */
    STDMETHOD(GetWindow)(HWND *);
    STDMETHOD(ContextSensitiveHelp)(BOOL);
    STDMETHOD(InPlaceDeactivate)();
    STDMETHOD(UIDeactivate)();
    STDMETHOD(SetObjectRects)(LPCRECT, LPCRECT);
    STDMETHOD(ReactivateAndUndo)();

    /* IPersistFile methods */
    STDMETHOD(Load) ( LPCOLESTR pszFileName, DWORD dwMode );
    STDMETHOD(Save) ( LPCOLESTR pszFileName, BOOL fRemember );
    STDMETHOD(SaveCompleted) ( LPCOLESTR pszFileName );
    STDMETHOD(GetCurFile) ( LPOLESTR *ppszFileName );

    /* IDispatch methods */
    STDMETHOD(GetTypeInfoCount) ( unsigned int FAR*  pctinfo );
    STDMETHOD(GetTypeInfo) ( unsigned int iTInfo, LCID lcid, ITypeInfo FAR* FAR* ppTInfo );
    STDMETHOD(GetIDsOfNames) ( REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgDispId );
    STDMETHOD(Invoke) ( DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR* pDispParams, VARIANT FAR* pVarResult, EXCEPINFO FAR* pExcepInfo, unsigned int FAR* puArgErr );

    /* IExternalConnection methods */
    virtual DWORD STDMETHODCALLTYPE AddConnection( DWORD extconn, DWORD reserved);
    virtual DWORD STDMETHODCALLTYPE ReleaseConnection( DWORD extconn, DWORD reserved, BOOL fLastReleaseCloses);

    // c++ - methods

    void notify( bool bDataChanged = true );
    HRESULT SaveObject();
    HRESULT ShowObject();
    GUID GetGUID() const { return m_guid; }
    HRESULT OLENotifyClosing();

    void Deactivate();
    HRESULT OLENotifyDeactivation();

protected:
    oslInterlockedCount                 m_refCount;

    css::uno::Reference< css::lang::XMultiServiceFactory > m_xFactory;

    DocumentHolder*                     m_pDocHolder;
    OUString                     m_aFileName;

    CComPtr< IStorage >                 m_pMasterStorage;
    CComPtr< IStream >                  m_pOwnStream;
    CComPtr< IStream >                  m_pExtStream;
    GUID                                m_guid;

    sal_Bool                            m_bIsDirty;

    CComPtr< IOleClientSite >           m_pClientSite;
    CComPtr< IDataAdviseHolder >        m_pDAdviseHolder;

    AdviseSinkHashMap                   m_aAdviseHashMap;
    DWORD                               m_nAdviseNum;

    ::rtl::Reference< EmbeddedDocumentInstanceAccess_Impl > m_xOwnAccess;

    sal_Bool                            m_bIsInVerbHandling;
};

class BooleanGuard_Impl
{
    sal_Bool& m_bValue;

public:
    BooleanGuard_Impl( sal_Bool& bValue )
    : m_bValue( bValue )
    {
        m_bValue = sal_True;
    }

    ~BooleanGuard_Impl()
    {
        m_bValue = sal_False;
    }
};

#endif // INCLUDED_EMBEDSERV_SOURCE_INC_EMBEDDOC_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
