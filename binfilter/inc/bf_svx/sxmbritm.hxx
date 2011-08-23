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
#ifndef _SXMBRITM_HXX
#define _SXMBRITM_HXX

#ifndef _SVDDEF_HXX //autogen
#include <bf_svx/svddef.hxx>
#endif

#ifndef SXMBRITM_HXX
#include <bf_svx/sdynitm.hxx>
#endif
namespace binfilter {

// Die Masslinie unterhalb der Bezugskante
// Zur Bemassung einer Objektunterkante
// (Redundant zum drehen der Bezugskante um 180deg +
// TextUpsideDown, jedoch besser bedienbar)
// laesst sich auch draggen, indem man den Masslinienabstand
// (SdrMeasureLineDistItem) ueber die Bezugskante hinwegdraggt
class SdrMeasureBelowRefEdgeItem: public SdrYesNoItem {
public:
    SdrMeasureBelowRefEdgeItem(FASTBOOL bOn=FALSE): SdrYesNoItem(SDRATTR_MEASUREBELOWREFEDGE,bOn) {}
    SdrMeasureBelowRefEdgeItem(SvStream& rIn): SdrYesNoItem(SDRATTR_MEASUREBELOWREFEDGE,rIn) {}
};

}//end of namespace binfilter
#endif
