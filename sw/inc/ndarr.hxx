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

#ifndef SW_NDARR_HXX
#define SW_NDARR_HXX

#include <vector>

#include <com/sun/star/embed/XEmbeddedObject.hpp>

#include <svl/svarray.hxx>
#include <svtools/embedhlp.hxx>

#include <bparr.hxx>
#include <ndtyp.hxx>

class Graphic;
class GraphicObject;
class String;
class SwAttrSet;
class SfxItemSet;
class SwCntntNode;
class SwDoc;
class SwGrfFmtColl;
class SwGrfNode;
class SwHistory;
class SwNode;
class SwNodeIndex;
class SwNodeRange;
class SwOLENode;
class SwOutlineNodes;
class SwPaM;
class SwSectionData;
class SwSectionFmt;
class SwTOXBase;
class SwSectionNode;
class SwStartNode;
class SwTableBoxFmt;
class SwTableFmt;
class SwTableLine;
class SwTableLineFmt;
class SwTableNode;
class SwTblToTxtSaves;
class SwTxtFmtColl;
class SwTxtNode;
class SwUndoTblToTxt;
class SwUndoTxtToTbl;
struct SwPosition;

// --------------------
// class SwNodes
// --------------------

typedef SwNode * SwNodePtr;
typedef BOOL (*FnForEach_SwNodes)( const SwNodePtr&, void* pArgs );

SV_DECL_PTRARR_SORT( SwOutlineNodes, SwNodePtr, 0, 10 )

class SW_DLLPUBLIC SwNodes: private BigPtrArray
{
    friend class SwDoc;
    friend class SwNode;
    friend class SwNodeIndex;

    SwNodeIndex* pRoot;                 // Liste aller Indizies auf Nodes

    void InsertNode( const SwNodePtr pNode,
                     const SwNodeIndex& rPos );
    void InsertNode( const SwNodePtr pNode,
                     ULONG nPos );

    SwDoc* pMyDoc;                      // in diesem Doc ist das Nodes-Array

    SwNode *pEndOfPostIts, *pEndOfInserts,  // das sind die festen Bereiche
           *pEndOfAutotext, *pEndOfRedlines,
           *pEndOfContent;

    mutable SwOutlineNodes* pOutlineNds;        // Array aller GliederiungsNodes

    BOOL bInNodesDel : 1;               // falls rekursiv aufgerufen wird
                                        // Num/Outline nicht aktualisierem
    BOOL bInDelUpdOutl : 1;             // Flags fuers aktualisieren von Outl.
    BOOL bInDelUpdNum : 1;              // Flags fuers aktualisieren von Outl.

    // fuer dier Verwaltung der Indizies
    void RegisterIndex( SwNodeIndex& rIdx );
    void DeRegisterIndex( SwNodeIndex& rIdx );
    void RemoveNode( ULONG nDelPos, ULONG nLen, BOOL bDel );

    // Aktionen auf die Nodes
    void SectionUpDown( const SwNodeIndex & aStart, const SwNodeIndex & aEnd );
    void DelNodes( const SwNodeIndex& rStart, ULONG nCnt = 1 );

    void ChgNode( SwNodeIndex& rDelPos, ULONG nSize,
                  SwNodeIndex& rInsPos, BOOL bNewFrms );

    void UpdtOutlineIdx( const SwNode& );   // Update ab Node alle OutlineNodes

    void _CopyNodes( const SwNodeRange&, const SwNodeIndex&,
                    BOOL bNewFrms = TRUE, BOOL bTblInsDummyNode = FALSE ) const;
    void _DelDummyNodes( const SwNodeRange& rRg );

protected:
    SwNodes( SwDoc* pDoc );

public:
    ~SwNodes();

    typedef ::std::vector<SwNodeRange> NodeRanges_t;
    typedef ::std::vector<NodeRanges_t> TableRanges_t;

    SwNodePtr operator[]( ULONG n ) const
        { return (SwNodePtr)BigPtrArray::operator[] ( n ); }

    // impl. steht im ndindex.hxx - sollte moeglichst bald auf die
    // neue Schnittstelle angepasst werden
    inline SwNodePtr operator[]( const SwNodeIndex& rIdx ) const;

    ULONG Count() const { return BigPtrArray::Count(); }
    void ForEach( FnForEach_SwNodes fnForEach, void* pArgs = 0 )
    {
        BigPtrArray::ForEach( 0, BigPtrArray::Count(),
                                (FnForEach) fnForEach, pArgs );
    }
    void ForEach( ULONG nStt, ULONG nEnd, FnForEach_SwNodes fnForEach, void* pArgs = 0 )
    {
        BigPtrArray::ForEach( nStt, nEnd, (FnForEach) fnForEach, pArgs );
    }
    void ForEach( const SwNodeIndex& rStart, const SwNodeIndex& rEnd,
                    FnForEach_SwNodes fnForEach, void* pArgs = 0 );

    // eine noch leere Section
    SwNode& GetEndOfPostIts() const     { return *pEndOfPostIts; }
    // Section fuer alle Fussnoten
    SwNode& GetEndOfInserts() const     { return *pEndOfInserts; }
    // Section fuer alle Flys/Header/Footers
    SwNode& GetEndOfAutotext() const    { return *pEndOfAutotext; }
    // Section fuer alle Redlines
    SwNode& GetEndOfRedlines() const    { return *pEndOfRedlines; }
    // das ist der letzte EndNode einer SonderSection. Hier nach kommt nur
    // noch die normale ContentSection (also der BodyText)
    SwNode& GetEndOfExtras() const      { return *pEndOfRedlines; }
    // die normale ContentSection (also der BodyText)
    SwNode& GetEndOfContent() const     { return *pEndOfContent; }

    // ist das NodesArray das normale vom Doc? (nicht das UndoNds, .. )
    // Implementierung steht im doc.hxx (weil man dazu Doc kennen muss) !
    BOOL IsDocNodes() const;

    USHORT GetSectionLevel(const SwNodeIndex &rIndex) const;
    void Delete(const SwNodeIndex &rPos, ULONG nNodes = 1);

    BOOL _MoveNodes( const SwNodeRange&, SwNodes& rNodes, const SwNodeIndex&,
                BOOL bNewFrms = TRUE );
    void MoveRange( SwPaM&, SwPosition&, SwNodes& rNodes );

    void _Copy( const SwNodeRange& rRg, const SwNodeIndex& rInsPos,
                BOOL bNewFrms = TRUE ) const
        {   _CopyNodes( rRg, rInsPos, bNewFrms ); }

    void SectionUp( SwNodeRange *);
    void SectionDown( SwNodeRange *pRange, SwStartNodeType = SwNormalStartNode );

    BOOL CheckNodesRange( const SwNodeIndex& rStt, const SwNodeIndex& rEnd ) const;

    void GoStartOfSection(SwNodeIndex *) const;
    void GoEndOfSection(SwNodeIndex *) const;

    SwCntntNode* GoNext(SwNodeIndex *) const;
    SwCntntNode* GoPrevious(SwNodeIndex *) const;

    //Gehe zum naechsten/vorherigen Cntnt/Tabellennode, fuer den
    //es LayoutFrames gibt, dabei Kopf-/Fusszeilen/Rahmen etc. nicht verlassen
    SwNode* GoNextWithFrm(SwNodeIndex *) const;
    SwNode* GoPreviousWithFrm(SwNodeIndex *) const;

    // zum naechsten Content-Node, der nicht geschuetzt oder versteckt ist
    // (beides auf FALSE ==> GoNext/GoPrevious!!!)
    SwCntntNode* GoNextSection( SwNodeIndex *, int bSkipHidden  = TRUE,
                                           int bSkipProtect = TRUE ) const;
    SwCntntNode* GoPrevSection( SwNodeIndex *, int bSkipHidden  = TRUE,
                                           int bSkipProtect = TRUE ) const;

    // erzeuge ein leere Section von Start und EndNode. Darf nur gerufen
    // werden, wenn eine neue Section mit Inhalt erzeugt werden soll.
    // Zum Beispiel bei den Filtern/Undo/...
    SwStartNode* MakeEmptySection( const SwNodeIndex& rIdx,
                                    SwStartNodeType = SwNormalStartNode );

    // die Impl. von "Make...Node" stehen in den angegebenen .ccx-Files
    SwTxtNode *MakeTxtNode( const SwNodeIndex & rWhere,
                            SwTxtFmtColl *pColl,
                            SwAttrSet* pAutoAttr = 0 ); // in ndtxt.cxx
    SwStartNode* MakeTextSection( const SwNodeIndex & rWhere,
                            SwStartNodeType eSttNdTyp,
                            SwTxtFmtColl *pColl,
                            SwAttrSet* pAutoAttr = 0 );

    SwGrfNode *MakeGrfNode( const SwNodeIndex & rWhere,
                            const String& rGrfName,
                            const String& rFltName,
                            const Graphic* pGraphic,
                            SwGrfFmtColl *pColl,
                            SwAttrSet* pAutoAttr = 0,
                            BOOL bDelayed = FALSE );    // in ndgrf.cxx

    SwGrfNode *MakeGrfNode( const SwNodeIndex & rWhere,
                            const GraphicObject& rGrfObj,
                            SwGrfFmtColl *pColl,
                            SwAttrSet* pAutoAttr = 0 ); // in ndgrf.cxx

    SwOLENode *MakeOLENode( const SwNodeIndex & rWhere,
                            const svt::EmbeddedObjectRef&,
                            SwGrfFmtColl *pColl,
                            SwAttrSet* pAutoAttr = 0 ); // in ndole.cxx
    SwOLENode *MakeOLENode( const SwNodeIndex & rWhere,
                            const String &rName,
                            sal_Int64 nAspect,
                            SwGrfFmtColl *pColl,
                            SwAttrSet* pAutoAttr ); // in ndole.cxx

        // Array aller GliederiungsNodes;
    const SwOutlineNodes& GetOutLineNds() const;

    //void UpdateOutlineNode( const SwNode&, BYTE nOldLevel, BYTE nNewLevel );//#outline level,removed by zhaojianwei
        // alle Nodes Updaten - Rule/Format-Aenderung
    void UpdateOutlineNode(SwNode & rNd);

        // fuege die Nodes fuer die Tabelle ein
        // wenn Lines angegeben, erzeuge die Matrix aus Lines & Boxen
        // ansonsten nur die Anzahl von Boxen.
    /*
       New parameter pAttrSet: If pAttrSet is non-null and contains an
       adjust item it is propagated to the table cells. If there is an
       adjust in pCntntTxtColl or pHeadlineTxtColl this adjust item
       overrides the item in pAttrSet.

     */
    SwTableNode* InsertTable( const SwNodeIndex& rNdIdx,
                        USHORT nBoxes, SwTxtFmtColl* pCntntTxtColl,
                        USHORT nLines = 0, USHORT nRepeat = 0,
                        SwTxtFmtColl* pHeadlineTxtColl = 0,
                        const SwAttrSet * pAttrSet = 0);

        // erzeuge aus dem makierten Bereich eine ausgeglichene Tabelle
    SwTableNode* TextToTable( const SwNodeRange& rRange, sal_Unicode cCh,
                                SwTableFmt* pTblFmt,
                                SwTableLineFmt* pLineFmt,
                                SwTableBoxFmt* pBoxFmt,
                                SwTxtFmtColl* pTxtColl,
                                SwUndoTxtToTbl* pUndo = 0 );

    SwNodeRange * ExpandRangeForTableBox(const SwNodeRange & rRange);

    //create a table from a vector of NodeRanges - API support
    SwTableNode* TextToTable( const TableRanges_t& rTableNodes,
                                SwTableFmt* pTblFmt,
                                SwTableLineFmt* pLineFmt,
                                SwTableBoxFmt* pBoxFmt,
                                SwTxtFmtColl* pTxtColl
                                /*, SwUndo... pUndo*/ );

        // erzeuge aus der Tabelle wieder normalen Text
    BOOL TableToText( const SwNodeRange& rRange, sal_Unicode cCh,
                        SwUndoTblToTxt* = 0 );
        // steht im untbl.cxx und darf nur vom Undoobject gerufen werden
    SwTableNode* UndoTableToText( ULONG nStt, ULONG nEnd,
                        const SwTblToTxtSaves& rSavedData );

        // fuege in der Line, vor der InsPos eine neue Box ein. Das Format
        // wird von der nachfolgenden (vorhergenden;wenn an Ende) genommen
        // in der Line muss schon eine Box vorhanden sein !
    BOOL InsBoxen( SwTableNode*, SwTableLine*, SwTableBoxFmt*,
                        // Formate fuer den TextNode der Box
                        SwTxtFmtColl*, const SfxItemSet* pAutoAttr,
                        USHORT nInsPos, USHORT nCnt = 1 );
        // Splittet eine Tabelle in der Grund-Zeile, in der der Index steht.
        // Alle GrundZeilen dahinter wandern in eine neue Tabelle/-Node.
        // Ist das Flag bCalcNewSize auf TRUE, wird fuer beide neuen Tabellen
        // die neue SSize aus dem Max der Boxen errechnet; vorrausgesetzt,
        // die SSize ist "absolut" gesetzt (LONG_MAX)
        // (Wird zur Zeit nur fuer den RTF-Parser benoetigt)
    SwTableNode* SplitTable( const SwNodeIndex& rPos, BOOL bAfter = TRUE,
                                BOOL bCalcNewSize = FALSE );
        // fuegt 2 Tabellen, die hintereinander stehen, wieder zusammen
    BOOL MergeTable( const SwNodeIndex& rPos, BOOL bWithPrev = TRUE,
                    USHORT nMode = 0, SwHistory* pHistory = 0 );

        // fuege eine neue SwSection ein
    SwSectionNode* InsertTextSection(SwNodeIndex const& rNdIdx,
                                SwSectionFmt& rSectionFmt,
                                SwSectionData const&,
                                SwTOXBase const*const pTOXBase,
                                SwNodeIndex const*const pEnde,
                                bool const bInsAtStart = true,
                                bool const bCreateFrms = true);

        // in welchem Doc steht das Nodes-Array ?
            SwDoc* GetDoc()         { return pMyDoc; }
    const   SwDoc* GetDoc() const   { return pMyDoc; }

        // suche den vorhergehenden [/nachfolgenden ] ContentNode oder
        // TabellenNode mit Frames. Wird kein Ende angeben, dann wird mit
        // dem FrameIndex begonnen; ansonsten, wird mit dem vor rFrmIdx und
        // dem hintern pEnd die Suche gestartet. Sollte kein gueltiger Node
        // gefunden werden, wird 0 returnt. rFrmIdx zeigt auf dem Node mit
        // Frames
    SwNode* FindPrvNxtFrmNode( SwNodeIndex& rFrmIdx,
                                const SwNode* pEnd = 0 ) const;

    SwNode * DocumentSectionStartNode(SwNode * pNode) const;
    SwNode * DocumentSectionEndNode(SwNode * pNode) const;
private:
    // privater Constructor, weil nie kopiert werden darf !!
    SwNodes( const SwNodes & rNodes );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
