/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cshtyp.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 07:58:02 $
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
#ifndef _CSHTYP_HXX
#define _CSHTYP_HXX

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

#ifndef INCLUDED_SWDLLAPI_H
#include "swdllapi.h"
#endif

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
typedef BOOL (*SwWhichPara)( SwPaM&, SwPosPara );
extern SwWhichPara fnParaPrev, fnParaCurr, fnParaNext;
extern SwPosPara fnParaStart, fnParaEnd;

// Richtungsparameter fuer MoveSection
typedef SwMoveFnCollection* SwPosSection;
typedef BOOL (*SwWhichSection)( SwPaM&, SwPosSection );
extern SwWhichSection fnSectionPrev, fnSectionCurr, fnSectionNext;
extern SwPosSection fnSectionStart, fnSectionEnd;

// Richtungsparameter fuer MoveTable
typedef SwMoveFnCollection* SwPosTable;
typedef BOOL (*SwWhichTable)( SwPaM&, SwPosTable, BOOL bInReadOnly );
extern SwWhichTable fnTablePrev, fnTableCurr, fnTableNext;
extern SwPosTable fnTableStart, fnTableEnd;

// Richtungsparameter fuer MoveColumn
typedef SwLayoutFrm * (*SwWhichColumn)( const SwLayoutFrm * );
typedef SwCntntFrm  * (*SwPosColumn)( const SwLayoutFrm * );
extern SwWhichColumn fnColumnPrev, fnColumnCurr, fnColumnNext;
extern SwPosColumn fnColumnStart, fnColumnEnd;

// Richtungsparameter fuer MoveRegion   (Bereiche!)
typedef SwMoveFnCollection* SwPosRegion;
typedef BOOL (*SwWhichRegion)( SwPaM&, SwPosRegion, BOOL bInReadOnly );
extern SwWhichRegion fnRegionPrev, fnRegionCurr, fnRegionNext, fnRegionCurrAndSkip;
extern SwPosRegion fnRegionStart, fnRegionEnd;



/*
 * folgende Kombinationen sind erlaubt:
 *  - suche einen im Body:                  -> FND_IN_BODY
 *  - suche alle im Body:                   -> FND_IN_BODYONLY | FND_IN_SELALL
 *  - suche in Selectionen: einen / alle    -> FND_IN_SEL  [ | FND_IN_SELALL ]
 *  - suche im nicht Body: einen / alle     -> FND_IN_OTHER [ | FND_IN_SELALL ]
 *  - suche ueberall alle:                  -> FND_IN_SELALL
 */
enum FindRanges
{
    FND_IN_BODY     = 0x00,     // suche "eins" mur im Body-Text
    FND_IN_OTHER    = 0x02,     // suche "alles" in Footer/Header/Fly...
    FND_IN_SEL      = 0x04,     // suche in Selectionen
    FND_IN_BODYONLY = 0x08,     // suche nur im Body - nur in Verbindung mit
                                // FND_IN_SELALL !!!
    FND_IN_SELALL   = 0x01      // - alle ( nur im nicht Body und Selectionen)
};


enum SwDocPositions
{
    DOCPOS_START,
    DOCPOS_CURR,
    DOCPOS_END,
    DOCPOS_OTHERSTART,
    DOCPOS_OTHEREND
};

SW_DLLPUBLIC SwWhichPara GetfnParaCurr();
SW_DLLPUBLIC SwPosPara GetfnParaStart();
SW_DLLPUBLIC SwPosPara GetfnParaEnd();

SW_DLLPUBLIC SwWhichTable GetfnTablePrev();
SW_DLLPUBLIC SwWhichTable GetfnTableCurr();
SW_DLLPUBLIC SwPosTable GetfnTableStart();
SW_DLLPUBLIC SwPosTable GetfnTableEnd();

#endif  // _CSHTYP_HXX
