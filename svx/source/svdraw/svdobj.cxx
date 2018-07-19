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

#include <svx/svdobj.hxx>
#include <config_features.h>

#include <sal/config.h>

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/text/RelOrientation.hpp>

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
#include <editeng/outlobj.hxx>
#include <o3tl/deleter.hxx>
#include <math.h>
#include <sfx2/objface.hxx>
#include <sfx2/objsh.hxx>
#include <svl/whiter.hxx>
#include <svl/grabbagitem.hxx>
#include <svtools/colorcfg.hxx>
#include <tools/bigint.hxx>
#include <tools/diagnose_ex.h>
#include <tools/helpers.hxx>
#include <tools/line.hxx>
#include <unotools/configmgr.hxx>
#include <vcl/graphictools.hxx>
#include <vcl/metaact.hxx>
#include <vcl/virdev.hxx>
#include <vector>

#include <svx/shapepropertynotifier.hxx>
#include <svx/svdotable.hxx>
#include <svx/xlinjoit.hxx>

#include <svx/fmmodel.hxx>
#include <svx/sdr/contact/displayinfo.hxx>
#include <svx/sdr/contact/objectcontactofobjlistpainter.hxx>
#include <sdr/contact/viewcontactofgraphic.hxx>
#include <svx/sdr/contact/viewcontactofsdrobj.hxx>
#include <sdr/properties/emptyproperties.hxx>
#include <svx/sdrhittesthelper.hxx>
#include <svx/sdrobjectuser.hxx>
#include <svx/sdrobjectfilter.hxx>
#include <svx/svddrag.hxx>
#include <svx/svdetc.hxx>
#include <svx/svdhdl.hxx>
#include <svx/svditer.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdoashp.hxx>
#include <svx/svdocapt.hxx>
#include <svx/svdocirc.hxx>
#include <svx/svdoedge.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svdomeas.hxx>
#include <svx/svdomedia.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdopage.hxx>
#include <svx/svdopath.hxx>
#include <svx/svdorect.hxx>
#include <svx/svdotext.hxx>
#include <svx/svdouno.hxx>
#include <svx/svdovirt.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdpool.hxx>
#include <svx/strings.hrc>
#include <svx/dialmgr.hxx>
#include <svx/svdtrans.hxx>
#include <svx/svdundo.hxx>
#include <svx/svdview.hxx>
#include <sxlayitm.hxx>
#include <sxlogitm.hxx>
#include <sxmovitm.hxx>
#include <sxoneitm.hxx>
#include <sxopitm.hxx>
#include <sxreoitm.hxx>
#include <sxrooitm.hxx>
#include <sxsaitm.hxx>
#include <sxsoitm.hxx>
#include <sxtraitm.hxx>
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
#include <svx/svdglue.hxx>
#include <svx/svdsob.hxx>
#include <rtl/strbuf.hxx>
#include <svdobjplusdata.hxx>
#include <svdobjuserdatalist.hxx>
#include <o3tl/make_unique.hxx>

#include <boost/optional.hpp>
#include <libxml/xmlwriter.h>
#include <memory>

using namespace ::com::sun::star;


SdrObjUserCall::~SdrObjUserCall()
{
}

void SdrObjUserCall::Changed(const SdrObject& /*rObj*/, SdrUserCallType /*eType*/, const tools::Rectangle& /*rOldBoundRect*/)
{
}

SdrObjMacroHitRec::SdrObjMacroHitRec() :
    pOut(nullptr),
    pVisiLayer(nullptr),
    pPageView(nullptr),
    nTol(0) {}


SdrObjUserData::SdrObjUserData(SdrInventor nInv, sal_uInt16 nId) :
    nInventor(nInv),
    nIdentifier(nId) {}

SdrObjUserData::SdrObjUserData(const SdrObjUserData& rData) :
    nInventor(rData.nInventor),
    nIdentifier(rData.nIdentifier) {}

SdrObjUserData::~SdrObjUserData() {}

SdrObjGeoData::SdrObjGeoData():
    pGPL(nullptr),
    bMovProt(false),
    bSizProt(false),
    bNoPrint(false),
    bClosedObj(false),
    mbVisible(true),
    mnLayerID(0)
{
}

SdrObjGeoData::~SdrObjGeoData()
{
}

SdrObjTransformInfoRec::SdrObjTransformInfoRec() :
    bMoveAllowed(true),
    bResizeFreeAllowed(true),
    bResizePropAllowed(true),
    bRotateFreeAllowed(true),
    bRotate90Allowed(true),
    bMirrorFreeAllowed(true),
    bMirror45Allowed(true),
    bMirror90Allowed(true),
    bTransparenceAllowed(true),
    bShearAllowed(true),
    bEdgeRadiusAllowed(true),
    bNoOrthoDesired(true),
    bNoContortion(true),
    bCanConvToPath(true),
    bCanConvToPoly(true),
    bCanConvToContour(false),
    bCanConvToPathLineToArea(true),
    bCanConvToPolyLineToArea(true) {}

struct SdrObject::Impl
{
    sdr::ObjectUserVector maObjectUsers;

    boost::optional<double> mnRelativeWidth;
    sal_Int16               meRelativeWidthRelation;
    boost::optional<double> mnRelativeHeight;
    sal_Int16               meRelativeHeightRelation;

    Impl() :
        meRelativeWidthRelation(text::RelOrientation::PAGE_FRAME),
        meRelativeHeightRelation(text::RelOrientation::PAGE_FRAME) {}
};


// BaseProperties section

std::unique_ptr<sdr::properties::BaseProperties> SdrObject::CreateObjectSpecificProperties()
{
    return o3tl::make_unique<sdr::properties::EmptyProperties>(*this);
}

sdr::properties::BaseProperties& SdrObject::GetProperties() const
{
    if(!mpProperties)
    {
        // CAUTION(!) Do *not* call this during SdrObject construction,
        // that will lead to wrong type-casts (dependent on constructor-level)
        // and thus eventually create the wrong sdr::properties (!). Is there
        // a way to check if on the stack is a SdrObject-constructor (?)
        const_cast< SdrObject* >(this)->mpProperties =
            const_cast< SdrObject* >(this)->CreateObjectSpecificProperties();
    }

    return *mpProperties;
}


// ObjectUser section

void SdrObject::AddObjectUser(sdr::ObjectUser& rNewUser)
{
    mpImpl->maObjectUsers.push_back(&rNewUser);
}

void SdrObject::RemoveObjectUser(sdr::ObjectUser& rOldUser)
{
    const sdr::ObjectUserVector::iterator aFindResult =
        std::find(mpImpl->maObjectUsers.begin(), mpImpl->maObjectUsers.end(), &rOldUser);
    if (aFindResult != mpImpl->maObjectUsers.end())
    {
        mpImpl->maObjectUsers.erase(aFindResult);
    }
}


// DrawContact section

std::unique_ptr<sdr::contact::ViewContact> SdrObject::CreateObjectSpecificViewContact()
{
    return o3tl::make_unique<sdr::contact::ViewContactOfSdrObj>(*this);
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

SdrPage* SdrObject::getSdrPageFromSdrObject() const
{
    if(getParentSdrObjListFromSdrObject())
    {
        return getParentSdrObjListFromSdrObject()->getSdrPageFromSdrObjList();
    }

    return nullptr;
}

SdrModel& SdrObject::getSdrModelFromSdrObject() const
{
    return mrSdrModelFromSdrObject;
}

void SdrObject::setParentOfSdrObject(SdrObjList* pNewObjList)
{
    if(getParentSdrObjListFromSdrObject() != pNewObjList)
    {
        // remember current page
        SdrPage* pOldPage(getSdrPageFromSdrObject());

        // set new parent
        mpParentOfSdrObject = pNewObjList;

        // get new page
        SdrPage* pNewPage(getSdrPageFromSdrObject());

        // broadcast page change over objects if needed
        if(pOldPage != pNewPage)
        {
            handlePageChange(pOldPage, pNewPage);
        }
    }
}

SdrObjList* SdrObject::getParentSdrObjListFromSdrObject() const
{
    return mpParentOfSdrObject;
}

SdrObjList* SdrObject::getChildrenOfSdrObject() const
{
    // default has no children
    return nullptr;
}

void SdrObject::SetBoundRectDirty()
{
    aOutRect = tools::Rectangle();
}


SdrObject::SdrObject(SdrModel& rSdrModel)
:   mpFillGeometryDefiningShape(nullptr)
    ,mrSdrModelFromSdrObject(rSdrModel)
    ,pUserCall(nullptr)
    ,pPlusData(nullptr)
    ,mpImpl(new Impl)
    ,mpParentOfSdrObject(nullptr)
    ,nOrdNum(0)
    ,pGrabBagItem(nullptr)
    ,mnNavigationPosition(SAL_MAX_UINT32)
    ,mnLayerID(0)
    ,mpProperties(nullptr)
    ,mpViewContact(nullptr)
    ,mbDelayBroadcastObjectChange(false)
    ,mbBroadcastObjectChangePending(false)
    ,mpSvxShape( nullptr )
    ,maWeakUnoShape()
    ,mbDoNotInsertIntoPageAutomatically(false)
{
    bVirtObj         =false;
    bSnapRectDirty   =true;
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
    sdr::ObjectUserVector aListCopy(mpImpl->maObjectUsers.begin(), mpImpl->maObjectUsers.end());
    for(sdr::ObjectUserVector::iterator aIterator = aListCopy.begin(); aIterator != aListCopy.end(); ++aIterator)
    {
        sdr::ObjectUser* pObjectUser = *aIterator;
        DBG_ASSERT(pObjectUser, "SdrObject::~SdrObject: corrupt ObjectUser list (!)");
        pObjectUser->ObjectInDestruction(*this);
    }

    // Clear the vector. This means that user do not need to call RemoveObjectUser()
    // when they get called from ObjectInDestruction().
    mpImpl->maObjectUsers.clear();

    // UserCall
    SendUserCall(SdrUserCallType::Delete, GetLastBoundRect());
    o3tl::reset_preserve_ptr_during(pPlusData);

    pGrabBagItem.reset();
    mpProperties.reset();
    mpViewContact.reset();
}

void SdrObject::Free( SdrObject*& _rpObject )
{
    SdrObject* pObject = _rpObject; _rpObject = nullptr;

    if(nullptr == pObject)
    {
        // nothing to do
        return;
    }

    SvxShape* pShape(pObject->getSvxShape());

    if(pShape)
    {
        if(pShape->HasSdrObjectOwnership())
        {
            // only the SvxShape is allowed to delete me, and will reset
            // the ownership before doing so
            return;
        }
        else
        {
            // not only delete pObject, but also need to dispose uno shape
            try
            {
                pShape->InvalidateSdrObject();
                uno::Reference< lang::XComponent > xShapeComp( pObject->getWeakUnoShape(), uno::UNO_QUERY_THROW );
                xShapeComp->dispose();
            }
            catch( const uno::Exception& )
            {
                DBG_UNHANDLED_EXCEPTION("svx");
            }
        }
    }

    delete pObject;
}

void SdrObject::SetRectsDirty(bool bNotMyself, bool bRecursive)
{
    if (!bNotMyself)
    {
        SetBoundRectDirty();
        bSnapRectDirty=true;
    }

    if (bRecursive && nullptr != getParentSdrObjListFromSdrObject())
    {
        getParentSdrObjListFromSdrObject()->SetSdrObjListRectsDirty();
    }
}

void SdrObject::handlePageChange(SdrPage* pOldPage, SdrPage* pNewPage)
{
    // The creation of the UNO shape in SdrObject::getUnoShape is influenced
    // by pPage, so when the page changes we need to discard the cached UNO
    // shape so that a new one will be created.
    // If the page is changing to another page with the same model, we
    // assume they create compatible UNO shape objects so we shouldn't have
    // to invalidate.
    // TTTT: This causes quite some problems in SvxDrawPage::add when used
    // e.g. from Writer - the SdrObject may be cloned to target model, and
    // the xShape was added to it by purpose (see there). Thus it will be
    // good to think about if this is really needed - it *seems* to be intended
    // for a xShape being a on-demand-creatable resource - with the argument that
    // the SdrPage/UnoPage used influences the SvxShape creation. This uses
    // resources and would be nice to get rid of anyways.
    if(nullptr == pOldPage || nullptr == pNewPage)
    {
        SvxShape* const pShape(getSvxShape());

        if (pShape && !pShape->HasSdrObjectOwnership())
        {
            setUnoShape(nullptr);
        }
    }
}

// init global static itempool
SdrItemPool* SdrObject::mpGlobalItemPool = nullptr;

SdrItemPool& SdrObject::GetGlobalDrawObjectItemPool()
{
    if(!mpGlobalItemPool)
    {
        mpGlobalItemPool = new SdrItemPool();
        SfxItemPool* pGlobalOutlPool = EditEngine::CreatePool();
        mpGlobalItemPool->SetSecondaryPool(pGlobalOutlPool);
        mpGlobalItemPool->SetDefaultMetric(SdrEngineDefaults::GetMapUnit());
        mpGlobalItemPool->FreezeIdRanges();
    }

    return *mpGlobalItemPool;
}

void SdrObject::SetRelativeWidth( double nValue )
{
    mpImpl->mnRelativeWidth.reset( nValue );
}

void SdrObject::SetRelativeWidthRelation( sal_Int16 eValue )
{
    mpImpl->meRelativeWidthRelation = eValue;
}

void SdrObject::SetRelativeHeight( double nValue )
{
    mpImpl->mnRelativeHeight.reset( nValue );
}

void SdrObject::SetRelativeHeightRelation( sal_Int16 eValue )
{
    mpImpl->meRelativeHeightRelation = eValue;
}

const double* SdrObject::GetRelativeWidth( ) const
{
    if (!mpImpl->mnRelativeWidth)
        return nullptr;

    return &mpImpl->mnRelativeWidth.get();
}

sal_Int16 SdrObject::GetRelativeWidthRelation() const
{
    return mpImpl->meRelativeWidthRelation;
}

const double* SdrObject::GetRelativeHeight( ) const
{
    if (!mpImpl->mnRelativeHeight)
        return nullptr;

    return &mpImpl->mnRelativeHeight.get();
}

sal_Int16 SdrObject::GetRelativeHeightRelation() const
{
    return mpImpl->meRelativeHeightRelation;
}

SfxItemPool& SdrObject::GetObjectItemPool() const
{
    return getSdrModelFromSdrObject().GetItemPool();
}

SdrInventor SdrObject::GetObjInventor()   const
{
    return SdrInventor::Default;
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

void SdrObject::getMergedHierarchySdrLayerIDSet(SdrLayerIDSet& rSet) const
{
    rSet.Set(GetLayer());
    SdrObjList* pOL=GetSubList();
    if (pOL!=nullptr) {
        const size_t nObjCount = pOL->GetObjCount();
        for (size_t nObjNum = 0; nObjNum<nObjCount; ++nObjNum) {
            pOL->GetObj(nObjNum)->getMergedHierarchySdrLayerIDSet(rSet);
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
    if (pPlusData->pBroadcast==nullptr) pPlusData->pBroadcast.reset(new SfxBroadcaster);

    // SdrEdgeObj may be connected to same SdrObject on both ends so allow it
    // to listen twice
    SdrEdgeObj const*const pEdge(dynamic_cast<SdrEdgeObj const*>(&rListener));
    rListener.StartListening(*pPlusData->pBroadcast, pEdge ? DuplicateHandling::Allow : DuplicateHandling::Unexpected);
}

void SdrObject::RemoveListener(SfxListener& rListener)
{
    if (pPlusData!=nullptr && pPlusData->pBroadcast!=nullptr) {
        rListener.EndListening(*pPlusData->pBroadcast);
        if (!pPlusData->pBroadcast->HasListeners()) {
            pPlusData->pBroadcast.reset();
        }
    }
}

const SfxBroadcaster* SdrObject::GetBroadcaster() const
{
    return pPlusData!=nullptr ? pPlusData->pBroadcast.get() : nullptr;
}

void SdrObject::AddReference(SdrVirtObj& rVrtObj)
{
    AddListener(rVrtObj);
}

void SdrObject::DelReference(SdrVirtObj& rVrtObj)
{
    RemoveListener(rVrtObj);
}

bool SdrObject::IsGroupObject() const
{
    return GetSubList()!=nullptr;
}

SdrObjList* SdrObject::GetSubList() const
{
    return nullptr;
}

SdrObject* SdrObject::getParentSdrObjectFromSdrObject() const
{
    SdrObjList* pParent(getParentSdrObjListFromSdrObject());

    if(nullptr == pParent)
    {
        return nullptr;
    }

    return pParent->getSdrObjectFromSdrObjList();
}

void SdrObject::SetName(const OUString& rStr)
{
    if (!rStr.isEmpty() && !pPlusData)
    {
        ImpForcePlusData();
    }

    if(pPlusData && pPlusData->aObjName != rStr)
    {
        // Undo/Redo for setting object's name (#i73249#)
        bool bUndo( false );
        if ( getSdrModelFromSdrObject().IsUndoEnabled() )
        {
            bUndo = true;
            SdrUndoAction* pUndoAction =
                    SdrUndoFactory::CreateUndoObjectStrAttr(
                                                    *this,
                                                    SdrUndoObjStrAttr::ObjStrAttrType::Name,
                                                    GetName(),
                                                    rStr );
            getSdrModelFromSdrObject().BegUndo( pUndoAction->GetComment() );
            getSdrModelFromSdrObject().AddUndo( pUndoAction );
        }
        pPlusData->aObjName = rStr;
        // Undo/Redo for setting object's name (#i73249#)
        if ( bUndo )
        {
            getSdrModelFromSdrObject().EndUndo();
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
        if ( getSdrModelFromSdrObject().IsUndoEnabled() )
        {
            bUndo = true;
            SdrUndoAction* pUndoAction =
                    SdrUndoFactory::CreateUndoObjectStrAttr(
                                                    *this,
                                                    SdrUndoObjStrAttr::ObjStrAttrType::Title,
                                                    GetTitle(),
                                                    rStr );
            getSdrModelFromSdrObject().BegUndo( pUndoAction->GetComment() );
            getSdrModelFromSdrObject().AddUndo( pUndoAction );
        }
        pPlusData->aObjTitle = rStr;
        // Undo/Redo for setting object's title (#i73249#)
        if ( bUndo )
        {
            getSdrModelFromSdrObject().EndUndo();
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

    if(pPlusData && pPlusData->aObjDescription != rStr)
    {
        // Undo/Redo for setting object's description (#i73249#)
        bool bUndo( false );
        if ( getSdrModelFromSdrObject().IsUndoEnabled() )
        {
            bUndo = true;
            SdrUndoAction* pUndoAction =
                    SdrUndoFactory::CreateUndoObjectStrAttr(
                                                    *this,
                                                    SdrUndoObjStrAttr::ObjStrAttrType::Description,
                                                    GetDescription(),
                                                    rStr );
            getSdrModelFromSdrObject().BegUndo( pUndoAction->GetComment() );
            getSdrModelFromSdrObject().AddUndo( pUndoAction );
        }
        pPlusData->aObjDescription = rStr;
        // Undo/Redo for setting object's description (#i73249#)
        if ( bUndo )
        {
            getSdrModelFromSdrObject().EndUndo();
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
    if (nullptr != getParentSdrObjListFromSdrObject())
    {
        if (getParentSdrObjListFromSdrObject()->IsObjOrdNumsDirty())
        {
            getParentSdrObjListFromSdrObject()->RecalcObjOrdNums();
        }
    } else const_cast<SdrObject*>(this)->nOrdNum=0;
    return nOrdNum;
}


void SdrObject::SetOrdNum(sal_uInt32 nNum)
{
    nOrdNum = nNum;
}

void SdrObject::GetGrabBagItem(css::uno::Any& rVal) const
{
    if (pGrabBagItem != nullptr)
        pGrabBagItem->QueryValue(rVal);
    else
        rVal <<= uno::Sequence<beans::PropertyValue>();
}

void SdrObject::SetGrabBagItem(const css::uno::Any& rVal)
{
    if (pGrabBagItem == nullptr)
        pGrabBagItem.reset(new SfxGrabBagItem);

    pGrabBagItem->PutValue(rVal, 0);

    SetChanged();
    BroadcastObjectChange();
}

sal_uInt32 SdrObject::GetNavigationPosition()
{
    if (nullptr != getParentSdrObjListFromSdrObject() && getParentSdrObjListFromSdrObject()->RecalcNavigationPositions())
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
const tools::Rectangle& SdrObject::GetCurrentBoundRect() const
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
const tools::Rectangle& SdrObject::GetLastBoundRect() const
{
    return aOutRect;
}

void SdrObject::RecalcBoundRect()
{
    // #i101680# suppress BoundRect calculations on import(s)
    if ((getSdrModelFromSdrObject().isLocked()) || utl::ConfigManager::IsFuzzing())
        return;

    // central new method which will calculate the BoundRect using primitive geometry
    if(aOutRect.IsEmpty())
    {
        const drawinglayer::primitive2d::Primitive2DContainer xPrimitives(GetViewContact().getViewIndependentPrimitive2DContainer());

        if(!xPrimitives.empty())
        {
            // use neutral ViewInformation and get the range of the primitives
            const drawinglayer::geometry::ViewInformation2D aViewInformation2D;
            const basegfx::B2DRange aRange(xPrimitives.getB2DRange(aViewInformation2D));

            if(!aRange.isEmpty())
            {
                aOutRect = tools::Rectangle(
                    static_cast<long>(floor(aRange.getMinX())),
                    static_cast<long>(floor(aRange.getMinY())),
                    static_cast<long>(ceil(aRange.getMaxX())),
                    static_cast<long>(ceil(aRange.getMaxY())));
                aOutRect -= GetGridOffset(); // don't include grid offset
                return;
            }
        }
    }
}

void SdrObject::BroadcastObjectChange() const
{
    if ((getSdrModelFromSdrObject().isLocked()) || utl::ConfigManager::IsFuzzing())
        return;

    if (mbDelayBroadcastObjectChange)
    {
        mbBroadcastObjectChangePending = true;
        return;
    }

    bool bPlusDataBroadcast(pPlusData && pPlusData->pBroadcast);
    bool bObjectChange(IsInserted());

    if(bPlusDataBroadcast || bObjectChange)
    {
        SdrHint aHint(SdrHintKind::ObjectChange, *this);

        if(bPlusDataBroadcast)
        {
            pPlusData->pBroadcast->Broadcast(aHint);
        }

        if(bObjectChange)
        {
            getSdrModelFromSdrObject().Broadcast(aHint);
        }
    }
}

void SdrObject::SetChanged()
{
    // For testing purposes, use the new ViewContact for change
    // notification now.
    ActionChanged();

    // TTTT Need to check meaning/usage of IsInserted in one
    // of the next changes. It should not mean to have a SdrModel
    // set (this is guaranteed now), but should be connected to
    // being added to a SdrPage (?)
    if(IsInserted())
    {
        getSdrModelFromSdrObject().SetChanged();
    }
}

// tooling for painting a single object to an OutputDevice.
void SdrObject::SingleObjectPainter(OutputDevice& rOut) const
{
    sdr::contact::SdrObjectVector aObjectVector;
    aObjectVector.push_back(const_cast< SdrObject* >(this));

    sdr::contact::ObjectContactOfObjListPainter aPainter(rOut, aObjectVector, getSdrPageFromSdrObject());
    sdr::contact::DisplayInfo aDisplayInfo;

    aPainter.ProcessDisplay(aDisplayInfo);
}

bool SdrObject::LineGeometryUsageIsNecessary() const
{
    drawing::LineStyle eXLS = GetMergedItem(XATTR_LINESTYLE).GetValue();
    return (eXLS != drawing::LineStyle_NONE);
}

bool SdrObject::HasLimitedRotation() const
{
    // RotGrfFlyFrame: Default is false, support full rotation
    return false;
}

SdrObject* SdrObject::CloneSdrObject(SdrModel& rTargetModel) const
{
    return CloneHelper< SdrObject >(rTargetModel);
}

SdrObject& SdrObject::operator=(const SdrObject& rObj)
{
    if( this == &rObj )
        return *this;

    mpProperties.reset();
    mpViewContact.reset();

    // The CloneSdrObject() method uses the local copy constructor from the individual
    // sdr::properties::BaseProperties class. Since the target class maybe for another
    // draw object, an SdrObject needs to be provided, as in the normal constructor.
    mpProperties = rObj.GetProperties().Clone(*this);

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
    pPlusData.reset();
    if (rObj.pPlusData!=nullptr) {
        pPlusData.reset(rObj.pPlusData->Clone(this));
    }
    if (pPlusData!=nullptr && pPlusData->pBroadcast!=nullptr) {
        pPlusData->pBroadcast.reset(); // broadcaster isn't copied
    }

    pGrabBagItem.reset();
    if (rObj.pGrabBagItem!=nullptr)
        pGrabBagItem.reset(static_cast< SfxGrabBagItem* >( rObj.pGrabBagItem->Clone() ));

    aGridOffset = rObj.aGridOffset;
    return *this;
}

OUString SdrObject::TakeObjNameSingul() const
{
    OUStringBuffer sName(SvxResId(STR_ObjNameSingulNONE));

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

OUString SdrObject::TakeObjNamePlural() const
{
    return SvxResId(STR_ObjNamePluralNONE);
}

void SdrObject::ImpTakeDescriptionStr(const char* pStrCacheID, OUString& rStr) const
{
    rStr = SvxResId(pStrCacheID);
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
        rStr = rStr.replaceAt(nPos, 2, "0");
}

void SdrObject::ImpForcePlusData()
{
    if (!pPlusData)
        pPlusData.reset( new SdrObjPlusData );
}

OUString SdrObject::GetMetrStr(long nVal) const
{
    return getSdrModelFromSdrObject().GetMetricString(nVal);
}

basegfx::B2DPolyPolygon SdrObject::TakeXorPoly() const
{
    basegfx::B2DPolyPolygon aRetval;
    const tools::Rectangle aR(GetCurrentBoundRect());
    const basegfx::B2DRange aRange(aR.Left(), aR.Top(), aR.Right(), aR.Bottom());
    aRetval.append(basegfx::utils::createPolygonFromRect(aRange));

    return aRetval;
}

basegfx::B2DPolyPolygon SdrObject::TakeContour() const
{
    basegfx::B2DPolyPolygon aRetval;

    // create cloned object without text, but with drawing::LineStyle_SOLID,
    // COL_BLACK as line color and drawing::FillStyle_NONE
    SdrObject* pClone(CloneSdrObject(getSdrModelFromSdrObject()));

    if(pClone)
    {
        const SdrTextObj* pTextObj = dynamic_cast< const SdrTextObj* >(this);

        if(pTextObj)
        {
            // no text and no text animation
            pClone->SetMergedItem(SdrTextAniKindItem(SdrTextAniKind::NONE));
            pClone->SetOutlinerParaObject(nullptr);
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

        SfxItemSet aNewSet(GetObjectItemPool());

        // #i101980# ignore LineWidth; that's what the old implementation
        // did. With line width, the result may be huge due to fat/thick
        // line decompositions
        aNewSet.Put(XLineWidthItem(0));

        // solid black lines and no fill
        aNewSet.Put(XLineStyleItem(drawing::LineStyle_SOLID));
        aNewSet.Put(XLineColorItem(OUString(), COL_BLACK));
        aNewSet.Put(XFillStyleItem(drawing::FillStyle_NONE));
        pClone->SetMergedItemSet(aNewSet);

        // get sequence from clone
        const sdr::contact::ViewContact& rVC(pClone->GetViewContact());
        const drawinglayer::primitive2d::Primitive2DContainer xSequence(rVC.getViewIndependentPrimitive2DContainer());

        if(!xSequence.empty())
        {
            // use neutral ViewInformation
            const drawinglayer::geometry::ViewInformation2D aViewInformation2D;

            // create extractor, process and get result (with hairlines as opened polygons)
            drawinglayer::processor2d::ContourExtractor2D aExtractor(aViewInformation2D, false);
            aExtractor.process(xSequence);
            const basegfx::B2DPolyPolygonVector& rResult(aExtractor.getExtractedContour());
            const sal_uInt32 nSize(rResult.size());

            // when count is one, it is implied that the object has only its normal
            // contour anyways and TakeContour() is to return an empty PolyPolygon
            // (see old implementation for historical reasons)
            if(nSize > 1)
            {
                // the topology for contour is correctly a vector of PolyPolygons; for
                // historical reasons cut it back to a single tools::PolyPolygon here
                for(sal_uInt32 a(0); a < nSize; a++)
                {
                    aRetval.append(rResult[a]);
                }
            }
        }

        // Always use SdrObject::Free to delete SdrObjects (!)
        SdrObject::Free(pClone);
    }

    return aRetval;
}

sal_uInt32 SdrObject::GetHdlCount() const
{
    return 8L;
}

SdrHdl* SdrObject::GetHdl(sal_uInt32 nHdlNum) const
{
    SdrHdl* pH=nullptr;
    const tools::Rectangle& rR=GetSnapRect();
    switch (nHdlNum) {
        case 0: pH=new SdrHdl(rR.TopLeft(),     SdrHdlKind::UpperLeft); break;
        case 1: pH=new SdrHdl(rR.TopCenter(),   SdrHdlKind::Upper); break;
        case 2: pH=new SdrHdl(rR.TopRight(),    SdrHdlKind::UpperRight); break;
        case 3: pH=new SdrHdl(rR.LeftCenter(),  SdrHdlKind::Left ); break;
        case 4: pH=new SdrHdl(rR.RightCenter(), SdrHdlKind::Right); break;
        case 5: pH=new SdrHdl(rR.BottomLeft(),  SdrHdlKind::LowerLeft); break;
        case 6: pH=new SdrHdl(rR.BottomCenter(),SdrHdlKind::Lower); break;
        case 7: pH=new SdrHdl(rR.BottomRight(), SdrHdlKind::LowerRight); break;
    }
    return pH;
}

sal_uInt32 SdrObject::GetPlusHdlCount(const SdrHdl& /*rHdl*/) const
{
    return 0L;
}

SdrHdl* SdrObject::GetPlusHdl(const SdrHdl& /*rHdl*/, sal_uInt32 /*nPlNum*/) const
{
    return nullptr;
}

void SdrObject::AddToHdlList(SdrHdlList& rHdlList) const
{
    sal_uInt32 nCount=GetHdlCount();
    for (sal_uInt32 i=0; i<nCount; i++) {
        SdrHdl* pHdl=GetHdl(i);
        if (pHdl!=nullptr) {
            rHdlList.AddHdl(pHdl);
        }
    }
}

void SdrObject::addCropHandles(SdrHdlList& /*rTarget*/) const
{
    // Default implementation, does nothing. Overloaded in
    // SdrGrafObj and SwVirtFlyDrawObj
}

tools::Rectangle SdrObject::ImpDragCalcRect(const SdrDragStat& rDrag) const
{
    tools::Rectangle aTmpRect(GetSnapRect());
    tools::Rectangle aRect(aTmpRect);
    const SdrHdl* pHdl=rDrag.GetHdl();
    SdrHdlKind eHdl=pHdl==nullptr ? SdrHdlKind::Move : pHdl->GetKind();
    bool bEcke=(eHdl==SdrHdlKind::UpperLeft || eHdl==SdrHdlKind::UpperRight || eHdl==SdrHdlKind::LowerLeft || eHdl==SdrHdlKind::LowerRight);
    bool bOrtho=rDrag.GetView()!=nullptr && rDrag.GetView()->IsOrtho();
    bool bBigOrtho=bEcke && bOrtho && rDrag.GetView()->IsBigOrtho();
    Point aPos(rDrag.GetNow());
    bool bLft=(eHdl==SdrHdlKind::UpperLeft || eHdl==SdrHdlKind::Left  || eHdl==SdrHdlKind::LowerLeft);
    bool bRgt=(eHdl==SdrHdlKind::UpperRight || eHdl==SdrHdlKind::Right || eHdl==SdrHdlKind::LowerRight);
    bool bTop=(eHdl==SdrHdlKind::UpperRight || eHdl==SdrHdlKind::Upper || eHdl==SdrHdlKind::UpperLeft);
    bool bBtm=(eHdl==SdrHdlKind::LowerRight || eHdl==SdrHdlKind::Lower || eHdl==SdrHdlKind::LowerLeft);
    if (bLft) aTmpRect.SetLeft(aPos.X() );
    if (bRgt) aTmpRect.SetRight(aPos.X() );
    if (bTop) aTmpRect.SetTop(aPos.Y() );
    if (bBtm) aTmpRect.SetBottom(aPos.Y() );
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
                if (bTop) aTmpRect.SetTop(aTmpRect.Bottom()-nNeed );
                if (bBtm) aTmpRect.SetBottom(aTmpRect.Top()+nNeed );
            } else {
                long nNeed=long(BigInt(nWdt0)*BigInt(nYMul)/BigInt(nYDiv));
                if (bXNeg) nNeed=-nNeed;
                if (bLft) aTmpRect.SetLeft(aTmpRect.Right()-nNeed );
                if (bRgt) aTmpRect.SetRight(aTmpRect.Left()+nNeed );
            }
        } else { // apex handles
            if ((bLft || bRgt) && nXDiv!=0) {
                long nHgt0b=aRect.Bottom()-aRect.Top();
                long nNeed=long(BigInt(nHgt0b)*BigInt(nXMul)/BigInt(nXDiv));
                aTmpRect.AdjustTop( -((nNeed-nHgt0b)/2) );
                aTmpRect.SetBottom(aTmpRect.Top()+nNeed );
            }
            if ((bTop || bBtm) && nYDiv!=0) {
                long nWdt0b=aRect.Right()-aRect.Left();
                long nNeed=long(BigInt(nWdt0b)*BigInt(nYMul)/BigInt(nYDiv));
                aTmpRect.AdjustLeft( -((nNeed-nWdt0b)/2) );
                aTmpRect.SetRight(aTmpRect.Left()+nNeed );
            }
        }
    }
    aTmpRect.Justify();
    return aTmpRect;
}


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
    return CloneSdrObject(getSdrModelFromSdrObject());
}

bool SdrObject::beginSpecialDrag(SdrDragStat& rDrag) const
{
    const SdrHdl* pHdl = rDrag.GetHdl();

    SdrHdlKind eHdl = (pHdl == nullptr) ? SdrHdlKind::Move : pHdl->GetKind();

    return eHdl==SdrHdlKind::UpperLeft || eHdl==SdrHdlKind::Upper || eHdl==SdrHdlKind::UpperRight ||
        eHdl==SdrHdlKind::Left || eHdl==SdrHdlKind::Right || eHdl==SdrHdlKind::LowerLeft ||
        eHdl==SdrHdlKind::Lower || eHdl==SdrHdlKind::LowerRight;
}

bool SdrObject::applySpecialDrag(SdrDragStat& rDrag)
{
    tools::Rectangle aNewRect(ImpDragCalcRect(rDrag));

    if(aNewRect != GetSnapRect())
    {
           NbcSetSnapRect(aNewRect);
    }

    return true;
}

OUString SdrObject::getSpecialDragComment(const SdrDragStat& /*rDrag*/) const
{
    return OUString();
}

basegfx::B2DPolyPolygon SdrObject::getSpecialDragPoly(const SdrDragStat& /*rDrag*/) const
{
    // default has nothing to add
    return basegfx::B2DPolyPolygon();
}


// Create
bool SdrObject::BegCreate(SdrDragStat& rStat)
{
    rStat.SetOrtho4Possible();
    tools::Rectangle aRect1(rStat.GetStart(), rStat.GetNow());
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

    return (eCmd==SdrCreateCmd::ForceEnd || rStat.GetPointCount()>=2);
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
    tools::Rectangle aRect1;
    rDrag.TakeCreateRect(aRect1);
    aRect1.Justify();

    basegfx::B2DPolyPolygon aRetval;
    const basegfx::B2DRange aRange(aRect1.Left(), aRect1.Top(), aRect1.Right(), aRect1.Bottom());
    aRetval.append(basegfx::utils::createPolygonFromRect(aRange));
    return aRetval;
}

Pointer SdrObject::GetCreatePointer() const
{
    return Pointer(PointerStyle::Cross);
}

// transformations
void SdrObject::NbcMove(const Size& rSiz)
{
    aOutRect.Move(rSiz);
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
            aRef2.AdjustY( 1 );
            NbcMirrorGluePoints(aRef1,aRef2);
        }
        if (bYMirr) {
            Point aRef2(aRef1);
            aRef2.AdjustX( 1 );
            NbcMirrorGluePoints(aRef1,aRef2);
        }
    }
    ResizeRect(aOutRect,rRef,xFact,yFact);
    SetRectsDirty();
}

void SdrObject::NbcRotate(const Point& rRef, long nAngle, double sn, double cs)
{
    SetGlueReallyAbsolute(true);
    aOutRect.Move(-rRef.X(),-rRef.Y());
    tools::Rectangle R(aOutRect);
    if (sn==1.0 && cs==0.0) { // 90deg
        aOutRect.SetLeft(-R.Bottom() );
        aOutRect.SetRight(-R.Top() );
        aOutRect.SetTop(R.Left() );
        aOutRect.SetBottom(R.Right() );
    } else if (sn==0.0 && cs==-1.0) { // 180deg
        aOutRect.SetLeft(-R.Right() );
        aOutRect.SetRight(-R.Left() );
        aOutRect.SetTop(-R.Bottom() );
        aOutRect.SetBottom(-R.Top() );
    } else if (sn==-1.0 && cs==0.0) { // 270deg
        aOutRect.SetLeft(R.Top() );
        aOutRect.SetRight(R.Bottom() );
        aOutRect.SetTop(-R.Right() );
        aOutRect.SetBottom(-R.Left() );
    }
    aOutRect.Move(rRef.X(),rRef.Y());
    aOutRect.Justify(); // just in case
    SetRectsDirty();
    NbcRotateGluePoints(rRef,nAngle,sn,cs);
    SetGlueReallyAbsolute(false);
}

void SdrObject::NbcMirror(const Point& rRef1, const Point& rRef2)
{
    SetGlueReallyAbsolute(true);
    aOutRect.Move(-rRef1.X(),-rRef1.Y());
    tools::Rectangle R(aOutRect);
    long dx=rRef2.X()-rRef1.X();
    long dy=rRef2.Y()-rRef1.Y();
    if (dx==0) {          // vertical axis
        aOutRect.SetLeft(-R.Right() );
        aOutRect.SetRight(-R.Left() );
    } else if (dy==0) {   // horizontal axis
        aOutRect.SetTop(-R.Bottom() );
        aOutRect.SetBottom(-R.Top() );
    } else if (dx==dy) {  // 45deg axis
        aOutRect.SetLeft(R.Top() );
        aOutRect.SetRight(R.Bottom() );
        aOutRect.SetTop(R.Left() );
        aOutRect.SetBottom(R.Right() );
    } else if (dx==-dy) { // 45deg axis
        aOutRect.SetLeft(-R.Bottom() );
        aOutRect.SetRight(-R.Top() );
        aOutRect.SetTop(-R.Right() );
        aOutRect.SetBottom(-R.Left() );
    }
    aOutRect.Move(rRef1.X(),rRef1.Y());
    aOutRect.Justify(); // just in case
    SetRectsDirty();
    NbcMirrorGluePoints(rRef1,rRef2);
    SetGlueReallyAbsolute(false);
}

void SdrObject::NbcShear(const Point& rRef, long /*nAngle*/, double tn, bool bVShear)
{
    SetGlueReallyAbsolute(true);
    NbcShearGluePoints(rRef,tn,bVShear);
    SetGlueReallyAbsolute(false);
}

void SdrObject::Move(const Size& rSiz)
{
    if (rSiz.Width()!=0 || rSiz.Height()!=0) {
        tools::Rectangle aBoundRect0; if (pUserCall!=nullptr) aBoundRect0=GetLastBoundRect();
        NbcMove(rSiz);
        SetChanged();
        BroadcastObjectChange();
        SendUserCall(SdrUserCallType::MoveOnly,aBoundRect0);
    }
}

void SdrObject::NbcCrop(const basegfx::B2DPoint& /*aRef*/, double /*fxFact*/, double /*fyFact*/)
{
    // Default: does nothing. Real behaviour in SwVirtFlyDrawObj and SdrDragCrop::EndSdrDrag.
    // Where SwVirtFlyDrawObj is the only real user of it to do something local
}

void SdrObject::Resize(const Point& rRef, const Fraction& xFact, const Fraction& yFact, bool bUnsetRelative)
{
    if (xFact.GetNumerator()!=xFact.GetDenominator() || yFact.GetNumerator()!=yFact.GetDenominator()) {
        if (bUnsetRelative)
        {
            mpImpl->mnRelativeWidth.reset();
            mpImpl->meRelativeWidthRelation = text::RelOrientation::PAGE_FRAME;
            mpImpl->meRelativeHeightRelation = text::RelOrientation::PAGE_FRAME;
            mpImpl->mnRelativeHeight.reset();
        }
        tools::Rectangle aBoundRect0; if (pUserCall!=nullptr) aBoundRect0=GetLastBoundRect();
        NbcResize(rRef,xFact,yFact);
        SetChanged();
        BroadcastObjectChange();
        SendUserCall(SdrUserCallType::Resize,aBoundRect0);
    }
}

void SdrObject::Crop(const basegfx::B2DPoint& rRef, double fxFact, double fyFact)
{
    tools::Rectangle aBoundRect0; if (pUserCall!=nullptr) aBoundRect0=GetLastBoundRect();
    NbcCrop(rRef, fxFact, fyFact);
    SetChanged();
    BroadcastObjectChange();
    SendUserCall(SdrUserCallType::Resize,aBoundRect0);
}

void SdrObject::Rotate(const Point& rRef, long nAngle, double sn, double cs)
{
    if (nAngle!=0) {
        tools::Rectangle aBoundRect0; if (pUserCall!=nullptr) aBoundRect0=GetLastBoundRect();
        NbcRotate(rRef,nAngle,sn,cs);
        SetChanged();
        BroadcastObjectChange();
        SendUserCall(SdrUserCallType::Resize,aBoundRect0);
    }
}

void SdrObject::Mirror(const Point& rRef1, const Point& rRef2)
{
    tools::Rectangle aBoundRect0; if (pUserCall!=nullptr) aBoundRect0=GetLastBoundRect();
    NbcMirror(rRef1,rRef2);
    SetChanged();
    BroadcastObjectChange();
    SendUserCall(SdrUserCallType::Resize,aBoundRect0);
}

void SdrObject::Shear(const Point& rRef, long nAngle, double tn, bool bVShear)
{
    if (nAngle!=0) {
        tools::Rectangle aBoundRect0; if (pUserCall!=nullptr) aBoundRect0=GetLastBoundRect();
        NbcShear(rRef,nAngle,tn,bVShear);
        SetChanged();
        BroadcastObjectChange();
        SendUserCall(SdrUserCallType::Resize,aBoundRect0);
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
        tools::Rectangle aBoundRect0; if (pUserCall!=nullptr) aBoundRect0=GetLastBoundRect();
        NbcSetRelativePos(rPnt);
        SetChanged();
        BroadcastObjectChange();
        SendUserCall(SdrUserCallType::MoveOnly,aBoundRect0);
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
        tools::Rectangle aBoundRect0; if (pUserCall!=nullptr) aBoundRect0=GetLastBoundRect();
        NbcSetAnchorPos(rPnt);
        SetChanged();
        BroadcastObjectChange();
        SendUserCall(SdrUserCallType::MoveOnly,aBoundRect0);
    }
}

const Point& SdrObject::GetAnchorPos() const
{
    return aAnchor;
}

void SdrObject::RecalcSnapRect()
{
}

const tools::Rectangle& SdrObject::GetSnapRect() const
{
    return aOutRect;
}

void SdrObject::NbcSetSnapRect(const tools::Rectangle& rRect)
{
    aOutRect=rRect;
}

const tools::Rectangle& SdrObject::GetLogicRect() const
{
    return GetSnapRect();
}

void SdrObject::NbcSetLogicRect(const tools::Rectangle& rRect)
{
    NbcSetSnapRect(rRect);
}

void SdrObject::AdjustToMaxRect( const tools::Rectangle& rMaxRect, bool /* bShrinkOnly = false */ )
{
    SetLogicRect( rMaxRect );
}

void SdrObject::SetSnapRect(const tools::Rectangle& rRect)
{
    tools::Rectangle aBoundRect0; if (pUserCall!=nullptr) aBoundRect0=GetLastBoundRect();
    NbcSetSnapRect(rRect);
    SetChanged();
    BroadcastObjectChange();
    SendUserCall(SdrUserCallType::Resize,aBoundRect0);
}

void SdrObject::SetLogicRect(const tools::Rectangle& rRect)
{
    tools::Rectangle aBoundRect0; if (pUserCall!=nullptr) aBoundRect0=GetLastBoundRect();
    NbcSetLogicRect(rRect);
    SetChanged();
    BroadcastObjectChange();
    SendUserCall(SdrUserCallType::Resize,aBoundRect0);
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

bool SdrObject::IsPolyObj() const
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
    tools::Rectangle aBoundRect0; if (pUserCall!=nullptr) aBoundRect0=GetLastBoundRect();
    NbcSetPoint(rPnt, i);
    SetChanged();
    BroadcastObjectChange();
    SendUserCall(SdrUserCallType::Resize,aBoundRect0);
}

void SdrObject::NbcSetPoint(const Point& /*rPnt*/, sal_uInt32 /*i*/)
{
}

bool SdrObject::HasTextEdit() const
{
    return false;
}

bool SdrObject::Equals(const SdrObject& rOtherObj) const
{
    return (aAnchor.X() == rOtherObj.aAnchor.X() && aAnchor.Y() == rOtherObj.aAnchor.Y() &&
            aGridOffset.X() == rOtherObj.aGridOffset.X() && aGridOffset.Y() == rOtherObj.aGridOffset.Y() &&
            nOrdNum == rOtherObj.nOrdNum && mnNavigationPosition == rOtherObj.mnNavigationPosition &&
            mbSupportTextIndentingOnLineWidthChange == rOtherObj.mbSupportTextIndentingOnLineWidthChange &&
            mbLineIsOutsideGeometry == rOtherObj.mbLineIsOutsideGeometry && bMarkProt == rOtherObj.bMarkProt &&
            bIs3DObj == rOtherObj.bIs3DObj && bIsEdge == rOtherObj.bIsEdge && bClosedObj == rOtherObj.bClosedObj &&
            bNotVisibleAsMaster == rOtherObj.bNotVisibleAsMaster && bEmptyPresObj == rOtherObj.bEmptyPresObj &&
            mbVisible == rOtherObj.mbVisible && bNoPrint == rOtherObj.bNoPrint && bSizProt == rOtherObj.bSizProt &&
            bMovProt == rOtherObj.bMovProt && bVirtObj == rOtherObj.bVirtObj &&
            mnLayerID == rOtherObj.mnLayerID && GetMergedItemSet().Equals(rOtherObj.GetMergedItemSet(), false) );
}

void SdrObject::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("SdrObject"));
    xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("ptr"), "%p", this);
    xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("symbol"), "%s", BAD_CAST(typeid(*this).name()));
    xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("name"), "%s", BAD_CAST(GetName().toUtf8().getStr()));
    xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("title"), "%s", BAD_CAST(GetTitle().toUtf8().getStr()));
    xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("description"), "%s", BAD_CAST(GetDescription().toUtf8().getStr()));
    xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("nOrdNum"), "%" SAL_PRIuUINT32, GetOrdNumDirect());
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("aOutRect"), BAD_CAST(aOutRect.toString().getStr()));

    if (const OutlinerParaObject* pOutliner = GetOutlinerParaObject())
        pOutliner->dumpAsXml(pWriter);

    xmlTextWriterEndElement(pWriter);
}

void SdrObject::SetOutlinerParaObject(std::unique_ptr<OutlinerParaObject> pTextObject)
{
    tools::Rectangle aBoundRect0; if (pUserCall!=nullptr) aBoundRect0=GetLastBoundRect();
    NbcSetOutlinerParaObject(std::move(pTextObject));
    SetChanged();
    BroadcastObjectChange();
    if (GetCurrentBoundRect()!=aBoundRect0) {
        SendUserCall(SdrUserCallType::Resize,aBoundRect0);
    }
}

void SdrObject::NbcSetOutlinerParaObject(std::unique_ptr<OutlinerParaObject> /*pTextObject*/)
{
}

OutlinerParaObject* SdrObject::GetOutlinerParaObject() const
{
    return nullptr;
}

void SdrObject::NbcReformatText()
{
}

void SdrObject::ReformatText()
{
    tools::Rectangle aBoundRect0; if (pUserCall!=nullptr) aBoundRect0=GetLastBoundRect();
    NbcReformatText();
    SetChanged();
    BroadcastObjectChange();
    if (GetCurrentBoundRect()!=aBoundRect0) {
        SendUserCall(SdrUserCallType::Resize,aBoundRect0);
    }
}

void SdrObject::BurnInStyleSheetAttributes()
{
    GetProperties().ForceStyleToHardAttributes();
}

bool SdrObject::HasMacro() const
{
    return false;
}

SdrObject* SdrObject::CheckMacroHit(const SdrObjMacroHitRec& rRec) const
{
    if(rRec.pPageView)
    {
        return SdrObjectPrimitiveHit(*this, rRec.aPos, rRec.nTol, *rRec.pPageView, rRec.pVisiLayer, false);
    }

    return nullptr;
}

Pointer SdrObject::GetMacroPointer(const SdrObjMacroHitRec&) const
{
    return Pointer(PointerStyle::RefHand);
}

void SdrObject::PaintMacro(OutputDevice& rOut, const tools::Rectangle& , const SdrObjMacroHitRec& ) const
{
    const RasterOp eRop(rOut.GetRasterOp());
    const basegfx::B2DPolyPolygon aPolyPolygon(TakeXorPoly());
    const sal_uInt32 nCount(aPolyPolygon.count());

    rOut.SetLineColor(COL_BLACK);
    rOut.SetFillColor();
    rOut.SetRasterOp(RasterOp::Invert);

    for(sal_uInt32 a(0); a < nCount; a++)
    {
        rOut.DrawPolyLine(aPolyPolygon.getB2DPolygon(a));
    }

    rOut.SetRasterOp(eRop);
}

bool SdrObject::DoMacro(const SdrObjMacroHitRec&)
{
    return false;
}

OUString SdrObject::GetMacroPopupComment(const SdrObjMacroHitRec&) const
{
    return OUString();
}

bool SdrObject::IsMacroHit(const SdrObjMacroHitRec& rRec) const
{
    return CheckMacroHit(rRec) != nullptr;
}


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
    if (pPlusData!=nullptr && pPlusData->pGluePoints!=nullptr) {
        if (rGeo.pGPL!=nullptr) {
            *rGeo.pGPL=*pPlusData->pGluePoints;
        } else {
            rGeo.pGPL.reset( new SdrGluePointList(*pPlusData->pGluePoints) );
        }
    } else {
        rGeo.pGPL.reset();
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
    if (rGeo.pGPL!=nullptr) {
        ImpForcePlusData();
        if (pPlusData->pGluePoints!=nullptr) {
            *pPlusData->pGluePoints=*rGeo.pGPL;
        } else {
            pPlusData->pGluePoints.reset(new SdrGluePointList(*rGeo.pGPL));
        }
    } else {
        if (pPlusData!=nullptr && pPlusData->pGluePoints!=nullptr) {
            pPlusData->pGluePoints.reset();
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
    tools::Rectangle aBoundRect0; if (pUserCall!=nullptr) aBoundRect0=GetLastBoundRect();
    RestGeoData(rGeo);
    SetChanged();
    BroadcastObjectChange();
    SendUserCall(SdrUserCallType::Resize,aBoundRect0);
}


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
    tools::Rectangle aBoundRect0; if (pUserCall!=nullptr) aBoundRect0=GetLastBoundRect();
    NbcApplyNotPersistAttr(rAttr);
    SetChanged();
    BroadcastObjectChange();
    SendUserCall(SdrUserCallType::Resize,aBoundRect0);
}

void SdrObject::NbcApplyNotPersistAttr(const SfxItemSet& rAttr)
{
    const tools::Rectangle& rSnap=GetSnapRect();
    const tools::Rectangle& rLogic=GetLogicRect();
    Point aRef1(rSnap.Center());
    const SfxPoolItem *pPoolItem=nullptr;
    if (rAttr.GetItemState(SDRATTR_TRANSFORMREF1X,true,&pPoolItem)==SfxItemState::SET) {
        aRef1.setX(static_cast<const SdrTransformRef1XItem*>(pPoolItem)->GetValue() );
    }
    if (rAttr.GetItemState(SDRATTR_TRANSFORMREF1Y,true,&pPoolItem)==SfxItemState::SET) {
        aRef1.setY(static_cast<const SdrTransformRef1YItem*>(pPoolItem)->GetValue() );
    }

    tools::Rectangle aNewSnap(rSnap);
    if (rAttr.GetItemState(SDRATTR_MOVEX,true,&pPoolItem)==SfxItemState::SET) {
        long n=static_cast<const SdrMoveXItem*>(pPoolItem)->GetValue();
        aNewSnap.Move(n,0);
    }
    if (rAttr.GetItemState(SDRATTR_MOVEY,true,&pPoolItem)==SfxItemState::SET) {
        long n=static_cast<const SdrMoveYItem*>(pPoolItem)->GetValue();
        aNewSnap.Move(0,n);
    }
    if (rAttr.GetItemState(SDRATTR_ONEPOSITIONX,true,&pPoolItem)==SfxItemState::SET) {
        long n=static_cast<const SdrOnePositionXItem*>(pPoolItem)->GetValue();
        aNewSnap.Move(n-aNewSnap.Left(),0);
    }
    if (rAttr.GetItemState(SDRATTR_ONEPOSITIONY,true,&pPoolItem)==SfxItemState::SET) {
        long n=static_cast<const SdrOnePositionYItem*>(pPoolItem)->GetValue();
        aNewSnap.Move(0,n-aNewSnap.Top());
    }
    if (rAttr.GetItemState(SDRATTR_ONESIZEWIDTH,true,&pPoolItem)==SfxItemState::SET) {
        long n=static_cast<const SdrOneSizeWidthItem*>(pPoolItem)->GetValue();
        aNewSnap.SetRight(aNewSnap.Left()+n );
    }
    if (rAttr.GetItemState(SDRATTR_ONESIZEHEIGHT,true,&pPoolItem)==SfxItemState::SET) {
        long n=static_cast<const SdrOneSizeHeightItem*>(pPoolItem)->GetValue();
        aNewSnap.SetBottom(aNewSnap.Top()+n );
    }
    if (aNewSnap!=rSnap) {
        if (aNewSnap.GetSize()==rSnap.GetSize()) {
            NbcMove(Size(aNewSnap.Left()-rSnap.Left(),aNewSnap.Top()-rSnap.Top()));
        } else {
            NbcSetSnapRect(aNewSnap);
        }
    }

    if (rAttr.GetItemState(SDRATTR_SHEARANGLE,true,&pPoolItem)==SfxItemState::SET) {
        long n=static_cast<const SdrShearAngleItem*>(pPoolItem)->GetValue();
        n-=GetShearAngle();
        if (n!=0) {
            double nTan=tan(n*nPi180);
            NbcShear(aRef1,n,nTan,false);
        }
    }
    if (rAttr.GetItemState(SDRATTR_ROTATEANGLE,true,&pPoolItem)==SfxItemState::SET) {
        long n=static_cast<const SdrAngleItem*>(pPoolItem)->GetValue();
        n-=GetRotateAngle();
        if (n!=0) {
            double nSin=sin(n*nPi180);
            double nCos=cos(n*nPi180);
            NbcRotate(aRef1,n,nSin,nCos);
        }
    }
    if (rAttr.GetItemState(SDRATTR_ROTATEONE,true,&pPoolItem)==SfxItemState::SET) {
        long n=static_cast<const SdrRotateOneItem*>(pPoolItem)->GetValue();
        double nSin=sin(n*nPi180);
        double nCos=cos(n*nPi180);
        NbcRotate(aRef1,n,nSin,nCos);
    }
    if (rAttr.GetItemState(SDRATTR_HORZSHEARONE,true,&pPoolItem)==SfxItemState::SET) {
        long n=static_cast<const SdrHorzShearOneItem*>(pPoolItem)->GetValue();
        double nTan=tan(n*nPi180);
        NbcShear(aRef1,n,nTan,false);
    }
    if (rAttr.GetItemState(SDRATTR_VERTSHEARONE,true,&pPoolItem)==SfxItemState::SET) {
        long n=static_cast<const SdrVertShearOneItem*>(pPoolItem)->GetValue();
        double nTan=tan(n*nPi180);
        NbcShear(aRef1,n,nTan,true);
    }

    if (rAttr.GetItemState(SDRATTR_OBJMOVEPROTECT,true,&pPoolItem)==SfxItemState::SET) {
        bool b=static_cast<const SdrYesNoItem*>(pPoolItem)->GetValue();
        SetMoveProtect(b);
    }
    if (rAttr.GetItemState(SDRATTR_OBJSIZEPROTECT,true,&pPoolItem)==SfxItemState::SET) {
        bool b=static_cast<const SdrYesNoItem*>(pPoolItem)->GetValue();
        SetResizeProtect(b);
    }

    /* move protect always sets size protect */
    if( IsMoveProtect() )
        SetResizeProtect( true );

    if (rAttr.GetItemState(SDRATTR_OBJPRINTABLE,true,&pPoolItem)==SfxItemState::SET) {
        bool b=static_cast<const SdrObjPrintableItem*>(pPoolItem)->GetValue();
        SetPrintable(b);
    }

    if (rAttr.GetItemState(SDRATTR_OBJVISIBLE,true,&pPoolItem)==SfxItemState::SET) {
        bool b=static_cast<const SdrObjVisibleItem*>(pPoolItem)->GetValue();
        SetVisible(b);
    }

    SdrLayerID nLayer=SDRLAYER_NOTFOUND;
    if (rAttr.GetItemState(SDRATTR_LAYERID,true,&pPoolItem)==SfxItemState::SET) {
        nLayer=static_cast<const SdrLayerIdItem*>(pPoolItem)->GetValue();
    }
    if (rAttr.GetItemState(SDRATTR_LAYERNAME,true,&pPoolItem)==SfxItemState::SET)
    {
        OUString aLayerName = static_cast<const SdrLayerNameItem*>(pPoolItem)->GetValue();
        const SdrLayerAdmin& rLayAd(nullptr != getSdrPageFromSdrObject()
            ? getSdrPageFromSdrObject()->GetLayerAdmin()
            : getSdrModelFromSdrObject().GetLayerAdmin());
        const SdrLayer* pLayer = rLayAd.GetLayer(aLayerName);

        if(nullptr != pLayer)
        {
            nLayer=pLayer->GetID();
        }
    }
    if (nLayer!=SDRLAYER_NOTFOUND) {
        NbcSetLayer(nLayer);
    }

    if (rAttr.GetItemState(SDRATTR_OBJECTNAME,true,&pPoolItem)==SfxItemState::SET) {
        OUString aName=static_cast<const SfxStringItem*>(pPoolItem)->GetValue();
        SetName(aName);
    }
    tools::Rectangle aNewLogic(rLogic);
    if (rAttr.GetItemState(SDRATTR_LOGICSIZEWIDTH,true,&pPoolItem)==SfxItemState::SET) {
        long n=static_cast<const SdrLogicSizeWidthItem*>(pPoolItem)->GetValue();
        aNewLogic.SetRight(aNewLogic.Left()+n );
    }
    if (rAttr.GetItemState(SDRATTR_LOGICSIZEHEIGHT,true,&pPoolItem)==SfxItemState::SET) {
        long n=static_cast<const SdrLogicSizeHeightItem*>(pPoolItem)->GetValue();
        aNewLogic.SetBottom(aNewLogic.Top()+n );
    }
    if (aNewLogic!=rLogic) {
        NbcSetLogicRect(aNewLogic);
    }
    Fraction aResizeX(1,1);
    Fraction aResizeY(1,1);
    if (rAttr.GetItemState(SDRATTR_RESIZEXONE,true,&pPoolItem)==SfxItemState::SET) {
        aResizeX*=static_cast<const SdrResizeXOneItem*>(pPoolItem)->GetValue();
    }
    if (rAttr.GetItemState(SDRATTR_RESIZEYONE,true,&pPoolItem)==SfxItemState::SET) {
        aResizeY*=static_cast<const SdrResizeYOneItem*>(pPoolItem)->GetValue();
    }
    if (aResizeX!=Fraction(1,1) || aResizeY!=Fraction(1,1)) {
        NbcResize(aRef1,aResizeX,aResizeY);
    }
}

void SdrObject::TakeNotPersistAttr(SfxItemSet& rAttr) const
{
    const tools::Rectangle& rSnap=GetSnapRect();
    const tools::Rectangle& rLogic=GetLogicRect();
    rAttr.Put(SdrYesNoItem(SDRATTR_OBJMOVEPROTECT, IsMoveProtect()));
    rAttr.Put(SdrYesNoItem(SDRATTR_OBJSIZEPROTECT, IsResizeProtect()));
    rAttr.Put(SdrObjPrintableItem(IsPrintable()));
    rAttr.Put(SdrObjVisibleItem(IsVisible()));
    rAttr.Put(SdrAngleItem(SDRATTR_ROTATEANGLE, GetRotateAngle()));
    rAttr.Put(SdrShearAngleItem(GetShearAngle()));
    rAttr.Put(SdrOneSizeWidthItem(rSnap.GetWidth()-1));
    rAttr.Put(SdrOneSizeHeightItem(rSnap.GetHeight()-1));
    rAttr.Put(SdrOnePositionXItem(rSnap.Left()));
    rAttr.Put(SdrOnePositionYItem(rSnap.Top()));
    if (rLogic.GetWidth()!=rSnap.GetWidth()) {
        rAttr.Put(SdrLogicSizeWidthItem(rLogic.GetWidth()-1));
    }
    if (rLogic.GetHeight()!=rSnap.GetHeight()) {
        rAttr.Put(SdrLogicSizeHeightItem(rLogic.GetHeight()-1));
    }
    OUString aName(GetName());

    if (!aName.isEmpty())
    {
        rAttr.Put(SfxStringItem(SDRATTR_OBJECTNAME, aName));
    }

    rAttr.Put(SdrLayerIdItem(GetLayer()));
    const SdrLayerAdmin& rLayAd(nullptr != getSdrPageFromSdrObject()
        ? getSdrPageFromSdrObject()->GetLayerAdmin()
        : getSdrModelFromSdrObject().GetLayerAdmin());
    const SdrLayer* pLayer = rLayAd.GetLayerPerID(GetLayer());
    if(nullptr != pLayer)
    {
        rAttr.Put(SdrLayerNameItem(pLayer->GetName()));
    }
    Point aRef1(rSnap.Center());
    Point aRef2(aRef1); aRef2.AdjustY( 1 );
    rAttr.Put(SdrTransformRef1XItem(aRef1.X()));
    rAttr.Put(SdrTransformRef1YItem(aRef1.Y()));
    rAttr.Put(SdrTransformRef2XItem(aRef2.X()));
    rAttr.Put(SdrTransformRef2YItem(aRef2.Y()));
}

SfxStyleSheet* SdrObject::GetStyleSheet() const
{
    return GetProperties().GetStyleSheet();
}

void SdrObject::SetStyleSheet(SfxStyleSheet* pNewStyleSheet, bool bDontRemoveHardAttr)
{
    tools::Rectangle aBoundRect0;

    if(pUserCall)
        aBoundRect0 = GetLastBoundRect();

    NbcSetStyleSheet(pNewStyleSheet, bDontRemoveHardAttr);
    SetChanged();
    BroadcastObjectChange();
    SendUserCall(SdrUserCallType::ChangeAttr, aBoundRect0);
}

void SdrObject::NbcSetStyleSheet(SfxStyleSheet* pNewStyleSheet, bool bDontRemoveHardAttr)
{
    // only allow graphic and presentation styles for shapes
    if( pNewStyleSheet && (pNewStyleSheet->GetFamily() == SfxStyleFamily::Para) && (pNewStyleSheet->GetFamily() == SfxStyleFamily::Page) )
        return;

    GetProperties().SetStyleSheet(pNewStyleSheet, bDontRemoveHardAttr);
}

// Broadcasting while setting attributes is managed by the AttrObj.


bool SdrObject::IsNode() const
{
    return true;
}

SdrGluePoint SdrObject::GetVertexGluePoint(sal_uInt16 nPosNum) const
{
    // #i41936# Use SnapRect for default GluePoints
    const tools::Rectangle aR(GetSnapRect());
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
    tools::Rectangle aR(GetCurrentBoundRect());
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
    if (pPlusData!=nullptr) return pPlusData->pGluePoints.get();
    return nullptr;
}


SdrGluePointList* SdrObject::ForceGluePointList()
{
    ImpForcePlusData();
    if (pPlusData->pGluePoints==nullptr) {
        pPlusData->pGluePoints.reset(new SdrGluePointList);
    }
    return pPlusData->pGluePoints.get();
}

void SdrObject::SetGlueReallyAbsolute(bool bOn)
{
    // First a const call to see whether there are any glue points.
    // Force const call!
    if (GetGluePointList()!=nullptr) {
        SdrGluePointList* pGPL=ForceGluePointList();
        pGPL->SetReallyAbsolute(bOn,*this);
    }
}

void SdrObject::NbcRotateGluePoints(const Point& rRef, long nAngle, double sn, double cs)
{
    // First a const call to see whether there are any glue points.
    // Force const call!
    if (GetGluePointList()!=nullptr) {
        SdrGluePointList* pGPL=ForceGluePointList();
        pGPL->Rotate(rRef,nAngle,sn,cs,this);
    }
}

void SdrObject::NbcMirrorGluePoints(const Point& rRef1, const Point& rRef2)
{
    // First a const call to see whether there are any glue points.
    // Force const call!
    if (GetGluePointList()!=nullptr) {
        SdrGluePointList* pGPL=ForceGluePointList();
        pGPL->Mirror(rRef1,rRef2,this);
    }
}

void SdrObject::NbcShearGluePoints(const Point& rRef, double tn, bool bVShear)
{
    // First a const call to see whether there are any glue points.
    // Force const call!
    if (GetGluePointList()!=nullptr) {
        SdrGluePointList* pGPL=ForceGluePointList();
        pGPL->Shear(rRef,tn,bVShear,this);
    }
}

void SdrObject::ConnectToNode(bool /*bTail1*/, SdrObject* /*pObj*/)
{
}

void SdrObject::DisconnectFromNode(bool /*bTail1*/)
{
}

SdrObject* SdrObject::GetConnectedNode(bool /*bTail1*/) const
{
    return nullptr;
}


void extractLineContourFromPrimitive2DSequence(
    const drawinglayer::primitive2d::Primitive2DContainer& rxSequence,
    basegfx::B2DPolygonVector& rExtractedHairlines,
    basegfx::B2DPolyPolygonVector& rExtractedLineFills)
{
    rExtractedHairlines.clear();
    rExtractedLineFills.clear();

    if(!rxSequence.empty())
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


SdrObject* SdrObject::ImpConvertToContourObj(bool bForceLineDash)
{
    SdrObject* pRetval(nullptr);

    if(LineGeometryUsageIsNecessary())
    {
        basegfx::B2DPolyPolygon aMergedLineFillPolyPolygon;
        basegfx::B2DPolyPolygon aMergedHairlinePolyPolygon;
        const drawinglayer::primitive2d::Primitive2DContainer xSequence(GetViewContact().getViewIndependentPrimitive2DContainer());

        if(!xSequence.empty())
        {
            basegfx::B2DPolygonVector aExtractedHairlines;
            basegfx::B2DPolyPolygonVector aExtractedLineFills;

            extractLineContourFromPrimitive2DSequence(xSequence, aExtractedHairlines, aExtractedLineFills);

            if(!aExtractedHairlines.empty())
            {
                // for SdrObject creation, just copy all to a single Hairline-PolyPolygon
                for(basegfx::B2DPolygon & rExtractedHairline : aExtractedHairlines)
                {
                    aMergedHairlinePolyPolygon.append(rExtractedHairline);
                }
            }

            // check for fill rsults
            if (!aExtractedLineFills.empty() && !utl::ConfigManager::IsFuzzing())
            {
                // merge to a single tools::PolyPolygon (OR)
                aMergedLineFillPolyPolygon = basegfx::utils::mergeToSinglePolyPolygon(aExtractedLineFills);
            }
        }

        if(aMergedLineFillPolyPolygon.count() || (bForceLineDash && aMergedHairlinePolyPolygon.count()))
        {
            SfxItemSet aSet(GetMergedItemSet());
            drawing::FillStyle eOldFillStyle = aSet.Get(XATTR_FILLSTYLE).GetValue();
            SdrPathObj* aLinePolygonPart = nullptr;
            SdrPathObj* aLineHairlinePart = nullptr;
            bool bBuildGroup(false);

            if(aMergedLineFillPolyPolygon.count())
            {
                // create SdrObject for filled line geometry
                aLinePolygonPart = new SdrPathObj(
                    getSdrModelFromSdrObject(),
                    OBJ_PATHFILL,
                    aMergedLineFillPolyPolygon);

                // correct item properties
                aSet.Put(XLineWidthItem(0));
                aSet.Put(XLineStyleItem(drawing::LineStyle_NONE));
                Color aColorLine = aSet.Get(XATTR_LINECOLOR).GetColorValue();
                sal_uInt16 nTransLine = aSet.Get(XATTR_LINETRANSPARENCE).GetValue();
                aSet.Put(XFillColorItem(OUString(), aColorLine));
                aSet.Put(XFillStyleItem(drawing::FillStyle_SOLID));
                aSet.Put(XFillTransparenceItem(nTransLine));

                aLinePolygonPart->SetMergedItemSet(aSet);
            }

            if(aMergedHairlinePolyPolygon.count())
            {
                // create SdrObject for hairline geometry
                // OBJ_PATHLINE is necessary here, not OBJ_PATHFILL. This is intended
                // to get a non-filled object. If the poly is closed, the PathObj takes care for
                // the correct closed state.
                aLineHairlinePart = new SdrPathObj(
                    getSdrModelFromSdrObject(),
                    OBJ_PATHLINE,
                    aMergedHairlinePolyPolygon);

                aSet.Put(XLineWidthItem(0));
                aSet.Put(XFillStyleItem(drawing::FillStyle_NONE));
                aSet.Put(XLineStyleItem(drawing::LineStyle_SOLID));

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
            SdrPathObj* pPath = dynamic_cast<SdrPathObj*>(this);

            if(pPath && pPath->IsClosed())
            {
                if(eOldFillStyle != drawing::FillStyle_NONE)
                {
                    bAddOriginalGeometry = true;
                }
            }

            // do we need a group?
            if(bBuildGroup || bAddOriginalGeometry)
            {
                SdrObject* pGroup = new SdrObjGroup(getSdrModelFromSdrObject());

                if(bAddOriginalGeometry)
                {
                    // Add a clone of the original geometry.
                    aSet.ClearItem();
                    aSet.Put(GetMergedItemSet());
                    aSet.Put(XLineStyleItem(drawing::LineStyle_NONE));
                    aSet.Put(XLineWidthItem(0));

                    SdrObject* pClone(CloneSdrObject(getSdrModelFromSdrObject()));
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

                pRetval = pGroup;
            }
            else
            {
                if(aLinePolygonPart)
                {
                    pRetval = aLinePolygonPart;
                }
                else if(aLineHairlinePart)
                {
                    pRetval = aLineHairlinePart;
                }
            }
        }
    }

    if(nullptr == pRetval)
    {
        // due to current method usage, create and return a clone when nothing has changed
        SdrObject* pClone(CloneSdrObject(getSdrModelFromSdrObject()));
        pRetval = pClone;
    }

    return pRetval;
}


void SdrObject::SetMarkProtect(bool bProt)
{
    bMarkProt = bProt;
}


void SdrObject::SetEmptyPresObj(bool bEpt)
{
    bEmptyPresObj = bEpt;
}


void SdrObject::SetNotVisibleAsMaster(bool bFlg)
{
    bNotVisibleAsMaster=bFlg;
}


// convert this path object to contour object, even when it is a group
SdrObject* SdrObject::ConvertToContourObj(SdrObject* pRet, bool bForceLineDash) const
{
    if(dynamic_cast<const SdrObjGroup*>( pRet) !=  nullptr)
    {
        SdrObjList* pObjList2 = pRet->GetSubList();
        SdrObject* pGroup = new SdrObjGroup(getSdrModelFromSdrObject());

        for(size_t a=0; a<pObjList2->GetObjCount(); ++a)
        {
            SdrObject* pIterObj = pObjList2->GetObj(a);
            pGroup->GetSubList()->NbcInsertObject(ConvertToContourObj(pIterObj, bForceLineDash));
        }

        pRet = pGroup;
    }
    else
    {
        if (SdrPathObj *pPathObj = dynamic_cast<SdrPathObj*>(pRet))
        {
            // bezier geometry got created, even for straight edges since the given
            // object is a result of DoConvertToPolyObj. For conversion to contour
            // this is not really needed and can be reduced again AFAP
            pPathObj->SetPathPoly(basegfx::utils::simplifyCurveSegments(pPathObj->GetPathPoly()));
        }

        pRet = pRet->ImpConvertToContourObj(bForceLineDash);
    }

    // #i73441# preserve LayerID
    if(pRet && pRet->GetLayer() != GetLayer())
    {
        pRet->SetLayer(GetLayer());
    }

    return pRet;
}


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


SdrObject* SdrObject::DoConvertToPolyObj(bool /*bBezier*/, bool /*bAddText*/) const
{
    return nullptr;
}


void SdrObject::InsertedStateChange()
{
    const bool bIsInserted(nullptr != getParentSdrObjListFromSdrObject());
    const tools::Rectangle aBoundRect0(GetLastBoundRect());

    if(bIsInserted)
    {
        SendUserCall(SdrUserCallType::Inserted, aBoundRect0);
    }
    else
    {
        SendUserCall(SdrUserCallType::Removed, aBoundRect0);
    }

    if(nullptr != pPlusData && nullptr != pPlusData->pBroadcast)
    {
        SdrHint aHint(bIsInserted ? SdrHintKind::ObjectInserted : SdrHintKind::ObjectRemoved, *this);
        pPlusData->pBroadcast->Broadcast(aHint);
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
        if (IsInserted())
        {
            SdrHint aHint(SdrHintKind::ObjectChange, *this);
            getSdrModelFromSdrObject().Broadcast(aHint);
        }
    }
}

void SdrObject::SetVisible(bool bVisible)
{
    if( bVisible != mbVisible )
    {
        mbVisible = bVisible;
        SetChanged();
        if (IsInserted())
        {
            SdrHint aHint(SdrHintKind::ObjectChange, *this);
            getSdrModelFromSdrObject().Broadcast(aHint);
        }
    }
}


sal_uInt16 SdrObject::GetUserDataCount() const
{
    if (pPlusData==nullptr || pPlusData->pUserDataList==nullptr) return 0;
    return pPlusData->pUserDataList->GetUserDataCount();
}

SdrObjUserData* SdrObject::GetUserData(sal_uInt16 nNum) const
{
    if (pPlusData==nullptr || pPlusData->pUserDataList==nullptr) return nullptr;
    return &pPlusData->pUserDataList->GetUserData(nNum);
}

void SdrObject::AppendUserData(std::unique_ptr<SdrObjUserData> pData)
{
    if (!pData)
    {
        OSL_FAIL("SdrObject::AppendUserData(): pData is NULL pointer.");
        return;
    }

    ImpForcePlusData();
    if (!pPlusData->pUserDataList)
        pPlusData->pUserDataList.reset( new SdrObjUserDataList );

    pPlusData->pUserDataList->AppendUserData(std::move(pData));
}

void SdrObject::DeleteUserData(sal_uInt16 nNum)
{
    sal_uInt16 nCount=GetUserDataCount();
    if (nNum<nCount) {
        pPlusData->pUserDataList->DeleteUserData(nNum);
        if (nCount==1)  {
            pPlusData->pUserDataList.reset();
        }
    } else {
        OSL_FAIL("SdrObject::DeleteUserData(): Invalid Index.");
    }
}

void SdrObject::SetUserCall(SdrObjUserCall* pUser)
{
    pUserCall = pUser;
}


void SdrObject::SendUserCall(SdrUserCallType eUserCall, const tools::Rectangle& rBoundRect) const
{
    SdrObject* pGroup(getParentSdrObjectFromSdrObject());

    if ( pUserCall )
    {
        pUserCall->Changed( *this, eUserCall, rBoundRect );
    }

    if(nullptr != pGroup && pGroup->GetUserCall())
    {
        // broadcast to group
        SdrUserCallType eChildUserType = SdrUserCallType::ChildChangeAttr;

        switch( eUserCall )
        {
            case SdrUserCallType::MoveOnly:
                eChildUserType = SdrUserCallType::ChildMoveOnly;
            break;

            case SdrUserCallType::Resize:
                eChildUserType = SdrUserCallType::ChildResize;
            break;

            case SdrUserCallType::ChangeAttr:
                eChildUserType = SdrUserCallType::ChildChangeAttr;
            break;

            case SdrUserCallType::Delete:
                eChildUserType = SdrUserCallType::ChildDelete;
            break;

            case SdrUserCallType::Inserted:
                eChildUserType = SdrUserCallType::ChildInserted;
            break;

            case SdrUserCallType::Removed:
                eChildUserType = SdrUserCallType::ChildRemoved;
            break;

            default: break;
        }

        pGroup->GetUserCall()->Changed( *this, eChildUserType, rBoundRect );
    }

    // notify our UNO shape listeners
    switch ( eUserCall )
    {
    case SdrUserCallType::Resize:
        notifyShapePropertyChange( svx::ShapeProperty::Size );
        SAL_FALLTHROUGH; // RESIZE might also imply a change of the position
    case SdrUserCallType::MoveOnly:
        notifyShapePropertyChange( svx::ShapeProperty::Position );
        break;
    default:
        // not interested in
        break;
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
            mpSvxShape = nullptr;
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
        if (mpSvxShape)
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
        mpSvxShape = nullptr;

    return mpSvxShape;
}

css::uno::Reference< css::uno::XInterface > SdrObject::getUnoShape()
{
    // try weak reference first
    uno::Reference< uno::XInterface > xShape( getWeakUnoShape() );
    if( !xShape.is() )
    {
        OSL_ENSURE( mpSvxShape == nullptr, "SdrObject::getUnoShape: XShape already dead, but still an IMPL pointer!" );
        if ( getSdrPageFromSdrObject() )
        {
            uno::Reference< uno::XInterface > xPage( getSdrPageFromSdrObject()->getUnoPage() );
            if( xPage.is() )
            {
                SvxDrawPage* pDrawPage = SvxDrawPage::getImplementation(xPage);
                if( pDrawPage )
                {
                    // create one
                    xShape = pDrawPage->CreateShape( this );
                    impl_setUnoShape( xShape );
                }
            }
        }
        else
        {
            mpSvxShape = SvxDrawPage::CreateShapeByTypeAndInventor( GetObjIdentifier(), GetObjInventor(), this );
            maWeakUnoShape = xShape = static_cast< ::cppu::OWeakObject* >( mpSvxShape );
        }
    }

    return xShape;
}

void SdrObject::setUnoShape(const uno::Reference<uno::XInterface >& _rxUnoShape)
{
    impl_setUnoShape( _rxUnoShape );
}

svx::PropertyChangeNotifier& SdrObject::getShapePropertyChangeNotifier()
{
    DBG_TESTSOLARMUTEX();

    SvxShape* pSvxShape = getSvxShape();
    ENSURE_OR_THROW( pSvxShape, "no SvxShape, yet!" );
    return pSvxShape->getShapePropertyChangeNotifier();
}

void SdrObject::notifyShapePropertyChange( const svx::ShapeProperty _eProperty ) const
{
    DBG_TESTSOLARMUTEX();

    SvxShape* pSvxShape = const_cast< SdrObject* >( this )->getSvxShape();
    if ( pSvxShape )
        return pSvxShape->getShapePropertyChangeNotifier().notifyPropertyChange( _eProperty );
}


// transformation interface for StarOfficeAPI. This implements support for
// homogeneous 3x3 matrices containing the transformation of the SdrObject. At the
// moment it contains a shearX, rotation and translation, but for setting all linear
// transforms like Scale, ShearX, ShearY, Rotate and Translate are supported.


// gets base transformation and rectangle of object. If it's an SdrPathObj it fills the PolyPolygon
// with the base geometry and returns TRUE. Otherwise it returns FALSE.
bool SdrObject::TRGetBaseGeometry(basegfx::B2DHomMatrix& rMatrix, basegfx::B2DPolyPolygon& /*rPolyPolygon*/) const
{
    // any kind of SdrObject, just use SnapRect
    tools::Rectangle aRectangle(GetSnapRect());

    // convert to transformation values
    basegfx::B2DTuple aScale(aRectangle.GetWidth(), aRectangle.GetHeight());
    basegfx::B2DTuple aTranslate(aRectangle.Left(), aRectangle.Top());

    // position maybe relative to anchorpos, convert
    if(getSdrModelFromSdrObject().IsWriter())
    {
        if(GetAnchorPos().X() || GetAnchorPos().Y())
        {
            aTranslate -= basegfx::B2DTuple(GetAnchorPos().X(), GetAnchorPos().Y());
        }
    }

    // build matrix
    rMatrix = basegfx::utils::createScaleTranslateB2DHomMatrix(aScale, aTranslate);

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
    }

    // if anchor is used, make position relative to it
    if(getSdrModelFromSdrObject().IsWriter())
    {
        if(GetAnchorPos().X() || GetAnchorPos().Y())
        {
            aTranslate += basegfx::B2DTuple(GetAnchorPos().X(), GetAnchorPos().Y());
        }
    }

    // build BaseRect
    Point aPoint(FRound(aTranslate.getX()), FRound(aTranslate.getY()));
    tools::Rectangle aBaseRect(aPoint, Size(FRound(aScale.getX()), FRound(aScale.getY())));

    // set BaseRect
    SetSnapRect(aBaseRect);
}

// Give info if object is in destruction
bool SdrObject::IsInDestruction() const
{
    return getSdrModelFromSdrObject().IsInDestruction();
}

// return if fill is != drawing::FillStyle_NONE
bool SdrObject::HasFillStyle() const
{
    return GetObjectItem(XATTR_FILLSTYLE).GetValue() != drawing::FillStyle_NONE;
}

bool SdrObject::HasLineStyle() const
{
    return GetObjectItem(XATTR_LINESTYLE).GetValue() != drawing::LineStyle_NONE;
}


// #i52224#
// on import of OLE object from MS documents the BLIP size might be retrieved,
// the following four methods are used to control it;
// usually this data makes no sense after the import is finished, since the object
// might be resized


void SdrObject::SetBLIPSizeRectangle( const tools::Rectangle& aRect )
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


// #i121917#
bool SdrObject::HasText() const
{
    return false;
}

bool SdrObject::IsTextBox() const
{
    return false;
}

SdrDelayBroadcastObjectChange::SdrDelayBroadcastObjectChange( SdrObject& rObj ) :
    mrObj(rObj), mbOldDelayBroadcastObjectChange( rObj.mbDelayBroadcastObjectChange)
{
    mrObj.mbDelayBroadcastObjectChange = true;
}

SdrDelayBroadcastObjectChange::~SdrDelayBroadcastObjectChange()
{
    if (!mbOldDelayBroadcastObjectChange)
    {
        mrObj.mbDelayBroadcastObjectChange = false;
        if (mrObj.mbBroadcastObjectChangePending)
        {
            mrObj.mbBroadcastObjectChangePending = false;
            mrObj.BroadcastObjectChange();
        }
    }
}


SdrObject* SdrObjFactory::CreateObjectFromFactory(SdrModel& rSdrModel, SdrInventor nInventor, sal_uInt16 nObjIdentifier)
{
    SdrObjCreatorParams aParams { nInventor, nObjIdentifier, rSdrModel };
    for (const auto & i : ImpGetUserMakeObjHdl()) {
        SdrObject* pObj = i.Call(aParams);
        if (pObj) {
            return pObj;
        }
    }
    return nullptr;
}

SdrObject* SdrObjFactory::MakeNewObject(
    SdrModel& rSdrModel,
    SdrInventor nInventor,
    sal_uInt16 nIdentifier,
    const tools::Rectangle* pSnapRect)
{
    SdrObject* pObj(nullptr);
    bool bSetSnapRect(nullptr != pSnapRect);

    if (nInventor == SdrInventor::Default)
    {
        switch (nIdentifier)
        {
            case OBJ_MEASURE:
            {
                if(nullptr != pSnapRect)
                {
                    pObj = new SdrMeasureObj(
                        rSdrModel,
                        pSnapRect->TopLeft(),
                        pSnapRect->BottomRight());
                }
                else
                {
                    pObj = new SdrMeasureObj(rSdrModel);
                }
            }
            break;
            case OBJ_LINE:
            {
                if(nullptr != pSnapRect)
                {
                    basegfx::B2DPolygon aPoly;

                    aPoly.append(
                        basegfx::B2DPoint(
                            pSnapRect->Left(),
                            pSnapRect->Top()));
                    aPoly.append(
                        basegfx::B2DPoint(
                            pSnapRect->Right(),
                            pSnapRect->Bottom()));
                    pObj = new SdrPathObj(
                        rSdrModel,
                        OBJ_LINE,
                        basegfx::B2DPolyPolygon(aPoly));
                }
                else
                {
                    pObj = new SdrPathObj(
                        rSdrModel,
                        OBJ_LINE);
                }
            }
            break;
            case OBJ_TEXT:
            case OBJ_TITLETEXT:
            case OBJ_OUTLINETEXT:
            {
                if(nullptr != pSnapRect)
                {
                    pObj = new SdrRectObj(
                        rSdrModel,
                        static_cast<SdrObjKind>(nIdentifier),
                        *pSnapRect);
                    bSetSnapRect = false;
                }
                else
                {
                    pObj = new SdrRectObj(
                        rSdrModel,
                        static_cast<SdrObjKind>(nIdentifier));
                }
            }
            break;
            case OBJ_CIRC:
            case OBJ_SECT:
            case OBJ_CARC:
            case OBJ_CCUT:
            {
                if(nullptr != pSnapRect)
                {
                    pObj = new SdrCircObj(
                        rSdrModel,
                        static_cast<SdrObjKind>(nIdentifier),
                        *pSnapRect);
                    bSetSnapRect = false;
                }
                else
                {
                    pObj = new SdrCircObj(
                        rSdrModel,
                        static_cast<SdrObjKind>(nIdentifier));
                }
            }
            break;
            case sal_uInt16(OBJ_NONE       ): pObj=new SdrObject(rSdrModel);                   break;
            case sal_uInt16(OBJ_GRUP       ): pObj=new SdrObjGroup(rSdrModel);                 break;
            case sal_uInt16(OBJ_POLY       ): pObj=new SdrPathObj(rSdrModel, OBJ_POLY       ); break;
            case sal_uInt16(OBJ_PLIN       ): pObj=new SdrPathObj(rSdrModel, OBJ_PLIN       ); break;
            case sal_uInt16(OBJ_PATHLINE   ): pObj=new SdrPathObj(rSdrModel, OBJ_PATHLINE   ); break;
            case sal_uInt16(OBJ_PATHFILL   ): pObj=new SdrPathObj(rSdrModel, OBJ_PATHFILL   ); break;
            case sal_uInt16(OBJ_FREELINE   ): pObj=new SdrPathObj(rSdrModel, OBJ_FREELINE   ); break;
            case sal_uInt16(OBJ_FREEFILL   ): pObj=new SdrPathObj(rSdrModel, OBJ_FREEFILL   ); break;
            case sal_uInt16(OBJ_PATHPOLY   ): pObj=new SdrPathObj(rSdrModel, OBJ_POLY       ); break;
            case sal_uInt16(OBJ_PATHPLIN   ): pObj=new SdrPathObj(rSdrModel, OBJ_PLIN       ); break;
            case sal_uInt16(OBJ_EDGE       ): pObj=new SdrEdgeObj(rSdrModel);                  break;
            case sal_uInt16(OBJ_RECT       ): pObj=new SdrRectObj(rSdrModel);                  break;
            case sal_uInt16(OBJ_GRAF       ): pObj=new SdrGrafObj(rSdrModel);                  break;
            case sal_uInt16(OBJ_OLE2       ): pObj=new SdrOle2Obj(rSdrModel);                  break;
            case sal_uInt16(OBJ_FRAME      ): pObj=new SdrOle2Obj(rSdrModel, true);            break;
            case sal_uInt16(OBJ_CAPTION    ): pObj=new SdrCaptionObj(rSdrModel);               break;
            case sal_uInt16(OBJ_PAGE       ): pObj=new SdrPageObj(rSdrModel);                  break;
            case sal_uInt16(OBJ_UNO        ): pObj=new SdrUnoObj(rSdrModel, OUString());       break;
            case sal_uInt16(OBJ_CUSTOMSHAPE  ): pObj=new SdrObjCustomShape(rSdrModel);       break;
#if HAVE_FEATURE_AVMEDIA
            case sal_uInt16(OBJ_MEDIA      ): pObj=new SdrMediaObj(rSdrModel);               break;
#endif
            case sal_uInt16(OBJ_TABLE      ): pObj=new sdr::table::SdrTableObj(rSdrModel);   break;
        }
    }

    if (!pObj)
    {
        pObj = CreateObjectFromFactory(rSdrModel, nInventor, nIdentifier);
    }

    if (!pObj)
    {
        // Well, if no one wants it...
        return nullptr;
    }

    if(bSetSnapRect && nullptr != pSnapRect)
    {
        pObj->SetSnapRect(*pSnapRect);
    }

    return pObj;
}

void SdrObjFactory::InsertMakeObjectHdl(Link<SdrObjCreatorParams, SdrObject*> const & rLink)
{
    std::vector<Link<SdrObjCreatorParams, SdrObject*>>& rLL=ImpGetUserMakeObjHdl();
    auto it = std::find(rLL.begin(), rLL.end(), rLink);
    if (it != rLL.end()) {
        OSL_FAIL("SdrObjFactory::InsertMakeObjectHdl(): Link already in place.");
    } else {
        rLL.push_back(rLink);
    }
}

void SdrObjFactory::RemoveMakeObjectHdl(Link<SdrObjCreatorParams, SdrObject*> const & rLink)
{
    std::vector<Link<SdrObjCreatorParams, SdrObject*>>& rLL=ImpGetUserMakeObjHdl();
    auto it = std::find(rLL.begin(), rLL.end(), rLink);
    if (it != rLL.end())
        rLL.erase(it);
}

namespace svx
{
    ISdrObjectFilter::~ISdrObjectFilter()
    {
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
