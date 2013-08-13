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

#ifndef SC_EDITSH_HXX
#define SC_EDITSH_HXX

#include <sfx2/shell.hxx>
#include <sfx2/module.hxx>
#include <tools/link.hxx>

#include "shellids.hxx"
#include <editeng/editview.hxx>

class EditView;
class ScViewData;
class ScInputHandler;
class SvxURLField;
class TransferableDataHelper;
class TransferableClipboardListener;

class ScEditShell : public SfxShell
{
private:
    EditView*   pEditView;
    ScViewData* pViewData;
    TransferableClipboardListener* pClipEvtLstnr;
    sal_Bool        bPastePossible;
    sal_Bool        bIsInsertMode;

    const SvxURLField* GetURLField();
    ScInputHandler* GetMyInputHdl();

    DECL_LINK( ClipboardChanged, TransferableDataHelper* );

public:
    TYPEINFO();
    SFX_DECL_INTERFACE(SCID_EDIT_SHELL)

                    ScEditShell(EditView* pView, ScViewData* pData);
                    ~ScEditShell();

    void    SetEditView(EditView* pView);
    EditView* GetEditView() {return pEditView;}

    void    Execute(SfxRequest& rReq);
    void    ExecuteTrans(SfxRequest& rReq);
    void    GetState(SfxItemSet &rSet);
    void    GetClipState(SfxItemSet& rSet);

    void    ExecuteAttr(SfxRequest& rReq);
    void    GetAttrState(SfxItemSet &rSet);

    void    ExecuteUndo(SfxRequest& rReq);
    void    GetUndoState(SfxItemSet &rSet);

    OUString GetSelectionText( bool bWholeWord );
};



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
