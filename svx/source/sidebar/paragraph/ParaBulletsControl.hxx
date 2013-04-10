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
#ifndef _SVX_SIDEBAR_BULLETS_CONTROL_HXX_
#define _SVX_SIDEBAR_BULLETS_CONTROL_HXX_

#include "svx/sidebar/PopupControl.hxx"
#include "svx/sidebar/ValueSetWithTextControl.hxx"
#include <sfx2/bindings.hxx>
#include <svtools/ctrlbox.hxx>
#include <svtools/ctrltool.hxx>
#include "ParaPropertyPanel.hxx"
#include <vcl/fixed.hxx>
#include <svl/poolitem.hxx>
#include <editeng/lspcitem.hxx>
#include <sfx2/sidebar/EnumContext.hxx>
#include <vcl/button.hxx>


namespace svx { namespace sidebar {

class ParaBulletsControl:public svx::sidebar::PopupControl
{
private:
    SvxNumValueSet3             maBulletsVS;
    FixedImage                  maFISep;
    PushButton maMoreButton;
    ParaPropertyPanel&     mrParaPropertyPanel;
    SfxBindings*        mpBindings;

    DECL_LINK(BulletSelectHdl_Impl, ValueSet*);
    DECL_LINK(MoreButtonClickHdl_Impl, void*);

public:
    ParaBulletsControl(Window* pParent, svx::sidebar::ParaPropertyPanel& rPanel);
    ~ParaBulletsControl();
    void ToGetFocus();
    void UpdateValueSet();
};

}}

#endif

