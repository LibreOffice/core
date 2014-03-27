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

#ifndef INCLUDED_SVX_SVDHDL_HXX
#define INCLUDED_SVX_SVDHDL_HXX

#include <tools/gen.hxx>

#include <vcl/pointr.hxx>

#include <svl/solar.hrc>

#include <svx/xpoly.hxx>
#include <svx/svdoedge.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <svx/sdgcpitm.hxx>
#include <svx/sdr/overlay/overlayobjectlist.hxx>
#include <svx/svxdllapi.h>

class VirtualDevice;
class OutputDevice;
class Region;
class Window;
class SdrHdlList;
class SdrMarkView;
class SdrObject;
class SdrPageView;
class MouseEvent;



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
    HDL_GLUE_DESELECTED,       // GluePoint
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



enum BitmapColorIndex
{
    LightGreen,
    Cyan,
    LightCyan,
    Red,
    LightRed,
    Yellow
};



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
    Glue_Deselected,
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



class SVX_DLLPUBLIC SdrHdl
{
    friend class                SdrMarkView; // fuer den Zugriff auf nObjHdlNum
    friend class                SdrHdlList;

    static BitmapEx ImpGetBitmapEx(BitmapMarkerKind eKindOfMarker, sal_uInt16 nInd);

protected:
    SdrObject*                  pObj;      // Gehoert das Handle zu einem Objekt?
    SdrPageView*                pPV;       // Gehoert das Handle zu einem Objekt in einer bestimmten PageView?
    SdrHdlList*                 pHdlList;  // Zum Feststelen der Handlegroesse

    // OVERLAYMANAGER
    ::sdr::overlay::OverlayObjectList           maOverlayGroup;

    Point                       aPos;

    SdrHdlKind                  eKind;

    long                        nDrehWink; // Handles bzw. Mauszeiger drehen
    sal_uInt32                  nObjHdlNum; // wird von der MarkView benoetigt
    sal_uInt32                  nPolyNum;  // Polygonpunktes
    sal_uInt32                  nPPntNum;  // Punktnummer des Polygons
    sal_uInt32                  nSourceHdlNum; // ist noch vollstaendig zu implementieren

    bool                        bSelect : 1;   // Ein selektierter Polygonpunkt?
    bool                        b1PixMore : 1; // True=Handle wird 1 Pixel groesser dargestellt
    bool                        bPlusHdl : 1;  // u.a. fuer Hld-Paint Optimierung bei MarkPoint/UnmarkPoint, ...

    bool                        mbMoveOutside; // forces this handle to be moved outside of the selection rectangle

    // create marker for this kind
    virtual void CreateB2dIAObject();

    // cleanup marker if one exists
    void GetRidOfIAObject();

private:
    bool                        mbMouseOver;    // is true if the mouse is over this handle

protected:
    ::sdr::overlay::OverlayObject* CreateOverlayObject(
        const basegfx::B2DPoint& rPos,
        BitmapColorIndex eColIndex, BitmapMarkerKind eKindOfMarker, Point aMoveOutsideOffset = Point());
    BitmapMarkerKind GetNextBigger(BitmapMarkerKind eKnd) const;

public:
    SdrHdl();
    explicit SdrHdl(const Point& rPnt, SdrHdlKind eNewKind=HDL_MOVE);
    virtual ~SdrHdl();

    const ::sdr::overlay::OverlayObjectList& getOverlayObjectList() const { return maOverlayGroup; }

    void SetHdlList(SdrHdlList* pList);
    SdrHdlKind GetKind() const { return eKind; }
    void Touch();

    const Point& GetPos() const { return aPos; }
    void SetPos(const Point& rPnt);

    SdrPageView* GetPageView() const { return pPV; }
    void SetPageView(SdrPageView* pNewPV) { pPV=pNewPV; }

    SdrObject* GetObj() const { return pObj;  }
    void SetObj(SdrObject* pNewObj);

    bool IsSelected() const { return bSelect; }
    void SetSelected(bool bJa=true);

    void Set1PixMore(bool bJa=true);
    void SetDrehWink(long n);

    bool IsCornerHdl() const { return eKind==HDL_UPLFT || eKind==HDL_UPRGT || eKind==HDL_LWLFT || eKind==HDL_LWRGT; }
    bool IsVertexHdl() const { return eKind==HDL_UPPER || eKind==HDL_LOWER || eKind==HDL_LEFT  || eKind==HDL_RIGHT; }

    void SetObjHdlNum(sal_uInt32 nNum) { nObjHdlNum=nNum; }
    sal_uInt32 GetObjHdlNum() const { return nObjHdlNum; }

    void SetPolyNum(sal_uInt32 nNum) { nPolyNum=nNum; }
    sal_uInt32 GetPolyNum() const { return nPolyNum; }

    void SetPointNum(sal_uInt32 nNum) { nPPntNum=nNum; }
    sal_uInt32 GetPointNum() const { return nPPntNum; }

    void SetPlusHdl(bool bOn) { bPlusHdl=bOn; }
    bool IsPlusHdl() const { return bPlusHdl; }

    void SetSourceHdlNum(sal_uInt32 nNum) { nSourceHdlNum=nNum; }
    sal_uInt32 GetSourceHdlNum() const { return nSourceHdlNum; }

    virtual Pointer GetPointer() const;
    bool IsHdlHit(const Point& rPnt) const;

    // #97016# II
    virtual bool IsFocusHdl() const;

    void SetMoveOutside( bool bMoveOutside );

    /** is called when the mouse enters the area of this handle. If the handle changes his
        visualisation during mouse over it must override this method and call Touch(). */
    virtual void onMouseEnter(const MouseEvent& rMEvt);

    /** is called when the mouse leaves the area of this handle. If the handle changes his
        visualisation during mouse over it must override this method and call Touch(). */
    virtual void onMouseLeave();

    static BitmapEx createGluePointBitmap() { return ImpGetBitmapEx(Glue_Deselected, 0); }
};



#define SDR_HANDLE_COLOR_SIZE_NORMAL            Size(13, 13)
#define SDR_HANDLE_COLOR_SIZE_SELECTED          Size(17, 17)

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

    // create marker for this kind
    SVX_DLLPRIVATE virtual void CreateB2dIAObject() SAL_OVERRIDE;

    // help functions
    SVX_DLLPRIVATE Bitmap CreateColorDropper(Color aCol);
    SVX_DLLPRIVATE Color GetLuminance(const Color& rCol);
    SVX_DLLPRIVATE void CallColorChangeLink();

public:
    explicit SdrHdlColor(const Point& rRef, Color aCol = Color(COL_BLACK), const Size& rSize = Size(11, 11), bool bLum = false);
    virtual ~SdrHdlColor();

    bool IsUseLuminance() const { return bUseLuminance; }

    Color GetColor() const { return aMarkerColor; }
    void SetColor(Color aNew, bool bCallLink = false);

    const Size& GetSize() const { return aMarkerSize; }
    void SetSize(const Size& rNew);

    void SetColorChangeHdl(const Link& rLink) { aColorChangeHdl = rLink; }
    const Link& GetColorChangeHdl() const { return aColorChangeHdl; }
};



class SdrHdlGradient : public SdrHdl
{
private:
    // pointer to used color handles
    SdrHdlColor*                pColHdl1;
    SdrHdlColor*                pColHdl2;

    // 2nd position
    Point                       a2ndPos;

    // is this a gradient or a transparence
    bool                        bGradient : 1;

    // select which handle to move
    bool                        bMoveSingleHandle : 1;
    bool                        bMoveFirstHandle : 1;

    // create marker for this kind
    virtual void CreateB2dIAObject() SAL_OVERRIDE;

public:
    SdrHdlGradient(const Point& rRef1, const Point& rRef2, bool bGrad = true);
    virtual ~SdrHdlGradient();

    bool IsGradient() const { return bGradient; }

    // set the associated color handles
    void SetColorHandles(SdrHdlColor* pL1, SdrHdlColor* pL2) { pColHdl1 = pL1; pColHdl2 = pL2; }
    SdrHdlColor* GetColorHdl1() const { return pColHdl1; }
    SdrHdlColor* GetColorHdl2() const { return pColHdl2; }

    const Point& Get2ndPos() const { return a2ndPos; }
    void Set2ndPos(const Point& rPnt);

    // the link called by the color handles
    DECL_LINK(ColorChangeHdl, SdrHdl*);

    // transformation call, create gradient from this handle
    void FromIAOToItem(SdrObject* pObj, bool bSetItemOnObject, bool bUndo);

    // selection flags for interaction
    bool IsMoveSingleHandle() const { return bMoveSingleHandle; }
    void SetMoveSingleHandle(bool bNew) { bMoveSingleHandle = bNew; }
    bool IsMoveFirstHandle() const { return bMoveFirstHandle; }
    void SetMoveFirstHandle(bool bNew) { bMoveFirstHandle = bNew; }
};



// Spiegelachse
class SdrHdlLine: public SdrHdl
{
    // create marker for this kind
    virtual void CreateB2dIAObject() SAL_OVERRIDE;

protected:
    SdrHdl*                     pHdl1;
    SdrHdl*                     pHdl2;

public:
    SdrHdlLine(SdrHdl& rHdl1, SdrHdl& rHdl2, SdrHdlKind eNewKind=HDL_MIRX) { eKind=eNewKind; pHdl1=&rHdl1; pHdl2=&rHdl2; }
    virtual ~SdrHdlLine();

    virtual Pointer GetPointer() const SAL_OVERRIDE;
};

// Ein SdrHdlBezWgt hat Kenntnis von seinem "BasisHandle". Seine Draw-Methode
// zeichnet zusaetzlich eine Linie von seiner Position zur Position dieses
// BasisHandles.
class SdrHdlBezWgt: public SdrHdl
{
    // create marker for this kind
    virtual void CreateB2dIAObject() SAL_OVERRIDE;

protected:
    const SdrHdl* pHdl1;

public:
    // das ist kein Copy-Ctor!!!
    SdrHdlBezWgt(const SdrHdl* pRefHdl1, SdrHdlKind eNewKind=HDL_BWGT) { eKind=eNewKind; pHdl1=pRefHdl1; }
    virtual ~SdrHdlBezWgt();
};



class E3dVolumeMarker : public SdrHdl
{
    basegfx::B2DPolyPolygon             aWireframePoly;

    // create marker for this kind
    virtual void CreateB2dIAObject() SAL_OVERRIDE;

public:
    explicit E3dVolumeMarker(const basegfx::B2DPolyPolygon& rWireframePoly);
};



class ImpEdgeHdl: public SdrHdl
{
    SdrEdgeLineCode eLineCode;

    // create marker for this kind
    virtual void CreateB2dIAObject() SAL_OVERRIDE;

public:
    ImpEdgeHdl(const Point& rPnt, SdrHdlKind eNewKind): SdrHdl(rPnt,eNewKind),eLineCode(MIDDLELINE) {}
    virtual ~ImpEdgeHdl();

    void SetLineCode(SdrEdgeLineCode eCode);
    SdrEdgeLineCode GetLineCode() const     { return eLineCode; }
    bool IsHorzDrag() const;
    virtual Pointer GetPointer() const SAL_OVERRIDE;
};



class ImpMeasureHdl: public SdrHdl
{
    // create marker for this kind
    virtual void CreateB2dIAObject() SAL_OVERRIDE;

public:
    ImpMeasureHdl(const Point& rPnt, SdrHdlKind eNewKind): SdrHdl(rPnt,eNewKind) {}
    virtual ~ImpMeasureHdl();

    virtual Pointer GetPointer() const SAL_OVERRIDE;
};



class ImpTextframeHdl: public SdrHdl
{
    const Rectangle maRect;

    // create marker for this kind
    virtual void CreateB2dIAObject() SAL_OVERRIDE;

public:
    explicit ImpTextframeHdl(const Rectangle& rRect);
};





class SVX_DLLPUBLIC SdrHdlList
{
protected:
    sal_uIntPtr                 mnFocusIndex;
    SdrMarkView*                pView;
    std::deque<SdrHdl*>         aList;
    sal_uInt16                  nHdlSize;

    bool                        bRotateShear : 1;
    bool                        bDistortShear : 1;
    bool                        bMoveOutside : 1;      // Handles nach aussen ruecken (fuer TextEdit)

private:
    SVX_DLLPRIVATE SdrHdlList(const SdrHdlList&): aList()  {}
    SVX_DLLPRIVATE void operator=(const SdrHdlList&)                 {}
    SVX_DLLPRIVATE bool operator==(const SdrHdlList&) const      { return false; }
    SVX_DLLPRIVATE bool operator!=(const SdrHdlList&) const      { return false; }

public:
    explicit SdrHdlList(SdrMarkView* pV);
    ~SdrHdlList();
    void Clear();

    // #97016# II
    void TravelFocusHdl(bool bForward);
    SdrHdl* GetFocusHdl() const;
    void SetFocusHdl(SdrHdl* pNew);
    void ResetFocusHdl();

    // Access to View
    SdrMarkView* GetView() const;

    // Sortierung: 1.Level Erst Refpunkt-Handles, dann normale Handles, dann Glue, dann User, dann Plushandles
    //             2.Level PageView (Pointer)
    //             3.Level Position (x+y)
    void     Sort();
    sal_uIntPtr    GetHdlCount() const                       { return aList.size(); }
    SdrHdl*  GetHdl(sal_uIntPtr nNum) const                  { return nNum != CONTAINER_ENTRY_NOTFOUND ? aList[nNum] : NULL; }
    sal_uIntPtr    GetHdlNum(const SdrHdl* pHdl) const;
    void     SetHdlSize(sal_uInt16 nSiz);
    sal_uInt16   GetHdlSize() const                        { return nHdlSize; }
    void     SetMoveOutside(bool bOn);
    bool IsMoveOutside() const                     { return bMoveOutside; }
    void     SetRotateShear(bool bOn);
    bool IsRotateShear() const                     { return bRotateShear; }
    void     SetDistortShear(bool bOn);
    bool IsDistortShear() const                    { return bDistortShear; }

    // AddHdl uebernimmt das Handle in sein Eigentum. Es muss
    // also auf dem Heap stehen, da Clear() ein delete macht.
    void    AddHdl(SdrHdl* pHdl, bool bAtBegin=false);
    SdrHdl* RemoveHdl(sal_uIntPtr nNum);
    void RemoveAllByKind(SdrHdlKind eKind);

    // Zuletzt eingefuegte Handles werden am ehesten getroffen
    // (wenn Handles uebereinander liegen).
    SdrHdl* IsHdlListHit(const Point& rPnt, bool bBack=false, bool bNext=false, SdrHdl* pHdl0=NULL) const;
    SdrHdl* GetHdl(SdrHdlKind eKind1) const;
};



class SVX_DLLPUBLIC SdrCropHdl : public SdrHdl
{
private:
    // evtl. shear and rotation, equal to the object's one to allow adaption of
    // the visualization handles
    double          mfShearX;
    double          mfRotation;

public:
    SdrCropHdl(
        const Point& rPnt,
        SdrHdlKind eNewKind,
        double fShearX,
        double fRotation);

protected:
    // create marker for this kind
    virtual void CreateB2dIAObject() SAL_OVERRIDE;

    BitmapEx GetBitmapForHandle( const BitmapEx& rBitmap, int nSize );

    static BitmapEx GetHandlesBitmap();
};



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
        const basegfx::B2DHomMatrix& rObjectTransform,
        const Graphic& rGraphic,
        double fCropLeft,
        double fCropTop,
        double fCropRight,
        double fCropBottom);

protected:
    // create marker for this kind
    virtual void CreateB2dIAObject() SAL_OVERRIDE;
};



#endif // INCLUDED_SVX_SVDHDL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
