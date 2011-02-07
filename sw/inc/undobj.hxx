/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include <memory>

#include <boost/shared_ptr.hpp>

#include <com/sun/star/uno/Sequence.h>

#include <tools/mempool.hxx>
#ifndef _SVSTDARR_HXX
#define _SVSTDARR_USHORTS
#define _SVSTDARR_ULONGS
#define _SVSTDARR_BOOLS
#define _SVSTDARR_BYTES
#define _SVSTDARR_USHORTSSORT
#include <svl/svstdarr.hxx>
#endif
#include <svl/itemset.hxx>

#include <svx/svdundo.hxx>

#include <numrule.hxx>
#include <itabenum.hxx>
#include <format.hxx>
#include <SwRewriter.hxx>
#include <swundo.hxx>
#include <IMark.hxx>
#include <IDocumentContentOperations.hxx>

class SwUndoIter;
class SwHistory;
class SwIndex;
class SwPaM;
struct SwPosition;
class SwDoc;
class SwFmt;
class SwFmtColl;
class SwTxtFmtColl;
class SwTxtNode;
class SwTableNode;
class SwTable;
class SwTableBox;
struct SwSortOptions;
class SwFrmFmt;
class SwHistoryBookmark;
class SwSectionData;
class SwSectionFmt;
class SwTOXBase;
class SvxTabStopItem;
class SwDDEFieldType;
class Graphic;
class SwGrfNode;
class SwUndos;
class SwFtnInfo;
class SwEndNoteInfo;
class SwNodeIndex;
class SwNodeRange;
class SwFmtAnchor;
struct SwUndoGroupObjImpl;
class SdrMark;
class SdrMarkList;
class SdrObject;
class SdrObjGroup;
class SdrUndoAction;
class SwDrawFrmFmt;
class _SaveTable;
class SwTableAutoFmt;
class SwSelBoxes;
class SwTableSortBoxes;
class SwUndoSaveSections;
class SwUndoMoves;
class SwStartNode;
class _SaveFlyArr;
class SwTblToTxtSaves;
class SwRedlineData;
class SwRedlineSaveData;
class SwRedline;
struct SwSaveRowSpan;

namespace sfx2 {
    class MetadatableUndo;
}

namespace utl {
    class TransliterationWrapper;
}

SW_DLLPUBLIC extern const String UNDO_ARG1;
SW_DLLPUBLIC extern const String UNDO_ARG2;
SW_DLLPUBLIC extern const String UNDO_ARG3;

typedef SwRedlineSaveData* SwRedlineSaveDataPtr;
SV_DECL_PTRARR_DEL( SwRedlineSaveDatas, SwRedlineSaveDataPtr, 8, 8 )

class SwUndo
{
    SwUndoId nId;
    USHORT nOrigRedlineMode;

protected:
    bool bCacheComment;
    mutable String * pComment;

    void RemoveIdxFromSection( SwDoc&, ULONG nSttIdx, ULONG* pEndIdx = 0 );
    void RemoveIdxFromRange( SwPaM& rPam, BOOL bMoveNext );
    void RemoveIdxRel( ULONG, const SwPosition& );

    void SetId( SwUndoId nNew ) { nId = nNew; }

    static BOOL CanRedlineGroup( SwRedlineSaveDatas& rCurr,
                                const SwRedlineSaveDatas& rCheck,
                                BOOL bCurrIsEnd );

    /**
       Returns the rewriter for this object.

       @return the rewriter for this object
    */
    virtual SwRewriter GetRewriter() const;
public:
    SwUndo( SwUndoId nI );
    virtual ~SwUndo();

    SwUndoId GetId() const { return nId; }
    virtual SwUndoId GetEffectiveId() const;
    virtual void Undo( SwUndoIter& ) = 0;
    virtual void Redo( SwUndoIter& ) = 0;
    virtual void Repeat( SwUndoIter& );

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
    USHORT GetRedlineMode() const { return nOrigRedlineMode; }
    void SetRedlineMode( USHORT eMode ) { nOrigRedlineMode = eMode; }

    bool IsDelBox() const;

        // sicher und setze die RedlineDaten
    static BOOL FillSaveData( const SwPaM& rRange, SwRedlineSaveDatas& rSData,
                            BOOL bDelRange = TRUE, BOOL bCopyNext = TRUE );
    static BOOL FillSaveDataForFmt( const SwPaM& , SwRedlineSaveDatas& );
    static void SetSaveData( SwDoc& rDoc, const SwRedlineSaveDatas& rSData );
    static BOOL HasHiddenRedlines( const SwRedlineSaveDatas& rSData );
};

typedef USHORT DelCntntType;
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
                        ULONG* pEndNdIdx = 0, xub_StrLen * pEndCntIdx = 0 );
    void MoveFromUndoNds( SwDoc& rDoc, ULONG nNodeIdx, xub_StrLen nCntntIdx,
                          SwPosition& rInsPos,
                          ULONG* pEndNdIdx = 0, xub_StrLen * pEndCntIdx = 0 );

    // diese beiden Methoden bewegen den SPoint vom Pam zurueck/vor. Damit
    // kann fuer ein Undo/Redo ein Bereich aufgespannt werden. (Der
    // SPoint liegt dann vor dem manipuliertem Bereich !!)
    // Das Flag gibt an, ob noch vorm SPoint Inhalt steht.
    BOOL MovePtBackward( SwPaM& rPam );
    void MovePtForward( SwPaM& rPam, BOOL bMvBkwrd );

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
    ULONG nMvLen;           // Index ins UndoNodes-Array
    ULONG nStartPos;

protected:
    SwNodeIndex* GetMvSttIdx() const { return pMvStt; }
    ULONG GetMvNodeCnt() const { return nMvLen; }

public:
    SwUndoSaveSection();
    ~SwUndoSaveSection();

    void SaveSection( SwDoc* pDoc, const SwNodeIndex& rSttIdx );
    void SaveSection( SwDoc* pDoc, const SwNodeRange& rRange );
    void RestoreSection( SwDoc* pDoc, SwNodeIndex* pIdx, USHORT nSectType );
    void RestoreSection( SwDoc* pDoc, const SwNodeIndex& rInsPos );

    const SwHistory* GetHistory() const { return pHistory; }
          SwHistory* GetHistory()       { return pHistory; }
};

// Diese Klasse speichert den Pam als USHORT's und kann diese wieder zu
// einem PaM zusammensetzen
class SwUndRng
{
public:
    ULONG nSttNode, nEndNode;
    xub_StrLen nSttCntnt, nEndCntnt;

    SwUndRng();
    SwUndRng( const SwPaM& );

    void SetValues( const SwPaM& rPam );
    void SetPaM( SwPaM&, BOOL bCorrToCntnt = FALSE ) const;
    void SetPaM( SwUndoIter&, BOOL bCorrToCntnt = FALSE ) const;
};

class SwUndoStart: public SwUndo
{
    // Um innerhalb von Undo zuerkennen, wann ein Start vorliegt, gibt
    // GetId() immer die UNDO_START zurueck. Die UserId kann ueber
    // GetUserId() erfragt werden.
    SwUndoId nUserId;
    // fuer die "Verpointerung" von Start- und End-Undos
    USHORT nEndOffset;

    SwRewriter mRewriter;

public:
    SwUndoStart( SwUndoId nId );
    virtual void Undo( SwUndoIter& );
    virtual void Redo( SwUndoIter& );
    virtual void Repeat( SwUndoIter& );

    virtual String GetComment() const;
    void SetRewriter(const SwRewriter & rRewriter);
    virtual SwRewriter GetRewriter() const;

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

    virtual String GetComment() const;
    void SetRewriter(const SwRewriter & rRewriter);
    virtual SwRewriter GetRewriter() const;

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

    // for PageDesc/PageBreak Attribute of a table
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
    BOOL bInsChar : 1;      // no more OverWrite; use Insert
    BOOL bGroup : 1;        // TRUE: ist schon eine Gruppe; wird in
                            //       CanGrouping() ausgwertet !!
public:
    SwUndoOverwrite( SwDoc*, SwPosition&, sal_Unicode cIns );
    virtual ~SwUndoOverwrite();
    virtual void Undo( SwUndoIter& );
    virtual void Redo( SwUndoIter& );
    virtual void Repeat( SwUndoIter& );

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
    // Return value indicates, if anchor attribute is restored.
    // Notes: - If anchor attribute is restored, all other existing attributes
    //          are also restored.
    //        - Anchor attribute isn't restored successfully, if it contains
    //          an invalid anchor position and all other existing attributes
    //          aren't restored.
    //          This situation occurs for undo of styles.
    bool RestoreFlyAnchor( SwUndoIter& rIter );

    void Init();

public:
    // register at the Format and save old attributes
    SwUndoFmtAttr( const SfxItemSet& rOldSet,
                   SwFmt& rFmt,
                   bool bSaveDrawPt = true );
    SwUndoFmtAttr( const SfxPoolItem& rItem,
                   SwFmt& rFmt,
                   bool bSaveDrawPt = true );
    virtual ~SwUndoFmtAttr();
    virtual void Undo( SwUndoIter& );

    virtual void Redo( SwUndoIter& );

    virtual void Repeat( SwUndoIter& );
    virtual SwRewriter GetRewriter() const;

    void PutAttr( const SfxPoolItem& rItem );
    SwFmt* GetFmt( SwDoc& rDoc );   // checks if it is still in the Doc!
};

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

public:
    SwUndoFmtAttrHelper( SwFmt& rFmt, bool bSaveDrawPt = true );

    virtual void Modify( SfxPoolItem*, SfxPoolItem* );

    SwUndoFmtAttr* GetUndo() const  { return m_pUndo.get(); }
    // release the undo object (so it is not deleted here), and return it
    SwUndoFmtAttr* ReleaseUndo()    { return m_pUndo.release(); }
};

class SwUndoFmtColl : public SwUndo, private SwUndRng
{
    String aFmtName;
    SwHistory* pHistory;
    SwFmtColl* pFmtColl;
    // boolean, which indicates that the attributes are reseted at the nodes
    // before the format has been applied.
    const bool mbReset;
    // boolean, which indicates that the list attributes had been reseted at
    // the nodes before the format has been applied.
    const bool mbResetListAttrs;
public:

    SwUndoFmtColl( const SwPaM&, SwFmtColl*,
                   const bool bReset,
                   const bool bResetListAttrs );
    virtual ~SwUndoFmtColl();
    virtual void Undo( SwUndoIter& );
    virtual void Redo( SwUndoIter& );
    virtual void Repeat( SwUndoIter& );

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
class SwUndoInserts : public SwUndo, public SwUndRng, private SwUndoSaveCntnt
{
    SwTxtFmtColl *pTxtFmtColl, *pLastNdColl;
    SvPtrarr* pFrmFmts;
    SwUndos* pFlyUndos;
    SwRedlineData* pRedlData;
    BOOL bSttWasTxtNd;
protected:
    ULONG nNdDiff;
    SwPosition *pPos;                   // Inhalt fuers Redo
    USHORT nSetPos;                     // Start in der History-Liste

    SwUndoInserts( SwUndoId nUndoId, const SwPaM& );
public:
    virtual ~SwUndoInserts();

    virtual void Undo( SwUndoIter& );
    virtual void Redo( SwUndoIter& );
    virtual void Repeat( SwUndoIter& );
    // setze den Destination-Bereich nach dem Einlesen.
    void SetInsertRange( const SwPaM&, BOOL bScanFlys = TRUE,
                        BOOL bSttWasTxtNd = TRUE );
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

class SwUndoInsTbl : public SwUndo
{
    String sTblNm;
    SwInsertTableOptions aInsTblOpts;
    SwDDEFieldType* pDDEFldType;
    SvUShorts* pColWidth;
    SwRedlineData*  pRedlData;
    SwTableAutoFmt* pAutoFmt;
    ULONG nSttNode;
    USHORT nRows, nCols;
    USHORT nAdjust;

public:
    SwUndoInsTbl( const SwPosition&, USHORT nCols, USHORT nRows,
                    USHORT eAdjust, const SwInsertTableOptions& rInsTblOpts,
                    const SwTableAutoFmt* pTAFmt, const SvUShorts* pColArr,
                  const String & rName);
    virtual ~SwUndoInsTbl();
    virtual void Undo( SwUndoIter& );
    virtual void Redo( SwUndoIter& );
    virtual void Repeat( SwUndoIter& );

    virtual SwRewriter GetRewriter() const;

};

class SwUndoTxtToTbl : public SwUndo, public SwUndRng
{
    String sTblNm;
    SwInsertTableOptions aInsTblOpts;
    SvULongs* pDelBoxes;
    SwTableAutoFmt* pAutoFmt;
    SwHistory* pHistory;
    sal_Unicode cTrenner;
    USHORT nAdjust;
    BOOL bSplitEnd : 1;

public:
    SwUndoTxtToTbl( const SwPaM&, const SwInsertTableOptions&, sal_Unicode , USHORT,
                    const SwTableAutoFmt* pAFmt );
    virtual ~SwUndoTxtToTbl();

    virtual void Undo( SwUndoIter& );
    virtual void Redo( SwUndoIter& );
    virtual void Repeat( SwUndoIter& );

    SwHistory& GetHistory();        // wird ggfs. angelegt
    void AddFillBox( const SwTableBox& rBox );
};

class SwUndoTblToTxt : public SwUndo
{
    String sTblNm;
    SwDDEFieldType* pDDEFldType;
    _SaveTable* pTblSave;
    SwTblToTxtSaves* pBoxSaves;
    SwHistory* pHistory;
    ULONG nSttNd, nEndNd;
    USHORT nAdjust;
    sal_Unicode cTrenner;
    USHORT nHdlnRpt;
    BOOL bCheckNumFmt : 1;

public:
    SwUndoTblToTxt( const SwTable& rTbl, sal_Unicode cCh );
    virtual ~SwUndoTblToTxt();
    virtual void Undo( SwUndoIter& );
    virtual void Redo( SwUndoIter& );
    virtual void Repeat( SwUndoIter& );

    void SetRange( const SwNodeRange& );
    void AddBoxPos( SwDoc& rDoc, ULONG nNdIdx, ULONG nEndIdx,
                    xub_StrLen nCntntIdx = STRING_MAXLEN);
};

class SwUndoAttrTbl : public SwUndo
{
    ULONG nSttNode;
    _SaveTable* pSaveTbl;
    BOOL bClearTabCol : 1;
public:
    SwUndoAttrTbl( const SwTableNode& rTblNd, BOOL bClearTabCols = FALSE );
    virtual ~SwUndoAttrTbl();
    virtual void Undo( SwUndoIter& );
    virtual void Redo( SwUndoIter& );
};

class SwUndoTblAutoFmt : public SwUndo
{
    ULONG nSttNode;
    _SaveTable* pSaveTbl;
    SwUndos* pUndos;
    BOOL bSaveCntntAttr;

    void UndoRedo( BOOL bUndo, SwUndoIter& rUndoIter );

public:
    SwUndoTblAutoFmt( const SwTableNode& rTblNd, const SwTableAutoFmt& );
    virtual ~SwUndoTblAutoFmt();
    virtual void Undo( SwUndoIter& );
    virtual void Redo( SwUndoIter& );
    void SaveBoxCntnt( const SwTableBox& rBox );
};

class SwUndoTblNdsChg : public SwUndo
{
    _SaveTable* pSaveTbl;
    SvULongs aBoxes;

    union {
        SvULongs* pNewSttNds;
        SwUndoSaveSections* pDelSects;
    } Ptrs;
    SvBools aMvBoxes;       // fuers SplitRow (aufgeteilte Nodes einer Box)
    long nMin, nMax;        // for redo of delete column
    ULONG nSttNode, nCurrBox;
    USHORT nCount, nRelDiff, nAbsDiff, nSetColType;
    BOOL bFlag;
    BOOL bSameHeight;                   // only used for SplitRow
public:
    SwUndoTblNdsChg( SwUndoId UndoId,
                    const SwSelBoxes& rBoxes,
                    const SwTableNode& rTblNd,
                    long nMn, long nMx,
                    USHORT nCnt, BOOL bFlg, BOOL bSameHeight );

    // fuer SetColWidth
    SwUndoTblNdsChg( SwUndoId UndoId, const SwSelBoxes& rBoxes,
                    const SwTableNode& rTblNd );

    virtual ~SwUndoTblNdsChg();
    virtual void Undo( SwUndoIter& );
    virtual void Redo( SwUndoIter& );

    void SaveNewBoxes( const SwTableNode& rTblNd, const SwTableSortBoxes& rOld );
    void SaveNewBoxes( const SwTableNode& rTblNd, const SwTableSortBoxes& rOld,
                       const SwSelBoxes& rBoxes, const SvULongs& rNodeCnts );
    void SaveSection( SwStartNode* pSttNd );
    void ReNewBoxes( const SwSelBoxes& rBoxes );

    void SetColWidthParam( ULONG nBoxIdx, USHORT nMode, USHORT nType,
                            SwTwips nAbsDif, SwTwips nRelDif )
    {
        nCurrBox = nBoxIdx;
        nCount = nMode;
        nSetColType = nType;
        nAbsDiff = (USHORT)nAbsDif;
        nRelDiff = (USHORT)nRelDif;
    }

};

class SwUndoTblMerge : public SwUndo, private SwUndRng
{
    ULONG nTblNode;
    _SaveTable* pSaveTbl;
    SvULongs aBoxes, aNewSttNds;
    SwUndoMoves* pMoves;
    SwHistory* pHistory;

public:
    SwUndoTblMerge( const SwPaM& rTblSel );
    virtual ~SwUndoTblMerge();
    virtual void Undo( SwUndoIter& );
    virtual void Redo( SwUndoIter& );

    void MoveBoxCntnt( SwDoc* pDoc, SwNodeRange& rRg, SwNodeIndex& rPos );

    void SetSelBoxes( const SwSelBoxes& rBoxes );

    void AddNewBox( ULONG nSttNdIdx )
        { aNewSttNds.Insert( nSttNdIdx, aNewSttNds.Count() ); }

    void SaveCollection( const SwTableBox& rBox );

};

class SwUndoTblNumFmt : public SwUndo
{
    SfxItemSet *pBoxSet;
    SwHistory* pHistory;
    String aStr, aNewFml;

    ULONG nFmtIdx, nNewFmtIdx;
    double fNum, fNewNum;
    ULONG nNode;
    ULONG nNdPos;

    BOOL bNewFmt : 1;
    BOOL bNewFml : 1;
    BOOL bNewValue : 1;

public:
    SwUndoTblNumFmt( const SwTableBox& rBox, const SfxItemSet* pNewSet = 0 );
    virtual ~SwUndoTblNumFmt();
    virtual void Undo( SwUndoIter& );
    virtual void Redo( SwUndoIter& );

    void SetNumFmt( ULONG nNewNumFmtIdx, const double& rNewNumber )
            { nFmtIdx = nNewNumFmtIdx; fNum = rNewNumber; }
    void SetBox( const SwTableBox& rBox );
};

class _UndoTblCpyTbl_Entries;

class SwUndoTblCpyTbl : public SwUndo
{
    _UndoTblCpyTbl_Entries* pArr;
    SwUndoTblNdsChg* pInsRowUndo;

    //b6341295: When redlining is active, PrepareRedline has to create the redlining attributes
    //for the new and the old table cell content
    SwUndo* PrepareRedline( SwDoc* pDoc, const SwTableBox& rBox, const SwPosition& rPos,
        bool& rJoin, bool bRedo );
public:
    SwUndoTblCpyTbl();
    virtual ~SwUndoTblCpyTbl();
    virtual void Undo( SwUndoIter& );
    virtual void Redo( SwUndoIter& );

    void AddBoxBefore( const SwTableBox& rBox, BOOL bDelCntnt );
    void AddBoxAfter( const SwTableBox& rBox, const SwNodeIndex& rIdx, BOOL bDelCntnt );

    BOOL IsEmpty() const;
    BOOL InsertRow( SwTable& rTbl, const SwSelBoxes& rBoxes, USHORT nCnt );
};

class SwUndoCpyTbl : public SwUndo
{
    SwUndoDelete* pDel;
    ULONG nTblNode;
public:
    SwUndoCpyTbl();
    virtual ~SwUndoCpyTbl();
    virtual void Undo( SwUndoIter& );
    virtual void Redo( SwUndoIter& );

    void SetTableSttIdx( ULONG nIdx )           { nTblNode = nIdx; }
};

class SwUndoSplitTbl : public SwUndo
{
    ULONG nTblNode, nOffset;
    SwSaveRowSpan* mpSaveRowSpan; // stores the row span values at the splitting row
    _SaveTable* pSavTbl;
    SwHistory* pHistory;
    USHORT nMode, nFmlEnd;
    BOOL bCalcNewSize;
public:
    SwUndoSplitTbl( const SwTableNode& rTblNd, SwSaveRowSpan* pRowSp, USHORT nMode, BOOL bCalcNewSize );
    virtual ~SwUndoSplitTbl();
    virtual void Undo( SwUndoIter& );
    virtual void Redo( SwUndoIter& );
    virtual void Repeat( SwUndoIter& );

    void SetTblNodeOffset( ULONG nIdx )     { nOffset = nIdx - nTblNode; }
    SwHistory* GetHistory()                 { return pHistory; }
    void SaveFormula( SwHistory& rHistory );
};

class SwUndoMergeTbl : public SwUndo
{
    String aName;
    ULONG nTblNode;
    _SaveTable* pSavTbl, *pSavHdl;
    SwHistory* pHistory;
    USHORT nMode;
    BOOL bWithPrev;
public:
    SwUndoMergeTbl( const SwTableNode& rTblNd, const SwTableNode& rDelTblNd,
                    BOOL bWithPrev, USHORT nMode );
    virtual ~SwUndoMergeTbl();
    virtual void Undo( SwUndoIter& );
    virtual void Redo( SwUndoIter& );
    virtual void Repeat( SwUndoIter& );

    void SaveFormula( SwHistory& rHistory );
};

class SwUndoBookmark : public SwUndo
{
    const ::std::auto_ptr<SwHistoryBookmark> m_pHistoryBookmark;

protected:
    SwUndoBookmark( SwUndoId nUndoId, const ::sw::mark::IMark& );

    void SetInDoc( SwDoc* );
    void ResetInDoc( SwDoc* );

public:
    virtual ~SwUndoBookmark();

    /**
       Returns the rewriter for this undo object.

       The rewriter contains the following rule:

           $1 -> <name of bookmark>

       <name of bookmark> is the name of the bookmark whose
       insertion/deletion is recorded by this undo object.

       @return the rewriter for this undo object
     */
    virtual SwRewriter GetRewriter() const;
};

class SwUndoInsBookmark : public SwUndoBookmark
{
public:
    SwUndoInsBookmark( const ::sw::mark::IMark& );
    virtual void Undo( SwUndoIter& );
    virtual void Redo( SwUndoIter& );
};

/*--------------------------------------------------------------------
    Beschreibung: Undo auf Sorting
 --------------------------------------------------------------------*/

struct SwSortUndoElement
{
    union {
        struct {
            ULONG nKenn;
            ULONG nSource, nTarget;
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
    SwSortUndoElement( ULONG nS, ULONG nT )
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
    SwSortOptions*  pSortOpt;       // die Optionen mit den Sortier-Kriterien
    SwSortList      aSortList;
    SwUndoAttrTbl*  pUndoTblAttr;
    SwRedlineData*  pRedlData;
    ULONG           nTblNd;

    void RemoveIdx( SwPaM& rPam );
public:
    SwUndoSort( const SwPaM&, const SwSortOptions& );
    SwUndoSort( ULONG nStt, ULONG nEnd, const SwTableNode&,
                const SwSortOptions&, BOOL bSaveTable );
    virtual ~SwUndoSort();

    virtual void Undo( SwUndoIter& );
    virtual void Redo( SwUndoIter& );
    virtual void Repeat( SwUndoIter& );

    void Insert( const String& rOrgPos, const String& rNewPos );
    void Insert( ULONG nOrgPos, ULONG nNewPos );

};

//--------------------------------------------------------------------

class SwUndoFlyBase : public SwUndo, private SwUndoSaveSection
{
protected:
    SwFrmFmt* pFrmFmt;                  // das gespeicherte FlyFormat
    ULONG nNdPgPos;
    xub_StrLen nCntPos;                 // Seite/am Absatz/im Absatz
    USHORT nRndId;
    BOOL bDelFmt;                       // loesche das gespeicherte Format

    void InsFly( SwUndoIter&, BOOL bShowSel = TRUE );
    void DelFly( SwDoc* );

    SwUndoFlyBase( SwFrmFmt* pFormat, SwUndoId nUndoId );

    SwNodeIndex* GetMvSttIdx() const { return SwUndoSaveSection::GetMvSttIdx(); }
    ULONG GetMvNodeCnt() const { return SwUndoSaveSection::GetMvNodeCnt(); }

public:
    virtual ~SwUndoFlyBase();

    virtual void Undo( SwUndoIter& ) = 0;
    virtual void Redo( SwUndoIter& ) = 0;

};

class SwUndoInsLayFmt : public SwUndoFlyBase
{
    ULONG mnCrsrSaveIndexPara;           // Cursor position
    xub_StrLen mnCrsrSaveIndexPos;            // for undo
public:
    SwUndoInsLayFmt( SwFrmFmt* pFormat, ULONG nNodeIdx, xub_StrLen nCntIdx );
    ~SwUndoInsLayFmt();

    virtual void Undo( SwUndoIter& );
    virtual void Redo( SwUndoIter& );
    virtual void Repeat( SwUndoIter& );

    String GetComment() const;

};

class SwUndoDelLayFmt : public SwUndoFlyBase
{
    BOOL bShowSelFrm;
public:
    SwUndoDelLayFmt( SwFrmFmt* pFormat );

    virtual void Undo( SwUndoIter& );
    virtual void Redo( SwUndoIter& );
    void Redo();        // Schnittstelle fuers Rollback

    void ChgShowSel( BOOL bNew ) { bShowSelFrm = bNew; }

    virtual SwRewriter GetRewriter() const;

};

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
    void Modify( SfxPoolItem*, SfxPoolItem* );
    void GetAnchor( SwFmtAnchor& rAnhor, ULONG nNode, xub_StrLen nCntnt );

public:
    SwUndoSetFlyFmt( SwFrmFmt& rFlyFmt, SwFrmFmt& rNewFrmFmt );
    virtual ~SwUndoSetFlyFmt();

    virtual void Undo( SwUndoIter& );
    virtual void Redo( SwUndoIter& );

    virtual SwRewriter GetRewriter() const;
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

//------------ Undo of Insert-/Delete-Sections ----------------------

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
// ---------- Undo for Numbering ----------------------------------

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
    struct NodeLevel
    {
        ULONG index;
        int level;
        inline NodeLevel(ULONG idx, int lvl) : index(idx), level(lvl) {};
    };
    std::vector<NodeLevel> aNodes;
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
// ---------- Undo for DrawObjecte ----------------------------------

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
            // for NoTxtFrms
            SwUndoInsLayFmt* pUndoFly;
            SwUndoFmtAttr* pUndoAttr;
        } OBJECT;
        struct {
            // for Tables/TextBoxes
            SwUndoDelete* pUndoInsNd;
            ULONG nNode;
        } NODE;
    };

    String sText;
    String sSeparator;
    String sNumberSeparator;
    String sCharacterStyle;
    USHORT nFldId;
    SwLabelType eType;
    BYTE nLayerId;              // fuer Zeichen-Objekte
    BOOL bBefore        :1;
    BOOL bUndoKeep      :1;
    BOOL bCpyBrd        :1;

public:
    SwUndoInsertLabel( const SwLabelType eTyp, const String &rText,
                        const String& rSeparator,
                        const String& rNumberSeparator,
                        const BOOL bBefore, const USHORT nId,
                        const String& rCharacterStyle,
                        const BOOL bCpyBrd );
    virtual ~SwUndoInsertLabel();

    virtual void Undo( SwUndoIter& );
    virtual void Redo( SwUndoIter& );
    virtual void Repeat( SwUndoIter& );

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

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
