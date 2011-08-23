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
#ifndef _SXMTFITM_HXX
#define _SXMTFITM_HXX

#include <bf_svx/svddef.hxx>

#include <bf_svx/sdynitm.hxx>

#include <bf_svx/sdangitm.hxx>
namespace binfilter {

// die 2 folgenden sind noch nicht implementiert!
// Text auf einen wirklich fixen Winkel festnageln.
// Der Textwinkel ist dann auch unabhaengig vom Winkel der Masslinie.
// Setzt TextUpsideDown, TextRota90 sowie TextAutoAngle ausser Kraft. (n.i.)
class SdrMeasureTextIsFixedAngleItem: public SdrYesNoItem {
public:
    SdrMeasureTextIsFixedAngleItem(FASTBOOL bOn=FALSE): SdrYesNoItem(SDRATTR_MEASURETEXTISFIXEDANGLE,bOn) {}
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
    SdrMeasureDecimalPlacesItem(INT16 nVal=2): SfxInt16Item(SDRATTR_MEASUREDECIMALPLACES,nVal)  {}
    SdrMeasureDecimalPlacesItem(SvStream& rIn): SfxInt16Item(SDRATTR_MEASUREDECIMALPLACES,rIn) {}
};

}//end of namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
