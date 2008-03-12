/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: svdoashp.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 09:29:57 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SVDOASHP_HXX
#define _SVDOASHP_HXX

#ifndef _SVDOBJ_HXX
#include <svx/svdobj.hxx>
#endif
#ifndef _SVDOATTR_HXX
#include <svx/svdotext.hxx>
#endif
#ifndef _SVDHDL_HXX
#include <svx/svdhdl.hxx>
#endif
#include <vector>
#include <com/sun/star/uno/Reference.h>
#ifndef _COM_SUN_STAR_DRAWING_XCUSTOMSHAPEENGINE_HPP_
#include <com/sun/star/drawing/XCustomShapeEngine.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_ENHANCEDCUSTOMSHAPEADJUSTMENTVALUE_HPP_
#include <com/sun/star/drawing/EnhancedCustomShapeAdjustmentValue.hpp>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

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
    // fObjectRotation is containing the object rotation in degrees.
    double fObjectRotation;

private:
    // BaseProperties section
    virtual sdr::properties::BaseProperties* CreateObjectSpecificProperties();

    virtual sdr::contact::ViewContact* CreateObjectSpecificViewContact();

    // to allow sdr::properties::CustomShapeProperties access
    friend class sdr::properties::CustomShapeProperties;

    com::sun::star::uno::Reference< com::sun::star::drawing::XShape > mXRenderedCustomShape;

    // #i37011# render geometry shadow
    SdrObject*                                          mpLastShadowGeometry;

    SVX_DLLPRIVATE com::sun::star::uno::Reference< com::sun::star::drawing::XCustomShapeEngine > GetCustomShapeEngine( const SdrObjCustomShape* pCustomShape ) const;
    SVX_DLLPRIVATE basegfx::B2DPolyPolygon GetLineGeometry( const SdrObjCustomShape* pCustomShape, const sal_Bool bBezierAllowed ) const;

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
    Rectangle ImpCalculateTextFrame( const FASTBOOL bHgt, const FASTBOOL bWdt );

public:
    // #i37011#
    const SdrObject* GetSdrObjectFromCustomShape() const;
    const SdrObject* GetSdrObjectShadowFromCustomShape() const;
    const sal_Bool GetTextBounds( Rectangle& rTextBound ) const;
    const sal_Bool IsTextPath() const;

protected:

    String      aName;

public:

    const sal_Bool UseNoFillStyle() const;

    const sal_Bool IsMirroredX() const;
    const sal_Bool IsMirroredY() const;
    void SetMirroredX( const sal_Bool bMirroredX );
    void SetMirroredY( const sal_Bool bMirroredY );

    const double GetObjectRotation() const;
    const double GetExtraTextRotation() const;

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

    virtual UINT16 GetObjIdentifier() const;
    virtual void TakeObjInfo(SdrObjTransformInfoRec& rInfo) const;

    virtual void RecalcSnapRect();
    virtual void RecalcBoundRect();

    virtual const Rectangle& GetSnapRect()  const;
    virtual const Rectangle& GetCurrentBoundRect() const;
    virtual const Rectangle& GetLogicRect() const;

    virtual void Move(const Size& rSiz);
    virtual void Resize(const Point& rRef, const Fraction& xFact, const Fraction& yFact);
    virtual void Shear(const Point& rRef, long nWink, double tn, FASTBOOL bVShear);
    virtual void SetSnapRect(const Rectangle& rRect);
    virtual void SetLogicRect(const Rectangle& rRect);

    virtual void NbcMove(const Size& rSiz);
    virtual void NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact);
    virtual void NbcRotate(const Point& rRef, long nWink, double sn, double cs);
    virtual void NbcMirror(const Point& rRef1, const Point& rRef2);
    virtual void NbcShear(const Point& rRef, long nWink, double tn, FASTBOOL bVShear);
    virtual void NbcSetSnapRect(const Rectangle& rRect);
    virtual void NbcSetLogicRect(const Rectangle& rRect);

    virtual void NbcSetStyleSheet( SfxStyleSheet* pNewStyleSheet, sal_Bool bDontRemoveHardAttr );
    virtual sal_Bool DoPaintObject(XOutputDevice& rOut, const SdrPaintInfoRec& rInfoRec) const;

    virtual FASTBOOL HasSpecialDrag() const;
    virtual FASTBOOL BegDrag( SdrDragStat& rDrag )  const;
    virtual FASTBOOL EndDrag( SdrDragStat& rDrag );

    virtual FASTBOOL BegCreate( SdrDragStat& rStat );
    virtual FASTBOOL MovCreate(SdrDragStat& rStat); // #i37448#
    virtual FASTBOOL EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd);

    virtual void     BrkDrag( SdrDragStat& rDrag ) const;
    virtual FASTBOOL MovDrag( SdrDragStat& rDrag ) const;
    virtual basegfx::B2DPolyPolygon TakeDragPoly(const SdrDragStat& rDrag) const;

    virtual FASTBOOL AdjustTextFrameWidthAndHeight(Rectangle& rR, FASTBOOL bHgt=TRUE, FASTBOOL bWdt=TRUE) const;
    virtual FASTBOOL NbcAdjustTextFrameWidthAndHeight(FASTBOOL bHgt=TRUE, FASTBOOL bWdt=TRUE);
    virtual FASTBOOL AdjustTextFrameWidthAndHeight(FASTBOOL bHgt=TRUE, FASTBOOL bWdt=TRUE);
    virtual FASTBOOL IsAutoGrowHeight() const;
    virtual FASTBOOL IsAutoGrowWidth() const;
    virtual void SetVerticalWriting( sal_Bool bVertical );
    virtual sal_Bool BegTextEdit( SdrOutliner& rOutl );
    virtual void TakeTextEditArea(Size* pPaperMin, Size* pPaperMax, Rectangle* pViewInit, Rectangle* pViewMin) const;
    virtual void EndTextEdit( SdrOutliner& rOutl );
    virtual void TakeTextAnchorRect( Rectangle& rAnchorRect ) const;
    virtual void TakeTextRect( SdrOutliner& rOutliner, Rectangle& rTextRect, FASTBOOL bNoEditText=FALSE,
        Rectangle* pAnchorRect=NULL, BOOL bLineWidth=TRUE ) const;

    virtual SdrObject* CheckHit(const Point& rPnt, USHORT nTol, const SetOfByte* pVisiLayer) const;
    virtual void operator=(const SdrObject& rObj);

    virtual void TakeObjNameSingul(String& rName) const;
    virtual void TakeObjNamePlural(String& rName) const;

    virtual basegfx::B2DPolyPolygon TakeCreatePoly( const SdrDragStat& rDrag) const;

    virtual basegfx::B2DPolyPolygon TakeXorPoly(sal_Bool bDetail) const;
    virtual basegfx::B2DPolyPolygon TakeContour() const;

    virtual void NbcSetOutlinerParaObject(OutlinerParaObject* pTextObject);

    virtual SdrObject* DoConvertToPolyObj(BOOL bBezier) const;

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

