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

#ifndef _VCL_APPTYPES_HXX
#define _VCL_APPTYPES_HXX

#include <vcl/sv.h>
#include <vcl/dllapi.h>
#include <tools/rtti.hxx>

// ---------------------
// - Application-Types -
// ---------------------

#define EXC_RSCNOTLOADED            ((sal_uInt16)0x0100)
#define EXC_SYSOBJNOTCREATED        ((sal_uInt16)0x0200)
#define EXC_SYSTEM                  ((sal_uInt16)0x0300)
#define EXC_DISPLAY                 ((sal_uInt16)0x0400)
#define EXC_REMOTE                  ((sal_uInt16)0x0500)
#define EXC_USER                    ((sal_uInt16)0x1000)
#define EXC_MAJORTYPE               ((sal_uInt16)0xFF00)
#define EXC_MINORTYPE               ((sal_uInt16)0x00FF)

#define UNIQUEID_SV_BEGIN           64000

class VCL_DLLPUBLIC ApplicationProperty
{
public:

    TYPEINFO();
};

#define INPUT_MOUSE                 0x0001
#define INPUT_KEYBOARD              0x0002
#define INPUT_PAINT                 0x0004
#define INPUT_TIMER                 0x0008
#define INPUT_OTHER                 0x0010
#define INPUT_APPEVENT              0x0020
#define INPUT_MOUSEANDKEYBOARD      (INPUT_MOUSE | INPUT_KEYBOARD)
#define INPUT_ANY                   (INPUT_MOUSEANDKEYBOARD | INPUT_PAINT | INPUT_TIMER | INPUT_OTHER | INPUT_APPEVENT)

#define DISPATCH_OPEN               0x0001
#define DISPATCH_PRINT              0x0002
#define DISPATCH_SERVER             0x0004

// --------------
// - UserActive -
// --------------

#define USERACTIVE_MOUSEDRAG            ((sal_uInt16)0x0001)
#define USERACTIVE_INPUT                ((sal_uInt16)0x0002)
#define USERACTIVE_MODALDIALOG          ((sal_uInt16)0x0004)
#define USERACTIVE_ALL                  ((sal_uInt16)0xFFFF)

#endif // _VCL_APPTYPES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
