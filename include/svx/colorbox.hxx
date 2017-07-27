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
#include <svx/colorwindow.hxx>
#include <sfx2/controlwrapper.hxx>

class SvxColorListBox;

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
    std::unique_ptr<PaletteManager> m_xPaletteManager;
    BorderColorStatus m_aBorderColorStatus;

    DECL_LINK(MenuActivateHdl, MenuButton *, void);
    void Selected(const NamedColor& rNamedColor);
    void createColorWindow();
    void LockWidthRequest();
    VclPtr<SvxColorWindow> getColorWindow() const;
public:
    SvxColorListBox(vcl::Window* pParent, WinBits nStyle = 0);
    virtual ~SvxColorListBox() override;
    virtual void dispose() override;

    void SetSelectHdl(const Link<SvxColorListBox&, void>& rLink)
    {
        m_aSelectedLink = rLink;
    }

    void SetSlotId(sal_uInt16 nSlotId, bool bShowNoneButton = false);

    Color GetSelectEntryColor() const { return m_aSelectedColor.first; }
    NamedColor GetSelectEntry() const { return m_aSelectedColor; }

    void SelectEntry(const NamedColor& rColor);
    void SelectEntry(const Color& rColor);

    void SetNoSelection() { getColorWindow()->SetNoSelection(); }
    bool IsNoSelection() const { return getColorWindow()->IsNoSelection(); }

    void SetAutoDisplayColor(const Color &rColor) { m_aAutoDisplayColor = rColor; }
    void ShowPreview(const NamedColor &rColor);
    void EnsurePaletteManager();

    void SaveValue() { m_aSaveColor = GetSelectEntryColor(); }
    bool IsValueChangedFromSaved() const { return m_aSaveColor != GetSelectEntryColor(); }
};

/** A wrapper for SvxColorListBox. */
class SVX_DLLPUBLIC SvxColorListBoxWrapper
    : public sfx::SingleControlWrapper<SvxColorListBox, Color>
{
    /*  Note: cannot use 'const Color&' as template argument, because the
        SvxColorListBox returns the color by value and not by reference,
        therefore GetControlValue() must return a temporary object too. */
public:
    explicit SvxColorListBoxWrapper(SvxColorListBox& rListBox);

    virtual ~SvxColorListBoxWrapper() override;

    virtual bool        IsControlDontKnow() const override;
    virtual void        SetControlDontKnow( bool bSet ) override;

    virtual Color       GetControlValue() const override;
    virtual void        SetControlValue( Color aColor ) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
