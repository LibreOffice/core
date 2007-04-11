/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: scene3d.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 16:07:03 $
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

#ifndef _E3D_SCENE3D_HXX
#define _E3D_SCENE3D_HXX

#ifndef _CAMERA3D_HXX
#include <svx/camera3d.hxx>
#endif

#ifndef _E3D_LABEL3D_HXX
#include <svx/label3d.hxx>
#endif

#ifndef _B3D_BASE3D_HXX
#include <goodies/base3d.hxx>
#endif

#ifndef _B3D_B3DTRANS_HXX
#include <goodies/b3dtrans.hxx>
#endif

#ifndef _TOOLS_TIME_HXX
#include <tools/time.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

namespace sdr
{
    namespace properties
    {
        class BaseProperties;
        class E3dSceneProperties;
    } // end of namespace properties
} // end of namespace sdr

/*************************************************************************
|*
|* GeoData relevant fuer Undo-Actions
|*
\************************************************************************/

class E3DSceneGeoData : public E3DObjGeoData
{
public:
    Camera3D                    aCamera;
    E3dLabelList                aLabelList;

    E3DSceneGeoData() {}
};

// #110988#
class Imp3DDepthRemapper;

/*************************************************************************
|*
|* Basisklasse fuer 3D-Szenen
|*
\************************************************************************/

class SVX_DLLPUBLIC E3dScene : public E3dObject
{
    // BaseProperties section
    virtual sdr::properties::BaseProperties* CreateObjectSpecificProperties();

    // to allow sdr::properties::E3dSceneProperties access to StructureChanged()
    friend class sdr::properties::E3dSceneProperties;

    // #110094# DrawContact section
private:
    virtual sdr::contact::ViewContact* CreateObjectSpecificViewContact();

protected:
    // Transformationen
    B3dCamera                   aCameraSet;

    // Beleuchtung
    B3dLightGroup               aLightGroup;

    Camera3D                    aCamera;
    E3dLabelList                aLabelList;

    // Zeit, die der letzte Paint() benoetigte
    Time                        aPaintTime;

    // Darstellungsqualitaet, wird in Base3D verwendet
    UINT8                       nDisplayQuality;

    // #110988#
    Imp3DDepthRemapper*         mp3DDepthRemapper;

    // BOOLean Flags
    unsigned                    bDoubleBuffered         : 1;
    unsigned                    bClipping               : 1;
    unsigned                    bFitInSnapRect          : 1;
    unsigned                    bDither                 : 1;

    // Merker, ob die Szene urspruenglich selektiert war
    unsigned                    bWasSelectedWhenCopy    : 1;

    // Flag to determine if only selected objects should be drawn
    unsigned                    bDrawOnlySelected       : 1;

    virtual void NewObjectInserted(const E3dObject* p3DObj);
    virtual void StructureChanged(const E3dObject* p3DObj);

    void RebuildLists();

    virtual void SFX_NOTIFY(SfxBroadcaster &rBC,
                            const TypeId   &rBCType,
                            const SfxHint  &rHint,
                            const TypeId   &rHintType);

    BOOL AreThereTransparentParts() const;

protected:
    void SetDefaultAttributes(E3dDefaultAttributes& rDefault);

    // #110988#
    void ImpCleanup3DDepthMapper();

public:
    TYPEINFO();
    E3dScene();
    E3dScene(E3dDefaultAttributes& rDefault);
    virtual ~E3dScene();

    // #110988#
    sal_uInt32 RemapOrdNum(sal_uInt32 nOrdNum) const;

    // Perspective: enum ProjectionType { PR_PARALLEL, PR_PERSPECTIVE }
    ProjectionType GetPerspective() const
        { return (ProjectionType)((const Svx3DPerspectiveItem&)GetObjectItemSet().Get(SDRATTR_3DSCENE_PERSPECTIVE)).GetValue(); }

    // Distance:
    double GetDistance() const
        { return (double)((const Svx3DDistanceItem&)GetObjectItemSet().Get(SDRATTR_3DSCENE_DISTANCE)).GetValue(); }

    // Focal length: before cm, now 1/10th mm (*100)
    double GetFocalLength() const
        { return ((const Svx3DFocalLengthItem&)GetObjectItemSet().Get(SDRATTR_3DSCENE_FOCAL_LENGTH)).GetValue(); }

    // Two sided lighting:
    BOOL GetTwoSidedLighting() const
        { return ((const Svx3DTwoSidedLightingItem&)GetObjectItemSet().Get(SDRATTR_3DSCENE_TWO_SIDED_LIGHTING)).GetValue(); }

    // Lightcolor:
    Color GetLightColor1() const
        { return ((const Svx3DLightcolor1Item&)GetObjectItemSet().Get(SDRATTR_3DSCENE_LIGHTCOLOR_1)).GetValue(); }
    Color GetLightColor2() const
        { return ((const Svx3DLightcolor2Item&)GetObjectItemSet().Get(SDRATTR_3DSCENE_LIGHTCOLOR_2)).GetValue(); }
    Color GetLightColor3() const
        { return ((const Svx3DLightcolor3Item&)GetObjectItemSet().Get(SDRATTR_3DSCENE_LIGHTCOLOR_3)).GetValue(); }
    Color GetLightColor4() const
        { return ((const Svx3DLightcolor4Item&)GetObjectItemSet().Get(SDRATTR_3DSCENE_LIGHTCOLOR_4)).GetValue(); }
    Color GetLightColor5() const
        { return ((const Svx3DLightcolor5Item&)GetObjectItemSet().Get(SDRATTR_3DSCENE_LIGHTCOLOR_5)).GetValue(); }
    Color GetLightColor6() const
        { return ((const Svx3DLightcolor6Item&)GetObjectItemSet().Get(SDRATTR_3DSCENE_LIGHTCOLOR_6)).GetValue(); }
    Color GetLightColor7() const
        { return ((const Svx3DLightcolor7Item&)GetObjectItemSet().Get(SDRATTR_3DSCENE_LIGHTCOLOR_7)).GetValue(); }
    Color GetLightColor8() const
        { return ((const Svx3DLightcolor8Item&)GetObjectItemSet().Get(SDRATTR_3DSCENE_LIGHTCOLOR_8)).GetValue(); }

    // Ambient color:
    Color GetGlobalAmbientColor() const
        { return ((const Svx3DAmbientcolorItem&)GetObjectItemSet().Get(SDRATTR_3DSCENE_AMBIENTCOLOR)).GetValue(); }

    // Light on/off:
    BOOL GetLightOnOff1() const
        { return ((const Svx3DLightOnOff1Item&)GetObjectItemSet().Get(SDRATTR_3DSCENE_LIGHTON_1)).GetValue(); }
    BOOL GetLightOnOff2() const
        { return ((const Svx3DLightOnOff2Item&)GetObjectItemSet().Get(SDRATTR_3DSCENE_LIGHTON_2)).GetValue(); }
    BOOL GetLightOnOff3() const
        { return ((const Svx3DLightOnOff3Item&)GetObjectItemSet().Get(SDRATTR_3DSCENE_LIGHTON_3)).GetValue(); }
    BOOL GetLightOnOff4() const
        { return ((const Svx3DLightOnOff4Item&)GetObjectItemSet().Get(SDRATTR_3DSCENE_LIGHTON_4)).GetValue(); }
    BOOL GetLightOnOff5() const
        { return ((const Svx3DLightOnOff5Item&)GetObjectItemSet().Get(SDRATTR_3DSCENE_LIGHTON_5)).GetValue(); }
    BOOL GetLightOnOff6() const
        { return ((const Svx3DLightOnOff6Item&)GetObjectItemSet().Get(SDRATTR_3DSCENE_LIGHTON_6)).GetValue(); }
    BOOL GetLightOnOff7() const
        { return ((const Svx3DLightOnOff7Item&)GetObjectItemSet().Get(SDRATTR_3DSCENE_LIGHTON_7)).GetValue(); }
    BOOL GetLightOnOff8() const
        { return ((const Svx3DLightOnOff8Item&)GetObjectItemSet().Get(SDRATTR_3DSCENE_LIGHTON_8)).GetValue(); }

    // Light direction:
    basegfx::B3DVector GetLightDirection1() const
        { return ((const Svx3DLightDirection1Item&)GetObjectItemSet().Get(SDRATTR_3DSCENE_LIGHTDIRECTION_1)).GetValue(); }
    basegfx::B3DVector GetLightDirection2() const
        { return ((const Svx3DLightDirection2Item&)GetObjectItemSet().Get(SDRATTR_3DSCENE_LIGHTDIRECTION_2)).GetValue(); }
    basegfx::B3DVector GetLightDirection3() const
        { return ((const Svx3DLightDirection3Item&)GetObjectItemSet().Get(SDRATTR_3DSCENE_LIGHTDIRECTION_3)).GetValue(); }
    basegfx::B3DVector GetLightDirection4() const
        { return ((const Svx3DLightDirection4Item&)GetObjectItemSet().Get(SDRATTR_3DSCENE_LIGHTDIRECTION_4)).GetValue(); }
    basegfx::B3DVector GetLightDirection5() const
        { return ((const Svx3DLightDirection5Item&)GetObjectItemSet().Get(SDRATTR_3DSCENE_LIGHTDIRECTION_5)).GetValue(); }
    basegfx::B3DVector GetLightDirection6() const
        { return ((const Svx3DLightDirection6Item&)GetObjectItemSet().Get(SDRATTR_3DSCENE_LIGHTDIRECTION_6)).GetValue(); }
    basegfx::B3DVector GetLightDirection7() const
        { return ((const Svx3DLightDirection7Item&)GetObjectItemSet().Get(SDRATTR_3DSCENE_LIGHTDIRECTION_7)).GetValue(); }
    basegfx::B3DVector GetLightDirection8() const
        { return ((const Svx3DLightDirection8Item&)GetObjectItemSet().Get(SDRATTR_3DSCENE_LIGHTDIRECTION_8)).GetValue(); }

    // ShadowSlant:
    sal_uInt16 GetShadowSlant() const
        { return ((const Svx3DShadowSlantItem&)GetObjectItemSet().Get(SDRATTR_3DSCENE_SHADOW_SLANT)).GetValue(); }

    // ShadeMode: 0 == FLAT, 1 == PHONG, 2 == SMOOTH, 3 == ForceDraft
    sal_uInt16 GetShadeMode() const
        { return ((const Svx3DShadeModeItem&)GetObjectItemSet().Get(SDRATTR_3DSCENE_SHADE_MODE)).GetValue(); }

    // set flag to draw only selected
    void SetDrawOnlySelected(BOOL bNew) { bDrawOnlySelected = bNew; }
    BOOL DoDrawOnlySelected() const { return bDrawOnlySelected; }

    virtual basegfx::B3DRange FitInSnapRect();

    virtual UINT16 GetObjIdentifier() const;

    virtual sal_uInt32 GetHdlCount() const;
    virtual void    AddToHdlList(SdrHdlList& rHdlList) const;
    virtual FASTBOOL HasSpecialDrag() const;

    virtual void    NbcSetSnapRect(const Rectangle& rRect);
    virtual void    NbcMove(const Size& rSize);
    virtual void    NbcResize(const Point& rRef, const Fraction& rXFact,
                                                 const Fraction& rYFact);
    virtual void    RecalcSnapRect();

    // Paint MUSS ueberladen werden
    virtual sal_Bool DoPaintObject(XOutputDevice&, const SdrPaintInfoRec&) const = 0;

    virtual E3dScene* GetScene() const;

    // TransformationSet vorbereiten
    void InitTransformationSet();

    Time GetLastPaintTime() { return aPaintTime; }

    // Darstellungsqualitaet
    void SetDisplayQuality(UINT8 nNew) { nDisplayQuality = nNew; }
    UINT8 GetDisplayQuality() { return nDisplayQuality; }

    void SetCamera(const Camera3D& rNewCamera);
    const Camera3D& GetCamera() const { return aCamera; }

    void SetDoubleBuffered(FASTBOOL bBuff = TRUE);
    FASTBOOL IsDoubleBuffered() const { return bDoubleBuffered; }

    // Clipping auf umschliessendes Rechteck der Szene
    // (Double Buffering ist immer geclipt!)
    void SetClipping(FASTBOOL bClip = TRUE);
    FASTBOOL IsClipping() const { return bClipping; }

    void SetFitInSnapRect(FASTBOOL bFit = TRUE);
    void CorrectSceneDimensions();
    FASTBOOL IsFitInSnapRect() const { return bFitInSnapRect; }

    void FitSnapRectToBoundVol();
    virtual void operator=(const SdrObject&);

    virtual SdrObjGeoData *NewGeoData() const;
    virtual void          SaveGeoData(SdrObjGeoData& rGeo) const;
    virtual void          RestGeoData(const SdrObjGeoData& rGeo);

    virtual void NbcRotate(const Point& rRef, long nWink, double sn, double cs);
    void RotateScene(const Point& rRef, long nWink, double sn, double cs);

    // TakeObjName...() ist fuer die Anzeige in der UI, z.B. "3 Rahmen selektiert".
    virtual void TakeObjNameSingul(String& rName) const;
    virtual void TakeObjNamePlural(String& rName) const;

    // Transformationen rausgeben
    B3dCamera& GetCameraSet() { return aCameraSet; }

    // Beleuchtung rausgeben
    B3dLightGroup& GetLightGroup() { return aLightGroup; }

    // Dithering
    BOOL GetDither() { return bDither; }
    void SetDither(BOOL bNew) { bDither = bNew; }

    // Marker fuer WasSelectedWhenCopy
    BOOL GetWasSelectedWhenCopy() { return bWasSelectedWhenCopy; }
    void SetWasSelectedWhenCopy(BOOL bNew) { bWasSelectedWhenCopy = bNew; }

    // Aufbrechen
    virtual BOOL IsBreakObjPossible();

    basegfx::B3DVector GetShadowPlaneDirection() const;
    void SetShadowPlaneDirection(const basegfx::B3DVector& rVec);

    // #115662#
    // For new chart, calculate the number of hit contained 3D objects at given point,
    // give back the count and a depth-sorted list of SdrObjects (a Vector). The vector will be
    // changed, at least cleared.
    sal_uInt32 HitTest(const Point& rHitTestPosition, ::std::vector< SdrObject* >& o_rResult);

    // Polygon das waehrend des Erzeugens aufgezogen wird
    virtual basegfx::B2DPolyPolygon TakeCreatePoly(const SdrDragStat& rDrag) const;

    // create moves
    virtual FASTBOOL BegCreate(SdrDragStat& rStat);
    virtual FASTBOOL MovCreate(SdrDragStat& rStat); // TRUE=Xor muss repainted werden
    virtual FASTBOOL EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd);
    virtual FASTBOOL BckCreate(SdrDragStat& rStat);
    virtual void BrkCreate(SdrDragStat& rStat);
};

#endif          // _E3D_SCENE3D_HXX
