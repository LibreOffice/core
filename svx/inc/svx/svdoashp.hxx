/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

class SdrAShapeObjGeoData : public SdrObjGeoData
{
    public:

    // TTTT: MirrorX/Y removed, is part of object transformation now
//  bool    bMirroredX;
//  bool    bMirroredY;

    // TTTT: Should be obsolete
    // double       fObjectRotation; // TTTT: Is this needed? Check, may remove some extra handling

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
    // TTTT: SHould be obsolete
    // double fObjectRotation;

protected:
    virtual sdr::properties::BaseProperties* CreateObjectSpecificProperties();
    virtual sdr::contact::ViewContact* CreateObjectSpecificViewContact();
    virtual sdr::glue::GluePointProvider* CreateObjectSpecificGluePointProvider();

public:
    // to allow sdr::properties::CustomShapeProperties access
    friend class sdr::properties::CustomShapeProperties;

    com::sun::star::uno::Reference< com::sun::star::drawing::XShape > mXRenderedCustomShape;

    // #i37011# render geometry shadow
    SdrObject*                                          mpLastShadowGeometry;

    static com::sun::star::uno::Reference< com::sun::star::drawing::XCustomShapeEngine > GetCustomShapeEngine( const SdrObjCustomShape* pCustomShape );

    SVX_DLLPRIVATE std::vector< SdrCustomShapeInteraction > GetInteractionHandles( const SdrObjCustomShape* pCustomShape ) const;
    SVX_DLLPRIVATE void DragCreateObject( SdrDragStat& rDrag );
    SVX_DLLPRIVATE void DragMoveCustomShapeHdl( const basegfx::B2DPoint& rDestination, const sal_uInt32 nCustomShapeHdlNum, SdrObjCustomShape* pObj ) const;

    // #i37011# centralize throw-away of render geometry
    void InvalidateRenderGeometry();

    // returns the new text range that corresponds to the current logic range. The return value can be empty if nothing changed.
    basegfx::B2DRange ImpCalculateTextFrame();

public:
    // #i37011#
    const SdrObject* GetSdrObjectFromCustomShape() const;
    const SdrObject* GetSdrObjectShadowFromCustomShape() const;
    basegfx::B2DRange getRawUnifiedTextRange() const;
    bool IsTextPath() const;
    static basegfx::B2DPolyPolygon GetLineGeometry( const SdrObjCustomShape* pCustomShape, const bool bBezierAllowed );

protected:
    // #115391# new method for SdrObjCustomShape and SdrTextObj to correctly handle and set
    // SdrTextMinFrameWidthItem and SdrTextMinFrameHeightItem based on all settings, necessities
    // and object sizes
    virtual void AdaptTextMinSize();

    String      aName;

    virtual ~SdrObjCustomShape();

    /// method to copy all data from given source
    virtual void copyDataFromSdrObject(const SdrObject& rSource);

    virtual basegfx::B2DRange AdjustTextFrameWidthAndHeight(const basegfx::B2DRange& rRange) const;

public:
    /// create a copy, evtl. with a different target model (if given)
    virtual SdrObject* CloneSdrObject(SdrModel* pTargetModel = 0) const;

    // react on model/page change
    virtual void handlePageChange(SdrPage* pOldPage, SdrPage* pNewPage);

    bool UseNoFillStyle() const;

    // TTTT: MirroredX/Y removed
    //bool IsMirroredX() const;
    //bool IsMirroredY() const;
    //void SetMirroredX( const bool bMirroredX );
    //void SetMirroredY( const bool bMirroredY );

    //double GetObjectRotation() const;
    double GetExtraTextRotation() const;

    SdrObjCustomShape(SdrModel& rSdrModel);

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
    bool IsDefaultGeometry( const DefaultType eDefaultType ) const;

    virtual sal_uInt16 GetObjIdentifier() const;
    virtual void TakeObjInfo(SdrObjTransformInfoRec& rInfo) const;
    virtual void SetStyleSheet( SfxStyleSheet* pNewStyleSheet, bool bDontRemoveHardAttr );

    // special drag methods
    virtual bool hasSpecialDrag() const;
    virtual bool beginSpecialDrag(SdrDragStat& rDrag) const;
    virtual bool applySpecialDrag(SdrDragStat& rDrag);

    virtual bool MovCreate(SdrDragStat& rStat); // #i37448#
    virtual bool EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd);

    virtual void AdjustTextFrameWidthAndHeight();
    virtual bool IsAutoGrowHeight() const;
    virtual bool IsAutoGrowWidth() const;
    virtual void SetVerticalWriting( bool bVertical );
    virtual bool BegTextEdit( SdrOutliner& rOutl );
    virtual void TakeTextEditArea(basegfx::B2DVector* pPaperMin, basegfx::B2DVector* pPaperMax, basegfx::B2DRange* pViewInit, basegfx::B2DRange* pViewMin) const;
    virtual void EndTextEdit( SdrOutliner& rOutl );
    virtual basegfx::B2DRange getUnifiedTextRange() const;
    virtual void TakeTextRange(SdrOutliner& rOutliner, basegfx::B2DRange& rTextRange, basegfx::B2DRange& rAnchorRange) const;
    virtual void SetChanged();

    virtual void TakeObjNameSingul(String& rName) const;
    virtual void TakeObjNamePlural(String& rName) const;

    virtual basegfx::B2DPolyPolygon TakeCreatePoly( const SdrDragStat& rDrag) const;

    virtual basegfx::B2DPolyPolygon TakeXorPoly() const;
    virtual SdrObject* DoConvertToPolygonObject(bool bBezier, bool bAddText) const;

    virtual SdrObjGeoData *NewGeoData() const;
    virtual void          SaveGeoData(SdrObjGeoData &rGeo) const;
    virtual void          RestGeoData(const SdrObjGeoData &rGeo);

    virtual void AddToHdlList(SdrHdlList& rHdlList) const;

    // #i33136#
    static bool doConstructOrthogonal(const ::rtl::OUString& rName);

    virtual void setSdrObjectTransformation(const basegfx::B2DHomMatrix& rTransformation);

    //IAccessibility2 Implementation 2009-----
    ::rtl::OUString GetCustomShapeName();
    //-----IAccessibility2 Implementation 2009
};

#endif //_SVDOASHP_HXX

