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

#ifndef SW_UNDO_REDLINE_HXX
#define SW_UNDO_REDLINE_HXX

#include <undobj.hxx>

struct SwSortOptions;
class SwRedline;
class SwRedlineSaveDatas;
class SwUndoDelete;

class SwUndoRedline : public SwUndo, public SwUndRng
{
protected:
    SwRedlineData* pRedlData;
    SwRedlineSaveDatas* pRedlSaveData;
    SwUndoId nUserId;
    sal_Bool bHiddenRedlines;

    virtual void UndoRedlineImpl(SwDoc & rDoc, SwPaM & rPam);
    virtual void RedoRedlineImpl(SwDoc & rDoc, SwPaM & rPam);

public:
    SwUndoRedline( SwUndoId nUserId, const SwPaM& rRange );

    virtual ~SwUndoRedline();

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );

    SwUndoId GetUserId() const { return nUserId; }
    sal_uInt16 GetRedlSaveCount() const;
};

class SwUndoRedlineDelete : public SwUndoRedline
{
    sal_Bool bCanGroup : 1;
    sal_Bool bIsDelim : 1;
    sal_Bool bIsBackspace : 1;

    virtual void UndoRedlineImpl(SwDoc & rDoc, SwPaM & rPam);
    virtual void RedoRedlineImpl(SwDoc & rDoc, SwPaM & rPam);

public:
    SwUndoRedlineDelete( const SwPaM& rRange, SwUndoId nUserId = UNDO_EMPTY );

    sal_Bool CanGrouping( const SwUndoRedlineDelete& rPrev );

    // SwUndoTblCpyTbl needs this information:
    long NodeDiff() const { return nSttNode - nEndNode; }
    xub_StrLen ContentStart() const { return nSttCntnt; }
};

class SwUndoRedlineSort : public SwUndoRedline
{
    SwSortOptions* pOpt;
    sal_uLong nSaveEndNode, nOffset;
    xub_StrLen nSaveEndCntnt;

    virtual void UndoRedlineImpl(SwDoc & rDoc, SwPaM & rPam);
    virtual void RedoRedlineImpl(SwDoc & rDoc, SwPaM & rPam);

public:
    SwUndoRedlineSort( const SwPaM& rRange, const SwSortOptions& rOpt );

    virtual ~SwUndoRedlineSort();

    virtual void RepeatImpl( ::sw::RepeatContext & );

    void SetSaveRange( const SwPaM& rRange );
    void SetOffset( const SwNodeIndex& rIdx );
};

class SwUndoAcceptRedline : public SwUndoRedline
{
private:
    virtual void RedoRedlineImpl(SwDoc & rDoc, SwPaM & rPam);

public:
    SwUndoAcceptRedline( const SwPaM& rRange );

    virtual void RepeatImpl( ::sw::RepeatContext & );
};

class SwUndoRejectRedline : public SwUndoRedline
{
private:
    virtual void RedoRedlineImpl(SwDoc & rDoc, SwPaM & rPam);

public:
    SwUndoRejectRedline( const SwPaM& rRange );

    virtual void RepeatImpl( ::sw::RepeatContext & );
};

class SwUndoCompDoc : public SwUndo, public SwUndRng
{
    SwRedlineData* pRedlData;
    SwUndoDelete* pUnDel, *pUnDel2;
    SwRedlineSaveDatas* pRedlSaveData;
    sal_Bool bInsert;

public:
    SwUndoCompDoc( const SwPaM& rRg, sal_Bool bIns );
    SwUndoCompDoc( const SwRedline& rRedl );

    virtual ~SwUndoCompDoc();

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );
};

#endif // SW_UNDO_REDLINE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
