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

#ifndef _SV_ACCEL_H
#define _SV_ACCEL_H

#include <vcl/sv.h>
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
    sal_uInt16          mnId;
    KeyCode         maKeyCode;
    Accelerator*    mpAccel;
    Accelerator*    mpAutoAccel;
    sal_Bool            mbEnabled;
};

// -----------------
// - Hilfemethoden -
// -----------------

// in KEYCOD.CXX
void ImplGetKeyCode( KeyFuncType eFunc, sal_uInt16& rCode1, sal_uInt16& rCode2, sal_uInt16& rCode3, sal_uInt16& rCode4 );

#endif  // _SV_ACCEL_H
