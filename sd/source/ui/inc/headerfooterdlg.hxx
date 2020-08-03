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
#pragma once

#include <vcl/weld.hxx>

#include <sdpage.hxx>

class SdUndoGroup;

namespace sd
{
class ViewShell;

class HeaderFooterTabPage;

class HeaderFooterDialog : public weld::GenericDialogController
{
private:
    DECL_LINK( ActivatePageHdl, const OString&, void );
    DECL_LINK( ClickApplyToAllHdl, weld::Button&, void );
    DECL_LINK( ClickApplyHdl, weld::Button&, void );
    DECL_LINK( ClickCancelHdl, weld::Button&, void );

    HeaderFooterSettings    maSlideSettings;
    HeaderFooterSettings    maNotesHandoutSettings;

    SdDrawDocument*         mpDoc;
    SdPage*                 mpCurrentPage;
    ViewShell*              mpViewShell;

    std::unique_ptr<weld::Notebook> mxTabCtrl;
    std::unique_ptr<weld::Button> mxPBApplyToAll;
    std::unique_ptr<weld::Button> mxPBApply;
    std::unique_ptr<weld::Button> mxPBCancel;
    std::unique_ptr<HeaderFooterTabPage> mxSlideTabPage;
    std::unique_ptr<HeaderFooterTabPage> mxNotesHandoutsTabPage;

    void apply( bool bToAll, bool bForceSlides );
    void change( SdUndoGroup* pUndoGroup, SdPage* pPage, const HeaderFooterSettings& rNewSettings );

public:
    HeaderFooterDialog(ViewShell* pViewShell, weld::Window* pParent, SdDrawDocument* pDoc, SdPage* pCurrentPage);
    virtual ~HeaderFooterDialog() override;

    void ApplyToAll();
    void Apply();

    virtual short run() override;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
