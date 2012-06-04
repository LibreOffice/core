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

#include "rangelst.hxx"

#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/scoped_ptr.hpp>

class ScDocument;
class ScConditionalFormat;
class ScFormatEntry;
class ScConditionalFormat;

enum ScCondFormatEntryType
{
    CONDITION,
    COLORSCALE,
    DATABAR,
    COLLAPSED
};

class ScCondFrmtEntry : public Control
{
private:
    bool mbActive;
    ScCondFormatEntryType meType;

    Link maClickHdl;

    //general ui elements
    ListBox maLbType;
    FixedText maFtCondNr;
    FixedText maFtCondition;

    //cond format ui elements
    ListBox maLbCondType;
    Edit maEdVal1;
    Edit maEdVal2;
    FixedText maFtStyle;
    ListBox maLbStyle;
    SvxFontPrevWindow maWdPreview;

    //color format ui elements
    ListBox maLbColorFormat;
    //color scale ui elements
    ListBox maLbColScale2;
    ListBox maLbColScale3;

    ListBox maLbEntryTypeMin;
    ListBox maLbEntryTypeMiddle;
    ListBox maLbEntryTypeMax;

    Edit maEdMin;
    Edit maEdMiddle;
    Edit maEdMax;

    //data bar ui elements
    //
    //
    void SwitchToType(ScCondFormatEntryType eType);
    void SetCondType();
    void SetColorScaleType();
    void SetDataBarType();
    void HideCondElements();
    void HideColorScaleElements();
    void HideDataBarElements();

    void SetHeight();
    void Init();

    ScDocument* mpDoc;

    DECL_LINK( TypeListHdl, void*);
    DECL_LINK( ColFormatTypeHdl, void*);
    DECL_LINK( StyleSelectHdl, void* );

public:
    ScCondFrmtEntry( Window* pParent, ScDocument* pDoc );
    ScCondFrmtEntry( Window* pParent, ScDocument* pDoc, const ScFormatEntry* pFormatEntry );

    virtual long Notify( NotifyEvent& rNEvt );

    void Select();
    void Deselect();

    bool IsSelected() const;
};

class ScCondFormatList : public Control
{
private:
    typedef boost::ptr_vector<ScCondFrmtEntry> EntryContainer;
    EntryContainer maEntries;

    bool mbHasScrollBar;
    boost::scoped_ptr<ScrollBar> mpScrollBar;
    long mnTopIndex;

    ScDocument* mpDoc;

    void RecalcAll();
    void DoScroll(long nDiff);
public:
    ScCondFormatList( Window* pParent, const ResId& rResId, ScDocument* pDoc );
    ScCondFormatList( Window* pParent, const ResId& rResId, ScDocument* pDoc, ScConditionalFormat* pFormat);

    DECL_LINK( AddBtnHdl, void* );
    DECL_LINK( RemoveBtnHdl, void* );
    DECL_LINK( ScrollHdl, void* );
    DECL_LINK( EntrySelectHdl, ScCondFrmtEntry* );

};

class ScCondFormatDlg : public ModalDialog
{
private:
    PushButton maBtnAdd;
    PushButton maBtnRemove;
    OKButton maBtnOk;
    CancelButton maBtnCancel;

    ScCondFormatList maCondFormList;

    ScDocument* mpDoc;
    ScConditionalFormat* mpFormat;

public:
    ScCondFormatDlg(Window* pWindow, ScDocument* pDoc, ScConditionalFormat* pFormat, const ScRangeList& rRange);

};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
