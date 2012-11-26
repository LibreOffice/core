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

#include "scitems.hxx"
#include <svl/srchitem.hxx>
#include <sfx2/app.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/request.hxx>
#include <svl/whiter.hxx>

#include "pgbrksh.hxx"
#include "tabvwsh.hxx"
#include "scresid.hxx"
#include "document.hxx"
#include "sc.hrc"

//------------------------------------------------------------------------

#define ScPageBreakShell
#include "scslots.hxx"

//------------------------------------------------------------------------

SFX_IMPL_INTERFACE(ScPageBreakShell, SfxShell, ScResId(SCSTR_PAGEBREAKSHELL))
{
    SFX_POPUPMENU_REGISTRATION( ScResId(RID_POPUP_PAGEBREAK) );
}


//------------------------------------------------------------------------
ScPageBreakShell::ScPageBreakShell( ScTabViewShell* pViewSh ) :
    SfxShell(pViewSh)
{
    SetPool( &pViewSh->GetPool() );
    ScViewData* pViewData = pViewSh->GetViewData();
    ::svl::IUndoManager* pMgr = pViewData->GetSfxDocShell()->GetUndoManager();
    SetUndoManager( pMgr );
    if ( !pViewData->GetDocument()->IsUndoEnabled() )
    {
        pMgr->SetMaxUndoActionCount( 0 );
    }
    SetHelpId( HID_SCSHELL_PAGEBREAK );
    SetName(String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("PageBreak")));
}

//------------------------------------------------------------------------
ScPageBreakShell::~ScPageBreakShell()
{
}


