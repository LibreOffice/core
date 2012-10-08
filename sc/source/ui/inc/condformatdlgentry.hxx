#include "colorscale.hxx"
#include "conditio.hxx"

class ScCondFrmtEntry : public Control
{
private:
    bool mbActive;

    Link maClickHdl;

    //general ui elements
    ListBox maLbType;
    FixedText maFtCondNr;
    FixedText maFtCondition;

    void SetHeight();
    void Init();

    sal_Int32 mnIndex;
    rtl::OUString maStrCondition;
protected:
    ScDocument* mpDoc;
    ScAddress maPos;

    DECL_LINK( EdModifyHdl, Edit* );

public:
    ScCondFrmtEntry( Window* pParent, ScDocument* pDoc, const ScAddress& rPos );
    virtual ~ScCondFrmtEntry();

    virtual long Notify( NotifyEvent& rNEvt );

    void Select();
    void Deselect();

    bool IsSelected() const;
    void SetIndex(sal_Int32 nIndex);

    virtual ScFormatEntry* GetEntry() const = 0;
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
    ScFormatEntry* createFormulaEntry() const;

    void SetHeight();
    void Init();
    DECL_LINK( StyleSelectHdl, void* );
    DECL_LINK( ConditionTypeSelectHdl, void* );
public:
    ScConditionFrmtEntry( Window* pParent, ScDocument* pDoc, const ScAddress& rPos, const ScCondFormatEntry* pFormatEntry = NULL );

    virtual ScFormatEntry* GetEntry() const;
};

class ScColorScaleFrmtEntry : public ScCondFrmtEntry
{

    //color format ui elements
    //ListBox maLbColorFormat;

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

    void SetHeight();
    void Init();

    DECL_LINK( EntryTypeHdl, ListBox* );
public:
    ScColorScaleFrmtEntry( Window* pParent, ScDocument* pDoc, const ScAddress& rPos, const ScColorScaleFormat* pFormat = NULL );
    virtual ScFormatEntry* GetEntry() const;
};

class ScDataBarFrmtEntry : public ScCondFrmtEntry
{
    //color format ui elements
    //ListBox maLbColorFormat;

    //data bar ui elements
    ListBox maLbDataBarMinType;
    ListBox maLbDataBarMaxType;
    Edit maEdDataBarMin;
    Edit maEdDataBarMax;

    PushButton maBtOptions;

    boost::scoped_ptr<ScDataBarFormatData> mpDataBarData;

    ScFormatEntry* createDatabarEntry() const;

    void SetHeight();
    void Init();

    DECL_LINK( OptionBtnHdl, void* );
    DECL_LINK( DataBarTypeSelectHdl, void* );
public:
    ScDataBarFrmtEntry( Window* pParemt, ScDocument* pDoc, const ScAddress& rPos, const ScDataBarFormat* pFormat = NULL );
    virtual ScFormatEntry* GetEntry() const;
};

