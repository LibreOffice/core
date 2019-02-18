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
#include <vcl/scrbar.hxx>
#include <vcl/fixed.hxx>

#include <rangelst.hxx>
#include "condformatdlgitem.hxx"
#include "condformatdlgentry.hxx"

#include "anyrefdg.hxx"

#include <memory>

#define DLG_RET_ADD         8
#define DLG_RET_EDIT        16

class ScDocument;
class ScConditionalFormat;
class ScViewData;

class ScCondFormatDlg;

class ScCondFormatList : public Control
{
private:
    typedef std::vector<VclPtr<ScCondFrmtEntry>> EntryContainer;
    EntryContainer maEntries;

    bool mbHasScrollBar;
    bool mbFrozen;
    bool mbNewEntry;
    VclPtr<ScrollBar> mpScrollBar;

    ScDocument* mpDoc;
    ScAddress maPos;
    ScRangeList maRanges;
    VclPtr<ScCondFormatDlg> mpDialogParent;

    void DoScroll(long nDiff);

public:
    ScCondFormatList(vcl::Window* pParent, WinBits nStyle);
    virtual ~ScCondFormatList() override;
    virtual void dispose() override;

    void init(ScDocument* pDoc, ScCondFormatDlg* pDialogParent, const ScConditionalFormat* pFormat,
        const ScRangeList& rRanges, const ScAddress& rPos,
        condformat::dialog::ScCondFormatDialogType eType);

    void SetRange(const ScRangeList& rRange);

    virtual Size GetOptimalSize() const override;
    virtual void queue_resize(StateChangedType eReason = StateChangedType::Layout) override;
    virtual void Resize() override;

    std::unique_ptr<ScConditionalFormat> GetConditionalFormat() const;
    void Freeze() { mbFrozen = true; }
    void Thaw() { mbFrozen = false; }
    void RecalcAll();

    DECL_LINK( AddBtnHdl, Button*, void );
    DECL_LINK( RemoveBtnHdl, Button*, void );
    DECL_LINK( UpBtnHdl, Button*, void );
    DECL_LINK( DownBtnHdl, Button*, void );
    DECL_LINK( ScrollHdl, ScrollBar*, void );
    DECL_LINK( EntrySelectHdl, ScCondFrmtEntry&, void );

    DECL_LINK( TypeListHdl, ListBox&, void );
    DECL_LINK( AfterTypeListHdl, void*, void );
    DECL_LINK( ColFormatTypeHdl, ListBox&, void );
};

class ScCondFormatDlg : public ScAnyRefDlg
{
private:
    VclPtr<PushButton> mpBtnOk;
    VclPtr<PushButton> mpBtnAdd;
    VclPtr<PushButton> mpBtnRemove;
    VclPtr<PushButton> mpBtnUp;
    VclPtr<PushButton> mpBtnDown;
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

    DECL_LINK( EdRangeModifyHdl, Edit&, void );
protected:

    virtual void RefInputDone( bool bForced = false ) override;
    void OkPressed();
    void CancelPressed();

public:
    SC_DLLPUBLIC ScCondFormatDlg(SfxBindings* pB, SfxChildWindow* pCW, vcl::Window* pWindow,
                                 ScViewData* pViewData, const ScCondFormatDlgItem* pDlgItem);
    virtual ~ScCondFormatDlg() override;
    virtual void dispose() override;

    SC_DLLPUBLIC std::unique_ptr<ScConditionalFormat> GetConditionalFormat() const;

    virtual void SetReference(const ScRange&, ScDocument*) override;
    virtual bool IsRefInputMode() const override;
    virtual void SetActive() override;
    virtual bool IsTableLocked() const override;
    virtual bool Close() override;

    void InvalidateRefData();
    void OnSelectionChange(size_t nIndex, size_t nSize, bool bSelected = true);

    DECL_LINK( BtnPressedHdl, Button*, void );
    DECL_LINK( RangeGetFocusHdl, Control&, void );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
