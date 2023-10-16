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
#ifndef INCLUDED_SFX2_TEMPLDLG_HXX
#define INCLUDED_SFX2_TEMPLDLG_HXX

#include <config_options.h>
#include <memory>
#include <sal/config.h>
#include <sfx2/dllapi.h>
#include <sfx2/sidebar/PanelLayout.hxx>
#include <sfx2/sidebar/ControllerItem.hxx>

class SfxBindings;
class SfxTemplateDialog_Impl;

class UNLESS_MERGELIBS(SFX2_DLLPUBLIC) SfxTemplatePanelControl final
    : public PanelLayout,
      public ::sfx2::sidebar::ControllerItem::ItemUpdateReceiverInterface
{
public:
    SfxTemplatePanelControl(SfxBindings* pBindings, weld::Widget* pParent);
    virtual ~SfxTemplatePanelControl() override;

    weld::Builder* get_builder() { return m_xBuilder.get(); }
    weld::Container* get_container() { return m_xContainer.get(); }

    virtual void NotifyItemUpdate(const sal_uInt16 nSId, const SfxItemState eState,
                                  const SfxPoolItem* pState) override;

    virtual void GetControlState(const sal_uInt16 /*nSId*/,
                                 boost::property_tree::ptree& /*rState*/) override{};

private:
    ::sfx2::sidebar::ControllerItem m_aSpotlightParaStyles;
    ::sfx2::sidebar::ControllerItem m_aSpotlightCharStyles;

    std::unique_ptr<SfxTemplateDialog_Impl> pImpl;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
