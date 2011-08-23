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

#ifndef _NODE_HXX
#define _NODE_HXX


#include <tools/mempool.hxx>
#include <tools/gen.hxx>

#include <ndarr.hxx>
#include <ndtyp.hxx>
#include <index.hxx>
#include <fmtcol.hxx>
namespace binfilter {
class SvUShorts;
class SwCntntFrm;
class SwCntntNode;
class SwDoc;
class SwEndNode;
class SwFrm;
class SwFrmFmt;
class SwGrfNode;
class SwNoTxtNode;
class SwNodeIndex;
class SwNodeRange;
class SwOLENode;
class SwRect;
class SwSection;
class SwSectionFmt;
class SwSectionNode;
class SwStartNode;
class SwTabFrm;
class SwTable;
class SwTableNode;
class SwTableBox;
class SwTxtNode;
class SwPageDesc;
class ViewShell;
struct SwPosition;

#if defined(MACOSX) && ( __GNUC__ < 3 )
#define INLINE /* GrP revisit with gcc3 */
#else
#define INLINE inline
#endif

// --------------------
// class SwNode
// --------------------
class SwNode : private /* public*/ BigPtrEntry
{
    friend class SwNodes;

    BYTE nNodeType;
    BOOL bWrongDirty : 1; 		// Ist das Wrong-Feld auf invalid? (nur TxtNodes)
    BOOL bACmplWrdDirty : 1;	// die ACompl-Liste muss angepasst werden (erstmal nur TxtNodes)

    // JP 28.03.96
    // fuer Textnodes: Stufungslevel der Autoformatierung. Ist erstmal hier
    //					gelandet, weil noch Bits frei sind
    BYTE nAFmtNumLvl : 3;
    BOOL bSetNumLSpace : 1;			// fuer Numerierung: TRUE Einzug setzen
    BOOL bIgnoreDontExpand : 1;		// for Text Attributes - ignore the flag

protected:
    SwStartNode* pStartOfSection;

    SwNode( const SwNodeIndex &rWhere, const BYTE nNodeId );

    // fuer den initialen StartNode
    SwNode( SwNodes& rNodes, ULONG nPos, const BYTE nNodeId );

public:
    virtual ~SwNode();

    inline ULONG StartOfSectionIndex() const;
    inline const SwStartNode* StartOfSectionNode() const;
    inline 		 SwStartNode* StartOfSectionNode();
    inline ULONG EndOfSectionIndex() const;
    inline const SwEndNode* EndOfSectionNode() const;
    inline 		 SwEndNode* EndOfSectionNode();

    inline BOOL IsWrongDirty() const		{ return bWrongDirty; }
    inline void SetWrongDirty( BOOL bNew )		{ bWrongDirty = bNew; }

    inline BOOL IsAutoCompleteWordDirty() const	{ return bACmplWrdDirty; }
    inline void SetAutoCompleteWordDirty( BOOL bNew )	{ bACmplWrdDirty = bNew; }

    inline BYTE GetAutoFmtLvl() const		{ return nAFmtNumLvl; }
    inline void SetAutoFmtLvl( BYTE nVal )		{ nAFmtNumLvl = nVal; }

    inline BOOL IsSetNumLSpace() const 	{ return bSetNumLSpace; }
    inline void SetNumLSpace( BOOL bFlag )		{ bSetNumLSpace = bFlag; }

    inline BOOL IsIgnoreDontExpand() const	{ return bIgnoreDontExpand; }
    inline void SetIgnoreDontExpand( BOOL bNew )	{ bIgnoreDontExpand = bNew; }

    BYTE 	GetNodeType() const { return nNodeType; }

    inline       SwStartNode *GetStartNode();
    inline const SwStartNode *GetStartNode() const;
    inline       SwCntntNode *GetCntntNode();
    inline const SwCntntNode *GetCntntNode() const;
    inline       SwEndNode   *GetEndNode();
    inline const SwEndNode   *GetEndNode() const;
#ifndef	 ICC
  INLINE
#endif
    SwTxtNode   *GetTxtNode();

#ifndef	 ICC
  INLINE
#endif
    const SwTxtNode   *GetTxtNode() const;
#ifndef COMPACT
    INLINE        SwOLENode   *GetOLENode();
    INLINE  const SwOLENode   *GetOLENode() const;
    INLINE        SwNoTxtNode *GetNoTxtNode();
    INLINE  const SwNoTxtNode *GetNoTxtNode() const;
    INLINE        SwGrfNode   *GetGrfNode();
    INLINE  const SwGrfNode   *GetGrfNode() const;
#endif
    inline       SwTableNode *GetTableNode();
    inline const SwTableNode *GetTableNode() const;
    inline       SwSectionNode *GetSectionNode();
    inline const SwSectionNode *GetSectionNode() const;

    inline BOOL IsStartNode() const;
    inline BOOL IsCntntNode() const;
    inline BOOL IsEndNode() const;
    inline BOOL IsTxtNode() const;
    inline BOOL IsTableNode() const;
    inline BOOL IsSectionNode() const;
#ifndef COMPACT
    inline BOOL IsOLENode() const;
    inline BOOL IsNoTxtNode() const;
    inline BOOL IsGrfNode() const;
#endif

    // suche den TabellenNode, in dem dieser steht. Wenn in keiner
    // Tabelle wird 0 returnt.
                    SwTableNode *FindTableNode();
    inline const	SwTableNode *FindTableNode() const;

    // suche den SectionNode, in dem dieser steht. Wenn es in keiner
    // Section steht wird 0 returnt.
                    SwSectionNode *FindSectionNode();
    inline 	const	SwSectionNode *FindSectionNode() const;

    const	SwStartNode *FindStartNode() const	{ return pStartOfSection; }
            SwStartNode *FindStartNode()		{ return pStartOfSection; }

    SwStartNode* FindSttNodeByType( SwStartNodeType eTyp );
    inline const SwStartNode* FindSttNodeByType( SwStartNodeType eTyp ) const;

    const SwStartNode* FindTableBoxStartNode() const
                        { return FindSttNodeByType( SwTableBoxStartNode ); }
    const SwStartNode* FindFlyStartNode() const
                        { return FindSttNodeByType( SwFlyStartNode ); }
    const SwStartNode* FindFootnoteStartNode() const
                        { return FindSttNodeByType( SwFootnoteStartNode ); }
    const SwStartNode* FindHeaderStartNode() const
                        { return FindSttNodeByType( SwHeaderStartNode ); }
    const SwStartNode* FindFooterStartNode() const
                        { return FindSttNodeByType( SwFooterStartNode ); }

        // in welchem Nodes-Array/Doc steht der Node ?
    inline 		  SwNodes& GetNodes();
    inline const  SwNodes& GetNodes() const;
    inline 			SwDoc* GetDoc();
    inline const	SwDoc* GetDoc() const;

    // liegt der Node im Sichtbarenbereich der Shell ?
    // befindet sich der Node in einem geschuetzten Bereich?
    BOOL IsInProtectSect() const;
    // befindet sich der Node in irgendetwas geschuetzten ?
    // (Bereich/Rahmen/Tabellenzellen/... incl. des Ankers bei
    //	Rahmen/Fussnoten/..)
    BOOL IsProtect() const;
    // suche den PageDesc, mit dem dieser Node formatiert ist. Wenn das
    // Layout vorhanden ist wird ueber das gesucht, ansonsten gibt es nur
    // die harte Tour ueber die Nodes nach vorne suchen!!
    // OD 18.03.2003 #106326#

    // falls der Node in einem Fly steht, dann wird das entsprechende Format
    // returnt
    SwFrmFmt* GetFlyFmt() const;

    // liefert das Format, an dem die LayoutFrames des StartNodes registriert sind,
    // wird von SwSectionNode::MakeFrms benutzt

    // falls der Node in einer Tabelle steht, dann wird die entsprechende
    // TabellenBox returnt
    SwTableBox* GetTblBox() const;

    inline ULONG GetIndex() const { return GetPos(); }

    const SwTxtNode* FindOutlineNodeOfLevel( BYTE nLvl ) const;


private:
    // privater Constructor, weil nie kopiert werden darf !!
    SwNode( const SwNode & rNodes );
};

// --------------------
// class SwStartNode
// --------------------
class SwStartNode: public SwNode
{
    friend class SwNode;
    friend class SwNodes;
    friend class SwEndNode;		// um theEndOfSection zu setzen !!

    SwEndNode* pEndOfSection;
    SwStartNodeType eSttNdTyp;

    // fuer den initialen StartNode
    SwStartNode( SwNodes& rNodes, ULONG nPos );

protected:
    SwStartNode( const SwNodeIndex &rWhere,
                 const BYTE nNodeType = ND_STARTNODE,
                 SwStartNodeType = SwNormalStartNode );
public:
    DECL_FIXEDMEMPOOL_NEWDEL(SwStartNode)

    SwStartNodeType GetStartNodeType() const 		{ return eSttNdTyp; }

    // an alle ContentNodes der Section das ChkCondColl rufen
    void CheckSectionCondColl() const;

private:
    // privater Constructor, weil nie kopiert werden darf !!
    SwStartNode( const SwStartNode & rNode );
};


// --------------------
// class SwEndNode
// --------------------
class SwEndNode : public SwNode
{
    friend class SwNodes;
    friend class SwTableNode;		// um seinen EndNode anlegen zukoennen
    friend class SwSectionNode;		// um seinen EndNode anlegen zukoennen

    // fuer den initialen StartNode
    SwEndNode( SwNodes& rNodes, ULONG nPos, SwStartNode& rSttNd );

protected:
    SwEndNode( const SwNodeIndex &rWhere, SwStartNode& rSttNd );

    DECL_FIXEDMEMPOOL_NEWDEL(SwEndNode)

private:
    // privater Constructor, weil nie kopiert werden darf !!
    SwEndNode( const SwEndNode & rNode );
};


// --------------------
// SwCntntNode
// --------------------
class SwCntntNode: public SwModify, public SwNode, public SwIndexReg
{
    // Der Reader darf NewAttrSet() aufrufen!
    friend class SwSwgReader;
    friend class Sw3IoImp;

//FEATURE::CONDCOLL
    SwDepend* pCondColl;
//FEATURE::CONDCOLL

protected:
    SwCntntNode( const SwNodeIndex &rWhere, const BYTE nNodeType,
                SwFmtColl *pFmtColl );
    virtual ~SwCntntNode();

    // Attribut-Set fuer alle AUTO-Attribute eines CntntNodes
    //	( z.B: TxtNode oder NoTxtNode
    SwAttrSet *pAttrSet;
    // lasse von den entsprechenden Nodes die spz. AttrSets anlegen
    virtual void NewAttrSet( SwAttrPool& ) = 0;

public:
    TYPEINFO();		//Bereits in Basisklasse Client drin.

    virtual void Modify( SfxPoolItem *pOld, SfxPoolItem *pNew);

    virtual SwCntntFrm *MakeFrm() = 0;
     virtual SwCntntNode *SplitNode(const SwPosition & ) = 0;
    virtual SwCntntNode *JoinNext();
    virtual SwCntntNode *JoinPrev();
    // koennen 2 Nodes zusammengefasst werden ?
    // in pIdx kann die 2. Position returnt werden.
    int CanJoinNext( SwNodeIndex* pIdx =0 ) const;
    int CanJoinPrev( SwNodeIndex* pIdx =0 ) const;

    void MakeStartIndex( SwIndex * pIdx )	{ pIdx->Assign( this, 0 ); }
    void MakeEndIndex( SwIndex * pIdx )		{ pIdx->Assign( this, Len() ); }

    BOOL GoNext(SwIndex *, USHORT nMode ) const;
    BOOL GoPrevious(SwIndex *, USHORT nMode ) const;

    SwCntntFrm *GetFrm( const Point* pDocPos = 0,
                        const SwPosition *pPos = 0,
                        const BOOL bCalcFrm = TRUE ) const;
    // Gibt die tatsaechlcheGroesse des Frames zurueck bzw. ein leeres
    // Rechteck, wenn kein Layout existiert.
    // Wird fuer die Export Filter benoetigt.
    /*
     * Methode erzeugt fuer den angegebenen Node alle Ansichten vom
     * Dokument. Die erzeugten Contentframes werden in das entsprechende
     * Layout gehaengt.
     */
    void MakeFrms( SwCntntNode& rNode );
    /*
     * Methode loescht fuer den Node alle Ansichten vom
     * Dokument. Die Contentframes werden aus dem entsprechenden
     * Layout ausgehaengt.
     */
    void DelFrms();

    /*
     * liefert die Anzahl der Elemente des Inhalts des Nodes;
     * Default ist 1, Unterschiede gibt es beim TextNode und beim
     * Formelnode.
     */
    virtual xub_StrLen Len() const;

    virtual SwCntntNode* MakeCopy( SwDoc*, const SwNodeIndex& ) const = 0;
        // erfrage vom Client Informationen
    virtual BOOL GetInfo( SfxPoolItem& ) const;

    // SS fuer die PoolItems: (Harte-(Fmt)Attrbutierung)

    // Ist bInParent FALSE, wird nur in diesem Node nach dem Attribut gesucht.
    const SfxPoolItem& GetAttr( USHORT nWhich, BOOL bInParent=TRUE ) const;
    BOOL GetAttr( SfxItemSet& rSet, BOOL bInParent=TRUE ) const;
    BOOL SetAttr( const SfxPoolItem& );
    BOOL SetAttr( const SfxItemSet& rSet );
    BOOL ResetAttr( USHORT nWhich1, USHORT nWhich2 = 0 );
    USHORT ResetAllAttr();
    // liefert das Attribut, das nicht ueber die bedingte Vorlage kommt!
    const SfxPoolItem* GetNoCondAttr( USHORT nWhich, BOOL bInParents ) const;

    // hat der Node schon eigene Auto-Attribute ?
    // Zugriff auf SwAttrSet
    inline 		 SwAttrSet &GetSwAttrSet();
    inline const SwAttrSet &GetSwAttrSet() const;
    inline 		 SwAttrSet *GetpSwAttrSet() { return pAttrSet; }
    inline const SwAttrSet *GetpSwAttrSet() const { return pAttrSet; }
    inline BOOL  HasSwAttrSet() const { return pAttrSet ? TRUE : FALSE; }

    virtual SwFmtColl* ChgFmtColl( SwFmtColl* );
    SwFmtColl* GetFmtColl() const { return (SwFmtColl*)GetRegisteredIn(); }

//FEATURE::CONDCOLL
    inline SwFmtColl& GetAnyFmtColl() const;
    void SetCondFmtColl( SwFmtColl* );
    inline SwFmtColl* GetCondFmtColl() const;

    BOOL IsAnyCondition( SwCollCondition& rTmp ) const;
    void ChkCondColl();
//FEATURE::CONDCOLL

    // invalidiert die am Node gesetzte NumRule. Diese wird
    // spaetestend in EndAction einer Shell geupdatet
    BOOL InvalidateNumRule();

private:
    // privater Constructor, weil nie kopiert werden darf !!
    SwCntntNode( const SwCntntNode & rNode );
};



//---------
// SwTableNode
//---------
class SwTableNode : public SwStartNode
{
    friend class SwNodes;
    SwTable* pTable;
protected:
    virtual ~SwTableNode();

public:
    SwTableNode( const SwNodeIndex & );

    const SwTable& GetTable() const { return *pTable; }
    SwTable& GetTable() { return *pTable; }
    SwTabFrm *MakeFrm();

    //Legt die Frms fuer den TableNode (also die TabFrms) an.
    void MakeFrms( SwNodeIndex* pIdxBehind );

    //Methode loescht fuer den Node alle Ansichten vom
    //Dokument. Die Contentframes werden aus dem entsprechenden
    //Layout ausgehaengt.
    void DelFrms();

    SwTableNode* MakeCopy( SwDoc*, const SwNodeIndex& ) const;
     void SetNewTable( SwTable* , BOOL bNewFrames=TRUE );

private:
    // privater Constructor, weil nie kopiert werden darf !!
    SwTableNode( const SwTableNode & rNode );
};


//---------
// SwSectionNode
//---------
class SwSectionNode : public SwStartNode
{
    friend class SwNodes;
    SwSection* pSection;
protected:
    virtual ~SwSectionNode();

public:
    SwSectionNode( const SwNodeIndex&, SwSectionFmt& rFmt );

    const SwSection& GetSection() const { return *pSection; }
    SwSection& GetSection() { return *pSection; }

    // setze ein neues SectionObject. Erstmal nur gedacht fuer die
    // neuen VerzeichnisSections. Der geht ueber in den Besitz des Nodes!
    void SetNewSection( SwSection* pNewSection );

    SwFrm *MakeFrm();

    //Legt die Frms fuer den SectionNode (also die SectionFrms) an.
    //Im Defaultfall wird bis die Frames bis zum Ende des Bereichs angelegt,
    //uebergibt man einen weiteren NodeIndex pEnd, so wird bis zu diesem
    //ein MakeFrms gerufen, genutzt wird dies von TableToText
    void MakeFrms( SwNodeIndex* pIdxBehind, SwNodeIndex* pEnd = NULL );

    //Methode loescht fuer den Node alle Ansichten vom
    //Dokument. Die Contentframes werden aus dem entsprechenden
    //Layout ausgehaengt.
    void DelFrms();

private:
    // privater Constructor, weil nie kopiert werden darf !!
    SwSectionNode( const SwSection& rNode );
};



// ---------------------- einige Inline Methoden ----------------------
inline       SwEndNode   *SwNode::GetEndNode()
{
     return ND_ENDNODE == nNodeType ? (SwEndNode*)this : 0;
}
inline const SwEndNode   *SwNode::GetEndNode() const
{
     return ND_ENDNODE == nNodeType ? (const SwEndNode*)this : 0;
}
inline       SwStartNode *SwNode::GetStartNode()
{
     return ND_STARTNODE & nNodeType ? (SwStartNode*)this : 0;
}
inline const SwStartNode *SwNode::GetStartNode() const
{
     return ND_STARTNODE & nNodeType ? (const SwStartNode*)this : 0;
}
inline       SwTableNode *SwNode::GetTableNode()
{
     return ND_TABLENODE == nNodeType ? (SwTableNode*)this : 0;
}
inline const SwTableNode *SwNode::GetTableNode() const
{
     return ND_TABLENODE == nNodeType ? (const SwTableNode*)this : 0;
}
inline       SwSectionNode *SwNode::GetSectionNode()
{
     return ND_SECTIONNODE == nNodeType ? (SwSectionNode*)this : 0;
}
inline const SwSectionNode *SwNode::GetSectionNode() const
{
     return ND_SECTIONNODE == nNodeType ? (const SwSectionNode*)this : 0;
}
inline       SwCntntNode *SwNode::GetCntntNode()
{
     return ND_CONTENTNODE & nNodeType ? (SwCntntNode*)this : 0;
}
inline const SwCntntNode *SwNode::GetCntntNode() const
{
     return ND_CONTENTNODE & nNodeType ? (const SwCntntNode*)this : 0;
}


inline BOOL SwNode::IsStartNode() const
{
    return ND_STARTNODE & nNodeType  ? TRUE : FALSE;
}
inline BOOL SwNode::IsCntntNode() const
{
    return ND_CONTENTNODE & nNodeType  ? TRUE : FALSE;
}
inline BOOL SwNode::IsEndNode() const
{
    return ND_ENDNODE == nNodeType  ? TRUE : FALSE;
}
inline BOOL SwNode::IsTxtNode() const
{
    return ND_TEXTNODE == nNodeType  ? TRUE : FALSE;
}
inline BOOL SwNode::IsTableNode() const
{
    return ND_TABLENODE == nNodeType  ? TRUE : FALSE;
}
inline BOOL SwNode::IsSectionNode() const
{
    return ND_SECTIONNODE == nNodeType  ? TRUE : FALSE;
}
#ifndef COMPACT
inline BOOL SwNode::IsNoTxtNode() const
{
    return ND_NOTXTNODE & nNodeType  ? TRUE : FALSE;
}
inline BOOL SwNode::IsOLENode() const
{
    return ND_OLENODE == nNodeType  ? TRUE : FALSE;
}
inline BOOL SwNode::IsGrfNode() const
{
    return ND_GRFNODE == nNodeType  ? TRUE : FALSE;
}
#endif

inline const SwStartNode* SwNode::FindSttNodeByType( SwStartNodeType eTyp ) const
{
    return ((SwNode*)this)->FindSttNodeByType( eTyp );
}
inline const SwTableNode* SwNode::FindTableNode() const
{
    return ((SwNode*)this)->FindTableNode();
}
inline const SwSectionNode* SwNode::FindSectionNode() const
{
    return ((SwNode*)this)->FindSectionNode();
}
inline ULONG SwNode::StartOfSectionIndex() const
{
    return pStartOfSection->GetIndex();
}
inline const SwStartNode* SwNode::StartOfSectionNode() const
{
    return pStartOfSection;
}
inline SwStartNode* SwNode::StartOfSectionNode()
{
    return pStartOfSection;
}
inline ULONG SwNode::EndOfSectionIndex() const
{
    const SwStartNode* pStNd = IsStartNode() ? (SwStartNode*)this : pStartOfSection;
    return pStNd->pEndOfSection->GetIndex();
}
inline const SwEndNode* SwNode::EndOfSectionNode() const
{
    const SwStartNode* pStNd = IsStartNode() ? (SwStartNode*)this : pStartOfSection;
    return pStNd->pEndOfSection;
}
inline SwEndNode* SwNode::EndOfSectionNode()
{
    SwStartNode* pStNd = IsStartNode() ? (SwStartNode*)this : pStartOfSection;
    return pStNd->pEndOfSection;
}

inline SwNodes& SwNode::GetNodes()
{
    return (SwNodes&)GetArray();
}
inline const SwNodes& SwNode::GetNodes() const
{
    return (SwNodes&)GetArray();
}

inline SwDoc* SwNode::GetDoc()
{
    return GetNodes().GetDoc();
}
inline const SwDoc* SwNode::GetDoc() const
{
    return GetNodes().GetDoc();
}

inline SwFmtColl* SwCntntNode::GetCondFmtColl() const
{
    return pCondColl ? (SwFmtColl*)pCondColl->GetRegisteredIn() : 0;
}


inline SwFmtColl& SwCntntNode::GetAnyFmtColl() const
{
    return pCondColl && pCondColl->GetRegisteredIn()
                ? *(SwFmtColl*)pCondColl->GetRegisteredIn()
                : *(SwFmtColl*)GetRegisteredIn();
}

inline SwAttrSet& SwCntntNode::GetSwAttrSet()
{
    return pAttrSet ? ( (SwAttrSet &) ( *pAttrSet ) )
                    : ( (SwAttrSet &) ( GetAnyFmtColl().GetAttrSet() ) );
}
inline const SwAttrSet& SwCntntNode::GetSwAttrSet() const
{
    return pAttrSet ? ( (SwAttrSet &) ( *pAttrSet ) )
                    : ( (SwAttrSet &) ( GetAnyFmtColl().GetAttrSet() ) );
}
//FEATURE::CONDCOLL

inline const SfxPoolItem& SwCntntNode::GetAttr( USHORT nWhich,
                                                BOOL bInParents ) const
{
    return GetSwAttrSet().Get( nWhich, bInParents );
}


#undef INLINE

} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
