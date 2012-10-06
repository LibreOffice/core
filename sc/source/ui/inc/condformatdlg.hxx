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

    ColorListBox maLbColMin;
    ColorListBox maLbColMiddle;
    ColorListBox maLbColMax;

    //data bar ui elements
    ListBox maLbDataBarMinType;
    ListBox maLbDataBarMaxType;
    Edit maEdDataBarMin;
    Edit maEdDataBarMax;
    PushButton maBtOptions;

    boost::scoped_ptr<ScDataBarFormatData> mpDataBarData;

    //
    void SetCondType();
    void SetColorScaleType();
    void SetDataBarType();
    void SetFormulaType();
    void HideCondElements();
    void HideColorScaleElements();
    void HideDataBarElements();
    void SwitchToType(ScCondFormatEntryType eType);

    void SetHeight();
    void Init();

    ScFormatEntry* createConditionEntry() const;
    ScFormatEntry* createColorscaleEntry() const;
    ScFormatEntry* createDatabarEntry() const;
    ScFormatEntry* createFormulaEntry() const;

    ScDocument* mpDoc;
    ScAddress maPos;
    sal_Int32 mnIndex;
    rtl::OUString maStrCondition;

    DECL_LINK( TypeListHdl, void*);
    DECL_LINK( ColFormatTypeHdl, void*);
    DECL_LINK( StyleSelectHdl, void* );
    DECL_LINK( OptionBtnHdl, void* );
    DECL_LINK( DataBarTypeSelectHdl, void* );
    DECL_LINK( ConditionTypeSelectHdl, void* );
    DECL_LINK( EntryTypeHdl, ListBox* );
    DECL_LINK( EdModifyHdl, Edit* );

public:
    ScCondFrmtEntry( Window* pParent, ScDocument* pDoc, const ScAddress& rPos );
    ScCondFrmtEntry( Window* pParent, ScDocument* pDoc, const ScFormatEntry* pFormatEntry, const ScAddress& rPos );
    virtual ~ScCondFrmtEntry();

    virtual long Notify( NotifyEvent& rNEvt );

    void SetType( ScCondFormatEntryType eType );

    void Select();
    void Deselect();

    bool IsSelected() const;
    void SetIndex(sal_Int32 nIndex);

    ScFormatEntry* GetEntry() const;
};

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

};

class ScCondFormatDlg : public ScAnyRefDlg
{
private:
    PushButton maBtnAdd;
    PushButton maBtnRemove;
    OKButton maBtnOk;
    CancelButton maBtnCancel;
    FixedText maFtRange;
    Edit maEdRange;

    ScCondFormatList maCondFormList;

    ScAddress maPos;
    ScDocument* mpDoc;

    DECL_LINK( EdRangeModifyHdl, Edit* );
    DECL_LINK( OkBtnHdl, void* );
    DECL_LINK( CancelBtnHdl, void* );

public:
    ScCondFormatDlg(SfxBindings* pB, SfxChildWindow* pSW, Window* pWindow, ScDocument* pDoc, const ScConditionalFormat* pFormat,
            const ScRangeList& rRange, const ScAddress& rPos, condformat::dialog::ScCondFormatDialogType eType);
    virtual ~ScCondFormatDlg();

    SC_DLLPUBLIC ScConditionalFormat* GetConditionalFormat() const;

    virtual void SetReference(const ScRange&, ScDocument*);
    virtual void SetActive();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
