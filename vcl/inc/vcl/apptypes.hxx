/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef _VCL_APPTYPES_HXX
#define _VCL_APPTYPES_HXX

#include <tools/solar.h>
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

#define VCL_INPUT_MOUSE                 0x0001
#define VCL_INPUT_KEYBOARD              0x0002
#define VCL_INPUT_PAINT                 0x0004
#define VCL_INPUT_TIMER                 0x0008
#define VCL_INPUT_OTHER                 0x0010
#define VCL_INPUT_APPEVENT              0x0020
#define VCL_INPUT_MOUSEANDKEYBOARD      (VCL_INPUT_MOUSE | VCL_INPUT_KEYBOARD)
#define VCL_INPUT_ANY                   (VCL_INPUT_MOUSEANDKEYBOARD | VCL_INPUT_PAINT | VCL_INPUT_TIMER | VCL_INPUT_OTHER | VCL_INPUT_APPEVENT)

#endif // _VCL_APPTYPES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
