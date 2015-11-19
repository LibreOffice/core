/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_SW_INC_CSHTYP_HXX
#define INCLUDED_SW_INC_CSHTYP_HXX

#include <tools/solar.h>
#include "swdllapi.h"

class SwPaM;
class SwContentFrame;
class SwLayoutFrame;

// Structure for SwPaM. Contains the method-pointers for cursor movement.
struct SwMoveFnCollection;
typedef SwMoveFnCollection* SwMoveFn;

// Type definition for CursorShell.
// Direction-parameter for MovePage (initialized in SwContentFrame).
typedef SwLayoutFrame * (*SwWhichPage)( const SwLayoutFrame * );
typedef SwContentFrame  * (*SwPosPage)( const SwLayoutFrame * );
extern SwWhichPage fnPagePrev, fnPageCurr, fnPageNext;
extern SwPosPage fnPageStart, fnPageEnd;

// Direction-parameter for MovePara (initialized in SwContentFrame).
typedef SwMoveFnCollection* SwPosPara;
typedef bool (*SwWhichPara)( SwPaM&, SwPosPara );
extern SwWhichPara fnParaPrev, fnParaCurr, fnParaNext;
extern SwPosPara fnParaStart, fnParaEnd;

// Direction-parameter for MoveSection.
typedef SwMoveFnCollection* SwPosSection;
typedef bool (*SwWhichSection)( SwPaM&, SwPosSection );
extern SwWhichSection fnSectionPrev, fnSectionCurr, fnSectionNext;
extern SwPosSection fnSectionStart, fnSectionEnd;

// Direction-parameter for MoveTable
typedef SwMoveFnCollection* SwPosTable;
typedef bool (*SwWhichTable)( SwPaM&, SwPosTable, bool bInReadOnly );
extern SwWhichTable fnTablePrev, fnTableCurr, fnTableNext;
extern SwPosTable fnTableStart, fnTableEnd;

// Direction-parameter for MoveColumn
typedef SwLayoutFrame * (*SwWhichColumn)( const SwLayoutFrame * );
typedef SwContentFrame  * (*SwPosColumn)( const SwLayoutFrame * );
extern SwWhichColumn fnColumnPrev, fnColumnCurr, fnColumnNext;
extern SwPosColumn fnColumnStart, fnColumnEnd;

// Direction-parameter for MoveRegion (ranges!)
typedef SwMoveFnCollection* SwPosRegion;
typedef bool (*SwWhichRegion)( SwPaM&, SwPosRegion, bool bInReadOnly );
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
    FND_IN_BODY     = 0x00,     ///< Find "one" only in body text.
    FND_IN_OTHER    = 0x02,     ///< Find "all" in Footer/Header/Fly...
    FND_IN_SEL      = 0x04,     ///< Find in selections.
    FND_IN_BODYONLY = 0x08,     ///< Find only in body - only in combination with FND_IN_SELALL !!!
    FND_IN_SELALL   = 0x01      ///< All (only in non-body and selections).
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

#endif // INCLUDED_SW_INC_CSHTYP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
