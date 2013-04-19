/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

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

#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/scoped_ptr.hpp>
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

class ScCondFormatList : public Control
{
private:
    typedef boost::ptr_vector<ScCondFrmtEntry> EntryContainer;
    EntryContainer maEntries;

    bool mbHasScrollBar;
    boost::scoped_ptr<ScrollBar> mpScrollBar;

    ScDocument* mpDoc;
    ScAddress maPos;
    ScRangeList maRanges;

    void DoScroll(long nDiff);

public:
    ScCondFormatList( Window* pParent, const ResId& rResId, ScDocument* pDoc, const ScConditionalFormat* pFormat,
            const ScRangeList& rRanges, const ScAddress& rPos, condformat::dialog::ScCondFormatDialogType eType);

    ScConditionalFormat* GetConditionalFormat() const;
    void RecalcAll();

    DECL_LINK( AddBtnHdl, void* );
    DECL_LINK( RemoveBtnHdl, void* );
    DECL_LINK( ScrollHdl, void* );
    DECL_LINK( EntrySelectHdl, ScCondFrmtEntry* );

    DECL_LINK( TypeListHdl, ListBox*);
    DECL_LINK( ColFormatTypeHdl, ListBox*);
};

class ScCondFormatDlg : public ScAnyRefModalDlg
{
private:
    PushButton maBtnAdd;
    PushButton maBtnRemove;
    OKButton maBtnOk;
    CancelButton maBtnCancel;
    FixedText maFtRange;
    formula::RefEdit maEdRange;
    formula::RefButton maRbRange;

    ScCondFormatList maCondFormList;

    ScAddress maPos;
    ScDocument* mpDoc;

    formula::RefEdit* mpLastEdit;

    DECL_LINK( EdRangeModifyHdl, Edit* );
protected:

    virtual void RefInputDone( sal_Bool bForced = false );


public:
    SC_DLLPUBLIC ScCondFormatDlg(Window* pWindow, ScDocument* pDoc, const ScConditionalFormat* pFormat,
            const ScRangeList& rRange, const ScAddress& rPos, condformat::dialog::ScCondFormatDialogType eType);
    virtual ~ScCondFormatDlg();

    SC_DLLPUBLIC ScConditionalFormat* GetConditionalFormat() const;

    virtual void SetReference(const ScRange&, ScDocument*);
    virtual sal_Bool IsRefInputMode() const;
    virtual void SetActive();
    virtual sal_Bool IsTableLocked() const;

    void InvalidateRefData();

    DECL_LINK( RangeGetFocusHdl, formula::RefEdit* );
    DECL_LINK( RangeLoseFocusHdl, void* );
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
