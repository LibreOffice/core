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


#ifndef _DFLYOBJ_HXX
#define _DFLYOBJ_HXX

#include <svx/svdobj.hxx>

class SwFlyFrm;
class SwFrmFmt;
class SdrObjMacroHitRec;

const sal_uInt32 SWGInventor =  sal_uInt32('S')*0x00000001+
                            sal_uInt32('W')*0x00000100+
                            sal_uInt32('G')*0x00010000;

const sal_uInt16 SwFlyDrawObjIdentifier = 0x0001;
const sal_uInt16 SwDrawFirst            = 0x0001;

//---------------------------------------
//SwFlyDrawObj, Die DrawObjekte fuer Flys.

class SwFlyDrawObj : public SdrObject
{
private:
    virtual sdr::properties::BaseProperties* CreateObjectSpecificProperties();

protected:
    // #i95264# SwFlyDrawObj needs an own VC since createViewIndependentPrimitive2DSequence()
    // is called when the view independent range of the object is used
    virtual sdr::contact::ViewContact* CreateObjectSpecificViewContact();

    ~SwFlyDrawObj();

    /// method to copy all data from given source
    virtual void copyDataFromSdrObject(const SdrObject& rSource);

public:
    SwFlyDrawObj(SdrModel& rSdrModel);

    /// create a copy, evtl. with a different target model (if given)
    virtual SdrObject* CloneSdrObject(SdrModel* pTargetModel = 0) const;

    //Damit eine Instanz dieser Klasse beim laden erzeugt werden kann
    //(per Factory).
    virtual sal_uInt32 GetObjInventor()     const;
    virtual sal_uInt16 GetObjIdentifier()   const;
    virtual sal_uInt16 GetObjVersion()      const;
};

//---------------------------------------
//SwVirtFlyDrawObj, die virtuellen Objekte fuer Flys.
//Flys werden immer mit virtuellen Objekten angezeigt. Nur so koennen sie
//ggf. mehrfach angezeigt werden (Kopf-/Fusszeilen).

class SwVirtFlyDrawObj : public SdrObject
{
private:
    SwFlyFrm *pFlyFrm;

    //////////////////////////////////////////////////////////////////////////
    // members which were missing from SdrVirtObj
    SdrObject&                  rRefObj;

protected:
    // AW: Need own sdr::contact::ViewContact since AnchorPos from parent is
    // not used but something own (top left of new SnapRect minus top left
    // of original SnapRect)
    virtual sdr::contact::ViewContact* CreateObjectSpecificViewContact();

    // original Notify from SdrVirtObj
    virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint);

    void impReactOnGeometryChange();

    /// method to copy all data from given source
    virtual void copyDataFromSdrObject(const SdrObject& rSource);

    ~SwVirtFlyDrawObj();

public:
    // for paints triggered form ExecutePrimitive
    void wrap_DoPaintObject() const;

    // for simple access to inner and outer bounds
    basegfx::B2DRange getOuterBound() const;
    basegfx::B2DRange getInnerBound() const;

    SwVirtFlyDrawObj(SdrObject& rNew, SwFlyFrm* pFly);

    /// create a copy, evtl. with a different target model (if given)
    virtual SdrObject* CloneSdrObject(SdrModel* pTargetModel = 0) const;

    virtual bool IsClosedObj() const;

    virtual void     TakeObjInfo( SdrObjTransformInfoRec& rInfo ) const;
//  virtual       void       recalculateObjectRange();
    virtual ::basegfx::B2DPolyPolygon TakeXorPoly() const;

    const SwFrmFmt *GetFmt() const;
          SwFrmFmt *GetFmt();

    // Get Methoden fuer die Fly Verpointerung
          SwFlyFrm* GetFlyFrm()         { return pFlyFrm; }
    const SwFlyFrm* GetFlyFrm() const   { return pFlyFrm; }

    void SetRect() const;

    // ist eine URL an einer Grafik gesetzt, dann ist das ein Makro-Object
    virtual bool HasMacro() const;
    virtual SdrObject* CheckMacroHit       (const SdrObjMacroHitRec& rRec) const;
    virtual Pointer    GetMacroPointer     (const SdrObjMacroHitRec& rRec) const;

    // FullDrag support
    virtual bool supportsFullDrag() const;
    virtual SdrObject* getFullDragClone() const;

    //////////////////////////////////////////////////////////////////////////
    // methods which were missing from SdrVirtObj which do something
    SdrObject& ReferencedObj();
    const SdrObject& GetReferencedObj() const;

    virtual sdr::properties::BaseProperties& GetProperties() const;
    //virtual void SetModel(SdrModel* pNewModel);
    virtual sal_uInt32 GetObjInventor() const;
    virtual sal_uInt16 GetObjIdentifier() const;
    virtual SdrObjList* getChildrenOfSdrObject() const;
    //virtual SdrObjList* GetSubList() const;
    virtual void SetChanged();
    virtual void TakeObjNameSingul(String& rName) const;
    virtual void TakeObjNamePlural(String& rName) const;
    virtual sal_uInt32 GetPlusHdlCount(const SdrHdl& rHdl) const;
    virtual void GetPlusHdl(SdrHdlList& rHdlList, SdrObject& rSdrObject, const SdrHdl& rHdl, sal_uInt32 nPlNum) const;
    virtual void AddToHdlList(SdrHdlList& rHdlList) const;
    virtual bool hasSpecialDrag() const;
    virtual bool beginSpecialDrag(SdrDragStat& rDrag) const;
    virtual bool applySpecialDrag(SdrDragStat& rDrag);
    virtual String getSpecialDragComment(const SdrDragStat& rDrag) const;
    virtual basegfx::B2DPolyPolygon getSpecialDragPoly(const SdrDragStat& rDrag) const;
    virtual bool BegCreate(SdrDragStat& rStat);
    virtual bool MovCreate(SdrDragStat& rStat);
    virtual bool EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd);
    virtual bool BckCreate(SdrDragStat& rStat);
    virtual void BrkCreate(SdrDragStat& rStat);
    virtual basegfx::B2DPolyPolygon TakeCreatePoly(const SdrDragStat& rDrag) const;
    virtual sal_uInt32 GetSnapPointCount() const;
    virtual basegfx::B2DPoint GetSnapPoint(sal_uInt32 i) const;
    virtual bool IsPolygonObject() const;
    virtual sal_uInt32 GetObjectPointCount() const;
    virtual basegfx::B2DPoint GetObjectPoint(sal_uInt32 i) const;
    virtual void SetObjectPoint(const basegfx::B2DPoint& rPnt, sal_uInt32 i);
    virtual SdrObjGeoData* GetGeoData() const;
    virtual void SetGeoData(const SdrObjGeoData& rGeo);
    //virtual void NbcReformatText();
    virtual void ReformatText();
    virtual void PaintMacro (OutputDevice& rOut, const SdrObjMacroHitRec& rRec) const;
    virtual bool DoMacro (const SdrObjMacroHitRec& rRec);
    virtual XubString GetMacroPopupComment(const SdrObjMacroHitRec& rRec) const;
    virtual const Point GetOffset() const;

    virtual void setSdrObjectTransformation(const basegfx::B2DHomMatrix& rTransformation);
};


#endif
