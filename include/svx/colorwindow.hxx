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

#include <sfx2/tbxctrl.hxx>
#include <svtools/toolbarmenu.hxx>
#include <svtools/valueset.hxx>
#include <svl/lstner.hxx>
#include <rtl/ustring.hxx>
#include <com/sun/star/frame/XFrame.hpp>
#include <svx/SvxColorValueSet.hxx>
#include <svx/PaletteManager.hxx>
#include <vcl/lstbox.hxx>

#include <functional>

class SVX_DLLPUBLIC BorderColorStatus
{
    Color maColor;
    Color maTLBRColor;
    Color maBLTRColor;
public:
    BorderColorStatus();
    ~BorderColorStatus();
    bool statusChanged( const css::frame::FeatureStateEvent& rEvent );
    Color GetColor();
};

class Button;
typedef std::function<void(const OUString&, const NamedColor&)> ColorSelectFunction;

#define COL_NONE_COLOR    TRGB_COLORDATA(0x80, 0xFF, 0xFF, 0xFF)

class SVX_DLLPUBLIC SvxColorWindow : public svtools::ToolbarPopup
{
private:
    const sal_uInt16    theSlotId;
    VclPtr<SvxColorValueSet>   mpColorSet;
    VclPtr<SvxColorValueSet>   mpRecentColorSet;

    VclPtr<ListBox>     mpPaletteListBox;
    VclPtr<PushButton>  mpButtonAutoColor;
    VclPtr<PushButton>  mpButtonNoneColor;
    VclPtr<PushButton>  mpButtonPicker;
    VclPtr<FixedLine>   mpAutomaticSeparator;
    OUString            maCommand;
    Link<const NamedColor&, void> maSelectedLink;

    std::shared_ptr<PaletteManager> mxPaletteManager;
    BorderColorStatus&  mrBorderColorStatus;

    ColorSelectFunction maColorSelectFunction;

    DECL_LINK( SelectHdl, ValueSet*, void );
    DECL_LINK( SelectPaletteHdl, ListBox&, void);
    DECL_LINK( AutoColorClickHdl, Button*, void );
    DECL_LINK( OpenPickerClickHdl, Button*, void );

    static bool SelectValueSetEntry(SvxColorValueSet* pColorSet, const Color& rColor);
    static NamedColor GetSelectEntryColor(ValueSet const * pColorSet);
    NamedColor GetAutoColor() const;

public:
    SvxColorWindow(const OUString& rCommand,
                   std::shared_ptr<PaletteManager>& rPaletteManager,
                   BorderColorStatus& rBorderColorStatus,
                   sal_uInt16 nSlotId,
                   const css::uno::Reference< css::frame::XFrame >& rFrame,
                   vcl::Window* pParentWindow,
                   ColorSelectFunction const& rColorSelectFunction);
    virtual ~SvxColorWindow() override;
    virtual void        dispose() override;
    void                ShowNoneButton();
    void                StartSelection();
    void                SetNoSelection();
    bool                IsNoSelection() const;
    void                SelectEntry(const NamedColor& rColor);
    void                SelectEntry(const Color& rColor);
    NamedColor          GetSelectEntryColor() const;

    virtual void        KeyInput( const KeyEvent& rKEvt ) override;
    virtual void        statusChanged( const css::frame::FeatureStateEvent& rEvent ) override;

    void SetSelectedHdl( const Link<const NamedColor&, void>& rLink ) { maSelectedLink = rLink; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
