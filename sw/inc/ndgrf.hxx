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
#include <svtools/grfmgr.hxx>
#include <ndnotxt.hxx>
#include <com/sun/star/embed/XStorage.hpp>
#include <memory>
class SwAsyncRetrieveInputStreamThreadConsumer;

class SwGrfFormatColl;
class SwDoc;
class GraphicAttr;
class SotStorage;

// SwGrfNode
class SW_DLLPUBLIC SwGrfNode: public SwNoTextNode
{
    friend class SwNodes;

    GraphicObject maGrfObj;
    GraphicObject *mpReplacementGraphic;
    tools::SvRef<sfx2::SvBaseLink> refLink;       ///< If graphics only as link then pointer is set.
    Size nGrfSize;
    bool bInSwapIn              :1;

    bool bGraphicArrived        :1;
    bool bChgTwipSize           :1;
    bool bFrameInPaint          :1; ///< To avoid Start-/EndActions in Paint via SwapIn.
    bool bScaleImageMap         :1; ///< Scale image map in SetTwipSize.

    std::shared_ptr< SwAsyncRetrieveInputStreamThreadConsumer > mpThreadConsumer;
    bool mbLinkedInputStreamReady;
    css::uno::Reference<css::io::XInputStream> mxInputStream;
    bool mbIsStreamReadOnly;

    SwGrfNode( const SwNodeIndex& rWhere,
               const OUString& rGrfName, const OUString& rFltName,
               const Graphic* pGraphic,
               SwGrfFormatColl* pGrfColl,
               SwAttrSet* pAutoAttr );
    ///< Ctor for reading (SW/G) without graphics.
    SwGrfNode( const SwNodeIndex& rWhere,
               const OUString& rGrfName, const OUString& rFltName,
               SwGrfFormatColl* pGrfColl,
               SwAttrSet* pAutoAttr );
    SwGrfNode( const SwNodeIndex& rWhere,
               const GraphicObject& rGrfObj,
               SwGrfFormatColl* pGrfColl,
               SwAttrSet* pAutoAttr );

    void InsertLink( const OUString& rGrfName, const OUString& rFltName );
    bool ImportGraphic( SvStream& rStrm );

    DECL_LINK( SwapGraphic, const GraphicObject*, SvStream* );

    /** helper method to determine stream for the embedded graphic.

        Important note: caller of this method has to handle the thrown exceptions
        Storage, which should contain the stream of the embedded graphic, is
        provided via parameter. Otherwise the returned stream will be closed
        after the method returns, because its parent stream is closed and deleted.
        Proposed name of embedded graphic stream is also provided by parameter.

        @author OD

        @param _refPics
        input parameter - reference to storage, which should contain the
        embedded graphic stream.

        @param rStrmName
        input parameter - proposed name of the embedded graphic stream.

        @return SvStream*
        new created stream of the embedded graphic, which has to be destroyed
        after its usage. Could be NULL, if the stream isn't found.
    */
    SvStream* GetStreamForEmbedGrf(
            const css::uno::Reference< css::embed::XStorage >& _refPics,
            const OUString& rStreamName ) const;

    /** helper method to get a substorage of the document storage for readonly access.

        A substorage with the specified name will be opened readonly. If the provided
        name is empty the root storage will be returned.

        @param _aStgName
        input parameter - name of substorage. Can be empty.

        @return XStorage
        reference to substorage or the root storage
    */
    css::uno::Reference< css::embed::XStorage > GetDocSubstorageOrRoot(
                                                const OUString& aStgName ) const;

    /// allow reaction on change of content of GraphicObject, so always call
    /// when GraphicObject content changes
    void onGraphicChanged();

public:
    virtual ~SwGrfNode() override;
    const Graphic&          GetGrf(bool bWait = false) const;
    const GraphicObject&    GetGrfObj(bool bWait = false) const;
    const GraphicObject* GetReplacementGrfObj() const;
    virtual SwContentNode *SplitContentNode( const SwPosition & ) override;

    /// isolated only way to set GraphicObject to allow more actions when doing so
    void SetGraphic(const Graphic& rGraphic, const OUString& rLink);

    /// wrappers for non-const calls at GraphicObject
    void StartGraphicAnimation(OutputDevice* pOut, const Point& rPt, const Size& rSz, long nExtraData, OutputDevice* pFirstFrameOutDev)
    { maGrfObj.StartAnimation(pOut, rPt, rSz, nExtraData, pFirstFrameOutDev); }
    void StopGraphicAnimation(OutputDevice* pOut, long nExtraData) { maGrfObj.StopAnimation(pOut, nExtraData); }

    virtual Size GetTwipSize() const override;
    void SetTwipSize( const Size& rSz );

    bool IsTransparent() const;

    bool IsAnimated() const              { return maGrfObj.IsAnimated(); }

    bool IsChgTwipSize() const           { return bChgTwipSize; }
    void SetChgTwipSize( bool b)
    {
        bChgTwipSize = b;
    }

    bool IsGraphicArrived() const        { return bGraphicArrived; }
    void SetGraphicArrived( bool b )     { bGraphicArrived = b; }

    bool IsFrameInPaint() const          { return bFrameInPaint; }
    void SetFrameInPaint( bool b )       { bFrameInPaint = b; }

    bool IsScaleImageMap() const         { return bScaleImageMap; }
    void SetScaleImageMap( bool b )      { bScaleImageMap = b; }

    /// in ndcopy.cxx
    virtual SwContentNode* MakeCopy( SwDoc*, const SwNodeIndex& ) const override;

    /** Re-read in case graphic was not OK. The current one
       gets replaced by the new one. */
    bool ReRead( const OUString& rGrfName, const OUString& rFltName,
                 const Graphic* pGraphic = nullptr,
                 const GraphicObject* pGrfObj = nullptr,
                 bool bModify = true );
private:
    /// Loading of graphic immediately before displaying.
    bool SwapIn( bool bWaitForData = false );
    /// Remove graphic in order to free memory.
    bool SwapOut();

public:
    bool HasEmbeddedStreamName() const { return maGrfObj.HasUserData(); }

    /// Is this node selected by any shell?
    bool IsSelected() const;

    /// Communicate to graphic that node is in Undo-range.
    virtual bool SavePersistentData() override;
    virtual bool RestorePersistentData() override;

    /// Query link-data.
    bool IsGrfLink() const                  { return refLink.is(); }
    bool IsLinkedFile() const;
    bool IsLinkedDDE() const;
    const tools::SvRef<sfx2::SvBaseLink>& GetLink() const    { return refLink; }
    bool GetFileFilterNms( OUString* pFileNm, OUString* pFilterNm ) const;
    void ReleaseLink();

    /** Scale an image-map: the image-map becomes zoomed in / out by
       factor between graphic-size and border-size. */
    void ScaleImageMap();

    /// Returns the with our graphic attributes filled Graphic-Attr-Structure.
    GraphicAttr& GetGraphicAttr( GraphicAttr&, const SwFrame* pFrame ) const;

    std::weak_ptr< SwAsyncRetrieveInputStreamThreadConsumer > GetThreadConsumer() { return mpThreadConsumer;}
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
    return refLink.is() && OBJECT_CLIENT_GRF == refLink->GetObjType();
}

inline bool SwGrfNode::IsLinkedDDE() const
{
    return refLink.is() && OBJECT_CLIENT_DDE == refLink->GetObjType();
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
