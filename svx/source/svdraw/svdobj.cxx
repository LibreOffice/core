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

#include "sal/config.h"

#include <com/sun/star/lang/XComponent.hpp>

#include "svdconv.hxx"

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygoncutter.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/range/b2drange.hxx>
#include <drawinglayer/processor2d/contourextractor2d.hxx>
#include <drawinglayer/processor2d/linegeometryextractor2d.hxx>
#include <editeng/editeng.hxx>
#include <editeng/eeitem.hxx>
#include <math.h>
#include <sfx2/objface.hxx>
#include <sfx2/objsh.hxx>
#include <svl/whiter.hxx>
#include <svtools/colorcfg.hxx>
#include <tools/bigint.hxx>
#include <tools/diagnose_ex.h>
#include <tools/helpers.hxx>
#include <tools/line.hxx>
#include <vcl/cvtsvm.hxx>
#include <vcl/graphictools.hxx>
#include <vcl/metaact.hxx>   // for TakeContour
#include <vcl/virdev.hxx>
#include <vector>

#include "svx/shapepropertynotifier.hxx"
#include "svx/svdglob.hxx"   // StringCache
#include "svx/svdotable.hxx"
#include "svx/xlinjoit.hxx"

#include <svx/fmmodel.hxx>
#include <svx/polysc3d.hxx>
#include <svx/sdr/contact/displayinfo.hxx>
#include <svx/sdr/contact/objectcontactofobjlistpainter.hxx>
#include <svx/sdr/contact/viewcontactofgraphic.hxx>
#include <svx/sdr/contact/viewcontactofsdrobj.hxx>
#include <svx/sdr/properties/emptyproperties.hxx>
#include <svx/sdrhittesthelper.hxx>
#include <svx/sdrobjectfilter.hxx>
#include <svx/svddrag.hxx>
#include <svx/svdetc.hxx>
#include <svx/svdhdl.hxx>
#include <svx/svditer.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdoashp.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdocapt.hxx>  // Factory
#include <svx/svdocirc.hxx>  // Factory
#include <svx/svdoedge.hxx>  // Factory
#include <svx/svdograf.hxx>  // Factory
#include <svx/svdogrp.hxx>   // Factory
#include <svx/svdomeas.hxx>  // Factory
#include <svx/svdomedia.hxx>
#include <svx/svdoole2.hxx>  // Factory
#include <svx/svdopage.hxx>  // Factory
#include <svx/svdopath.hxx>  // Factory
#include <svx/svdorect.hxx>  // Factory
#include <svx/svdotext.hxx>  // Factory
#include <svx/svdouno.hxx>   // Factory
#include <svx/svdovirt.hxx>  // for Add/Del Ref
#include <svx/svdpage.hxx>
#include <svx/svdpool.hxx>
#include <svx/svdstr.hrc>    // the object's name
#include <svx/svdtrans.hxx>
#include <svx/svdundo.hxx>
#include <svx/svdview.hxx>   // for Dragging (check Ortho)
#include <svx/sxlayitm.hxx>
#include <svx/sxlogitm.hxx>
#include <svx/sxmovitm.hxx>
#include <svx/sxmspitm.hxx>
#include <svx/sxoneitm.hxx>
#include <svx/sxonitm.hxx>
#include <svx/sxopitm.hxx>
#include <svx/sxraitm.hxx>
#include <svx/sxreoitm.hxx>
#include <svx/sxrooitm.hxx>
#include <svx/sxsaitm.hxx>
#include <svx/sxsoitm.hxx>
#include <svx/sxtraitm.hxx>
#include <svx/unopage.hxx>
#include <svx/unoshape.hxx>
#include <svx/xbtmpit.hxx>
#include <svx/xenum.hxx>
#include <svx/xflclit.hxx>
#include <svx/xflftrit.hxx>
#include <svx/xflhtit.hxx>
#include <svx/xfltrit.hxx>
#include <svx/xgrad.hxx>
#include <svx/xhatch.hxx>
#include <svx/xlnclit.hxx>
#include <svx/xlndsit.hxx>
#include <svx/xlnedcit.hxx>
#include <svx/xlnedit.hxx>
#include <svx/xlnedwit.hxx>
#include <svx/xlnstcit.hxx>
#include <svx/xlnstit.hxx>
#include <svx/xlnstwit.hxx>
#include <svx/xlntrit.hxx>
#include <svx/xlnwtit.hxx>
#include <svx/xpoly.hxx>

using namespace ::com::sun::star;

////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEINIT0(SdrObjUserCall);

SdrObjUserCall::~SdrObjUserCall()
{
}

void SdrObjUserCall::Changed(const SdrObject& /*rObj*/, SdrUserCallType /*eType*/, const Rectangle& /*rOldBoundRect*/)
{
}

SdrObjMacroHitRec::SdrObjMacroHitRec() :
    pOut(NULL),
    pVisiLayer(NULL),
    pPageView(NULL),
    nTol(0),
    bDown(false) {}

////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEINIT0(SdrObjUserData);

SdrObjUserData::SdrObjUserData(sal_uInt32 nInv, sal_uInt16 nId, sal_uInt16 nVer) :
    nInventor(nInv),
    nIdentifier(nId),
    nVersion(nVer) {}

SdrObjUserData::SdrObjUserData(const SdrObjUserData& rData) :
    nInventor(rData.nInventor),
    nIdentifier(rData.nIdentifier),
    nVersion(rData.nVersion) {}

SdrObjUserData::~SdrObjUserData() {}

sal_uInt32 SdrObjUserData::GetInventor() const
{
    return nInventor;
}

sal_uInt16 SdrObjUserData::GetId() const
{
    return nIdentifier;
}

bool SdrObjUserData::HasMacro(const SdrObject* /*pObj*/) const
{
    return false;
}

SdrObject* SdrObjUserData::CheckMacroHit(const SdrObjMacroHitRec& rRec, const SdrObject* pObj) const
{
    if(pObj)
    {
        if(rRec.pPageView)
        {
            return SdrObjectPrimitiveHit(*pObj, rRec.aPos, rRec.nTol, *rRec.pPageView, rRec.pVisiLayer, false);
        }
    }

    return 0;
}

Pointer SdrObjUserData::GetMacroPointer(const SdrObjMacroHitRec& /*rRec*/, const SdrObject* /*pObj*/) const
{
    return Pointer(POINTER_REFHAND);
}

void SdrObjUserData::PaintMacro(OutputDevice& rOut, const Rectangle& /*rDirtyRect*/, const SdrObjMacroHitRec& /*rRec*/, const SdrObject* pObj) const
{
    if(!pObj)
        return;

    const RasterOp eRop(rOut.GetRasterOp());
    const basegfx::B2DPolyPolygon aPolyPolygon(pObj->TakeXorPoly());
    const sal_uInt32 nCount(aPolyPolygon.count());

    rOut.SetLineColor(COL_BLACK);
    rOut.SetFillColor();
    rOut.SetRasterOp(ROP_INVERT);

    for(sal_uInt32 a(0); a < nCount; a++)
    {
        rOut.DrawPolyLine(aPolyPolygon.getB2DPolygon(a));
    }

    rOut.SetRasterOp(eRop);
}

bool SdrObjUserData::DoMacro(const SdrObjMacroHitRec& /*rRec*/, SdrObject* /*pObj*/)
{
    return false;
}

OUString SdrObjUserData::GetMacroPopupComment(const SdrObjMacroHitRec& /*rRec*/, const SdrObject* /*pObj*/) const
{
    return OUString();
}

SdrObjUserDataList::SdrObjUserDataList() {}
SdrObjUserDataList::~SdrObjUserDataList() {}

size_t SdrObjUserDataList::GetUserDataCount() const
{
    return static_cast<sal_uInt16>(maList.size());
}

const SdrObjUserData* SdrObjUserDataList::GetUserData(size_t nNum) const
{
    return &maList.at(nNum);
}

SdrObjUserData* SdrObjUserDataList::GetUserData(size_t nNum)
{
    return &maList.at(nNum);
}

void SdrObjUserDataList::AppendUserData(SdrObjUserData* pData)
{
    maList.push_back(pData);
}

void SdrObjUserDataList::DeleteUserData(size_t nNum)
{
    maList.erase(maList.begin()+nNum);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

DBG_NAME(SdrObjGeoData);

SdrObjGeoData::SdrObjGeoData():
    pGPL(NULL),
    bMovProt(false),
    bSizProt(false),
    bNoPrint(false),
    bClosedObj(false),
    mbVisible(true),
    mnLayerID(0)
{
    DBG_CTOR(SdrObjGeoData,NULL);
}

SdrObjGeoData::~SdrObjGeoData()
{
    DBG_DTOR(SdrObjGeoData,NULL);
    delete pGPL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEINIT0(SdrObjPlusData);

SdrObjPlusData::SdrObjPlusData():
    pBroadcast(NULL),
    pUserDataList(NULL),
    pGluePoints(NULL)
{
}

SdrObjPlusData::~SdrObjPlusData()
{
    delete pBroadcast;
    delete pUserDataList;
    delete pGluePoints;
}

SdrObjPlusData* SdrObjPlusData::Clone(SdrObject* pObj1) const
{
    SdrObjPlusData* pNeuPlusData=new SdrObjPlusData;
    if (pUserDataList!=NULL) {
        sal_uInt16 nAnz=pUserDataList->GetUserDataCount();
        if (nAnz!=0) {
            pNeuPlusData->pUserDataList=new SdrObjUserDataList;
            for (sal_uInt16 i=0; i<nAnz; i++) {
                SdrObjUserData* pNeuUserData=pUserDataList->GetUserData(i)->Clone(pObj1);
                if (pNeuUserData!=NULL) {
                    pNeuPlusData->pUserDataList->AppendUserData(pNeuUserData);
                } else {
                    OSL_FAIL("SdrObjPlusData::Clone(): UserData.Clone() returns NULL.");
                }
            }
        }
    }
    if (pGluePoints!=NULL) pNeuPlusData->pGluePoints=new SdrGluePointList(*pGluePoints);
    // MtfAnimator isn't copied either

    // #i68101#
    // copy object name, title and description
    pNeuPlusData->aObjName = aObjName;
    pNeuPlusData->aObjTitle = aObjTitle;
    pNeuPlusData->aObjDescription = aObjDescription;

    return pNeuPlusData;
}

void SdrObjPlusData::SetGluePoints(const SdrGluePointList& rPts)
{
    return *pGluePoints = rPts;
}

SdrObjTransformInfoRec::SdrObjTransformInfoRec() :
    bSelectAllowed(true),
    bMoveAllowed(true),
    bResizeFreeAllowed(true),
    bResizePropAllowed(true),
    bRotateFreeAllowed(true),
    bRotate90Allowed(true),
    bMirrorFreeAllowed(true),
    bMirror45Allowed(true),
    bMirror90Allowed(true),
    bTransparenceAllowed(true),
    bGradientAllowed(true),
    bShearAllowed(true),
    bEdgeRadiusAllowed(true),
    bNoOrthoDesired(true),
    bNoContortion(true),
    bCanConvToPath(true),
    bCanConvToPoly(true),
    bCanConvToContour(false),
    bCanConvToPathLineToArea(true),
    bCanConvToPolyLineToArea(true) {}

//////////////////////////////////////////////////////////////////////////////
// BaseProperties section

sdr::properties::BaseProperties* SdrObject::CreateObjectSpecificProperties()
{
    return new sdr::properties::EmptyProperties(*this);
}

sdr::properties::BaseProperties& SdrObject::GetProperties() const
{
    if(!mpProperties)
    {
        const_cast< SdrObject* >(this)->mpProperties =
            const_cast< SdrObject* >(this)->CreateObjectSpecificProperties();
    }

    return *mpProperties;
}

//////////////////////////////////////////////////////////////////////////////
// ObjectUser section

void SdrObject::AddObjectUser(sdr::ObjectUser& rNewUser)
{
    maObjectUsers.push_back(&rNewUser);
}

void SdrObject::RemoveObjectUser(sdr::ObjectUser& rOldUser)
{
    const ::sdr::ObjectUserVector::iterator aFindResult = ::std::find(maObjectUsers.begin(), maObjectUsers.end(), &rOldUser);
    if(aFindResult != maObjectUsers.end())
    {
        maObjectUsers.erase(aFindResult);
    }
}

//////////////////////////////////////////////////////////////////////////////
// DrawContact section

sdr::contact::ViewContact* SdrObject::CreateObjectSpecificViewContact()
{
    return new sdr::contact::ViewContactOfSdrObj(*this);
}

sdr::contact::ViewContact& SdrObject::GetViewContact() const
{
    if(!mpViewContact)
    {
        const_cast< SdrObject* >(this)->mpViewContact =
            const_cast< SdrObject* >(this)->CreateObjectSpecificViewContact();
    }

    return *mpViewContact;
}

// DrawContact support: Methods for handling Object changes
void SdrObject::ActionChanged() const
{
    // Do necessary ViewContact actions
    GetViewContact().ActionChanged();
}

//////////////////////////////////////////////////////////////////////////////

void SdrObject::SetBoundRectDirty()
{
    aOutRect = Rectangle();
}

//////////////////////////////////////////////////////////////////////////////

DBG_NAME(SdrObject);
TYPEINIT1(SdrObject,SfxListener);

SdrObject::SdrObject()
    :mpProperties(0L)
    ,mpViewContact(0L)
    ,pObjList(NULL)
    ,pPage(NULL)
    ,pModel(NULL)
    ,pUserCall(NULL)
    ,pPlusData(NULL)
    ,nOrdNum(0)
    ,mnNavigationPosition(SAL_MAX_UINT32)
    ,mnLayerID(0)
    ,mpSvxShape( NULL )
    ,maWeakUnoShape()
    ,mbDoNotInsertIntoPageAutomatically(false)
{
    DBG_CTOR(SdrObject,NULL);
    bVirtObj         =false;
    bSnapRectDirty   =true;
    bNetLock         =false;
    bInserted        =false;
    bGrouped         =false;
    bMovProt         =false;
    bSizProt         =false;
    bNoPrint         =false;
    bEmptyPresObj    =false;
    bNotVisibleAsMaster=false;
    bClosedObj       =false;
    mbVisible        = true;

    // #i25616#
    mbLineIsOutsideGeometry = false;

    // #i25616#
    mbSupportTextIndentingOnLineWidthChange = false;

    bNotMasterCachable=false;
    bIsEdge=false;
    bIs3DObj=false;
    bMarkProt=false;
    bIsUnoObj=false;
}

SdrObject::~SdrObject()
{
    // tell all the registered ObjectUsers that the page is in destruction
    ::sdr::ObjectUserVector aListCopy(maObjectUsers.begin(), maObjectUsers.end());
    for(::sdr::ObjectUserVector::iterator aIterator = aListCopy.begin(); aIterator != aListCopy.end(); ++aIterator)
    {
        sdr::ObjectUser* pObjectUser = *aIterator;
        DBG_ASSERT(pObjectUser, "SdrObject::~SdrObject: corrupt ObjectUser list (!)");
        pObjectUser->ObjectInDestruction(*this);
    }

    // Clear the vector. This means that user do not need to call RemoveObjectUser()
    // when they get called from ObjectInDestruction().
    maObjectUsers.clear();

    try
    {
        SvxShape* pSvxShape = getSvxShape();
        if ( pSvxShape )
        {
            OSL_ENSURE(!pSvxShape->HasSdrObjectOwnership(),"Please check where this call come from and replace it with SdrObject::Free");
            pSvxShape->InvalidateSdrObject();
            uno::Reference< lang::XComponent > xShapeComp( getWeakUnoShape(), uno::UNO_QUERY_THROW );
            xShapeComp->dispose();
        }
    }
    catch( const uno::Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    DBG_DTOR(SdrObject,NULL);
    SendUserCall(SDRUSERCALL_DELETE, GetLastBoundRect());
    delete pPlusData;

    if(mpProperties)
    {
        delete mpProperties;
        mpProperties = 0L;
    }

    if(mpViewContact)
    {
        delete mpViewContact;
        mpViewContact = 0L;
    }
}

void SdrObject::Free( SdrObject*& _rpObject )
{
    SdrObject* pObject = _rpObject; _rpObject = NULL;
    if ( pObject == NULL )
        // nothing to do
        return;

    SvxShape* pShape = pObject->getSvxShape();
    if ( pShape && pShape->HasSdrObjectOwnership() )
        // only the shape is allowed to delete me, and will reset the ownership before doing so
        return;

    delete pObject;
}

SdrObjPlusData* SdrObject::NewPlusData() const
{
    return new SdrObjPlusData;
}

void SdrObject::SetRectsDirty(sal_Bool bNotMyself)
{
    if (!bNotMyself) {
        SetBoundRectDirty();
        bSnapRectDirty=true;
    }
    if (pObjList!=NULL) {
        pObjList->SetRectsDirty();
    }
}

void SdrObject::SetModel(SdrModel* pNewModel)
{
    if(pNewModel && pPage)
    {
        if(pPage->GetModel() != pNewModel)
        {
            pPage = NULL;
        }
    }

    // update listeners at possible API wrapper object
    if( pModel != pNewModel )
    {
        SvxShape* pShape = getSvxShape();
        if( pShape )
            pShape->ChangeModel( pNewModel );
    }

    pModel = pNewModel;
}

SdrModel* SdrObject::GetModel() const
{
    return pModel;
}

void SdrObject::SetObjList(SdrObjList* pNewObjList)
{
    pObjList=pNewObjList;
}

SdrObjList* SdrObject::GetObjList() const
{
    return pObjList;
}

void SdrObject::SetPage(SdrPage* pNewPage)
{
    SdrModel* pOldModel = pModel;
    SdrPage* pOldPage = pPage;

    pPage=pNewPage;
    if (pPage!=NULL) {
        SdrModel* pMod=pPage->GetModel();
        if (pMod!=pModel && pMod!=NULL) {
            SetModel(pMod);
        }}

    // The creation of the UNO shape in SdrObject::getUnoShape is influenced
    // by pPage, so when the page changes we need to discard the cached UNO
    // shape so that a new one will be created.
    // If the page is changing to another page with the same model, we
    // assume they create compatible UNO shape objects so we shouldn't have
    // to invalidate.
    if (pOldPage != pPage && !(pOldPage && pPage && pOldModel == pModel))
    {
        SvxShape* const pShape(getSvxShape());
        if (pShape && !pShape->HasSdrObjectOwnership())
            setUnoShape(NULL);
    }
}

SdrPage* SdrObject::GetPage() const
{
    return pPage;
}

// init global static itempool
SdrItemPool* SdrObject::mpGlobalItemPool = NULL;

SdrItemPool& SdrObject::GetGlobalDrawObjectItemPool()
{
    if(!mpGlobalItemPool)
    {
        mpGlobalItemPool = new SdrItemPool();
        SfxItemPool* pGlobalOutlPool = EditEngine::CreatePool();
        mpGlobalItemPool->SetSecondaryPool(pGlobalOutlPool);
        mpGlobalItemPool->SetDefaultMetric((SfxMapUnit)SdrEngineDefaults::GetMapUnit());
        mpGlobalItemPool->FreezeIdRanges();
    }

    return *mpGlobalItemPool;
}

SdrItemPool* SdrObject::GetObjectItemPool() const
{
    if(pModel)
        return (SdrItemPool*)(&pModel->GetItemPool());

    // use a static global default pool
    return &SdrObject::GetGlobalDrawObjectItemPool();
}

sal_uInt32 SdrObject::GetObjInventor()   const
{
    return SdrInventor;
}

sal_uInt16 SdrObject::GetObjIdentifier() const
{
    return sal_uInt16(OBJ_NONE);
}

void SdrObject::TakeObjInfo(SdrObjTransformInfoRec& rInfo) const
{
    rInfo.bRotateFreeAllowed=false;
    rInfo.bMirrorFreeAllowed=false;
    rInfo.bTransparenceAllowed = false;
    rInfo.bGradientAllowed = false;
    rInfo.bShearAllowed     =false;
    rInfo.bEdgeRadiusAllowed=false;
    rInfo.bCanConvToPath    =false;
    rInfo.bCanConvToPoly    =false;
    rInfo.bCanConvToContour = false;
    rInfo.bCanConvToPathLineToArea=false;
    rInfo.bCanConvToPolyLineToArea=false;
}

SdrLayerID SdrObject::GetLayer() const
{
    return mnLayerID;
}

void SdrObject::getMergedHierarchyLayerSet(SetOfByte& rSet) const
{
    rSet.Set(GetLayer());
    SdrObjList* pOL=GetSubList();
    if (pOL!=NULL) {
        sal_uIntPtr nObjAnz=pOL->GetObjCount();
        for (sal_uIntPtr nObjNum=0; nObjNum<nObjAnz; nObjNum++) {
            pOL->GetObj(nObjNum)->getMergedHierarchyLayerSet(rSet);
        }
    }
}

void SdrObject::NbcSetLayer(SdrLayerID nLayer)
{
    mnLayerID = nLayer;
}

void SdrObject::SetLayer(SdrLayerID nLayer)
{
    NbcSetLayer(nLayer);
    SetChanged();
    BroadcastObjectChange();
}

void SdrObject::AddListener(SfxListener& rListener)
{
    ImpForcePlusData();
    if (pPlusData->pBroadcast==NULL) pPlusData->pBroadcast=new SfxBroadcaster;
    rListener.StartListening(*pPlusData->pBroadcast);
}

void SdrObject::RemoveListener(SfxListener& rListener)
{
    if (pPlusData!=NULL && pPlusData->pBroadcast!=NULL) {
        rListener.EndListening(*pPlusData->pBroadcast);
        if (!pPlusData->pBroadcast->HasListeners()) {
            delete pPlusData->pBroadcast;
            pPlusData->pBroadcast=NULL;
        }
    }
}

const SfxBroadcaster* SdrObject::GetBroadcaster() const
{
    return pPlusData!=NULL ? pPlusData->pBroadcast : NULL;
}

void SdrObject::AddReference(SdrVirtObj& rVrtObj)
{
    AddListener(rVrtObj);
}

void SdrObject::DelReference(SdrVirtObj& rVrtObj)
{
    RemoveListener(rVrtObj);
}

bool SdrObject::HasRefPoint() const
{
    return false;
}

Point SdrObject::GetRefPoint() const
{
    return GetCurrentBoundRect().Center();
}

void SdrObject::SetRefPoint(const Point& /*rPnt*/)
{
}

bool SdrObject::IsGroupObject() const
{
    return GetSubList()!=NULL;
}

SdrObjList* SdrObject::GetSubList() const
{
    return NULL;
}

SdrObject* SdrObject::GetUpGroup() const
{
    return pObjList!=NULL ? pObjList->GetOwnerObj() : NULL;
}

void SdrObject::SetName(const OUString& rStr)
{
    if (!rStr.isEmpty() && !pPlusData)
    {
        ImpForcePlusData();
    }

    if(pPlusData && !pPlusData->aObjName.equals(rStr))
    {
        // Undo/Redo for setting object's name (#i73249#)
        bool bUndo( false );
        if ( GetModel() && GetModel()->IsUndoEnabled() )
        {
            bUndo = true;
            SdrUndoAction* pUndoAction =
                    GetModel()->GetSdrUndoFactory().CreateUndoObjectStrAttr(
                                                    *this,
                                                    SdrUndoObjStrAttr::OBJ_NAME,
                                                    GetName(),
                                                    rStr );
            GetModel()->BegUndo( pUndoAction->GetComment() );
            GetModel()->AddUndo( pUndoAction );
        }
        pPlusData->aObjName = rStr;
        // Undo/Redo for setting object's name (#i73249#)
        if ( bUndo )
        {
            GetModel()->EndUndo();
        }
        SetChanged();
        BroadcastObjectChange();
    }
}

OUString SdrObject::GetName() const
{
    if(pPlusData)
    {
        return pPlusData->aObjName;
    }

    return OUString();
}

void SdrObject::SetTitle(const OUString& rStr)
{
    if (!rStr.isEmpty() && !pPlusData)
    {
        ImpForcePlusData();
    }

    if(pPlusData && pPlusData->aObjTitle != rStr)
    {
        // Undo/Redo for setting object's title (#i73249#)
        bool bUndo( false );
        if ( GetModel() && GetModel()->IsUndoEnabled() )
        {
            bUndo = true;
            SdrUndoAction* pUndoAction =
                    GetModel()->GetSdrUndoFactory().CreateUndoObjectStrAttr(
                                                    *this,
                                                    SdrUndoObjStrAttr::OBJ_TITLE,
                                                    GetTitle(),
                                                    rStr );
            GetModel()->BegUndo( pUndoAction->GetComment() );
            GetModel()->AddUndo( pUndoAction );
        }
        pPlusData->aObjTitle = rStr;
        // Undo/Redo for setting object's title (#i73249#)
        if ( bUndo )
        {
            GetModel()->EndUndo();
        }
        SetChanged();
        BroadcastObjectChange();
    }
}

OUString SdrObject::GetTitle() const
{
    if(pPlusData)
    {
        return pPlusData->aObjTitle;
    }

    return OUString();
}

void SdrObject::SetDescription(const OUString& rStr)
{
    if (!rStr.isEmpty() && !pPlusData)
    {
        ImpForcePlusData();
    }

    if(pPlusData && !pPlusData->aObjDescription.equals(rStr))
    {
        // Undo/Redo for setting object's description (#i73249#)
        bool bUndo( false );
        if ( GetModel() && GetModel()->IsUndoEnabled() )
        {
            bUndo = true;
            SdrUndoAction* pUndoAction =
                    GetModel()->GetSdrUndoFactory().CreateUndoObjectStrAttr(
                                                    *this,
                                                    SdrUndoObjStrAttr::OBJ_DESCRIPTION,
                                                    GetDescription(),
                                                    rStr );
            GetModel()->BegUndo( pUndoAction->GetComment() );
            GetModel()->AddUndo( pUndoAction );
        }
        pPlusData->aObjDescription = rStr;
        // Undo/Redo for setting object's description (#i73249#)
        if ( bUndo )
        {
            GetModel()->EndUndo();
        }
        SetChanged();
        BroadcastObjectChange();
    }
}

OUString SdrObject::GetDescription() const
{
    if(pPlusData)
    {
        return pPlusData->aObjDescription;
    }

    return OUString();
}

sal_uInt32 SdrObject::GetOrdNum() const
{
    if (pObjList!=NULL) {
        if (pObjList->IsObjOrdNumsDirty()) {
            pObjList->RecalcObjOrdNums();
        }
    } else ((SdrObject*)this)->nOrdNum=0;
    return nOrdNum;
}

sal_uInt32 SdrObject::GetOrdNumDirect() const
{
    return nOrdNum;
}

void SdrObject::SetOrdNum(sal_uInt32 nNum)
{
    nOrdNum = nNum;
}

sal_uInt32 SdrObject::GetNavigationPosition (void)
{
    if (pObjList!=NULL && pObjList->RecalcNavigationPositions())
    {
        return mnNavigationPosition;
    }
    else
        return GetOrdNum();
}




void SdrObject::SetNavigationPosition (const sal_uInt32 nNewPosition)
{
    mnNavigationPosition = nNewPosition;
}




// To make clearer that this method may trigger RecalcBoundRect and thus may be
// expensive and sometimes problematic (inside a bigger object change you will get
// non-useful BoundRects sometimes) I rename that method from GetBoundRect() to
// GetCurrentBoundRect().
const Rectangle& SdrObject::GetCurrentBoundRect() const
{
    if(aOutRect.IsEmpty())
    {
        const_cast< SdrObject* >(this)->RecalcBoundRect();
    }

    return aOutRect;
}

// To have a possibility to get the last calculated BoundRect e.g for producing
// the first rectangle for repaints (old and new need to be used) without forcing
// a RecalcBoundRect (which may be problematical and expensive sometimes) I add here
// a new method for accessing the last BoundRect.
const Rectangle& SdrObject::GetLastBoundRect() const
{
    return aOutRect;
}

void SdrObject::RecalcBoundRect()
{
    // #i101680# suppress BoundRect calculations on import(s)
    if(pModel && pModel->isLocked())
        return;

    // central new method which will calculate the BoundRect using primitive geometry
    if(aOutRect.IsEmpty())
    {
        const drawinglayer::primitive2d::Primitive2DSequence xPrimitives(GetViewContact().getViewIndependentPrimitive2DSequence());

        if(xPrimitives.hasElements())
        {
            // use neutral ViewInformation and get the range of the primitives
            const drawinglayer::geometry::ViewInformation2D aViewInformation2D;
            const basegfx::B2DRange aRange(drawinglayer::primitive2d::getB2DRangeFromPrimitive2DSequence(xPrimitives, aViewInformation2D));

            if(!aRange.isEmpty())
            {
                aOutRect = Rectangle(
                        (sal_Int32)floor(aRange.getMinX()), (sal_Int32)floor(aRange.getMinY()),
                        (sal_Int32)ceil(aRange.getMaxX()), (sal_Int32)ceil(aRange.getMaxY()));
                aOutRect -= GetGridOffset(); // don't include grid offset
                return;
            }
        }
    }
}

void SdrObject::BroadcastObjectChange() const
{
    if( pModel && pModel->isLocked() )
        return;

    bool bPlusDataBroadcast(pPlusData && pPlusData->pBroadcast);
    bool bObjectChange(IsInserted() && pModel);

    if(bPlusDataBroadcast || bObjectChange)
    {
        SdrHint aHint(*this);

        if(bPlusDataBroadcast)
        {
            pPlusData->pBroadcast->Broadcast(aHint);
        }

        if(bObjectChange)
        {
            pModel->Broadcast(aHint);
        }
    }
}

void SdrObject::SetChanged()
{
    // For testing purposes, use the new ViewContact for change
    // notification now.
    ActionChanged();

    if(IsInserted() && pModel)
    {
        pModel->SetChanged();
    }
}

// tooling for painting a single object to an OutputDevice.
bool SdrObject::SingleObjectPainter(OutputDevice& rOut) const
{
    sdr::contact::SdrObjectVector aObjectVector;
    aObjectVector.push_back(const_cast< SdrObject* >(this));

    sdr::contact::ObjectContactOfObjListPainter aPainter(rOut, aObjectVector, GetPage());
    sdr::contact::DisplayInfo aDisplayInfo;

    aPainter.ProcessDisplay(aDisplayInfo);

    return true;
}

bool SdrObject::LineGeometryUsageIsNecessary() const
{
    XLineStyle eXLS = (XLineStyle)((const XLineStyleItem&)GetMergedItem(XATTR_LINESTYLE)).GetValue();
    return (eXLS != XLINE_NONE);
}

SdrObject* SdrObject::Clone() const
{
    return CloneHelper< SdrObject >();
}

SdrObject& SdrObject::operator=(const SdrObject& rObj)
{
    if( this == &rObj )
        return *this;

    if(mpProperties)
    {
        delete mpProperties;
        mpProperties = 0L;
    }

    if(mpViewContact)
    {
        delete mpViewContact;
        mpViewContact = 0L;
    }

    // The Clone() method uses the local copy constructor from the individual
    // sdr::properties::BaseProperties class. Since the target class maybe for another
    // draw object, an SdrObject needs to be provided, as in the normal constructor.
    mpProperties = &rObj.GetProperties().Clone(*this);

    pModel  =rObj.pModel;
    pPage = rObj.pPage;
    aOutRect=rObj.aOutRect;
    mnLayerID = rObj.mnLayerID;
    aAnchor =rObj.aAnchor;
    bVirtObj=rObj.bVirtObj;
    bSizProt=rObj.bSizProt;
    bMovProt=rObj.bMovProt;
    bNoPrint=rObj.bNoPrint;
    mbVisible=rObj.mbVisible;
    bMarkProt=rObj.bMarkProt;
    bEmptyPresObj =rObj.bEmptyPresObj;
    bNotVisibleAsMaster=rObj.bNotVisibleAsMaster;
    bSnapRectDirty=true;
    bNotMasterCachable=rObj.bNotMasterCachable;
    delete pPlusData;
    pPlusData=NULL;
    if (rObj.pPlusData!=NULL) {
        pPlusData=rObj.pPlusData->Clone(this);
    }
    if (pPlusData!=NULL && pPlusData->pBroadcast!=NULL) {
        delete pPlusData->pBroadcast; // broadcaster isn't copied
        pPlusData->pBroadcast=NULL;
    }
    aGridOffset = rObj.aGridOffset;
    return *this;
}

OUString SdrObject::TakeObjNameSingul() const
{
    OUStringBuffer sName(ImpGetResStr(STR_ObjNameSingulNONE));

    OUString aName(GetName());
    if (!aName.isEmpty())
    {
        sName.append(' ');
        sName.append('\'');
        sName.append(aName);
        sName.append('\'');
    }
    return sName.makeStringAndClear();
}

void SdrObject::TakeObjNamePlural(XubString& rName) const
{
    rName=ImpGetResStr(STR_ObjNamePluralNONE);
}

void SdrObject::ImpTakeDescriptionStr(sal_uInt16 nStrCacheID, OUString& rStr, sal_uInt16 nVal) const
{
    rStr = ImpGetResStr(nStrCacheID);
    sal_Int32 nPos = rStr.indexOf("%1");
    if (nPos >= 0)
    {
        // Replace '%1' with the object name.
        OUString aObjName(TakeObjNameSingul());
        rStr = rStr.replaceAt(nPos, 2, aObjName);
    }

    nPos = rStr.indexOf("%2");
    if (nPos >= 0)
        // Replace '%2' with the passed value.
        rStr = rStr.replaceAt(
            nPos, 2, OUString::number(nVal));
}

void SdrObject::ImpForcePlusData()
{
    if (!pPlusData)
        pPlusData = NewPlusData();
}

OUString SdrObject::GetWinkStr(long nWink, bool bNoDegChar) const
{
    OUString aStr;
    if (pModel!=NULL) {
        pModel->TakeWinkStr(nWink,aStr,bNoDegChar);
    }
    return aStr;
}

OUString SdrObject::GetMetrStr(long nVal, MapUnit /*eWantMap*/, bool bNoUnitChars) const
{
    OUString aStr;
    if (pModel!=NULL) {
        pModel->TakeMetricStr(nVal,aStr,bNoUnitChars);
    }
    return aStr;
}

basegfx::B2DPolyPolygon SdrObject::TakeXorPoly() const
{
    basegfx::B2DPolyPolygon aRetval;
    const Rectangle aR(GetCurrentBoundRect());
    const basegfx::B2DRange aRange(aR.Left(), aR.Top(), aR.Right(), aR.Bottom());
    aRetval.append(basegfx::tools::createPolygonFromRect(aRange));

    return aRetval;
}

basegfx::B2DPolyPolygon SdrObject::TakeContour() const
{
    basegfx::B2DPolyPolygon aRetval;

    // create cloned object without text, but with XLINE_SOLID,
    // COL_BLACK as line color and XFILL_NONE
    SdrObject* pClone = Clone();

    if(pClone)
    {
        const SdrTextObj* pTextObj = dynamic_cast< const SdrTextObj* >(this);

        if(pTextObj)
        {
            // no text and no text animation
            pClone->SetMergedItem(SdrTextAniKindItem(SDRTEXTANI_NONE));
            pClone->SetOutlinerParaObject(0);
        }

        const SdrEdgeObj* pEdgeObj = dynamic_cast< const SdrEdgeObj* >(this);

        if(pEdgeObj)
        {
            // create connections if connector, will be cleaned up when
            // deleting the connector again
            SdrObject* pLeft = pEdgeObj->GetConnectedNode(true);
            SdrObject* pRight = pEdgeObj->GetConnectedNode(false);

            if(pLeft)
            {
                pClone->ConnectToNode(true, pLeft);
            }

            if(pRight)
            {
                pClone->ConnectToNode(false, pRight);
            }
        }

        SfxItemSet aNewSet(*GetObjectItemPool());

        // #i101980# ignore LineWidth; that's what the old implementation
        // did. With line width, the result may be huge due to fat/thick
        // line decompositions
        aNewSet.Put(XLineWidthItem(0));

        // solid black lines and no fill
        aNewSet.Put(XLineStyleItem(XLINE_SOLID));
        aNewSet.Put(XLineColorItem(String(), Color(COL_BLACK)));
        aNewSet.Put(XFillStyleItem(XFILL_NONE));
        pClone->SetMergedItemSet(aNewSet);

        // get sequence from clone
        const sdr::contact::ViewContact& rVC(pClone->GetViewContact());
        const drawinglayer::primitive2d::Primitive2DSequence xSequence(rVC.getViewIndependentPrimitive2DSequence());

        if(xSequence.hasElements())
        {
            // use neutral ViewInformation
            const drawinglayer::geometry::ViewInformation2D aViewInformation2D;

            // create extractor, process and get result (with hairlines as opened polygons)
            drawinglayer::processor2d::ContourExtractor2D aExtractor(aViewInformation2D, false);
            aExtractor.process(xSequence);
            const basegfx::B2DPolyPolygonVector& rResult(aExtractor.getExtractedContour());
            const sal_uInt32 nSize(rResult.size());

            // when count is one, it is implied that the object has only its normal
            // contour anyways and TakeCountour() is to return an empty PolyPolygon
            // (see old implementation for historical reasons)
            if(nSize > 1)
            {
                // the topology for contour is correctly a vector of PolyPolygons; for
                // historical reasons cut it back to a single PolyPolygon here
                for(sal_uInt32 a(0); a < nSize; a++)
                {
                    aRetval.append(rResult[a]);
                }
            }
        }

        delete pClone;
    }

    return aRetval;
}

sal_uInt32 SdrObject::GetHdlCount() const
{
    return 8L;
}

SdrHdl* SdrObject::GetHdl(sal_uInt32 nHdlNum) const
{
    SdrHdl* pH=NULL;
    const Rectangle& rR=GetSnapRect();
    switch (nHdlNum) {
        case 0: pH=new SdrHdl(rR.TopLeft(),     HDL_UPLFT); break;
        case 1: pH=new SdrHdl(rR.TopCenter(),   HDL_UPPER); break;
        case 2: pH=new SdrHdl(rR.TopRight(),    HDL_UPRGT); break;
        case 3: pH=new SdrHdl(rR.LeftCenter(),  HDL_LEFT ); break;
        case 4: pH=new SdrHdl(rR.RightCenter(), HDL_RIGHT); break;
        case 5: pH=new SdrHdl(rR.BottomLeft(),  HDL_LWLFT); break;
        case 6: pH=new SdrHdl(rR.BottomCenter(),HDL_LOWER); break;
        case 7: pH=new SdrHdl(rR.BottomRight(), HDL_LWRGT); break;
    }
    return pH;
}

sal_uInt32 SdrObject::GetPlusHdlCount(const SdrHdl& /*rHdl*/) const
{
    return 0L;
}

SdrHdl* SdrObject::GetPlusHdl(const SdrHdl& /*rHdl*/, sal_uInt32 /*nPlNum*/) const
{
    return 0L;
}

void SdrObject::AddToHdlList(SdrHdlList& rHdlList) const
{
    sal_uInt32 nAnz=GetHdlCount();
    for (sal_uInt32 i=0L; i<nAnz; i++) {
        SdrHdl* pHdl=GetHdl(i);
        if (pHdl!=NULL) {
            rHdlList.AddHdl(pHdl);
        }
    }
}

Rectangle SdrObject::ImpDragCalcRect(const SdrDragStat& rDrag) const
{
    Rectangle aTmpRect(GetSnapRect());
    Rectangle aRect(aTmpRect);
    const SdrHdl* pHdl=rDrag.GetHdl();
    SdrHdlKind eHdl=pHdl==NULL ? HDL_MOVE : pHdl->GetKind();
    bool bEcke=(eHdl==HDL_UPLFT || eHdl==HDL_UPRGT || eHdl==HDL_LWLFT || eHdl==HDL_LWRGT);
    bool bOrtho=rDrag.GetView()!=NULL && rDrag.GetView()->IsOrtho();
    bool bBigOrtho=bEcke && bOrtho && rDrag.GetView()->IsBigOrtho();
    Point aPos(rDrag.GetNow());
    bool bLft=(eHdl==HDL_UPLFT || eHdl==HDL_LEFT  || eHdl==HDL_LWLFT);
    bool bRgt=(eHdl==HDL_UPRGT || eHdl==HDL_RIGHT || eHdl==HDL_LWRGT);
    bool bTop=(eHdl==HDL_UPRGT || eHdl==HDL_UPPER || eHdl==HDL_UPLFT);
    bool bBtm=(eHdl==HDL_LWRGT || eHdl==HDL_LOWER || eHdl==HDL_LWLFT);
    if (bLft) aTmpRect.Left()  =aPos.X();
    if (bRgt) aTmpRect.Right() =aPos.X();
    if (bTop) aTmpRect.Top()   =aPos.Y();
    if (bBtm) aTmpRect.Bottom()=aPos.Y();
    if (bOrtho) { // Ortho
        long nWdt0=aRect.Right() -aRect.Left();
        long nHgt0=aRect.Bottom()-aRect.Top();
        long nXMul=aTmpRect.Right() -aTmpRect.Left();
        long nYMul=aTmpRect.Bottom()-aTmpRect.Top();
        long nXDiv=nWdt0;
        long nYDiv=nHgt0;
        bool bXNeg=(nXMul<0)!=(nXDiv<0);
        bool bYNeg=(nYMul<0)!=(nYDiv<0);
        nXMul=std::abs(nXMul);
        nYMul=std::abs(nYMul);
        nXDiv=std::abs(nXDiv);
        nYDiv=std::abs(nYDiv);
        Fraction aXFact(nXMul,nXDiv); // fractions for canceling
        Fraction aYFact(nYMul,nYDiv); // and for comparing
        nXMul=aXFact.GetNumerator();
        nYMul=aYFact.GetNumerator();
        nXDiv=aXFact.GetDenominator();
        nYDiv=aYFact.GetDenominator();
        if (bEcke) { // corner point handles
            bool bUseX=(aXFact<aYFact) != bBigOrtho;
            if (bUseX) {
                long nNeed=long(BigInt(nHgt0)*BigInt(nXMul)/BigInt(nXDiv));
                if (bYNeg) nNeed=-nNeed;
                if (bTop) aTmpRect.Top()=aTmpRect.Bottom()-nNeed;
                if (bBtm) aTmpRect.Bottom()=aTmpRect.Top()+nNeed;
            } else {
                long nNeed=long(BigInt(nWdt0)*BigInt(nYMul)/BigInt(nYDiv));
                if (bXNeg) nNeed=-nNeed;
                if (bLft) aTmpRect.Left()=aTmpRect.Right()-nNeed;
                if (bRgt) aTmpRect.Right()=aTmpRect.Left()+nNeed;
            }
        } else { // apex handles
            if ((bLft || bRgt) && nXDiv!=0) {
                long nHgt0b=aRect.Bottom()-aRect.Top();
                long nNeed=long(BigInt(nHgt0b)*BigInt(nXMul)/BigInt(nXDiv));
                aTmpRect.Top()-=(nNeed-nHgt0b)/2;
                aTmpRect.Bottom()=aTmpRect.Top()+nNeed;
            }
            if ((bTop || bBtm) && nYDiv!=0) {
                long nWdt0b=aRect.Right()-aRect.Left();
                long nNeed=long(BigInt(nWdt0b)*BigInt(nYMul)/BigInt(nYDiv));
                aTmpRect.Left()-=(nNeed-nWdt0b)/2;
                aTmpRect.Right()=aTmpRect.Left()+nNeed;
            }
        }
    }
    aTmpRect.Justify();
    return aTmpRect;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool SdrObject::hasSpecialDrag() const
{
    return false;
}

bool SdrObject::supportsFullDrag() const
{
    return true;
}

SdrObject* SdrObject::getFullDragClone() const
{
    // default uses simple clone
    return Clone();
}

bool SdrObject::beginSpecialDrag(SdrDragStat& rDrag) const
{
    const SdrHdl* pHdl = rDrag.GetHdl();

    SdrHdlKind eHdl = (pHdl == NULL) ? HDL_MOVE : pHdl->GetKind();

    if(eHdl==HDL_UPLFT || eHdl==HDL_UPPER || eHdl==HDL_UPRGT ||
        eHdl==HDL_LEFT || eHdl==HDL_RIGHT || eHdl==HDL_LWLFT ||
        eHdl==HDL_LOWER || eHdl==HDL_LWRGT)
    {
        return true;
    }

    return false;
}

bool SdrObject::applySpecialDrag(SdrDragStat& rDrag)
{
    Rectangle aNewRect(ImpDragCalcRect(rDrag));

    if(aNewRect != GetSnapRect())
    {
           NbcSetSnapRect(aNewRect);
    }

    return true;
}

String SdrObject::getSpecialDragComment(const SdrDragStat& /*rDrag*/) const
{
    return String();
}

basegfx::B2DPolyPolygon SdrObject::getSpecialDragPoly(const SdrDragStat& /*rDrag*/) const
{
    // default has nothing to add
    return basegfx::B2DPolyPolygon();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Create
bool SdrObject::BegCreate(SdrDragStat& rStat)
{
    rStat.SetOrtho4Possible();
    Rectangle aRect1(rStat.GetStart(), rStat.GetNow());
    aRect1.Justify();
    rStat.SetActionRect(aRect1);
    aOutRect = aRect1;
    return true;
}

bool SdrObject::MovCreate(SdrDragStat& rStat)
{
    rStat.TakeCreateRect(aOutRect);
    rStat.SetActionRect(aOutRect);
    aOutRect.Justify();

    return true;
}

bool SdrObject::EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd)
{
    rStat.TakeCreateRect(aOutRect);
    aOutRect.Justify();

    return (eCmd==SDRCREATE_FORCEEND || rStat.GetPointAnz()>=2);
}

void SdrObject::BrkCreate(SdrDragStat& /*rStat*/)
{
}

bool SdrObject::BckCreate(SdrDragStat& /*rStat*/)
{
    return false;
}

basegfx::B2DPolyPolygon SdrObject::TakeCreatePoly(const SdrDragStat& rDrag) const
{
    Rectangle aRect1;
    rDrag.TakeCreateRect(aRect1);
    aRect1.Justify();

    basegfx::B2DPolyPolygon aRetval;
    const basegfx::B2DRange aRange(aRect1.Left(), aRect1.Top(), aRect1.Right(), aRect1.Bottom());
    aRetval.append(basegfx::tools::createPolygonFromRect(aRange));
    return aRetval;
}

Pointer SdrObject::GetCreatePointer() const
{
    return Pointer(POINTER_CROSS);
}

// transformations
void SdrObject::NbcMove(const Size& rSiz)
{
    MoveRect(aOutRect,rSiz);
    SetRectsDirty();
}

void SdrObject::NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact)
{
    bool bXMirr=(xFact.GetNumerator()<0) != (xFact.GetDenominator()<0);
    bool bYMirr=(yFact.GetNumerator()<0) != (yFact.GetDenominator()<0);
    if (bXMirr || bYMirr) {
        Point aRef1(GetSnapRect().Center());
        if (bXMirr) {
            Point aRef2(aRef1);
            aRef2.Y()++;
            NbcMirrorGluePoints(aRef1,aRef2);
        }
        if (bYMirr) {
            Point aRef2(aRef1);
            aRef2.X()++;
            NbcMirrorGluePoints(aRef1,aRef2);
        }
    }
    ResizeRect(aOutRect,rRef,xFact,yFact);
    SetRectsDirty();
}

void SdrObject::NbcRotate(const Point& rRef, long nWink, double sn, double cs)
{
    SetGlueReallyAbsolute(true);
    aOutRect.Move(-rRef.X(),-rRef.Y());
    Rectangle R(aOutRect);
    if (sn==1.0 && cs==0.0) { // 90deg
        aOutRect.Left()  =-R.Bottom();
        aOutRect.Right() =-R.Top();
        aOutRect.Top()   =R.Left();
        aOutRect.Bottom()=R.Right();
    } else if (sn==0.0 && cs==-1.0) { // 180deg
        aOutRect.Left()  =-R.Right();
        aOutRect.Right() =-R.Left();
        aOutRect.Top()   =-R.Bottom();
        aOutRect.Bottom()=-R.Top();
    } else if (sn==-1.0 && cs==0.0) { // 270deg
        aOutRect.Left()  =R.Top();
        aOutRect.Right() =R.Bottom();
        aOutRect.Top()   =-R.Right();
        aOutRect.Bottom()=-R.Left();
    }
    aOutRect.Move(rRef.X(),rRef.Y());
    aOutRect.Justify(); // just in case
    SetRectsDirty();
    NbcRotateGluePoints(rRef,nWink,sn,cs);
    SetGlueReallyAbsolute(false);
}

void SdrObject::NbcMirror(const Point& rRef1, const Point& rRef2)
{
    SetGlueReallyAbsolute(true);
    aOutRect.Move(-rRef1.X(),-rRef1.Y());
    Rectangle R(aOutRect);
    long dx=rRef2.X()-rRef1.X();
    long dy=rRef2.Y()-rRef1.Y();
    if (dx==0) {          // vertical axis
        aOutRect.Left() =-R.Right();
        aOutRect.Right()=-R.Left();
    } else if (dy==0) {   // horizontal axis
        aOutRect.Top()   =-R.Bottom();
        aOutRect.Bottom()=-R.Top();
    } else if (dx==dy) {  // 45deg axis
        aOutRect.Left()  =R.Top();
        aOutRect.Right() =R.Bottom();
        aOutRect.Top()   =R.Left();
        aOutRect.Bottom()=R.Right();
    } else if (dx==-dy) { // 45deg axis
        aOutRect.Left()  =-R.Bottom();
        aOutRect.Right() =-R.Top();
        aOutRect.Top()   =-R.Right();
        aOutRect.Bottom()=-R.Left();
    }
    aOutRect.Move(rRef1.X(),rRef1.Y());
    aOutRect.Justify(); // just in case
    SetRectsDirty();
    NbcMirrorGluePoints(rRef1,rRef2);
    SetGlueReallyAbsolute(false);
}

void SdrObject::NbcShear(const Point& rRef, long nWink, double tn, bool bVShear)
{
    SetGlueReallyAbsolute(true);
    NbcShearGluePoints(rRef,nWink,tn,bVShear);
    SetGlueReallyAbsolute(false);
}

void SdrObject::Move(const Size& rSiz)
{
    if (rSiz.Width()!=0 || rSiz.Height()!=0) {
        Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetLastBoundRect();
        NbcMove(rSiz);
        SetChanged();
        BroadcastObjectChange();
        SendUserCall(SDRUSERCALL_MOVEONLY,aBoundRect0);
    }
}

void SdrObject::Resize(const Point& rRef, const Fraction& xFact, const Fraction& yFact, bool bUnsetRelative)
{
    if (xFact.GetNumerator()!=xFact.GetDenominator() || yFact.GetNumerator()!=yFact.GetDenominator()) {
        if (bUnsetRelative)
        {
            mnRelativeWidth.reset( );
            mnRelativeHeight.reset( );
        }
        Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetLastBoundRect();
        NbcResize(rRef,xFact,yFact);
        SetChanged();
        BroadcastObjectChange();
        SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
    }
}

void SdrObject::Rotate(const Point& rRef, long nWink, double sn, double cs)
{
    if (nWink!=0) {
        Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetLastBoundRect();
        NbcRotate(rRef,nWink,sn,cs);
        SetChanged();
        BroadcastObjectChange();
        SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
    }
}

void SdrObject::Mirror(const Point& rRef1, const Point& rRef2)
{
    Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetLastBoundRect();
    NbcMirror(rRef1,rRef2);
    SetChanged();
    BroadcastObjectChange();
    SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
}

void SdrObject::Shear(const Point& rRef, long nWink, double tn, bool bVShear)
{
    if (nWink!=0) {
        Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetLastBoundRect();
        NbcShear(rRef,nWink,tn,bVShear);
        SetChanged();
        BroadcastObjectChange();
        SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
    }
}

void SdrObject::NbcSetRelativePos(const Point& rPnt)
{
    Point aRelPos0(GetSnapRect().TopLeft()-aAnchor);
    Size aSiz(rPnt.X()-aRelPos0.X(),rPnt.Y()-aRelPos0.Y());
    NbcMove(aSiz); // This also calls SetRectsDirty()
}

void SdrObject::SetRelativePos(const Point& rPnt)
{
    if (rPnt!=GetRelativePos()) {
        Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetLastBoundRect();
        NbcSetRelativePos(rPnt);
        SetChanged();
        BroadcastObjectChange();
        SendUserCall(SDRUSERCALL_MOVEONLY,aBoundRect0);
    }
}

Point SdrObject::GetRelativePos() const
{
    return GetSnapRect().TopLeft()-aAnchor;
}

void SdrObject::ImpSetAnchorPos(const Point& rPnt)
{
    aAnchor = rPnt;
}

void SdrObject::NbcSetAnchorPos(const Point& rPnt)
{
    Size aSiz(rPnt.X()-aAnchor.X(),rPnt.Y()-aAnchor.Y());
    aAnchor=rPnt;
    NbcMove(aSiz); // This also calls SetRectsDirty()
}

void SdrObject::SetAnchorPos(const Point& rPnt)
{
    if (rPnt!=aAnchor) {
        Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetLastBoundRect();
        NbcSetAnchorPos(rPnt);
        SetChanged();
        BroadcastObjectChange();
        SendUserCall(SDRUSERCALL_MOVEONLY,aBoundRect0);
    }
}

const Point& SdrObject::GetAnchorPos() const
{
    return aAnchor;
}

void SdrObject::RecalcSnapRect()
{
}

const Rectangle& SdrObject::GetSnapRect() const
{
    return aOutRect;
}

void SdrObject::NbcSetSnapRect(const Rectangle& rRect)
{
    aOutRect=rRect;
}

const Rectangle& SdrObject::GetLogicRect() const
{
    return GetSnapRect();
}

void SdrObject::NbcSetLogicRect(const Rectangle& rRect)
{
    NbcSetSnapRect(rRect);
}

void SdrObject::AdjustToMaxRect( const Rectangle& rMaxRect, bool /* bShrinkOnly = false */ )
{
    SetLogicRect( rMaxRect );
}

void SdrObject::SetSnapRect(const Rectangle& rRect)
{
    Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetLastBoundRect();
    NbcSetSnapRect(rRect);
    SetChanged();
    BroadcastObjectChange();
    SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
}

void SdrObject::SetLogicRect(const Rectangle& rRect)
{
    Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetLastBoundRect();
    NbcSetLogicRect(rRect);
    SetChanged();
    BroadcastObjectChange();
    SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
}

long SdrObject::GetRotateAngle() const
{
    return 0;
}

long SdrObject::GetShearAngle(bool /*bVertical*/) const
{
    return 0;
}

sal_uInt32 SdrObject::GetSnapPointCount() const
{
    return GetPointCount();
}

Point SdrObject::GetSnapPoint(sal_uInt32 i) const
{
    return GetPoint(i);
}

sal_Bool SdrObject::IsPolyObj() const
{
    return false;
}

sal_uInt32 SdrObject::GetPointCount() const
{
    return 0L;
}

Point SdrObject::GetPoint(sal_uInt32 /*i*/) const
{
    return Point();
}

void SdrObject::SetPoint(const Point& rPnt, sal_uInt32 i)
{
    Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetLastBoundRect();
    NbcSetPoint(rPnt, i);
    SetChanged();
    BroadcastObjectChange();
    SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
}

void SdrObject::NbcSetPoint(const Point& /*rPnt*/, sal_uInt32 /*i*/)
{
}

bool SdrObject::HasTextEdit() const
{
    return false;
}

sal_Bool SdrObject::BegTextEdit(SdrOutliner& /*rOutl*/)
{
    return false;
}

void SdrObject::EndTextEdit(SdrOutliner& /*rOutl*/)
{
}

void SdrObject::SetOutlinerParaObject(OutlinerParaObject* pTextObject)
{
    Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetLastBoundRect();
    NbcSetOutlinerParaObject(pTextObject);
    SetChanged();
    BroadcastObjectChange();
    if (GetCurrentBoundRect()!=aBoundRect0) {
        SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
    }
}

void SdrObject::NbcSetOutlinerParaObject(OutlinerParaObject* /*pTextObject*/)
{
}

OutlinerParaObject* SdrObject::GetOutlinerParaObject() const
{
    return NULL;
}

void SdrObject::NbcReformatText()
{
}

void SdrObject::ReformatText()
{
    Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetLastBoundRect();
    NbcReformatText();
    SetChanged();
    BroadcastObjectChange();
    if (GetCurrentBoundRect()!=aBoundRect0) {
        SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
    }
}

void SdrObject::BurnInStyleSheetAttributes()
{
    GetProperties().ForceStyleToHardAttributes();
}

SdrObjUserData* SdrObject::ImpGetMacroUserData() const
{
    SdrObjUserData* pData=NULL;
    sal_uInt16 nAnz=GetUserDataCount();
    for (sal_uInt16 nNum=nAnz; nNum>0 && pData==NULL;) {
        nNum--;
        pData=GetUserData(nNum);
        if (!pData->HasMacro(this)) pData=NULL;
    }
    return pData;
}

bool SdrObject::HasMacro() const
{
    SdrObjUserData* pData=ImpGetMacroUserData();
    return pData!=NULL ? pData->HasMacro(this) : false;
}

SdrObject* SdrObject::CheckMacroHit(const SdrObjMacroHitRec& rRec) const
{
    SdrObjUserData* pData = ImpGetMacroUserData();

    if(pData)
    {
        return pData->CheckMacroHit(rRec, this);
    }

    if(rRec.pPageView)
    {
        return SdrObjectPrimitiveHit(*this, rRec.aPos, rRec.nTol, *rRec.pPageView, rRec.pVisiLayer, false);
    }

    return 0;
}

Pointer SdrObject::GetMacroPointer(const SdrObjMacroHitRec& rRec) const
{
    SdrObjUserData* pData=ImpGetMacroUserData();
    if (pData!=NULL) {
        return pData->GetMacroPointer(rRec,this);
    }
    return Pointer(POINTER_REFHAND);
}

void SdrObject::PaintMacro(OutputDevice& rOut, const Rectangle& rDirtyRect, const SdrObjMacroHitRec& rRec) const
{
    SdrObjUserData* pData=ImpGetMacroUserData();

    if(pData)
    {
        pData->PaintMacro(rOut,rDirtyRect,rRec,this);
    }
    else
    {
        const RasterOp eRop(rOut.GetRasterOp());
        const basegfx::B2DPolyPolygon aPolyPolygon(TakeXorPoly());
        const sal_uInt32 nCount(aPolyPolygon.count());

        rOut.SetLineColor(COL_BLACK);
        rOut.SetFillColor();
        rOut.SetRasterOp(ROP_INVERT);

        for(sal_uInt32 a(0); a < nCount; a++)
        {
            rOut.DrawPolyLine(aPolyPolygon.getB2DPolygon(a));
        }

        rOut.SetRasterOp(eRop);
    }
}

bool SdrObject::DoMacro(const SdrObjMacroHitRec& rRec)
{
    SdrObjUserData* pData=ImpGetMacroUserData();
    if (pData!=NULL) {
        return pData->DoMacro(rRec,this);
    }
    return false;
}

OUString SdrObject::GetMacroPopupComment(const SdrObjMacroHitRec& rRec) const
{
    SdrObjUserData* pData=ImpGetMacroUserData();
    if (pData!=NULL) {
        return pData->GetMacroPopupComment(rRec,this);
    }
    return OUString();
}

bool SdrObject::IsMacroHit(const SdrObjMacroHitRec& rRec) const
{
    return CheckMacroHit(rRec) != NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrObjGeoData* SdrObject::NewGeoData() const
{
    return new SdrObjGeoData;
}

void SdrObject::SaveGeoData(SdrObjGeoData& rGeo) const
{
    rGeo.aBoundRect    =GetCurrentBoundRect();
    rGeo.aAnchor       =aAnchor       ;
    rGeo.bMovProt      =bMovProt      ;
    rGeo.bSizProt      =bSizProt      ;
    rGeo.bNoPrint      =bNoPrint      ;
    rGeo.mbVisible     =mbVisible     ;
    rGeo.bClosedObj    =bClosedObj    ;
    rGeo.mnLayerID = mnLayerID;

    // user-defined glue points
    if (pPlusData!=NULL && pPlusData->pGluePoints!=NULL) {
        if (rGeo.pGPL!=NULL) {
            *rGeo.pGPL=*pPlusData->pGluePoints;
        } else {
            rGeo.pGPL=new SdrGluePointList(*pPlusData->pGluePoints);
        }
    } else {
        if (rGeo.pGPL!=NULL) {
            delete rGeo.pGPL;
            rGeo.pGPL=NULL;
        }
    }
}

void SdrObject::RestGeoData(const SdrObjGeoData& rGeo)
{
    SetRectsDirty();
    aOutRect      =rGeo.aBoundRect    ;
    aAnchor       =rGeo.aAnchor       ;
    bMovProt      =rGeo.bMovProt      ;
    bSizProt      =rGeo.bSizProt      ;
    bNoPrint      =rGeo.bNoPrint      ;
    mbVisible     =rGeo.mbVisible     ;
    bClosedObj    =rGeo.bClosedObj    ;
    mnLayerID = rGeo.mnLayerID;

    // user-defined glue points
    if (rGeo.pGPL!=NULL) {
        ImpForcePlusData();
        if (pPlusData->pGluePoints!=NULL) {
            *pPlusData->pGluePoints=*rGeo.pGPL;
        } else {
            pPlusData->pGluePoints=new SdrGluePointList(*rGeo.pGPL);
        }
    } else {
        if (pPlusData!=NULL && pPlusData->pGluePoints!=NULL) {
            delete pPlusData->pGluePoints;
            pPlusData->pGluePoints=NULL;
        }
    }
}

SdrObjGeoData* SdrObject::GetGeoData() const
{
    SdrObjGeoData* pGeo=NewGeoData();
    SaveGeoData(*pGeo);
    return pGeo;
}

void SdrObject::SetGeoData(const SdrObjGeoData& rGeo)
{
    Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetLastBoundRect();
    RestGeoData(rGeo);
    SetChanged();
    BroadcastObjectChange();
    SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// ItemSet access

const SfxItemSet& SdrObject::GetObjectItemSet() const
{
    return GetProperties().GetObjectItemSet();
}

const SfxItemSet& SdrObject::GetMergedItemSet() const
{
    return GetProperties().GetMergedItemSet();
}

void SdrObject::SetObjectItem(const SfxPoolItem& rItem)
{
    GetProperties().SetObjectItem(rItem);
}

void SdrObject::SetMergedItem(const SfxPoolItem& rItem)
{
    GetProperties().SetMergedItem(rItem);
}

void SdrObject::ClearMergedItem(const sal_uInt16 nWhich)
{
    GetProperties().ClearMergedItem(nWhich);
}

void SdrObject::SetObjectItemSet(const SfxItemSet& rSet)
{
    GetProperties().SetObjectItemSet(rSet);
}

void SdrObject::SetMergedItemSet(const SfxItemSet& rSet, bool bClearAllItems)
{
    GetProperties().SetMergedItemSet(rSet, bClearAllItems);
}

const SfxPoolItem& SdrObject::GetObjectItem(const sal_uInt16 nWhich) const
{
    return GetObjectItemSet().Get(nWhich);
}

SfxMapUnit SdrObject::GetObjectMapUnit() const
{
    SfxMapUnit aRetval(SFX_MAPUNIT_100TH_MM);
    SdrItemPool* pPool = GetObjectItemPool();

    if(pPool)
    {
        aRetval = pPool->GetMetric(0);
    }
    else
    {
        OSL_ENSURE(pPool, "SdrObjects always need a pool (!)");
    }

    return aRetval;
}

const SfxPoolItem& SdrObject::GetMergedItem(const sal_uInt16 nWhich) const
{
    return GetMergedItemSet().Get(nWhich);
}

void SdrObject::SetMergedItemSetAndBroadcast(const SfxItemSet& rSet, bool bClearAllItems)
{
    GetProperties().SetMergedItemSetAndBroadcast(rSet, bClearAllItems);
}

void SdrObject::ApplyNotPersistAttr(const SfxItemSet& rAttr)
{
    Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetLastBoundRect();
    NbcApplyNotPersistAttr(rAttr);
    SetChanged();
    BroadcastObjectChange();
    SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
}

void SdrObject::NbcApplyNotPersistAttr(const SfxItemSet& rAttr)
{
    const Rectangle& rSnap=GetSnapRect();
    const Rectangle& rLogic=GetLogicRect();
    Point aRef1(rSnap.Center());
    Point aRef2(aRef1); aRef2.Y()++;
    const SfxPoolItem *pPoolItem=NULL;
    if (rAttr.GetItemState(SDRATTR_TRANSFORMREF1X,true,&pPoolItem)==SFX_ITEM_SET) {
        aRef1.X()=((const SdrTransformRef1XItem*)pPoolItem)->GetValue();
    }
    if (rAttr.GetItemState(SDRATTR_TRANSFORMREF1Y,true,&pPoolItem)==SFX_ITEM_SET) {
        aRef1.Y()=((const SdrTransformRef1YItem*)pPoolItem)->GetValue();
    }
    if (rAttr.GetItemState(SDRATTR_TRANSFORMREF2X,true,&pPoolItem)==SFX_ITEM_SET) {
        aRef2.X()=((const SdrTransformRef2XItem*)pPoolItem)->GetValue();
    }
    if (rAttr.GetItemState(SDRATTR_TRANSFORMREF2Y,true,&pPoolItem)==SFX_ITEM_SET) {
        aRef2.Y()=((const SdrTransformRef2YItem*)pPoolItem)->GetValue();
    }

    Rectangle aNewSnap(rSnap);
    if (rAttr.GetItemState(SDRATTR_MOVEX,true,&pPoolItem)==SFX_ITEM_SET) {
        long n=((const SdrMoveXItem*)pPoolItem)->GetValue();
        aNewSnap.Move(n,0);
    }
    if (rAttr.GetItemState(SDRATTR_MOVEY,true,&pPoolItem)==SFX_ITEM_SET) {
        long n=((const SdrMoveYItem*)pPoolItem)->GetValue();
        aNewSnap.Move(0,n);
    }
    if (rAttr.GetItemState(SDRATTR_ONEPOSITIONX,true,&pPoolItem)==SFX_ITEM_SET) {
        long n=((const SdrOnePositionXItem*)pPoolItem)->GetValue();
        aNewSnap.Move(n-aNewSnap.Left(),0);
    }
    if (rAttr.GetItemState(SDRATTR_ONEPOSITIONY,true,&pPoolItem)==SFX_ITEM_SET) {
        long n=((const SdrOnePositionYItem*)pPoolItem)->GetValue();
        aNewSnap.Move(0,n-aNewSnap.Top());
    }
    if (rAttr.GetItemState(SDRATTR_ONESIZEWIDTH,true,&pPoolItem)==SFX_ITEM_SET) {
        long n=((const SdrOneSizeWidthItem*)pPoolItem)->GetValue();
        aNewSnap.Right()=aNewSnap.Left()+n;
    }
    if (rAttr.GetItemState(SDRATTR_ONESIZEHEIGHT,true,&pPoolItem)==SFX_ITEM_SET) {
        long n=((const SdrOneSizeHeightItem*)pPoolItem)->GetValue();
        aNewSnap.Bottom()=aNewSnap.Top()+n;
    }
    if (aNewSnap!=rSnap) {
        if (aNewSnap.GetSize()==rSnap.GetSize()) {
            NbcMove(Size(aNewSnap.Left()-rSnap.Left(),aNewSnap.Top()-rSnap.Top()));
        } else {
            NbcSetSnapRect(aNewSnap);
        }
    }

    if (rAttr.GetItemState(SDRATTR_SHEARANGLE,true,&pPoolItem)==SFX_ITEM_SET) {
        long n=((const SdrShearAngleItem*)pPoolItem)->GetValue();
        n-=GetShearAngle();
        if (n!=0) {
            double nTan=tan(n*nPi180);
            NbcShear(aRef1,n,nTan,false);
        }
    }
    if (rAttr.GetItemState(SDRATTR_ROTATEANGLE,true,&pPoolItem)==SFX_ITEM_SET) {
        long n=((const SdrRotateAngleItem*)pPoolItem)->GetValue();
        n-=GetRotateAngle();
        if (n!=0) {
            double nSin=sin(n*nPi180);
            double nCos=cos(n*nPi180);
            NbcRotate(aRef1,n,nSin,nCos);
        }
    }
    if (rAttr.GetItemState(SDRATTR_ROTATEONE,true,&pPoolItem)==SFX_ITEM_SET) {
        long n=((const SdrRotateOneItem*)pPoolItem)->GetValue();
        double nSin=sin(n*nPi180);
        double nCos=cos(n*nPi180);
        NbcRotate(aRef1,n,nSin,nCos);
    }
    if (rAttr.GetItemState(SDRATTR_HORZSHEARONE,true,&pPoolItem)==SFX_ITEM_SET) {
        long n=((const SdrHorzShearOneItem*)pPoolItem)->GetValue();
        double nTan=tan(n*nPi180);
        NbcShear(aRef1,n,nTan,false);
    }
    if (rAttr.GetItemState(SDRATTR_VERTSHEARONE,true,&pPoolItem)==SFX_ITEM_SET) {
        long n=((const SdrVertShearOneItem*)pPoolItem)->GetValue();
        double nTan=tan(n*nPi180);
        NbcShear(aRef1,n,nTan,true);
    }

    if (rAttr.GetItemState(SDRATTR_OBJMOVEPROTECT,true,&pPoolItem)==SFX_ITEM_SET) {
        bool b=((const SdrObjMoveProtectItem*)pPoolItem)->GetValue();
        SetMoveProtect(b);
    }
    if (rAttr.GetItemState(SDRATTR_OBJSIZEPROTECT,true,&pPoolItem)==SFX_ITEM_SET) {
        bool b=((const SdrObjSizeProtectItem*)pPoolItem)->GetValue();
        SetResizeProtect(b);
    }

    /* move protect always sets size protect */
    if( IsMoveProtect() )
        SetResizeProtect( true );

    if (rAttr.GetItemState(SDRATTR_OBJPRINTABLE,true,&pPoolItem)==SFX_ITEM_SET) {
        bool b=((const SdrObjPrintableItem*)pPoolItem)->GetValue();
        SetPrintable(b);
    }

    if (rAttr.GetItemState(SDRATTR_OBJVISIBLE,true,&pPoolItem)==SFX_ITEM_SET) {
        bool b=((const SdrObjVisibleItem*)pPoolItem)->GetValue();
        SetVisible(b);
    }

    SdrLayerID nLayer=SDRLAYER_NOTFOUND;
    if (rAttr.GetItemState(SDRATTR_LAYERID,true,&pPoolItem)==SFX_ITEM_SET) {
        nLayer=((const SdrLayerIdItem*)pPoolItem)->GetValue();
    }
    if (rAttr.GetItemState(SDRATTR_LAYERNAME,true,&pPoolItem)==SFX_ITEM_SET && pModel!=NULL) {
        XubString aLayerName=((const SdrLayerNameItem*)pPoolItem)->GetValue();
        const SdrLayerAdmin* pLayAd=pPage!=NULL ? &pPage->GetLayerAdmin() : pModel!=NULL ? &pModel->GetLayerAdmin() : NULL;
        if (pLayAd!=NULL) {
            const SdrLayer* pLayer=pLayAd->GetLayer(aLayerName, true);
            if (pLayer!=NULL) {
                nLayer=pLayer->GetID();
            }
        }

    }
    if (nLayer!=SDRLAYER_NOTFOUND) {
        NbcSetLayer(nLayer);
    }

    if (rAttr.GetItemState(SDRATTR_OBJECTNAME,true,&pPoolItem)==SFX_ITEM_SET) {
        XubString aName=((const SdrObjectNameItem*)pPoolItem)->GetValue();
        SetName(aName);
    }
    Rectangle aNewLogic(rLogic);
    if (rAttr.GetItemState(SDRATTR_LOGICSIZEWIDTH,true,&pPoolItem)==SFX_ITEM_SET) {
        long n=((const SdrLogicSizeWidthItem*)pPoolItem)->GetValue();
        aNewLogic.Right()=aNewLogic.Left()+n;
    }
    if (rAttr.GetItemState(SDRATTR_LOGICSIZEHEIGHT,true,&pPoolItem)==SFX_ITEM_SET) {
        long n=((const SdrLogicSizeHeightItem*)pPoolItem)->GetValue();
        aNewLogic.Bottom()=aNewLogic.Top()+n;
    }
    if (aNewLogic!=rLogic) {
        NbcSetLogicRect(aNewLogic);
    }
    Fraction aResizeX(1,1);
    Fraction aResizeY(1,1);
    if (rAttr.GetItemState(SDRATTR_RESIZEXONE,true,&pPoolItem)==SFX_ITEM_SET) {
        aResizeX*=((const SdrResizeXOneItem*)pPoolItem)->GetValue();
    }
    if (rAttr.GetItemState(SDRATTR_RESIZEYONE,true,&pPoolItem)==SFX_ITEM_SET) {
        aResizeY*=((const SdrResizeYOneItem*)pPoolItem)->GetValue();
    }
    if (aResizeX!=Fraction(1,1) || aResizeY!=Fraction(1,1)) {
        NbcResize(aRef1,aResizeX,aResizeY);
    }
}

static void lcl_SetItem(SfxItemSet& rAttr, bool bMerge, const SfxPoolItem& rItem)
{
    if (bMerge) rAttr.MergeValue(rItem,true);
    else rAttr.Put(rItem);
}

void SdrObject::TakeNotPersistAttr(SfxItemSet& rAttr, bool bMerge) const
{
    const Rectangle& rSnap=GetSnapRect();
    const Rectangle& rLogic=GetLogicRect();
    lcl_SetItem(rAttr,bMerge,SdrObjMoveProtectItem(IsMoveProtect()));
    lcl_SetItem(rAttr,bMerge,SdrObjSizeProtectItem(IsResizeProtect()));
    lcl_SetItem(rAttr,bMerge,SdrObjPrintableItem(IsPrintable()));
    lcl_SetItem(rAttr,bMerge,SdrObjVisibleItem(IsVisible()));
    lcl_SetItem(rAttr,bMerge,SdrRotateAngleItem(GetRotateAngle()));
    lcl_SetItem(rAttr,bMerge,SdrShearAngleItem(GetShearAngle()));
    lcl_SetItem(rAttr,bMerge,SdrOneSizeWidthItem(rSnap.GetWidth()-1));
    lcl_SetItem(rAttr,bMerge,SdrOneSizeHeightItem(rSnap.GetHeight()-1));
    lcl_SetItem(rAttr,bMerge,SdrOnePositionXItem(rSnap.Left()));
    lcl_SetItem(rAttr,bMerge,SdrOnePositionYItem(rSnap.Top()));
    if (rLogic.GetWidth()!=rSnap.GetWidth()) {
        lcl_SetItem(rAttr,bMerge,SdrLogicSizeWidthItem(rLogic.GetWidth()-1));
    }
    if (rLogic.GetHeight()!=rSnap.GetHeight()) {
        lcl_SetItem(rAttr,bMerge,SdrLogicSizeHeightItem(rLogic.GetHeight()-1));
    }
    XubString aName(GetName());

    if(aName.Len())
    {
        lcl_SetItem(rAttr, bMerge, SdrObjectNameItem(aName));
    }

    lcl_SetItem(rAttr,bMerge,SdrLayerIdItem(GetLayer()));
    const SdrLayerAdmin* pLayAd=pPage!=NULL ? &pPage->GetLayerAdmin() : pModel!=NULL ? &pModel->GetLayerAdmin() : NULL;
    if (pLayAd!=NULL) {
        const SdrLayer* pLayer=pLayAd->GetLayerPerID(GetLayer());
        if (pLayer!=NULL) {
            lcl_SetItem(rAttr,bMerge,SdrLayerNameItem(pLayer->GetName()));
        }
    }
    Point aRef1(rSnap.Center());
    Point aRef2(aRef1); aRef2.Y()++;
    lcl_SetItem(rAttr,bMerge,SdrTransformRef1XItem(aRef1.X()));
    lcl_SetItem(rAttr,bMerge,SdrTransformRef1YItem(aRef1.Y()));
    lcl_SetItem(rAttr,bMerge,SdrTransformRef2XItem(aRef2.X()));
    lcl_SetItem(rAttr,bMerge,SdrTransformRef2YItem(aRef2.Y()));
}

SfxStyleSheet* SdrObject::GetStyleSheet() const
{
    return GetProperties().GetStyleSheet();
}

void SdrObject::SetStyleSheet(SfxStyleSheet* pNewStyleSheet, bool bDontRemoveHardAttr)
{
    Rectangle aBoundRect0;

    if(pUserCall)
        aBoundRect0 = GetLastBoundRect();

    NbcSetStyleSheet(pNewStyleSheet, bDontRemoveHardAttr);
    SetChanged();
    BroadcastObjectChange();
    SendUserCall(SDRUSERCALL_CHGATTR, aBoundRect0);
}

void SdrObject::NbcSetStyleSheet(SfxStyleSheet* pNewStyleSheet, sal_Bool bDontRemoveHardAttr)
{
    // only allow graphic and presentation styles for shapes
    if( pNewStyleSheet && (pNewStyleSheet->GetFamily() == SFX_STYLE_FAMILY_PARA) && (pNewStyleSheet->GetFamily() == SFX_STYLE_FAMILY_PAGE) )
        return;

    GetProperties().SetStyleSheet(pNewStyleSheet, bDontRemoveHardAttr);
}

// Broadcasting while setting attributes is managed by the AttrObj.
////////////////////////////////////////////////////////////////////////////////////////////////////

bool SdrObject::IsNode() const
{
    return true;
}

SdrGluePoint SdrObject::GetVertexGluePoint(sal_uInt16 nPosNum) const
{
    // #i41936# Use SnapRect for default GluePoints
    const Rectangle aR(GetSnapRect());
    Point aPt;

    switch(nPosNum)
    {
        case 0 : aPt = aR.TopCenter();    break;
        case 1 : aPt = aR.RightCenter();  break;
        case 2 : aPt = aR.BottomCenter(); break;
        case 3 : aPt = aR.LeftCenter();   break;
    }

    aPt -= aR.Center();
    SdrGluePoint aGP(aPt);
    aGP.SetPercent(false);

    return aGP;
}

SdrGluePoint SdrObject::GetCornerGluePoint(sal_uInt16 nPosNum) const
{
    Rectangle aR(GetCurrentBoundRect());
    Point aPt;
    switch (nPosNum) {
        case 0 : aPt=aR.TopLeft();     break;
        case 1 : aPt=aR.TopRight();    break;
        case 2 : aPt=aR.BottomRight(); break;
        case 3 : aPt=aR.BottomLeft();  break;
    }
    aPt-=GetSnapRect().Center();
    SdrGluePoint aGP(aPt);
    aGP.SetPercent(false);
    return aGP;
}

const SdrGluePointList* SdrObject::GetGluePointList() const
{
    if (pPlusData!=NULL) return pPlusData->pGluePoints;
    return NULL;
}


SdrGluePointList* SdrObject::ForceGluePointList()
{
    ImpForcePlusData();
    if (pPlusData->pGluePoints==NULL) {
        pPlusData->pGluePoints=new SdrGluePointList;
    }
    return pPlusData->pGluePoints;
}

void SdrObject::SetGlueReallyAbsolute(bool bOn)
{
    // First a const call to see whether there are any glue points.
    // Force const call!
    if (GetGluePointList()!=NULL) {
        SdrGluePointList* pGPL=ForceGluePointList();
        pGPL->SetReallyAbsolute(bOn,*this);
    }
}

void SdrObject::NbcRotateGluePoints(const Point& rRef, long nWink, double sn, double cs)
{
    // First a const call to see whether there are any glue points.
    // Force const call!
    if (GetGluePointList()!=NULL) {
        SdrGluePointList* pGPL=ForceGluePointList();
        pGPL->Rotate(rRef,nWink,sn,cs,this);
    }
}

void SdrObject::NbcMirrorGluePoints(const Point& rRef1, const Point& rRef2)
{
    // First a const call to see whether there are any glue points.
    // Force const call!
    if (GetGluePointList()!=NULL) {
        SdrGluePointList* pGPL=ForceGluePointList();
        pGPL->Mirror(rRef1,rRef2,this);
    }
}

void SdrObject::NbcShearGluePoints(const Point& rRef, long nWink, double tn, bool bVShear)
{
    // First a const call to see whether there are any glue points.
    // Force const call!
    if (GetGluePointList()!=NULL) {
        SdrGluePointList* pGPL=ForceGluePointList();
        pGPL->Shear(rRef,nWink,tn,bVShear,this);
    }
}

bool SdrObject::IsEdge() const
{
    return false;
}

void SdrObject::ConnectToNode(bool /*bTail1*/, SdrObject* /*pObj*/)
{
}

void SdrObject::DisconnectFromNode(bool /*bTail1*/)
{
}

SdrObject* SdrObject::GetConnectedNode(bool /*bTail1*/) const
{
    return NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void extractLineContourFromPrimitive2DSequence(
    const drawinglayer::primitive2d::Primitive2DSequence& rxSequence,
    basegfx::B2DPolygonVector& rExtractedHairlines,
    basegfx::B2DPolyPolygonVector& rExtractedLineFills)
{
    rExtractedHairlines.clear();
    rExtractedLineFills.clear();

    if(rxSequence.hasElements())
    {
        // use neutral ViewInformation
        const drawinglayer::geometry::ViewInformation2D aViewInformation2D;

        // create extractor, process and get result
        drawinglayer::processor2d::LineGeometryExtractor2D aExtractor(aViewInformation2D);
        aExtractor.process(rxSequence);

        // copy line results
        rExtractedHairlines = aExtractor.getExtractedHairlines();

        // copy fill rsults
        rExtractedLineFills = aExtractor.getExtractedLineFills();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrObject* SdrObject::ImpConvertToContourObj(SdrObject* pRet, bool bForceLineDash) const
{
    bool bNoChange(true);

    if(pRet->LineGeometryUsageIsNecessary())
    {
        basegfx::B2DPolyPolygon aMergedLineFillPolyPolygon;
        basegfx::B2DPolyPolygon aMergedHairlinePolyPolygon;
        const drawinglayer::primitive2d::Primitive2DSequence xSequence(pRet->GetViewContact().getViewIndependentPrimitive2DSequence());

        if(xSequence.hasElements())
        {
            basegfx::B2DPolygonVector aExtractedHairlines;
            basegfx::B2DPolyPolygonVector aExtractedLineFills;

            extractLineContourFromPrimitive2DSequence(xSequence, aExtractedHairlines, aExtractedLineFills);

            if(!aExtractedHairlines.empty())
            {
                // for SdrObject creation, just copy all to a single Hairline-PolyPolygon
                for(sal_uInt32 a(0); a < aExtractedHairlines.size(); a++)
                {
                    aMergedHairlinePolyPolygon.append(aExtractedHairlines[a]);
                }
            }

            // check for fill rsults
            if(!aExtractedLineFills.empty())
            {
                // merge to a single PolyPolygon (OR)
                aMergedLineFillPolyPolygon = basegfx::tools::mergeToSinglePolyPolygon(aExtractedLineFills);
            }
        }

        if(aMergedLineFillPolyPolygon.count() || (bForceLineDash && aMergedHairlinePolyPolygon.count()))
        {
            SfxItemSet aSet(pRet->GetMergedItemSet());
            XFillStyle eOldFillStyle = ((const XFillStyleItem&)(aSet.Get(XATTR_FILLSTYLE))).GetValue();
            SdrPathObj* aLinePolygonPart = NULL;
            SdrPathObj* aLineHairlinePart = NULL;
            bool bBuildGroup(false);

            if(aMergedLineFillPolyPolygon.count())
            {
                // create SdrObject for filled line geometry
                aLinePolygonPart = new SdrPathObj(OBJ_PATHFILL, aMergedLineFillPolyPolygon);
                aLinePolygonPart->SetModel(pRet->GetModel());

                // correct item properties
                aSet.Put(XLineWidthItem(0L));
                aSet.Put(XLineStyleItem(XLINE_NONE));
                Color aColorLine = ((const XLineColorItem&)(aSet.Get(XATTR_LINECOLOR))).GetColorValue();
                sal_uInt16 nTransLine = ((const XLineTransparenceItem&)(aSet.Get(XATTR_LINETRANSPARENCE))).GetValue();
                aSet.Put(XFillColorItem(XubString(), aColorLine));
                aSet.Put(XFillStyleItem(XFILL_SOLID));
                aSet.Put(XFillTransparenceItem(nTransLine));

                aLinePolygonPart->SetMergedItemSet(aSet);
            }

            if(aMergedHairlinePolyPolygon.count())
            {
                // create SdrObject for hairline geometry
                // OBJ_PATHLINE is necessary here, not OBJ_PATHFILL. This is intended
                // to get a non-filled object. If the poly is closed, the PathObj takes care for
                // the correct closed state.
                aLineHairlinePart = new SdrPathObj(OBJ_PATHLINE, aMergedHairlinePolyPolygon);
                aLineHairlinePart->SetModel(pRet->GetModel());

                aSet.Put(XLineWidthItem(0L));
                aSet.Put(XFillStyleItem(XFILL_NONE));
                aSet.Put(XLineStyleItem(XLINE_SOLID));

                // it is also necessary to switch off line start and ends here
                aSet.Put(XLineStartWidthItem(0));
                aSet.Put(XLineEndWidthItem(0));

                aLineHairlinePart->SetMergedItemSet(aSet);

                if(aLinePolygonPart)
                {
                    bBuildGroup = true;
                }
            }

            // check if original geometry should be added (e.g. filled and closed)
            bool bAddOriginalGeometry(false);
            SdrPathObj* pPath = PTR_CAST(SdrPathObj, pRet);

            if(pPath && pPath->IsClosed())
            {
                if(eOldFillStyle != XFILL_NONE)
                {
                    bAddOriginalGeometry = true;
                }
            }

            // do we need a group?
            if(bBuildGroup || bAddOriginalGeometry)
            {
                SdrObject* pGroup = new SdrObjGroup;
                pGroup->SetModel(pRet->GetModel());

                if(bAddOriginalGeometry)
                {
                    // Add a clone of the original geometry.
                    aSet.ClearItem();
                    aSet.Put(pRet->GetMergedItemSet());
                    aSet.Put(XLineStyleItem(XLINE_NONE));
                    aSet.Put(XLineWidthItem(0L));

                    SdrObject* pClone = pRet->Clone();

                    pClone->SetModel(pRet->GetModel());
                    pClone->SetMergedItemSet(aSet);

                    pGroup->GetSubList()->NbcInsertObject(pClone);
                }

                if(aLinePolygonPart)
                {
                    pGroup->GetSubList()->NbcInsertObject(aLinePolygonPart);
                }

                if(aLineHairlinePart)
                {
                    pGroup->GetSubList()->NbcInsertObject(aLineHairlinePart);
                }

                pRet = pGroup;

                // be more careful with the state describing bool
                bNoChange = false;
            }
            else
            {
                if(aLinePolygonPart)
                {
                    pRet = aLinePolygonPart;
                    // be more careful with the state describing bool
                    bNoChange = false;
                }
                else if(aLineHairlinePart)
                {
                    pRet = aLineHairlinePart;
                    // be more careful with the state describing bool
                    bNoChange = false;
                }
            }
        }
    }

    if(bNoChange)
    {
        // due to current method usage, create and return a clone when nothing has changed
        SdrObject* pClone = pRet->Clone();
        pClone->SetModel(pRet->GetModel());
        pRet = pClone;
    }

    return pRet;
}

bool SdrObject::IsVirtualObj() const
{
    return bVirtObj;
}

bool SdrObject::IsClosedObj() const
{
    return bClosedObj;
}

bool SdrObject::IsEdgeObj() const
{
    return bIsEdge;
}

bool SdrObject::Is3DObj() const
{
    return bIs3DObj;
}

bool SdrObject::IsUnoObj() const
{
    return bIsUnoObj;
}

void SdrObject::SetMarkProtect(bool bProt)
{
    bMarkProt = bProt;
}

bool SdrObject::IsMarkProtect() const
{
    return bMarkProt;
}

bool SdrObject::IsInserted() const
{
    return bInserted;
}

bool SdrObject::IsMoveProtect() const
{
    return bMovProt;
}

bool SdrObject::IsResizeProtect() const
{
    return bSizProt;
}

bool SdrObject::IsPrintable() const
{
    return !bNoPrint;
}

bool SdrObject::IsVisible() const
{
    return mbVisible;
}

void SdrObject::SetEmptyPresObj(bool bEpt)
{
    bEmptyPresObj = bEpt;
}

bool SdrObject::IsEmptyPresObj() const
{
    return bEmptyPresObj;
}

void SdrObject::SetNotVisibleAsMaster(bool bFlg)
{
    bNotVisibleAsMaster=bFlg;
}

bool SdrObject::IsNotVisibleAsMaster() const
{
    return bNotVisibleAsMaster;
}

bool SdrObject::LineIsOutsideGeometry() const
{
    return mbLineIsOutsideGeometry;
}

bool SdrObject::DoesSupportTextIndentingOnLineWidthChange() const
{
    return mbSupportTextIndentingOnLineWidthChange;
}

// convert this path object to contour object, even when it is a group
SdrObject* SdrObject::ConvertToContourObj(SdrObject* pRet, bool bForceLineDash) const
{
    if(pRet->ISA(SdrObjGroup))
    {
        SdrObjList* pObjList2 = pRet->GetSubList();
        SdrObject* pGroup = new SdrObjGroup;
        pGroup->SetModel(pRet->GetModel());

        for(sal_uInt32 a=0;a<pObjList2->GetObjCount();a++)
        {
            SdrObject* pIterObj = pObjList2->GetObj(a);
            pGroup->GetSubList()->NbcInsertObject(ConvertToContourObj(pIterObj, bForceLineDash));
        }

        pRet = pGroup;
    }
    else
    {
        if(pRet && pRet->ISA(SdrPathObj))
        {
            SdrPathObj* pPathObj = (SdrPathObj*)pRet;

            // bezier geometry got created, even for straight edges since the given
            // object is a result of DoConvertToPolyObj. For conversion to contour
            // this is not really needed and can be reduced again AFAP
            pPathObj->SetPathPoly(basegfx::tools::simplifyCurveSegments(pPathObj->GetPathPoly()));
        }

        pRet = ImpConvertToContourObj(pRet, bForceLineDash);
    }

    // #i73441# preserve LayerID
    if(pRet && pRet->GetLayer() != GetLayer())
    {
        pRet->SetLayer(GetLayer());
    }

    return pRet;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrObject* SdrObject::ConvertToPolyObj(bool bBezier, bool bLineToArea) const
{
    SdrObject* pRet = DoConvertToPolyObj(bBezier, true);

    if(pRet && bLineToArea)
    {
        SdrObject* pNewRet = ConvertToContourObj(pRet);
        delete pRet;
        pRet = pNewRet;
    }

    // #i73441# preserve LayerID
    if(pRet && pRet->GetLayer() != GetLayer())
    {
        pRet->SetLayer(GetLayer());
    }

    return pRet;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrObject* SdrObject::DoConvertToPolyObj(sal_Bool /*bBezier*/, bool /*bAddText*/) const
{
    return NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrObject::SetInserted(bool bIns)
{
    if (bIns!=IsInserted()) {
        bInserted=bIns;
        Rectangle aBoundRect0(GetLastBoundRect());
        if (bIns) SendUserCall(SDRUSERCALL_INSERTED,aBoundRect0);
        else SendUserCall(SDRUSERCALL_REMOVED,aBoundRect0);

        if (pPlusData!=NULL && pPlusData->pBroadcast!=NULL) {
            SdrHint aHint(*this);
            aHint.SetKind(bIns?HINT_OBJINSERTED:HINT_OBJREMOVED);
            pPlusData->pBroadcast->Broadcast(aHint);
        }
    }
}

void SdrObject::SetMoveProtect(bool bProt)
{
    if(IsMoveProtect() != bProt)
    {
        // #i77187# secured and simplified
        bMovProt = bProt;
        SetChanged();
        BroadcastObjectChange();
    }
}

void SdrObject::SetResizeProtect(bool bProt)
{
    if(IsResizeProtect() != bProt)
    {
        // #i77187# secured and simplified
        bSizProt = bProt;
        SetChanged();
        BroadcastObjectChange();
    }
}

void SdrObject::SetPrintable(bool bPrn)
{
    if( bPrn == bNoPrint )
    {
        bNoPrint=!bPrn;
        SetChanged();
        if (IsInserted() && pModel!=NULL)
        {
            SdrHint aHint(*this);
            pModel->Broadcast(aHint);
        }
    }
}

void SdrObject::SetVisible(bool bVisible)
{
    if( bVisible != mbVisible )
    {
        mbVisible = bVisible;
        SetChanged();
        if (IsInserted() && pModel!=NULL)
        {
            SdrHint aHint(*this);
            pModel->Broadcast(aHint);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

sal_uInt16 SdrObject::GetUserDataCount() const
{
    if (pPlusData==NULL || pPlusData->pUserDataList==NULL) return 0;
    return pPlusData->pUserDataList->GetUserDataCount();
}

SdrObjUserData* SdrObject::GetUserData(sal_uInt16 nNum) const
{
    if (pPlusData==NULL || pPlusData->pUserDataList==NULL) return NULL;
    return pPlusData->pUserDataList->GetUserData(nNum);
}

void SdrObject::AppendUserData(SdrObjUserData* pData)
{
    if (!pData)
    {
        OSL_FAIL("SdrObject::AppendUserData(): pData is NULL pointer.");
        return;
    }

    ImpForcePlusData();
    if (!pPlusData->pUserDataList)
        pPlusData->pUserDataList = new SdrObjUserDataList;

    pPlusData->pUserDataList->AppendUserData(pData);
}

void SdrObject::DeleteUserData(sal_uInt16 nNum)
{
    sal_uInt16 nAnz=GetUserDataCount();
    if (nNum<nAnz) {
        pPlusData->pUserDataList->DeleteUserData(nNum);
        if (nAnz==1)  {
            delete pPlusData->pUserDataList;
            pPlusData->pUserDataList=NULL;
        }
    } else {
        OSL_FAIL("SdrObject::DeleteUserData(): Invalid Index.");
    }
}

void SdrObject::SetUserCall(SdrObjUserCall* pUser)
{
    pUserCall = pUser;
}

SdrObjUserCall* SdrObject::GetUserCall() const
{
    return pUserCall;
}

void SdrObject::SendUserCall(SdrUserCallType eUserCall, const Rectangle& rBoundRect) const
{
    SdrObjGroup* pGroup = NULL;

    if( pObjList && pObjList->GetListKind() == SDROBJLIST_GROUPOBJ )
        pGroup = (SdrObjGroup*) pObjList->GetOwnerObj();

    if ( pUserCall )
    {
        pUserCall->Changed( *this, eUserCall, rBoundRect );
    }

    while( pGroup )
    {
        // broadcast to group
        if( pGroup->GetUserCall() )
        {
            SdrUserCallType eChildUserType = SDRUSERCALL_CHILD_CHGATTR;

            switch( eUserCall )
            {
                case SDRUSERCALL_MOVEONLY:
                    eChildUserType = SDRUSERCALL_CHILD_MOVEONLY;
                break;

                case SDRUSERCALL_RESIZE:
                    eChildUserType = SDRUSERCALL_CHILD_RESIZE;
                break;

                case SDRUSERCALL_CHGATTR:
                    eChildUserType = SDRUSERCALL_CHILD_CHGATTR;
                break;

                case SDRUSERCALL_DELETE:
                    eChildUserType = SDRUSERCALL_CHILD_DELETE;
                break;

                case SDRUSERCALL_COPY:
                    eChildUserType = SDRUSERCALL_CHILD_COPY;
                break;

                case SDRUSERCALL_INSERTED:
                    eChildUserType = SDRUSERCALL_CHILD_INSERTED;
                break;

                case SDRUSERCALL_REMOVED:
                    eChildUserType = SDRUSERCALL_CHILD_REMOVED;
                break;

                default: break;
            }

            pGroup->GetUserCall()->Changed( *this, eChildUserType, rBoundRect );
        }

        if( pGroup->GetObjList()                                       &&
            pGroup->GetObjList()->GetListKind() == SDROBJLIST_GROUPOBJ &&
            pGroup != (SdrObjGroup*) pObjList->GetOwnerObj() )
            pGroup = (SdrObjGroup*) pObjList->GetOwnerObj();
        else
            pGroup = NULL;
    }

    // notify our UNO shape listeners
    switch ( eUserCall )
    {
    case SDRUSERCALL_RESIZE:
        notifyShapePropertyChange( ::svx::eShapeSize );
        // fall through - RESIZE might also imply a change of the position
    case SDRUSERCALL_MOVEONLY:
        notifyShapePropertyChange( ::svx::eShapePosition );
        break;
    default:
        // not interested in
        break;
    }
}

// change ItemPool for this object
void SdrObject::MigrateItemPool(SfxItemPool* pSrcPool, SfxItemPool* pDestPool, SdrModel* pNewModel)
{
    if(pSrcPool && pDestPool && (pSrcPool != pDestPool))
    {
        GetProperties().MoveToItemPool(pSrcPool, pDestPool, pNewModel);
    }
}

void SdrObject::impl_setUnoShape( const uno::Reference< uno::XInterface >& _rxUnoShape )
{
    const uno::Reference< uno::XInterface>& xOldUnoShape( maWeakUnoShape );
    // the UNO shape would be gutted by the following code; return early
    if ( _rxUnoShape == xOldUnoShape )
    {
        if ( !xOldUnoShape.is() )
        {
            // make sure there is no stale impl. pointer if the UNO
            // shape was destroyed meanwhile (remember we only hold weak
            // reference to it!)
            mpSvxShape = 0;
        }
        return;
    }

    bool bTransferOwnership( false );
    if ( xOldUnoShape.is() )
    {
        bTransferOwnership = mpSvxShape->HasSdrObjectOwnership();
        // Remove yourself from the current UNO shape. Its destructor
        // will reset our UNO shape otherwise.
        mpSvxShape->InvalidateSdrObject();
    }

    maWeakUnoShape = _rxUnoShape;
    mpSvxShape = SvxShape::getImplementation( _rxUnoShape );

    // I think this may never happen... But I am not sure enough .-)
    if ( bTransferOwnership )
    {
        if ( _rxUnoShape.is() )
            mpSvxShape->TakeSdrObjectOwnership();
        SAL_WARN( "svx.uno", "a UNO shape took over an SdrObject previously owned by another UNO shape!");
    }
}

/** only for internal use! */
SvxShape* SdrObject::getSvxShape()
{
    DBG_TESTSOLARMUTEX();
        // retrieving the impl pointer and subsequently using it is not thread-safe, of course, so it needs to be
        // guarded by the SolarMutex

    uno::Reference< uno::XInterface > xShape( maWeakUnoShape );
#if OSL_DEBUG_LEVEL > 0
    OSL_ENSURE( !( !xShape.is() && mpSvxShape ),
        "SdrObject::getSvxShape: still having IMPL-Pointer to dead object!" );
#endif
    //#113608#, make sure mpSvxShape is always synchronized with maWeakUnoShape
    if ( mpSvxShape && !xShape.is() )
        mpSvxShape = NULL;

    return mpSvxShape;
}

::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SdrObject::getUnoShape()
{
    // try weak reference first
    uno::Reference< uno::XInterface > xShape( getWeakUnoShape() );
    if( !xShape.is() )
    {
        OSL_ENSURE( mpSvxShape == NULL, "SdrObject::getUnoShape: XShape already dead, but still an IMPL pointer!" );
        if ( pPage )
        {
            uno::Reference< uno::XInterface > xPage( pPage->getUnoPage() );
            if( xPage.is() )
            {
                SvxDrawPage* pDrawPage = SvxDrawPage::getImplementation(xPage);
                if( pDrawPage )
                {
                    // create one
                    xShape = pDrawPage->_CreateShape( this );
                    impl_setUnoShape( xShape );
                }
            }
        }
        else
        {
            mpSvxShape = SvxDrawPage::CreateShapeByTypeAndInventor( GetObjIdentifier(), GetObjInventor(), this, NULL );
            maWeakUnoShape = xShape = static_cast< ::cppu::OWeakObject* >( mpSvxShape );
        }
    }

    return xShape;
}

void SdrObject::setUnoShape(const uno::Reference<uno::XInterface >& _rxUnoShape)
{
    impl_setUnoShape( _rxUnoShape );
}

::svx::PropertyChangeNotifier& SdrObject::getShapePropertyChangeNotifier()
{
    DBG_TESTSOLARMUTEX();

    SvxShape* pSvxShape = getSvxShape();
    ENSURE_OR_THROW( pSvxShape, "no SvxShape, yet!" );
    return pSvxShape->getShapePropertyChangeNotifier();
}

void SdrObject::notifyShapePropertyChange( const ::svx::ShapeProperty _eProperty ) const
{
    DBG_TESTSOLARMUTEX();

    SvxShape* pSvxShape = const_cast< SdrObject* >( this )->getSvxShape();
    if ( pSvxShape )
        return pSvxShape->getShapePropertyChangeNotifier().notifyPropertyChange( _eProperty );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// transformation interface for StarOfficeAPI. This implements support for
// homogeneous 3x3 matrices containing the transformation of the SdrObject. At the
// moment it contains a shearX, rotation and translation, but for setting all linear
// transforms like Scale, ShearX, ShearY, Rotate and Translate are supported.
//
////////////////////////////////////////////////////////////////////////////////////////////////////
// gets base transformation and rectangle of object. If it's an SdrPathObj it fills the PolyPolygon
// with the base geometry and returns TRUE. Otherwise it returns FALSE.
sal_Bool SdrObject::TRGetBaseGeometry(basegfx::B2DHomMatrix& rMatrix, basegfx::B2DPolyPolygon& /*rPolyPolygon*/) const
{
    // any kind of SdrObject, just use SnapRect
    Rectangle aRectangle(GetSnapRect());

    // convert to transformation values
    basegfx::B2DTuple aScale(aRectangle.GetWidth(), aRectangle.GetHeight());
    basegfx::B2DTuple aTranslate(aRectangle.Left(), aRectangle.Top());

    // position maybe relative to anchorpos, convert
    if( pModel && pModel->IsWriter() )
    {
        if(GetAnchorPos().X() || GetAnchorPos().Y())
        {
            aTranslate -= basegfx::B2DTuple(GetAnchorPos().X(), GetAnchorPos().Y());
        }
    }

    // force MapUnit to 100th mm
    const SfxMapUnit eMapUnit(GetObjectMapUnit());
    if(eMapUnit != SFX_MAPUNIT_100TH_MM)
    {
        switch(eMapUnit)
        {
            case SFX_MAPUNIT_TWIP :
            {
                // position
                aTranslate.setX(ImplTwipsToMM(aTranslate.getX()));
                aTranslate.setY(ImplTwipsToMM(aTranslate.getY()));

                // size
                aScale.setX(ImplTwipsToMM(aScale.getX()));
                aScale.setY(ImplTwipsToMM(aScale.getY()));

                break;
            }
            default:
            {
                OSL_FAIL("TRGetBaseGeometry: Missing unit translation to 100th mm!");
            }
        }
    }

    // build matrix
    rMatrix = basegfx::tools::createScaleTranslateB2DHomMatrix(aScale, aTranslate);

    return false;
}

// sets the base geometry of the object using infos contained in the homogeneous 3x3 matrix.
// If it's an SdrPathObj it will use the provided geometry information. The Polygon has
// to use (0,0) as upper left and will be scaled to the given size in the matrix.
void SdrObject::TRSetBaseGeometry(const basegfx::B2DHomMatrix& rMatrix, const basegfx::B2DPolyPolygon& /*rPolyPolygon*/)
{
    // break up matrix
    basegfx::B2DTuple aScale;
    basegfx::B2DTuple aTranslate;
    double fRotate, fShearX;
    rMatrix.decompose(aScale, aTranslate, fRotate, fShearX);

    // #i75086# Old DrawingLayer (GeoStat and geometry) does not support holding negative scalings
    // in X and Y which equal a 180 degree rotation. Recognize it and react accordingly
    if(basegfx::fTools::less(aScale.getX(), 0.0) && basegfx::fTools::less(aScale.getY(), 0.0))
    {
        aScale.setX(fabs(aScale.getX()));
        aScale.setY(fabs(aScale.getY()));
        fRotate = fmod(fRotate + F_PI, F_2PI);
    }

    // force metric to pool metric
    const SfxMapUnit eMapUnit(GetObjectMapUnit());
    if(eMapUnit != SFX_MAPUNIT_100TH_MM)
    {
        switch(eMapUnit)
        {
            case SFX_MAPUNIT_TWIP :
            {
                // position
                aTranslate.setX(ImplMMToTwips(aTranslate.getX()));
                aTranslate.setY(ImplMMToTwips(aTranslate.getY()));

                // size
                aScale.setX(ImplMMToTwips(aScale.getX()));
                aScale.setY(ImplMMToTwips(aScale.getY()));

                break;
            }
            default:
            {
                OSL_FAIL("TRSetBaseGeometry: Missing unit translation to PoolMetric!");
            }
        }
    }

    // if anchor is used, make position relative to it
    if( pModel && pModel->IsWriter() )
    {
        if(GetAnchorPos().X() || GetAnchorPos().Y())
        {
            aTranslate += basegfx::B2DTuple(GetAnchorPos().X(), GetAnchorPos().Y());
        }
    }

    // build BaseRect
    Point aPoint(FRound(aTranslate.getX()), FRound(aTranslate.getY()));
    Rectangle aBaseRect(aPoint, Size(FRound(aScale.getX()), FRound(aScale.getY())));

    // set BaseRect
    SetSnapRect(aBaseRect);
}

// Give info if object is in destruction
bool SdrObject::IsInDestruction() const
{
    if(pModel)
        return pModel->IsInDestruction();
    return false;
}

// return if fill is != XFILL_NONE
bool SdrObject::HasFillStyle() const
{
    return (((const XFillStyleItem&)GetObjectItem(XATTR_FILLSTYLE)).GetValue() != XFILL_NONE);
}

bool SdrObject::HasLineStyle() const
{
    return (((const XLineStyleItem&)GetObjectItem(XATTR_LINESTYLE)).GetValue() != XLINE_NONE);
}


// #i52224#
// on import of OLE object from MS documents the BLIP size might be retrieved,
// the following four methods are used to control it;
// usually this data makes no sence after the import is finished, since the object
// might be resized

Rectangle SdrObject::GetBLIPSizeRectangle() const
{
    return maBLIPSizeRectangle;
}

void SdrObject::SetBLIPSizeRectangle( const Rectangle& aRect )
{
    maBLIPSizeRectangle = aRect;
}

void SdrObject::SetContextWritingMode( const sal_Int16 /*_nContextWritingMode*/ )
{
    // this base class does not support different writing modes, so ignore the call
}

void SdrObject::SetDoNotInsertIntoPageAutomatically(const bool bSet)
{
    mbDoNotInsertIntoPageAutomatically = bSet;
}

bool SdrObject::IsDoNotInsertIntoPageAutomatically() const
{
    return mbDoNotInsertIntoPageAutomatically;
}

// #i121917#
bool SdrObject::HasText() const
{
    return false;
}

SdrObjFactory::SdrObjFactory(sal_uInt32 nInvent, sal_uInt16 nIdent, SdrPage* pNewPage, SdrModel* pNewModel)
{
    nInventor=nInvent;
    nIdentifier=nIdent;
    pNewObj=NULL;
    pPage=pNewPage;
    pModel=pNewModel;
    pObj=NULL;
    pNewData=NULL;
}

SdrObject* SdrObjFactory::MakeNewObject(sal_uInt32 nInvent, sal_uInt16 nIdent, SdrPage* pPage, SdrModel* pModel)
{
    if(pModel == NULL && pPage != NULL)
        pModel = pPage->GetModel();
    SdrObject* pObj = NULL;

    if(nInvent == SdrInventor)
    {
        switch (nIdent)
        {
            case sal_uInt16(OBJ_NONE       ): pObj=new SdrObject;                   break;
            case sal_uInt16(OBJ_GRUP       ): pObj=new SdrObjGroup;                 break;
            case sal_uInt16(OBJ_LINE       ): pObj=new SdrPathObj(OBJ_LINE       ); break;
            case sal_uInt16(OBJ_POLY       ): pObj=new SdrPathObj(OBJ_POLY       ); break;
            case sal_uInt16(OBJ_PLIN       ): pObj=new SdrPathObj(OBJ_PLIN       ); break;
            case sal_uInt16(OBJ_PATHLINE   ): pObj=new SdrPathObj(OBJ_PATHLINE   ); break;
            case sal_uInt16(OBJ_PATHFILL   ): pObj=new SdrPathObj(OBJ_PATHFILL   ); break;
            case sal_uInt16(OBJ_FREELINE   ): pObj=new SdrPathObj(OBJ_FREELINE   ); break;
            case sal_uInt16(OBJ_FREEFILL   ): pObj=new SdrPathObj(OBJ_FREEFILL   ); break;
            case sal_uInt16(OBJ_PATHPOLY   ): pObj=new SdrPathObj(OBJ_POLY       ); break;
            case sal_uInt16(OBJ_PATHPLIN   ): pObj=new SdrPathObj(OBJ_PLIN       ); break;
            case sal_uInt16(OBJ_EDGE       ): pObj=new SdrEdgeObj;                  break;
            case sal_uInt16(OBJ_RECT       ): pObj=new SdrRectObj;                  break;
            case sal_uInt16(OBJ_CIRC       ): pObj=new SdrCircObj(OBJ_CIRC       ); break;
            case sal_uInt16(OBJ_SECT       ): pObj=new SdrCircObj(OBJ_SECT       ); break;
            case sal_uInt16(OBJ_CARC       ): pObj=new SdrCircObj(OBJ_CARC       ); break;
            case sal_uInt16(OBJ_CCUT       ): pObj=new SdrCircObj(OBJ_CCUT       ); break;
            case sal_uInt16(OBJ_TEXT       ): pObj=new SdrRectObj(OBJ_TEXT       ); break;
            case sal_uInt16(OBJ_TEXTEXT    ): pObj=new SdrRectObj(OBJ_TEXTEXT    ); break;
            case sal_uInt16(OBJ_TITLETEXT  ): pObj=new SdrRectObj(OBJ_TITLETEXT  ); break;
            case sal_uInt16(OBJ_OUTLINETEXT): pObj=new SdrRectObj(OBJ_OUTLINETEXT); break;
            case sal_uInt16(OBJ_MEASURE    ): pObj=new SdrMeasureObj;               break;
            case sal_uInt16(OBJ_GRAF       ): pObj=new SdrGrafObj;                  break;
            case sal_uInt16(OBJ_OLE2       ): pObj=new SdrOle2Obj;                  break;
            case sal_uInt16(OBJ_FRAME      ): pObj=new SdrOle2Obj(true);            break;
            case sal_uInt16(OBJ_CAPTION    ): pObj=new SdrCaptionObj;               break;
            case sal_uInt16(OBJ_PAGE       ): pObj=new SdrPageObj;                  break;
            case sal_uInt16(OBJ_UNO        ): pObj=new SdrUnoObj(String());         break;
            case sal_uInt16(OBJ_CUSTOMSHAPE  ): pObj=new SdrObjCustomShape();       break;
            case sal_uInt16(OBJ_MEDIA      ): pObj=new SdrMediaObj();               break;
            case sal_uInt16(OBJ_TABLE      ): pObj=new ::sdr::table::SdrTableObj(pModel);   break;
        }
    }

    if(pObj == NULL)
    {
        SdrObjFactory* pFact=new SdrObjFactory(nInvent,nIdent,pPage,pModel);
        SdrLinkList& rLL=ImpGetUserMakeObjHdl();
        unsigned nAnz=rLL.GetLinkCount();
        unsigned i=0;
        while (i<nAnz && pObj==NULL) {
            rLL.GetLink(i).Call((void*)pFact);
            pObj=pFact->pNewObj;
            i++;
        }
        delete pFact;
    }

    if(pObj == NULL)
    {
        // Well, if no one wants it...
    }

    if(pObj != NULL)
    {
        if(pPage != NULL)
            pObj->SetPage(pPage);
        else if(pModel != NULL)
            pObj->SetModel(pModel);
    }

    return pObj;
}

void SdrObjFactory::InsertMakeObjectHdl(const Link& rLink)
{
    SdrLinkList& rLL=ImpGetUserMakeObjHdl();
    rLL.InsertLink(rLink);
}

void SdrObjFactory::RemoveMakeObjectHdl(const Link& rLink)
{
    SdrLinkList& rLL=ImpGetUserMakeObjHdl();
    rLL.RemoveLink(rLink);
}

void SdrObjFactory::InsertMakeUserDataHdl(const Link& rLink)
{
    SdrLinkList& rLL=ImpGetUserMakeObjUserDataHdl();
    rLL.InsertLink(rLink);
}

void SdrObjFactory::RemoveMakeUserDataHdl(const Link& rLink)
{
    SdrLinkList& rLL=ImpGetUserMakeObjUserDataHdl();
    rLL.RemoveLink(rLink);
}

namespace svx
{
    ISdrObjectFilter::~ISdrObjectFilter()
    {
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
