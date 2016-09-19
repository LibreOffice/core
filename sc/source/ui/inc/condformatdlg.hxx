/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_SOURCE_UI_INC_CONDFORMATDLG_HXX
#define INCLUDED_SC_SOURCE_UI_INC_CONDFORMATDLG_HXX

#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/scrbar.hxx>
#include <vcl/fixed.hxx>
#include <vcl/edit.hxx>
#include <svx/fntctrl.hxx>
#include <svtools/ctrlbox.hxx>

#include "rangelst.hxx"
#include "condformathelper.hxx"
#include "viewdata.hxx"
#include "condformatdlgitem.hxx"

#include "anyrefdg.hxx"

#include <memory>

#define DLG_RET_ADD         8
#define DLG_RET_EDIT        16

class ScDocument;
class ScConditionalFormat;
class ScFormatEntry;
class ScConditionalFormat;
struct ScDataBarFormatData;
class ScCondFrmtEntry;

class ScCondFormatDlg;

class ScCondFormatList : public Control
{
private:
    typedef std::vector<VclPtr<ScCondFrmtEntry>> EntryContainer;
    EntryContainer maEntries;

    bool mbHasScrollBar;
    VclPtr<ScrollBar> mpScrollBar;

    ScDocument* mpDoc;
    ScAddress maPos;
    ScRangeList maRanges;
    VclPtr<ScCondFormatDlg> mpDialogParent;

    void DoScroll(long nDiff);

public:
    ScCondFormatList(vcl::Window* pParent, WinBits nStyle);
    virtual ~ScCondFormatList();
    virtual void dispose() override;

    void init(ScDocument* pDoc, ScCondFormatDlg* pDialogParent, const ScConditionalFormat* pFormat,
        const ScRangeList& rRanges, const ScAddress& rPos,
        condformat::dialog::ScCondFormatDialogType eType);

    virtual Size GetOptimalSize() const override;
    virtual void Resize() override;

    ScConditionalFormat* GetConditionalFormat() const;
    void RecalcAll();

    DECL_LINK_TYPED( AddBtnHdl, Button*, void );
    DECL_LINK_TYPED( RemoveBtnHdl, Button*, void );
    DECL_LINK_TYPED( ScrollHdl, ScrollBar*, void );
    DECL_LINK_TYPED( EntrySelectHdl, ScCondFrmtEntry&, void );

    DECL_LINK_TYPED( TypeListHdl, ListBox&, void );
    DECL_LINK_TYPED( AfterTypeListHdl, void*, void );
    DECL_LINK_TYPED( ColFormatTypeHdl, ListBox&, void );
};

class ScCondFormatDlg : public ScAnyRefDlg
{
private:
    VclPtr<PushButton> mpBtnOk;
    VclPtr<PushButton> mpBtnAdd;
    VclPtr<PushButton> mpBtnRemove;
    VclPtr<PushButton> mpBtnCancel;
    VclPtr<FixedText> mpFtRange;
    VclPtr<formula::RefEdit> mpEdRange;
    VclPtr<formula::RefButton> mpRbRange;

    VclPtr<ScCondFormatList> mpCondFormList;
    sal_Int32 mnKey;

    ScAddress maPos;
    ScViewData* mpViewData;

    VclPtr<formula::RefEdit> mpLastEdit;

    std::shared_ptr<ScCondFormatDlgItem> mpDlgItem;

    OUString msBaseTitle;
    void updateTitle();

    DECL_LINK_TYPED( EdRangeModifyHdl, Edit&, void );
protected:

    virtual void RefInputDone( bool bForced = false ) override;
    void OkPressed();
    void CancelPressed();

public:
    SC_DLLPUBLIC ScCondFormatDlg(SfxBindings* pB, SfxChildWindow* pCW, vcl::Window* pWindow,
                                 ScViewData* pViewData, const ScCondFormatDlgItem* pDlgItem);
    virtual ~ScCondFormatDlg();
    virtual void dispose() override;

    SC_DLLPUBLIC ScConditionalFormat* GetConditionalFormat() const;

    virtual void SetReference(const ScRange&, ScDocument*) override;
    virtual bool IsRefInputMode() const override;
    virtual void SetActive() override;
    virtual bool IsTableLocked() const override;
    virtual bool Close() override;

    void InvalidateRefData();

    DECL_LINK_TYPED( BtnPressedHdl, Button*, void );
    DECL_LINK_TYPED( RangeGetFocusHdl, Control&, void );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
