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
    OUString            sFileNm;
    OUString            sFilter;
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
    sal_Bool bNativFormat : 1;
    sal_Bool bClearMedium : 1;
    sal_Bool bStateChangeCalled : 1;
    sal_Bool bInCallDownLoad : 1;

    sal_Bool GetGraphic_Impl( Graphic&, SvStream* pStream = 0 );
    sal_Bool LoadFile_Impl();
    void SendStateChg_Impl( sfx2::LinkManager::LinkState nState );

    DECL_STATIC_LINK( SvFileObject, DelMedium_Impl, SfxMediumRef* );
    DECL_STATIC_LINK( SvFileObject, LoadGrfReady_Impl, void* );
    DECL_LINK( DialogClosedHdl, sfx2::FileDialogHelper* );

protected:
    virtual ~SvFileObject();

public:
    SvFileObject();

    virtual sal_Bool GetData( ::com::sun::star::uno::Any & rData /*out param*/,
                            const OUString & rMimeType,
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
