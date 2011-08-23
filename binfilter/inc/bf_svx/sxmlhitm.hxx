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
#ifndef _SXMLHITM_HXX
#define _SXMLHITM_HXX

#include <bf_svx/svddef.hxx>

#include <bf_svx/sdmetitm.hxx>
namespace binfilter {

/*************************************************************************/
/* Measure                                                               */
/*************************************************************************/
//                             ___
//    ?       100,00km       ?2mm = SdrMeasureHelplineOverhangItem
//    ?--------------------->?--
//    ?                      ?8mm = SdrMeasureLineDistItem
//    ?                      ?
//    浜様様様様様様?        ?--
//    ?            ?        ?SdrMeasureHelpline1/2LenItem
//    ?            藩様様様様?--
//    ?Zu bemassendes Objekt ?
//    藩様様様様様様様様様様様?

// Abstand der Masslinie zur Bezugskante (Norm=8mm)
// laesst sich auch draggen an den beiden Handles
// die auf den Pfeilspitzen liegen
class SdrMeasureLineDistItem: public SdrMetricItem {
public:
    SdrMeasureLineDistItem(long nVal=0): SdrMetricItem(SDRATTR_MEASURELINEDIST,nVal)  {}
    SdrMeasureLineDistItem(SvStream& rIn): SdrMetricItem(SDRATTR_MEASURELINEDIST,rIn) {}
};

// Ueberhang der Masshilfslinien an der Masslinie (Norm=2mm)
class SdrMeasureHelplineOverhangItem: public SdrMetricItem {
public:
    SdrMeasureHelplineOverhangItem(long nVal=0): SdrMetricItem(SDRATTR_MEASUREHELPLINEOVERHANG,nVal)  {}
    SdrMeasureHelplineOverhangItem(SvStream& rIn): SdrMetricItem(SDRATTR_MEASUREHELPLINEOVERHANG,rIn) {}
};

// Abstand der der Masshilfslinienenden zur Bezugskante
// Damit die Masshilfslinien nicht das Objekt beruehren
class SdrMeasureHelplineDistItem: public SdrMetricItem {
public:
    SdrMeasureHelplineDistItem(long nVal=0): SdrMetricItem(SDRATTR_MEASUREHELPLINEDIST,nVal)  {}
    SdrMeasureHelplineDistItem(SvStream& rIn): SdrMetricItem(SDRATTR_MEASUREHELPLINEDIST,rIn) {}
};

// Ueberlaenge der Hilfslinien ueber die Bezugskante
// laesst sich auch draggen an den beiden runden Handles,
// die sich default unter den beiden Referenzpunkten befinden
class SdrMeasureHelpline1LenItem: public SdrMetricItem {
public:
    SdrMeasureHelpline1LenItem(long nVal=0): SdrMetricItem(SDRATTR_MEASUREHELPLINE1LEN,nVal)  {}
    SdrMeasureHelpline1LenItem(SvStream& rIn): SdrMetricItem(SDRATTR_MEASUREHELPLINE1LEN,rIn) {}
};
class SdrMeasureHelpline2LenItem: public SdrMetricItem {
public:
    SdrMeasureHelpline2LenItem(long nVal=0): SdrMetricItem(SDRATTR_MEASUREHELPLINE2LEN,nVal)  {}
    SdrMeasureHelpline2LenItem(SvStream& rIn): SdrMetricItem(SDRATTR_MEASUREHELPLINE2LEN,rIn) {}
};

}//end of namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
