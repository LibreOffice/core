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

#include <vcl/menubtn.hxx>
#include <svx/colorwindow.hxx>

class SvxColorListBox;

class SvxListBoxColorWrapper
{
public:
    SvxListBoxColorWrapper(SvxColorListBox* pControl);
    void operator()(const OUString& rCommand, const NamedColor& rColor);
private:
    SvxColorListBox* mpControl;
};

class SVT_DLLPUBLIC SvxColorListBox : public MenuButton
{
private:
    friend class SvxListBoxColorWrapper;
    VclPtr<SvxColorWindow> m_xColorWindow;
    Link<const NamedColor&, void> m_aSelectedLink;
    SvxListBoxColorWrapper m_aColorWrapper;
    Color m_aSelectedColor;
    sal_uInt16 m_nSlotId;
    PaletteManager m_aPaletteManager;
    BorderColorStatus m_aBorderColorStatus;

    DECL_LINK(MenuActivateHdl, MenuButton *, void);
    void Selected(const NamedColor& rNamedColor);
    void createColorWindow();
    void LockWidthRequest();
public:
    SvxColorListBox(vcl::Window* pParent);
    virtual ~SvxColorListBox() override;
    virtual void dispose() override;

    void SetSelectHdl(const Link<const NamedColor&, void>& rLink)
    {
        m_aSelectedLink = rLink;
    }

    void SetSlotId(sal_uInt16 nSlotId);

    Color GetSelectEntryColor() const
    {
        return m_aSelectedColor;
    }

    void SelectEntry(const Color& rColor);

    void SetNoSelection() { m_xColorWindow->SetNoSelection(); }

    void ShowPreview(const NamedColor &rColor);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
