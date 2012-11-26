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



#ifndef _SVDOPATH_HXX
#define _SVDOPATH_HXX

#include <svx/svdotext.hxx>
#include <svx/xpoly.hxx>
#include "svx/svxdllapi.h"
#include <basegfx/vector/b2enums.hxx>

//////////////////////////////////////////////////////////////////////////////
// predefines

class ImpPathForDragAndCreate;

//////////////////////////////////////////////////////////////////////////////
// helper classes

class SdrPathObjGeoData : public SdrObjGeoData
{
public:
    basegfx::B2DPolyPolygon maPathPolygon;

    SdrPathObjGeoData();
    virtual ~SdrPathObjGeoData();
};

//////////////////////////////////////////////////////////////////////////////
// defines for path type

enum SdrPathObjType
{
    /// basic types the path may have, derived from it's defining
    /// polygon
    PathType_Line = 0,          // old OBJ_LINE
    PathType_OpenPolygon,       // old OBJ_PLIN
    PathType_ClosedPolygon,     // old OBJ_POLY
    PathType_OpenBezier,        // old OBJ_PATHLINE
    PathType_ClosedBezier,      // old OBJ_PATHFILL
};

//////////////////////////////////////////////////////////////////////////////

class SVX_DLLPUBLIC SdrPathObj : public SdrTextObj
{
private:
    friend class ImpPathForDragAndCreate;

protected:
    virtual sdr::contact::ViewContact* CreateObjectSpecificViewContact();

    // the geometry data in object coordinates. This means that it is not
    // in normalized form. To get it in normalized form You need to transform
    // with the inverse of the object matrix
    basegfx::B2DPolyPolygon     maPathPolygon;

    // for isolation of old Drag/Create code
    ImpPathForDragAndCreate*    mpDAC;

    // helpers for GET/SET/INS/etc. PNT
    void ImpSetClosed(bool bClose);
    void ImpForceLineWink();
    ImpPathForDragAndCreate& impGetDAC(const SdrView& rView) const;
    void impDeleteDAC() const;

    // helper for adapting the object transformation when the geometry
    // (maPathPolygon) in world coordinates has changed
    void impAdaptTransformation();

    virtual ~SdrPathObj();

    /// method to copy all data from given source
    virtual void copyDataFromSdrObject(const SdrObject& rSource);

    /// GeoData support
    virtual SdrObjGeoData* NewGeoData() const;
    virtual void SaveGeoData(SdrObjGeoData& rGeo) const;
    virtual void RestGeoData(const SdrObjGeoData& rGeo);

public:
    // possible path types, derived from set polygon, thus readonly and useful
    // for various behaviours. Default is PathType_Line when not enough info in polygon
    SdrPathObjType getSdrPathObjType() const;

    /// create a copy, evtl. with a different target model (if given)
    virtual SdrObject* CloneSdrObject(SdrModel* pTargetModel = 0) const;

    static sal_Bool ImpFindPolyPnt(const basegfx::B2DPolyPolygon& rPoly, sal_uInt32 nAbsPnt, sal_uInt32& rPolyNum, sal_uInt32& rPointNum);
    SdrPathObj(
        SdrModel& rSdrModel,
        const basegfx::B2DPolyPolygon& rPathPoly = basegfx::B2DPolyPolygon());
    virtual bool IsClosedObj() const;

    virtual void TakeObjInfo(SdrObjTransformInfoRec& rInfo) const;
    virtual sal_uInt16 GetObjIdentifier() const;

    virtual void TakeObjNameSingul(String& rName) const;
    virtual void TakeObjNamePlural(String& rName) const;
    virtual basegfx::B2DPolyPolygon TakeXorPoly() const;
    virtual sal_uInt32 GetPlusHdlCount(const SdrHdl& rHdl) const;
    virtual void GetPlusHdl(SdrHdlList& rHdlList, const SdrObject& rSdrObject, const SdrHdl& rHdl, sal_uInt32 nPlNum) const;
    virtual void AddToHdlList(SdrHdlList& rHdlList) const;

    // special drag methods
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
    virtual Pointer GetCreatePointer(const SdrView& rSdrView) const;

    // during drag or create, allow accessing the so-far created/modified polyPolygon
    basegfx::B2DPolyPolygon getObjectPolyPolygon(const SdrDragStat& rDrag) const;
    basegfx::B2DPolyPolygon getDragPolyPolygon(const SdrDragStat& rDrag) const;

    virtual sal_uInt32 GetSnapPointCount() const;
    virtual basegfx::B2DPoint GetSnapPoint(sal_uInt32 i) const;

    virtual bool IsPolygonObject() const;
    virtual sal_uInt32 GetObjectPointCount() const;
    virtual basegfx::B2DPoint GetObjectPoint(sal_uInt32 nHdlNum) const;
    virtual void SetObjectPoint(const basegfx::B2DPoint& rPnt, sal_uInt32 nHdlNum);

    // insert point
    sal_uInt32 InsPointOld(const Point& rPos, sal_Bool bNewObj);
    sal_uInt32 InsPoint(const Point& rPos, sal_Bool bNewObj);

    // split at tis point
    SdrObject* RipPoint(sal_uInt32 nHdlNum, sal_uInt32& rNewPt0Index);

    virtual SdrObject* DoConvertToPolygonObject(bool bBezier, bool bAddText) const;
    virtual void setSdrObjectTransformation(const basegfx::B2DHomMatrix& rTransformation);

    // get/set polygon data in object coordiates
    basegfx::B2DPolyPolygon getB2DPolyPolygonInObjectCoordinates() const;
    void setB2DPolyPolygonInObjectCoordinates(const basegfx::B2DPolyPolygon& rPathPoly);

    // get/set polygon data in normalized coordiates
    basegfx::B2DPolyPolygon getB2DPolyPolygonInNormalizedCoordinates() const;
    void setB2DPolyPolygonInNormalizedCoordinates(const basegfx::B2DPolyPolygon& rPathPoly);

    // helpers for states
    bool isClosed() const;
    bool isLine() const;
    bool isBezier() const;

    void ToggleClosed();
};

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //_SVDOPATH_HXX

