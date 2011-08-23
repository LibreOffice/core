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

#ifndef _ORNTENUM_HXX
#define _ORNTENUM_HXX
namespace binfilter {

//SwFmtVertOrient, wie und woran orientiert --
//	sich der FlyFrm in der Vertikalen -----------

enum SwRelationOrient
{
    FRAME,			// Absatz inkl. Raender
    PRTAREA,		// Absatz ohne Raender
    REL_CHAR,		// an einem Zeichen
    REL_PG_LEFT,	// im linken Seitenrand
    REL_PG_RIGHT,   // im rechten Seitenrand
    REL_FRM_LEFT,   // im linken Absatzrand
    REL_FRM_RIGHT,  // im rechten Absatzrand
    REL_PG_FRAME,	// Seite inkl. Raender, bei seitengeb. identisch mit FRAME
    REL_PG_PRTAREA,	// Seite ohne Raender, bei seitengeb. identisch mit PRTAREA
    // Wenn neue enums benoetigt werden, muessen die Ausweich-Arrays mitgepflegt
    // werden, LAST_ENUM_DUMMY muss immer das letzte enum bleiben
    LAST_ENUM_DUMMY
};

enum SwVertOrient
{
    VERT_NONE,	//Der Wert in nYPos gibt die RelPos direkt an.
    VERT_TOP,	//Der Rest ist fuer automatische Ausrichtung.
    VERT_CENTER,
    VERT_BOTTOM,
    VERT_CHAR_TOP,		//Ausrichtung _nur_ fuer Zeichengebundene Rahmen
    VERT_CHAR_CENTER,	//wie der Name jew. sagt wird der RefPoint des Rahmens
    VERT_CHAR_BOTTOM,	//entsprechend auf die Oberkante, Mitte oder Unterkante
    VERT_LINE_TOP,		//der Zeile gesetzt. Der Rahmen richtet sich  dann
    VERT_LINE_CENTER,	//entsprechend aus.
    VERT_LINE_BOTTOM
};

} //namespace binfilter
#endif
