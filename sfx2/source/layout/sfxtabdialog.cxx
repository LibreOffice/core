/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
