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


#include <sfx2/app.hxx>
#include <sfx2/request.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/objitem.hxx>
#include <sfx2/sidebar/EnumContext.hxx>
#include <wrtsh.hxx>
#include <view.hxx>
#ifndef _HELPID_H
#include <helpid.h>
#endif
#ifndef _GLOBALS_HRC
#include <globals.hrc>
#endif
#include <frmsh.hxx>
#ifndef _OLESH_HXX
#include <olesh.hxx>
#endif

#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _POPUP_HRC
#include <popup.hrc>
#endif
#ifndef _SHELLS_HRC
#include <shells.hrc>
#endif


#define SwOleShell
#include <sfx2/msg.hxx>
#include <swslots.hxx>


SFX_IMPL_INTERFACE(SwOleShell, SwFrameShell, SW_RES(STR_SHELLNAME_OBJECT))
{
    SFX_POPUPMENU_REGISTRATION(SW_RES(MN_OLE_POPUPMENU));
    SFX_OBJECTBAR_REGISTRATION(SFX_OBJECTBAR_OBJECT, SW_RES(RID_OLE_TOOLBOX));
}


SwOleShell::SwOleShell(SwView &_rView) :
    SwFrameShell(_rView)

{
    SetName(String::CreateFromAscii("Object"));
    SetHelpId(SW_OLESHELL);
    SfxShell::SetContextName(sfx2::sidebar::EnumContext::GetContextName(sfx2::sidebar::EnumContext::Context_OLE));
}
