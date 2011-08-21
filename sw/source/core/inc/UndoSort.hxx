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

#ifndef SW_UNDO_SORT_HXX
#define SW_UNDO_SORT_HXX

#include <undobj.hxx>


struct SwSortOptions;
class SwTableNode;
class SwUndoAttrTbl;


/*--------------------------------------------------------------------
    Undo for Sorting
 --------------------------------------------------------------------*/

struct SwSortUndoElement
{
    union {
        struct {
            sal_uLong nKenn;
            sal_uLong nSource, nTarget;
        } TXT;
        struct {
            String *pSource, *pTarget;
        } TBL;
    } SORT_TXT_TBL;

    SwSortUndoElement( const String& aS, const String& aT )
    {
        SORT_TXT_TBL.TBL.pSource = new String( aS );
        SORT_TXT_TBL.TBL.pTarget = new String( aT );
    }
    SwSortUndoElement( sal_uLong nS, sal_uLong nT )
    {
        SORT_TXT_TBL.TXT.nSource = nS;
        SORT_TXT_TBL.TXT.nTarget = nT;
        SORT_TXT_TBL.TXT.nKenn   = 0xffffffff;
    }
    ~SwSortUndoElement();
};

SV_DECL_PTRARR_DEL(SwSortList, SwSortUndoElement*, 10,30)
SV_DECL_PTRARR(SwUndoSortList, SwNodeIndex*, 10,30)

class SwUndoSort : public SwUndo, private SwUndRng
{
    SwSortOptions*  pSortOpt;
    SwSortList      aSortList;
    SwUndoAttrTbl*  pUndoTblAttr;
    SwRedlineData*  pRedlData;
    sal_uLong           nTblNd;

public:
    SwUndoSort( const SwPaM&, const SwSortOptions& );
    SwUndoSort( sal_uLong nStt, sal_uLong nEnd, const SwTableNode&,
                const SwSortOptions&, sal_Bool bSaveTable );

    virtual ~SwUndoSort();

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );
    virtual void RepeatImpl( ::sw::RepeatContext & );

    void Insert( const String& rOrgPos, const String& rNewPos );
    void Insert( sal_uLong nOrgPos, sal_uLong nNewPos );

};

#endif // SW_UNDO_SORT_HXX

