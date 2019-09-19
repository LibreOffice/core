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

#include <osl/diagnose.h>
#include <tools/debug.hxx>
#include <editeng/editids.hrc>
#include <svtools/colorcfg.hxx>
#include <svtools/extcolorcfg.hxx>
#include <vcl/headbar.hxx>
#include <vcl/scrbar.hxx>
#include <svx/colorbox.hxx>
#include <unotools/moduleoptions.hxx>
#include <vcl/layout.hxx>
#include <vcl/settings.hxx>
#include <vcl/weld.hxx>
#include <svx/svxdlg.hxx>
#include <helpids.h>
#include <dialmgr.hxx>
#include "optcolor.hxx"
#include <strings.hrc>

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
    // has checkbox?
    bool bCheckBox;
}
const vEntryInfo[] =
{
    #define IDS(Name) \
        SAL_STRINGIFY(Name), SAL_STRINGIFY(Name##_lb), false

    #define IDS_CB(Name) \
        SAL_STRINGIFY(Name), SAL_STRINGIFY(Name##_lb), true

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
    { Group_Calc,    IDS(values) },
    { Group_Calc,    IDS(formulas) },
    { Group_Calc,    IDS(text) },
    { Group_Calc,    IDS(protectedcells) },

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
{
public:
    explicit ColorConfigWindow_Impl(weld::Window* pTopLevel, weld::Container* pParent);

public:
    void SetLinks(Link<weld::ToggleButton&,void> const&, Link<ColorListBox&,void> const&);
    void Update(EditableColorConfig const*, EditableExtendedColorConfig const*);
    void ClickHdl(EditableColorConfig*, weld::ToggleButton&);
    void ColorHdl(EditableColorConfig*, EditableExtendedColorConfig*, ColorListBox*);
    void Init(weld::ScrolledWindow* pVScroll);

    weld::Widget& GetWidget1()
    {
        return *m_xWidget1;
    }

    weld::Widget& GetWidget2()
    {
        return *m_xWidget2;
    }

    void AdjustExtraWidths(int nTextWidth);

private:
    // Chapter -- horizontal group separator stripe with text
    class Chapter
    {
        // text
        std::unique_ptr<weld::Label> m_xText;
    public:
        Chapter(weld::Builder& rBuilder, const char* pLabelWidget, bool bShow);
        void SetText(const OUString& rLabel) { m_xText->set_label(rLabel); }
    };

    // Entry -- a color config entry:
    // text (checkbox) + color list box
    class Entry
    {
    public:
        Entry(weld::Window* pTopLevel, weld::Builder& rBuilder, const char* pTextWidget, const char* pColorWidget,
              const Color& rColor, long nCheckBoxLabelOffset, bool bCheckBox, bool bShow);
    public:
        void Show ();
        void SetText(const OUString& rLabel) { dynamic_cast<weld::Label&>(*m_xText).set_label(rLabel); }
        void set_width_request(int nTextWidth) { m_xText->set_size_request(nTextWidth, -1); }
        void Hide ();
    public:
        void SetLinks(Link<weld::ToggleButton&,void> const&, Link<ColorListBox&,void> const&);
        void Update (ColorConfigValue const&);
        void Update (ExtendedColorConfigValue const&);
        void ColorChanged (ColorConfigValue&);
        void ColorChanged (ExtendedColorConfigValue&);
    public:
        bool Is(weld::ToggleButton* pBox) const { return m_xText.get() == pBox; }
        bool Is(ColorListBox* pBox) const { return m_xColorList.get() == pBox; }
    private:
        // checkbox (CheckBox) or simple text (FixedText)
        std::unique_ptr<weld::Widget> m_xText;
        // color list box
        std::unique_ptr<ColorListBox> m_xColorList;
        // default color
        Color m_aDefaultColor;
    };

private:
    weld::Window* m_pTopLevel;
    weld::ScrolledWindow* m_pVScroll;
    std::unique_ptr<weld::Builder> m_xBuilder;
    std::unique_ptr<weld::Container> m_xGrid;
    std::unique_ptr<weld::Widget> m_xWidget1;
    std::unique_ptr<weld::Widget> m_xWidget2;

    std::vector<std::unique_ptr<weld::Builder>> vExtBuilders;
    std::vector<std::unique_ptr<weld::Container>> vExtContainers;
    // vChapters -- groups (group headers)
    std::vector<std::shared_ptr<Chapter> > vChapters;
    // vEntries -- color options
    std::vector<std::shared_ptr<Entry> > vEntries;

    // module options
    SvtModuleOptions aModuleOptions;

    // initialization
    void CreateEntries();

private:

    bool IsGroupVisible (Group) const;
};

// ColorConfigWindow_Impl::Chapter

// ctor for default groups
// rParent: parent window (ColorConfigWindow_Impl)
// eGroup: which group is this?
ColorConfigWindow_Impl::Chapter::Chapter(weld::Builder& rBuilder, const char* pLabelWidget, bool bShow)
    : m_xText(rBuilder.weld_label(pLabelWidget))
{
    if (!bShow)
        m_xText->hide();
}

// ColorConfigWindow_Impl::Entry
ColorConfigWindow_Impl::Entry::Entry(weld::Window* pTopLevel, weld::Builder& rBuilder,
                                     const char* pTextWidget, const char* pColorWidget,
                                     const Color& rColor,
                                     long nCheckBoxLabelOffset, bool bCheckBox, bool bShow)
    : m_xColorList(new ColorListBox(rBuilder.weld_menu_button(pColorWidget), pTopLevel))
    , m_aDefaultColor(rColor)
{
    if (bCheckBox)
        m_xText = rBuilder.weld_check_button(pTextWidget);
    else
        m_xText = rBuilder.weld_label(pTextWidget);

    // color list
    m_xColorList->SetSlotId(SID_ATTR_CHAR_COLOR);
    m_xColorList->SetAutoDisplayColor(m_aDefaultColor);

    if (!bCheckBox)
    {
        m_xText->set_margin_left(m_xText->get_margin_left() +
            nCheckBoxLabelOffset);
    }

    if (!bShow)
        Hide();
}

void ColorConfigWindow_Impl::Entry::Hide()
{
    m_xText->hide();
    m_xColorList->hide();
}

// SetLinks()
void ColorConfigWindow_Impl::Entry::SetLinks(Link<weld::ToggleButton&,void> const& aCheckLink,
                                             Link<ColorListBox&,void> const& aColorLink)
{
    m_xColorList->SetSelectHdl(aColorLink);
    if (weld::ToggleButton* pCheckBox = dynamic_cast<weld::ToggleButton*>(m_xText.get()))
    {
        pCheckBox->connect_toggled(aCheckLink);
    }
}

// updates a default color config entry
void ColorConfigWindow_Impl::Entry::Update(ColorConfigValue const& rValue)
{
    Color aColor(rValue.nColor);
    m_xColorList->SelectEntry(aColor);
    if (weld::ToggleButton* pCheckBox = dynamic_cast<weld::ToggleButton*>(m_xText.get()))
        pCheckBox->set_active(rValue.bIsVisible);
}

// updates an extended color config entry
void ColorConfigWindow_Impl::Entry::Update(ExtendedColorConfigValue const& rValue)
{
    Color aColor(rValue.getColor());
    if (rValue.getColor() == rValue.getDefaultColor())
        m_xColorList->SelectEntry(COL_AUTO);
    else
        m_xColorList->SelectEntry(aColor);
}

// color of a default entry has changed
void ColorConfigWindow_Impl::Entry::ColorChanged(ColorConfigValue& rValue)
{
    Color aColor = m_xColorList->GetSelectEntryColor();
    rValue.nColor = aColor;
}

// color of an extended entry has changed
void ColorConfigWindow_Impl::Entry::ColorChanged(ExtendedColorConfigValue& rValue)
{
    Color aColor = m_xColorList->GetSelectEntryColor();
    rValue.setColor(aColor);
    if (aColor == COL_AUTO)
    {
        rValue.setColor(rValue.getDefaultColor());
    }
}

// ColorConfigWindow_Impl
ColorConfigWindow_Impl::ColorConfigWindow_Impl(weld::Window* pTopLevel, weld::Container* pParent)
    : m_pTopLevel(pTopLevel)
    , m_xBuilder(Application::CreateBuilder(pParent, "cui/ui/colorconfigwin.ui"))
    , m_xGrid(m_xBuilder->weld_container("ColorConfigWindow"))
    , m_xWidget1(m_xBuilder->weld_widget("doccolor"))
    , m_xWidget2(m_xBuilder->weld_widget("doccolor_lb"))
{
    CreateEntries();
}

void ColorConfigWindow_Impl::CreateEntries()
{
    std::bitset<nGroupCount> aModulesInstalled;
    // creating group headers
    vChapters.reserve(nGroupCount);
    for (unsigned i = 0; i != nGroupCount; ++i)
    {
        aModulesInstalled[i] = IsGroupVisible(vGroupInfo[i].eGroup);
        vChapters.push_back(std::make_shared<Chapter>(*m_xBuilder, vGroupInfo[i].pGroup, aModulesInstalled[i]));
    }

    // Here we want to get the amount to add to the position of a FixedText to
    // get it to align its contents with that of a CheckBox
    long nCheckBoxLabelOffset = 0;
    {
        OUString sSampleText("X");
        std::unique_ptr<weld::CheckButton> xCheckBox(m_xBuilder->weld_check_button("docboundaries"));
        std::unique_ptr<weld::Label> xFixedText(m_xBuilder->weld_label("doccolor"));
        OUString sOrigCheck(xCheckBox->get_label());
        OUString sOrigFixed(xFixedText->get_label());
        xCheckBox->set_label(sSampleText);
        xFixedText->set_label(sSampleText);
        Size aCheckSize(xCheckBox->get_preferred_size());
        Size aFixedSize(xFixedText->get_preferred_size());
        xCheckBox->set_label(sOrigCheck);
        xFixedText->set_label(sOrigFixed);
        nCheckBoxLabelOffset = aCheckSize.Width() - aFixedSize.Width();
    }

    // creating entries
    vEntries.reserve(ColorConfigEntryCount);
    for (size_t i = 0; i < SAL_N_ELEMENTS(vEntryInfo); ++i)
    {
        vEntries.push_back(std::make_shared<Entry>(m_pTopLevel, *m_xBuilder,
            vEntryInfo[i].pText, vEntryInfo[i].pColor,
            ColorConfig::GetDefaultColor(static_cast<ColorConfigEntry>(i)),
            nCheckBoxLabelOffset,
            vEntryInfo[i].bCheckBox,
            aModulesInstalled[vEntryInfo[i].eGroup]));
    }

    // extended entries
    ExtendedColorConfig aExtConfig;
    if (unsigned const nExtGroupCount = aExtConfig.GetComponentCount())
    {
        size_t nLineNum = vChapters.size() + vEntries.size() + 1;
        for (unsigned j = 0; j != nExtGroupCount; ++j)
        {
            vExtBuilders.emplace_back(Application::CreateBuilder(m_xGrid.get(), "cui/ui/chapterfragment.ui"));
            vExtContainers.emplace_back(vExtBuilders.back()->weld_container("ChapterFragment"));

            vExtContainers.back()->set_grid_width(3);
            vExtContainers.back()->set_grid_left_attach(0);
            vExtContainers.back()->set_grid_top_attach(nLineNum);

            OUString const sComponentName = aExtConfig.GetComponentName(j);
            vChapters.push_back(std::make_shared<Chapter>(
                *vExtBuilders.back(), "chapter", true));
            vChapters.back()->SetText(aExtConfig.GetComponentDisplayName(sComponentName));
            ++nLineNum;
            unsigned nColorCount = aExtConfig.GetComponentColorCount(sComponentName);
            for (unsigned i = 0; i != nColorCount; ++i)
            {
                vExtBuilders.emplace_back(Application::CreateBuilder(m_xGrid.get(), "cui/ui/colorfragment.ui"));
                vExtContainers.emplace_back(vExtBuilders.back()->weld_container("ColorFragment"));

                vExtContainers.back()->set_grid_width(3);
                vExtContainers.back()->set_grid_left_attach(0);
                vExtContainers.back()->set_grid_top_attach(nLineNum);

                ExtendedColorConfigValue const aColorEntry =
                    aExtConfig.GetComponentColorConfigValue(sComponentName, i);
                vEntries.push_back(std::make_shared<Entry>(m_pTopLevel, *vExtBuilders.back(),
                    "label", "button", aColorEntry.getDefaultColor(),
                    nCheckBoxLabelOffset, false, true));
                vEntries.back()->SetText(aColorEntry.getDisplayName());
                ++nLineNum;
            }
        }
    }
}

void ColorConfigWindow_Impl::AdjustExtraWidths(int nTextWidth)
{
    for (size_t i = SAL_N_ELEMENTS(vEntryInfo); i < vEntries.size(); ++i)
        vEntries[i]->set_width_request(nTextWidth);
}

void ColorConfigWindow_Impl::Init(weld::ScrolledWindow* pVScroll)
{
    m_pVScroll = pVScroll;
}

// SetLinks()
void ColorConfigWindow_Impl::SetLinks(Link<weld::ToggleButton&,void> const& aCheckLink,
                                      Link<ColorListBox&,void> const& aColorLink)
{
    for (auto const & i: vEntries)
        i->SetLinks(aCheckLink, aColorLink);
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
            pConfig->GetColorValue(aColorEntry)
        );
    }

    // updating extended entries
    decltype(vEntries)::size_type i = ColorConfigEntryCount;
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

// ClickHdl()
void ColorConfigWindow_Impl::ClickHdl(EditableColorConfig* pConfig, weld::ToggleButton& rBox)
{
    for (unsigned i = 0; i != ColorConfigEntryCount; ++i)
    {
        if (vEntries[i]->Is(&rBox))
        {
            ColorConfigEntry const aEntry = static_cast<ColorConfigEntry>(i);
            ColorConfigValue aValue = pConfig->GetColorValue(aEntry);
            aValue.bIsVisible = rBox.get_active();
            pConfig->SetColorValue(aEntry, aValue);
            break;
        }
    }
}

// ColorHdl()
void ColorConfigWindow_Impl::ColorHdl(
    EditableColorConfig* pConfig, EditableExtendedColorConfig* pExtConfig,
    ColorListBox* pBox)
{
    unsigned i = 0;

    // default entries
    for ( ; i != ColorConfigEntryCount; ++i)
    {
        if (pBox && vEntries[i]->Is(pBox))
        {
            ColorConfigEntry const aColorEntry = static_cast<ColorConfigEntry>(i);
            ColorConfigValue aValue = pConfig->GetColorValue(aColorEntry);
            vEntries[i]->ColorChanged(aValue);
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

class ColorConfigCtrl_Impl
{
    std::unique_ptr<weld::ScrolledWindow> m_xVScroll;
    std::unique_ptr<weld::Container> m_xBody;
    std::unique_ptr<ColorConfigWindow_Impl> m_xScrollWindow;

    EditableColorConfig*            pColorConfig;
    EditableExtendedColorConfig*    pExtColorConfig;

    DECL_LINK(ClickHdl, weld::ToggleButton&, void);
    DECL_LINK(ColorHdl, ColorListBox&, void);
//TODO    DECL_LINK(ControlFocusHdl, Control&, void);

public:
    explicit ColorConfigCtrl_Impl(weld::Window* pTopLevel, weld::Builder& rbuilder);

    void AdjustExtraWidths(int nTextWidth) { m_xScrollWindow->AdjustExtraWidths(nTextWidth); }
    void SetConfig (EditableColorConfig& rConfig) { pColorConfig = &rConfig; }
    void SetExtendedConfig (EditableExtendedColorConfig& rConfig) { pExtColorConfig = &rConfig; }
    void Update();
    long GetScrollPosition() const
    {
        return m_xVScroll->vadjustment_get_value();
    }
    void SetScrollPosition(long nSet)
    {
        m_xVScroll->vadjustment_set_value(nSet);
    }
    weld::Widget& GetWidget1()
    {
        return m_xScrollWindow->GetWidget1();
    }
    weld::Widget& GetWidget2()
    {
        return m_xScrollWindow->GetWidget2();
    }
};

ColorConfigCtrl_Impl::ColorConfigCtrl_Impl(weld::Window* pTopLevel, weld::Builder& rBuilder)
    : m_xVScroll(rBuilder.weld_scrolled_window("scroll"))
    , m_xBody(rBuilder.weld_container("colorconfig"))
    , m_xScrollWindow(std::make_unique<ColorConfigWindow_Impl>(pTopLevel, m_xBody.get()))
    , pColorConfig(nullptr)
    , pExtColorConfig(nullptr)
{
    m_xScrollWindow->Init(m_xVScroll.get());
    m_xBody->set_stack_background();

    Link<weld::ToggleButton&,void> aCheckLink = LINK(this, ColorConfigCtrl_Impl, ClickHdl);
    Link<ColorListBox&,void> aColorLink = LINK(this, ColorConfigCtrl_Impl, ColorHdl);
    m_xScrollWindow->SetLinks(aCheckLink, aColorLink);
}

void ColorConfigCtrl_Impl::Update ()
{
    DBG_ASSERT(pColorConfig, "Configuration not set");
    m_xScrollWindow->Update(pColorConfig, pExtColorConfig);
}

IMPL_LINK(ColorConfigCtrl_Impl, ClickHdl, weld::ToggleButton&, rBox, void)
{
    DBG_ASSERT(pColorConfig, "Configuration not set");
    m_xScrollWindow->ClickHdl(pColorConfig, rBox);
}

// a color list has changed
IMPL_LINK(ColorConfigCtrl_Impl, ColorHdl, ColorListBox&, rBox, void)
{
    DBG_ASSERT(pColorConfig, "Configuration not set" );
    m_xScrollWindow->ColorHdl(pColorConfig, pExtColorConfig, &rBox);
}

#if 0
IMPL_LINK(ColorConfigCtrl_Impl, ControlFocusHdl, Control&, rCtrl, void)
{
    // determine whether a control is completely visible
    // and make it visible
    unsigned const nWinHeight = m_xScrollWindow->GetSizePixel().Height();
    unsigned const nEntryHeight = m_xScrollWindow->GetEntryHeight();

    // calc visible area
    long const nScrollOffset = m_xScrollWindow->GetScrollOffset();
    long nThumbPos = m_pVScroll->GetThumbPos();
    long const nWinTop = nThumbPos * nScrollOffset;
    long const nWinBottom = nWinTop + nWinHeight;

    long const nCtrlPosY = rCtrl.GetPosPixel().Y();
    long const nSelectedItemPos = nCtrlPosY + nEntryHeight;
    bool const shouldScrollDown = nSelectedItemPos >= nWinBottom;
    bool const shouldScrollUp = nSelectedItemPos <= nWinTop;
    bool const isNeedToScroll = shouldScrollDown || shouldScrollUp || nCtrlPosY < 0;

    if ((GetFocusFlags::Tab & rCtrl.GetGetFocusFlags()) && isNeedToScroll)
    {
        if (shouldScrollDown)
        {
            long nOffset = (nSelectedItemPos - nWinBottom) / nScrollOffset;
            nThumbPos += nOffset + 2;
        }
        else
        {
            long nOffset = (nWinTop - nSelectedItemPos) / nScrollOffset;
            nThumbPos -= nOffset + 2;
            if(nThumbPos < 0)
                nThumbPos = 0;
        }
        m_xVScroll->vadjustment_set_value(nThumbPos);
    }
};
#endif

// SvxColorOptionsTabPage
SvxColorOptionsTabPage::SvxColorOptionsTabPage(TabPageParent pParent, const SfxItemSet& rCoreSet)
    : SfxTabPage(pParent, "cui/ui/optappearancepage.ui", "OptAppearancePage", &rCoreSet)
    , bFillItemSetCalled(false)
    , m_xColorSchemeLB(m_xBuilder->weld_combo_box("colorschemelb"))
    , m_xSaveSchemePB(m_xBuilder->weld_button("save"))
    , m_xDeleteSchemePB(m_xBuilder->weld_button("delete"))
    , m_xColorConfigCT(new ColorConfigCtrl_Impl(pParent.GetFrameWeld(), *m_xBuilder))
    , m_xTable(m_xBuilder->weld_widget("table"))
    , m_xOnFT(m_xBuilder->weld_label("on"))
    , m_xElementFT(m_xBuilder->weld_label("uielements"))
    , m_xColorFT(m_xBuilder->weld_label("colorsetting"))
    , m_rWidget1(m_xColorConfigCT->GetWidget1())
    , m_rWidget2(m_xColorConfigCT->GetWidget2())
{
    m_xColorSchemeLB->make_sorted();
    m_xColorSchemeLB->connect_changed(LINK(this, SvxColorOptionsTabPage, SchemeChangedHdl_Impl));
    Link<weld::Button&,void> aLk = LINK(this, SvxColorOptionsTabPage, SaveDeleteHdl_Impl );
    m_xSaveSchemePB->connect_clicked(aLk);
    m_xDeleteSchemePB->connect_clicked(aLk);

    m_rWidget1.connect_size_allocate(LINK(this, SvxColorOptionsTabPage, AdjustHeaderBar));
    m_rWidget2.connect_size_allocate(LINK(this, SvxColorOptionsTabPage, AdjustHeaderBar));
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
        if (!bFillItemSetCalled && m_xColorSchemeLB->get_value_changed_from_saved())
        {
            OUString sOldScheme = m_xColorSchemeLB->get_saved_value();
            if(!sOldScheme.isEmpty())
            {
                pColorConfig->SetCurrentSchemeName(sOldScheme);
                pExtColorConfig->SetCurrentSchemeName(sOldScheme);
            }
        }
        pColorConfig->ClearModified();
        pColorConfig->EnableBroadcast();
        pColorConfig.reset();

        pExtColorConfig->ClearModified();
        pExtColorConfig->EnableBroadcast();
        pExtColorConfig.reset();
    }
    m_xColorConfigCT.reset();
    SfxTabPage::dispose();
}

VclPtr<SfxTabPage> SvxColorOptionsTabPage::Create(TabPageParent pParent, const SfxItemSet* rAttrSet)
{
    return VclPtr<SvxColorOptionsTabPage>::Create(pParent, *rAttrSet);
}

bool SvxColorOptionsTabPage::FillItemSet( SfxItemSet*  )
{
    bFillItemSetCalled = true;
    if (m_xColorSchemeLB->get_value_changed_from_saved())
    {
        pColorConfig->SetModified();
        pExtColorConfig->SetModified();
    }
    if (pColorConfig->IsModified())
        pColorConfig->Commit();
    if (pExtColorConfig->IsModified())
        pExtColorConfig->Commit();
    return true;
}

void SvxColorOptionsTabPage::Reset( const SfxItemSet* )
{
    if(pColorConfig)
    {
        pColorConfig->ClearModified();
        pColorConfig->DisableBroadcast();
    }
    pColorConfig.reset(new EditableColorConfig);
    m_xColorConfigCT->SetConfig(*pColorConfig);

    if(pExtColorConfig)
    {
        pExtColorConfig->ClearModified();
        pExtColorConfig->DisableBroadcast();
    }
    pExtColorConfig.reset(new EditableExtendedColorConfig);
    m_xColorConfigCT->SetExtendedConfig(*pExtColorConfig);

    OUString sUser = GetUserData();
    //has to be called always to speed up accessibility tools
    m_xColorConfigCT->SetScrollPosition(sUser.toInt32());
    m_xColorSchemeLB->clear();
    uno::Sequence< OUString >  aSchemes = pColorConfig->GetSchemeNames();
    const OUString* pSchemes = aSchemes.getConstArray();
    for(sal_Int32 i = 0; i < aSchemes.getLength(); i++)
        m_xColorSchemeLB->append_text(pSchemes[i]);
    m_xColorSchemeLB->set_active_text(pColorConfig->GetCurrentSchemeName());
    m_xColorSchemeLB->save_value();
    m_xDeleteSchemePB->set_sensitive( aSchemes.getLength() > 1 );
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
    m_xColorConfigCT->Update();
}

IMPL_LINK(SvxColorOptionsTabPage, SchemeChangedHdl_Impl, weld::ComboBox&, rBox, void)
{
    pColorConfig->LoadScheme(rBox.get_active_text());
    pExtColorConfig->LoadScheme(rBox.get_active_text());
    UpdateColorConfig();
}

IMPL_LINK(SvxColorOptionsTabPage, SaveDeleteHdl_Impl, weld::Button&, rButton, void)
{
    if (m_xSaveSchemePB.get() == &rButton)
    {
        OUString sName;

        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        ScopedVclPtr<AbstractSvxNameDialog> aNameDlg(pFact->CreateSvxNameDialog(GetDialogFrameWeld(),
                            sName, CuiResId(RID_SVXSTR_COLOR_CONFIG_SAVE2) ));
        aNameDlg->SetCheckNameHdl( LINK(this, SvxColorOptionsTabPage, CheckNameHdl_Impl));
        aNameDlg->SetText(CuiResId(RID_SVXSTR_COLOR_CONFIG_SAVE1));
        aNameDlg->SetHelpId(HID_OPTIONS_COLORCONFIG_SAVE_SCHEME);
        aNameDlg->SetCheckNameHdl( LINK(this, SvxColorOptionsTabPage, CheckNameHdl_Impl));
        if(RET_OK == aNameDlg->Execute())
        {
            aNameDlg->GetName(sName);
            pColorConfig->AddScheme(sName);
            pExtColorConfig->AddScheme(sName);
            m_xColorSchemeLB->append_text(sName);
            m_xColorSchemeLB->set_active_text(sName);
            SchemeChangedHdl_Impl(*m_xColorSchemeLB);
        }
    }
    else
    {
        DBG_ASSERT(m_xColorSchemeLB->get_count() > 1, "don't delete the last scheme");
        std::unique_ptr<weld::MessageDialog> xQuery(Application::CreateMessageDialog(GetDialogFrameWeld(),
                                                    VclMessageType::Question, VclButtonsType::YesNo,
                                                    CuiResId(RID_SVXSTR_COLOR_CONFIG_DELETE)));
        xQuery->set_title(CuiResId(RID_SVXSTR_COLOR_CONFIG_DELETE_TITLE));
        if (RET_YES == xQuery->run())
        {
            OUString sDeleteScheme(m_xColorSchemeLB->get_active_text());
            m_xColorSchemeLB->remove(m_xColorSchemeLB->get_active());
            m_xColorSchemeLB->set_active(0);
            SchemeChangedHdl_Impl(*m_xColorSchemeLB);
            //first select the new scheme and then delete the old one
            pColorConfig->DeleteScheme(sDeleteScheme);
            pExtColorConfig->DeleteScheme(sDeleteScheme);
        }
    }
    m_xDeleteSchemePB->set_sensitive(m_xColorSchemeLB->get_count() > 1);
}

IMPL_LINK(SvxColorOptionsTabPage, CheckNameHdl_Impl, AbstractSvxNameDialog&, rDialog, bool )
{
    OUString sName;
    rDialog.GetName(sName);
    return !sName.isEmpty() && m_xColorSchemeLB->find_text(sName) == -1;
}

void SvxColorOptionsTabPage::FillUserData()
{
    SetUserData(OUString::number(m_xColorConfigCT->GetScrollPosition()));
}

IMPL_LINK_NOARG(SvxColorOptionsTabPage, AdjustHeaderBar, const Size&, void)
{
    // horizontal positions
    int nX0 = 0, nX1, nX2, y, width, height;
    m_rWidget1.get_extents_relative_to(*m_xTable, nX1, y, width, height);
    m_rWidget2.get_extents_relative_to(*m_xTable, nX2, y, width, height);
    auto nTextWidth1 = nX1 - nX0;
    auto nTextWidth2 = nX2 - nX1;
    m_xOnFT->set_size_request(nTextWidth1, -1);
    m_xElementFT->set_size_request(nTextWidth2, -1);
    m_xColorConfigCT->AdjustExtraWidths(nTextWidth2 - 12);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
