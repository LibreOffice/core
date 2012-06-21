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
#ifndef _SXMTFITM_HXX
#define _SXMTFITM_HXX

#include <svx/svddef.hxx>
#include <svx/sdynitm.hxx>
#include <svx/sdangitm.hxx>

// die 2 folgenden sind noch nicht implementiert!
// Text auf einen wirklich fixen Winkel festnageln.
// Der Textwinkel ist dann auch unabhaengig vom Winkel der Masslinie.
// Setzt TextUpsideDown, TextRota90 sowie TextAutoAngle ausser Kraft. (n.i.)
class SdrMeasureTextIsFixedAngleItem: public SdrYesNoItem {
public:
    SdrMeasureTextIsFixedAngleItem(bool bOn=false): SdrYesNoItem(SDRATTR_MEASURETEXTISFIXEDANGLE,bOn) {}
    SdrMeasureTextIsFixedAngleItem(SvStream& rIn): SdrYesNoItem(SDRATTR_MEASURETEXTISFIXEDANGLE,rIn) {}
};

// Der Winkel des Textes in 1/100deg. 0=Horizontal von links nach rechts zu lesen. (n.i.)
class SdrMeasureTextFixedAngleItem: public SdrAngleItem {
public:
    SdrMeasureTextFixedAngleItem(long nVal=0): SdrAngleItem(SDRATTR_MEASURETEXTFIXEDANGLE,nVal)  {}
    SdrMeasureTextFixedAngleItem(SvStream& rIn): SdrAngleItem(SDRATTR_MEASURETEXTFIXEDANGLE,rIn) {}
};

// The decimal places used for the measure value
class SdrMeasureDecimalPlacesItem: public SfxInt16Item {
public:
    SdrMeasureDecimalPlacesItem(sal_Int16 nVal=2): SfxInt16Item(SDRATTR_MEASUREDECIMALPLACES,nVal)  {}
    SdrMeasureDecimalPlacesItem(SvStream& rIn): SfxInt16Item(SDRATTR_MEASUREDECIMALPLACES,rIn) {}
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
