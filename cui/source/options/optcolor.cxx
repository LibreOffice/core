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

#include <com/sun/star/configuration/ReadWriteAccess.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <tools/debug.hxx>
#include <editeng/editids.hrc>
#include <svtools/colorcfg.hxx>
#include <svtools/extcolorcfg.hxx>
#include <svx/colorbox.hxx>
#include <unotools/moduleoptions.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <svx/svxdlg.hxx>
#include <helpids.h>
#include <dialmgr.hxx>
#include "optcolor.hxx"
#include <strings.hrc>
#include <svtools/miscopt.hxx>
#include <officecfg/Office/Common.hxx>
#include <officecfg/Office/ExtendedColorScheme.hxx>
#include <officecfg/Office/UI.hxx>
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
    OUString pGroup;
}
const vGroupInfo[] =
{
    // the groups are in the same order as in enum Group above
    { Group_General, u"general"_ustr },
    { Group_Writer, u"writer"_ustr },
    { Group_Html, u"html"_ustr },
    { Group_Calc, u"calc"_ustr },
    { Group_Draw, u"draw"_ustr },
    { Group_Basic, u"basic"_ustr },
    { Group_Sql, u"sql"_ustr }
};

// color config entry data (see ColorConfigWindow_Impl::Entry below)
struct
{
    // group
    Group eGroup;
    //checkbox (or simple text)
    OUString pText;
    //color listbox
    OUString pColor;
    // has checkbox?
    bool bCheckBox;
    //prop name
    std::u16string_view sPropName;
}
const vEntryInfo[] =
{
    #define IDS(Name) \
        u"" SAL_STRINGIFY(Name) ""_ustr, u"" SAL_STRINGIFY(Name##_lb) ""_ustr, false

    #define IDS_CB(Name) \
        u"" SAL_STRINGIFY(Name) ""_ustr, u"" SAL_STRINGIFY(Name##_lb) ""_ustr, true

    // The list of these entries (enum ColorConfigEntry) are in colorcfg.hxx.

    { Group_General, IDS(doccolor), std::u16string_view(u"/DocColor") },
    { Group_General, IDS_CB(docboundaries), std::u16string_view(u"/DocBoundaries") },
    { Group_General, IDS(appback), std::u16string_view(u"/AppBackground") },
    { Group_General, IDS_CB(objboundaries), std::u16string_view(u"/ObjectBoundaries") },
    { Group_General, IDS_CB(tblboundaries), std::u16string_view(u"/TableBoundaries") },
    { Group_General, IDS(font), std::u16string_view(u"/FontColor") },
    { Group_General, IDS_CB(unvisitedlinks), std::u16string_view(u"/Links") },
    { Group_General, IDS_CB(visitedlinks), std::u16string_view(u"/LinksVisited") },
    { Group_General, IDS(autospellcheck), std::u16string_view(u"/Spell") },
    { Group_General, IDS(grammarcheck), std::u16string_view(u"/Grammar") },
    { Group_General, IDS(smarttags), std::u16string_view(u"/SmartTags") },
    { Group_General, IDS_CB(shadows), std::u16string_view(u"/Shadow") },

    { Group_Writer,  IDS(writergrid), std::u16string_view(u"/WriterTextGrid") },
    { Group_Writer,  IDS_CB(field), std::u16string_view(u"/WriterFieldShadings") },
    { Group_Writer,  IDS_CB(index), std::u16string_view(u"/WriterIdxShadings") },
    { Group_Writer,  IDS(direct), std::u16string_view(u"/WriterDirectCursor") },
    { Group_Writer,  IDS(script), std::u16string_view(u"/WriterScriptIndicator") },
    { Group_Writer,  IDS_CB(section), std::u16string_view(u"/WriterSectionBoundaries") },
    { Group_Writer,  IDS(hdft), std::u16string_view(u"/WriterHeaderFooterMark") },
    { Group_Writer,  IDS(pagebreak), std::u16string_view(u"/WriterPageBreaks") },

    { Group_Html,    IDS(sgml), std::u16string_view(u"/HTMLSGML") },
    { Group_Html,    IDS(htmlcomment), std::u16string_view(u"/HTMLComment") },
    { Group_Html,    IDS(htmlkeyword), std::u16string_view(u"/HTMLKeyword") },
    { Group_Html,    IDS(unknown), std::u16string_view(u"/HTMLUnknown") },

    { Group_Calc,    IDS(calcgrid), std::u16string_view(u"/CalcGrid") },
    { Group_Calc,    IDS(calccellfocus), std::u16string_view(u"/CalcCellFocus") },
    { Group_Calc,    IDS(brk), std::u16string_view(u"/CalcPageBreak") },
    { Group_Calc,    IDS(brkmanual), std::u16string_view(u"/CalcPageBreakManual") },
    { Group_Calc,    IDS(brkauto), std::u16string_view(u"/CalcPageBreakAutomatic") },
    { Group_Calc,    IDS_CB(hiddencolrow), std::u16string_view(u"/CalcHiddenColRow") },
    { Group_Calc,    IDS_CB(textoverflow), std::u16string_view(u"/CalcTextOverflow") },
    { Group_Calc,    IDS(comments), std::u16string_view(u"/CalcComments") },
    { Group_Calc,    IDS(det), std::u16string_view(u"/CalcDetective") },
    { Group_Calc,    IDS(deterror), std::u16string_view(u"/CalcDetectiveError") },
    { Group_Calc,    IDS(ref), std::u16string_view(u"/CalcReference") },
    { Group_Calc,    IDS(notes), std::u16string_view(u"/CalcNotesBackground") },
    { Group_Calc,    IDS(values), std::u16string_view(u"/CalcValue") },
    { Group_Calc,    IDS(formulas), std::u16string_view(u"/CalcFormula") },
    { Group_Calc,    IDS(text), std::u16string_view(u"/CalcText") },
    { Group_Calc,    IDS(protectedcells), std::u16string_view(u"/CalcProtectedBackground") },

    { Group_Draw,    IDS(drawgrid), std::u16string_view(u"/DrawGrid") },

    { Group_Basic,   IDS(basiceditor), std::u16string_view(u"/BASICEditor") },
    { Group_Basic,   IDS(basicid), std::u16string_view(u"/BASICIdentifier") },
    { Group_Basic,   IDS(basiccomment), std::u16string_view(u"/BASICComment") },
    { Group_Basic,   IDS(basicnumber), std::u16string_view(u"/BASICNumber") },
    { Group_Basic,   IDS(basicstring), std::u16string_view(u"/BASICString") },
    { Group_Basic,   IDS(basicop), std::u16string_view(u"/BASICOperator") },
    { Group_Basic,   IDS(basickeyword), std::u16string_view(u"/BASICKeyword") },
    { Group_Basic,   IDS(error), std::u16string_view(u"/BASICError") },

    { Group_Sql,     IDS(sqlid), std::u16string_view(u"/SQLIdentifier") },
    { Group_Sql,     IDS(sqlnumber), std::u16string_view(u"/SQLNumber") },
    { Group_Sql,     IDS(sqlstring), std::u16string_view(u"/SQLString") },
    { Group_Sql,     IDS(sqlop), std::u16string_view(u"/SQLOperator") },
    { Group_Sql,     IDS(sqlkeyword), std::u16string_view(u"/SQLKeyword") },
    { Group_Sql,     IDS(sqlparam), std::u16string_view(u"/SQLParameter") },
    { Group_Sql,     IDS(sqlcomment), std::u16string_view(u"/SQLComment") }

    #undef IDS
};

// Maps the names of default color schemes to the corresponding TranslateId
const std::map<OUString, OUString> &getColorSchemes()
{
    static std::map<OUString, OUString> const vColorSchemes = {
        {"COLOR_SCHEME_LIBREOFFICE_AUTOMATIC", CuiResId(RID_COLOR_SCHEME_LIBREOFFICE_AUTOMATIC)},
    };
    return vColorSchemes;
};

// If the color scheme name has a translated string, then return the translation
// Or else simply return the input string
// For non-translatable color schemes, the ID and the name are the same
OUString lcl_SchemeIdToTranslatedName(const OUString& sSchemeId)
{
    auto it = getColorSchemes().find(sSchemeId);
    if (it != getColorSchemes().end())
        return it->second;
    return sSchemeId;
}

// Given a translated color scheme name, return the scheme ID used in the UI.xcu file
// For non-translatable color schemes, the ID and the name are the same
OUString lcl_TranslatedNameToSchemeId(const OUString& sName)
{
    for (auto it = getColorSchemes().begin(); it != getColorSchemes().end(); ++it)
        if (it->second == sName)
            return it->first;
    return sName;
}

// ColorConfigWindow_Impl

class ColorConfigWindow_Impl
{
public:
    explicit ColorConfigWindow_Impl(weld::Window* pTopLevel, weld::Container* pParent);

public:
    void SetLinks(Link<weld::Toggleable&,void> const&,
                  Link<ColorListBox&,void> const&,
                  Link<weld::Widget&,void> const&,
                  weld::ScrolledWindow& rScroll);
    void Update(EditableColorConfig const*, EditableExtendedColorConfig const*);
    void UpdateEntries();
    void ClickHdl(EditableColorConfig*, const weld::Toggleable&);
    void ColorHdl(EditableColorConfig*, EditableExtendedColorConfig*, const ColorListBox*);

    weld::Widget& GetWidget1()
    {
        return *m_xWidget1;
    }

    weld::Widget& GetWidget2()
    {
        return *m_xWidget2;
    }

    weld::Widget& GetBody()
    {
        return *m_xBox;
    }

    int GetLabelIndent() const
    {
        return m_nCheckBoxLabelOffset;
    }

private:
    // Chapter -- horizontal group separator stripe with text
    class Chapter
    {
        // text
        std::unique_ptr<weld::Label> m_xText;
    public:
        Chapter(weld::Builder& rBuilder, const OUString& pLabelWidget, bool bShow);
        void SetText(const OUString& rLabel) { m_xText->set_label(rLabel); }
    };

    // Entry -- a color config entry:
    // text (checkbox) + color list box
    struct Entry
    {
        Entry(weld::Window* pTopLevel, weld::Builder& rBuilder, const OUString& pTextWidget, const OUString& pColorWidget,
              const Color& rColor, int nCheckBoxLabelOffset, const ColorListBox* pCache, bool bCheckBox, bool bShow);
        void SetText(const OUString& rLabel) { dynamic_cast<weld::Label&>(*m_xText).set_label(rLabel); }
        int get_height_request() const
        {
            return std::max(m_xText->get_preferred_size().Height(),
                            m_xColorList->get_widget().get_preferred_size().Height());
        }
        void Hide();

        void SetLinks(Link<weld::Toggleable&,void> const&,
                      Link<ColorListBox&,void> const&,
                      Link<weld::Widget&,void> const&);
        void Update (ColorConfigValue const&, const std::u16string_view&,
            css::uno::Reference<css::configuration::XReadWriteAccess> const&);
        void Update (ExtendedColorConfigValue const&, const std::u16string_view&,
            css::uno::Reference<css::configuration::XReadWriteAccess> const&);
        void ColorChanged (ColorConfigValue&);
        void ColorChanged (ExtendedColorConfigValue&);

        bool Is(const weld::Toggleable* pBox) const { return m_xText.get() == pBox; }
        bool Is(const ColorListBox* pBox) const { return m_xColorList.get() == pBox; }

        // checkbox (CheckBox) or simple text (FixedText)
        std::unique_ptr<weld::Widget> m_xText;
        // color list box
        std::unique_ptr<ColorListBox> m_xColorList;
        // default color
        Color m_aDefaultColor;
    };

    css::uno::Reference<css::configuration::XReadWriteAccess> m_xReadWriteAccess;

private:
    weld::Window* m_pTopLevel;
    int m_nCheckBoxLabelOffset;
    std::unique_ptr<weld::Builder> m_xBuilder;
    std::unique_ptr<weld::Box> m_xBox;
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

} // namespace

// ColorConfigWindow_Impl::Chapter

// ctor for default groups
// rParent: parent window (ColorConfigWindow_Impl)
// eGroup: which group is this?
ColorConfigWindow_Impl::Chapter::Chapter(weld::Builder& rBuilder, const OUString& pLabelWidget, bool bShow)
    : m_xText(rBuilder.weld_label(pLabelWidget))
{
    if (!bShow)
        m_xText->hide();
}

// ColorConfigWindow_Impl::Entry
ColorConfigWindow_Impl::Entry::Entry(weld::Window* pTopLevel, weld::Builder& rBuilder,
                                     const OUString& pTextWidget, const OUString& pColorWidget,
                                     const Color& rColor, int nCheckBoxLabelOffset,
                                     const ColorListBox* pCache, bool bCheckBox, bool bShow)
    : m_xColorList(new ColorListBox(rBuilder.weld_menu_button(pColorWidget),
                                    [pTopLevel]{ return pTopLevel; }, pCache))
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
        m_xText->set_margin_start(m_xText->get_margin_start() +
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
void ColorConfigWindow_Impl::Entry::SetLinks(Link<weld::Toggleable&,void> const& rCheckLink,
                                             Link<ColorListBox&,void> const& rColorLink,
                                             Link<weld::Widget&,void> const& rGetFocusLink)
{
    m_xColorList->SetSelectHdl(rColorLink);
    m_xColorList->connect_focus_in(rGetFocusLink);
    if (weld::Toggleable* pCheckBox = dynamic_cast<weld::Toggleable*>(m_xText.get()))
    {
        pCheckBox->connect_toggled(rCheckLink);
        pCheckBox->connect_focus_in(rGetFocusLink);
    }
}

// updates a default color config entry
void ColorConfigWindow_Impl::Entry::Update(ColorConfigValue const& rValue, const std::u16string_view& rConfigPath,
    css::uno::Reference<css::configuration::XReadWriteAccess> const& xReadWriteAccess)
{
    Color aColor(rValue.nColor);
    m_xColorList->SelectEntry(aColor);

    bool bReadOnly = false;
    OUString aConfigPath = OUString::Concat(rConfigPath) + "/Color";
    if (xReadWriteAccess->hasPropertyByHierarchicalName(aConfigPath))
    {
        css::beans::Property aProperty = xReadWriteAccess->getPropertyByHierarchicalName(aConfigPath);
        bReadOnly = (aProperty.Attributes & css::beans::PropertyAttribute::READONLY) != 0;
    }
    m_xColorList->set_sensitive(!bReadOnly);

    if (weld::Toggleable* pCheckBox = dynamic_cast<weld::Toggleable*>(m_xText.get()))
    {
        bReadOnly = false;
        pCheckBox->set_active(rValue.bIsVisible);

        aConfigPath = OUString::Concat(rConfigPath) + "/IsVisible";
        if (xReadWriteAccess->hasPropertyByHierarchicalName(aConfigPath))
        {
            css::beans::Property aProperty = xReadWriteAccess->getPropertyByHierarchicalName(aConfigPath);
            bReadOnly = (aProperty.Attributes & css::beans::PropertyAttribute::READONLY) != 0;
        }
        pCheckBox->set_sensitive(!bReadOnly);
    }
}

// updates an extended color config entry
void ColorConfigWindow_Impl::Entry::Update(ExtendedColorConfigValue const& rValue, const std::u16string_view& rConfigPath,
    css::uno::Reference<css::configuration::XReadWriteAccess> const& xReadWriteAccess)
{
    Color aColor(rValue.getColor());
    if (rValue.getColor() == rValue.getDefaultColor())
        m_xColorList->SelectEntry(COL_AUTO);
    else
        m_xColorList->SelectEntry(aColor);

    bool bReadOnly = false;
    OUString aConfigPath = OUString::Concat(rConfigPath) + rValue.getName() + "/Color";
    if (xReadWriteAccess->hasPropertyByHierarchicalName(aConfigPath))
    {
        css::beans::Property aProperty = xReadWriteAccess->getPropertyByHierarchicalName(aConfigPath);
        bReadOnly = (aProperty.Attributes & css::beans::PropertyAttribute::READONLY) != 0;
    }
    m_xColorList->set_sensitive(!bReadOnly);
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
    , m_xBuilder(Application::CreateBuilder(pParent, u"cui/ui/colorconfigwin.ui"_ustr))
    , m_xBox(m_xBuilder->weld_box(u"ColorConfigWindow"_ustr))
    , m_xWidget1(m_xBuilder->weld_widget(u"docboundaries"_ustr))
    , m_xWidget2(m_xBuilder->weld_widget(u"docboundaries_lb"_ustr))
{
    css::uno::Reference < css::uno::XComponentContext > xContext(::comphelper::getProcessComponentContext());
    m_xReadWriteAccess = css::configuration::ReadWriteAccess::create(xContext, u"*"_ustr);

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
    {
        OUString sSampleText(u"XXXXXX"_ustr);
        std::unique_ptr<weld::CheckButton> xCheckBox(m_xBuilder->weld_check_button(u"docboundaries"_ustr));
        std::unique_ptr<weld::Label> xFixedText(m_xBuilder->weld_label(u"doccolor"_ustr));
        OUString sOrigCheck(xCheckBox->get_label());
        OUString sOrigFixed(xFixedText->get_label());
        xCheckBox->set_label(sSampleText);
        xFixedText->set_label(sSampleText);
        Size aCheckSize(xCheckBox->get_preferred_size());
        Size aFixedSize(xFixedText->get_preferred_size());
        xCheckBox->set_label(sOrigCheck);
        xFixedText->set_label(sOrigFixed);
        m_nCheckBoxLabelOffset = aCheckSize.Width() - aFixedSize.Width();
    }

    const ColorListBox* pCache = nullptr;

    // creating entries
    vEntries.reserve(ColorConfigEntryCount);
    for (size_t i = 0; i < std::size(vEntryInfo); ++i)
    {
        vEntries.push_back(std::make_shared<Entry>(m_pTopLevel, *m_xBuilder,
            vEntryInfo[i].pText, vEntryInfo[i].pColor,
            ColorConfig::GetDefaultColor(static_cast<ColorConfigEntry>(i)),
            m_nCheckBoxLabelOffset, pCache,
            vEntryInfo[i].bCheckBox,
            aModulesInstalled[vEntryInfo[i].eGroup]));
        if (!pCache)
            pCache = vEntries.back()->m_xColorList.get();
    }

    // extended entries
    ExtendedColorConfig aExtConfig;
    unsigned const nExtGroupCount = aExtConfig.GetComponentCount();
    if (!nExtGroupCount)
        return;

    for (unsigned j = 0; j != nExtGroupCount; ++j)
    {
        vExtBuilders.emplace_back(Application::CreateBuilder(m_xBox.get(), u"cui/ui/chapterfragment.ui"_ustr));
        vExtContainers.emplace_back(vExtBuilders.back()->weld_frame(u"ChapterFragment"_ustr));

        OUString const sComponentName = aExtConfig.GetComponentName(j);
        vChapters.push_back(std::make_shared<Chapter>(
            *vExtBuilders.back(), "chapter", true));
        vChapters.back()->SetText(aExtConfig.GetComponentDisplayName(sComponentName));

        vExtContainers.emplace_back(vExtBuilders.back()->weld_box(u"contents"_ustr));
        weld::Container* pChapterBox = vExtContainers.back().get();

        unsigned nColorCount = aExtConfig.GetComponentColorCount(sComponentName);
        for (unsigned i = 0; i != nColorCount; ++i)
        {
            vExtBuilders.emplace_back(Application::CreateBuilder(pChapterBox, u"cui/ui/colorfragment.ui"_ustr));
            vExtContainers.emplace_back(vExtBuilders.back()->weld_container(u"ColorFragment"_ustr));

            ExtendedColorConfigValue const aColorEntry =
                aExtConfig.GetComponentColorConfigValue(sComponentName, i);
            vEntries.push_back(std::make_shared<Entry>(m_pTopLevel, *vExtBuilders.back(),
                "label", "button", aColorEntry.getDefaultColor(),
                m_nCheckBoxLabelOffset, pCache, false, true));
            vEntries.back()->SetText(aColorEntry.getDisplayName());
        }
    }
}

// SetLinks()
void ColorConfigWindow_Impl::SetLinks(Link<weld::Toggleable&,void> const& aCheckLink,
                                      Link<ColorListBox&,void> const& aColorLink,
                                      Link<weld::Widget&,void> const& rGetFocusLink,
                                      weld::ScrolledWindow& rScroll)
{
    if (vEntries.empty())
        return;
    for (auto const & i: vEntries)
        i->SetLinks(aCheckLink, aColorLink, rGetFocusLink);
    // 6 is the spacing set on ColorConfigWindow
    rScroll.vadjustment_set_step_increment(vEntries[0]->get_height_request() + 6);
}

// Update()
void ColorConfigWindow_Impl::Update (
    EditableColorConfig const* pConfig,
    EditableExtendedColorConfig const* pExtConfig)
{
    // updating default entries
    std::optional<OUString> aUIColorSchemeName = officecfg::Office::UI::ColorScheme::CurrentColorScheme::get();
    OUString aUIColorSchemePath = officecfg::Office::UI::ColorScheme::ColorSchemes::path() + u"/" + aUIColorSchemeName.value();

    for (unsigned i = 0; i != ColorConfigEntryCount; ++i)
    {
        OUString sPath = aUIColorSchemePath + vEntryInfo[i].sPropName;
        ColorConfigEntry const aColorEntry = static_cast<ColorConfigEntry>(i);
        vEntries[i]->Update(
            pConfig->GetColorValue(aColorEntry),
            sPath,
            m_xReadWriteAccess
        );
    }

    // updating extended entries
    decltype(vEntries)::size_type i = ColorConfigEntryCount;
    unsigned const nExtCount = pExtConfig->GetComponentCount();
    for (unsigned j = 0; j != nExtCount; ++j)
    {
        OUString sComponentName = pExtConfig->GetComponentName(j);
        aUIColorSchemePath = officecfg::Office::ExtendedColorScheme::ExtendedColorScheme::ColorSchemes::path() + u"/" +
            aUIColorSchemeName.value() + u"/" + sComponentName + u"/Entries/";
        unsigned const nColorCount = pExtConfig->GetComponentColorCount(sComponentName);
        for (unsigned k = 0; i != vEntries.size() && k != nColorCount; ++i, ++k)
            vEntries[i]->Update(
                pExtConfig->GetComponentColorConfigValue(sComponentName, k),
                aUIColorSchemePath,
                m_xReadWriteAccess
            );
    }
}

void ColorConfigWindow_Impl::UpdateEntries()
{
    for (unsigned i = 0; i != ColorConfigEntryCount; ++i)
    {
        ColorConfigEntry const aEntry = static_cast<ColorConfigEntry>(i);
        Color aColor = ColorConfig::GetDefaultColor(aEntry);
        vEntries[i]->m_xColorList->SetAutoDisplayColor(aColor);
    }
}

// ClickHdl()
void ColorConfigWindow_Impl::ClickHdl(EditableColorConfig* pConfig, const weld::Toggleable& rBox)
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
    const ColorListBox* pBox)
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

    DECL_LINK(ClickHdl, weld::Toggleable&, void);
    DECL_LINK(ColorHdl, ColorListBox&, void);
    DECL_LINK(ControlFocusHdl, weld::Widget&, void);

public:
    explicit ColorConfigCtrl_Impl(weld::Window* pTopLevel, weld::Builder& rbuilder);

    void SetConfig (EditableColorConfig& rConfig) { pColorConfig = &rConfig; }
    void SetExtendedConfig (EditableExtendedColorConfig& rConfig) { pExtColorConfig = &rConfig; }
    void Update();
    void UpdateEntries();
    tools::Long GetScrollPosition() const
    {
        return m_xVScroll->vadjustment_get_value();
    }
    void SetScrollPosition(tools::Long nSet)
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
    int GetLabelIndent() const
    {
        return m_xScrollWindow->GetLabelIndent();
    }
};

ColorConfigCtrl_Impl::ColorConfigCtrl_Impl(weld::Window* pTopLevel, weld::Builder& rBuilder)
    : m_xVScroll(rBuilder.weld_scrolled_window(u"scroll"_ustr))
    , m_xBody(rBuilder.weld_container(u"colorconfig"_ustr))
    , m_xScrollWindow(std::make_unique<ColorConfigWindow_Impl>(pTopLevel, m_xBody.get()))
    , pColorConfig(nullptr)
    , pExtColorConfig(nullptr)
{
    m_xBody->set_stack_background();

    Link<weld::Toggleable&,void> aCheckLink = LINK(this, ColorConfigCtrl_Impl, ClickHdl);
    Link<ColorListBox&,void> aColorLink = LINK(this, ColorConfigCtrl_Impl, ColorHdl);
    Link<weld::Widget&,void> const& aGetFocusLink = LINK(this, ColorConfigCtrl_Impl, ControlFocusHdl);
    m_xScrollWindow->SetLinks(aCheckLink, aColorLink, aGetFocusLink, *m_xVScroll);
}

void ColorConfigCtrl_Impl::Update ()
{
    DBG_ASSERT(pColorConfig, "Configuration not set");
    m_xScrollWindow->Update(pColorConfig, pExtColorConfig);
}

void ColorConfigCtrl_Impl::UpdateEntries()
{
    m_xScrollWindow->UpdateEntries();
}

IMPL_LINK(ColorConfigCtrl_Impl, ClickHdl, weld::Toggleable&, rBox, void)
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

IMPL_LINK(ColorConfigCtrl_Impl, ControlFocusHdl, weld::Widget&, rCtrl, void)
{
    // determine whether a control is completely visible
    // and make it visible
    unsigned const nWinHeight = m_xVScroll->vadjustment_get_page_size();

    // calc visible area
    auto nThumbPos = m_xVScroll->vadjustment_get_value();
    int const nWinTop = nThumbPos;
    int const nWinBottom = nWinTop + nWinHeight;

    int x, nCtrlPosY, width, nHeight;
    rCtrl.get_extents_relative_to(m_xScrollWindow->GetBody(), x, nCtrlPosY, width, nHeight);

    int const nSelectedItemTop = nCtrlPosY;
    int const nSelectedItemBottom = nCtrlPosY + nHeight;
    bool const shouldScrollDown = nSelectedItemBottom >= nWinBottom;
    bool const shouldScrollUp = nSelectedItemTop <= nWinTop;
    bool const isNeedToScroll = shouldScrollDown || shouldScrollUp || nCtrlPosY < 0;

    if (!isNeedToScroll)
        return;

    if (shouldScrollDown)
    {
        int nOffset = nSelectedItemBottom - nWinBottom;
        nThumbPos += nOffset + 2;
    }
    else
    {
        int nOffset = nWinTop - nSelectedItemTop;
        nThumbPos -= nOffset + 2;
        if(nThumbPos < 0)
            nThumbPos = 0;
    }
    m_xVScroll->vadjustment_set_value(nThumbPos);
}

// SvxColorOptionsTabPage
SvxColorOptionsTabPage::SvxColorOptionsTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rCoreSet)
    : SfxTabPage(pPage, pController, u"cui/ui/optappearancepage.ui"_ustr, u"OptAppearancePage"_ustr, &rCoreSet)
    , bFillItemSetCalled(false)
    , m_nSizeAllocEventId(nullptr)
    , m_xAutoColorLB(m_xBuilder->weld_combo_box(u"autocolorlb"_ustr))
    , m_xAutoColorImg(m_xBuilder->weld_widget(u"lockautocolorlb"_ustr))
    , m_xColorSchemeLB(m_xBuilder->weld_combo_box(u"colorschemelb"_ustr))
    , m_xColorSchemeImg(m_xBuilder->weld_widget(u"lockcolorschemelb"_ustr))
    , m_xSaveSchemePB(m_xBuilder->weld_button(u"save"_ustr))
    , m_xDeleteSchemePB(m_xBuilder->weld_button(u"delete"_ustr))
    , m_xColorConfigCT(new ColorConfigCtrl_Impl(pController->getDialog(), *m_xBuilder))
    , m_xTable(m_xBuilder->weld_widget(u"table"_ustr))
    , m_xOnFT(m_xBuilder->weld_label(u"on"_ustr))
    , m_xColorFT(m_xBuilder->weld_label(u"colorsetting"_ustr))
    , m_rWidget1(m_xColorConfigCT->GetWidget1())
    , m_rWidget2(m_xColorConfigCT->GetWidget2())
{
    m_xColorSchemeLB->make_sorted();
    m_xColorSchemeLB->connect_changed(LINK(this, SvxColorOptionsTabPage, SchemeChangedHdl_Impl));
    m_xAutoColorLB->connect_changed(LINK(this, SvxColorOptionsTabPage, onAutoColorChanged));
    Link<weld::Button&,void> aLk = LINK(this, SvxColorOptionsTabPage, SaveDeleteHdl_Impl );
    m_xSaveSchemePB->connect_clicked(aLk);
    m_xDeleteSchemePB->connect_clicked(aLk);

    m_rWidget1.connect_size_allocate(LINK(this, SvxColorOptionsTabPage, AdjustHeaderBar));
    m_rWidget2.connect_size_allocate(LINK(this, SvxColorOptionsTabPage, AdjustHeaderBar));
}

SvxColorOptionsTabPage::~SvxColorOptionsTabPage()
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
    if (m_nSizeAllocEventId)
        Application::RemoveUserEvent(m_nSizeAllocEventId);
}

std::unique_ptr<SfxTabPage> SvxColorOptionsTabPage::Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rAttrSet)
{
    return std::make_unique<SvxColorOptionsTabPage>(pPage, pController, *rAttrSet);
}

OUString SvxColorOptionsTabPage::GetAllStrings()
{
    // buttons are excluded
    OUString sAllStrings;
    OUString labels[] = { u"label2"_ustr, u"label3"_ustr, u"autocolor"_ustr, u"uielements"_ustr, u"colorsetting"_ustr };

    for (const auto& label : labels)
    {
        if (const auto& pString = m_xBuilder->weld_label(label))
            sAllStrings += pString->get_label() + " ";
    }

    return sAllStrings.replaceAll("_", "");
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

    m_xAutoColorLB->set_active( MiscSettings::GetAppColorMode() );

    bool bReadOnly = officecfg::Office::ExtendedColorScheme::ExtendedColorScheme::CurrentColorScheme::isReadOnly() ||
        officecfg::Office::UI::ColorScheme::CurrentColorScheme::isReadOnly();
    m_xAutoColorLB->set_sensitive(!bReadOnly);
    m_xSaveSchemePB->set_sensitive(!bReadOnly);
    m_xDeleteSchemePB->set_sensitive(!bReadOnly);
    m_xAutoColorImg->set_visible(bReadOnly);

    OUString sUser = GetUserData();
    //has to be called always to speed up accessibility tools
    m_xColorConfigCT->SetScrollPosition(sUser.toInt32());
    m_xColorSchemeLB->clear();
    const uno::Sequence< OUString >  aSchemes = pColorConfig->GetSchemeNames();
    for(const OUString& s : aSchemes)
        m_xColorSchemeLB->append_text(lcl_SchemeIdToTranslatedName(s));

    m_xColorSchemeLB->set_active_text(lcl_SchemeIdToTranslatedName(pColorConfig->GetCurrentSchemeName()));
    m_xColorSchemeLB->set_sensitive(!officecfg::Office::Common::Misc::ApplicationAppearance::isReadOnly());
    m_xColorSchemeImg->set_visible(officecfg::Office::Common::Misc::ApplicationAppearance::isReadOnly());
    m_xColorSchemeLB->save_value();

    m_xDeleteSchemePB->set_sensitive( aSchemes.getLength() > 1 &&
        !officecfg::Office::ExtendedColorScheme::ExtendedColorScheme::CurrentColorScheme::isReadOnly() );
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

IMPL_LINK_NOARG(SvxColorOptionsTabPage, onAutoColorChanged, weld::ComboBox&, void)
{
    MiscSettings::SetAppColorMode( m_xAutoColorLB->get_active() );

    m_xColorConfigCT->UpdateEntries();

    pColorConfig->LoadScheme(lcl_TranslatedNameToSchemeId(m_xColorSchemeLB->get_active_text()));
    pExtColorConfig->LoadScheme(lcl_TranslatedNameToSchemeId(m_xColorSchemeLB->get_active_text()));
    UpdateColorConfig();
}

IMPL_LINK(SvxColorOptionsTabPage, SchemeChangedHdl_Impl, weld::ComboBox&, rBox, void)
{
    pColorConfig->LoadScheme(lcl_TranslatedNameToSchemeId(rBox.get_active_text()));
    pExtColorConfig->LoadScheme(lcl_TranslatedNameToSchemeId(rBox.get_active_text()));
    UpdateColorConfig();
}

IMPL_LINK(SvxColorOptionsTabPage, SaveDeleteHdl_Impl, weld::Button&, rButton, void)
{
    if (m_xSaveSchemePB.get() == &rButton)
    {
        OUString sName;

        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        ScopedVclPtr<AbstractSvxNameDialog> aNameDlg(pFact->CreateSvxNameDialog(GetFrameWeld(),
                            sName, CuiResId(RID_CUISTR_COLOR_CONFIG_SAVE2) ));
        aNameDlg->SetCheckNameHdl( LINK(this, SvxColorOptionsTabPage, CheckNameHdl_Impl));
        aNameDlg->SetText(CuiResId(RID_CUISTR_COLOR_CONFIG_SAVE1));
        aNameDlg->SetHelpId(HID_OPTIONS_COLORCONFIG_SAVE_SCHEME);
        aNameDlg->SetCheckNameHdl( LINK(this, SvxColorOptionsTabPage, CheckNameHdl_Impl));
        if(RET_OK == aNameDlg->Execute())
        {
            sName = aNameDlg->GetName();
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
        std::unique_ptr<weld::MessageDialog> xQuery(Application::CreateMessageDialog(GetFrameWeld(),
                                                    VclMessageType::Question, VclButtonsType::YesNo,
                                                    CuiResId(RID_CUISTR_COLOR_CONFIG_DELETE)));
        xQuery->set_title(CuiResId(RID_CUISTR_COLOR_CONFIG_DELETE_TITLE));
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
    OUString sName = rDialog.GetName();
    return !sName.isEmpty() && m_xColorSchemeLB->find_text(sName) == -1;
}

void SvxColorOptionsTabPage::FillUserData()
{
    SetUserData(OUString::number(m_xColorConfigCT->GetScrollPosition()));
}

IMPL_LINK_NOARG(SvxColorOptionsTabPage, AdjustHeaderBar, const Size&, void)
{
    if (m_nSizeAllocEventId)
        return;
    m_nSizeAllocEventId = Application::PostUserEvent(LINK(this, SvxColorOptionsTabPage, PostAdjustHeaderBar));
}

IMPL_LINK_NOARG(SvxColorOptionsTabPage, PostAdjustHeaderBar, void*, void)
{
    m_nSizeAllocEventId = nullptr;

    // horizontal positions
    int nX1, nX2, nX3, y, width, height;
    if (!m_rWidget1.get_extents_relative_to(*m_xTable, nX1, y, width, height))
        return;
    if (!m_rWidget2.get_extents_relative_to(*m_xTable, nX2, y, width, height))
        return;
    if (!m_xTable->get_extents_relative_to(*m_xTable, nX3, y, width, height))
        return;

    // 6 is the column-spacing of the parent grid of these labels
    auto nTextWidth1 = nX1 + m_xColorConfigCT->GetLabelIndent() - 6;
    m_xOnFT->set_size_request(nTextWidth1, -1);
    auto nTextWidth3 = width - nX2;
    m_xColorFT->set_size_request(nTextWidth3, -1);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
