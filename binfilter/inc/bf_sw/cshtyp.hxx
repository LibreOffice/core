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
#ifndef _CSHTYP_HXX
#define _CSHTYP_HXX

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif
class UniString; 
namespace binfilter {


class SwPaM;
class SwCntntFrm;
class SwLayoutFrm;

// eine Struktur fuer den SwPaM. In dieser stehen die Methoden-Pointer
// fuer das richtungsabhaengige Bewegen des Cursors.
struct SwMoveFnCollection;
typedef SwMoveFnCollection* SwMoveFn;


// Type-Definition fuer die CrsrShell
// Richtungsparameter fuer MovePage ( wird in SwCntntFrm initialisiert )
typedef SwLayoutFrm * (*SwWhichPage)( const SwLayoutFrm * );
typedef SwCntntFrm  * (*SwPosPage)( const SwLayoutFrm * );
extern SwWhichPage fnPagePrev, fnPageCurr, fnPageNext;
extern SwPosPage fnPageStart, fnPageEnd;

// Richtungsparameter fuer MovePara ( wird in SwPaM initialisiert )
typedef SwMoveFnCollection* SwPosPara;
typedef FASTBOOL (*SwWhichPara)( SwPaM&, SwPosPara );
extern SwWhichPara fnParaPrev, fnParaCurr, fnParaNext;
extern SwPosPara fnParaStart, fnParaEnd;

// Richtungsparameter fuer MoveSection
typedef SwMoveFnCollection* SwPosSection;
typedef FASTBOOL (*SwWhichSection)( SwPaM&, SwPosSection );
extern SwWhichSection fnSectionPrev, fnSectionCurr, fnSectionNext;
extern SwPosSection fnSectionStart, fnSectionEnd;

// Richtungsparameter fuer MoveTable
typedef SwMoveFnCollection* SwPosTable;
typedef FASTBOOL (*SwWhichTable)( SwPaM&, SwPosTable, FASTBOOL bInReadOnly );
extern SwWhichTable fnTablePrev, fnTableCurr, fnTableNext;
extern SwPosTable fnTableStart, fnTableEnd;

// Richtungsparameter fuer MoveColumn
typedef SwLayoutFrm * (*SwWhichColumn)( const SwLayoutFrm * );
typedef SwCntntFrm  * (*SwPosColumn)( const SwLayoutFrm * );
extern SwWhichColumn fnColumnPrev, fnColumnCurr, fnColumnNext;
extern SwPosColumn fnColumnStart, fnColumnEnd;

// Richtungsparameter fuer MoveRegion	(Bereiche!)
typedef SwMoveFnCollection* SwPosRegion;
typedef FASTBOOL (*SwWhichRegion)( SwPaM&, SwPosRegion, FASTBOOL bInReadOnly );
extern SwWhichRegion fnRegionPrev, fnRegionCurr, fnRegionNext, fnRegionCurrAndSkip;
extern SwPosRegion fnRegionStart, fnRegionEnd;



/*
 * folgende Kombinationen sind erlaubt:
 *  - suche einen im Body:					-> FND_IN_BODY
 *  - suche alle im Body: 					-> FND_IN_BODYONLY | FND_IN_SELALL
 *  - suche in Selectionen: einen / alle    -> FND_IN_SEL  [ | FND_IN_SELALL ]
 *  - suche im nicht Body: einen / alle     -> FND_IN_OTHER [ | FND_IN_SELALL ]
 *  - suche ueberall alle: 					-> FND_IN_SELALL
 */
enum FindRanges
{
    FND_IN_BODY     = 0x00,     // suche "eins" mur im Body-Text
    FND_IN_OTHER    = 0x02,     // suche "alles" in Footer/Header/Fly...
    FND_IN_SEL      = 0x04,     // suche in Selectionen
    FND_IN_BODYONLY	= 0x08,		// suche nur im Body - nur in Verbindung mit
                                // FND_IN_SELALL !!!
    FND_IN_SELALL   = 0x01		// - alle ( nur im nicht Body und Selectionen)
};


enum SwDocPositions
{
    DOCPOS_START,
    DOCPOS_CURR,
    DOCPOS_END,
    DOCPOS_OTHERSTART,
    DOCPOS_OTHEREND
};

} //namespace binfilter
#endif	// _CSHTYP_HXX
