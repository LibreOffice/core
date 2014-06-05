/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_SOURCE_UI_INC_CONDFORMATMGR_HXX
#define INCLUDED_SC_SOURCE_UI_INC_CONDFORMATMGR_HXX

#include "sc.hrc"

#include <vcl/dialog.hxx>
#include <vcl/layout.hxx>
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
    ScCondFormatManagerWindow( Window* pParent, ScDocument* pDoc, ScConditionalFormatList* pFormatList);
    void ChangeSize(Size aSize);

    void DeleteSelection();
    ScConditionalFormat* GetSelection();
    void Update();
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
    PushButton *m_pBtnAdd;
    PushButton *m_pBtnRemove;
    PushButton *m_pBtnEdit;
    ScConditionalFormatList* mpFormatList;
    Window *m_pContainer;
    VclVBox *m_pGrid;
    ScCondFormatManagerWindow *m_pCtrlManager;

    ScDocument* mpDoc;
    ScAddress maPos;

    DECL_LINK(RemoveBtnHdl, void*);
    DECL_LINK(EditBtnHdl, void*);
    DECL_LINK(AddBtnHdl, void*);

    virtual void Resize();

    bool mbModified;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
