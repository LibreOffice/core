/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

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

class ScCondFormatList
{
private:
    std::unique_ptr<weld::ScrolledWindow> mxScrollWindow;
    std::unique_ptr<weld::Container> mxGrid;

    typedef std::vector<std::unique_ptr<ScCondFrmtEntry>> EntryContainer;
    EntryContainer maEntries;

    bool mbFrozen;
    bool mbNewEntry;

    ScDocument* mpDoc;
    ScAddress maPos;
    ScRangeList maRanges;
    ScCondFormatDlg* mpDialogParent;

public:
    ScCondFormatList(ScCondFormatDlg* pParent,
                     std::unique_ptr<weld::ScrolledWindow> xWindow,
                     std::unique_ptr<weld::Container> xGrid);
    weld::Container* GetContainer() { return mxGrid.get(); }
    ~ScCondFormatList();

    void init(ScDocument& rDoc, const ScConditionalFormat* pFormat,
        const ScRangeList& rRanges, const ScAddress& rPos,
        condformat::dialog::ScCondFormatDialogType eType);

    void SetRange(const ScRangeList& rRange);

    std::unique_ptr<ScConditionalFormat> GetConditionalFormat() const;
    weld::Window* GetFrameWeld();
    void Freeze() { mbFrozen = true; }
    void Thaw() { mbFrozen = false; }
    void RecalcAll();

    DECL_LINK( AddBtnHdl, weld::Button&, void );
    DECL_LINK( RemoveBtnHdl, weld::Button&, void );
    DECL_LINK( UpBtnHdl, weld::Button&, void );
    DECL_LINK( DownBtnHdl, weld::Button&, void );
    DECL_LINK( EntrySelectHdl, ScCondFrmtEntry&, void );

    DECL_LINK( TypeListHdl, weld::ComboBox&, void );
    DECL_LINK( AfterTypeListHdl, void*, void );
    DECL_LINK( ColFormatTypeHdl, weld::ComboBox&, void );
    DECL_LINK( AfterColFormatTypeHdl, void*, void );
};

class ScCondFormatDlg : public ScAnyRefDlgController
{
private:
    sal_Int32 mnKey;

    ScAddress maPos;
    ScViewData* mpViewData;

    std::shared_ptr<ScCondFormatDlgItem> mpDlgItem;

    OUString msBaseTitle;

    formula::RefEdit* mpLastEdit;
    std::unique_ptr<weld::Button> mxBtnOk;
    std::unique_ptr<weld::Button> mxBtnAdd;
    std::unique_ptr<weld::Button> mxBtnRemove;
    std::unique_ptr<weld::Button> mxBtnUp;
    std::unique_ptr<weld::Button> mxBtnDown;
    std::unique_ptr<weld::Button> mxBtnCancel;
    std::unique_ptr<weld::Label> mxFtRange;
    std::unique_ptr<formula::RefEdit> mxEdRange;
    std::unique_ptr<formula::RefButton> mxRbRange;
    std::unique_ptr<ScCondFormatList> mxCondFormList;

    void updateTitle();
    DECL_LINK( EdRangeModifyHdl, formula::RefEdit&, void );
protected:

    virtual void RefInputDone( bool bForced = false ) override;
    void OkPressed();
    void CancelPressed();

public:
    ScCondFormatDlg(SfxBindings* pB, SfxChildWindow* pCW, weld::Window* pWindow,
                                 ScViewData* pViewData, const ScCondFormatDlgItem* pDlgItem);
    virtual ~ScCondFormatDlg() override;

    std::unique_ptr<ScConditionalFormat> GetConditionalFormat() const;

    virtual void SetReference(const ScRange&, ScDocument&) override;
    virtual bool IsRefInputMode() const override;
    virtual void SetActive() override;
    virtual bool IsTableLocked() const override;
    virtual void Close() override;

    void InvalidateRefData();
    void OnSelectionChange(size_t nIndex, size_t nSize, bool bSelected = true);

    DECL_LINK( BtnPressedHdl, weld::Button&, void );
    DECL_LINK( RangeGetFocusHdl, formula::RefEdit&, void );
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
