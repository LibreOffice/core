/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: salatype.hxx,v $
 * $Revision: 1.3 $
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

#ifndef _SV_SALATYPE_HXX
#define _SV_SALATYPE_HXX

#include <vcl/sv.h>

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
