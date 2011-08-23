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


#ifdef _MSC_VER
#pragma hdrstop
#endif

#ifndef _SVX_SVXIDS_HRC //autogen
#include <bf_svx/svxids.hrc>
#endif


#ifndef _HORIORNT_HXX
#include <horiornt.hxx>
#endif

#include "wrtsh.hxx"
#include "wrtsh.hrc"

namespace binfilter {

#define C2U(cChar) ::rtl::OUString::createFromAscii(cChar)

using namespace ::com::sun::star;
using namespace ::rtl;

extern sal_Bool bNoInterrupt;       // in mainwn.cxx



/*--------------------------------------------------------------------
    Beschreibung: FontWork-Slots invalidieren
 --------------------------------------------------------------------*/


/*N*/ void SwWrtShell::DrawSelChanged(SdrView* pView)
/*N*/ {
//STRIP001  static sal_uInt16 __READONLY_DATA aInval[] =
//STRIP001 /*N*/     {
//STRIP001 /*N*/         SID_ATTR_FILL_STYLE, SID_ATTR_FILL_COLOR, SID_ATTR_LINE_STYLE,
//STRIP001 /*N*/         SID_ATTR_LINE_WIDTH, SID_ATTR_LINE_COLOR, 0
//STRIP001 /*N*/     };
//STRIP001 /*N*/ 
//STRIP001 /*N*/     GetView().GetViewFrame()->GetBindings().Invalidate(aInval);
//STRIP001 /*N*/ 
//STRIP001 /*N*/     sal_Bool bOldVal = bNoInterrupt;
//STRIP001 /*N*/ /*N*/     bNoInterrupt = sal_True;    // Trick, um AttrChangedNotify ueber Timer auszufuehren
//STRIP001 /*N*/ /*N*/     GetView().AttrChangedNotify(this);
//STRIP001 /*N*/ /*N*/     bNoInterrupt = bOldVal;
/*N*/ }

}
