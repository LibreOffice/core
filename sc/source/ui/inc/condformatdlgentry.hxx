/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SC_CONDFORMATDLGENTRY_HXX
#define SC_CONDFORMATDLGENTRY_HXX

#include "colorscale.hxx"
#include "conditio.hxx"
#include <formula/funcutl.hxx>

#include <svl/lstner.hxx>

class ScIconSetFrmtDataEntry;

namespace condformat {

namespace entry {

enum ScCondFrmtEntryType
{
    CONDITION,
    FORMULA,
    COLORSCALE2,
    COLORSCALE3,
    DATABAR,
    ICONSET,
    DATE
};

}

}

class ScCondFrmtEntry : public Control
{
private:
    bool mbActive;

    Link maClickHdl;

    //general ui elements
    FixedText maFtCondNr;
    FixedText maFtCondition;

    sal_Int32 mnIndex;
    OUString maStrCondition;
protected:
    ListBox maLbType;

    ScDocument* mpDoc;
    ScAddress maPos;

    DECL_LINK( EdModifyHdl, Edit* );

    void Select();
    void Deselect();

    virtual OUString GetExpressionString() = 0;

public:
    ScCondFrmtEntry( Window* pParent, ScDocument* pDoc, const ScAddress& rPos );
    virtual ~ScCondFrmtEntry();

    virtual bool Notify( NotifyEvent& rNEvt ) SAL_OVERRIDE;

    bool IsSelected() const;
    void SetIndex(sal_Int32 nIndex);
    sal_Int32 GetIndex() const { return mnIndex; }
    void SetHeight();

    virtual ScFormatEntry* GetEntry() const = 0;
    virtual void SetActive() = 0;
    virtual void SetInactive() = 0;

    virtual condformat::entry::ScCondFrmtEntryType GetType() = 0;
};

class ScConditionFrmtEntry : public ScCondFrmtEntry, public SfxListener
{
    //cond format ui elements
    ListBox maLbCondType;
    formula::RefEdit maEdVal1;
    formula::RefEdit maEdVal2;
    FixedText maFtStyle;
    ListBox maLbStyle;
    SvxFontPrevWindow maWdPreview;
    bool mbIsInStyleCreate;

    ScFormatEntry* createConditionEntry() const;

    virtual OUString GetExpressionString() SAL_OVERRIDE;
    void Init();
    DECL_LINK( StyleSelectHdl, void* );
    DECL_LINK( ConditionTypeSelectHdl, void* );

public:
    ScConditionFrmtEntry( Window* pParent, ScDocument* pDoc, const ScAddress& rPos, const ScCondFormatEntry* pFormatEntry = NULL );

    virtual ScFormatEntry* GetEntry() const SAL_OVERRIDE;
    virtual void SetActive() SAL_OVERRIDE;
    virtual void SetInactive() SAL_OVERRIDE;

    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) SAL_OVERRIDE;
    using ScCondFrmtEntry::Notify;

    virtual condformat::entry::ScCondFrmtEntryType GetType() SAL_OVERRIDE { return condformat::entry::CONDITION; }
};

class ScFormulaFrmtEntry : public ScCondFrmtEntry
{
    FixedText maFtStyle;
    ListBox maLbStyle;
    SvxFontPrevWindow maWdPreview;
    formula::RefEdit maEdFormula;

    ScFormatEntry* createFormulaEntry() const;
    virtual OUString GetExpressionString() SAL_OVERRIDE;
    void Init();

    DECL_LINK( StyleSelectHdl, void* );

public:
    ScFormulaFrmtEntry( Window* pParent, ScDocument* PDoc, const ScAddress& rPos, const ScCondFormatEntry* pFormatEntry = NULL );

    virtual ScFormatEntry* GetEntry() const SAL_OVERRIDE;
    virtual void SetActive() SAL_OVERRIDE;
    virtual void SetInactive() SAL_OVERRIDE;
    virtual condformat::entry::ScCondFrmtEntryType GetType() SAL_OVERRIDE { return condformat::entry::FORMULA; }
};

class ScColorScale2FrmtEntry : public ScCondFrmtEntry
{

    //color format ui elements
    ListBox maLbColorFormat;

    //color scale ui elements
    ListBox maLbEntryTypeMin;
    ListBox maLbEntryTypeMax;

    Edit maEdMin;
    Edit maEdMax;

    ColorListBox maLbColMin;
    ColorListBox maLbColMax;

    ScFormatEntry* createColorscaleEntry() const;

    virtual OUString GetExpressionString() SAL_OVERRIDE;
    void Init();

    DECL_LINK( EntryTypeHdl, ListBox* );
public:
    ScColorScale2FrmtEntry( Window* pParent, ScDocument* pDoc, const ScAddress& rPos, const ScColorScaleFormat* pFormat = NULL );
    virtual ScFormatEntry* GetEntry() const SAL_OVERRIDE;
    virtual void SetActive() SAL_OVERRIDE;
    virtual void SetInactive() SAL_OVERRIDE;
    virtual condformat::entry::ScCondFrmtEntryType GetType() SAL_OVERRIDE { return condformat::entry::COLORSCALE2; }
};

class ScColorScale3FrmtEntry : public ScCondFrmtEntry
{

    //color format ui elements
    ListBox maLbColorFormat;

    //color scale ui elements
    ListBox maLbEntryTypeMin;
    ListBox maLbEntryTypeMiddle;
    ListBox maLbEntryTypeMax;

    Edit maEdMin;
    Edit maEdMiddle;
    Edit maEdMax;

    ColorListBox maLbColMin;
    ColorListBox maLbColMiddle;
    ColorListBox maLbColMax;

    ScFormatEntry* createColorscaleEntry() const;

    virtual OUString GetExpressionString() SAL_OVERRIDE;
    void Init();

    DECL_LINK( EntryTypeHdl, ListBox* );
public:
    ScColorScale3FrmtEntry( Window* pParent, ScDocument* pDoc, const ScAddress& rPos, const ScColorScaleFormat* pFormat = NULL );
    virtual ScFormatEntry* GetEntry() const SAL_OVERRIDE;
    virtual void SetActive() SAL_OVERRIDE;
    virtual void SetInactive() SAL_OVERRIDE;
    virtual condformat::entry::ScCondFrmtEntryType GetType() SAL_OVERRIDE { return condformat::entry::COLORSCALE3; }
};

class ScDataBarFrmtEntry : public ScCondFrmtEntry
{
    //color format ui elements
    ListBox maLbColorFormat;

    //data bar ui elements
    ListBox maLbDataBarMinType;
    ListBox maLbDataBarMaxType;
    Edit maEdDataBarMin;
    Edit maEdDataBarMax;

    PushButton maBtOptions;

    boost::scoped_ptr<ScDataBarFormatData> mpDataBarData;

    ScFormatEntry* createDatabarEntry() const;

    virtual OUString GetExpressionString() SAL_OVERRIDE;
    void Init();

    DECL_LINK( OptionBtnHdl, void* );
    DECL_LINK( DataBarTypeSelectHdl, void* );
public:
    ScDataBarFrmtEntry( Window* pParemt, ScDocument* pDoc, const ScAddress& rPos, const ScDataBarFormat* pFormat = NULL );
    virtual ScFormatEntry* GetEntry() const SAL_OVERRIDE;
    virtual void SetActive() SAL_OVERRIDE;
    virtual void SetInactive() SAL_OVERRIDE;

    virtual condformat::entry::ScCondFrmtEntryType GetType() SAL_OVERRIDE { return condformat::entry::DATABAR; }
};

class ScDateFrmtEntry : public ScCondFrmtEntry, public SfxListener
{
public:
    ScDateFrmtEntry( Window* pParent, ScDocument* pDoc, const ScCondDateFormatEntry* pFormat = NULL );
    virtual ScFormatEntry* GetEntry() const SAL_OVERRIDE;
    virtual void SetActive() SAL_OVERRIDE;
    virtual void SetInactive() SAL_OVERRIDE;
    virtual condformat::entry::ScCondFrmtEntryType GetType() SAL_OVERRIDE { return condformat::entry::DATE; }

    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) SAL_OVERRIDE;
    using ScCondFrmtEntry::Notify;
protected:
    virtual OUString GetExpressionString() SAL_OVERRIDE;

private:
    void Init();

    DECL_LINK( StyleSelectHdl, void* );

    ListBox maLbDateEntry;
    FixedText maFtStyle;
    ListBox maLbStyle;
    SvxFontPrevWindow maWdPreview;

    bool mbIsInStyleCreate;
};

class ScIconSetFrmtEntry : public ScCondFrmtEntry
{
    //color format ui elements
    ListBox maLbColorFormat;

    // icon set ui elements
    ListBox maLbIconSetType;

    typedef boost::ptr_vector<ScIconSetFrmtDataEntry> ScIconSetFrmtDataEntriesType;
    ScIconSetFrmtDataEntriesType maEntries;

    virtual OUString GetExpressionString() SAL_OVERRIDE;

    void Init();

    DECL_LINK( IconSetTypeHdl, void* );

public:
    ScIconSetFrmtEntry( Window* pParent, ScDocument* pDoc, const ScAddress& rPos, const ScIconSetFormat* pFormat = NULL );
    virtual ScFormatEntry* GetEntry() const SAL_OVERRIDE;
    virtual void SetActive() SAL_OVERRIDE;
    virtual void SetInactive() SAL_OVERRIDE;
    virtual condformat::entry::ScCondFrmtEntryType GetType() SAL_OVERRIDE { return condformat::entry::ICONSET; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
