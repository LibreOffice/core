/*************************************************************************
 *
 *  $RCSfile: ndarr.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jp $ $Date: 2000-11-28 20:33:09 $
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

#ifndef _NDARR_HXX
#define _NDARR_HXX

#ifndef _SVARRAY_HXX //autogen
#include <svtools/svarray.hxx>
#endif

#ifndef _BPARR_HXX
#include <bparr.hxx>
#endif
#ifndef _NDTYP_HXX
#include <ndtyp.hxx>
#endif

class Graphic;
class GraphicObject;
class UniString;
class SvInPlaceObject;
class SwAttrSet;
class SwCntntFrm;
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
class SwSection;
class SwSectionFmt;
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
class SwUndoInsNum;
class SwUndoTblToTxt;
class SwUndoTxtToTbl;
struct SwPosition;


// --------------------
// class SwNodes
// --------------------

typedef SwNode * SwNodePtr;
typedef BOOL (*FnForEach_SwNodes)( const SwNodePtr&, void* pArgs );

SV_DECL_PTRARR_SORT( SwOutlineNodes, SwNodePtr, 0, 10 )

class SwNodes: private BigPtrArray
{
    friend class SwDoc;
    friend class SwNode;
    friend class SwNodeIndex;

    SwNodePtr operator[]( USHORT n ) const;
    SwNodePtr operator[]( int n ) const;

    SwNodeIndex* pRoot;                 // Liste aller Indizies auf Nodes

    void Insert( const SwNodePtr pNode, const SwNodeIndex& rPos );
    void Insert( const SwNodePtr pNode, ULONG nPos);
//  void Remove( const SwNodeIndex& rPos, USHORT nLen = 1 );
//  void Remove( ULONG nPos, USHORT nLen = 1 );
//  BOOL Move( const SwIndex & rOldPos, const SwIndex & rNewPos );


    SwDoc* pMyDoc;                      // in diesem Doc ist das Nodes-Array

    SwNode *pEndOfPostIts, *pEndOfInserts,  // das sind die festen Bereiche
           *pEndOfAutotext, *pEndOfRedlines,
           *pEndOfContent;

    SwOutlineNodes* pOutlineNds;        // Array aller GliederiungsNodes

    BOOL bInNodesDel : 1;               // falls rekursiv aufgerufen wird
                                        // Num/Outline nicht aktualisierem
    BOOL bInDelUpdOutl : 1;             // Flags fuers aktualisieren von Outl.
    BOOL bInDelUpdNum : 1;              // Flags fuers aktualisieren von Outl.

    // fuer dier Verwaltung der Indizies
    void RegisterIndex( SwNodeIndex& rIdx );
    void DeRegisterIndex( SwNodeIndex& rIdx );
    void RemoveNode( ULONG nDelPos, ULONG nLen, FASTBOOL bDel );

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

    SwNodePtr operator[]( ULONG n ) const
        { return (SwNodePtr)BigPtrArray::operator[] ( n ); }

//JP 29.09.97: impl. steht im ndindex.hxx - sollte moeglichst bald auf die
//              neue Schnittstelle angepasst werden
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
    inline BOOL IsDocNodes() const;

    USHORT GetSectionLevel(const SwNodeIndex &rIndex) const;
    void Delete(const SwNodeIndex &rPos, ULONG nNodes = 1);

    BOOL _MoveNodes( const SwNodeRange&, SwNodes& rNodes, const SwNodeIndex&,
                BOOL bNewFrms = TRUE );
    void Move( SwPaM&, SwPosition&, SwNodes& rNodes, BOOL bSplitNd=TRUE );

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

    SwCntntFrm *MakeFrm(const SwNodeIndex &rIndex);

    // die Impl. von "Make...Node" stehen in den angegebenen .ccx-Files
    SwTxtNode *MakeTxtNode( const SwNodeIndex & rWhere,
                            SwTxtFmtColl *pColl,
                            SwAttrSet* pAutoAttr = 0 ); // in ndtxt.cxx
    SwStartNode* MakeTextSection( const SwNodeIndex & rWhere,
                            SwStartNodeType eSttNdTyp,
                            SwTxtFmtColl *pColl,
                            SwAttrSet* pAutoAttr = 0 );

    SwGrfNode *MakeGrfNode( const SwNodeIndex & rWhere,
                            const UniString& rGrfName,
                            const UniString& rFltName,
                            const Graphic* pGraphic,
                            SwGrfFmtColl *pColl,
                            SwAttrSet* pAutoAttr = 0,
                            BOOL bDelayed = FALSE );    // in ndgrf.cxx

    SwGrfNode *MakeGrfNode( const SwNodeIndex & rWhere,
                            const GraphicObject& rGrfObj,
                            SwGrfFmtColl *pColl,
                            SwAttrSet* pAutoAttr = 0 ); // in ndgrf.cxx

    SwOLENode *MakeOLENode( const SwNodeIndex & rWhere,
                            SvInPlaceObject *pObj,
                            SwGrfFmtColl *pColl,
                            SwAttrSet* pAutoAttr = 0 ); // in ndole.cxx
    SwOLENode *MakeOLENode( const SwNodeIndex & rWhere,
                            UniString &rName,
                            SwGrfFmtColl *pColl,
                            SwAttrSet* pAutoAttr = 0 ); // in ndole.cxx

        // Array aller GliederiungsNodes;
    const SwOutlineNodes& GetOutLineNds() const { return *pOutlineNds; }
        // ab einem bestimmten TextNode alle Updaten
    void UpdateOutlineNode( const SwNode&, BYTE nOldLevel, BYTE nNewLevel );
        // alle Nodes Updaten - Rule/Format-Aenderung
    void UpdateOutlineNodes();

        // fuege die Nodes fuer die Tabelle ein
        // wenn Lines angegeben, erzeuge die Matrix aus Lines & Boxen
        // ansonsten nur die Anzahl von Boxen.
    SwTableNode* InsertTable( const SwNodeIndex& rNdIdx,
                        USHORT nBoxes, SwTxtFmtColl* pCntntTxtColl,
                        USHORT nLines=0, SwTxtFmtColl* pHeadlineTxtColl=0 );
        // erzeuge aus dem makierten Bereich eine ausgeglichene Tabelle
    SwTableNode* TextToTable( const SwNodeRange& rRange, sal_Unicode cCh,
                                SwTableFmt* pTblFmt,
                                SwTableLineFmt* pLineFmt,
                                SwTableBoxFmt* pBoxFmt,
                                SwTxtFmtColl* pTxtColl,
                                SwUndoTxtToTbl* pUndo = 0 );
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
                        SwTxtFmtColl*, SwAttrSet* pAutoAttr,
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
    SwSectionNode* InsertSection( const SwNodeIndex& rNdIdx,
                                SwSectionFmt& rSectionFmt,
                                const SwSection&,
                                const SwNodeIndex* pEnde,
                                BOOL bInsAtStart = TRUE,
                                BOOL bCreateFrms = TRUE );

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
private:
    // privater Constructor, weil nie kopiert werden darf !!
    SwNodes( const SwNodes & rNodes );
};



#endif
