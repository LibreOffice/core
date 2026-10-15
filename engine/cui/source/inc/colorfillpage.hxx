/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sfx2/namedcolor.hxx>
#include <sfx2/tabdlg.hxx>
#include <svx/PaletteManager.hxx>
#include <svx/dlgctrl.hxx>
#include <svx/tabarea.hxx>
#include <svx/xtable.hxx>
#include <vcl/weld.hxx>

#include <memory>
#include <vector>

/// The Color page of an Area or Background tab: the swatches of a palette, Pattern Style and the
/// custom palette beside them, and one preview of the whole fill under both.
class SvxColorFillTabPage final : public SfxTabPage
{
public:
    SvxColorFillTabPage(weld::Container* pPage, weld::DialogController* pController,
                        const SfxItemSet& rInAttrs);
    virtual ~SvxColorFillTabPage() override;

    static std::unique_ptr<SfxTabPage> Create(weld::Container* pPage,
                                              weld::DialogController* pController,
                                              const SfxItemSet* pAttrs);

    // Fills the palette list. The color list and the handlers are set by then.
    void Construct();

    virtual bool FillItemSet(SfxItemSet* pSet) override;
    virtual void Reset(const SfxItemSet* pSet) override;
    virtual void ActivatePage(const SfxItemSet& rSet) override;
    virtual DeactivateRC DeactivatePage(SfxItemSet* pSet) override;

    void SetColorList(const XColorListRef& rColorList) { m_xColorList = rColorList; }
    void SetColorChgd(ChangeType* pColorListState) { m_pColorListState = pColorListState; }

    // Called after the user changes the color, not when the page loads one.
    void SetColorChangedHdl(const Link<SvxColorFillTabPage&, void>& rLink)
    {
        m_aColorChangedHdl = rLink;
    }
    // Called after the user selects None in the palette list. While unset, the list has no None.
    void SetNoneSelectedHdl(const Link<SvxColorFillTabPage&, void>& rLink)
    {
        m_aNoneSelectedHdl = rLink;
    }
    // Called when the preview has to be drawn again. While unset, the preview shows the color.
    void SetUpdatePreviewHdl(const Link<SvxColorFillTabPage&, void>& rLink)
    {
        m_aUpdatePreviewHdl = rLink;
    }

    // Shows None in the palette list, or the palette again.
    void SetNoFill(bool bNoFill);

    // Draws the preview again, through the handler when one is set.
    void UpdatePreview();
    void ShowPreview(const SfxItemSet& rFill, const OUString& rCaption);
    // The color fill of the page and the caption that names it.
    const SfxItemSet& GetColorFill() const { return m_aColorFill; }
    OUString GetColorCaption() const;

    weld::Button& GetCustomPatternButton() { return *m_xCustomPattern; }
    weld::Button& GetMoreEffectsButton() { return *m_xMoreEffects; }
    // The place right of the palette, above Custom pattern and More effects, for Pattern Style.
    weld::Container& GetPatternBox() { return *m_xPatternSlot; }
    // Hides Pattern Style and Custom pattern, and shows More effects beside Custom color.
    void HidePatternControls();

private:
    // The palette index of the selected list entry, or -1 for None.
    sal_Int32 SelectedPalette() const;
    void FillPaletteList();
    void ShowPalette(sal_Int32 nPalette);
    void ShareColorList();
    // Selects the swatch of the current color in the palette, if the palette has it.
    void SelectCurrentColor();

    void SetColor(const NamedColor& rColor);
    // The name of the current color, from the palette when the color has none of its own.
    OUString GetColorName() const;
    void UpdateButtons();

    void AskForColorName(const OUString& rSuggestedName);
    void AddToCustomPalette(const OUString& rName);

    DECL_LINK(PaletteSelectedHdl, weld::ComboBox&, void);
    DECL_LINK(ColorSelectedHdl, weld::IconView&, void);
    DECL_LINK(ColorTooltipHdl, const weld::TreeIter&, OUString);
    DECL_LINK(RecentColorTooltipHdl, const weld::TreeIter&, OUString);
    DECL_LINK(AddHdl, weld::Button&, void);
    DECL_LINK(RemoveHdl, weld::Button&, void);
    DECL_LINK(CustomColorHdl, weld::Button&, void);

    XColorListRef m_xColorList;
    ChangeType* m_pColorListState = nullptr;

    Link<SvxColorFillTabPage&, void> m_aColorChangedHdl;
    Link<SvxColorFillTabPage&, void> m_aNoneSelectedHdl;
    Link<SvxColorFillTabPage&, void> m_aUpdatePreviewHdl;

    // The fill of the current color, with the color item and a solid fill style.
    SfxItemSet m_aColorFill;
    NamedColor m_aColor;
    // The palette whose swatches the page shows, which stays the same while None is selected.
    sal_Int32 m_nPalette = -1;
    // True when the fill is None.
    bool m_bNoFill = false;

    PaletteManager m_aPalettes;
    std::vector<NamedColor> m_aPaletteColors;
    std::vector<NamedColor> m_aRecentColors;

    std::unique_ptr<weld::ComboBox> m_xPaletteList;
    std::unique_ptr<weld::IconView> m_xColors;
    std::unique_ptr<weld::IconView> m_xRecentColors;
    std::unique_ptr<weld::Button> m_xAdd;
    std::unique_ptr<weld::Button> m_xRemove;
    std::unique_ptr<weld::Button> m_xCustomColor;
    std::unique_ptr<weld::Button> m_xCustomPattern;
    std::unique_ptr<weld::Button> m_xMoreEffects;
    std::unique_ptr<weld::Container> m_xPatternSlot;
    std::unique_ptr<weld::Label> m_xCaption;
    SvxXRectPreview m_aPreview;
    std::unique_ptr<weld::CustomWeld> m_xPreview;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
