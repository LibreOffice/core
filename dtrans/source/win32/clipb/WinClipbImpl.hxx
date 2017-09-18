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

#ifndef INCLUDED_DTRANS_SOURCE_WIN32_CLIPB_WINCLIPBIMPL_HXX
#define INCLUDED_DTRANS_SOURCE_WIN32_CLIPB_WINCLIPBIMPL_HXX

#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <com/sun/star/datatransfer/XTransferable.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboardListener.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboardOwner.hpp>
#include "MtaOleClipb.hxx"

#if defined _MSC_VER
#pragma warning(push,1)
#endif
#if !defined WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif

class CWinClipboard;
class CXNotifyingDataObject;

// impl class to avoid deadlocks between XTDataObject
// and the clipboard implementation

class CWinClipbImpl
{
public:
    ~CWinClipbImpl( );

protected:
    CWinClipbImpl( const OUString& aClipboardName, CWinClipboard* theWinClipboard );

    /// @throws css::uno::RuntimeException
    css::uno::Reference< css::datatransfer::XTransferable > SAL_CALL getContents(  );

    /// @throws css::uno::RuntimeException
    void SAL_CALL setContents(
        const css::uno::Reference< css::datatransfer::XTransferable >& xTransferable,
        const css::uno::Reference< css::datatransfer::clipboard::XClipboardOwner >& xClipboardOwner );

    /// @throws css::uno::RuntimeException
    OUString SAL_CALL getName(  );

    // XClipboardEx

    /// @throws css::uno::RuntimeException
    static sal_Int8 SAL_CALL getRenderingCapabilities(  );

    // XFlushableClipboard

    /// @throws css::uno::RuntimeException
    void SAL_CALL flushClipboard( );

    // XComponent

    /// @throws css::uno::RuntimeException
    void SAL_CALL dispose( );

    // member functions

    void SAL_CALL registerClipboardViewer( );
    void SAL_CALL unregisterClipboardViewer( );

    static void WINAPI onClipboardContentChanged();

private:
    void SAL_CALL onReleaseDataObject( CXNotifyingDataObject* theCaller );

private:
    OUString                m_itsName;
    CMtaOleClipboard        m_MtaOleClipboard;
    CWinClipboard*          m_pWinClipboard;
    CXNotifyingDataObject*  m_pCurrentClipContent;
    osl::Mutex              m_ClipContentMutex;

    static osl::Mutex       s_aMutex;
    static CWinClipbImpl*   s_pCWinClipbImpl;

private:
    CWinClipbImpl( const CWinClipbImpl& );
    CWinClipbImpl& operator=( const CWinClipbImpl& );

    friend class CWinClipboard;
    friend class CXNotifyingDataObject;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
