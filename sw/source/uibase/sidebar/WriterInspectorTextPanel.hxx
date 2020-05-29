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
#pragma once

#include <sfx2/weldutils.hxx>
#include <sfx2/sidebar/ControllerItem.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <svx/sidebar/InspectorTextPanel.hxx>
namespace sw
{
namespace sidebar
{
class WriterInspectorTextPanel final
    : public svx::sidebar::InspectorTextPanel,
      public sfx2::sidebar::ControllerItem::ItemUpdateReceiverInterface
{
public:
    static VclPtr<vcl::Window> Create(vcl::Window* pParent,
                                      const css::uno::Reference<css::frame::XFrame>& rxFrame,
                                      SfxBindings* pBindings);

    WriterInspectorTextPanel(vcl::Window* pParent,
                             const css::uno::Reference<css::frame::XFrame>& rxFrame,
                             SfxBindings* pBindings);

    // virtual ~WriterInspectorTextPanel();
    virtual void NotifyItemUpdate(const sal_uInt16 nSId, const SfxItemState eState,
                                  const SfxPoolItem* pState) override;

    virtual void GetControlState(const sal_uInt16 /*nSId*/,
                                 boost::property_tree::ptree& /*rState*/) override{};

private:
    sfx2::sidebar::ControllerItem maCharStyle;

    static void GetPropertyValues(const css::beans::Property rProperty, const css::uno::Any& rAny,
                                  OUString& rString);
};
}
} // end of namespace svx::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
