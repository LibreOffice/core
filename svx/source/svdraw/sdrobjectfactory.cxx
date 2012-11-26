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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include <svx/sdrobjectfactory.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svdoedge.hxx>
#include <svx/svdomeas.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdocapt.hxx>
#include <svx/svdopage.hxx>
#include <svx/svdouno.hxx>
#include <svx/svdoashp.hxx>
#include <svx/svdomedia.hxx>
#include <svx/svdotable.hxx>
#include <svx/svdetc.hxx>

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrObjFactory::SdrObjFactory(SdrModel& rTargetModel, const SdrObjectCreationInfo& rSdrObjectCreationInfo)
:   mrTargetModel(rTargetModel),
    maSdrObjectCreationInfo(rSdrObjectCreationInfo),
    mpNewObj(0),
    mpObj(0),
    mpNewData(0)
{
}

SdrObjFactory::SdrObjFactory(const SdrObjectCreationInfo& rSdrObjectCreationInfo, SdrObject& rObj1)
:   mrTargetModel(rObj1.getSdrModelFromSdrObject()),
    maSdrObjectCreationInfo(rSdrObjectCreationInfo),
    mpNewObj(0),
    mpObj(&rObj1),
    mpNewData(0)
{
}

SdrObject* SdrObjFactory::MakeNewObject(SdrModel& rTargetModel, const SdrObjectCreationInfo& rSdrObjectCreationInfo)
{
    SdrObject* pObj = 0;

    if(SdrInventor == rSdrObjectCreationInfo.getInvent())
    {
        switch(rSdrObjectCreationInfo.getIdent())
        {
            case sal_uInt16(OBJ_NONE):
            {
                pObj = new SdrObject(rTargetModel);
                break;
            }
            case sal_uInt16(OBJ_GRUP):
            {
                pObj = new SdrObjGroup(rTargetModel);
                break;
            }
            case sal_uInt16(OBJ_POLY):
            {
                pObj = new SdrPathObj(rTargetModel);
                break;
            }
            case sal_uInt16(OBJ_EDGE):
            {
                pObj = new SdrEdgeObj(rTargetModel);
                break;
            }
            case sal_uInt16(OBJ_RECT):
            {
                pObj = new SdrRectObj(rTargetModel);
                break;
            }
            case sal_uInt16(OBJ_CIRC):
            {
                pObj = new SdrCircObj(rTargetModel, rSdrObjectCreationInfo.getSdrCircleObjType());
                break;
            }
            case sal_uInt16(OBJ_TEXT):
            {
                pObj = new SdrRectObj(rTargetModel, basegfx::B2DHomMatrix(), OBJ_TEXT, true);
                break;
            }
            case sal_uInt16(OBJ_TITLETEXT):
            {
                pObj = new SdrRectObj(rTargetModel, basegfx::B2DHomMatrix(), OBJ_TITLETEXT, true);
                break;
            }
            case sal_uInt16(OBJ_OUTLINETEXT):
            {
                pObj = new SdrRectObj(rTargetModel, basegfx::B2DHomMatrix(), OBJ_OUTLINETEXT, true);
                break;
            }
            case sal_uInt16(OBJ_MEASURE):
            {
                pObj = new SdrMeasureObj(rTargetModel);
                break;
            }
            case sal_uInt16(OBJ_GRAF):
            {
                pObj = new SdrGrafObj(rTargetModel, Graphic());
                break;
            }
            case sal_uInt16(OBJ_OLE2):
            {
                pObj = new SdrOle2Obj(rTargetModel, svt::EmbeddedObjectRef(), String(), basegfx::B2DHomMatrix(), true);
                break;
            }
            case sal_uInt16(OBJ_FRAME):
            {
                pObj = new SdrOle2Obj(rTargetModel, svt::EmbeddedObjectRef(), String(), basegfx::B2DHomMatrix(), true);
                break;
            }
            case sal_uInt16(OBJ_CAPTION):
            {
                pObj = new SdrCaptionObj(rTargetModel);
                break;
            }
            case sal_uInt16(OBJ_PAGE):
            {
                pObj = new SdrPageObj(rTargetModel);
                break;
            }
            case sal_uInt16(OBJ_UNO):
            {
                pObj = new SdrUnoObj(rTargetModel, String());
                break;
            }
            case sal_uInt16(OBJ_CUSTOMSHAPE):
            {
                pObj = new SdrObjCustomShape(rTargetModel);
                break;
            }
            case sal_uInt16(OBJ_MEDIA):
            {
                pObj = new SdrMediaObj(rTargetModel);
                break;
            }
            case sal_uInt16(OBJ_TABLE):
            {
                pObj = new ::sdr::table::SdrTableObj(rTargetModel);
                break;
            }
        }
    }

    if(!pObj)
    {
        SdrObjFactory* pFact = new SdrObjFactory(rTargetModel, rSdrObjectCreationInfo);
        SdrLinkList& rLL = ImpGetUserMakeObjHdl();
        unsigned nAnz(rLL.GetLinkCount());
        unsigned i(0);

        while(i < nAnz && !pObj)
        {
            rLL.GetLink(i).Call((void*)pFact);
            pObj = pFact->mpNewObj;
            i++;
        }

        delete pFact;
    }

    return pObj;
}

SdrObjUserData* SdrObjFactory::MakeNewObjUserData(const SdrObjectCreationInfo& rSdrObjectCreationInfo, SdrObject& rObj1)
{
    SdrObjUserData* pData = 0;

    if(rSdrObjectCreationInfo.getInvent() == SdrInventor)
    {
        switch(rSdrObjectCreationInfo.getIdent())
        {
            case sal_uInt16(SDRUSERDATA_OBJTEXTLINK) :
            {
                pData = new ImpSdrObjTextLinkUserData(dynamic_cast< SdrTextObj* >(&rObj1));
                break;
            }
        }
    }

    if(!pData)
    {
        SdrObjFactory aFact(rSdrObjectCreationInfo, rObj1);
        SdrLinkList& rLL = ImpGetUserMakeObjUserDataHdl();
        unsigned nAnz(rLL.GetLinkCount());
        unsigned i(0);

        while(i < nAnz && !pData)
        {
            rLL.GetLink(i).Call((void*)&aFact);
            pData = aFact.mpNewData;
            i++;
        }
    }

    return pData;
}

void SdrObjFactory::InsertMakeObjectHdl(const Link& rLink)
{
    SdrLinkList& rLL = ImpGetUserMakeObjHdl();
    rLL.InsertLink(rLink);
}

void SdrObjFactory::RemoveMakeObjectHdl(const Link& rLink)
{
    SdrLinkList& rLL = ImpGetUserMakeObjHdl();
    rLL.RemoveLink(rLink);
}

void SdrObjFactory::InsertMakeUserDataHdl(const Link& rLink)
{
    SdrLinkList& rLL = ImpGetUserMakeObjUserDataHdl();
    rLL.InsertLink(rLink);
}

void SdrObjFactory::RemoveMakeUserDataHdl(const Link& rLink)
{
    SdrLinkList& rLL = ImpGetUserMakeObjUserDataHdl();
    rLL.RemoveLink(rLink);
}

//////////////////////////////////////////////////////////////////////////////
// eof
