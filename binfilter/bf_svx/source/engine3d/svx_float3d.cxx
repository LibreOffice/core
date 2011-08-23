/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifdef _MSC_VER
#pragma hdrstop
#endif

#include <bf_sfx2/dispatch.hxx>

#include <bf_sfx2/dockwin.hxx>

#include <bf_svx/f3dchild.hxx>

#include "float3d.hrc"
namespace binfilter {

/*N*/ SFX_IMPL_DOCKINGWINDOW( Svx3DChildWindow, SID_3D_WIN )


/*************************************************************************
|*
|* Ableitung vom SfxChildWindow als "Behaelter" fuer Effekte
|*
\************************************************************************/
/*N*/ __EXPORT Svx3DChildWindow::Svx3DChildWindow( Window* pParent,
/*N*/ 														 USHORT nId,
/*N*/ 														 SfxBindings* pBindings,
/*N*/ 														 SfxChildWinInfo* pInfo ) :
/*N*/ 	SfxChildWindow( pParent, nId )
/*N*/ {
DBG_BF_ASSERT(0, "STRIP"); //STRIP001 	Svx3DWin* pWin = new Svx3DWin( pBindings, this, pParent );
/*N*/ }


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
