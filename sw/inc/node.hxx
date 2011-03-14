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
// forward declarations
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

// --------------------
// class SwNode
// --------------------
class SW_DLLPUBLIC SwNode : private /* public*/ BigPtrEntry
{
    friend class SwNodes;

#if OSL_DEBUG_LEVEL > 1
    static long nSerial;
    long nMySerial;
#endif

    sal_uInt8 nNodeType;

    // For text nodes: level of auto format. Was put here because we had still free bits.
    sal_uInt8 nAFmtNumLvl : 3;
    sal_Bool bSetNumLSpace : 1;         // For numbering: TRUE: set indent.
    sal_Bool bIgnoreDontExpand : 1;     // for Text Attributes - ignore the flag

protected:
    SwStartNode* pStartOfSection;

    SwNode( const SwNodeIndex &rWhere, const sal_uInt8 nNodeId );

    // for the initial StartNode
    SwNode( SwNodes& rNodes, sal_uLong nPos, const sal_uInt8 nNodeId );

public:
    virtual ~SwNode();

#if OSL_DEBUG_LEVEL > 1
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

    // Search table node, in which it is. If it is in no table
    // return 0.
                    SwTableNode *FindTableNode();
    inline const  SwTableNode *FindTableNode() const;

    // Search section node, in which it is. If it is in no section
    // return 0.
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

    // Node is in which nodes-array/doc?
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

        @author OD
    */
    IDocumentListItems& getIDocumentListItems();

    // Is node in the visible area of the Shell?
    sal_Bool IsInVisibleArea( ViewShell* pSh = 0 ) const;
    // Is node in an protected area?
    sal_Bool IsInProtectSect() const;
    //  Is node in something that is protected (range, frame,
    //  table cells ... including anchor in case of frames or footnotes)?
    sal_Bool IsProtect() const;

    // Search PageDesc with which this node is formated. If layout is existent
    // search over layout, else only the hard way is left: search over the nodes
    // to the front!!
    const SwPageDesc* FindPageDesc( sal_Bool bCalcLay, sal_uInt32* pPgDescNdIdx = 0 ) const;

    // If node is in a fly return the respective format.
    SwFrmFmt* GetFlyFmt() const;

    // If node is in a table return the respective table box.
    SwTableBox* GetTblBox() const;

    inline sal_uLong GetIndex() const { return GetPos(); }

    const SwTxtNode* FindOutlineNodeOfLevel( sal_uInt8 nLvl ) const;

    sal_uInt8 HasPrevNextLayNode() const;

private:
    // Private constructor because copying is never allowed!!
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
    friend class SwEndNode;     // to set the theEndOfSection !!

    SwEndNode* pEndOfSection;
    SwStartNodeType eSttNdTyp;

    // for the initial StartNode
    SwStartNode( SwNodes& rNodes, sal_uLong nPos );

protected:
    SwStartNode( const SwNodeIndex &rWhere,
                 const sal_uInt8 nNodeType = ND_STARTNODE,
                 SwStartNodeType = SwNormalStartNode );
public:
    DECL_FIXEDMEMPOOL_NEWDEL(SwStartNode)

    SwStartNodeType GetStartNodeType() const        { return eSttNdTyp; }

    // Call ChkCondcoll to all ContentNodes of section.
    void CheckSectionCondColl() const;

private:
    // Private constructor because copying is never allowed!!
    SwStartNode( const SwStartNode & rNode );
    SwStartNode & operator= ( const SwStartNode & rNode );
};


// --------------------
// class SwEndNode
// --------------------
class SwEndNode : public SwNode
{
    friend class SwNodes;
    friend class SwTableNode;       // To enable creation of its EndNote.
    friend class SwSectionNode;     // To enable creation of its EndNote.

    // for the initial StartNode
    SwEndNode( SwNodes& rNodes, sal_uLong nPos, SwStartNode& rSttNd );

protected:
    SwEndNode( const SwNodeIndex &rWhere, SwStartNode& rSttNd );

    DECL_FIXEDMEMPOOL_NEWDEL(SwEndNode)

private:
    // Private constructor because copying is never allowed!!
    SwEndNode( const SwEndNode & rNode );
    SwEndNode & operator= ( const SwEndNode & rNode );
};


// --------------------
// SwCntntNode
// --------------------
class SW_DLLPUBLIC SwCntntNode: public SwModify, public SwNode, public SwIndexReg
{

//FEATURE::CONDCOLL
    SwDepend* pCondColl;
//FEATURE::CONDCOLL
    mutable bool mbSetModifyAtAttr;

protected:
    SwCntntNode( const SwNodeIndex &rWhere, const sal_uInt8 nNodeType,
                SwFmtColl *pFmtColl );
    virtual ~SwCntntNode();

    //  Attribute-set for all auto attributes of a CntntNode.
    //  (e.g. TxtNode or NoTxtNode).
    boost::shared_ptr<const SfxItemSet> mpAttrSet;

    // Make respective nodes create the specific AttrSets.
    virtual void NewAttrSet( SwAttrPool& ) = 0;

    // There some functions that like to remove items from the internal
    // SwAttrSet (handle):
    sal_uInt16 ClearItemsFromAttrSet( const std::vector<sal_uInt16>& rWhichIds );

public:
    TYPEINFO();     //Already contained in base class Client.

    virtual void Modify( SfxPoolItem *pOld, SfxPoolItem *pNew);

    virtual SwCntntFrm *MakeFrm() = 0;
    virtual SwCntntNode *SplitCntntNode(const SwPosition & ) = 0;
    virtual SwCntntNode *JoinNext();
    virtual SwCntntNode *JoinPrev();
    // Is it possible to join two nodes?
    // In pIdx the second position can be returned.
    int CanJoinNext( SwNodeIndex* pIdx =0 ) const;
    int CanJoinPrev( SwNodeIndex* pIdx =0 ) const;

    void MakeStartIndex( SwIndex * pIdx )   { pIdx->Assign( this, 0 ); }
    void MakeEndIndex( SwIndex * pIdx )     { pIdx->Assign( this, Len() ); }

    sal_Bool GoNext(SwIndex *, sal_uInt16 nMode ) const;
    sal_Bool GoPrevious(SwIndex *, sal_uInt16 nMode ) const;

    SwCntntFrm *GetFrm( const Point* pDocPos = 0,
                        const SwPosition *pPos = 0,
                        const sal_Bool bCalcFrm = sal_True ) const;
    // Returns the real size of the frame or an empty rectangle if
    // no layout exists. Needed for export filters.
    SwRect FindLayoutRect( const sal_Bool bPrtArea = sal_False,
                            const Point* pPoint = 0,
                            const sal_Bool bCalcFrm = sal_False  ) const;
    SwRect FindPageFrmRect( const sal_Bool bPrtArea = sal_False,
                            const Point* pPoint = 0,
                            const sal_Bool bCalcFrm = sal_False  ) const;

    // Method creates all views of document for given node. The content
    // frames that are created are put in the respective layout.
    void MakeFrms( SwCntntNode& rNode );

    // Method deletes all vies of document for the node. The content-
    // frames are removed from the respective layout.
    void DelFrms();

    // Returns count of elements of node content. Default is 1.
    // There are differences between text node and formula node.
    virtual xub_StrLen Len() const;

    virtual SwCntntNode* MakeCopy( SwDoc*, const SwNodeIndex& ) const = 0;

    // Get information from Client.
    virtual sal_Bool GetInfo( SfxPoolItem& ) const;

    // SS for PoolItems: hard attributation.

    // If bInParent is FALSE search for attribute only in this node.
    const SfxPoolItem& GetAttr( sal_uInt16 nWhich, sal_Bool bInParent=sal_True ) const;
    sal_Bool GetAttr( SfxItemSet& rSet, sal_Bool bInParent=sal_True ) const;
    // made virtual
    virtual sal_Bool SetAttr( const SfxPoolItem& );
    virtual sal_Bool SetAttr( const SfxItemSet& rSet );
    virtual sal_Bool ResetAttr( sal_uInt16 nWhich1, sal_uInt16 nWhich2 = 0 );
    virtual sal_Bool ResetAttr( const SvUShorts& rWhichArr );
    virtual sal_uInt16 ResetAllAttr();

    // Obtains attribute that is not delivered via conditional style!
    const SfxPoolItem* GetNoCondAttr( sal_uInt16 nWhich, sal_Bool bInParents ) const;

    // Does node has already its own auto-attributes?
    // Access to SwAttrSet.
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

    // Invalidates NumRule at the node. NumRule is updated
    // on EndAction of a Shell at the latest.
    sal_Bool InvalidateNumRule();

    // determines the text direction for a certain
    // position. Return -1, if text direction could *not* be determined.
    short GetTextDirection( const SwPosition& rPos,
                            const Point* pPt ) const;

    inline void SetModifyAtAttr( bool bSetModifyAtAttr ) const { mbSetModifyAtAttr = bSetModifyAtAttr; }
    inline bool GetModifyAtAttr() const { return mbSetModifyAtAttr; }

private:
    // Private constructor because copying is never allowed!!
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
    SwTabFrm *MakeFrm();

    // Creates the frms for the table node (i.e. the TabFrms).
    void MakeFrms( SwNodeIndex* pIdxBehind );

    // Method deletes all views of document for the node.
    // The content frames are removed from the respective layout.
    void DelFrms();

    // Method creates all views of the document for the previous node.
    // The content frames that are created are put into the respective layout.
    void MakeFrms( const SwNodeIndex & rIdx );

    SwTableNode* MakeCopy( SwDoc*, const SwNodeIndex& ) const;
    void SetNewTable( SwTable* , sal_Bool bNewFrames=sal_True );

private:
    // Private constructor because copying is never allowed!!
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

    SwFrm *MakeFrm();

    // Creates the frms for the SectionNode (i.e. the SectionFrms).
    // On default the frames are created until the end of the range.
    // When another NodeIndex pEnd is passed a MakeFrms is called up to it.
    // Used by TableToText.
    void MakeFrms( SwNodeIndex* pIdxBehind, SwNodeIndex* pEnd = NULL );

    // Method deletes all views of document for the node. The
    // content frames are removed from the respective layout.
    void DelFrms();

    // Method creates all views of document for the previous node.
    // The content frames created are put into the respective layout.
    void MakeFrms( const SwNodeIndex & rIdx );

    SwSectionNode* MakeCopy( SwDoc*, const SwNodeIndex& ) const;

    // Set pointer in format of section on itself.
    void NodesArrChgd();

    // ueberprueft bei _nicht_ versteckten Bereichen, ob es Inhalt gibt, der
    // _nicht_ in einem versteckten (Unter-)Bereich liegt
    // Check for not hidden areas whether there is content that is not in
    // a hidden sub-area.
    sal_Bool IsCntntHidden() const;


};



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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
