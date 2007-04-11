/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: salatype.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 18:05:01 $
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

#ifndef _SV_SALATYPE_HXX
#define _SV_SALATYPE_HXX

#ifndef _SV_SV_H
#include <vcl/sv.h>
#endif

// ---------------------
// - Application-Types -
// ---------------------

// Derzeit doppelt und in apptypes.hxx auch vorhanden

#define INPUT_MOUSE                 0x0001
#define INPUT_KEYBOARD              0x0002
#define INPUT_PAINT                 0x0004
#define INPUT_TIMER                 0x0008
#define INPUT_OTHER                 0x0010
#define INPUT_MOUSEANDKEYBOARD      (INPUT_MOUSE | INPUT_KEYBOARD)
#define INPUT_ANY                   (INPUT_MOUSEANDKEYBOARD | INPUT_PAINT | INPUT_TIMER | INPUT_OTHER)

#endif // _SV_SALATYPE_HXX
