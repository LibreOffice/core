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

#ifndef INCLUDED_EMBEDSERV_SOURCE_INPROCSERV_INPROCEMBOBJ_H
#define INCLUDED_EMBEDSERV_SOURCE_INPROCSERV_INPROCEMBOBJ_H

#if defined _MSC_VER
#pragma warning(disable : 4668)
#endif

#include <windows.h>
#include <oleidl.h>

#include "smartpointer.hxx"
#include "advisesink.hxx"

#define DEFAULT_ARRAY_LEN 256

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

    ComSmart< IUnknown > m_pDefHandler;
    InitModes m_nInitMode;

    DWORD m_nFileOpenMode;
    wchar_t* m_pFileName;

    ComSmart< IStorage > m_pStorage;

    ComSmart< IOleClientSite > m_pClientSite;
    ComSmart< IOleContainer >  m_pOleContainer;

    ULONG m_nCallsOnStack;

    // the listeners have wrappers that are directly connected to the object and call the listeners,
    // the wrappers will be reconnected correctly to the new default inprocess holder object
    ComSmart< OleWrapperAdviseSink > m_pOleAdvises[DEFAULT_ARRAY_LEN];
    ComSmart< OleWrapperAdviseSink > m_pDataAdvises[DEFAULT_ARRAY_LEN];
    ComSmart< OleWrapperAdviseSink > m_pViewAdvise;

    class InternalCacheWrapper : public IOleCache2
    {
        InprocEmbedDocument_Impl& m_rOwnDocument;

        public:
        InternalCacheWrapper( InprocEmbedDocument_Impl& rOwnDocument )
        : m_rOwnDocument( rOwnDocument )
        {}

        /* IUnknown methods */
        STDMETHOD(QueryInterface)(REFIID riid, LPVOID FAR * ppvObj);
        STDMETHOD_(ULONG, AddRef)();
        STDMETHOD_(ULONG, Release)();

        /* IOleCache2 methods */
        STDMETHOD(Cache)( FORMATETC *pformatetc, DWORD advf, DWORD *pdwConnection);
        STDMETHOD(Uncache)( DWORD dwConnection);
        STDMETHOD(EnumCache)( IEnumSTATDATA **ppenumSTATDATA);
        STDMETHOD(InitCache)( IDataObject *pDataObject);
        STDMETHOD(SetData)( FORMATETC *pformatetc, STGMEDIUM *pmedium, BOOL fRelease);
        STDMETHOD(UpdateCache)( LPDATAOBJECT pDataObject, DWORD grfUpdf, LPVOID pReserved);
        STDMETHOD(DiscardCache)( DWORD dwDiscardOptions);
    } m_aInternalCache;


    DWORD InsertAdviseLinkToList( const ComSmart<OleWrapperAdviseSink>& pOwnAdvise, ComSmart<  OleWrapperAdviseSink > pAdvises[] );
    void Clean();


public:

    InprocEmbedDocument_Impl( const GUID& guid )
    : m_refCount( 0 )
    , m_bDeleted( FALSE )
    , m_guid( guid )
    , m_nInitMode( NOINIT )
    , m_nFileOpenMode( 0 )
    , m_pFileName( NULL )
    , m_nCallsOnStack( 0 )
    , m_aInternalCache( *this )
    {}

    virtual ~InprocEmbedDocument_Impl()
    {}

    HRESULT Init();
    void SetName( LPCOLESTR pszNameFromOutside, wchar_t*& pOwnName );

    BOOL CheckDefHandler();
    ComSmart< IUnknown >& GetDefHandler() { return m_pDefHandler; }

    /* IUnknown methods */
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID FAR * ppvObj);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

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

    /* IPersistMethod */
    STDMETHOD(GetClassID)(CLSID *pClassID);

    /* IPersistStorage methods */
    STDMETHOD(IsDirty) ();
    STDMETHOD(InitNew) ( IStorage *pStg );
    STDMETHOD(Load) ( IStorage* pStr );
    STDMETHOD(Save) ( IStorage *pStgSave, BOOL fSameAsLoad );
    STDMETHOD(SaveCompleted) ( IStorage *pStgNew );
    STDMETHOD(HandsOffStorage) (void);

    /* IPersistFile methods */
    STDMETHOD(Load) ( LPCOLESTR pszFileName, DWORD dwMode );
    STDMETHOD(Save) ( LPCOLESTR pszFileName, BOOL fRemember );
    STDMETHOD(SaveCompleted) ( LPCOLESTR pszFileName );
    STDMETHOD(GetCurFile) ( LPOLESTR *ppszFileName );

    /* IRunnableObject methods */
    STDMETHOD(GetRunningClass) ( LPCLSID lpClsid);
    STDMETHOD(Run) ( LPBINDCTX pbc);
    virtual BOOL STDMETHODCALLTYPE IsRunning( void);
    STDMETHOD(LockRunning) ( BOOL fLock, BOOL fLastUnlockCloses );
    STDMETHOD(SetContainedObject) ( BOOL fContained);

    /* IViewObject2 methods */
    STDMETHOD(Draw)( DWORD dwDrawAspect, LONG lindex, void *pvAspect, DVTARGETDEVICE *ptd, HDC hdcTargetDev, HDC hdcDraw, LPCRECTL lprcBounds, LPCRECTL lprcWBounds, BOOL ( STDMETHODCALLTYPE *pfnContinue )( ULONG_PTR dwContinue ), ULONG_PTR dwContinue);
    STDMETHOD(GetColorSet)( DWORD dwDrawAspect, LONG lindex, void *pvAspect, DVTARGETDEVICE *ptd, HDC hicTargetDev, LOGPALETTE **ppColorSet);
    STDMETHOD(Freeze)( DWORD dwDrawAspect, LONG lindex, void *pvAspect, DWORD *pdwFreeze);
    STDMETHOD(Unfreeze)( DWORD dwFreeze);
    STDMETHOD(SetAdvise)( DWORD aspects, DWORD advf, IAdviseSink *pAdvSink);
    STDMETHOD(GetAdvise)( DWORD *pAspects, DWORD *pAdvf, IAdviseSink **ppAdvSink);
    STDMETHOD(GetExtent)( DWORD dwDrawAspect, LONG lindex, DVTARGETDEVICE *ptd, LPSIZEL lpsizel);

    /* IOleWindow methods */
    STDMETHOD(GetWindow)( HWND *phwnd);
    STDMETHOD(ContextSensitiveHelp)( BOOL fEnterMode);

    /* IOleInPlaceObject methods */
    STDMETHOD(InPlaceDeactivate)( void);
    STDMETHOD(UIDeactivate)( void);
    STDMETHOD(SetObjectRects)( LPCRECT lprcPosRect, LPCRECT lprcClipRect);
    STDMETHOD(ReactivateAndUndo)( void);

    /*IDispatch methods*/
    STDMETHOD(GetTypeInfoCount)( UINT *pctinfo);
    STDMETHOD(GetTypeInfo)( UINT iTInfo, LCID lcid, ITypeInfo **ppTInfo);
    STDMETHOD(GetIDsOfNames)( REFIID riid, LPOLESTR *rgszNames, UINT cNames, LCID lcid, DISPID *rgDispId);
    STDMETHOD(Invoke)( DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS *pDispParams, VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr);

};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
