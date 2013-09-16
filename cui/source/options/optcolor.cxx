/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <svtools/colorcfg.hxx>
#include <svtools/extcolorcfg.hxx>
#include <svtools/headbar.hxx>
#include <svtools/ctrlbox.hxx>
#include <vcl/scrbar.hxx>
#include <svx/xtable.hxx>
#include <unotools/moduleoptions.hxx>
#include <unotools/pathoptions.hxx>
#include <vcl/msgbox.hxx>
#include <boost/shared_ptr.hpp>
#include <svx/svxdlg.hxx>
#include <helpid.hrc>
#include <dialmgr.hxx>
#include "optcolor.hxx"
#include <cuires.hrc>
#include <svx/dlgutil.hxx>

using namespace ::com::sun::star;
using namespace ::svtools;

namespace
{

// list of default groups
enum Group
{
    Group_Unknown = -1,

    Group_General,
    Group_Writer,
    Group_Html,
    Group_Calc,
    Group_Draw,
    Group_Basic,
    Group_Sql,

    nGroupCount
};

// group data
struct
{
    // group
    Group eGroup;
    // .ui group name
    const char *pGroup;
}
const vGroupInfo[] =
{
    // the groups are in the same order as in enum Group above
    { Group_General, "general" },
    { Group_Writer, "writer" },
    { Group_Html, "html" },
    { Group_Calc, "calc" },
    { Group_Draw, "draw" },
    { Group_Basic, "basic" },
    { Group_Sql, "sql" }
};

// color config entry data (see ColorConfigWindow_Impl::Entry below)
struct
{
    // group
    Group eGroup;
    //checkbox (or simple text)
    const char *pText;
    //color listbox
    const char *pColor;
    //preview box
    const char *pPreview;
    // has checkbox?
    bool bCheckBox;
}
const vEntryInfo[] =
{
    #define IDS(Name) \
        SAL_STRINGIFY(Name), SAL_STRINGIFY(Name##_lb), SAL_STRINGIFY(Name##_wn), false

    #define IDS_CB(Name) \
        SAL_STRINGIFY(Name), SAL_STRINGIFY(Name##_lb), SAL_STRINGIFY(Name##_wn), true

    // The list of these entries (enum ColorConfigEntry) are in colorcfg.hxx.

    { Group_General, IDS(doccolor) },
    { Group_General, IDS_CB(docboundaries) },
    { Group_General, IDS(appback) },
    { Group_General, IDS_CB(objboundaries) },
    { Group_General, IDS_CB(tblboundaries) },
    { Group_General, IDS(font) },
    { Group_General, IDS_CB(unvisitedlinks) },
    { Group_General, IDS_CB(visitedlinks) },
    { Group_General, IDS(autospellcheck) },
    { Group_General, IDS(smarttags) },
    { Group_General, IDS_CB(shadows) },

    { Group_Writer,  IDS(writergrid) },
    { Group_Writer,  IDS_CB(field) },
    { Group_Writer,  IDS_CB(index) },
    { Group_Writer,  IDS(direct) },
    { Group_Writer,  IDS(script) },
    { Group_Writer,  IDS_CB(section) },
    { Group_Writer,  IDS(hdft) },
    { Group_Writer,  IDS(pagebreak) },

    { Group_Html,    IDS(sgml) },
    { Group_Html,    IDS(htmlcomment) },
    { Group_Html,    IDS(htmlkeyword) },
    { Group_Html,    IDS(unknown) },

    { Group_Calc,    IDS(calcgrid) },
    { Group_Calc,    IDS(brk) },
    { Group_Calc,    IDS(brkmanual) },
    { Group_Calc,    IDS(brkauto) },
    { Group_Calc,    IDS(det) },
    { Group_Calc,    IDS(deterror) },
    { Group_Calc,    IDS(ref) },
    { Group_Calc,    IDS(notes) },

    { Group_Draw,    IDS(drawgrid) },

    { Group_Basic,   IDS(basicid) },
    { Group_Basic,   IDS(basiccomment) },
    { Group_Basic,   IDS(basicnumber) },
    { Group_Basic,   IDS(basicstring) },
    { Group_Basic,   IDS(basicop) },
    { Group_Basic,   IDS(basickeyword) },
    { Group_Basic,   IDS(error) },

    { Group_Sql,     IDS(sqlid) },
    { Group_Sql,     IDS(sqlnumber) },
    { Group_Sql,     IDS(sqlstring) },
    { Group_Sql,     IDS(sqlop) },
    { Group_Sql,     IDS(sqlkeyword) },
    { Group_Sql,     IDS(sqlparam) },
    { Group_Sql,     IDS(sqlcomment) }

    #undef IDS
};

} // namespace

//
// ColorConfigWindow_Impl
//

class ColorConfigWindow_Impl
    : public VclContainer
    , public VclBuilderContainer
{
public:
    ColorConfigWindow_Impl(Window* pParent);
    ~ColorConfigWindow_Impl();

public:
    void SetLinks (Link const&, Link const&, Link const&);
    unsigned GetEntryHeight () const { return vEntries[0]->GetHeight(); }
    void Update (EditableColorConfig const*, EditableExtendedColorConfig const*);
    void ScrollHdl(const ScrollBar&);
    void ClickHdl (EditableColorConfig*, CheckBox*);
    void ColorHdl (EditableColorConfig*, EditableExtendedColorConfig*, ColorListBox*);
    void Init(ScrollBar *pVScroll, HeaderBar *m_pHeaderHB);
    void AdjustScrollBar();
    void AdjustHeaderBar();

private:
    // Chapter -- horizontal group separator stripe with text
    class Chapter
    {
        // text
        bool m_bOwnsWidget;
        FixedText *m_pText;
    public:
        Chapter(FixedText *pText, bool bShow);
        Chapter(Window *pGrid, unsigned nYPos, const OUString& sDisplayName);
        ~Chapter();
    public:
        void SetBackground(const Wallpaper& W) { m_pText->SetBackground(W); }
        long GetHeight() const { return m_pText->GetSizePixel().Height(); }
        long GetLeft() const { return m_pText->GetPosPixel().X(); }
        void Show(const Wallpaper& rBackWall);
        void Hide();
    };

    // Entry -- a color config entry:
    // text (checkbox) + color list box + preview box
    class Entry
    {
    public:
        Entry(ColorConfigWindow_Impl& rParent, unsigned iEntry, long nCheckBoxLabelOffset, bool bShow);
        Entry(Window* pGrid, unsigned nYPos, const ExtendedColorConfigValue& aColorEntry,
            long nCheckBoxLabelOffset);
        ~Entry();
    public:
        void Show ();
        void Hide ();
        void SetAppearance(Wallpaper const& aTextWall, ColorListBox const& aSampleList);
        void SetTextColor (Color C) { m_pText->SetTextColor(C); }
    public:
        void SetLinks (Link const&, Link const&, Link const&);
        void Update (ColorConfigEntry, ColorConfigValue const&);
        void Update (ExtendedColorConfigValue const&);
        void ColorChanged (ColorConfigEntry, ColorConfigValue&);
        void ColorChanged (ExtendedColorConfigValue&);
    public:
        long GetTop () const { return m_pPreview->GetPosPixel().Y(); }
        long GetBottom () const { return GetTop() + m_pPreview->GetSizePixel().Height(); }
        unsigned GetHeight () const { return m_pColorList->GetSizePixel().Height(); }
    public:
        bool Is (CheckBox* pBox) const { return m_pText == pBox; }
        bool Is (ColorListBox* pBox) const { return m_pColorList == pBox; }

    private:
        bool m_bOwnsWidgets;
        // checkbox (CheckBox) or simple text (FixedText)
        Control* m_pText;
        // color list box
        ColorListBox* m_pColorList;
        // color preview box
        Window* m_pPreview;
        // default color
        Color m_aDefaultColor;
    private:
        void SetColor (Color);
    };

    // vChapters -- groups (group headers)
    std::vector<boost::shared_ptr<Chapter> > vChapters;
    // vEntries -- color options
    std::vector<boost::shared_ptr<Entry> > vEntries;

    // module options
    SvtModuleOptions aModuleOptions;


private:
    VclGrid *m_pGrid;
    ScrollBar *m_pVScroll;
    HeaderBar *m_pHeaderHB;

    // initialization
    void CreateEntries();
    void SetAppearance();

private:
    virtual void Command (CommandEvent const& rCEvt);
    virtual void DataChanged (DataChangedEvent const& rDCEvt);

    virtual Size calculateRequisition() const;
    virtual void setAllocation(const Size &rAllocation);

    bool IsGroupVisible (Group) const;
};

//
// ColorConfigWindow_Impl::Chapter
//

// ctor for default groups
// rParent: parent window (ColorConfigWindow_Impl)
// eGroup: which group is this?
// rResMgr: resource manager
ColorConfigWindow_Impl::Chapter::Chapter(FixedText* pText, bool bShow)
    : m_bOwnsWidget(false)
    , m_pText(pText)
{
    if (!bShow)
        Hide();
}

// ctor for extended groups
ColorConfigWindow_Impl::Chapter::Chapter(Window *pGrid,
    unsigned nYPos, const OUString& rDisplayName)
    : m_bOwnsWidget(true)
{
    m_pText = new FixedText(pGrid, WB_LEFT|WB_VCENTER|WB_3DLOOK);
    m_pText->set_font_attribute("weight", "bold");
    m_pText->set_grid_width(3);
    m_pText->set_grid_left_attach(0);
    m_pText->set_grid_top_attach(nYPos);
    m_pText->SetText(rDisplayName);
}

ColorConfigWindow_Impl::Chapter::~Chapter()
{
    if (m_bOwnsWidget)
        delete m_pText;
}

void ColorConfigWindow_Impl::Chapter::Show(Wallpaper const& rBackWall)
{
    // background
    m_pText->SetBackground(rBackWall);
    m_pText->Show();
}

void ColorConfigWindow_Impl::Chapter::Hide ()
{
    m_pText->Hide();
}

//
// ColorConfigWindow_Impl::Entry
//

ColorConfigWindow_Impl::Entry::Entry(ColorConfigWindow_Impl& rParent, unsigned iEntry,
    long nCheckBoxLabelOffset, bool bShow)
    : m_bOwnsWidgets(false)
    , m_aDefaultColor(ColorConfig::GetDefaultColor(static_cast<ColorConfigEntry>(iEntry)))
{
    rParent.get(m_pText, vEntryInfo[iEntry].pText);
    if (!vEntryInfo[iEntry].bCheckBox)
    {
        m_pText->set_margin_left(m_pText->get_margin_left() +
            nCheckBoxLabelOffset);
    }
    rParent.get(m_pColorList, vEntryInfo[iEntry].pColor);
    rParent.get(m_pPreview, vEntryInfo[iEntry].pPreview);

    if (!bShow)
        Hide();
}

// ctor for extended entries
ColorConfigWindow_Impl::Entry::Entry( Window *pGrid, unsigned nYPos,
    ExtendedColorConfigValue const& rColorEntry, long nCheckBoxLabelOffset)
    : m_bOwnsWidgets(true)
    , m_aDefaultColor(rColorEntry.getDefaultColor())
{
    m_pText = new FixedText(pGrid, WB_LEFT|WB_VCENTER|WB_3DLOOK);
    m_pText->set_grid_width(3);
    m_pText->set_grid_left_attach(0);
    m_pText->set_grid_top_attach(nYPos);
    m_pText->set_margin_left(6 + nCheckBoxLabelOffset);
    m_pText->SetText(rColorEntry.getDisplayName());

    WinBits nWinBits = WB_LEFT|WB_VCENTER|WB_3DLOOK|WB_TABSTOP|WB_DROPDOWN;
    m_pColorList = new ColorListBox(pGrid, nWinBits);
    m_pColorList->EnableAutoSize(true);
    m_pColorList->set_grid_left_attach(1);
    m_pColorList->set_grid_top_attach(nYPos);

    m_pPreview = new Window(pGrid, WB_BORDER);
    m_pPreview->set_grid_left_attach(2);
    m_pPreview->set_grid_top_attach(nYPos);
    m_pPreview->set_margin_right(6);

    Show();
}

ColorConfigWindow_Impl::Entry::~Entry()
{
    if (m_bOwnsWidgets)
    {
        delete m_pText;
        delete m_pColorList;
        delete m_pPreview;
    }
}

void ColorConfigWindow_Impl::Entry::Show()
{
    m_pText->Show();
    m_pColorList->Show();
    m_pPreview->Show();
}

void ColorConfigWindow_Impl::Entry::Hide()
{
    m_pText->Hide();
    m_pColorList->Hide();
    m_pPreview->Hide();
}

// SetAppearance()
// iEntry: which entry is this?
// aTextWall: background of the text (transparent)
// aSampleList: sample color listbox (to copy from)
void ColorConfigWindow_Impl::Entry::SetAppearance(
    Wallpaper const& aTextWall,
    ColorListBox const& aSampleList)
{
    // text (and optionally checkbox)
    m_pText->SetBackground(aTextWall);
    // preview
    m_pPreview->SetBorderStyle(WINDOW_BORDER_MONO);
    // color list
    m_pColorList->CopyEntries(aSampleList);
    m_pColorList->InsertAutomaticEntryColor(m_aDefaultColor);
}

// SetLinks()
void ColorConfigWindow_Impl::Entry::SetLinks(
    Link const& aCheckLink, Link const& aColorLink,
    Link const& aGetFocusLink)
{
    m_pColorList->SetSelectHdl(aColorLink);
    m_pColorList->SetGetFocusHdl(aGetFocusLink);
    if (CheckBox* pCheckBox = dynamic_cast<CheckBox*>(m_pText))
    {
        pCheckBox->SetClickHdl(aCheckLink);
        pCheckBox->SetGetFocusHdl(aGetFocusLink);
    }
}

// updates a default color config entry
void ColorConfigWindow_Impl::Entry::Update (
    ColorConfigEntry aColorEntry, ColorConfigValue const& rValue
) {
    Color aColor;
    if ((unsigned)rValue.nColor == COL_AUTO)
    {
        aColor = ColorConfig::GetDefaultColor(aColorEntry);
        m_pColorList->SelectEntryPos(0);
    }
    else
    {
        aColor = Color(rValue.nColor);
        m_pColorList->SelectEntry(aColor);
    }
    m_pPreview->SetBackground(Wallpaper(aColor));
    if (CheckBox* pCheckBox = dynamic_cast<CheckBox*>(m_pText))
        pCheckBox->Check(rValue.bIsVisible);
}

// updates an extended color config entry
void ColorConfigWindow_Impl::Entry::Update (
    ExtendedColorConfigValue const& rValue
) {
    Color aColor(rValue.getColor());
    if (rValue.getColor() == rValue.getDefaultColor())
        m_pColorList->SelectEntryPos(0);
    else
        m_pColorList->SelectEntry(aColor);
    SetColor(aColor);
}

// color of a default entry has changed
void ColorConfigWindow_Impl::Entry::ColorChanged (
    ColorConfigEntry aColorEntry,
    ColorConfigValue& rValue
) {
    Color aColor;
    if (m_pColorList->IsAutomaticSelected())
    {
        aColor = ColorConfig::GetDefaultColor(aColorEntry);
        rValue.nColor = COL_AUTO;
    }
    else
    {
        aColor = m_pColorList->GetSelectEntryColor();
        rValue.nColor = aColor.GetColor();
    }
    SetColor(aColor);
}

// color of an extended entry has changed
void ColorConfigWindow_Impl::Entry::ColorChanged (
    ExtendedColorConfigValue& rValue
) {
    Color aColor = m_pColorList->GetSelectEntryColor();
    rValue.setColor(aColor.GetColor());
    // automatic?
    if (m_pColorList->GetSelectEntryPos() == 0)
    {
        rValue.setColor(rValue.getDefaultColor());
        aColor.SetColor(rValue.getColor());
    }
    SetColor(aColor);
}

void ColorConfigWindow_Impl::Entry::SetColor (Color aColor)
{
    m_pPreview->SetBackground(Wallpaper(aColor));
    m_pPreview->Invalidate();
}


//
// ColorConfigWindow_Impl
//

ColorConfigWindow_Impl::ColorConfigWindow_Impl(Window* pParent)
    : VclContainer(pParent)
{
    m_pUIBuilder = new VclBuilder(this, getUIRootDir(), "cui/ui/colorconfigwin.ui");
    get(m_pGrid, "ColorConfigWindow");
    CreateEntries();
    SetAppearance();
}

Size ColorConfigWindow_Impl::calculateRequisition() const
{
    return getLayoutRequisition(*m_pGrid);
}

void ColorConfigWindow_Impl::setAllocation(const Size &rAllocation)
{
    Point aChildPos(0, 0);
    Size aChildSize(getLayoutRequisition(*m_pGrid));
    aChildSize.Width() = rAllocation.Width();
    setLayoutPosSize(*m_pGrid, aChildPos, aChildSize);
    AdjustScrollBar();
    AdjustHeaderBar();
    ScrollHdl(*m_pVScroll);
}

void ColorConfigWindow_Impl::CreateEntries()
{
    std::vector<int> aModulesInstalled;
    // creating group headers
    vChapters.reserve(nGroupCount);
    aModulesInstalled.reserve(nGroupCount);
    for (unsigned i = 0; i != nGroupCount; ++i)
    {
        aModulesInstalled.push_back(IsGroupVisible(vGroupInfo[i].eGroup));
        vChapters.push_back(boost::shared_ptr<Chapter>(
            new Chapter(get<FixedText>(vGroupInfo[i].pGroup), aModulesInstalled[i])));
    }

    //Here we want to get the amount to add to the position
    //of a FixedText to get it to align its contents
    //with that of a CheckBox
    //We should have something like a Control::getTextOrigin
    //Ideally we could use something like GetCharacterBounds,
    //but I think that only works on truly visible controls
    long nCheckBoxLabelOffset = 0;
    {
        OUString sSampleText("X");
        CheckBox aCheckBox(this);
        FixedText aFixedText(this);
        aCheckBox.SetText(sSampleText);
        aFixedText.SetText(sSampleText);
        Size aCheckSize(aCheckBox.CalcMinimumSize(0x7fffffff));
        Size aFixedSize(aFixedText.CalcMinimumSize(0x7fffffff));
        nCheckBoxLabelOffset = aCheckSize.Width() - aFixedSize.Width();
    }

    // creating entries
    vEntries.reserve(ColorConfigEntryCount);
    for (unsigned i = 0; i < SAL_N_ELEMENTS(vEntryInfo); ++i)
    {
        vEntries.push_back(boost::shared_ptr<Entry>(new Entry(*this, i, nCheckBoxLabelOffset,
            aModulesInstalled[vEntryInfo[i].eGroup])));
    }

    // extended entries
    ExtendedColorConfig aExtConfig;
    if (unsigned const nExtGroupCount = aExtConfig.GetComponentCount())
    {
        size_t nLineNum = vChapters.size() + vEntries.size() + 1;
        for (unsigned j = 0; j != nExtGroupCount; ++j)
        {
            OUString const sComponentName = aExtConfig.GetComponentName(j);
            vChapters.push_back(boost::shared_ptr<Chapter>(new Chapter(
                m_pGrid, nLineNum,
                aExtConfig.GetComponentDisplayName(sComponentName)
            )));
            ++nLineNum;
            unsigned nColorCount = aExtConfig.GetComponentColorCount(sComponentName);
            for (unsigned i = 0; i != nColorCount; ++i)
            {
                ExtendedColorConfigValue const aColorEntry =
                    aExtConfig.GetComponentColorConfigValue(sComponentName, i);
                vEntries.push_back(boost::shared_ptr<Entry>( new Entry (
                    m_pGrid, nLineNum, aColorEntry, nCheckBoxLabelOffset
                )));
                ++nLineNum;
            }
        }
    }
}

void ColorConfigWindow_Impl::SetAppearance ()
{
    Color TempColor(COL_TRANSPARENT);
    Wallpaper const aTransparentWall(TempColor);
    StyleSettings const& rStyleSettings = GetSettings().GetStyleSettings();
    Color const aBackColor = rStyleSettings.GetHighContrastMode() ?
        rStyleSettings.GetShadowColor() : Color(COL_LIGHTGRAY);
    Wallpaper const aBackWall(aBackColor);
    for (size_t i = 0; i != vChapters.size(); ++i)
        vChapters[i]->Show(aBackWall);
    SetBackground(Wallpaper(rStyleSettings.GetFieldColor()));

    // #104195# when the window color is the same as the text color it has to be changed
    Color aWinCol = rStyleSettings.GetWindowColor();
    Color aRCheckCol = rStyleSettings.GetRadioCheckTextColor();
    if (aWinCol == aRCheckCol)
    {
        aRCheckCol.Invert();
        // if inversion didn't work (gray) then it's set to black
        if (aRCheckCol == aWinCol)
            aRCheckCol = Color(COL_BLACK);
        // setting new text color for each entry
        for (size_t i = 0; i != vEntries.size(); ++i)
            vEntries[i]->SetTextColor(aRCheckCol);
    }

    OSL_ENSURE( vEntries.size() >= sizeof vEntryInfo / sizeof vEntryInfo[0], "wrong number of helpIDs for color listboxes" );

    // creating a sample color listbox with the color entries
    ColorListBox aSampleColorList(this);
    {
        XColorListRef const xColorTable = XColorList::CreateStdColorList();
        for (sal_Int32 i = 0; i != xColorTable->Count(); ++i)
        {
            XColorEntry& rEntry = *xColorTable->GetColor(i);
            aSampleColorList.InsertEntry(rEntry.GetColor(), rEntry.GetName());
        }
    }

    // appearance
    for (size_t i = 0; i != vEntries.size(); ++i)
    {
        // appearance
        vEntries[i]->SetAppearance(aTransparentWall, aSampleColorList);
    }
}

ColorConfigWindow_Impl::~ColorConfigWindow_Impl ()
{ }

void ColorConfigWindow_Impl::AdjustHeaderBar()
{
    // horizontal positions
    unsigned const nX0 = 0;
    unsigned const nX1 = get<Window>("doccolor")->GetPosPixel().X();
    unsigned const nX2 = get<Window>("doccolor_lb")->GetPosPixel().X();
    unsigned const nX3 = get<Window>("doccolor_wn")->GetPosPixel().X();
    unsigned const nX4 = m_pHeaderHB->GetSizePixel().Width();
    m_pHeaderHB->SetItemSize(1, nX1 - nX0);
    m_pHeaderHB->SetItemSize(2, nX2 - nX1);
    m_pHeaderHB->SetItemSize(3, nX3 - nX2);
    m_pHeaderHB->SetItemSize(4, nX4 - nX3);
}

void ColorConfigWindow_Impl::AdjustScrollBar()
{
    unsigned const nScrollOffset =
        vEntries[1]->GetTop() - vEntries[0]->GetTop();
    unsigned const nVisibleEntries = GetSizePixel().Height() / nScrollOffset;
    m_pVScroll->SetPageSize(nVisibleEntries - 1);
    m_pVScroll->SetVisibleSize(nVisibleEntries);
}

void ColorConfigWindow_Impl::Init(ScrollBar *pVScroll, HeaderBar *pHeaderHB)
{
    m_pHeaderHB = pHeaderHB;
    m_pVScroll = pVScroll;
    m_pVScroll->EnableDrag();
    m_pVScroll->SetRangeMin(0);
    m_pVScroll->SetRangeMax(vEntries.size() + vChapters.size());
}

// SetLinks()
void ColorConfigWindow_Impl::SetLinks (
    Link const& aCheckLink, Link const& aColorLink, Link const& aGetFocusLink
) {
    for (unsigned i = 0; i != vEntries.size(); ++i)
        vEntries[i]->SetLinks(aCheckLink, aColorLink, aGetFocusLink);
}

// Update()
void ColorConfigWindow_Impl::Update (
    EditableColorConfig const* pConfig,
    EditableExtendedColorConfig const* pExtConfig)
{
    // updating default entries
    for (unsigned i = 0; i != ColorConfigEntryCount; ++i)
    {
        ColorConfigEntry const aColorEntry = static_cast<ColorConfigEntry>(i);
        vEntries[i]->Update(
            aColorEntry, pConfig->GetColorValue(aColorEntry)
        );
    }

    // updating extended entries
    unsigned i = ColorConfigEntryCount;
    unsigned const nExtCount = pExtConfig->GetComponentCount();
    for (unsigned j = 0; j != nExtCount; ++j)
    {
        OUString sComponentName = pExtConfig->GetComponentName(j);
        unsigned const nColorCount = pExtConfig->GetComponentColorCount(sComponentName);
        for (unsigned k = 0; i != vEntries.size() && k != nColorCount; ++i, ++k)
            vEntries[i]->Update(
                pExtConfig->GetComponentColorConfigValue(sComponentName, k)
            );
    }
}

// ScrollHdl()
void ColorConfigWindow_Impl::ScrollHdl(const ScrollBar& rVScroll)
{
    SetUpdateMode(true);
    const long nRowHeight = (vEntries[1]->GetTop() - vEntries[0]->GetTop());
    Point aPos(0, 0 - rVScroll.GetThumbPos() * nRowHeight);
    m_pGrid->SetPosPixel(aPos);
    SetUpdateMode(true);
}

// ClickHdl()
void ColorConfigWindow_Impl::ClickHdl (EditableColorConfig* pConfig, CheckBox* pBox)
{
    for (unsigned i = 0; i != ColorConfigEntryCount; ++i)
    {
        if (vEntries[i]->Is(pBox))
        {
            ColorConfigEntry const aEntry = static_cast<ColorConfigEntry>(i);
            ColorConfigValue aValue = pConfig->GetColorValue(aEntry);
            aValue.bIsVisible = pBox->IsChecked();
            pConfig->SetColorValue(aEntry, aValue);
            break;
        }
    }
}

// ColorHdl()
void ColorConfigWindow_Impl::ColorHdl (
    EditableColorConfig* pConfig, EditableExtendedColorConfig* pExtConfig,
    ColorListBox* pBox
) {
    unsigned i = 0;

    // default entries
    for ( ; i != ColorConfigEntryCount; ++i)
    {
        if (pBox && vEntries[i]->Is(pBox))
        {
            ColorConfigEntry const aColorEntry = static_cast<ColorConfigEntry>(i);
            ColorConfigValue aValue = pConfig->GetColorValue(aColorEntry);
            vEntries[i]->ColorChanged(aColorEntry, aValue);
            pConfig->SetColorValue(aColorEntry, aValue);
            break;
        }
    }

    // extended entries
    unsigned const nExtCount = pExtConfig->GetComponentCount();
    i = ColorConfigEntryCount;
    for (unsigned j = 0; j != nExtCount; ++j)
    {
        OUString sComponentName = pExtConfig->GetComponentName(j);
        unsigned const nColorCount = pExtConfig->GetComponentColorCount(sComponentName);
        unsigned const nCount = vEntries.size();
        for (unsigned k = 0; i != nCount && k != nColorCount; ++i, ++k)
        {
            if (pBox && vEntries[i]->Is(pBox))
            {
                ExtendedColorConfigValue aValue =
                    pExtConfig->GetComponentColorConfigValue(sComponentName, k);
                vEntries[i]->ColorChanged(aValue);
                pExtConfig->SetColorValue(sComponentName, aValue);
                break;
            }
        }
    }
}


// IsGroupVisible()
bool ColorConfigWindow_Impl::IsGroupVisible (Group eGroup) const
{
    switch (eGroup)
    {
        case Group_Writer:
        case Group_Html:
            return aModuleOptions.IsModuleInstalled(SvtModuleOptions::E_SWRITER);
        case Group_Calc:
            return aModuleOptions.IsModuleInstalled(SvtModuleOptions::E_SCALC);
        case Group_Draw:
            return
                aModuleOptions.IsModuleInstalled(SvtModuleOptions::E_SDRAW) ||
                aModuleOptions.IsModuleInstalled(SvtModuleOptions::E_SIMPRESS);
        case Group_Sql:
            return aModuleOptions.IsModuleInstalled(SvtModuleOptions::E_SDATABASE);
        default:
            return true;
    }
}

void ColorConfigWindow_Impl::DataChanged (DataChangedEvent const& rDCEvt)
{
    Window::DataChanged( rDCEvt );
    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
         (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        StyleSettings const& rStyleSettings = GetSettings().GetStyleSettings();
        bool const bHighContrast = rStyleSettings.GetHighContrastMode();
        Wallpaper const aBackWall(Color(bHighContrast ? COL_TRANSPARENT : COL_LIGHTGRAY));
        for (unsigned i = 0; i != vChapters.size(); ++i)
            vChapters[i]->SetBackground(aBackWall);
        SetBackground(Wallpaper(rStyleSettings.GetWindowColor()));
    }
}


void ColorConfigWindow_Impl::Command( const CommandEvent& rCEvt )
{
    GetParent()->Command(rCEvt);
}

class ColorConfigCtrl_Impl : public VclVBox
{
    HeaderBar*              m_pHeaderHB;
    VclHBox*                m_pBody;
    ColorConfigWindow_Impl* m_pScrollWindow;
    ScrollBar*              m_pVScroll;

    EditableColorConfig*            pColorConfig;
    EditableExtendedColorConfig*    pExtColorConfig;

    DECL_LINK(ScrollHdl, ScrollBar*);
    DECL_LINK(ClickHdl, CheckBox*);
    DECL_LINK(ColorHdl, ColorListBox*);
    DECL_LINK(ControlFocusHdl, Control*);

    virtual long PreNotify (NotifyEvent& rNEvt);
    virtual void Command (CommandEvent const& rCEvt);
    virtual void DataChanged (DataChangedEvent const& rDCEvt);
public:
    ColorConfigCtrl_Impl(Window* pParent);
    ~ColorConfigCtrl_Impl();

    void InitHeaderBar(const OUString &rOn, const OUString &rUIElems,
        const OUString &rColorSetting, const OUString &rPreview);
    void SetConfig (EditableColorConfig& rConfig) { pColorConfig = &rConfig; }
    void SetExtendedConfig (EditableExtendedColorConfig& rConfig) { pExtColorConfig = &rConfig; }
    void Update ();
    long GetScrollPosition ()
    {
        return m_pVScroll->GetThumbPos();
    }
    void SetScrollPosition (long nSet)
    {
        m_pVScroll->SetThumbPos(nSet);
        ScrollHdl(m_pVScroll);
    }
};

ColorConfigCtrl_Impl::ColorConfigCtrl_Impl(Window* pParent)
    : VclVBox(pParent)
    , pColorConfig(0)
    , pExtColorConfig(0)
{
    m_pHeaderHB = new HeaderBar(this, WB_BUTTONSTYLE | WB_BOTTOMBORDER);

    m_pBody = new VclHBox(this);
    m_pScrollWindow = new ColorConfigWindow_Impl(m_pBody);
    m_pVScroll = new ScrollBar(m_pBody, WB_VERT);
    m_pScrollWindow->Init(m_pVScroll, m_pHeaderHB);

    m_pBody->set_hexpand(true);
    m_pBody->set_vexpand(true);
    m_pBody->set_expand(true);
    m_pBody->set_fill(true);

    m_pScrollWindow->set_hexpand(true);
    m_pScrollWindow->set_vexpand(true);
    m_pScrollWindow->set_expand(true);
    m_pScrollWindow->set_fill(true);

    Link aScrollLink = LINK(this, ColorConfigCtrl_Impl, ScrollHdl);
    m_pVScroll->SetScrollHdl(aScrollLink);
    m_pVScroll->SetEndScrollHdl(aScrollLink);

    Link aCheckLink = LINK(this, ColorConfigCtrl_Impl, ClickHdl);
    Link aColorLink = LINK(this, ColorConfigCtrl_Impl, ColorHdl);
    Link aGetFocusLink = LINK(this, ColorConfigCtrl_Impl, ControlFocusHdl);
    m_pScrollWindow->SetLinks(aCheckLink, aColorLink, aGetFocusLink);

    m_pHeaderHB->Show();
    m_pVScroll->Show();
    m_pBody->Show();
    m_pScrollWindow->Show();
}

void ColorConfigCtrl_Impl::InitHeaderBar(const OUString &rOn, const OUString &rUIElems,
    const OUString &rColorSetting, const OUString &rPreview)
{
    // filling
    const WinBits nHeadBits = HIB_VCENTER | HIB_FIXED | HIB_FIXEDPOS;
    m_pHeaderHB->InsertItem(1, rOn, 0, nHeadBits | HIB_CENTER);
    m_pHeaderHB->InsertItem(2, rUIElems, 0, nHeadBits | HIB_LEFT);
    m_pHeaderHB->InsertItem(3, rColorSetting, 0, nHeadBits | HIB_LEFT);
    m_pHeaderHB->InsertItem(4, rPreview, 0, nHeadBits | HIB_LEFT);
    m_pHeaderHB->set_height_request(GetTextHeight() + 6);
}

ColorConfigCtrl_Impl::~ColorConfigCtrl_Impl()
{
    delete m_pVScroll;
    delete m_pScrollWindow;
    delete m_pBody;
    delete m_pHeaderHB;
}

extern "C" SAL_DLLPUBLIC_EXPORT Window* SAL_CALL makeColorConfigCtrl(Window *pParent, VclBuilder::stringmap &)
{
    return new ColorConfigCtrl_Impl(pParent);
}

void ColorConfigCtrl_Impl::Update ()
{
    DBG_ASSERT(pColorConfig, "Configuration not set");
    m_pScrollWindow->Update(pColorConfig, pExtColorConfig);
}

IMPL_LINK(ColorConfigCtrl_Impl, ScrollHdl, ScrollBar*, pScrollBar)
{
    m_pScrollWindow->ScrollHdl(*pScrollBar);
    return 0;
}

long ColorConfigCtrl_Impl::PreNotify( NotifyEvent& rNEvt )
{
    if(rNEvt.GetType() == EVENT_COMMAND)
    {
        const CommandEvent* pCEvt = rNEvt.GetCommandEvent();
        sal_uInt16 nCmd = pCEvt->GetCommand();
        if( COMMAND_WHEEL == nCmd )
        {
            Command(*pCEvt);
            return 1;
        }
    }
    return VclVBox::PreNotify(rNEvt);
}

void ColorConfigCtrl_Impl::Command( const CommandEvent& rCEvt )
{
    switch ( rCEvt.GetCommand() )
    {

        case COMMAND_WHEEL:
        case COMMAND_STARTAUTOSCROLL:
        case COMMAND_AUTOSCROLL:
        {
            const CommandWheelData* pWheelData = rCEvt.GetWheelData();
            if(pWheelData && !pWheelData->IsHorz() && COMMAND_WHEEL_ZOOM != pWheelData->GetMode())
            {
                HandleScrollCommand(rCEvt, 0, m_pVScroll);
            }
        }
        break;
        default:
            VclVBox::Command(rCEvt);
    }
}

void ColorConfigCtrl_Impl::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged( rDCEvt );
    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
         (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
        SetBackground(Wallpaper(rStyleSettings.GetFieldColor()));
    }
}

IMPL_LINK(ColorConfigCtrl_Impl, ClickHdl, CheckBox*, pBox)
{
    DBG_ASSERT(pColorConfig, "Configuration not set");
    m_pScrollWindow->ClickHdl(pColorConfig, pBox);
    return 0;
}

// a color list has changed
IMPL_LINK(ColorConfigCtrl_Impl, ColorHdl, ColorListBox*, pBox)
{
    DBG_ASSERT(pColorConfig, "Configuration not set" );
    if (pBox)
        m_pScrollWindow->ColorHdl(pColorConfig, pExtColorConfig, pBox);
    return 0;
}
IMPL_LINK(ColorConfigCtrl_Impl, ControlFocusHdl, Control*, pCtrl)
{
    // determine whether a control is completely visible
    // and make it visible
    long aCtrlPosY = pCtrl->GetPosPixel().Y();
    unsigned const nWinHeight = m_pScrollWindow->GetSizePixel().Height();
    unsigned const nEntryHeight = m_pScrollWindow->GetEntryHeight();
    if (0 != (GETFOCUS_TAB & pCtrl->GetGetFocusFlags()) &&
        (aCtrlPosY < 0 || nWinHeight < aCtrlPosY + nEntryHeight)
    ) {
        long nThumbPos = m_pVScroll->GetThumbPos();
        if (nWinHeight < aCtrlPosY + nEntryHeight)
        {
            //scroll down
            nThumbPos += 2;
        }
        else
        {
            //scroll up
            nThumbPos -= 2;
            if(nThumbPos < 0)
                nThumbPos = 0;
        }
        m_pVScroll->SetThumbPos(nThumbPos);
        ScrollHdl(m_pVScroll);
    }
    return 0;
};


//
// SvxColorOptionsTabPage
//

SvxColorOptionsTabPage::SvxColorOptionsTabPage(
    Window* pParent, const SfxItemSet& rCoreSet)
    : SfxTabPage(pParent, "OptAppearancePage", "cui/ui/optappearancepage.ui", rCoreSet)
    , bFillItemSetCalled(false)
    , pColorConfig(0)
    , pExtColorConfig(0)
{
    get(m_pColorSchemeLB, "colorschemelb");
    m_pColorSchemeLB->SetStyle(m_pColorSchemeLB->GetStyle() | WB_SORT);
    get(m_pSaveSchemePB, "save");
    get(m_pDeleteSchemePB, "delete");
    get(m_pColorConfigCT, "colorconfig");

    m_pColorConfigCT->InitHeaderBar(
        get<Window>("on")->GetText(),
        get<Window>("uielements")->GetText(),
        get<Window>("colorsetting")->GetText(),
        get<Window>("preview")->GetText());

    m_pColorSchemeLB->SetSelectHdl(LINK(this, SvxColorOptionsTabPage, SchemeChangedHdl_Impl));
    Link aLk = LINK(this, SvxColorOptionsTabPage, SaveDeleteHdl_Impl );
    m_pSaveSchemePB->SetClickHdl(aLk);
    m_pDeleteSchemePB->SetClickHdl(aLk);
}

SvxColorOptionsTabPage::~SvxColorOptionsTabPage()
{
    //when the dialog is cancelled but the color scheme ListBox has been changed these
    //changes need to be undone
    if(!bFillItemSetCalled && m_pColorSchemeLB->GetSavedValue() != m_pColorSchemeLB->GetSelectEntryPos())
    {
        OUString sOldScheme =  m_pColorSchemeLB->GetEntry(m_pColorSchemeLB->GetSavedValue());
        if(!sOldScheme.isEmpty())
        {
            pColorConfig->SetCurrentSchemeName(sOldScheme);
            pExtColorConfig->SetCurrentSchemeName(sOldScheme);
        }
    }
    pColorConfig->ClearModified();
    pColorConfig->EnableBroadcast();
    delete pColorConfig;
    pExtColorConfig->ClearModified();
    pExtColorConfig->EnableBroadcast();
    delete pExtColorConfig;
}

SfxTabPage* SvxColorOptionsTabPage::Create( Window* pParent, const SfxItemSet& rAttrSet )
{
    return ( new SvxColorOptionsTabPage( pParent, rAttrSet ) );
}

sal_Bool SvxColorOptionsTabPage::FillItemSet( SfxItemSet&  )
{
    bFillItemSetCalled = sal_True;
    if(m_pColorSchemeLB->GetSavedValue() != m_pColorSchemeLB->GetSelectEntryPos())
    {
        pColorConfig->SetModified();
        pExtColorConfig->SetModified();
    }
    if(pColorConfig->IsModified())
        pColorConfig->Commit();
    if(pExtColorConfig->IsModified())
        pExtColorConfig->Commit();
    return sal_True;
}

void SvxColorOptionsTabPage::Reset( const SfxItemSet& )
{
    if(pColorConfig)
    {
        pColorConfig->ClearModified();
        pColorConfig->DisableBroadcast();
        delete pColorConfig;
    }
    pColorConfig = new EditableColorConfig;
    m_pColorConfigCT->SetConfig(*pColorConfig);

    if(pExtColorConfig)
    {
        pExtColorConfig->ClearModified();
        pExtColorConfig->DisableBroadcast();
        delete pExtColorConfig;
    }
    pExtColorConfig = new EditableExtendedColorConfig;
    m_pColorConfigCT->SetExtendedConfig(*pExtColorConfig);

    String sUser = GetUserData();
    //has to be called always to speed up accessibility tools
    m_pColorConfigCT->SetScrollPosition(sUser.ToInt32());
    m_pColorSchemeLB->Clear();
    uno::Sequence< OUString >  aSchemes = pColorConfig->GetSchemeNames();
    const OUString* pSchemes = aSchemes.getConstArray();
    for(sal_Int32 i = 0; i < aSchemes.getLength(); i++)
        m_pColorSchemeLB->InsertEntry(pSchemes[i]);
    m_pColorSchemeLB->SelectEntry(pColorConfig->GetCurrentSchemeName());
    m_pColorSchemeLB->SaveValue();
    m_pDeleteSchemePB->Enable( aSchemes.getLength() > 1 );
    UpdateColorConfig();
}

int SvxColorOptionsTabPage::DeactivatePage( SfxItemSet* pSet_ )
{
    if ( pSet_ )
        FillItemSet( *pSet_ );
    return( LEAVE_PAGE );
}

void SvxColorOptionsTabPage::UpdateColorConfig()
{
    //update the color config control
    m_pColorConfigCT->Update();
}

IMPL_LINK(SvxColorOptionsTabPage, SchemeChangedHdl_Impl, ListBox*, pBox)
{
    pColorConfig->LoadScheme(pBox->GetSelectEntry());
    pExtColorConfig->LoadScheme(pBox->GetSelectEntry());
    UpdateColorConfig();
    return 0;
}

IMPL_LINK(SvxColorOptionsTabPage, SaveDeleteHdl_Impl, PushButton*, pButton )
{
    if (m_pSaveSchemePB == pButton)
    {
        OUString sName;

        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        DBG_ASSERT(pFact, "Dialogdiet fail!");
        AbstractSvxNameDialog* aNameDlg = pFact->CreateSvxNameDialog( pButton,
                            sName, String(CUI_RES(RID_SVXSTR_COLOR_CONFIG_SAVE2)) );
        DBG_ASSERT(aNameDlg, "Dialogdiet fail!");
        aNameDlg->SetCheckNameHdl( LINK(this, SvxColorOptionsTabPage, CheckNameHdl_Impl));
        aNameDlg->SetText(String(CUI_RES(RID_SVXSTR_COLOR_CONFIG_SAVE1)));
        aNameDlg->SetHelpId(HID_OPTIONS_COLORCONFIG_SAVE_SCHEME);
        aNameDlg->SetCheckNameHdl( LINK(this, SvxColorOptionsTabPage, CheckNameHdl_Impl));
        if(RET_OK == aNameDlg->Execute())
        {
            aNameDlg->GetName(sName);
            pColorConfig->AddScheme(sName);
            pExtColorConfig->AddScheme(sName);
            m_pColorSchemeLB->InsertEntry(sName);
            m_pColorSchemeLB->SelectEntry(sName);
            m_pColorSchemeLB->GetSelectHdl().Call(m_pColorSchemeLB);
        }
        delete aNameDlg;
    }
    else
    {
        DBG_ASSERT(m_pColorSchemeLB->GetEntryCount() > 1, "don't delete the last scheme");
        QueryBox aQuery(pButton, CUI_RES(RID_SVXQB_DELETE_COLOR_CONFIG));
        aQuery.SetText(String(CUI_RES(RID_SVXSTR_COLOR_CONFIG_DELETE)));
        if(RET_YES == aQuery.Execute())
        {
            OUString sDeleteScheme(m_pColorSchemeLB->GetSelectEntry());
            m_pColorSchemeLB->RemoveEntry(m_pColorSchemeLB->GetSelectEntryPos());
            m_pColorSchemeLB->SelectEntryPos(0);
            m_pColorSchemeLB->GetSelectHdl().Call(m_pColorSchemeLB);
            //first select the new scheme and then delete the old one
            pColorConfig->DeleteScheme(sDeleteScheme);
            pExtColorConfig->DeleteScheme(sDeleteScheme);
        }
    }
    m_pDeleteSchemePB->Enable( m_pColorSchemeLB->GetEntryCount() > 1 );
    return 0;
}

IMPL_LINK(SvxColorOptionsTabPage, CheckNameHdl_Impl, AbstractSvxNameDialog*, pDialog )
{
    OUString sName;
    pDialog->GetName(sName);
    return !sName.isEmpty() && LISTBOX_ENTRY_NOTFOUND == m_pColorSchemeLB->GetEntryPos( sName );
}

void SvxColorOptionsTabPage::FillUserData()
{
    SetUserData(OUString::number(m_pColorConfigCT->GetScrollPosition()));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
