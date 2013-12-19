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
    SwUndoDelete(
        SwPaM&,
        sal_Bool bFullPara = sal_False,
        sal_Bool bCalledByTblCpy = sal_False );
    virtual ~SwUndoDelete();

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );
    virtual void RepeatImpl( ::sw::RepeatContext & );

    // #111827#
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

