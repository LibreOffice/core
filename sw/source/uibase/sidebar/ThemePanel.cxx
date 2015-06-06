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

#include <swtypes.hxx>
#include <cmdid.h>

#include <tools/helpers.hxx>

#include <svl/intitem.hxx>
#include <svx/svxids.hrc>
#include <svx/dlgutil.hxx>
#include <svx/rulritem.hxx>

#include <sfx2/sidebar/ControlFactory.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/objsh.hxx>

#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/frame/DocumentTemplates.hpp>
#include <com/sun/star/frame/XDocumentTemplates.hpp>
#include <com/sun/star/document/XUndoManagerSupplier.hpp>

#include <editeng/fontitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/borderline.hxx>
#include "charatr.hxx"
#include "charfmt.hxx"
#include "docstyle.hxx"
#include "fmtcol.hxx"
#include "format.hxx"

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

class ColorSet
{
public:
    OUString maName;
    Color maColors[10];
};

class ColorVariable
{
public:
    long mnIndex;
    Color maColor;
    sal_Int16 mnTintShade;

    ColorVariable()
        : mnIndex(-1)
        , maColor()
        , mnTintShade()
    {}

    ColorVariable(long nIndex, sal_Int16 nTintShade = 0)
        : mnIndex(nIndex)
        , maColor()
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

    Color getColor(ColorSet& rColorSet)
    {
        Color aColor;
        if (maVariable.mnIndex > -1)
        {
            aColor.SetColor(rColorSet.maColors[maVariable.mnIndex].GetColor());
            if (maVariable.mnTintShade < 0)
            {
                double fFactor = std::abs(maVariable.mnTintShade) / 10000.0;
                aColor.SetRed(MinMax(aColor.GetRed() + (fFactor * (255.0 - aColor.GetRed())), 0, 255));
                aColor.SetGreen(MinMax(aColor.GetGreen() + (fFactor * (255.0 - aColor.GetGreen())), 0, 255));
                aColor.SetBlue(MinMax(aColor.GetBlue() + (fFactor * (255.0 - aColor.GetBlue())), 0, 255));
            }
            else if (maVariable.mnTintShade > 0)
            {
                double fFactor = 1.0 - std::abs(maVariable.mnTintShade) / 10000.0;
                aColor.SetRed(MinMax(aColor.GetRed() * fFactor, 0, 255));
                aColor.SetGreen(MinMax(aColor.GetGreen() * fFactor, 0, 255));
                aColor.SetBlue(MinMax(aColor.GetBlue() * fFactor, 0, 255));
            }
        }
        else
        {
            aColor.SetColor(maVariable.maColor.GetColor());
        }
        return aColor;
    }
};

class StyleSet
{
    OUString maName;
    std::vector<StyleRedefinition> maStyles;

public:
    explicit StyleSet(const OUString& aName)
        : maName(aName)
        , maStyles()
    {}

    void add(StyleRedefinition aRedefinition)
    {
        maStyles.push_back(aRedefinition);
    }

    StyleRedefinition* get(const OUString& aString)
    {
        for (size_t i = 0; i < maStyles.size(); i++)
        {
            if (maStyles[i].maElementName == aString)
            {
                return &maStyles[i];
            }
        }
        return nullptr;
    }
};

StyleSet setupThemes()
{
    StyleSet aSet("Default");

    {
        StyleRedefinition aRedefinition("Heading 1");
        aRedefinition.setColorVariable(ColorVariable(0, 4000));
        aSet.add(aRedefinition);
    }

    {
        StyleRedefinition aRedefinition("Heading 2");
        aRedefinition.setColorVariable(ColorVariable(0, 2500));
        aSet.add(aRedefinition);
    }

    {
        StyleRedefinition aRedefinition("Heading 3");
        aRedefinition.setColorVariable(ColorVariable(0, 1000));
        aSet.add(aRedefinition);
    }

    {
        StyleRedefinition aRedefinition("Heading 4");
        aRedefinition.setColorVariable(ColorVariable(0));
        aSet.add(aRedefinition);
    }

    {
        StyleRedefinition aRedefinition("Heading 5");
        aRedefinition.setColorVariable(ColorVariable(0, -500));
        aSet.add(aRedefinition);
    }

    {
        StyleRedefinition aRedefinition("Heading 6");
        aRedefinition.setColorVariable(ColorVariable(0, -1000));
        aSet.add(aRedefinition);
    }

    {
        StyleRedefinition aRedefinition("Heading 7");
        aRedefinition.setColorVariable(ColorVariable(0, -1500));
        aSet.add(aRedefinition);
    }

    {
        StyleRedefinition aRedefinition("Heading 8");
        aRedefinition.setColorVariable(ColorVariable(0, -2000));
        aSet.add(aRedefinition);
    }

    {
        StyleRedefinition aRedefinition("Heading 9");
        aRedefinition.setColorVariable(ColorVariable(0, -2500));
        aSet.add(aRedefinition);
    }

    {
        StyleRedefinition aRedefinition("Heading 10");
        aRedefinition.setColorVariable(ColorVariable(0, -3000));
        aSet.add(aRedefinition);
    }

    return aSet;
}

void changeFont(SwFormat* pFormat, SwDocStyleSheet* pStyle, FontSet& rFontSet)
{
    bool bChanged = false;

    if (pStyle->GetName() != "Default Style" && pFormat->GetAttrSet().GetItem(RES_CHRATR_FONT, false) == nullptr)
    {
        return;
    }

    SvxFontItem aFontItem(static_cast<const SvxFontItem&>(pFormat->GetFont(false)));

    FontPitch ePitch = aFontItem.GetPitch();

    if (ePitch == PITCH_FIXED)
    {
        aFontItem.SetFamilyName(rFontSet.msMonoFont);
        bChanged = true;
    }
    else
    {
        if (pStyle->GetName() == "Heading")
        {
            aFontItem.SetFamilyName(rFontSet.msHeadingFont);
            bChanged = true;
        }
        else
        {
            aFontItem.SetFamilyName(rFontSet.msBaseFont);
            bChanged = true;
        }
    }

    if (bChanged)
    {
        pFormat->SetFormatAttr(aFontItem);
    }
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

void changeColor(SwTextFormatColl* pCollection, ColorSet& rColorSet, StyleRedefinition* pRedefinition)
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
    for (size_t i = 0; i < aFontSets.size(); ++i)
    {
        if (aFontSets[i].maName == rFontVariant)
            return aFontSets[i];
    }
    return aFontSets[0];
}

std::vector<ColorSet> initColorSets()
{
    std::vector<ColorSet> aColorSets;
    {
        ColorSet aColorSet;
        aColorSet.maName = "Default";
        aColorSet.maColors[0] = Color(0x00, 0x00, 0x00);
        aColorSets.push_back(aColorSet);
    }
    {
        ColorSet aColorSet;
        aColorSet.maName = "Red";
        aColorSet.maColors[0] = Color(0xa4, 0x00, 0x00);
        aColorSets.push_back(aColorSet);
    }
    {
        ColorSet aColorSet;
        aColorSet.maName = "Green";
        aColorSet.maColors[0] = Color(0x00, 0xa4, 0x00);
        aColorSets.push_back(aColorSet);
    }
    {
        ColorSet aColorSet;
        aColorSet.maName = "Blue";
        aColorSet.maColors[0] = Color(0x00, 0x00, 0xa4);
        aColorSets.push_back(aColorSet);
    }
    {
        ColorSet aColorSet;
        aColorSet.maName = "Sky";
        aColorSet.maColors[0] = Color(0x72, 0x9f, 0xcf);
        aColorSets.push_back(aColorSet);
    }

    return aColorSets;
}

ColorSet getColorSet(const OUString& rColorVariant, std::vector<ColorSet>& aColorSets)
{
    for (size_t i = 0; i < aColorSets.size(); ++i)
    {
        if (aColorSets[i].maName == rColorVariant)
            return aColorSets[i];
    }
    return aColorSets[0];
}

void applyTheme(SfxStyleSheetBasePool* pPool, const OUString& sFontSetName, const OUString& sColorSetName, StyleSet& rStyleSet)
{
    SwDocStyleSheet* pStyle;

    std::vector<FontSet> aFontSets = initFontSets();
    FontSet aFontSet = getFontSet(sFontSetName, aFontSets);

    std::vector<ColorSet> aColorSets = initColorSets();
    ColorSet aColorSet = getColorSet(sColorSetName, aColorSets);

    pPool->SetSearchMask(SFX_STYLE_FAMILY_PARA, SFXSTYLEBIT_ALL);
    pStyle = static_cast<SwDocStyleSheet*>(pPool->First());

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

    pPool->SetSearchMask(SFX_STYLE_FAMILY_CHAR, SFXSTYLEBIT_ALL);
    pStyle = static_cast<SwDocStyleSheet*>(pPool->First());

    while (pStyle)
    {
        SwCharFormat* pCharFormat = pStyle->GetCharFormat();

        changeFont(static_cast<SwFormat*>(pCharFormat), pStyle, aFontSet);

        pStyle = static_cast<SwDocStyleSheet*>(pPool->Next());
    }
}

} // end anonymous namespace

namespace sw { namespace sidebar {

VclPtr<vcl::Window> ThemePanel::Create (vcl::Window* pParent,
                                        const css::uno::Reference<css::frame::XFrame>& rxFrame,
                                        SfxBindings* pBindings)
{
    if (pParent == NULL)
        throw css::lang::IllegalArgumentException("no parent Window given to PagePropertyPanel::Create", NULL, 0);
    if (!rxFrame.is())
        throw css::lang::IllegalArgumentException("no XFrame given to PagePropertyPanel::Create", NULL, 1);
    if (pBindings == NULL)
        throw css::lang::IllegalArgumentException("no SfxBindings given to PagePropertyPanel::Create", NULL, 2);

    return VclPtr<ThemePanel>::Create(pParent, rxFrame, pBindings);
}

ThemePanel::ThemePanel(vcl::Window* pParent,
                               const css::uno::Reference<css::frame::XFrame>& rxFrame,
                               SfxBindings* pBindings)
    : PanelLayout(pParent, "ThemePanel", "modules/swriter/ui/sidebartheme.ui", rxFrame)
    , mpBindings(pBindings)
{
    get(mpListBoxFonts,  "listbox_fonts");
    get(mpListBoxColors, "listbox_colors");
    get(mpApplyButton,   "apply");

    mpApplyButton->SetClickHdl(LINK(this, ThemePanel, ClickHdl));
    mpListBoxFonts->SetDoubleClickHdl(LINK(this, ThemePanel, ClickHdl));
    mpListBoxColors->SetDoubleClickHdl(LINK(this, ThemePanel, ClickHdl));

    std::vector<FontSet> aFontSets = initFontSets();
    for (size_t i = 0; i < aFontSets.size(); ++i)
    {
        mpListBoxFonts->InsertEntry(aFontSets[i].maName);
    }

    std::vector<ColorSet> aColorSets = initColorSets();
    for (size_t i = 0; i < aColorSets.size(); ++i)
    {
        mpListBoxColors->InsertEntry(aColorSets[i].maName);
    }
}

ThemePanel::~ThemePanel()
{
    disposeOnce();
}

void ThemePanel::dispose()
{
    mpListBoxFonts.clear();
    mpListBoxColors.clear();
    mpApplyButton.clear();

    PanelLayout::dispose();
}

IMPL_LINK_NOARG(ThemePanel, ClickHdl)
{
    SwDocShell* pDocSh = static_cast<SwDocShell*>(SfxObjectShell::Current());
    if (pDocSh)
    {
        OUString sEntryFonts = mpListBoxFonts->GetSelectEntry();
        OUString sEntryColors = mpListBoxColors->GetSelectEntry();

        StyleSet aStyleSet = setupThemes();

        applyTheme(pDocSh->GetStyleSheetPool(), sEntryFonts, sEntryColors, aStyleSet);
    }
    return 1;
}

void ThemePanel::NotifyItemUpdate(const sal_uInt16 /*nSId*/,
                                         const SfxItemState /*eState*/,
                                         const SfxPoolItem* /*pState*/,
                                         const bool /*bIsEnabled*/)
{
}

}} // end of namespace ::sw::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
