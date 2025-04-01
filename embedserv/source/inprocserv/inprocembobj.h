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

#pragma once


#if !defined WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <oleidl.h>

#include <systools/win32/comtools.hxx>
#include "advisesink.hxx"

 256

namespace inprocserv {

enum InitModes {
    NOINIT,
    INIT_FROM_STORAGE,
    LOAD_FROM_STORAGE,
    LOAD_FROM_FILE
};


// this is a common baseclass that is used to count the objects

class InprocCountedObject_Impl
{
public:
    InprocCountedObject_Impl();
    ~InprocCountedObject_Impl();
};


// this is the inprocess embedded object implementation class

class InprocEmbedDocument_Impl : public InprocCountedObject_Impl
                               , public IOleObject
                               , public IDataObject
                               , public IPersistStorage
                               , public IPersistFile
                               , public IRunnableObject
                               , public IViewObject2
                               // , public IExternalConnection
                               , public IOleInPlaceObject
                               , public IDispatch
{
    ULONG m_refCount;
    BOOLEAN m_bDeleted;

    GUID  m_guid;

    sal::systools::COMReference< IUnknown > m_pDefHandler;
    InitModes m_nInitMode;

    DWORD m_nFileOpenMode;
    wchar_t* m_pFileName;

    sal::systools::COMReference< IStorage > m_pStorage;

    sal::systools::COMReference< IOleClientSite > m_pClientSite;
    sal::systools::COMReference< IOleContainer >  m_pOleContainer;

    ULONG m_nCallsOnStack;

    // the listeners have wrappers that are directly connected to the object and call the listeners,
    // the wrappers will be reconnected correctly to the new default inprocess holder object
    sal::systools::COMReference< OleWrapperAdviseSink > m_pOleAdvises[DEFAULT_ARRAY_LEN];
    sal::systools::COMReference< OleWrapperAdviseSink > m_pDataAdvises[DEFAULT_ARRAY_LEN];
    sal::systools::COMReference< OleWrapperAdviseSink > m_pViewAdvise;

    class InternalCacheWrapper final: public IOleCache2
    {
        InprocEmbedDocument_Impl& m_rOwnDocument;

        public:
        explicit InternalCacheWrapper( InprocEmbedDocument_Impl& rOwnDocument )
        : m_rOwnDocument( rOwnDocument )
        {}

        virtual ~InternalCacheWrapper() {}

        /* IUnknown methods */
        STDMETHOD(QueryInterface)(REFIID riid, void ** ppvObj) override;
        STDMETHOD_(ULONG, AddRef)() override;
        STDMETHOD_(ULONG, Release)() override;

        /* IOleCache2 methods */
        STDMETHOD(Cache)( FORMATETC *pformatetc, DWORD advf, DWORD *pdwConnection) override;
        STDMETHOD(Uncache)( DWORD dwConnection) override;
        STDMETHOD(EnumCache)( IEnumSTATDATA **ppenumSTATDATA) override;
        STDMETHOD(InitCache)( IDataObject *pDataObject) override;
        STDMETHOD(SetData)( FORMATETC *pformatetc, STGMEDIUM *pmedium, BOOL fRelease) override;
        STDMETHOD(UpdateCache)( LPDATAOBJECT pDataObject, DWORD grfUpdf, LPVOID pReserved) override;
        STDMETHOD(DiscardCache)( DWORD dwDiscardOptions) override;
    } m_aInternalCache;

    void Clean();


public:

    explicit InprocEmbedDocument_Impl( const GUID& guid )
    : m_refCount( 0 )
    , m_bDeleted( FALSE )
    , m_guid( guid )
    , m_nInitMode( NOINIT )
    , m_nFileOpenMode( 0 )
    , m_pFileName( nullptr )
    , m_nCallsOnStack( 0 )
    , m_aInternalCache( *this )
    {}

    virtual ~InprocEmbedDocument_Impl()
    {}

    BOOL CheckDefHandler();
    sal::systools::COMReference< IUnknown >& GetDefHandler() { return m_pDefHandler; }

    /* IUnknown methods */
    STDMETHOD(QueryInterface)(REFIID riid, void ** ppvObj) override;
    STDMETHOD_(ULONG, AddRef)() override;
    STDMETHOD_(ULONG, Release)() override;

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

    /* IPersistMethod */
    STDMETHOD(GetClassID)(CLSID *pClassID) override;

    /* IPersistStorage methods */
    STDMETHOD(IsDirty) () override;
    STDMETHOD(InitNew) ( IStorage *pStg ) override;
    STDMETHOD(Load) ( IStorage* pStr ) override;
    STDMETHOD(Save) ( IStorage *pStgSave, BOOL fSameAsLoad ) override;
    STDMETHOD(SaveCompleted) ( IStorage *pStgNew ) override;
    STDMETHOD(HandsOffStorage) (void) override;

    /* IPersistFile methods */
    STDMETHOD(Load) ( LPCOLESTR pszFileName, DWORD dwMode ) override;
    STDMETHOD(Save) ( LPCOLESTR pszFileName, BOOL fRemember ) override;
    STDMETHOD(SaveCompleted) ( LPCOLESTR pszFileName ) override;
    STDMETHOD(GetCurFile) ( LPOLESTR *ppszFileName ) override;

    /* IRunnableObject methods */
    STDMETHOD(GetRunningClass) ( LPCLSID lpClsid) override;
    STDMETHOD(Run) ( LPBINDCTX pbc) override;
    virtual BOOL STDMETHODCALLTYPE IsRunning( void) override;
    STDMETHOD(LockRunning) ( BOOL fLock, BOOL fLastUnlockCloses ) override;
    STDMETHOD(SetContainedObject) ( BOOL fContained) override;

    /* IViewObject2 methods */
    STDMETHOD(Draw)( DWORD dwDrawAspect, LONG lindex, void *pvAspect, DVTARGETDEVICE *ptd, HDC hdcTargetDev, HDC hdcDraw, LPCRECTL lprcBounds, LPCRECTL lprcWBounds, BOOL ( STDMETHODCALLTYPE *pfnContinue )( ULONG_PTR dwContinue ), ULONG_PTR dwContinue) override;
    STDMETHOD(GetColorSet)( DWORD dwDrawAspect, LONG lindex, void *pvAspect, DVTARGETDEVICE *ptd, HDC hicTargetDev, LOGPALETTE **ppColorSet) override;
    STDMETHOD(Freeze)( DWORD dwDrawAspect, LONG lindex, void *pvAspect, DWORD *pdwFreeze) override;
    STDMETHOD(Unfreeze)( DWORD dwFreeze) override;
    STDMETHOD(SetAdvise)( DWORD aspects, DWORD advf, IAdviseSink *pAdvSink) override;
    STDMETHOD(GetAdvise)( DWORD *pAspects, DWORD *pAdvf, IAdviseSink **ppAdvSink) override;
    STDMETHOD(GetExtent)( DWORD dwDrawAspect, LONG lindex, DVTARGETDEVICE *ptd, LPSIZEL lpsizel) override;

    /* IOleWindow methods */
    STDMETHOD(GetWindow)( HWND *phwnd) override;
    STDMETHOD(ContextSensitiveHelp)( BOOL fEnterMode) override;

    /* IOleInPlaceObject methods */
    STDMETHOD(InPlaceDeactivate)( void) override;
    STDMETHOD(UIDeactivate)( void) override;
    STDMETHOD(SetObjectRects)( LPCRECT lprcPosRect, LPCRECT lprcClipRect) override;
    STDMETHOD(ReactivateAndUndo)( void) override;

    /*IDispatch methods*/
    STDMETHOD(GetTypeInfoCount)( UINT *pctinfo) override;
    STDMETHOD(GetTypeInfo)( UINT iTInfo, LCID lcid, ITypeInfo **ppTInfo) override;
    STDMETHOD(GetIDsOfNames)( REFIID riid, LPOLESTR *rgszNames, UINT cNames, LCID lcid, DISPID *rgDispId) override;
    STDMETHOD(Invoke)( DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS *pDispParams, VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr) override;

};

} // namespace inprocserv

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
