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
#ifndef INCLUDED_SW_INC_FRMFMT_HXX
#define INCLUDED_SW_INC_FRMFMT_HXX

#include <com/sun/star/text/PositionLayoutDir.hpp>
#include <cppuhelper/weakref.hxx>
#include <tools/gen.hxx>
#include <format.hxx>
#include "swdllapi.h"
#include <list>

class SwFlyFrame;
class SwAnchoredObject;
class Graphic;
class ImageMap;
class IMapObject;
class SwRect;
class SwContact;
class SdrObject;
class SwRootFrame;
class SwFlyDrawContact;
namespace sw
{
    class DocumentLayoutManager;
    // This is cheating: we are not really decoupling much with this hint.
    // SwDrawFrameFormat should probably bookkeep its SdrObject (and
    // SwDrawFrameFormat too) as members
    struct SW_DLLPUBLIC FindSdrObjectHint final : SfxHint
    {
        SdrObject*& m_rpObject;
        FindSdrObjectHint(SdrObject*& rpObject) : m_rpObject(rpObject) {};
        virtual ~FindSdrObjectHint() override;
    };
}
class SwFrameFormats;

/// Style of a layout element.
class SW_DLLPUBLIC SwFrameFormat: public SwFormat
{
    friend class SwDoc;
    friend class SwPageDesc;    ///< Is allowed to call protected CTor.
    friend class ::sw::DocumentLayoutManager; ///< Is allowed to call protected CTor.
    friend class SwFrameFormats;     ///< Is allowed to update the list backref.
    friend class SwTextBoxHelper;

    css::uno::WeakReference<css::uno::XInterface> m_wXObject;

    //UUUU DrawingLayer FillAttributes in a preprocessed form for primitive usage
    drawinglayer::attribute::SdrAllFillAttributesHelperPtr  maFillAttributes;

    // The assigned SwFrmFmt list.
    SwFrameFormats *m_ffList;

    SwFrameFormat *m_pOtherTextBoxFormat;

    struct change_name
    {
        change_name(const OUString &rName) : mName(rName) {}
        void operator()(SwFormat *pFormat) { pFormat->m_aFormatName = mName; }
        const OUString &mName;
    };

protected:
    SwFrameFormat(
        SwAttrPool& rPool,
        const sal_Char* pFormatNm,
        SwFrameFormat *pDrvdFrame,
        sal_uInt16 nFormatWhich = RES_FRMFMT,
        const sal_uInt16* pWhichRange = nullptr);

    SwFrameFormat(
        SwAttrPool& rPool,
        const OUString &rFormatNm,
        SwFrameFormat *pDrvdFrame,
        sal_uInt16 nFormatWhich = RES_FRMFMT,
        const sal_uInt16* pWhichRange = nullptr);

    virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem* pNewValue ) override;

    SwFrameFormat* GetOtherTextBoxFormat() const { return m_pOtherTextBoxFormat; }
    void SetOtherTextBoxFormat( SwFrameFormat *pFormat );

public:
    virtual ~SwFrameFormat() override;

    /// Destroys all Frames in aDepend (Frames are identified via dynamic_cast).
    virtual void DelFrames();

    /// Creates the views.
    virtual void MakeFrames();

    virtual Graphic MakeGraphic( ImageMap* pMap = nullptr );

    /**  @return the IMapObject defined at format (Fly)
        in the ImageMap at position Point.
        rPoint - test on DocPosition.
        pFly - optional FlyFrame, in case it is already known. */
    IMapObject* GetIMapObject( const Point& rPoint,
                                const SwFlyFrame *pFly = nullptr ) const;

    /** @return the real size of the frame - or an empty rectangle
       if no layout exists.
       If pPoint is given, look for the frame closest to it. */
    SwRect FindLayoutRect( const bool bPrtArea = false,
                           const Point* pPoint = nullptr ) const;

    /** @return the SdrObject, that is connected to the ContactObject.
       Only DrawFrameFormats are connected to the "real SdrObject". FlyFrameFormats
       are connected to a Master and all FlyFrames has the "real SdrObject".
       "Real SdrObject" has position and a Z-order. */
    SdrObject* FindSdrObject()
    {
        SdrObject* pObject(nullptr);
        CallSwClientNotify(sw::FindSdrObjectHint(pObject));
        return pObject;
    }
    const SdrObject *FindSdrObject() const
        { return const_cast<SwFrameFormat*>(this)->FindSdrObject(); }

          SdrObject *FindRealSdrObject();
    const SdrObject *FindRealSdrObject() const
        { return const_cast<SwFrameFormat*>(this)->FindRealSdrObject(); }

    bool IsLowerOf( const SwFrameFormat& rFormat ) const;

    enum tLayoutDir
    {
        HORI_L2R,
        HORI_R2L,
        VERT_R2L
    };

    virtual SwFrameFormat::tLayoutDir GetLayoutDir() const;
    virtual void SetLayoutDir( const SwFrameFormat::tLayoutDir _eLayoutDir );

    virtual sal_Int16 GetPositionLayoutDir() const;
    virtual void SetPositionLayoutDir( const sal_Int16 _nPositionLayoutDir );

    virtual OUString GetDescription() const;

    SAL_DLLPRIVATE css::uno::WeakReference<css::uno::XInterface> const& GetXObject() const
            { return m_wXObject; }
    SAL_DLLPRIVATE void SetXObject(css::uno::Reference<css::uno::XInterface> const& xObject)
            { m_wXObject = xObject; }

    DECL_FIXEDMEMPOOL_NEWDEL_DLL(SwFrameFormat)
    void RegisterToFormat( SwFormat& rFormat );

    //UUUU Access to DrawingLayer FillAttributes in a preprocessed form for primitive usage
    virtual drawinglayer::attribute::SdrAllFillAttributesHelperPtr getSdrAllFillAttributesHelper() const override;
    virtual bool supportsFullDrawingLayerFillAttributeSet() const override;

    void dumpAsXml(struct _xmlTextWriter* pWriter) const;

    virtual void SetName( const OUString& rNewName, bool bBroadcast=false ) SAL_OVERRIDE;
};

// The FlyFrame-Format

class SW_DLLPUBLIC SwFlyFrameFormat: public SwFrameFormat
{
    friend class SwDoc;
    OUString msTitle;
    OUString msDesc;

    /** Both not existent.
       it stores the previous position of Prt rectangle from RequestObjectResize
       so it can be used to move frames of non-resizable objects to align them correctly
       when they get borders (this is done in SwWrtShell::CalcAndGetScale) */
    Point   m_aLastFlyFramePrtRectPos;

    SwFlyFrameFormat( const SwFlyFrameFormat &rCpy ) = delete;
    SwFlyFrameFormat &operator=( const SwFlyFrameFormat &rCpy ) = delete;

protected:
    SwFlyFrameFormat( SwAttrPool& rPool, const OUString &rFormatNm,
                    SwFrameFormat *pDrvdFrame )
        : SwFrameFormat( rPool, rFormatNm, pDrvdFrame, RES_FLYFRMFMT )
    {}

public:
    virtual ~SwFlyFrameFormat() override;

    /// Creates the views.
    virtual void MakeFrames() override;

    SwFlyFrame* GetFrame( const Point* pDocPos = nullptr ) const;

    SwAnchoredObject* GetAnchoredObj() const;

    virtual Graphic MakeGraphic( ImageMap* pMap = nullptr ) override;

    virtual bool GetInfo( SfxPoolItem& rInfo ) const override;

    OUString GetObjTitle() const;
    void SetObjTitle( const OUString& rTitle, bool bBroadcast = false );
    OUString GetObjDescription() const;
    void SetObjDescription( const OUString& rDescription, bool bBroadcast = false );

    /** SwFlyFrameFormat::IsBackgroundTransparent

        Override virtual method and its default implementation,
        because format of fly frame provides transparent backgrounds.
        Method determines, if background of fly frame is transparent.

        @author OD

        @return true, if background color is transparent, but not "no fill"
        or a existing background graphic is transparent.
    */
    virtual bool IsBackgroundTransparent() const override;

    /** SwFlyFrameFormat::IsBackgroundBrushInherited

        Method to determine, if the brush for drawing the
        background is "inherited" from its parent/grandparent.
        This is the case, if no background graphic is set and the background
        color is "no fill"/"auto fill"

        @author OD

        @return true, if background brush is "inherited" from parent/grandparent
    */
    bool IsBackgroundBrushInherited() const;

    const Point & GetLastFlyFramePrtRectPos() const       { return m_aLastFlyFramePrtRectPos; }
    void SetLastFlyFramePrtRectPos( const Point &rPoint ) { m_aLastFlyFramePrtRectPos = rPoint; }

    DECL_FIXEDMEMPOOL_NEWDEL(SwFlyFrameFormat)
};

//The DrawFrame-Format


class SwDrawFrameFormat;
class SwDrawContact;
class SdrTextObj;

namespace sw
{
    enum class DrawFrameFormatHintId {
        DYING,
        DYING_FLYFRAMEFORMAT, /* possibly can be merged with DYING, if all client handle it and handle it the same */
        PREPPASTING,
        PREP_INSERT_FLY,
        PREP_DELETE_FLY,
        PAGE_OUT_OF_BOUNDS,
        MAKE_FRAMES,
        DELETE_FRAMES,
        POST_RESTORE_FLY_ANCHOR,
    };
    struct SW_DLLPUBLIC DrawFrameFormatHint final: SfxHint
    {
        DrawFrameFormatHintId m_eId;
        DrawFrameFormatHint(DrawFrameFormatHintId eId) : m_eId(eId) {};
        virtual ~DrawFrameFormatHint() override;
    };
    struct SW_DLLPUBLIC CheckDrawFrameFormatLayerHint final: SfxHint
    {
        bool* m_bCheckControlLayer;
        CheckDrawFrameFormatLayerHint(bool* bCheckControlLayer) : m_bCheckControlLayer(bCheckControlLayer) {};
        virtual ~CheckDrawFrameFormatLayerHint() override;
    };
    struct SW_DLLPUBLIC ContactChangedHint final: SfxHint
    {
        SdrObject** m_ppObject;
        ContactChangedHint(SdrObject** ppObject) : m_ppObject(ppObject) {};
        virtual ~ContactChangedHint() override;
    };
    struct SW_DLLPUBLIC DrawFormatLayoutCopyHint final : SfxHint
    {
        SwDrawFrameFormat& m_rDestFormat;
        SwDoc& m_rDestDoc;
        DrawFormatLayoutCopyHint(SwDrawFrameFormat& rDestFormat, SwDoc& rDestDoc) : m_rDestFormat(rDestFormat), m_rDestDoc(rDestDoc) {};
        virtual ~DrawFormatLayoutCopyHint() override;
    };
    enum class WW8AnchorConv
    {
        NO_CONV,
        CONV2PG,
        CONV2COL_OR_PARA,
        CONV2CHAR,
        CONV2LINE,
        RELTOTABLECELL
    };
    struct WW8AnchorConvResult final
    {
        WW8AnchorConv m_eHoriConv;
        WW8AnchorConv m_eVertConv;
        bool m_bConverted;
        Point m_aPos;
        WW8AnchorConvResult(WW8AnchorConv eHoriConv, WW8AnchorConv eVertConv) : m_eHoriConv(eHoriConv), m_eVertConv(eVertConv), m_bConverted(false) {};
    };
    struct SW_DLLPUBLIC WW8AnchorConvHint final : SfxHint
    {
        WW8AnchorConvResult& m_rResult;
        WW8AnchorConvHint(WW8AnchorConvResult& rResult) : m_rResult(rResult) {};
        virtual ~WW8AnchorConvHint() override;
    };
    struct SW_DLLPUBLIC RestoreFlyAnchorHint final : SfxHint
    {
        const Point m_aPos;
        RestoreFlyAnchorHint(Point aPos) : m_aPos(aPos) {};
        virtual ~RestoreFlyAnchorHint() override;
    };
    struct SW_DLLPUBLIC CreatePortionHint final : SfxHint
    {
        SwDrawContact** m_ppContact;
        CreatePortionHint(SwDrawContact** ppContact) : m_ppContact(ppContact) {};
        virtual ~CreatePortionHint() override;
    };
    struct SW_DLLPUBLIC CollectTextObjectsHint final : SfxHint
    {
        std::list<SdrTextObj*>& m_rTextObjects;
        CollectTextObjectsHint(std::list<SdrTextObj*>& rTextObjects) : m_rTextObjects(rTextObjects) {};
        virtual ~CollectTextObjectsHint() override;
    };
    struct SW_DLLPUBLIC GetZOrderHint final : SfxHint
    {
        sal_uInt32& m_rnZOrder;
        GetZOrderHint(sal_uInt32& rnZOrder) : m_rnZOrder(rnZOrder) {};
        virtual ~GetZOrderHint() override;
    };
    struct SW_DLLPUBLIC GetObjectConnectedHint final : SfxHint
    {
        bool& m_risConnected;
        const SwRootFrame* m_pRoot;
        GetObjectConnectedHint(bool& risConnected, const SwRootFrame* pRoot) : m_risConnected(risConnected), m_pRoot(pRoot) {};
        virtual ~GetObjectConnectedHint() override;
    };
    struct SW_DLLPUBLIC KillDrawHint final : SfxHint
    {
        const SwFrame* m_pKillingFrame;
        bool& m_rbOtherFramesAround;
        SwFlyDrawContact*& m_rpContact;
        KillDrawHint(const SwFrame* pKillingFrame, bool& rbOtherFramesAround, SwFlyDrawContact*& rpContact) : m_pKillingFrame(pKillingFrame), m_rbOtherFramesAround(rbOtherFramesAround), m_rpContact(rpContact) {};
        virtual ~KillDrawHint() override;
    };
}

class SW_DLLPUBLIC SwDrawFrameFormat: public SwFrameFormat
{
    friend class SwDoc;

    mutable const SdrObject * m_pSdrObjectCached;
    mutable OUString m_sSdrObjectCachedComment;

    SwDrawFrameFormat( const SwDrawFrameFormat &rCpy ) = delete;
    SwDrawFrameFormat &operator=( const SwDrawFrameFormat &rCpy ) = delete;

    SwFrameFormat::tLayoutDir meLayoutDir;

    sal_Int16 mnPositionLayoutDir;

    bool mbPosAttrSet;

protected:
    SwDrawFrameFormat( SwAttrPool& rPool, const OUString &rFormatNm,
                    SwFrameFormat *pDrvdFrame )
        : SwFrameFormat( rPool, rFormatNm, pDrvdFrame, RES_DRAWFRMFMT ),
          m_pSdrObjectCached(nullptr),
          meLayoutDir( SwFrameFormat::HORI_L2R ),

          mnPositionLayoutDir( css::text::PositionLayoutDir::PositionInLayoutDirOfAnchor ),

          mbPosAttrSet( false )
    {}

public:
    virtual ~SwDrawFrameFormat() override;

    /** DrawObjects are removed from the arrays at the layout.
     The DrawObjects are marked as deleted. */
    virtual void DelFrames() override;

    /** Register DrawObjects in the arrays at layout.
     Reset delete marks. */
    virtual void MakeFrames() override;

    virtual Graphic MakeGraphic( ImageMap* pMap = nullptr ) override;

    virtual SwFrameFormat::tLayoutDir GetLayoutDir() const override;
    virtual void SetLayoutDir( const SwFrameFormat::tLayoutDir _eLayoutDir ) override;

    virtual sal_Int16 GetPositionLayoutDir() const override;
    virtual void SetPositionLayoutDir( const sal_Int16 _nPositionLayoutDir ) override;

    inline bool IsPosAttrSet() const { return mbPosAttrSet; }
    inline void PosAttrSet() { mbPosAttrSet = true; }

    virtual OUString GetDescription() const override;

    DECL_FIXEDMEMPOOL_NEWDEL(SwDrawFrameFormat);
};

namespace sw {

SW_DLLPUBLIC bool IsFlyFrameFormatInHeader(const SwFrameFormat& rFormat);

void CheckAnchoredFlyConsistency(SwDoc const& rDoc);

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
