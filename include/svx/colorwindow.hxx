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

#ifndef INCLUDED_SVX_SOURCE_TBXCTRLS_COLORWINDOW_HXX
#define INCLUDED_SVX_SOURCE_TBXCTRLS_COLORWINDOW_HXX

#include <svtools/toolbarmenu.hxx>
#include <rtl/ustring.hxx>
#include <svx/SvxColorValueSet.hxx>
#include <svx/Palette.hxx>
#include <vcl/toolbox.hxx>

typedef std::function<weld::Window*()> TopLevelParentFunction;

namespace com::sun::star::frame { class XFrame; }

class PaletteManager;

class SVXCORE_DLLPUBLIC ColorStatus
{
    Color maColor;
    Color maTLBRColor;
    Color maBLTRColor;
public:
    ColorStatus();
    ~ColorStatus();
    void statusChanged( const css::frame::FeatureStateEvent& rEvent );
    Color GetColor();
};

class Button;

#define COL_NONE_COLOR    ::Color(ColorTransparency, 0x80, 0xFF, 0xFF, 0xFF)

class SvxColorToolBoxControl;

class SVXCORE_DLLPUBLIC MenuOrToolMenuButton
{
private:
    // either
    weld::MenuButton* m_pMenuButton;
    // or
    weld::Toolbar* m_pToolbar;
    OString m_aIdent;
    // or
    SvxColorToolBoxControl* m_pControl;
    VclPtr<ToolBox> m_xToolBox;
    sal_uInt16 m_nId;
public:
    MenuOrToolMenuButton(weld::MenuButton* pMenuButton);
    MenuOrToolMenuButton(weld::Toolbar* pToolbar, const OString& rIdent);
    MenuOrToolMenuButton(SvxColorToolBoxControl* pControl, ToolBox* pToolbar, sal_uInt16 nId);
    ~MenuOrToolMenuButton();

    MenuOrToolMenuButton(MenuOrToolMenuButton const &) = default;
    MenuOrToolMenuButton(MenuOrToolMenuButton &&) = default;
    MenuOrToolMenuButton & operator =(MenuOrToolMenuButton const &) = default;
    MenuOrToolMenuButton & operator =(MenuOrToolMenuButton &&) = default;

    bool get_active() const;
    void set_inactive() const;
    weld::Widget* get_widget() const;
};

class SVXCORE_DLLPUBLIC ColorWindow final : public WeldToolbarPopup
{
private:
    const sal_uInt16    theSlotId;
    OUString            maCommand;
    MenuOrToolMenuButton maMenuButton;
    std::shared_ptr<PaletteManager> mxPaletteManager;
    ColorStatus& mrColorStatus;
    TopLevelParentFunction maTopLevelParentFunction;
    ColorSelectFunction maColorSelectFunction;

    std::unique_ptr<SvxColorValueSet> mxColorSet;
    std::unique_ptr<SvxColorValueSet> mxRecentColorSet;
    std::unique_ptr<weld::ComboBox> mxPaletteListBox;
    std::unique_ptr<weld::Button> mxButtonAutoColor;
    std::unique_ptr<weld::Button> mxButtonNoneColor;
    std::unique_ptr<weld::Button> mxButtonPicker;
    std::unique_ptr<weld::Widget> mxAutomaticSeparator;
    std::unique_ptr<weld::CustomWeld> mxColorSetWin;
    std::unique_ptr<weld::CustomWeld> mxRecentColorSetWin;
    weld::Button* mpDefaultButton;

    Link<const NamedColor&, void> maSelectedLink;
    DECL_LINK(SelectHdl, ValueSet*, void);
    DECL_LINK(SelectPaletteHdl, weld::ComboBox&, void);
    DECL_LINK(AutoColorClickHdl, weld::Button&, void);
    DECL_LINK(OpenPickerClickHdl, weld::Button&, void);

    static bool SelectValueSetEntry(SvxColorValueSet* pColorSet, const Color& rColor);
    static NamedColor GetSelectEntryColor(ValueSet const * pColorSet);
    NamedColor GetAutoColor() const;

public:
    ColorWindow(const OUString& rCommand,
                std::shared_ptr<PaletteManager> const & rPaletteManager,
                ColorStatus& rColorStatus,
                sal_uInt16 nSlotId,
                const css::uno::Reference<css::frame::XFrame>& rFrame,
                const MenuOrToolMenuButton &rMenuButton,
                TopLevelParentFunction const& rTopLevelParentFunction,
                ColorSelectFunction const& rColorSelectFunction);
    virtual ~ColorWindow() override;
    void                ShowNoneButton();
    void                SetNoSelection();
    bool                IsNoSelection() const;
    void                SelectEntry(const NamedColor& rColor);
    void                SelectEntry(const Color& rColor);
    NamedColor          GetSelectEntryColor() const;

    virtual void        statusChanged( const css::frame::FeatureStateEvent& rEvent ) override;

    void SetSelectedHdl( const Link<const NamedColor&, void>& rLink ) { maSelectedLink = rLink; }

    virtual void GrabFocus() override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
