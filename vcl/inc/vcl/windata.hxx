/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: windata.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2007-10-09 15:18:59 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SV_WINDATA_HXX
#define _SV_WINDATA_HXX

#ifndef _SV_SV_H
#include <vcl/sv.h>
#endif

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
