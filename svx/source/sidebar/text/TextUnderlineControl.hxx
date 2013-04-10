/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/
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
    TextUnderlineControl(Window* pParent, svx::sidebar::TextPropertyPanel& rPanel);
    void GetFocus();
    void Rearrange(FontUnderline eLine);
    ValueSet& GetValueSet();
    Control& GetPB();
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

    //add  for high contract
    Image               maIMGSingleH;
    Image               maIMGDoubleH;
    Image               maIMGBoldH;
    Image               maIMGDotH;
    Image               maIMGDotBoldH;
    Image               maIMGDashH;
    Image               maIMGDashLongH;
    Image               maIMGDashDotH;
    Image               maIMGDashDotDotH;
    Image               maIMGWaveH;

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

