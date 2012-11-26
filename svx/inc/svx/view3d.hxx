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



#ifndef _E3D_VIEW3D_HXX
#define _E3D_VIEW3D_HXX

#include <svx/svdview.hxx>
#include <svx/def3d.hxx>
#include <svx/deflt3d.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include "svx/svxdllapi.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
//   Vorausdeklarationen

class E3dObject;
class E3dScene;
class SceneList;
class Impl3DMirrorConstructOverlay;

////////////////////////////////////////////////////////////////////////////////////////////////////

class SVX_DLLPUBLIC E3dView : public SdrView
{
private:
protected:
    E3dDefaultAttributes        a3DDefaultAttr;
    MouseEvent                  aMouseEvent;                    // Die Parameter der letzten Events (Mouse, Keyboard)
    Color                       aDefaultLightColor;             // dito mit den Farben
    Color                       aDefaultAmbientColor;

    double                      fDefaultScaleX;                 // Verzerrungen
    double                      fDefaultScaleY;
    double                      fDefaultScaleZ;
    double                      fDefaultRotateX;                // und Drehungen
    double                      fDefaultRotateY;
    double                      fDefaultRotateZ;
    double                      fDefaultExtrusionDeepth;        // Extrusionstiefe
    double                      fDefaultLightIntensity;         // Intensitaeten der beiden (notwendigen) Licht-
    double                      fDefaultAmbientIntensity;       // quellen
    sal_Int32                   nHDefaultSegments;              // wieviele HSegmente braucht mein Lathe-Ojekt
    sal_Int32                   nVDefaultSegments;              // wieviele VSegmente braucht mein Lathe-Ojekt

    E3dDragConstraint           eDragConstraint;

    // Migrate selections
    Impl3DMirrorConstructOverlay*                   mpMirrorOverlay;

    /// bitfield
    bool                        bDoubleSided : 1;

    void InitView();

    void ImpCreate3DObject(E3dScene* pScene, SdrObject* pObj, bool bExtrude, double fDepth, basegfx::B2DHomMatrix& rLatheMat);
    void ImpCreateSingle3DObjectFlat(E3dScene* pScene, SdrObject* pObj, bool bExtrude, double fDepth, basegfx::B2DHomMatrix& rLatheMat);
    void ImpChangeSomeAttributesFor3DConversion(SdrObject* pObj);
    void ImpChangeSomeAttributesFor3DConversion2(SdrObject* pObj);

    void InitScene(E3dScene* pScene, double fW, double fH, double fCamZ);
    void ImpIsConvertTo3DPossible(SdrObject* pObj, bool& rAny3D, bool& rGroupSelected) const;
    void BreakSingle3DObj(E3dObject* pObj);

public:
    E3dView(SdrModel& rModel, OutputDevice* pOut = 0);
    virtual ~E3dView();

    // Alle markierten Objekte auf dem angegebenen OutputDevice ausgeben.
    virtual void DrawMarkedObj(OutputDevice& rOut) const;

    // Zugriff auf die Default-Attribute
    const E3dDefaultAttributes& Get3DDefaultAttributes() const { return a3DDefaultAttr; }
    virtual bool BegDragObj(const basegfx::B2DPoint& rPnt, const SdrHdl* pHdl = 0, double fMinMovLogic = 3.0,
        SdrDragMethod* pForcedMeth = 0);
    virtual void CheckPossibilities();

    // Event setzen/rausruecken
    void SetMouseEvent(const MouseEvent& rNew) { aMouseEvent = rNew; }
    const MouseEvent& GetMouseEvent() { return aMouseEvent; }

    // Model holen ueberladen, da bei einzelnen 3D Objekten noch eine Szene
    // untergeschoben werden muss
    virtual SdrModel* GetMarkedObjModel() const;

    // Bei Paste muss - falls in eine Scene eingefuegt wird - die
    // Objekte der Szene eingefuegt werden, die Szene selbst aber nicht
    using SdrView::Paste;
    virtual bool Paste(const SdrModel& rMod, const basegfx::B2DPoint& rPos, SdrObjList* pLst = 0, sal_uInt32 nOptions = 0);

    // #83403# Service routine used from local Clone() and from SdrCreateView::EndCreateObj(...)
    bool ImpCloneAll3DObjectsToDestScene(E3dScene* pSrcScene, E3dScene* pDstScene);

    bool HasMarkedScene();
    E3dScene* GetMarkedScene();

    bool IsConvertTo3DObjPossible() const;
    void ConvertMarkedObjTo3D(bool bExtrude=true, basegfx::B2DPoint aPnt1 = basegfx::B2DPoint(0.0, 0.0), basegfx::B2DPoint aPnt2 = basegfx::B2DPoint(0.0, 1.0));

    // Nachtraeglichhe Korrekturmoeglichkeit um alle Extrudes in einer
    // bestimmten Tiefensortierung anzulegen
    void DoDepthArrange(E3dScene* pScene, double fDepth);
    void ConvertMarkedToPolyObj(bool bLineToArea);
    E3dScene* SetCurrent3DObj(E3dObject* p3DObj);
    void Start3DCreation();

    // migration of overlay
    sal_Bool Is3DRotationCreationActive() const { return (0L != mpMirrorOverlay); }

    virtual void MovAction(const basegfx::B2DPoint& rPnt);
    void End3DCreation(bool bUseDefaultValuesForMirrorAxes=false);
    void ResetCreationActive();

    double GetDefaultCamPosZ();

    double &DefaultScaleX ()
    {
        return fDefaultScaleX;
    }

    double DefaultScaleX () const
    {
        return fDefaultScaleX;
    }

    double &DefaultScaleY ()
    {
        return fDefaultScaleY;
    }

    double DefaultScaleY () const
    {
        return fDefaultScaleY;
    }

    double &DefaultScaleZ ()
    {
        return fDefaultScaleZ;
    }

    double DefaultScaleZ () const
    {
        return fDefaultScaleZ;
    }

    double &DefaultRotateX ()
    {
        return fDefaultRotateX;
    }

    double DefaultRotateX () const
    {
        return fDefaultRotateX;
    }

    double &DefaultRotateY ()
    {
        return fDefaultRotateY;
    }

    double DefaultRotateY () const
    {
        return fDefaultRotateY;
    }

    double &DefaultRotateZ ()
    {
        return fDefaultRotateZ;
    }

    double DefaultRotateZ () const
    {
        return fDefaultRotateZ;
    }

    double &DefaultExtrusionDeepth ()
    {
        return fDefaultExtrusionDeepth;
    }

    double DefaultExtrusionDeepth () const
    {
        return fDefaultExtrusionDeepth;
    }

    double GetDefaultCamFocal();

    double &DefaultLightIntensity ()
    {
        return fDefaultLightIntensity;
    }

    double DefaultLightIntensity () const
    {
        return fDefaultLightIntensity;
    }

    double &DefaultAmbientIntensity ()
    {
        return fDefaultAmbientIntensity;
    }

    double DefaultAmbientIntensity () const
    {
        return fDefaultAmbientIntensity;
    }

    const Color &DefaultLightColor () const
    {
        return aDefaultLightColor;
    }

    Color DefaultLightColor ()
    {
        return aDefaultLightColor;
    }

    const Color &DefaultAmbientColor () const
    {
        return aDefaultAmbientColor;
    }

    Color DefaultAmbientColor ()
    {
        return aDefaultAmbientColor;
    }

    sal_Int32 GetHDefaultSegments() const { return nHDefaultSegments; }
    void SetHDefaultSegments(sal_Int32 nSegs) { nHDefaultSegments = nSegs; }

    sal_Int32 GetVDefaultSegments() const { return nVDefaultSegments; }
    void SetVDefaultSegments(sal_Int32 nSegs) { nVDefaultSegments = nSegs; }

    bool IsBreak3DObjPossible() const;
    void Break3DObj();

    bool DoubleSided () const
    {
        return bDoubleSided;
    }

    SfxItemSet Get3DAttributes(E3dScene* pInScene = NULL) const;
    void Set3DAttributes(const SfxItemSet& rAttr, E3dScene* pInScene = NULL, bool bOnly3DAttr=false);
};

#endif          // _E3D_VIEW3D_HXX

////////////////////////////////////////////////////////////////////////////////////////////////////
// eof
