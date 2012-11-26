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



#ifndef _SVDOBJFACTORY_HXX
#define _SVDOBJFACTORY_HXX

#include <svx/svdmodel.hxx>
#include <svx/svdocirc.hxx>
#include <svx/svdopath.hxx>

//////////////////////////////////////////////////////////////////////////////

class SVX_DLLPUBLIC SdrObjectCreationInfo
{
private:
    sal_uInt16          mnIdent;    // object identifier (SdrObjKind)
    sal_uInt32          mnInvent;   // object inventor
    SdrPathObjType      maSdrPathObjType; // if OBJ_POLY defines target object type
    SdrCircleObjType    maSdrCircleObjType; // if OBJ_CIRC defines target object type

    /// bitfield
    bool                mbFreehandMode : 1; // if OBJ_POLY, allow freehand creation

public:
    SdrObjectCreationInfo(
        sal_uInt16 nIdent = OBJ_NONE,
        sal_uInt32 nInvent = SdrInventor,
        SdrPathObjType aSdrPathObjType = PathType_Line,
        SdrCircleObjType aSdrCircleObjType = CircleType_Circle,
        bool bFreehandMode = false)
    :   mnIdent(nIdent),
        mnInvent(nInvent),
        maSdrPathObjType(aSdrPathObjType),
        maSdrCircleObjType(aSdrCircleObjType),
        mbFreehandMode(bFreehandMode)
    {
    }

    sal_uInt16 getIdent() const { return mnIdent; }
    void setIdent(const sal_uInt16 nNew) { mnIdent = nNew; }

    sal_uInt32 getInvent() const { return mnInvent; }
    void setInvent(const sal_uInt32 nNew) { mnInvent = nNew; }

    const SdrPathObjType& getSdrPathObjType() const { return maSdrPathObjType; }
    void setSdrPathObjType(const SdrPathObjType& rNew) { maSdrPathObjType = rNew; }

    const SdrCircleObjType& getSdrCircleObjType() const { return maSdrCircleObjType; }
    void setSdrCircleObjType(const SdrCircleObjType& rNew) { maSdrCircleObjType = rNew; }

    bool getFreehandMode() const { return mbFreehandMode; }
    void setFreehandMode(const bool bNew) { mbFreehandMode = bNew; }
};

//////////////////////////////////////////////////////////////////////////////

class SVX_DLLPUBLIC SdrObjFactory
{
private:
    SdrModel&                       mrTargetModel;
    SdrObjectCreationInfo           maSdrObjectCreationInfo;

    // fuer MakeNewObj():
    SdrObject*                      mpNewObj;

    // fuer MakeNewObjUserData():
    SdrObject*                      mpObj;
    SdrObjUserData*                 mpNewData;

    SVX_DLLPRIVATE SdrObjFactory(SdrModel& rTargetModel, const SdrObjectCreationInfo& rSdrObjectCreationInfo);
    SVX_DLLPRIVATE SdrObjFactory(const SdrObjectCreationInfo& rSdrObjectCreationInfo, SdrObject& rObj1);

public:
    static SdrObject* MakeNewObject(SdrModel& rTargetModel, const SdrObjectCreationInfo& rSdrObjectCreationInfo);
    static void InsertMakeObjectHdl(const Link& rLink);
    static void RemoveMakeObjectHdl(const Link& rLink);

    static SdrObjUserData* MakeNewObjUserData(const SdrObjectCreationInfo& rSdrObjectCreationInfo, SdrObject& rObj);
    static void InsertMakeUserDataHdl(const Link& rLink);
    static void RemoveMakeUserDataHdl(const Link& rLink);

    SdrModel& getTargetModel() const { return mrTargetModel; }
    const SdrObjectCreationInfo& getSdrObjectCreationInfo() const { return maSdrObjectCreationInfo; }
    SdrObject* getSdrObjUserDataTargetSdrObject() const { return mpObj; }
    SdrObjUserData* getNewSdrObjUserData() const { return mpNewData; }

    void setNewSdrObject(SdrObject* pNew) { mpNewObj = pNew; }
    void setNewSdrObjUserData(SdrObjUserData* pNew) { mpNewData = pNew; }
};

//////////////////////////////////////////////////////////////////////////////

#endif //_SVDOBJFACTORY_HXX

//////////////////////////////////////////////////////////////////////////////
// eof
