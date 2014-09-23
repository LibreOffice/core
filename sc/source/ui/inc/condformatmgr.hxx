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
#include <svtools/simptabl.hxx>

#include "conditio.hxx"

#include <map>

class ScDocument;

class ScCondFormatManagerWindow : public SvSimpleTable
{
private:
    void Init();
    OUString createEntryString(const ScConditionalFormat& rFormat);
    void setColSizes();

    ScDocument* mpDoc;
    ScConditionalFormatList* mpFormatList;
    std::map<SvTreeListEntry*, sal_Int32> maMapLBoxEntryToCondIndex;

public:
    ScCondFormatManagerWindow(SvSimpleTableContainer& rParent, ScDocument* pDoc, ScConditionalFormatList* pFormatList);

    void DeleteSelection();
    ScConditionalFormat* GetSelection();
    void Update();
    virtual void Resize() SAL_OVERRIDE;
};

class ScCondFormatManagerDlg : public ModalDialog
{
public:
    ScCondFormatManagerDlg(vcl::Window* pParent, ScDocument* pDoc, const ScConditionalFormatList* pFormatList, const ScAddress& rPos);
    virtual ~ScCondFormatManagerDlg();

    ScConditionalFormatList* GetConditionalFormatList();

    bool CondFormatsChanged() { return mbModified;}

    virtual bool IsInRefMode() const;

private:
    PushButton *m_pBtnAdd;
    PushButton *m_pBtnRemove;
    PushButton *m_pBtnEdit;
    ScConditionalFormatList* mpFormatList;
    ScCondFormatManagerWindow *m_pCtrlManager;

    ScDocument* mpDoc;
    ScAddress maPos;

    DECL_LINK(RemoveBtnHdl, void*);
    DECL_LINK(EditBtnHdl, void*);
    DECL_LINK(AddBtnHdl, void*);

    bool mbModified;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
