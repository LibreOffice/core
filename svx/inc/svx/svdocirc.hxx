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



#ifndef _SVDOCIRC_HXX
#define _SVDOCIRC_HXX

#include <svx/svdorect.hxx>
#include "svx/svxdllapi.h"

//////////////////////////////////////////////////////////////////////////////
// defines for circle type

enum SdrCircleObjType
{
    /// basic types the circle may have
    CircleType_Circle = 0,          // old OBJ_CIRC
    CircleType_Sector,              // old OBJ_SECT
    CircleType_Arc,                 // old OBJ_CARC
    CircleType_Segment,             // old OBJ_CCUT
};

//////////////////////////////////////////////////////////////////////////////
//   SdrCircObj

class SVX_DLLPUBLIC SdrCircObj : public SdrRectObj
{
protected:
    virtual sdr::contact::ViewContact* CreateObjectSpecificViewContact();

    // type of object
    SdrCircleObjType            meSdrCircleObjType;

    // [0.0 .. F_2PI], default is 0.0 for start and F_2PI for end
    double                      mfStartAngle;
    double                      mfEndAngle;

    virtual ~SdrCircObj();

    /// method to copy all data from given source
    virtual void copyDataFromSdrObject(const SdrObject& rSource);

public:
    /// create a copy, evtl. with a different target model (if given)
    virtual SdrObject* CloneSdrObject(SdrModel* pTargetModel = 0) const;

    SdrCircObj(
        SdrModel& rSdrModel,
        SdrCircleObjType eSdrCircleObjType = CircleType_Circle,
        const basegfx::B2DHomMatrix& rTransform = basegfx::B2DHomMatrix(),
        double fNewStartWink = 0.0,
        double fNewEndWink = F_2PI);

    virtual bool IsClosedObj() const;
    double GetStartAngle() const;
    double GetEndAngle() const;

    SdrCircleObjType GetSdrCircleObjType() const;
    void SetSdrCircleObjType(SdrCircleObjType eNew);

    void SetStartAngle(double fNew);
    void SetEndAngle(double fNew);

    virtual void TakeObjInfo(SdrObjTransformInfoRec& rInfo) const;
    virtual sal_uInt16 GetObjIdentifier() const;

    virtual void TakeObjNameSingul(String& rName) const;
    virtual void TakeObjNamePlural(String& rName) const;

    virtual basegfx::B2DPolyPolygon TakeXorPoly() const;

    virtual sal_uInt32 GetSnapPointCount() const;
    virtual basegfx::B2DPoint GetSnapPoint(sal_uInt32 i) const;

    virtual void AddToHdlList(SdrHdlList& rHdlList) const;

    // special drag methods
    virtual bool hasSpecialDrag() const;
    virtual bool beginSpecialDrag(SdrDragStat& rDrag) const;
    virtual bool applySpecialDrag(SdrDragStat& rDrag);
    virtual String getSpecialDragComment(const SdrDragStat& rDrag) const;

    virtual bool MovCreate(SdrDragStat& rStat);
    virtual bool EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd);
    virtual bool BckCreate(SdrDragStat& rStat);

    virtual basegfx::B2DPolyPolygon TakeCreatePoly(const SdrDragStat& rDrag) const;
    virtual Pointer GetCreatePointer(const SdrView& rSdrView) const;
    virtual SdrObject* DoConvertToPolygonObject(bool bBezier, bool bAddText) const;

protected:
    virtual SdrObjGeoData* NewGeoData() const;
    virtual void SaveGeoData(SdrObjGeoData& rGeo) const;
    virtual void RestGeoData(const SdrObjGeoData& rGeo);
};

//////////////////////////////////////////////////////////////////////////////

#endif //_SVDOCIRC_HXX

//////////////////////////////////////////////////////////////////////////////
// eof
