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

#ifndef INCLUDED_SW_INC_NDGRF_HXX
#define INCLUDED_SW_INC_NDGRF_HXX

#include <sfx2/lnkbase.hxx>
#include <vcl/GraphicObject.hxx>
#include "ndnotxt.hxx"
#include <memory>

class SwAsyncRetrieveInputStreamThreadConsumer;

class SwGrfFormatColl;
class SwDoc;

// SwGrfNode
class SW_DLLPUBLIC SwGrfNode final: public SwNoTextNode
{
    friend class SwNodes;

    GraphicObject maGrfObj;
    std::unique_ptr<GraphicObject> mpReplacementGraphic;
    tools::SvRef<sfx2::SvBaseLink> mxLink;       ///< If graphics only as link then pointer is set.
    Size mnGrfSize;
    bool mbInSwapIn              :1; // to avoid recursion in SwGrfNode::SwapIn
    bool mbInBaseLinkSwapIn      :1; // to avoid recursion in SwBaseLink::SwapIn

    bool mbChangeTwipSize           :1;
    bool mbFrameInPaint          :1; ///< To avoid Start-/EndActions in Paint via SwapIn.
    bool mbScaleImageMap         :1; ///< Scale image map in SetTwipSize.

    std::shared_ptr< SwAsyncRetrieveInputStreamThreadConsumer > mpThreadConsumer;
    bool mbLinkedInputStreamReady;
    css::uno::Reference<css::io::XInputStream> mxInputStream;
    bool mbIsStreamReadOnly;

    SwGrfNode( const SwNodeIndex& rWhere,
               const OUString& rGrfName, const OUString& rFltName,
               const Graphic* pGraphic,
               SwGrfFormatColl* pGrfColl,
               SwAttrSet const * pAutoAttr );
    ///< Ctor for reading (SW/G) without graphics.
    SwGrfNode( const SwNodeIndex& rWhere,
               const OUString& rGrfName, const OUString& rFltName,
               SwGrfFormatColl* pGrfColl,
               SwAttrSet const * pAutoAttr );
    SwGrfNode( const SwNodeIndex& rWhere,
               const GraphicObject& rGrfObj,
               SwGrfFormatColl* pGrfColl,
               SwAttrSet const * pAutoAttr );

    void InsertLink( const OUString& rGrfName, const OUString& rFltName );

    /// allow reaction on change of content of GraphicObject, so always call
    /// when GraphicObject content changes
    void onGraphicChanged();

public:
    virtual ~SwGrfNode() override;
    const Graphic&          GetGrf(bool bWait = false) const;
    const GraphicObject&    GetGrfObj(bool bWait = false) const;
    const GraphicObject* GetReplacementGrfObj() const;

    /// isolated only way to set GraphicObject to allow more actions when doing so
    void SetGraphic(const Graphic& rGraphic);
    void TriggerGraphicArrived();

    /// wrappers for non-const calls at GraphicObject
    void StartGraphicAnimation(OutputDevice* pOut, const Point& rPt, const Size& rSz, tools::Long nExtraData, OutputDevice* pFirstFrameOutDev)
    { maGrfObj.StartAnimation(pOut, rPt, rSz, nExtraData, pFirstFrameOutDev); }
    void StopGraphicAnimation(const OutputDevice* pOut, tools::Long nExtraData) { maGrfObj.StopAnimation(pOut, nExtraData); }

    virtual Size GetTwipSize() const override;
    void SetTwipSize( const Size& rSz );

    bool IsTransparent() const;

    bool IsAnimated() const              { return maGrfObj.IsAnimated(); }

    bool IsChgTwipSize() const           { return mbChangeTwipSize; }
    void SetChgTwipSize( bool b)
    {
        mbChangeTwipSize = b;
    }

    bool IsFrameInPaint() const          { return mbFrameInPaint; }
    void SetFrameInPaint( bool b )       { mbFrameInPaint = b; }

    bool IsScaleImageMap() const         { return mbScaleImageMap; }
    void SetScaleImageMap( bool b )      { mbScaleImageMap = b; }

    /// in ndcopy.cxx
    virtual SwContentNode* MakeCopy(SwDoc&, const SwNodeIndex&, bool bNewFrames) const override;

    /** Re-read in case graphic was not OK. The current one
       gets replaced by the new one. */
    bool ReRead( const OUString& rGrfName, const OUString& rFltName,
                 const Graphic* pGraphic = nullptr,
                 bool bModify = true );
private:
    /// Loading of graphic immediately before displaying.
    bool SwapIn( bool bWaitForData = false );

public:
    bool HasEmbeddedStreamName() const { return maGrfObj.HasUserData(); }

    /// Communicate to graphic that node is in Undo-range.
    virtual bool SavePersistentData() override;
    virtual bool RestorePersistentData() override;

    /// Query link-data.
    bool IsGrfLink() const                  { return mxLink.is(); }
    bool IsLinkedFile() const;
    bool IsLinkedDDE() const;
    const tools::SvRef<sfx2::SvBaseLink>& GetLink() const    { return mxLink; }
    bool GetFileFilterNms( OUString* pFileNm, OUString* pFilterNm ) const;
    void ReleaseLink();

    /** Scale an image-map: the image-map becomes zoomed in / out by
       factor between graphic-size and border-size. */
    void ScaleImageMap();

    /// Returns the with our graphic attributes filled Graphic-Attr-Structure.
    GraphicAttr& GetGraphicAttr( GraphicAttr&, const SwFrame* pFrame ) const;

    std::weak_ptr< SwAsyncRetrieveInputStreamThreadConsumer > GetThreadConsumer() const { return mpThreadConsumer;}
    bool IsLinkedInputStreamReady() const { return mbLinkedInputStreamReady;}
    void TriggerAsyncRetrieveInputStream();
    void ApplyInputStream(
        const css::uno::Reference<css::io::XInputStream>& xInputStream,
        const bool bIsStreamReadOnly );
    void UpdateLinkWithInputStream();
    bool IsAsyncRetrieveInputStreamPossible() const;
};

// Inline methods from Node.hxx - it is only now that we know TextNode!!
inline       SwGrfNode   *SwNode::GetGrfNode()
{
     return SwNodeType::Grf == m_nNodeType ? static_cast<SwGrfNode*>(this) : nullptr;
}

inline const SwGrfNode   *SwNode::GetGrfNode() const
{
     return SwNodeType::Grf == m_nNodeType ? static_cast<const SwGrfNode*>(this) : nullptr;
}

inline bool SwGrfNode::IsLinkedFile() const
{
    return mxLink.is() && sfx2::SvBaseLinkObjectType::ClientGraphic == mxLink->GetObjType();
}

inline bool SwGrfNode::IsLinkedDDE() const
{
    return mxLink.is() && sfx2::SvBaseLinkObjectType::ClientDde == mxLink->GetObjType();
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
