/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: windata.hxx,v $
 * $Revision: 1.4 $
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

#ifndef _SV_WINDATA_HXX
#define _SV_WINDATA_HXX

#include <vcl/sv.h>

class SalFrame;
class Window;

// --------------
// - Prototypes -
// --------------

long ImplWindowFrameProc( Window* pInst, SalFrame* pFrame, USHORT nEvent, const void* pEvent );

// -----------
// - HitTest -
// -----------

#define WINDOW_HITTEST_INSIDE           ((USHORT)0x0001)
#define WINDOW_HITTEST_TRANSPARENT      ((USHORT)0x0002)

#endif  // _SV_WINDATA_HXX
