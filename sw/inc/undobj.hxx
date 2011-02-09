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
#ifndef SW_UNDOBJ_HXX
#define SW_UNDOBJ_HXX

#include <vector>

#include <boost/shared_ptr.hpp>

#include <svl/undo.hxx>

#include <SwRewriter.hxx>
#include <swundo.hxx>


class SwHistory;
class SwIndex;
class SwPaM;
struct SwPosition;
class SwDoc;
class SwTxtFmtColl;
class SwFrmFmt;
class SwNodeIndex;
class SwNodeRange;
class SwRedlineData;
class SwRedlineSaveDatas;

namespace sw {
    class UndoRedoContext;
    class RepeatContext;
}


class SwUndo
    : public SfxUndoAction
{
    SwUndoId const m_nId;
    sal_uInt16 nOrigRedlineMode;

protected:
    bool bCacheComment;
    mutable String * pComment;

    void RemoveIdxFromSection( SwDoc&, sal_uLong nSttIdx, sal_uLong* pEndIdx = 0 );
    void RemoveIdxFromRange( SwPaM& rPam, sal_Bool bMoveNext );
    void RemoveIdxRel( sal_uLong, const SwPosition& );

    static sal_Bool CanRedlineGroup( SwRedlineSaveDatas& rCurr,
                                const SwRedlineSaveDatas& rCheck,
                                sal_Bool bCurrIsEnd );

    // #111827#
    /**
       Returns the rewriter for this object.

       @return the rewriter for this object
    */
    virtual SwRewriter GetRewriter() const;

    // return type is sal_uInt16 because this overrides SfxUndoAction::GetId()
    virtual sal_uInt16 GetId() const { return static_cast<sal_uInt16>(m_nId); }

    // the 4 methods that derived classes have to override
    // base implementation does nothing
    virtual void RepeatImpl( ::sw::RepeatContext & );
    virtual bool CanRepeatImpl( ::sw::RepeatContext & ) const;
public: // should not be public, but ran into trouble in untbl.cxx
    virtual void UndoImpl( ::sw::UndoRedoContext & ) = 0;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) = 0;

private:
    // SfxUndoAction
    virtual void Undo();
    virtual void Redo();
    virtual void UndoWithContext(SfxUndoContext &);
    virtual void RedoWithContext(SfxUndoContext &);
    virtual void Repeat(SfxRepeatTarget &);
    virtual sal_Bool CanRepeat(SfxRepeatTarget &) const;

public:
    SwUndo(SwUndoId const nId);
    virtual ~SwUndo();

    // #111827#
    /**
       Returns textual comment for this undo object.

       The textual comment is created from the resource string
       corresponding to this object's ID. The rewriter of this object
       is applied to the resource string to get the final comment.

       @return textual comment for this undo object
    */
    virtual String GetComment() const;

        // das UndoObject merkt sich, welcher Mode eingeschaltet war.
        // In Undo/Redo/Repeat wird dann immer auf diesen zurueck geschaltet
    sal_uInt16 GetRedlineMode() const { return nOrigRedlineMode; }
    void SetRedlineMode( sal_uInt16 eMode ) { nOrigRedlineMode = eMode; }

    bool IsDelBox() const;

        // sicher und setze die RedlineDaten
    static sal_Bool FillSaveData( const SwPaM& rRange, SwRedlineSaveDatas& rSData,
                            sal_Bool bDelRange = sal_True, sal_Bool bCopyNext = sal_True );
    static sal_Bool FillSaveDataForFmt( const SwPaM& , SwRedlineSaveDatas& );
    static void SetSaveData( SwDoc& rDoc, const SwRedlineSaveDatas& rSData );
    static sal_Bool HasHiddenRedlines( const SwRedlineSaveDatas& rSData );
};

typedef sal_uInt16 DelCntntType;
namespace nsDelCntntType
{
    const DelCntntType DELCNT_FTN = 0x01;
    const DelCntntType DELCNT_FLY = 0x02;
    const DelCntntType DELCNT_TOC = 0x04;
    const DelCntntType DELCNT_BKM = 0x08;
    const DelCntntType DELCNT_ALL = 0x0F;
    const DelCntntType DELCNT_CHKNOCNTNT = 0x80;
}

/// will DelCntntIndex destroy a frame anchored at character at rAnchorPos?
bool IsDestroyFrameAnchoredAtChar(SwPosition const & rAnchorPos,
        SwPosition const & rStart, SwPosition const & rEnd,
        DelCntntType const nDelCntntType = nsDelCntntType::DELCNT_ALL);

// diese Klasse muss in ein Undo-Object vererbt werden, wenn dieses Inhalt
// fuers Redo/Undo ... speichert
class SwUndoSaveCntnt
{
protected:

    SwHistory* pHistory;

    // wird fuer das loeschen von Inhalt benoetigt. Fuer das ReDo werden
    // Inhalte in das UndoNodesArray verschoben. Diese Methoden fuegen
    // am Ende eines TextNodes ein Trenner fuer die Attribute. Damit werden
    // Attribute nicht expandiert.
    // MoveTo..     verschiebt aus dem NodesArray in das UndoNodesArray
    // MoveFrom..   verschiebt aus dem UndoNodesArray in das NodesArray
    void MoveToUndoNds( SwPaM& rPam,
                        SwNodeIndex* pNodeIdx = 0, SwIndex* pCntIdx = 0,
                        sal_uLong* pEndNdIdx = 0, xub_StrLen * pEndCntIdx = 0 );
    void MoveFromUndoNds( SwDoc& rDoc, sal_uLong nNodeIdx, xub_StrLen nCntntIdx,
                          SwPosition& rInsPos,
                          sal_uLong* pEndNdIdx = 0, xub_StrLen * pEndCntIdx = 0 );

    // diese beiden Methoden bewegen den SPoint vom Pam zurueck/vor. Damit
    // kann fuer ein Undo/Redo ein Bereich aufgespannt werden. (Der
    // SPoint liegt dann vor dem manipuliertem Bereich !!)
    // Das Flag gibt an, ob noch vorm SPoint Inhalt steht.
    sal_Bool MovePtBackward( SwPaM& rPam );
    void MovePtForward( SwPaM& rPam, sal_Bool bMvBkwrd );

    // vor dem Move ins UndoNodes-Array muss dafuer gesorgt werden, das
    // die Inhaltstragenden Attribute aus dem Nodes-Array entfernt werden.
    void DelCntntIndex( const SwPosition& pMark, const SwPosition& pPoint,
                        DelCntntType nDelCntntType = nsDelCntntType::DELCNT_ALL );

public:
    SwUndoSaveCntnt();
    ~SwUndoSaveCntnt();
};


// sicher eine vollstaendige Section im Nodes-Array
class SwUndoSaveSection : private SwUndoSaveCntnt
{
    SwNodeIndex *pMvStt;
    SwRedlineSaveDatas* pRedlSaveData;
    sal_uLong nMvLen;           // Index ins UndoNodes-Array
    sal_uLong nStartPos;

protected:
    SwNodeIndex* GetMvSttIdx() const { return pMvStt; }
    sal_uLong GetMvNodeCnt() const { return nMvLen; }

public:
    SwUndoSaveSection();
    ~SwUndoSaveSection();

    void SaveSection( SwDoc* pDoc, const SwNodeIndex& rSttIdx );
    void SaveSection( SwDoc* pDoc, const SwNodeRange& rRange );
    void RestoreSection( SwDoc* pDoc, SwNodeIndex* pIdx, sal_uInt16 nSectType );
    void RestoreSection( SwDoc* pDoc, const SwNodeIndex& rInsPos );

    const SwHistory* GetHistory() const { return pHistory; }
          SwHistory* GetHistory()       { return pHistory; }
};


// Diese Klasse speichert den Pam als sal_uInt16's und kann diese wieder zu
// einem PaM zusammensetzen
class SwUndRng
{
public:
    sal_uLong nSttNode, nEndNode;
    xub_StrLen nSttCntnt, nEndCntnt;

    SwUndRng();
    SwUndRng( const SwPaM& );

    void SetValues( const SwPaM& rPam );
    void SetPaM( SwPaM&, sal_Bool bCorrToCntnt = sal_False ) const;
    SwPaM & AddUndoRedoPaM(
        ::sw::UndoRedoContext &, bool const bCorrToCntnt = false) const;
};



class SwUndoInsLayFmt;

<<<<<<< local
public:
    SwUndoStart( SwUndoId nId );
    virtual void Undo( SwUndoIter& );
    virtual void Redo( SwUndoIter& );
    virtual void Repeat( SwUndoIter& );

    // -> #111827#
    virtual String GetComment() const;
    void SetRewriter(const SwRewriter & rRewriter);
    virtual SwRewriter GetRewriter() const;
    // <- #111827#

    virtual SwUndoId GetEffectiveId() const;
    SwUndoId GetUserId() const { return nUserId; }
    // Setzen vom End-Undo-Offset geschieht im Doc::EndUndo
    USHORT GetEndOffset() const { return nEndOffset; }
    void SetEndOffset( USHORT n ) { nEndOffset = n; }
};

class SwUndoEnd: public SwUndo
{
    // Um innerhalb von Undo zuerkennen, wann ein Ende vorliegt, gibt
    // GetId() immer die UNDO_END zurueck. Die UserId kann ueber
    // GetUserId() erfragt werden.
    SwUndoId nUserId;
    // fuer die "Verpointerung" von Start- und End-Undos
    USHORT nSttOffset;

    SwRewriter mRewriter;

public:
    SwUndoEnd( SwUndoId nId );
    virtual void Undo( SwUndoIter& );
    virtual void Redo( SwUndoIter& );
    virtual void Repeat( SwUndoIter& );

    // -> #111827#
    virtual String GetComment() const;
    void SetRewriter(const SwRewriter & rRewriter);
    virtual SwRewriter GetRewriter() const;
    // <- #111827#

    virtual SwUndoId GetEffectiveId() const;
    SwUndoId GetUserId() const { return nUserId; }

    // Setzen vom Start-Undo-Offset geschieht im Doc::EndUndo
    void SetSttOffset(USHORT _nSttOffSet) { nSttOffset = _nSttOffSet; }
    USHORT GetSttOffset() const { return nSttOffset; }
};

class SwUndoInsert: public SwUndo, private SwUndoSaveCntnt
{
    SwPosition *pPos;                   // Inhalt fuers Redo
    String *pTxt, *pUndoTxt;
    SwRedlineData* pRedlData;
    ULONG nNode;
    xub_StrLen nCntnt, nLen;
    BOOL bIsWordDelim : 1;
    BOOL bIsAppend : 1;

    const IDocumentContentOperations::InsertFlags m_nInsertFlags;

    friend class SwDoc;     // eigentlich nur SwDoc::Insert( String )
    BOOL CanGrouping( sal_Unicode cIns );
    BOOL CanGrouping( const SwPosition& rPos );

    SwDoc * pDoc;

    void Init(const SwNodeIndex & rNode);
    String * GetTxtFromDoc() const;

public:
    SwUndoInsert( const SwNodeIndex& rNode, xub_StrLen nCntnt, xub_StrLen nLen,
                  const IDocumentContentOperations::InsertFlags nInsertFlags,
                  BOOL bWDelim = TRUE );
    SwUndoInsert( const SwNodeIndex& rNode );
    virtual ~SwUndoInsert();

    virtual void Undo( SwUndoIter& );
    virtual void Redo( SwUndoIter& );
    virtual void Repeat( SwUndoIter& );

    // #111827#
    /**
       Returns rewriter for this undo object.

       The returned rewriter has the following rule:

           $1 -> '<inserted text>'

       <inserted text> is shortened to a length of nUndoStringLength.

       @return rewriter for this undo object
     */
    virtual SwRewriter GetRewriter() const;


    DECL_FIXEDMEMPOOL_NEWDEL(SwUndoInsert)
};


class SwUndoDelete: public SwUndo, private SwUndRng, private SwUndoSaveCntnt
{
    SwNodeIndex* pMvStt;            // Position der Nodes im UndoNodes-Array
    String *pSttStr, *pEndStr;
    SwRedlineData* pRedlData;
    SwRedlineSaveDatas* pRedlSaveData;
    ::boost::shared_ptr< ::sfx2::MetadatableUndo > m_pMetadataUndoStart;
    ::boost::shared_ptr< ::sfx2::MetadatableUndo > m_pMetadataUndoEnd;

    String sTableName;

    ULONG nNode;
    ULONG nNdDiff;              // Differenz von Nodes vor-nach Delete
    ULONG nSectDiff;            // Diff. von Nodes vor/nach Move mit SectionNodes
    ULONG nReplaceDummy;        // Diff. to a temporary dummy object
    USHORT nSetPos;

    BOOL bGroup : 1;    // TRUE: ist schon eine Gruppe; wird in CanGrouping() ausgwertet !!
    BOOL bBackSp : 1;   // TRUE: wenn Gruppierung und der Inhalt davor geloescht wird
    BOOL bJoinNext: 1;  // TRUE: wenn der Bereich von Oben nach unten geht
    BOOL bTblDelLastNd : 1; // TRUE: TextNode hinter der Tabelle einf./loeschen
    BOOL bDelFullPara : 1;  // TRUE: gesamte Nodes wurden geloescht
    BOOL bResetPgDesc : 1;  // TRUE: am nachfolgenden Node das PgDsc zuruecksetzen
    BOOL bResetPgBrk : 1;   // TRUE: am nachfolgenden Node das PgBreak zuruecksetzen
    BOOL bFromTableCopy : 1; // TRUE: called by SwUndoTblCpyTbl

    BOOL SaveCntnt( const SwPosition* pStt, const SwPosition* pEnd,
                    SwTxtNode* pSttTxtNd, SwTxtNode* pEndTxtNd );
public:
    SwUndoDelete( SwPaM&, BOOL bFullPara = FALSE, BOOL bCalledByTblCpy = FALSE );
    virtual ~SwUndoDelete();
    virtual void Undo( SwUndoIter& );
    virtual void Redo( SwUndoIter& );
    virtual void Repeat( SwUndoIter& );

    // #111827#
    /**
       Returns rewriter for this undo object.

       The rewriter consists of the following rule:

           $1 -> '<deleted text>'

       <deleted text> is shortened to nUndoStringLength characters.

       @return rewriter for this undo object
    */
    virtual SwRewriter GetRewriter() const;

    BOOL CanGrouping( SwDoc*, const SwPaM& );

    void SetTblDelLastNd()      { bTblDelLastNd = TRUE; }

    // fuer die PageDesc/PageBreak Attribute einer Tabelle
    void SetPgBrkFlags( BOOL bPageBreak, BOOL bPageDesc )
        { bResetPgDesc = bPageDesc; bResetPgBrk = bPageBreak; }

    void SetTableName(const String & rName);

    // SwUndoTblCpyTbl needs this information:
    long NodeDiff() const { return nSttNode - nEndNode; }
    xub_StrLen ContentStart() const { return nSttCntnt; }
    BOOL IsDelFullPara() const { return bDelFullPara; }

    DECL_FIXEDMEMPOOL_NEWDEL(SwUndoDelete)
};


class SwUndoOverwrite: public SwUndo, private SwUndoSaveCntnt
{
    String aDelStr, aInsStr;
    SwRedlineSaveDatas* pRedlSaveData;
    ULONG nSttNode;
    xub_StrLen nSttCntnt;
    BOOL bInsChar : 1;      // kein Overwrite mehr; sondern Insert
    BOOL bGroup : 1;        // TRUE: ist schon eine Gruppe; wird in
                            //       CanGrouping() ausgwertet !!
public:
    SwUndoOverwrite( SwDoc*, SwPosition&, sal_Unicode cIns );
    virtual ~SwUndoOverwrite();
    virtual void Undo( SwUndoIter& );
    virtual void Redo( SwUndoIter& );
    virtual void Repeat( SwUndoIter& );

    // #111827#
    /**
       Returns the rewriter of this undo object.

       The rewriter contains the following rule:

           $1 -> '<overwritten text>'

       <overwritten text> is shortened to nUndoStringLength characters.

       @return the rewriter of this undo object
     */
    virtual SwRewriter GetRewriter() const;

    BOOL CanGrouping( SwDoc*, SwPosition&, sal_Unicode cIns );
};


class SwUndoSplitNode: public SwUndo
{
    SwHistory* pHistory;
    SwRedlineData* pRedlData;
    ULONG nNode;
    xub_StrLen nCntnt;
    BOOL bTblFlag : 1;
    BOOL bChkTblStt : 1;
public:
    SwUndoSplitNode( SwDoc* pDoc, const SwPosition& rPos, BOOL bChkTbl );
    virtual ~SwUndoSplitNode();
    virtual void Undo( SwUndoIter& );
    virtual void Redo( SwUndoIter& );
    virtual void Repeat( SwUndoIter& );
    void SetTblFlag()       { bTblFlag = TRUE; }
};


class SwUndoMove : public SwUndo, private SwUndRng, private SwUndoSaveCntnt
{
    // nDest.. - Bereich, in den verschoben wurde (nach dem Move!)
    // nIns.. - Position, von der verschoben wurde und wieder die neue InsPos. ist
    // nMv.. Position auf die verschoben wird (vor dem Move!) ; fuers REDO
    ULONG nDestSttNode, nDestEndNode, nInsPosNode, nMvDestNode;
    xub_StrLen nDestSttCntnt, nDestEndCntnt, nInsPosCntnt, nMvDestCntnt;

    USHORT nFtnStt;         // StartPos der Fussnoten in der History

    BOOL bJoinNext : 1,
         bJoinPrev : 1,
         bMoveRange : 1;

    bool bMoveRedlines; // use DOC_MOVEREDLINES when calling SwDoc::Move

    void DelFtn( const SwPaM& );
public:
    SwUndoMove( const SwPaM&, const SwPosition& );
    SwUndoMove( SwDoc* pDoc, const SwNodeRange&, const SwNodeIndex& );
    virtual void Undo( SwUndoIter& );
    virtual void Redo( SwUndoIter& );
    // setze den Destination-Bereich nach dem Verschieben.
    void SetDestRange( const SwPaM&, const SwPosition&, BOOL, BOOL );
    void SetDestRange( const SwNodeIndex& rStt, const SwNodeIndex& rEnd,
                        const SwNodeIndex& rInsPos );

    BOOL IsMoveRange() const        { return bMoveRange; }
    ULONG GetEndNode() const        { return nEndNode; }
    ULONG GetDestSttNode() const    { return nDestSttNode; }
    xub_StrLen GetDestSttCntnt() const  { return nDestSttCntnt; }

    void SetMoveRedlines( bool b )       { bMoveRedlines = b; }

};


class SwUndoAttr : public SwUndo, private SwUndRng
{
    SfxItemSet m_AttrSet;                           // attributes for Redo
    const ::std::auto_ptr<SwHistory> m_pHistory;    // History for Undo
    ::std::auto_ptr<SwRedlineData> m_pRedlineData;  // Redlining
    ::std::auto_ptr<SwRedlineSaveDatas> m_pRedlineSaveData;
    ULONG m_nNodeIndex;                             // Offset: for Redlining
    const SetAttrMode m_nInsertFlags;               // insert flags

    void RemoveIdx( SwDoc& rDoc );

public:
    SwUndoAttr( const SwPaM&, const SfxItemSet &, const SetAttrMode nFlags );
    SwUndoAttr( const SwPaM&, const SfxPoolItem&, const SetAttrMode nFlags );
    virtual ~SwUndoAttr();
    virtual void Undo( SwUndoIter& );
    virtual void Redo( SwUndoIter& );
    virtual void Repeat( SwUndoIter& );
    void SaveRedlineData( const SwPaM& rPam, BOOL bInsCntnt );

    SwHistory& GetHistory() { return *m_pHistory; }

};

class SwUndoResetAttr : public SwUndo, private SwUndRng
{
    const ::std::auto_ptr<SwHistory> m_pHistory;
    SvUShortsSort m_Ids;
    const USHORT m_nFormatId;             // Format-Id for Redo

public:
    SwUndoResetAttr( const SwPaM&, USHORT nFmtId );
    SwUndoResetAttr( const SwPosition&, USHORT nFmtId );
    virtual ~SwUndoResetAttr();
    virtual void Undo( SwUndoIter& );
    virtual void Redo( SwUndoIter& );
    virtual void Repeat( SwUndoIter& );
    void SetAttrs( const SvUShortsSort& rArr );

    SwHistory& GetHistory() { return *m_pHistory; }

};

class SwUndoFmtAttr : public SwUndo
{
    friend class SwUndoDefaultAttr;
    SwFmt * m_pFmt;
    ::std::auto_ptr<SfxItemSet> m_pOldSet;    // old attributes
    ULONG m_nNodeIndex;
    const USHORT m_nFmtWhich;
    const bool m_bSaveDrawPt;

    bool IsFmtInDoc( SwDoc* );   //is the attribute format still in the Doc?
    void SaveFlyAnchor( bool bSaveDrawPt = false );
    // --> OD 2004-10-26 #i35443# - Add return value, type <bool>.
    // Return value indicates, if anchor attribute is restored.
    // Notes: - If anchor attribute is restored, all other existing attributes
    //          are also restored.
    //        - Anchor attribute isn't restored successfully, if it contains
    //          an invalid anchor position and all other existing attributes
    //          aren't restored.
    //          This situation occurs for undo of styles.
    bool RestoreFlyAnchor( SwUndoIter& rIter );
    // <--
    // --> OD 2008-02-27 #refactorlists# - removed <rAffectedItemSet>
    void Init();
    // <--

public:
    // register at the Format and save old attributes
    // --> OD 2008-02-27 #refactorlists# - removed <rNewSet>
    SwUndoFmtAttr( const SfxItemSet& rOldSet,
                   SwFmt& rFmt,
                   bool bSaveDrawPt = true );
    // <--
    SwUndoFmtAttr( const SfxPoolItem& rItem,
                   SwFmt& rFmt,
                   bool bSaveDrawPt = true );
    virtual ~SwUndoFmtAttr();
    virtual void Undo( SwUndoIter& );
    // --> OD 2004-10-26 #i35443# - <Redo(..)> calls <Undo(..)> - nothing else
    virtual void Redo( SwUndoIter& );
    // <--
    virtual void Repeat( SwUndoIter& );
    virtual SwRewriter GetRewriter() const;

    void PutAttr( const SfxPoolItem& rItem );
    SwFmt* GetFmt( SwDoc& rDoc );   // checks if it is still in the Doc!
};

// --> OD 2008-02-12 #newlistlevelattrs#
class SwUndoFmtResetAttr : public SwUndo
{
    public:
        SwUndoFmtResetAttr( SwFmt& rChangedFormat,
                            const USHORT nWhichId );
        ~SwUndoFmtResetAttr();

        virtual void Undo( SwUndoIter& );
        virtual void Redo( SwUndoIter& );

    private:
        // format at which a certain attribute is reset.
        SwFmt * const m_pChangedFormat;
        // which ID of the reset attribute
        const USHORT m_nWhichId;
        // old attribute which has been reset - needed for undo.
        ::std::auto_ptr<SfxPoolItem> m_pOldItem;
};
// <--

class SwUndoDontExpandFmt : public SwUndo
{
    const ULONG m_nNodeIndex;
    const xub_StrLen m_nContentIndex;

public:
    SwUndoDontExpandFmt( const SwPosition& rPos );
    virtual void Undo( SwUndoIter& );
    virtual void Redo( SwUndoIter& );
    virtual void Repeat( SwUndoIter& );
};

// helper class to receive changed attribute sets
class SwUndoFmtAttrHelper : public SwClient
{
    ::std::auto_ptr<SwUndoFmtAttr> m_pUndo;
    const bool m_bSaveDrawPt;
protected:
    virtual void Modify( const SfxPoolItem*, const SfxPoolItem* );

public:
    SwUndoFmtAttrHelper( SwFmt& rFmt, bool bSaveDrawPt = true );

    SwUndoFmtAttr* GetUndo() const  { return m_pUndo.get(); }
    // release the undo object (so it is not deleted here), and return it
    SwUndoFmtAttr* ReleaseUndo()    { return m_pUndo.release(); }
};


class SwUndoFmtColl : public SwUndo, private SwUndRng
{
    String aFmtName;
    SwHistory* pHistory;
    SwFmtColl* pFmtColl;
    // --> OD 2008-04-15 #refactorlists# - for correct <ReDo(..)> and <Repeat(..)>
    // boolean, which indicates that the attributes are reseted at the nodes
    // before the format has been applied.
    const bool mbReset;
    // boolean, which indicates that the list attributes had been reseted at
    // the nodes before the format has been applied.
    const bool mbResetListAttrs;
    // <--
public:
    // --> OD 2008-04-15 #refactorlists#
//    SwUndoFmtColl( const SwPaM&, SwFmtColl* );
    SwUndoFmtColl( const SwPaM&, SwFmtColl*,
                   const bool bReset,
                   const bool bResetListAttrs );
    // <--
    virtual ~SwUndoFmtColl();
    virtual void Undo( SwUndoIter& );
    virtual void Redo( SwUndoIter& );
    virtual void Repeat( SwUndoIter& );

    // #111827#
    /**
       Returns the rewriter for this undo object.

       The rewriter contains one rule:

           $1 -> <name of format collection>

       <name of format collection> is the name of the format
       collection that is applied by the action recorded by this undo
       object.

       @return the rewriter for this undo object
    */
    virtual SwRewriter GetRewriter() const;

    SwHistory* GetHistory() { return pHistory; }

};


class SwUndoMoveLeftMargin : public SwUndo, private SwUndRng
{
    const ::std::auto_ptr<SwHistory> m_pHistory;
    const bool m_bModulus;

public:
    SwUndoMoveLeftMargin( const SwPaM&, BOOL bRight, BOOL bModulus );
    virtual ~SwUndoMoveLeftMargin();
    virtual void Undo( SwUndoIter& );
    virtual void Redo( SwUndoIter& );
    virtual void Repeat( SwUndoIter& );

    SwHistory& GetHistory() { return *m_pHistory; }

};

// Basis-Klasse fuer Insert von Dokument, Glossaries und Kopieren
=======
// base class for insertion of Document, Glossaries and Copy
>>>>>>> other
class SwUndoInserts : public SwUndo, public SwUndRng, private SwUndoSaveCntnt
{
    SwTxtFmtColl *pTxtFmtColl, *pLastNdColl;
    SvPtrarr* pFrmFmts;
    ::std::vector< ::boost::shared_ptr<SwUndoInsLayFmt> > m_FlyUndos;
    SwRedlineData* pRedlData;
    sal_Bool bSttWasTxtNd;
protected:
    sal_uLong nNdDiff;
    SwPosition *pPos;                   // Inhalt fuers Redo
    sal_uInt16 nSetPos;                     // Start in der History-Liste

    SwUndoInserts( SwUndoId nUndoId, const SwPaM& );
public:
    virtual ~SwUndoInserts();

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );
    virtual void RepeatImpl( ::sw::RepeatContext & );

    // setze den Destination-Bereich nach dem Einlesen.
    void SetInsertRange( const SwPaM&, sal_Bool bScanFlys = sal_True,
                        sal_Bool bSttWasTxtNd = sal_True );
};

class SwUndoInsDoc : public SwUndoInserts
{
public:
    SwUndoInsDoc( const SwPaM& );
};

class SwUndoCpyDoc : public SwUndoInserts
{
public:
    SwUndoCpyDoc( const SwPaM& );
};



//--------------------------------------------------------------------

class SwUndoFlyBase : public SwUndo, private SwUndoSaveSection
{
protected:
    SwFrmFmt* pFrmFmt;                  // das gespeicherte FlyFormat
    sal_uLong nNdPgPos;
    xub_StrLen nCntPos;                 // Seite/am Absatz/im Absatz
    sal_uInt16 nRndId;
    sal_Bool bDelFmt;                       // loesche das gespeicherte Format

    void InsFly(::sw::UndoRedoContext & rContext, bool bShowSel = true);
    void DelFly( SwDoc* );

    SwUndoFlyBase( SwFrmFmt* pFormat, SwUndoId nUndoId );

    SwNodeIndex* GetMvSttIdx() const { return SwUndoSaveSection::GetMvSttIdx(); }
    sal_uLong GetMvNodeCnt() const { return SwUndoSaveSection::GetMvNodeCnt(); }

public:
    virtual ~SwUndoFlyBase();

};

class SwUndoInsLayFmt : public SwUndoFlyBase
{
    sal_uLong mnCrsrSaveIndexPara;           // Cursor position
    xub_StrLen mnCrsrSaveIndexPos;            // for undo
public:
    SwUndoInsLayFmt( SwFrmFmt* pFormat, sal_uLong nNodeIdx, xub_StrLen nCntIdx );

    virtual ~SwUndoInsLayFmt();

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );
    virtual void RepeatImpl( ::sw::RepeatContext & );

    String GetComment() const;

};

class SwUndoDelLayFmt : public SwUndoFlyBase
{
    sal_Bool bShowSelFrm;
public:
    SwUndoDelLayFmt( SwFrmFmt* pFormat );

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );

    void RedoForRollback();

    void ChgShowSel( sal_Bool bNew ) { bShowSelFrm = bNew; }

    virtual SwRewriter GetRewriter() const;

};

<<<<<<< local

class SwUndoSetFlyFmt : public SwUndo, public SwClient
{
    SwFrmFmt* pFrmFmt;                  // das gespeicherte FlyFormat
    SwFrmFmt* pOldFmt;                  // die alte Fly Vorlage
    SwFrmFmt* pNewFmt;                  // die neue Fly Vorlage
    SfxItemSet* pItemSet;               // die zurueck-/ gesetzten Attribute
    ULONG nOldNode, nNewNode;
    xub_StrLen nOldCntnt, nNewCntnt;
    USHORT nOldAnchorTyp, nNewAnchorTyp;
    BOOL bAnchorChgd;

    void PutAttr( USHORT nWhich, const SfxPoolItem* pItem );
    void GetAnchor( SwFmtAnchor& rAnhor, ULONG nNode, xub_StrLen nCntnt );

protected:
   virtual void Modify( const SfxPoolItem*, const SfxPoolItem* );

public:
    SwUndoSetFlyFmt( SwFrmFmt& rFlyFmt, SwFrmFmt& rNewFrmFmt );
    virtual ~SwUndoSetFlyFmt();

    virtual void Undo( SwUndoIter& );
    virtual void Redo( SwUndoIter& );

    virtual SwRewriter GetRewriter() const;
    void DeRegisterFromFormat( SwFmt& );
};

//--------------------------------------------------------------------

class _UnReplaceData;
SV_DECL_PTRARR_DEL( _UnReplaceDatas, _UnReplaceData*, 10, 25 )

class SwUndoReplace : public SwUndo
{
    friend class SwDoc;

    BOOL bOldIterFlag;      // Status vom Undo-Iter vorm 1. Aufruf
    USHORT nAktPos;         // fuer GetUndoRange und Undo/Redo
    _UnReplaceDatas aArr;
    SwRedlineData* pRedlData;

public:
    SwUndoReplace();
    virtual ~SwUndoReplace();
    virtual void Undo( SwUndoIter& );
    virtual void Redo( SwUndoIter& );

    // #111827#
    /**
       Returns the rewriter of this undo object.

       If this undo object represents several replacements the
       rewriter contains the following rules:

           $1 -> <number of replacements>
           $2 -> occurrences of
           $3 -> <replaced text>

       If this undo object represents one replacement the rewriter
       contains these rules:

           $1 -> <replaced text>
           $2 -> "->"                   (STR_YIELDS)
           $3 -> <replacing text>

       @return the rewriter of this undo object
    */
    virtual SwRewriter GetRewriter() const;

    void AddEntry( const SwPaM& rPam, const String& rInsert, BOOL bRegExp );
    void SetEntryEnd( const SwPaM& rPam );

    BOOL IsFull() const
        { return ((USHRT_MAX / sizeof( void* )) - 50 ) < aArr.Count(); }

};


//--------------------------------------------------------------------


class SwUndoTblHeadline : public SwUndo
{
    ULONG nTblNd;
    USHORT nOldHeadline;
    USHORT nNewHeadline;
public:
    SwUndoTblHeadline( const SwTable&, USHORT nOldHdl,  USHORT nNewHdl );
    virtual void Undo( SwUndoIter& );
    virtual void Redo( SwUndoIter& );
    virtual void Repeat( SwUndoIter& );
};


//------------ Undo von Insert-/Delete-Sections ----------------------

class SwUndoInsSection : public SwUndo, private SwUndRng
{
private:
    const ::std::auto_ptr<SwSectionData> m_pSectionData;
    const ::std::auto_ptr<SwTOXBase> m_pTOXBase; /// set iff section is TOX
    const ::std::auto_ptr<SfxItemSet> m_pAttrSet;
    ::std::auto_ptr<SwHistory> m_pHistory;
    ::std::auto_ptr<SwRedlineData> m_pRedlData;
    ULONG m_nSectionNodePos;
    bool m_bSplitAtStart : 1;
    bool m_bSplitAtEnd : 1;
    bool m_bUpdateFtn : 1;

    void Join( SwDoc& rDoc, ULONG nNode );

public:
    SwUndoInsSection(SwPaM const&, SwSectionData const&,
        SfxItemSet const*const pSet, SwTOXBase const*const pTOXBase);
    virtual ~SwUndoInsSection();
    virtual void Undo( SwUndoIter& );
    virtual void Redo( SwUndoIter& );
    virtual void Repeat( SwUndoIter& );

    void SetSectNdPos(ULONG const nPos)     { m_nSectionNodePos = nPos; }
    void SaveSplitNode(SwTxtNode *const pTxtNd, bool const bAtStart);
    void SetUpdtFtnFlag(bool const bFlag)   { m_bUpdateFtn = bFlag; }
};

SW_DLLPRIVATE SwUndo * MakeUndoDelSection(SwSectionFmt const&);

SW_DLLPRIVATE SwUndo * MakeUndoUpdateSection(SwSectionFmt const&, bool const);


//------------ Undo von verschieben/stufen von Gliederung ----------------

class SwUndoOutlineLeftRight : public SwUndo, private SwUndRng
{
    short nOffset;
public:
    SwUndoOutlineLeftRight( const SwPaM& rPam, short nOffset );
    virtual void Undo( SwUndoIter& );
    virtual void Redo( SwUndoIter& );
    virtual void Repeat( SwUndoIter& );
};

//--------------------------------------------------------------------

class SwUndoDefaultAttr : public SwUndo
{
    ::std::auto_ptr<SfxItemSet> m_pOldSet;        // the old attributes
    ::std::auto_ptr<SvxTabStopItem> m_pTabStop;

public:
    // registers at the format and saves old attributes
    SwUndoDefaultAttr( const SfxItemSet& rOldSet );
    virtual ~SwUndoDefaultAttr();
    virtual void Undo( SwUndoIter& );
    virtual void Redo( SwUndoIter& );
};

//--------------------------------------------------------------------
// ---------- Undo fuer Numerierung ----------------------------------

class SwUndoInsNum : public SwUndo, private SwUndRng
{
    SwNumRule aNumRule;
    SwHistory* pHistory;
    ULONG nSttSet;
    SwNumRule* pOldNumRule;
    String sReplaceRule;
    USHORT nLRSavePos;
public:
    SwUndoInsNum( const SwPaM& rPam, const SwNumRule& rRule );
    SwUndoInsNum( const SwNumRule& rOldRule, const SwNumRule& rNewRule,
                  SwUndoId nUndoId = UNDO_INSFMTATTR );
    SwUndoInsNum( const SwPosition& rPos, const SwNumRule& rRule,
                            const String& rReplaceRule );
    virtual ~SwUndoInsNum();
    virtual void Undo( SwUndoIter& );
    virtual void Redo( SwUndoIter& );
    virtual void Repeat( SwUndoIter& );
    virtual SwRewriter GetRewriter() const;

    SwHistory* GetHistory();        // wird ggfs. neu angelegt!
    void SetSttNum( ULONG nNdIdx ) { nSttSet = nNdIdx; }
    void SaveOldNumRule( const SwNumRule& rOld );

    void SetLRSpaceEndPos();

};

class SwUndoDelNum : public SwUndo, private SwUndRng
{
    SvULongs aNodeIdx;
    SvBytes aLevels;
    SvBools aRstLRSpaces;
    SwHistory* pHistory;
public:
    SwUndoDelNum( const SwPaM& rPam );
    virtual ~SwUndoDelNum();
    virtual void Undo( SwUndoIter& );
    virtual void Redo( SwUndoIter& );
    virtual void Repeat( SwUndoIter& );

    void AddNode( const SwTxtNode& rNd, BOOL bResetLRSpace );
    SwHistory* GetHistory() { return pHistory; }

};

class SwUndoMoveNum : public SwUndo, private SwUndRng
{
    ULONG nNewStt;
    long nOffset;
public:
    SwUndoMoveNum( const SwPaM& rPam, long nOffset, BOOL bIsOutlMv = FALSE );
    virtual void Undo( SwUndoIter& );
    virtual void Redo( SwUndoIter& );
    virtual void Repeat( SwUndoIter& );
    void SetStartNode( ULONG nValue ) { nNewStt = nValue; }
};

class SwUndoNumUpDown : public SwUndo, private SwUndRng
{
    short nOffset;
public:
    SwUndoNumUpDown( const SwPaM& rPam, short nOffset );
    virtual void Undo( SwUndoIter& );
    virtual void Redo( SwUndoIter& );
    virtual void Repeat( SwUndoIter& );
};

class SwUndoNumOrNoNum : public SwUndo
{
    ULONG nIdx;
    BOOL mbNewNum, mbOldNum;

public:
    SwUndoNumOrNoNum( const SwNodeIndex& rIdx, BOOL mbOldNum,
                      BOOL mbNewNum );
    virtual void Undo( SwUndoIter& );
    virtual void Redo( SwUndoIter& );
    virtual void Repeat( SwUndoIter& );
};

class SwUndoNumRuleStart : public SwUndo
{
    ULONG nIdx;
    USHORT nOldStt, nNewStt;
    BOOL bSetSttValue : 1;
    BOOL bFlag : 1;
public:
    SwUndoNumRuleStart( const SwPosition& rPos, BOOL bDelete );
    SwUndoNumRuleStart( const SwPosition& rPos, USHORT nStt );
    virtual void Undo( SwUndoIter& );
    virtual void Redo( SwUndoIter& );
    virtual void Repeat( SwUndoIter& );
};

//--------------------------------------------------------------------
// ---------- Undo fuer DrawObjecte ----------------------------------

class SwSdrUndo : public SwUndo
{
    SdrUndoAction* pSdrUndo;
    SdrMarkList* pMarkList; // MarkList for all selected SdrObjects
public:
    SwSdrUndo( SdrUndoAction* , const SdrMarkList* pMarkList );
    virtual ~SwSdrUndo();
    virtual void Undo( SwUndoIter& );
    virtual void Redo( SwUndoIter& );

    String GetComment() const;
};

class SwUndoDrawGroup : public SwUndo
{
    SwUndoGroupObjImpl* pObjArr;
    USHORT nSize;
    BOOL bDelFmt;

public:
    SwUndoDrawGroup( USHORT nCnt );
    virtual ~SwUndoDrawGroup();
    virtual void Undo( SwUndoIter& );
    virtual void Redo( SwUndoIter& );

    void AddObj( USHORT nPos, SwDrawFrmFmt*, SdrObject* );
    void SetGroupFmt( SwDrawFrmFmt* );
};

// --> OD 2006-11-01 #130889#
// Action "ungroup drawing object" is now splitted into three parts - see
// method <SwDoc::UnGroupSelection(..)>:
// - creation for <SwDrawFrmFmt> instances for the group members of the
//   selected group objects
// - intrinsic ungroup of the selected group objects
// - creation of <SwDrawContact> instances for the former group members and
//   connection to the Writer layout.
// Thus, two undo actions (instances of <SwUndo>) are needed:
// - Existing class <SwUndoDrawUnGroup> takes over the part for the formats.
// - New class <SwUndoDrawUnGroupConnectToLayout> takes over the part for
//   contact object.
class SwUndoDrawUnGroup : public SwUndo
{
    SwUndoGroupObjImpl* pObjArr;
    USHORT nSize;
    BOOL bDelFmt;

public:
    SwUndoDrawUnGroup( SdrObjGroup* );
    virtual ~SwUndoDrawUnGroup();
    virtual void Undo( SwUndoIter& );
    virtual void Redo( SwUndoIter& );

    void AddObj( USHORT nPos, SwDrawFrmFmt* );
};

// --> OD 2006-11-01 #130889#
class SwUndoDrawUnGroupConnectToLayout : public SwUndo
{
    private:
        std::vector< std::pair< SwDrawFrmFmt*, SdrObject* > > aDrawFmtsAndObjs;

    public:
        SwUndoDrawUnGroupConnectToLayout();
        virtual ~SwUndoDrawUnGroupConnectToLayout();
        virtual void Undo( SwUndoIter& );
        virtual void Redo( SwUndoIter& );

        void AddFmtAndObj( SwDrawFrmFmt* pDrawFrmFmt,
                           SdrObject* pDrawObject );
};
// <--


class SwUndoDrawDelete : public SwUndo
{
    SwUndoGroupObjImpl* pObjArr;
    SdrMarkList* pMarkLst;  // MarkList for all selected SdrObjects
    USHORT nSize;
    BOOL bDelFmt;

public:
    SwUndoDrawDelete( USHORT nCnt );
    virtual ~SwUndoDrawDelete();
    virtual void Undo( SwUndoIter& );
    virtual void Redo( SwUndoIter& );

    void AddObj( USHORT nPos, SwDrawFrmFmt*, const SdrMark& );
};

//--------------------------------------------------------------------

class SwUndoReRead : public SwUndo
{
    Graphic *pGrf;
    String *pNm, *pFltr;
    ULONG nPos;
    USHORT nMirr;

    void SaveGraphicData( const SwGrfNode& );
    void SetAndSave( SwUndoIter& );

public:
    SwUndoReRead( const SwPaM& rPam, const SwGrfNode& pGrfNd );
    virtual ~SwUndoReRead();

    virtual void Undo( SwUndoIter& );
    virtual void Redo( SwUndoIter& );
};

//--------------------------------------------------------------------

class SwUndoInsertLabel : public SwUndo
{
    union {
        struct {
            // fuer NoTxtFrms
            SwUndoInsLayFmt* pUndoFly;
            SwUndoFmtAttr* pUndoAttr;
        } OBJECT;
        struct {
            // fuer Tabelle/TextRahmen
            SwUndoDelete* pUndoInsNd;
            ULONG nNode;
        } NODE;
    };

    String sText;
    // --> PB 2005-01-06 #i39983# - the separator will be drawed with a character style
    String sSeparator;
    // <--
    String sNumberSeparator;
    String sCharacterStyle;
    // OD 2004-04-15 #i26791# - re-store of drawing object position no longer needed
    USHORT nFldId;
    SwLabelType eType;
    BYTE nLayerId;              // fuer Zeichen-Objekte
    BOOL bBefore        :1;
    BOOL bUndoKeep      :1;
    BOOL bCpyBrd        :1;

public:
    SwUndoInsertLabel( const SwLabelType eTyp, const String &rText,
    // --> PB 2005-01-06 #i39983# - the separator will be drawed with a character style
                        const String& rSeparator,
    // <--
                        const String& rNumberSeparator, //#i61007# order of captions
                        const BOOL bBefore, const USHORT nId,
                        const String& rCharacterStyle,
                        const BOOL bCpyBrd );
    virtual ~SwUndoInsertLabel();

    virtual void Undo( SwUndoIter& );
    virtual void Redo( SwUndoIter& );
    virtual void Repeat( SwUndoIter& );

    // #111827#
    /**
       Returns the rewriter of this undo object.

       The rewriter contains this rule:

           $1 -> '<text of inserted label>'

       <text of inserted label> is shortened to nUndoStringLength
       characters.

       @return the rewriter of this undo object
     */
    virtual SwRewriter GetRewriter() const;

    void SetNodePos( ULONG nNd )
        { if( LTYPE_OBJECT != eType ) NODE.nNode = nNd; }

    void SetUndoKeep()  { bUndoKeep = TRUE; }
    void SetFlys( SwFrmFmt& rOldFly, SfxItemSet& rChgSet, SwFrmFmt& rNewFly );
    void SetDrawObj( BYTE nLayerId );
};

//--------------------------------------------------------------------

class SwUndoChangeFootNote : public SwUndo, private SwUndRng
{
    const ::std::auto_ptr<SwHistory> m_pHistory;
    const String m_Text;
    const USHORT m_nNumber;
    const bool m_bEndNote;

public:
    SwUndoChangeFootNote( const SwPaM& rRange, const String& rTxt,
                          USHORT nNum, bool bIsEndNote );
    virtual ~SwUndoChangeFootNote();

    virtual void Undo( SwUndoIter& );
    virtual void Redo( SwUndoIter& );
    virtual void Repeat( SwUndoIter& );

    SwHistory& GetHistory() { return *m_pHistory; }
};

class SwUndoFootNoteInfo : public SwUndo
{
    ::std::auto_ptr<SwFtnInfo> m_pFootNoteInfo;

public:
    SwUndoFootNoteInfo( const SwFtnInfo &rInfo );
    virtual ~SwUndoFootNoteInfo();

    virtual void Undo( SwUndoIter& );
    virtual void Redo( SwUndoIter& );
};

class SwUndoEndNoteInfo : public SwUndo
{
    ::std::auto_ptr<SwEndNoteInfo> m_pEndNoteInfo;

public:
    SwUndoEndNoteInfo( const SwEndNoteInfo &rInfo );
    virtual ~SwUndoEndNoteInfo();

    virtual void Undo( SwUndoIter& );
    virtual void Redo( SwUndoIter& );
};


//--------------------------------------------------------------------

struct _UndoTransliterate_Data;
class SwUndoTransliterate : public SwUndo, public SwUndRng
{
    std::vector< _UndoTransliterate_Data * >    aChanges;
    sal_uInt32 nType;

public:
    SwUndoTransliterate( const SwPaM& rPam,
                            const utl::TransliterationWrapper& rTrans );
    virtual ~SwUndoTransliterate();

    virtual void Undo( SwUndoIter& rUndoIter );
    virtual void Redo( SwUndoIter& rUndoIter );
    virtual void Repeat( SwUndoIter& rUndoIter );

    void AddChanges( SwTxtNode& rTNd, xub_StrLen nStart, xub_StrLen nLen,
                     ::com::sun::star::uno::Sequence <sal_Int32>& rOffsets );
    BOOL HasData() const { return aChanges.size() > 0; }
};

//--------------------------------------------------------------------

class SwUndoRedline : public SwUndo, public SwUndRng
{
protected:
    SwRedlineData* pRedlData;
    SwRedlineSaveDatas* pRedlSaveData;
    SwUndoId nUserId;
    BOOL bHiddenRedlines;

    virtual void _Undo( SwUndoIter& );
    virtual void _Redo( SwUndoIter& );

public:
    SwUndoRedline( SwUndoId nUserId, const SwPaM& rRange );
    virtual ~SwUndoRedline();
    virtual void Undo( SwUndoIter& );
    virtual void Redo( SwUndoIter& );

    SwUndoId GetUserId() const { return nUserId; }
    USHORT GetRedlSaveCount() const
        { return pRedlSaveData ? pRedlSaveData->Count() : 0; }
};

class SwUndoRedlineDelete : public SwUndoRedline
{
    BOOL bCanGroup : 1;
    BOOL bIsDelim : 1;
    BOOL bIsBackspace : 1;

    virtual void _Undo( SwUndoIter& );
    virtual void _Redo( SwUndoIter& );

public:
    SwUndoRedlineDelete( const SwPaM& rRange, SwUndoId nUserId = UNDO_EMPTY );

    BOOL CanGrouping( const SwUndoRedlineDelete& rPrev );
};

class SwUndoRedlineSort : public SwUndoRedline
{
    SwSortOptions* pOpt;
    ULONG nSaveEndNode, nOffset;
    xub_StrLen nSaveEndCntnt;

    virtual void _Undo( SwUndoIter& );
    virtual void _Redo( SwUndoIter& );

public:
    SwUndoRedlineSort( const SwPaM& rRange, const SwSortOptions& rOpt );
    virtual ~SwUndoRedlineSort();
    virtual void Repeat( SwUndoIter& );

    void SetSaveRange( const SwPaM& rRange );
    void SetOffset( const SwNodeIndex& rIdx );
};

class SwUndoAcceptRedline : public SwUndoRedline
{
    virtual void _Redo( SwUndoIter& );
public:
    SwUndoAcceptRedline( const SwPaM& rRange );
    virtual void Repeat( SwUndoIter& );
};

class SwUndoRejectRedline : public SwUndoRedline
{
    virtual void _Redo( SwUndoIter& );
public:
    SwUndoRejectRedline( const SwPaM& rRange );
    virtual void Repeat( SwUndoIter& );
};

//--------------------------------------------------------------------

class SwUndoCompDoc : public SwUndo, public SwUndRng
{
    SwRedlineData* pRedlData;
    SwUndoDelete* pUnDel, *pUnDel2;
    SwRedlineSaveDatas* pRedlSaveData;
    BOOL bInsert;
public:
    SwUndoCompDoc( const SwPaM& rRg, BOOL bIns );
    SwUndoCompDoc( const SwRedline& rRedl );

    virtual ~SwUndoCompDoc();
    virtual void Undo( SwUndoIter& );
    virtual void Redo( SwUndoIter& );
};


//--------------------------------------------------------------------

// Object der als Iterator durch die Undo-Liste laeuft, bis die
// letze oder die angegebene Klammerung/Id erreicht ist.

class SwUndoIter
{
    friend class SwDoc;     // um im SwDoc::Undo  bWeiter zu stezen
    friend void SwUndoEnd::Undo( SwUndoIter& );
    friend void SwUndoStart::Undo( SwUndoIter& );
    friend void SwUndoEnd::Redo( SwUndoIter& );
    friend void SwUndoStart::Redo( SwUndoIter& );
    friend void SwUndoEnd::Repeat( SwUndoIter& );
    friend void SwUndoStart::Repeat( SwUndoIter& );
    friend void SwUndoReplace::Undo( SwUndoIter& );
    friend void SwUndoReplace::Redo( SwUndoIter& );

    SwUndoId nUndoId;
    USHORT nEndCnt;
    BOOL bWeiter : 1;
    BOOL bUpdateAttr : 1;   // Setze das GCAttr an der CursorShell

public:
    SwPaM * pAktPam;        // Member fuer das Undo
    SwUndo* pLastUndoObj;   // fuers Redo, das vorherige UndoObj.
    SwFrmFmt* pSelFmt;      // ggfs. das Format Rahmen/Object-Selektionen
    SdrMarkList* pMarkList; // MarkList for all selected SdrObjects

    SwUndoIter( SwPaM * pPam, SwUndoId nId = UNDO_EMPTY );

    BOOL IsNextUndo() const             { return bWeiter; }
    BOOL IsUpdateAttr() const           { return bUpdateAttr; }
    void SetUpdateAttr( BOOL bNew )     { bUpdateAttr = bNew; }

    inline SwDoc& GetDoc() const;
    SwUndoId GetId() const  { return nUndoId; }
    SwUndoId GetLastUndoId() const
        { return  pLastUndoObj ? pLastUndoObj->GetId() : UNDO_EMPTY ; }
    void ClearSelections()  { pSelFmt = 0; pMarkList = 0; }
};


// -> #111827#
const int nUndoStringLength = 20;

/**
   Shortens a string to a maximum length.

   @param rStr      the string to be shortened
   @param nLength   the maximum length for rStr
   @param rFillStr  string to replace cut out characters with

   If rStr has less than nLength characters it will be returned unaltered.

   If rStr has more than nLength characters the following algorithm
   generates the shortened string:

       frontLength = (nLength - length(rFillStr)) / 2
       rearLength = nLength - length(rFillStr) - frontLength
       shortenedString = concat(<first frontLength characters of rStr,
                                rFillStr,
                                <last rearLength characters of rStr>)

   Preconditions:
      - nLength - length(rFillStr) >= 2

   @return the shortened string
 */
String ShortenString(const String & rStr, xub_StrLen nLength, const String & rFillStr);
// <- #111827#

// #16487#
/**
   Denotes special characters in a string.

   The rStr is split into parts containing special characters and
   parts not containing special characters. In a part containing
   special characters all characters are equal. These parts are
   maximal.

   @param rStr     the string to denote in

   The resulting string is generated by concatenating the found
   parts. The parts without special characters are surrounded by
   "'". The parts containing special characters are denoted as "n x",
   where n is the length of the part and x is the representation of
   the special character (i. e. "tab(s)").

   @return the denoted string
*/
String DenoteSpecialCharacters(const String & rStr);

=======
>>>>>>> other
#endif
