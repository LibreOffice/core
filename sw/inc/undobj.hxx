/*************************************************************************
 *
 *  $RCSfile: undobj.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: jp $ $Date: 2001-03-02 14:35:07 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _UNDOBJ_HXX
#define _UNDOBJ_HXX

#ifndef _SVMEMPOOL_HXX //autogen
#include <tools/mempool.hxx>
#endif
#ifndef _SVSTDARR_HXX
#define _SVSTDARR_USHORTS
#define _SVSTDARR_ULONGS
#define _SVSTDARR_BOOLS
#define _SVSTDARR_BYTES
#define _SVSTDARR_USHORTSSORT
#include <svtools/svstdarr.hxx>
#endif
#ifndef _SFXITEMSET_HXX //autogen
#include <svtools/itemset.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_H_
#include <com/sun/star/uno/Sequence.h>
#endif
#ifndef _NUMRULE_HXX
#include <numrule.hxx>
#endif
#ifndef _REDLENUM_HXX
#include <redlenum.hxx>
#endif

class SwUndoIter;
class SwHistory;
class SwIndex;
class SwPaM;
struct SwPosition;
class SwDoc;
class SwNodes;
class SwFmt;
class SwFmtColl;
class SwTxtFmtColl;
class SwTxtNode;
class SwpHints;
class SwBookmark;
class SwTableNode;
class SwTable;
class SwTableBox;
struct SwSortOptions;
class SwFrmFmt;
class SwHstryBookmark;
class SwSection;
class SwSectionFmt;
class SvxTabStopItem;
class SwNumSection;
class SwDDEFieldType;
class Graphic;
class SwGrfNode;
class SwUndos;
class SwUndoRange;
class SwFtnInfo;
class SwEndNoteInfo;
class SwNodeIndex;
class SwNodeRange;
class SwFmtAnchor;
struct SwUndoGroupObjImpl;
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
class SwSectionNode;

namespace utl {
    class TransliterationWrapper;
};

#ifndef PRODUCT
class Writer;
class SwUndo;
#define OUT_UNDOBJ( name )  \
    friend Writer& OutUndo_ ## name( Writer&, const SwUndo& );
#else
#define OUT_UNDOBJ( name )
#endif


typedef SwRedlineSaveData* SwRedlineSaveDataPtr;
SV_DECL_PTRARR_DEL( SwRedlineSaveDatas, SwRedlineSaveDataPtr, 8, 8 )

class SwUndo
{
    USHORT nId;
    USHORT nOrigRedlineMode;
protected:
    void RemoveIdxFromSection( SwDoc&, ULONG nSttIdx, ULONG* pEndIdx = 0 );
    void RemoveIdxFromRange( SwPaM& rPam, BOOL bMoveNext );
    void RemoveIdxRel( ULONG, const SwPosition& );

    void SetId( USHORT nNew ) { nId = nNew; }

    static BOOL CanRedlineGroup( SwRedlineSaveDatas& rCurr,
                                const SwRedlineSaveDatas& rCheck,
                                BOOL bCurrIsEnd );
public:
    SwUndo( USHORT nI ) { nId = nI; nOrigRedlineMode = REDLINE_NONE; }
    virtual ~SwUndo();

    USHORT GetId() const { return nId; }
    virtual void Undo( SwUndoIter& ) = 0;
    virtual void Redo( SwUndoIter& ) = 0;
    virtual void Repeat( SwUndoIter& );

        // das UndoObject merkt sich, welcher Mode eingeschaltet war.
        // In Undo/Redo/Repeat wird dann immer auf diesen zurueck geschaltet
    USHORT GetRedlineMode() const { return nOrigRedlineMode; }
    void SetRedlineMode( USHORT eMode ) { nOrigRedlineMode = eMode; }

        // sicher und setze die RedlineDaten
    static BOOL FillSaveData( const SwPaM& rRange, SwRedlineSaveDatas& rSData,
                            BOOL bDelRange = TRUE, BOOL bCopyNext = TRUE );
    static BOOL FillSaveDataForFmt( const SwPaM& , SwRedlineSaveDatas& );
    static void SetSaveData( SwDoc& rDoc, const SwRedlineSaveDatas& rSData );
    static BOOL HasHiddenRedlines( const SwRedlineSaveDatas& rSData );
};


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
    enum DelCntntType{  DELCNT_FTN = 0x01,
                        DELCNT_FLY = 0x02,
                        DELCNT_TOC = 0x04,
                        DELCNT_BKM = 0x08,
                        DELCNT_ALL = 0x0F,
                        DELCNT_CHKNOCNTNT = 0x80    // nur den NodeIndex beachten
                    };
    void DelCntntIndex( const SwPosition& pMark, const SwPosition& pPoint,
                        DelCntntType nDelCntntType = DELCNT_ALL );

public:
    SwUndoSaveCntnt();
    ~SwUndoSaveCntnt();
    OUT_UNDOBJ( SaveCntnt )
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
    OUT_UNDOBJ( SaveSection )
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
    USHORT nUserId;
    // fuer die "Verpointerung" von Start- und End-Undos
    USHORT nEndOffset;
public:
    SwUndoStart( USHORT nId );
    virtual void Undo( SwUndoIter& );
    virtual void Redo( SwUndoIter& );
    virtual void Repeat( SwUndoIter& );
    USHORT GetUserId() const { return nUserId; }
    // Setzen vom End-Undo-Offset geschieht im Doc::EndUndo
    USHORT GetEndOffset() const { return nEndOffset; }
    void SetEndOffset( USHORT n ) { nEndOffset = n; }
    OUT_UNDOBJ( Start )
};

class SwUndoEnd: public SwUndo
{
    // Um innerhalb von Undo zuerkennen, wann ein Ende vorliegt, gibt
    // GetId() immer die UNDO_END zurueck. Die UserId kann ueber
    // GetUserId() erfragt werden.
    USHORT nUserId;
    // fuer die "Verpointerung" von Start- und End-Undos
    USHORT nSttOffset;
public:
    SwUndoEnd( USHORT nId );
    virtual void Undo( SwUndoIter& );
    virtual void Redo( SwUndoIter& );
    virtual void Repeat( SwUndoIter& );
    USHORT GetUserId() const { return nUserId; }
    // Setzen vom Start-Undo-Offset geschieht im Doc::EndUndo
    USHORT GetSttOffset() const { return nSttOffset; }
    void SetSttOffset( USHORT n ) { nSttOffset = n; }
    OUT_UNDOBJ( End )
};

class SwUndoInsert: public SwUndo, private SwUndoSaveCntnt
{
    SwPosition *pPos;                   // Inhalt fuers Redo
    String* pTxt;
    SwRedlineData* pRedlData;
    ULONG nNode;
    xub_StrLen nCntnt, nLen;
    BOOL bIsWordDelim : 1;
    BOOL bIsAppend : 1;

    friend class SwDoc;     // eigentlich nur SwDoc::Insert( String )
    BOOL CanGrouping( sal_Unicode cIns );
    BOOL CanGrouping( const SwPosition& rPos );

public:
    SwUndoInsert( const SwNodeIndex& rNode, xub_StrLen nCntnt, xub_StrLen nLen,
                  BOOL bWDelim = TRUE );
    SwUndoInsert( const SwNodeIndex& rNode );
    virtual ~SwUndoInsert();
    virtual void Undo( SwUndoIter& );
    virtual void Redo( SwUndoIter& );
    virtual void Repeat( SwUndoIter& );

    BOOL CanGrouping( const SwPosition&, sal_Unicode cIns );
    OUT_UNDOBJ( Insert )

    DECL_FIXEDMEMPOOL_NEWDEL(SwUndoInsert)
};


class SwUndoDelete: public SwUndo, private SwUndRng, private SwUndoSaveCntnt
{
    SwNodeIndex* pMvStt;            // Position der Nodes im UndoNodes-Array
    String *pSttStr, *pEndStr;
    SwRedlineData* pRedlData;
    SwRedlineSaveDatas* pRedlSaveData;

    ULONG nNode;
    ULONG nNdDiff;              // Differenz von Nodes vor-nach Delete
    ULONG nSectDiff;            // Diff. von Nodes vor/nach Move mit SectionNodes
    USHORT nSetPos;

    BOOL bGroup : 1;    // TRUE: ist schon eine Gruppe; wird in CanGrouping() ausgwertet !!
    BOOL bBackSp : 1;   // TRUE: wenn Gruppierung und der Inhalt davor geloescht wird
    BOOL bJoinNext: 1;  // TRUE: wenn der Bereich von Oben nach unten geht
    BOOL bSectNdFnd : 1;    // TRUE: Sonderbehandlung fuer SectionNodes
    BOOL bMvAroundSectNd :1;// TRUE: Sonderbehandlung fuer SectionNodes
    BOOL bTblDelLastNd : 1; // TRUE: TextNode hinter der Tabelle einf./loeschen
    BOOL bDelFullPara : 1;  // TRUE: gesamte Nodes wurden geloescht
    BOOL bResetPgDesc : 1;  // TRUE: am nachfolgenden Node das PgDsc zuruecksetzen
    BOOL bResetPgBrk : 1;   // TRUE: am nachfolgenden Node das PgBreak zuruecksetzen

    BOOL SaveCntnt( const SwPosition* pStt, const SwPosition* pEnd,
                    SwTxtNode* pSttTxtNd, SwTxtNode* pEndTxtNd );
public:
    SwUndoDelete( SwPaM&, BOOL bFullPara = FALSE );
    virtual ~SwUndoDelete();
    virtual void Undo( SwUndoIter& );
    virtual void Redo( SwUndoIter& );
    virtual void Repeat( SwUndoIter& );
    BOOL CanGrouping( SwDoc*, const SwPaM& );

    void SetTblDelLastNd()      { bTblDelLastNd = TRUE; }

    // fuer die PageDesc/PageBreak Attribute einer Tabelle
    void SetPgBrkFlags( BOOL bPageBreak, BOOL bPageDesc )
        { bResetPgDesc = bPageDesc; bResetPgBrk = bPageBreak; }

    OUT_UNDOBJ( Delete )

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
    BOOL CanGrouping( SwDoc*, SwPosition&, sal_Unicode cIns );
    OUT_UNDOBJ( Overwrite )
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
    OUT_UNDOBJ( SplitNode )
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
    void AddTblMrgFlyHstry( SwHistory& rHstr );

    OUT_UNDOBJ( Move )
};


class SwUndoAttr : public SwUndo, private SwUndRng
{
    SfxItemSet aSet;                // Attribute fuers Redo
    SwHistory* pHistory;            // History fuers Undo
    SwRedlineData* pRedlData;       // Redlining
    SwRedlineSaveDatas* pRedlSaveData;
    ULONG nNdIdx;                   // fuers Redlining - Offset
    USHORT nInsFlags;               // Einfuege Flags

    void RemoveIdx( SwDoc& rDoc );
public:
    SwUndoAttr( const SwPaM&, const SfxItemSet&, USHORT nFlags = 0  );
    SwUndoAttr( const SwPaM&, const SfxPoolItem&, USHORT nFlags = 0 );
    virtual ~SwUndoAttr();
    virtual void Undo( SwUndoIter& );
    virtual void Redo( SwUndoIter& );
    virtual void Repeat( SwUndoIter& );
    SwHistory* GetHistory() { return pHistory; }
    void SaveRedlineData( const SwPaM& rPam, BOOL bInsCntnt );

    OUT_UNDOBJ( InsAttr )
};

class SwUndoRstAttr : public SwUndo, private SwUndRng
{
    SwHistory* pHistory;
    SvUShortsSort aIds;
    USHORT nFmtId;                  // Format-Id fuer das Redo
public:
    SwUndoRstAttr( const SwPaM&, USHORT nFmtId );
    SwUndoRstAttr( const SwDoc&, const SwPosition&, USHORT nWhichId );
    virtual ~SwUndoRstAttr();
    virtual void Undo( SwUndoIter& );
    virtual void Redo( SwUndoIter& );
    virtual void Repeat( SwUndoIter& );
    void SetAttrs( const SvUShortsSort& rArr );
    SwHistory* GetHistory() { return pHistory; }
    OUT_UNDOBJ( ResetAttr )
};


class SwUndoFmtAttr : public SwUndo
{
    friend class SwUndoDefaultAttr;
    SwFmt* pFmt;
    SfxItemSet* pOldSet;            // die alten Attribute
    ULONG nNode;
    USHORT nFmtWhich;
    BOOL bSaveDrawPt;

    int IsFmtInDoc( SwDoc* );       // ist das Attribut-Format noch im Doc ?
    void SaveFlyAnchor( BOOL bSaveDrawPt = FALSE );
    void RestoreFlyAnchor( SwUndoIter& rIter );
    void Init();

public:
    // meldet sich im Format an und sichert sich die alten Attribute
    SwUndoFmtAttr( const SfxItemSet& rOldSet, SwFmt& rFmt,
                    BOOL bSaveDrawPt = TRUE );
    SwUndoFmtAttr( const SfxPoolItem& rItem, SwFmt& rFmt,
                    BOOL bSaveDrawPt = TRUE );
    virtual ~SwUndoFmtAttr();
    virtual void Undo( SwUndoIter& );
    virtual void Redo( SwUndoIter& );
    virtual void Repeat( SwUndoIter& );
    OUT_UNDOBJ( InsFmtAttr )

    void PutAttr( const SfxPoolItem& rItem );
    SwFmt* GetFmt( SwDoc& rDoc );       // prueft, ob es noch im Doc ist!
};

class SwUndoDontExpandFmt : public SwUndo
{
    ULONG nNode;
    xub_StrLen nCntnt;
public:
    SwUndoDontExpandFmt( const SwPosition& rPos );
    virtual void Undo( SwUndoIter& );
    virtual void Redo( SwUndoIter& );
    virtual void Repeat( SwUndoIter& );
};

// Hilfs-Klasse, um die geaenderten Sets zu "empfangen"
struct _UndoFmtAttr : public SwClient
{
    SwUndoFmtAttr* pUndo;
    BOOL bSaveDrawPt;

    _UndoFmtAttr( SwFmt& rFmt, BOOL bSaveDrawPt = TRUE );
    virtual void Modify( SfxPoolItem*, SfxPoolItem* );
};


class SwUndoFmtColl : public SwUndo, private SwUndRng
{
    SwHistory* pHistory;
    SwFmtColl* pFmtColl;
public:
    SwUndoFmtColl( const SwPaM&, SwFmtColl* );
    virtual ~SwUndoFmtColl();
    virtual void Undo( SwUndoIter& );
    virtual void Redo( SwUndoIter& );
    virtual void Repeat( SwUndoIter& );
    SwHistory* GetHistory() { return pHistory; }
    OUT_UNDOBJ( SetFmtColl )
};


class SwUndoMoveLeftMargin : public SwUndo, private SwUndRng
{
    SwHistory* pHistory;
    BOOL bModulus;
public:
    SwUndoMoveLeftMargin( const SwPaM&, BOOL bRight, BOOL bModulus );
    virtual ~SwUndoMoveLeftMargin();
    virtual void Undo( SwUndoIter& );
    virtual void Redo( SwUndoIter& );
    virtual void Repeat( SwUndoIter& );
    SwHistory* GetHistory() { return pHistory; }
    OUT_UNDOBJ( MoveLeftMargin )
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

    SwUndoInserts( USHORT nUndoId, const SwPaM& );
public:
    virtual ~SwUndoInserts();

    virtual void Undo( SwUndoIter& );
    virtual void Redo( SwUndoIter& );
    virtual void Repeat( SwUndoIter& );
    // setze den Destination-Bereich nach dem Einlesen.
    void SetInsertRange( const SwPaM&, BOOL bScanFlys = TRUE,
                        BOOL bSttWasTxtNd = TRUE );
    OUT_UNDOBJ( Inserts )
};

class SwUndoInsDoc : public SwUndoInserts
{
public:
    SwUndoInsDoc( const SwPaM& );
    OUT_UNDOBJ( InsDoc )
};

class SwUndoCpyDoc : public SwUndoInserts
{
public:
    SwUndoCpyDoc( const SwPaM& );
    OUT_UNDOBJ( Copy )
};

class SwUndoInsTbl : public SwUndo
{
    String sTblNm;
    SwDDEFieldType* pDDEFldType;
    SvUShorts* pColWidth;
    SwRedlineData*  pRedlData;
    SwTableAutoFmt* pAutoFmt;
    ULONG nSttNode;
    USHORT nRows, nCols;
    USHORT nAdjust;
    USHORT nInsTblFlags;

public:
    SwUndoInsTbl( const SwPosition&, USHORT nCols, USHORT nRows,
                    USHORT eAdjust, USHORT nInsert,
                    const SwTableAutoFmt* pTAFmt, const SvUShorts* pColArr );
    virtual ~SwUndoInsTbl();
    virtual void Undo( SwUndoIter& );
    virtual void Redo( SwUndoIter& );
    virtual void Repeat( SwUndoIter& );
    OUT_UNDOBJ( InsTable )
};

class SwUndoTxtToTbl : public SwUndo, public SwUndRng
{
    String sTblNm;
    SvULongs* pDelBoxes;
    SwTableAutoFmt* pAutoFmt;
    SwHistory* pHistory;
    sal_Unicode cTrenner;
    USHORT nAdjust;
    USHORT nInsTblFlags;
    BOOL bSplitEnd : 1;

public:
    SwUndoTxtToTbl( const SwPaM&, sal_Unicode , USHORT, USHORT nInsert,
                    const SwTableAutoFmt* pAFmt );
    virtual ~SwUndoTxtToTbl();

    virtual void Undo( SwUndoIter& );
    virtual void Redo( SwUndoIter& );
    virtual void Repeat( SwUndoIter& );

    SwHistory& GetHistory();        // wird ggfs. angelegt
    void AddFillBox( const SwTableBox& rBox );
    OUT_UNDOBJ( TextToTable )
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
    BOOL bCheckNumFmt : 1;
    BOOL bHdlnRpt : 1;

public:
    SwUndoTblToTxt( const SwTable& rTbl, sal_Unicode cCh );
    virtual ~SwUndoTblToTxt();
    virtual void Undo( SwUndoIter& );
    virtual void Redo( SwUndoIter& );
    virtual void Repeat( SwUndoIter& );

    void SetRange( const SwNodeRange& );
    void AddBoxPos( SwDoc& rDoc, ULONG nNdIdx,
                    xub_StrLen nCntntIdx = STRING_MAXLEN);
    OUT_UNDOBJ( TableToText )
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
    OUT_UNDOBJ( SetTableAttr )
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
    OUT_UNDOBJ( TableAutoFmt )
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

    ULONG nSttNode, nCurrBox;
    USHORT nCount, nRelDiff, nAbsDiff, nSetColType;
    BOOL bFlag;
public:
    SwUndoTblNdsChg( USHORT UndoId,
                    const SwSelBoxes& rBoxes,
                    const SwTableNode& rTblNd,
                    USHORT nCnt, BOOL bFlg );

    // fuer SetColWidth
    SwUndoTblNdsChg( USHORT UndoId, const SwSelBoxes& rBoxes,
                    const SwTableNode& rTblNd );

    virtual ~SwUndoTblNdsChg();
    virtual void Undo( SwUndoIter& );
    virtual void Redo( SwUndoIter& );

    void SaveNewBoxes( const SwTableNode& rTblNd, const SwTableSortBoxes& rOld );
    void SaveNewBoxes( const SwTableNode& rTblNd, const SwTableSortBoxes& rOld,
                       const SwSelBoxes& rBoxes, const SvULongs& rNodeCnts );
    void SaveSection( SwStartNode* pSttNd );

    void SetColWidthParam( ULONG nBoxIdx, USHORT nMode, USHORT nType,
                            SwTwips nAbsDif, SwTwips nRelDif )
    {
        nCurrBox = nBoxIdx;
        nCount = nMode;
        nSetColType = nType;
        nAbsDiff = (USHORT)nAbsDif;
        nRelDiff = (USHORT)nRelDif;
    }
    OUT_UNDOBJ( TblNodesChg )
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

    void MoveBoxCntnt( SwPaM& rPam, SwPosition& rPos, const _SaveFlyArr& );
    void MoveBoxCntnt( SwDoc* pDoc, SwNodeRange& rRg, SwNodeIndex& rPos );

    void SetSelBoxes( const SwSelBoxes& rBoxes );

    void AddNewBox( ULONG nSttNdIdx )
        { aNewSttNds.Insert( nSttNdIdx, aNewSttNds.Count() ); }

    void SaveCollection( const SwTableBox& rBox );

    OUT_UNDOBJ( TblMerge )
};


class SwUndoTblNumFmt : public SwUndo
{
    SfxItemSet *pBoxSet;
    SwHistory* pHistory;
    String aStr, aNewFml;

    ULONG nFmtIdx, nNewFmtIdx;
    double fNum, fNewNum;
    ULONG nNode;

    BOOL bNewFmt : 1;
    BOOL bNewFml : 1;
    BOOL bNewValue : 1;

public:
    SwUndoTblNumFmt( const SwTableBox& rBox, const SfxItemSet* pNewSet = 0 );
    virtual ~SwUndoTblNumFmt();
    virtual void Undo( SwUndoIter& );
    virtual void Redo( SwUndoIter& );

    void SetNumFmt( ULONG nNewFmtIdx, const double& rNewNumber )
            { nFmtIdx = nNewFmtIdx; fNum = rNewNumber; }
    void SetBox( const SwTableBox& rBox );
    OUT_UNDOBJ( TblNumFmt )
};

class _UndoTblCpyTbl_Entries;

class SwUndoTblCpyTbl : public SwUndo
{
    _UndoTblCpyTbl_Entries* pArr;
    SwUndoTblNdsChg* pInsRowUndo;
public:
    SwUndoTblCpyTbl();
    virtual ~SwUndoTblCpyTbl();
    virtual void Undo( SwUndoIter& );
    virtual void Redo( SwUndoIter& );

    void AddBoxBefore( const SwTableBox& rBox, BOOL bDelCntnt );
    void AddBoxAfter( const SwTableBox& rBox, BOOL bDelCntnt );

    BOOL IsEmpty() const;
    BOOL InsertRow( SwTable& rTbl, const SwSelBoxes& rBoxes, USHORT nCnt );
    OUT_UNDOBJ( TblCpyTbl )
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
    OUT_UNDOBJ( CpyTbl )
};

class SwUndoSplitTbl : public SwUndo
{
    ULONG nTblNode, nOffset;
    _SaveTable* pSavTbl;
    SwHistory* pHistory;
    USHORT nMode, nFmlEnd;
    BOOL bCalcNewSize;
public:
    SwUndoSplitTbl( const SwTableNode& rTblNd, USHORT nMode, BOOL bCalcNewSize );
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

    SwHistory* GetHistory()                 { return pHistory; }
    void SaveFormula( SwHistory& rHistory );
};


class SwUndoBookmark : public SwUndo
{
    SwHstryBookmark* pHBookmark;
protected:
    SwUndoBookmark( USHORT nUndoId, const SwBookmark& );

    void SetInDoc( SwDoc* );
    void ResetInDoc( SwDoc* );

public:
    virtual ~SwUndoBookmark();
};


class SwUndoDelBookmark : public SwUndoBookmark
{
public:
    SwUndoDelBookmark( const SwBookmark& );
    virtual void Undo( SwUndoIter& );
    virtual void Redo( SwUndoIter& );
    OUT_UNDOBJ( DelBookmark )
};

class SwUndoInsBookmark : public SwUndoBookmark
{
public:
    SwUndoInsBookmark( const SwBookmark& );
    virtual void Undo( SwUndoIter& );
    virtual void Redo( SwUndoIter& );
    OUT_UNDOBJ( InsBookmark )
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

    OUT_UNDOBJ( Sort )
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

    SwUndoFlyBase( SwFrmFmt* pFormat, USHORT nUndoId );

    SwNodeIndex* GetMvSttIdx() const { return SwUndoSaveSection::GetMvSttIdx(); }
    ULONG GetMvNodeCnt() const { return SwUndoSaveSection::GetMvNodeCnt(); }

public:
    virtual ~SwUndoFlyBase();

    virtual void Undo( SwUndoIter& ) = 0;
    virtual void Redo( SwUndoIter& ) = 0;
    OUT_UNDOBJ( FlyBase )
};

class SwUndoInsLayFmt : public SwUndoFlyBase
{
public:
    SwUndoInsLayFmt( SwFrmFmt* pFormat );

    virtual void Undo( SwUndoIter& );
    virtual void Redo( SwUndoIter& );
    virtual void Repeat( SwUndoIter& );
    OUT_UNDOBJ( InsLayFmt )
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
    BOOL GetShowSel() const { return bShowSelFrm; }
    OUT_UNDOBJ( DelLayFmt )
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
};

//--------------------------------------------------------------------

class _UnReplaceData;
SV_DECL_PTRARR_DEL( _UnReplaceDatas, _UnReplaceData*, 10, 25 );

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

    void AddEntry( const SwPaM& rPam, const String& rInsert, BOOL bRegExp );
    void SetEntryEnd( const SwPaM& rPam );

    BOOL IsFull() const
        { return ((USHRT_MAX / sizeof( void* )) - 50 ) < aArr.Count(); }

    OUT_UNDOBJ( Replace )
};


//--------------------------------------------------------------------


class SwUndoTblHeadline : public SwUndo
{
    ULONG nTblNd;
    BOOL bOldHeadline;
public:
    SwUndoTblHeadline( const SwTable&, BOOL bOldHdl );
    virtual void Undo( SwUndoIter& );
    virtual void Redo( SwUndoIter& );
    virtual void Repeat( SwUndoIter& );
    OUT_UNDOBJ( SwUndoTblHeadline )
};


//------------ Undo von Insert-/Delete-Sections ----------------------

class SwUndoInsSection : public SwUndo, private SwUndRng
{
    SwHistory* pHistory;
    SwSection* pSection;
    SwRedlineData* pRedlData;
    SfxItemSet* pAttr;
    ULONG nSectNodePos;
    BOOL bSplitAtStt : 1;
    BOOL bSplitAtEnd : 1;
    BOOL bUpdateFtn : 1;

    void Join( SwDoc& rDoc, ULONG nNode );

public:
    SwUndoInsSection( const SwPaM&, const SwSection&,
                        const SfxItemSet* pSet );
    virtual ~SwUndoInsSection();
    virtual void Undo( SwUndoIter& );
    virtual void Redo( SwUndoIter& );
    virtual void Repeat( SwUndoIter& );
    OUT_UNDOBJ( SwUndoInsSection )

    void SetSectNdPos( ULONG nPos )         { nSectNodePos = nPos; }
    void SaveSplitNode( SwTxtNode* pTxtNd, BOOL bAtStt );
    void SetUpdtFtnFlag( BOOL bFlag )       { bUpdateFtn = bFlag; }
};

class SwUndoDelSection : public SwUndo
{
    ULONG nSttNd, nEndNd;
    SwSection* pSection;
    SfxItemSet* pAttr;
public:
    SwUndoDelSection( const SwSectionFmt& );
    virtual ~SwUndoDelSection();
    virtual void Undo( SwUndoIter& );
    virtual void Redo( SwUndoIter& );
    OUT_UNDOBJ( SwUndoDelSection )
};

class SwUndoChgSection : public SwUndo
{
    ULONG nSttNd;
    SwSection* pSection;
    SfxItemSet* pAttr;
    BOOL bOnlyAttrChgd;
public:
    SwUndoChgSection( const SwSectionFmt&, BOOL bOnlyAttrChgd );
    virtual ~SwUndoChgSection();
    virtual void Undo( SwUndoIter& );
    virtual void Redo( SwUndoIter& );
    OUT_UNDOBJ( SwUndoChgSection )
};

class SwUndoChgSectPsswd : public SwUndo
{
    ::com::sun::star::uno::Sequence <sal_Int8> aPasswd;
    ULONG nSectNd;
public:
    SwUndoChgSectPsswd(
                const ::com::sun::star::uno::Sequence <sal_Int8> & rOld,
                const SwSectionNode* pSectNd = 0 );
    virtual void Undo( SwUndoIter& );
    virtual void Redo( SwUndoIter& );
    OUT_UNDOBJ( SwUndoChgSectPsswd )
};

//------------ Undo von verschieben/stufen von Gliederung ----------------

class SwUndoOutlineLeftRight : public SwUndo, private SwUndRng
{
    short nOffset;
public:
    SwUndoOutlineLeftRight( const SwPaM& rPam, short nOffset );
    virtual void Undo( SwUndoIter& );
    virtual void Redo( SwUndoIter& );
    virtual void Repeat( SwUndoIter& );
    OUT_UNDOBJ( SwUndoOutlineLeftRight )
};

//--------------------------------------------------------------------

class SwUndoDefaultAttr : public SwUndo
{
    SfxItemSet* pOldSet;            // die alten Attribute
    SvxTabStopItem* pTabStop;
public:
    // meldet sich im Format an und sichert sich die alten Attribute
    SwUndoDefaultAttr( const SfxItemSet& rOldSet );
    virtual ~SwUndoDefaultAttr();
    virtual void Undo( SwUndoIter& );
    virtual void Redo( SwUndoIter& );
    OUT_UNDOBJ( DefaultAttr )
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
    SwUndoInsNum( const SwNumRule& rOldRule, const SwNumRule& rNewRule );
    SwUndoInsNum( const SwPosition& rPos, const SwNumRule& rRule,
                            const String& rReplaceRule );
    virtual ~SwUndoInsNum();
    virtual void Undo( SwUndoIter& );
    virtual void Redo( SwUndoIter& );
    virtual void Repeat( SwUndoIter& );

    SwHistory* GetHistory();        // wird ggfs. neu angelegt!
    void SetSttNum( ULONG nNdIdx ) { nSttSet = nNdIdx; }
    void SaveOldNumRule( const SwNumRule& rOld );

    void SetLRSpaceEndPos();

    OUT_UNDOBJ( InsNum )
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

    OUT_UNDOBJ( DelNum )
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
    OUT_UNDOBJ( MoveNum )
};

class SwUndoNumUpDown : public SwUndo, private SwUndRng
{
    short nOffset;
public:
    SwUndoNumUpDown( const SwPaM& rPam, short nOffset );
    virtual void Undo( SwUndoIter& );
    virtual void Redo( SwUndoIter& );
    virtual void Repeat( SwUndoIter& );
    OUT_UNDOBJ( NumUpDown )
};

class SwUndoNumOrNoNum : public SwUndo
{
    ULONG nIdx;
    BOOL bDel : 1;
    BOOL bOutline : 1;
public:
    SwUndoNumOrNoNum( const SwNodeIndex& rIdx, BOOL bDelete, BOOL bOutline );
    virtual void Undo( SwUndoIter& );
    virtual void Redo( SwUndoIter& );
    virtual void Repeat( SwUndoIter& );
    OUT_UNDOBJ( NumOrNoNum )
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
    OUT_UNDOBJ( NumRuleStart )
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

class SwUndoDrawDelete : public SwUndo
{
    SwUndoGroupObjImpl* pObjArr;
    USHORT nSize;
    BOOL bDelFmt;

public:
    SwUndoDrawDelete( USHORT nCnt );
    virtual ~SwUndoDrawDelete();
    virtual void Undo( SwUndoIter& );
    virtual void Redo( SwUndoIter& );

    void AddObj( USHORT nPos, SwDrawFrmFmt*, SdrObject* );
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
    Point aPos;                 // fuer Zeichen-Objekte
    USHORT nFldId;
    SwLabelType eType;
    BYTE nLayerId;              // fuer Zeichen-Objekte
    BOOL bBefore        :1;
    BOOL bUndoKeep      :1;
    BOOL bCpyBrd        :1;

public:
    SwUndoInsertLabel( const SwLabelType eTyp, const String &rText,
                        const BOOL bBefore, const USHORT nId,
                        const BOOL bCpyBrd );
    virtual ~SwUndoInsertLabel();

    virtual void Undo( SwUndoIter& );
    virtual void Redo( SwUndoIter& );
    virtual void Repeat( SwUndoIter& );

    void SetNodePos( ULONG nNd )
        { if( LTYPE_OBJECT != eType ) NODE.nNode = nNd; }

    void SetUndoKeep()  { bUndoKeep = TRUE; }
    void SetFlys( SwFrmFmt& rOldFly, SfxItemSet& rChgSet, SwFrmFmt& rNewFly );
    void SetDrawObj( const Point& rPos, BYTE nLayerId );
};

//--------------------------------------------------------------------

class SwUndoChgFtn : public SwUndo, private SwUndRng
{
    SwHistory* pHistory;
    String sTxt;
    USHORT nNo;
    BOOL bEndNote;
public:
    SwUndoChgFtn( const SwPaM& rRange, const String& rTxt,
                    USHORT nNum, BOOL bIsEndNote );
    virtual ~SwUndoChgFtn();

    virtual void Undo( SwUndoIter& );
    virtual void Redo( SwUndoIter& );
    virtual void Repeat( SwUndoIter& );

    SwHistory* GetHistory() { return pHistory; }
};

class SwUndoFtnInfo : public SwUndo
{
    SwFtnInfo *pFtnInfo;

public:
    SwUndoFtnInfo( const SwFtnInfo &rInfo );
    virtual ~SwUndoFtnInfo();

    virtual void Undo( SwUndoIter& );
    virtual void Redo( SwUndoIter& );
};

class SwUndoEndNoteInfo : public SwUndo
{
    SwEndNoteInfo *pEndNoteInfo;

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
    _UndoTransliterate_Data *pData, *pLastData;
    sal_uInt32 nType;

public:
    SwUndoTransliterate( const SwPaM& rPam,
                            const utl::TransliterationWrapper& rTrans );
    virtual ~SwUndoTransliterate();

    virtual void Undo( SwUndoIter& rUndoIter );
    virtual void Redo( SwUndoIter& rUndoIter );
    virtual void Repeat( SwUndoIter& rUndoIter );

    void AddChanges( const SwTxtNode& rTNd, xub_StrLen nStart, xub_StrLen nLen,
                     ::com::sun::star::uno::Sequence <long>& rOffsets );
    BOOL HasData() const {return 0 != pData; }
};

//--------------------------------------------------------------------

class SwUndoRedline : public SwUndo, public SwUndRng
{
protected:
    SwRedlineData* pRedlData;
    SwRedlineSaveDatas* pRedlSaveData;
    USHORT nUserId;
    BOOL bHiddenRedlines;

    virtual void _Undo( SwUndoIter& );
    virtual void _Redo( SwUndoIter& );

public:
    SwUndoRedline( USHORT nUserId, const SwPaM& rRange );
    virtual ~SwUndoRedline();
    virtual void Undo( SwUndoIter& );
    virtual void Redo( SwUndoIter& );

    USHORT GetUserId() const { return nUserId; }
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
    SwUndoRedlineDelete( const SwPaM& rRange, USHORT nUserId = 0 );

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
    SwUndoDelete* pUnDel;
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

    USHORT nUndoId;
    USHORT nEndCnt;
    BOOL bWeiter : 1;
    BOOL bUpdateAttr : 1;   // Setze das GCAttr an der CursorShell

public:
    SwPaM * pAktPam;        // Member fuer das Undo
    SwUndo* pLastUndoObj;   // fuers Redo, das vorherige UndoObj.
    SwFrmFmt* pSelFmt;      // ggfs. das Format Rahmen/Object-Selektionen
    SdrMarkList* pMarkList; // MarkList for all selected SdrObjects

    SwUndoIter( SwPaM * pPam, USHORT nId = 0 );

    BOOL IsNextUndo() const             { return bWeiter; }
    BOOL IsUpdateAttr() const           { return bUpdateAttr; }
    void SetUpdateAttr( BOOL bNew )     { bUpdateAttr = bNew; }

    inline SwDoc& GetDoc() const;
    USHORT GetId() const    { return nUndoId; }
    USHORT GetLastUndoId() const
        { return  pLastUndoObj ? pLastUndoObj->GetId() : 0 ; }
    void ClearSelections()  { pSelFmt = 0; pMarkList = 0; }
};




#endif
