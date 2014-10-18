/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_SW_INC_NODE_HXX
#define INCLUDED_SW_INC_NODE_HXX

#include <vector>

#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>

#include <sal/types.h>
#include <tools/mempool.hxx>

#include "swdllapi.h"
#include <ndarr.hxx>
#include <ndtyp.hxx>
#include <index.hxx>
#include <fmtcol.hxx>
#include "docary.hxx"

// forward declarations

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
class SwViewShell;
struct SwPosition;
class IStyleAccess;
class IDocumentSettingAccess;
class IDocumentDeviceAccess;
class IDocumentMarkAccess;
class IDocumentRedlineAccess;
class IDocumentStylePoolAccess;
class IDocumentLinksAdministration;
class IDocumentFieldsAccess;
class IDocumentContentOperations;
class IDocumentListItems;
class Point;

//UUUU
namespace drawinglayer { namespace attribute {
    class SdrAllFillAttributesHelper;
    typedef boost::shared_ptr< SdrAllFillAttributesHelper > SdrAllFillAttributesHelperPtr;
}}

/// Base class of the Writer document model elements.
class SW_DLLPUBLIC SwNode
    : private BigPtrEntry
{
    friend class SwNodes;

    sal_uInt8 nNodeType;

    /// For text nodes: level of auto format. Was put here because we had still free bits.
    sal_uInt8 nAFmtNumLvl : 3;
    bool bSetNumLSpace : 1;         ///< For numbering: TRUE: set indent.
    bool bIgnoreDontExpand : 1;     ///< for Text Attributes - ignore the flag

#ifdef DBG_UTIL
    static long s_nSerial;
    long m_nSerial;
#endif

protected:
    SwStartNode* pStartOfSection;

    SwNode( const SwNodeIndex &rWhere, const sal_uInt8 nNodeId );

    /// for the initial StartNode
    SwNode( SwNodes& rNodes, sal_uLong nPos, const sal_uInt8 nNodeId );

public:
    /** the = 0 forces the class to be an abstract base class, but the dtor can be still called
       from subclasses */
    virtual ~SwNode() = 0;

#ifdef DBG_UTIL
    long GetSerial() const { return m_nSerial; }
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

    inline bool IsSetNumLSpace() const  { return bSetNumLSpace; }
    inline void SetNumLSpace( bool bFlag )        { bSetNumLSpace = bFlag; }

    inline bool IsIgnoreDontExpand() const  { return bIgnoreDontExpand; }
    inline void SetIgnoreDontExpand( bool bNew )  { bIgnoreDontExpand = bNew; }

    sal_uInt8   GetNodeType() const { return nNodeType; }

    inline       SwStartNode *GetStartNode();
    inline const SwStartNode *GetStartNode() const;
    inline       SwCntntNode *GetCntntNode();
    inline const SwCntntNode *GetCntntNode() const;
    inline       SwEndNode   *GetEndNode();
    inline const SwEndNode   *GetEndNode() const;
    inline       SwTxtNode   *GetTxtNode();
    inline const SwTxtNode   *GetTxtNode() const;
    inline       SwOLENode   *GetOLENode();
    inline const SwOLENode   *GetOLENode() const;
    inline       SwNoTxtNode *GetNoTxtNode();
    inline const SwNoTxtNode *GetNoTxtNode() const;
    inline       SwGrfNode   *GetGrfNode();
    inline const SwGrfNode   *GetGrfNode() const;
    inline       SwTableNode *GetTableNode();
    inline const SwTableNode *GetTableNode() const;
    inline       SwSectionNode *GetSectionNode();
    inline const SwSectionNode *GetSectionNode() const;

    inline bool IsStartNode() const;
    inline bool IsCntntNode() const;
    inline bool IsEndNode() const;
    inline bool IsTxtNode() const;
    inline bool IsTableNode() const;
    inline bool IsSectionNode() const;
    inline bool IsOLENode() const;
    inline bool IsNoTxtNode() const;
    inline bool IsGrfNode() const;

    /**
       Checks if this node is in redlines.

       @retval sal_True       this node is in redlines
       @retval sal_False      else
     */
    bool IsInRedlines() const;

    /** Search table node, in which it is. If it is in no table
       @return 0. */
                    SwTableNode *FindTableNode();
    inline const  SwTableNode *FindTableNode() const;

    /** Search section node, in which it is. If it is in no section
       @return 0. */
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

    /// Node is in which nodes-array/doc?
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

    /// Is node in the visible area of the Shell?
    bool IsInVisibleArea( SwViewShell const * pSh = 0 ) const;
    /// Is node in an protected area?
    bool IsInProtectSect() const;
    /**  Is node in something that is protected (range, frame,
        table cells ... including anchor in case of frames or footnotes)? */
    bool IsProtect() const;

    /** Search PageDesc with which this node is formated. If layout is existent
       search over layout, else only the hard way is left: search over the nodes
       to the front!! */
    const SwPageDesc* FindPageDesc( bool bCalcLay, sal_uInt32* pPgDescNdIdx = 0 ) const;

    /// If node is in a fly return the respective format.
    SwFrmFmt* GetFlyFmt() const;

    /// If node is in a table return the respective table box.
    SwTableBox* GetTblBox() const;

    inline sal_uLong GetIndex() const { return GetPos(); }

    const SwTxtNode* FindOutlineNodeOfLevel( sal_uInt8 nLvl ) const;

    sal_uInt8 HasPrevNextLayNode() const;

    /**
     * Dumps the node structure to the given destination (file nodes.xml in the current directory by default)
     * @since 3.5
     */
    virtual void dumpAsXml( xmlTextWriterPtr writer = NULL ) const;

private:
    /// Private constructor because copying is never allowed!!
    SwNode( const SwNode & rNodes );
    SwNode & operator= ( const SwNode & rNodes );
};

/// Starts a section of nodes in the document model.
class SwStartNode: public SwNode
{
    friend class SwNode;
    friend class SwNodes;
    friend class SwEndNode;     ///< to set the theEndOfSection !!

    SwEndNode* pEndOfSection;
    SwStartNodeType eSttNdTyp;

    /// for the initial StartNode
    SwStartNode( SwNodes& rNodes, sal_uLong nPos );

protected:
    SwStartNode( const SwNodeIndex &rWhere,
                 const sal_uInt8 nNodeType = ND_STARTNODE,
                 SwStartNodeType = SwNormalStartNode );
public:
    DECL_FIXEDMEMPOOL_NEWDEL(SwStartNode)

    SwStartNodeType GetStartNodeType() const        { return eSttNdTyp; }

    /// Call ChkCondcoll to all ContentNodes of section.
    void CheckSectionCondColl() const;

    virtual void dumpAsXml( xmlTextWriterPtr writer = NULL ) const SAL_OVERRIDE;

private:
    /// Private constructor because copying is never allowed!!
    SwStartNode( const SwStartNode & rNode );
    SwStartNode & operator= ( const SwStartNode & rNode );
};

/// Ends a section of nodes in the document model.
class SwEndNode : public SwNode
{
    friend class SwNodes;
    friend class SwTableNode;       ///< To enable creation of its EndNote.
    friend class SwSectionNode;     ///< To enable creation of its EndNote.

    /// for the initial StartNode
    SwEndNode( SwNodes& rNodes, sal_uLong nPos, SwStartNode& rSttNd );

protected:
    SwEndNode( const SwNodeIndex &rWhere, SwStartNode& rSttNd );

    DECL_FIXEDMEMPOOL_NEWDEL(SwEndNode)

private:
    /// Private constructor because copying is never allowed!!
    SwEndNode( const SwEndNode & rNode );
    SwEndNode & operator= ( const SwEndNode & rNode );
};

// SwCntntNode

class SW_DLLPUBLIC SwCntntNode: public SwModify, public SwNode, public SwIndexReg
{

//FEATURE::CONDCOLL
    SwDepend* pCondColl;
//FEATURE::CONDCOLL
    mutable bool mbSetModifyAtAttr;

protected:
    SwCntntNode( const SwNodeIndex &rWhere, const sal_uInt8 nNodeType,
                SwFmtColl *pFmtColl );
    /** the = 0 forces the class to be an abstract base class, but the dtor can be still called
       from subclasses */
    virtual ~SwCntntNode() = 0;

    /**  Attribute-set for all auto attributes of a CntntNode.
      (e.g. TxtNode or NoTxtNode). */
    boost::shared_ptr<const SfxItemSet> mpAttrSet;

    /// Make respective nodes create the specific AttrSets.
    virtual void NewAttrSet( SwAttrPool& ) = 0;

    /** There some functions that like to remove items from the internal
       SwAttrSet (handle): */
    sal_uInt16 ClearItemsFromAttrSet( const std::vector<sal_uInt16>& rWhichIds );

   virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew) SAL_OVERRIDE;

public:
    TYPEINFO_OVERRIDE();     /// Already contained in base class Client.

    /** MakeFrm will be called for a certain layout
       pSib is another SwFrm of the same layout (e.g. the SwRootFrm itself, a sibling, the parent) */
    virtual SwCntntFrm *MakeFrm( SwFrm* pSib ) = 0;

    virtual SwCntntNode *SplitCntntNode(const SwPosition & ) = 0;

    virtual SwCntntNode *JoinNext();
    virtual SwCntntNode *JoinPrev();
    /** Is it possible to join two nodes?
       In pIdx the second position can be returned. */
    bool CanJoinNext( SwNodeIndex* pIdx =0 ) const;
    bool CanJoinPrev( SwNodeIndex* pIdx =0 ) const;

    void MakeStartIndex( SwIndex * pIdx )   { pIdx->Assign( this, 0 ); }
    void MakeEndIndex( SwIndex * pIdx )     { pIdx->Assign( this, Len() ); }

    bool GoNext(SwIndex *, sal_uInt16 nMode ) const;
    bool GoPrevious(SwIndex *, sal_uInt16 nMode ) const;

    /// Replacement for good old GetFrm(..):
    SwCntntFrm *getLayoutFrm( const SwRootFrm*,
                        const Point* pDocPos = 0,
                        const SwPosition *pPos = 0,
                        const bool bCalcFrm = true ) const;
    /** @return the real size of the frame or an empty rectangle if
       no layout exists. Needed for export filters. */
    SwRect FindLayoutRect( const bool bPrtArea = false,
                            const Point* pPoint = 0,
                            const bool bCalcFrm = false  ) const;
    SwRect FindPageFrmRect( const bool bPrtArea = false,
                            const Point* pPoint = 0,
                            const bool bCalcFrm = false  ) const;

    /** Method creates all views of document for given node. The content
       frames that are created are put in the respective layout. */
    void MakeFrms( SwCntntNode& rNode );

    /** Method deletes all views of document for the node. The content-
        frames are removed from the respective layout.

        Add an input param to identify if acc table should be disposed
    */
    void DelFrms( bool bIsAccTableDispose = true );

    /** @return count of elements of node content. Default is 1.
       There are differences between text node and formula node. */
    virtual sal_Int32 Len() const;

    virtual SwCntntNode* MakeCopy( SwDoc*, const SwNodeIndex& ) const = 0;

    /// Get information from Client.
    virtual bool GetInfo( SfxPoolItem& ) const SAL_OVERRIDE;

    /// SS for PoolItems: hard attributation.

    /// If bInParent is FALSE search for attribute only in this node.
    const SfxPoolItem& GetAttr( sal_uInt16 nWhich, bool bInParent=true ) const;
    bool GetAttr( SfxItemSet& rSet, bool bInParent=true ) const;
    /// made virtual
    virtual bool SetAttr( const SfxPoolItem& );
    virtual bool SetAttr( const SfxItemSet& rSet );
    virtual bool ResetAttr( sal_uInt16 nWhich1, sal_uInt16 nWhich2 = 0 );
    virtual bool ResetAttr( const std::vector<sal_uInt16>& rWhichArr );
    virtual sal_uInt16 ResetAllAttr();

    /// Obtains attribute that is not delivered via conditional style!
    const SfxPoolItem* GetNoCondAttr( sal_uInt16 nWhich, bool bInParents ) const;

    /** Does node has already its own auto-attributes?
     Access to SwAttrSet. */
    inline const SwAttrSet &GetSwAttrSet() const;
    inline const SwAttrSet *GetpSwAttrSet() const { return static_cast<const SwAttrSet*>(mpAttrSet.get()); }
    inline bool  HasSwAttrSet() const { return mpAttrSet ? sal_True : sal_False; }

    virtual SwFmtColl* ChgFmtColl( SwFmtColl* );
    SwFmtColl* GetFmtColl() const { return (SwFmtColl*)GetRegisteredIn(); }

//FEATURE::CONDCOLL
    inline SwFmtColl& GetAnyFmtColl() const;
    void SetCondFmtColl( SwFmtColl* );
    inline SwFmtColl* GetCondFmtColl() const;

    bool IsAnyCondition( SwCollCondition& rTmp ) const;
    void ChkCondColl();
//FEATURE::CONDCOLL

    /** Invalidates NumRule at the node. NumRule is updated
       on EndAction of a Shell at the latest. */
    bool InvalidateNumRule();

    /** determines the text direction for a certain
       position. @return -1, if text direction could *not* be determined. */
    short GetTextDirection( const SwPosition& rPos,
                            const Point* pPt ) const;

    inline void SetModifyAtAttr( bool bSetModifyAtAttr ) const { mbSetModifyAtAttr = bSetModifyAtAttr; }
    inline bool GetModifyAtAttr() const { return mbSetModifyAtAttr; }

    static SwOLENodes* CreateOLENodesArray( const SwFmtColl& rColl, bool bOnlyWithInvalidSize );

    //UUUU Access to DrawingLayer FillAttributes in a preprocessed form for primitive usage
    virtual drawinglayer::attribute::SdrAllFillAttributesHelperPtr getSdrAllFillAttributesHelper() const;

private:
    /// Private constructor because copying is never allowed!!
    SwCntntNode( const SwCntntNode & rNode );
    SwCntntNode & operator= ( const SwCntntNode & rNode );
};

// SwTableNode

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

    /// Creates the frms for the table node (i.e. the TabFrms).
    void MakeFrms( SwNodeIndex* pIdxBehind );

    /** Method deletes all views of document for the node.
       The content frames are removed from the respective layout. */
    void DelFrms();

    /** Method creates all views of the document for the previous node.
       The content frames that are created are put into the respective layout. */
    void MakeFrms( const SwNodeIndex & rIdx );

    SwTableNode* MakeCopy( SwDoc*, const SwNodeIndex& ) const;
    void SetNewTable( SwTable* , bool bNewFrames=true );

    // Removes redline objects that relate to this table from the 'Extra Redlines' table
    void RemoveRedlines();

private:
    /// Private constructor because copying is never allowed!!
    SwTableNode( const SwTableNode & rNode );
    SwTableNode & operator= ( const SwTableNode & rNode );
};

class SwSectionNode
    : public SwStartNode
    , private ::boost::noncopyable
{
    friend class SwNodes;

private:
    ::std::unique_ptr<SwSection> const m_pSection;

protected:
    virtual ~SwSectionNode();

public:
    SwSectionNode(SwNodeIndex const&,
        SwSectionFmt & rFmt, SwTOXBase const*const pTOXBase);

    const SwSection& GetSection() const { return *m_pSection; }
          SwSection& GetSection()       { return *m_pSection; }

    SwFrm *MakeFrm( SwFrm* );

    /** Creates the frms for the SectionNode (i.e. the SectionFrms).
       On default the frames are created until the end of the range.
       When another NodeIndex pEnd is passed a MakeFrms is called up to it.
       Used by TableToText. */
    void MakeFrms( SwNodeIndex* pIdxBehind, SwNodeIndex* pEnd = NULL );

    /** Method deletes all views of document for the node. The
     content frames are removed from the respective layout. */
    void DelFrms();

    /** Method creates all views of document for the previous node.
       The content frames created are put into the respective layout. */
    void MakeFrms( const SwNodeIndex & rIdx );

    SwSectionNode* MakeCopy( SwDoc*, const SwNodeIndex& ) const;

    /// Set pointer in format of section on itself.
    void NodesArrChgd();

    /** Check for not hidden areas whether there is content that is not in
       a hidden sub-area. */
    bool IsCntntHidden() const;

};

/** This class is internal. And quite frankly I don't know what ND_SECTIONDUMMY is for,
   the class has been merely created to replace "SwNode( ND_SECTIONDUMMY )", the only case
   of instantiating SwNode directly. Now SwNode can be an abstract base class. */
class SwDummySectionNode
    : private SwNode
{
private:
    friend class SwNodes;
    SwDummySectionNode( const SwNodeIndex &rWhere );
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

inline bool SwNode::IsStartNode() const
{
    return ND_STARTNODE & nNodeType  ? sal_True : sal_False;
}
inline bool SwNode::IsCntntNode() const
{
    return ND_CONTENTNODE & nNodeType  ? sal_True : sal_False;
}
inline bool SwNode::IsEndNode() const
{
    return ND_ENDNODE == nNodeType  ? sal_True : sal_False;
}
inline bool SwNode::IsTxtNode() const
{
    return ND_TEXTNODE == nNodeType  ? sal_True : sal_False;
}
inline bool SwNode::IsTableNode() const
{
    return ND_TABLENODE == nNodeType  ? sal_True : sal_False;
}
inline bool SwNode::IsSectionNode() const
{
    return ND_SECTIONNODE == nNodeType  ? sal_True : sal_False;
}
inline bool SwNode::IsNoTxtNode() const
{
    return ND_NOTXTNODE & nNodeType  ? sal_True : sal_False;
}
inline bool SwNode::IsOLENode() const
{
    return ND_OLENODE == nNodeType  ? sal_True : sal_False;
}
inline bool SwNode::IsGrfNode() const
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
                                                bool bInParents ) const
{
    return GetSwAttrSet().Get( nWhich, bInParents );
}

inline SwDummySectionNode::SwDummySectionNode( const SwNodeIndex &rWhere )
    : SwNode( rWhere, ND_SECTIONDUMMY )
{
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
