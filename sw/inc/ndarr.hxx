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

#include <boost/utility.hpp>

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
typedef sal_Bool (*FnForEach_SwNodes)( const SwNodePtr&, void* pArgs );

SV_DECL_PTRARR_SORT( SwOutlineNodes, SwNodePtr, 0, 10 )

class SW_DLLPUBLIC SwNodes
    : private BigPtrArray
    , private ::boost::noncopyable
{
    friend class SwDoc;
    friend class SwNode;
    friend class SwNodeIndex;

    SwNodeIndex* pRoot;                 // Liste aller Indizies auf Nodes

    // --> OD 2008-05-14 #refactorlists# - removed <bSyncNumberAndNumRule>
    void InsertNode( const SwNodePtr pNode,
                     const SwNodeIndex& rPos );
    void InsertNode( const SwNodePtr pNode,
                     sal_uLong nPos );
    // <--


    SwDoc* pMyDoc;                      // in diesem Doc ist das Nodes-Array

    SwNode *pEndOfPostIts, *pEndOfInserts,  // das sind die festen Bereiche
           *pEndOfAutotext, *pEndOfRedlines,
           *pEndOfContent;

    mutable SwOutlineNodes* pOutlineNds;        // Array aller GliederiungsNodes

    sal_Bool bInNodesDel : 1;               // falls rekursiv aufgerufen wird
                                        // Num/Outline nicht aktualisierem
    sal_Bool bInDelUpdOutl : 1;             // Flags fuers aktualisieren von Outl.
    sal_Bool bInDelUpdNum : 1;              // Flags fuers aktualisieren von Outl.

    // fuer dier Verwaltung der Indizies
    void RegisterIndex( SwNodeIndex& rIdx );
    void DeRegisterIndex( SwNodeIndex& rIdx );
    void RemoveNode( sal_uLong nDelPos, sal_uLong nLen, sal_Bool bDel );

    // Aktionen auf die Nodes
    void SectionUpDown( const SwNodeIndex & aStart, const SwNodeIndex & aEnd );
    void DelNodes( const SwNodeIndex& rStart, sal_uLong nCnt = 1 );

    void ChgNode( SwNodeIndex& rDelPos, sal_uLong nSize,
                  SwNodeIndex& rInsPos, sal_Bool bNewFrms );

    void UpdtOutlineIdx( const SwNode& );   // Update ab Node alle OutlineNodes

    void _CopyNodes( const SwNodeRange&, const SwNodeIndex&,
                    sal_Bool bNewFrms = sal_True, sal_Bool bTblInsDummyNode = sal_False ) const;
    void _DelDummyNodes( const SwNodeRange& rRg );

protected:
    SwNodes( SwDoc* pDoc );

public:
    ~SwNodes();

    typedef ::std::vector<SwNodeRange> NodeRanges_t;
    typedef ::std::vector<NodeRanges_t> TableRanges_t;

    SwNodePtr operator[]( sal_uLong n ) const
        { return (SwNodePtr)BigPtrArray::operator[] ( n ); }

    sal_uLong Count() const { return BigPtrArray::Count(); }
    void ForEach( FnForEach_SwNodes fnForEach, void* pArgs = 0 )
    {
        BigPtrArray::ForEach( 0, BigPtrArray::Count(),
                                (FnForEach) fnForEach, pArgs );
    }
    void ForEach( sal_uLong nStt, sal_uLong nEnd, FnForEach_SwNodes fnForEach, void* pArgs = 0 )
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
    sal_Bool IsDocNodes() const;

    sal_uInt16 GetSectionLevel(const SwNodeIndex &rIndex) const;
    void Delete(const SwNodeIndex &rPos, sal_uLong nNodes = 1);

    sal_Bool _MoveNodes( const SwNodeRange&, SwNodes& rNodes, const SwNodeIndex&,
                sal_Bool bNewFrms = sal_True );
    void MoveRange( SwPaM&, SwPosition&, SwNodes& rNodes );

    void _Copy( const SwNodeRange& rRg, const SwNodeIndex& rInsPos,
                sal_Bool bNewFrms = sal_True ) const
        {   _CopyNodes( rRg, rInsPos, bNewFrms ); }

    void SectionUp( SwNodeRange *);
    void SectionDown( SwNodeRange *pRange, SwStartNodeType = SwNormalStartNode );

    sal_Bool CheckNodesRange( const SwNodeIndex& rStt, const SwNodeIndex& rEnd ) const;

    void GoStartOfSection(SwNodeIndex *) const;
    void GoEndOfSection(SwNodeIndex *) const;

    SwCntntNode* GoNext(SwNodeIndex *) const;
    SwCntntNode* GoPrevious(SwNodeIndex *) const;

    //Gehe zum naechsten/vorherigen Cntnt/Tabellennode, fuer den
    //es LayoutFrames gibt, dabei Kopf-/Fusszeilen/Rahmen etc. nicht verlassen
    SwNode* GoNextWithFrm(SwNodeIndex *) const;
    SwNode* GoPreviousWithFrm(SwNodeIndex *) const;

    // zum naechsten Content-Node, der nicht geschuetzt oder versteckt ist
    // (beides auf sal_False ==> GoNext/GoPrevious!!!)
    SwCntntNode* GoNextSection( SwNodeIndex *, int bSkipHidden  = sal_True,
                                           int bSkipProtect = sal_True ) const;
    SwCntntNode* GoPrevSection( SwNodeIndex *, int bSkipHidden  = sal_True,
                                           int bSkipProtect = sal_True ) const;

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
                            sal_Bool bDelayed = sal_False );    // in ndgrf.cxx

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

    //void UpdateOutlineNode( const SwNode&, sal_uInt8 nOldLevel, sal_uInt8 nNewLevel );//#outline level,removed by zhaojianwei
        // alle Nodes Updaten - Rule/Format-Aenderung
    void UpdateOutlineNode(SwNode & rNd);

        // fuege die Nodes fuer die Tabelle ein
        // wenn Lines angegeben, erzeuge die Matrix aus Lines & Boxen
        // ansonsten nur die Anzahl von Boxen.
    /* #109161#

       New parameter pAttrSet: If pAttrSet is non-null and contains an
       adjust item it is propagated to the table cells. If there is an
       adjust in pCntntTxtColl or pHeadlineTxtColl this adjust item
       overrides the item in pAttrSet.

     */
    SwTableNode* InsertTable( const SwNodeIndex& rNdIdx,
                        sal_uInt16 nBoxes, SwTxtFmtColl* pCntntTxtColl,
                        sal_uInt16 nLines = 0, sal_uInt16 nRepeat = 0,
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
    sal_Bool TableToText( const SwNodeRange& rRange, sal_Unicode cCh,
                        SwUndoTblToTxt* = 0 );
        // steht im untbl.cxx und darf nur vom Undoobject gerufen werden
    SwTableNode* UndoTableToText( sal_uLong nStt, sal_uLong nEnd,
                        const SwTblToTxtSaves& rSavedData );

        // fuege in der Line, vor der InsPos eine neue Box ein. Das Format
        // wird von der nachfolgenden (vorhergenden;wenn an Ende) genommen
        // in der Line muss schon eine Box vorhanden sein !
    sal_Bool InsBoxen( SwTableNode*, SwTableLine*, SwTableBoxFmt*,
                        // Formate fuer den TextNode der Box
                        SwTxtFmtColl*, const SfxItemSet* pAutoAttr,
                        sal_uInt16 nInsPos, sal_uInt16 nCnt = 1 );
        // Splittet eine Tabelle in der Grund-Zeile, in der der Index steht.
        // Alle GrundZeilen dahinter wandern in eine neue Tabelle/-Node.
        // Ist das Flag bCalcNewSize auf sal_True, wird fuer beide neuen Tabellen
        // die neue SSize aus dem Max der Boxen errechnet; vorrausgesetzt,
        // die SSize ist "absolut" gesetzt (LONG_MAX)
        // (Wird zur Zeit nur fuer den RTF-Parser benoetigt)
    SwTableNode* SplitTable( const SwNodeIndex& rPos, sal_Bool bAfter = sal_True,
                                sal_Bool bCalcNewSize = sal_False );
        // fuegt 2 Tabellen, die hintereinander stehen, wieder zusammen
    sal_Bool MergeTable( const SwNodeIndex& rPos, sal_Bool bWithPrev = sal_True,
                    sal_uInt16 nMode = 0, SwHistory* pHistory = 0 );

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

    //-> #112139#
    SwNode * DocumentSectionStartNode(SwNode * pNode) const;
    SwNode * DocumentSectionEndNode(SwNode * pNode) const;
    //<- #112139#
};

#endif
