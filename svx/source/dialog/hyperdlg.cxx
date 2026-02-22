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

#include <svx/hyperdlg.hxx>
#include <sfx2/sfxdlg.hxx>
#include <sfx2/sfxsids.hrc>
#include <sfx2/viewfrm.hxx>
#include <sfx2/dispatch.hxx>
#include <svl/eitem.hxx>
#include <vcl/svapp.hxx>

//#                                                                      #
//# Childwindow-Wrapper-Class                                            #
//#                                                                      #
SFX_IMPL_CHILDWINDOW_WITHID(SvxHlinkDlgWrapper, SID_HYPERLINK_DIALOG)

SvxHlinkDlgWrapper::SvxHlinkDlgWrapper( vcl::Window* _pParent, sal_uInt16 nId,
                                        SfxBindings*,
                                        SfxChildWinInfo*) :
    SfxChildWindow( _pParent, nId )
{
    SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
    mpDlg = pFact->CreateHyperlinkDialog(_pParent->GetFrameWeld(), this);
    mpDlg->StartExecuteAsync([](sal_Int32) {
        SfxViewFrame* pViewFrame = SfxViewFrame::Current();
        if (!pViewFrame || !pViewFrame->HasChildWindow(SID_HYPERLINK_DIALOG))
            return;
        SfxBoolItem aValue(SID_HYPERLINK_DIALOG, false);
        pViewFrame->GetDispatcher()->ExecuteList(
            SID_HYPERLINK_DIALOG,
            SfxCallMode::RECORD , { &aValue });
    });
}

SfxChildWinInfo SvxHlinkDlgWrapper::GetInfo() const
{
    SfxChildWinInfo aInfo = SfxChildWindow::GetInfo();
    aInfo.bVisible = false;
    return aInfo;
}

SvxHlinkDlgWrapper::~SvxHlinkDlgWrapper()
{
    mpDlg.disposeAndClear();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

