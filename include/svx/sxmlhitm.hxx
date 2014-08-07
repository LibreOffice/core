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
#ifndef INCLUDED_SVX_SXMLHITM_HXX
#define INCLUDED_SVX_SXMLHITM_HXX

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
inline SdrMetricItem makeSdrMeasureLineDistItem(long nVal) {
    return SdrMetricItem(SDRATTR_MEASURELINEDIST, nVal);
}

// Ueberhang der Masshilfslinien an der Masslinie (Norm=2mm)
inline SdrMetricItem makeSdrMeasureHelplineOverhangItem(long nVal) {
    return SdrMetricItem(SDRATTR_MEASUREHELPLINEOVERHANG, nVal);
}

// Abstand der der Masshilfslinienenden zur Bezugskante
// Damit die Masshilfslinien nicht das Objekt beruehren
inline SdrMetricItem makeSdrMeasureHelplineDistItem(long nVal) {
    return SdrMetricItem(SDRATTR_MEASUREHELPLINEDIST, nVal);
}

// Ueberlaenge der Hilfslinien ueber die Bezugskante
// laesst sich auch draggen an den beiden runden Handles,
// die sich default unter den beiden Referenzpunkten befinden
inline SdrMetricItem makeSdrMeasureHelpline1LenItem(long nVal) {
    return SdrMetricItem(SDRATTR_MEASUREHELPLINE1LEN, nVal);
}
inline SdrMetricItem makeSdrMeasureHelpline2LenItem(long nVal) {
    return SdrMetricItem(SDRATTR_MEASUREHELPLINE2LEN, nVal);
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
