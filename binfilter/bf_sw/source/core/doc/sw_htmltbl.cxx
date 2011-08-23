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


//#define TEST_DELAYED_RESIZE

#ifdef TEST_DELAYED_RESIZE
#endif

#ifndef _HORIORNT_HXX
#include <horiornt.hxx>
#endif
#ifndef _ERRHDL_HXX
#include <errhdl.hxx>
#endif


#include "htmltbl.hxx"
namespace binfilter {

#define COLFUZZY 20
#define MAX_TABWIDTH (USHRT_MAX - 2001)

/*N*/ SwHTMLTableLayout::~SwHTMLTableLayout()
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ }

// Die Breiten der Umrandung werden zunaechst wie in Netscape berechnet:
// Aussere Umrandung: BORDER + CELLSPACING + CELLPADDING
// Innere Umrandung: CELLSPACING + CELLPADDING
// Allerdings wird die Breite der Umrandung im SW trotzdem beachtet, wenn
// bSwBorders gesetzt ist, damit nicht faellschlich umgebrochen wird.
// MIB 27.6.97: Dabei muss auch der Abstand zum Inhalt berueckichtigt werden,
// und zwar auch dann, wenn wenn nur die gegenueberliegende Seite
// eine Umrandung hat.













// nAbsAvail ist der verfuegbare Platz in TWIPS.
// nRelAvail ist der auf USHRT_MAX bezogene verfuegbare Platz oder 0
// nAbsSpace ist der Anteil von nAbsAvail, der durch der umgebende Zelle
//           fur die Umrandung und den Abstand zum Inhalt reserviert ist.











}
