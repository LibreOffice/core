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

#ifndef _E3D_SCENE3D_HXX
#define _E3D_SCENE3D_HXX

#ifndef _CAMERA3D_HXX
#include <bf_svx/camera3d.hxx>
#endif

#ifndef _E3D_LIGHT3D_HXX
#include <bf_svx/light3d.hxx>
#endif

#ifndef _E3D_LABEL3D_HXX
#include <bf_svx/label3d.hxx>
#endif

#ifndef _B3D_BASE3D_HXX
#include <bf_goodies/base3d.hxx>
#endif

#ifndef _B3D_B3DTRANS_HXX
#include <bf_goodies/b3dtrans.hxx>
#endif

#ifndef _TOOLS_TIME_HXX
#include <tools/time.hxx>
#endif
namespace binfilter {

//************************************************************
//   Defines
//************************************************************

#define E3D_SORT_NO_SORTING                 0x00000001
#define E3D_SORT_FAST_SORTING               0x00000002
#define E3D_SORT_IN_PARENTS                 0x00000004
#define E3D_SORT_NON_POLYOBJ                0x00000008
#define E3D_SORT_LOOKUP_FIELD               0x00000010
#define E3D_SORT_TEST_LENGTH                0x00000020

/*************************************************************************
|*
|* GeoData relevant fuer Undo-Actions
|*
\************************************************************************/

class E3DSceneGeoData : public E3DObjGeoData
{
public:
    Camera3D					aCamera;
    E3dLabelList				aLabelList;

    E3DSceneGeoData() {}
};

/*************************************************************************
|*
|* Basisklasse fuer 3D-Szenen
|*
\************************************************************************/

class E3dScene : public E3dObject
{
protected:
    // Transformationen
    B3dCamera					aCameraSet;

    // Beleuchtung
    B3dLightGroup				aLightGroup;

    Camera3D					aCamera;
    E3dLabelList				aLabelList;

    long						nSaveStatus;
    long						nRestStatus;

    UINT32						nSortingMode;

    // Zeit, die der letzte Paint() benoetigte
    Time						aPaintTime;

    // Darstellungsqualitaet, wird in Base3D verwendet
    UINT8						nDisplayQuality;

    // BOOLean Flags
    unsigned					bDoubleBuffered			: 1;
    unsigned					bClipping				: 1;
    unsigned					bFitInSnapRect			: 1;
    unsigned					bDither					: 1;

    // Merker, ob die Szene urspruenglich selektiert war
    unsigned					bWasSelectedWhenCopy	: 1;

    // Flag to determine if only selected objects should be drawn
    unsigned					bDrawOnlySelected		: 1;

    // static range data
    const static sal_uInt16		mnSceneRangeData[4];
    const static sal_uInt16		mnAllRangeData[4];

    virtual void NewObjectInserted(const E3dObject* p3DObj);
    virtual void StructureChanged(const E3dObject* p3DObj);

    void RebuildLists();
    virtual void ForceDefaultAttr();

    virtual void SFX_NOTIFY(SfxBroadcaster &rBC,
                            const TypeId   &rBCType,
                            const SfxHint  &rHint,
                            const TypeId   &rHintType);


protected:
    void SetDefaultAttributes(E3dDefaultAttributes& rDefault);
    void ImpSetLightItemsFromLightGroup();
    void ImpSetSceneItemsFromCamera();

public:
    TYPEINFO();
    E3dScene();
    virtual ~E3dScene();

    // Perspective: enum ProjectionType	{ PR_PARALLEL, PR_PERSPECTIVE }
    ProjectionType GetPerspective() const
        { return (ProjectionType)((const Svx3DPerspectiveItem&)GetUnmergedItemSet().Get(SDRATTR_3DSCENE_PERSPECTIVE)).GetValue(); }

    // Distance:
    double GetDistance() const
        { return (double)((const Svx3DDistanceItem&)GetUnmergedItemSet().Get(SDRATTR_3DSCENE_DISTANCE)).GetValue(); }

    // Focal length: before cm, now 1/10th mm (*100)
    double GetFocalLength() const
        { return ((const Svx3DFocalLengthItem&)GetUnmergedItemSet().Get(SDRATTR_3DSCENE_FOCAL_LENGTH)).GetValue(); }

    // Two sided lighting:
    BOOL GetTwoSidedLighting() const
        { return ((const Svx3DTwoSidedLightingItem&)GetUnmergedItemSet().Get(SDRATTR_3DSCENE_TWO_SIDED_LIGHTING)).GetValue(); }

    // Lightcolor:
    Color GetLightColor1() const
        { return ((const Svx3DLightcolor1Item&)GetUnmergedItemSet().Get(SDRATTR_3DSCENE_LIGHTCOLOR_1)).GetValue(); }
    Color GetLightColor2() const
        { return ((const Svx3DLightcolor2Item&)GetUnmergedItemSet().Get(SDRATTR_3DSCENE_LIGHTCOLOR_2)).GetValue(); }
    Color GetLightColor3() const
        { return ((const Svx3DLightcolor3Item&)GetUnmergedItemSet().Get(SDRATTR_3DSCENE_LIGHTCOLOR_3)).GetValue(); }
    Color GetLightColor4() const
        { return ((const Svx3DLightcolor4Item&)GetUnmergedItemSet().Get(SDRATTR_3DSCENE_LIGHTCOLOR_4)).GetValue(); }
    Color GetLightColor5() const
        { return ((const Svx3DLightcolor5Item&)GetUnmergedItemSet().Get(SDRATTR_3DSCENE_LIGHTCOLOR_5)).GetValue(); }
    Color GetLightColor6() const
        { return ((const Svx3DLightcolor6Item&)GetUnmergedItemSet().Get(SDRATTR_3DSCENE_LIGHTCOLOR_6)).GetValue(); }
    Color GetLightColor7() const
        { return ((const Svx3DLightcolor7Item&)GetUnmergedItemSet().Get(SDRATTR_3DSCENE_LIGHTCOLOR_7)).GetValue(); }
    Color GetLightColor8() const
        { return ((const Svx3DLightcolor8Item&)GetUnmergedItemSet().Get(SDRATTR_3DSCENE_LIGHTCOLOR_8)).GetValue(); }

    // Ambient color:
    Color GetGlobalAmbientColor() const
        { return ((const Svx3DAmbientcolorItem&)GetUnmergedItemSet().Get(SDRATTR_3DSCENE_AMBIENTCOLOR)).GetValue(); }

    // Light on/off:
    BOOL GetLightOnOff1() const
        { return ((const Svx3DLightOnOff1Item&)GetUnmergedItemSet().Get(SDRATTR_3DSCENE_LIGHTON_1)).GetValue(); }
    BOOL GetLightOnOff2() const
        { return ((const Svx3DLightOnOff2Item&)GetUnmergedItemSet().Get(SDRATTR_3DSCENE_LIGHTON_2)).GetValue(); }
    BOOL GetLightOnOff3() const
        { return ((const Svx3DLightOnOff3Item&)GetUnmergedItemSet().Get(SDRATTR_3DSCENE_LIGHTON_3)).GetValue(); }
    BOOL GetLightOnOff4() const
        { return ((const Svx3DLightOnOff4Item&)GetUnmergedItemSet().Get(SDRATTR_3DSCENE_LIGHTON_4)).GetValue(); }
    BOOL GetLightOnOff5() const
        { return ((const Svx3DLightOnOff5Item&)GetUnmergedItemSet().Get(SDRATTR_3DSCENE_LIGHTON_5)).GetValue(); }
    BOOL GetLightOnOff6() const
        { return ((const Svx3DLightOnOff6Item&)GetUnmergedItemSet().Get(SDRATTR_3DSCENE_LIGHTON_6)).GetValue(); }
    BOOL GetLightOnOff7() const
        { return ((const Svx3DLightOnOff7Item&)GetUnmergedItemSet().Get(SDRATTR_3DSCENE_LIGHTON_7)).GetValue(); }
    BOOL GetLightOnOff8() const
        { return ((const Svx3DLightOnOff8Item&)GetUnmergedItemSet().Get(SDRATTR_3DSCENE_LIGHTON_8)).GetValue(); }

    // Light direction:
    Vector3D GetLightDirection1() const
        { return ((const Svx3DLightDirection1Item&)GetUnmergedItemSet().Get(SDRATTR_3DSCENE_LIGHTDIRECTION_1)).GetValue(); }
    Vector3D GetLightDirection2() const
        { return ((const Svx3DLightDirection2Item&)GetUnmergedItemSet().Get(SDRATTR_3DSCENE_LIGHTDIRECTION_2)).GetValue(); }
    Vector3D GetLightDirection3() const
        { return ((const Svx3DLightDirection3Item&)GetUnmergedItemSet().Get(SDRATTR_3DSCENE_LIGHTDIRECTION_3)).GetValue(); }
    Vector3D GetLightDirection4() const
        { return ((const Svx3DLightDirection4Item&)GetUnmergedItemSet().Get(SDRATTR_3DSCENE_LIGHTDIRECTION_4)).GetValue(); }
    Vector3D GetLightDirection5() const
        { return ((const Svx3DLightDirection5Item&)GetUnmergedItemSet().Get(SDRATTR_3DSCENE_LIGHTDIRECTION_5)).GetValue(); }
    Vector3D GetLightDirection6() const
        { return ((const Svx3DLightDirection6Item&)GetUnmergedItemSet().Get(SDRATTR_3DSCENE_LIGHTDIRECTION_6)).GetValue(); }
    Vector3D GetLightDirection7() const
        { return ((const Svx3DLightDirection7Item&)GetUnmergedItemSet().Get(SDRATTR_3DSCENE_LIGHTDIRECTION_7)).GetValue(); }
    Vector3D GetLightDirection8() const
        { return ((const Svx3DLightDirection8Item&)GetUnmergedItemSet().Get(SDRATTR_3DSCENE_LIGHTDIRECTION_8)).GetValue(); }

    // ShadowSlant:
    sal_uInt16 GetShadowSlant() const
        { return ((const Svx3DShadowSlantItem&)GetUnmergedItemSet().Get(SDRATTR_3DSCENE_SHADOW_SLANT)).GetValue(); }

    // ShadeMode: 0 == FLAT, 1 == PHONG, 2 == SMOOTH, 3 == ForceDraft
    sal_uInt16 GetShadeMode() const
        { return ((const Svx3DShadeModeItem&)GetUnmergedItemSet().Get(SDRATTR_3DSCENE_SHADE_MODE)).GetValue(); }

    // set flag to draw only selected
    void SetDrawOnlySelected(BOOL bNew) { bDrawOnlySelected = bNew; }
    BOOL DoDrawOnlySelected() const { return bDrawOnlySelected; }

    virtual Volume3D FitInSnapRect();



    virtual void	NbcSetSnapRect(const Rectangle& rRect);
    virtual void	NbcResize(const Point& rRef, const Fraction& rXFact,
                                                 const Fraction& rYFact);
    virtual void	RecalcSnapRect();

    // Paint MUSS ueberladen werden
    virtual FASTBOOL Paint(ExtOutputDevice&, const SdrPaintInfoRec&) const = 0;

    virtual E3dScene* GetScene() const;

    // TransformationSet vorbereiten
    void InitTransformationSet();

    Time GetLastPaintTime() { return aPaintTime; }

    // Darstellungsqualitaet
    void SetDisplayQuality(UINT8 nNew) { nDisplayQuality = nNew; }
    UINT8 GetDisplayQuality() { return nDisplayQuality; }

    void SetCamera(const Camera3D& rNewCamera);
    const Camera3D& GetCamera() const { return aCamera; }

    FASTBOOL IsDoubleBuffered() const { return bDoubleBuffered; }

    // Clipping auf umschliessendes Rechteck der Szene
    // (Double Buffering ist immer geclipt!)
    FASTBOOL IsClipping() const { return bClipping; }

    void CorrectSceneDimensions();
    FASTBOOL IsFitInSnapRect() const { return bFitInSnapRect; }

    void FitSnapRectToBoundVol();
    virtual void WriteData(SvStream& rOut) const;
    virtual void ReadData(const SdrObjIOHeader& rHead, SvStream& rIn);


    virtual SdrObjGeoData *NewGeoData() const;
    virtual void          SaveGeoData(SdrObjGeoData& rGeo) const;
    virtual void          RestGeoData(const SdrObjGeoData& rGeo);

    // ItemSet access
    virtual const SfxItemSet& GetItemSet() const;
    virtual void ItemSetChanged(const SfxItemSet& rSet);

    // private support routines for ItemSet access. NULL pointer means clear item.
    virtual void ItemChange(const sal_uInt16 nWhich, const SfxPoolItem* pNewItem = 0);
    virtual void PostItemChange(const sal_uInt16 nWhich);

    // pre- and postprocessing for objects for saving
    virtual void PreSave();
    virtual void PostSave();

    virtual void NbcSetStyleSheet(SfxStyleSheet* pNewStyleSheet, FASTBOOL bDontRemoveHardAttr);
    virtual SfxStyleSheet* GetStyleSheet() const;


    // TakeObjName...() ist fuer die Anzeige in der UI, z.B. "3 Rahmen selektiert".

    void   SetSortingMode(UINT32 nMode) {nSortingMode = nMode;}
    UINT32 GetSortingMode() {return nSortingMode;}

    // Transformationen rausgeben
    B3dCamera& GetCameraSet() { return aCameraSet; }

    // Beleuchtung rausgeben
    B3dLightGroup& GetLightGroup() { return aLightGroup; }

    // LightGroup aus geladenen PointObj's fuellen
    void FillLightGroup();

    // Licht-Objekte rauswerfen
    void RemoveLightObjects();

    // Licht-Objekte erzeugen, um kompatibel zur 4.0
    // speichern zu koennen
    void CreateLightObjectsFromLightGroup();

    // Lichter zaehlen
    UINT16 CountNumberOfLights();

    // Dithering
    BOOL GetDither() { return bDither; }
    void SetDither(BOOL bNew) { bDither = bNew; }

    // Marker fuer WasSelectedWhenCopy
    BOOL GetWasSelectedWhenCopy() { return bWasSelectedWhenCopy; }
    void SetWasSelectedWhenCopy(BOOL bNew) { bWasSelectedWhenCopy = bNew; }

    // Aufbrechen

    // ItemPool fuer dieses Objekt wechseln
    virtual void MigrateItemPool(SfxItemPool* pSrcPool, SfxItemPool* pDestPool, SdrModel* pNewModel = NULL );

    void SetShadowPlaneDirection(const Vector3D& rVec);
};

}//end of namespace binfilter
#endif			// _E3D_SCENE3D_HXX
