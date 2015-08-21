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
#ifndef INCLUDED_SW_SOURCE_UIBASE_SIDEBAR_PAGEMARGINPANEL_HXX
#define INCLUDED_SW_SOURCE_UIBASE_SIDEBAR_PAGEMARGINPANEL_HXX

#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/document/XUndoManager.hpp>

#include <svx/sidebar/Popup.hxx>
#include <svx/sidebar/PanelLayout.hxx>

#include <sfx2/sidebar/ControllerItem.hxx>

#include <i18nutil/paper.hxx>

#include <svx/pageitem.hxx>
#include <svx/rulritem.hxx>
#include <editeng/sizeitem.hxx>

#include <vcl/ctrl.hxx>
#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/field.hxx>
#include <svl/intitem.hxx>
#include <tools/fldunit.hxx>
#include <svl/poolitem.hxx>
#include <svx/rulritem.hxx>

namespace sw { namespace sidebar {

class PageMarginPanel:
    public PanelLayout,
    public ::sfx2::sidebar::ControllerItem::ItemUpdateReceiverInterface
{
public:
    static VclPtr<vcl::Window> Create(
        vcl::Window* pParent,
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rxFrame,
        SfxBindings* pBindings);

    virtual void NotifyItemUpdate(
        const sal_uInt16 nSID,
        const SfxItemState eState,
        const SfxPoolItem* pState,
        const bool bIsEnabled);

    SfxBindings* GetBindings() const { return mpBindings; }

    PageMarginPanel(
        vcl::Window* pParent,
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rxFrame,
        SfxBindings* pBindings);
    virtual ~PageMarginPanel();
    virtual void dispose();

private:

    SfxBindings* mpBindings;

    VclPtr<MetricField> mpLeftMarginEdit;
    VclPtr<MetricField> mpRightMarginEdit;
    VclPtr<MetricField> mpTopMarginEdit;
    VclPtr<MetricField> mpBottomMarginEdit;

    FieldUnit meFUnit;
    SfxMapUnit meUnit;

    long mnPageLeftMargin;
    long mnPageRightMargin;
    long mnPageTopMargin;
    long mnPageBottomMargin;

    void Initialize();
    void ExecuteMarginLRChange( const long nPageLeftMargin, const long nPageRightMargin );
    void ExecuteMarginULChange( const long nPageTopMargin, const long  nPageBottomMargin);
    void MetricState( SfxItemState eState, const SfxPoolItem* pState );

    std::unique_ptr<SvxLongLRSpaceItem> mpPageLRMarginItem;
    std::unique_ptr<SvxLongULSpaceItem> mpPageULMarginItem;

    ::sfx2::sidebar::ControllerItem maSwPageLRControl;
    ::sfx2::sidebar::ControllerItem maSwPageULControl;
    ::sfx2::sidebar::ControllerItem maSwPageSizeControl;
    ::sfx2::sidebar::ControllerItem maSwPageMetricControl;

    DECL_LINK_TYPED( ModifyLRMarginHdl, Edit&, void );
    DECL_LINK_TYPED( ModifyULMarginHdl, Edit&, void );

};

} } // end of namespace sw::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
