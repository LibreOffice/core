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

#include "swdllapi.h"
#include <o3tl/typed_flags_set.hxx>

class SwPaM;
class SwContentFrame;
class SwLayoutFrame;

// Structure for SwPaM. Contains the method-pointers for cursor movement.
struct SwMoveFnCollection;

// Type definition for CursorShell.
// Direction-parameter for MovePage.
typedef SwLayoutFrame * (*SwWhichPage)( const SwLayoutFrame * );
SwLayoutFrame *GetPrevFrame( const SwLayoutFrame *pFrame );
SwLayoutFrame *GetThisFrame( const SwLayoutFrame *pFrame );
SwLayoutFrame *GetNextFrame( const SwLayoutFrame *pFrame );
typedef SwContentFrame  * (*SwPosPage)( const SwLayoutFrame * );
SwContentFrame *GetFirstSub( const SwLayoutFrame *pLayout );
SwContentFrame *GetLastSub( const SwLayoutFrame *pLayout );

// Direction-parameter for MovePara.
typedef bool (*SwWhichPara)( SwPaM&, SwMoveFnCollection const & );
bool GoPrevPara( SwPaM&, SwMoveFnCollection const &);
SW_DLLPUBLIC bool GoCurrPara( SwPaM&, SwMoveFnCollection const &);
bool GoNextPara( SwPaM&, SwMoveFnCollection const &);
extern SW_DLLPUBLIC SwMoveFnCollection const & fnParaStart;
extern SW_DLLPUBLIC SwMoveFnCollection const & fnParaEnd;

// Direction-parameter for MoveSection.
typedef bool (*SwWhichSection)( SwPaM&, SwMoveFnCollection const & );
extern SwMoveFnCollection const & fnSectionStart;
extern SwMoveFnCollection const & fnSectionEnd;

bool GoCurrSection( SwPaM&, SwMoveFnCollection const &);

// Direction-parameter for MoveTable
typedef bool (*SwWhichTable)( SwPaM&, SwMoveFnCollection const &, bool bInReadOnly );
SW_DLLPUBLIC bool GotoPrevTable( SwPaM&, SwMoveFnCollection const &, bool bInReadOnly );
SW_DLLPUBLIC bool GotoCurrTable( SwPaM&, SwMoveFnCollection const &, bool bInReadOnly );
bool GotoNextTable( SwPaM&, SwMoveFnCollection const &, bool bInReadOnly );
extern SW_DLLPUBLIC SwMoveFnCollection const & fnTableStart;
extern SW_DLLPUBLIC SwMoveFnCollection const & fnTableEnd;

// Direction-parameter for MoveColumn
typedef SwLayoutFrame * (*SwWhichColumn)( const SwLayoutFrame * );
typedef SwContentFrame  * (*SwPosColumn)( const SwLayoutFrame * );
SwLayoutFrame* GetPrevColumn( const SwLayoutFrame* pLayFrame );
SwLayoutFrame* GetCurrColumn( const SwLayoutFrame* pLayFrame );
SwLayoutFrame* GetNextColumn( const SwLayoutFrame* pLayFrame );
SwContentFrame* GetColumnStt( const SwLayoutFrame* pColFrame );
SwContentFrame* GetColumnEnd( const SwLayoutFrame* pColFrame );

// Direction-parameter for MoveRegion (ranges!)
typedef bool (*SwWhichRegion)( SwPaM&, SwMoveFnCollection const &, bool bInReadOnly );
bool GotoPrevRegion( SwPaM&, SwMoveFnCollection const &, bool bInReadOnly );
bool GotoNextRegion( SwPaM&, SwMoveFnCollection const &, bool bInReadOnly );
bool GotoCurrRegionAndSkip( SwPaM&, SwMoveFnCollection const &, bool bInReadOnly );
extern SwMoveFnCollection const & fnRegionStart;
extern SwMoveFnCollection const & fnRegionEnd;

/*
 * The following combinations are allowed:
 *  - find one in body                      -> FindRanges::InBody
 *  - find all in body:                     -> FindRanges::InBodyOnly | FindRanges::InSelAll
 *  - find in selections: one/all           -> FindRanges::InSel  [ | FindRanges::InSelAll ]
 *  - find not in body: one/all             -> FindRanges::InOther [ | FindRanges::InSelAll ]
 *  - find all everywhere                   -> FindRanges::InSelAll
 */
enum class FindRanges
{
    InBody     = 0x00,     ///< Find "one" only in body text.
    InSelAll   = 0x01,     ///< All (only in non-body and selections).
    InOther    = 0x02,     ///< Find "all" in Footer/Header/Fly...
    InSel      = 0x04,     ///< Find in selections.
    InBodyOnly = 0x08,     ///< Find only in body - only in combination with FindRanges::InSelAll !!!
};
namespace o3tl
{
    template<> struct typed_flags<FindRanges> : is_typed_flags<FindRanges, 0x0f> {};
}

enum class SwDocPositions
{
    Start,
    Curr,
    End,
    OtherStart,
    OtherEnd
};

#endif // INCLUDED_SW_INC_CSHTYP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
