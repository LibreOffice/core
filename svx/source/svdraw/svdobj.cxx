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
#include <sal/log.hxx>
#include <rtl/ustrbuf.hxx>

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <com/sun/star/frame/XTerminateListener.hpp>
#include <com/sun/star/frame/Desktop.hpp>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygoncutter.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/range/b2drange.hxx>
#include <drawinglayer/processor2d/contourextractor2d.hxx>
#include <drawinglayer/processor2d/linegeometryextractor2d.hxx>
#include <comphelper/processfactory.hxx>
#include <editeng/editeng.hxx>
#include <editeng/outlobj.hxx>
#include <o3tl/deleter.hxx>
#include <math.h>
#include <svl/grabbagitem.hxx>
#include <tools/bigint.hxx>
#include <tools/diagnose_ex.h>
#include <tools/helpers.hxx>
#include <unotools/configmgr.hxx>
#include <vcl/canvastools.hxx>
#include <vcl/ptrstyle.hxx>
#include <vector>

#include <svx/shapepropertynotifier.hxx>
#include <svx/svdotable.hxx>

#include <svx/sdr/contact/displayinfo.hxx>
#include <sdr/contact/objectcontactofobjlistpainter.hxx>
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
#include <svx/xfillit0.hxx>
#include <svx/xflclit.hxx>
#include <svx/xfltrit.hxx>
#include <svx/xlineit0.hxx>
#include <svx/xlnclit.hxx>
#include <svx/xlnedwit.hxx>
#include <svx/xlnstwit.hxx>
#include <svx/xlntrit.hxx>
#include <svx/xlnwtit.hxx>
#include <svx/svdglue.hxx>
#include <svx/svdsob.hxx>
#include <svdobjplusdata.hxx>
#include <svdobjuserdatalist.hxx>

#include <unordered_set>

#include <optional>
#include <libxml/xmlwriter.h>
#include <memory>

#include <svx/scene3d.hxx>
#include <rtl/character.hxx>

using namespace ::com::sun::star;


SdrObjUserCall::~SdrObjUserCall()
{
}

void SdrObjUserCall::Changed(const SdrObject& /*rObj*/, SdrUserCallType /*eType*/, const tools::Rectangle& /*rOldBoundRect*/)
{
}

SdrObjMacroHitRec::SdrObjMacroHitRec() :
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
    std::shared_ptr<DiagramDataInterface> mpDiagramData;
    std::optional<double> mnRelativeWidth;
    std::optional<double> mnRelativeHeight;
    sal_Int16               meRelativeWidthRelation;
    sal_Int16               meRelativeHeightRelation;

    Impl() :
        meRelativeWidthRelation(text::RelOrientation::PAGE_FRAME),
        meRelativeHeightRelation(text::RelOrientation::PAGE_FRAME) {}
};


// BaseProperties section

std::unique_ptr<sdr::properties::BaseProperties> SdrObject::CreateObjectSpecificProperties()
{
    return std::make_unique<sdr::properties::EmptyProperties>(*this);
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
    return std::make_unique<sdr::contact::ViewContactOfSdrObj>(*this);
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
    if (SdrObjList* pParentList = getParentSdrObjListFromSdrObject())
    {
        return pParentList->getSdrPageFromSdrObjList();
    }

    return nullptr;
}

SdrModel& SdrObject::getSdrModelFromSdrObject() const
{
    return mrSdrModelFromSdrObject;
}

void SdrObject::setParentOfSdrObject(SdrObjList* pNewObjList)
{
    if(getParentSdrObjListFromSdrObject() == pNewObjList)
        return;

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
    m_aOutRect = tools::Rectangle();
}

#ifdef DBG_UTIL
// SdrObjectLifetimeWatchDog:
void impAddIncarnatedSdrObjectToSdrModel(const SdrObject& rSdrObject, SdrModel& rSdrModel)
{
    rSdrModel.maAllIncarnatedObjects.insert(&rSdrObject);
}
void impRemoveIncarnatedSdrObjectToSdrModel(const SdrObject& rSdrObject, SdrModel& rSdrModel)
{
    if(!rSdrModel.maAllIncarnatedObjects.erase(&rSdrObject))
    {
        SAL_WARN("svx","SdrObject::~SdrObject: Destructed incarnation of SdrObject not member of this SdrModel (!)");
    }
}
#endif

SdrObject::SdrObject(SdrModel& rSdrModel)
:   mpFillGeometryDefiningShape(nullptr)
    ,mrSdrModelFromSdrObject(rSdrModel)
    ,m_pUserCall(nullptr)
    ,mpImpl(new Impl)
    ,mpParentOfSdrObject(nullptr)
    ,m_nOrdNum(0)
    ,mnNavigationPosition(SAL_MAX_UINT32)
    ,mnLayerID(0)
    ,mpSvxShape( nullptr )
    ,mbDoNotInsertIntoPageAutomatically(false)
{
    m_bVirtObj         =false;
    m_bSnapRectDirty   =true;
    m_bMovProt         =false;
    m_bSizProt         =false;
    m_bNoPrint         =false;
    m_bEmptyPresObj    =false;
    m_bNotVisibleAsMaster=false;
    m_bClosedObj       =false;
    mbVisible        = true;

    // #i25616#
    mbLineIsOutsideGeometry = false;

    // #i25616#
    mbSupportTextIndentingOnLineWidthChange = false;

    m_bIsEdge=false;
    m_bIs3DObj=false;
    m_bMarkProt=false;
    m_bIsUnoObj=false;
#ifdef DBG_UTIL
    // SdrObjectLifetimeWatchDog:
    impAddIncarnatedSdrObjectToSdrModel(*this, getSdrModelFromSdrObject());
#endif
}

SdrObject::SdrObject(SdrModel& rSdrModel, SdrObject const & rSource)
:   mpFillGeometryDefiningShape(nullptr)
    ,mrSdrModelFromSdrObject(rSdrModel)
    ,m_pUserCall(nullptr)
    ,mpImpl(new Impl)
    ,mpParentOfSdrObject(nullptr)
    ,m_nOrdNum(0)
    ,mnNavigationPosition(SAL_MAX_UINT32)
    ,mnLayerID(0)
    ,mpSvxShape( nullptr )
    ,mbDoNotInsertIntoPageAutomatically(false)
{
    m_bVirtObj         =false;
    m_bSnapRectDirty   =true;
    m_bMovProt         =false;
    m_bSizProt         =false;
    m_bNoPrint         =false;
    m_bEmptyPresObj    =false;
    m_bNotVisibleAsMaster=false;
    m_bClosedObj       =false;
    mbVisible        = true;

    // #i25616#
    mbLineIsOutsideGeometry = false;

    // #i25616#
    mbSupportTextIndentingOnLineWidthChange = false;

    m_bIsEdge=false;
    m_bIs3DObj=false;
    m_bMarkProt=false;
    m_bIsUnoObj=false;
#ifdef DBG_UTIL
    // SdrObjectLifetimeWatchDog:
    impAddIncarnatedSdrObjectToSdrModel(*this, getSdrModelFromSdrObject());
#endif

    mpProperties.reset();
    mpViewContact.reset();

    // The CloneSdrObject() method uses the local copy constructor from the individual
    // sdr::properties::BaseProperties class. Since the target class maybe for another
    // draw object, an SdrObject needs to be provided, as in the normal constructor.
    mpProperties = rSource.GetProperties().Clone(*this);

    m_aOutRect=rSource.m_aOutRect;
    mnLayerID = rSource.mnLayerID;
    m_aAnchor =rSource.m_aAnchor;
    m_bVirtObj=rSource.m_bVirtObj;
    m_bSizProt=rSource.m_bSizProt;
    m_bMovProt=rSource.m_bMovProt;
    m_bNoPrint=rSource.m_bNoPrint;
    mbVisible=rSource.mbVisible;
    m_bMarkProt=rSource.m_bMarkProt;
    m_bEmptyPresObj =rSource.m_bEmptyPresObj;
    m_bNotVisibleAsMaster=rSource.m_bNotVisibleAsMaster;
    m_bSnapRectDirty=true;
    m_pPlusData.reset();
    if (rSource.m_pPlusData!=nullptr) {
        m_pPlusData.reset(rSource.m_pPlusData->Clone(this));
    }
    if (m_pPlusData!=nullptr && m_pPlusData->pBroadcast!=nullptr) {
        m_pPlusData->pBroadcast.reset(); // broadcaster isn't copied
    }

    m_pGrabBagItem.reset();
    if (rSource.m_pGrabBagItem!=nullptr)
        m_pGrabBagItem.reset(rSource.m_pGrabBagItem->Clone());
}

SdrObject::~SdrObject()
{
    // Tell all the registered ObjectUsers that the page is in destruction.
    // And clear the vector. This means that user do not need to call RemoveObjectUser()
    // when they get called from ObjectInDestruction().
    sdr::ObjectUserVector aList;
    aList.swap(mpImpl->maObjectUsers);
    for(sdr::ObjectUser* pObjectUser : aList)
    {
        DBG_ASSERT(pObjectUser, "SdrObject::~SdrObject: corrupt ObjectUser list (!)");
        pObjectUser->ObjectInDestruction(*this);
    }

    // UserCall
    SendUserCall(SdrUserCallType::Delete, GetLastBoundRect());
    o3tl::reset_preserve_ptr_during(m_pPlusData);

    m_pGrabBagItem.reset();
    mpProperties.reset();
    mpViewContact.reset();

#ifdef DBG_UTIL
    // SdrObjectLifetimeWatchDog:
    impRemoveIncarnatedSdrObjectToSdrModel(*this, getSdrModelFromSdrObject());
#endif
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
                uno::Reference< lang::XComponent > xShapeComp( pObject->getWeakUnoShape().get(), uno::UNO_QUERY_THROW );
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

void SdrObject::SetBoundAndSnapRectsDirty(bool bNotMyself, bool bRecursive)
{
    if (!bNotMyself)
    {
        SetBoundRectDirty();
        m_bSnapRectDirty=true;
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


// global static ItemPool for not-yet-inserted items
static rtl::Reference<SdrItemPool> mpGlobalItemPool;

/** If we let the libc runtime clean us up, we trigger a crash */
namespace
{
class TerminateListener : public ::cppu::WeakImplHelper< css::frame::XTerminateListener >
{
    void SAL_CALL queryTermination( const lang::EventObject& ) override
    {}
    void SAL_CALL notifyTermination( const lang::EventObject& ) override
    {
        mpGlobalItemPool.clear();
    }
    virtual void SAL_CALL disposing( const ::css::lang::EventObject& ) override
    {}
};
};

// init global static itempool
SdrItemPool& SdrObject::GetGlobalDrawObjectItemPool()
{
    if(!mpGlobalItemPool)
    {
        mpGlobalItemPool = new SdrItemPool();
        rtl::Reference<SfxItemPool> pGlobalOutlPool = EditEngine::CreatePool();
        mpGlobalItemPool->SetSecondaryPool(pGlobalOutlPool.get());
        mpGlobalItemPool->SetDefaultMetric(SdrEngineDefaults::GetMapUnit());
        mpGlobalItemPool->FreezeIdRanges();
        if (utl::ConfigManager::IsFuzzing())
            mpGlobalItemPool->acquire();
        else
        {
            uno::Reference< frame::XDesktop2 > xDesktop = frame::Desktop::create(comphelper::getProcessComponentContext());
            uno::Reference< frame::XTerminateListener > xListener( new TerminateListener );
            xDesktop->addTerminateListener( xListener );
        }
    }

    return *mpGlobalItemPool;
}

void SdrObject::SetRelativeWidth( double nValue )
{
    mpImpl->mnRelativeWidth = nValue;
}

void SdrObject::SetRelativeWidthRelation( sal_Int16 eValue )
{
    mpImpl->meRelativeWidthRelation = eValue;
}

void SdrObject::SetRelativeHeight( double nValue )
{
    mpImpl->mnRelativeHeight = nValue;
}

void SdrObject::SetRelativeHeightRelation( sal_Int16 eValue )
{
    mpImpl->meRelativeHeightRelation = eValue;
}

const double* SdrObject::GetRelativeWidth( ) const
{
    if (!mpImpl->mnRelativeWidth)
        return nullptr;

    return &*mpImpl->mnRelativeWidth;
}

sal_Int16 SdrObject::GetRelativeWidthRelation() const
{
    return mpImpl->meRelativeWidthRelation;
}

const double* SdrObject::GetRelativeHeight( ) const
{
    if (!mpImpl->mnRelativeHeight)
        return nullptr;

    return &*mpImpl->mnRelativeHeight;
}

sal_Int16 SdrObject::GetRelativeHeightRelation() const
{
    return mpImpl->meRelativeHeightRelation;
}

void SdrObject::SetDiagramData(std::shared_ptr<DiagramDataInterface> pDiagramData)
{
    mpImpl->mpDiagramData = pDiagramData;
}

const std::shared_ptr<DiagramDataInterface> & SdrObject::GetDiagramData() const
{
    return mpImpl->mpDiagramData;
}

SfxItemPool& SdrObject::GetObjectItemPool() const
{
    return getSdrModelFromSdrObject().GetItemPool();
}

SdrInventor SdrObject::GetObjInventor()   const
{
    return SdrInventor::Default;
}

SdrObjKind SdrObject::GetObjIdentifier() const
{
    return OBJ_NONE;
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
    if (m_pPlusData->pBroadcast==nullptr) m_pPlusData->pBroadcast.reset(new SfxBroadcaster);

    // SdrEdgeObj may be connected to same SdrObject on both ends so allow it
    // to listen twice
    SdrEdgeObj const*const pEdge(dynamic_cast<SdrEdgeObj const*>(&rListener));
    rListener.StartListening(*m_pPlusData->pBroadcast, pEdge ? DuplicateHandling::Allow : DuplicateHandling::Unexpected);
}

void SdrObject::RemoveListener(SfxListener& rListener)
{
    if (m_pPlusData!=nullptr && m_pPlusData->pBroadcast!=nullptr) {
        rListener.EndListening(*m_pPlusData->pBroadcast);
        if (!m_pPlusData->pBroadcast->HasListeners()) {
            m_pPlusData->pBroadcast.reset();
        }
    }
}

const SfxBroadcaster* SdrObject::GetBroadcaster() const
{
    return m_pPlusData!=nullptr ? m_pPlusData->pBroadcast.get() : nullptr;
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

void SdrObject::SetName(const OUString& rStr, const bool bSetChanged)
{
    if (!rStr.isEmpty() && !m_pPlusData)
    {
        ImpForcePlusData();
    }

    if(!(m_pPlusData && m_pPlusData->aObjName != rStr))
        return;

    // Undo/Redo for setting object's name (#i73249#)
    bool bUndo( false );
    if ( getSdrModelFromSdrObject().IsUndoEnabled() )
    {
        bUndo = true;
        std::unique_ptr<SdrUndoAction> pUndoAction =
                SdrUndoFactory::CreateUndoObjectStrAttr(
                                                *this,
                                                SdrUndoObjStrAttr::ObjStrAttrType::Name,
                                                GetName(),
                                                rStr );
        getSdrModelFromSdrObject().BegUndo( pUndoAction->GetComment() );
        getSdrModelFromSdrObject().AddUndo( std::move(pUndoAction) );
    }
    m_pPlusData->aObjName = rStr;
    // Undo/Redo for setting object's name (#i73249#)
    if ( bUndo )
    {
        getSdrModelFromSdrObject().EndUndo();
    }
    if (bSetChanged)
    {
        SetChanged();
        BroadcastObjectChange();
    }
}

OUString SdrObject::GetName() const
{
    if(m_pPlusData)
    {
        return m_pPlusData->aObjName;
    }

    return OUString();
}

void SdrObject::SetTitle(const OUString& rStr)
{
    if (!rStr.isEmpty() && !m_pPlusData)
    {
        ImpForcePlusData();
    }

    if(!(m_pPlusData && m_pPlusData->aObjTitle != rStr))
        return;

    // Undo/Redo for setting object's title (#i73249#)
    bool bUndo( false );
    if ( getSdrModelFromSdrObject().IsUndoEnabled() )
    {
        bUndo = true;
        std::unique_ptr<SdrUndoAction> pUndoAction =
                SdrUndoFactory::CreateUndoObjectStrAttr(
                                                *this,
                                                SdrUndoObjStrAttr::ObjStrAttrType::Title,
                                                GetTitle(),
                                                rStr );
        getSdrModelFromSdrObject().BegUndo( pUndoAction->GetComment() );
        getSdrModelFromSdrObject().AddUndo( std::move(pUndoAction) );
    }
    m_pPlusData->aObjTitle = rStr;
    // Undo/Redo for setting object's title (#i73249#)
    if ( bUndo )
    {
        getSdrModelFromSdrObject().EndUndo();
    }
    SetChanged();
    BroadcastObjectChange();
}

OUString SdrObject::GetTitle() const
{
    if(m_pPlusData)
    {
        return m_pPlusData->aObjTitle;
    }

    return OUString();
}

void SdrObject::SetDescription(const OUString& rStr)
{
    if (!rStr.isEmpty() && !m_pPlusData)
    {
        ImpForcePlusData();
    }

    if(!(m_pPlusData && m_pPlusData->aObjDescription != rStr))
        return;

    // Undo/Redo for setting object's description (#i73249#)
    bool bUndo( false );
    if ( getSdrModelFromSdrObject().IsUndoEnabled() )
    {
        bUndo = true;
        std::unique_ptr<SdrUndoAction> pUndoAction =
                SdrUndoFactory::CreateUndoObjectStrAttr(
                                                *this,
                                                SdrUndoObjStrAttr::ObjStrAttrType::Description,
                                                GetDescription(),
                                                rStr );
        getSdrModelFromSdrObject().BegUndo( pUndoAction->GetComment() );
        getSdrModelFromSdrObject().AddUndo( std::move(pUndoAction) );
    }
    m_pPlusData->aObjDescription = rStr;
    // Undo/Redo for setting object's description (#i73249#)
    if ( bUndo )
    {
        getSdrModelFromSdrObject().EndUndo();
    }
    SetChanged();
    BroadcastObjectChange();
}

OUString SdrObject::GetDescription() const
{
    if(m_pPlusData)
    {
        return m_pPlusData->aObjDescription;
    }

    return OUString();
}

sal_uInt32 SdrObject::GetOrdNum() const
{
    if (SdrObjList* pParentList = getParentSdrObjListFromSdrObject())
    {
        if (pParentList->IsObjOrdNumsDirty())
        {
            pParentList->RecalcObjOrdNums();
        }
    } else const_cast<SdrObject*>(this)->m_nOrdNum=0;
    return m_nOrdNum;
}

void SdrObject::SetOrdNum(sal_uInt32 nNum)
{
    m_nOrdNum = nNum;
}

void SdrObject::GetGrabBagItem(css::uno::Any& rVal) const
{
    if (m_pGrabBagItem != nullptr)
        m_pGrabBagItem->QueryValue(rVal);
    else
        rVal <<= uno::Sequence<beans::PropertyValue>();
}

void SdrObject::SetGrabBagItem(const css::uno::Any& rVal)
{
    if (m_pGrabBagItem == nullptr)
        m_pGrabBagItem.reset(new SfxGrabBagItem);

    m_pGrabBagItem->PutValue(rVal, 0);

    SetChanged();
    BroadcastObjectChange();
}

sal_uInt32 SdrObject::GetNavigationPosition() const
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
    if(m_aOutRect.IsEmpty())
    {
        const_cast< SdrObject* >(this)->RecalcBoundRect();
    }

    return m_aOutRect;
}

// To have a possibility to get the last calculated BoundRect e.g for producing
// the first rectangle for repaints (old and new need to be used) without forcing
// a RecalcBoundRect (which may be problematical and expensive sometimes) I add here
// a new method for accessing the last BoundRect.
const tools::Rectangle& SdrObject::GetLastBoundRect() const
{
    return m_aOutRect;
}

void SdrObject::RecalcBoundRect()
{
    // #i101680# suppress BoundRect calculations on import(s)
    if ((getSdrModelFromSdrObject().isLocked()) || utl::ConfigManager::IsFuzzing())
        return;

    // central new method which will calculate the BoundRect using primitive geometry
    if(!m_aOutRect.IsEmpty())
        return;

    // Use view-independent data - we do not want any connections
    // to e.g. GridOffset in SdrObject-level
    drawinglayer::primitive2d::Primitive2DContainer xPrimitives;
    GetViewContact().getViewIndependentPrimitive2DContainer(xPrimitives);

    if(xPrimitives.empty())
        return;

    // use neutral ViewInformation and get the range of the primitives
    const drawinglayer::geometry::ViewInformation2D aViewInformation2D;
    const basegfx::B2DRange aRange(xPrimitives.getB2DRange(aViewInformation2D));

    if(!aRange.isEmpty())
    {
        m_aOutRect = tools::Rectangle(
            static_cast<tools::Long>(floor(aRange.getMinX())),
            static_cast<tools::Long>(floor(aRange.getMinY())),
            static_cast<tools::Long>(ceil(aRange.getMaxX())),
            static_cast<tools::Long>(ceil(aRange.getMaxY())));
        return;
    }
}

void SdrObject::BroadcastObjectChange() const
{
    if ((getSdrModelFromSdrObject().isLocked()) || utl::ConfigManager::IsFuzzing())
        return;

    bool bPlusDataBroadcast(m_pPlusData && m_pPlusData->pBroadcast);
    bool bObjectChange(IsInserted());

    if(!(bPlusDataBroadcast || bObjectChange))
        return;

    SdrHint aHint(SdrHintKind::ObjectChange, *this);

    if(bPlusDataBroadcast)
    {
        m_pPlusData->pBroadcast->Broadcast(aHint);
    }

    if(bObjectChange)
    {
        getSdrModelFromSdrObject().Broadcast(aHint);
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
    // TTTT tdf#120066 Indeed - This triggers e.g. by CustomShape
    // geometry-presenting SdrObjects that are in a SdrObjGroup,
    // but the SdrObjGroup is *by purpose* not inserted.
    // Need to check deeper and maybe identify all ::IsInserted()
    // calls by rename and let the compiler work...
    if(nullptr != getSdrPageFromSdrObject())
    {
        getSdrModelFromSdrObject().SetChanged();
    }
}

// tooling for painting a single object to an OutputDevice.
void SdrObject::SingleObjectPainter(OutputDevice& rOut) const
{
    sdr::contact::SdrObjectVector aObjectVector;
    aObjectVector.push_back(const_cast< SdrObject* >(this));

    sdr::contact::ObjectContactOfObjListPainter aPainter(rOut, std::move(aObjectVector), getSdrPageFromSdrObject());
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
    return new SdrObject(rTargetModel, *this);
}

OUString SdrObject::TakeObjNameSingul() const
{
    OUString sName(SvxResId(STR_ObjNameSingulNONE));

    OUString aName(GetName());
    if (!aName.isEmpty())
        sName += " '" + aName + "'";
    return sName;
}

OUString SdrObject::TakeObjNamePlural() const
{
    return SvxResId(STR_ObjNamePluralNONE);
}

OUString SdrObject::ImpGetDescriptionStr(TranslateId pStrCacheID) const
{
    OUString aStr = SvxResId(pStrCacheID);
    sal_Int32 nPos = aStr.indexOf("%1");
    if (nPos >= 0)
    {
        // Replace '%1' with the object name.
        OUString aObjName(TakeObjNameSingul());
        aStr = aStr.replaceAt(nPos, 2, aObjName);
    }

    nPos = aStr.indexOf("%2");
    if (nPos >= 0)
        // Replace '%2' with the passed value.
        aStr = aStr.replaceAt(nPos, 2, u"0");
    return aStr;
}

void SdrObject::ImpForcePlusData()
{
    if (!m_pPlusData)
        m_pPlusData.reset( new SdrObjPlusData );
}

OUString SdrObject::GetMetrStr(tools::Long nVal) const
{
    return getSdrModelFromSdrObject().GetMetricString(nVal);
}

basegfx::B2DPolyPolygon SdrObject::TakeXorPoly() const
{
    basegfx::B2DPolyPolygon aRetval;
    const tools::Rectangle aR(GetCurrentBoundRect());
    aRetval.append(basegfx::utils::createPolygonFromRect(vcl::unotools::b2DRectangleFromRectangle(aR)));

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
            pClone->SetOutlinerParaObject(std::nullopt);
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
        drawinglayer::primitive2d::Primitive2DContainer xSequence;
        rVC.getViewIndependentPrimitive2DContainer(xSequence);

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
    return 8;
}

void SdrObject::AddToHdlList(SdrHdlList& rHdlList) const
{
    const tools::Rectangle& rR=GetSnapRect();
    for (sal_uInt32 nHdlNum=0; nHdlNum<8; ++nHdlNum)
    {
        std::unique_ptr<SdrHdl> pH;
        switch (nHdlNum) {
            case 0: pH.reset(new SdrHdl(rR.TopLeft(),     SdrHdlKind::UpperLeft)); break;
            case 1: pH.reset(new SdrHdl(rR.TopCenter(),   SdrHdlKind::Upper)); break;
            case 2: pH.reset(new SdrHdl(rR.TopRight(),    SdrHdlKind::UpperRight)); break;
            case 3: pH.reset(new SdrHdl(rR.LeftCenter(),  SdrHdlKind::Left )); break;
            case 4: pH.reset(new SdrHdl(rR.RightCenter(), SdrHdlKind::Right)); break;
            case 5: pH.reset(new SdrHdl(rR.BottomLeft(),  SdrHdlKind::LowerLeft)); break;
            case 6: pH.reset(new SdrHdl(rR.BottomCenter(),SdrHdlKind::Lower)); break;
            case 7: pH.reset(new SdrHdl(rR.BottomRight(), SdrHdlKind::LowerRight)); break;
        }
        rHdlList.AddHdl(std::move(pH));
    }
}

void SdrObject::AddToPlusHdlList(SdrHdlList&, SdrHdl&) const
{
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
        tools::Long nWdt0=aRect.Right() -aRect.Left();
        tools::Long nHgt0=aRect.Bottom()-aRect.Top();
        tools::Long nXMul=aTmpRect.Right() -aTmpRect.Left();
        tools::Long nYMul=aTmpRect.Bottom()-aTmpRect.Top();
        tools::Long nXDiv=nWdt0;
        tools::Long nYDiv=nHgt0;
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
                tools::Long nNeed=tools::Long(BigInt(nHgt0)*BigInt(nXMul)/BigInt(nXDiv));
                if (bYNeg) nNeed=-nNeed;
                if (bTop) aTmpRect.SetTop(aTmpRect.Bottom()-nNeed );
                if (bBtm) aTmpRect.SetBottom(aTmpRect.Top()+nNeed );
            } else {
                tools::Long nNeed=tools::Long(BigInt(nWdt0)*BigInt(nYMul)/BigInt(nYDiv));
                if (bXNeg) nNeed=-nNeed;
                if (bLft) aTmpRect.SetLeft(aTmpRect.Right()-nNeed );
                if (bRgt) aTmpRect.SetRight(aTmpRect.Left()+nNeed );
            }
        } else { // apex handles
            if ((bLft || bRgt) && nXDiv!=0) {
                tools::Long nHgt0b=aRect.Bottom()-aRect.Top();
                tools::Long nNeed=tools::Long(BigInt(nHgt0b)*BigInt(nXMul)/BigInt(nXDiv));
                aTmpRect.AdjustTop( -((nNeed-nHgt0b)/2) );
                aTmpRect.SetBottom(aTmpRect.Top()+nNeed );
            }
            if ((bTop || bBtm) && nYDiv!=0) {
                tools::Long nWdt0b=aRect.Right()-aRect.Left();
                tools::Long nNeed=tools::Long(BigInt(nWdt0b)*BigInt(nYMul)/BigInt(nYDiv));
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

SdrObjectUniquePtr SdrObject::getFullDragClone() const
{
    // default uses simple clone
    return SdrObjectUniquePtr(CloneSdrObject(getSdrModelFromSdrObject()));
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
    m_aOutRect = aRect1;
    return true;
}

bool SdrObject::MovCreate(SdrDragStat& rStat)
{
    rStat.TakeCreateRect(m_aOutRect);
    rStat.SetActionRect(m_aOutRect);
    m_aOutRect.Justify();

    return true;
}

bool SdrObject::EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd)
{
    rStat.TakeCreateRect(m_aOutRect);
    m_aOutRect.Justify();

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
    aRetval.append(basegfx::utils::createPolygonFromRect(vcl::unotools::b2DRectangleFromRectangle(aRect1)));
    return aRetval;
}

PointerStyle SdrObject::GetCreatePointer() const
{
    return PointerStyle::Cross;
}

// transformations
void SdrObject::NbcMove(const Size& rSiz)
{
    m_aOutRect.Move(rSiz);
    SetBoundAndSnapRectsDirty();
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
    ResizeRect(m_aOutRect,rRef,xFact,yFact);
    SetBoundAndSnapRectsDirty();
}

void SdrObject::NbcRotate(const Point& rRef, Degree100 nAngle)
{
    if (nAngle)
    {
        double a = toRadians(nAngle);
        NbcRotate( rRef, nAngle, sin( a ), cos( a ) );
    }
}

void SdrObject::NbcRotate(const Point& rRef,  Degree100 nAngle, double sn, double cs)
{
    SetGlueReallyAbsolute(true);
    m_aOutRect.Move(-rRef.X(),-rRef.Y());
    tools::Rectangle R(m_aOutRect);
    if (sn==1.0 && cs==0.0) { // 90deg
        m_aOutRect.SetLeft(-R.Bottom() );
        m_aOutRect.SetRight(-R.Top() );
        m_aOutRect.SetTop(R.Left() );
        m_aOutRect.SetBottom(R.Right() );
    } else if (sn==0.0 && cs==-1.0) { // 180deg
        m_aOutRect.SetLeft(-R.Right() );
        m_aOutRect.SetRight(-R.Left() );
        m_aOutRect.SetTop(-R.Bottom() );
        m_aOutRect.SetBottom(-R.Top() );
    } else if (sn==-1.0 && cs==0.0) { // 270deg
        m_aOutRect.SetLeft(R.Top() );
        m_aOutRect.SetRight(R.Bottom() );
        m_aOutRect.SetTop(-R.Right() );
        m_aOutRect.SetBottom(-R.Left() );
    }
    m_aOutRect.Move(rRef.X(),rRef.Y());
    m_aOutRect.Justify(); // just in case
    SetBoundAndSnapRectsDirty();
    NbcRotateGluePoints(rRef,nAngle,sn,cs);
    SetGlueReallyAbsolute(false);
}

void SdrObject::NbcMirror(const Point& rRef1, const Point& rRef2)
{
    SetGlueReallyAbsolute(true);
    m_aOutRect.Move(-rRef1.X(),-rRef1.Y());
    tools::Rectangle R(m_aOutRect);
    tools::Long dx=rRef2.X()-rRef1.X();
    tools::Long dy=rRef2.Y()-rRef1.Y();
    if (dx==0) {          // vertical axis
        m_aOutRect.SetLeft(-R.Right() );
        m_aOutRect.SetRight(-R.Left() );
    } else if (dy==0) {   // horizontal axis
        m_aOutRect.SetTop(-R.Bottom() );
        m_aOutRect.SetBottom(-R.Top() );
    } else if (dx==dy) {  // 45deg axis
        m_aOutRect.SetLeft(R.Top() );
        m_aOutRect.SetRight(R.Bottom() );
        m_aOutRect.SetTop(R.Left() );
        m_aOutRect.SetBottom(R.Right() );
    } else if (dx==-dy) { // 45deg axis
        m_aOutRect.SetLeft(-R.Bottom() );
        m_aOutRect.SetRight(-R.Top() );
        m_aOutRect.SetTop(-R.Right() );
        m_aOutRect.SetBottom(-R.Left() );
    }
    m_aOutRect.Move(rRef1.X(),rRef1.Y());
    m_aOutRect.Justify(); // just in case
    SetBoundAndSnapRectsDirty();
    NbcMirrorGluePoints(rRef1,rRef2);
    SetGlueReallyAbsolute(false);
}

void SdrObject::NbcShear(const Point& rRef, Degree100 /*nAngle*/, double tn, bool bVShear)
{
    SetGlueReallyAbsolute(true);
    NbcShearGluePoints(rRef,tn,bVShear);
    SetGlueReallyAbsolute(false);
}

void SdrObject::Move(const Size& rSiz)
{
    if (rSiz.Width()!=0 || rSiz.Height()!=0) {
        tools::Rectangle aBoundRect0; if (m_pUserCall!=nullptr) aBoundRect0=GetLastBoundRect();
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
    if (xFact.GetNumerator() == xFact.GetDenominator() && yFact.GetNumerator() == yFact.GetDenominator())
        return;

    if (bUnsetRelative)
    {
        mpImpl->mnRelativeWidth.reset();
        mpImpl->meRelativeWidthRelation = text::RelOrientation::PAGE_FRAME;
        mpImpl->meRelativeHeightRelation = text::RelOrientation::PAGE_FRAME;
        mpImpl->mnRelativeHeight.reset();
    }
    tools::Rectangle aBoundRect0; if (m_pUserCall!=nullptr) aBoundRect0=GetLastBoundRect();
    NbcResize(rRef,xFact,yFact);
    SetChanged();
    BroadcastObjectChange();
    SendUserCall(SdrUserCallType::Resize,aBoundRect0);
}

void SdrObject::Crop(const basegfx::B2DPoint& rRef, double fxFact, double fyFact)
{
    tools::Rectangle aBoundRect0; if (m_pUserCall!=nullptr) aBoundRect0=GetLastBoundRect();
    NbcCrop(rRef, fxFact, fyFact);
    SetChanged();
    BroadcastObjectChange();
    SendUserCall(SdrUserCallType::Resize,aBoundRect0);
}

void SdrObject::Rotate(const Point& rRef, Degree100 nAngle, double sn, double cs)
{
    if (nAngle) {
        tools::Rectangle aBoundRect0; if (m_pUserCall!=nullptr) aBoundRect0=GetLastBoundRect();
        NbcRotate(rRef,nAngle,sn,cs);
        SetChanged();
        BroadcastObjectChange();
        SendUserCall(SdrUserCallType::Resize,aBoundRect0);
    }
}

void SdrObject::Mirror(const Point& rRef1, const Point& rRef2)
{
    tools::Rectangle aBoundRect0; if (m_pUserCall!=nullptr) aBoundRect0=GetLastBoundRect();
    NbcMirror(rRef1,rRef2);
    SetChanged();
    BroadcastObjectChange();
    SendUserCall(SdrUserCallType::Resize,aBoundRect0);
}

void SdrObject::Shear(const Point& rRef, Degree100 nAngle, double tn, bool bVShear)
{
    if (nAngle) {
        tools::Rectangle aBoundRect0; if (m_pUserCall!=nullptr) aBoundRect0=GetLastBoundRect();
        NbcShear(rRef,nAngle,tn,bVShear);
        SetChanged();
        BroadcastObjectChange();
        SendUserCall(SdrUserCallType::Resize,aBoundRect0);
    }
}

void SdrObject::NbcSetRelativePos(const Point& rPnt)
{
    Point aRelPos0(GetSnapRect().TopLeft()-m_aAnchor);
    Size aSiz(rPnt.X()-aRelPos0.X(),rPnt.Y()-aRelPos0.Y());
    NbcMove(aSiz); // This also calls SetRectsDirty()
}

void SdrObject::SetRelativePos(const Point& rPnt)
{
    if (rPnt!=GetRelativePos()) {
        tools::Rectangle aBoundRect0; if (m_pUserCall!=nullptr) aBoundRect0=GetLastBoundRect();
        NbcSetRelativePos(rPnt);
        SetChanged();
        BroadcastObjectChange();
        SendUserCall(SdrUserCallType::MoveOnly,aBoundRect0);
    }
}

Point SdrObject::GetRelativePos() const
{
    return GetSnapRect().TopLeft()-m_aAnchor;
}

void SdrObject::ImpSetAnchorPos(const Point& rPnt)
{
    m_aAnchor = rPnt;
}

void SdrObject::NbcSetAnchorPos(const Point& rPnt)
{
    Size aSiz(rPnt.X()-m_aAnchor.X(),rPnt.Y()-m_aAnchor.Y());
    m_aAnchor=rPnt;
    NbcMove(aSiz); // This also calls SetRectsDirty()
}

void SdrObject::SetAnchorPos(const Point& rPnt)
{
    if (rPnt!=m_aAnchor) {
        tools::Rectangle aBoundRect0; if (m_pUserCall!=nullptr) aBoundRect0=GetLastBoundRect();
        NbcSetAnchorPos(rPnt);
        SetChanged();
        BroadcastObjectChange();
        SendUserCall(SdrUserCallType::MoveOnly,aBoundRect0);
    }
}

const Point& SdrObject::GetAnchorPos() const
{
    return m_aAnchor;
}

void SdrObject::RecalcSnapRect()
{
}

const tools::Rectangle& SdrObject::GetSnapRect() const
{
    return m_aOutRect;
}

void SdrObject::NbcSetSnapRect(const tools::Rectangle& rRect)
{
    m_aOutRect=rRect;
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
    tools::Rectangle aBoundRect0; if (m_pUserCall!=nullptr) aBoundRect0=GetLastBoundRect();
    NbcSetSnapRect(rRect);
    SetChanged();
    BroadcastObjectChange();
    SendUserCall(SdrUserCallType::Resize,aBoundRect0);
}

void SdrObject::SetLogicRect(const tools::Rectangle& rRect)
{
    tools::Rectangle aBoundRect0; if (m_pUserCall!=nullptr) aBoundRect0=GetLastBoundRect();
    NbcSetLogicRect(rRect);
    SetChanged();
    BroadcastObjectChange();
    SendUserCall(SdrUserCallType::Resize,aBoundRect0);
}

Degree100 SdrObject::GetRotateAngle() const
{
    return 0_deg100;
}

Degree100 SdrObject::GetShearAngle(bool /*bVertical*/) const
{
    return 0_deg100;
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
    return 0;
}

Point SdrObject::GetPoint(sal_uInt32 /*i*/) const
{
    return Point();
}

void SdrObject::SetPoint(const Point& rPnt, sal_uInt32 i)
{
    tools::Rectangle aBoundRect0; if (m_pUserCall!=nullptr) aBoundRect0=GetLastBoundRect();
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
    return (m_aAnchor.X() == rOtherObj.m_aAnchor.X() && m_aAnchor.Y() == rOtherObj.m_aAnchor.Y() &&
            m_nOrdNum == rOtherObj.m_nOrdNum && mnNavigationPosition == rOtherObj.mnNavigationPosition &&
            mbSupportTextIndentingOnLineWidthChange == rOtherObj.mbSupportTextIndentingOnLineWidthChange &&
            mbLineIsOutsideGeometry == rOtherObj.mbLineIsOutsideGeometry && m_bMarkProt == rOtherObj.m_bMarkProt &&
            m_bIs3DObj == rOtherObj.m_bIs3DObj && m_bIsEdge == rOtherObj.m_bIsEdge && m_bClosedObj == rOtherObj.m_bClosedObj &&
            m_bNotVisibleAsMaster == rOtherObj.m_bNotVisibleAsMaster && m_bEmptyPresObj == rOtherObj.m_bEmptyPresObj &&
            mbVisible == rOtherObj.mbVisible && m_bNoPrint == rOtherObj.m_bNoPrint && m_bSizProt == rOtherObj.m_bSizProt &&
            m_bMovProt == rOtherObj.m_bMovProt && m_bVirtObj == rOtherObj.m_bVirtObj &&
            mnLayerID == rOtherObj.mnLayerID && GetMergedItemSet().Equals(rOtherObj.GetMergedItemSet(), false) );
}

void SdrObject::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SdrObject"));
    (void)xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("ptr"), "%p", this);
    (void)xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("symbol"), "%s", BAD_CAST(typeid(*this).name()));
    (void)xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("name"), "%s", BAD_CAST(GetName().toUtf8().getStr()));
    (void)xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("title"), "%s", BAD_CAST(GetTitle().toUtf8().getStr()));
    (void)xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("description"), "%s", BAD_CAST(GetDescription().toUtf8().getStr()));
    (void)xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("nOrdNum"), "%" SAL_PRIuUINT32, GetOrdNumDirect());
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("aOutRect"), BAD_CAST(m_aOutRect.toString().getStr()));

    if (m_pGrabBagItem)
    {
        m_pGrabBagItem->dumpAsXml(pWriter);
    }

    if (mpProperties)
    {
        mpProperties->dumpAsXml(pWriter);
    }

    if (const OutlinerParaObject* pOutliner = GetOutlinerParaObject())
        pOutliner->dumpAsXml(pWriter);

    (void)xmlTextWriterEndElement(pWriter);
}

void SdrObject::SetOutlinerParaObject(std::optional<OutlinerParaObject> pTextObject)
{
    tools::Rectangle aBoundRect0; if (m_pUserCall!=nullptr) aBoundRect0=GetLastBoundRect();
    NbcSetOutlinerParaObject(std::move(pTextObject));
    SetChanged();
    BroadcastObjectChange();
    if (GetCurrentBoundRect()!=aBoundRect0) {
        SendUserCall(SdrUserCallType::Resize,aBoundRect0);
    }

    if (!getSdrModelFromSdrObject().IsUndoEnabled())
        return;

    // Don't do this during import.
    SdrObject* pTopGroupObj = nullptr;
    if (getParentSdrObjectFromSdrObject())
    {
        pTopGroupObj = getParentSdrObjectFromSdrObject();
        while (pTopGroupObj->getParentSdrObjectFromSdrObject())
        {
            pTopGroupObj = pTopGroupObj->getParentSdrObjectFromSdrObject();
        }
    }
    if (pTopGroupObj)
    {
        // A shape was modified, which is in a group shape. Empty the group shape's grab-bag,
        // which potentially contains the old text of the shapes in case of diagrams.
        pTopGroupObj->SetGrabBagItem(uno::makeAny(uno::Sequence<beans::PropertyValue>()));
    }
}

void SdrObject::NbcSetOutlinerParaObject(std::optional<OutlinerParaObject> /*pTextObject*/)
{
}

OutlinerParaObject* SdrObject::GetOutlinerParaObject() const
{
    return nullptr;
}

void SdrObject::NbcReformatText()
{
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

PointerStyle SdrObject::GetMacroPointer(const SdrObjMacroHitRec&) const
{
    return PointerStyle::RefHand;
}

void SdrObject::PaintMacro(OutputDevice& rOut, const tools::Rectangle& , const SdrObjMacroHitRec& ) const
{
    const RasterOp eRop(rOut.GetRasterOp());
    const basegfx::B2DPolyPolygon aPolyPolygon(TakeXorPoly());

    rOut.SetLineColor(COL_BLACK);
    rOut.SetFillColor();
    rOut.SetRasterOp(RasterOp::Invert);

    for(auto const& rPolygon : aPolyPolygon)
    {
        rOut.DrawPolyLine(rPolygon);
    }

    rOut.SetRasterOp(eRop);
}

bool SdrObject::DoMacro(const SdrObjMacroHitRec&)
{
    return false;
}

bool SdrObject::IsMacroHit(const SdrObjMacroHitRec& rRec) const
{
    return CheckMacroHit(rRec) != nullptr;
}


std::unique_ptr<SdrObjGeoData> SdrObject::NewGeoData() const
{
    return std::make_unique<SdrObjGeoData>();
}

void SdrObject::SaveGeoData(SdrObjGeoData& rGeo) const
{
    rGeo.aBoundRect    =GetCurrentBoundRect();
    rGeo.aAnchor       =m_aAnchor       ;
    rGeo.bMovProt      =m_bMovProt      ;
    rGeo.bSizProt      =m_bSizProt      ;
    rGeo.bNoPrint      =m_bNoPrint      ;
    rGeo.mbVisible     =mbVisible     ;
    rGeo.bClosedObj    =m_bClosedObj    ;
    rGeo.mnLayerID = mnLayerID;

    // user-defined gluepoints
    if (m_pPlusData!=nullptr && m_pPlusData->pGluePoints!=nullptr) {
        if (rGeo.pGPL!=nullptr) {
            *rGeo.pGPL=*m_pPlusData->pGluePoints;
        } else {
            rGeo.pGPL.reset( new SdrGluePointList(*m_pPlusData->pGluePoints) );
        }
    } else {
        rGeo.pGPL.reset();
    }
}

void SdrObject::RestoreGeoData(const SdrObjGeoData& rGeo)
{
    SetBoundAndSnapRectsDirty();
    m_aOutRect      =rGeo.aBoundRect    ;
    m_aAnchor       =rGeo.aAnchor       ;
    m_bMovProt      =rGeo.bMovProt      ;
    m_bSizProt      =rGeo.bSizProt      ;
    m_bNoPrint      =rGeo.bNoPrint      ;
    mbVisible     =rGeo.mbVisible     ;
    m_bClosedObj    =rGeo.bClosedObj    ;
    mnLayerID = rGeo.mnLayerID;

    // user-defined gluepoints
    if (rGeo.pGPL!=nullptr) {
        ImpForcePlusData();
        if (m_pPlusData->pGluePoints!=nullptr) {
            *m_pPlusData->pGluePoints=*rGeo.pGPL;
        } else {
            m_pPlusData->pGluePoints.reset(new SdrGluePointList(*rGeo.pGPL));
        }
    } else {
        if (m_pPlusData!=nullptr && m_pPlusData->pGluePoints!=nullptr) {
            m_pPlusData->pGluePoints.reset();
        }
    }
}

std::unique_ptr<SdrObjGeoData> SdrObject::GetGeoData() const
{
    std::unique_ptr<SdrObjGeoData> pGeo = NewGeoData();
    SaveGeoData(*pGeo);
    return pGeo;
}

void SdrObject::SetGeoData(const SdrObjGeoData& rGeo)
{
    tools::Rectangle aBoundRect0; if (m_pUserCall!=nullptr) aBoundRect0=GetLastBoundRect();
    RestoreGeoData(rGeo);
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
    tools::Rectangle aBoundRect0; if (m_pUserCall!=nullptr) aBoundRect0=GetLastBoundRect();
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
        tools::Long n=static_cast<const SdrMoveXItem*>(pPoolItem)->GetValue();
        aNewSnap.Move(n,0);
    }
    if (rAttr.GetItemState(SDRATTR_MOVEY,true,&pPoolItem)==SfxItemState::SET) {
        tools::Long n=static_cast<const SdrMoveYItem*>(pPoolItem)->GetValue();
        aNewSnap.Move(0,n);
    }
    if (rAttr.GetItemState(SDRATTR_ONEPOSITIONX,true,&pPoolItem)==SfxItemState::SET) {
        tools::Long n=static_cast<const SdrOnePositionXItem*>(pPoolItem)->GetValue();
        aNewSnap.Move(n-aNewSnap.Left(),0);
    }
    if (rAttr.GetItemState(SDRATTR_ONEPOSITIONY,true,&pPoolItem)==SfxItemState::SET) {
        tools::Long n=static_cast<const SdrOnePositionYItem*>(pPoolItem)->GetValue();
        aNewSnap.Move(0,n-aNewSnap.Top());
    }
    if (rAttr.GetItemState(SDRATTR_ONESIZEWIDTH,true,&pPoolItem)==SfxItemState::SET) {
        tools::Long n=static_cast<const SdrOneSizeWidthItem*>(pPoolItem)->GetValue();
        aNewSnap.SetRight(aNewSnap.Left()+n );
    }
    if (rAttr.GetItemState(SDRATTR_ONESIZEHEIGHT,true,&pPoolItem)==SfxItemState::SET) {
        tools::Long n=static_cast<const SdrOneSizeHeightItem*>(pPoolItem)->GetValue();
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
        Degree100 n=static_cast<const SdrShearAngleItem*>(pPoolItem)->GetValue();
        n-=GetShearAngle();
        if (n) {
            double nTan = tan(toRadians(n));
            NbcShear(aRef1,n,nTan,false);
        }
    }
    if (rAttr.GetItemState(SDRATTR_ROTATEANGLE,true,&pPoolItem)==SfxItemState::SET) {
        Degree100 n=static_cast<const SdrAngleItem*>(pPoolItem)->GetValue();
        n-=GetRotateAngle();
        if (n) {
            NbcRotate(aRef1,n);
        }
    }
    if (rAttr.GetItemState(SDRATTR_ROTATEONE,true,&pPoolItem)==SfxItemState::SET) {
        Degree100 n=static_cast<const SdrRotateOneItem*>(pPoolItem)->GetValue();
        NbcRotate(aRef1,n);
    }
    if (rAttr.GetItemState(SDRATTR_HORZSHEARONE,true,&pPoolItem)==SfxItemState::SET) {
        Degree100 n=static_cast<const SdrHorzShearOneItem*>(pPoolItem)->GetValue();
        double nTan = tan(toRadians(n));
        NbcShear(aRef1,n,nTan,false);
    }
    if (rAttr.GetItemState(SDRATTR_VERTSHEARONE,true,&pPoolItem)==SfxItemState::SET) {
        Degree100 n=static_cast<const SdrVertShearOneItem*>(pPoolItem)->GetValue();
        double nTan = tan(toRadians(n));
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
        tools::Long n=static_cast<const SdrLogicSizeWidthItem*>(pPoolItem)->GetValue();
        aNewLogic.SetRight(aNewLogic.Left()+n );
    }
    if (rAttr.GetItemState(SDRATTR_LOGICSIZEHEIGHT,true,&pPoolItem)==SfxItemState::SET) {
        tools::Long n=static_cast<const SdrLogicSizeHeightItem*>(pPoolItem)->GetValue();
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

    if(m_pUserCall)
        aBoundRect0 = GetLastBoundRect();

    NbcSetStyleSheet(pNewStyleSheet, bDontRemoveHardAttr);
    SetChanged();
    BroadcastObjectChange();
    SendUserCall(SdrUserCallType::ChangeAttr, aBoundRect0);
}

void SdrObject::NbcSetStyleSheet(SfxStyleSheet* pNewStyleSheet, bool bDontRemoveHardAttr)
{
    GetProperties().SetStyleSheet(pNewStyleSheet, bDontRemoveHardAttr);
}

// Broadcasting while setting attributes is managed by the AttrObj.


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
    if (m_pPlusData!=nullptr) return m_pPlusData->pGluePoints.get();
    return nullptr;
}


SdrGluePointList* SdrObject::ForceGluePointList()
{
    ImpForcePlusData();
    if (m_pPlusData->pGluePoints==nullptr) {
        m_pPlusData->pGluePoints.reset(new SdrGluePointList);
    }
    return m_pPlusData->pGluePoints.get();
}

void SdrObject::SetGlueReallyAbsolute(bool bOn)
{
    // First a const call to see whether there are any gluepoints.
    // Force const call!
    if (GetGluePointList()!=nullptr) {
        SdrGluePointList* pGPL=ForceGluePointList();
        pGPL->SetReallyAbsolute(bOn,*this);
    }
}

void SdrObject::NbcRotateGluePoints(const Point& rRef, Degree100 nAngle, double sn, double cs)
{
    // First a const call to see whether there are any gluepoints.
    // Force const call!
    if (GetGluePointList()!=nullptr) {
        SdrGluePointList* pGPL=ForceGluePointList();
        pGPL->Rotate(rRef,nAngle,sn,cs,this);
    }
}

void SdrObject::NbcMirrorGluePoints(const Point& rRef1, const Point& rRef2)
{
    // First a const call to see whether there are any gluepoints.
    // Force const call!
    if (GetGluePointList()!=nullptr) {
        SdrGluePointList* pGPL=ForceGluePointList();
        pGPL->Mirror(rRef1,rRef2,this);
    }
}

void SdrObject::NbcShearGluePoints(const Point& rRef, double tn, bool bVShear)
{
    // First a const call to see whether there are any gluepoints.
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


static void extractLineContourFromPrimitive2DSequence(
    const drawinglayer::primitive2d::Primitive2DContainer& rxSequence,
    basegfx::B2DPolygonVector& rExtractedHairlines,
    basegfx::B2DPolyPolygonVector& rExtractedLineFills)
{
    rExtractedHairlines.clear();
    rExtractedLineFills.clear();

    if(rxSequence.empty())
        return;

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


SdrObject* SdrObject::ImpConvertToContourObj(bool bForceLineDash)
{
    SdrObject* pRetval(nullptr);

    if(LineGeometryUsageIsNecessary())
    {
        basegfx::B2DPolyPolygon aMergedLineFillPolyPolygon;
        basegfx::B2DPolyPolygon aMergedHairlinePolyPolygon;
        drawinglayer::primitive2d::Primitive2DContainer xSequence;
        GetViewContact().getViewIndependentPrimitive2DContainer(xSequence);

        if(!xSequence.empty())
        {
            basegfx::B2DPolygonVector aExtractedHairlines;
            basegfx::B2DPolyPolygonVector aExtractedLineFills;

            extractLineContourFromPrimitive2DSequence(xSequence, aExtractedHairlines, aExtractedLineFills);

            // for SdrObject creation, just copy all to a single Hairline-PolyPolygon
            for(const basegfx::B2DPolygon & rExtractedHairline : aExtractedHairlines)
            {
                aMergedHairlinePolyPolygon.append(rExtractedHairline);
            }

            // check for fill rsults
            if (!aExtractedLineFills.empty() && !utl::ConfigManager::IsFuzzing())
            {
                // merge to a single tools::PolyPolygon (OR)
                aMergedLineFillPolyPolygon = basegfx::utils::mergeToSinglePolyPolygon(std::move(aExtractedLineFills));
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
    m_bMarkProt = bProt;
}


void SdrObject::SetEmptyPresObj(bool bEpt)
{
    m_bEmptyPresObj = bEpt;
}


void SdrObject::SetNotVisibleAsMaster(bool bFlg)
{
    m_bNotVisibleAsMaster=bFlg;
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


SdrObjectUniquePtr SdrObject::ConvertToPolyObj(bool bBezier, bool bLineToArea) const
{
    SdrObjectUniquePtr pRet = DoConvertToPolyObj(bBezier, true);

    if(pRet && bLineToArea)
    {
        SdrObject* pNewRet = ConvertToContourObj(pRet.get());
        pRet.reset(pNewRet);
    }

    // #i73441# preserve LayerID
    if(pRet && pRet->GetLayer() != GetLayer())
    {
        pRet->SetLayer(GetLayer());
    }

    return pRet;
}


SdrObjectUniquePtr SdrObject::DoConvertToPolyObj(bool /*bBezier*/, bool /*bAddText*/) const
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

    if(nullptr != m_pPlusData && nullptr != m_pPlusData->pBroadcast)
    {
        SdrHint aHint(bIsInserted ? SdrHintKind::ObjectInserted : SdrHintKind::ObjectRemoved, *this);
        m_pPlusData->pBroadcast->Broadcast(aHint);
    }
}

void SdrObject::SetMoveProtect(bool bProt)
{
    if(IsMoveProtect() != bProt)
    {
        // #i77187# secured and simplified
        m_bMovProt = bProt;
        SetChanged();
        BroadcastObjectChange();
    }
}

void SdrObject::SetResizeProtect(bool bProt)
{
    if(IsResizeProtect() != bProt)
    {
        // #i77187# secured and simplified
        m_bSizProt = bProt;
        SetChanged();
        BroadcastObjectChange();
    }
}

void SdrObject::SetPrintable(bool bPrn)
{
    if( bPrn == m_bNoPrint )
    {
        m_bNoPrint=!bPrn;
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
    if (m_pPlusData==nullptr || m_pPlusData->pUserDataList==nullptr) return 0;
    return m_pPlusData->pUserDataList->GetUserDataCount();
}

SdrObjUserData* SdrObject::GetUserData(sal_uInt16 nNum) const
{
    if (m_pPlusData==nullptr || m_pPlusData->pUserDataList==nullptr) return nullptr;
    return &m_pPlusData->pUserDataList->GetUserData(nNum);
}

void SdrObject::AppendUserData(std::unique_ptr<SdrObjUserData> pData)
{
    if (!pData)
    {
        OSL_FAIL("SdrObject::AppendUserData(): pData is NULL pointer.");
        return;
    }

    ImpForcePlusData();
    if (!m_pPlusData->pUserDataList)
        m_pPlusData->pUserDataList.reset( new SdrObjUserDataList );

    m_pPlusData->pUserDataList->AppendUserData(std::move(pData));
}

void SdrObject::DeleteUserData(sal_uInt16 nNum)
{
    sal_uInt16 nCount=GetUserDataCount();
    if (nNum<nCount) {
        m_pPlusData->pUserDataList->DeleteUserData(nNum);
        if (nCount==1)  {
            m_pPlusData->pUserDataList.reset();
        }
    } else {
        OSL_FAIL("SdrObject::DeleteUserData(): Invalid Index.");
    }
}

void SdrObject::SetUserCall(SdrObjUserCall* pUser)
{
    m_pUserCall = pUser;
}


void SdrObject::SendUserCall(SdrUserCallType eUserCall, const tools::Rectangle& rBoundRect) const
{
    SdrObject* pGroup(getParentSdrObjectFromSdrObject());

    if ( m_pUserCall )
    {
        m_pUserCall->Changed( *this, eUserCall, rBoundRect );
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
        [[fallthrough]]; // RESIZE might also imply a change of the position
    case SdrUserCallType::MoveOnly:
        notifyShapePropertyChange( svx::ShapeProperty::Position );
        break;
    default:
        // not interested in
        break;
    }
}

void SdrObject::setUnoShape( const uno::Reference< drawing::XShape >& _rxUnoShape )
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
    mpSvxShape = comphelper::getFromUnoTunnel<SvxShape>( _rxUnoShape );

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

css::uno::Reference< css::drawing::XShape > SdrObject::getUnoShape()
{
    // try weak reference first
    uno::Reference< css::drawing::XShape > xShape( getWeakUnoShape() );
    if( xShape )
        return xShape;

    OSL_ENSURE( mpSvxShape == nullptr, "SdrObject::getUnoShape: XShape already dead, but still an IMPL pointer!" );

    // try to access SdrPage from this SdrObject. This will only exist if the SdrObject is
    // inserted in a SdrObjList (page/group/3dScene)
    SdrPage* pPageCandidate(getSdrPageFromSdrObject());

    // tdf#12152, tdf#120728
    //
    // With the paradigm change to only get a SdrPage for a SdrObject when the SdrObject
    // is *inserted*, the functionality for creating 1:1 associated UNO API implementation
    // SvxShapes was partially broken: The used ::CreateShape relies on the SvxPage being
    // derived and the CreateShape method overloaded, implementing additional SdrInventor
    // types as needed.
    //
    // The fallback to use SvxDrawPage::CreateShapeByTypeAndInventor is a trap: It's only
    // a static fallback that handles the SdrInventor types SdrInventor::E3d and
    // SdrInventor::Default. Due to that, e.g. the ReportDesigner broke in various conditions.
    //
    // That again has to do with the ReportDesigner being implemented using the UNO API
    // aspects of SdrObjects early during their construction, not just after these are
    // inserted to a SdrPage - but that is not illegal or wrong, the SdrObject exists already.
    //
    // As a current solution, use the (now always available) SdrModel and any of the
    // existing SdrPages. The only important thing is to get a SdrPage where ::CreateShape is
    // overloaded and implemented as needed.
    //
    // Note for the future:
    // In a more ideal world there would be only one factory method for creating SdrObjects (not
    // ::CreateShape and ::CreateShapeByTypeAndInventor). This also would not be placed at
    // SdrPage/SvxPage at all, but at the Model where it belongs - where else would you expect
    // objects for the current Model to be constructed? To have this at the Page only would make
    // sense if different shapes would need to be constructed for different Pages in the same Model
    // - this is never the case.
    // At that Model extended functionality for that factory (or overloads and implementations)
    // should be placed. But to be realistic, migrating the factories to Model now is too much
    // work - maybe over time when melting SdrObject/SvxObject one day...
    //
    // More Note (added by noel grandin)
    // Except that sd/ is being naughty and doing all kinds of magic during CreateShape that
    // requires knowing which page the object is being created for. Fixing that would require
    // moving a bunch of nasty logic from object creation time, to the point in time when
    // it is actually added to a page.
    if(nullptr == pPageCandidate)
    {
        // If not inserted, alternatively access a SdrPage using the SdrModel. There is
        // no reason not to create and return a UNO API XShape when the SdrObject is not
        // inserted - it may be in construction. Main paradigm is that it exists.
        if(0 != getSdrModelFromSdrObject().GetPageCount())
        {
            // Take 1st SdrPage. That may be e.g. a special page (in SD), but the
            // to-be-used method ::CreateShape will be correctly overloaded in
            // all cases
            pPageCandidate = getSdrModelFromSdrObject().GetPage(0);
        }
    }

    if(nullptr != pPageCandidate)
    {
        uno::Reference< uno::XInterface > xPage(pPageCandidate->getUnoPage());
        if( xPage.is() )
        {
            SvxDrawPage* pDrawPage = comphelper::getFromUnoTunnel<SvxDrawPage>(xPage);
            if( pDrawPage )
            {
                // create one
                xShape = pDrawPage->CreateShape( this );
                setUnoShape( xShape );
            }
        }
    }
    else
    {
        // Fallback to static base functionality. CAUTION: This will only support
        // the most basic stuff like SdrInventor::E3d and SdrInventor::Default. All
        // the other SdrInventor enum entries are from overloads and are *not accessible*
        // using this fallback (!) - what a bad trap
        rtl::Reference<SvxShape> xNewShape = SvxDrawPage::CreateShapeByTypeAndInventor( GetObjIdentifier(), GetObjInventor(), this );
        mpSvxShape = xNewShape.get();
        maWeakUnoShape = xShape = mpSvxShape;
    }

    return xShape;
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

void SdrObject::MakeNameUnique()
{
    if (GetName().isEmpty())
    {
        if (const E3dScene* pE3dObj = dynamic_cast<const E3dScene*>(this))
        {
            SdrObjList* pObjList = pE3dObj->GetSubList();
            if (pObjList)
            {
                SdrObject* pObj0 = pObjList->GetObj(0);
                if (pObj0)
                    SetName(pObj0->TakeObjNameSingul());
            }
        }
        else
            SetName(TakeObjNameSingul());
    }

    std::unordered_set<OUString> aNameSet;
    MakeNameUnique(aNameSet);
}

void SdrObject::MakeNameUnique(std::unordered_set<OUString>& rNameSet)
{
    if (GetName().isEmpty())
        return;

    if (rNameSet.empty())
    {
        SdrPage* pPage;
        SdrObject* pObj;
        for (sal_uInt16 nPage(0); nPage < mrSdrModelFromSdrObject.GetPageCount(); ++nPage)
        {
            pPage = mrSdrModelFromSdrObject.GetPage(nPage);
            SdrObjListIter aIter(pPage, SdrIterMode::DeepWithGroups);
            while (aIter.IsMore())
            {
                pObj = aIter.Next();
                if (pObj != this)
                    rNameSet.insert(pObj->GetName());
            }
        }
    }

    OUString sName(GetName().trim());
    OUString sRootName(sName);

    if (!sName.isEmpty() && rtl::isAsciiDigit(sName[sName.getLength() - 1]))
    {
        sal_Int32 nPos(sName.getLength() - 1);
        while (nPos > 0 && rtl::isAsciiDigit(sName[--nPos]));
        sRootName = sName.copy(0, nPos + 1).trim();
    }
    else
        sName += " 1";

    for (sal_uInt32 n = 1; rNameSet.find(sName) != rNameSet.end(); n++)
        sName = sRootName + " " + OUString::number(n);
    rNameSet.insert(sName);

    SetName(sName);
}

SdrObject* SdrObjFactory::CreateObjectFromFactory(SdrModel& rSdrModel, SdrInventor nInventor, SdrObjKind nObjIdentifier)
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
    SdrObjKind nIdentifier,
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
                        nIdentifier,
                        *pSnapRect);
                    bSetSnapRect = false;
                }
                else
                {
                    pObj = new SdrRectObj(
                        rSdrModel,
                        nIdentifier);
                }
            }
            break;
            case OBJ_CIRC:
            case OBJ_SECT:
            case OBJ_CARC:
            case OBJ_CCUT:
            {
                SdrCircKind eCircKind = ToSdrCircKind(nIdentifier);
                if(nullptr != pSnapRect)
                {
                    pObj = new SdrCircObj(rSdrModel, eCircKind, *pSnapRect);
                    bSetSnapRect = false;
                }
                else
                {
                    pObj = new SdrCircObj(rSdrModel, eCircKind);
                }
            }
            break;
            case OBJ_NONE       : pObj=new SdrObject(rSdrModel);                   break;
            case OBJ_GRUP       : pObj=new SdrObjGroup(rSdrModel);                 break;
            case OBJ_POLY       : pObj=new SdrPathObj(rSdrModel, OBJ_POLY       ); break;
            case OBJ_PLIN       : pObj=new SdrPathObj(rSdrModel, OBJ_PLIN       ); break;
            case OBJ_PATHLINE   : pObj=new SdrPathObj(rSdrModel, OBJ_PATHLINE   ); break;
            case OBJ_PATHFILL   : pObj=new SdrPathObj(rSdrModel, OBJ_PATHFILL   ); break;
            case OBJ_FREELINE   : pObj=new SdrPathObj(rSdrModel, OBJ_FREELINE   ); break;
            case OBJ_FREEFILL   : pObj=new SdrPathObj(rSdrModel, OBJ_FREEFILL   ); break;
            case OBJ_PATHPOLY   : pObj=new SdrPathObj(rSdrModel, OBJ_POLY       ); break;
            case OBJ_PATHPLIN   : pObj=new SdrPathObj(rSdrModel, OBJ_PLIN       ); break;
            case OBJ_EDGE       : pObj=new SdrEdgeObj(rSdrModel);                  break;
            case OBJ_RECT       : pObj=new SdrRectObj(rSdrModel);                  break;
            case OBJ_GRAF       : pObj=new SdrGrafObj(rSdrModel);                  break;
            case OBJ_OLE2       : pObj=new SdrOle2Obj(rSdrModel);                  break;
            case OBJ_FRAME      : pObj=new SdrOle2Obj(rSdrModel, true);            break;
            case OBJ_CAPTION    : pObj=new SdrCaptionObj(rSdrModel);               break;
            case OBJ_PAGE       : pObj=new SdrPageObj(rSdrModel);                  break;
            case OBJ_UNO        : pObj=new SdrUnoObj(rSdrModel, OUString());       break;
            case OBJ_CUSTOMSHAPE: pObj=new SdrObjCustomShape(rSdrModel);       break;
#if HAVE_FEATURE_AVMEDIA
            case OBJ_MEDIA      : pObj=new SdrMediaObj(rSdrModel);               break;
#endif
            case OBJ_TABLE      : pObj=new sdr::table::SdrTableObj(rSdrModel);   break;
            default: break;
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
