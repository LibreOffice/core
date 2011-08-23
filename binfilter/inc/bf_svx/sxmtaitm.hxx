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
#ifndef _SXMTAITM_HXX
#define _SXMTAITM_HXX

#ifndef _SVDDEF_HXX //autogen
#include <bf_svx/svddef.hxx>
#endif

#ifndef _SDYNITM_HXX
#include <bf_svx/sdynitm.hxx>
#endif

#ifndef _SDANGITM_HXX
#include <bf_svx/sdangitm.hxx>
#endif
namespace binfilter {

// Den Text automatisch zurechtdrehen (Automatisches UpsideDown).
// TextUpsideDown bleibt trotzdem weiterhin wirksam und dreht
// den Text bei TRUE nochmal.
class SdrMeasureTextAutoAngleItem: public SdrYesNoItem {
public:
    SdrMeasureTextAutoAngleItem(FASTBOOL bOn=TRUE): SdrYesNoItem(SDRATTR_MEASURETEXTAUTOANGLE,bOn) {}
    SdrMeasureTextAutoAngleItem(SvStream& rIn): SdrYesNoItem(SDRATTR_MEASURETEXTAUTOANGLE,rIn) {}
};

// Der bevorzugte Blickwinkel zum lesen des Textes. Wird nur ausgewertet, wenn
// TextAutoAngle=TRUE. Winkel in 1/100deg aus der Zeichnung zum Betrachter.
class SdrMeasureTextAutoAngleViewItem: public SdrAngleItem {
public:
    SdrMeasureTextAutoAngleViewItem(long nVal=31500): SdrAngleItem(SDRATTR_MEASURETEXTAUTOANGLEVIEW,nVal)  {}
    SdrMeasureTextAutoAngleViewItem(SvStream& rIn): SdrAngleItem(SDRATTR_MEASURETEXTAUTOANGLEVIEW,rIn) {}
};

}//end of namespace binfilter
#endif
