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

#include <osl/thread.hxx>
#include <vclpluginapi.h>
#include <salinst.hxx>
#include "unx/geninst.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>

namespace com { namespace sun { namespace star { namespace datatransfer {
    namespace clipboard { class XClipboard; }
} } } }

class SalXLib;
class X11SalGraphics;
class SalX11Display;

class VCLPLUG_GEN_PUBLIC X11SalInstance : public SalGenericInstance
{
private:
    std::unordered_map< Atom, css::uno::Reference< css::datatransfer::clipboard::XClipboard > > m_aInstances;

protected:
    SalXLib *mpXLib;

    virtual SalX11Display* CreateDisplay() const;

public:
    explicit X11SalInstance(SalYieldMutex* pMutex);
    virtual ~X11SalInstance() override;

    virtual SalFrame*           CreateChildFrame( SystemParentData* pParent, SalFrameStyleFlags nStyle ) override;
    virtual SalFrame*           CreateFrame( SalFrame* pParent, SalFrameStyleFlags nStyle ) override;
    virtual void                DestroyFrame( SalFrame* pFrame ) override;

    virtual SalObject*          CreateObject( SalFrame* pParent, SystemWindowData* pWindowData, bool bShow ) override;
    virtual void                DestroyObject( SalObject* pObject ) override;

    /// Gtk vclplug needs to pass GtkSalGraphics to X11SalVirtualDevice, so create it, and pass as pNewGraphics.
    static SalVirtualDevice* CreateX11VirtualDevice(SalGraphics* pGraphics, long &nDX, long &nDY,
            DeviceFormat eFormat, const SystemGraphicsData* pData, X11SalGraphics* pNewGraphics);

    virtual SalVirtualDevice*   CreateVirtualDevice( SalGraphics* pGraphics,
                                                     long &nDX, long &nDY,
                                                     DeviceFormat eFormat, const SystemGraphicsData *pData = nullptr ) override;
    virtual void                PostPrintersChanged() override;
    virtual GenPspGraphics     *CreatePrintGraphics() override;

    virtual SalTimer*           CreateSalTimer() override;
    virtual SalI18NImeStatus*   CreateI18NImeStatus() override;
    virtual SalSystem*          CreateSalSystem() override;
    virtual SalBitmap*          CreateSalBitmap() override;
    virtual SalSession*         CreateSalSession() override;
    virtual OpenGLContext*      CreateOpenGLContext() override;

    virtual bool                DoYield(bool bWait, bool bHandleAllCurrentEvents, sal_uLong nReleased) override;
    virtual bool                AnyInput( VclInputFlags nType ) override;

    virtual void*               GetConnectionIdentifier( ConnectionIdentifierType& rReturnedType, int& rReturnedBytes ) override;
    void                        SetLib( SalXLib *pXLib ) { mpXLib = pXLib; }

    virtual void                AfterAppInit() override;

    // dtrans implementation
    virtual css::uno::Reference< css::uno::XInterface >
        CreateClipboard( const css::uno::Sequence< css::uno::Any >& i_rArguments ) override;
    virtual css::uno::Reference< css::uno::XInterface > CreateDragSource() override;
    virtual css::uno::Reference< css::uno::XInterface > CreateDropTarget() override;
    virtual void            AddToRecentDocumentList(const OUString& rFileUrl, const OUString& rMimeType, const OUString& rDocumentService) override;
};

#endif // INCLUDED_VCL_INC_UNX_SALINST_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
