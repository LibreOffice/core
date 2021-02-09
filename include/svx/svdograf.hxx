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

#ifndef INCLUDED_SVX_SVDOGRAF_HXX
#define INCLUDED_SVX_SVDOGRAF_HXX

#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/drawing/QRCode.hpp>
#include <vcl/graph.hxx>
#include <svx/svdorect.hxx>
#include <vcl/GraphicObject.hxx>
#include <svx/svxdllapi.h>
#include <o3tl/typed_flags_set.hxx>
#include <memory>

namespace sdr
{
    namespace properties
    {
        class GraphicProperties;
    } // end of namespace properties
    namespace contact
    {
        class ViewObjectContactOfGraphic;
    } // end of namespace contact
} // end of namespace sdr

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
class SVXCORE_DLLPUBLIC SdrGrafObj : public SdrRectObj
{
private:
    // to allow sdr::properties::GraphicProperties access to SetXPolyDirty()
    friend class sdr::properties::GraphicProperties;

    // to allow sdr::contact::ViewObjectContactOfGraphic access to ImpUpdateGraphicLink()
    friend class sdr::contact::ViewObjectContactOfGraphic;
    friend class SdrExchangeView; // Only for a ForceSwapIn() call.
    friend class SdrGraphicLink;

    virtual std::unique_ptr<sdr::contact::ViewContact> CreateObjectSpecificViewContact() override;
    virtual std::unique_ptr<sdr::properties::BaseProperties> CreateObjectSpecificProperties() override;

    void ImpSetAttrToGrafInfo(); // Copy values from the pool
    GraphicAttr aGrafInfo;

    OUString aFileName; // If it's a Link, the filename can be found in here
    OUString aFilterName;
    std::unique_ptr<GraphicObject> mpGraphicObject; // In order to speed up output of bitmaps, especially rotated ones
    std::unique_ptr<GraphicObject> mpReplacementGraphicObject;
    SdrGraphicLink* pGraphicLink; // And here a pointer for linked graphics
    bool bMirrored:1; // True: the graphic is horizontal, which means it's mirrored along the y-axis

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

    std::unique_ptr<css::drawing::QRCode> mpQrCode;
    void                    ImpRegisterLink();
    void                    ImpDeregisterLink();
    void                    ImpSetLinkedGraphic( const Graphic& rGraphic );
    void onGraphicChanged();
    GDIMetaFile             GetMetaFile(GraphicType &rGraphicType) const;

protected:
    // protected destructor
    virtual ~SdrGrafObj() override;

public:
    SdrGrafObj(SdrModel& rSdrModel);
    // Copy constructor
    SdrGrafObj(SdrModel& rSdrModel, SdrGrafObj const & rSource);
    SdrGrafObj(
        SdrModel& rSdrModel,
        const Graphic& rGrf);
    SdrGrafObj(
        SdrModel& rSdrModel,
        const Graphic& rGrf,
        const tools::Rectangle& rRect);

    void                    SetGraphicObject( const GraphicObject& rGrfObj );
    const GraphicObject&    GetGraphicObject(bool bForceSwapIn = false) const;
    const GraphicObject*    GetReplacementGraphicObject() const;

    void                    NbcSetGraphic(const Graphic& rGrf);
    void                    SetGraphic(const Graphic& rGrf);
    const Graphic&          GetGraphic() const;

    Graphic                 GetTransformedGraphic( SdrGrafObjTransformsAttrs nTransformFlags = SdrGrafObjTransformsAttrs::ALL ) const;

    GraphicType             GetGraphicType() const;

    GraphicAttr             GetGraphicAttr( SdrGrafObjTransformsAttrs nTransformFlags = SdrGrafObjTransformsAttrs::ALL  ) const;

    // Keep ATM for SD.
    bool IsAnimated() const;
    bool IsEPS() const;

    MapMode          GetGrafPrefMapMode() const;
    Size             GetGrafPrefSize() const;

    void                    SetGrafStreamURL( const OUString& rGraphicStreamURL );
    OUString const &        GetGrafStreamURL() const;
    Size                    getOriginalSize() const;

private:
    void                    ForceSwapIn() const;

public:
    void                    SetGraphicLink(const OUString& rFileName);
    void                    ReleaseGraphicLink();
    bool IsLinkedGraphic() const;

    const OUString& GetFileName() const { return aFileName;}

    void                    StartAnimation();

    virtual void            TakeObjInfo(SdrObjTransformInfoRec& rInfo) const override;
    virtual SdrObjKind      GetObjIdentifier() const override;

    virtual OUString        TakeObjNameSingul() const override;
    virtual OUString        TakeObjNamePlural() const override;

    virtual SdrGrafObj* CloneSdrObject(SdrModel& rTargetModel) const override;

    virtual sal_uInt32 GetHdlCount() const override;
    virtual void AddToHdlList(SdrHdlList& rHdlList) const override;

    virtual void            NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact) override;
    virtual void            NbcMirror(const Point& rRef1, const Point& rRef2) override;
    virtual std::unique_ptr<SdrObjGeoData>  NewGeoData() const override;
    virtual void            SaveGeoData(SdrObjGeoData& rGeo) const override;
    virtual void            RestGeoData(const SdrObjGeoData& rGeo) override;

    bool                    HasGDIMetaFile() const;

    // react on model/page change
    virtual void handlePageChange(SdrPage* pOldPage, SdrPage* pNewPage) override;

    bool isEmbeddedVectorGraphicData() const;
    GDIMetaFile getMetafileFromEmbeddedVectorGraphicData() const;

    virtual SdrObjectUniquePtr DoConvertToPolyObj(bool bBezier, bool bAddText) const override;

    virtual void            AdjustToMaxRect( const tools::Rectangle& rMaxRect, bool bShrinkOnly = false ) override;

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    bool IsMirrored() const { return bMirrored;}
    void SetMirrored( bool _bMirrored );

    virtual bool shouldKeepAspectRatio() const override { return true; }

    // Access to GrafAnimationAllowed flag
    void SetGrafAnimationAllowed(bool bNew);

    css::uno::Reference< css::io::XInputStream > getInputStream() const;

    // #i103116# FullDrag support
    virtual SdrObjectUniquePtr getFullDragClone() const override;

    // add handles for crop mode when selected
    virtual void addCropHandles(SdrHdlList& rTarget) const override;

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

    void setSignatureLineUnsignedGraphic(css::uno::Reference<css::graphic::XGraphic> rGraphic)
    {
        mpSignatureLineUnsignedGraphic = rGraphic;
    }

    bool isSignatureLineSigned() const { return mbSignatureLineIsSigned; }

    void setSignatureLineIsSigned(bool bIsSigned) { mbSignatureLineIsSigned = bIsSigned; }

    // Qr Code
    void setQrCode(css::drawing::QRCode& rQrCode)
    {
        mpQrCode = std::make_unique<css::drawing::QRCode>(rQrCode);
    }

    css::drawing::QRCode* getQrCode() const
    {
        return mpQrCode.get();
    }
};

#endif // INCLUDED_SVX_SVDOGRAF_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
