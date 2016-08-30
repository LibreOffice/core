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

// Type definition for CursorShell.
// Direction-parameter for MovePage (initialized in SwContentFrame).
typedef SwLayoutFrame * (*SwWhichPage)( const SwLayoutFrame * );
SwLayoutFrame *GetPrevFrame( const SwLayoutFrame *pFrame );
SwLayoutFrame *GetThisFrame( const SwLayoutFrame *pFrame );
SwLayoutFrame *GetNextFrame( const SwLayoutFrame *pFrame );
typedef SwContentFrame  * (*SwPosPage)( const SwLayoutFrame * );
SwContentFrame *GetFirstSub( const SwLayoutFrame *pLayout );
SwContentFrame *GetLastSub( const SwLayoutFrame *pLayout );

// Direction-parameter for MovePara (initialized in SwContentFrame).
typedef bool (*SwWhichPara)( SwPaM&, SwMoveFnCollection const & );
extern SwWhichPara fnParaPrev, fnParaCurr, fnParaNext;
extern SwMoveFnCollection const & fnParaStart;
extern SwMoveFnCollection const & fnParaEnd;

// Direction-parameter for MoveSection.
typedef bool (*SwWhichSection)( SwPaM&, SwMoveFnCollection const & );
extern SwMoveFnCollection const & fnSectionStart;
extern SwMoveFnCollection const & fnSectionEnd;

bool GoCurrSection( SwPaM&, SwMoveFnCollection const &);

// Direction-parameter for MoveTable
typedef bool (*SwWhichTable)( SwPaM&, SwMoveFnCollection const &, bool bInReadOnly );
extern SwWhichTable fnTablePrev, fnTableCurr, fnTableNext;
extern SwMoveFnCollection const & fnTableStart;
extern SwMoveFnCollection const & fnTableEnd;

// Direction-parameter for MoveColumn
typedef SwLayoutFrame * (*SwWhichColumn)( const SwLayoutFrame * );
typedef SwContentFrame  * (*SwPosColumn)( const SwLayoutFrame * );
extern SwWhichColumn fnColumnPrev, fnColumnCurr, fnColumnNext;
extern SwPosColumn fnColumnStart, fnColumnEnd;

// Direction-parameter for MoveRegion (ranges!)
typedef bool (*SwWhichRegion)( SwPaM&, SwMoveFnCollection const &, bool bInReadOnly );
extern SwWhichRegion fnRegionPrev, fnRegionCurr, fnRegionNext, fnRegionCurrAndSkip;
extern SwMoveFnCollection const & fnRegionStart;
extern SwMoveFnCollection const & fnRegionEnd;

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

enum class SwDocPositions
{
    Start,
    Curr,
    End,
    OtherStart,
    OtherEnd
};

SW_DLLPUBLIC SwWhichPara GetfnParaCurr();
SW_DLLPUBLIC SwMoveFnCollection const & GetfnParaStart();
SW_DLLPUBLIC SwMoveFnCollection const & GetfnParaEnd();

SW_DLLPUBLIC SwWhichTable GetfnTablePrev();
SW_DLLPUBLIC SwWhichTable GetfnTableCurr();
SW_DLLPUBLIC SwMoveFnCollection const & GetfnTableStart();
SW_DLLPUBLIC SwMoveFnCollection const & GetfnTableEnd();

#endif // INCLUDED_SW_INC_CSHTYP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
