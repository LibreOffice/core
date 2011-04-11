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
#ifndef _FILEOBJ_HXX
#define _FILEOBJ_HXX

#include <tools/string.hxx>
#include <sfx2/linksrc.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/linkmgr.hxx>

class Graphic;
struct Impl_DownLoadData;
namespace sfx2 { class FileDialogHelper; }

class SvFileObject : public sfx2::SvLinkSource
{
    String              sFileNm;
    String              sFilter;
    String              sReferer;
    Link                aEndEditLink;
    SfxMediumRef        xMed;
    Impl_DownLoadData*  pDownLoadData;
    Window*             pOldParent;

    sal_uInt8 nType;

    sal_Bool bLoadAgain : 1;
    sal_Bool bSynchron : 1;
    sal_Bool bLoadError : 1;
    sal_Bool bWaitForData : 1;
    sal_Bool bInNewData : 1;
    sal_Bool bDataReady : 1;
    sal_Bool bMedUseCache : 1;
    sal_Bool bNativFormat : 1;
    sal_Bool bClearMedium : 1;
    sal_Bool bStateChangeCalled : 1;
    sal_Bool bInCallDownLoad : 1;

    sal_Bool GetGraphic_Impl( Graphic&, SvStream* pStream = 0 );
    sal_Bool LoadFile_Impl();
    void SendStateChg_Impl( sfx2::LinkManager::LinkState nState );

    DECL_STATIC_LINK( SvFileObject, DelMedium_Impl, SfxMediumRef* );
    DECL_STATIC_LINK( SvFileObject, LoadGrfReady_Impl, void* );
    DECL_STATIC_LINK( SvFileObject, LoadGrfNewData_Impl, void* );
    DECL_LINK( DialogClosedHdl, sfx2::FileDialogHelper* );

protected:
    virtual ~SvFileObject();

public:
    SvFileObject();

    virtual sal_Bool GetData( ::com::sun::star::uno::Any & rData /*out param*/,
                            const String & rMimeType,
                            sal_Bool bSynchron = sal_False );

    virtual sal_Bool    Connect( sfx2::SvBaseLink* );
    virtual void    Edit( Window *, sfx2::SvBaseLink *, const Link& rEndEditHdl );

    // Ask whether you can access data directly or whether it has to be triggered
    virtual sal_Bool IsPending() const;
    virtual sal_Bool IsDataComplete() const;

    void CancelTransfers();
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
