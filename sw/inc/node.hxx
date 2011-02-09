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

#ifndef SW_NODE_HXX
#define SW_NODE_HXX

#include <vector>

#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>

#include <tools/mempool.hxx>
#include <tools/gen.hxx>

#include "swdllapi.h"
#include <ndarr.hxx>
#include <ndtyp.hxx>
#include <index.hxx>
#include <fmtcol.hxx>

// ---------------------
// forward Deklarationen
// ---------------------

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
class SwOLENode;
class SwRect;
class SwSection;
class SwSectionFmt;
class SwTOXBase;
class SwSectionNode;
class SwStartNode;
class SwTabFrm;
class SwRootFrm;
class SwTable;
class SwTableNode;
class SwTableBox;
class SwTxtNode;
class SwPageDesc;
class ViewShell;
struct SwPosition;
class IStyleAccess;
class IDocumentSettingAccess;
class IDocumentDeviceAccess;
class IDocumentMarkAccess;
class IDocumentRedlineAccess;
class IDocumentStylePoolAccess;
class IDocumentLineNumberAccess;
class IDocumentLinksAdministration;
class IDocumentFieldsAccess;
class IDocumentContentOperations;
class IDocumentListItems;
class SwOLENodes;

// --------------------
// class SwNode
// --------------------
class SW_DLLPUBLIC SwNode : private /* public*/ BigPtrEntry
{
    friend class SwNodes;

#ifdef DBG_UTIL
    static long nSerial;
    long nMySerial;
#endif

    sal_uInt8 nNodeType;

    // JP 28.03.96
    // fuer Textnodes: Stufungslevel der Autoformatierung. Ist erstmal hier
    //                  gelandet, weil noch Bits frei sind
    sal_uInt8 nAFmtNumLvl : 3;
    sal_Bool bSetNumLSpace : 1;         // fuer Numerierung: sal_True Einzug setzen
    sal_Bool bIgnoreDontExpand : 1;     // for Text Attributes - ignore the flag

protected:
    SwStartNode* pStartOfSection;

    SwNode( const SwNodeIndex &rWhere, const sal_uInt8 nNodeId );

    // fuer den initialen StartNode
    SwNode( SwNodes& rNodes, sal_uLong nPos, const sal_uInt8 nNodeId );

public:
    virtual ~SwNode();

#ifdef DBG_UTIL
    long int GetSerial() const { return nMySerial; }
#endif

    sal_uInt16 GetSectionLevel() const;

    inline sal_uLong StartOfSectionIndex() const;
    inline const SwStartNode* StartOfSectionNode() const { return pStartOfSection; }
    inline       SwStartNode* StartOfSectionNode() { return pStartOfSection; }

    inline sal_uLong EndOfSectionIndex() const;
    inline const SwEndNode* EndOfSectionNode() const;
    inline         SwEndNode* EndOfSectionNode();

    inline sal_uInt8 GetAutoFmtLvl() const     { return nAFmtNumLvl; }
    inline void SetAutoFmtLvl( sal_uInt8 nVal )      { nAFmtNumLvl = nVal; }

    inline sal_Bool IsSetNumLSpace() const  { return bSetNumLSpace; }
    inline void SetNumLSpace( sal_Bool bFlag )        { bSetNumLSpace = bFlag; }

    inline sal_Bool IsIgnoreDontExpand() const  { return bIgnoreDontExpand; }
    inline void SetIgnoreDontExpand( sal_Bool bNew )  { bIgnoreDontExpand = bNew; }

    sal_uInt8   GetNodeType() const { return nNodeType; }

    inline       SwStartNode *GetStartNode();
    inline const SwStartNode *GetStartNode() const;
    inline       SwCntntNode *GetCntntNode();
    inline const SwCntntNode *GetCntntNode() const;
    inline       SwEndNode   *GetEndNode();
    inline const SwEndNode   *GetEndNode() const;
#ifndef  ICC
  inline
#endif
    SwTxtNode   *GetTxtNode();

#ifndef  ICC
  inline
#endif
    const SwTxtNode   *GetTxtNode() const;
    inline        SwOLENode   *GetOLENode();
    inline  const SwOLENode   *GetOLENode() const;
    inline        SwNoTxtNode *GetNoTxtNode();
    inline  const SwNoTxtNode *GetNoTxtNode() const;
    inline        SwGrfNode   *GetGrfNode();
    inline  const SwGrfNode   *GetGrfNode() const;
    inline       SwTableNode *GetTableNode();
    inline const SwTableNode *GetTableNode() const;
    inline       SwSectionNode *GetSectionNode();
    inline const SwSectionNode *GetSectionNode() const;

    inline sal_Bool IsStartNode() const;
    inline sal_Bool IsCntntNode() const;
    inline sal_Bool IsEndNode() const;
    inline sal_Bool IsTxtNode() const;
    inline sal_Bool IsTableNode() const;
    inline sal_Bool IsSectionNode() const;
    inline sal_Bool IsOLENode() const;
    inline sal_Bool IsNoTxtNode() const;
    inline sal_Bool IsGrfNode() const;

    /**
       Checks if this node is in redlines.

       @retval sal_True       this node is in redlines
       @retval sal_False      else
     */
    sal_Bool IsInRedlines() const;

    // suche den TabellenNode, in dem dieser steht. Wenn in keiner
    // Tabelle wird 0 returnt.
                    SwTableNode *FindTableNode();
    inline const  SwTableNode *FindTableNode() const;

    // suche den SectionNode, in dem dieser steht. Wenn es in keiner
    // Section steht wird 0 returnt.
                    SwSectionNode *FindSectionNode();
    inline    const   SwSectionNode *FindSectionNode() const;

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
    inline          SwNodes& GetNodes();
    inline const  SwNodes& GetNodes() const;
    inline            SwDoc* GetDoc();
    inline const  SwDoc* GetDoc() const;

    /** Provides access to the document setting interface
     */
    const IDocumentSettingAccess* getIDocumentSettingAccess() const;

    /** Provides access to the document device interface
     */
    const IDocumentDeviceAccess* getIDocumentDeviceAccess() const;

    /** Provides access to the document bookmark interface
     */
    const IDocumentMarkAccess* getIDocumentMarkAccess() const;

    /** Provides access to the document redline interface
     */
    const IDocumentRedlineAccess* getIDocumentRedlineAccess() const;

    /** Provides access to the document style pool interface
     */
    const IDocumentStylePoolAccess* getIDocumentStylePoolAccess() const;

    /** Provides access to the document line number information interface
     */
    const IDocumentLineNumberAccess* getIDocumentLineNumberAccess() const;

    /** Provides access to the document draw model interface
     */
    const IDocumentDrawModelAccess* getIDocumentDrawModelAccess() const;

    /** Provides access to the document layout interface
     */
    const IDocumentLayoutAccess* getIDocumentLayoutAccess() const;
          IDocumentLayoutAccess* getIDocumentLayoutAccess();

    /** Provides access to the document links administration interface
     */
    const IDocumentLinksAdministration* getIDocumentLinksAdministration() const;
          IDocumentLinksAdministration* getIDocumentLinksAdministration();

    /** Provides access to the document fields administration interface
     */
    const IDocumentFieldsAccess* getIDocumentFieldsAccess() const;
          IDocumentFieldsAccess* getIDocumentFieldsAccess();

    /** Provides access to the document content operations interface
     */
          IDocumentContentOperations* getIDocumentContentOperations();

    /** Provides access to the document automatic styles interface
     */
          IStyleAccess& getIDocumentStyleAccess();

    /** Provides access to the document's numbered items interface

        OD 2007-10-31 #i83479#

        @author OD
    */
    IDocumentListItems& getIDocumentListItems();

    // liegt der Node im Sichtbarenbereich der Shell ?
    sal_Bool IsInVisibleArea( ViewShell* pSh = 0 ) const;
    // befindet sich der Node in einem geschuetzten Bereich?
    sal_Bool IsInProtectSect() const;
    // befindet sich der Node in irgendetwas geschuetzten ?
    // (Bereich/Rahmen/Tabellenzellen/... incl. des Ankers bei
    //  Rahmen/Fussnoten/..)
    sal_Bool IsProtect() const;
    // suche den PageDesc, mit dem dieser Node formatiert ist. Wenn das
    // Layout vorhanden ist wird ueber das gesucht, ansonsten gibt es nur
    // die harte Tour ueber die Nodes nach vorne suchen!!
    // OD 18.03.2003 #106326#
    const SwPageDesc* FindPageDesc( sal_Bool bCalcLay, sal_uInt32* pPgDescNdIdx = 0 ) const;

    // falls der Node in einem Fly steht, dann wird das entsprechende Format
    // returnt
    SwFrmFmt* GetFlyFmt() const;

    // falls der Node in einer Tabelle steht, dann wird die entsprechende
    // TabellenBox returnt
    SwTableBox* GetTblBox() const;

    inline sal_uLong GetIndex() const { return GetPos(); }

    const SwTxtNode* FindOutlineNodeOfLevel( sal_uInt8 nLvl ) const;

    sal_uInt8 HasPrevNextLayNode() const;

private:
    // privater Constructor, weil nie kopiert werden darf !!
    SwNode( const SwNode & rNodes );
    SwNode & operator= ( const SwNode & rNodes );
};

// --------------------
// class SwStartNode
// --------------------
class SwStartNode: public SwNode
{
    friend class SwNode;
    friend class SwNodes;
    friend class SwEndNode;     // um theEndOfSection zu setzen !!

    SwEndNode* pEndOfSection;
    SwStartNodeType eSttNdTyp;

    // fuer den initialen StartNode
    SwStartNode( SwNodes& rNodes, sal_uLong nPos );

protected:
    SwStartNode( const SwNodeIndex &rWhere,
                 const sal_uInt8 nNodeType = ND_STARTNODE,
                 SwStartNodeType = SwNormalStartNode );
public:
    DECL_FIXEDMEMPOOL_NEWDEL(SwStartNode)

    SwStartNodeType GetStartNodeType() const        { return eSttNdTyp; }

    // an alle ContentNodes der Section das ChkCondColl rufen
    void CheckSectionCondColl() const;

private:
    // privater Constructor, weil nie kopiert werden darf !!
    SwStartNode( const SwStartNode & rNode );
    SwStartNode & operator= ( const SwStartNode & rNode );
};


// --------------------
// class SwEndNode
// --------------------
class SwEndNode : public SwNode
{
    friend class SwNodes;
    friend class SwTableNode;       // um seinen EndNode anlegen zukoennen
    friend class SwSectionNode;     // um seinen EndNode anlegen zukoennen

    // fuer den initialen StartNode
    SwEndNode( SwNodes& rNodes, sal_uLong nPos, SwStartNode& rSttNd );

protected:
    SwEndNode( const SwNodeIndex &rWhere, SwStartNode& rSttNd );

    DECL_FIXEDMEMPOOL_NEWDEL(SwEndNode)

private:
    // privater Constructor, weil nie kopiert werden darf !!
    SwEndNode( const SwEndNode & rNode );
    SwEndNode & operator= ( const SwEndNode & rNode );
};


// --------------------
// SwCntntNode
// --------------------
class SW_DLLPUBLIC SwCntntNode: public SwModify, public SwNode, public SwIndexReg
{
    // Der Reader darf NewAttrSet() aufrufen!
//  friend class SwSwgReader;
//  friend class Sw3IoImp;

//FEATURE::CONDCOLL
    SwDepend* pCondColl;
//FEATURE::CONDCOLL
    mutable bool mbSetModifyAtAttr;

protected:
    SwCntntNode( const SwNodeIndex &rWhere, const sal_uInt8 nNodeType,
                SwFmtColl *pFmtColl );
    virtual ~SwCntntNode();

    // Attribut-Set fuer alle AUTO-Attribute eines CntntNodes
    //  ( z.B: TxtNode oder NoTxtNode
    boost::shared_ptr<const SfxItemSet> mpAttrSet;

    // lasse von den entsprechenden Nodes die spz. AttrSets anlegen
    virtual void NewAttrSet( SwAttrPool& ) = 0;

    // There some functions that like to remove items from the internal
    // SwAttrSet (handle):
    sal_uInt16 ClearItemsFromAttrSet( const std::vector<sal_uInt16>& rWhichIds );

   virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew);

public:
    TYPEINFO();     //Bereits in Basisklasse Client drin.

    // MakeFrm will be called for a certain layout
    // pSib is another SwFrm of the same layout (e.g. the SwRootFrm itself, a sibling, the parent)
    virtual SwCntntFrm *MakeFrm( SwFrm* pSib ) = 0;

    virtual SwCntntNode *SplitCntntNode(const SwPosition & ) = 0;

    virtual SwCntntNode *JoinNext();
    virtual SwCntntNode *JoinPrev();
    // koennen 2 Nodes zusammengefasst werden ?
    // in pIdx kann die 2. Position returnt werden.
    int CanJoinNext( SwNodeIndex* pIdx =0 ) const;
    int CanJoinPrev( SwNodeIndex* pIdx =0 ) const;

    void MakeStartIndex( SwIndex * pIdx )   { pIdx->Assign( this, 0 ); }
    void MakeEndIndex( SwIndex * pIdx )     { pIdx->Assign( this, Len() ); }

    sal_Bool GoNext(SwIndex *, sal_uInt16 nMode ) const;
    sal_Bool GoPrevious(SwIndex *, sal_uInt16 nMode ) const;

    // Replacement for good old GetFrm(..):
    SwCntntFrm *getLayoutFrm( const SwRootFrm*,
                        const Point* pDocPos = 0,
                        const SwPosition *pPos = 0,
                        const sal_Bool bCalcFrm = sal_True ) const;
    // Gibt die tatsaechlcheGroesse des Frames zurueck bzw. ein leeres
    // Rechteck, wenn kein Layout existiert.
    // Wird fuer die Export Filter benoetigt.
    SwRect FindLayoutRect( const sal_Bool bPrtArea = sal_False,
                            const Point* pPoint = 0,
                            const sal_Bool bCalcFrm = sal_False  ) const;
    SwRect FindPageFrmRect( const sal_Bool bPrtArea = sal_False,
                            const Point* pPoint = 0,
                            const sal_Bool bCalcFrm = sal_False  ) const;
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
    virtual sal_Bool GetInfo( SfxPoolItem& ) const;

    // SS fuer die PoolItems: (Harte-(Fmt)Attrbutierung)

    // Ist bInParent sal_False, wird nur in diesem Node nach dem Attribut gesucht.
    const SfxPoolItem& GetAttr( sal_uInt16 nWhich, sal_Bool bInParent=sal_True ) const;
    sal_Bool GetAttr( SfxItemSet& rSet, sal_Bool bInParent=sal_True ) const;
    // --> OD 2008-03-13 #refactorlists#
    // made virtual
    virtual sal_Bool SetAttr( const SfxPoolItem& );
    virtual sal_Bool SetAttr( const SfxItemSet& rSet );
    virtual sal_Bool ResetAttr( sal_uInt16 nWhich1, sal_uInt16 nWhich2 = 0 );
    virtual sal_Bool ResetAttr( const SvUShorts& rWhichArr );
    virtual sal_uInt16 ResetAllAttr();
    // <--

    // liefert das Attribut, das nicht ueber die bedingte Vorlage kommt!
    const SfxPoolItem* GetNoCondAttr( sal_uInt16 nWhich, sal_Bool bInParents ) const;

    // hat der Node schon eigene Auto-Attribute ?
    // Zugriff auf SwAttrSet
    inline const SwAttrSet &GetSwAttrSet() const;
    inline const SwAttrSet *GetpSwAttrSet() const { return static_cast<const SwAttrSet*>(mpAttrSet.get()); }
    inline sal_Bool  HasSwAttrSet() const { return mpAttrSet ? sal_True : sal_False; }

    virtual SwFmtColl* ChgFmtColl( SwFmtColl* );
    SwFmtColl* GetFmtColl() const { return (SwFmtColl*)GetRegisteredIn(); }

//FEATURE::CONDCOLL
    inline SwFmtColl& GetAnyFmtColl() const;
    void SetCondFmtColl( SwFmtColl* );
    inline SwFmtColl* GetCondFmtColl() const;

    sal_Bool IsAnyCondition( SwCollCondition& rTmp ) const;
    void ChkCondColl();
//FEATURE::CONDCOLL

    // invalidiert die am Node gesetzte NumRule. Diese wird
    // spaetestend in EndAction einer Shell geupdatet
    sal_Bool InvalidateNumRule();

    // --> OD 2005-02-21 #i42921# - determines the text direction for a certain
    // position. Return -1, if text direction could *not* be determined.
    short GetTextDirection( const SwPosition& rPos,
                            const Point* pPt ) const;
    // <--

    inline void SetModifyAtAttr( bool bSetModifyAtAttr ) const { mbSetModifyAtAttr = bSetModifyAtAttr; }
    inline bool GetModifyAtAttr() const { return mbSetModifyAtAttr; }

    static SwOLENodes* CreateOLENodesArray( const SwFmtColl& rColl, bool bOnlyWithInvalidSize );

private:
    // privater Constructor, weil nie kopiert werden darf !!
    SwCntntNode( const SwCntntNode & rNode );
    SwCntntNode & operator= ( const SwCntntNode & rNode );
};



//---------
// SwTableNode
//---------
class SW_DLLPUBLIC SwTableNode : public SwStartNode, public SwModify
{
    friend class SwNodes;
    SwTable* pTable;
protected:
    virtual ~SwTableNode();

public:
    SwTableNode( const SwNodeIndex & );

    const SwTable& GetTable() const { return *pTable; }
    SwTable& GetTable() { return *pTable; }
    SwTabFrm *MakeFrm( SwFrm* );

    //Legt die Frms fuer den TableNode (also die TabFrms) an.
    void MakeFrms( SwNodeIndex* pIdxBehind );

    //Methode loescht fuer den Node alle Ansichten vom
    //Dokument. Die Contentframes werden aus dem entsprechenden
    //Layout ausgehaengt.
    void DelFrms();

    //Methode erzeugt fuer den vorhergehenden Node alle Ansichten vom
    //Dokument. Die erzeugten Contentframes werden in das entsprechende
    //Layout gehaengt.
    void MakeFrms( const SwNodeIndex & rIdx );

    SwTableNode* MakeCopy( SwDoc*, const SwNodeIndex& ) const;
    void SetNewTable( SwTable* , sal_Bool bNewFrames=sal_True );

private:
    // privater Constructor, weil nie kopiert werden darf !!
    SwTableNode( const SwTableNode & rNode );
    SwTableNode & operator= ( const SwTableNode & rNode );
};


//---------
// SwSectionNode
//---------
class SwSectionNode
    : public SwStartNode
    , private ::boost::noncopyable
{
    friend class SwNodes;

private:
    ::std::auto_ptr<SwSection> const m_pSection;

protected:
    virtual ~SwSectionNode();

public:
    SwSectionNode(SwNodeIndex const&,
        SwSectionFmt & rFmt, SwTOXBase const*const pTOXBase);

    const SwSection& GetSection() const { return *m_pSection; }
          SwSection& GetSection()       { return *m_pSection; }

    SwFrm *MakeFrm( SwFrm* );

    //Legt die Frms fuer den SectionNode (also die SectionFrms) an.
    //Im Defaultfall wird bis die Frames bis zum Ende des Bereichs angelegt,
    //uebergibt man einen weiteren NodeIndex pEnd, so wird bis zu diesem
    //ein MakeFrms gerufen, genutzt wird dies von TableToText
    void MakeFrms( SwNodeIndex* pIdxBehind, SwNodeIndex* pEnd = NULL );

    //Methode loescht fuer den Node alle Ansichten vom
    //Dokument. Die Contentframes werden aus dem entsprechenden
    //Layout ausgehaengt.
    void DelFrms();

    //Methode erzeugt fuer den vorhergehenden Node alle Ansichten vom
    //Dokument. Die erzeugten Contentframes werden in das entsprechende
    //Layout gehaengt.
    void MakeFrms( const SwNodeIndex & rIdx );

    SwSectionNode* MakeCopy( SwDoc*, const SwNodeIndex& ) const;

    // setze den Verweis im Format der Section auf sich selbst
    void NodesArrChgd();

    // ueberprueft bei _nicht_ versteckten Bereichen, ob es Inhalt gibt, der
    // _nicht_ in einem versteckten (Unter-)Bereich liegt
    sal_Bool IsCntntHidden() const;

};



// ---------------------- einige inline Methoden ----------------------
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


inline sal_Bool SwNode::IsStartNode() const
{
    return ND_STARTNODE & nNodeType  ? sal_True : sal_False;
}
inline sal_Bool SwNode::IsCntntNode() const
{
    return ND_CONTENTNODE & nNodeType  ? sal_True : sal_False;
}
inline sal_Bool SwNode::IsEndNode() const
{
    return ND_ENDNODE == nNodeType  ? sal_True : sal_False;
}
inline sal_Bool SwNode::IsTxtNode() const
{
    return ND_TEXTNODE == nNodeType  ? sal_True : sal_False;
}
inline sal_Bool SwNode::IsTableNode() const
{
    return ND_TABLENODE == nNodeType  ? sal_True : sal_False;
}
inline sal_Bool SwNode::IsSectionNode() const
{
    return ND_SECTIONNODE == nNodeType  ? sal_True : sal_False;
}
inline sal_Bool SwNode::IsNoTxtNode() const
{
    return ND_NOTXTNODE & nNodeType  ? sal_True : sal_False;
}
inline sal_Bool SwNode::IsOLENode() const
{
    return ND_OLENODE == nNodeType  ? sal_True : sal_False;
}
inline sal_Bool SwNode::IsGrfNode() const
{
    return ND_GRFNODE == nNodeType  ? sal_True : sal_False;
}

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
inline sal_uLong SwNode::StartOfSectionIndex() const
{
    return pStartOfSection->GetIndex();
}
inline sal_uLong SwNode::EndOfSectionIndex() const
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

inline const SwAttrSet& SwCntntNode::GetSwAttrSet() const
{
    return mpAttrSet ? *GetpSwAttrSet() : GetAnyFmtColl().GetAttrSet();
}

//FEATURE::CONDCOLL

inline const SfxPoolItem& SwCntntNode::GetAttr( sal_uInt16 nWhich,
                                                sal_Bool bInParents ) const
{
    return GetSwAttrSet().Get( nWhich, bInParents );
}
#endif
