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

#include <memory>
#include <com/sun/star/text/PositionLayoutDir.hpp>
#include <cppuhelper/weakref.hxx>
#include <tools/gen.hxx>
namespace sw
{
    template<class T> class FrameFormats;
    class SpzFrameFormat;
}
#include "format.hxx"
#include "hintids.hxx"
#include "swdllapi.h"
#include <list>
#include "textboxhelper.hxx"

class Graphic;
class IMapObject;
class ImageMap;
class SdrObject;
class SwAnchoredObject;
class SwDrawFrameFormat;
class SwFlyDrawContact;
class SwFlyFrame;
class SwFlyFrameFormat;
class SwRect;
class SwRootFrame;
class SwTableBox;

namespace sw
{
    class DocumentLayoutManager;
    // This is cheating: we are not really decoupling much with this hint.
    // SwDrawFrameFormat should probably bookkeep its SdrObject (and
    // SwDrawFrameFormat too) as members
    struct SW_DLLPUBLIC FindSdrObjectHint final : SfxHint
    {
        SdrObject*& m_rpObject;
        FindSdrObjectHint(SdrObject*& rpObject)
            : SfxHint(SfxHintId::SwFindSdrObject), m_rpObject(rpObject) {};
        virtual ~FindSdrObjectHint() override;
    };
    template<class T> class FrameFormats;
    class SpzFrameFormat;
}
class SwFormatsBase;
class SwTableFormat;

/// Style of a layout element.
class SAL_DLLPUBLIC_RTTI SwFrameFormat
    : public SwFormat
{
    friend class SwDoc;
    friend class SwPageDesc;    ///< Is allowed to call protected CTor.
    friend class ::sw::DocumentLayoutManager; ///< Is allowed to call protected CTor.
    friend class sw::FrameFormats<SwTableFormat*>;     ///< Is allowed to update the list backref.
    friend class sw::FrameFormats<sw::SpzFrameFormat*>;     ///< Is allowed to update the list backref.
    friend class sw::FrameFormats<SwFrameFormat*>;     ///< Is allowed to update the list backref.
    friend class SwTextBoxHelper;
    friend class SwUndoFlyBase; ///< calls SetOtherTextBoxFormat

    css::uno::WeakReference<css::uno::XInterface> m_wXObject;

    // DrawingLayer FillAttributes in a preprocessed form for primitive usage
    drawinglayer::attribute::SdrAllFillAttributesHelperPtr  maFillAttributes;

    // The assigned SwFrmFmt list.
    SwFormatsBase* m_ffList;

    std::shared_ptr< SwTextBoxNode > m_pOtherTextBoxFormats;

    struct change_name
    {
        change_name(const OUString &rName) : mName(rName) {}
        void operator()(SwFormat *pFormat) { pFormat->m_aFormatName = mName; }
        const OUString &mName;
    };

protected:
    SwFrameFormat(
        SwAttrPool& rPool,
        const char* pFormatNm,
        SwFrameFormat *pDrvdFrame,
        sal_uInt16 nFormatWhich = RES_FRMFMT,
        const WhichRangesContainer& pWhichRange = aFrameFormatSetRange);

    SwFrameFormat(
        SwAttrPool& rPool,
        const OUString &rFormatNm,
        SwFrameFormat *pDrvdFrame,
        sal_uInt16 nFormatWhich = RES_FRMFMT,
        const WhichRangesContainer& pWhichRange = aFrameFormatSetRange);

    virtual void SwClientNotify(const SwModify&, const SfxHint&) override;

public:

    const std::shared_ptr< SwTextBoxNode >& GetOtherTextBoxFormats() const { return m_pOtherTextBoxFormats; };
    void SetOtherTextBoxFormats(const std::shared_ptr<SwTextBoxNode>& rNew) { m_pOtherTextBoxFormats = rNew; };

    virtual ~SwFrameFormat() override;

    SwFrameFormat(SwFrameFormat const &) = default;
    SwFrameFormat(SwFrameFormat &&) = default;
    SwFrameFormat & operator =(SwFrameFormat const &) = default;
    SwFrameFormat & operator =(SwFrameFormat &&) = default;

    /// Destroys all Frames in aDepend (Frames are identified via dynamic_cast).
    virtual void DelFrames();

    /// Creates the views.
    virtual void MakeFrames();

    virtual Graphic MakeGraphic( ImageMap* pMap = nullptr, const sal_uInt32 nMaximumQuadraticPixels = 500000, const std::optional<Size>& rTargetDPI = std::nullopt );

    /**  @return the IMapObject defined at format (Fly)
        in the ImageMap at position Point.
        rPoint - test on DocPosition.
        pFly - optional FlyFrame, in case it is already known. */
    IMapObject* GetIMapObject( const Point& rPoint,
                                const SwFlyFrame *pFly = nullptr ) const;

    /** @return the real size of the frame - or an empty rectangle
       if no layout exists.
       If pPoint is given, look for the frame closest to it. */
    SW_DLLPUBLIC SwRect FindLayoutRect( const bool bPrtArea = false,
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

    SW_DLLPUBLIC SdrObject *FindRealSdrObject();
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

    void RegisterToFormat( SwFormat& rFormat );

    // Access to DrawingLayer FillAttributes in a preprocessed form for primitive usage
    virtual drawinglayer::attribute::SdrAllFillAttributesHelperPtr getSdrAllFillAttributesHelper() const override;
    virtual bool supportsFullDrawingLayerFillAttributeSet() const override;

    void dumpAsXml(xmlTextWriterPtr pWriter) const;

    virtual void SetFormatName( const OUString& rNewName, bool bBroadcast=false ) override;
    void MoveTableBox(SwTableBox& rTableBox, const SwFrameFormat* pOldFormat);
    virtual bool IsVisible() const;
};

namespace sw
{
    class SW_DLLPUBLIC SpzFrameFormat: public SwFrameFormat {
        friend ::SwDrawFrameFormat;
        friend ::SwFlyFrameFormat;
        SpzFrameFormat(
            SwAttrPool& rPool,
            const OUString& rFormatName,
            SwFrameFormat* pDerivedFrame,
            sal_uInt16 nFormatWhich)
            : SwFrameFormat(rPool, rFormatName, pDerivedFrame, nFormatWhich)
        {
            assert(nFormatWhich == RES_DRAWFRMFMT || nFormatWhich == RES_FLYFRMFMT);
        };
    };
}
// The FlyFrame-Format

class SW_DLLPUBLIC SwFlyFrameFormat final : public sw::SpzFrameFormat
{
    friend class SwDoc;
    OUString msTitle;
    OUString msDesc;
    /// A tooltip has priority over an SwFormatURL and is not persisted to files.
    OUString msTooltip;

    /** Both not existent.
       it stores the previous position of Prt rectangle from RequestObjectResize
       so it can be used to move frames of non-resizable objects to align them correctly
       when they get borders (this is done in SwWrtShell::CalcAndGetScale) */
    Point   m_aLastFlyFramePrtRectPos;
    std::unique_ptr<SwFlyDrawContact> m_pContact;

    SwFlyFrameFormat( const SwFlyFrameFormat &rCpy ) = delete;
    SwFlyFrameFormat &operator=( const SwFlyFrameFormat &rCpy ) = delete;

    SwFlyFrameFormat( SwAttrPool& rPool, const OUString &rFormatNm, SwFrameFormat *pDrvdFrame );

public:
    virtual ~SwFlyFrameFormat() override;

    /// Creates the views.
    virtual void MakeFrames() override;

    SwFlyFrame* GetFrame( const Point* pDocPos = nullptr ) const;

    SwAnchoredObject* GetAnchoredObj() const;

    virtual Graphic MakeGraphic( ImageMap* pMap = nullptr, const sal_uInt32 nMaximumQuadraticPixels = 500000, const std::optional<Size>& rTargetDPI = std::nullopt ) override;

    OUString GetObjTitle() const;
    void SetObjTitle( const OUString& rTitle, bool bBroadcast = false );

    const OUString & GetObjTooltip() const;
    void SetObjTooltip(const OUString& rTooltip);

    OUString GetObjDescription() const;
    void SetObjDescription( const OUString& rDescription, bool bBroadcast = false );

    bool IsDecorative() const;
    void SetObjDecorative(bool isDecorative);

    /** SwFlyFrameFormat::IsBackgroundTransparent

        Override virtual method and its default implementation,
        because format of fly frame provides transparent backgrounds.
        Method determines, if background of fly frame is transparent.

        @return true, if background color is transparent, but not "no fill"
        or an existing background graphic is transparent.
    */
    virtual bool IsBackgroundTransparent() const override;

    /** SwFlyFrameFormat::IsBackgroundBrushInherited

        Method to determine, if the brush for drawing the
        background is "inherited" from its parent/grandparent.
        This is the case, if no background graphic is set and the background
        color is "no fill"/"auto fill"

        @return true, if background brush is "inherited" from parent/grandparent
    */
    bool IsBackgroundBrushInherited() const;

    const Point & GetLastFlyFramePrtRectPos() const       { return m_aLastFlyFramePrtRectPos; }
    void SetLastFlyFramePrtRectPos( const Point &rPoint ) { m_aLastFlyFramePrtRectPos = rPoint; }

    SwFlyDrawContact* GetOrCreateContact();
};

//The DrawFrame-Format


class SwDrawFrameFormat;
class SwDrawContact;
class SdrTextObj;

namespace sw
{
    enum class DrawFrameFormatHintId {
        DYING,
        PREPPASTING,
        PREP_INSERT_FLY,
        PREP_DELETE_FLY,
        PAGE_OUT_OF_BOUNDS,
        MAKE_FRAMES,
        DELETE_FRAMES,
        POST_RESTORE_FLY_ANCHOR,
    };
    struct DrawFrameFormatHint final: SfxHint
    {
        DrawFrameFormatHintId m_eId;
        DrawFrameFormatHint(DrawFrameFormatHintId eId)
            : SfxHint(SfxHintId::SwDrawFrameFormat), m_eId(eId) {};
        virtual ~DrawFrameFormatHint() override;
    };
    struct CheckDrawFrameFormatLayerHint final: SfxHint
    {
        bool* m_bCheckControlLayer;
        CheckDrawFrameFormatLayerHint(bool* bCheckControlLayer)
            : SfxHint(SfxHintId::SwCheckDrawFrameFormatLayer),
              m_bCheckControlLayer(bCheckControlLayer) {};
        virtual ~CheckDrawFrameFormatLayerHint() override;
    };
    struct ContactChangedHint final: SfxHint
    {
        SdrObject** m_ppObject;
        ContactChangedHint(SdrObject** ppObject)
            : SfxHint(SfxHintId::SwContactChanged),
              m_ppObject(ppObject) {};
        virtual ~ContactChangedHint() override;
    };
    struct DrawFormatLayoutCopyHint final : SfxHint
    {
        SwDrawFrameFormat& m_rDestFormat;
        SwDoc& m_rDestDoc;
        DrawFormatLayoutCopyHint(SwDrawFrameFormat& rDestFormat, SwDoc& rDestDoc)
            : SfxHint(SfxHintId::SwDrawFormatLayoutCopy),
              m_rDestFormat(rDestFormat), m_rDestDoc(rDestDoc) {};
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
        WW8AnchorConvHint(WW8AnchorConvResult& rResult)
            : SfxHint(SfxHintId::SwWW8AnchorConv),
              m_rResult(rResult) {};
        virtual ~WW8AnchorConvHint() override;
    };
    struct RestoreFlyAnchorHint final : SfxHint
    {
        const Point m_aPos;
        RestoreFlyAnchorHint(Point aPos)
            : SfxHint(SfxHintId::SwRestoreFlyAnchor),
              m_aPos(aPos) {};
        virtual ~RestoreFlyAnchorHint() override;
    };
    struct CreatePortionHint final : SfxHint
    {
        SwDrawContact** m_ppContact;
        CreatePortionHint(SwDrawContact** ppContact)
            : SfxHint(SfxHintId::SwCreatePortion),
              m_ppContact(ppContact) {};
        virtual ~CreatePortionHint() override;
    };
    struct CollectTextObjectsHint final : SfxHint
    {
        std::list<SdrTextObj*>& m_rTextObjects;
        CollectTextObjectsHint(std::list<SdrTextObj*>& rTextObjects)
            : SfxHint(SfxHintId::SwCollectTextObjects),
              m_rTextObjects(rTextObjects) {};
        virtual ~CollectTextObjectsHint() override;
    };
    struct GetZOrderHint final : SfxHint
    {
        sal_uInt32& m_rnZOrder;
        GetZOrderHint(sal_uInt32& rnZOrder)
            : SfxHint(SfxHintId::SwGetZOrder),
              m_rnZOrder(rnZOrder) {};
        virtual ~GetZOrderHint() override;
    };
    struct GetObjectConnectedHint final : SfxHint
    {
        bool& m_risConnected;
        const SwRootFrame* m_pRoot;
        GetObjectConnectedHint(bool& risConnected, const SwRootFrame* pRoot)
            : SfxHint(SfxHintId::SwGetObjectConnected),
              m_risConnected(risConnected), m_pRoot(pRoot) {};
        virtual ~GetObjectConnectedHint() override;
    };
}

class SW_DLLPUBLIC SwDrawFrameFormat final : public sw::SpzFrameFormat
{
    friend class SwDoc;

    mutable const SdrObject * m_pSdrObjectCached;
    mutable OUString m_sSdrObjectCachedComment;

    SwDrawFrameFormat( const SwDrawFrameFormat &rCpy ) = delete;
    SwDrawFrameFormat &operator=( const SwDrawFrameFormat &rCpy ) = delete;

    SwFrameFormat::tLayoutDir meLayoutDir;

    sal_Int16 mnPositionLayoutDir;

    bool mbPosAttrSet;

    SwDrawFrameFormat(SwAttrPool& rPool, const OUString& rFormatName, SwFrameFormat* pDerivedFrame)
        : sw::SpzFrameFormat(rPool, rFormatName, pDerivedFrame, RES_DRAWFRMFMT),
          m_pSdrObjectCached(nullptr),
          meLayoutDir(SwFrameFormat::HORI_L2R),
          mnPositionLayoutDir(css::text::PositionLayoutDir::PositionInLayoutDirOfAnchor),
          mbPosAttrSet(false)
    {}

public:
    virtual ~SwDrawFrameFormat() override;

    /** DrawObjects are removed from the arrays at the layout.
     The DrawObjects are marked as deleted. */
    virtual void DelFrames() override;

    /** Register DrawObjects in the arrays at layout.
     Reset delete marks. */
    virtual void MakeFrames() override;

    virtual Graphic MakeGraphic( ImageMap* pMap = nullptr, const sal_uInt32 nMaximumQuadraticPixels = 500000, const std::optional<Size>& rTargetDPI = std::nullopt ) override;

    virtual SwFrameFormat::tLayoutDir GetLayoutDir() const override;
    virtual void SetLayoutDir( const SwFrameFormat::tLayoutDir _eLayoutDir ) override;

    virtual sal_Int16 GetPositionLayoutDir() const override;
    virtual void SetPositionLayoutDir( const sal_Int16 _nPositionLayoutDir ) override;

    bool IsPosAttrSet() const { return mbPosAttrSet; }
    void PosAttrSet() { mbPosAttrSet = true; }

    virtual OUString GetDescription() const override;
};

namespace sw {

SW_DLLPUBLIC bool IsFlyFrameFormatInHeader(const SwFrameFormat& rFormat);

void CheckAnchoredFlyConsistency(SwDoc const& rDoc);

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
