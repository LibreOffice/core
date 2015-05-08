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

#include "anyrefdg.hxx"

class ScDocument;
class ScConditionalFormat;
class ScFormatEntry;
class ScConditionalFormat;
struct ScDataBarFormatData;
class ScCondFrmtEntry;

namespace condformat {

namespace dialog {

enum ScCondFormatDialogType
{
    NONE,
    CONDITION,
    COLORSCALE,
    DATABAR,
    ICONSET,
    DATE
};

}

}

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
    virtual void dispose() SAL_OVERRIDE;

    void init(ScDocument* pDoc, ScCondFormatDlg* pDialogParent, const ScConditionalFormat* pFormat,
        const ScRangeList& rRanges, const ScAddress& rPos,
        condformat::dialog::ScCondFormatDialogType eType);

    virtual Size GetOptimalSize() const SAL_OVERRIDE;
    virtual void Resize() SAL_OVERRIDE;

    ScConditionalFormat* GetConditionalFormat() const;
    void RecalcAll();

    DECL_LINK( AddBtnHdl, void* );
    DECL_LINK( RemoveBtnHdl, void* );
    DECL_LINK( ScrollHdl, void* );
    DECL_LINK( EntrySelectHdl, ScCondFrmtEntry* );

    DECL_LINK( TypeListHdl, ListBox*);
    DECL_LINK( AfterTypeListHdl, ListBox*);
    DECL_LINK( ColFormatTypeHdl, ListBox*);
};

class ScCondFormatDlg : public ScAnyRefModalDlg
{
private:
    VclPtr<PushButton> mpBtnAdd;
    VclPtr<PushButton> mpBtnRemove;
    VclPtr<FixedText> mpFtRange;
    VclPtr<formula::RefEdit> mpEdRange;
    VclPtr<formula::RefButton> mpRbRange;

    VclPtr<ScCondFormatList> mpCondFormList;

    ScAddress maPos;
    ScDocument* mpDoc;

    VclPtr<formula::RefEdit> mpLastEdit;

    DECL_LINK( EdRangeModifyHdl, Edit* );
protected:

    virtual void RefInputDone( bool bForced = false ) SAL_OVERRIDE;

public:
    SC_DLLPUBLIC ScCondFormatDlg(vcl::Window* pWindow, ScDocument* pDoc, const ScConditionalFormat* pFormat,
            const ScRangeList& rRange, const ScAddress& rPos, condformat::dialog::ScCondFormatDialogType eType);
    virtual ~ScCondFormatDlg();
    virtual void dispose() SAL_OVERRIDE;

    SC_DLLPUBLIC ScConditionalFormat* GetConditionalFormat() const;

    virtual void SetReference(const ScRange&, ScDocument*) SAL_OVERRIDE;
    virtual bool IsRefInputMode() const SAL_OVERRIDE;
    virtual void SetActive() SAL_OVERRIDE;
    virtual bool IsTableLocked() const SAL_OVERRIDE;

    void InvalidateRefData();

    DECL_LINK( RangeGetFocusHdl, formula::RefEdit* );
    DECL_STATIC_LINK( ScCondFormatDlg, RangeLoseFocusHdl, void* );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
