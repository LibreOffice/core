/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sxmtaitm.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 16:37:17 $
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
#ifndef _SXMTAITM_HXX
#define _SXMTAITM_HXX

#ifndef _SVDDEF_HXX //autogen
#include <svx/svddef.hxx>
#endif

#ifndef _SDYNITM_HXX
#include <svx/sdynitm.hxx>
#endif

#ifndef _SDANGITM_HXX
#include <svx/sdangitm.hxx>
#endif

// Den Text automatisch zurechtdrehen (Automatisches UpsideDown).
// TextUpsideDown bleibt trotzdem weiterhin wirksam und dreht
// den Text bei TRUE nochmal.
class SdrMeasureTextAutoAngleItem: public SdrYesNoItem {
public:
    SdrMeasureTextAutoAngleItem(bool bOn=true): SdrYesNoItem(SDRATTR_MEASURETEXTAUTOANGLE,bOn) {}
    SdrMeasureTextAutoAngleItem(SvStream& rIn): SdrYesNoItem(SDRATTR_MEASURETEXTAUTOANGLE,rIn) {}
};

// Der bevorzugte Blickwinkel zum lesen des Textes. Wird nur ausgewertet, wenn
// TextAutoAngle=TRUE. Winkel in 1/100deg aus der Zeichnung zum Betrachter.
class SdrMeasureTextAutoAngleViewItem: public SdrAngleItem {
public:
    SdrMeasureTextAutoAngleViewItem(long nVal=31500): SdrAngleItem(SDRATTR_MEASURETEXTAUTOANGLEVIEW,nVal)  {}
    SdrMeasureTextAutoAngleViewItem(SvStream& rIn): SdrAngleItem(SDRATTR_MEASURETEXTAUTOANGLEVIEW,rIn) {}
};

#endif
