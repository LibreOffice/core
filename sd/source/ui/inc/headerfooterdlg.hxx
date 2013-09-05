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
#ifndef _SD_HEADERFOOTER_DIALOG_HXX
#define _SD_HEADERFOOTER_DIALOG_HXX

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
    DECL_LINK( ActivatePageHdl, TabControl * );
    DECL_LINK( DeactivatePageHdl, void * );

    DECL_LINK( ClickApplyToAllHdl, void * );
    DECL_LINK( ClickApplyHdl, void * );
    DECL_LINK( ClickCancelHdl, void * );

    TabControl*      mpTabCtrl;

    HeaderFooterTabPage*    mpSlideTabPage;
    HeaderFooterTabPage*    mpNotesHandoutsTabPage;

    sal_uInt16 mnSlidesId;
    sal_uInt16 mnNotesId;

    PushButton*      maPBApplyToAll;
    PushButton*      maPBApply;
    CancelButton*    maPBCancel;

    HeaderFooterSettings    maSlideSettings;
    HeaderFooterSettings    maNotesHandoutSettings;

    SdDrawDocument*         mpDoc;
    SdPage*                 mpCurrentPage;
    ViewShell*              mpViewShell;

    void apply( bool bToAll, bool bForceSlides );
    void change( SdUndoGroup* pUndoGroup, SdPage* pPage, const HeaderFooterSettings& rNewSettings );

public:
    HeaderFooterDialog( ViewShell* pViewShell, ::Window* pParent, SdDrawDocument* pDoc, SdPage* pCurrentPage );
    ~HeaderFooterDialog();

    void ApplyToAll();
    void Apply();
    void Cancel();

    virtual short Execute();
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
