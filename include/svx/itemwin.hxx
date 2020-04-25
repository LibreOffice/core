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
#ifndef INCLUDED_SVX_ITEMWIN_HXX
#define INCLUDED_SVX_ITEMWIN_HXX

#include <svtools/InterimItemWindow.hxx>
#include <svtools/toolbarmenu.hxx>
#include <svx/dlgctrl.hxx>
#include <svx/svxdllapi.h>

class XLineWidthItem;
class SfxObjectShell;
class ValueSet;
class SvxLineStyleToolBoxControl;

class SvxLineBox final : public WeldToolbarPopup
{
    rtl::Reference<SvxLineStyleToolBoxControl> mxControl;
    std::unique_ptr<ValueSet> mxLineStyleSet;
    std::unique_ptr<weld::CustomWeld> mxLineStyleSetWin;

    void FillControl();
    void Fill(const XDashListRef &pList);

    DECL_LINK(SelectHdl, ValueSet*, void);

    virtual void GrabFocus() override;

public:
    SvxLineBox(SvxLineStyleToolBoxControl* pControl, weld::Widget* pParent, int nInitialIndex);
    virtual ~SvxLineBox() override;
};

class SvxMetricField final : public InterimItemWindow
{
private:
    std::unique_ptr<weld::MetricSpinButton> m_xWidget;
    int             nCurValue;
    MapUnit         ePoolUnit;
    FieldUnit       eDlgUnit;
    css::uno::Reference< css::frame::XFrame > mxFrame;

    DECL_LINK(ModifyHdl, weld::MetricSpinButton&, void);
    DECL_LINK(KeyInputHdl, const KeyEvent&, bool);
    DECL_LINK(FocusInHdl, weld::Widget&, void);

    static void     ReleaseFocus_Impl();

    virtual void    DataChanged( const DataChangedEvent& rDCEvt ) override;

    virtual void GetFocus() override;

public:
    SvxMetricField( vcl::Window* pParent,
                    const css::uno::Reference< css::frame::XFrame >& rFrame );
    virtual void dispose() override;
    virtual ~SvxMetricField() override;

    void            Update( const XLineWidthItem* pItem );
    void            SetCoreUnit( MapUnit eUnit );
    void            RefreshDlgUnit();

    void            set_sensitive(bool bSensitive);
};

namespace SvxFillTypeBox
{
    SVX_DLLPUBLIC void Fill(weld::ComboBox& rListBox);
}

namespace SvxFillAttrBox
{
    SVX_DLLPUBLIC void Fill(weld::ComboBox&, const XHatchListRef &pList);
    SVX_DLLPUBLIC void Fill(weld::ComboBox&, const XGradientListRef &pList);
    SVX_DLLPUBLIC void Fill(weld::ComboBox&, const XBitmapListRef &pList);
    SVX_DLLPUBLIC void Fill(weld::ComboBox&, const XPatternListRef &pList);
}

#endif // INCLUDED_SVX_ITEMWIN_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
