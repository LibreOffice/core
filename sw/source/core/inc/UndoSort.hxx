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

