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



#include "hintids.hxx"
#include <sfx2/app.hxx>
#include <tools/globname.hxx>
#include <sfx2/objface.hxx>
#include <svl/srchitem.hxx>


#include "cmdid.h"
#include "globals.hrc"
#include "uitool.hxx"
#include "helpid.h"
#include "popup.hrc"
#include "shells.hrc"
#include "table.hrc"
#include "wrtsh.hxx"
#include "wtabsh.hxx"

#define SwWebTableShell
#include <sfx2/msg.hxx>
#include "svx/svxids.hrc"
#include "swslots.hxx"

SFX_IMPL_INTERFACE(SwWebTableShell, SwTableShell, SW_RES(STR_SHELLNAME_TABLE))
{
    SFX_POPUPMENU_REGISTRATION(SW_RES(MN_TAB_POPUPMENU));
    SFX_OBJECTBAR_REGISTRATION(SFX_OBJECTBAR_OBJECT, SW_RES(RID_TABLE_TOOLBOX));
}

SwWebTableShell::SwWebTableShell(SwView &_rView) :
    SwTableShell(_rView)
{
    GetShell().UpdateTable();
    SetName(String::CreateFromAscii("Table"));
    SetHelpId(SW_TABSHELL);
}

__EXPORT SwWebTableShell::~SwWebTableShell()
{
}



