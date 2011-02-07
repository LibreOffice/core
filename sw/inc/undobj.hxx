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

    static BOOL CanRedlineGroup( SwRedlineSaveDatas& rCurr,
                                const SwRedlineSaveDatas& rCheck,
                                sal_Bool bCurrIsEnd );

    // #111827#
    /**
       Returns the rewriter for this object.

       @return the rewriter for this object
    */
    virtual SwRewriter GetRewriter() const;

    // return type is USHORT because this overrides SfxUndoAction::GetId()
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
    virtual BOOL CanRepeat(SfxRepeatTarget &) const;

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

// base class for insertion of Document, Glossaries and Copy
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

#endif
