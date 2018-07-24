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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_SRCVIEW_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_SRCVIEW_HXX

#include <sfx2/viewfac.hxx>
#include <sfx2/viewsh.hxx>
#include <vcl/outdev.hxx>

#include "srcedtw.hxx"
#include <shellid.hxx>

class SwDocShell;
class SvxSearchItem;
class SfxMedium;

class SwSrcView: public SfxViewShell
{
    VclPtr<SwSrcEditWindow> aEditWin;

    std::unique_ptr<SvxSearchItem> pSearchItem;

    bool                bSourceSaved    :1;
    rtl_TextEncoding    eLoadEncoding;
    void                Init();

    // for read-only switching
    virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

protected:
    void                StartSearchAndReplace(const SvxSearchItem& rItem,
                                                  bool bApi,
                                                  bool bRecursive = false);

public:
    SFX_DECL_VIEWFACTORY(SwSrcView);
    SFX_DECL_INTERFACE(SW_SRC_VIEWSHELL)

private:
    /// SfxInterface initializer.
    static void InitInterface_Impl();

public:
    SwSrcView(SfxViewFrame* pFrame, SfxViewShell*);

    virtual ~SwSrcView() override;

    SwDocShell*         GetDocShell();
    void                SaveContent(const OUString& rTmpFile);
    void                SaveContentTo(SfxMedium& rMed);

    bool                IsModified() {return aEditWin->IsModified();}

    void            Execute(SfxRequest&);
    void            GetState(SfxItemSet&);

    SvxSearchItem*  GetSearchItem();
    void            SetSearchItem( const SvxSearchItem& rItem );

    void            Load(SwDocShell* pDocShell);

    virtual sal_uInt16  SetPrinter( SfxPrinter* pNew, SfxPrinterChangeFlags nDiff = SFX_PRINTER_ALL ) override;
    virtual SfxPrinter* GetPrinter( bool bCreate = false ) override;

    sal_Int32       PrintSource( OutputDevice *pOutDev, sal_Int32 nPage, bool bCalcNumPagesOnly );

    bool            HasSourceSaved() const {return bSourceSaved;}

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
