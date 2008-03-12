/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: svdobj.cxx,v $
 *
 *  $Revision: 1.95 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 09:54:34 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif

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
#include "svdxout.hxx"
#include <svx/svdetc.hxx>
#include <svx/svdtrans.hxx>
#include <svx/svdhdl.hxx>
#include <svx/svddrag.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdovirt.hxx>  // Fuer Add/Del Ref
#include <svx/svdview.hxx>   // fuer Dragging (Ortho abfragen)
#include "svdscrol.hxx"
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

////////////////////////////////////////////////////////////////////////////////////////////////////

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
#include "xlinjoit.hxx"
#include <svx/unopage.hxx>
#include <svx/eeitem.hxx>
#include <svx/xenum.hxx>
#include <svx/xgrad.hxx>
#include <svx/xhatch.hxx>
#include <svx/xflhtit.hxx>
#include <svx/xbtmpit.hxx>

#ifndef _SVDPOOL_HXX
#include <svx/svdpool.hxx>
#endif

#ifndef _MyEDITENG_HXX
#include <svx/editeng.hxx>
#endif

#ifndef _SV_SALBTYPE_HXX
#include <vcl/salbtype.hxx>     // FRound
#endif

#ifndef _SFX_WHITER_HXX
#include <svtools/whiter.hxx>
#endif

// #97849#
#ifndef _SVX_FMMODEL_HXX
#include <svx/fmmodel.hxx>
#endif

#ifndef _SFX_OBJSH_HXX
#include <sfx2/objsh.hxx>
#endif

#ifndef _SFXOBJFACE_HXX
#include <sfx2/objface.hxx>
#endif

#ifndef _SVX_SVDOIMP_HXX
#include "svdoimp.hxx"
#endif

#ifndef _VCL_GRAPHICTOOLS_HXX_
#include <vcl/graphictools.hxx>
#endif

#ifndef INCLUDED_SVTOOLS_COLORCFG_HXX
#include <svtools/colorcfg.hxx>
#endif

#ifndef _SDR_PROPERTIES_EMPTYPROPERTIES_HXX
#include <svx/sdr/properties/emptyproperties.hxx>
#endif

// #110094#
#ifndef _SDR_CONTACT_VIEWCONTACTOFSDROBJ_HXX
#include <svx/sdr/contact/viewcontactofsdrobj.hxx>
#endif

#ifndef _SDR_CONTACT_VIEWCONTACTOFGRAPHIC_HXX
#include <svx/sdr/contact/viewcontactofgraphic.hxx>
#endif

#ifndef _SDR_CONTACT_OBJECTCONTACTOFOBJLISTPAINTER_HXX
#include <svx/sdr/contact/objectcontactofobjlistpainter.hxx>
#endif

#ifndef _SDR_CONTACT_DISPLAYINFO_HXX
#include <svx/sdr/contact/displayinfo.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYGON_HXX
#include <basegfx/polygon/b2dpolygon.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b2dpolygontools.hxx>
#endif

#ifndef _BGFX_MATRIX_B2DHOMMATRIX_HXX
#include <basegfx/matrix/b2dhommatrix.hxx>
#endif

#ifndef _BGFX_POLYPOLYGON_B2DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#endif

#ifndef _BGFX_RANGE_B2DRANGE_HXX
#include <basegfx/range/b2drange.hxx>
#endif
#ifndef _SVX_UNOSHAPE_HXX
#include <svx/unoshape.hxx>
#endif

#ifndef _SV_VIRDEV_HXX
#include <vcl/virdev.hxx>
#endif

#include "svx/svdotable.hxx"

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

FASTBOOL SdrObjUserData::HasMacro(const SdrObject* /*pObj*/) const
{
    return FALSE;
}

SdrObject* SdrObjUserData::CheckMacroHit(const SdrObjMacroHitRec& rRec, const SdrObject* pObj) const
{
    if (pObj==NULL) return NULL;
    return pObj->CheckHit(rRec.aPos,rRec.nTol,rRec.pVisiLayer);
}

Pointer SdrObjUserData::GetMacroPointer(const SdrObjMacroHitRec& /*rRec*/, const SdrObject* /*pObj*/) const
{
    return Pointer(POINTER_REFHAND);
}

void SdrObjUserData::PaintMacro(XOutputDevice& rXOut, const Rectangle& /*rDirtyRect*/, const SdrObjMacroHitRec& /*rRec*/, const SdrObject* pObj) const
{
    if (pObj==NULL) return;
    Color aBlackColor( COL_BLACK );
    Color aTranspColor( COL_TRANSPARENT );
    rXOut.OverrideLineColor( aBlackColor );
    rXOut.OverrideFillColor( aTranspColor );
    RasterOp eRop0=rXOut.GetRasterOp();
    rXOut.SetRasterOp(ROP_INVERT);
    basegfx::B2DPolyPolygon aPP(pObj->TakeXorPoly(sal_True));
    const sal_uInt32 nAnz(aPP.count());
    for(sal_uInt32 nNum(0L); nNum < nAnz; nNum++)
    {
        rXOut.DrawPolyLine(aPP.getB2DPolygon(nNum));
    }
    rXOut.SetRasterOp(eRop0);
}

FASTBOOL SdrObjUserData::DoMacro(const SdrObjMacroHitRec& /*rRec*/, SdrObject* /*pObj*/)
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
                    DBG_ERROR("SdrObjPlusData::Clone(): UserData.Clone() liefert NULL");
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
        ((SdrObject*)this)->mpProperties = ((SdrObject*)this)->CreateObjectSpecificProperties();
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
        ((SdrObject*)this)->mpViewContact = ((SdrObject*)this)->CreateObjectSpecificViewContact();
    }

    return *mpViewContact;
}

// DrawContact support: Methods for handling DrawHierarchy changes
void SdrObject::ActionRemoved() const
{
    // Do necessary ViewContact actions
    GetViewContact().ActionRemoved();
}

// DrawContact support: Methods for handling DrawHierarchy changes
void SdrObject::ActionInserted() const
{
    // Do necessary ViewContact actions
    GetViewContact().ActionInserted();
}

// DrawContact support: Methods for handling Object changes
void SdrObject::ActionChanged() const
{
    // #i34008#
    // Forward change call to MasterPageDescriptor if BackgroundObject was changed
    if(GetPage()
        && GetPage()->GetBackgroundObj()
        && GetPage()->GetBackgroundObj() == this)
    {
        if(GetPage()->TRG_HasMasterPage())
        {
            GetPage()->TRG_GetMasterPageDescriptorViewContact().ActionChanged();
        }
    }

    // Do necessary ViewContact actions
    GetViewContact().ActionChanged();
}

//////////////////////////////////////////////////////////////////////////////

void SdrObject::SetBoundRectDirty()
{
    bBoundRectDirty = sal_True;
}

//////////////////////////////////////////////////////////////////////////////

DBG_NAME(SdrObject);
TYPEINIT1(SdrObject,SfxListener);

SdrObject::SdrObject()
:   mpProperties(0L),
    mpViewContact(0L),
    pObjList(NULL),
    pPage(NULL),
    pModel(NULL),
    pUserCall(NULL),
    pPlusData(NULL),
    nOrdNum(0),
    mnNavigationPosition(SAL_MAX_UINT32),
    mnLayerID(0),
    mpSvxShape(0)
{
    DBG_CTOR(SdrObject,NULL);
    bVirtObj         =FALSE;
    bBoundRectDirty  =TRUE;
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
        uno::Reference< uno::XInterface > xShape;
        SvxShape* pSvxShape = getSvxShape( xShape );
        if ( pSvxShape )
        {
            pSvxShape->InvalidateSdrObject();
            uno::Reference< lang::XComponent > xShapeComp( xShape, uno::UNO_QUERY_THROW );
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
        mpViewContact->PrepareDelete();
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

    uno::Reference< uno::XInterface > xShape;
    SvxShape* pShape = pObject->getSvxShape( xShape );
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
        bBoundRectDirty=TRUE;
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
        uno::Reference< uno::XInterface > xShapeGuard;
        SvxShape* pShape = getSvxShape( xShapeGuard );
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
        delete mpGlobalItemPool;
        delete pGlobalOutlPool;
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

FASTBOOL SdrObject::HasRefPoint() const
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
        pPlusData->aObjName = rStr;
        SetChanged();
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
        pPlusData->aObjTitle = rStr;
        SetChanged();
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
        pPlusData->aObjDescription = rStr;
        SetChanged();
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
    if (bBoundRectDirty) {
        ((SdrObject*)this)->RecalcBoundRect();
        ((SdrObject*)this)->bBoundRectDirty=FALSE;
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

sal_Bool SdrObject::DoPaintObject(XOutputDevice& rXOut, const SdrPaintInfoRec& /*rInfoRec*/) const
{
    Color aRedColor( COL_RED );
    Color aYellowColor( COL_YELLOW );
    rXOut.OverrideLineColor( aRedColor );
    rXOut.OverrideFillColor( aYellowColor );
    rXOut.DrawRect(GetCurrentBoundRect());

    return TRUE;
}

// Tooling for painting a single object to a OutputDevice.
sal_Bool SdrObject::SingleObjectPainter(XOutputDevice& rXOut, const SdrPaintInfoRec& rInfoRec) const
{
    sdr::contact::SdrObjectVector aObjectVector;
    aObjectVector.push_back(this);

    sdr::contact::ObjectContactOfObjListPainter aPainter(aObjectVector);
    sdr::contact::DisplayInfo aDisplayInfo;

    aDisplayInfo.SetExtendedOutputDevice(&rXOut);
    aDisplayInfo.SetPaintInfoRec((SdrPaintInfoRec*)&rInfoRec);
    aDisplayInfo.SetOutputDevice(rXOut.GetOutDev());

    // do processing
    aPainter.ProcessDisplay(aDisplayInfo);

    // prepare delete
    aPainter.PrepareDelete();

    return sal_True;
}

::std::auto_ptr< SdrLineGeometry >  SdrObject::CreateLinePoly(sal_Bool bForceOnePixel, sal_Bool bForceTwoPixel) const
{
    basegfx::B2DPolyPolygon aAreaPolyPolygon;
    basegfx::B2DPolyPolygon aLinePolyPolygon;

    // get XOR Poly as base
    basegfx::B2DPolyPolygon aTmpPolyPolygon(TakeXorPoly(TRUE));

    // get ImpLineStyleParameterPack
    ImpLineStyleParameterPack aLineAttr(GetMergedItemSet(), bForceOnePixel || bForceTwoPixel);
    ImpLineGeometryCreator aLineCreator(aLineAttr, aAreaPolyPolygon, aLinePolyPolygon);

    // compute single lines
    for(sal_uInt32 a(0L); a < aTmpPolyPolygon.count(); a++)
    {
        // expand splines into polygons and convert to double
        basegfx::B2DPolygon aCandidate(aTmpPolyPolygon.getB2DPolygon(a));
        aCandidate.removeDoublePoints();

        if(aCandidate.areControlPointsUsed())
        {
            aCandidate = basegfx::tools::adaptiveSubdivideByAngle(aCandidate);
        }

        aLineCreator.AddPolygon(aCandidate);
    }

    if(aAreaPolyPolygon.count() || aLinePolyPolygon.count())
    {
        return ::std::auto_ptr< SdrLineGeometry > (new SdrLineGeometry(
            aAreaPolyPolygon, aLinePolyPolygon, aLineAttr, bForceOnePixel, bForceTwoPixel));
    }
    else
    {
        return ::std::auto_ptr< SdrLineGeometry > (NULL);
    }
}

::std::auto_ptr< SdrLineGeometry > SdrObject::ImpPrepareLineGeometry( XOutputDevice& rXOut, const SfxItemSet& rSet) const
{
    XLineStyle eXLS = (XLineStyle)((const XLineStyleItem&)rSet.Get(XATTR_LINESTYLE)).GetValue();
    if(eXLS != XLINE_NONE)
    {
        // need to force single point line?
        INT32 nLineWidth = ((const XLineWidthItem&)(rSet.Get(XATTR_LINEWIDTH))).GetValue();
        Size aSize(nLineWidth, nLineWidth);
        aSize = rXOut.GetOutDev()->LogicToPixel(aSize);
        BOOL bForceOnePixel(aSize.Width() <= 1 || aSize.Height() <= 1);
        BOOL bForceTwoPixel(!bForceOnePixel && (aSize.Width() <= 2 || aSize.Height() <= 2));

        // no force to two pixel when connected to MetaFile, so that not
        // four lines instead of one is recorded (e.g.)
        if(bForceTwoPixel && rXOut.GetOutDev()->GetConnectMetaFile())
            bForceTwoPixel = FALSE;

        // #78210# switch off bForceTwoPixel when line draft mode
        if(bForceTwoPixel)
        {
            bForceTwoPixel = FALSE;
            bForceOnePixel = TRUE;
        }

        // create line geometry
        return CreateLinePoly(bForceOnePixel, bForceTwoPixel);
    }

    return ::std::auto_ptr< SdrLineGeometry > (0L);
}

void SdrObject::ImpDrawShadowLineGeometry(
    XOutputDevice& rXOut, const SfxItemSet& rSet, SdrLineGeometry& rLineGeometry) const
{
    sal_Int32 nXDist = ((SdrShadowXDistItem&)(rSet.Get(SDRATTR_SHADOWXDIST))).GetValue();
    sal_Int32 nYDist = ((SdrShadowYDistItem&)(rSet.Get(SDRATTR_SHADOWYDIST))).GetValue();
    const SdrShadowColorItem& rShadColItem = ((SdrShadowColorItem&)(rSet.Get(SDRATTR_SHADOWCOLOR)));
    Color aColor(rShadColItem.GetColorValue());
    sal_uInt16 nTrans = ((SdrShadowTransparenceItem&)(rSet.Get(SDRATTR_SHADOWTRANSPARENCE))).GetValue();

    // draw shadow line geometry
    ImpDrawLineGeometry(rXOut, aColor, nTrans, rLineGeometry, nXDist, nYDist);
}

void SdrObject::ImpDrawColorLineGeometry(
    XOutputDevice& rXOut, const SfxItemSet& rSet, SdrLineGeometry& rLineGeometry) const
{
    Color aColor = ((XLineColorItem&)rSet.Get(XATTR_LINECOLOR)).GetColorValue();
    sal_uInt16 nTrans = ((const XLineTransparenceItem&)(rSet.Get(XATTR_LINETRANSPARENCE))).GetValue();

    // draw the line geometry
    ImpDrawLineGeometry(rXOut, aColor, nTrans, rLineGeometry);
}

void SdrObject::ImpDrawLineGeometry(   XOutputDevice&   rXOut,
                                       Color&              rColor,
                                       sal_uInt16           nTransparence,
                                       SdrLineGeometry&    rLineGeometry,
                                       sal_Int32            nDX,
                                       sal_Int32            nDY             ) const
{
    Color aLineColor( rColor );

    // #72796# black/white option active?
    const UINT32 nOldDrawMode(rXOut.GetOutDev()->GetDrawMode());

    // #72796# if yes, force to DRAWMODE_BLACKFILL for these are LINES to be drawn as polygons
    if( ( nOldDrawMode & DRAWMODE_WHITEFILL ) && ( nOldDrawMode & DRAWMODE_BLACKLINE ) )
    {
        aLineColor = Color( COL_BLACK );
        rXOut.GetOutDev()->SetDrawMode( nOldDrawMode & (~DRAWMODE_WHITEFILL) );
    }
    else if( ( nOldDrawMode & DRAWMODE_SETTINGSFILL ) && ( nOldDrawMode & DRAWMODE_SETTINGSLINE ) )
    {
        svtools::ColorConfig aColorConfig;
        aLineColor = Color( aColorConfig.GetColorValue( svtools::FONTCOLOR ).nColor );
        rXOut.GetOutDev()->SetDrawMode( nOldDrawMode & (~DRAWMODE_SETTINGSFILL) );
    }

    // #103692# Hold local copy of geometry
    basegfx::B2DPolyPolygon aAreaPolyPolygon(rLineGeometry.GetAreaPolyPolygon());
    basegfx::B2DPolyPolygon aLinePolyPolygon(rLineGeometry.GetLinePolyPolygon());

    // #103692# Offset geometry (extracted from SdrObject::ImpDrawShadowLineGeometry)
    if( nDX || nDY )
    {
        // transformation necessary
        basegfx::B2DHomMatrix aMatrix;

        aMatrix.translate((double)nDX, (double)nDY);
        aAreaPolyPolygon.transform(aMatrix);
        aLinePolyPolygon.transform(aMatrix);
    }

    // #100127# Bracket output with a comment, if recording a Mtf
    bool bMtfCommentWritten( false );
    GDIMetaFile* pMtf=rXOut.GetOutDev()->GetConnectMetaFile();
    if( pMtf )
    {
        basegfx::B2DPolyPolygon aPolyPoly(TakeXorPoly(TRUE));

        // #103692# Offset original geometry, too
        if( nDX || nDY )
        {
            // transformation necessary
            basegfx::B2DHomMatrix aMatrix;
            aMatrix.translate(nDX, nDY);
            aPolyPoly.transform(aMatrix);
        }

        // for geometries with more than one polygon, dashing, arrows
        // etc. become ambiguous (e.g. measure objects have no arrows
        // on the end line), thus refrain from writing the comment
        // here.
        if( 1L == aPolyPoly.count() )
        {
            // add completely superfluous color action (gets overwritten
            // below), to store our line color reliably
            rXOut.GetOutDev()->SetLineColor(aLineColor);

            const ImpLineStyleParameterPack& rLineParameters = rLineGeometry.GetLineAttr();
            PolyPolygon aStartPoly( rLineParameters.GetStartPolyPolygon() );
            PolyPolygon aEndPoly( rLineParameters.GetEndPolyPolygon() );

            // scale arrows to specified stroke width
            if( aStartPoly.Count() && aStartPoly.GetObject(0).GetSize() )
            {
                Rectangle aBounds( aStartPoly.GetBoundRect() );

                // mirror and translate to origin
                aStartPoly.Scale(-1,-1);
                aStartPoly.Translate( Point(aBounds.GetWidth() / 2, aBounds.GetHeight()) );

                if( aBounds.GetWidth() )
                {
                    // #104527# Avoid division by zero. If rLineParameters.GetLineWidth
                    // is zero this is a hairline which can be handled as 1.0.
                    double fLineWidth(rLineParameters.GetLineWidth() ? (double)rLineParameters.GetLineWidth() : 1.0);

                    double fScale( (double)rLineParameters.GetStartWidth() / fLineWidth *
                                   (double)SvtGraphicStroke::normalizedArrowWidth / (double)aBounds.GetWidth() );
                    aStartPoly.Scale( fScale, fScale );
                }

                if( rLineParameters.IsStartCentered() )
                    aStartPoly.Translate( Point(0, -aStartPoly.GetBoundRect().GetHeight() / 2) );
            }
            if( aEndPoly.Count() && aEndPoly.GetObject(0).GetSize() )
            {
                Rectangle aBounds( aEndPoly.GetBoundRect() );

                // mirror and translate to origin
                aEndPoly.Scale(-1,-1);
                aEndPoly.Translate( Point(aBounds.GetWidth() / 2, aBounds.GetHeight()) );

                if( aBounds.GetWidth() )
                {
                    // #104527# Avoid division by zero. If rLineParameters.GetLineWidth
                    // is zero this is a hairline which we can be handled as 1.0.
                    double fLineWidth(rLineParameters.GetLineWidth() ? (double)rLineParameters.GetLineWidth() : 1.0);

                    double fScale( (double)rLineParameters.GetEndWidth() / fLineWidth *
                                   (double)SvtGraphicStroke::normalizedArrowWidth / (double)aBounds.GetWidth() );
                    aEndPoly.Scale( fScale, fScale );
                }

                if( rLineParameters.IsEndCentered() )
                    aEndPoly.Translate( Point(0, -aEndPoly.GetBoundRect().GetHeight() / 2) );
            }

            SvtGraphicStroke::JoinType eJoin;
            double fMiterLength = rLineParameters.GetLineWidth();
            switch( rLineParameters.GetLineJoint() )
            {
                case XLINEJOINT_NONE: eJoin = SvtGraphicStroke::joinNone;break;
                case XLINEJOINT_MIDDLE:
                case XLINEJOINT_MITER:
                {
                    eJoin = SvtGraphicStroke::joinMiter;
                    // assume GetLinejointMiterMinimumAngle returns degrees (currently returns 15.0)
                    double fSin = rtl::math::sin( M_PI * rLineParameters.GetLinejointMiterMinimumAngle() / 360.0 );
                    if( ! rtl::math::isNan( fSin ) )
                        fMiterLength /= fSin;
                }
                break;
                case XLINEJOINT_BEVEL: eJoin = SvtGraphicStroke::joinBevel;break;
                default:
                case XLINEJOINT_ROUND: eJoin = SvtGraphicStroke::joinRound;break;
            }

            const Polygon aSingleStrokePoly(aPolyPoly.getB2DPolygon(0L));
            SvtGraphicStroke aStroke( aSingleStrokePoly,
                                      aStartPoly,
                                      aEndPoly,
                                      nTransparence / 100.0,
                                      rLineParameters.GetLineWidth(),
                                      SvtGraphicStroke::capButt,
                                      eJoin,
                                      fMiterLength,
                                      rLineParameters.IsLineStyleSolid() ? SvtGraphicStroke::DashArray() : rLineParameters.GetDotDash() );

#ifdef DBG_UTIL
            ::rtl::OString aStr( aStroke.toString() );
#endif

            SvMemoryStream  aMemStm;

            aMemStm << aStroke;

            pMtf->AddAction( new MetaCommentAction( "XPATHSTROKE_SEQ_BEGIN", 0,
                                                    static_cast<const BYTE*>(aMemStm.GetData()),
                                                    aMemStm.Seek( STREAM_SEEK_TO_END ) ) );
            bMtfCommentWritten = true;
        }
    }

    if(nTransparence)
    {
        if(nTransparence != 100)
        {
            // to be shown line has transparence, output via MetaFile
            UINT8 nScaledTrans((UINT8)((nTransparence * 255)/100));
            Color aTransColor(nScaledTrans, nScaledTrans, nScaledTrans);
            Gradient aGradient(GRADIENT_LINEAR, aTransColor, aTransColor);
            GDIMetaFile aMetaFile;
            VirtualDevice aVDev;

            basegfx::B2DRange aFullRange;
            //Volume3D aVolume;

            MapMode aMap(rXOut.GetOutDev()->GetMapMode());

            // StepCount to someting small
            aGradient.SetSteps(3);

            // get bounds of geometry
            if(aAreaPolyPolygon.count())
            {
                basegfx::B2DRange aRange = basegfx::tools::getRange(aAreaPolyPolygon);
                aFullRange.expand(aRange);
            }

            if(aLinePolyPolygon.count())
            {
                basegfx::B2DRange aRange = basegfx::tools::getRange(aLinePolyPolygon);
                aFullRange.expand(aRange);
            }

            // get pixel size in logic coor for 1,2 pixel cases
            Size aSizeSinglePixel(1, 1);

            if(rLineGeometry.DoForceOnePixel() || rLineGeometry.DoForceTwoPixel())
            {
                aSizeSinglePixel = rXOut.GetOutDev()->PixelToLogic(aSizeSinglePixel);
            }

            // create BoundRectangle
            basegfx::B2DTuple aMinimum(aFullRange.getMinimum());
            basegfx::B2DTuple aMaximum(aFullRange.getMaximum());

            Rectangle aBound(
                FRound(aMinimum.getX()), FRound(aMinimum.getY()),
                FRound(aMaximum.getX()), FRound(aMaximum.getY()));

            if(rLineGeometry.DoForceOnePixel() || rLineGeometry.DoForceTwoPixel())
            {
                // enlarge aBound
                if(rLineGeometry.DoForceTwoPixel())
                {
                    aBound.Right() += 2 * (aSizeSinglePixel.Width() - 1);
                    aBound.Bottom() += 2 * (aSizeSinglePixel.Height() - 1);
                }
                else
                {
                    aBound.Right() += (aSizeSinglePixel.Width() - 1);
                    aBound.Bottom() += (aSizeSinglePixel.Height() - 1);
                }
            }

            // prepare VDev and MetaFile
            aVDev.EnableOutput(FALSE);
            aVDev.SetMapMode(rXOut.GetOutDev()->GetMapMode());
            aMetaFile.Record(&aVDev);
            aVDev.SetLineColor(aLineColor);
            aVDev.SetFillColor(aLineColor);
            aVDev.SetFont(rXOut.GetOutDev()->GetFont());
            aVDev.SetDrawMode(rXOut.GetOutDev()->GetDrawMode());
            aVDev.SetSettings(rXOut.GetOutDev()->GetSettings());
            aVDev.SetRefPoint(rXOut.GetOutDev()->GetRefPoint());

            // create output
            if(aAreaPolyPolygon.count())
            {
                PolyPolygon aVCLPolyPoly(aAreaPolyPolygon);

                for(UINT16 a=0;a<aVCLPolyPoly.Count();a++)
                {
                    aMetaFile.AddAction(new MetaPolygonAction(aVCLPolyPoly[a]));
                }
            }

            if(aLinePolyPolygon.count())
            {
                PolyPolygon aVCLLinePoly(aLinePolyPolygon);

                if(rLineGeometry.DoForceTwoPixel())
                {
                    UINT16 a;

                    for(a=0;a<aVCLLinePoly.Count();a++)
                        aMetaFile.AddAction(new MetaPolyLineAction(aVCLLinePoly[a]));

                    aVCLLinePoly.Move(aSizeSinglePixel.Width() - 1, 0);

                    for(a=0;a<aVCLLinePoly.Count();a++)
                        aMetaFile.AddAction(new MetaPolyLineAction(aVCLLinePoly[a]));

                    aVCLLinePoly.Move(0, aSizeSinglePixel.Height() - 1);

                    for(a=0;a<aVCLLinePoly.Count();a++)
                        aMetaFile.AddAction(new MetaPolyLineAction(aVCLLinePoly[a]));

                    aVCLLinePoly.Move(-aSizeSinglePixel.Width() - 1, 0);

                    for(a=0;a<aVCLLinePoly.Count();a++)
                        aMetaFile.AddAction(new MetaPolyLineAction(aVCLLinePoly[a]));
                }
                else
                {
                    for(UINT16 a=0;a<aVCLLinePoly.Count();a++)
                        aMetaFile.AddAction(new MetaPolyLineAction(aVCLLinePoly[a]));
                }
            }

            // draw metafile
            aMetaFile.Stop();
            aMetaFile.WindStart();
            aMap.SetOrigin(aBound.TopLeft());
            aMetaFile.SetPrefMapMode(aMap);
            aMetaFile.SetPrefSize(aBound.GetSize());
            rXOut.GetOutDev()->DrawTransparent(aMetaFile, aBound.TopLeft(), aBound.GetSize(), aGradient);
        }
    }
    else
    {
        // no transparence, simple output
        if(aAreaPolyPolygon.count())
        {
            PolyPolygon aVCLPolyPoly(aAreaPolyPolygon);

            rXOut.GetOutDev()->SetLineColor();
            rXOut.GetOutDev()->SetFillColor(aLineColor);

            for(UINT16 a=0;a<aVCLPolyPoly.Count();a++)
                rXOut.GetOutDev()->DrawPolygon(aVCLPolyPoly[a]);
        }

        if(aLinePolyPolygon.count())
        {
            PolyPolygon aVCLLinePoly(aLinePolyPolygon);
            rXOut.GetOutDev()->SetLineColor(aLineColor);
            rXOut.GetOutDev()->SetFillColor();

            if(rLineGeometry.DoForceTwoPixel())
            {
                PolyPolygon aPolyPolyPixel( rXOut.GetOutDev()->LogicToPixel(aVCLLinePoly) );
                BOOL bWasEnabled = rXOut.GetOutDev()->IsMapModeEnabled();
                rXOut.GetOutDev()->EnableMapMode(FALSE);
                UINT16 a;

                for(a=0;a<aVCLLinePoly.Count();a++)
                    rXOut.GetOutDev()->DrawPolyLine(aPolyPolyPixel[a]);

                aPolyPolyPixel.Move(1,0);

                for(a=0;a<aVCLLinePoly.Count();a++)
                    rXOut.GetOutDev()->DrawPolyLine(aPolyPolyPixel[a]);

                aPolyPolyPixel.Move(0,1);

                for(a=0;a<aVCLLinePoly.Count();a++)
                    rXOut.GetOutDev()->DrawPolyLine(aPolyPolyPixel[a]);

                aPolyPolyPixel.Move(-1,0);

                for(a=0;a<aVCLLinePoly.Count();a++)
                    rXOut.GetOutDev()->DrawPolyLine(aPolyPolyPixel[a]);

                rXOut.GetOutDev()->EnableMapMode(bWasEnabled);
            }
            else
            {
                for( UINT16 a = 0; a < aVCLLinePoly.Count(); a++ )
                {
                    const Polygon&  rPoly = aVCLLinePoly[ a ];
                    BOOL            bDrawn = FALSE;

                    if( rPoly.GetSize() == 2 )
                    {
                        if ( !rXOut.GetOutDev()->GetConnectMetaFile() )
                        {
                            const Line  aLine( rXOut.GetOutDev()->LogicToPixel( rPoly[ 0 ] ),
                                               rXOut.GetOutDev()->LogicToPixel( rPoly[ 1 ] ) );

                            if( aLine.GetLength() > 16000 )
                            {
                                Point       aPoint;
                                Rectangle   aOutRect2( aPoint, rXOut.GetOutDev()->GetOutputSizePixel() );
                                Line        aIntersection;

                                if( aLine.Intersection( aOutRect2, aIntersection ) )
                                {
                                    rXOut.GetOutDev()->DrawLine( rXOut.GetOutDev()->PixelToLogic( aIntersection.GetStart() ),
                                                                 rXOut.GetOutDev()->PixelToLogic( aIntersection.GetEnd() ) );
                                }
                                bDrawn = TRUE;
                            }
                        }
                    }
                    if( !bDrawn )
                        rXOut.GetOutDev()->DrawPolyLine( rPoly );
                }
            }
        }
    }

    // #100127# Bracket output with a comment, if recording a Mtf
    if( bMtfCommentWritten && pMtf )
        pMtf->AddAction( new MetaCommentAction( "XPATHSTROKE_SEQ_END" ) );

    rXOut.GetOutDev()->SetDrawMode( nOldDrawMode );
}

BOOL SdrObject::LineGeometryUsageIsNecessary() const
{
    XLineStyle eXLS = (XLineStyle)((const XLineStyleItem&)GetMergedItem(XATTR_LINESTYLE)).GetValue();
    return (eXLS != XLINE_NONE);
}

SdrObject* SdrObject::CheckHit(const Point& rPnt, USHORT nTol, const SetOfByte* pVisiLayer) const
{
    if(pVisiLayer && !pVisiLayer->IsSet(sal::static_int_cast< sal_uInt8 >(GetLayer())))
    {
        return 0L;
    }

    Rectangle aO(GetCurrentBoundRect());
    aO.Left()-=nTol; aO.Top()-=nTol; aO.Right()+=nTol; aO.Bottom()+=nTol;
    FASTBOOL bRet=aO.IsInside(rPnt);
    return bRet ? (SdrObject*)this : NULL;
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
    aOutRect=rObj.GetCurrentBoundRect();
    mnLayerID = rObj.mnLayerID;
    aAnchor =rObj.aAnchor;
    bVirtObj=rObj.bVirtObj;
    bSizProt=rObj.bSizProt;
    bMovProt=rObj.bMovProt;
    bNoPrint=rObj.bNoPrint;
    bMarkProt=rObj.bMarkProt;
    //EmptyPresObj wird nicht kopiert: nun doch! (25-07-1995, Joe)
    bEmptyPresObj =rObj.bEmptyPresObj;
    //NotVisibleAsMaster wird nicht kopiert: nun doch! (25-07-1995, Joe)
    bNotVisibleAsMaster=rObj.bNotVisibleAsMaster;

    bBoundRectDirty=rObj.bBoundRectDirty;
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

XubString SdrObject::GetWinkStr(long nWink, FASTBOOL bNoDegChar) const
{
    XubString aStr;
    if (pModel!=NULL) {
        pModel->TakeWinkStr(nWink,aStr,bNoDegChar);
    }
    return aStr;
}

XubString SdrObject::GetMetrStr(long nVal, MapUnit /*eWantMap*/, FASTBOOL bNoUnitChars) const
{
    XubString aStr;
    if (pModel!=NULL) {
        pModel->TakeMetricStr(nVal,aStr,bNoUnitChars);
    }
    return aStr;
}

basegfx::B2DPolyPolygon SdrObject::TakeXorPoly(sal_Bool /*bDetail*/) const
{
    basegfx::B2DPolyPolygon aRetval;
    const Rectangle aR(GetCurrentBoundRect());
    const basegfx::B2DRange aRange(aR.Left(), aR.Top(), aR.Right(), aR.Bottom());
    aRetval.append(basegfx::tools::createPolygonFromRect(aRange));

    return aRetval;
}

basegfx::B2DPolyPolygon SdrObject::TakeContour() const
{
    VirtualDevice   aBlackHole;
    GDIMetaFile     aMtf;
    SdrPaintInfoRec aPaintInfo;
    basegfx::B2DPolygon aXPoly;

    aBlackHole.EnableOutput( FALSE );
    aBlackHole.SetDrawMode( DRAWMODE_NOFILL );

    XOutputDevice   aXOut( &aBlackHole );
    SdrObject*      pClone = Clone();

    // #114164#
    // there was a loop when asking the contour from a shape with textanimation.
    // therefore we disable textanimation here since it doesn't add to the actual
    // contour of this clone
    if(pClone && ISA(SdrTextObj))
    {
        pClone->SetMergedItem( SdrTextAniKindItem(SDRTEXTANI_NONE));
    }

    if(pClone && ISA(SdrEdgeObj))
    {
        // #102344# Flat cloned SdrEdgeObj, copy connections to original object(s).
        // This is deleted later at delete pClone.
        SdrObject* pLeft = ((SdrEdgeObj*)this)->GetConnectedNode(TRUE);
        SdrObject* pRight = ((SdrEdgeObj*)this)->GetConnectedNode(FALSE);

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
    aNewSet.Put(XLineStyleItem(XLINE_SOLID));
    aNewSet.Put(XLineColorItem(String(), Color(COL_BLACK)));
    aNewSet.Put(XFillStyleItem(XFILL_NONE));
    pClone->SetMergedItemSet(aNewSet);

    aMtf.Record( &aBlackHole );

    //#i80528# SDRPAINTMODE_DRAFTTEXT|SDRPAINTMODE_DRAFTGRAF removed, need to test if contour
    // still works. Maybe i need to add evaluation of graphic and text actions and create
    // polygons accordingly.
    // I knew it: With text-using objects, this runs into a recursion with SdrTextObj::DoPaintObject
    // and the DrawOutliner setup. That setup uses TakeContour itself. Need to protect using the new
    // SDRPAINTMODE_CONTOUR. I will also use that for graphics.
    aPaintInfo.nPaintMode = SDRPAINTMODE_CONTOUR;

    pClone->SingleObjectPainter( aXOut, aPaintInfo ); // #110094#-17
    delete pClone;
    aMtf.Stop();
    aMtf.WindStart();
    basegfx::B2DPolyPolygon aNewPoly;

    for( ULONG nActionNum = 0, nActionAnz = aMtf.GetActionCount(); nActionNum < nActionAnz; nActionNum++ )
    {
        const MetaAction&   rAct = *aMtf.GetAction( nActionNum );
        BOOL                bXPoly = FALSE;

        switch( rAct.GetType() )
        {
            case META_RECT_ACTION:
            {
                const Rectangle& rRect = ( (const MetaRectAction&) rAct ).GetRect();

                if( rRect.GetWidth() && rRect.GetHeight() )
                {
                    const basegfx::B2DRange aRange(rRect.Left(), rRect.Top(), rRect.Right(), rRect.Bottom());
                    aXPoly = basegfx::tools::createPolygonFromRect(aRange);
                    bXPoly = TRUE;
                }
            }
            break;

            case META_ELLIPSE_ACTION:
            {
                const Rectangle& rRect = ( (const MetaEllipseAction&) rAct ).GetRect();

                if( rRect.GetWidth() && rRect.GetHeight() )
                {
                    aXPoly = basegfx::tools::createPolygonFromEllipse(basegfx::B2DPoint(rRect.Center().X(), rRect.Center().Y()), rRect.GetWidth() >> 1, rRect.GetHeight() >> 1);
                    bXPoly = TRUE;
                }
            }
            break;

            case META_POLYGON_ACTION:
            {
                const Polygon& rPoly2 = ( (const MetaPolygonAction&) rAct ).GetPolygon();

                if( rPoly2.GetSize() > 2 )
                {
                    aXPoly = rPoly2.getB2DPolygon();
                    bXPoly = TRUE;
                }
            }
            break;

            case META_POLYPOLYGON_ACTION:
            {
                const PolyPolygon& rPolyPoly = ( (const MetaPolyPolygonAction&) rAct ).GetPolyPolygon();

                if( rPolyPoly.Count() && ( rPolyPoly[ 0 ].GetSize() > 2 ) )
                    aNewPoly.append( rPolyPoly.getB2DPolyPolygon() );
            }
            break;

            case META_POLYLINE_ACTION:
            {
                const Polygon& rPoly2 = ( (const MetaPolyLineAction&) rAct ).GetPolygon();

                if( rPoly2.GetSize() > 1 )
                {
                    aXPoly = rPoly2.getB2DPolygon();
                    bXPoly = TRUE;
                }
            }
            break;

            case META_LINE_ACTION:
            {
                const Point aStart(((const MetaLineAction&) rAct ).GetStartPoint());
                const Point aEnd(((const MetaLineAction&) rAct ).GetEndPoint());
                aXPoly.clear();
                aXPoly.append(basegfx::B2DPoint(aStart.X(), aStart.Y()));
                aXPoly.append(basegfx::B2DPoint(aEnd.X(), aEnd.Y()));
                bXPoly = TRUE;
            }
            break;

            default:
            break;
        }

        if( bXPoly )
            aNewPoly.append( aXPoly );
    }

    // if we only have the outline of the object, we have _no_ contouir
    if( 1L == aNewPoly.count() )
        aNewPoly.clear();

    return aNewPoly;
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
    FASTBOOL bEcke=(eHdl==HDL_UPLFT || eHdl==HDL_UPRGT || eHdl==HDL_LWLFT || eHdl==HDL_LWRGT);
    FASTBOOL bOrtho=rDrag.GetView()!=NULL && rDrag.GetView()->IsOrtho();
    FASTBOOL bBigOrtho=bEcke && bOrtho && rDrag.GetView()->IsBigOrtho();
    Point aPos(rDrag.GetNow());
    FASTBOOL bLft=(eHdl==HDL_UPLFT || eHdl==HDL_LEFT  || eHdl==HDL_LWLFT);
    FASTBOOL bRgt=(eHdl==HDL_UPRGT || eHdl==HDL_RIGHT || eHdl==HDL_LWRGT);
    FASTBOOL bTop=(eHdl==HDL_UPRGT || eHdl==HDL_UPPER || eHdl==HDL_UPLFT);
    FASTBOOL bBtm=(eHdl==HDL_LWRGT || eHdl==HDL_LOWER || eHdl==HDL_LWLFT);
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
        FASTBOOL bXNeg=(nXMul<0)!=(nXDiv<0);
        FASTBOOL bYNeg=(nYMul<0)!=(nYDiv<0);
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
            FASTBOOL bUseX=(aXFact<aYFact) != bBigOrtho;
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

FASTBOOL SdrObject::HasSpecialDrag() const
{
    return FALSE; // noch nicht ganz fertig ...
}

FASTBOOL SdrObject::BegDrag(SdrDragStat& rDrag) const
{
    if (bSizProt) return FALSE; // Groesse geschuetzt
    const SdrHdl* pHdl=rDrag.GetHdl();
    SdrHdlKind eHdl=pHdl==NULL ? HDL_MOVE : pHdl->GetKind();
    if (eHdl==HDL_UPLFT || eHdl==HDL_UPPER || eHdl==HDL_UPRGT ||
        eHdl==HDL_LEFT  ||                    eHdl==HDL_RIGHT ||
        eHdl==HDL_LWLFT || eHdl==HDL_LOWER || eHdl==HDL_LWRGT) return TRUE;
    return FALSE;
}

FASTBOOL SdrObject::MovDrag(SdrDragStat& /*rDrag*/) const
{
    return TRUE;
}

FASTBOOL SdrObject::EndDrag(SdrDragStat& rDrag)
{
    Rectangle aNewRect(ImpDragCalcRect(rDrag));
    if (aNewRect!=GetSnapRect()) {
        SetSnapRect(aNewRect);
    }
    return TRUE;
}

void SdrObject::BrkDrag(SdrDragStat& /*rDrag*/) const
{
}

XubString SdrObject::GetDragComment(const SdrDragStat& /*rDrag*/, FASTBOOL /*bDragUndoComment*/, FASTBOOL /*bCreateComment*/) const
{
    return String();
}

basegfx::B2DPolyPolygon SdrObject::TakeDragPoly(const SdrDragStat& rDrag) const
{
    basegfx::B2DPolyPolygon aRetval;
    Rectangle aTmpRect(ImpDragCalcRect(rDrag));

    aRetval.append(basegfx::tools::createPolygonFromRect(basegfx::B2DRange(aTmpRect.Left(), aTmpRect.Top(), aTmpRect.Right(), aTmpRect.Bottom())));

    return aRetval;
}

// Create
FASTBOOL SdrObject::BegCreate(SdrDragStat& rStat)
{
    rStat.SetOrtho4Possible();
    Rectangle aRect1(rStat.GetStart(), rStat.GetNow());
    aRect1.Justify();
    rStat.SetActionRect(aRect1);
    aOutRect = aRect1;
    return TRUE;
}

FASTBOOL SdrObject::MovCreate(SdrDragStat& rStat)
{
    rStat.TakeCreateRect(aOutRect);
    rStat.SetActionRect(aOutRect);
    aOutRect.Justify();
    bBoundRectDirty=TRUE;
    bSnapRectDirty=TRUE;
    return TRUE;
}

FASTBOOL SdrObject::EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd)
{
    rStat.TakeCreateRect(aOutRect);
    aOutRect.Justify();
    SetRectsDirty();
    return (eCmd==SDRCREATE_FORCEEND || rStat.GetPointAnz()>=2);
}

void SdrObject::BrkCreate(SdrDragStat& /*rStat*/)
{
}

FASTBOOL SdrObject::BckCreate(SdrDragStat& /*rStat*/)
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
    FASTBOOL bXMirr=(xFact.GetNumerator()<0) != (xFact.GetDenominator()<0);
    FASTBOOL bYMirr=(yFact.GetNumerator()<0) != (yFact.GetDenominator()<0);
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

void SdrObject::NbcShear(const Point& rRef, long nWink, double tn, FASTBOOL bVShear)
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

void SdrObject::Shear(const Point& rRef, long nWink, double tn, FASTBOOL bVShear)
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

long SdrObject::GetShearAngle(FASTBOOL /*bVertical*/) const
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

FASTBOOL SdrObject::HasTextEdit() const
{
    return FALSE;
}

SdrObject* SdrObject::CheckTextEditHit(const Point& rPnt, USHORT nTol, const SetOfByte* pVisiLayer) const
{
    return CheckHit(rPnt,nTol,pVisiLayer);
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

FASTBOOL SdrObject::HasMacro() const
{
    SdrObjUserData* pData=ImpGetMacroUserData();
    return pData!=NULL ? pData->HasMacro(this) : FALSE;
}

SdrObject* SdrObject::CheckMacroHit(const SdrObjMacroHitRec& rRec) const
{
    SdrObjUserData* pData=ImpGetMacroUserData();
    if (pData!=NULL) {
        return pData->CheckMacroHit(rRec,this);
    }
    return CheckHit(rRec.aPos,rRec.nTol,rRec.pVisiLayer);
}

Pointer SdrObject::GetMacroPointer(const SdrObjMacroHitRec& rRec) const
{
    SdrObjUserData* pData=ImpGetMacroUserData();
    if (pData!=NULL) {
        return pData->GetMacroPointer(rRec,this);
    }
    return Pointer(POINTER_REFHAND);
}

void SdrObject::PaintMacro(XOutputDevice& rXOut, const Rectangle& rDirtyRect, const SdrObjMacroHitRec& rRec) const
{
    SdrObjUserData* pData=ImpGetMacroUserData();
    if (pData!=NULL) {
        pData->PaintMacro(rXOut,rDirtyRect,rRec,this);
    } else {
        Color aBlackColor( COL_BLACK );
        Color aTranspColor( COL_TRANSPARENT );
        rXOut.OverrideLineColor( aBlackColor );
        rXOut.OverrideFillColor( aTranspColor );
        RasterOp eRop0=rXOut.GetRasterOp();
        rXOut.SetRasterOp(ROP_INVERT);
        basegfx::B2DPolyPolygon aPP(TakeXorPoly(sal_True));
        const sal_uInt32 nAnz(aPP.count());
        for (sal_uInt32 nNum(0L); nNum < nAnz; nNum++)
        {
            rXOut.DrawPolyLine(aPP.getB2DPolygon(nNum));
        }
        rXOut.SetRasterOp(eRop0);
    }
}

FASTBOOL SdrObject::DoMacro(const SdrObjMacroHitRec& rRec)
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

void lcl_SetItem(SfxItemSet& rAttr, FASTBOOL bMerge, const SfxPoolItem& rItem)
{
    if (bMerge) rAttr.MergeValue(rItem,TRUE);
    else rAttr.Put(rItem);
}

void SdrObject::TakeNotPersistAttr(SfxItemSet& rAttr, FASTBOOL bMerge) const
{
    const Rectangle& rSnap=GetSnapRect();
    const Rectangle& rLogic=GetLogicRect();
    lcl_SetItem(rAttr,bMerge,SdrObjMoveProtectItem(IsMoveProtect()));
    lcl_SetItem(rAttr,bMerge,SdrObjSizeProtectItem(IsResizeProtect()));
    lcl_SetItem(rAttr,bMerge,SdrObjPrintableItem(IsPrintable()));
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

FASTBOOL SdrObject::IsNode() const
{
    return TRUE;
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

void SdrObject::SetGlueReallyAbsolute(FASTBOOL bOn)
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

void SdrObject::NbcShearGluePoints(const Point& rRef, long nWink, double tn, FASTBOOL bVShear)
{
    // erst Const-Aufruf um zu sehen, ob
    // ueberhaupt Klebepunkte da sind
    // const-Aufruf erzwingen!
    if (GetGluePointList()!=NULL) {
        SdrGluePointList* pGPL=ForceGluePointList();
        pGPL->Shear(rRef,nWink,tn,bVShear,this);
    }
}

FASTBOOL SdrObject::IsEdge() const
{
    return FALSE;
}

void SdrObject::ConnectToNode(FASTBOOL /*bTail1*/, SdrObject* /*pObj*/)
{
}

void SdrObject::DisconnectFromNode(FASTBOOL /*bTail1*/)
{
}

SdrObject* SdrObject::GetConnectedNode(FASTBOOL /*bTail1*/) const
{
    return NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrObject* SdrObject::ImpConvertToContourObj(SdrObject* pRet, BOOL bForceLineDash) const
{
    BOOL bNoChange(TRUE);

    if(pRet->LineGeometryUsageIsNecessary())
    {
        ::std::auto_ptr< SdrLineGeometry > aLineGeom( pRet->CreateLinePoly(sal_False, sal_False));
        if( aLineGeom.get() )
        {
            basegfx::B2DPolyPolygon aAreaPolyPolygon = aLineGeom->GetAreaPolyPolygon();
            basegfx::B2DPolyPolygon aLinePolyPolygon = aLineGeom->GetLinePolyPolygon();

            // #107201#
            // Since this may in some cases lead to a count of 0 after
            // the merge i moved the merge to the front.
            if(aAreaPolyPolygon.count())
            {
                aAreaPolyPolygon = basegfx::tools::removeAllIntersections(aAreaPolyPolygon);
                aAreaPolyPolygon = basegfx::tools::removeNeutralPolygons(aAreaPolyPolygon, sal_True);
            }

            //  || aLinePolyPolygon.Count() removed; the conversion is ONLY
            // useful when new closed filled polygons are created
            if(aAreaPolyPolygon.count() || (bForceLineDash && aLinePolyPolygon.count()))
            {
                SfxItemSet aSet(pRet->GetMergedItemSet());
                XFillStyle eOldFillStyle = ((const XFillStyleItem&)(aSet.Get(XATTR_FILLSTYLE))).GetValue();
                SdrPathObj* aLinePolygonPart = NULL;
                SdrPathObj* aLineLinePart = NULL;
                BOOL bBuildGroup(FALSE);

                // #107600#
                sal_Bool bAddOriginalGeometry(sal_False);

                if(aAreaPolyPolygon.count())
                {
                    aLinePolygonPart = new SdrPathObj(OBJ_PATHFILL, aAreaPolyPolygon);
                    aLinePolygonPart->SetModel(pRet->GetModel());

                    aSet.Put(XLineWidthItem(0L));
                    Color aColorLine = ((const XLineColorItem&)(aSet.Get(XATTR_LINECOLOR))).GetColorValue();
                    UINT16 nTransLine = ((const XLineTransparenceItem&)(aSet.Get(XATTR_LINETRANSPARENCE))).GetValue();
                    aSet.Put(XFillColorItem(XubString(), aColorLine));
                    aSet.Put(XFillStyleItem(XFILL_SOLID));
                    aSet.Put(XLineStyleItem(XLINE_NONE));
                    aSet.Put(XFillTransparenceItem(nTransLine));

                    aLinePolygonPart->SetMergedItemSet(aSet);
                }

                if(aLinePolyPolygon.count())
                {
                    // #106907#
                    // OBJ_PATHLINE is necessary here, not OBJ_PATHFILL. This is intended
                    // to get a non-filled object. If the poly is closed, the PathObj takes care for
                    // the correct closed state.
                    aLineLinePart = new SdrPathObj(OBJ_PATHLINE, aLinePolyPolygon);
                    aLineLinePart->SetModel(pRet->GetModel());

                    aSet.Put(XLineWidthItem(0L));
                    aSet.Put(XFillStyleItem(XFILL_NONE));
                    aSet.Put(XLineStyleItem(XLINE_SOLID));

                    // #106907#
                    // it is also necessary to switch off line start and ends here
                    aSet.Put(XLineStartWidthItem(0));
                    aSet.Put(XLineEndWidthItem(0));

                    aLineLinePart->SetMergedItemSet(aSet);

                    if(aLinePolygonPart)
                        bBuildGroup = TRUE;
                }

                // #107600# This test does not depend on !bBuildGroup
                SdrPathObj* pPath = PTR_CAST(SdrPathObj, pRet);
                if(pPath && pPath->IsClosed())
                {
                    if(eOldFillStyle != XFILL_NONE)
                    {
                        // #107600# use new boolean here
                        bAddOriginalGeometry = sal_True;
                    }
                }

                // #107600# ask for new boolean, too.
                if(bBuildGroup || bAddOriginalGeometry)
                {
                    SdrObject* pGroup = new SdrObjGroup;
                    pGroup->SetModel(pRet->GetModel());

                    if(bAddOriginalGeometry)
                    {
                        // #107600# Add a clone of the original geometry.
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

                    if(aLineLinePart)
                    {
                        pGroup->GetSubList()->NbcInsertObject(aLineLinePart);
                    }

                    pRet = pGroup;

                    // #107201#
                    // be more careful with the state describing bool
                    bNoChange = FALSE;
                }
                else
                {
                    if(aLinePolygonPart)
                    {
                        pRet = aLinePolygonPart;
                        // #107201#
                        // be more careful with the state describing bool
                        bNoChange = FALSE;
                    }
                    else if(aLineLinePart)
                    {
                        pRet = aLineLinePart;
                        // #107201#
                        // be more careful with the state describing bool
                        bNoChange = FALSE;
                    }
                }
            }
        }
    }

    if(bNoChange)
    {
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
    bMovProt = bProt;
    SetChanged();

    if(IsInserted() && pModel)
    {
        SdrHint aHint(*this);
        pModel->Broadcast(aHint);
    }
}

void SdrObject::SetResizeProtect(sal_Bool bProt)
{
    bSizProt=bProt;
    SetChanged();
    if (IsInserted() && pModel!=NULL) {
        SdrHint aHint(*this);
        pModel->Broadcast(aHint);
    }
}

void SdrObject::SetPrintable(sal_Bool bPrn)
{
    bNoPrint=!bPrn;
    SetChanged();
    if (IsInserted() && pModel!=NULL) {
        SdrHint aHint(*this);
        pModel->Broadcast(aHint);
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
        DBG_ERROR("SdrObject::InsertUserData(): pData ist NULL-Pointer");
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
        DBG_ERROR("SdrObject::DeleteUserData(): ungueltiger Index");
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

    if( eUserCall == SDRUSERCALL_CHGATTR )
    {
        if( pModel && pModel->IsAllowShapePropertyChangeListener() )
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > xShapeGuard;
            SvxShape* pShape = const_cast< SdrObject* >(this)->getSvxShape( xShapeGuard );
            if( pShape )
                pShape->onUserCall( eUserCall, rBoundRect );
        }
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

void SdrObject::setUnoShape(
    const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxUnoShape,
    SdrObject::GrantXShapeAccess /*aGrant*/
)
{
    mxUnoShape = _rxUnoShape;
    mpSvxShape = 0;
}

/** only for internal use! */
SvxShape* SdrObject::getSvxShape( ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& xShapeGuard )
{
    xShapeGuard = xShapeGuard.query( mxUnoShape );
    if( xShapeGuard.is() )
    {
        if( !mpSvxShape )
        {
            mpSvxShape = SvxShape::getImplementation( xShapeGuard );
        }
    }
    else if( mpSvxShape )
    {
        mpSvxShape = NULL;
    }

    return mpSvxShape;
}

::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SdrObject::getUnoShape()
{
    // try weak reference first
    uno::Reference< uno::XInterface > xShape( mxUnoShape );
    if( !xShape.is() )
    {
        if ( pPage )
        {
            mpSvxShape = 0;
            uno::Reference< uno::XInterface > xPage( pPage->getUnoPage() );
            if( xPage.is() )
            {
                SvxDrawPage* pDrawPage = SvxDrawPage::getImplementation(xPage);
                if( pDrawPage )
                {
                    // create one
                    mxUnoShape = xShape = pDrawPage->_CreateShape( this );
                }
            }
        }
        else
        {
            mpSvxShape = SvxDrawPage::CreateShapeByTypeAndInventor( GetObjIdentifier(), GetObjInventor(), this, NULL );
            mxUnoShape = xShape = static_cast< ::cppu::OWeakObject* >( mpSvxShape );
        }
    }

    return xShape;
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
    if( pModel->IsWriter() )
    {
        if(GetAnchorPos().X() || GetAnchorPos().Y())
        {
            aTranslate -= basegfx::B2DTuple(GetAnchorPos().X(), GetAnchorPos().Y());
        }
    }

    // force MapUnit to 100th mm
    SfxMapUnit eMapUnit = pModel->GetItemPool().GetMetric(0);
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
                DBG_ERROR("TRGetBaseGeometry: Missing unit translation to 100th mm!");
            }
        }
    }

    // build matrix
    rMatrix.identity();

    if(1.0 != aScale.getX() || 1.0 != aScale.getY())
    {
        rMatrix.scale(aScale.getX(), aScale.getY());
    }

    if(0.0 != aTranslate.getX() || 0.0 != aTranslate.getY())
    {
        rMatrix.translate(aTranslate.getX(), aTranslate.getY());
    }

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
    SfxMapUnit eMapUnit = pModel->GetItemPool().GetMetric(0);
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
                DBG_ERROR("TRSetBaseGeometry: Missing unit translation to PoolMetric!");
            }
        }
    }

    // if anchor is used, make position relative to it
    if( pModel->IsWriter() )
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

// #111111#
// Needed again and again i will now add a test for finding out if
// this object is the BackgroundObject of the page.
sal_Bool SdrObject::IsMasterPageBackgroundObject() const
{
    if(pObjList
        && pObjList == pPage
        && pPage->IsMasterPage()
        && pObjList->GetObj(0) == this
        && 1L == (pPage->GetPageNum() % 2))
    {
        // 0'th object, directly on page, page is MasterPage,
        // MasterPagePageNum is 1,3,5,...
        // --> It's the background object (!)
        return sal_True;
    }

    return sal_False;
}

// #116168#
// Give info if object is in destruction
sal_Bool SdrObject::IsInDestruction() const
{
    if(pModel)
        return pModel->IsInDestruction();
    return sal_False;
}

bool SdrObject::ImpAddLineGeomteryForMiteredLines()
{
    //sal_Int32 nLineWidth(0L);
    bool bRetval(false);

    if(XLINE_NONE != ((const XLineStyleItem&)(GetObjectItem(XATTR_LINESTYLE))).GetValue())
    {
        if(0 != ((const XLineWidthItem&)(GetObjectItem(XATTR_LINEWIDTH))).GetValue())
        {
            if(XLINEJOINT_MITER == ((const XLineJointItem&)(GetObjectItem(XATTR_LINEJOINT))).GetValue())
            {
                basegfx::B2DPolyPolygon aAreaPolyPolygon;
                basegfx::B2DPolyPolygon aLinePolyPolygon;

                // get XOR Poly as base
                basegfx::B2DPolyPolygon aTmpPolyPolygon(TakeXorPoly(TRUE));
                ImpLineStyleParameterPack aLineAttr(GetMergedItemSet(), false);
                ImpLineGeometryCreator aLineCreator(aLineAttr, aAreaPolyPolygon, aLinePolyPolygon);

                // compute single lines
                for(sal_uInt32 a(0L); a < aTmpPolyPolygon.count(); a++)
                {
                    // expand splines into polygons and convert to double
                    basegfx::B2DPolygon aCandidate(aTmpPolyPolygon.getB2DPolygon(a));
                    aCandidate.removeDoublePoints();

                    if(aCandidate.areControlPointsUsed())
                    {
                        aCandidate = basegfx::tools::adaptiveSubdivideByAngle(aCandidate);
                    }

                    // convert line to single Polygons; make sure the part
                    // polygons are all clockwise oriented
                    aLineCreator.AddPolygon(aCandidate);
                }

                // get bounds for areas, expand aOutRect
                if(aAreaPolyPolygon.count())
                {
                    basegfx::B2DRange aRange(basegfx::tools::getRange(aAreaPolyPolygon));
                    basegfx::B2DTuple aMinimum(aRange.getMinimum());
                    basegfx::B2DTuple aMaximum(aRange.getMaximum());

                    const Rectangle aBound(
                        FRound(aMinimum.getX()), FRound(aMinimum.getY()),
                        FRound(aMaximum.getX()), FRound(aMaximum.getY()));

                    if(aBound.Left() < aOutRect.Left())
                    {
                        aOutRect.Left() = aBound.Left();
                        bRetval = true;
                    }
                    if(aBound.Right() > aOutRect.Right())
                    {
                        aOutRect.Right() = aBound.Right();
                        bRetval = true;
                    }
                    if(aBound.Top() < aOutRect.Top())
                    {
                        aOutRect.Top() = aBound.Top();
                        bRetval = true;
                    }
                    if(aBound.Bottom() > aOutRect.Bottom())
                    {
                        aOutRect.Bottom() = aBound.Bottom();
                        bRetval = true;
                    }
                }
            }
        }
    }

    return bRetval;
}

// #i34682#
// return if fill is != XFILL_NONE
sal_Bool SdrObject::HasFillStyle() const
{
    return (((const XFillStyleItem&)GetObjectItem(XATTR_FILLSTYLE)).GetValue() != XFILL_NONE);
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

// eof
