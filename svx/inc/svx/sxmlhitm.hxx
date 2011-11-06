/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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
