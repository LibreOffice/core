/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_SOURCE_UI_INC_CONDFORMATDLGENTRY_HXX
#define INCLUDED_SC_SOURCE_UI_INC_CONDFORMATDLGENTRY_HXX

#include "colorscale.hxx"
#include "conditio.hxx"
#include <formula/funcutl.hxx>

#include <svl/lstner.hxx>

class ScIconSetFrmtDataEntry;
class ScCondFormatDlg;

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

    Link<ScCondFrmtEntry&,void> maClickHdl;

    //general ui elements
    VclPtr<FixedText> maFtCondNr;
    VclPtr<FixedText> maFtCondition;

    sal_Int32 mnIndex;
    OUString maStrCondition;
protected:
    VclPtr<ListBox> maLbType;

    ScDocument* mpDoc;
    ScAddress maPos;

    virtual void Select();
    virtual void Deselect();

    virtual OUString GetExpressionString() = 0;

public:
    ScCondFrmtEntry( vcl::Window* pParent, ScDocument* pDoc, const ScAddress& rPos );
    virtual ~ScCondFrmtEntry();
    virtual void dispose() override;

    virtual bool Notify( NotifyEvent& rNEvt ) override;

    bool IsSelected() const { return mbActive;}
    void SetIndex(sal_Int32 nIndex);
    void SetHeight();

    virtual ScFormatEntry* GetEntry() const = 0;
    virtual void SetActive() = 0;
    virtual void SetInactive() = 0;

    virtual condformat::entry::ScCondFrmtEntryType GetType() = 0;
};

class ScConditionFrmtEntry : public ScCondFrmtEntry, public SfxListener
{
    //cond format ui elements
    VclPtr<ListBox> maLbCondType;
    VclPtr<formula::RefEdit> maEdVal1;
    VclPtr<formula::RefEdit> maEdVal2;
    VclPtr<FixedText> maFtVal;
    VclPtr<FixedText> maFtStyle;
    VclPtr<ListBox> maLbStyle;
    VclPtr<SvxFontPrevWindow> maWdPreview;
    bool mbIsInStyleCreate;

    static const sal_Int32 NUM_COND_ENTRIES = 24;
    // Lookup table from positions in maLbCondType to the condition mode enum
    static const ScConditionMode mpEntryToCond[NUM_COND_ENTRIES];

    ScFormatEntry* createConditionEntry() const;

    virtual OUString GetExpressionString() override;
    void Init(ScCondFormatDlg* pDialogParent);
    DECL_LINK_TYPED( StyleSelectHdl, ListBox&, void );
    DECL_LINK_TYPED( ConditionTypeSelectHdl, ListBox&, void );
    DECL_LINK_TYPED( OnEdChanged, Edit&, void );

    // Searches the lookup table for the entry position, given condition mode
    static sal_Int32 ConditionModeToEntryPos( ScConditionMode eMode );
    // Accesses the lookup table for the condition mode, given entry position
    static ScConditionMode EntryPosToConditionMode( sal_Int32 aEntryPos );
    // Returns the number of edit fields used for a given condition mode
    static sal_Int32 GetNumberEditFields( ScConditionMode eMode );

protected:
    virtual void Select() override;
    virtual void Deselect() override;

public:
    ScConditionFrmtEntry( vcl::Window* pParent, ScDocument* pDoc, ScCondFormatDlg* pDialogParent,
            const ScAddress& rPos, const ScCondFormatEntry* pFormatEntry = nullptr );
    virtual ~ScConditionFrmtEntry();
    virtual void dispose() override;

    virtual ScFormatEntry* GetEntry() const override;
    virtual void SetActive() override;
    virtual void SetInactive() override;

    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;
    using ScCondFrmtEntry::Notify;

    virtual condformat::entry::ScCondFrmtEntryType GetType() override { return condformat::entry::CONDITION; }
};

class ScFormulaFrmtEntry : public ScCondFrmtEntry
{
    VclPtr<FixedText> maFtStyle;
    VclPtr<ListBox> maLbStyle;
    VclPtr<SvxFontPrevWindow> maWdPreview;
    VclPtr<formula::RefEdit> maEdFormula;

    ScFormatEntry* createFormulaEntry() const;
    virtual OUString GetExpressionString() override;
    void Init(ScCondFormatDlg* pDialogParent);

    DECL_LINK_TYPED( StyleSelectHdl, ListBox&, void );

public:
    ScFormulaFrmtEntry( vcl::Window* pParent, ScDocument* PDoc, ScCondFormatDlg* pDialogParent, const ScAddress& rPos, const ScCondFormatEntry* pFormatEntry = nullptr );
    virtual ~ScFormulaFrmtEntry();
    virtual void dispose() override;

    virtual ScFormatEntry* GetEntry() const override;
    virtual void SetActive() override;
    virtual void SetInactive() override;
    virtual condformat::entry::ScCondFrmtEntryType GetType() override { return condformat::entry::FORMULA; }
};

class ScColorScale2FrmtEntry : public ScCondFrmtEntry
{

    //color format ui elements
    VclPtr<ListBox> maLbColorFormat;

    //color scale ui elements
    VclPtr<ListBox> maLbEntryTypeMin;
    VclPtr<ListBox> maLbEntryTypeMax;

    VclPtr<Edit> maEdMin;
    VclPtr<Edit> maEdMax;

    VclPtr<ColorListBox> maLbColMin;
    VclPtr<ColorListBox> maLbColMax;

    ScFormatEntry* createColorscaleEntry() const;

    virtual OUString GetExpressionString() override;
    void Init();

    DECL_LINK_TYPED( EntryTypeHdl, ListBox&, void );
public:
    ScColorScale2FrmtEntry( vcl::Window* pParent, ScDocument* pDoc, const ScAddress& rPos, const ScColorScaleFormat* pFormat = nullptr );
    virtual ~ScColorScale2FrmtEntry();
    virtual void dispose() override;
    virtual ScFormatEntry* GetEntry() const override;
    virtual void SetActive() override;
    virtual void SetInactive() override;
    virtual condformat::entry::ScCondFrmtEntryType GetType() override { return condformat::entry::COLORSCALE2; }
};

class ScColorScale3FrmtEntry : public ScCondFrmtEntry
{

    //color format ui elements
    VclPtr<ListBox> maLbColorFormat;

    //color scale ui elements
    VclPtr<ListBox> maLbEntryTypeMin;
    VclPtr<ListBox> maLbEntryTypeMiddle;
    VclPtr<ListBox> maLbEntryTypeMax;

    VclPtr<Edit> maEdMin;
    VclPtr<Edit> maEdMiddle;
    VclPtr<Edit> maEdMax;

    VclPtr<ColorListBox> maLbColMin;
    VclPtr<ColorListBox> maLbColMiddle;
    VclPtr<ColorListBox> maLbColMax;

    ScFormatEntry* createColorscaleEntry() const;

    virtual OUString GetExpressionString() override;
    void Init();

    DECL_LINK_TYPED( EntryTypeHdl, ListBox&, void );
public:
    ScColorScale3FrmtEntry( vcl::Window* pParent, ScDocument* pDoc, const ScAddress& rPos, const ScColorScaleFormat* pFormat = nullptr );
    virtual ~ScColorScale3FrmtEntry();
    virtual void dispose() override;
    virtual ScFormatEntry* GetEntry() const override;
    virtual void SetActive() override;
    virtual void SetInactive() override;
    virtual condformat::entry::ScCondFrmtEntryType GetType() override { return condformat::entry::COLORSCALE3; }
};

class ScDataBarFrmtEntry : public ScCondFrmtEntry
{
    //color format ui elements
    VclPtr<ListBox> maLbColorFormat;

    //data bar ui elements
    VclPtr<ListBox> maLbDataBarMinType;
    VclPtr<ListBox> maLbDataBarMaxType;
    VclPtr<Edit> maEdDataBarMin;
    VclPtr<Edit> maEdDataBarMax;

    VclPtr<PushButton> maBtOptions;

    std::unique_ptr<ScDataBarFormatData> mpDataBarData;

    ScFormatEntry* createDatabarEntry() const;

    virtual OUString GetExpressionString() override;
    void Init();

    DECL_LINK_TYPED( OptionBtnHdl, Button*, void );
    DECL_LINK_TYPED( DataBarTypeSelectHdl, ListBox&, void );
public:
    ScDataBarFrmtEntry( vcl::Window* pParemt, ScDocument* pDoc, const ScAddress& rPos, const ScDataBarFormat* pFormat = nullptr );
    virtual ~ScDataBarFrmtEntry();
    virtual void dispose() override;
    virtual ScFormatEntry* GetEntry() const override;
    virtual void SetActive() override;
    virtual void SetInactive() override;

    virtual condformat::entry::ScCondFrmtEntryType GetType() override { return condformat::entry::DATABAR; }
};

class ScDateFrmtEntry : public ScCondFrmtEntry, public SfxListener
{
public:
    ScDateFrmtEntry( vcl::Window* pParent, ScDocument* pDoc, const ScCondDateFormatEntry* pFormat = nullptr );
    virtual ~ScDateFrmtEntry();
    virtual void dispose() override;
    virtual ScFormatEntry* GetEntry() const override;
    virtual void SetActive() override;
    virtual void SetInactive() override;
    virtual condformat::entry::ScCondFrmtEntryType GetType() override { return condformat::entry::DATE; }

    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;
    using ScCondFrmtEntry::Notify;
protected:
    virtual OUString GetExpressionString() override;

private:
    void Init();

    DECL_LINK_TYPED( StyleSelectHdl, ListBox&, void );

    VclPtr<ListBox> maLbDateEntry;
    VclPtr<FixedText> maFtStyle;
    VclPtr<ListBox> maLbStyle;
    VclPtr<SvxFontPrevWindow> maWdPreview;

    bool mbIsInStyleCreate;
};

class ScIconSetFrmtEntry : public ScCondFrmtEntry
{
    //color format ui elements
    VclPtr<ListBox> maLbColorFormat;

    // icon set ui elements
    VclPtr<ListBox> maLbIconSetType;

    typedef std::vector<VclPtr<ScIconSetFrmtDataEntry>> ScIconSetFrmtDataEntriesType;
    ScIconSetFrmtDataEntriesType maEntries;

    virtual OUString GetExpressionString() override;

    void Init();

    DECL_LINK_TYPED( IconSetTypeHdl, ListBox&, void );

public:
    ScIconSetFrmtEntry( vcl::Window* pParent, ScDocument* pDoc, const ScAddress& rPos, const ScIconSetFormat* pFormat = nullptr );
    virtual ~ScIconSetFrmtEntry();
    virtual void dispose() override;
    virtual ScFormatEntry* GetEntry() const override;
    virtual void SetActive() override;
    virtual void SetInactive() override;
    virtual condformat::entry::ScCondFrmtEntryType GetType() override { return condformat::entry::ICONSET; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
