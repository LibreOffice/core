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

#include <sfx2/shell.hxx>
#include <tools/link.hxx>
#include <rtl/ref.hxx>

#include <shellids.hxx>

sal_uInt16 ScGetFontWorkId();       // instead of SvxFontWorkChildWindow::GetChildWindowId()

class SfxModule;
class ScViewData;
class TransferableDataHelper;
class TransferableClipboardListener;

class ScDrawTextObjectBar final : public SfxShell
{
    ScViewData&         mrViewData;
    rtl::Reference<TransferableClipboardListener> mxClipEvtLstnr;
    bool                bPastePossible;

    DECL_LINK( ClipboardChanged, TransferableDataHelper*, void );

public:
    SFX_DECL_INTERFACE(SCID_DRAW_TEXT_SHELL)

private:
    /// SfxInterface initializer.
    static void InitInterface_Impl();

public:
    ScDrawTextObjectBar(ScViewData& rData);
    virtual ~ScDrawTextObjectBar() override;

    static void StateDisableItems( SfxItemSet &rSet );

    void Execute( SfxRequest &rReq );
    void ExecuteTrans( const SfxRequest& rReq );
    void GetState( SfxItemSet& rSet );
    void GetClipState( SfxItemSet& rSet );

    void ExecuteAttr( SfxRequest &rReq );
    void GetAttrState( SfxItemSet& rSet );
    void ExecuteToggle( SfxRequest &rReq );
    void GetStatePropPanelAttr(SfxItemSet &);

    bool ExecuteCharDlg( const SfxItemSet& rArgs, SfxItemSet& rOutSet , sal_uInt16 nSlot);
    bool ExecuteParaDlg( const SfxItemSet& rArgs, SfxItemSet& rOutSet );

    void ExecuteExtra( SfxRequest &rReq );
    void ExecFormText(const SfxRequest& rReq);        // StarFontWork
    void GetFormTextState(SfxItemSet& rSet);

private:
    void ExecuteGlobal( SfxRequest &rReq );         // called by Execute for all objects
    static void GetGlobalClipState( SfxItemSet& rSet );
    void ExecutePasteContents( SfxRequest &rReq );
    bool IsNoteEdit() const;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
