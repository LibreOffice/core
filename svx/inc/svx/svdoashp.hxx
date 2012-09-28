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

#ifndef _SVDOASHP_HXX
#define _SVDOASHP_HXX

#include <svx/svdobj.hxx>
#include <svx/svdotext.hxx>
#include <svx/svdhdl.hxx>
#include <vector>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/drawing/XCustomShapeEngine.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeAdjustmentValue.hpp>
#include "svx/svxdllapi.h"

//************************************************************
//   Vorausdeklarationen
//************************************************************

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

    sal_Bool    bMirroredX;
    sal_Bool    bMirroredY;
    double      fObjectRotation;

    com::sun::star::uno::Sequence< com::sun::star::drawing::EnhancedCustomShapeAdjustmentValue >
                aAdjustmentSeq;
};

#define CUSTOMSHAPE_HANDLE_RESIZE_FIXED         1
#define CUSTOMSHAPE_HANDLE_CREATE_FIXED         2
#define CUSTOMSHAPE_HANDLE_RESIZE_ABSOLUTE_X    4
#define CUSTOMSHAPE_HANDLE_RESIZE_ABSOLUTE_Y    8
#define CUSTOMSHAPE_HANDLE_MOVE_SHAPE           16
#define CUSTOMSHAPE_HANDLE_ORTHO4               32

struct SdrCustomShapeInteraction
{
    com::sun::star::uno::Reference< com::sun::star::drawing::XCustomShapeHandle >   xInteraction;
    com::sun::star::awt::Point                                                      aPosition;
    sal_Int32                                                                       nMode;
};

//************************************************************
//   SdrObjCustomShape
//************************************************************

class SVX_DLLPUBLIC SdrObjCustomShape : public SdrTextObj
{
private:
    // fObjectRotation is containing the object rotation in degrees.
    double fObjectRotation;

protected:
    virtual sdr::contact::ViewContact* CreateObjectSpecificViewContact();

public:
    virtual sdr::properties::BaseProperties* CreateObjectSpecificProperties();

    // to allow sdr::properties::CustomShapeProperties access
    friend class sdr::properties::CustomShapeProperties;

    com::sun::star::uno::Reference< com::sun::star::drawing::XShape > mXRenderedCustomShape;

    // #i37011# render geometry shadow
    SdrObject*                                          mpLastShadowGeometry;

    static com::sun::star::uno::Reference< com::sun::star::drawing::XCustomShapeEngine > GetCustomShapeEngine( const SdrObjCustomShape* pCustomShape );

//  SVX_DLLPRIVATE com::sun::star::uno::Sequence< com::sun::star::uno::Reference< com::sun::star::drawing::XCustomShapeHandle > >
//      SdrObjCustomShape::GetInteraction( const SdrObjCustomShape* pCustomShape ) const;
// #i47293#
//  SVX_DLLPRIVATE std::vector< com::sun::star::uno::Reference< com::sun::star::drawing::XCustomShapeHandle > > GetFixedInteractionHandle() const;

    SVX_DLLPRIVATE std::vector< SdrCustomShapeInteraction > GetInteractionHandles( const SdrObjCustomShape* pCustomShape ) const;

    SVX_DLLPRIVATE void DragCreateObject( SdrDragStat& rDrag );

    SVX_DLLPRIVATE void DragResizeCustomShape( const Rectangle& rNewRect, SdrObjCustomShape* pObj ) const;
    SVX_DLLPRIVATE void DragMoveCustomShapeHdl( const Point aDestination, const sal_uInt16 nCustomShapeHdlNum, SdrObjCustomShape* pObj ) const;

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
    sal_Bool GetTextBounds( Rectangle& rTextBound ) const;
    sal_Bool IsTextPath() const;
    static basegfx::B2DPolyPolygon GetLineGeometry( const SdrObjCustomShape* pCustomShape, const sal_Bool bBezierAllowed );

protected:

    String      aName;

public:

    sal_Bool UseNoFillStyle() const;

    sal_Bool IsMirroredX() const;
    sal_Bool IsMirroredY() const;
    bool     IsPostRotate() const;
    void SetMirroredX( const sal_Bool bMirroredX );
    void SetMirroredY( const sal_Bool bMirroredY );

    double GetObjectRotation() const;
    double GetExtraTextRotation( const bool bPreRotation = false ) const;

    TYPEINFO();
    SdrObjCustomShape();
    virtual ~SdrObjCustomShape();

    /* is merging default attributes from type-shype into the SdrCustomShapeGeometryItem. If pType
    is NULL then the type is being taken from the "Type" property of the SdrCustomShapeGeometryItem.
    MergeDefaultAttributes is called when creating CustomShapes via UI and after importing */
    void MergeDefaultAttributes( const rtl::OUString* pType = NULL );

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
    sal_Bool IsDefaultGeometry( const DefaultType eDefaultType ) const;

    virtual sal_uInt16 GetObjIdentifier() const;
    virtual void TakeObjInfo(SdrObjTransformInfoRec& rInfo) const;

    virtual void SetModel(SdrModel* pNewModel);

    virtual void RecalcSnapRect();

    virtual const Rectangle& GetSnapRect()  const;
    virtual const Rectangle& GetCurrentBoundRect() const;
    virtual const Rectangle& GetLogicRect() const;

    virtual void Move(const Size& rSiz);
    virtual void Resize(const Point& rRef, const Fraction& xFact, const Fraction& yFact, bool bUnsetRelative = true);
    virtual void Shear(const Point& rRef, long nWink, double tn, bool bVShear);
    virtual void SetSnapRect(const Rectangle& rRect);
    virtual void SetLogicRect(const Rectangle& rRect);

    virtual void NbcMove(const Size& rSiz);
    virtual void NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact);
    virtual void NbcRotate(const Point& rRef, long nWink, double sn, double cs);
    virtual void NbcMirror(const Point& rRef1, const Point& rRef2);
    virtual void NbcShear(const Point& rRef, long nWink, double tn, bool bVShear);
    virtual void NbcSetSnapRect(const Rectangle& rRect);
    virtual void NbcSetLogicRect(const Rectangle& rRect);

    virtual SdrGluePoint GetVertexGluePoint(sal_uInt16 nNum) const;

    virtual void NbcSetStyleSheet( SfxStyleSheet* pNewStyleSheet, sal_Bool bDontRemoveHardAttr );

    // special drag methods
    virtual bool hasSpecialDrag() const;
    virtual bool beginSpecialDrag(SdrDragStat& rDrag) const;
    virtual bool applySpecialDrag(SdrDragStat& rDrag);

    virtual bool BegCreate( SdrDragStat& rStat );
    virtual bool MovCreate(SdrDragStat& rStat); // #i37448#
    virtual bool EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd);

    virtual bool AdjustTextFrameWidthAndHeight(Rectangle& rR, bool bHgt = true, bool bWdt = true) const;
    virtual bool NbcAdjustTextFrameWidthAndHeight(bool bHgt = true, bool bWdt = true);
    virtual bool AdjustTextFrameWidthAndHeight(bool bHgt = true, bool bWdt = true);
    virtual bool IsAutoGrowHeight() const;
    virtual bool IsAutoGrowWidth() const;
    virtual void SetVerticalWriting( sal_Bool bVertical );
    virtual sal_Bool BegTextEdit( SdrOutliner& rOutl );
    virtual void TakeTextEditArea(Size* pPaperMin, Size* pPaperMax, Rectangle* pViewInit, Rectangle* pViewMin) const;
    virtual void EndTextEdit( SdrOutliner& rOutl );
    virtual void TakeTextAnchorRect( Rectangle& rAnchorRect ) const;
    virtual void TakeTextRect( SdrOutliner& rOutliner, Rectangle& rTextRect, bool bNoEditText = false,
        Rectangle* pAnchorRect=NULL, bool bLineWidth = true ) const;
    virtual SdrObjCustomShape* Clone() const;
    SdrObjCustomShape& operator=(const SdrObjCustomShape& rObj);

    virtual void TakeObjNameSingul(String& rName) const;
    virtual void TakeObjNamePlural(String& rName) const;

    virtual basegfx::B2DPolyPolygon TakeCreatePoly( const SdrDragStat& rDrag) const;

    virtual basegfx::B2DPolyPolygon TakeXorPoly() const;
    virtual basegfx::B2DPolyPolygon TakeContour() const;

    virtual void NbcSetOutlinerParaObject(OutlinerParaObject* pTextObject);

    virtual SdrObject* DoConvertToPolyObj(sal_Bool bBezier) const;

    virtual void SetPage( SdrPage* pNewPage );

    virtual SdrObjGeoData *NewGeoData() const;
    virtual void          SaveGeoData(SdrObjGeoData &rGeo) const;
    virtual void          RestGeoData(const SdrObjGeoData &rGeo);

    // need to take fObjectRotation instead of aGeo.nWink, replace it temporary
    virtual sal_Bool TRGetBaseGeometry(basegfx::B2DHomMatrix& rMatrix, basegfx::B2DPolyPolygon& rPolyPolygon) const;
    virtual void TRSetBaseGeometry(const basegfx::B2DHomMatrix& rMatrix, const basegfx::B2DPolyPolygon& rPolyPolygon);

    virtual const SdrGluePointList* GetGluePointList() const;
    //virtual SdrGluePointList* GetGluePointList();
    virtual SdrGluePointList* ForceGluePointList();

    virtual sal_uInt32 GetHdlCount() const;
    virtual SdrHdl* GetHdl( sal_uInt32 nHdlNum ) const;

    // #i33136#
    static bool doConstructOrthogonal(const ::rtl::OUString& rName);

    using SdrTextObj::NbcSetOutlinerParaObject;
};

#endif //_SVDOASHP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
