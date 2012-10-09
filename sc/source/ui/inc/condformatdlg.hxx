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
    DATABAR
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

    void RecalcAll();
    void DoScroll(long nDiff);

public:
    ScCondFormatList( Window* pParent, const ResId& rResId, ScDocument* pDoc, const ScConditionalFormat* pFormat,
            const ScRangeList& rRanges, const ScAddress& rPos, condformat::dialog::ScCondFormatDialogType eType);

    ScConditionalFormat* GetConditionalFormat() const;

    DECL_LINK( AddBtnHdl, void* );
    DECL_LINK( RemoveBtnHdl, void* );
    DECL_LINK( ScrollHdl, void* );
    DECL_LINK( EntrySelectHdl, ScCondFrmtEntry* );

    DECL_LINK( TypeListHdl, ListBox*);
    DECL_LINK( ColFormatTypeHdl, ListBox*);
};

class ScCondFormatDlg : public ScAnyRefDlg
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

    const ScConditionalFormat* mpFormat;

    condformat::dialog::ScCondFormatDialogType meType;

    DECL_LINK( EdRangeModifyHdl, Edit* );
    DECL_LINK( OkBtnHdl, void* );
    DECL_LINK( CancelBtnHdl, void* );
    DECL_LINK( RangeGetFocusHdl, void* );

    virtual sal_Bool Close();
protected:

    virtual void RefInputDone( sal_Bool bForced = false );


public:
    ScCondFormatDlg(SfxBindings* pB, SfxChildWindow* pSW, Window* pWindow, ScDocument* pDoc, const ScConditionalFormat* pFormat,
            const ScRangeList& rRange, const ScAddress& rPos, condformat::dialog::ScCondFormatDialogType eType);
    virtual ~ScCondFormatDlg();

    SC_DLLPUBLIC ScConditionalFormat* GetConditionalFormat() const;

    virtual void SetReference(const ScRange&, ScDocument*);
    virtual sal_Bool IsRefInputMode() const;
    virtual void SetActive();
    virtual sal_Bool IsTableLocked() const { return sal_True; }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
