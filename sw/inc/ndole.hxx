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

#include "ndnotxt.hxx"
#include <svtools/embedhlp.hxx>
#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <rtl/ref.hxx>

class SwGrfFormatColl;
class SwDoc;
class SwOLENode;
class SwOLEListener_Impl;
class SwEmbedObjectLink;
class DeflateData;

class SW_DLLPUBLIC SwOLEObj
{
    friend class SwOLENode;

    const SwOLENode* m_pOLENode;
    rtl::Reference<SwOLEListener_Impl> m_xListener;

    /** Either ref or name are known. If only name is known, ref is obtained
       on demand by GetOleRef() from Sfx. */
    svt::EmbeddedObjectRef m_xOLERef;
    OUString m_aName;

    // eventually buffered data if it is a chart OLE
    drawinglayer::primitive2d::Primitive2DContainer     m_aPrimitive2DSequence;
    basegfx::B2DRange                                   m_aRange;
    std::unique_ptr<DeflateData>                        m_pDeflateData;

    SwOLEObj( const SwOLEObj& rObj ) = delete;

    void SetNode( SwOLENode* pNode );

    DECL_LINK(IsProtectedHdl, LinkParamNone*, bool);

public:
    SwOLEObj( const svt::EmbeddedObjectRef& pObj );
    SwOLEObj( const OUString &rName, sal_Int64 nAspect );
    ~SwOLEObj() COVERITY_NOEXCEPT_FALSE;

    bool UnloadObject();
    static bool UnloadObject( css::uno::Reference< css::embed::XEmbeddedObject > const & xObj,
                                const SwDoc* pDoc,
                                sal_Int64 nAspect );

    OUString GetDescription();

    css::uno::Reference < css::embed::XEmbeddedObject > const & GetOleRef();
    svt::EmbeddedObjectRef& GetObject();
    const OUString& GetCurrentPersistName() const { return m_aName; }
    OUString GetStyleString();
    bool IsOleRef() const;  ///< To avoid unnecessary loading of object.
    bool IsProtected() const;

    // try to get OLE visualization in form of a Primitive2DSequence
    // and the corresponding B2DRange. This data may be locally buffered
    drawinglayer::primitive2d::Primitive2DContainer const & tryToGetChartContentAsPrimitive2DSequence(
        basegfx::B2DRange& rRange,
        bool bSynchron);
    void resetBufferedData();

    void dumpAsXml(xmlTextWriterPtr pWriter) const;
};

// SwOLENode

class SW_DLLPUBLIC SwOLENode final: public SwNoTextNode
{
    friend class SwNodes;
    mutable SwOLEObj maOLEObj;
    OUString msChartTableName;     ///< with chart objects: name of referenced table.
    bool   mbOLESizeInvalid; /**< Should be considered at SwDoc::PrtOLENotify
                                   (e.g. copied). Is not persistent. */

    SwEmbedObjectLink*  mpObjectLink;
    OUString maLinkURL;

    SwOLENode(  const SwNodeIndex &rWhere,
                const svt::EmbeddedObjectRef&,
                SwGrfFormatColl *pGrfColl,
                SwAttrSet const * pAutoAttr );

    SwOLENode(  const SwNodeIndex &rWhere,
                const OUString &rName,
                sal_Int64 nAspect,
                SwGrfFormatColl *pGrfColl,
                SwAttrSet const * pAutoAttr );

    SwOLENode( const SwOLENode & ) = delete;

    using SwNoTextNode::GetGraphic;

public:
    const SwOLEObj& GetOLEObj() const { return maOLEObj; }
          SwOLEObj& GetOLEObj()       { return maOLEObj; }
    virtual ~SwOLENode() override;

    /// Is in ndcopy.cxx.
    virtual SwContentNode* MakeCopy(SwDoc&, const SwNodeIndex&, bool bNewFrames) const override;

    virtual Size GetTwipSize() const override;

    const Graphic* GetGraphic();

    void GetNewReplacement();

    virtual bool SavePersistentData() override;
    virtual bool RestorePersistentData() override;

    virtual void dumpAsXml(xmlTextWriterPtr pWriter) const override;

    bool IsInGlobalDocSection() const;
    bool IsOLEObjectDeleted() const;

    bool IsOLESizeInvalid() const   { return mbOLESizeInvalid; }
    void SetOLESizeInvalid( bool b ){ mbOLESizeInvalid = b; }

    sal_Int64 GetAspect() const { return maOLEObj.GetObject().GetViewAspect(); }
    void SetAspect( sal_Int64 nAspect) { maOLEObj.GetObject().SetViewAspect( nAspect ); }

    /** Remove OLE-object from "memory".
       inline void Unload() { aOLEObj.Unload(); } */
    OUString GetDescription() const { return maOLEObj.GetDescription(); }

    bool UpdateLinkURL_Impl();
    void BreakFileLink_Impl();
    void DisconnectFileLink_Impl();

    void CheckFileLink_Impl();

    // #i99665#
    bool IsChart() const;

    const OUString& GetChartTableName() const { return msChartTableName; }
    void SetChartTableName( const OUString& rNm ) { msChartTableName = rNm; }


    // react on visual change (invalidate)
    void SetChanged();
};

/// Inline methods from Node.hxx
inline SwOLENode *SwNode::GetOLENode()
{
     return SwNodeType::Ole == m_nNodeType ? static_cast<SwOLENode*>(this) : nullptr;
}

inline const SwOLENode *SwNode::GetOLENode() const
{
     return SwNodeType::Ole == m_nNodeType ? static_cast<const SwOLENode*>(this) : nullptr;
}

namespace sw
{
    class DocumentSettingManager;
}

class PurgeGuard
{
private:
    ::sw::DocumentSettingManager &m_rManager;
    bool m_bOrigPurgeOle;
public:
    PurgeGuard(const SwDoc& rDoc);
    ~PurgeGuard() COVERITY_NOEXCEPT_FALSE;
};

#endif  // _ INCLUDED_SW_INC_NDOLE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
