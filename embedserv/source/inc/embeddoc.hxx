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

#include "common.h"
#include <oleidl.h>
#include <objidl.h>

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Sequence.h>
#include <unordered_map>

#include "embeddocaccess.hxx"
#include "docholder.hxx"

typedef std::unordered_map< DWORD, IAdviseSink* > AdviseSinkHashMap;

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
                fillArgsForLoading_Impl( css::uno::Reference< css::io::XInputStream > const & xStream,
                                         DWORD nStreamMode,
                                         LPCOLESTR pFilePath = nullptr );
    css::uno::Sequence< css::beans::PropertyValue >
                fillArgsForStoring_Impl( css::uno::Reference< css::io::XOutputStream > const & xStream );

    HRESULT SaveTo_Impl( IStorage* pStg );

    sal_uInt64 getMetaFileHandle_Impl( bool isEnhMeta );

public:
    EmbedDocument_Impl( const css::uno::Reference< css::lang::XMultiServiceFactory >& smgr,
                        const GUID* guid );
    virtual ~EmbedDocument_Impl();

    /* IUnknown methods */
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID* ppvObj) override;
    STDMETHOD_(ULONG, AddRef)() override;
    STDMETHOD_(ULONG, Release)() override;

    /* IPersistMethod */
    STDMETHOD(GetClassID)(CLSID *pClassID) override;

    /* IPersistStorage methods */
    STDMETHOD(IsDirty) () override;
    STDMETHOD(InitNew) ( IStorage *pStg ) override;
    STDMETHOD(Load) ( IStorage* pStr ) override;
    STDMETHOD(Save) ( IStorage *pStgSave, BOOL fSameAsLoad ) override;
    STDMETHOD(SaveCompleted) ( IStorage *pStgNew ) override;
    STDMETHOD(HandsOffStorage) (void) override;

    /* IDataObject methods */
    STDMETHOD(GetData) ( FORMATETC * pFormatetc, STGMEDIUM * pMedium ) override;
    STDMETHOD(GetDataHere) ( FORMATETC * pFormatetc, STGMEDIUM * pMedium ) override;
    STDMETHOD(QueryGetData) ( FORMATETC * pFormatetc ) override;
    STDMETHOD(GetCanonicalFormatEtc) ( FORMATETC * pFormatetcIn, FORMATETC * pFormatetcOut ) override;
    STDMETHOD(SetData) ( FORMATETC * pFormatetc, STGMEDIUM * pMedium, BOOL fRelease ) override;
    STDMETHOD(EnumFormatEtc) ( DWORD dwDirection, IEnumFORMATETC ** ppFormatetc ) override;
    STDMETHOD(DAdvise) ( FORMATETC * pFormatetc, DWORD advf, IAdviseSink * pAdvSink, DWORD * pdwConnection ) override;
    STDMETHOD(DUnadvise) ( DWORD dwConnection ) override;
    STDMETHOD(EnumDAdvise) ( IEnumSTATDATA ** ppenumAdvise ) override;

    /* IOleObject methods */
    STDMETHOD(SetClientSite) ( IOleClientSite* pSite ) override;
    STDMETHOD(GetClientSite) ( IOleClientSite** pSite ) override;
    STDMETHOD(SetHostNames) ( LPCOLESTR szContainerApp, LPCOLESTR szContainerObj ) override;
    STDMETHOD(Close) ( DWORD dwSaveOption) override;
    STDMETHOD(SetMoniker) ( DWORD dwWhichMoniker, IMoniker *pmk ) override;
    STDMETHOD(GetMoniker) ( DWORD dwAssign, DWORD dwWhichMoniker, IMoniker **ppmk ) override;
    STDMETHOD(InitFromData) ( IDataObject *pDataObject, BOOL fCreation, DWORD dwReserved ) override;
    STDMETHOD(GetClipboardData) ( DWORD dwReserved, IDataObject **ppDataObject ) override;
    STDMETHOD(DoVerb) ( LONG iVerb, LPMSG lpmsg, IOleClientSite *pActiveSite, LONG lindex, HWND hwndParent, LPCRECT lprcPosRect ) override;
    STDMETHOD(EnumVerbs) ( IEnumOLEVERB **ppEnumOleVerb ) override;
    STDMETHOD(Update) () override;
    STDMETHOD(IsUpToDate) () override;
    STDMETHOD(GetUserClassID) ( CLSID *pClsid ) override;
    STDMETHOD(GetUserType) ( DWORD dwFormOfType, LPOLESTR *pszUserType ) override;
    STDMETHOD(SetExtent) ( DWORD dwDrawAspect, SIZEL *psizel ) override;
    STDMETHOD(GetExtent) ( DWORD dwDrawAspect, SIZEL *psizel ) override;
    STDMETHOD(Advise) ( IAdviseSink *pAdvSink, DWORD *pdwConnection ) override;
    STDMETHOD(Unadvise) ( DWORD dwConnection ) override;
    STDMETHOD(EnumAdvise) ( IEnumSTATDATA **ppenumAdvise ) override;
    STDMETHOD(GetMiscStatus) ( DWORD dwAspect, DWORD *pdwStatus ) override;
    STDMETHOD(SetColorScheme) ( LOGPALETTE *pLogpal ) override;

    /* IOleInPlaceObject methods */
    STDMETHOD(GetWindow)(HWND *) override;
    STDMETHOD(ContextSensitiveHelp)(BOOL) override;
    STDMETHOD(InPlaceDeactivate)() override;
    STDMETHOD(UIDeactivate)() override;
    STDMETHOD(SetObjectRects)(LPCRECT, LPCRECT) override;
    STDMETHOD(ReactivateAndUndo)() override;

    /* IPersistFile methods */
    STDMETHOD(Load) ( LPCOLESTR pszFileName, DWORD dwMode ) override;
    STDMETHOD(Save) ( LPCOLESTR pszFileName, BOOL fRemember ) override;
    STDMETHOD(SaveCompleted) ( LPCOLESTR pszFileName ) override;
    STDMETHOD(GetCurFile) ( LPOLESTR *ppszFileName ) override;

    /* IDispatch methods */
    STDMETHOD(GetTypeInfoCount)(unsigned int* pctinfo) override;
    STDMETHOD(GetTypeInfo)(unsigned int iTInfo, LCID lcid, ITypeInfo** ppTInfo) override;
    STDMETHOD(GetIDsOfNames)(REFIID riid, OLECHAR** rgszNames, unsigned int cNames, LCID lcid, DISPID* rgDispId) override;
    STDMETHOD(Invoke)(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, unsigned int* puArgErr) override;

    /* IExternalConnection methods */
    virtual DWORD STDMETHODCALLTYPE AddConnection( DWORD extconn, DWORD reserved) override;
    virtual DWORD STDMETHODCALLTYPE ReleaseConnection( DWORD extconn, DWORD reserved, BOOL fLastReleaseCloses) override;

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

    bool                                m_bIsDirty;

    CComPtr< IOleClientSite >           m_pClientSite;
    CComPtr< IDataAdviseHolder >        m_pDAdviseHolder;

    AdviseSinkHashMap                   m_aAdviseHashMap;
    DWORD                               m_nAdviseNum;

    ::rtl::Reference< EmbeddedDocumentInstanceAccess_Impl > m_xOwnAccess;

    bool                                m_bIsInVerbHandling;
};

class BooleanGuard_Impl
{
    bool& m_bValue;

public:
    BooleanGuard_Impl( bool& bValue )
    : m_bValue( bValue )
    {
        m_bValue = true;
    }

    ~BooleanGuard_Impl()
    {
        m_bValue = false;
    }
};

#endif // INCLUDED_EMBEDSERV_SOURCE_INC_EMBEDDOC_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
