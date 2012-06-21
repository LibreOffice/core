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

#ifndef _SV_SYMBOL_HXX
#define _SV_SYMBOL_HXX

#include <tools/solar.h>

// ----------------
// - Symbol-Types -
// ----------------

// By changes you must also change: rsc/vclrsc.hxx
typedef sal_uInt16 SymbolType;
#define SYMBOL_DONTKNOW         ((SymbolType)0)
#define SYMBOL_IMAGE            ((SymbolType)1)
#define SYMBOL_ARROW_UP         ((SymbolType)2)
#define SYMBOL_ARROW_DOWN       ((SymbolType)3)
#define SYMBOL_ARROW_LEFT       ((SymbolType)4)
#define SYMBOL_ARROW_RIGHT      ((SymbolType)5)
#define SYMBOL_SPIN_UP          ((SymbolType)6)
#define SYMBOL_SPIN_DOWN        ((SymbolType)7)
#define SYMBOL_SPIN_LEFT        ((SymbolType)8)
#define SYMBOL_SPIN_RIGHT       ((SymbolType)9)
#define SYMBOL_FIRST            ((SymbolType)10)
#define SYMBOL_LAST             ((SymbolType)11)
#define SYMBOL_PREV             ((SymbolType)12)
#define SYMBOL_NEXT             ((SymbolType)13)
#define SYMBOL_PAGEUP           ((SymbolType)14)
#define SYMBOL_PAGEDOWN         ((SymbolType)15)
#define SYMBOL_PLAY             ((SymbolType)16)
#define SYMBOL_REVERSEPLAY      ((SymbolType)17)
#define SYMBOL_RECORD           ((SymbolType)18)
#define SYMBOL_STOP             ((SymbolType)19)
#define SYMBOL_PAUSE            ((SymbolType)20)
#define SYMBOL_WINDSTART        ((SymbolType)21)
#define SYMBOL_WINDEND          ((SymbolType)22)
#define SYMBOL_WINDBACKWARD     ((SymbolType)23)
#define SYMBOL_WINDFORWARD      ((SymbolType)24)
#define SYMBOL_CLOSE            ((SymbolType)25)
#define SYMBOL_ROLLUP           ((SymbolType)26)
#define SYMBOL_ROLLDOWN         ((SymbolType)27)
#define SYMBOL_CHECKMARK        ((SymbolType)28)
#define SYMBOL_RADIOCHECKMARK   ((SymbolType)29)
#define SYMBOL_SPIN_UPDOWN      ((SymbolType)30)
#define SYMBOL_FLOAT            ((SymbolType)31)
#define SYMBOL_DOCK             ((SymbolType)32)
#define SYMBOL_HIDE             ((SymbolType)33)
#define SYMBOL_HELP             ((SymbolType)34)
#define SYMBOL_PLUS             ((SymbolType)35)
#define SYMBOL_MENU             SYMBOL_SPIN_DOWN

#define SYMBOL_NOSYMBOL         (SYMBOL_DONTKNOW)

#endif // _SV_SYMBOL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
