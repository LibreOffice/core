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

#include <sal/config.h>

#include <bitset>

#include <editeng/editids.hrc>
#include <svtools/colorcfg.hxx>
#include <svtools/extcolorcfg.hxx>
#include <svtools/headbar.hxx>
#include <svtools/ctrlbox.hxx>
#include <vcl/scrbar.hxx>
#include <svx/colorbox.hxx>
#include <svx/xtable.hxx>
#include <unotools/moduleoptions.hxx>
#include <unotools/pathoptions.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/settings.hxx>
#include <vcl/builderfactory.hxx>
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


// ColorConfigWindow_Impl


class ColorConfigWindow_Impl
    : public VclContainer
    , public VclBuilderContainer
{
public:
    explicit ColorConfigWindow_Impl(vcl::Window* pParent);
    virtual ~ColorConfigWindow_Impl() override { disposeOnce(); }
    virtual void dispose() override;

public:
    void SetLinks (Link<Button*,void> const&, Link<SvxColorListBox&,void> const&, Link<Control&,void> const&);
    unsigned GetEntryHeight () const { return vEntries[0]->GetHeight(); }
    void Update (EditableColorConfig const*, EditableExtendedColorConfig const*);
    void ScrollHdl(const ScrollBar&);
    void ClickHdl (EditableColorConfig*, CheckBox*);
    void ColorHdl (EditableColorConfig*, EditableExtendedColorConfig*, SvxColorListBox*);
    void Init(ScrollBar *pVScroll, HeaderBar *m_pHeaderHB);
    void AdjustScrollBar();
    void AdjustHeaderBar();

private:
    // Chapter -- horizontal group separator stripe with text
    class Chapter
    {
        // text
        VclPtr<FixedText> m_pText;
    public:
        Chapter(FixedText *pText, bool bShow);
        Chapter(vcl::Window *pGrid, unsigned nYPos, const OUString& sDisplayName);
        ~Chapter();
        void dispose() { m_pText.disposeAndClear(); }
        void SetBackground(const Wallpaper& W) { m_pText->SetBackground(W); }
        void Show(const Wallpaper& rBackWall);
    };

    // Entry -- a color config entry:
    // text (checkbox) + color list box + preview box
    class Entry
    {
    public:
        Entry(ColorConfigWindow_Impl& rParent, unsigned iEntry, long nCheckBoxLabelOffset, bool bShow);
        Entry(vcl::Window* pGrid, unsigned nYPos, const ExtendedColorConfigValue& aColorEntry,
            long nCheckBoxLabelOffset);
        ~Entry();
    public:
        void Show ();
        void Hide ();
        void SetAppearance(Wallpaper const& rTextWall);
        void SetTextColor (Color C) { m_pText->SetTextColor(C); }
    public:
        void SetLinks (Link<Button*,void> const&, Link<SvxColorListBox&,void> const&, Link<Control&,void> const&);
        void Update (ColorConfigEntry, ColorConfigValue const&);
        void Update (ExtendedColorConfigValue const&);
        void ColorChanged (ColorConfigEntry, ColorConfigValue&);
        void ColorChanged (ExtendedColorConfigValue&);
    public:
        long GetTop () const { return m_pPreview->GetPosPixel().Y(); }
        unsigned GetHeight () const { return m_pColorList->GetSizePixel().Height(); }
    public:
        bool Is (CheckBox* pBox) const { return m_pText == pBox; }
        bool Is (SvxColorListBox* pBox) const { return m_pColorList == pBox; }
        void dispose()
        {
            m_pText.disposeAndClear();
            m_pColorList.disposeAndClear();
            m_pPreview.disposeAndClear();
        }
    private:
        bool m_bOwnsWidgets;
        // checkbox (CheckBox) or simple text (FixedText)
        VclPtr<Control> m_pText;
        // color list box
        VclPtr<SvxColorListBox> m_pColorList;
        // color preview box
        VclPtr<vcl::Window> m_pPreview;
        // default color
        Color m_aDefaultColor;
    private:
        void SetColor (Color);
    };

    // vChapters -- groups (group headers)
    std::vector<std::shared_ptr<Chapter> > vChapters;
    // vEntries -- color options
    std::vector<std::shared_ptr<Entry> > vEntries;

    // module options
    SvtModuleOptions aModuleOptions;


private:
    VclPtr<VclGrid>   m_pGrid;
    VclPtr<ScrollBar> m_pVScroll;
    VclPtr<HeaderBar> m_pHeaderHB;

    // initialization
    void CreateEntries();
    void SetAppearance();

private:
    virtual void Command (CommandEvent const& rCEvt) override;
    virtual void DataChanged (DataChangedEvent const& rDCEvt) override;

    virtual Size calculateRequisition() const override;
    virtual void setAllocation(const Size &rAllocation) override;

    bool IsGroupVisible (Group) const;
};


// ColorConfigWindow_Impl::Chapter


// ctor for default groups
// rParent: parent window (ColorConfigWindow_Impl)
// eGroup: which group is this?
// rResMgr: resource manager
ColorConfigWindow_Impl::Chapter::Chapter(FixedText* pText, bool bShow)
    : m_pText(pText)
{
    if (!bShow)
        m_pText->Hide();
}

// ctor for extended groups
ColorConfigWindow_Impl::Chapter::Chapter(vcl::Window *pGrid,
    unsigned nYPos, const OUString& rDisplayName)
{
    m_pText = VclPtr<FixedText>::Create(pGrid, WB_LEFT|WB_VCENTER|WB_3DLOOK);
    m_pText->set_font_attribute("weight", "bold");
    m_pText->set_grid_width(3);
    m_pText->set_grid_left_attach(0);
    m_pText->set_grid_top_attach(nYPos);
    m_pText->SetText(rDisplayName);
}

ColorConfigWindow_Impl::Chapter::~Chapter()
{
    // FIXME: we had an horrible m_bOwnsWidget const
    m_pText.disposeAndClear();
}

void ColorConfigWindow_Impl::Chapter::Show(Wallpaper const& rBackWall)
{
    // background
    m_pText->SetBackground(rBackWall);
    m_pText->Show();
}


// ColorConfigWindow_Impl::Entry


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
ColorConfigWindow_Impl::Entry::Entry( vcl::Window *pGrid, unsigned nYPos,
    ExtendedColorConfigValue const& rColorEntry, long nCheckBoxLabelOffset)
    : m_bOwnsWidgets(true)
    , m_aDefaultColor(rColorEntry.getDefaultColor())
{
    m_pText = VclPtr<FixedText>::Create(pGrid, WB_LEFT|WB_VCENTER|WB_3DLOOK);
    m_pText->set_grid_left_attach(0);
    m_pText->set_grid_top_attach(nYPos);
    m_pText->set_margin_left(6 + nCheckBoxLabelOffset);
    m_pText->SetText(rColorEntry.getDisplayName());

    m_pColorList = VclPtr<SvxColorListBox>::Create(pGrid);
    m_pColorList->set_grid_left_attach(1);
    m_pColorList->set_grid_top_attach(nYPos);

    m_pPreview = VclPtr<vcl::Window>::Create(pGrid, WB_BORDER);
    m_pPreview->set_grid_left_attach(2);
    m_pPreview->set_grid_top_attach(nYPos);
    m_pPreview->set_margin_right(6);

    Show();
}

ColorConfigWindow_Impl::Entry::~Entry()
{
    if (m_bOwnsWidgets)
    {
        m_pText.disposeAndClear();
        m_pColorList.disposeAndClear();
        m_pPreview.disposeAndClear();
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
// rTextWall: background of the text (transparent)
// aSampleList: sample color listbox (to copy from)
void ColorConfigWindow_Impl::Entry::SetAppearance(Wallpaper const& rTextWall)
{
    // text (and optionally checkbox)
    m_pText->SetBackground(rTextWall);
    // preview
    m_pPreview->SetBorderStyle(WindowBorderStyle::MONO);
    // color list
    m_pColorList->SetSlotId(SID_ATTR_CHAR_COLOR);
    m_pColorList->SetAutoDisplayColor(m_aDefaultColor);
}

// SetLinks()
void ColorConfigWindow_Impl::Entry::SetLinks(
    Link<Button*,void> const& aCheckLink, Link<SvxColorListBox&,void> const& aColorLink,
    Link<Control&,void> const& aGetFocusLink)
{
    m_pColorList->SetSelectHdl(aColorLink);
    m_pColorList->SetGetFocusHdl(aGetFocusLink);
    if (CheckBox* pCheckBox = dynamic_cast<CheckBox*>(m_pText.get()))
    {
        pCheckBox->SetClickHdl(aCheckLink);
        pCheckBox->SetGetFocusHdl(aGetFocusLink);
    }
}

// updates a default color config entry
void ColorConfigWindow_Impl::Entry::Update (
    ColorConfigEntry aColorEntry, ColorConfigValue const& rValue
) {
    Color aColor(rValue.nColor);
    m_pColorList->SelectEntry(aColor);
    if (aColor.GetColor() == COL_AUTO)
        aColor = ColorConfig::GetDefaultColor(aColorEntry);
    m_pPreview->SetBackground(Wallpaper(aColor));
    if (CheckBox* pCheckBox = dynamic_cast<CheckBox*>(m_pText.get()))
        pCheckBox->Check(rValue.bIsVisible);
}

// updates an extended color config entry
void ColorConfigWindow_Impl::Entry::Update (
    ExtendedColorConfigValue const& rValue
) {
    Color aColor(rValue.getColor());
    if (rValue.getColor() == rValue.getDefaultColor())
        m_pColorList->SelectEntry(Color(COL_AUTO));
    else
        m_pColorList->SelectEntry(aColor);
    SetColor(aColor);
}

// color of a default entry has changed
void ColorConfigWindow_Impl::Entry::ColorChanged (
    ColorConfigEntry aColorEntry,
    ColorConfigValue& rValue
) {
    Color aColor = m_pColorList->GetSelectEntryColor();
    rValue.nColor = aColor.GetColor();
    if (aColor.GetColor() == COL_AUTO)
        aColor = ColorConfig::GetDefaultColor(aColorEntry);
    SetColor(aColor);
}

// color of an extended entry has changed
void ColorConfigWindow_Impl::Entry::ColorChanged (
    ExtendedColorConfigValue& rValue
) {
    Color aColor = m_pColorList->GetSelectEntryColor();
    rValue.setColor(aColor.GetColor());
    if (aColor.GetColor() == COL_AUTO)
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


// ColorConfigWindow_Impl


ColorConfigWindow_Impl::ColorConfigWindow_Impl(vcl::Window* pParent)
    : VclContainer(pParent)
{
    m_pUIBuilder.reset(new VclBuilder(this, getUIRootDir(), "cui/ui/colorconfigwin.ui"));
    get(m_pGrid, "ColorConfigWindow");
    CreateEntries();
    SetAppearance();
}

void ColorConfigWindow_Impl::dispose()
{
    m_pGrid.clear();
    m_pVScroll.clear();
    m_pHeaderHB.clear();
    for (auto i = vChapters.begin(); i != vChapters.end(); ++i)
        (*i)->dispose();
    for (auto i = vEntries.begin(); i != vEntries.end(); ++i)
        (*i)->dispose();
    disposeBuilder();
    VclContainer::dispose();
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
    std::bitset<nGroupCount> aModulesInstalled;
    // creating group headers
    vChapters.reserve(nGroupCount);
    for (unsigned i = 0; i != nGroupCount; ++i)
    {
        aModulesInstalled[i] = IsGroupVisible(vGroupInfo[i].eGroup);
        vChapters.push_back(std::make_shared<Chapter>(
            get<FixedText>(vGroupInfo[i].pGroup), aModulesInstalled[i]));
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
        ScopedVclPtrInstance< CheckBox > aCheckBox(this);
        ScopedVclPtrInstance< FixedText > aFixedText(this);
        aCheckBox->SetText(sSampleText);
        aFixedText->SetText(sSampleText);
        Size aCheckSize(aCheckBox->CalcMinimumSize(0x7fffffff));
        Size aFixedSize(aFixedText->CalcMinimumSize());
        nCheckBoxLabelOffset = aCheckSize.Width() - aFixedSize.Width();
    }

    // creating entries
    vEntries.reserve(ColorConfigEntryCount);
    for (unsigned i = 0; i < SAL_N_ELEMENTS(vEntryInfo); ++i)
    {
        vEntries.push_back(std::make_shared<Entry>(*this, i, nCheckBoxLabelOffset,
            aModulesInstalled[vEntryInfo[i].eGroup]));
    }

    // extended entries
    ExtendedColorConfig aExtConfig;
    if (unsigned const nExtGroupCount = aExtConfig.GetComponentCount())
    {
        size_t nLineNum = vChapters.size() + vEntries.size() + 1;
        for (unsigned j = 0; j != nExtGroupCount; ++j)
        {
            OUString const sComponentName = aExtConfig.GetComponentName(j);
            vChapters.push_back(std::make_shared<Chapter>(
                m_pGrid, nLineNum,
                aExtConfig.GetComponentDisplayName(sComponentName)
            ));
            ++nLineNum;
            unsigned nColorCount = aExtConfig.GetComponentColorCount(sComponentName);
            for (unsigned i = 0; i != nColorCount; ++i)
            {
                ExtendedColorConfigValue const aColorEntry =
                    aExtConfig.GetComponentColorConfigValue(sComponentName, i);
                vEntries.push_back(std::make_shared<Entry>(
                    m_pGrid, nLineNum, aColorEntry, nCheckBoxLabelOffset
                ));
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
    Wallpaper aBack(rStyleSettings.GetFieldColor());
    SetBackground(aBack);
    m_pGrid->SetBackground(aBack);

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

    OSL_ENSURE( vEntries.size() >= SAL_N_ELEMENTS(vEntryInfo), "wrong number of helpIDs for color listboxes" );

    // appearance
    for (size_t i = 0; i != vEntries.size(); ++i)
    {
        // appearance
        vEntries[i]->SetAppearance(aTransparentWall);
    }
}

void ColorConfigWindow_Impl::AdjustHeaderBar()
{
    // horizontal positions
    unsigned const nX0 = 0;
    unsigned const nX1 = get<vcl::Window>("doccolor")->GetPosPixel().X();
    unsigned const nX2 = get<vcl::Window>("doccolor_lb")->GetPosPixel().X();
    unsigned const nX3 = get<vcl::Window>("doccolor_wn")->GetPosPixel().X();
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
    Link<Button*,void> const& aCheckLink, Link<SvxColorListBox&,void> const& aColorLink, Link<Control&,void> const& aGetFocusLink
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
void ColorConfigWindow_Impl::ColorHdl(
    EditableColorConfig* pConfig, EditableExtendedColorConfig* pExtConfig,
    SvxColorListBox* pBox)
{
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
            return aModuleOptions.IsModuleInstalled(SvtModuleOptions::EModule::WRITER);
        case Group_Calc:
            return aModuleOptions.IsModuleInstalled(SvtModuleOptions::EModule::CALC);
        case Group_Draw:
            return
                aModuleOptions.IsModuleInstalled(SvtModuleOptions::EModule::DRAW) ||
                aModuleOptions.IsModuleInstalled(SvtModuleOptions::EModule::IMPRESS);
        case Group_Sql:
            return aModuleOptions.IsModuleInstalled(SvtModuleOptions::EModule::DATABASE);
        default:
            return true;
    }
}

void ColorConfigWindow_Impl::DataChanged (DataChangedEvent const& rDCEvt)
{
    Window::DataChanged( rDCEvt );
    if ( (rDCEvt.GetType() == DataChangedEventType::SETTINGS) &&
         (rDCEvt.GetFlags() & AllSettingsFlags::STYLE) )
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
    VclPtr<HeaderBar>              m_pHeaderHB;
    VclPtr<VclHBox>                m_pBody;
    VclPtr<ColorConfigWindow_Impl> m_pScrollWindow;
    VclPtr<ScrollBar>              m_pVScroll;

    EditableColorConfig*            pColorConfig;
    EditableExtendedColorConfig*    pExtColorConfig;

    DECL_LINK(ScrollHdl, ScrollBar*, void);
    DECL_LINK(ClickHdl, Button*, void);
    DECL_LINK(ColorHdl, SvxColorListBox&, void);
    DECL_LINK(ControlFocusHdl, Control&, void);

    virtual bool PreNotify (NotifyEvent& rNEvt) override;
    virtual void Command (CommandEvent const& rCEvt) override;
    virtual void DataChanged (DataChangedEvent const& rDCEvt) override;
public:
    explicit ColorConfigCtrl_Impl(vcl::Window* pParent);
    virtual ~ColorConfigCtrl_Impl() override;
    virtual void dispose() override;

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

ColorConfigCtrl_Impl::ColorConfigCtrl_Impl(vcl::Window* pParent)
    : VclVBox(pParent)
    , pColorConfig(nullptr)
    , pExtColorConfig(nullptr)
{
    m_pHeaderHB = VclPtr<HeaderBar>::Create(this, WB_BUTTONSTYLE | WB_BOTTOMBORDER);

    m_pBody = VclPtr<VclHBox>::Create(this);
    m_pScrollWindow = VclPtr<ColorConfigWindow_Impl>::Create(m_pBody);
    m_pVScroll = VclPtr<ScrollBar>::Create(m_pBody, WB_VERT);
    m_pScrollWindow->Init(m_pVScroll, m_pHeaderHB);

    m_pBody->set_hexpand(true);
    m_pBody->set_vexpand(true);
    m_pBody->set_expand(true);
    m_pBody->set_fill(true);

    m_pScrollWindow->set_hexpand(true);
    m_pScrollWindow->set_vexpand(true);
    m_pScrollWindow->set_expand(true);
    m_pScrollWindow->set_fill(true);

    Link<ScrollBar*,void> aScrollLink = LINK(this, ColorConfigCtrl_Impl, ScrollHdl);
    m_pVScroll->SetScrollHdl(aScrollLink);
    m_pVScroll->SetEndScrollHdl(aScrollLink);

    Link<Button*,void> aCheckLink = LINK(this, ColorConfigCtrl_Impl, ClickHdl);
    Link<SvxColorListBox&,void> aColorLink = LINK(this, ColorConfigCtrl_Impl, ColorHdl);
    Link<Control&,void> aGetFocusLink = LINK(this, ColorConfigCtrl_Impl, ControlFocusHdl);
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
    const HeaderBarItemBits nHeadBits = HeaderBarItemBits::VCENTER | HeaderBarItemBits::FIXED | HeaderBarItemBits::FIXEDPOS;
    m_pHeaderHB->InsertItem(1, rOn, 0, nHeadBits | HeaderBarItemBits::CENTER);
    m_pHeaderHB->InsertItem(2, rUIElems, 0, nHeadBits | HeaderBarItemBits::LEFT);
    m_pHeaderHB->InsertItem(3, rColorSetting, 0, nHeadBits | HeaderBarItemBits::LEFT);
    m_pHeaderHB->InsertItem(4, rPreview, 0, nHeadBits | HeaderBarItemBits::LEFT);
    m_pHeaderHB->set_height_request(GetTextHeight() + 6);
}

ColorConfigCtrl_Impl::~ColorConfigCtrl_Impl()
{
    disposeOnce();
}

void ColorConfigCtrl_Impl::dispose()
{
    m_pVScroll.disposeAndClear();
    m_pScrollWindow.disposeAndClear();
    m_pBody.disposeAndClear();
    m_pHeaderHB.disposeAndClear();
    VclVBox::dispose();
}

VCL_BUILDER_DECL_FACTORY(ColorConfigCtrl)
{
    (void)rMap;
    rRet = VclPtr<ColorConfigCtrl_Impl>::Create(pParent);
}

void ColorConfigCtrl_Impl::Update ()
{
    DBG_ASSERT(pColorConfig, "Configuration not set");
    m_pScrollWindow->Update(pColorConfig, pExtColorConfig);
}

IMPL_LINK(ColorConfigCtrl_Impl, ScrollHdl, ScrollBar*, pScrollBar, void)
{
    m_pScrollWindow->ScrollHdl(*pScrollBar);
}

bool ColorConfigCtrl_Impl::PreNotify( NotifyEvent& rNEvt )
{
    if(rNEvt.GetType() == MouseNotifyEvent::COMMAND)
    {
        const CommandEvent* pCEvt = rNEvt.GetCommandEvent();
        if( pCEvt->GetCommand() == CommandEventId::Wheel )
        {
            Command(*pCEvt);
            return true;
        }
    }
    return VclVBox::PreNotify(rNEvt);
}

void ColorConfigCtrl_Impl::Command( const CommandEvent& rCEvt )
{
    switch ( rCEvt.GetCommand() )
    {

        case CommandEventId::Wheel:
        case CommandEventId::StartAutoScroll:
        case CommandEventId::AutoScroll:
        {
            const CommandWheelData* pWheelData = rCEvt.GetWheelData();
            if(pWheelData && !pWheelData->IsHorz() && CommandWheelMode::ZOOM != pWheelData->GetMode())
            {
                HandleScrollCommand(rCEvt, nullptr, m_pVScroll);
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
    if ( (rDCEvt.GetType() == DataChangedEventType::SETTINGS) &&
         (rDCEvt.GetFlags() & AllSettingsFlags::STYLE) )
    {
        const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
        SetBackground(Wallpaper(rStyleSettings.GetFieldColor()));
    }
}

IMPL_LINK(ColorConfigCtrl_Impl, ClickHdl, Button*, pBox, void)
{
    DBG_ASSERT(pColorConfig, "Configuration not set");
    m_pScrollWindow->ClickHdl(pColorConfig, static_cast<CheckBox*>(pBox));
}

// a color list has changed
IMPL_LINK(ColorConfigCtrl_Impl, ColorHdl, SvxColorListBox&, rBox, void)
{
    DBG_ASSERT(pColorConfig, "Configuration not set" );
    m_pScrollWindow->ColorHdl(pColorConfig, pExtColorConfig, &rBox);
}

IMPL_LINK(ColorConfigCtrl_Impl, ControlFocusHdl, Control&, rCtrl, void)
{
    // determine whether a control is completely visible
    // and make it visible
    long aCtrlPosY = rCtrl.GetPosPixel().Y();
    unsigned const nWinHeight = m_pScrollWindow->GetSizePixel().Height();
    unsigned const nEntryHeight = m_pScrollWindow->GetEntryHeight();
    if ((GetFocusFlags::Tab & rCtrl.GetGetFocusFlags()) &&
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
};


// SvxColorOptionsTabPage


SvxColorOptionsTabPage::SvxColorOptionsTabPage(
    vcl::Window* pParent, const SfxItemSet& rCoreSet)
    : SfxTabPage(pParent, "OptAppearancePage", "cui/ui/optappearancepage.ui", &rCoreSet)
    , bFillItemSetCalled(false)
    , pColorConfig(nullptr)
    , pExtColorConfig(nullptr)
{
    get(m_pColorSchemeLB, "colorschemelb");
    m_pColorSchemeLB->SetStyle(m_pColorSchemeLB->GetStyle() | WB_SORT);
    get(m_pSaveSchemePB, "save");
    get(m_pDeleteSchemePB, "delete");
    get(m_pColorConfigCT, "colorconfig");

    m_pColorConfigCT->InitHeaderBar(
        get<vcl::Window>("on")->GetText(),
        get<vcl::Window>("uielements")->GetText(),
        get<vcl::Window>("colorsetting")->GetText(),
        get<vcl::Window>("preview")->GetText());

    m_pColorSchemeLB->SetSelectHdl(LINK(this, SvxColorOptionsTabPage, SchemeChangedHdl_Impl));
    Link<Button*,void> aLk = LINK(this, SvxColorOptionsTabPage, SaveDeleteHdl_Impl );
    m_pSaveSchemePB->SetClickHdl(aLk);
    m_pDeleteSchemePB->SetClickHdl(aLk);
}

SvxColorOptionsTabPage::~SvxColorOptionsTabPage()
{
    disposeOnce();
}

void SvxColorOptionsTabPage::dispose()
{
    if (pColorConfig)
    {
        //when the dialog is cancelled but the color scheme ListBox has been changed these
        //changes need to be undone
        if(!bFillItemSetCalled && m_pColorSchemeLB->IsValueChangedFromSaved())
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
        pColorConfig = nullptr;

        pExtColorConfig->ClearModified();
        pExtColorConfig->EnableBroadcast();
        delete pExtColorConfig;
        pExtColorConfig = nullptr;
    }
    m_pColorSchemeLB.clear();
    m_pSaveSchemePB.clear();
    m_pDeleteSchemePB.clear();
    m_pColorConfigCT.clear();
    SfxTabPage::dispose();
}

VclPtr<SfxTabPage> SvxColorOptionsTabPage::Create( vcl::Window* pParent, const SfxItemSet* rAttrSet )
{
    return VclPtr<SvxColorOptionsTabPage>::Create( pParent, *rAttrSet );
}

bool SvxColorOptionsTabPage::FillItemSet( SfxItemSet*  )
{
    bFillItemSetCalled = true;
    if(m_pColorSchemeLB->IsValueChangedFromSaved())
    {
        pColorConfig->SetModified();
        pExtColorConfig->SetModified();
    }
    if(pColorConfig->IsModified())
        pColorConfig->Commit();
    if(pExtColorConfig->IsModified())
        pExtColorConfig->Commit();
    return true;
}

void SvxColorOptionsTabPage::Reset( const SfxItemSet* )
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

    OUString sUser = GetUserData();
    //has to be called always to speed up accessibility tools
    m_pColorConfigCT->SetScrollPosition(sUser.toInt32());
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

DeactivateRC SvxColorOptionsTabPage::DeactivatePage( SfxItemSet* pSet_ )
{
    if ( pSet_ )
        FillItemSet( pSet_ );
    return DeactivateRC::LeavePage;
}

void SvxColorOptionsTabPage::UpdateColorConfig()
{
    //update the color config control
    m_pColorConfigCT->Update();
}

IMPL_LINK(SvxColorOptionsTabPage, SchemeChangedHdl_Impl, ListBox&, rBox, void)
{
    pColorConfig->LoadScheme(rBox.GetSelectEntry());
    pExtColorConfig->LoadScheme(rBox.GetSelectEntry());
    UpdateColorConfig();
}

IMPL_LINK(SvxColorOptionsTabPage, SaveDeleteHdl_Impl, Button*, pButton, void )
{
    if (m_pSaveSchemePB == pButton)
    {
        OUString sName;

        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        DBG_ASSERT(pFact, "Dialog creation failed!");
        ScopedVclPtr<AbstractSvxNameDialog> aNameDlg(pFact->CreateSvxNameDialog( pButton,
                            sName, CUI_RES(RID_SVXSTR_COLOR_CONFIG_SAVE2) ));
        DBG_ASSERT(aNameDlg, "Dialog creation failed!");
        aNameDlg->SetCheckNameHdl( LINK(this, SvxColorOptionsTabPage, CheckNameHdl_Impl));
        aNameDlg->SetText(CUI_RES(RID_SVXSTR_COLOR_CONFIG_SAVE1));
        aNameDlg->SetHelpId(HID_OPTIONS_COLORCONFIG_SAVE_SCHEME);
        aNameDlg->SetCheckNameHdl( LINK(this, SvxColorOptionsTabPage, CheckNameHdl_Impl));
        if(RET_OK == aNameDlg->Execute())
        {
            aNameDlg->GetName(sName);
            pColorConfig->AddScheme(sName);
            pExtColorConfig->AddScheme(sName);
            m_pColorSchemeLB->InsertEntry(sName);
            m_pColorSchemeLB->SelectEntry(sName);
            m_pColorSchemeLB->GetSelectHdl().Call(*m_pColorSchemeLB);
        }
    }
    else
    {
        DBG_ASSERT(m_pColorSchemeLB->GetEntryCount() > 1, "don't delete the last scheme");
        ScopedVclPtrInstance< MessageDialog > aQuery(pButton, CUI_RES(RID_SVXSTR_COLOR_CONFIG_DELETE), VclMessageType::Question, VclButtonsType::YesNo);
        aQuery->SetText(CUI_RES(RID_SVXSTR_COLOR_CONFIG_DELETE_TITLE));
        if(RET_YES == aQuery->Execute())
        {
            OUString sDeleteScheme(m_pColorSchemeLB->GetSelectEntry());
            m_pColorSchemeLB->RemoveEntry(m_pColorSchemeLB->GetSelectEntryPos());
            m_pColorSchemeLB->SelectEntryPos(0);
            m_pColorSchemeLB->GetSelectHdl().Call(*m_pColorSchemeLB);
            //first select the new scheme and then delete the old one
            pColorConfig->DeleteScheme(sDeleteScheme);
            pExtColorConfig->DeleteScheme(sDeleteScheme);
        }
    }
    m_pDeleteSchemePB->Enable( m_pColorSchemeLB->GetEntryCount() > 1 );
}

IMPL_LINK(SvxColorOptionsTabPage, CheckNameHdl_Impl, AbstractSvxNameDialog&, rDialog, bool )
{
    OUString sName;
    rDialog.GetName(sName);
    return !sName.isEmpty() && LISTBOX_ENTRY_NOTFOUND == m_pColorSchemeLB->GetEntryPos( sName );
}

void SvxColorOptionsTabPage::FillUserData()
{
    SetUserData(OUString::number(m_pColorConfigCT->GetScrollPosition()));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
