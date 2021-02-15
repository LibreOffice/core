/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <sal/config.h>

#include "ThemePanel.hxx"

#include <sfx2/objsh.hxx>

#include <com/sun/star/lang/IllegalArgumentException.hpp>

#include <editeng/fontitem.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/image.hxx>
#include <vcl/svapp.hxx>
#include <vcl/virdev.hxx>
#include <charatr.hxx>
#include <charfmt.hxx>
#include <docsh.hxx>
#include <docstyle.hxx>
#include <fmtcol.hxx>
#include <format.hxx>

namespace
{

class FontSet
{
public:
    OUString maName;
    OUString msMonoFont;
    OUString msHeadingFont;
    OUString msBaseFont;
};

class ColorVariable
{
public:
    tools::Long mnIndex;
    sal_Int16 mnTintShade;

    ColorVariable()
        : mnIndex(-1)
        , mnTintShade()
    {}

    ColorVariable(tools::Long nIndex, sal_Int16 nTintShade)
        : mnIndex(nIndex)
        , mnTintShade(nTintShade)
    {}
};

class StyleRedefinition
{
    ColorVariable maVariable;

public:
    OUString maElementName;

public:
    explicit StyleRedefinition(const OUString& aElementName)
        : maElementName(aElementName)
    {}

    void setColorVariable(ColorVariable aVariable)
    {
        maVariable = aVariable;
    }

    Color getColor(svx::ColorSet const & rColorSet)
    {
        Color aColor;
        if (maVariable.mnIndex > -1)
        {
            aColor = rColorSet.getColor(maVariable.mnIndex);
            aColor.ApplyTintOrShade(maVariable.mnTintShade);
        }
        else
        {
            aColor = COL_BLACK;
        }
        return aColor;
    }
};

class StyleSet
{
    std::vector<StyleRedefinition> maStyles;

public:
    explicit StyleSet()
        : maStyles()
    {}

    void add(StyleRedefinition const & aRedefinition)
    {
        maStyles.push_back(aRedefinition);
    }

    StyleRedefinition* get(const OUString& aString)
    {
        for (StyleRedefinition & rStyle : maStyles)
        {
            if (rStyle.maElementName == aString)
            {
                return &rStyle;
            }
        }
        return nullptr;
    }
};

StyleSet setupThemes()
{
    StyleSet aSet;

    {
        StyleRedefinition aRedefinition("Heading 1");
        aRedefinition.setColorVariable(ColorVariable(10, -1000));
        aSet.add(aRedefinition);
    }

    {
        StyleRedefinition aRedefinition("Heading 2");
        aRedefinition.setColorVariable(ColorVariable(7, -500));
        aSet.add(aRedefinition);
    }

    {
        StyleRedefinition aRedefinition("Heading 3");
        aRedefinition.setColorVariable(ColorVariable(5, 0));
        aSet.add(aRedefinition);
    }

    {
        StyleRedefinition aRedefinition("Heading 4");
        aRedefinition.setColorVariable(ColorVariable(6, -1000));
        aSet.add(aRedefinition);
    }

    {
        StyleRedefinition aRedefinition("Heading 5");
        aRedefinition.setColorVariable(ColorVariable(4, -1500));
        aSet.add(aRedefinition);
    }

    {
        StyleRedefinition aRedefinition("Heading 6");
        aRedefinition.setColorVariable(ColorVariable(3, -2500));
        aSet.add(aRedefinition);
    }

    {
        StyleRedefinition aRedefinition("Heading 7");
        aRedefinition.setColorVariable(ColorVariable(3, -2500));
        aSet.add(aRedefinition);
    }

    {
        StyleRedefinition aRedefinition("Heading 8");
        aRedefinition.setColorVariable(ColorVariable(2, 0));
        aSet.add(aRedefinition);
    }

    {
        StyleRedefinition aRedefinition("Heading 9");
        aRedefinition.setColorVariable(ColorVariable(2, 0));
        aSet.add(aRedefinition);
    }

    {
        StyleRedefinition aRedefinition("Heading 10");
        aRedefinition.setColorVariable(ColorVariable(0, 0));
        aSet.add(aRedefinition);
    }

    return aSet;
}

void changeFont(SwFormat* pFormat, SwDocStyleSheet const * pStyle, FontSet const & rFontSet)
{
    if (pStyle->GetName() != "Default Style" && pFormat->GetAttrSet().GetItem(RES_CHRATR_FONT, false) == nullptr)
    {
        return;
    }

    SvxFontItem aFontItem(pFormat->GetFont(false));

    FontPitch ePitch = aFontItem.GetPitch();

    if (ePitch == PITCH_FIXED)
    {
        aFontItem.SetFamilyName(rFontSet.msMonoFont);
    }
    else
    {
        if (pStyle->GetName() == "Heading")
        {
            aFontItem.SetFamilyName(rFontSet.msHeadingFont);
        }
        else
        {
            aFontItem.SetFamilyName(rFontSet.msBaseFont);
        }
    }

    pFormat->SetFormatAttr(aFontItem);
}

/*void changeBorder(SwTextFormatColl* pCollection, SwDocStyleSheet* pStyle, StyleSet& rStyleSet)
{
    if (pStyle->GetName() == "Heading")
    {
        SvxBoxItem aBoxItem(pCollection->GetBox());
        editeng::SvxBorderLine aBorderLine;
        aBorderLine.SetWidth(40); //20 = 1pt
        aBorderLine.SetColor(rColorSet.mBaseColors[0]);
        aBoxItem.SetLine(&aBorderLine, SvxBoxItemLine::BOTTOM);

        pCollection->SetFormatAttr(aBoxItem);
    }
}*/

void changeColor(SwTextFormatColl* pCollection, svx::ColorSet const & rColorSet, StyleRedefinition* pRedefinition)
{
    Color aColor = pRedefinition->getColor(rColorSet);

    SvxColorItem aColorItem(pCollection->GetColor());
    aColorItem.SetValue(aColor);
    pCollection->SetFormatAttr(aColorItem);
}

std::vector<FontSet> initFontSets()
{
    std::vector<FontSet> aFontSets;
    {
        FontSet aFontSet;
        aFontSet.maName = "Liberation Family";
        aFontSet.msHeadingFont = "Liberation Sans";
        aFontSet.msBaseFont = "Liberation Serif";
        aFontSet.msMonoFont = "Liberation Mono";
        aFontSets.push_back(aFontSet);
    }
    {
        FontSet aFontSet;
        aFontSet.maName = "DejaVu Family";
        aFontSet.msHeadingFont = "DejaVu Sans";
        aFontSet.msBaseFont = "DejaVu Serif";
        aFontSet.msMonoFont = "DejaVu Sans Mono";
        aFontSets.push_back(aFontSet);
    }
    {
        FontSet aFontSet;
        aFontSet.maName = "Croscore Modern";
        aFontSet.msHeadingFont = "Caladea";
        aFontSet.msBaseFont = "Carlito";
        aFontSet.msMonoFont = "Liberation Mono";
        aFontSets.push_back(aFontSet);
    }
    {
        FontSet aFontSet;
        aFontSet.maName = "Carlito";
        aFontSet.msHeadingFont = "Carlito";
        aFontSet.msBaseFont = "Carlito";
        aFontSet.msMonoFont = "Liberation Mono";
        aFontSets.push_back(aFontSet);
    }
    {
        FontSet aFontSet;
        aFontSet.maName = "Source Sans Family";
        aFontSet.msHeadingFont = "Source Sans Pro";
        aFontSet.msBaseFont = "Source Sans Pro";
        aFontSet.msMonoFont = "Source Code Pro";
        aFontSets.push_back(aFontSet);
    }
    {
        FontSet aFontSet;
        aFontSet.maName = "Source Sans Family 2";
        aFontSet.msHeadingFont = "Source Sans Pro";
        aFontSet.msBaseFont = "Source Sans Pro Light";
        aFontSet.msMonoFont = "Source Code Pro";
        aFontSets.push_back(aFontSet);
    }
    {
        FontSet aFontSet;
        aFontSet.maName = "Libertine Family";
        aFontSet.msHeadingFont = "Linux Biolinum G";
        aFontSet.msBaseFont = "Linux Libertine G";
        aFontSet.msMonoFont = "Liberation Mono";
        aFontSets.push_back(aFontSet);
    }
    {
        FontSet aFontSet;
        aFontSet.maName = "Open Sans";
        aFontSet.msHeadingFont = "Open Sans";
        aFontSet.msBaseFont = "Open Sans";
        aFontSet.msMonoFont = "Droid Sans Mono";
        aFontSets.push_back(aFontSet);
    }
    {
        FontSet aFontSet;
        aFontSet.maName = "Droid Sans";
        aFontSet.msHeadingFont = "Droid Sans";
        aFontSet.msBaseFont = "Droid Sans";
        aFontSet.msMonoFont = "Droid Sans Mono";
        aFontSets.push_back(aFontSet);
    }
    return aFontSets;
}

FontSet getFontSet(const OUString& rFontVariant, std::vector<FontSet>& aFontSets)
{
    for (const FontSet & rFontSet : aFontSets)
    {
        if (rFontSet.maName == rFontVariant)
            return rFontSet;
    }
    return aFontSets[0];
}

void applyTheme(SfxStyleSheetBasePool* pPool, const OUString& sFontSetName, const OUString& sColorSetName,
                StyleSet& rStyleSet, svx::ColorSets& rColorSets)
{
    SwDocStyleSheet* pStyle;

    std::vector<FontSet> aFontSets = initFontSets();
    FontSet aFontSet = getFontSet(sFontSetName, aFontSets);

    svx::ColorSet aColorSet = rColorSets.getColorSet(sColorSetName);

    pStyle = static_cast<SwDocStyleSheet*>(pPool->First(SfxStyleFamily::Para));
    while (pStyle)
    {
        SwTextFormatColl* pCollection = pStyle->GetCollection();

        changeFont(pCollection, pStyle, aFontSet);

        StyleRedefinition* pRedefinition = rStyleSet.get(pStyle->GetName());

        if (pRedefinition)
        {
            changeColor(pCollection, aColorSet, pRedefinition);
        }

        pStyle = static_cast<SwDocStyleSheet*>(pPool->Next());
    }

    pStyle = static_cast<SwDocStyleSheet*>(pPool->First(SfxStyleFamily::Char));
    while (pStyle)
    {
        SwCharFormat* pCharFormat = pStyle->GetCharFormat();

        changeFont(static_cast<SwFormat*>(pCharFormat), pStyle, aFontSet);

        pStyle = static_cast<SwDocStyleSheet*>(pPool->Next());
    }
}

BitmapEx GenerateColorPreview(const svx::ColorSet& rColorSet)
{
    ScopedVclPtrInstance<VirtualDevice> pVirtualDev(*Application::GetDefaultDevice());
    float fScaleFactor = pVirtualDev->GetDPIScaleFactor();
    tools::Long BORDER = 2 * fScaleFactor;
    tools::Long SIZE = 12 * fScaleFactor;

    Size aSize(BORDER * 7 + SIZE * 6, BORDER * 3 + SIZE * 2);
    pVirtualDev->SetOutputSizePixel(aSize);

    tools::Long x = BORDER;
    tools::Long y1 = BORDER;
    tools::Long y2 = y1 + SIZE + BORDER;

    pVirtualDev->SetLineColor(COL_LIGHTGRAY);

    for (sal_uInt32 i = 0; i < 12; i += 2)
    {
        pVirtualDev->SetFillColor(rColorSet.getColor(i));
        pVirtualDev->DrawRect(tools::Rectangle(x, y1, x + SIZE, y1 + SIZE));

        pVirtualDev->SetFillColor(rColorSet.getColor(i + 1));
        pVirtualDev->DrawRect(tools::Rectangle(x, y2, x + SIZE, y2 + SIZE));

        x += SIZE + BORDER;
    }

    return pVirtualDev->GetBitmapEx(Point(), aSize);
}

} // end anonymous namespace

namespace sw::sidebar {

VclPtr<vcl::Window> ThemePanel::Create (vcl::Window* pParent,
                                        const css::uno::Reference<css::frame::XFrame>& rxFrame)
{
    if (pParent == nullptr)
        throw css::lang::IllegalArgumentException("no parent Window given to PagePropertyPanel::Create", nullptr, 0);
    if (!rxFrame.is())
        throw css::lang::IllegalArgumentException("no XFrame given to PagePropertyPanel::Create", nullptr, 1);

    return VclPtr<ThemePanel>::Create(pParent, rxFrame);
}

ThemePanel::ThemePanel(vcl::Window* pParent,
                       const css::uno::Reference<css::frame::XFrame>& rxFrame)
    : PanelLayout(pParent, "ThemePanel", "modules/swriter/ui/sidebartheme.ui", rxFrame)
    , mxListBoxFonts(m_xBuilder->weld_tree_view("listbox_fonts"))
    , mxValueSetColors(new ValueSet(nullptr))
    , mxValueSetColorsWin(new weld::CustomWeld(*m_xBuilder, "valueset_colors", *mxValueSetColors))
    , mxApplyButton(m_xBuilder->weld_button("apply"))
    , maColorSets()
{
    mxValueSetColors->SetColCount(2);
    mxValueSetColors->SetLineCount(3);

    mxApplyButton->connect_clicked(LINK(this, ThemePanel, ClickHdl));
    mxListBoxFonts->connect_row_activated(LINK(this, ThemePanel, DoubleClickHdl));
    mxValueSetColors->SetDoubleClickHdl(LINK(this, ThemePanel, DoubleClickValueSetHdl));

    std::vector<FontSet> aFontSets = initFontSets();
    for (const FontSet & rFontSet : aFontSets)
        mxListBoxFonts->append_text(rFontSet.maName);
    mxListBoxFonts->set_size_request(-1, mxListBoxFonts->get_height_rows(aFontSets.size()));

    maColorSets.init();

    const std::vector<svx::ColorSet>& aColorSets = maColorSets.getColorSets();
    for (size_t i = 0; i < aColorSets.size(); ++i)
    {
        const svx::ColorSet& rColorSet = aColorSets[i];

        const OUString& aName = rColorSet.getName();
        BitmapEx aPreview = GenerateColorPreview(rColorSet);

        sal_uInt16 nId = i + 1;
        mxValueSetColors->InsertItem(nId, Image(aPreview), aName);
    }

    mxValueSetColors->SetOptimalSize();

    if (!aColorSets.empty())
        mxValueSetColors->SelectItem(1); // ItemId 1, position 0

    m_pInitialFocusWidget = mxListBoxFonts.get();
}

ThemePanel::~ThemePanel()
{
    disposeOnce();
}

void ThemePanel::dispose()
{
    mxListBoxFonts.reset();
    mxValueSetColorsWin.reset();
    mxValueSetColors.reset();
    mxApplyButton.reset();

    PanelLayout::dispose();
}

IMPL_LINK_NOARG(ThemePanel, ClickHdl, weld::Button&, void)
{
    DoubleClickHdl();
}

IMPL_LINK_NOARG(ThemePanel, DoubleClickValueSetHdl, ValueSet*, void)
{
    DoubleClickHdl();
}

IMPL_LINK_NOARG(ThemePanel, DoubleClickHdl, weld::TreeView&, bool)
{
    DoubleClickHdl();
    return true;
}

void ThemePanel::DoubleClickHdl()
{
    SwDocShell* pDocSh = static_cast<SwDocShell*>(SfxObjectShell::Current());
    if (!pDocSh)
        return;

    sal_uInt32 nItemId = mxValueSetColors->GetSelectedItemId();
    if (!nItemId)
        return;
    OUString sEntryFonts = mxListBoxFonts->get_selected_text();
    sal_uInt32 nIndex = nItemId - 1;
    OUString sEntryColors = maColorSets.getColorSet(nIndex).getName();

    StyleSet aStyleSet = setupThemes();

    applyTheme(pDocSh->GetStyleSheetPool(), sEntryFonts, sEntryColors, aStyleSet, maColorSets);
}

void ThemePanel::NotifyItemUpdate(const sal_uInt16 /*nSId*/,
                                         const SfxItemState /*eState*/,
                                         const SfxPoolItem* /*pState*/)
{
}

} // end of namespace ::sw::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
