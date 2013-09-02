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

#ifndef _SVDOGRAF_HXX
#define _SVDOGRAF_HXX

#include <com/sun/star/io/XInputStream.hpp>
#include <vcl/graph.hxx>
#include <svx/svdorect.hxx>
#include <svtools/grfmgr.hxx>
#include "svx/svxdllapi.h"

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

#define SDRGRAFOBJ_TRANSFORMATTR_NONE       0x00000000UL
#define SDRGRAFOBJ_TRANSFORMATTR_COLOR      0x00000001UL
#define SDRGRAFOBJ_TRANSFORMATTR_MIRROR     0x00000002UL
#define SDRGRAFOBJ_TRANSFORMATTR_ROTATE     0x00000004UL
#define SDRGRAFOBJ_TRANSFORMATTR_ALL        0xffffffffUL

class SdrGrafObjGeoData : public SdrTextObjGeoData
{
public:
    sal_Bool                    bMirrored;

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
    friend class SdrGraphicLink;

private:
    virtual sdr::contact::ViewContact* CreateObjectSpecificViewContact();
    virtual sdr::properties::BaseProperties* CreateObjectSpecificProperties();

    void                    ImpSetAttrToGrafInfo(); // Werte vom Pool kopieren
    GraphicAttr             aGrafInfo;

    OUString aFileName;          // Wenn es sich um einen Link handelt, steht hier der Dateiname drin.
    OUString aFilterName;
    GraphicObject*          pGraphic;           // Zur Beschleunigung von Bitmapausgaben, besonders von gedrehten.
    GraphicObject*          mpReplacementGraphic;
    SdrGraphicLink*         pGraphicLink;       // Und hier noch ein Pointer fuer gelinkte Grafiken
    bool                    bMirrored:1;        // True bedeutet, die Grafik ist horizontal, d.h. ueber die Y-Achse gespiegelt auszugeben.

    // #111096#
    // Flag for allowing text animation. Default is sal_true.
    bool mbGrafAnimationAllowed:1;

    // #i25616#
    bool mbInsidePaint:1;
    bool mbIsPreview:1;

private:

    void                    ImpLinkAnmeldung();
    void                    ImpLinkAbmeldung();
    bool                    ImpUpdateGraphicLink( bool bAsynchron = true ) const;
    void                    ImpSetLinkedGraphic( const Graphic& rGraphic );
                            DECL_LINK( ImpSwapHdl, GraphicObject* );
    void onGraphicChanged();

public:
                            TYPEINFO();

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

    Graphic                 GetTransformedGraphic( sal_uIntPtr nTransformFlags = SDRGRAFOBJ_TRANSFORMATTR_ALL ) const;

    GraphicType             GetGraphicType() const;

    // #111096#
    // Keep ATM for SD.
    bool IsAnimated() const;
    bool IsEPS() const;
    bool IsSwappedOut() const;

    const MapMode&          GetGrafPrefMapMode() const;
    const Size&             GetGrafPrefSize() const;

    void                    SetGrafStreamURL( const OUString& rGraphicStreamURL );
    OUString                GetGrafStreamURL() const;

    void                    ForceSwapIn() const;
    void                    ForceSwapOut() const;

    void                    SetGraphicLink(const OUString& rFileName, const OUString& rFilterName);
    void                    ReleaseGraphicLink();
    bool IsLinkedGraphic() const;

    const OUString& GetFileName() const;
    const OUString& GetFilterName() const;

    void                    StartAnimation(OutputDevice* pOutDev, const Point& rPoint, const Size& rSize, long nExtraData=0L);

    virtual void            TakeObjInfo(SdrObjTransformInfoRec& rInfo) const;
    virtual sal_uInt16          GetObjIdentifier() const;

    virtual OUString        TakeObjNameSingul() const;
    virtual OUString        TakeObjNamePlural() const;

    // #i25616#
    virtual basegfx::B2DPolyPolygon TakeXorPoly() const;

    virtual SdrGrafObj* Clone() const;
    SdrGrafObj&             operator=(const SdrGrafObj& rObj);

    virtual sal_uInt32 GetHdlCount() const;
    virtual SdrHdl*         GetHdl(sal_uInt32 nHdlNum) const;

    virtual void            NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact);
    virtual void            NbcRotate(const Point& rRef, long nWink, double sn, double cs);
    virtual void            NbcMirror(const Point& rRef1, const Point& rRef2);
    virtual void            NbcShear (const Point& rRef, long nWink, double tn, bool bVShear);
    virtual void            NbcSetSnapRect(const Rectangle& rRect);
    virtual void            NbcSetLogicRect(const Rectangle& rRect);
    virtual SdrObjGeoData*  NewGeoData() const;
    virtual void            SaveGeoData(SdrObjGeoData& rGeo) const;
    virtual void            RestGeoData(const SdrObjGeoData& rGeo);

    bool                    HasGDIMetaFile() const;

    virtual void            SetPage(SdrPage* pNewPage);
    virtual void            SetModel(SdrModel* pNewModel);

    bool isEmbeddedSvg() const;
    GDIMetaFile getMetafileFromEmbeddedSvg() const;

    virtual SdrObject*      DoConvertToPolyObj(sal_Bool bBezier, bool bAddText) const;

    virtual void            AdjustToMaxRect( const Rectangle& rMaxRect, bool bShrinkOnly = false );

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    bool IsMirrored() const;
    void SetMirrored( bool _bMirrored );

    // #111096#
    // Access to GrafAnimationAllowed flag
    void SetGrafAnimationAllowed(bool bNew);

    ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > getInputStream();

    // #i103116# FullDrag support
    virtual SdrObject* getFullDragClone() const;
};

#endif //_SVDOGRAF_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
