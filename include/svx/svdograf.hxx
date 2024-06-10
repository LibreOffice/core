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

#pragma once

#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/drawing/BarCode.hpp>
#include <vcl/graph.hxx>
#include <svx/svdorect.hxx>
#include <vcl/GraphicObject.hxx>
#include <svx/svxdllapi.h>
#include <o3tl/typed_flags_set.hxx>
#include <memory>

namespace sdr::properties { class GraphicProperties; }
namespace sdr::contact { class ViewObjectContactOfGraphic; }

/**
 * Options for GetTransformedGraphic()
 */
enum class SdrGrafObjTransformsAttrs
{
    NONE       = 0x00,
    MIRROR     = 0x01,
    ROTATE     = 0x02,
    ALL        = 0x03,
};
namespace o3tl
{
    template<> struct typed_flags<SdrGrafObjTransformsAttrs> : is_typed_flags<SdrGrafObjTransformsAttrs, 0x03> {};
}

class SdrGrafObjGeoData final : public SdrTextObjGeoData
{
public:
    bool                    bMirrored;

    SdrGrafObjGeoData()
    :   bMirrored(false)
    {
    }
};

class GraphicObject;
class SdrGraphicLink;

/**
 * This class represents an embedded or linked bitmap graphic object.
 */
class SVXCORE_DLLPUBLIC SdrGrafObj final : public SdrRectObj
{
private:
    // to allow sdr::properties::GraphicProperties access to SetXPolyDirty()
    friend class sdr::properties::GraphicProperties;

    // to allow sdr::contact::ViewObjectContactOfGraphic access to ImpUpdateGraphicLink()
    friend class sdr::contact::ViewObjectContactOfGraphic;
    friend class SdrExchangeView; // Only for a ForceSwapIn() call.
    friend class SdrGraphicLink;

    SAL_DLLPRIVATE virtual std::unique_ptr<sdr::contact::ViewContact> CreateObjectSpecificViewContact() override;
    SAL_DLLPRIVATE virtual std::unique_ptr<sdr::properties::BaseProperties> CreateObjectSpecificProperties() override;

    SAL_DLLPRIVATE void ImpSetAttrToGrafInfo(); // Copy values from the pool
    GraphicAttr m_aGrafInfo;

    OUString m_aFileName; // If it's a Link, the filename can be found in here
    OUString m_aFilterName;
    std::unique_ptr<GraphicObject> mpGraphicObject; // In order to speed up output of bitmaps, especially rotated ones
    std::unique_ptr<GraphicObject> mpReplacementGraphicObject;
    SdrGraphicLink* m_pGraphicLink; // And here a pointer for linked graphics
    bool m_bMirrored:1; // True: the graphic is horizontal, which means it's mirrored along the y-axis

    // Flag for allowing text animation. Default is true.
    bool mbGrafAnimationAllowed:1;

    bool mbIsSignatureLine;
    OUString maSignatureLineId;
    OUString maSignatureLineSuggestedSignerName;
    OUString maSignatureLineSuggestedSignerTitle;
    OUString maSignatureLineSuggestedSignerEmail;
    OUString maSignatureLineSigningInstructions;
    bool mbIsSignatureLineShowSignDate;
    bool mbIsSignatureLineCanAddComment;
    bool mbSignatureLineIsSigned;
    css::uno::Reference<css::graphic::XGraphic> mpSignatureLineUnsignedGraphic;

    std::unique_ptr<css::drawing::BarCode> mpBarCode;
    SAL_DLLPRIVATE void                    ImpRegisterLink();
    SAL_DLLPRIVATE void                    ImpDeregisterLink();
    SAL_DLLPRIVATE void                    ImpSetLinkedGraphic( const Graphic& rGraphic );
    SAL_DLLPRIVATE void onGraphicChanged();
    SAL_DLLPRIVATE GDIMetaFile             GetMetaFile(GraphicType &rGraphicType) const;

    // protected destructor
    SAL_DLLPRIVATE virtual ~SdrGrafObj() override;

public:
    SdrGrafObj(SdrModel& rSdrModel);
    // Copy constructor
    SAL_DLLPRIVATE SdrGrafObj(SdrModel& rSdrModel, SdrGrafObj const & rSource);
    SdrGrafObj(
        SdrModel& rSdrModel,
        const Graphic& rGrf);
    SdrGrafObj(
        SdrModel& rSdrModel,
        const Graphic& rGrf,
        const tools::Rectangle& rRect);

    void                    SetGraphicObject( const GraphicObject& rGrfObj );
    const GraphicObject&    GetGraphicObject(bool bForceSwapIn = false) const;
    SAL_DLLPRIVATE const GraphicObject*    GetReplacementGraphicObject() const;

    SAL_DLLPRIVATE void                    NbcSetGraphic(const Graphic& rGrf);
    void                    SetGraphic(const Graphic& rGrf);
    const Graphic&          GetGraphic() const;

    Graphic                 GetTransformedGraphic( SdrGrafObjTransformsAttrs nTransformFlags = SdrGrafObjTransformsAttrs::ALL ) const;

    GraphicType             GetGraphicType() const;

    GraphicAttr             GetGraphicAttr( SdrGrafObjTransformsAttrs nTransformFlags = SdrGrafObjTransformsAttrs::ALL  ) const;

    // Keep ATM for SD.
    bool IsAnimated() const;
    bool IsEPS() const;

    SAL_DLLPRIVATE MapMode          GetGrafPrefMapMode() const;
    SAL_DLLPRIVATE Size             GetGrafPrefSize() const;

    SAL_DLLPRIVATE void                    SetGrafStreamURL( const OUString& rGraphicStreamURL );
    SAL_DLLPRIVATE OUString const &        GetGrafStreamURL() const;
    Size                    getOriginalSize() const;

private:
    SAL_DLLPRIVATE void                    ForceSwapIn() const;

public:
    void                    SetGraphicLink(const OUString& rFileName);
    void                    ReleaseGraphicLink();
    bool IsLinkedGraphic() const;

    const OUString& GetFileName() const { return m_aFileName;}

    void                    StartAnimation();

    SAL_DLLPRIVATE virtual void            TakeObjInfo(SdrObjTransformInfoRec& rInfo) const override;
    SAL_DLLPRIVATE virtual SdrObjKind      GetObjIdentifier() const override;

    SAL_DLLPRIVATE virtual OUString        TakeObjNameSingul() const override;
    SAL_DLLPRIVATE virtual OUString        TakeObjNamePlural() const override;

    virtual rtl::Reference<SdrObject> CloneSdrObject(SdrModel& rTargetModel) const override;

    SAL_DLLPRIVATE virtual sal_uInt32 GetHdlCount() const override;
    SAL_DLLPRIVATE virtual void AddToHdlList(SdrHdlList& rHdlList) const override;

    virtual void            NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact) override;
    SAL_DLLPRIVATE virtual void            NbcMirror(const Point& rRef1, const Point& rRef2) override;
    SAL_DLLPRIVATE virtual std::unique_ptr<SdrObjGeoData>  NewGeoData() const override;
    SAL_DLLPRIVATE virtual void            SaveGeoData(SdrObjGeoData& rGeo) const override;
    SAL_DLLPRIVATE virtual void            RestoreGeoData(const SdrObjGeoData& rGeo) override;

    bool                    HasGDIMetaFile() const;

    // react on model/page change
    SAL_DLLPRIVATE virtual void handlePageChange(SdrPage* pOldPage, SdrPage* pNewPage) override;

    bool isEmbeddedVectorGraphicData() const;
    GDIMetaFile getMetafileFromEmbeddedVectorGraphicData() const;

    SAL_DLLPRIVATE virtual rtl::Reference<SdrObject> DoConvertToPolyObj(bool bBezier, bool bAddText) const override;

    virtual void            AdjustToMaxRect( const tools::Rectangle& rMaxRect, bool bShrinkOnly = false ) override;

   SAL_DLLPRIVATE  virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    bool IsMirrored() const { return m_bMirrored;}
    SAL_DLLPRIVATE void SetMirrored( bool _bMirrored );

    virtual bool shouldKeepAspectRatio() const override { return true; }

    // Access to GrafAnimationAllowed flag
    SAL_DLLPRIVATE void SetGrafAnimationAllowed(bool bNew);

    SAL_DLLPRIVATE css::uno::Reference< css::io::XInputStream > getInputStream() const;

    // #i103116# FullDrag support
    SAL_DLLPRIVATE virtual rtl::Reference<SdrObject> getFullDragClone() const override;

    // add handles for crop mode when selected
    SAL_DLLPRIVATE virtual void addCropHandles(SdrHdlList& rTarget) const override;

    // Signature Line
    void setIsSignatureLine(bool bIsSignatureLine) { mbIsSignatureLine = bIsSignatureLine; }
    bool isSignatureLine() const { return mbIsSignatureLine; }

    void setSignatureLineId(const OUString& rSignatureLineId)
    {
        maSignatureLineId = rSignatureLineId;
    }

    const OUString& getSignatureLineId() const { return maSignatureLineId; }

    void setSignatureLineSuggestedSignerName(const OUString& rSuggestedSignerName)
    {
        maSignatureLineSuggestedSignerName = rSuggestedSignerName;
    }

    const OUString& getSignatureLineSuggestedSignerName() const
    {
        return maSignatureLineSuggestedSignerName;
    }

    void setSignatureLineSuggestedSignerTitle(const OUString& rSuggestedSignerTitle)
    {
        maSignatureLineSuggestedSignerTitle = rSuggestedSignerTitle;
    }

    const OUString& getSignatureLineSuggestedSignerTitle() const
    {
        return maSignatureLineSuggestedSignerTitle;
    }

    void setSignatureLineSuggestedSignerEmail(const OUString& rSuggestedSignerEmail)
    {
        maSignatureLineSuggestedSignerEmail = rSuggestedSignerEmail;
    }
    const OUString& getSignatureLineSuggestedSignerEmail() const
    {
        return maSignatureLineSuggestedSignerEmail;
    }

    void setSignatureLineSigningInstructions(const OUString& rSigningInstructions)
    {
        maSignatureLineSigningInstructions = rSigningInstructions;
    }

    const OUString& getSignatureLineSigningInstructions() const
    {
        return maSignatureLineSigningInstructions;
    }

    void setSignatureLineShowSignDate(bool bIsSignatureLineShowSignDate)
    {
        mbIsSignatureLineShowSignDate = bIsSignatureLineShowSignDate;
    }

    bool isSignatureLineShowSignDate() const { return mbIsSignatureLineShowSignDate; }

    void setSignatureLineCanAddComment(bool bIsSignatureCanAddComment)
    {
        mbIsSignatureLineCanAddComment = bIsSignatureCanAddComment;
    }

    bool isSignatureLineCanAddComment() const { return mbIsSignatureLineCanAddComment; }

    css::uno::Reference<css::graphic::XGraphic> const & getSignatureLineUnsignedGraphic() const
    {
        return mpSignatureLineUnsignedGraphic;
    }

    void setSignatureLineUnsignedGraphic(const css::uno::Reference<css::graphic::XGraphic>& rGraphic)
    {
        mpSignatureLineUnsignedGraphic = rGraphic;
    }

    bool isSignatureLineSigned() const { return mbSignatureLineIsSigned; }

    void setSignatureLineIsSigned(bool bIsSigned) { mbSignatureLineIsSigned = bIsSigned; }

    // Qr Code
    void setQrCode(css::drawing::BarCode& rBarCode)
    {
        mpBarCode = std::make_unique<css::drawing::BarCode>(rBarCode);
    }

    css::drawing::BarCode* getQrCode() const
    {
        return mpBarCode.get();
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
