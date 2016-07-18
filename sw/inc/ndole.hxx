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
#ifndef INCLUDED_SW_INC_NDOLE_HXX
#define INCLUDED_SW_INC_NDOLE_HXX

#include <ndnotxt.hxx>
#include <svtools/embedhlp.hxx>
#include <drawinglayer/primitive2d/baseprimitive2d.hxx>

class SwGrfFormatColl;
class SwDoc;
class SwOLENode;
class SwOLEListener_Impl;
class SwEmbedObjectLink;
class DeflateData;

class SW_DLLPUBLIC SwOLEObj
{
    friend class SwOLENode;

    const SwOLENode* pOLENd;
    SwOLEListener_Impl* pListener;

    /** Either ref or name are known. If only name is known, ref is obtained
       on demand by GetOleRef() from Sfx. */
    svt::EmbeddedObjectRef xOLERef;
    OUString aName;

    // eventually buffered data if it is a chart OLE
    drawinglayer::primitive2d::Primitive2DContainer     m_aPrimitive2DSequence;
    basegfx::B2DRange                                   m_aRange;
    class DeflateData*                                  m_pDeflateData;

    SwOLEObj( const SwOLEObj& rObj ) = delete;

    void SetNode( SwOLENode* pNode );

public:
    SwOLEObj( const svt::EmbeddedObjectRef& pObj );
    SwOLEObj( const OUString &rName, sal_Int64 nAspect );
    ~SwOLEObj();

    bool UnloadObject();
    static bool UnloadObject( css::uno::Reference< css::embed::XEmbeddedObject > const & xObj,
                                const SwDoc* pDoc,
                                sal_Int64 nAspect );

    OUString GetDescription();

    const css::uno::Reference < css::embed::XEmbeddedObject > GetOleRef();
    svt::EmbeddedObjectRef& GetObject();
    const OUString& GetCurrentPersistName() const { return aName; }
    OUString GetStyleString();
    bool IsOleRef() const;  ///< To avoid unnecessary loading of object.

    // try to get OLE visualization in form of a Primitive2DSequence
    // and the corresponding B2DRange. This data may be locally buffered
    drawinglayer::primitive2d::Primitive2DContainer const & tryToGetChartContentAsPrimitive2DSequence(
        basegfx::B2DRange& rRange,
        bool bSynchron);
    void resetBufferedData();
};

// SwOLENode

class SW_DLLPUBLIC SwOLENode: public SwNoTextNode
{
    friend class SwNodes;
    mutable SwOLEObj aOLEObj;
    OUString sChartTableName;     ///< with chart objects: name of referenced table.
    bool   bOLESizeInvalid; /**< Should be considered at SwDoc::PrtOLENotify
                                   (e.g. copied). Is not persistent. */

    SwEmbedObjectLink*  mpObjectLink;
    OUString maLinkURL;

    SwOLENode(  const SwNodeIndex &rWhere,
                const svt::EmbeddedObjectRef&,
                SwGrfFormatColl *pGrfColl,
                SwAttrSet* pAutoAttr = nullptr );

    SwOLENode(  const SwNodeIndex &rWhere,
                const OUString &rName,
                sal_Int64 nAspect,
                SwGrfFormatColl *pGrfColl,
                SwAttrSet* pAutoAttr = nullptr );

    SwOLENode( const SwOLENode & ) = delete;

    using SwNoTextNode::GetGraphic;

public:
    const SwOLEObj& GetOLEObj() const { return aOLEObj; }
          SwOLEObj& GetOLEObj()       { return aOLEObj; }
    virtual ~SwOLENode();

    virtual SwContentNode *SplitContentNode( const SwPosition & ) override;

    /// Is in ndcopy.cxx.
    virtual SwContentNode* MakeCopy( SwDoc*, const SwNodeIndex& ) const override;

    virtual Size GetTwipSize() const override;

    const Graphic* GetGraphic();

    void GetNewReplacement();

    virtual bool SavePersistentData() override;
    virtual bool RestorePersistentData() override;

    bool IsInGlobalDocSection() const;
    bool IsOLEObjectDeleted() const;

    bool IsOLESizeInvalid() const   { return bOLESizeInvalid; }
    void SetOLESizeInvalid( bool b ){ bOLESizeInvalid = b; }

    sal_Int64 GetAspect() const { return aOLEObj.GetObject().GetViewAspect(); }
    void SetAspect( sal_Int64 nAspect) { aOLEObj.GetObject().SetViewAspect( nAspect ); }

    /** Remove OLE-object from "memory".
       inline void Unload() { aOLEObj.Unload(); } */
    OUString GetDescription() const { return aOLEObj.GetDescription(); }

    bool UpdateLinkURL_Impl();
    void BreakFileLink_Impl();
    void DisconnectFileLink_Impl();

    void CheckFileLink_Impl();

    // #i99665#
    bool IsChart() const;

    const OUString& GetChartTableName() const { return sChartTableName; }
    void SetChartTableName( const OUString& rNm ) { sChartTableName = rNm; }
};

/// Inline methods from Node.hxx
inline SwOLENode *SwNode::GetOLENode()
{
     return ND_OLENODE == m_nNodeType ? static_cast<SwOLENode*>(this) : nullptr;
}

inline const SwOLENode *SwNode::GetOLENode() const
{
     return ND_OLENODE == m_nNodeType ? static_cast<const SwOLENode*>(this) : nullptr;
}

#endif  // _ INCLUDED_SW_INC_NDOLE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
