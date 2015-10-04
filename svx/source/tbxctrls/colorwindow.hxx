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
#include <svtools/valueset.hxx>
#include <svl/lstner.hxx>
#include <rtl/ustring.hxx>
#include <com/sun/star/frame/XFrame.hpp>
#include <svx/SvxColorValueSet.hxx>
#include <svx/PaletteManager.hxx>
#include <vcl/lstbox.hxx>

#include <functional>

class BorderColorStatus;

class SvxColorWindow_Impl : public SfxPopupWindow
{
    using FloatingWindow::StateChanged;

private:
    const sal_uInt16    theSlotId;
    VclPtr<SvxColorValueSet>   mpColorSet;
    VclPtr<SvxColorValueSet>   mpRecentColorSet;

    VclPtr<ListBox>     mpPaletteListBox;
    VclPtr<PushButton>  mpButtonAutoColor;
    VclPtr<PushButton>  mpButtonPicker;
    VclPtr<FixedLine>   mpAutomaticSeparator;
    OUString            maCommand;
    Link<const Color&, void> maSelectedLink;

    PaletteManager&     mrPaletteManager;
    BorderColorStatus&  mrBorderColorStatus;

    std::function<void(const OUString&, const Color&)> maColorSelectFunction;

    DECL_LINK_TYPED( SelectHdl, ValueSet*, void );
    DECL_LINK_TYPED( SelectPaletteHdl, ListBox&, void);
    DECL_LINK_TYPED( AutoColorClickHdl, Button*, void );
    DECL_LINK_TYPED( OpenPickerClickHdl, Button*, void );

protected:
    virtual void    Resize() SAL_OVERRIDE;
    virtual bool    Close() SAL_OVERRIDE;

public:
    SvxColorWindow_Impl( const OUString& rCommand,
                         PaletteManager& rPaletteManager,
                         BorderColorStatus& rBorderColorStatus,
                         sal_uInt16 nSlotId,
                         const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame,
                         const OUString& rWndTitle,
                         vcl::Window* pParentWindow,
                         std::function<void(const OUString&, const Color&)> maColorSelectFunction);
    virtual ~SvxColorWindow_Impl();
    virtual void        dispose() SAL_OVERRIDE;
    void                StartSelection();

    virtual void        KeyInput( const KeyEvent& rKEvt ) SAL_OVERRIDE;
    virtual void        StateChanged( sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState ) SAL_OVERRIDE;

    void SetSelectedHdl( const Link<const Color&, void>& rLink ) { maSelectedLink = rLink; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
