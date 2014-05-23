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
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
class SwAsyncRetrieveInputStreamThreadConsumer;

class SwGrfFmtColl;
class SwDoc;
class GraphicAttr;
class SvStorage;

// SwGrfNode
class SW_DLLPUBLIC SwGrfNode: public SwNoTxtNode
{
    friend class SwNodes;

    GraphicObject maGrfObj;
    GraphicObject *mpReplacementGraphic;
    ::sfx2::SvBaseLinkRef refLink;       ///< If graphics only as link then pointer is set.
    Size nGrfSize;
    OUString aLowResGrf;                   ///< HTML: LowRes graphics (substitute until regular HighRes graphics is loaded).
    sal_Bool bTransparentFlagValid  :1;
    sal_Bool bInSwapIn              :1;

    sal_Bool bGrafikArrived         :1;
    sal_Bool bChgTwipSize           :1;
    sal_Bool bChgTwipSizeFromPixel  :1;
    sal_Bool bLoadLowResGrf         :1;
    sal_Bool bFrameInPaint          :1; ///< To avoid Start-/EndActions in Paint via SwapIn.
    sal_Bool bScaleImageMap         :1; ///< Scale image map in SetTwipSize.

    boost::shared_ptr< SwAsyncRetrieveInputStreamThreadConsumer > mpThreadConsumer;
    bool mbLinkedInputStreamReady;
    com::sun::star::uno::Reference<com::sun::star::io::XInputStream> mxInputStream;
    sal_Bool mbIsStreamReadOnly;

    SwGrfNode( const SwNodeIndex& rWhere,
               const OUString& rGrfName, const OUString& rFltName,
               const Graphic* pGraphic,
               SwGrfFmtColl* pGrfColl,
               SwAttrSet* pAutoAttr = 0 );
    ///< Ctor for reading (SW/G) without graphics.
    SwGrfNode( const SwNodeIndex& rWhere,
               const OUString& rGrfName, const OUString& rFltName,
               SwGrfFmtColl* pGrfColl,
               SwAttrSet* pAutoAttr = 0 );
    SwGrfNode( const SwNodeIndex& rWhere,
               const GraphicObject& rGrfObj,
               SwGrfFmtColl* pGrfColl,
               SwAttrSet* pAutoAttr = 0 );

    void InsertLink( const OUString& rGrfName, const OUString& rFltName );
    sal_Bool ImportGraphic( SvStream& rStrm );
    /** adjust return type and rename method to
       indicate that its an private one. */

    void DelStreamName();
    DECL_LINK( SwapGraphic, GraphicObject* );

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
    SvStream* _GetStreamForEmbedGrf(
            const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& _refPics,
            const OUString& rStreamName ) const;

    /** helper method to get a substorage of the document storage for readonly access.

        A substorage with the specified name will be opened readonly. If the provided
        name is empty the root storage will be returned.

        @param _aStgName
        input parameter - name of substorage. Can be empty.

        @return XStorage
        reference to substorage or the root storage
    */
    ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage > _GetDocSubstorageOrRoot(
                                                const OUString& aStgName ) const;

public:
    virtual ~SwGrfNode();
    const Graphic&          GetGrf() const      { return maGrfObj.GetGraphic(); }
    const GraphicObject&    GetGrfObj() const   { return maGrfObj; }
    const GraphicObject* GetReplacementGrfObj() const;
    virtual SwCntntNode *SplitCntntNode( const SwPosition & );

    /// isolated only way to set GraphicObject to allow more actions when doing so
    void SetGraphic(const Graphic& rGraphic, const OUString& rLink);

    /// wrappers for non-const calls at GraphicObject
    void ReleaseGraphicFromCache() { maGrfObj.ReleaseFromCache(); }
    void StartGraphicAnimation(OutputDevice* pOut, const Point& rPt, const Size& rSz, long nExtraData = 0, const GraphicAttr* pAttr = NULL, sal_uLong nFlags = GRFMGR_DRAW_STANDARD, OutputDevice* pFirstFrameOutDev = NULL) { maGrfObj.StartAnimation(pOut, rPt, rSz, nExtraData, pAttr, nFlags, pFirstFrameOutDev); }
    void StopGraphicAnimation(OutputDevice* pOut = NULL, long nExtraData = 0) { maGrfObj.StopAnimation(pOut, nExtraData); }

    /// allow reaction on change of content of GraphicObject, so always call
    /// when GraphicObject content changes
    void onGraphicChanged();

    virtual Size GetTwipSize() const;
    void SetTwipSize( const Size& rSz );

    sal_Bool IsTransparent() const;

    inline sal_Bool IsAnimated() const              { return maGrfObj.IsAnimated(); }

    inline sal_Bool IsChgTwipSize() const           { return bChgTwipSize; }
    inline sal_Bool IsChgTwipSizeFromPixel() const  { return bChgTwipSizeFromPixel; }
    inline void SetChgTwipSize( sal_Bool b, sal_Bool bFromPx=sal_False )        { bChgTwipSize = b; bChgTwipSizeFromPixel = bFromPx; }

    inline sal_Bool IsGrafikArrived() const         { return bGrafikArrived; }
    inline void SetGrafikArrived( sal_Bool b )      { bGrafikArrived = b; }

    inline sal_Bool IsFrameInPaint() const          { return bFrameInPaint; }
    inline void SetFrameInPaint( sal_Bool b )       { bFrameInPaint = b; }

    inline sal_Bool IsScaleImageMap() const         { return bScaleImageMap; }
    inline void SetScaleImageMap( sal_Bool b )      { bScaleImageMap = b; }

    /// in ndcopy.cxx
    virtual SwCntntNode* MakeCopy( SwDoc*, const SwNodeIndex& ) const;

    /** Re-read in case graphic was not OK. The current one
       gets replaced by the new one. */
    sal_Bool ReRead( const OUString& rGrfName, const OUString& rFltName,
                 const Graphic* pGraphic = 0,
                 const GraphicObject* pGrfObj = 0,
                 sal_Bool bModify = sal_True );
    /// Loading of graphic immediately before displaying.
    short SwapIn( sal_Bool bWaitForData = sal_False );
    /// Remove graphic in order to free memory.
    short SwapOut();
    bool HasEmbeddedStreamName() const { return maGrfObj.HasUserData(); }
    /// applying new stream name for embedded graphic - needed as saving the document might change this stream name
    void ApplyNewEmbeddedStreamName(const OUString& r)
    {
        maGrfObj.SetUserData(r);
    }
    /// Is this node selected by any shell?
    sal_Bool IsSelected() const;

    /// Communicate to graphic that node is in Undo-range.
    virtual sal_Bool SavePersistentData();
    virtual sal_Bool RestorePersistentData();

    /// Query link-data.
    bool IsGrfLink() const                  { return refLink.Is(); }
    inline bool IsLinkedFile() const;
    inline bool IsLinkedDDE() const;
    ::sfx2::SvBaseLinkRef GetLink() const    { return refLink; }
    bool GetFileFilterNms( OUString* pFileNm, OUString* pFilterNm ) const;
    void ReleaseLink();

    /** Scale an image-map: the image-map becomes zoomed in / out by
       factor between graphic-size and border-size. */
    void ScaleImageMap();

    /// Returns the with our graphic attributes filled Graphic-Attr-Structure.
    GraphicAttr& GetGraphicAttr( GraphicAttr&, const SwFrm* pFrm ) const;

    boost::weak_ptr< SwAsyncRetrieveInputStreamThreadConsumer > GetThreadConsumer();
    bool IsLinkedInputStreamReady() const;
    void TriggerAsyncRetrieveInputStream();
    void ApplyInputStream(
        com::sun::star::uno::Reference<com::sun::star::io::XInputStream> xInputStream,
        const sal_Bool bIsStreamReadOnly );
    void UpdateLinkWithInputStream();
    bool IsAsyncRetrieveInputStreamPossible() const;
};


// Inline methods from Node.hxx - it is only now that we know TxtNode!!
inline       SwGrfNode   *SwNode::GetGrfNode()
{
     return ND_GRFNODE == nNodeType ? (SwGrfNode*)this : 0;
}

inline const SwGrfNode   *SwNode::GetGrfNode() const
{
     return ND_GRFNODE == nNodeType ? (const SwGrfNode*)this : 0;
}

inline bool SwGrfNode::IsLinkedFile() const
{
    return refLink.Is() && OBJECT_CLIENT_GRF == refLink->GetObjType();
}

inline bool SwGrfNode::IsLinkedDDE() const
{
    return refLink.Is() && OBJECT_CLIENT_DDE == refLink->GetObjType();
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
