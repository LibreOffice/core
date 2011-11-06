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


#ifndef _CSHTYP_HXX
#define _CSHTYP_HXX

#include <tools/solar.h>
#include "swdllapi.h"

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
typedef sal_Bool (*SwWhichPara)( SwPaM&, SwPosPara );
extern SwWhichPara fnParaPrev, fnParaCurr, fnParaNext;
extern SwPosPara fnParaStart, fnParaEnd;

// Richtungsparameter fuer MoveSection
typedef SwMoveFnCollection* SwPosSection;
typedef sal_Bool (*SwWhichSection)( SwPaM&, SwPosSection );
extern SwWhichSection fnSectionPrev, fnSectionCurr, fnSectionNext;
extern SwPosSection fnSectionStart, fnSectionEnd;

// Richtungsparameter fuer MoveTable
typedef SwMoveFnCollection* SwPosTable;
typedef sal_Bool (*SwWhichTable)( SwPaM&, SwPosTable, sal_Bool bInReadOnly );
extern SwWhichTable fnTablePrev, fnTableCurr, fnTableNext;
extern SwPosTable fnTableStart, fnTableEnd;

// Richtungsparameter fuer MoveColumn
typedef SwLayoutFrm * (*SwWhichColumn)( const SwLayoutFrm * );
typedef SwCntntFrm  * (*SwPosColumn)( const SwLayoutFrm * );
extern SwWhichColumn fnColumnPrev, fnColumnCurr, fnColumnNext;
extern SwPosColumn fnColumnStart, fnColumnEnd;

// Richtungsparameter fuer MoveRegion   (Bereiche!)
typedef SwMoveFnCollection* SwPosRegion;
typedef sal_Bool (*SwWhichRegion)( SwPaM&, SwPosRegion, sal_Bool bInReadOnly );
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
