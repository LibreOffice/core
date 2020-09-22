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

#ifndef INCLUDED_DTRANS_SOURCE_WIN32_DTOBJ_XNOTIFYINGDATAOBJECT_HXX
#define INCLUDED_DTRANS_SOURCE_WIN32_DTOBJ_XNOTIFYINGDATAOBJECT_HXX

#include <com/sun/star/datatransfer/XTransferable.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboardOwner.hpp>

#if defined _MSC_VER
#pragma warning(push,1)
#endif
#include <objidl.h>
#include <windows.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif

#include <systools/win32/comtools.hxx>

/*--------------------------------------------------------------------------
    To implement the lostOwnership mechanism cleanly we need this wrapper
    object
----------------------------------------------------------------------------*/

// forward
class CWinClipbImpl;

class CXNotifyingDataObject : public IDataObject
{
public:
    CXNotifyingDataObject(
        const IDataObjectPtr& aIDataObject,
        const css::uno::Reference< css::datatransfer::XTransferable >& aXTransferable,
        const css::uno::Reference< css::datatransfer::clipboard::XClipboardOwner >& aXClipOwner,
        CWinClipbImpl* theWinClipImpl );

    virtual ~CXNotifyingDataObject() {}

    // ole interface implementation

    //IUnknown interface methods
    STDMETHODIMP           QueryInterface(REFIID iid, LPVOID* ppvObject) override;
    STDMETHODIMP_( ULONG ) AddRef( ) override;
    STDMETHODIMP_( ULONG ) Release( ) override;

    // IDataObject interface methods
    STDMETHODIMP GetData( LPFORMATETC pFormatetc, LPSTGMEDIUM pmedium ) override;
    STDMETHODIMP GetDataHere( LPFORMATETC pFormatetc, LPSTGMEDIUM pmedium ) override;
    STDMETHODIMP QueryGetData( LPFORMATETC pFormatetc ) override;
    STDMETHODIMP GetCanonicalFormatEtc( LPFORMATETC pFormatectIn, LPFORMATETC pFormatetcOut ) override;
    STDMETHODIMP SetData( LPFORMATETC pFormatetc, LPSTGMEDIUM pmedium, BOOL fRelease ) override;
    STDMETHODIMP EnumFormatEtc( DWORD dwDirection, IEnumFORMATETC** ppenumFormatetc ) override;
    STDMETHODIMP DAdvise( LPFORMATETC pFormatetc, DWORD advf, LPADVISESINK pAdvSink, DWORD* pdwConnection ) override;
    STDMETHODIMP DUnadvise( DWORD dwConnection ) override;
    STDMETHODIMP EnumDAdvise( LPENUMSTATDATA* ppenumAdvise ) override;

    operator IDataObject*( );

private:
    void SAL_CALL lostOwnership( );

private:
    sal_Int32                                                                     m_nRefCnt;
    IDataObjectPtr                                                                m_aIDataObject;
    const css::uno::Reference< css::datatransfer::XTransferable >                 m_XTransferable;
    const css::uno::Reference< css::datatransfer::clipboard::XClipboardOwner >    m_XClipboardOwner;
    CWinClipbImpl*                                                                m_pWinClipImpl;

    friend class CWinClipbImpl;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
