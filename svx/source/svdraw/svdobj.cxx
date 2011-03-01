/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"
#include <com/sun/star/lang/XComponent.hpp>

#define _USE_MATH_DEFINES
#include <math.h>
#include <vcl/metaact.hxx>   // fuer TakeContour
#include <vcl/cvtsvm.hxx>
#include <tools/line.hxx>
#include <tools/bigint.hxx>
#include <tools/diagnose_ex.h>
#include <vector>
#include <svx/svdobj.hxx>
#include <svx/xpoly.hxx>
#include <svx/svdetc.hxx>
#include <svx/svdtrans.hxx>
#include <svx/svdhdl.hxx>
#include <svx/svddrag.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdovirt.hxx>  // Fuer Add/Del Ref
#include <svx/svdview.hxx>   // fuer Dragging (Ortho abfragen)
#include "svdglob.hxx"   // StringCache
#include "svdstr.hrc"    // Objektname
#include <svx/svdogrp.hxx>   // Factory
#include <svx/svdopath.hxx>  // Factory
#include <svx/svdoedge.hxx>  // Factory
#include <svx/svdorect.hxx>  // Factory
#include <svx/svdocirc.hxx>  // Factory
#include <svx/svdotext.hxx>  // Factory
#include <svx/svdomeas.hxx>  // Factory
#include <svx/svdograf.hxx>  // Factory
#include <svx/svdoole2.hxx>  // Factory
#include <svx/svdocapt.hxx>  // Factory
#include <svx/svdopage.hxx>  // Factory
#include <svx/svdouno.hxx>   // Factory
#include <svx/svdattrx.hxx> // NotPersistItems
#include <svx/svdoashp.hxx>
#include <svx/svdomedia.hxx>
#include <svx/xlnwtit.hxx>
#include <svx/xlnstwit.hxx>
#include <svx/xlnedwit.hxx>
#include <svx/xlnstit.hxx>
#include <svx/xlnedit.hxx>
#include <svx/xlnstcit.hxx>
#include <svx/xlnedcit.hxx>
#include <svx/xlndsit.hxx>
#include <svx/xlnclit.hxx>
#include <svx/xflclit.hxx>
#include "svditer.hxx"
#include <svx/xlntrit.hxx>
#include <svx/xfltrit.hxx>
#include <svx/xfltrit.hxx>
#include <svx/xflftrit.hxx>
#include "svx/xlinjoit.hxx"
#include <svx/unopage.hxx>
#include <editeng/eeitem.hxx>
#include <svx/xenum.hxx>
#include <svx/xgrad.hxx>
#include <svx/xhatch.hxx>
#include <svx/xflhtit.hxx>
#include <svx/xbtmpit.hxx>
#include <svx/svdpool.hxx>
#include <editeng/editeng.hxx>
#include <vcl/salbtype.hxx>     // FRound
#include <svl/whiter.hxx>

// #97849#
#include <svx/fmmodel.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/objface.hxx>
#include "svdoimp.hxx"
#include <vcl/graphictools.hxx>
#include <svtools/colorcfg.hxx>
#include <svx/sdr/properties/emptyproperties.hxx>
#include <svx/sdr/contact/viewcontactofsdrobj.hxx>
#include <svx/sdr/contact/viewcontactofgraphic.hxx>
#include <svx/sdr/contact/objectcontactofobjlistpainter.hxx>
#include <svx/sdr/contact/displayinfo.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/range/b2drange.hxx>
#include <svx/unoshape.hxx>
#include <vcl/virdev.hxx>
#include <basegfx/polygon/b2dpolypolygoncutter.hxx>
#include <drawinglayer/processor2d/contourextractor2d.hxx>
#include <drawinglayer/processor2d/linegeometryextractor2d.hxx>
#include <svx/polysc3d.hxx>
#include "svx/svdotable.hxx"
#include "svx/shapepropertynotifier.hxx"
#include <svx/sdrhittesthelper.hxx>
#include <svx/svdundo.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <svx/sdrobjectfilter.hxx>

using namespace ::com::sun::star;

// #104018# replace macros above with type-detecting methods
inline double ImplTwipsToMM(double fVal) { return (fVal * (127.0 / 72.0)); }
inline double ImplMMToTwips(double fVal) { return (fVal * (72.0 / 127.0)); }

////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEINIT0(SdrObjUserCall);

SdrObjUserCall::~SdrObjUserCall()
{
}

void SdrObjUserCall::Changed(const SdrObject& /*rObj*/, SdrUserCallType /*eType*/, const Rectangle& /*rOldBoundRect*/)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEINIT0(SdrObjUserData);

void SdrObjUserData::operator=(const SdrObjUserData& /*rData*/)    // nicht implementiert
{
}

sal_Bool SdrObjUserData::operator==(const SdrObjUserData& /*rData*/) const // nicht implementiert
{
    return FALSE;
}

sal_Bool SdrObjUserData::operator!=(const SdrObjUserData& /*rData*/) const // nicht implementiert
{
    return FALSE;
}

SdrObjUserData::~SdrObjUserData()
{
}

bool SdrObjUserData::HasMacro(const SdrObject* /*pObj*/) const
{
    return FALSE;
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
    return FALSE;
}

XubString SdrObjUserData::GetMacroPopupComment(const SdrObjMacroHitRec& /*rRec*/, const SdrObject* /*pObj*/) const
{
    return String();
}

void SdrObjUserDataList::Clear()
{
    USHORT nAnz=GetUserDataCount();
    for (USHORT i=0; i<nAnz; i++) {
        delete GetUserData(i);
    }
    aList.Clear();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

DBG_NAME(SdrObjGeoData);

SdrObjGeoData::SdrObjGeoData():
    pGPL(NULL),
    bMovProt(FALSE),
    bSizProt(FALSE),
    bNoPrint(FALSE),
    bClosedObj(FALSE),
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
    pGluePoints(NULL),
    pAutoTimer(NULL)
{
}

SdrObjPlusData::~SdrObjPlusData()
{
    if (pBroadcast   !=NULL) delete pBroadcast;
    if (pUserDataList!=NULL) delete pUserDataList;
    if (pGluePoints  !=NULL) delete pGluePoints;
    if (pAutoTimer   !=NULL) delete pAutoTimer;
}

SdrObjPlusData* SdrObjPlusData::Clone(SdrObject* pObj1) const
{
    SdrObjPlusData* pNeuPlusData=new SdrObjPlusData;
    if (pUserDataList!=NULL) {
        USHORT nAnz=pUserDataList->GetUserDataCount();
        if (nAnz!=0) {
            pNeuPlusData->pUserDataList=new SdrObjUserDataList;
            for (USHORT i=0; i<nAnz; i++) {
                SdrObjUserData* pNeuUserData=pUserDataList->GetUserData(i)->Clone(pObj1);
                if (pNeuUserData!=NULL) {
                    pNeuPlusData->pUserDataList->InsertUserData(pNeuUserData);
                } else {
                    OSL_FAIL("SdrObjPlusData::Clone(): UserData.Clone() liefert NULL");
                }
            }
        }
    }
    if (pGluePoints!=NULL) pNeuPlusData->pGluePoints=new SdrGluePointList(*pGluePoints);
    // MtfAnimator wird auch nicht mitkopiert

    // #i68101#
    // copy object name, title and description
    pNeuPlusData->aObjName = aObjName;
    pNeuPlusData->aObjTitle = aObjTitle;
    pNeuPlusData->aObjDescription = aObjDescription;

    if (pAutoTimer!=NULL) {
        pNeuPlusData->pAutoTimer=new AutoTimer;
        // Handler, etc. nicht mitkopieren!
    }

    // For HTMLName: Do not clone, leave uninitialized (empty string)

    return pNeuPlusData;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//   @@@@  @@@@@  @@@@@@ @@@@@  @@@@  @@@@@@
//  @@  @@ @@  @@     @@ @@    @@  @@   @@
//  @@  @@ @@  @@     @@ @@    @@       @@
//  @@  @@ @@@@@      @@ @@@@  @@       @@
//  @@  @@ @@  @@     @@ @@    @@       @@
//  @@  @@ @@  @@ @@  @@ @@    @@  @@   @@
//   @@@@  @@@@@   @@@@  @@@@@  @@@@    @@
//
////////////////////////////////////////////////////////////////////////////////////////////////////

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
// #110094# DrawContact section

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
{
    DBG_CTOR(SdrObject,NULL);
    bVirtObj         =FALSE;
    bSnapRectDirty   =TRUE;
    bNetLock         =FALSE;
    bInserted        =FALSE;
    bGrouped         =FALSE;
    bMovProt         =FALSE;
    bSizProt         =FALSE;
    bNoPrint         =FALSE;
    bEmptyPresObj    =FALSE;
    bNotVisibleAsMaster=FALSE;
    bClosedObj       =FALSE;
    mbVisible        = true;

    // #i25616#
    mbLineIsOutsideGeometry = sal_False;

    // #i25616#
    mbSupportTextIndentingOnLineWidthChange = sal_False;

    //#110094#-1
    //bWriterFlyFrame  =FALSE;

    bNotMasterCachable=FALSE;
    bIsEdge=FALSE;
    bIs3DObj=FALSE;
    bMarkProt=FALSE;
    bIsUnoObj=FALSE;
}

SdrObject::~SdrObject()
{
    // tell all the registered ObjectUsers that the page is in destruction
    ::sdr::ObjectUserVector aListCopy(maObjectUsers.begin(), maObjectUsers.end());
    for(::sdr::ObjectUserVector::iterator aIterator = aListCopy.begin(); aIterator != aListCopy.end(); aIterator++)
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
    if (pPlusData!=NULL) delete pPlusData;

    if(mpProperties)
    {
        delete mpProperties;
        mpProperties = 0L;
    }

    // #110094#
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
        bSnapRectDirty=TRUE;
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

    // update listeners at possible api wrapper object
    if( pModel != pNewModel )
    {
        SvxShape* pShape = getSvxShape();
        if( pShape )
            pShape->ChangeModel( pNewModel );
    }

    pModel = pNewModel;
}

void SdrObject::SetObjList(SdrObjList* pNewObjList)
{
    pObjList=pNewObjList;
}

void SdrObject::SetPage(SdrPage* pNewPage)
{
    pPage=pNewPage;
    if (pPage!=NULL) {
        SdrModel* pMod=pPage->GetModel();
        if (pMod!=pModel && pMod!=NULL) {
            SetModel(pMod);
        }}
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

void SdrObject::FreeGlobalDrawObjectItemPool()
{
    // code for deletion of GlobalItemPool
    if(mpGlobalItemPool)
    {
        SfxItemPool* pGlobalOutlPool = mpGlobalItemPool->GetSecondaryPool();
        SfxItemPool::Free(mpGlobalItemPool);
        SfxItemPool::Free(pGlobalOutlPool);
    }
}

SdrItemPool* SdrObject::GetObjectItemPool() const
{
    if(pModel)
        return (SdrItemPool*)(&pModel->GetItemPool());

    // use a static global default pool
    return &SdrObject::GetGlobalDrawObjectItemPool();
}

UINT32 SdrObject::GetObjInventor()   const
{
    return SdrInventor;
}

UINT16 SdrObject::GetObjIdentifier() const
{
    return UINT16(OBJ_NONE);
}

void SdrObject::TakeObjInfo(SdrObjTransformInfoRec& rInfo) const
{
    rInfo.bRotateFreeAllowed=FALSE;
    rInfo.bMirrorFreeAllowed=FALSE;
    rInfo.bTransparenceAllowed = FALSE;
    rInfo.bGradientAllowed = FALSE;
    rInfo.bShearAllowed     =FALSE;
    rInfo.bEdgeRadiusAllowed=FALSE;
    rInfo.bCanConvToPath    =FALSE;
    rInfo.bCanConvToPoly    =FALSE;
    rInfo.bCanConvToContour = FALSE;
    rInfo.bCanConvToPathLineToArea=FALSE;
    rInfo.bCanConvToPolyLineToArea=FALSE;
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
        ULONG nObjAnz=pOL->GetObjCount();
        for (ULONG nObjNum=0; nObjNum<nObjAnz; nObjNum++) {
            pOL->GetObj(nObjNum)->getMergedHierarchyLayerSet(rSet);
        }
    }
}

void SdrObject::NbcSetLayer(SdrLayerID nLayer)
{
    if(GetLayer() != nLayer)
    {
        mnLayerID = nLayer;
    }
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

void SdrObject::AddReference(SdrVirtObj& rVrtObj)
{
    AddListener(rVrtObj);
}

void SdrObject::DelReference(SdrVirtObj& rVrtObj)
{
    RemoveListener(rVrtObj);
}

AutoTimer* SdrObject::ForceAutoTimer()
{
    ImpForcePlusData();
    if (pPlusData->pAutoTimer==NULL) pPlusData->pAutoTimer=new AutoTimer;
    return pPlusData->pAutoTimer;
}

bool SdrObject::HasRefPoint() const
{
    return FALSE;
}

Point SdrObject::GetRefPoint() const
{
    return GetCurrentBoundRect().Center();
}

void SdrObject::SetRefPoint(const Point& /*rPnt*/)
{
}

SdrObjList* SdrObject::GetSubList() const
{
    return NULL;
}

SdrObject* SdrObject::GetUpGroup() const
{
    return pObjList!=NULL ? pObjList->GetOwnerObj() : NULL;
}

void SdrObject::SetName(const String& rStr)
{
    if(rStr.Len() && !pPlusData)
    {
        ImpForcePlusData();
    }

    if(pPlusData && pPlusData->aObjName != rStr)
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

String SdrObject::GetName() const
{
    if(pPlusData)
    {
        return pPlusData->aObjName;
    }

    return String();
}

void SdrObject::SetTitle(const String& rStr)
{
    if(rStr.Len() && !pPlusData)
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

String SdrObject::GetTitle() const
{
    if(pPlusData)
    {
        return pPlusData->aObjTitle;
    }

    return String();
}

void SdrObject::SetDescription(const String& rStr)
{
    if(rStr.Len() && !pPlusData)
    {
        ImpForcePlusData();
    }

    if(pPlusData && pPlusData->aObjDescription != rStr)
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

String SdrObject::GetDescription() const
{
    if(pPlusData)
    {
        return pPlusData->aObjDescription;
    }

    return String();
}

void SdrObject::SetHTMLName(const String& rStr)
{
    if(rStr.Len() && !pPlusData)
    {
        ImpForcePlusData();
    }

    if(pPlusData && pPlusData->aObjName != rStr)
    {
        pPlusData->aHTMLName = rStr;
        SetChanged();
    }
}

String SdrObject::GetHTMLName() const
{
    if(pPlusData)
    {
        return pPlusData->aHTMLName;
    }

    return String();
}

UINT32 SdrObject::GetOrdNum() const
{
    if (pObjList!=NULL) {
        if (pObjList->IsObjOrdNumsDirty()) {
            pObjList->RecalcObjOrdNums();
        }
    } else ((SdrObject*)this)->nOrdNum=0;
    return nOrdNum;
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




// #111111#
// To make clearer that this method may trigger RecalcBoundRect and thus may be
// expensive and somtimes problematic (inside a bigger object change You will get
// non-useful BoundRects sometimes) i rename that method from GetBoundRect() to
// GetCurrentBoundRect().
const Rectangle& SdrObject::GetCurrentBoundRect() const
{
    if(aOutRect.IsEmpty())
    {
        const_cast< SdrObject* >(this)->RecalcBoundRect();
    }

    return aOutRect;
}

// #111111#
// To have a possibility to get the last calculated BoundRect e.g for producing
// the first rectangle for repaints (old and new need to be used) without forcing
// a RecalcBoundRect (which may be problematical and expensive sometimes) i add here
// a new method for accessing the last BoundRect.
const Rectangle& SdrObject::GetLastBoundRect() const
{
    return aOutRect;
}

void SdrObject::RecalcBoundRect()
{
    // #i101680# suppress BoundRect calculations on import(s)
    if(pModel && pModel->isLocked() )
        return;

    // central new method which will calculate the BoundRect using primitive geometry
    if(aOutRect.IsEmpty())
    {
        const drawinglayer::primitive2d::Primitive2DSequence xPrimitives(GetViewContact().getViewIndependentPrimitive2DSequence());

        if(xPrimitives.hasElements())
        {
            // use neutral ViewInformation and get the range of the primitives
            const drawinglayer::geometry::ViewInformation2D aViewInformation2D(0);
            const basegfx::B2DRange aRange(drawinglayer::primitive2d::getB2DRangeFromPrimitive2DSequence(xPrimitives, aViewInformation2D));

            if(!aRange.isEmpty())
            {
                aOutRect = Rectangle(
                        (sal_Int32)floor(aRange.getMinX()), (sal_Int32)floor(aRange.getMinY()),
                        (sal_Int32)ceil(aRange.getMaxX()), (sal_Int32)ceil(aRange.getMaxY()));
                return;
            }
        }
    }
}

void SdrObject::BroadcastObjectChange() const
{
    if( pModel && pModel->isLocked() )
        return;

    sal_Bool bPlusDataBroadcast(pPlusData && pPlusData->pBroadcast);
    sal_Bool bObjectChange(IsInserted() && pModel);

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
    // #110094#-11
    // For test purposes, use the new ViewContact for change
    // notification now.
    ActionChanged();

    if(IsInserted() && pModel)
    {
        pModel->SetChanged();
    }
}

// Tooling for painting a single object to a OutputDevice.
sal_Bool SdrObject::SingleObjectPainter(OutputDevice& rOut) const
{
    sdr::contact::SdrObjectVector aObjectVector;
    aObjectVector.push_back(const_cast< SdrObject* >(this));

    sdr::contact::ObjectContactOfObjListPainter aPainter(rOut, aObjectVector, GetPage());
    sdr::contact::DisplayInfo aDisplayInfo;

    // do processing
    aPainter.ProcessDisplay(aDisplayInfo);

    return sal_True;
}

BOOL SdrObject::LineGeometryUsageIsNecessary() const
{
    XLineStyle eXLS = (XLineStyle)((const XLineStyleItem&)GetMergedItem(XATTR_LINESTYLE)).GetValue();
    return (eXLS != XLINE_NONE);
}

SdrObject* SdrObject::Clone() const
{
    SdrObject* pObj=SdrObjFactory::MakeNewObject(GetObjInventor(),GetObjIdentifier(),NULL);
    if (pObj!=NULL) {
        pObj->pModel=pModel;
        pObj->pPage=pPage;
        *pObj=*this;
    }
    return pObj;
}

void SdrObject::operator=(const SdrObject& rObj)
{
    if(mpProperties)
    {
        delete mpProperties;
        mpProperties = 0L;
    }

    // #110094#
    if(mpViewContact)
    {
        delete mpViewContact;
        mpViewContact = 0L;
    }

    // The Clone() method uses the local copy constructor from the individual
    // sdr::properties::BaseProperties class. Since the target class maybe for another
    // draw object a SdrObject needs to be provided, as in the nromal constructor.
    mpProperties = &rObj.GetProperties().Clone(*this);

    pModel  =rObj.pModel;
    aOutRect=rObj.aOutRect;
    mnLayerID = rObj.mnLayerID;
    aAnchor =rObj.aAnchor;
    bVirtObj=rObj.bVirtObj;
    bSizProt=rObj.bSizProt;
    bMovProt=rObj.bMovProt;
    bNoPrint=rObj.bNoPrint;
    mbVisible=rObj.mbVisible;
    bMarkProt=rObj.bMarkProt;
    //EmptyPresObj wird nicht kopiert: nun doch! (25-07-1995, Joe)
    bEmptyPresObj =rObj.bEmptyPresObj;
    //NotVisibleAsMaster wird nicht kopiert: nun doch! (25-07-1995, Joe)
    bNotVisibleAsMaster=rObj.bNotVisibleAsMaster;
    bSnapRectDirty=TRUE; //rObj.bSnapRectDirty;
    bNotMasterCachable=rObj.bNotMasterCachable;
    if (pPlusData!=NULL) { delete pPlusData; pPlusData=NULL; }
    if (rObj.pPlusData!=NULL) {
        pPlusData=rObj.pPlusData->Clone(this);
    }
    if (pPlusData!=NULL && pPlusData->pBroadcast!=NULL) {
        delete pPlusData->pBroadcast; // der Broadcaster wird nicht mitkopiert
        pPlusData->pBroadcast=NULL;
    }
}

void SdrObject::TakeObjNameSingul(XubString& rName) const
{
    rName=ImpGetResStr(STR_ObjNameSingulNONE);

    String aName( GetName() );
    if(aName.Len())
    {
        rName += sal_Unicode(' ');
        rName += sal_Unicode('\'');
        rName += aName;
        rName += sal_Unicode('\'');
    }
}

void SdrObject::TakeObjNamePlural(XubString& rName) const
{
    rName=ImpGetResStr(STR_ObjNamePluralNONE);
}

void SdrObject::ImpTakeDescriptionStr(USHORT nStrCacheID, XubString& rStr, USHORT nVal) const
{
    rStr = ImpGetResStr(nStrCacheID);

    sal_Char aSearchText1[] = "%1";
    sal_Char aSearchText2[] = "%2";
    xub_StrLen nPos = rStr.SearchAscii(aSearchText1);

    if(nPos != STRING_NOTFOUND)
    {
        rStr.Erase(nPos, 2);

        XubString aObjName;

        TakeObjNameSingul(aObjName);
        rStr.Insert(aObjName, nPos);
    }

    nPos = rStr.SearchAscii(aSearchText2);

    if(nPos != STRING_NOTFOUND)
    {
        rStr.Erase(nPos, 2);
        rStr.Insert(UniString::CreateFromInt32(nVal), nPos);
    }
}

XubString SdrObject::GetWinkStr(long nWink, bool bNoDegChar) const
{
    XubString aStr;
    if (pModel!=NULL) {
        pModel->TakeWinkStr(nWink,aStr,bNoDegChar);
    }
    return aStr;
}

XubString SdrObject::GetMetrStr(long nVal, MapUnit /*eWantMap*/, bool bNoUnitChars) const
{
    XubString aStr;
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
            SdrObject* pLeft = pEdgeObj->GetConnectedNode(TRUE);
            SdrObject* pRight = pEdgeObj->GetConnectedNode(FALSE);

            if(pLeft)
            {
                pClone->ConnectToNode(TRUE, pLeft);
            }

            if(pRight)
            {
                pClone->ConnectToNode(FALSE, pRight);
            }
        }

        SfxItemSet aNewSet(*GetObjectItemPool());

        // #i101980# ignore LineWidth; that's what the old implementation
        // did. With linewidth, the result may be huge due to fat/thick
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
            const drawinglayer::geometry::ViewInformation2D aViewInformation2D(0);

            // create extractor, process and get result
            drawinglayer::processor2d::ContourExtractor2D aExtractor(aViewInformation2D);
            aExtractor.process(xSequence);
            const std::vector< basegfx::B2DPolyPolygon >& rResult(aExtractor.getExtractedContour());
            const sal_uInt32 nSize(rResult.size());

            // when count is one, it is implied that the object has only it's normal
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
        case 0: pH=new SdrHdl(rR.TopLeft(),     HDL_UPLFT); break; // Oben links
        case 1: pH=new SdrHdl(rR.TopCenter(),   HDL_UPPER); break; // Oben
        case 2: pH=new SdrHdl(rR.TopRight(),    HDL_UPRGT); break; // Oben rechts
        case 3: pH=new SdrHdl(rR.LeftCenter(),  HDL_LEFT ); break; // Links
        case 4: pH=new SdrHdl(rR.RightCenter(), HDL_RIGHT); break; // Rechts
        case 5: pH=new SdrHdl(rR.BottomLeft(),  HDL_LWLFT); break; // Unten links
        case 6: pH=new SdrHdl(rR.BottomCenter(),HDL_LOWER); break; // Unten
        case 7: pH=new SdrHdl(rR.BottomRight(), HDL_LWRGT); break; // Unten rechts
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
        nXMul=Abs(nXMul);
        nYMul=Abs(nYMul);
        nXDiv=Abs(nXDiv);
        nYDiv=Abs(nYDiv);
        Fraction aXFact(nXMul,nXDiv); // Fractions zum kuerzen
        Fraction aYFact(nYMul,nYDiv); // und zum vergleichen
        nXMul=aXFact.GetNumerator();
        nYMul=aYFact.GetNumerator();
        nXDiv=aXFact.GetDenominator();
        nYDiv=aYFact.GetDenominator();
        if (bEcke) { // Eckpunkthandles
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
        } else { // Scheitelpunkthandles
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
    return TRUE;
}

bool SdrObject::MovCreate(SdrDragStat& rStat)
{
    rStat.TakeCreateRect(aOutRect);
    rStat.SetActionRect(aOutRect);
    aOutRect.Justify();

    // #i101648# for naked (non-derived) SdrObjects, do not invalidate aOutRect
    // by calling SetBoundRectDirty(); aOutRect IS the geometry for such objects.
    // No derivation implementation calls the parent implementation, so this will
    // cause no further prolems
    //
    // SetBoundRectDirty();
    // bSnapRectDirty=TRUE;

    return TRUE;
}

bool SdrObject::EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd)
{
    rStat.TakeCreateRect(aOutRect);
    aOutRect.Justify();

    // #i101648# see description at MovCreate
    //
    // SetRectsDirty();

    return (eCmd==SDRCREATE_FORCEEND || rStat.GetPointAnz()>=2);
}

void SdrObject::BrkCreate(SdrDragStat& /*rStat*/)
{
}

bool SdrObject::BckCreate(SdrDragStat& /*rStat*/)
{
    return FALSE;
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

// Transformationen
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
    SetGlueReallyAbsolute(TRUE);
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
    aOutRect.Justify(); // Sicherheitshalber
    SetRectsDirty();
    NbcRotateGluePoints(rRef,nWink,sn,cs);
    SetGlueReallyAbsolute(FALSE);
}

void SdrObject::NbcMirror(const Point& rRef1, const Point& rRef2)
{
    SetGlueReallyAbsolute(TRUE);
    aOutRect.Move(-rRef1.X(),-rRef1.Y());
    Rectangle R(aOutRect);
    long dx=rRef2.X()-rRef1.X();
    long dy=rRef2.Y()-rRef1.Y();
    if (dx==0) {          // Vertikale Achse
        aOutRect.Left() =-R.Right();
        aOutRect.Right()=-R.Left();
    } else if (dy==0) {   // Horizontale Achse
        aOutRect.Top()   =-R.Bottom();
        aOutRect.Bottom()=-R.Top();
    } else if (dx==dy) {  /* 45 Grad Achse \ */
        aOutRect.Left()  =R.Top();
        aOutRect.Right() =R.Bottom();
        aOutRect.Top()   =R.Left();
        aOutRect.Bottom()=R.Right();
    } else if (dx==-dy) { // 45 Grad Achse /
        aOutRect.Left()  =-R.Bottom();
        aOutRect.Right() =-R.Top();
        aOutRect.Top()   =-R.Right();
        aOutRect.Bottom()=-R.Left();
    }
    aOutRect.Move(rRef1.X(),rRef1.Y());
    aOutRect.Justify(); // Sicherheitshalber
    SetRectsDirty();
    NbcMirrorGluePoints(rRef1,rRef2);
    SetGlueReallyAbsolute(FALSE);
}

void SdrObject::NbcShear(const Point& rRef, long nWink, double tn, bool bVShear)
{
    SetGlueReallyAbsolute(TRUE);
    NbcShearGluePoints(rRef,nWink,tn,bVShear);
    SetGlueReallyAbsolute(FALSE);
}

void SdrObject::Move(const Size& rSiz)
{
    if (rSiz.Width()!=0 || rSiz.Height()!=0) {
        Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetLastBoundRect();
        // #110094#-14 SendRepaintBroadcast();
        NbcMove(rSiz);
        SetChanged();
        BroadcastObjectChange();
        SendUserCall(SDRUSERCALL_MOVEONLY,aBoundRect0);
    }
}

void SdrObject::Resize(const Point& rRef, const Fraction& xFact, const Fraction& yFact)
{
    if (xFact.GetNumerator()!=xFact.GetDenominator() || yFact.GetNumerator()!=yFact.GetDenominator()) {
        Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetLastBoundRect();
        // #110094#-14 SendRepaintBroadcast();
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
        // #110094#-14 SendRepaintBroadcast();
        NbcRotate(rRef,nWink,sn,cs);
        SetChanged();
        BroadcastObjectChange();
        SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
    }
}

void SdrObject::Mirror(const Point& rRef1, const Point& rRef2)
{
    Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetLastBoundRect();
    // #110094#-14 SendRepaintBroadcast();
    NbcMirror(rRef1,rRef2);
    SetChanged();
    BroadcastObjectChange();
    SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
}

void SdrObject::Shear(const Point& rRef, long nWink, double tn, bool bVShear)
{
    if (nWink!=0) {
        Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetLastBoundRect();
        // #110094#-14 SendRepaintBroadcast();
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
    NbcMove(aSiz); // Der ruft auch das SetRectsDirty()
}

void SdrObject::SetRelativePos(const Point& rPnt)
{
    if (rPnt!=GetRelativePos()) {
        Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetLastBoundRect();
        // #110094#-14 SendRepaintBroadcast();
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

void SdrObject::NbcSetAnchorPos(const Point& rPnt)
{
    Size aSiz(rPnt.X()-aAnchor.X(),rPnt.Y()-aAnchor.Y());
    aAnchor=rPnt;
    NbcMove(aSiz); // Der ruft auch das SetRectsDirty()
}

void SdrObject::SetAnchorPos(const Point& rPnt)
{
    if (rPnt!=aAnchor) {
        Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetLastBoundRect();
        // #110094#-14 SendRepaintBroadcast();
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
    // #110094#-14 SendRepaintBroadcast();
    NbcSetSnapRect(rRect);
    SetChanged();
    BroadcastObjectChange();
    SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
}

void SdrObject::SetLogicRect(const Rectangle& rRect)
{
    Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetLastBoundRect();
    // #110094#-14 SendRepaintBroadcast();
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
    return sal_False;
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
    // #110094#-14 SendRepaintBroadcast();
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
    return FALSE;
}

sal_Bool SdrObject::BegTextEdit(SdrOutliner& /*rOutl*/)
{
    return FALSE;
}

void SdrObject::EndTextEdit(SdrOutliner& /*rOutl*/)
{
}

void SdrObject::SetOutlinerParaObject(OutlinerParaObject* pTextObject)
{
    Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetLastBoundRect();
    // #110094#-14 SendRepaintBroadcast();
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

#define Imp2ndKennung (0x434F4D43)
SdrObjUserData* SdrObject::ImpGetMacroUserData() const
{
    SdrObjUserData* pData=NULL;
    USHORT nAnz=GetUserDataCount();
    for (USHORT nNum=nAnz; nNum>0 && pData==NULL;) {
        nNum--;
        pData=GetUserData(nNum);
        if (!pData->HasMacro(this)) pData=NULL;
    }
    return pData;
}

bool SdrObject::HasMacro() const
{
    SdrObjUserData* pData=ImpGetMacroUserData();
    return pData!=NULL ? pData->HasMacro(this) : FALSE;
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
    return FALSE;
}

XubString SdrObject::GetMacroPopupComment(const SdrObjMacroHitRec& rRec) const
{
    SdrObjUserData* pData=ImpGetMacroUserData();
    if (pData!=NULL) {
        return pData->GetMacroPopupComment(rRec,this);
    }
    return String();
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

    // Benutzerdefinierte Klebepunkte
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

    // Benutzerdefinierte Klebepunkte
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

void SdrObject::ClearObjectItem(const sal_uInt16 nWhich)
{
    GetProperties().ClearObjectItem(nWhich);
}

void SdrObject::ClearMergedItem(const sal_uInt16 nWhich)
{
    GetProperties().ClearMergedItem(nWhich);
}

void SdrObject::SetObjectItemSet(const SfxItemSet& rSet)
{
    GetProperties().SetObjectItemSet(rSet);
}

void SdrObject::SetMergedItemSet(const SfxItemSet& rSet, sal_Bool bClearAllItems)
{
    GetProperties().SetMergedItemSet(rSet, bClearAllItems);
}

const SfxPoolItem& SdrObject::GetObjectItem(const sal_uInt16 nWhich) const
{
    return GetObjectItemSet().Get(nWhich);
}

const SfxPoolItem& SdrObject::GetMergedItem(const sal_uInt16 nWhich) const
{
    return GetMergedItemSet().Get(nWhich);
}

void SdrObject::SetMergedItemSetAndBroadcast(const SfxItemSet& rSet, sal_Bool bClearAllItems)
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
    if (rAttr.GetItemState(SDRATTR_TRANSFORMREF1X,TRUE,&pPoolItem)==SFX_ITEM_SET) {
        aRef1.X()=((const SdrTransformRef1XItem*)pPoolItem)->GetValue();
    }
    if (rAttr.GetItemState(SDRATTR_TRANSFORMREF1Y,TRUE,&pPoolItem)==SFX_ITEM_SET) {
        aRef1.Y()=((const SdrTransformRef1YItem*)pPoolItem)->GetValue();
    }
    if (rAttr.GetItemState(SDRATTR_TRANSFORMREF2X,TRUE,&pPoolItem)==SFX_ITEM_SET) {
        aRef2.X()=((const SdrTransformRef2XItem*)pPoolItem)->GetValue();
    }
    if (rAttr.GetItemState(SDRATTR_TRANSFORMREF2Y,TRUE,&pPoolItem)==SFX_ITEM_SET) {
        aRef2.Y()=((const SdrTransformRef2YItem*)pPoolItem)->GetValue();
    }

    Rectangle aNewSnap(rSnap);
    if (rAttr.GetItemState(SDRATTR_MOVEX,TRUE,&pPoolItem)==SFX_ITEM_SET) {
        long n=((const SdrMoveXItem*)pPoolItem)->GetValue();
        aNewSnap.Move(n,0);
    }
    if (rAttr.GetItemState(SDRATTR_MOVEY,TRUE,&pPoolItem)==SFX_ITEM_SET) {
        long n=((const SdrMoveYItem*)pPoolItem)->GetValue();
        aNewSnap.Move(0,n);
    }
    if (rAttr.GetItemState(SDRATTR_ONEPOSITIONX,TRUE,&pPoolItem)==SFX_ITEM_SET) {
        long n=((const SdrOnePositionXItem*)pPoolItem)->GetValue();
        aNewSnap.Move(n-aNewSnap.Left(),0);
    }
    if (rAttr.GetItemState(SDRATTR_ONEPOSITIONY,TRUE,&pPoolItem)==SFX_ITEM_SET) {
        long n=((const SdrOnePositionYItem*)pPoolItem)->GetValue();
        aNewSnap.Move(0,n-aNewSnap.Top());
    }
    if (rAttr.GetItemState(SDRATTR_ONESIZEWIDTH,TRUE,&pPoolItem)==SFX_ITEM_SET) {
        long n=((const SdrOneSizeWidthItem*)pPoolItem)->GetValue();
        aNewSnap.Right()=aNewSnap.Left()+n;
    }
    if (rAttr.GetItemState(SDRATTR_ONESIZEHEIGHT,TRUE,&pPoolItem)==SFX_ITEM_SET) {
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

    if (rAttr.GetItemState(SDRATTR_SHEARANGLE,TRUE,&pPoolItem)==SFX_ITEM_SET) {
        long n=((const SdrShearAngleItem*)pPoolItem)->GetValue();
        n-=GetShearAngle();
        if (n!=0) {
            double nTan=tan(n*nPi180);
            NbcShear(aRef1,n,nTan,FALSE);
        }
    }
    if (rAttr.GetItemState(SDRATTR_ROTATEANGLE,TRUE,&pPoolItem)==SFX_ITEM_SET) {
        long n=((const SdrRotateAngleItem*)pPoolItem)->GetValue();
        n-=GetRotateAngle();
        if (n!=0) {
            double nSin=sin(n*nPi180);
            double nCos=cos(n*nPi180);
            NbcRotate(aRef1,n,nSin,nCos);
        }
    }
    if (rAttr.GetItemState(SDRATTR_ROTATEONE,TRUE,&pPoolItem)==SFX_ITEM_SET) {
        long n=((const SdrRotateOneItem*)pPoolItem)->GetValue();
        double nSin=sin(n*nPi180);
        double nCos=cos(n*nPi180);
        NbcRotate(aRef1,n,nSin,nCos);
    }
    if (rAttr.GetItemState(SDRATTR_HORZSHEARONE,TRUE,&pPoolItem)==SFX_ITEM_SET) {
        long n=((const SdrHorzShearOneItem*)pPoolItem)->GetValue();
        double nTan=tan(n*nPi180);
        NbcShear(aRef1,n,nTan,FALSE);
    }
    if (rAttr.GetItemState(SDRATTR_VERTSHEARONE,TRUE,&pPoolItem)==SFX_ITEM_SET) {
        long n=((const SdrVertShearOneItem*)pPoolItem)->GetValue();
        double nTan=tan(n*nPi180);
        NbcShear(aRef1,n,nTan,TRUE);
    }

    if (rAttr.GetItemState(SDRATTR_OBJMOVEPROTECT,TRUE,&pPoolItem)==SFX_ITEM_SET) {
        bool b=((const SdrObjMoveProtectItem*)pPoolItem)->GetValue();
        SetMoveProtect(b);
    }
    if (rAttr.GetItemState(SDRATTR_OBJSIZEPROTECT,TRUE,&pPoolItem)==SFX_ITEM_SET) {
        bool b=((const SdrObjSizeProtectItem*)pPoolItem)->GetValue();
        SetResizeProtect(b);
    }

    /* #67368# move protect always sets size protect */
    if( IsMoveProtect() )
        SetResizeProtect( true );

    if (rAttr.GetItemState(SDRATTR_OBJPRINTABLE,TRUE,&pPoolItem)==SFX_ITEM_SET) {
        bool b=((const SdrObjPrintableItem*)pPoolItem)->GetValue();
        SetPrintable(b);
    }

    if (rAttr.GetItemState(SDRATTR_OBJVISIBLE,TRUE,&pPoolItem)==SFX_ITEM_SET) {
        bool b=((const SdrObjVisibleItem*)pPoolItem)->GetValue();
        SetVisible(b);
    }

    SdrLayerID nLayer=SDRLAYER_NOTFOUND;
    if (rAttr.GetItemState(SDRATTR_LAYERID,TRUE,&pPoolItem)==SFX_ITEM_SET) {
        nLayer=((const SdrLayerIdItem*)pPoolItem)->GetValue();
    }
    if (rAttr.GetItemState(SDRATTR_LAYERNAME,TRUE,&pPoolItem)==SFX_ITEM_SET && pModel!=NULL) {
        XubString aLayerName=((const SdrLayerNameItem*)pPoolItem)->GetValue();
        const SdrLayerAdmin* pLayAd=pPage!=NULL ? &pPage->GetLayerAdmin() : pModel!=NULL ? &pModel->GetLayerAdmin() : NULL;
        if (pLayAd!=NULL) {
            const SdrLayer* pLayer=pLayAd->GetLayer(aLayerName, TRUE);
            if (pLayer!=NULL) {
                nLayer=pLayer->GetID();
            }
        }

    }
    if (nLayer!=SDRLAYER_NOTFOUND) {
        NbcSetLayer(nLayer);
    }

    if (rAttr.GetItemState(SDRATTR_OBJECTNAME,TRUE,&pPoolItem)==SFX_ITEM_SET) {
        XubString aName=((const SdrObjectNameItem*)pPoolItem)->GetValue();
        SetName(aName);
    }
    Rectangle aNewLogic(rLogic);
    if (rAttr.GetItemState(SDRATTR_LOGICSIZEWIDTH,TRUE,&pPoolItem)==SFX_ITEM_SET) {
        long n=((const SdrLogicSizeWidthItem*)pPoolItem)->GetValue();
        aNewLogic.Right()=aNewLogic.Left()+n;
    }
    if (rAttr.GetItemState(SDRATTR_LOGICSIZEHEIGHT,TRUE,&pPoolItem)==SFX_ITEM_SET) {
        long n=((const SdrLogicSizeHeightItem*)pPoolItem)->GetValue();
        aNewLogic.Bottom()=aNewLogic.Top()+n;
    }
    if (aNewLogic!=rLogic) {
        NbcSetLogicRect(aNewLogic);
    }
    Fraction aResizeX(1,1);
    Fraction aResizeY(1,1);
    if (rAttr.GetItemState(SDRATTR_RESIZEXONE,TRUE,&pPoolItem)==SFX_ITEM_SET) {
        aResizeX*=((const SdrResizeXOneItem*)pPoolItem)->GetValue();
    }
    if (rAttr.GetItemState(SDRATTR_RESIZEYONE,TRUE,&pPoolItem)==SFX_ITEM_SET) {
        aResizeY*=((const SdrResizeYOneItem*)pPoolItem)->GetValue();
    }
    if (aResizeX!=Fraction(1,1) || aResizeY!=Fraction(1,1)) {
        NbcResize(aRef1,aResizeX,aResizeY);
    }
}

void lcl_SetItem(SfxItemSet& rAttr, bool bMerge, const SfxPoolItem& rItem)
{
    if (bMerge) rAttr.MergeValue(rItem,TRUE);
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

void SdrObject::SetStyleSheet(SfxStyleSheet* pNewStyleSheet, sal_Bool bDontRemoveHardAttr)
{
    Rectangle aBoundRect0;

    if(pUserCall)
        aBoundRect0 = GetLastBoundRect();

    // #110094#-14 SendRepaintBroadcast();
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

// Das Broadcasting beim Setzen der Attribute wird vom AttrObj gemanagt
////////////////////////////////////////////////////////////////////////////////////////////////////

bool SdrObject::IsNode() const
{
    return true;
}

SdrGluePoint SdrObject::GetVertexGluePoint(USHORT nPosNum) const
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
    aGP.SetPercent(FALSE);

    return aGP;
}

SdrGluePoint SdrObject::GetCornerGluePoint(USHORT nPosNum) const
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
    aGP.SetPercent(FALSE);
    return aGP;
}

const SdrGluePointList* SdrObject::GetGluePointList() const
{
    if (pPlusData!=NULL) return pPlusData->pGluePoints;
    return NULL;
}

//SdrGluePointList* SdrObject::GetGluePointList()
//{
//  if (pPlusData!=NULL) return pPlusData->pGluePoints;
//  return NULL;
//}

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
    // erst Const-Aufruf um zu sehen, ob
    // ueberhaupt Klebepunkte da sind
    // const-Aufruf erzwingen!
    if (GetGluePointList()!=NULL) {
        SdrGluePointList* pGPL=ForceGluePointList();
        pGPL->SetReallyAbsolute(bOn,*this);
    }
}

void SdrObject::NbcRotateGluePoints(const Point& rRef, long nWink, double sn, double cs)
{
    // erst Const-Aufruf um zu sehen, ob
    // ueberhaupt Klebepunkte da sind
    // const-Aufruf erzwingen!
    if (GetGluePointList()!=NULL) {
        SdrGluePointList* pGPL=ForceGluePointList();
        pGPL->Rotate(rRef,nWink,sn,cs,this);
    }
}

void SdrObject::NbcMirrorGluePoints(const Point& rRef1, const Point& rRef2)
{
    // erst Const-Aufruf um zu sehen, ob
    // ueberhaupt Klebepunkte da sind
    // const-Aufruf erzwingen!
    if (GetGluePointList()!=NULL) {
        SdrGluePointList* pGPL=ForceGluePointList();
        pGPL->Mirror(rRef1,rRef2,this);
    }
}

void SdrObject::NbcShearGluePoints(const Point& rRef, long nWink, double tn, bool bVShear)
{
    // erst Const-Aufruf um zu sehen, ob
    // ueberhaupt Klebepunkte da sind
    // const-Aufruf erzwingen!
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

SdrObject* SdrObject::ImpConvertToContourObj(SdrObject* pRet, BOOL bForceLineDash) const
{
    bool bNoChange(true);

    if(pRet->LineGeometryUsageIsNecessary())
    {
        basegfx::B2DPolyPolygon aMergedLineFillPolyPolygon;
        basegfx::B2DPolyPolygon aMergedHairlinePolyPolygon;
        const drawinglayer::primitive2d::Primitive2DSequence xSequence(pRet->GetViewContact().getViewIndependentPrimitive2DSequence());

        if(xSequence.hasElements())
        {
            // use neutral ViewInformation
            const drawinglayer::geometry::ViewInformation2D aViewInformation2D(0);

            // create extractor, process and get result
            drawinglayer::processor2d::LineGeometryExtractor2D aExtractor(aViewInformation2D);
            aExtractor.process(xSequence);

            // #i102241# check for line results
            const std::vector< basegfx::B2DPolygon >& rHairlineVector = aExtractor.getExtractedHairlines();

            if(rHairlineVector.size())
            {
                // for SdrObject creation, just copy all to a single Hairline-PolyPolygon
                for(sal_uInt32 a(0); a < rHairlineVector.size(); a++)
                {
                    aMergedHairlinePolyPolygon.append(rHairlineVector[a]);
                }
            }

            // #i102241# check for fill rsults
            const std::vector< basegfx::B2DPolyPolygon >& rLineFillVector(aExtractor.getExtractedLineFills());

            if(rLineFillVector.size())
            {
                // merge to a single PolyPolygon (OR)
                aMergedLineFillPolyPolygon = basegfx::tools::mergeToSinglePolyPolygon(rLineFillVector);
            }
        }

        //  || aMergedHairlinePolyPolygon.Count() removed; the conversion is ONLY
        // useful when new closed filled polygons are created
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
                UINT16 nTransLine = ((const XLineTransparenceItem&)(aSet.Get(XATTR_LINETRANSPARENCE))).GetValue();
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
                    // #107600# use new boolean here
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

// convert this path object to contour object, even when it is a group
SdrObject* SdrObject::ConvertToContourObj(SdrObject* pRet, BOOL bForceLineDash) const
{
    if(pRet->ISA(SdrObjGroup))
    {
        SdrObjList* pObjList2 = pRet->GetSubList();
        SdrObject* pGroup = new SdrObjGroup;
        pGroup->SetModel(pRet->GetModel());

        for(UINT32 a=0;a<pObjList2->GetObjCount();a++)
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

SdrObject* SdrObject::ConvertToPolyObj(BOOL bBezier, BOOL bLineToArea) const
{
    SdrObject* pRet = DoConvertToPolyObj(bBezier);

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

SdrObject* SdrObject::DoConvertToPolyObj(BOOL /*bBezier*/) const
{
    return NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrObject::SetInserted(sal_Bool bIns)
{
    if (bIns!=IsInserted()) {
        bInserted=bIns;
        Rectangle aBoundRect0(GetLastBoundRect());
        if (bIns) SendUserCall(SDRUSERCALL_INSERTED,aBoundRect0);
        else SendUserCall(SDRUSERCALL_REMOVED,aBoundRect0);

        if (pPlusData!=NULL && pPlusData->pBroadcast!=NULL) { // #42522#
            SdrHint aHint(*this);
            aHint.SetKind(bIns?HINT_OBJINSERTED:HINT_OBJREMOVED);
            pPlusData->pBroadcast->Broadcast(aHint);
        }
    }
}

void SdrObject::SetMoveProtect(sal_Bool bProt)
{
    if(IsMoveProtect() != bProt)
    {
        // #i77187# secured and simplified
        bMovProt = bProt;
        SetChanged();
        BroadcastObjectChange();
    }
}

void SdrObject::SetResizeProtect(sal_Bool bProt)
{
    if(IsResizeProtect() != bProt)
    {
        // #i77187# secured and simplified
        bSizProt = bProt;
        SetChanged();
        BroadcastObjectChange();
    }
}

void SdrObject::SetPrintable(sal_Bool bPrn)
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

void SdrObject::SetVisible(sal_Bool bVisible)
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

USHORT SdrObject::GetUserDataCount() const
{
    if (pPlusData==NULL || pPlusData->pUserDataList==NULL) return 0;
    return pPlusData->pUserDataList->GetUserDataCount();
}

SdrObjUserData* SdrObject::GetUserData(USHORT nNum) const
{
    if (pPlusData==NULL || pPlusData->pUserDataList==NULL) return NULL;
    return pPlusData->pUserDataList->GetUserData(nNum);
}

void SdrObject::InsertUserData(SdrObjUserData* pData, USHORT nPos)
{
    if (pData!=NULL) {
        ImpForcePlusData();
        if (pPlusData->pUserDataList==NULL) pPlusData->pUserDataList=new SdrObjUserDataList;
        pPlusData->pUserDataList->InsertUserData(pData,nPos);
    } else {
        OSL_FAIL("SdrObject::InsertUserData(): pData ist NULL-Pointer");
    }
}

void SdrObject::DeleteUserData(USHORT nNum)
{
    USHORT nAnz=GetUserDataCount();
    if (nNum<nAnz) {
        pPlusData->pUserDataList->DeleteUserData(nNum);
        if (nAnz==1)  {
            delete pPlusData->pUserDataList;
            pPlusData->pUserDataList=NULL;
        }
    } else {
        OSL_FAIL("SdrObject::DeleteUserData(): ungueltiger Index");
    }
}

void SdrObject::SendUserCall(SdrUserCallType eUserCall, const Rectangle& rBoundRect) const
{
    SdrObjGroup* pGroup = NULL;

    if( pObjList && pObjList->GetListKind() == SDROBJLIST_GROUPOBJ )
        pGroup = (SdrObjGroup*) pObjList->GetOwnerObj();

    if ( pUserCall )
    {
        // UserCall ausfuehren
        pUserCall->Changed( *this, eUserCall, rBoundRect );
    }

    while( pGroup )
    {
        // Gruppe benachrichtigen
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

// ItemPool fuer dieses Objekt wechseln
void SdrObject::MigrateItemPool(SfxItemPool* pSrcPool, SfxItemPool* pDestPool, SdrModel* pNewModel)
{
    if(pSrcPool && pDestPool && (pSrcPool != pDestPool))
    {
        GetProperties().MoveToItemPool(pSrcPool, pDestPool, pNewModel);
    }
}

sal_Bool SdrObject::IsTransparent( BOOL /*bCheckForAlphaChannel*/) const
{
    bool bRet = false;

    if( IsGroupObject() )
    {
        SdrObjListIter aIter( *GetSubList(), IM_DEEPNOGROUPS );

        for( SdrObject* pO = aIter.Next(); pO && !bRet; pO = aIter.Next() )
        {
            const SfxItemSet& rAttr = pO->GetMergedItemSet();

            if( ( ( (const XFillTransparenceItem&) rAttr.Get( XATTR_FILLTRANSPARENCE ) ).GetValue() ||
                  ( (const XLineTransparenceItem&) rAttr.Get( XATTR_LINETRANSPARENCE ) ).GetValue() ) ||
                ( ( rAttr.GetItemState( XATTR_FILLFLOATTRANSPARENCE ) == SFX_ITEM_SET ) &&
                  ( (const XFillFloatTransparenceItem&) rAttr.Get( XATTR_FILLFLOATTRANSPARENCE ) ).IsEnabled() ) )
            {
                bRet = TRUE;
            }
            else if( pO->ISA( SdrGrafObj ) )
            {
                SdrGrafObj* pGrafObj = (SdrGrafObj*) pO;
                if( ( (const SdrGrafTransparenceItem&) rAttr.Get( SDRATTR_GRAFTRANSPARENCE ) ).GetValue() ||
                    ( pGrafObj->GetGraphicType() == GRAPHIC_BITMAP && pGrafObj->GetGraphic().GetBitmapEx().IsAlpha() ) )
                {
                    bRet = TRUE;
                }
            }
        }
    }
    else
    {
        const SfxItemSet& rAttr = GetMergedItemSet();

        if( ( ( (const XFillTransparenceItem&) rAttr.Get( XATTR_FILLTRANSPARENCE ) ).GetValue() ||
              ( (const XLineTransparenceItem&) rAttr.Get( XATTR_LINETRANSPARENCE ) ).GetValue() ) ||
            ( ( rAttr.GetItemState( XATTR_FILLFLOATTRANSPARENCE ) == SFX_ITEM_SET ) &&
              ( (const XFillFloatTransparenceItem&) rAttr.Get( XATTR_FILLFLOATTRANSPARENCE ) ).IsEnabled() ) )
        {
            bRet = TRUE;
        }
        else if( ISA( SdrGrafObj ) )
        {
            SdrGrafObj* pGrafObj = (SdrGrafObj*) this;

            // #i25616#
            bRet = pGrafObj->IsObjectTransparent();
        }
    }

    return bRet;
}

void SdrObject::impl_setUnoShape( const uno::Reference< uno::XInterface >& _rxUnoShape )
{
    maWeakUnoShape = _rxUnoShape;
    mpSvxShape = SvxShape::getImplementation( _rxUnoShape );
//    OSL_ENSURE( mpSvxShape || !_rxUnoShape.is(),
//        "SdrObject::setUnoShape: not sure it's a good idea to have an XShape which is not implemented by SvxShape ..." );
}

/** only for internal use! */
SvxShape* SdrObject::getSvxShape() const
{
    DBG_TESTSOLARMUTEX();
        // retrieving the impl pointer and subsequently using it is not thread-safe, of course, so it needs to be
        // guarded by the SolarMutex

#if OSL_DEBUG_LEVE > 0
    uno::Reference< uno::XInterface > xShape( maWeakUnoShape );
    OSL_ENSURE( !( !xShapeGuard.is() && mpSvxShape ),
        "SdrObject::getSvxShape: still having IMPL-Pointer to dead object!" );
#endif

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
// homogen 3x3 matrices containing the transformation of the SdrObject. At the
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
    SfxMapUnit eMapUnit = GetObjectItemSet().GetPool()->GetMetric(0);
    if(eMapUnit != SFX_MAPUNIT_100TH_MM)
    {
        switch(eMapUnit)
        {
            case SFX_MAPUNIT_TWIP :
            {
                // postion
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

    return sal_False;
}

// sets the base geometry of the object using infos contained in the homogen 3x3 matrix.
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
    SfxMapUnit eMapUnit = GetObjectItemSet().GetPool()->GetMetric(0);
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

// #116168#
// Give info if object is in destruction
sal_Bool SdrObject::IsInDestruction() const
{
    if(pModel)
        return pModel->IsInDestruction();
    return sal_False;
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

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//   @@@@  @@@@@  @@@@@@  @@@@@  @@@@   @@@@  @@@@@@  @@@@  @@@@@  @@  @@
//  @@  @@ @@  @@     @@  @@    @@  @@ @@  @@   @@   @@  @@ @@  @@ @@  @@
//  @@  @@ @@  @@     @@  @@    @@  @@ @@       @@   @@  @@ @@  @@ @@  @@
//  @@  @@ @@@@@      @@  @@@@  @@@@@@ @@       @@   @@  @@ @@@@@   @@@@
//  @@  @@ @@  @@     @@  @@    @@  @@ @@       @@   @@  @@ @@  @@   @@
//  @@  @@ @@  @@ @@  @@  @@    @@  @@ @@  @@   @@   @@  @@ @@  @@   @@
//   @@@@  @@@@@   @@@@   @@    @@  @@  @@@@    @@    @@@@  @@  @@   @@
//
////////////////////////////////////////////////////////////////////////////////////////////////////

SdrObjFactory::SdrObjFactory(UINT32 nInvent, UINT16 nIdent, SdrPage* pNewPage, SdrModel* pNewModel)
{
    nInventor=nInvent;
    nIdentifier=nIdent;
    pNewObj=NULL;
    pPage=pNewPage;
    pModel=pNewModel;
    pObj=NULL;
    pNewData=NULL;
}

SdrObjFactory::SdrObjFactory(UINT32 nInvent, UINT16 nIdent, SdrObject* pObj1)
{
    nInventor=nInvent;
    nIdentifier=nIdent;
    pNewObj=NULL;
    pPage=NULL;
    pModel=NULL;
    pObj=pObj1;
    pNewData=NULL;
}

SdrObject* SdrObjFactory::MakeNewObject(UINT32 nInvent, UINT16 nIdent, SdrPage* pPage, SdrModel* pModel)
{
    if(pModel == NULL && pPage != NULL)
        pModel = pPage->GetModel();
    SdrObject* pObj = NULL;

    if(nInvent == SdrInventor)
    {
        switch (nIdent)
        {
            case USHORT(OBJ_NONE       ): pObj=new SdrObject;                   break;
            case USHORT(OBJ_GRUP       ): pObj=new SdrObjGroup;                 break;
            case USHORT(OBJ_LINE       ): pObj=new SdrPathObj(OBJ_LINE       ); break;
            case USHORT(OBJ_POLY       ): pObj=new SdrPathObj(OBJ_POLY       ); break;
            case USHORT(OBJ_PLIN       ): pObj=new SdrPathObj(OBJ_PLIN       ); break;
            case USHORT(OBJ_PATHLINE   ): pObj=new SdrPathObj(OBJ_PATHLINE   ); break;
            case USHORT(OBJ_PATHFILL   ): pObj=new SdrPathObj(OBJ_PATHFILL   ); break;
            case USHORT(OBJ_FREELINE   ): pObj=new SdrPathObj(OBJ_FREELINE   ); break;
            case USHORT(OBJ_FREEFILL   ): pObj=new SdrPathObj(OBJ_FREEFILL   ); break;
            case USHORT(OBJ_PATHPOLY   ): pObj=new SdrPathObj(OBJ_POLY       ); break;
            case USHORT(OBJ_PATHPLIN   ): pObj=new SdrPathObj(OBJ_PLIN       ); break;
            case USHORT(OBJ_EDGE       ): pObj=new SdrEdgeObj;                  break;
            case USHORT(OBJ_RECT       ): pObj=new SdrRectObj;                  break;
            case USHORT(OBJ_CIRC       ): pObj=new SdrCircObj(OBJ_CIRC       ); break;
            case USHORT(OBJ_SECT       ): pObj=new SdrCircObj(OBJ_SECT       ); break;
            case USHORT(OBJ_CARC       ): pObj=new SdrCircObj(OBJ_CARC       ); break;
            case USHORT(OBJ_CCUT       ): pObj=new SdrCircObj(OBJ_CCUT       ); break;
            case USHORT(OBJ_TEXT       ): pObj=new SdrRectObj(OBJ_TEXT       ); break;
            case USHORT(OBJ_TEXTEXT    ): pObj=new SdrRectObj(OBJ_TEXTEXT    ); break;
            case USHORT(OBJ_TITLETEXT  ): pObj=new SdrRectObj(OBJ_TITLETEXT  ); break;
            case USHORT(OBJ_OUTLINETEXT): pObj=new SdrRectObj(OBJ_OUTLINETEXT); break;
            case USHORT(OBJ_MEASURE    ): pObj=new SdrMeasureObj;               break;
            case USHORT(OBJ_GRAF       ): pObj=new SdrGrafObj;                  break;
            case USHORT(OBJ_OLE2       ): pObj=new SdrOle2Obj;                  break;
            case USHORT(OBJ_FRAME      ): pObj=new SdrOle2Obj(TRUE);            break;
            case USHORT(OBJ_CAPTION    ): pObj=new SdrCaptionObj;               break;
            case USHORT(OBJ_PAGE       ): pObj=new SdrPageObj;                  break;
            case USHORT(OBJ_UNO        ): pObj=new SdrUnoObj(String());         break;
            case USHORT(OBJ_CUSTOMSHAPE  ): pObj=new SdrObjCustomShape();       break;
            case USHORT(OBJ_MEDIA      ): pObj=new SdrMediaObj();               break;
            case USHORT(OBJ_TABLE      ): pObj=new ::sdr::table::SdrTableObj(pModel);   break;
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
        // Na wenn's denn keiner will ...
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

SdrObjUserData* SdrObjFactory::MakeNewObjUserData(UINT32 nInvent, UINT16 nIdent, SdrObject* pObj1)
{
    SdrObjUserData* pData=NULL;
    if (nInvent==SdrInventor) {
        switch (nIdent)
        {
            case USHORT(SDRUSERDATA_OBJTEXTLINK) : pData=new ImpSdrObjTextLinkUserData((SdrTextObj*)pObj1); break;
        }
    }
    if (pData==NULL) {
        SdrObjFactory aFact(nInvent,nIdent,pObj1);
        SdrLinkList& rLL=ImpGetUserMakeObjUserDataHdl();
        unsigned nAnz=rLL.GetLinkCount();
        unsigned i=0;
        while (i<nAnz && pData==NULL) {
            rLL.GetLink(i).Call((void*)&aFact);
            pData=aFact.pNewData;
            i++;
        }
    }
    return pData;
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

// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
