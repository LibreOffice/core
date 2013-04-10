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
#include "precompiled_sc.hxx"



//------------------------------------------------------------------

#include <editeng/eeitem.hxx>
#include <svx/fontwork.hxx>
//CHINA001 #include <svx/labdlg.hxx>
#include <svl/srchitem.hxx>
#include <svx/tabarea.hxx>
#include <svx/tabline.hxx>
//CHINA001 #include <svx/transfrm.hxx>
#include <sfx2/app.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/request.hxx>
#include <svl/whiter.hxx>
#include <vcl/msgbox.hxx>

#include "oleobjsh.hxx"
#include "drwlayer.hxx"
#include "sc.hrc"
#include "viewdata.hxx"
#include "document.hxx"
#include "docpool.hxx"
#include "drawview.hxx"
#include "scresid.hxx"
#include <svx/svdobj.hxx>
#include <sfx2/sidebar/EnumContext.hxx>

#define ScOleObjectShell
#include "scslots.hxx"


SFX_IMPL_INTERFACE(ScOleObjectShell, ScDrawShell, ScResId(SCSTR_OLEOBJECTSHELL) )
{
    SFX_OBJECTBAR_REGISTRATION( SFX_OBJECTBAR_OBJECT|SFX_VISIBILITY_STANDARD|SFX_VISIBILITY_SERVER,
                                ScResId(RID_DRAW_OBJECTBAR) );
    SFX_POPUPMENU_REGISTRATION( ScResId(RID_POPUP_OLE) );
}

TYPEINIT1( ScOleObjectShell, ScDrawShell );

ScOleObjectShell::ScOleObjectShell(ScViewData* pData) :
    ScDrawShell(pData)
{
    SetHelpId(HID_SCSHELL_OLEOBEJCTSH);
    SetName(String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("OleObject")));
    SfxShell::SetContextName(sfx2::sidebar::EnumContext::GetContextName(sfx2::sidebar::EnumContext::Context_OLE));
}

ScOleObjectShell::~ScOleObjectShell()
{
}


void ScOleObjectShell::HandleSelectionChange (void)
{
    // Do not call the implementation in the base class.  Let
    // Activate()/Deactivate() handle context switches.
}
