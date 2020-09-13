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

#include <com/sun/star/datatransfer/XTransferable.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboardOwner.hpp>

#include "DataFmtTransl.hxx"

#include "FetcList.hxx"

#if !defined WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <ole2.h>
#include <objidl.h>

/*--------------------------------------------------------------------------
    - the function principle of the windows clipboard:
      a data provider offers all formats he can deliver on the clipboard
      a clipboard client ask for the available formats on the clipboard
      and decides if there is a format he can use
      if there is one, he requests the data in this format

    - This class inherits from IDataObject and so can be placed on the
      OleClipboard. The class wraps a transferable object which is the
      original DataSource
    - DataFlavors offered by this transferable will be translated into
      appropriate clipboard formats
    - if the transferable contains text data always text and unicodetext
      will be offered or vice versa
    - text data will be automatically converted between text and unicode text
    - although the transferable may support text in different charsets
      (codepages) only text in one codepage can be offered by the clipboard

----------------------------------------------------------------------------*/

class CStgTransferHelper;

class CXTDataObject : public IDataObject
{
public:
    CXTDataObject( const css::uno::Reference< css::uno::XComponentContext >& rxContext,
                   const css::uno::Reference< css::datatransfer::XTransferable >& aXTransferable );
    virtual ~CXTDataObject();

    // ole interface implementation

    //IUnknown interface methods
    STDMETHODIMP           QueryInterface(REFIID iid, void** ppvObject) override;
    STDMETHODIMP_( ULONG ) AddRef( ) override;
    STDMETHODIMP_( ULONG ) Release( ) override;

    // IDataObject interface methods
    STDMETHODIMP GetData( FORMATETC * pFormatetc, STGMEDIUM * pmedium ) override;
    STDMETHODIMP GetDataHere( FORMATETC * pFormatetc, STGMEDIUM * pmedium ) override;
    STDMETHODIMP QueryGetData( FORMATETC * pFormatetc ) override;
    STDMETHODIMP GetCanonicalFormatEtc( FORMATETC * pFormatectIn, FORMATETC * pFormatetcOut ) override;
    STDMETHODIMP SetData( FORMATETC * pFormatetc, STGMEDIUM * pmedium, BOOL fRelease ) override;
    STDMETHODIMP EnumFormatEtc( DWORD dwDirection, IEnumFORMATETC** ppenumFormatetc ) override;
    STDMETHODIMP DAdvise( FORMATETC * pFormatetc, DWORD advf, IAdviseSink * pAdvSink, DWORD* pdwConnection ) override;
    STDMETHODIMP DUnadvise( DWORD dwConnection ) override;
    STDMETHODIMP EnumDAdvise( IEnumSTATDATA** ppenumAdvise ) override;

    operator IDataObject*( );

private:
    css::datatransfer::DataFlavor formatEtcToDataFlavor( const FORMATETC& aFormatEtc ) const;

    void renderLocaleAndSetupStgMedium( FORMATETC const & fetc, STGMEDIUM& stgmedium );
    void renderUnicodeAndSetupStgMedium( FORMATETC const & fetc, STGMEDIUM& stgmedium );
    void renderAnyDataAndSetupStgMedium( FORMATETC& fetc, STGMEDIUM& stgmedium );

    HRESULT renderSynthesizedFormatAndSetupStgMedium( FORMATETC& fetc, STGMEDIUM& stgmedium );
    void    renderSynthesizedUnicodeAndSetupStgMedium( FORMATETC const & fetc, STGMEDIUM& stgmedium );
    void    renderSynthesizedTextAndSetupStgMedium( FORMATETC& fetc, STGMEDIUM& stgmedium );
    void    renderSynthesizedHtmlAndSetupStgMedium( FORMATETC& fetc, STGMEDIUM& stgmedium );

    inline void InitializeFormatEtcContainer( );

private:
    LONG m_nRefCnt;
    css::uno::Reference< css::datatransfer::XTransferable >      m_XTransferable;
    css::uno::Reference< css::uno::XComponentContext>            m_XComponentContext;
    CFormatEtcContainer                                          m_FormatEtcContainer;
    bool                                                         m_bFormatEtcContainerInitialized;
    CDataFormatTranslator                                        m_DataFormatTranslator;
    CFormatRegistrar                                             m_FormatRegistrar;
};

class CEnumFormatEtc : public IEnumFORMATETC
{
public:
    CEnumFormatEtc( LPUNKNOWN lpUnkOuter, const CFormatEtcContainer& aFormatEtcContainer );
    virtual ~CEnumFormatEtc() {}

    // IUnknown
    STDMETHODIMP           QueryInterface( REFIID iid, void** ppvObject ) override;
    STDMETHODIMP_( ULONG ) AddRef( ) override;
    STDMETHODIMP_( ULONG ) Release( ) override;

    //IEnumFORMATETC
    STDMETHODIMP Next( ULONG nRequested, FORMATETC * lpDest, ULONG* lpFetched ) override;
    STDMETHODIMP Skip( ULONG celt ) override;
    STDMETHODIMP Reset( ) override;
    STDMETHODIMP Clone( IEnumFORMATETC** ppenum ) override;

private:
    LONG                m_nRefCnt;
    LPUNKNOWN           m_lpUnkOuter;
    CFormatEtcContainer m_FormatEtcContainer;
};

typedef CEnumFormatEtc *PCEnumFormatEtc;

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
