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

#include <sal/types.h>

#include "swdllapi.h"
#include "ndarr.hxx"
#include "ndtyp.hxx"
#include "index.hxx"
#include "fmtcol.hxx"

#include <memory>
#include <vector>

class SwContentFrame;
class SwContentNode;
class SwDoc;
class SwEndNode;
class SwFrame;
class SwFrameFormat;
class SwGrfNode;
class SwNoTextNode;
class SwNodeIndex;
class SwOLENode;
class SwRect;
class SwSection;
class SwSectionFormat;
class SwTOXBase;
class SwSectionNode;
class SwStartNode;
class SwTabFrame;
class SwRootFrame;
class SwTable;
class SwTableNode;
class SwTableBox;
class SwTextNode;
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
enum class SvxFrameDirection;
typedef std::vector<SwOLENode*> SwOLENodes; // docary.hxx

namespace drawinglayer { namespace attribute {
    class SdrAllFillAttributesHelper;
    typedef std::shared_ptr< SdrAllFillAttributesHelper > SdrAllFillAttributesHelperPtr;
}}

/// Base class of the Writer document model elements.
class SW_DLLPUBLIC SwNode
    : private BigPtrEntry
{
    friend class SwNodes;

    SwNodeType const m_nNodeType;

    /// For text nodes: level of auto format. Was put here because we had still free bits.
    sal_uInt8 m_nAFormatNumLvl : 3;
    bool m_bIgnoreDontExpand : 1;     ///< for Text Attributes - ignore the flag

public:
    /// sw_redlinehide: redline node merge state
    enum class Merge { None, First, NonFirst, Hidden };
    bool IsCreateFrameWhenHidingRedlines() {
        return m_eMerge == Merge::None || m_eMerge == Merge::First;
    }
    void SetRedlineMergeFlag(Merge const eMerge) { m_eMerge = eMerge; }
    Merge GetRedlineMergeFlag() const { return m_eMerge; }
private:
    Merge m_eMerge;

#ifdef DBG_UTIL
    static long s_nSerial;
    long const m_nSerial;
#endif

    /// all SwFrameFormat that are anchored at the node
    /// invariant: SwFrameFormat is in the list iff
    /// SwFrameFormat::GetAnchor().GetContentAnchor() points to this node
    std::unique_ptr<std::vector<SwFrameFormat*>> m_pAnchoredFlys;

protected:
    SwStartNode* m_pStartOfSection;

    SwNode( const SwNodeIndex &rWhere, const SwNodeType nNodeId );

    /// for the initial StartNode
    SwNode( SwNodes& rNodes, sal_uLong nPos, const SwNodeType nNodeId );

public:
    /** the = 0 forces the class to be an abstract base class, but the dtor can be still called
       from subclasses */
    virtual ~SwNode() override = 0;

#ifdef DBG_UTIL
    long GetSerial() const { return m_nSerial; }
#endif

    sal_uInt16 GetSectionLevel() const;

    inline sal_uLong StartOfSectionIndex() const;
    const SwStartNode* StartOfSectionNode() const { return m_pStartOfSection; }
    SwStartNode* StartOfSectionNode() { return m_pStartOfSection; }

    inline sal_uLong EndOfSectionIndex() const;
    inline const SwEndNode* EndOfSectionNode() const;
    inline         SwEndNode* EndOfSectionNode();

    sal_uInt8 GetAutoFormatLvl() const     { return m_nAFormatNumLvl; }
    void SetAutoFormatLvl( sal_uInt8 nVal )      { m_nAFormatNumLvl = nVal; }

    bool IsIgnoreDontExpand() const  { return m_bIgnoreDontExpand; }
    void SetIgnoreDontExpand( bool bNew )  { m_bIgnoreDontExpand = bNew; }

    SwNodeType   GetNodeType() const { return m_nNodeType; }

    inline       SwStartNode *GetStartNode();
    inline const SwStartNode *GetStartNode() const;
    inline       SwContentNode *GetContentNode();
    inline const SwContentNode *GetContentNode() const;
    inline       SwEndNode   *GetEndNode();
    inline const SwEndNode   *GetEndNode() const;
    inline       SwTextNode   *GetTextNode();
    inline const SwTextNode   *GetTextNode() const;
    inline       SwOLENode   *GetOLENode();
    inline const SwOLENode   *GetOLENode() const;
    inline       SwNoTextNode *GetNoTextNode();
    inline const SwNoTextNode *GetNoTextNode() const;
    inline       SwGrfNode   *GetGrfNode();
    inline const SwGrfNode   *GetGrfNode() const;
    inline       SwTableNode *GetTableNode();
    inline const SwTableNode *GetTableNode() const;
    inline       SwSectionNode *GetSectionNode();
    inline const SwSectionNode *GetSectionNode() const;

    inline bool IsStartNode() const;
    inline bool IsContentNode() const;
    inline bool IsEndNode() const;
    inline bool IsTextNode() const;
    inline bool IsTableNode() const;
    inline bool IsSectionNode() const;
    inline bool IsOLENode() const;
    inline bool IsNoTextNode() const;
    inline bool IsGrfNode() const;

    /**
       Checks if this node is in redlines.

       @retval true       this node is in redlines
       @retval false      else
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
    const IDocumentDeviceAccess& getIDocumentDeviceAccess() const;

    /** Provides access to the document bookmark interface
     */
    const IDocumentMarkAccess* getIDocumentMarkAccess() const;

    /** Provides access to the document redline interface
     */
    const IDocumentRedlineAccess& getIDocumentRedlineAccess() const;

    /** Provides access to the document style pool interface
     */
    const IDocumentStylePoolAccess& getIDocumentStylePoolAccess() const;

    /** Provides access to the document draw model interface
     */
    const IDocumentDrawModelAccess& getIDocumentDrawModelAccess() const;

    /** Provides access to the document layout interface
     */
    const IDocumentLayoutAccess& getIDocumentLayoutAccess() const;
          IDocumentLayoutAccess& getIDocumentLayoutAccess();

    /** Provides access to the document links administration interface
     */
    const IDocumentLinksAdministration& getIDocumentLinksAdministration() const;
          IDocumentLinksAdministration& getIDocumentLinksAdministration();

    /** Provides access to the document fields administration interface
     */
    const IDocumentFieldsAccess& getIDocumentFieldsAccess() const;
          IDocumentFieldsAccess& getIDocumentFieldsAccess();

    /** Provides access to the document content operations interface
     */
          IDocumentContentOperations& getIDocumentContentOperations();

    /** Provides access to the document automatic styles interface
     */
          IStyleAccess& getIDocumentStyleAccess();

    /** Provides access to the document's numbered items interface */
    IDocumentListItems& getIDocumentListItems();

    /// Is node in the visible area of the Shell?
    bool IsInVisibleArea( SwViewShell const * pSh ) const;
    /// Is node in an protected area?
    bool IsInProtectSect() const;
    /**  Is node in something that is protected (range, frame,
        table cells ... including anchor in case of frames or footnotes)? */
    bool IsProtect() const;

    /** Search PageDesc with which this node is formatted. If layout is existent
       search over layout, else only the hard way is left: search over the nodes
       to the front!! */
    const SwPageDesc* FindPageDesc( size_t* pPgDescNdIdx = nullptr ) const;

    /// If node is in a fly return the respective format.
    SwFrameFormat* GetFlyFormat() const;

    /// If node is in a table return the respective table box.
    SwTableBox* GetTableBox() const;

    sal_uLong GetIndex() const { return GetPos(); }

    const SwTextNode* FindOutlineNodeOfLevel(sal_uInt8 nLvl, SwRootFrame const* pLayout = nullptr) const;

    sal_uInt8 HasPrevNextLayNode() const;

    std::vector<SwFrameFormat *> const* GetAnchoredFlys() const { return m_pAnchoredFlys.get(); }
    void AddAnchoredFly(SwFrameFormat *);
    void RemoveAnchoredFly(SwFrameFormat *);

    /**
     * Dumps the node structure to the given destination (file nodes.xml in the current directory by default)
     */
    virtual void dumpAsXml(xmlTextWriterPtr pWriter) const;

private:
    SwNode( const SwNode & rNodes ) = delete;
    SwNode & operator= ( const SwNode & rNodes ) = delete;
};

/// Starts a section of nodes in the document model.
class SAL_DLLPUBLIC_RTTI SwStartNode: public SwNode
{
    friend class SwNode;
    friend class SwNodes;
    friend class SwEndNode;     ///< to set the theEndOfSection !!

    SwEndNode* m_pEndOfSection;
    SwStartNodeType const m_eStartNodeType;

    /// for the initial StartNode
    SwStartNode( SwNodes& rNodes, sal_uLong nPos );

protected:
    SwStartNode( const SwNodeIndex &rWhere,
                 const SwNodeType nNodeType = SwNodeType::Start,
                 SwStartNodeType = SwNormalStartNode );
public:
    SwStartNodeType GetStartNodeType() const        { return m_eStartNodeType; }

    /// Call ChkCondcoll to all ContentNodes of section.
    void CheckSectionCondColl() const;

    virtual void dumpAsXml(xmlTextWriterPtr pWriter) const override;

private:
    SwStartNode( const SwStartNode & rNode ) = delete;
    SwStartNode & operator= ( const SwStartNode & rNode ) = delete;
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

private:
    SwEndNode( const SwEndNode & rNode ) = delete;
    SwEndNode & operator= ( const SwEndNode & rNode ) = delete;
};

// SwContentNode

class SW_DLLPUBLIC SwContentNode: public SwModify, public SwNode, public SwIndexReg
{

    sw::WriterMultiListener m_aCondCollListener;
    SwFormatColl* m_pCondColl;
    mutable bool mbSetModifyAtAttr;

protected:
    SwContentNode( const SwNodeIndex &rWhere, const SwNodeType nNodeType,
                SwFormatColl *pFormatColl );
    /** the = 0 forces the class to be an abstract base class, but the dtor can be still called
       from subclasses */
    virtual ~SwContentNode() override = 0;

    /**  Attribute-set for all auto attributes of a ContentNode.
      (e.g. TextNode or NoTextNode). */
    std::shared_ptr<const SfxItemSet> mpAttrSet;

    /// Make respective nodes create the specific AttrSets.
    virtual void NewAttrSet( SwAttrPool& ) = 0;

    /** There some functions that like to remove items from the internal
       SwAttrSet (handle): */
    sal_uInt16 ClearItemsFromAttrSet( const std::vector<sal_uInt16>& rWhichIds );

    virtual void SwClientNotify( const SwModify&, const SfxHint& rHint) override;

public:

    /** MakeFrame will be called for a certain layout
       pSib is another SwFrame of the same layout (e.g. the SwRootFrame itself, a sibling, the parent) */
    virtual SwContentFrame *MakeFrame( SwFrame* pSib ) = 0;

    virtual SwContentNode *JoinNext();
    /** Is it possible to join two nodes?
       In pIdx the second position can be returned. */
    bool CanJoinNext( SwNodeIndex* pIdx =nullptr ) const;
    bool CanJoinPrev( SwNodeIndex* pIdx =nullptr ) const;

    void MakeStartIndex( SwIndex * pIdx )   { pIdx->Assign( this, 0 ); }
    void MakeEndIndex( SwIndex * pIdx )     { pIdx->Assign( this, Len() ); }

    bool GoNext(SwIndex *, sal_uInt16 nMode ) const;
    bool GoPrevious(SwIndex *, sal_uInt16 nMode ) const;

    /// @see GetFrameOfModify
    SwContentFrame *getLayoutFrame( const SwRootFrame*,
            const SwPosition *pPos = nullptr,
            std::pair<Point, bool> const* pViewPosAndCalcFrame = nullptr) const;
    /** @return the real size of the frame or an empty rectangle if
       no layout exists. Needed for export filters. */
    SwRect FindLayoutRect( const bool bPrtArea = false,
                            const Point* pPoint = nullptr  ) const;
    SwRect FindPageFrameRect() const;

    /** Method creates all views of document for given node. The content
       frames that are created are put in the respective layout. */
    void MakeFramesForAdjacentContentNode(SwContentNode& rNode);

    /** Method deletes all views of document for the node. The content-
        frames are removed from the respective layout.
    */
    void DelFrames(SwRootFrame const* pLayout);

    /** @return count of elements of node content. Default is 1.
       There are differences between text node and formula node. */
    virtual sal_Int32 Len() const;

    virtual SwContentNode* MakeCopy(SwDoc*, const SwNodeIndex&, bool bNewFrames) const = 0;

    /// Get information from Client.
    virtual bool GetInfo( SfxPoolItem& ) const override;

    /// SS for PoolItems: hard attributation.

    /// If bInParent is FALSE search for attribute only in this node.
    const SfxPoolItem& GetAttr( sal_uInt16 nWhich, bool bInParent=true ) const;
    bool GetAttr( SfxItemSet& rSet ) const;
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
    const SwAttrSet *GetpSwAttrSet() const { return static_cast<const SwAttrSet*>(mpAttrSet.get()); }
    bool  HasSwAttrSet() const { return mpAttrSet != nullptr; }

    virtual SwFormatColl* ChgFormatColl( SwFormatColl* );
    SwFormatColl* GetFormatColl() const { return const_cast<SwFormatColl*>(static_cast<const SwFormatColl*>(GetRegisteredIn())); }

//FEATURE::CONDCOLL
    inline SwFormatColl& GetAnyFormatColl() const;
    void SetCondFormatColl( SwFormatColl* );
    inline SwFormatColl* GetCondFormatColl() const;

    bool IsAnyCondition( SwCollCondition& rTmp ) const;
    void ChkCondColl();
//FEATURE::CONDCOLL

    /** Invalidates NumRule at the node. NumRule is updated
       on EndAction of a Shell at the latest. */
    bool InvalidateNumRule();

    /** determines the text direction for a certain
       position. @return -1, if text direction could *not* be determined. */
    SvxFrameDirection GetTextDirection( const SwPosition& rPos,
                            const Point* pPt ) const;

    void SetModifyAtAttr( bool bSetModifyAtAttr ) const { mbSetModifyAtAttr = bSetModifyAtAttr; }
    bool GetModifyAtAttr() const { return mbSetModifyAtAttr; }

    static std::unique_ptr<SwOLENodes> CreateOLENodesArray( const SwFormatColl& rColl, bool bOnlyWithInvalidSize );

    // Access to DrawingLayer FillAttributes in a preprocessed form for primitive usage
    virtual drawinglayer::attribute::SdrAllFillAttributesHelperPtr getSdrAllFillAttributesHelper() const;

    virtual void ModifyNotification(const SfxPoolItem* pOld, const SfxPoolItem* pNew) override
    {
        SwClientNotify(*this, sw::LegacyModifyHint(pOld, pNew));
    }

private:
    SwContentNode( const SwContentNode & rNode ) = delete;
    SwContentNode & operator= ( const SwContentNode & rNode ) = delete;
};

// SwTableNode

class SW_DLLPUBLIC SwTableNode : public SwStartNode, public SwModify
{
    friend class SwNodes;
    std::unique_ptr<SwTable> m_pTable;
protected:
    virtual ~SwTableNode() override;

public:
    SwTableNode( const SwNodeIndex & );

    const SwTable& GetTable() const { return *m_pTable; }
    SwTable& GetTable() { return *m_pTable; }
    SwTabFrame *MakeFrame( SwFrame* );

    /// Creates the frms for the table node (i.e. the TabFrames).
    void MakeOwnFrames(SwNodeIndex* pIdxBehind);

    /** Method deletes all views of document for the node.
       The content frames are removed from the respective layout. */
    void DelFrames(SwRootFrame const* pLayout = nullptr);

    /** Method creates all views of the document for the previous node.
       The content frames that are created are put into the respective layout. */
    void MakeFramesForAdjacentContentNode(const SwNodeIndex & rIdx);

    SwTableNode* MakeCopy( SwDoc*, const SwNodeIndex& ) const;
    void SetNewTable( std::unique_ptr<SwTable> , bool bNewFrames=true );

    // Removes redline objects that relate to this table from the 'Extra Redlines' table
    void RemoveRedlines();

private:
    SwTableNode( const SwTableNode & rNode ) = delete;
    SwTableNode & operator= ( const SwTableNode & rNode ) = delete;
};

class SwSectionNode
    : public SwStartNode
{
    friend class SwNodes;

private:
    SwSectionNode(const SwSectionNode&) = delete;
    SwSectionNode& operator=(const SwSectionNode&) = delete;

    std::unique_ptr<SwSection> const m_pSection;

protected:
    virtual ~SwSectionNode() override;

public:
    SwSectionNode(SwNodeIndex const&,
        SwSectionFormat & rFormat, SwTOXBase const*const pTOXBase);

    const SwSection& GetSection() const { return *m_pSection; }
          SwSection& GetSection()       { return *m_pSection; }

    SwFrame *MakeFrame( SwFrame* );

    /** Creates the frms for the SectionNode (i.e. the SectionFrames).
       On default the frames are created until the end of the range.
       When another NodeIndex pEnd is passed a MakeFrames is called up to it.
       Used by TableToText. */
    void MakeOwnFrames(SwNodeIndex* pIdxBehind, SwNodeIndex* pEnd = nullptr);

    /** Method deletes all views of document for the node. The
     content frames are removed from the respective layout. */
    void DelFrames(SwRootFrame const* pLayout = nullptr);

    /** Method creates all views of document for the previous node.
       The content frames created are put into the respective layout. */
    void MakeFramesForAdjacentContentNode(const SwNodeIndex & rIdx);

    SwSectionNode* MakeCopy( SwDoc*, const SwNodeIndex& ) const;

    /// Set pointer in format of section on itself.
    void NodesArrChgd();

    /** Check for not hidden areas whether there is content that is not in
       a hidden sub-area. */
    bool IsContentHidden() const;

};

/** This class is internal, used only during DocumentContentOperationsManager::CopyWithFlyInFly(), and for undo.

Some of the nodes are then replaced with SwPlaceholderNode, and at the end of the operation, removed again.
FIXME find out if this is really necessary, and if we can avoid creation of the SwPlaceholderNodes in the first place.
*/
class SwPlaceholderNode : private SwNode
{
private:
    friend class SwNodes;
    SwPlaceholderNode(const SwNodeIndex &rWhere);
};

inline       SwEndNode   *SwNode::GetEndNode()
{
     return SwNodeType::End == m_nNodeType ? static_cast<SwEndNode*>(this) : nullptr;
}
inline const SwEndNode   *SwNode::GetEndNode() const
{
     return SwNodeType::End == m_nNodeType ? static_cast<const SwEndNode*>(this) : nullptr;
}
inline       SwStartNode *SwNode::GetStartNode()
{
     return SwNodeType::Start & m_nNodeType ? static_cast<SwStartNode*>(this) : nullptr;
}
inline const SwStartNode *SwNode::GetStartNode() const
{
     return SwNodeType::Start & m_nNodeType ? static_cast<const SwStartNode*>(this) : nullptr;
}
inline       SwTableNode *SwNode::GetTableNode()
{
     return SwNodeType::Table == m_nNodeType ? static_cast<SwTableNode*>(this) : nullptr;
}
inline const SwTableNode *SwNode::GetTableNode() const
{
     return SwNodeType::Table == m_nNodeType ? static_cast<const SwTableNode*>(this) : nullptr;
}
inline       SwSectionNode *SwNode::GetSectionNode()
{
     return SwNodeType::Section == m_nNodeType ? static_cast<SwSectionNode*>(this) : nullptr;
}
inline const SwSectionNode *SwNode::GetSectionNode() const
{
     return SwNodeType::Section == m_nNodeType ? static_cast<const SwSectionNode*>(this) : nullptr;
}
inline       SwContentNode *SwNode::GetContentNode()
{
     return SwNodeType::ContentMask & m_nNodeType ? static_cast<SwContentNode*>(this) : nullptr;
}
inline const SwContentNode *SwNode::GetContentNode() const
{
     return SwNodeType::ContentMask & m_nNodeType ? static_cast<const SwContentNode*>(this) : nullptr;
}

inline bool SwNode::IsStartNode() const
{
    return bool(SwNodeType::Start & m_nNodeType);
}
inline bool SwNode::IsContentNode() const
{
    return bool(SwNodeType::ContentMask & m_nNodeType);
}
inline bool SwNode::IsEndNode() const
{
    return SwNodeType::End == m_nNodeType;
}
inline bool SwNode::IsTextNode() const
{
    return SwNodeType::Text == m_nNodeType;
}
inline bool SwNode::IsTableNode() const
{
    return SwNodeType::Table == m_nNodeType;
}
inline bool SwNode::IsSectionNode() const
{
    return SwNodeType::Section == m_nNodeType;
}
inline bool SwNode::IsNoTextNode() const
{
    return bool(SwNodeType::NoTextMask & m_nNodeType);
}
inline bool SwNode::IsOLENode() const
{
    return SwNodeType::Ole == m_nNodeType;
}
inline bool SwNode::IsGrfNode() const
{
    return SwNodeType::Grf == m_nNodeType;
}

inline const SwStartNode* SwNode::FindSttNodeByType( SwStartNodeType eTyp ) const
{
    return const_cast<SwNode*>(this)->FindSttNodeByType( eTyp );
}
inline const SwTableNode* SwNode::FindTableNode() const
{
    return const_cast<SwNode*>(this)->FindTableNode();
}
inline const SwSectionNode* SwNode::FindSectionNode() const
{
    return const_cast<SwNode*>(this)->FindSectionNode();
}
inline sal_uLong SwNode::StartOfSectionIndex() const
{
    return m_pStartOfSection->GetIndex();
}
inline sal_uLong SwNode::EndOfSectionIndex() const
{
    const SwStartNode* pStNd = IsStartNode() ? static_cast<const SwStartNode*>(this) : m_pStartOfSection;
    return pStNd->m_pEndOfSection->GetIndex();
}
inline const SwEndNode* SwNode::EndOfSectionNode() const
{
    const SwStartNode* pStNd = IsStartNode() ? static_cast<const SwStartNode*>(this) : m_pStartOfSection;
    return pStNd->m_pEndOfSection;
}
inline SwEndNode* SwNode::EndOfSectionNode()
{
    const SwStartNode* pStNd = IsStartNode() ? static_cast<const SwStartNode*>(this) : m_pStartOfSection;
    return pStNd->m_pEndOfSection;
}

inline SwNodes& SwNode::GetNodes()
{
    return static_cast<SwNodes&>(GetArray());
}
inline const SwNodes& SwNode::GetNodes() const
{
    return static_cast<SwNodes&>(GetArray());
}

inline SwDoc* SwNode::GetDoc()
{
    return GetNodes().GetDoc();
}
inline const SwDoc* SwNode::GetDoc() const
{
    return GetNodes().GetDoc();
}

inline SwFormatColl* SwContentNode::GetCondFormatColl() const
{
    return m_pCondColl;
}

inline SwFormatColl& SwContentNode::GetAnyFormatColl() const
{
    return m_pCondColl
            ? *m_pCondColl
            : *const_cast<SwFormatColl*>(static_cast<const SwFormatColl*>(GetRegisteredIn()));
}

inline const SwAttrSet& SwContentNode::GetSwAttrSet() const
{
    return mpAttrSet ? *GetpSwAttrSet() : GetAnyFormatColl().GetAttrSet();
}

//FEATURE::CONDCOLL

inline const SfxPoolItem& SwContentNode::GetAttr( sal_uInt16 nWhich,
                                                bool bInParents ) const
{
    return GetSwAttrSet().Get( nWhich, bInParents );
}

inline SwPlaceholderNode::SwPlaceholderNode(const SwNodeIndex &rWhere)
    : SwNode(rWhere, SwNodeType::PlaceHolder)
{
}

inline SwNodePtr SwNodes::operator[]( sal_uLong n ) const
{
    return static_cast<SwNodePtr>(BigPtrArray::operator[] ( n ));
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
