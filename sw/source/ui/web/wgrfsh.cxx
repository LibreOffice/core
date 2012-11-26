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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


#include <sfx2/msg.hxx>
#include <svl/srchitem.hxx>
#include <sfx2/app.hxx>

#include <sfx2/objface.hxx>

#include "cmdid.h"
#include "view.hxx"
#include "wrtsh.hxx"
#include "swmodule.hxx"
#include "uitool.hxx"
#include "docsh.hxx"
#include "shells.hrc"
#include "popup.hrc"
#include "globals.hrc"
#include "web.hrc"
#include "wgrfsh.hxx"

#define SwWebGrfShell
#include "swslots.hxx"

SFX_IMPL_INTERFACE(SwWebGrfShell, SwGrfShell, SW_RES(STR_SHELLNAME_GRAPHIC))
{
    SFX_POPUPMENU_REGISTRATION(SW_RES(MN_GRF_POPUPMENU));
    SFX_OBJECTBAR_REGISTRATION(SFX_OBJECTBAR_OBJECT, SW_RES(RID_WEBGRAPHIC_TOOLBOX));
}

SwWebGrfShell::SwWebGrfShell(SwView &_rView) :
    SwGrfShell(_rView)

{
    SetName(String::CreateFromAscii("Graphic"));
    SetHelpId(SW_GRFSHELL);
}

SwWebGrfShell::~SwWebGrfShell()
{
}



