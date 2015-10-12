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
#ifndef INCLUDED_SD_SOURCE_UI_INC_HEADERFOOTERDLG_HXX
#define INCLUDED_SD_SOURCE_UI_INC_HEADERFOOTERDLG_HXX

#include <vcl/tabdlg.hxx>
#include <vcl/button.hxx>
#include <vcl/tabctrl.hxx>
#include <vcl/tabpage.hxx>

#include "sdpage.hxx"

class SdUndoGroup;

namespace sd
{
class ViewShell;

class HeaderFooterTabPage;

class HeaderFooterDialog : public TabDialog
{
private:
    DECL_LINK_TYPED( ActivatePageHdl, TabControl*, void );
    DECL_LINK_TYPED( ClickApplyToAllHdl, Button*, void );
    DECL_LINK_TYPED( ClickApplyHdl, Button*, void );
    DECL_LINK_TYPED( ClickCancelHdl, Button*, void );

    VclPtr<TabControl>      mpTabCtrl;

    VclPtr<HeaderFooterTabPage>    mpSlideTabPage;
    VclPtr<HeaderFooterTabPage>    mpNotesHandoutsTabPage;

    sal_uInt16 mnSlidesId;
    sal_uInt16 mnNotesId;

    VclPtr<PushButton>      maPBApplyToAll;
    VclPtr<PushButton>      maPBApply;
    VclPtr<CancelButton>    maPBCancel;

    HeaderFooterSettings    maSlideSettings;
    HeaderFooterSettings    maNotesHandoutSettings;

    SdDrawDocument*         mpDoc;
    SdPage*                 mpCurrentPage;
    ViewShell*              mpViewShell;

    void apply( bool bToAll, bool bForceSlides );
    void change( SdUndoGroup* pUndoGroup, SdPage* pPage, const HeaderFooterSettings& rNewSettings );

public:
    HeaderFooterDialog( ViewShell* pViewShell, vcl::Window* pParent, SdDrawDocument* pDoc, SdPage* pCurrentPage );
    virtual ~HeaderFooterDialog();
    virtual void dispose() override;

    void ApplyToAll();
    void Apply();
    void Cancel();

    virtual short Execute() override;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
