/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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

    String  GetSelectionText( sal_Bool bWholeWord );
};



#endif

