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

#ifndef SC_DRTXTOB_HXX
#define SC_DRTXTOB_HXX

#include <sfx2/shell.hxx>
#include <sfx2/module.hxx>
#include <tools/link.hxx>

#include "shellids.hxx"

sal_uInt16 ScGetFontWorkId();       // instead of SvxFontWorkChildWindow::GetChildWindowId()

class ScViewData;
class TransferableDataHelper;
class TransferableClipboardListener;

class ScDrawTextObjectBar : public SfxShell
{
    ScViewData*         pViewData;
    TransferableClipboardListener* pClipEvtLstnr;
    sal_Bool                bPastePossible;

    DECL_LINK( ClipboardChanged, TransferableDataHelper* );

public:
    TYPEINFO();
    SFX_DECL_INTERFACE(SCID_DRAW_TEXT_SHELL)

        ScDrawTextObjectBar(ScViewData* pData);
        ~ScDrawTextObjectBar();

    void StateDisableItems( SfxItemSet &rSet );

    void Execute( SfxRequest &rReq );
    void ExecuteTrans( SfxRequest& rReq );
    void GetState( SfxItemSet& rSet );
    void GetClipState( SfxItemSet& rSet );

    void ExecuteAttr( SfxRequest &rReq );
    void GetAttrState( SfxItemSet& rSet );
    void ExecuteToggle( SfxRequest &rReq );

    sal_Bool ExecuteCharDlg( const SfxItemSet& rArgs, SfxItemSet& rOutSet );
    sal_Bool ExecuteParaDlg( const SfxItemSet& rArgs, SfxItemSet& rOutSet );

    void ExecuteExtra( SfxRequest &rReq );
    void ExecFormText(SfxRequest& rReq);        // StarFontWork
    void GetFormTextState(SfxItemSet& rSet);

private:
    void ExecuteGlobal( SfxRequest &rReq );         // called by Execute for all objects
    void GetGlobalClipState( SfxItemSet& rSet );
    void ExecutePasteContents( SfxRequest &rReq );
    sal_Bool IsNoteEdit();
};



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
