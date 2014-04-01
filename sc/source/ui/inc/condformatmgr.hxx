/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SC_CONDFORMATMGR_HXX
#define SC_CONDFORMATMGR_HXX

#include "sc.hrc"

#include <vcl/dialog.hxx>
#include <svtools/svtabbx.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <svtools/headbar.hxx>

#include "conditio.hxx"

#include <map>

class ScDocument;

class ScCondFormatManagerWindow : public SvTabListBox
{
private:
    void Init();
    OUString createEntryString(const ScConditionalFormat& rFormat);

    HeaderBar maHeaderBar;
    ScDocument* mpDoc;
    ScConditionalFormatList* mpFormatList;
    std::map<SvTreeListEntry*, sal_Int32> maMapLBoxEntryToCondIndex;

    DECL_LINK( HeaderEndDragHdl, void* );

public:
    ScCondFormatManagerWindow( Window* pParent, ScDocument* pDoc, ScConditionalFormatList* pFormatList );

    void DeleteSelection();
    ScConditionalFormat* GetSelection();
    void Update();
};

class ScCondFormatManagerCtrl : Control
{
public:
    ScCondFormatManagerCtrl(Window* pParent, ScDocument* pDoc, ScConditionalFormatList* pFormatList);

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
    virtual ~ScCondFormatManagerDlg();

    ScConditionalFormatList* GetConditionalFormatList();

    bool CondFormatsChanged();

    virtual bool IsInRefMode() const;

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
    DECL_LINK(AddBtnHdl, void*);

    bool mbModified;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
