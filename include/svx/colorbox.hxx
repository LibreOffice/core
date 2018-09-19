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
#include <vcl/menubtn.hxx>
#include <vcl/weld.hxx>
#include <svx/colorwindow.hxx>
#include <sfx2/controlwrapper.hxx>

class SvxColorListBox;
class ColorListBox;

class SvxListBoxColorWrapper
{
public:
    SvxListBoxColorWrapper(SvxColorListBox* pControl);
    void operator()(const OUString& rCommand, const NamedColor& rColor);
    void dispose();
private:
    VclPtr<SvxColorListBox> mxControl;
};

class SVX_DLLPUBLIC SvxColorListBox : public MenuButton
{
private:
    friend class SvxListBoxColorWrapper;
    VclPtr<SvxColorWindow> m_xColorWindow;
    Link<SvxColorListBox&, void> m_aSelectedLink;
    SvxListBoxColorWrapper m_aColorWrapper;
    Color m_aAutoDisplayColor;
    Color m_aSaveColor;
    NamedColor m_aSelectedColor;
    sal_uInt16 m_nSlotId;
    bool m_bShowNoneButton;
    std::shared_ptr<PaletteManager> m_xPaletteManager;
    BorderColorStatus m_aBorderColorStatus;

    DECL_LINK(MenuActivateHdl, MenuButton *, void);
    void Selected(const NamedColor& rNamedColor);
    void createColorWindow();
    void LockWidthRequest();
    VclPtr<SvxColorWindow> const & getColorWindow() const;
public:
    SvxColorListBox(vcl::Window* pParent, WinBits nStyle = 0);
    virtual ~SvxColorListBox() override;
    virtual void dispose() override;

    void SetSelectHdl(const Link<SvxColorListBox&, void>& rLink)
    {
        m_aSelectedLink = rLink;
    }

    void SetSlotId(sal_uInt16 nSlotId, bool bShowNoneButton = false);

    Color const & GetSelectEntryColor() const { return m_aSelectedColor.first; }
    NamedColor const & GetSelectedEntry() const { return m_aSelectedColor; }

    void SelectEntry(const NamedColor& rColor);
    void SelectEntry(const Color& rColor);

    void SetNoSelection() { getColorWindow()->SetNoSelection(); }
    bool IsNoSelection() const { return getColorWindow()->IsNoSelection(); }

    void SetAutoDisplayColor(const Color &rColor) { m_aAutoDisplayColor = rColor; }
    void ShowPreview(const NamedColor &rColor);
    void EnsurePaletteManager();

    void SaveValue() { m_aSaveColor = GetSelectEntryColor(); }
    bool IsValueChangedFromSaved() const { return m_aSaveColor != GetSelectEntryColor(); }

    DECL_LINK(WindowEventListener, VclWindowEvent&, void);
};

class ListBoxColorWrapper
{
public:
    ListBoxColorWrapper(ColorListBox* pControl);
    void operator()(const OUString& rCommand, const NamedColor& rColor);
private:
    ColorListBox* mpControl;
};

class SVX_DLLPUBLIC ColorListBox
{
private:
    friend class ListBoxColorWrapper;
    std::unique_ptr<ColorWindow> m_xColorWindow;
    std::unique_ptr<weld::MenuButton> m_xButton;
    weld::Window* m_pTopLevel;
    Link<ColorListBox&, void> m_aSelectedLink;
    ListBoxColorWrapper m_aColorWrapper;
    Color m_aAutoDisplayColor;
    Color m_aSaveColor;
    NamedColor m_aSelectedColor;
    sal_uInt16 m_nSlotId;
    bool m_bShowNoneButton;
    std::shared_ptr<PaletteManager> m_xPaletteManager;
    BorderColorStatus m_aBorderColorStatus;

    void Selected(const NamedColor& rNamedColor);
    void createColorWindow();
    void LockWidthRequest();
    ColorWindow* getColorWindow() const;
public:
    ColorListBox(std::unique_ptr<weld::MenuButton> pControl, weld::Window* pWindow);
    ~ColorListBox();

    void SetSelectHdl(const Link<ColorListBox&, void>& rLink)
    {
        m_aSelectedLink = rLink;
    }

    void SetSlotId(sal_uInt16 nSlotId, bool bShowNoneButton = false);

    Color const & GetSelectEntryColor() const { return m_aSelectedColor.first; }
    NamedColor const & GetSelectedEntry() const { return m_aSelectedColor; }

    void SelectEntry(const Color& rColor);

    void SetNoSelection() { getColorWindow()->SetNoSelection(); }
    bool IsNoSelection() const { return getColorWindow()->IsNoSelection(); }

    void ShowPreview(const NamedColor &rColor);
    void EnsurePaletteManager();

    void SaveValue() { m_aSaveColor = GetSelectEntryColor(); }
    bool IsValueChangedFromSaved() const { return m_aSaveColor != GetSelectEntryColor(); }

    void set_sensitive(bool sensitive) { m_xButton->set_sensitive(sensitive); }
    bool get_sensitive() const { return m_xButton->get_sensitive(); }
    void hide() { m_xButton->hide(); }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
