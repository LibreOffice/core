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

#include "NavigatorChildWindow.hxx"
#include "navigatr.hxx"
#include "app.hrc"
#include "navigatr.hrc"
#include <sfx2/app.hxx>

namespace sd {

SFX_IMPL_CHILDWINDOWCONTEXT(NavigatorChildWindow, SID_NAVIGATOR)

NavigatorChildWindow::NavigatorChildWindow (
    ::Window* pParent,
    sal_uInt16 nId,
    SfxBindings* pBindings,
    SfxChildWinInfo* )
    : SfxChildWindowContext( nId )
{
    SdNavigatorWin* pNavWin = new SdNavigatorWin( pParent, this,
                                        SdResId( FLT_NAVIGATOR ), pBindings );

    SetWindow( pNavWin );
}

} // end of namespace sd
