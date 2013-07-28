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

#ifndef _SV_SALINST_H
#define _SV_SALINST_H

#include <tools/solar.h>
#include <osl/thread.hxx>
#include <vclpluginapi.h>
#include <salinst.hxx>
#include <vcl/solarmutex.hxx>
#include "generic/geninst.h"

#include <prex.h>
#include <X11/Xlib.h>
#include <postx.h>

namespace com { namespace sun { namespace star { namespace datatransfer {
    namespace clipboard { class XClipboard; }
} } } }

class SalXLib;
class VCLPLUG_GEN_PUBLIC X11SalInstance : public SalGenericInstance
{
private:
    boost::unordered_map< Atom, com::sun::star::uno::Reference< com::sun::star::datatransfer::clipboard::XClipboard > > m_aInstances;

protected:
    SalXLib *mpXLib;
public:
    X11SalInstance( SalYieldMutex* pMutex ) : SalGenericInstance( pMutex ), mpXLib(NULL) {}
    virtual ~X11SalInstance();

    virtual SalFrame*           CreateChildFrame( SystemParentData* pParent, sal_uIntPtr nStyle );
    virtual SalFrame*           CreateFrame( SalFrame* pParent, sal_uIntPtr nStyle );
    virtual void                DestroyFrame( SalFrame* pFrame );

    virtual SalObject*          CreateObject( SalFrame* pParent, SystemWindowData* pWindowData, sal_Bool bShow = sal_True );
    virtual void                DestroyObject( SalObject* pObject );

    virtual SalVirtualDevice*   CreateVirtualDevice( SalGraphics* pGraphics,
                                                     long nDX, long nDY,
                                                     sal_uInt16 nBitCount, const SystemGraphicsData *pData = NULL );
    virtual void                DestroyVirtualDevice( SalVirtualDevice* pDevice );
    virtual void                PostPrintersChanged();
    virtual GenPspGraphics     *CreatePrintGraphics();

    virtual SalTimer*           CreateSalTimer();
    virtual SalI18NImeStatus*   CreateI18NImeStatus();
    virtual SalSystem*          CreateSalSystem();
    virtual SalBitmap*          CreateSalBitmap();
    virtual SalSession*         CreateSalSession();

    virtual void                Yield( bool bWait, bool bHandleAllCurrentEvents );
    virtual bool                AnyInput( sal_uInt16 nType );

    virtual void*               GetConnectionIdentifier( ConnectionIdentifierType& rReturnedType, int& rReturnedBytes );
    void                        FillFontPathList( std::list< OString >& o_rFontPaths );
    void                        SetLib( SalXLib *pXLib ) { mpXLib = pXLib; }

    // dtrans implementation
    virtual com::sun::star::uno::Reference< com::sun::star::uno::XInterface >
        CreateClipboard( const com::sun::star::uno::Sequence< com::sun::star::uno::Any >& i_rArguments );
    virtual com::sun::star::uno::Reference< com::sun::star::uno::XInterface > CreateDragSource();
    virtual com::sun::star::uno::Reference< com::sun::star::uno::XInterface > CreateDropTarget();
    virtual void            AddToRecentDocumentList(const OUString& rFileUrl, const OUString& rMimeType, const OUString& rDocumentService);
};

#endif // _SV_SALINST_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
