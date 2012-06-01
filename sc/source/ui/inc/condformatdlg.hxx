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

#include "rangelst.hxx"

#include <boost/ptr_container/ptr_vector.hpp>

class ScDocument;
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
    //color scale ui elements

    //data bar ui elements
    //
    //
    void SwitchToType(ScCondFormatEntryType eType);

public:
    ScCondFrmtEntry( Window* pParent );

    long GetHeight() const;
    void Select();
};

class ScCondFormatList : public Control
{
private:
    typedef boost::ptr_vector<ScCondFrmtEntry> EntryContainer;
    EntryContainer maEntries;

    bool mbHasScrollBar;
    ScrollBar* mpScrollBar;
    long mnTopIndex;

    void RecalcAll();
public:
    ScCondFormatList( Window* pParent, const ResId& rResId );

    DECL_LINK( AddBtnHdl, void* );

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
