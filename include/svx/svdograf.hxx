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
#include <vcl/graph.hxx>
#include <svx/svdorect.hxx>
#include <svtools/grfmgr.hxx>
#include <svx/svxdllapi.h>
#include <o3tl/typed_flags_set.hxx>

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
    COLOR      = 0x01,
    MIRROR     = 0x02,
    ROTATE     = 0x04,
    ALL        = 0x07,
};
namespace o3tl
{
    template<> struct typed_flags<SdrGrafObjTransformsAttrs> : is_typed_flags<SdrGrafObjTransformsAttrs, 7> {};
}

class SdrGrafObjGeoData : public SdrTextObjGeoData
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
class SVX_DLLPUBLIC SdrGrafObj : public SdrRectObj
{
private:
    // to allow sdr::properties::GraphicProperties access to SetXPolyDirty()
    friend class sdr::properties::GraphicProperties;

    // to allow sdr::contact::ViewObjectContactOfGraphic access to ImpUpdateGraphicLink()
    friend class sdr::contact::ViewObjectContactOfGraphic;
    friend class SdrExchangeView; // Only for a ForceSwapIn() call.
    friend class SdrGraphicLink;

private:
    virtual sdr::contact::ViewContact* CreateObjectSpecificViewContact() override;
    virtual sdr::properties::BaseProperties* CreateObjectSpecificProperties() override;

    void ImpSetAttrToGrafInfo(); // Copy values from the pool
    GraphicAttr aGrafInfo;

    OUString aFileName; // If it's a Link, the filename can be found in here
    OUString aReferer;
    OUString aFilterName;
    GraphicObject* pGraphic; // In order to speed up output of bitmaps, especially rotated ones
    GraphicObject* mpReplacementGraphic;
    SdrGraphicLink* pGraphicLink; // And here a pointer for linked graphics
    bool bMirrored:1; // True: the graphic is horizontal, which means it's mirrored along the y-axis

    // Flag for allowing text animation. Default is true.
    bool mbGrafAnimationAllowed:1;

    // #i25616#
    bool mbInsidePaint:1;
    bool mbIsPreview:1;

private:

    void                    ImpLinkAnmeldung();
    void                    ImpLinkAbmeldung();
    bool                    ImpUpdateGraphicLink( bool bAsynchron = true ) const;
    void                    ImpSetLinkedGraphic( const Graphic& rGraphic );
                            DECL_LINK_TYPED( ImpSwapHdl, const GraphicObject*, SvStream* );
    void onGraphicChanged();

public:
                            TYPEINFO_OVERRIDE();

                            SdrGrafObj();
                            SdrGrafObj(const Graphic& rGrf);
                            SdrGrafObj(const Graphic& rGrf, const Rectangle& rRect);
    virtual                 ~SdrGrafObj();

    void                    SetGraphicObject( const GraphicObject& rGrfObj );
    const GraphicObject&    GetGraphicObject(bool bForceSwapIn = false) const;
    const GraphicObject*    GetReplacementGraphicObject() const;

    void                    NbcSetGraphic(const Graphic& rGrf);
    void                    SetGraphic(const Graphic& rGrf);
    const Graphic&          GetGraphic() const;

    Graphic                 GetTransformedGraphic( SdrGrafObjTransformsAttrs nTransformFlags = SdrGrafObjTransformsAttrs::ALL ) const;

    GraphicType             GetGraphicType() const;

    // Keep ATM for SD.
    bool IsAnimated() const;
    bool IsEPS() const;
    bool IsSwappedOut() const;

    const MapMode&          GetGrafPrefMapMode() const;
    const Size&             GetGrafPrefSize() const;

    void                    SetGrafStreamURL( const OUString& rGraphicStreamURL );
    OUString                GetGrafStreamURL() const;

private:
    void                    ForceSwapIn() const;

public:
    void                    SetGraphicLink(const OUString& rFileName, const OUString& rReferer, const OUString& rFilterName);
    void                    ReleaseGraphicLink();
    bool IsLinkedGraphic() const;

    const OUString& GetFileName() const { return aFileName;}
    const OUString& GetFilterName() const { return aFilterName;}

    void                    StartAnimation(OutputDevice* pOutDev, const Point& rPoint, const Size& rSize, long nExtraData=0L);

    virtual void            TakeObjInfo(SdrObjTransformInfoRec& rInfo) const override;
    virtual sal_uInt16          GetObjIdentifier() const override;

    virtual OUString        TakeObjNameSingul() const override;
    virtual OUString        TakeObjNamePlural() const override;

    // #i25616#
    virtual basegfx::B2DPolyPolygon TakeXorPoly() const override;

    virtual SdrGrafObj* Clone() const override;
    SdrGrafObj&             operator=(const SdrGrafObj& rObj);

    virtual sal_uInt32 GetHdlCount() const override;
    virtual SdrHdl*         GetHdl(sal_uInt32 nHdlNum) const override;

    virtual void            NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact) override;
    virtual void            NbcRotate(const Point& rRef, long nAngle, double sn, double cs) override;
    virtual void            NbcMirror(const Point& rRef1, const Point& rRef2) override;
    virtual void            NbcShear (const Point& rRef, long nAngle, double tn, bool bVShear) override;
    virtual void            NbcSetSnapRect(const Rectangle& rRect) override;
    virtual void            NbcSetLogicRect(const Rectangle& rRect) override;
    virtual SdrObjGeoData*  NewGeoData() const override;
    virtual void            SaveGeoData(SdrObjGeoData& rGeo) const override;
    virtual void            RestGeoData(const SdrObjGeoData& rGeo) override;

    bool                    HasGDIMetaFile() const;

    virtual void            SetPage(SdrPage* pNewPage) override;
    virtual void            SetModel(SdrModel* pNewModel) override;

    bool isEmbeddedSvg() const;
    GDIMetaFile getMetafileFromEmbeddedSvg() const;

    virtual SdrObject*      DoConvertToPolyObj(bool bBezier, bool bAddText) const override;

    virtual void            AdjustToMaxRect( const Rectangle& rMaxRect, bool bShrinkOnly = false ) override;

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    bool IsMirrored() const { return bMirrored;}
    void SetMirrored( bool _bMirrored );

    // Access to GrafAnimationAllowed flag
    void SetGrafAnimationAllowed(bool bNew);

    css::uno::Reference< css::io::XInputStream > getInputStream();

    // #i103116# FullDrag support
    virtual SdrObject* getFullDragClone() const override;

    // add handles for crop mode when selected
    virtual void addCropHandles(SdrHdlList& rTarget) const override;
};

#endif // INCLUDED_SVX_SVDOGRAF_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
