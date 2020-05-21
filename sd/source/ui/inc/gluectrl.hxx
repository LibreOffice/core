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

#ifndef INCLUDED_SD_SOURCE_UI_INC_GLUECTRL_HXX
#define INCLUDED_SD_SOURCE_UI_INC_GLUECTRL_HXX

#include <vcl/InterimItemWindow.hxx>
#include <sfx2/tbxctrl.hxx>

enum class SdrEscapeDirection;

/**
 * GluePointEscDirLB
 */
class GlueEscDirLB final : public InterimItemWindow
{
private:
    css::uno::Reference< css::frame::XFrame > m_xFrame;
    std::unique_ptr<weld::ComboBox> m_xWidget;

    DECL_LINK(SelectHdl, weld::ComboBox&, void);
    DECL_LINK(KeyInputHdl, const KeyEvent&, bool);

public:
    GlueEscDirLB(vcl::Window* pParent, const css::uno::Reference<css::frame::XFrame>& rFrame);
    virtual void dispose() override;
    virtual ~GlueEscDirLB() override;

    virtual void GetFocus() override;

    void set_active(int nPos) { m_xWidget->set_active(nPos); }
    void set_sensitive(bool bSensitive);

    void Fill();
};

/**
 * Toolbox controller for glue-point escape direction
 */
class SdTbxCtlGlueEscDir: public SfxToolBoxControl
{
private:
    static sal_uInt16  GetEscDirPos( SdrEscapeDirection nEscDir );

public:
    virtual void StateChanged( sal_uInt16 nSId, SfxItemState eState,
                                const SfxPoolItem* pState ) override;
    virtual VclPtr<InterimItemWindow> CreateItemWindow( vcl::Window *pParent ) override;

            SFX_DECL_TOOLBOX_CONTROL();

            SdTbxCtlGlueEscDir( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx );
};

#endif // INCLUDED_SD_SOURCE_UI_INC_GLUECTRL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
