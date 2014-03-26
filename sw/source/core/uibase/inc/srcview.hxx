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
#ifndef INCLUDED_SW_SOURCE_UI_INC_SRCVIEW_HXX
#define INCLUDED_SW_SOURCE_UI_INC_SRCVIEW_HXX

#include <sfx2/viewfac.hxx>
#include <sfx2/viewsh.hxx>
#include <vcl/outdev.hxx>

#include "srcedtw.hxx"
#include "shellid.hxx"

class SwDocShell;
class SvxSearchItem;
class SfxMedium;

class SwSrcView: public SfxViewShell
{
    SwSrcEditWindow     aEditWin;

    SvxSearchItem*      pSearchItem;

    sal_Bool                bSourceSaved    :1;
    rtl_TextEncoding    eLoadEncoding;
    void                Init();

    // for read-only switching
    virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

protected:
    sal_uInt16              StartSearchAndReplace(const SvxSearchItem& rItem,
                                                        sal_Bool bFromStart,
                                                        sal_Bool bApi,
                                                        sal_Bool bRecursive = sal_False);

public:

    SFX_DECL_VIEWFACTORY(SwSrcView);
    SFX_DECL_INTERFACE(SW_SRC_VIEWSHELL)
    TYPEINFO_OVERRIDE();

    SwSrcView(SfxViewFrame* pFrame, SfxViewShell*);

    ~SwSrcView();

    SwDocShell*         GetDocShell();
    SwSrcEditWindow&    GetEditWin() {return aEditWin;}
    void                SaveContent(const OUString& rTmpFile);
    void                SaveContentTo(SfxMedium& rMed);

    sal_Bool                IsModified() {return aEditWin.IsModified();}

    void            Execute(SfxRequest&);
    void            GetState(SfxItemSet&);

    SvxSearchItem*  GetSearchItem();
    void            SetSearchItem( const SvxSearchItem& rItem );

    void            Load(SwDocShell* pDocShell);

    virtual sal_uInt16  SetPrinter( SfxPrinter* pNew, sal_uInt16 nDiff = SFX_PRINTER_ALL, bool bIsAPI=false );
    virtual SfxPrinter* GetPrinter( bool bCreate = false );

    sal_Int32       PrintSource( OutputDevice *pOutDev, sal_Int32 nPage, bool bCalcNumPagesOnly );

    void            SourceSaved() {bSourceSaved = sal_True;}
    sal_Bool            HasSourceSaved() const {return bSourceSaved;}

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
