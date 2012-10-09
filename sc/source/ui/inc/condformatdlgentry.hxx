#include "colorscale.hxx"
#include "conditio.hxx"

namespace condformat {

namespace entry {

enum ScCondFrmtEntryType
{
    CONDITION,
    FORMULA,
    COLORSCALE2,
    COLORSCALE3,
    DATABAR
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

    void Init();

    sal_Int32 mnIndex;
    rtl::OUString maStrCondition;
protected:
    ListBox maLbType;

    ScDocument* mpDoc;
    ScAddress maPos;

    DECL_LINK( EdModifyHdl, Edit* );

    void Select();
    void Deselect();

public:
    ScCondFrmtEntry( Window* pParent, ScDocument* pDoc, const ScAddress& rPos );
    virtual ~ScCondFrmtEntry();

    virtual long Notify( NotifyEvent& rNEvt );

    bool IsSelected() const;
    void SetIndex(sal_Int32 nIndex);
    sal_Int32 GetIndex() const { return mnIndex; }
    void SetHeight();

    virtual ScFormatEntry* GetEntry() const = 0;
    virtual void SetActive() = 0;
    virtual void SetInactive() = 0;

    virtual condformat::entry::ScCondFrmtEntryType GetType() = 0;
};

class ScConditionFrmtEntry : public ScCondFrmtEntry
{

    //cond format ui elements
    ListBox maLbCondType;
    Edit maEdVal1;
    Edit maEdVal2;
    FixedText maFtStyle;
    ListBox maLbStyle;
    SvxFontPrevWindow maWdPreview;

    ScFormatEntry* createConditionEntry() const;

    void Init();
    DECL_LINK( StyleSelectHdl, void* );
    DECL_LINK( ConditionTypeSelectHdl, void* );
public:
    ScConditionFrmtEntry( Window* pParent, ScDocument* pDoc, const ScAddress& rPos, const ScCondFormatEntry* pFormatEntry = NULL );

    virtual ScFormatEntry* GetEntry() const;
    virtual void SetActive();
    virtual void SetInactive();

    virtual condformat::entry::ScCondFrmtEntryType GetType() { return condformat::entry::CONDITION; }
};

class ScFormulaFrmtEntry : public ScCondFrmtEntry
{
    FixedText maFtStyle;
    ListBox maLbStyle;
    SvxFontPrevWindow maWdPreview;
    Edit maEdFormula;

    ScFormatEntry* createFormulaEntry() const;
    void Init();

    DECL_LINK( StyleSelectHdl, void* );
public:
    ScFormulaFrmtEntry( Window* pParent, ScDocument* PDoc, const ScAddress& rPos, const ScCondFormatEntry* pFormatEntry = NULL );

    virtual ScFormatEntry* GetEntry() const;
    virtual void SetActive();
    virtual void SetInactive();
    virtual condformat::entry::ScCondFrmtEntryType GetType() { return condformat::entry::FORMULA; }
};

class ScColorScale2FrmtEntry : public ScCondFrmtEntry
{

    //color format ui elements
    ListBox maLbColorFormat;

    //color scale ui elements
    ListBox maLbColScale2;

    ListBox maLbEntryTypeMin;
    ListBox maLbEntryTypeMax;

    Edit maEdMin;
    Edit maEdMax;

    ColorListBox maLbColMin;
    ColorListBox maLbColMax;

    ScFormatEntry* createColorscaleEntry() const;

    void Init();

    DECL_LINK( EntryTypeHdl, ListBox* );
public:
    ScColorScale2FrmtEntry( Window* pParent, ScDocument* pDoc, const ScAddress& rPos, const ScColorScaleFormat* pFormat = NULL );
    virtual ScFormatEntry* GetEntry() const;
    virtual void SetActive();
    virtual void SetInactive();
    virtual condformat::entry::ScCondFrmtEntryType GetType() { return condformat::entry::COLORSCALE2; }
};

class ScColorScale3FrmtEntry : public ScCondFrmtEntry
{

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

    ScFormatEntry* createColorscaleEntry() const;

    void Init();

    DECL_LINK( EntryTypeHdl, ListBox* );
public:
    ScColorScale3FrmtEntry( Window* pParent, ScDocument* pDoc, const ScAddress& rPos, const ScColorScaleFormat* pFormat = NULL );
    virtual ScFormatEntry* GetEntry() const;
    virtual void SetActive();
    virtual void SetInactive();
    virtual condformat::entry::ScCondFrmtEntryType GetType() { return condformat::entry::COLORSCALE3; }
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

    void Init();

    DECL_LINK( OptionBtnHdl, void* );
    DECL_LINK( DataBarTypeSelectHdl, void* );
public:
    ScDataBarFrmtEntry( Window* pParemt, ScDocument* pDoc, const ScAddress& rPos, const ScDataBarFormat* pFormat = NULL );
    virtual ScFormatEntry* GetEntry() const;
    virtual void SetActive();
    virtual void SetInactive();

    virtual condformat::entry::ScCondFrmtEntryType GetType() { return condformat::entry::DATABAR; }
};

