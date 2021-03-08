/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SVX_COLORBOX_HXX
#define INCLUDED_SVX_COLORBOX_HXX

#include <memory>
#include <vcl/weld.hxx>
#include <svx/colorwindow.hxx>

class ColorListBox;

class ListBoxColorWrapper
{
public:
    ListBoxColorWrapper(ColorListBox* pControl);
    void operator()(const OUString& rCommand,
                    const NamedColor& rColor); // ColorSelectFunction signature
private:
    ColorListBox* mpControl;
};

class SVXCORE_DLLPUBLIC ColorListBox
{
private:
    friend class ListBoxColorWrapper;
    std::unique_ptr<ColorWindow> m_xColorWindow;
    std::unique_ptr<weld::MenuButton> m_xButton;
    Link<ColorListBox&, void> m_aSelectedLink;
    ListBoxColorWrapper m_aColorWrapper;
    Color m_aAutoDisplayColor;
    Color m_aSaveColor;
    NamedColor m_aSelectedColor;
    sal_uInt16 m_nSlotId;
    bool m_bShowNoneButton;
    std::shared_ptr<PaletteManager> m_xPaletteManager;
    TopLevelParentFunction m_aTopLevelParentFunction;
    ColorStatus m_aColorStatus;

    void Selected(const NamedColor& rNamedColor);
    void createColorWindow();
    void LockWidthRequest();
    ColorWindow* getColorWindow() const;

    DECL_LINK(ToggleHdl, weld::ToggleButton&, void);

public:
    // rTopLevelParentFunction will be used to get parent for any color picker dialog created
    ColorListBox(std::unique_ptr<weld::MenuButton> pControl,
                 TopLevelParentFunction const& rTopLevelParentFunction);
    ~ColorListBox();

    void SetSelectHdl(const Link<ColorListBox&, void>& rLink) { m_aSelectedLink = rLink; }

    void SetSlotId(sal_uInt16 nSlotId, bool bShowNoneButton = false);

    Color const& GetSelectEntryColor() const { return m_aSelectedColor.first; }
    NamedColor const& GetSelectedEntry() const { return m_aSelectedColor; }

    void SelectEntry(const NamedColor& rColor);
    void SelectEntry(const Color& rColor);

    void SetNoSelection() { getColorWindow()->SetNoSelection(); }
    bool IsNoSelection() const { return getColorWindow()->IsNoSelection(); }

    void SetAutoDisplayColor(const Color& rColor) { m_aAutoDisplayColor = rColor; }

    void ShowPreview(const NamedColor& rColor);
    void EnsurePaletteManager();

    void SaveValue() { m_aSaveColor = GetSelectEntryColor(); }
    bool IsValueChangedFromSaved() const { return m_aSaveColor != GetSelectEntryColor(); }

    void set_sensitive(bool sensitive) { m_xButton->set_sensitive(sensitive); }
    bool get_sensitive() const { return m_xButton->get_sensitive(); }
    void show() { m_xButton->show(); }
    void hide() { m_xButton->hide(); }
    void set_visible(bool bShow) { m_xButton->set_visible(bShow); }
    void set_help_id(const OString& rHelpId) { m_xButton->set_help_id(rHelpId); }
    void connect_focus_in(const Link<weld::Widget&, void>& rLink)
    {
        m_xButton->connect_focus_in(rLink);
    }
    void connect_focus_out(const Link<weld::Widget&, void>& rLink)
    {
        m_xButton->connect_focus_out(rLink);
    }
    weld::MenuButton& get_widget() { return *m_xButton; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
