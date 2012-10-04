/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2012 Markus Mohrhard <markus.mohrhard@googlemail.com> (initial developer)
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include "sc.hrc"

#include <vcl/dialog.hxx>
#include <svtools/svtabbx.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <svtools/headbar.hxx>

#include "conditio.hxx"
#include "document.hxx"

#include <map>

class ScDocument;

class ScCondFormatManagerWindow : public SvTabListBox
{
private:
    void Init();
    String createEntryString(const ScConditionalFormat& rFormat);

    HeaderBar maHeaderBar;
    ScDocument* mpDoc;
    ScConditionalFormatList* mpFormatList;
    const ScAddress& mrPos;
    std::map<SvLBoxEntry*, sal_Int32> maMapLBoxEntryToCondIndex;

    DECL_LINK( HeaderEndDragHdl, void* );

public:
    ScCondFormatManagerWindow( Window* pParent, ScDocument* pDoc, ScConditionalFormatList* pFormatList, const ScAddress& rPos );

    void DeleteSelection();
    ScConditionalFormat* GetSelection();
    void Update();
};

class ScCondFormatManagerCtrl : Control
{
public:
    ScCondFormatManagerCtrl(Window* pParent, ScDocument* pDoc, ScConditionalFormatList* pFormatList, const ScAddress& rPos);

    ScConditionalFormat* GetSelection();
    void DeleteSelection();
    void Update();
    ScCondFormatManagerWindow &GetListControl() { return maWdManager; }

private:
    ScCondFormatManagerWindow maWdManager;
};

class ScCondFormatManagerDlg : public ModalDialog
{
public:
    ScCondFormatManagerDlg(Window* pParent, ScDocument* pDoc, const ScConditionalFormatList* pFormatList, const ScAddress& rPos);
    ~ScCondFormatManagerDlg();

    ScConditionalFormatList* GetConditionalFormatList();

    bool CondFormatsChanged();

private:
    PushButton maBtnAdd;
    PushButton maBtnRemove;
    PushButton maBtnEdit;

    OKButton maBtnOk;
    CancelButton maBtnCancel;

    FixedLine maFlLine;

    ScConditionalFormatList* mpFormatList;

    ScCondFormatManagerCtrl maCtrlManager;

    ScDocument* mpDoc;
    ScAddress maPos;

    DECL_LINK(RemoveBtnHdl, void*);
    DECL_LINK(EditBtnHdl, void*);

    bool mbModified;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
