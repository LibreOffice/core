/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include <tools/solar.h>
#include "swdllapi.h"

class SwPaM;
class SwCntntFrm;
class SwLayoutFrm;

// Structure for SwPaM. Contains the method-pointers for cursor movement.
struct SwMoveFnCollection;
typedef SwMoveFnCollection* SwMoveFn;


// Type definition for CrsrShell.
// Direction-parameter for MovePage (initialized in SwCntntFrm).
typedef SwLayoutFrm * (*SwWhichPage)( const SwLayoutFrm * );
typedef SwCntntFrm  * (*SwPosPage)( const SwLayoutFrm * );
extern SwWhichPage fnPagePrev, fnPageCurr, fnPageNext;
extern SwPosPage fnPageStart, fnPageEnd;

// Direction-parameter for MovePara (initialized in SwCntntFrm).
typedef SwMoveFnCollection* SwPosPara;
typedef sal_Bool (*SwWhichPara)( SwPaM&, SwPosPara );
extern SwWhichPara fnParaPrev, fnParaCurr, fnParaNext;
extern SwPosPara fnParaStart, fnParaEnd;

// Direction-parameter for MoveSection.
typedef SwMoveFnCollection* SwPosSection;
typedef sal_Bool (*SwWhichSection)( SwPaM&, SwPosSection );
extern SwWhichSection fnSectionPrev, fnSectionCurr, fnSectionNext;
extern SwPosSection fnSectionStart, fnSectionEnd;

// Direction-parameter for MoveTable
typedef SwMoveFnCollection* SwPosTable;
typedef sal_Bool (*SwWhichTable)( SwPaM&, SwPosTable, sal_Bool bInReadOnly );
extern SwWhichTable fnTablePrev, fnTableCurr, fnTableNext;
extern SwPosTable fnTableStart, fnTableEnd;

// Direction-parameter for MoveColumn
typedef SwLayoutFrm * (*SwWhichColumn)( const SwLayoutFrm * );
typedef SwCntntFrm  * (*SwPosColumn)( const SwLayoutFrm * );
extern SwWhichColumn fnColumnPrev, fnColumnCurr, fnColumnNext;
extern SwPosColumn fnColumnStart, fnColumnEnd;

// Direction-parameter for MoveRegion (ranges!)
typedef SwMoveFnCollection* SwPosRegion;
typedef sal_Bool (*SwWhichRegion)( SwPaM&, SwPosRegion, sal_Bool bInReadOnly );
extern SwWhichRegion fnRegionPrev, fnRegionCurr, fnRegionNext, fnRegionCurrAndSkip;
extern SwPosRegion fnRegionStart, fnRegionEnd;


/*
 * The following combinations are allowed:
 *  - find one in body                      -> FND_IN_BODY
 *  - find all in body:                     -> FND_IN_BODYONLY | FND_IN_SELALL
 *  - find in selections: one/all           -> FND_IN_SEL  [ | FND_IN_SELALL ]
 *  - find not in body: one/all             -> FND_IN_OTHER [ | FND_IN_SELALL ]
 *  - find all everywhere                   -> FND_IN_SELALL
 */
enum FindRanges
{
    FND_IN_BODY     = 0x00,     // Find "one" only in body text.
    FND_IN_OTHER    = 0x02,     // Find "all" in Footer/Header/Fly...
    FND_IN_SEL      = 0x04,     // Find in selections.
    FND_IN_BODYONLY = 0x08,     // Find only in body - only in combination with FND_IN_SELALL !!!
    FND_IN_SELALL   = 0x01      // All (only in non-body and selections).
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
