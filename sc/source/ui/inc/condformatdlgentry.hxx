/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <conditio.hxx>
#include <formula/funcutl.hxx>
#include <vcl/weld.hxx>
#include <svl/lstner.hxx>
#include <svx/fntctrl.hxx>

class ScIconSetFrmtDataEntry;
class ScCondFormatDlg;
class ScCondFormatList;
class ColorListBox;
class ScColorScaleFormat;
class ScDataBarFormat;
class ScIconSetFormat;
struct ScDataBarFormatData;

namespace condformat::entry {

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

class ScCondFrmtEntry
{
protected:
    ScCondFormatList* mpParent;
    std::unique_ptr<weld::Builder> mxBuilder;

private:
    //general ui elements
    std::unique_ptr<weld::Widget> mxBorder;
    std::unique_ptr<weld::Container> mxGrid;
    std::unique_ptr<weld::Label> mxFtCondNr;
    std::unique_ptr<weld::Label> mxFtCondition;

    bool mbActive;
    OUString const maStrCondition;
    Link<ScCondFrmtEntry&,void> maClickHdl;

    DECL_LINK( EntrySelectHdl, const MouseEvent&, bool );

protected:
    std::unique_ptr<weld::ComboBox> mxLbType;

    ScDocument* mpDoc;
    ScAddress maPos;

    virtual void Select();
    virtual void Deselect();

    virtual OUString GetExpressionString() = 0;

public:
    ScCondFrmtEntry(ScCondFormatList* pParent, ScDocument* pDoc, const ScAddress& rPos);
    virtual ~ScCondFrmtEntry();

    void Show() { mxGrid->show(); }

    void set_grid_top_attach(int nAttach) { mxBorder->set_grid_top_attach(nAttach); }
    int get_preferred_height() const { return mxBorder->get_preferred_size().Height(); }

    void SetPos(const ScAddress& rPos) { maPos = rPos; };
    bool IsSelected() const { return mbActive;}
    void SetIndex(sal_Int32 nIndex);

    virtual ScFormatEntry* GetEntry() const = 0;
    virtual void SetActive() = 0;
    virtual void SetInactive() = 0;

    virtual condformat::entry::ScCondFrmtEntryType GetType() = 0;
};

class ScConditionFrmtEntry : public ScCondFrmtEntry, public SfxListener
{
    //cond format ui elements
    SvxFontPrevWindow maWdPreview;
    std::unique_ptr<weld::ComboBox> mxLbCondType;
    std::unique_ptr<formula::RefEdit> mxEdVal1;
    std::unique_ptr<formula::RefEdit> mxEdVal2;
    std::unique_ptr<weld::Label> mxFtVal;
    std::unique_ptr<weld::Label> mxFtStyle;
    std::unique_ptr<weld::ComboBox> mxLbStyle;
    std::unique_ptr<weld::Widget> mxWdPreviewWin;
    std::unique_ptr<weld::CustomWeld> mxWdPreview;
    bool mbIsInStyleCreate;

    static const sal_Int32 NUM_COND_ENTRIES = 24;
    // Lookup table from positions in maLbCondType to the condition mode enum
    static const ScConditionMode mpEntryToCond[NUM_COND_ENTRIES];

    ScFormatEntry* createConditionEntry() const;

    virtual OUString GetExpressionString() override;
    void Init(ScCondFormatDlg* pDialogParent);
    DECL_LINK( StyleSelectHdl, weld::ComboBox&, void );
    DECL_LINK( ConditionTypeSelectHdl, weld::ComboBox&, void );
    DECL_LINK( OnEdChanged, formula::RefEdit&, void );

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
    ScConditionFrmtEntry(ScCondFormatList* pParent, ScDocument* pDoc, ScCondFormatDlg* pDialogParent,
            const ScAddress& rPos, const ScCondFormatEntry* pFormatEntry = nullptr);
    virtual ~ScConditionFrmtEntry() override;

    virtual ScFormatEntry* GetEntry() const override;
    virtual void SetActive() override;
    virtual void SetInactive() override;

    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    virtual condformat::entry::ScCondFrmtEntryType GetType() override { return condformat::entry::CONDITION; }
};

class ScFormulaFrmtEntry : public ScCondFrmtEntry
{
    SvxFontPrevWindow maWdPreview;
    std::unique_ptr<weld::Label> mxFtStyle;
    std::unique_ptr<weld::ComboBox> mxLbStyle;
    std::unique_ptr<weld::Widget> mxWdPreviewWin;
    std::unique_ptr<weld::CustomWeld> mxWdPreview;
    std::unique_ptr<formula::RefEdit> mxEdFormula;

    ScFormatEntry* createFormulaEntry() const;
    virtual OUString GetExpressionString() override;
    void Init(ScCondFormatDlg* pDialogParent);

    DECL_LINK(StyleSelectHdl, weld::ComboBox&, void);

public:
    ScFormulaFrmtEntry(ScCondFormatList* pParent, ScDocument* PDoc, ScCondFormatDlg* pDialogParent, const ScAddress& rPos, const ScCondFormatEntry* pFormatEntry = nullptr);
    virtual ~ScFormulaFrmtEntry() override;

    virtual ScFormatEntry* GetEntry() const override;
    virtual void SetActive() override;
    virtual void SetInactive() override;
    virtual condformat::entry::ScCondFrmtEntryType GetType() override { return condformat::entry::FORMULA; }
};

class ScColorScale2FrmtEntry : public ScCondFrmtEntry
{

    //color format ui elements
    std::unique_ptr<weld::ComboBox> mxLbColorFormat;

    //color scale ui elements
    std::unique_ptr<weld::ComboBox> mxLbEntryTypeMin;
    std::unique_ptr<weld::ComboBox> mxLbEntryTypeMax;

    std::unique_ptr<weld::Entry> mxEdMin;
    std::unique_ptr<weld::Entry> mxEdMax;

    std::unique_ptr<ColorListBox> mxLbColMin;
    std::unique_ptr<ColorListBox> mxLbColMax;

    std::unique_ptr<weld::Label> mxFtMin;
    std::unique_ptr<weld::Label> mxFtMax;

    ScFormatEntry* createColorscaleEntry() const;

    virtual OUString GetExpressionString() override;
    void Init();

    DECL_LINK( EntryTypeHdl, weld::ComboBox&, void );
public:
    ScColorScale2FrmtEntry(ScCondFormatList* pParent, ScDocument* pDoc, const ScAddress& rPos, const ScColorScaleFormat* pFormat = nullptr);
    virtual ~ScColorScale2FrmtEntry() override;
    virtual ScFormatEntry* GetEntry() const override;
    virtual void SetActive() override;
    virtual void SetInactive() override;
    virtual condformat::entry::ScCondFrmtEntryType GetType() override { return condformat::entry::COLORSCALE2; }
};

class ScColorScale3FrmtEntry : public ScCondFrmtEntry
{

    //color format ui elements
    std::unique_ptr<weld::ComboBox> mxLbColorFormat;

    //color scale ui elements
    std::unique_ptr<weld::ComboBox> mxLbEntryTypeMin;
    std::unique_ptr<weld::ComboBox> mxLbEntryTypeMiddle;
    std::unique_ptr<weld::ComboBox> mxLbEntryTypeMax;

    std::unique_ptr<weld::Entry> mxEdMin;
    std::unique_ptr<weld::Entry> mxEdMiddle;
    std::unique_ptr<weld::Entry> mxEdMax;

    std::unique_ptr<ColorListBox> mxLbColMin;
    std::unique_ptr<ColorListBox> mxLbColMiddle;
    std::unique_ptr<ColorListBox> mxLbColMax;

    std::unique_ptr<weld::Label> mxFtMin;
    std::unique_ptr<weld::Label> mxFtMax;

    ScFormatEntry* createColorscaleEntry() const;

    virtual OUString GetExpressionString() override;
    void Init();

    DECL_LINK( EntryTypeHdl, weld::ComboBox&, void );
public:
    ScColorScale3FrmtEntry(ScCondFormatList* pParent, ScDocument* pDoc, const ScAddress& rPos, const ScColorScaleFormat* pFormat = nullptr);
    virtual ~ScColorScale3FrmtEntry() override;
    virtual ScFormatEntry* GetEntry() const override;
    virtual void SetActive() override;
    virtual void SetInactive() override;
    virtual condformat::entry::ScCondFrmtEntryType GetType() override { return condformat::entry::COLORSCALE3; }
};

class ScDataBarFrmtEntry : public ScCondFrmtEntry
{
    //color format ui elements
    std::unique_ptr<weld::ComboBox> mxLbColorFormat;

    //data bar ui elements
    std::unique_ptr<weld::ComboBox> mxLbDataBarMinType;
    std::unique_ptr<weld::ComboBox> mxLbDataBarMaxType;
    std::unique_ptr<weld::Entry> mxEdDataBarMin;
    std::unique_ptr<weld::Entry> mxEdDataBarMax;
    std::unique_ptr<weld::Button> mxBtOptions;

    std::unique_ptr<weld::Label> mxFtMin;
    std::unique_ptr<weld::Label> mxFtMax;

    std::unique_ptr<ScDataBarFormatData> mpDataBarData;

    ScFormatEntry* createDatabarEntry() const;

    virtual OUString GetExpressionString() override;
    void Init();

    DECL_LINK( OptionBtnHdl, weld::Button&, void );
    DECL_LINK( DataBarTypeSelectHdl, weld::ComboBox&, void );
public:
    ScDataBarFrmtEntry(ScCondFormatList* pParemt, ScDocument* pDoc, const ScAddress& rPos, const ScDataBarFormat* pFormat = nullptr);
    virtual ~ScDataBarFrmtEntry() override;
    virtual ScFormatEntry* GetEntry() const override;
    virtual void SetActive() override;
    virtual void SetInactive() override;

    virtual condformat::entry::ScCondFrmtEntryType GetType() override { return condformat::entry::DATABAR; }
};

class ScDateFrmtEntry : public ScCondFrmtEntry, public SfxListener
{
public:
    ScDateFrmtEntry(ScCondFormatList* pParent, ScDocument* pDoc, const ScCondDateFormatEntry* pFormat = nullptr);
    virtual ~ScDateFrmtEntry() override;
    virtual ScFormatEntry* GetEntry() const override;
    virtual void SetActive() override;
    virtual void SetInactive() override;
    virtual condformat::entry::ScCondFrmtEntryType GetType() override { return condformat::entry::DATE; }

    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

protected:
    virtual OUString GetExpressionString() override;

private:
    void Init();

    DECL_LINK( StyleSelectHdl, weld::ComboBox&, void );

    SvxFontPrevWindow maWdPreview;
    std::unique_ptr<weld::ComboBox> mxLbDateEntry;
    std::unique_ptr<weld::Label> mxFtStyle;
    std::unique_ptr<weld::ComboBox> mxLbStyle;
    std::unique_ptr<weld::Widget> mxWdPreviewWin;
    std::unique_ptr<weld::CustomWeld> mxWdPreview;

    bool mbIsInStyleCreate;
};

class ScIconSetFrmtEntry : public ScCondFrmtEntry
{
    //color format ui elements
    std::unique_ptr<weld::ComboBox> mxLbColorFormat;

    // icon set ui elements
    std::unique_ptr<weld::ComboBox> mxLbIconSetType;

    std::unique_ptr<weld::Container> mxIconParent;

    typedef std::vector<std::unique_ptr<ScIconSetFrmtDataEntry>> ScIconSetFrmtDataEntriesType;
    ScIconSetFrmtDataEntriesType maEntries;

    virtual OUString GetExpressionString() override;

    void Init();

    DECL_LINK(IconSetTypeHdl, weld::ComboBox&, void);

public:
    ScIconSetFrmtEntry(ScCondFormatList* pParent, ScDocument* pDoc, const ScAddress& rPos, const ScIconSetFormat* pFormat = nullptr);
    virtual ~ScIconSetFrmtEntry() override;
    virtual ScFormatEntry* GetEntry() const override;
    virtual void SetActive() override;
    virtual void SetInactive() override;
    virtual condformat::entry::ScCondFrmtEntryType GetType() override { return condformat::entry::ICONSET; }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
