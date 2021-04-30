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

#ifndef INCLUDED_VCL_INC_UNX_SALINST_H
#define INCLUDED_VCL_INC_UNX_SALINST_H

#include <vclpluginapi.h>
#include <salinst.hxx>
#include <unx/geninst.h>

#include <X11/X.h>

namespace com::sun::star::datatransfer::clipboard { class XClipboard; }
class SalXLib;
class X11SalGraphics;
class SalX11Display;

class X11SalInstance final : public SalGenericInstance
{
private:
    std::unordered_map< Atom, css::uno::Reference< css::datatransfer::clipboard::XClipboard > > m_aInstances;

    SalXLib *mpXLib;

    virtual SalX11Display* CreateDisplay() const;

public:
    explicit X11SalInstance(std::unique_ptr<SalYieldMutex> pMutex);
    virtual ~X11SalInstance() override;

    virtual SalFrame*           CreateChildFrame( SystemParentData* pParent, SalFrameStyleFlags nStyle ) override;
    virtual SalFrame*           CreateFrame( SalFrame* pParent, SalFrameStyleFlags nStyle ) override;
    virtual void                DestroyFrame( SalFrame* pFrame ) override;

    virtual SalObject*          CreateObject( SalFrame* pParent, SystemWindowData* pWindowData, bool bShow ) override;
    virtual void                DestroyObject( SalObject* pObject ) override;

    /// Gtk vclplug needs to pass GtkSalGraphics to X11SalVirtualDevice, so create it, and pass as pNewGraphics.
    static std::unique_ptr<SalVirtualDevice> CreateX11VirtualDevice(const SalGraphics& rGraphics, tools::Long &nDX, tools::Long &nDY,
            DeviceFormat eFormat, const SystemGraphicsData* pData, std::unique_ptr<X11SalGraphics> pNewGraphics);

    virtual std::unique_ptr<SalVirtualDevice>
                                CreateVirtualDevice( SalGraphics& rGraphics,
                                                     tools::Long &nDX, tools::Long &nDY,
                                                     DeviceFormat eFormat, const SystemGraphicsData *pData = nullptr ) override;
    virtual void                PostPrintersChanged() override;
    virtual std::unique_ptr<GenPspGraphics> CreatePrintGraphics() override;

    virtual SalTimer*           CreateSalTimer() override;
    virtual SalSystem*          CreateSalSystem() override;
    virtual std::shared_ptr<SalBitmap>  CreateSalBitmap() override;
    virtual std::unique_ptr<SalSession> CreateSalSession() override;
    virtual OpenGLContext*      CreateOpenGLContext() override;

    virtual bool                DoYield(bool bWait, bool bHandleAllCurrentEvents) override;
    virtual bool                AnyInput( VclInputFlags nType ) override;
    virtual bool                IsMainThread() const override { return true; }

    virtual OUString            GetConnectionIdentifier() override;
    void                        SetLib( SalXLib *pXLib ) { mpXLib = pXLib; }

    virtual void                AfterAppInit() override;

    std::shared_ptr<vcl::BackendCapabilities> GetBackendCapabilities() override;

    // dtrans implementation
    virtual css::uno::Reference< css::uno::XInterface >
        CreateClipboard( const css::uno::Sequence< css::uno::Any >& i_rArguments ) override;
    virtual css::uno::Reference< css::uno::XInterface > CreateDragSource() override;
    virtual css::uno::Reference< css::uno::XInterface > CreateDropTarget() override;
    virtual void            AddToRecentDocumentList(const OUString& rFileUrl, const OUString& rMimeType, const OUString& rDocumentService) override;
};

#endif // INCLUDED_VCL_INC_UNX_SALINST_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
