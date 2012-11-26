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
#include "precompiled_sd.hxx"

#include "OutlineViewShellBase.hxx"
#include "sdresid.hxx"
#include "DrawDocShell.hxx"
#include "strings.hrc"
#include "framework/FrameworkHelper.hxx"

namespace sd {

class DrawDocShell;

// We have to expand the SFX_IMPL_VIEWFACTORY macro to call LateInit() after a
// new OutlineViewShellBase object has been constructed.

SfxViewFactory* OutlineViewShellBase::pFactory;
SfxViewShell* __EXPORT OutlineViewShellBase::CreateInstance (
    SfxViewFrame *pFrame, SfxViewShell *pOldView)
{
    OutlineViewShellBase* pBase = new OutlineViewShellBase(pFrame, pOldView);
    pBase->LateInit(framework::FrameworkHelper::msOutlineViewURL);
    return pBase;
}
void OutlineViewShellBase::RegisterFactory( sal_uInt16 nPrio )
{
    pFactory = new SfxViewFactory(
        &CreateInstance,&InitFactory,nPrio,"Outline");
    InitFactory();
}
void OutlineViewShellBase::InitFactory()
{
    SFX_VIEW_REGISTRATION(DrawDocShell);
}




OutlineViewShellBase::OutlineViewShellBase (
    SfxViewFrame* _pFrame,
    SfxViewShell* pOldShell)
    : ImpressViewShellBase (_pFrame, pOldShell)
{
}




OutlineViewShellBase::~OutlineViewShellBase (void)
{
}




} // end of namespace sd

