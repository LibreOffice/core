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
#include "precompiled_sfx2.hxx"

#include <limits.h>
#include <stdlib.h>
#include <vcl/msgbox.hxx>
#include <unotools/viewoptions.hxx>

#define _SVSTDARR_USHORTS
#include <svl/svstdarr.hxx>

#include "appdata.hxx"
#include "sfxtypes.hxx"
#include <sfx2/minarray.hxx>
#include <sfx2/tabdlg.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/app.hxx>
#include "sfx2/sfxresid.hxx"
#include "sfx2/sfxhelp.hxx"
#include <sfx2/ctrlitem.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/sfxdlg.hxx>
#include <sfx2/itemconnect.hxx>
#include <layout/layout.hxx>

//FIXME
#include <toolkit/awt/vclxwindow.hxx>
#include <com/sun/star/awt/PosSize.hpp>

using namespace ::com::sun::star;
namespace layout
{
using namespace ::com::sun::star;
}

#include <sfx2/layout.hxx>
#include <layout/layout-pre.hxx>

#undef TabPage
#undef SfxTabPage
#undef SfxTabDialog

#define TabDlgCmpUS_Impl layout__TabDlgCmpUS_Impl

#include "../dialog/tabdlg.cxx"
