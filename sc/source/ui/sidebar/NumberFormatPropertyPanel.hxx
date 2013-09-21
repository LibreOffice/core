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
#ifndef SC_PROPERTYPANEL_NUMFORMAT_HXX
#define SC_PROPERTYPANEL_NUMFORMAT_HXX

#include <sfx2/sidebar/ControllerItem.hxx>
#include <sfx2/sidebar/IContextChangeReceiver.hxx>
#include <svx/sidebar/PanelLayout.hxx>
#include <boost/scoped_ptr.hpp>

class FixedText;
class ListBox;
class NumericField;

namespace sc { namespace sidebar {

class NumberFormatPropertyPanel
:   public PanelLayout,
    public ::sfx2::sidebar::IContextChangeReceiver,
    public ::sfx2::sidebar::ControllerItem::ItemUpdateReceiverInterface
{
public:
public:
    static NumberFormatPropertyPanel* Create(
        Window* pParent,
        const cssu::Reference<css::frame::XFrame>& rxFrame,
        SfxBindings* pBindings);

    virtual void DataChanged(
        const DataChangedEvent& rEvent);

    virtual void HandleContextChange(
        const ::sfx2::sidebar::EnumContext aContext);

    virtual void NotifyItemUpdate(
        const sal_uInt16 nSId,
        const SfxItemState eState,
        const SfxPoolItem* pState,
        const bool bIsEnabled);

    SfxBindings* GetBindings();

private:
    //ui controls
    ListBox*                                mpLbCategory;
    ToolBox*                                mpTBCategory;
    NumericField*                           mpEdDecimals;
    NumericField*                           mpEdLeadZeroes;
    CheckBox*                               mpBtnNegRed;
    CheckBox*                               mpBtnThousand;

    ::sfx2::sidebar::ControllerItem         maNumFormatControl;
    ::sfx2::sidebar::ControllerItem         maFormatControl;

    sal_uInt16                              mnCategorySelected;

    cssu::Reference<css::frame::XFrame>     mxFrame;
    ::sfx2::sidebar::EnumContext            maContext;
    SfxBindings*                            mpBindings;

    DECL_LINK(NumFormatHdl, ToolBox*);
    DECL_LINK(NumFormatSelectHdl, ListBox*);
    DECL_LINK(NumFormatValueHdl, void*);

    // constructor/destuctor
    NumberFormatPropertyPanel(
        Window* pParent,
        const cssu::Reference<css::frame::XFrame>& rxFrame,
        SfxBindings* pBindings);
    virtual ~NumberFormatPropertyPanel();

    void Initialize();
};

} } // end of namespace ::sc::sidebar

#endif

// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
