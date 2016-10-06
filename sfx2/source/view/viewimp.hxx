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

#ifndef INCLUDED_SFX2_SOURCE_VIEW_VIEWIMP_HXX
#define INCLUDED_SFX2_SOURCE_VIEW_VIEWIMP_HXX

#include <basic/sbxobj.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/viewfrm.hxx>
#include <osl/mutex.hxx>
#include <comphelper/interfacecontainer2.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <svtools/acceleratorexecute.hxx>
#include <rtl/ref.hxx>
#include <vcl/print.hxx>
#include <queue>

class SfxBaseController;

typedef std::vector<SfxShell*> SfxShellArr_Impl;

class SfxClipboardChangeListener;
class SfxInPlaceClientList;

struct SfxViewShell_Impl
{
    ::osl::Mutex                aMutex;
    ::comphelper::OInterfaceContainerHelper2 aInterceptorContainer;
    bool                        m_bControllerSet;
    SfxShellArr_Impl            aArr;
    Size                        aMargin;
    bool                        m_bCanPrint;
    bool                        m_bHasPrintOptions;
    bool                        m_bIsShowView;
    bool                        m_bGotOwnership;
    bool                        m_bGotFrameOwnership;
    sal_uInt16                  m_nFamily;
    ::rtl::Reference<SfxBaseController> m_pController;
    std::unique_ptr< ::svt::AcceleratorExecute > m_xAccExec;
    ::rtl::Reference< SfxClipboardChangeListener > xClipboardListener;
    std::shared_ptr< vcl::PrinterController > m_xPrinterController;

    mutable SfxInPlaceClientList* mpIPClientList;

    LibreOfficeKitCallback m_pLibreOfficeKitViewCallback;
    void* m_pLibreOfficeKitViewData;
    /// Set if we are in the middle of a tiled search.
    bool m_bTiledSearching;
    static sal_uInt32 m_nLastViewShellId;
    const sal_uInt32 m_nViewShellId;

    explicit SfxViewShell_Impl(SfxViewShellFlags const nFlags);
    ~SfxViewShell_Impl();

    SfxInPlaceClientList* GetIPClientList_Impl( bool bCreate = true ) const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
