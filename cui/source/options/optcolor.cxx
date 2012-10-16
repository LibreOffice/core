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
#include "optcolor.hrc"
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
    // resource id for the title text
    int nTextResId;
}
const vGroupInfo[] =
{
    // the groups are in the same order as in enum Group above
    { FT_GENERAL },
    { FT_WRITER },
    { FT_HTML },
    { FT_CALC },
    { FT_DRAW },
    { FT_BASIC },
    { FT_SQL_COMMAND }, // FIXME
};

// color config entry data (see ColorConfigWindow_Impl::Entry below)
struct
{
    // group
    Group eGroup;

    // help id for ...
    //     color listbox
    char const* sColorListHid;
    //     checkbox
    char const* sCheckBoxHid;

    // resource id for ...
    //     checkbox (or simple text)
    int nTextResId;
    //     color listbox
    int nColorListResId;
    //     preview box
    int nPreviewResId;

    // has checkbox?
    bool bCheckBox;
}
const vEntryInfo[] =
{
    // These macros simplify the list of constants.
    // There is a unique name that is the same in the ids of the same entry
    // (except one).
    // Entries with and without checkboxes need different macros:
    #define IDS(Name) \
        HID_COLORPAGE_##Name##_LB, HID_COLORPAGE_##Name##_CB, \
        FT_##Name, LB_##Name, WN_##Name, false
    #define IDS_CB(Name) \
        HID_COLORPAGE_##Name##_LB, HID_COLORPAGE_##Name##_CB, \
        CB_##Name, LB_##Name, WN_##Name, true

    // resolve different names
    #define CB_DOCBOUNDARIES CB_DOCBOUND
    #define LB_DOCBOUNDARIES LB_DOCBOUND
    #define WN_DOCBOUNDARIES WN_DOCBOUND
    #define FT_CALCPAGEBREAKAUTOMATIC FT_CALCPAGEBREAKAUTO
    #define LB_CALCPAGEBREAKAUTOMATIC LB_CALCPAGEBREAKAUTO
    #define WN_CALCPAGEBREAKAUTOMATIC WN_CALCPAGEBREAKAUTO

    // The list of these entries (enum ColorConfigEntry) are in colorcfg.hxx.

    { Group_General, IDS(DOCCOLOR) },
    { Group_General, IDS_CB(DOCBOUNDARIES) },
    { Group_General, IDS(APPBACKGROUND) },
    { Group_General, IDS_CB(OBJECTBOUNDARIES) },
    { Group_General, IDS_CB(TABLEBOUNDARIES) },
    { Group_General, IDS(FONTCOLOR) },
    { Group_General, IDS_CB(LINKS) },
    { Group_General, IDS_CB(LINKSVISITED) },
    { Group_General, IDS(SPELL) },
    { Group_General, IDS(SMARTTAGS) },
    { Group_General, IDS_CB(SHADOWCOLOR) },
    { Group_Writer,  IDS(WRITERTEXTGRID) },
    { Group_Writer,  IDS_CB(WRITERFIELDSHADINGS) },
    { Group_Writer,  IDS_CB(WRITERIDXSHADINGS) },
    { Group_Writer,  IDS(WRITERDIRECTCURSOR) },
    { Group_Writer,  IDS(WRITERSCRIPTINDICATOR) },
    { Group_Writer,  IDS_CB(WRITERSECTIONBOUNDARIES) },
    { Group_Writer,  IDS(WRITERHEADERFOOTERMARK) },
    { Group_Writer,  IDS(WRITERPAGEBREAKS) },
    { Group_Html,    IDS(HTMLSGML) },
    { Group_Html,    IDS(HTMLCOMMENT) },
    { Group_Html,    IDS(HTMLKEYWORD) },
    { Group_Html,    IDS(HTMLUNKNOWN) },
    { Group_Calc,    IDS(CALCGRID) },
    { Group_Calc,    IDS(CALCPAGEBREAK) },
    { Group_Calc,    IDS(CALCPAGEBREAKMANUAL) },
    { Group_Calc,    IDS(CALCPAGEBREAKAUTOMATIC) },
    { Group_Calc,    IDS(CALCDETECTIVE) },
    { Group_Calc,    IDS(CALCDETECTIVEERROR) },
    { Group_Calc,    IDS(CALCREFERENCE) },
    { Group_Calc,    IDS(CALCNOTESBACKGROUND) },
    { Group_Draw,    IDS(DRAWGRID) },
    { Group_Basic,   IDS(BASICIDENTIFIER) },
    { Group_Basic,   IDS(BASICCOMMENT) },
    { Group_Basic,   IDS(BASICNUMBER) },
    { Group_Basic,   IDS(BASICSTRING) },
    { Group_Basic,   IDS(BASICOPERATOR) },
    { Group_Basic,   IDS(BASICKEYWORD) },
    { Group_Basic,   IDS(BASICERROR) },
    { Group_Sql,     IDS(SQLIDENTIFIER) },
    { Group_Sql,     IDS(SQLNUMBER) },
    { Group_Sql,     IDS(SQLSTRING) },
    { Group_Sql,     IDS(SQLOPERATOR) },
    { Group_Sql,     IDS(SQLKEYWORD) },
    { Group_Sql,     IDS(SQLPARAMETER) },
    { Group_Sql,     IDS(SQLCOMMENT) },

    #undef IDS_CB
    #undef IDS
};

} // namespace


//
// SvxExtFixedText_Impl
//

class SvxExtFixedText_Impl : public FixedText
{
private:
    long m_nGroupHeight;

protected:
    virtual void DataChanged (DataChangedEvent const& rDCEvt);

public:
    SvxExtFixedText_Impl (Window* pParent, ResId const& rResId) :
        FixedText(pParent, rResId), m_nGroupHeight(0)
    { }

    long GetGroupHeight () const { return m_nGroupHeight; }
    void SetGroupHeight (long nHeight) { m_nGroupHeight = nHeight; }
};

void SvxExtFixedText_Impl::DataChanged (DataChangedEvent const& rDCEvt)
{
    FixedText::DataChanged(rDCEvt);
    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
         (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        Font aFont = GetFont();
        aFont.SetWeight(WEIGHT_BOLD);
        SetFont(aFont);
        SetBackground(Wallpaper(Color(COL_TRANSPARENT)));
    }
}


//
// ColorConfigWindow_Impl
//

class ColorConfigWindow_Impl : public Window
{
public:
    ColorConfigWindow_Impl (Window* pParent, ResId const& rResId);
    ~ColorConfigWindow_Impl ();

public:
    void SetLinks (Link const&, Link const&, Link const&);
    unsigned GetEntryHeight () const { return vEntries[0]->GetHeight(); }
    void Update (EditableColorConfig const*, EditableExtendedColorConfig const*);
    void ScrollHdl (long& nScrollPos, ScrollBar const&);
    void ClickHdl (EditableColorConfig*, CheckBox*);
    void ColorHdl (EditableColorConfig*, EditableExtendedColorConfig*, ColorListBox*);
    void SetHeaderBar (HeaderBar&, ScrollBar const&, ResMgr&);
    void SetScrollBar (ScrollBar&);


private:
    // Chapter -- horizontal group separator stripe with text
    class Chapter
    {
        // parent window
        ColorConfigWindow_Impl& rParent;
        // gray (?) stripe
        Window aBackground;
        // text
        SvxExtFixedText_Impl aText;

    public:
        Chapter (ColorConfigWindow_Impl& rParent, Group, ResMgr&);
        Chapter (ColorConfigWindow_Impl& rParent, ResMgr&, unsigned nYPos, rtl::OUString const& sDisplayName);
    public:
        void Show (Wallpaper const& rBackWall);
        void Hide ();
        void MoveVertically (long nOffset);
        void SetBackground (Wallpaper const& W) { aBackground.SetBackground(W); }
        long GetHeight () const { return aText.GetGroupHeight(); }
        void SetHeight (long nHeight) { aText.SetGroupHeight(nHeight); }
        long GetLeft () const { return aText.GetPosPixel().X(); }
        long GetTop () const { return aText.GetPosPixel().Y(); }
    };

    // Entry -- a color config entry:
    // text (checkbox) + color list box + preview box
    class Entry
    {
    public:
        Entry (Window& rParent, unsigned iEntry, ResMgr&);
        Entry (Window& rParent, ResMgr&, unsigned nYPos, ExtendedColorConfigValue const& aColorEntry);
    public:
        void MoveVertically (long nOffset);
        bool MoveAndShow (long nOffset, long nMaxVisible, bool bShow);
        void Show ();
        void Hide ();
        void SetAppearance (unsigned iEntry, Wallpaper const& aTextWall, ColorListBox const& aSampleList);
        void SetTextColor (Color C) { pText->SetTextColor(C); }
    public:
        void SetLinks (Link const&, Link const&, Link const&);
        void SetHeader (ColorConfigWindow_Impl const& rParent, HeaderBar&, ResMgr&) const;
        void Update (ColorConfigEntry, ColorConfigValue const&);
        void Update (ExtendedColorConfigValue const&);
        void ColorChanged (ColorConfigEntry, ColorConfigValue&);
        void ColorChanged (ExtendedColorConfigValue&);
    public:
        long GetTop () const { return aPreview.GetPosPixel().Y(); }
        long GetBottom () const { return GetTop() + aPreview.GetSizePixel().Height(); }
        unsigned GetHeight () const { return aColorList.GetSizePixel().Height(); }
    public:
        bool Is (CheckBox* pBox) const { return pText.get() == pBox; }
        bool Is (ColorListBox* pBox) const { return &aColorList == pBox; }

    private:
        // checkbox (CheckBox) or simple text (FixedText)
        boost::shared_ptr<Control> pText;
        // color list box
        ColorListBox aColorList;
        // color preview box
        Window aPreview;
        // default color
        Color aDefaultColor;

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
    // initialization
    void CreateEntries (ResMgr&);
    void SetAppearance ();

private:
    long GetDeltaAbove (Group) const;

    virtual void Command (CommandEvent const& rCEvt);
    virtual void DataChanged (DataChangedEvent const& rDCEvt);

    unsigned GetPosBehindLastChapter () const;

    bool IsGroupVisible (Group) const;
};

namespace
{

// entry -> group
Group GetGroup (unsigned nEntry)
{
    if (nEntry >= ColorConfigEntryCount)
        return nGroupCount; // feature of an extension
    return vEntryInfo[nEntry].eGroup;
}

// moves a window vertically
void MoveVertically (Window& rWin, long nOffset)
{
    if (nOffset)
    {
        Point aPos = rWin.GetPosPixel();
        aPos.Y() += nOffset;
        rWin.SetPosPixel(aPos);
    }
}

// moves a window vertically and optionally shows it
bool MoveAndShow (Window& rWin, long nOffset, long nMaxVisible, bool bShow)
{
    // moves
    Point aPos = rWin.GetPosPixel();
    aPos.Y() += nOffset;
    rWin.SetPosPixel(aPos);
    // shows only if it is really visible
    if (bShow)
        bShow = aPos.Y() <= nMaxVisible && aPos.Y() + rWin.GetSizePixel().Height() >= 0;
    rWin.Show(bShow);
    return bShow;
}

} // namespace



//
// ColorConfigWindow_Impl::Chapter
//

// ctor for default groups
// rParent: parent window (ColorConfigWindow_Impl)
// eGroup: which group is this?
// rResMgr: resource manager
ColorConfigWindow_Impl::Chapter::Chapter (
    ColorConfigWindow_Impl& Parent, Group eGroup, ResMgr& rResMgr
) :
    rParent(Parent),
    aBackground(&rParent),
    aText(&rParent, ResId(vGroupInfo[eGroup].nTextResId, rResMgr))
{ }

// ctor for extended groups
ColorConfigWindow_Impl::Chapter::Chapter (
    ColorConfigWindow_Impl& Parent, ResMgr& rResMgr,
    unsigned nYPos, rtl::OUString const& sDisplayName
) :
    rParent(Parent),
    aBackground(&rParent),
    aText(&rParent, ResId(FT_SQL_COMMAND, rResMgr))
{
    Point const aTextPos = rParent.LogicToPixel(Point(FT_XPOS, nYPos), MAP_APPFONT);
    Size const aTextSize = rParent.LogicToPixel(Size(FT_WIDTH, SEP_HEIGHT), MAP_APPFONT);
    aText.SetPosSizePixel(aTextPos, aTextSize);
    aText.SetText(sDisplayName);
}


void ColorConfigWindow_Impl::Chapter::Show (Wallpaper const& rBackWall)
{
    { // background
        Point const aBgPos(
            rParent.LogicToPixel(Point(0, 0), MAP_APPFONT).X(),
            aText.GetPosPixel().Y()
        );
        Size const aBgSize(
            rParent.GetSizePixel().Width(),
            rParent.LogicToPixel(Size(0, SEP_HEIGHT), MAP_APPFONT).Height()
        );
        aBackground.SetPosSizePixel(aBgPos, aBgSize);
        aBackground.SetBackground(rBackWall);
        aBackground.Show();
    }

    { // text
        Font aFont = aText.GetFont();
        aFont.SetWeight(WEIGHT_BOLD);
        aText.SetFont(aFont);
        aText.SetBackground(rBackWall);
        aText.Show();
        aText.SetZOrder(0, WINDOW_ZORDER_FIRST);
    }
}

void ColorConfigWindow_Impl::Chapter::Hide ()
{
    aBackground.Hide();
    aText.Hide();
}

// moves the chapter title vertically by nOffset pixels
void ColorConfigWindow_Impl::Chapter::MoveVertically (long nOffset)
{
    ::MoveVertically(aBackground, nOffset);
    ::MoveVertically(aText, nOffset);
}


//
// ColorConfigWindow_Impl::Entry
//

// ctor for default entries
// pParent: parent window (ColorConfigWindow_Impl)
// iEntry: which entry is this? (in the vEntryInfo[] array above)
// rResMgr: resource manager
ColorConfigWindow_Impl::Entry::Entry (
    Window& rParent, unsigned iEntry, ResMgr& rResMgr
) :
    aColorList(&rParent, ResId(vEntryInfo[iEntry].nColorListResId, rResMgr)),
    aPreview(&rParent, ResId(vEntryInfo[iEntry].nPreviewResId, rResMgr)),
    aDefaultColor(ColorConfig::GetDefaultColor(static_cast<ColorConfigEntry>(iEntry)))
{
    // has checkbox?
    if (vEntryInfo[iEntry].bCheckBox)
    {
        pText = boost::shared_ptr<CheckBox>( new CheckBox (
            &rParent, ResId(vEntryInfo[iEntry].nTextResId, rResMgr)
        ) );
    }
    else
    {
        pText = boost::shared_ptr<FixedText>( new FixedText (
            &rParent, ResId(vEntryInfo[iEntry].nTextResId, rResMgr)
        ) );
    }
}

// ctor for extended entries
ColorConfigWindow_Impl::Entry::Entry (
    Window& rParent, ResMgr& rResMgr,
    unsigned nYPos, ExtendedColorConfigValue const& aColorEntry
) :
    pText(boost::shared_ptr<FixedText>(new FixedText (&rParent, ResId(FT_BASICERROR, rResMgr)))),
    aColorList(&rParent, ResId(LB_BASICERROR, rResMgr)),
    aPreview(&rParent, ResId(WN_BASICERROR, rResMgr)),
    aDefaultColor(aColorEntry.getDefaultColor())
{
    { // text (no checkbox)
        FixedText* const pFixedText = static_cast<FixedText*>(pText.get());
        Point const aTextPos = rParent.LogicToPixel(Point(FT_XPOS, nYPos), MAP_APPFONT);
        Size const aTextSize = rParent.LogicToPixel(Size(FT_WIDTH, FT_HEIGHT), MAP_APPFONT);
        pFixedText->SetPosSizePixel(aTextPos, aTextSize);
        pFixedText->SetText(aColorEntry.getDisplayName());
    }
    { // color listbox
        Point const aListPos = rParent.LogicToPixel(Point(LB_XPOS, nYPos), MAP_APPFONT);
        Size const aListSize = rParent.LogicToPixel(Size(LB_WIDTH, LB_HEIGHT), MAP_APPFONT);
        aColorList.SetPosSizePixel(aListPos, aListSize);
    }
    { // preview box
        Point const aPreviewPos = rParent.LogicToPixel(Point(WN_XPOS, nYPos), MAP_APPFONT);
        Size const aPreviewSize = rParent.LogicToPixel(Size(WN_WIDTH, WN_HEIGHT), MAP_APPFONT);
        aPreview.SetPosSizePixel(aPreviewPos, aPreviewSize);
    }
}

// moves entry vertically by nOffset pixels
void ColorConfigWindow_Impl::Entry::MoveVertically (long nOffset)
{
    // moving all components
    ::MoveVertically(*pText, nOffset);
    ::MoveVertically(aColorList, nOffset);
    ::MoveVertically(aPreview, nOffset);
}

// moves and shows
bool ColorConfigWindow_Impl::Entry::MoveAndShow (long nOffset, long nMaxVisible, bool bShow)
{
    // if any of the items on the current line is visible, the
    // whole line should be visible
    bool bRes = false;
    bRes = ::MoveAndShow(*pText,     nOffset, nMaxVisible, bShow) || bRes;
    bRes = ::MoveAndShow(aColorList, nOffset, nMaxVisible, bShow) || bRes;
    bRes = ::MoveAndShow(aPreview,   nOffset, nMaxVisible, bShow) || bRes;
    return bRes;
}

void ColorConfigWindow_Impl::Entry::Show ()
{
    pText->Show();
    aColorList.Show();
    aPreview.Show();
}

void ColorConfigWindow_Impl::Entry::Hide ()
{
    pText->Hide();
    aColorList.Hide();
    aPreview.Hide();
}

// SetAppearance()
// iEntry: which entry is this?
// aTextWall: background of the text (transparent)
// aSampleList: sample color listbox (to copy from)
void ColorConfigWindow_Impl::Entry::SetAppearance (
    unsigned iEntry, Wallpaper const& aTextWall,
    ColorListBox const& aSampleList
) {
    // text (and optionally checkbox)
    pText->SetBackground(aTextWall);
    if (CheckBox* pCheckBox = dynamic_cast<CheckBox*>(pText.get()))
        pCheckBox->SetHelpId(vEntryInfo[iEntry].sCheckBoxHid);
    // preview
    aPreview.SetBorderStyle(WINDOW_BORDER_MONO);
    // color list
    aColorList.CopyEntries(aSampleList);
    aColorList.InsertAutomaticEntryColor(aDefaultColor);
    if (iEntry < ColorConfigEntryCount)
        aColorList.SetHelpId(vEntryInfo[iEntry].sColorListHid);
}

// SetLinks()
void ColorConfigWindow_Impl::Entry::SetLinks (
    Link const& aCheckLink, Link const& aColorLink, Link const& aGetFocusLink
) {
    aColorList.SetSelectHdl(aColorLink);
    aColorList.SetGetFocusHdl(aGetFocusLink);
    if (CheckBox* pCheckBox = dynamic_cast<CheckBox*>(pText.get()))
    {
        pCheckBox->SetClickHdl(aCheckLink);
        pCheckBox->SetGetFocusHdl(aGetFocusLink);
    }
}

// fills the header bar
void ColorConfigWindow_Impl::Entry::SetHeader (
    ColorConfigWindow_Impl const& rParent,
    HeaderBar& rHeader, ResMgr& rResMgr
) const {
    // title strings
    String const sTitle[] =
    {
        ResId(ST_ON, rResMgr),
        ResId(ST_UIELEM, rResMgr),
        ResId(ST_COLSET, rResMgr),
        ResId(ST_PREVIEW, rResMgr),
    };
    // horizontal positions
    unsigned const nX0 = 0;
    unsigned const nX1 = rParent.vChapters.front()->GetLeft();
    unsigned const nX2 = aColorList.GetPosPixel().X();
    unsigned const nX3 = aPreview.GetPosPixel().X();
    unsigned const nX4 = rHeader.GetSizePixel().Width();
    // filling
    WinBits const nHeadBits = HIB_VCENTER | HIB_FIXED | HIB_FIXEDPOS;
    rHeader.InsertItem(1, sTitle[0], nX1 - nX0, nHeadBits | HIB_CENTER);
    rHeader.InsertItem(2, sTitle[1], nX2 - nX1, nHeadBits | HIB_LEFT);
    rHeader.InsertItem(3, sTitle[2], nX3 - nX2, nHeadBits | HIB_LEFT);
    rHeader.InsertItem(4, sTitle[3], nX4 - nX3, nHeadBits | HIB_LEFT);
}

// updates a default color config entry
void ColorConfigWindow_Impl::Entry::Update (
    ColorConfigEntry aColorEntry, ColorConfigValue const& rValue
) {
    Color aColor;
    if ((unsigned)rValue.nColor == COL_AUTO)
    {
        aColor = ColorConfig::GetDefaultColor(aColorEntry);
        aColorList.SelectEntryPos(0);
    }
    else
    {
        aColor = Color(rValue.nColor);
        aColorList.SelectEntry(aColor);
    }
    aPreview.SetBackground(Wallpaper(aColor));
    if (CheckBox* pCheckBox = dynamic_cast<CheckBox*>(pText.get()))
        pCheckBox->Check(rValue.bIsVisible);
}

// updates an extended color config entry
void ColorConfigWindow_Impl::Entry::Update (
    ExtendedColorConfigValue const& rValue
) {
    Color aColor(rValue.getColor());
    if (rValue.getColor() == rValue.getDefaultColor())
        aColorList.SelectEntryPos(0);
    else
        aColorList.SelectEntry(aColor);
    SetColor(aColor);
}

// color of a default entry has changed
void ColorConfigWindow_Impl::Entry::ColorChanged (
    ColorConfigEntry aColorEntry,
    ColorConfigValue& rValue
) {
    Color aColor;
    if (aColorList.IsAutomaticSelected())
    {
        aColor = ColorConfig::GetDefaultColor(aColorEntry);
        rValue.nColor = COL_AUTO;
    }
    else
    {
        aColor = aColorList.GetSelectEntryColor();
        rValue.nColor = aColor.GetColor();
    }
    SetColor(aColor);
}

// color of an extended entry has changed
void ColorConfigWindow_Impl::Entry::ColorChanged (
    ExtendedColorConfigValue& rValue
) {
    Color aColor = aColorList.GetSelectEntryColor();
    rValue.setColor(aColor.GetColor());
    // automatic?
    if (aColorList.GetSelectEntryPos() == 0)
    {
        rValue.setColor(rValue.getDefaultColor());
        aColor.SetColor(rValue.getColor());
    }
    SetColor(aColor);
}

void ColorConfigWindow_Impl::Entry::SetColor (Color aColor)
{
    aPreview.SetBackground(Wallpaper(aColor));
    aPreview.Invalidate();
}


//
// ColorConfigWindow_Impl
//

ColorConfigWindow_Impl::ColorConfigWindow_Impl (Window* pParent, ResId const& rResId) :
    Window(pParent, rResId)
{
    CreateEntries(*rResId.GetResMgr());
    Resource::FreeResource();
    SetAppearance();
}

void ColorConfigWindow_Impl::CreateEntries (ResMgr& rResMgr)
{
    // creating group headers
    vChapters.reserve(nGroupCount);
    for (unsigned i = 0; i != nGroupCount; ++i)
    {
        vChapters.push_back(boost::shared_ptr<Chapter> (
            new Chapter( *this, static_cast<Group>(i), rResMgr ) ) );
    }

    // creating entries
    vEntries.reserve(ColorConfigEntryCount);
    for (unsigned i = 0; i != ColorConfigEntryCount; ++i)
        vEntries.push_back( boost::shared_ptr<Entry>(new Entry (*this, i, rResMgr) ) );

    // calculate heights of groups which can be hidden
    {
        unsigned nNextY = GetPosBehindLastChapter(); // next Y coordinate
        for (int i = nGroupCount - 1; i >= 0; --i)
        {
            unsigned nY = vChapters[i]->GetTop();
            vChapters[i]->SetHeight(nNextY - nY);
            nNextY = nY;
        }
    }

    // extended entries
    ExtendedColorConfig aExtConfig;
    if (unsigned const nExtGroupCount = aExtConfig.GetComponentCount())
    {
        unsigned nLineNum = ( GetPosBehindLastChapter() /
            LogicToPixel(Size(0, LINE_HEIGHT), MAP_APPFONT).Height() ) + 1;
        for (unsigned j = 0; j != nExtGroupCount; ++j)
        {
            rtl::OUString const sComponentName = aExtConfig.GetComponentName(j);
            vChapters.push_back(boost::shared_ptr<Chapter>(new Chapter (
                *this, rResMgr, nLineNum * LINE_HEIGHT,
                aExtConfig.GetComponentDisplayName(sComponentName)
            )));
            ++nLineNum;
            unsigned nColorCount = aExtConfig.GetComponentColorCount(sComponentName);
            for (unsigned i = 0; i != nColorCount; ++i)
            {
                ExtendedColorConfigValue const aColorEntry =
                    aExtConfig.GetComponentColorConfigValue(sComponentName, i);
                vEntries.push_back(boost::shared_ptr<Entry>( new Entry (
                    *this, rResMgr, nLineNum * LINE_HEIGHT, aColorEntry
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
    for (unsigned i = 0; i != vChapters.size(); ++i)
    {
        if (IsGroupVisible(static_cast<Group>(i)))
            vChapters[i]->Show(aBackWall);
        else
            vChapters[i]->Hide();
    }
    SetBackground(Wallpaper(rStyleSettings.GetFieldColor()));
    SetHelpId(HID_OPTIONS_COLORCONFIG_COLORLIST_WIN);

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
        for (unsigned i = 0; i != vEntries.size(); ++i)
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

    // positioning and appearance
    Group eGroup = Group_Unknown;
    for (unsigned i = 0; i != vEntries.size(); ++i)
    {
        Group const eNewGroup = GetGroup(i);
        bool const bShow = IsGroupVisible(eNewGroup);
        long const nDelta = bShow ? -GetDeltaAbove(eNewGroup) : 0;

        // new group?
        if (eNewGroup > eGroup)
        {
            eGroup = eNewGroup;
            if (bShow)
                vChapters[eGroup]->MoveVertically(nDelta);
        }
        // positioning
        if (bShow)
            vEntries[i]->MoveVertically(nDelta);
        else
            vEntries[i]->Hide();
        // appearance
        vEntries[i]->SetAppearance(i, aTransparentWall, aSampleColorList);
    }
}


ColorConfigWindow_Impl::~ColorConfigWindow_Impl ()
{ }

void ColorConfigWindow_Impl::SetHeaderBar (
    HeaderBar& rHeaderBar, ScrollBar const& rVScroll, ResMgr& rResMgr
) {
    rHeaderBar.SetPosSizePixel(
        Point(0, 0),
        Size(GetParent()->GetOutputSizePixel().Width(), rVScroll.GetPosPixel().Y())
    );
    vEntries.front()->SetHeader(*this, rHeaderBar, rResMgr);
    rHeaderBar.Show();
}

void ColorConfigWindow_Impl::SetScrollBar (ScrollBar& rVScroll)
{
    rVScroll.EnableDrag();
    rVScroll.Show();
    rVScroll.SetRangeMin(0);
    unsigned const nScrollOffset =
        vEntries[1]->GetTop() - vEntries[0]->GetTop();
    unsigned const nVisibleEntries = GetSizePixel().Height() / nScrollOffset;

    rVScroll.SetRangeMax(vEntries.size() + vChapters.size());
    { // dynamic: calculate the hidden lines
        unsigned nInvisibleLines = 0;
        Group eGroup = Group_Unknown;
        for (unsigned i = 0; i != vEntries.size(); ++i)
        {
            Group const eNewGroup = GetGroup(i);
            bool const bVisible = IsGroupVisible(eNewGroup);
            if (!bVisible)
                nInvisibleLines++;
            if (eNewGroup > eGroup)
            {
                eGroup = eNewGroup;
                if (!bVisible)
                    nInvisibleLines++;
            }
        }
        rVScroll.SetRangeMax(rVScroll.GetRangeMax() - nInvisibleLines);
    }

    rVScroll.SetPageSize(nVisibleEntries - 1);
    rVScroll.SetVisibleSize(nVisibleEntries);
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
    EditableExtendedColorConfig const* pExtConfig
) {
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
        rtl::OUString sComponentName = pExtConfig->GetComponentName(j);
        unsigned const nColorCount = pExtConfig->GetComponentColorCount(sComponentName);
        for (unsigned k = 0; i != vEntries.size() && k != nColorCount; ++i, ++k)
            vEntries[i]->Update(
                pExtConfig->GetComponentColorConfigValue(sComponentName, k)
            );
    }
}

// ScrollHdl()
void ColorConfigWindow_Impl::ScrollHdl (long& nScrollPos, ScrollBar const& rVScroll)
{
    SetUpdateMode(true);
    long const nOffset =
        (vEntries[1]->GetTop() - vEntries[0]->GetTop()) *
        (nScrollPos - rVScroll.GetThumbPos());
    nScrollPos = rVScroll.GetThumbPos();
    long const nWindowHeight = GetSizePixel().Height();
    int nFirstVisible = -1, nLastVisible = -1;
    for (unsigned i = 0; i != vEntries.size(); ++i)
    {
        //controls outside of the view need to be hidden to speed up accessibility tools
        bool const bShowCtrl = IsGroupVisible(GetGroup(i));
        if (vEntries[i]->MoveAndShow(nOffset, nWindowHeight, bShowCtrl))
        {
            if (nFirstVisible == -1)
                nFirstVisible = i;
            else
                nLastVisible = i;
        }
    }

    // show the one prior to the first visible and the first after the last visble control
    // to enable KEY_TAB travelling
    if(nFirstVisible > 0)
    {
        --nFirstVisible;
        if (IsGroupVisible(GetGroup(nFirstVisible)))
            vEntries[nFirstVisible]->Show();
    }
    if (nLastVisible != -1 && (unsigned)nLastVisible < vEntries.size() - 1)
    {
        ++nLastVisible;
        if (IsGroupVisible(GetGroup(nLastVisible)))
            vEntries[nLastVisible]->Show();
    }

    for (unsigned i = 0; i != vChapters.size(); ++i)
        vChapters[i]->MoveVertically(nOffset);
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
    for (unsigned j = 0; j != nExtCount; ++j)
    {
        rtl::OUString sComponentName = pExtConfig->GetComponentName(j);
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

// calculate position behind last chapter
unsigned ColorConfigWindow_Impl::GetPosBehindLastChapter () const
{
    int nLastY = vEntries.back()->GetBottom();
    nLastY += LogicToPixel( Size(0, 3), MAP_APPFONT ).Height();
    return nLastY;
}

// calculates the overall height of the invisible groups above eGroup
long ColorConfigWindow_Impl::GetDeltaAbove (Group eGroup) const
{
    long nDelta = 0;
    for (int i = 0; i != eGroup; ++i)
        if (!IsGroupVisible(static_cast<Group>(i)))
            nDelta += vChapters[i]->GetHeight();
    return nDelta;
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

//
// ColorConfigCtrl_Impl
//

class ColorConfigCtrl_Impl : public Control
{
    HeaderBar               aHeaderHB;
    ScrollBar               aVScroll;

    ColorConfigWindow_Impl  aScrollWindow;

    EditableColorConfig*            pColorConfig;
    EditableExtendedColorConfig*    pExtColorConfig;

    long nScrollPos;

    DECL_LINK(ScrollHdl, ScrollBar*);
    DECL_LINK(ClickHdl, CheckBox*);
    DECL_LINK(ColorHdl, ColorListBox*);
    DECL_LINK(ControlFocusHdl, Control*);

    virtual long PreNotify (NotifyEvent& rNEvt);
    virtual void Command (CommandEvent const& rCEvt);
    virtual void DataChanged (DataChangedEvent const& rDCEvt);
public:
    ColorConfigCtrl_Impl (Window* pParent, ResId const& rResId);
    ~ColorConfigCtrl_Impl ();

    void SetConfig (EditableColorConfig& rConfig) { pColorConfig = &rConfig; }
    void SetExtendedConfig (EditableExtendedColorConfig& rConfig) { pExtColorConfig = &rConfig; }
    void Update ();
    long GetScrollPosition () { return aVScroll.GetThumbPos(); }
    void SetScrollPosition (long nSet)
    {
        aVScroll.SetThumbPos(nSet);
        ScrollHdl(&aVScroll);
    }
};

ColorConfigCtrl_Impl::ColorConfigCtrl_Impl (
    Window* pParent, ResId const& rResId
) :
    Control(pParent, rResId),

    aHeaderHB(this, WB_BUTTONSTYLE | WB_BOTTOMBORDER),
    aVScroll(this,      ResId(VB_VSCROLL, *rResId.GetResMgr())),
    aScrollWindow(this, ResId(WN_SCROLL,  *rResId.GetResMgr())),

    pColorConfig(0),
    pExtColorConfig(0),
    nScrollPos(0)
{
    aScrollWindow.SetHeaderBar(aHeaderHB, aVScroll, *rResId.GetResMgr());
    aScrollWindow.SetScrollBar(aVScroll);

    Resource::FreeResource();

    Link aScrollLink = LINK(this, ColorConfigCtrl_Impl, ScrollHdl);
    aVScroll.SetScrollHdl(aScrollLink);
    aVScroll.SetEndScrollHdl(aScrollLink);

    Link aCheckLink = LINK(this, ColorConfigCtrl_Impl, ClickHdl);
    Link aColorLink = LINK(this, ColorConfigCtrl_Impl, ColorHdl);
    Link aGetFocusLink = LINK(this, ColorConfigCtrl_Impl, ControlFocusHdl);
    aScrollWindow.SetLinks(aCheckLink, aColorLink, aGetFocusLink);
}

ColorConfigCtrl_Impl::~ColorConfigCtrl_Impl()
{
}

void ColorConfigCtrl_Impl::Update ()
{
    DBG_ASSERT(pColorConfig, "Configuration not set");
    aScrollWindow.Update(pColorConfig, pExtColorConfig);
}

IMPL_LINK(ColorConfigCtrl_Impl, ScrollHdl, ScrollBar*, pScrollBar)
{
    aScrollWindow.ScrollHdl(nScrollPos, *pScrollBar);
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
    return Control::PreNotify(rNEvt);
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
                HandleScrollCommand( rCEvt, 0, &aVScroll );
            }
        }
        break;
        default:
            Control::Command(rCEvt);
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
    aScrollWindow.ClickHdl(pColorConfig, pBox);
    return 0;
}

// a color list has changed
IMPL_LINK(ColorConfigCtrl_Impl, ColorHdl, ColorListBox*, pBox)
{
    DBG_ASSERT(pColorConfig, "Configuration not set" );
    if (pBox)
        aScrollWindow.ColorHdl(pColorConfig, pExtColorConfig, pBox);
    return 0;
}
IMPL_LINK(ColorConfigCtrl_Impl, ControlFocusHdl, Control*, pCtrl)
{
    // determine whether a control is completely visible
    // and make it visible
    long aCtrlPosY = pCtrl->GetPosPixel().Y();
    unsigned const nWinHeight = aScrollWindow.GetSizePixel().Height();
    unsigned const nEntryHeight = aScrollWindow.GetEntryHeight();
    if (0 != (GETFOCUS_TAB & pCtrl->GetGetFocusFlags()) &&
        (aCtrlPosY < 0 || nWinHeight < aCtrlPosY + nEntryHeight)
    ) {
        long nThumbPos = aVScroll.GetThumbPos();
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
        aVScroll.SetThumbPos(nThumbPos);
        ScrollHdl(&aVScroll);
    }
    return 0;
};


//
// SvxColorOptionsTabPage
//

SvxColorOptionsTabPage::SvxColorOptionsTabPage(
    Window* pParent, const SfxItemSet& rCoreSet) :
    SfxTabPage( pParent, CUI_RES( RID_SVXPAGE_COLORCONFIG ), rCoreSet ),
       aColorSchemeFL(  this, CUI_RES( FL_COLORSCHEME ) ),
       aColorSchemeFT(  this, CUI_RES( FT_COLORSCHEME ) ),
       aColorSchemeLB(  this, CUI_RES( LB_COLORSCHEME ) ),
       aSaveSchemePB(   this, CUI_RES( PB_SAVESCHEME) ),
       aDeleteSchemePB( this, CUI_RES( PB_DELETESCHEME ) ),
       aCustomColorsFL( this, CUI_RES( FL_CUSTOMCOLORS ) ),
       bFillItemSetCalled(sal_False),
       pColorConfig(0),
       pExtColorConfig(0),
       pColorConfigCT(  new ColorConfigCtrl_Impl(this, CUI_RES( CT_COLORCONFIG ) ))
{
    FreeResource();
    aColorSchemeLB.SetSelectHdl(LINK(this, SvxColorOptionsTabPage, SchemeChangedHdl_Impl));
    Link aLk = LINK(this, SvxColorOptionsTabPage, SaveDeleteHdl_Impl );
    aSaveSchemePB.SetClickHdl(aLk);
    aDeleteSchemePB.SetClickHdl(aLk);
}

SvxColorOptionsTabPage::~SvxColorOptionsTabPage()
{
    //when the dialog is cancelled but the color scheme ListBox has been changed these
    //changes need to be undone
    if(!bFillItemSetCalled && aColorSchemeLB.GetSavedValue() != aColorSchemeLB.GetSelectEntryPos())
    {
        rtl::OUString sOldScheme =  aColorSchemeLB.GetEntry(aColorSchemeLB.GetSavedValue());
        if(!sOldScheme.isEmpty())
        {
            pColorConfig->SetCurrentSchemeName(sOldScheme);
            pExtColorConfig->SetCurrentSchemeName(sOldScheme);
        }
    }
    delete pColorConfigCT;
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
    if(aColorSchemeLB.GetSavedValue() != aColorSchemeLB.GetSelectEntryPos())
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
    pColorConfigCT->SetConfig(*pColorConfig);

    if(pExtColorConfig)
    {
        pExtColorConfig->ClearModified();
        pExtColorConfig->DisableBroadcast();
        delete pExtColorConfig;
    }
    pExtColorConfig = new EditableExtendedColorConfig;
    pColorConfigCT->SetExtendedConfig(*pExtColorConfig);

    String sUser = GetUserData();
    //has to be called always to speed up accessibility tools
    pColorConfigCT->SetScrollPosition(sUser.ToInt32());
    aColorSchemeLB.Clear();
    uno::Sequence< ::rtl::OUString >  aSchemes = pColorConfig->GetSchemeNames();
    const rtl::OUString* pSchemes = aSchemes.getConstArray();
    for(sal_Int32 i = 0; i < aSchemes.getLength(); i++)
        aColorSchemeLB.InsertEntry(pSchemes[i]);
    aColorSchemeLB.SelectEntry(pColorConfig->GetCurrentSchemeName());
    aColorSchemeLB.SaveValue();
    aDeleteSchemePB.Enable( aSchemes.getLength() > 1 );
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
    pColorConfigCT->Update();
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
    if(&aSaveSchemePB == pButton)
    {
        String sName;

        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        DBG_ASSERT(pFact, "Dialogdiet fail!");
        AbstractSvxNameDialog* aNameDlg = pFact->CreateSvxNameDialog( pButton,
                            sName, String(CUI_RES(RID_SVXSTR_COLOR_CONFIG_SAVE2)) );
        DBG_ASSERT(aNameDlg, "Dialogdiet fail!");
        aNameDlg->SetCheckNameHdl( LINK(this, SvxColorOptionsTabPage, CheckNameHdl_Impl));
        aNameDlg->SetText(String(CUI_RES(RID_SVXSTR_COLOR_CONFIG_SAVE1)));
        aNameDlg->SetHelpId(HID_OPTIONS_COLORCONFIG_SAVE_SCHEME);
        aNameDlg->SetEditHelpId(HID_OPTIONS_COLORCONFIG_NAME_SCHEME);
        aNameDlg->SetCheckNameHdl( LINK(this, SvxColorOptionsTabPage, CheckNameHdl_Impl));
        if(RET_OK == aNameDlg->Execute())
        {
            aNameDlg->GetName(sName);
            pColorConfig->AddScheme(sName);
            pExtColorConfig->AddScheme(sName);
            aColorSchemeLB.InsertEntry(sName);
            aColorSchemeLB.SelectEntry(sName);
            aColorSchemeLB.GetSelectHdl().Call(&aColorSchemeLB);
        }
        delete aNameDlg;
    }
    else
    {
        DBG_ASSERT(aColorSchemeLB.GetEntryCount() > 1, "don't delete the last scheme");
        QueryBox aQuery(pButton, CUI_RES(RID_SVXQB_DELETE_COLOR_CONFIG));
        aQuery.SetText(String(CUI_RES(RID_SVXSTR_COLOR_CONFIG_DELETE)));
        if(RET_YES == aQuery.Execute())
        {
            rtl::OUString sDeleteScheme(aColorSchemeLB.GetSelectEntry());
            aColorSchemeLB.RemoveEntry(aColorSchemeLB.GetSelectEntryPos());
            aColorSchemeLB.SelectEntryPos(0);
            aColorSchemeLB.GetSelectHdl().Call(&aColorSchemeLB);
            //first select the new scheme and then delete the old one
            pColorConfig->DeleteScheme(sDeleteScheme);
            pExtColorConfig->DeleteScheme(sDeleteScheme);
        }
    }
    aDeleteSchemePB.Enable( aColorSchemeLB.GetEntryCount() > 1 );
    return 0;
}

IMPL_LINK(SvxColorOptionsTabPage, CheckNameHdl_Impl, AbstractSvxNameDialog*, pDialog )
{
    String sName;
    pDialog->GetName(sName);
    return sName.Len() && LISTBOX_ENTRY_NOTFOUND == aColorSchemeLB.GetEntryPos( sName );
}

void SvxColorOptionsTabPage::FillUserData()
{
    SetUserData(String::CreateFromInt32(pColorConfigCT->GetScrollPosition()));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
