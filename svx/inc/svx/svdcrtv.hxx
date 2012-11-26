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



#ifndef _SVDCRTV_HXX
#define _SVDCRTV_HXX

#include <svx/svddrgv.hxx>
#include "svx/svxdllapi.h"
#include <svx/sdrobjectfactory.hxx>

////////////////////////////////////////////////////////////////////////////////////////////////////
// predefines

class SdrObjConnection;
class ImplConnectMarkerOverlay;
class ImpSdrCreateViewExtraData;

////////////////////////////////////////////////////////////////////////////////////////////////////

class SVX_DLLPUBLIC SdrCreateView: public SdrDragView
{
private:
protected:
    SdrObject*                  mpAktCreate;   // Aktuell in Erzeugung befindliches Objekt
    ImplConnectMarkerOverlay*   mpCoMaOverlay;

    // for migrating stuff from XOR, use ImpSdrCreateViewExtraData ATM to not need to
    // compile the apps all the time
    ImpSdrCreateViewExtraData*  mpCreateViewExtraData;

    // necessary data for new object creation
    SdrObjectCreationInfo       maSdrObjectCreationInfo;
    Pointer                     maCreatePointer;

    sal_uInt16                  mnAutoCloseDistPix;
    sal_uInt16                  mnFreeHandMinDistPix;

    /// bitfield
    bool                        mbAutoTextEdit : 1; // Textedit nach dem erzeugen eines Textrahmens starten
    bool                        mb1stPointAsCenter : 1;
    bool                        mbAutoClosePolys : 1;

    void ImpClearConnectMarker();

    bool ImpBegCreateObj(
        const SdrObjectCreationInfo& rSdrObjectCreationInfo,
        const basegfx::B2DPoint& rPnt,
        double fMinMovLogic,
        const basegfx::B2DRange& rLogRange,
        SdrObject* pPreparedFactoryObject);

    void ShowCreateObj();
    void HideCreateObj();
    bool CheckEdgeMode();

    // #i71538# make constructors of SdrView sub-components protected to avoid incomplete incarnations which may get casted to SdrView
    SdrCreateView(SdrModel& rModel1, OutputDevice* pOut = 0);
    virtual ~SdrCreateView();

public:
    const SdrObjectCreationInfo& getSdrObjectCreationInfo() const { return maSdrObjectCreationInfo; }
    void setSdrObjectCreationInfo(const SdrObjectCreationInfo& rSdrObjectCreationInfo);

    const Pointer& getCreatePointer() const { return maCreatePointer; }
    void setCreatePointer(const Pointer& rNew) { maCreatePointer = rNew; }

    virtual bool IsAction() const;
    virtual void MovAction(const basegfx::B2DPoint& rPnt);
    virtual void EndAction();
    virtual void BckAction();
    virtual void BrkAction();
    virtual basegfx::B2DRange TakeActionRange() const;

    virtual bool MouseMove(const MouseEvent& rMEvt, Window* pWin);
    void SetMeasureLayer(const String& rName) { maMeasureLayer = rName; }
    const String& GetMeasureLayer() const { return maMeasureLayer; }

    // Ist der MeasureLayer nicht gesetzt (Leerstring), so
    // wird der ActiveLayer auch fuer Bemassung verwendet.
    virtual void SetViewEditMode(SdrViewEditMode eMode) { SdrDragView::SetViewEditMode(eMode); CheckEdgeMode(); }

    // Feststellen, ob Textwerkzeug aktiviert
    bool IsTextTool() const;

    // Feststellen, ob Objektverbinderwerkzeug aktiviert
    bool IsEdgeTool() const;

    // Feststellen, ob Bemassungswerkzeug aktiviert
    bool IsMeasureTool() const;

    // Starten des normalen Create
    bool BegCreateObj(const basegfx::B2DPoint& rPnt, double fMinMovLogic = 3.0);
    bool BegCreatePreparedObject(const basegfx::B2DPoint& rPnt, double fMinMovLogic, SdrObject* pPreparedFactoryObject);
    void MovCreateObj(const basegfx::B2DPoint& rPnt);
    bool EndCreateObj(SdrCreateCmd eCmd);
    void BckCreateObj();  // z.B. wieder 1 Polygonpunkt zurueck.
    void BrkCreateObj();
    SdrObject* GetCreateObj() const { return mpAktCreate; }

    // BegCreateCaptionObj() erzeugt ein SdrCaptionObj (Legendenobjekt).
    // rObjSiz ist die anfaengliche Groesse des Legenden-Textrahmens.
    // gedraggd wird lediglich die Laenge des Zipfel.
    bool BegCreateCaptionObj(const basegfx::B2DPoint& rPnt, const basegfx::B2DVector& rObjSiz, double fMinMovLogic = 3.0);

    // Wenn TextEditAfterCreate auf sal_True steht (das ist der Default),
    // dann wird nach dem erzeugen eines Textrahmenobjekts (OBJ_TEXT,
    // OBJ_OUTLINERTEXT, OBJ_TITLETEXT, OBJ_CAPTION)
    // automatisch ein TextEdit (SdrObjEditView::SdrBeginTextEdit) gestartet.
    bool IsTextEditAfterCreate() const { return mbAutoTextEdit; }
    void SetTextEditAfterCreate(bool bOn) { if(mbAutoTextEdit != bOn) mbAutoTextEdit = bOn; }

    // Erzeugen eines Kreises/Rechtecks/Textrahmens wobei der 1. Punkt
    // nicht die linke obere Ecke, sondern das Zentrum des Objekts vorgibt.
    // Persistentes Flag. Default=false.
    bool IsCreate1stPointAsCenter() const { return mb1stPointAsCenter; }
    void SetCreate1stPointAsCenter(bool bOn) { if(mb1stPointAsCenter != bOn) mb1stPointAsCenter = bOn; }

    // Fuer Polylines (OBJ_PLIN) und Freihandlinien (OBJ_FREELINE). Ist dieses
    // Flag sal_True, werden diese beiden Objekttypen implizit geschlossen und in
    // Polygon (OBJ_POLY) bzw. Freihandflaeche (OBJ_FREEFILL) gewandelt falls
    // zum Ende des Create die Distanz zwischen Startpunkt und Endpunkt des
    // Objekts <=n Pixel ist, wobei SetAutoCloseDistPix vorgegeben wird.
    // Default=true.
    bool IsAutoClosePolys() const { return mbAutoClosePolys; }
    void SetAutoClosePolys(bool bOn) { if(mbAutoClosePolys != bOn) mbAutoClosePolys = bOn; }

    // Default=5 Pixel.
    sal_uInt16 GetAutoCloseDistPix() const { return mnAutoCloseDistPix; }
    void SetAutoCloseDistPix(sal_uInt16 nVal) { if(mnAutoCloseDistPix != nVal) mnAutoCloseDistPix = nVal; }

    // Vorgabe des minimalen Pixelabstands zwischen 2 Bezierpunkten bei der
    // Erzeugung einer Freihandlinie.
    // Default=10 Pixel.
    sal_uInt16 GetFreeHandMinDistPix() const { return mnFreeHandMinDistPix; }
    void SetFreeHandMinDistPix(sal_uInt16 nVal) { if(mnFreeHandMinDistPix != nVal) mnFreeHandMinDistPix = nVal; }

    void SetConnectMarker(const SdrObjConnection& rCon);
    void HideConnectMarker();

    // Attribute des ggf. gerade in der Erzeugung befindlichen Objekts
    /* new interface src537 */
    bool GetAttributes(SfxItemSet& rTargetSet, bool bOnlyHardAttr=false) const;

    bool SetAttributes(const SfxItemSet& rSet, bool bReplaceAll);
    SfxStyleSheet* GetStyleSheet() const;
    bool SetStyleSheet(SfxStyleSheet* pStyleSheet, bool bDontRemoveHardAttr);
};

#endif //_SVDCRTV_HXX

////////////////////////////////////////////////////////////////////////////////////////////////////
// eof
