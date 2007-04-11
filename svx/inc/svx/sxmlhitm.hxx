/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sxmlhitm.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 16:35:41 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _SXMLHITM_HXX
#define _SXMLHITM_HXX

#ifndef _SVDDEF_HXX //autogen
#include <svx/svddef.hxx>
#endif

#ifndef _SDMETITM_HXX
#include <svx/sdmetitm.hxx>
#endif

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
