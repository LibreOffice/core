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

#include <sfx2/dllapi.h>
#include <vcl/customweld.hxx>
#include <vcl/virdev.hxx>
#include <vcl/weld.hxx>

class SAL_DLLPUBLIC_RTTI SvxCharView final : public weld::CustomWidgetController
{
private:
    VclPtr<VirtualDevice> mxVirDev;
    vcl::Font       maFont;
    bool            maHasInsert;
    OUString        m_sText;
    OUString        m_sToolTip;

    Link<SvxCharView&, void> maFocusInHdl;
    Link<SvxCharView&, void> maMouseClickHdl;
    Link<const CommandEvent&, void> maContextMenuHdl;

    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect) override;
    virtual void Resize() override;
    virtual bool MouseButtonDown(const MouseEvent&) override;
    virtual void GetFocus() override;
    virtual void LoseFocus() override;
    virtual OUString RequestHelp(tools::Rectangle&rHelpRect) override;
    virtual bool KeyInput(const KeyEvent&) override;
    virtual bool Command(const CommandEvent&) override;
    virtual void SetDrawingArea(weld::DrawingArea* pDrawingArea) override;

public:
    SvxCharView(const VclPtr<VirtualDevice>& rVirDev);
    SFX2_DLLPUBLIC virtual ~SvxCharView() override;

    void UpdateFont(const OUString& rFontFamilyName);
    vcl::Font const & GetFont() const { return maFont; }
    void            SetText( const OUString& rText );
    OUString const & GetText() const { return m_sText; }
    void SetToolTip(const OUString& rToolTip) { m_sToolTip = rToolTip; };
    void            SetHasInsert( bool bInsert );
    void            InsertCharToDoc();

    Size            get_preferred_size() const { return GetDrawingArea()->get_preferred_size(); }

    void setFocusInHdl(const Link<SvxCharView&, void>& rLink);
    void setMouseClickHdl(const Link<SvxCharView&, void>& rLink);
    void setContextMenuHdl(const Link<const CommandEvent&, void>& rLink);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
