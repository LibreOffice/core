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

#ifndef _VCL_CORETEXT_COMMON_H
#define _VCL_CORETEXT_COMMON_H

#include <iostream>

#include <premac.h>
#ifdef MACOSX
#include <ApplicationServices/ApplicationServices.h>
#else
#include <CoreGraphics/CoreGraphics.h>
#include <CoreText/CoreText.h>
#endif
#include <postmac.h>

#include <sal/types.h>

#include <tools/debug.hxx>

// CoreFoundation designers, in their wisdom, decided that CFRelease of NULL
// cause a Crash, yet few API can return NULL when asking for the creation
// of an object, which force us to peper the code with egly if construct everywhere
// and open the door to very nasty crash on rare occasion
// this macro hide the mess
#define SafeCFRelease(a) do { if(a) { CFRelease(a); (a)=NULL; } } while(false)

#define round_to_long(a) ((a) >= 0 ? ((long)((a) + 0.5)) : ((long)((a) - 0.5)))

#include "vcl/salgtype.hxx"

std::ostream &operator <<(std::ostream& s, CTFontRef pFont);

#endif /* _VCL_CORETEXT_COMMON_H */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
