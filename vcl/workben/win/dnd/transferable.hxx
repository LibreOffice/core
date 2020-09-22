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

#include <cppuhelper/servicefactory.hxx>
#include <com/sun/star/datatransfer/XTransferable.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboardOwner.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboardNotifier.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboardEx.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <cppuhelper/implbase.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <osl/diagnose.h>

#include <stdio.h>
#if defined _MSC_VER
#pragma warning(push,1)
#endif
#include <windows.h>
#include <objbase.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif

#include <memory>

#include <process.h>

#include "../../source/win32/ImplHelper.hxx"

// my defines

#define TEST_CLIPBOARD
#define RDB_SYSPATH  "d:\\projects\\src616\\dtrans\\wntmsci7\\bin\\applicat.rdb"
#define WINCLIPBOARD_SERVICE_NAME L"com.sun.star.datatransfer.clipboard.SystemClipboard"
#define  WRITE_CB
#define EVT_MANUAL_RESET     TRUE
#define EVT_INIT_NONSIGNALED FALSE
#define EVT_NONAME           ""

//  namespaces

using namespace ::std;
using namespace ::cppu;
using namespace ::com::sun::star::datatransfer;
using namespace ::com::sun::star::datatransfer::clipboard;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;

class CTransferable : public WeakImplHelper< XClipboardOwner, XTransferable >
{
public:
    CTransferable( ){};
    explicit CTransferable( wchar_t* dataString);

    // XTransferable

    virtual Any SAL_CALL getTransferData( const DataFlavor& aFlavor ) throw(UnsupportedFlavorException, IOException, RuntimeException);
    virtual Sequence< DataFlavor > SAL_CALL getTransferDataFlavors(  ) throw(RuntimeException);
    virtual sal_Bool SAL_CALL isDataFlavorSupported( const DataFlavor& aFlavor ) throw(RuntimeException);

    // XClipboardOwner

    virtual void SAL_CALL lostOwnership( const Reference< XClipboard >& xClipboard, const Reference< XTransferable >& xTrans ) throw(RuntimeException);

private:
    Sequence< DataFlavor > m_seqDFlv;
    OUString               m_Data;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
