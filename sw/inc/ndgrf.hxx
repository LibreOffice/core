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
#ifndef _NDGRF_HXX
#define _NDGRF_HXX
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

    GraphicObject aGrfObj;
    ::sfx2::SvBaseLinkRef refLink;       // If graphics only as link then pointer is set.
    Size nGrfSize;
    String aNewStrmName;        // SW3/XML: new stream name (either SW3 stream
                                // name or package url)
    String aLowResGrf;          // HTML: LowRes graphics (substitute until regular HighRes graphics is loaded).
    BOOL bTransparentFlagValid  :1;
    BOOL bInSwapIn              :1;

    BOOL bGrafikArrived         :1;
    BOOL bChgTwipSize           :1;
    BOOL bChgTwipSizeFromPixel  :1;
    BOOL bLoadLowResGrf         :1;
    BOOL bFrameInPaint          :1; // To avoid Start-/EndActions in Paint via SwapIn.
    BOOL bScaleImageMap         :1; // Scale image map in SetTwipSize.

    boost::shared_ptr< SwAsyncRetrieveInputStreamThreadConsumer > mpThreadConsumer;
    bool mbLinkedInputStreamReady;
    com::sun::star::uno::Reference<com::sun::star::io::XInputStream> mxInputStream;
    sal_Bool mbIsStreamReadOnly;

    SwGrfNode( const SwNodeIndex& rWhere,
               const String& rGrfName, const String& rFltName,
               const Graphic* pGraphic,
               SwGrfFmtColl* pGrfColl,
               SwAttrSet* pAutoAttr = 0 );
    // Ctor for reading (SW/G) without graphics.
    SwGrfNode( const SwNodeIndex& rWhere,
               const String& rGrfName, const String& rFltName,
               SwGrfFmtColl* pGrfColl,
               SwAttrSet* pAutoAttr = 0 );
    SwGrfNode( const SwNodeIndex& rWhere,
               const GraphicObject& rGrfObj,
               SwGrfFmtColl* pGrfColl,
               SwAttrSet* pAutoAttr = 0 );

    void InsertLink( const String& rGrfName, const String& rFltName );
    BOOL ImportGraphic( SvStream& rStrm );
    BOOL HasStreamName() const { return aGrfObj.HasUserData(); }
    // adjust return type and rename method to
    // indicate that its an private one.

    // embedded graphic stream couldn't be inside a 3.1 - 5.2 storage any more.
    // Thus, return value isn't needed any more.
    void _GetStreamStorageNames( String& rStrmName, String& rStgName ) const;

    void DelStreamName();
    DECL_LINK( SwapGraphic, GraphicObject* );

    /** helper method to determine stream for the embedded graphic.

        Important note: caller of this method has to handle the thrown exceptions
        Storage, which should contain the stream of the embedded graphic, is
        provided via parameter. Otherwise the returned stream will be closed
        after the the method returns, because its parent stream is closed and deleted.
        Proposed name of embedded graphic stream is also provided by parameter.

        @author OD

        @param _refPics
        input parameter - reference to storage, which should contain the
        embedded graphic stream.

        @param _aStrmName
        input parameter - proposed name of the embedded graphic stream.

        @return SvStream*
        new created stream of the embedded graphic, which has to be destroyed
        after its usage. Could be NULL, if the stream isn't found.
    */
    SvStream* _GetStreamForEmbedGrf(
            const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& _refPics,
            String& _aStrmName ) const;

    /** helper method to get a substorage of the document storage for readonly access.

        A substorage with the specified name will be opened readonly. If the provided
        name is empty the root storage will be returned.

        @param _aStgName
        input parameter - name of substorage. Can be empty.

        @return XStorage
        reference to substorage or the root storage
    */
    ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage > _GetDocSubstorageOrRoot(
                                                const String& aStgName ) const;

public:
    virtual ~SwGrfNode();
    const Graphic&          GetGrf() const      { return aGrfObj.GetGraphic(); }
    const GraphicObject&    GetGrfObj() const   { return aGrfObj; }
          GraphicObject&    GetGrfObj()         { return aGrfObj; }

    virtual SwCntntNode *SplitCntntNode( const SwPosition & );

    virtual Size GetTwipSize() const;
#ifndef _FESHVIEW_ONLY_INLINE_NEEDED
    void SetTwipSize( const Size& rSz );

    BOOL IsTransparent() const;

    inline BOOL IsAnimated() const              { return aGrfObj.IsAnimated(); }

    inline BOOL IsChgTwipSize() const           { return bChgTwipSize; }
    inline BOOL IsChgTwipSizeFromPixel() const  { return bChgTwipSizeFromPixel; }
    inline void SetChgTwipSize( BOOL b, BOOL bFromPx=FALSE )        { bChgTwipSize = b; bChgTwipSizeFromPixel = bFromPx; }

    inline BOOL IsGrafikArrived() const         { return bGrafikArrived; }
    inline void SetGrafikArrived( BOOL b )      { bGrafikArrived = b; }

    inline BOOL IsFrameInPaint() const          { return bFrameInPaint; }
    inline void SetFrameInPaint( BOOL b )       { bFrameInPaint = b; }

    inline BOOL IsScaleImageMap() const         { return bScaleImageMap; }
    inline void SetScaleImageMap( BOOL b )      { bScaleImageMap = b; }
#endif
        // in ndcopy.cxx
    virtual SwCntntNode* MakeCopy( SwDoc*, const SwNodeIndex& ) const;
#ifndef _FESHVIEW_ONLY_INLINE_NEEDED

    // Re-read in case graphic was not OK. The current one
    // gets replaced by the new one.
    BOOL ReRead( const String& rGrfName, const String& rFltName,
                 const Graphic* pGraphic = 0,
                 const GraphicObject* pGrfObj = 0,
                 BOOL bModify = TRUE );
    // Loading of graphic immediately before displaying.
    short SwapIn( BOOL bWaitForData = FALSE );
    // Remove graphic in order to free memory.
    short SwapOut();
    // Access to storage stream-name.
    void SetStreamName( const String& r ) { aGrfObj.SetUserData( r ); }
    void SetNewStreamName( const String& r ) { aNewStrmName = r; }
    // Is this node selected by any shell?
    BOOL IsSelected() const;
#endif

    // Communicate to graphic that node is in Undo-range.
    virtual BOOL SavePersistentData();
    virtual BOOL RestorePersistentData();

#ifndef _FESHVIEW_ONLY_INLINE_NEEDED

    // Query link-data.
    BOOL IsGrfLink() const                  { return refLink.Is(); }
    inline BOOL IsLinkedFile() const;
    inline BOOL IsLinkedDDE() const;
    ::sfx2::SvBaseLinkRef GetLink() const    { return refLink; }
    BOOL GetFileFilterNms( String* pFileNm, String* pFilterNm ) const;
    void ReleaseLink();

    // Scale an image-map: the image-map becomes zoomed in / out by
    // factor between graphic-size and border-size.
    void ScaleImageMap();

    // Returns the with our graphic attributes filled Graphic-Attr-Structure.
    GraphicAttr& GetGraphicAttr( GraphicAttr&, const SwFrm* pFrm ) const;

#endif
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

#ifndef _FESHVIEW_ONLY_INLINE_NEEDED
inline BOOL SwGrfNode::IsLinkedFile() const
{
    return refLink.Is() && OBJECT_CLIENT_GRF == refLink->GetObjType();
}
inline BOOL SwGrfNode::IsLinkedDDE() const
{
    return refLink.Is() && OBJECT_CLIENT_DDE == refLink->GetObjType();
}
#endif

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
