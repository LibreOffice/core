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
#include "BorderCacheOwner.hxx"
#include "ndarr.hxx"
#include "ndtyp.hxx"
#include "contentindex.hxx"
#include "fmtcol.hxx"
#include "nodeoffset.hxx"

#include <sfx2/AccessibilityIssue.hxx>

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
enum class SwCursorSkipMode;

namespace drawinglayer::attribute {
    class SdrAllFillAttributesHelper;
    typedef std::shared_ptr< SdrAllFillAttributesHelper > SdrAllFillAttributesHelperPtr;
}

// Accessibility check

namespace sw
{
struct AccessibilityCheckStatus
{
    std::unique_ptr<sfx::AccessibilityIssueCollection> pCollection;
    void reset();
};

}

/// Base class of the Writer document model elements.
class SAL_DLLPUBLIC_RTTI SwNode
    : public sw::BorderCacheOwner, private BigPtrEntry
{
    friend class SwNodes;

    SwNodeType m_nNodeType;

    /// For text nodes: level of auto format. Was put here because we had still free bits.
    sal_uInt8 m_nAFormatNumLvl : 3;
    bool m_bIgnoreDontExpand : 1;     ///< for Text Attributes - ignore the flag

    mutable sw::AccessibilityCheckStatus m_aAccessibilityCheckStatus;

public:
    /// sw_redlinehide: redline node merge state
    enum class Merge { None, First, NonFirst, Hidden };
    bool IsCreateFrameWhenHidingRedlines() const {
        return m_eMerge == Merge::None || m_eMerge == Merge::First;
    }
    void SetRedlineMergeFlag(Merge const eMerge) { m_eMerge = eMerge; }
    Merge GetRedlineMergeFlag() const { return m_eMerge; }
private:
    Merge m_eMerge;

#ifdef DBG_UTIL
    static tools::Long s_nSerial;
    tools::Long m_nSerial;
#endif

    /// all SwFrameFormat that are anchored at the node
    /// invariant: SwFrameFormat is in the list iff
    /// SwFrameFormat::GetAnchor().GetContentAnchor() points to this node
    std::vector<SwFrameFormat*> m_aAnchoredFlys;

protected:
    SwStartNode* m_pStartOfSection;

    /// only used by SwContentNodeTmp in SwTextNode::Update
    SwNode();

    SwNode( const SwNode& rWhere, const SwNodeType nNodeId );

    /// for the initial StartNode
    SwNode( SwNodes& rNodes, SwNodeOffset nPos, const SwNodeType nNodeId );

public:
    /** the = 0 forces the class to be an abstract base class, but the dtor can be still called
       from subclasses */
    virtual ~SwNode() override = 0;

#ifdef DBG_UTIL
    tools::Long GetSerial() const { return m_nSerial; }
#endif

    sal_uInt16 GetSectionLevel() const;

    inline SwNodeOffset StartOfSectionIndex() const;
    const SwStartNode* StartOfSectionNode() const { return m_pStartOfSection; }
    SwStartNode* StartOfSectionNode() { return m_pStartOfSection; }

    inline SwNodeOffset EndOfSectionIndex() const;
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

    bool IsStartNode() const { return bool(SwNodeType::Start & m_nNodeType); }
    bool IsContentNode() const { return bool(SwNodeType::ContentMask & m_nNodeType); }
    bool IsEndNode() const { return SwNodeType::End == m_nNodeType; }
    bool IsTextNode() const { return SwNodeType::Text == m_nNodeType; }
    bool IsTableNode() const { return SwNodeType::Table == m_nNodeType; }
    bool IsSectionNode() const { return SwNodeType::Section == m_nNodeType; }
    bool IsOLENode() const { return SwNodeType::Ole == m_nNodeType; }
    bool IsNoTextNode() const { return bool(SwNodeType::NoTextMask & m_nNodeType); }
    bool IsGrfNode() const { return SwNodeType::Grf == m_nNodeType; }

    /**
       Checks if this node is in redlines.

       @retval true       this node is in redlines
       @retval false      else
     */
    bool IsInRedlines() const;

    /** Search table node, in which it is. If it is in no table
       @return 0. */
    SW_DLLPUBLIC SwTableNode *FindTableNode();
    inline const  SwTableNode *FindTableNode() const;

    /** Search section node, in which it is. If it is in no section
       @return 0. */
    SW_DLLPUBLIC SwSectionNode *FindSectionNode();
    inline    const   SwSectionNode *FindSectionNode() const;

    SW_DLLPUBLIC SwStartNode* FindSttNodeByType( SwStartNodeType eTyp );
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

    SwDoc& GetDoc()
    {
        return GetNodes().GetDoc();
    }

    const SwDoc& GetDoc() const
    {
        return GetNodes().GetDoc();
    }

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
    /// Is node in a protected area?
    bool IsInProtectSect() const;
    /**  Is node in something that is protected (range, frame,
        table cells ... including anchor in case of frames or footnotes)? */
    bool IsProtect() const;

    /** Search PageDesc with which this node is formatted. If layout is existent
       search over layout, else only the hard way is left: search over the nodes
       to the front!! */
    SW_DLLPUBLIC const SwPageDesc* FindPageDesc( SwNodeOffset* pPgDescNdIdx = nullptr ) const;

    /// If node is in a fly return the respective format.
    SW_DLLPUBLIC SwFrameFormat* GetFlyFormat() const;

    /// If node is in a table return the respective table box.
    SW_DLLPUBLIC SwTableBox* GetTableBox() const;

    SwNodeOffset GetIndex() const { return SwNodeOffset(GetPos()); }

    SW_DLLPUBLIC const SwTextNode* FindOutlineNodeOfLevel(sal_uInt8 nLvl, SwRootFrame const* pLayout = nullptr) const;

    SW_DLLPUBLIC sal_uInt8 HasPrevNextLayNode() const;

    std::vector<SwFrameFormat *> const & GetAnchoredFlys() const { return m_aAnchoredFlys; }
    void AddAnchoredFly(SwFrameFormat *);
    void RemoveAnchoredFly(SwFrameFormat *);

    /**
     * Dumps the node structure to the given destination (file nodes.xml in the current directory by default)
     */
    virtual void dumpAsXml(xmlTextWriterPtr pWriter) const;

    bool operator==(const SwNode& rOther) const { return this == &rOther; }
    bool operator!=(const SwNode& rOther) const { return this != &rOther; }
    bool operator<(const SwNode& rOther) const { assert(&GetNodes() == &rOther.GetNodes()); return GetIndex() < rOther.GetIndex(); }
    bool operator<=(const SwNode& rOther) const { assert(&GetNodes() == &rOther.GetNodes()); return GetIndex() <= rOther.GetIndex(); }
    bool operator>(const SwNode& rOther) const { assert(&GetNodes() == &rOther.GetNodes()); return GetIndex() > rOther.GetIndex(); }
    bool operator>=(const SwNode& rOther) const { assert(&GetNodes() == &rOther.GetNodes()); return GetIndex() >= rOther.GetIndex(); }

    sw::AccessibilityCheckStatus& getAccessibilityCheckStatus()
    {
        return m_aAccessibilityCheckStatus;
    }

    void resetAndQueueAccessibilityCheck(bool bIssueObjectNameChanged = false);

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
    SwStartNodeType m_eStartNodeType;

    /// for the initial StartNode
    SwStartNode( SwNodes& rNodes, SwNodeOffset nPos );

protected:
    SwStartNode( const SwNode& rWhere,
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
class SwEndNode final : public SwNode
{
    friend class SwNodes;
    friend class SwTableNode;       ///< To enable creation of its EndNote.
    friend class SwSectionNode;     ///< To enable creation of its EndNote.

    /// for the initial StartNode
    SwEndNode( SwNodes& rNodes, SwNodeOffset nPos, SwStartNode& rSttNd );

    SwEndNode( const SwNode& rWhere, SwStartNode& rSttNd );

    SwEndNode( const SwEndNode & rNode ) = delete;
    SwEndNode & operator= ( const SwEndNode & rNode ) = delete;
};

// SwContentNode

class SAL_DLLPUBLIC_RTTI SwContentNode: public sw::BroadcastingModify, public SwNode, public SwContentIndexReg
{

    sw::WriterMultiListener m_aCondCollListener;
    SwFormatColl* m_pCondColl;
    mutable bool mbSetModifyAtAttr;

protected:
    /// only used by SwContentNodeTmp in SwTextNode::Update
    SwContentNode();

    SwContentNode( const SwNode& rWhere, const SwNodeType nNodeType,
                SwFormatColl *pFormatColl );
    /** the = 0 forces the class to be an abstract base class, but the dtor can be still called
       from subclasses */
    virtual ~SwContentNode() override = 0;

    /**  Attribute-set for all auto attributes of a ContentNode.
      (e.g. TextNode or NoTextNode). */
    std::shared_ptr<const SwAttrSet> mpAttrSet;

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
    SW_DLLPUBLIC bool CanJoinNext( SwNodeIndex* pIdx = nullptr ) const;
    bool CanJoinNext( SwPosition* pIdx ) const;
    SW_DLLPUBLIC bool CanJoinPrev( SwNodeIndex* pIdx =nullptr ) const;

    bool GoNext(SwContentIndex *, SwCursorSkipMode nMode ) const;
    bool GoNext(SwPosition*, SwCursorSkipMode nMode ) const;
    bool GoPrevious(SwContentIndex *, SwCursorSkipMode nMode ) const;

    /// @see GetFrameOfModify
    SW_DLLPUBLIC SwContentFrame *getLayoutFrame( const SwRootFrame*,
            const SwPosition *pPos = nullptr,
            std::pair<Point, bool> const* pViewPosAndCalcFrame = nullptr) const;
    /** @return the real size of the frame or an empty rectangle if
       no layout exists. Needed for export filters. */
    SW_DLLPUBLIC SwRect FindLayoutRect( const bool bPrtArea = false,
                            const Point* pPoint = nullptr  ) const;
    SW_DLLPUBLIC SwRect FindPageFrameRect() const;

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

    virtual SwContentNode* MakeCopy(SwDoc&, SwNode& rWhere, bool bNewFrames) const = 0;

    /// Get information from Client.
    virtual bool GetInfo( SfxPoolItem& ) const override;

    /// SS for PoolItems: hard attributation.

    /// If bInParent is FALSE search for attribute only in this node.
    const SfxPoolItem& GetAttr( sal_uInt16 nWhich, bool bInParent=true ) const;
    template<class T>
    const T& GetAttr( TypedWhichId<T> nWhich, bool bInParent=true ) const
    { return static_cast<const T&>(GetAttr(sal_uInt16(nWhich), bInParent)); }
    SW_DLLPUBLIC bool GetAttr( SfxItemSet& rSet ) const;
    /// made virtual
    SW_DLLPUBLIC virtual bool SetAttr( const SfxPoolItem& );
    virtual bool SetAttr( const SfxItemSet& rSet );
    virtual bool ResetAttr( sal_uInt16 nWhich1, sal_uInt16 nWhich2 = 0 );
    virtual bool ResetAttr( const std::vector<sal_uInt16>& rWhichArr );
    virtual sal_uInt16 ResetAllAttr();

    /// Obtains attribute that is not delivered via conditional style!
    SW_DLLPUBLIC const SfxPoolItem* GetNoCondAttr( sal_uInt16 nWhich, bool bInParents ) const;
    template<class T>
    const T* GetNoCondAttr( TypedWhichId<T> nWhich, bool bInParents ) const
    { return static_cast<const T*>(GetNoCondAttr(sal_uInt16(nWhich), bInParents)); }

    /** Does node has already its own auto-attributes?
     Access to SwAttrSet. */
    inline const SwAttrSet &GetSwAttrSet() const;
    const SwAttrSet *GetpSwAttrSet() const { return mpAttrSet.get(); }
    bool  HasSwAttrSet() const { return mpAttrSet != nullptr; }

    virtual SwFormatColl* ChgFormatColl( SwFormatColl* );
    SwFormatColl* GetFormatColl() const { return const_cast<SwFormatColl*>(static_cast<const SwFormatColl*>(GetRegisteredIn())); }

    inline SwFormatColl& GetAnyFormatColl() const;
    void SetCondFormatColl( SwFormatColl* );
    inline SwFormatColl* GetCondFormatColl() const;

    bool IsAnyCondition( SwCollCondition& rTmp ) const;
    void ChkCondColl(const SwTextFormatColl* pColl = nullptr);

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

    void UpdateAttr(const SwUpdateAttr&);

private:
    SwContentNode( const SwContentNode & rNode ) = delete;
    SwContentNode & operator= ( const SwContentNode & rNode ) = delete;
};

// SwTableNode

class SW_DLLPUBLIC SwTableNode final : public SwStartNode, public sw::BroadcastingModify
{
    friend class SwNodes;
    std::unique_ptr<SwTable> m_pTable;

    virtual ~SwTableNode() override;

public:
    SwTableNode( const SwNode& );

    const SwTable& GetTable() const { return *m_pTable; }
    SwTable& GetTable() { return *m_pTable; }
    SwTabFrame *MakeFrame( SwFrame* );

    /// Creates the frms for the table node (i.e. the TabFrames).
    /// pIdxBehind is optional parameter.
    void MakeOwnFrames(SwPosition* pIdxBehind = nullptr);

    /** Method deletes all views of document for the node.
       The content frames are removed from the respective layout. */
    void DelFrames(SwRootFrame const* pLayout = nullptr);

    /** Method creates all views of the document for the previous node.
       The content frames that are created are put into the respective layout. */
    void MakeFramesForAdjacentContentNode(const SwNodeIndex & rIdx);

    SwTableNode* MakeCopy( SwDoc&, const SwNodeIndex& ) const;
    void SetNewTable( std::unique_ptr<SwTable> , bool bNewFrames=true );

    // Removes redline objects that relate to this table from the 'Extra Redlines' table
    void RemoveRedlines();

    void dumpAsXml(xmlTextWriterPtr pWriter) const override;

private:
    SwTableNode( const SwTableNode & rNode ) = delete;
    SwTableNode & operator= ( const SwTableNode & rNode ) = delete;
};

/// A section node represents the start of a section on the UI, i.e. the container created using
/// Insert -> Section menu item.
class SAL_DLLPUBLIC_RTTI SwSectionNode final
    : public SwStartNode
{
    friend class SwNodes;

private:
    SwSectionNode(const SwSectionNode&) = delete;
    SwSectionNode& operator=(const SwSectionNode&) = delete;

    std::unique_ptr<SwSection> const m_pSection;

    virtual ~SwSectionNode() override;

public:
    SwSectionNode(const SwNode& rWhere,
        SwSectionFormat & rFormat, SwTOXBase const*const pTOXBase);

    const SwSection& GetSection() const { return *m_pSection; }
          SwSection& GetSection()       { return *m_pSection; }

    SwFrame* MakeFrame(SwFrame* pSib, bool bHidden);

    /** Creates the frms for the SectionNode (i.e. the SectionFrames).
       On default the frames are created until the end of the range.
       When another NodeIndex pEnd is passed a MakeFrames is called up to it.
       Used by TableToText. */
    void MakeOwnFrames(SwNodeIndex* pIdxBehind, SwNodeIndex* pEnd = nullptr);

    /** Method deletes all views of document for the node. The
     content frames are removed from the respective layout. */
    void DelFrames(SwRootFrame const* pLayout = nullptr, bool bForce = false);

    /** Method creates all views of document for the previous node.
       The content frames created are put into the respective layout. */
    void MakeFramesForAdjacentContentNode(const SwNodeIndex & rIdx);

    SwSectionNode* MakeCopy( SwDoc&, const SwNodeIndex& ) const;

    /// Set pointer in format of section on itself.
    void NodesArrChgd();

    /** Check for not hidden areas whether there is content that is not in
       a hidden sub-area. */
    bool IsContentHidden() const;

    void dumpAsXml(xmlTextWriterPtr pWriter) const override;

};

/** This class is internal, used only during DocumentContentOperationsManager::CopyWithFlyInFly(), and for undo.

Some of the nodes are then replaced with SwPlaceholderNode, and at the end of the operation, removed again.
FIXME find out if this is really necessary, and if we can avoid creation of the SwPlaceholderNodes in the first place.
*/
class SwPlaceholderNode final : private SwNode
{
private:
    friend class SwNodes;
    SwPlaceholderNode(const SwNode& rWhere);
};

inline       SwEndNode   *SwNode::GetEndNode()
{
     return IsEndNode() ? static_cast<SwEndNode*>(this) : nullptr;
}
inline const SwEndNode   *SwNode::GetEndNode() const
{
     return IsEndNode() ? static_cast<const SwEndNode*>(this) : nullptr;
}
inline       SwStartNode *SwNode::GetStartNode()
{
     return IsStartNode() ? static_cast<SwStartNode*>(this) : nullptr;
}
inline const SwStartNode *SwNode::GetStartNode() const
{
     return IsStartNode() ? static_cast<const SwStartNode*>(this) : nullptr;
}
inline       SwTableNode *SwNode::GetTableNode()
{
     return IsTableNode() ? static_cast<SwTableNode*>(this) : nullptr;
}
inline const SwTableNode *SwNode::GetTableNode() const
{
     return IsTableNode() ? static_cast<const SwTableNode*>(this) : nullptr;
}
inline       SwSectionNode *SwNode::GetSectionNode()
{
     return IsSectionNode() ? static_cast<SwSectionNode*>(this) : nullptr;
}
inline const SwSectionNode *SwNode::GetSectionNode() const
{
     return IsSectionNode() ? static_cast<const SwSectionNode*>(this) : nullptr;
}
inline       SwContentNode *SwNode::GetContentNode()
{
     return IsContentNode() ? static_cast<SwContentNode*>(this) : nullptr;
}
inline const SwContentNode *SwNode::GetContentNode() const
{
     return IsContentNode() ? static_cast<const SwContentNode*>(this) : nullptr;
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
inline SwNodeOffset SwNode::StartOfSectionIndex() const
{
    return m_pStartOfSection->GetIndex();
}
inline SwNodeOffset SwNode::EndOfSectionIndex() const
{
    return EndOfSectionNode()->GetIndex();
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

inline const SfxPoolItem& SwContentNode::GetAttr( sal_uInt16 nWhich,
                                                bool bInParents ) const
{
    return GetSwAttrSet().Get( nWhich, bInParents );
}

inline SwPlaceholderNode::SwPlaceholderNode(const SwNode& rWhere)
    : SwNode(rWhere, SwNodeType::PlaceHolder)
{
}

inline SwNode* SwNodes::operator[]( SwNodeOffset n ) const
{
    return static_cast<SwNode*>(BigPtrArray::operator[] ( sal_Int32(n) ));
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
