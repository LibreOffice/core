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

#ifndef _SVX_SIDEBAR_UNDERLINE_CONTROL_HXX_
#define _SVX_SIDEBAR_UNDERLINE_CONTROL_HXX_

#include "svx/sidebar/PopupControl.hxx"
#include <sfx2/bindings.hxx>
#include <svtools/ctrlbox.hxx>
#include <svtools/ctrltool.hxx>
#include "TextPropertyPanel.hxx"
#include <vcl/fixed.hxx>
#include <vcl/button.hxx>

#include <vcl/vclenum.hxx>
#include <svtools/valueset.hxx>

namespace svx{ namespace sidebar {

class TextUnderlineControl:public svx::sidebar::PopupControl
{
public:
    TextUnderlineControl (
        Window* pParent,
        svx::sidebar::TextPropertyPanel& rPanel,
        SfxBindings* pBindings);
    void Rearrange(FontUnderline eLine);

private:
    svx::sidebar::TextPropertyPanel&     mrTextPropertyPanel;
    SfxBindings*        mpBindings;
    ValueSet            maVSUnderline;
    PushButton          maPBOptions;

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

    DECL_LINK( PBClickHdl, PushButton *);
    DECL_LINK(VSSelectHdl, void *);
};
}}


#endif

