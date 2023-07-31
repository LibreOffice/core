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

class SfxModule;
class EditView;
class ScViewData;
class ScInputHandler;
class SvxURLField;
class TransferableDataHelper;
class TransferableClipboardListener;

class ScEditShell final : public SfxShell
{
private:
    EditView*   pEditView;
    ScViewData& rViewData;
    rtl::Reference<TransferableClipboardListener> mxClipEvtLstnr;
    bool        bPastePossible;
    bool        bIsInsertMode;

    // tdf#140361 at context menu popup time set if the EditHyperlink entry
    // should be disabled and use that state if queried about it if
    // EditHyperlink is dispatched from the menu. So ignoring where the mouse
    // currently happens to be when the menu was dismissed.
    std::optional<bool> moAtContextMenu_DisableEditHyperlink;

    const SvxURLField* GetURLField();
    const SvxURLField* GetFirstURLFieldFromCell();
    ScInputHandler* GetMyInputHdl();

    DECL_LINK( ClipboardChanged, TransferableDataHelper*, void );

public:
    SFX_DECL_INTERFACE(SCID_EDIT_SHELL)

private:
    /// SfxInterface initializer.
    static void InitInterface_Impl();

public:
    ScEditShell(EditView* pView, ScViewData& rData);
    virtual ~ScEditShell() override;

    void    SetEditView(EditView* pView);
    EditView* GetEditView() {return pEditView;}

    void    Execute(SfxRequest& rReq);
    void    ExecuteTrans(const SfxRequest& rReq);
    void    GetState(SfxItemSet &rSet);
    void    GetClipState(SfxItemSet& rSet);

    void    ExecuteAttr(SfxRequest& rReq);
    void    GetAttrState(SfxItemSet &rSet);

    void    ExecuteUndo(const SfxRequest& rReq);
    void    GetUndoState(SfxItemSet &rSet);

    OUString GetSelectionText( bool bWholeWord );

    /// return true if "Edit Hyperlink" in context menu should be disabled
    bool ShouldDisableEditHyperlink() const;
    /// force "Edit Hyperlink" to true, with the expectation that SID_EDIT_HYPERLINK is
    /// later Invalidated to reset it back to its natural value
    void EnableEditHyperlink();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
