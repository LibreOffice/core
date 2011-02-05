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

#ifndef _EMBEDDOC_HXX_
#define _EMBEDDOC_HXX_
#if defined(_MSC_VER) && (_MSC_VER >= 1300)
#undef _DEBUG
#endif
#if defined(_MSC_VER) && (_MSC_VER > 1310)
#pragma warning(disable : 4917 4555)
#endif

#include "common.h"
#include <oleidl.h>
#include <objidl.h>

#include <boost/unordered_map.hpp>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/SEQUENCE.h>

#include "embeddocaccess.hxx"
#include "docholder.hxx"

typedef ::boost::unordered_map< DWORD, IAdviseSink* > AdviseSinkHashMap;
typedef ::boost::unordered_map< DWORD, IAdviseSink* >::iterator AdviseSinkHashMapIterator;

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
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >
                fillArgsForLoading_Impl( ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > xStream,
                                         DWORD nStreamMode,
                                         LPCOLESTR pFilePath = NULL );
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >
                fillArgsForStoring_Impl( ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream > xStream );

    HRESULT SaveTo_Impl( IStorage* pStg );

    sal_uInt64 getMetaFileHandle_Impl( sal_Bool isEnhMeta );

public:
    EmbedDocument_Impl( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& smgr,
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

    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > m_xFactory;

    DocumentHolder*                     m_pDocHolder;
    ::rtl::OUString                     m_aFileName;

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

#endif //_EMBEDDOC_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
