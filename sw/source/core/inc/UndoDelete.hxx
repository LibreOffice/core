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

#ifndef SW_UNDO_DELETE_HXX
#define SW_UNDO_DELETE_HXX

#include <undobj.hxx>

#include <tools/mempool.hxx>


class SwRedlineSaveDatas;
class SwTxtNode;

namespace sfx2 {
    class MetadatableUndo;
}


class SwUndoDelete
    : public SwUndo
    , private SwUndRng
    , private SwUndoSaveCntnt
{
    SwNodeIndex* pMvStt;            // Position of Nodes in UndoNodes-Array
    String *pSttStr, *pEndStr;
    SwRedlineData* pRedlData;
    SwRedlineSaveDatas* pRedlSaveData;
    ::boost::shared_ptr< ::sfx2::MetadatableUndo > m_pMetadataUndoStart;
    ::boost::shared_ptr< ::sfx2::MetadatableUndo > m_pMetadataUndoEnd;

    String sTableName;

    sal_uLong nNode;
    sal_uLong nNdDiff;           // difference of Nodes before/after Delete
    sal_uLong nSectDiff;         // diff. of Nodes before/after Move w/ SectionNodes
    sal_uLong nReplaceDummy;     // diff. to a temporary dummy object
    sal_uInt16 nSetPos;

    sal_Bool bGroup : 1;         // TRUE: is already Grouped; see CanGrouping()
    sal_Bool bBackSp : 1;        // TRUE: if Grouped and preceding content deleted
    sal_Bool bJoinNext: 1;       // TRUE: if range is selected forwards
    sal_Bool bTblDelLastNd : 1;  // TRUE: TextNode following Table inserted/deleted
    sal_Bool bDelFullPara : 1;   // TRUE: entire Nodes were deleted
    sal_Bool bResetPgDesc : 1;   // TRUE: reset PgDsc on following node
    sal_Bool bResetPgBrk : 1;    // TRUE: reset PgBreak on following node
    sal_Bool bFromTableCopy : 1; // TRUE: called by SwUndoTblCpyTbl

    sal_Bool SaveCntnt( const SwPosition* pStt, const SwPosition* pEnd,
                    SwTxtNode* pSttTxtNd, SwTxtNode* pEndTxtNd );
public:
    SwUndoDelete( SwPaM&,
            sal_Bool bFullPara = sal_False, sal_Bool bCalledByTblCpy = sal_False );
    virtual ~SwUndoDelete();

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );
    virtual void RepeatImpl( ::sw::RepeatContext & );
    /**
       Returns rewriter for this undo object.

       The rewriter consists of the following rule:

           $1 -> '<deleted text>'

       <deleted text> is shortened to nUndoStringLength characters.

       @return rewriter for this undo object
    */
    virtual SwRewriter GetRewriter() const;

    sal_Bool CanGrouping( SwDoc*, const SwPaM& );

    void SetTblDelLastNd()      { bTblDelLastNd = sal_True; }

    // for PageDesc/PageBreak Attributes of a table
    void SetPgBrkFlags( sal_Bool bPageBreak, sal_Bool bPageDesc )
        { bResetPgDesc = bPageDesc; bResetPgBrk = bPageBreak; }

    void SetTableName(const String & rName);

    // SwUndoTblCpyTbl needs this information:
    sal_Bool IsDelFullPara() const { return bDelFullPara; }

    DECL_FIXEDMEMPOOL_NEWDEL(SwUndoDelete)
};

#endif // SW_UNDO_DELETE_HXX

