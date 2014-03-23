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



#ifndef _SVDHDL_HXX
#define _SVDHDL_HXX

#include <tools/gen.hxx>
#include <vcl/pointr.hxx>
#include <tools/contnr.hxx>
#include <svl/solar.hrc>
#include <svx/xpoly.hxx>
#include <svx/svdoedge.hxx>
#include <svx/sdr/overlay/overlayobjectlist.hxx>
#include "svx/svxdllapi.h"
#include <svx/xpoly.hxx>
#include <svx/svdoedge.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <svx/sdgcpitm.hxx>

////////////////////////////////////////////////////////////////////////////////////////////////////
// predefines

class SdrHdlList;
class SdrMarkView;
class SdrObject;
class MouseEvent;

#define SDR_HANDLE_COLOR_SIZE_NORMAL            Size(13, 13)
#define SDR_HANDLE_COLOR_SIZE_SELECTED          Size(17, 17)

////////////////////////////////////////////////////////////////////////////////////////////////////
// Jedes Objekt muss in der Lage seine Handles zu erzeugen. Diese werden dann
// bei einer Selektion abgeholt, bei der View angemeldet und sichtbar gemacht.
// Wird ein Handle von der Maus beruehrt (IsHit()), so wird von der View der
// entsprechende Mauszeiger vom Handle abgeholt und der App auf Anfrage zum
// reinschalten uebergeben.
// Handles wie z.B. der Rotationsmittelpunkt oder die Spiegelachse werden von
// der View generiert, wenn vom Controller der entsprechende Transformations-
// Modus selektiert wird.
// HDL_MOVE...HDL_LWRGT muessen im enum immer zusammen stehen bleiben!

enum SdrHdlKind
{
    HDL_MOVE,       // Handle zum Verschieben des Objekts
    HDL_UPLFT,      // Oben links
    HDL_UPPER,      // Oben
    HDL_UPRGT,      // Oben rechts
    HDL_LEFT,       // Links
    HDL_RIGHT,      // Rechts
    HDL_LWLFT,      // Unten links
    HDL_LOWER,      // Unten
    HDL_LWRGT,      // Unten rechts
    HDL_POLY,       // Punktselektion an Polygon oder Bezierkurve
    HDL_BWGT,       // Gewicht an einer Bezierkurve
    HDL_CIRC,       // Winkel an Kreissegmenten, Eckenradius am Rect
    HDL_REF1,       // Referenzpunkt 1, z.B. Rotationsmitte
    HDL_REF2,       // Referenzpunkt 2, z.B. Endpunkt der Spiegelachse
    HDL_MIRX,       // Die Spiegelachse selbst
    HDL_GLUE,       // GluePoint
    HDL_ANCHOR,     // anchor symbol (SD, SW)
    HDL_TRNS,       // interactive transparence
    HDL_GRAD,       // interactive gradient
    HDL_COLR,       // interactive color
    HDL_USER,
    HDL_ANCHOR_TR,  // #101688# Anchor handle with (0,0) at top right for SW

    // for SJ and the CustomShapeHandles:
    HDL_CUSTOMSHAPE1,

    HDL_SMARTTAG
};

////////////////////////////////////////////////////////////////////////////////////////////////////

enum BitmapColorIndex
{
    LightGreen,
    Cyan,
    LightCyan,
    Red,
    LightRed,
    Yellow
};

////////////////////////////////////////////////////////////////////////////////////////////////////

enum BitmapMarkerKind
{
    Rect_7x7,
    Rect_9x9,
    Rect_11x11,
    Rect_13x13,
    Circ_7x7,
    Circ_9x9,
    Circ_11x11,
    Elli_7x9,
    Elli_9x11,
    Elli_9x7,
    Elli_11x9,
    RectPlus_7x7,
    RectPlus_9x9,
    RectPlus_11x11,
    Crosshair,
    Glue,
    Anchor,

    // #98388# add AnchorPressed to be able to aninate anchor control, too.
    AnchorPressed,

    // #101688# AnchorTR for SW
    AnchorTR,
    AnchorPressedTR,

    // for SJ and the CustomShapeHandles:
    Customshape_7x7,
    Customshape_9x9,
    Customshape_11x11
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  SdrHdl
//      SmartHdl
//          SdPathHdl
//          AnnotationHdl
//          ImageButtonHdl
//      SdrHdlColor
//      SdrHdlGradient
//      SdrHdlLine
//      SdrHdlBezWgt
//      E3dVolumeMarker
//      ImpEdgeHdl
//      ImpMeasureHdl
//      ImpTextframeHdl
//      SdrCropHdl
//      TableEdgeHdl
//      TableBorderHdl
//      SwSdrHdl
//

typedef ::std::vector< SdrHdl* > SdrHdlContainerType;

class SVX_DLLPUBLIC SdrHdl : private boost::noncopyable
{
private:
    // #101928#
    BitmapEx ImpGetBitmapEx(BitmapMarkerKind eKindOfMarker, sal_uInt16 nInd, bool bFine, bool bIsHighContrast);

protected:
    SdrHdlList&                 mrHdlList;
    const SdrObject*            mpSdrHdlObject;
    SdrHdlKind                  meKind;
    basegfx::B2DPoint           maPosition;

    // OVERLAYMANAGER
    ::sdr::overlay::OverlayObjectList           maOverlayGroup;

    sal_uInt32                  mnObjHdlNum;    // incremental count without XPOLY_CONTROL based on old XPolygon
    sal_uInt32                  mnPolyNum;      // polygon index based old XPolygon
    sal_uInt32                  mnPPntNum;      // point index for !XPOLY_CONTROL based old XPolygon
    sal_uInt32                  mnSourceHdlNum; // incremental count including XPOLY_CONTROL based on old XPolygon

    bool                        mbSelect : 1;   // selected polygon/control point
    bool                        mb1PixMore : 1; // bigger polygon start handles
    bool                        mbPlusHdl : 1;  // XPOLY_CONTROL

    // Means green handle. This was implicitely decided in old version by late setting the mnObjHdlNum
    bool                        mbIsFrameHandle : 1;

    bool                        mbMoveOutside : 1; // forces this handle to be moved outside of the selection rectangle
    bool                        mbMouseOver : 1;    // is true if the mouse is over this handle

    // create all markers for all SdrPageWindows
    void CreateB2dIAObject();

    // create single marker of this kind
    virtual void CreateB2dIAObject(::sdr::overlay::OverlayManager& rOverlayManager);

    // cleanup marker if one exists
    void GetRidOfIAObject();

    ::sdr::overlay::OverlayObject* CreateOverlayObject(
        const basegfx::B2DPoint& rPos,
        BitmapColorIndex eColIndex,
        BitmapMarkerKind eKindOfMarker,
        sal_Int16 nMoveOutsideX,
        sal_Int16 nMoveOutsideY);
    BitmapMarkerKind GetNextBigger(BitmapMarkerKind eKnd) const;

public:
    SdrHdl(
        SdrHdlList& rHdlList,
        const SdrObject* pSdrHdlObject = 0,
        SdrHdlKind eNewKind = HDL_MOVE,
        const basegfx::B2DPoint& rPosition = basegfx::B2DPoint(),
        bool bIsFrameHandle = false);

protected:
    friend class SdrHdlList;
    virtual ~SdrHdl();

public:
    const ::sdr::overlay::OverlayObjectList& getOverlayObjectList() const { return maOverlayGroup; }

    SdrHdlKind GetKind() const { return meKind; }
    void Touch();

    virtual const basegfx::B2DPoint& getPosition() const;
    virtual void setPosition(const basegfx::B2DPoint& rNew);

    const SdrObject* GetObj() const { return mpSdrHdlObject;  }

    bool IsSelected() const { return mbSelect; }
    void SetSelected(bool bJa = true);

    void Set1PixMore(bool bJa = true);

    bool IsCornerHdl() const { return meKind==HDL_UPLFT || meKind==HDL_UPRGT || meKind==HDL_LWLFT || meKind==HDL_LWRGT; }
    bool IsVertexHdl() const { return meKind==HDL_UPPER || meKind==HDL_LOWER || meKind==HDL_LEFT  || meKind==HDL_RIGHT; }

    void SetObjHdlNum(sal_uInt32 nNum) { mnObjHdlNum=nNum; }
    sal_uInt32 GetObjHdlNum() const { return mnObjHdlNum; }

    void SetPolyNum(sal_uInt32 nNum) { mnPolyNum=nNum; }
    sal_uInt32 GetPolyNum() const { return mnPolyNum; }

    void SetPointNum(sal_uInt32 nNum) { mnPPntNum=nNum; }
    sal_uInt32 GetPointNum() const { return mnPPntNum; }

    bool IsPlusHdl() const { return mbPlusHdl; }
    bool IsFrameHandle() const { return mbIsFrameHandle; }

    void SetSourceHdlNum(sal_uInt32 nNum) { mnSourceHdlNum=nNum; }
    sal_uInt32 GetSourceHdlNum() const { return mnSourceHdlNum; }

    virtual Pointer GetPointer() const;
    bool IsHdlHit(const basegfx::B2DPoint& rPosition) const;

    // #97016# II
    virtual bool IsFocusHdl() const;

    void SetMoveOutside( bool bMoveOutside );

    /** is called when the mouse enters the area of this handle. If the handle changes his
        visualisation during mouse over it must override this method and call Touch(). */
    virtual void onMouseEnter(const MouseEvent& rMEvt);

    /** is called when the mouse leaves the area of this handle. If the handle changes his
        visualisation during mouse over it must override this method and call Touch(). */
    virtual void onMouseLeave();

    bool isMouseOver() const;
    void setMouseOver(bool bNew);
};

////////////////////////////////////////////////////////////////////////////////////////////////////

class SVX_DLLPUBLIC SdrHdlList : private boost::noncopyable
{
private:
    friend class SdrHdl;

    SVX_DLLPRIVATE bool operator==(const SdrHdlList&);
    SVX_DLLPRIVATE bool operator!=(const SdrHdlList&);

protected:
    sal_uInt32              mnFocusIndex;
    SdrMarkView&            mrView;
    SdrHdlContainerType     maList;
    sal_uInt16              mnHdlSize;

    /// bitfield
    bool                    mbRotateShear : 1;
    bool                    mbDistortShear : 1;
    bool                    mbMoveOutside : 1;      // Handles nach aussen ruecken (fuer TextEdit)
    bool                    mbFineHandles : 1;

public:
    SdrHdlList(SdrMarkView& rV);
    ~SdrHdlList();

    void Clear();
    void CreateVisualizations();

    // #97016# II
    void TravelFocusHdl(bool bForward);
    SdrHdl* GetFocusHdl() const;
    void SetFocusHdl(SdrHdl* pNew);
    void ResetFocusHdl();

    // Access to View
    SdrMarkView& GetViewFromSdrHdlList() const { return mrView; }

    // Sortierung: 1.Level Erst Refpunkt-Handles, dann normale Handles, dann Glue, dann User, dann Plushandles
    //             2.Level PageView (Pointer)
    //             3.Level Position (x+y)
    void Sort();
    sal_uInt32 GetHdlCount() const { return maList.size(); }
    SdrHdl* GetHdlByIndex(sal_uInt32 nNum) const;
    sal_uInt32 GetHdlNum(const SdrHdl* pHdl) const;

    void SetHdlSize(sal_uInt16 nSiz);
    sal_uInt16 GetHdlSize() const { return mnHdlSize; }

    void SetMoveOutside(bool bOn);
    bool IsMoveOutside() const { return mbMoveOutside; }

    void SetRotateShear(bool bOn) { mbRotateShear = bOn; }
    bool IsRotateShear() const { return mbRotateShear; }

    void SetDistortShear(bool bOn) { mbDistortShear = bOn; }
    bool IsDistortShear() const { return mbDistortShear; }

    void SetFineHdl(bool bOn);
    bool IsFineHdl() const { return mbFineHandles; }

    // Zuletzt eingefuegte Handles werden am ehesten getroffen
    // (wenn Handles uebereinander liegen).
    SdrHdl* IsHdlListHit(const basegfx::B2DPoint& rPosition) const;
    SdrHdl* GetHdlByKind(SdrHdlKind eKind1) const;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

class SVX_DLLPUBLIC SdrHdlColor : public SdrHdl
{
private:
    // size of colr markers
    Size                        aMarkerSize;

    // color
    Color                       aMarkerColor;

    // callback link when value changed
    Link                        aColorChangeHdl;

    // use luminance values only
    bool                        bUseLuminance : 1;

    // help functions
    SVX_DLLPRIVATE Bitmap CreateColorDropper(Color aCol);
    SVX_DLLPRIVATE Color GetLuminance(const Color& rCol);
    SVX_DLLPRIVATE void CallColorChangeLink();

protected:
    // create marker for this kind
    virtual void CreateB2dIAObject(::sdr::overlay::OverlayManager& rOverlayManager);

    virtual ~SdrHdlColor();

public:
    SdrHdlColor(
        SdrHdlList& rHdlList,
        const SdrObject& rSdrHdlObject,
        const basegfx::B2DPoint& rRef,
        Color aCol = Color(COL_BLACK),
        const Size& rSize = Size(11, 11),
        bool bLum = false);

    bool IsUseLuminance() const { return bUseLuminance; }

    Color GetColor() const { return aMarkerColor; }
    void SetColor(Color aNew, bool bCallLink = false);

    const Size& GetSize() const { return aMarkerSize; }
    void SetSize(const Size& rNew);

    void SetColorChangeHdl(const Link& rLink) { aColorChangeHdl = rLink; }
    const Link& GetColorChangeHdl() const { return aColorChangeHdl; }
};

////////////////////////////////////////////////////////////////////////////////////////////////////

class SdrHdlGradient : public SdrHdl
{
private:
    // pointer to used color handles
    SdrHdlColor&                mrColHdl1;
    SdrHdlColor&                mrColHdl2;

    // is this a gradient or a transparence
    bool                        bGradient : 1;

    // select which handle to move
    bool                        bMoveSingleHandle : 1;
    bool                        bMoveFirstHandle : 1;

protected:
    // create marker for this kind
    virtual void CreateB2dIAObject(::sdr::overlay::OverlayManager& rOverlayManager);

    virtual ~SdrHdlGradient();

public:
    SdrHdlGradient(
        SdrHdlList& rHdlList,
        const SdrObject& rSdrHdlObject,
        SdrHdlColor& rSdrHdlColor1,
        SdrHdlColor& rSdrHdlColor2,
        bool bGrad = false);

    bool IsGradient() const { return bGradient; }

    virtual const basegfx::B2DPoint& getPosition() const;
    virtual void setPosition(const basegfx::B2DPoint& rNew);

    virtual const basegfx::B2DPoint& get2ndPosition() const;
    virtual void set2ndPosition(const basegfx::B2DPoint& rNew);

    SdrHdlColor& getColHdl1() { return mrColHdl1; }
    SdrHdlColor& getColHdl2() { return mrColHdl2; }

    // the link called by the color handles
    DECL_LINK(ColorChangeHdl, SdrHdl*);

    // transformation call, create gradient from this handle
    void FromIAOToItem(bool bSetItemOnObject, bool bUndo);

    // selection flags for interaction
    bool IsMoveSingleHandle() const { return bMoveSingleHandle; }
    void SetMoveSingleHandle(bool bNew) { bMoveSingleHandle = bNew; }
    bool IsMoveFirstHandle() const { return bMoveFirstHandle; }
    void SetMoveFirstHandle(bool bNew) { bMoveFirstHandle = bNew; }
};

////////////////////////////////////////////////////////////////////////////////////////////////////

// Spiegelachse
class SdrHdlLine: public SdrHdl
{
private:
protected:
    // create marker for this kind
    virtual void CreateB2dIAObject(::sdr::overlay::OverlayManager& rOverlayManager);

    virtual ~SdrHdlLine();

    SdrHdl*                     pHdl1;
    SdrHdl*                     pHdl2;

public:
    SdrHdlLine(
        SdrHdlList& rHdlList,
        SdrHdlKind eNewKind = HDL_MIRX)
    :   SdrHdl(rHdlList, 0, eNewKind),
        pHdl1(0),
        pHdl2(0)
    {
    }

    virtual Pointer GetPointer() const;
    void SetHandles(SdrHdl* p1, SdrHdl* p2) { pHdl1 = p1; pHdl2 = p2; }
};

// Ein SdrHdlBezWgt hat Kenntnis von seinem "BasisHandle". Seine Draw-Methode
// zeichnet zusaetzlich eine Linie von seiner Position zur Position dieses
// BasisHandles.
class SdrHdlBezWgt: public SdrHdl
{
private:
protected:
    // create marker for this kind
    virtual void CreateB2dIAObject(::sdr::overlay::OverlayManager& rOverlayManager);

    virtual ~SdrHdlBezWgt();

    const SdrHdl* pHdl1;

public:
    // das ist kein Copy-Ctor!!!
    SdrHdlBezWgt(
        SdrHdlList& rHdlList,
        const SdrObject& rSdrHdlObject,
        const SdrHdl& rRefHdl1,
        SdrHdlKind eNewKind = HDL_BWGT)
    :   SdrHdl(rHdlList, &rSdrHdlObject, eNewKind),
        pHdl1(&rRefHdl1)
    {
        mbPlusHdl = true;
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////

class E3dVolumeMarker : public SdrHdl
{
private:
    basegfx::B2DPolyPolygon             aWireframePoly;

protected:
    // create marker for this kind
    virtual void CreateB2dIAObject(::sdr::overlay::OverlayManager& rOverlayManager);

    virtual ~E3dVolumeMarker();

public:
    E3dVolumeMarker(
        SdrHdlList& rHdlList,
        const SdrObject* pSdrHdlObject,
        const basegfx::B2DPolyPolygon& rWireframePoly);
};

////////////////////////////////////////////////////////////////////////////////////////////////////

class ImpEdgeHdl: public SdrHdl
{
private:
    SdrEdgeLineCode eLineCode;

protected:
    // create marker for this kind
    virtual void CreateB2dIAObject(::sdr::overlay::OverlayManager& rOverlayManager);

    virtual ~ImpEdgeHdl();

public:
    ImpEdgeHdl(
        SdrHdlList& rHdlList,
        const SdrObject& rSdrHdlObject,
        SdrHdlKind eNewKind,
        const basegfx::B2DPoint& rPnt)
    :   SdrHdl(rHdlList, &rSdrHdlObject, eNewKind, rPnt),
        eLineCode(MIDDLELINE)
    {
    }

    void SetLineCode(SdrEdgeLineCode eCode);
    SdrEdgeLineCode GetLineCode() const     { return eLineCode; }
    bool IsHorzDrag() const;
    virtual Pointer GetPointer() const;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

class ImpMeasureHdl: public SdrHdl
{
private:
protected:
    // create marker for this kind
    virtual void CreateB2dIAObject(::sdr::overlay::OverlayManager& rOverlayManager);

    virtual ~ImpMeasureHdl();

public:
    ImpMeasureHdl(
        SdrHdlList& rHdlList,
        const SdrObject& rSdrHdlObject,
        SdrHdlKind eNewKind,
        const basegfx::B2DPoint& rPnt)
    :   SdrHdl(rHdlList, &rSdrHdlObject, eNewKind, rPnt)
    {
    }

    virtual Pointer GetPointer() const;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

class ImpTextframeHdl: public SdrHdl
{
private:
    const basegfx::B2DHomMatrix     maTransformation;

protected:
    // create marker for this kind
    virtual void CreateB2dIAObject(::sdr::overlay::OverlayManager& rOverlayManager);

    virtual ~ImpTextframeHdl();

public:
    explicit ImpTextframeHdl(
        SdrHdlList& rHdlList,
        const SdrObject& rSdrHdlObject,
        const basegfx::B2DHomMatrix& rTransformation);
};

////////////////////////////////////////////////////////////////////////////////////////////////////

class SVX_DLLPUBLIC SdrCropHdl : public SdrHdl
{
private:
    // evtl. shear and rotation, equal to the object's one to allow adaption of
    // the visualization handles
    double          mfShearX;
    double          mfRotation;

public:
    SdrCropHdl(
        SdrHdlList& rHdlList,
        const SdrObject& rSdrHdlObject,
        SdrHdlKind eNewKind,
        const basegfx::B2DPoint& rPnt,
        double fShearX,
        double fRotation);

protected:
    virtual ~SdrCropHdl();

    // create marker for this kind
    virtual void CreateB2dIAObject(::sdr::overlay::OverlayManager& rOverlayManager);

    BitmapEx GetBitmapForHandle( const BitmapEx& rBitmap, sal_uInt16 nSize );

    static BitmapEx GetHandlesBitmap( bool bIsFineHdl, bool bIsHighContrast );
};

////////////////////////////////////////////////////////////////////////////////////////////////////

class SVX_DLLPUBLIC SdrCropViewHdl : public SdrHdl
{
private:
    basegfx::B2DHomMatrix       maObjectTransform;
    Graphic                     maGraphic;
    double                      mfCropLeft;
    double                      mfCropTop;
    double                      mfCropRight;
    double                      mfCropBottom;

public:
    SdrCropViewHdl(
        SdrHdlList& rHdlList,
        const SdrObject& rSdrHdlObject,
        const basegfx::B2DHomMatrix& rObjectTransform,
        const Graphic& rGraphic,
        double fCropLeft,
        double fCropTop,
        double fCropRight,
        double fCropBottom);

protected:
    // create marker for this kind
    virtual void CreateB2dIAObject(::sdr::overlay::OverlayManager& rOverlayManager);
};

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //_SVDHDL_HXX

////////////////////////////////////////////////////////////////////////////////////////////////////
//eof
