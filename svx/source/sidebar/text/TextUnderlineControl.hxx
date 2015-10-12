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

#ifndef INCLUDED_SVX_SOURCE_SIDEBAR_TEXT_TEXTUNDERLINECONTROL_HXX
#define INCLUDED_SVX_SOURCE_SIDEBAR_TEXT_TEXTUNDERLINECONTROL_HXX

#include "svx/sidebar/PopupControl.hxx"
#include <sfx2/bindings.hxx>
#include "TextPropertyPanel.hxx"
#include <vcl/button.hxx>
#include <vcl/vclenum.hxx>
#include <svtools/valueset.hxx>

namespace svx{ namespace sidebar {

class TextUnderlineControl:public svx::sidebar::PopupControl
{
public:
    TextUnderlineControl (
        vcl::Window* pParent,
        svx::sidebar::TextPropertyPanel& rPanel,
        SfxBindings* pBindings);
    virtual ~TextUnderlineControl();
    virtual void dispose() override;
    void Rearrange(FontUnderline eLine);

private:
    svx::sidebar::TextPropertyPanel&     mrTextPropertyPanel;
    SfxBindings*        mpBindings;
    VclPtr<ValueSet>    maVSUnderline;
    VclPtr<PushButton>  maPBOptions;

    Image               maIMGSingle;
    Image               maIMGDouble;
    Image               maIMGBold;
    Image               maIMGDot;
    Image               maIMGDotBold;
    Image               maIMGDash;
    Image               maIMGDashLong;
    Image               maIMGDashDot;
    Image               maIMGDashDotDot;
    Image               maIMGWave;

    Image               maIMGSingleSel;
    Image               maIMGDoubleSel;
    Image               maIMGBoldSel;
    Image               maIMGDotSel;
    Image               maIMGDotBoldSel;
    Image               maIMGDashSel;
    Image               maIMGDashLongSel;
    Image               maIMGDashDotSel;
    Image               maIMGDashDotDotSel;
    Image               maIMGWaveSel;

    void initial();

    DECL_LINK_TYPED( PBClickHdl, Button*, void);
    DECL_LINK_TYPED(VSSelectHdl, ValueSet*, void);
};
}}


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
