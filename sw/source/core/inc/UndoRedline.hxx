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

#ifndef SW_UNDO_REDLINE_HXX
#define SW_UNDO_REDLINE_HXX

#include <undobj.hxx>


struct SwSortOptions;
class SwRedline;
class SwRedlineSaveDatas;
class SwUndoDelete;


//--------------------------------------------------------------------

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

//--------------------------------------------------------------------

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

