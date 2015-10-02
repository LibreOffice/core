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
#include "generic/geninst.h"

#include <prex.h>
#include <X11/Xlib.h>
#include <postx.h>

namespace com { namespace sun { namespace star { namespace datatransfer {
    namespace clipboard { class XClipboard; }
} } } }

class SalXLib;
class X11SalGraphics;

class VCLPLUG_GEN_PUBLIC X11SalInstance : public SalGenericInstance
{
private:
    std::unordered_map< Atom, com::sun::star::uno::Reference< com::sun::star::datatransfer::clipboard::XClipboard > > m_aInstances;

protected:
    SalXLib *mpXLib;

public:
    X11SalInstance( SalYieldMutex* pMutex ) : SalGenericInstance( pMutex ), mpXLib(NULL) {}
    virtual ~X11SalInstance();

    virtual SalFrame*           CreateChildFrame( SystemParentData* pParent, SalFrameStyleFlags nStyle ) SAL_OVERRIDE;
    virtual SalFrame*           CreateFrame( SalFrame* pParent, SalFrameStyleFlags nStyle ) SAL_OVERRIDE;
    virtual void                DestroyFrame( SalFrame* pFrame ) SAL_OVERRIDE;

    virtual SalObject*          CreateObject( SalFrame* pParent, SystemWindowData* pWindowData, bool bShow = true ) SAL_OVERRIDE;
    virtual void                DestroyObject( SalObject* pObject ) SAL_OVERRIDE;

    /// Gtk vclplug needs to pass GtkSalGraphics to X11SalVirtualDevice, so create it, and pass as pNewGraphics.
    virtual SalVirtualDevice* CreateX11VirtualDevice(SalGraphics* pGraphics, long &nDX, long &nDY,
            sal_uInt16 nBitCount, const SystemGraphicsData* pData, X11SalGraphics* pNewGraphics);

    virtual SalVirtualDevice*   CreateVirtualDevice( SalGraphics* pGraphics,
                                                     long &nDX, long &nDY,
                                                     sal_uInt16 nBitCount, const SystemGraphicsData *pData = NULL ) SAL_OVERRIDE;
    virtual void                PostPrintersChanged() SAL_OVERRIDE;
    virtual GenPspGraphics     *CreatePrintGraphics() SAL_OVERRIDE;

    virtual SalTimer*           CreateSalTimer() SAL_OVERRIDE;
    virtual SalI18NImeStatus*   CreateI18NImeStatus() SAL_OVERRIDE;
    virtual SalSystem*          CreateSalSystem() SAL_OVERRIDE;
    virtual SalBitmap*          CreateSalBitmap() SAL_OVERRIDE;
    virtual SalSession*         CreateSalSession() SAL_OVERRIDE;

    virtual void                DoYield(bool bWait, bool bHandleAllCurrentEvents, sal_uLong nReleased) SAL_OVERRIDE;
    virtual bool                AnyInput( VclInputFlags nType ) SAL_OVERRIDE;

    virtual void*               GetConnectionIdentifier( ConnectionIdentifierType& rReturnedType, int& rReturnedBytes ) SAL_OVERRIDE;
    void                        SetLib( SalXLib *pXLib ) { mpXLib = pXLib; }

    // dtrans implementation
    virtual com::sun::star::uno::Reference< com::sun::star::uno::XInterface >
        CreateClipboard( const com::sun::star::uno::Sequence< com::sun::star::uno::Any >& i_rArguments ) SAL_OVERRIDE;
    virtual com::sun::star::uno::Reference< com::sun::star::uno::XInterface > CreateDragSource() SAL_OVERRIDE;
    virtual com::sun::star::uno::Reference< com::sun::star::uno::XInterface > CreateDropTarget() SAL_OVERRIDE;
    virtual void            AddToRecentDocumentList(const OUString& rFileUrl, const OUString& rMimeType, const OUString& rDocumentService) SAL_OVERRIDE;
};

#endif // INCLUDED_VCL_INC_UNX_SALINST_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
