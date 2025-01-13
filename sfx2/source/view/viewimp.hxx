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

#include <com/sun/star/ui/XContextMenuInterceptor.hpp>
#include <memory>
#include <sfx2/viewsh.hxx>
#include <mutex>
#include <comphelper/interfacecontainer4.hxx>
#include <svtools/acceleratorexecute.hxx>
#include <rtl/ref.hxx>
#include <vcl/print.hxx>
#include <svl/cryptosign.hxx>
#include <chrono>
#include <vector>

class SfxBaseController;
typedef std::vector<SfxShell*> SfxShellArr_Impl;
class SfxClipboardChangeListener;

struct SfxViewShell_Impl
{
    std::mutex aMutex;
    ::comphelper::OInterfaceContainerHelper4<css::ui::XContextMenuInterceptor>
        aInterceptorContainer;
    SfxShellArr_Impl aArr;
    Size aMargin;
    OUString m_sDefaultPrinterName;
    std::chrono::steady_clock::time_point m_nDefaultPrinterNameFetchTime;
    bool m_bHasPrintOptions;
    sal_uInt16 m_nFamily;
    ::rtl::Reference<SfxBaseController> m_pController;
    std::unique_ptr<::svt::AcceleratorExecute> m_xAccExec;
    ::rtl::Reference<SfxClipboardChangeListener> xClipboardListener;
    std::shared_ptr<vcl::PrinterController> m_xPrinterController;

    mutable std::vector<SfxInPlaceClient*> maIPClients;

    SfxLokCallbackInterface* m_pLibreOfficeKitViewCallback;
    /// Set if we are in the middle of a tiled search.
    bool m_bTiledSearching;
    static sal_uInt32 m_nLastViewShellId;
    const ViewShellId m_nViewShellId;
    const ViewShellDocId m_nDocId;
    svl::crypto::CertificateOrName m_aSigningCertificate;

    explicit SfxViewShell_Impl(SfxViewShellFlags const nFlags, ViewShellDocId nDocId);
    ~SfxViewShell_Impl();

    std::vector<SfxInPlaceClient*>& GetIPClients_Impl();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
