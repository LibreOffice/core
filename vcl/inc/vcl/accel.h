/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: accel.h,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-24 09:57:31 $
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

#ifndef _SV_ACCEL_H
#define _SV_ACCEL_H

#ifndef _SV_SV_H
#include <vcl/sv.h>
#endif
#ifndef _SV_KEYCOD_HXX
#include <vcl/keycod.hxx>
#endif

class Accelerator;

// ------------------
// - ImplAccelEntry -
// ------------------

class ImplAccelEntry
{
public:
    USHORT          mnId;
    KeyCode         maKeyCode;
    Accelerator*    mpAccel;
    Accelerator*    mpAutoAccel;
    BOOL            mbEnabled;
};

// -----------------
// - Hilfemethoden -
// -----------------

// in KEYCOD.CXX
void ImplGetKeyCode( KeyFuncType eFunc, USHORT& rCode1, USHORT& rCode2, USHORT& rCode3 );

#endif  // _SV_ACCEL_H
