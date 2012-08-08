/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _SV_SALINST_H
#define _SV_SALINST_H

#include <tools/solar.h>
#include <osl/thread.hxx>
#include <vclpluginapi.h>
#include <salinst.hxx>
#include <vcl/solarmutex.hxx>
#include "generic/geninst.h"


// -=-= SalInstanceData =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class SalXLib;
class VCLPLUG_GEN_PUBLIC X11SalInstance : public SalGenericInstance
{
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
    void                        FillFontPathList( std::list< rtl::OString >& o_rFontPaths );
    void                        SetLib( SalXLib *pXLib ) { mpXLib = pXLib; }

    // dtrans implementation
    virtual com::sun::star::uno::Reference< com::sun::star::uno::XInterface >
        CreateClipboard( const com::sun::star::uno::Sequence< com::sun::star::uno::Any >& i_rArguments );
    virtual com::sun::star::uno::Reference< com::sun::star::uno::XInterface > CreateDragSource();
    virtual com::sun::star::uno::Reference< com::sun::star::uno::XInterface > CreateDropTarget();
    virtual void            AddToRecentDocumentList(const rtl::OUString& rFileUrl, const rtl::OUString& rMimeType);
};

#endif // _SV_SALINST_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
