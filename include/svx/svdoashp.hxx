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

#ifndef INCLUDED_SVX_SVDOASHP_HXX
#define INCLUDED_SVX_SVDOASHP_HXX

#include <svx/svdobj.hxx>
#include <svx/svdotext.hxx>
#include <svx/svdhdl.hxx>
#include <vector>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/drawing/XCustomShapeEngine.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeAdjustmentValue.hpp>
#include <svx/svxdllapi.h>
#include <o3tl/typed_flags_set.hxx>


//   Initial Declarations


class SdrObjList;
class SdrObjListIter;
class SfxItemSet;

namespace sdr
{
    namespace properties
    {
        class CustomShapeProperties;
    } // end of namespace properties
} // end of namespace sdr

class SdrAShapeObjGeoData : public SdrTextObjGeoData
{
    public:

    bool        bMirroredX;
    bool        bMirroredY;
    double      fObjectRotation;

    css::uno::Sequence< css::drawing::EnhancedCustomShapeAdjustmentValue >
                aAdjustmentSeq;
};

enum class CustomShapeHandleModes
{
    NONE                 = 0,
    RESIZE_FIXED         = 1,
    CREATE_FIXED         = 2,
    RESIZE_ABSOLUTE_X    = 4,
    RESIZE_ABSOLUTE_Y    = 8,
    MOVE_SHAPE           = 16,
    ORTHO4               = 32,
};
namespace o3tl
{
    template<> struct typed_flags<CustomShapeHandleModes> : is_typed_flags<CustomShapeHandleModes, 63> {};
}

struct SdrCustomShapeInteraction
{
    css::uno::Reference< css::drawing::XCustomShapeHandle >   xInteraction;
    css::awt::Point                                                      aPosition;
    CustomShapeHandleModes                                                          nMode;
};


//   SdrObjCustomShape


class SVX_DLLPUBLIC SdrObjCustomShape : public SdrTextObj
{
private:
    // fObjectRotation is containing the object rotation in degrees.
    double fObjectRotation;

protected:
    virtual sdr::contact::ViewContact* CreateObjectSpecificViewContact() override;
    virtual void impl_setUnoShape(const css::uno::Reference<css::uno::XInterface>& rxUnoShape) override;

public:
    virtual sdr::properties::BaseProperties* CreateObjectSpecificProperties() override;

    // to allow sdr::properties::CustomShapeProperties access
    friend class sdr::properties::CustomShapeProperties;

    css::uno::Reference< css::drawing::XShape > mXRenderedCustomShape;

    mutable css::uno::Reference< css::drawing::XCustomShapeEngine > mxCustomShapeEngine;

    // #i37011# render geometry shadow
    SdrObject*                                          mpLastShadowGeometry;

    css::uno::Reference< css::drawing::XCustomShapeEngine > const & GetCustomShapeEngine() const;

//  SVX_DLLPRIVATE css::uno::Sequence< css::uno::Reference< css::drawing::XCustomShapeHandle > >
//      SdrObjCustomShape::GetInteraction( const SdrObjCustomShape* pCustomShape ) const;
// #i47293#
//  SVX_DLLPRIVATE std::vector< css::uno::Reference< css::drawing::XCustomShapeHandle > > GetFixedInteractionHandle() const;

    SVX_DLLPRIVATE std::vector< SdrCustomShapeInteraction > GetInteractionHandles() const;

    SVX_DLLPRIVATE void DragCreateObject( SdrDragStat& rDrag );

    SVX_DLLPRIVATE void DragResizeCustomShape( const Rectangle& rNewRect );
    SVX_DLLPRIVATE void DragMoveCustomShapeHdl( const Point& rDestination,
            const sal_uInt16 nCustomShapeHdlNum, bool bMoveCalloutRectangle );

    // #i37011# centralize throw-away of render geometry
    void InvalidateRenderGeometry();

    // #i38892#
    void ImpCheckCustomGluePointsAreAdded();

    // returns the new text rect that corresponds to the current logic rect, the return value can be empty if nothing changed.
    Rectangle ImpCalculateTextFrame( const bool bHgt, const bool bWdt );

public:
    // #i37011#
    const SdrObject* GetSdrObjectFromCustomShape() const;
    const SdrObject* GetSdrObjectShadowFromCustomShape() const;
    bool GetTextBounds( Rectangle& rTextBound ) const;
    bool IsTextPath() const;
    basegfx::B2DPolyPolygon GetLineGeometry( const bool bBezierAllowed ) const;

protected:
    // #115391# new method for SdrObjCustomShape and SdrTextObj to correctly handle and set
    // SdrTextMinFrameWidthItem and SdrTextMinFrameHeightItem based on all settings, necessities
    // and object sizes
    virtual void AdaptTextMinSize() override;

    Size          m_aSuggestedTextFrameSize;

public:

    bool UseNoFillStyle() const;

    bool IsMirroredX() const;
    bool IsMirroredY() const;
    bool IsPostRotate() const;
    void SetMirroredX( const bool bMirroredX );
    void SetMirroredY( const bool bMirroredY );

    double GetObjectRotation() const { return fObjectRotation;}
    double GetExtraTextRotation( const bool bPreRotation = false ) const;

    SdrObjCustomShape();
    virtual ~SdrObjCustomShape();

    /* is merging default attributes from type-shape into the SdrCustomShapeGeometryItem. If pType
    is NULL then the type is being taken from the "Type" property of the SdrCustomShapeGeometryItem.
    MergeDefaultAttributes is called when creating CustomShapes via UI and after importing */
    void MergeDefaultAttributes( const OUString* pType = nullptr );

    /* the method is checking if the geometry data is unchanged/default, in this case the data might not be stored */
    enum DefaultType
    {
        DEFAULT_PATH,
        DEFAULT_VIEWBOX,
        DEFAULT_SEGMENTS,
        DEFAULT_GLUEPOINTS,
        DEFAULT_STRETCHX,
        DEFAULT_STRETCHY,
        DEFAULT_EQUATIONS,
        DEFAULT_HANDLES,
        DEFAULT_TEXTFRAMES
    };
    bool IsDefaultGeometry( const DefaultType eDefaultType ) const;

    virtual sal_uInt16 GetObjIdentifier() const override;
    virtual void TakeObjInfo(SdrObjTransformInfoRec& rInfo) const override;

    virtual void SetModel(SdrModel* pNewModel) override;

    virtual const Rectangle& GetSnapRect()  const override;
    virtual const Rectangle& GetCurrentBoundRect() const override;
    virtual const Rectangle& GetLogicRect() const override;

    virtual void Move(const Size& rSiz) override;
    virtual void Resize(const Point& rRef, const Fraction& xFact, const Fraction& yFact, bool bUnsetRelative = true) override;
    virtual void Shear(const Point& rRef, long nAngle, double tn, bool bVShear) override;
    virtual void SetSnapRect(const Rectangle& rRect) override;
    virtual void SetLogicRect(const Rectangle& rRect) override;

    virtual void NbcMove(const Size& rSiz) override;
    virtual void NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact) override;
    virtual void NbcRotate(const Point& rRef, long nAngle, double sn, double cs) override;
    virtual void NbcMirror(const Point& rRef1, const Point& rRef2) override;
    virtual void NbcShear(const Point& rRef, long nAngle, double tn, bool bVShear) override;
    virtual void NbcSetSnapRect(const Rectangle& rRect) override;
    virtual void NbcSetLogicRect(const Rectangle& rRect) override;

    virtual SdrGluePoint GetVertexGluePoint(sal_uInt16 nNum) const override;

    virtual void NbcSetStyleSheet( SfxStyleSheet* pNewStyleSheet, bool bDontRemoveHardAttr ) override;

    // special drag methods
    virtual bool hasSpecialDrag() const override;
    virtual bool beginSpecialDrag(SdrDragStat& rDrag) const override;
    virtual bool applySpecialDrag(SdrDragStat& rDrag) override;

    virtual bool MovCreate(SdrDragStat& rStat) override; // #i37448#
    virtual bool EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd) override;

    /**
     * Allows suggesting the text frame size: in case the application has its
     * own text associated to the shape, instead of using the shape's editeng
     * text.
     */
    void SuggestTextFrameSize(Size aSuggestedTextFrameSize);
    virtual bool AdjustTextFrameWidthAndHeight(Rectangle& rR, bool bHgt = true, bool bWdt = true) const override;
    virtual bool NbcAdjustTextFrameWidthAndHeight(bool bHgt = true, bool bWdt = true) override;
    virtual bool AdjustTextFrameWidthAndHeight() override;
    virtual bool IsAutoGrowHeight() const override;
    virtual bool IsAutoGrowWidth() const override;
    virtual void SetVerticalWriting( bool bVertical ) override;
    virtual void TakeTextEditArea(Size* pPaperMin, Size* pPaperMax, Rectangle* pViewInit, Rectangle* pViewMin) const override;
    virtual void EndTextEdit( SdrOutliner& rOutl ) override;
    virtual void TakeTextAnchorRect( Rectangle& rAnchorRect ) const override;
    virtual void TakeTextRect( SdrOutliner& rOutliner, Rectangle& rTextRect, bool bNoEditText = false,
        Rectangle* pAnchorRect=nullptr, bool bLineWidth = true ) const override;
    virtual SdrObjCustomShape* Clone() const override;
    SdrObjCustomShape& operator=(const SdrObjCustomShape& rObj);

    virtual OUString TakeObjNameSingul() const override;
    virtual OUString TakeObjNamePlural() const override;

    virtual basegfx::B2DPolyPolygon TakeCreatePoly( const SdrDragStat& rDrag) const override;

    virtual basegfx::B2DPolyPolygon TakeXorPoly() const override;
    virtual basegfx::B2DPolyPolygon TakeContour() const override;

    virtual void NbcSetOutlinerParaObject(OutlinerParaObject* pTextObject) override;

    virtual SdrObject* DoConvertToPolyObj(bool bBezier, bool bAddText) const override;

    virtual void SetPage( SdrPage* pNewPage ) override;

    virtual SdrObjGeoData *NewGeoData() const override;
    virtual void          SaveGeoData(SdrObjGeoData &rGeo) const override;
    virtual void          RestGeoData(const SdrObjGeoData &rGeo) override;

    // need to take fObjectRotation instead of aGeo.nAngle, replace it temporary
    virtual bool TRGetBaseGeometry(basegfx::B2DHomMatrix& rMatrix, basegfx::B2DPolyPolygon& rPolyPolygon) const override;
    virtual void TRSetBaseGeometry(const basegfx::B2DHomMatrix& rMatrix, const basegfx::B2DPolyPolygon& rPolyPolygon) override;

    virtual const SdrGluePointList* GetGluePointList() const override;
    //virtual SdrGluePointList* GetGluePointList();
    virtual SdrGluePointList* ForceGluePointList() override;

    virtual sal_uInt32 GetHdlCount() const override;
    virtual SdrHdl* GetHdl( sal_uInt32 nHdlNum ) const override;

    // #i33136#
    static bool doConstructOrthogonal(const OUString& rName);

    using SdrTextObj::NbcSetOutlinerParaObject;

    OUString GetCustomShapeName();
};

#endif // INCLUDED_SVX_SVDOASHP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
