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
#ifndef _SXMLHITM_HXX
#define _SXMLHITM_HXX

#include <svx/svddef.hxx>
#include <svx/sdmetitm.hxx>

/*************************************************************************/
/* Measure                                                               */
/*************************************************************************/
//                             ___
//    |        100,00km       | 2mm = SdrMeasureHelplineOverhangItem
//    |<--------------------->|---
//    |                       | 8mm = SdrMeasureLineDistItem
//    |                       |
//    #=============#         |---
//    #             #         | SdrMeasureHelpline1/2LenItem
//    #             #=========#---
//    # Zu bemassendes Objekt #
//    #=======================#

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

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
