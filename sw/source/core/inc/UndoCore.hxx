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

#ifndef SW_UNDOCORE_HXX
#define SW_UNDOCORE_HXX

#include <undobj.hxx>

#include <memory>

#include <tools/mempool.hxx>

#include <calbck.hxx>


class SfxItemSet;
class SwFmtColl;
class SwTxtNode;
class SwSectionData;
class SwSectionFmt;
class SwTOXBase;
class SwFmtAnchor;
class SdrMarkList;
class SwUndoDelete;
class SwRedlineSaveData;

namespace sfx2 {
    class MetadatableUndo;
}

namespace sw {
    class UndoManager;
    class IShellCursorSupplier;
}


typedef SwRedlineSaveData* SwRedlineSaveDataPtr;
SV_DECL_PTRARR_DEL( SwRedlineSaveDatas, SwRedlineSaveDataPtr, 8, 8 )


namespace sw {

class SW_DLLPRIVATE UndoRedoContext
    : public SfxUndoContext
{
public:
    UndoRedoContext(SwDoc & rDoc, IShellCursorSupplier & rCursorSupplier)
        : m_rDoc(rDoc)
        , m_rCursorSupplier(rCursorSupplier)
        , m_pSelFmt(0)
        , m_pMarkList(0)
    { }

    SwDoc & GetDoc() const { return m_rDoc; }

    IShellCursorSupplier & GetCursorSupplier() { return m_rCursorSupplier; }

    void SetSelections(SwFrmFmt *const pSelFmt, SdrMarkList *const pMarkList)
    {
        m_pSelFmt = pSelFmt;
        m_pMarkList = pMarkList;
    }
    void GetSelections(SwFrmFmt *& o_rpSelFmt, SdrMarkList *& o_rpMarkList)
    {
        o_rpSelFmt = m_pSelFmt;
        o_rpMarkList = m_pMarkList;
    }

private:
    SwDoc & m_rDoc;
    IShellCursorSupplier & m_rCursorSupplier;
    SwFrmFmt * m_pSelFmt;
    SdrMarkList * m_pMarkList;
};

class SW_DLLPRIVATE RepeatContext
    : public SfxRepeatTarget
{
public:
    RepeatContext(SwDoc & rDoc, SwPaM & rPaM)
        : m_rDoc(rDoc)
        , m_pCurrentPaM(& rPaM)
        , m_bDeleteRepeated(false)
    { }

    SwDoc & GetDoc() const { return m_rDoc; }

    SwPaM & GetRepeatPaM()
    {
        return *m_pCurrentPaM;
    }

private:
    friend class ::sw::UndoManager;
    friend class ::SwUndoDelete;

    SwDoc & m_rDoc;
    SwPaM * m_pCurrentPaM;
    bool m_bDeleteRepeated; /// has a delete action been repeated?
};

} // namespace sw



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

    BOOL CanGrouping( SwDoc*, const SwPaM& );

    void SetTblDelLastNd()      { bTblDelLastNd = TRUE; }

    // fuer die PageDesc/PageBreak Attribute einer Tabelle
    void SetPgBrkFlags( BOOL bPageBreak, BOOL bPageDesc )
        { bResetPgDesc = bPageDesc; bResetPgBrk = bPageBreak; }

    void SetTableName(const String & rName);

    // SwUndoTblCpyTbl needs this information:
    BOOL IsDelFullPara() const { return bDelFullPara; }

    DECL_FIXEDMEMPOOL_NEWDEL(SwUndoDelete)
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

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );
    virtual void RepeatImpl( ::sw::RepeatContext & );

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

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );

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

    void DoSetFmtColl(SwDoc & rDoc, SwPaM & rPaM);

public:
    // --> OD 2008-04-15 #refactorlists#
//    SwUndoFmtColl( const SwPaM&, SwFmtColl* );
    SwUndoFmtColl( const SwPaM&, SwFmtColl*,
                   const bool bReset,
                   const bool bResetListAttrs );
    // <--
    virtual ~SwUndoFmtColl();

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );
    virtual void RepeatImpl( ::sw::RepeatContext & );

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

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );

    virtual SwRewriter GetRewriter() const;
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

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );
    virtual void RepeatImpl( ::sw::RepeatContext & );

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

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );
    virtual void RepeatImpl( ::sw::RepeatContext & );
};


//--------------------------------------------------------------------

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
String
ShortenString(const String & rStr, xub_StrLen nLength, const String & rFillStr);
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


#endif // SW_UNDOCORE_HXX

