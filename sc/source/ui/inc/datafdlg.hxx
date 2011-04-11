/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *       Novell Inc.
 * Portions created by the Initial Developer are Copyright (C) 2010 the
 * Initial Developer. All Rights Reserved.
 *
 * Contributor(s): Amelia Wang <amwang@novell.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#ifndef SC_DATAFDLG_HXX
#define SC_DATAFDLG_HXX

#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>

#include "global.hxx"

#include <tabvwsh.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>

#define MAX_DATAFORM_COLS   256
#define MAX_DATAFORM_ROWS   32000
#define CTRL_HEIGHT         22
#define FIXED_WIDTH         60
#define EDIT_WIDTH          140
#define FIXED_LEFT          12
#define EDIT_LEFT           78
#define LINE_HEIGHT         30

//zhangyun
class ScDataFormDlg : public ModalDialog
{
private:

    PushButton      aBtnNew;
    PushButton      aBtnDelete;
    PushButton      aBtnRestore;
    PushButton      aBtnLast;
    PushButton      aBtnNext;
    PushButton      aBtnClose;
    ScrollBar       aSlider;
    FixedText       aFixedText;

    ScTabViewShell* pTabViewShell;
    ScDocument*     pDoc;
    sal_uInt16      aColLength;
    SCROW           aCurrentRow;
    SCCOL           nStartCol;
    SCCOL           nEndCol;
    SCROW           nStartRow;
    SCROW           nEndRow;
    SCTAB           nTab;
    sal_Bool            bNoSelection;

    FixedText** pFixedTexts;
    Edit** pEdits;

public:
    ScDataFormDlg( Window* pParent, ScTabViewShell* pTabViewShell);
    ~ScDataFormDlg();

    void FillCtrls(SCROW nCurrentRow);
private:

    void SetButtonState();

    // Handler:
    DECL_LINK( Impl_NewHdl,     PushButton*    );
    DECL_LINK( Impl_LastHdl,    PushButton*    );
    DECL_LINK( Impl_NextHdl,    PushButton*    );

    DECL_LINK( Impl_RestoreHdl, PushButton*    );
    DECL_LINK( Impl_DeleteHdl,  PushButton*    );
    DECL_LINK( Impl_CloseHdl,   PushButton*    );

    DECL_LINK( Impl_ScrollHdl,  ScrollBar*    );
    DECL_LINK( Impl_DataModifyHdl,  Edit*    );
};
#endif // SC_DATAFDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
