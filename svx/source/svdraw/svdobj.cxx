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
#include <com/sun/star/lang/XComponent.hpp>

#define _USE_MATH_DEFINES
#include <math.h>
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
#include <svx/svdview.hxx>   // fuer Dragging (Ortho abfragen)
#include "svx/svdglob.hxx"   // StringCache
#include <svx/svdstr.hrc>    // Objektname
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
#include <svx/svditer.hxx>
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
#include <svx/fmmodel.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/objface.hxx>
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
#include "svx/svdotable.hxx"
#include "svx/shapepropertynotifier.hxx"
#include <svx/sdrhittesthelper.hxx>
#include <svx/svdundo.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <svx/gluepoint.hxx>
#include <svx/sdrobjectfilter.hxx>
#include <svx/svdlegacy.hxx>
#include <svx/sdr/contact/viewobjectcontact.hxx>
#include <svx/sdrpagewindow.hxx>

using namespace ::com::sun::star;

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrObjectChangeBroadcaster::SdrObjectChangeBroadcaster(
    const SdrObject& rSdrObject,
    SdrHintKind eSdrHintKind)
:   SdrBaseHint(rSdrObject, eSdrHintKind)
{
}

SdrObjectChangeBroadcaster::~SdrObjectChangeBroadcaster()
{
    // no notifications when model is locked
    if(GetSdrHintObject()->getSdrModelFromSdrObject().isLocked() )
    {
        return;
    }

    // UNO shape listener part
    switch(GetSdrHintKind())
    {
        case HINT_OBJCHG_RESIZE:
        {
            GetSdrHintObject()->notifyShapePropertyChange( ::svx::eShapeSize );
            // fall through - RESIZE might also imply a change of the position
        }
        case HINT_OBJCHG_MOVE:
        {
            GetSdrHintObject()->notifyShapePropertyChange( ::svx::eShapePosition );
            break;
        }
        default:
        {
            // not interested in
            break;
        }
    }

    // broadcast to listeners directly listening at SdrObject
    if(GetSdrHintObject()->HasListeners())
    {
        const_cast< SdrObject& >(*GetSdrHintObject()).Broadcast(*this);
    }

    // broadcast to listeners listening at SdrModel
    const bool bObjectChange(GetSdrHintObject()->IsObjectInserted());

    if(bObjectChange)
    {
        GetSdrHintObject()->getSdrModelFromSdrObject().Broadcast(*this);
    }

    // broadcast SdrObjList change up the hierarchy
    SdrObjList* pParent = GetSdrHintObject()->getParentOfSdrObject();

    while(pParent)
    {
        // tell parent about content change
        pParent->handleContentChange(*this);

        // get next upper SdrObject parent, ignore SdrPage
        SdrObject* pParentObject = pParent->getSdrObjectFromSdrObjList();
        pParent = pParentObject ? pParentObject->getParentOfSdrObject() : 0;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrObjUserData::SdrObjUserData(sal_uInt32 nInv, sal_uInt16 nId)
:   mnInventor(nInv),
    mnIdentifier(nId)
{
}

SdrObjUserData::SdrObjUserData(const SdrObjUserData& rData)
:   mnInventor(rData.mnInventor),
    mnIdentifier(rData.mnIdentifier)
{
}

SdrObjUserData::~SdrObjUserData()
{
}

bool SdrObjUserData::HasMacro(const SdrObject* /*pObj*/) const
{
    return false;
}

SdrObject* SdrObjUserData::CheckMacroHit(const SdrObjMacroHitRec& rRec, const SdrObject* pObj) const
{
    if(pObj)
    {
        if(rRec.mpSdrView)
        {
            return SdrObjectPrimitiveHit(*pObj, rRec.maPos, rRec.mfTol, *rRec.mpSdrView, false, 0);
        }
    }

    return 0;
}

Pointer SdrObjUserData::GetMacroPointer(const SdrObjMacroHitRec& /*rRec*/, const SdrObject* /*pObj*/) const
{
    return Pointer(POINTER_REFHAND);
}

void SdrObjUserData::PaintMacro(OutputDevice& rOut, const SdrObjMacroHitRec& /*rRec*/, const SdrObject* pObj) const
{
    if(!pObj)
    {
        return;
    }

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

XubString SdrObjUserData::GetMacroPopupComment(const SdrObjMacroHitRec& /*rRec*/, const SdrObject* /*pObj*/) const
{
    return String();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrObjUserDataList::SdrObjUserDataList()
:   maList()
{
}

SdrObjUserDataList::~SdrObjUserDataList()
{
    Clear();
}

SdrObjUserData* SdrObjUserDataList::GetUserData(sal_uInt32 nNum) const
{
    if(nNum < maList.size())
    {
        return *(maList.begin() + nNum);
    }
    else
    {
        OSL_ENSURE(false, "SdrObjUserDataList::GetUserData access out of range (!)");
        return 0;
    }
}

void SdrObjUserDataList::Clear()
{
    for(SdrObjUserDataContainerType::iterator aCandidate(maList.begin());
        aCandidate != maList.end(); aCandidate++)
    {
        delete *aCandidate;
    }

    maList.clear();
}

void SdrObjUserDataList::InsertUserData(SdrObjUserData* pData, sal_uInt32 nPos)
{
    if(nPos >= maList.size())
    {
        maList.push_back(pData);
    }
    else
    {
        maList.insert(maList.begin() + nPos, pData);
    }
}

SdrObjUserData* SdrObjUserDataList::RemoveUserData(sal_uInt32 nNum)
{
    SdrObjUserData* pRetval = 0;

    if(nNum < maList.size())
    {
        const SdrObjUserDataContainerType::iterator aCandidate(maList.begin() + nNum);

        pRetval = *aCandidate;
        maList.erase(aCandidate);
    }
    else
    {
        OSL_ENSURE(false, "SdrObjUserDataList::RemoveUserData with wrong index (!)");
    }

    return pRetval;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrObjGeoData::SdrObjGeoData()
:   maSdrObjectTransformation(),
    maObjectAnchor(0.0, 0.0),
    mpGPL(0),
    mnLayerID(0),
    mbMoveProtect(false),
    mbSizeProtect(false),
    mbNoPrint(false),
    mbVisible(true)
{
}

SdrObjGeoData::~SdrObjGeoData()
{
    delete mpGPL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrObjPlusData::SdrObjPlusData()
:   mpUserDataList(0),
    mpGluePoints(0),
    maObjName(),
    maObjTitle(),
    maObjDescription(),
    maHTMLName(),
    maBLIPSizeRange(),
    maObjectAnchor(0.0, 0.0)
{
}

SdrObjPlusData::~SdrObjPlusData()
{
    if(mpUserDataList)
    {
        delete mpUserDataList;
    }

    if(mpGluePoints)
    {
        delete mpGluePoints;
    }
}

SdrObjPlusData* SdrObjPlusData::Clone(SdrObject* pObj1) const
{
    SdrObjPlusData* pNeuPlusData = new SdrObjPlusData;

    // copy UserData
    if(mpUserDataList)
    {
        const sal_uInt32 nAnz(mpUserDataList->GetUserDataCount());

        if(nAnz)
        {
            pNeuPlusData->mpUserDataList = new SdrObjUserDataList;

            for(sal_uInt32 i(0); i < nAnz; i++)
            {
                SdrObjUserData* pNeuUserData = mpUserDataList->GetUserData(i)->Clone(pObj1);
                OSL_ENSURE(pNeuUserData, "SdrObjPlusData::Clone(): UserData.Clone() liefert NULL");

                if(pNeuUserData)
                {
                    pNeuPlusData->mpUserDataList->InsertUserData(pNeuUserData);
                }
            }
        }
    }

    // copy GluePoints
    if(mpGluePoints)
    {
        pNeuPlusData->mpGluePoints = new SdrGluePointList(*mpGluePoints);
    }

    // copy object name, title and description
    pNeuPlusData->maObjName = maObjName;
    pNeuPlusData->maObjTitle = maObjTitle;
    pNeuPlusData->maObjDescription = maObjDescription;

    // For HTMLName: Do not clone, leave uninitialized (empty string)

    // copy maBLIPSizeRange if used
    if(!maBLIPSizeRange.isEmpty())
    {
        pNeuPlusData->maBLIPSizeRange = maBLIPSizeRange;
    }

    // copy object anchor (sw only)
    pNeuPlusData->maObjectAnchor = maObjectAnchor;

    return pNeuPlusData;
}

//////////////////////////////////////////////////////////////////////////////

SdrObjTransformInfoRec::SdrObjTransformInfoRec()
:   mbSelectAllowed(true),
    mbMoveAllowed(true),
    mbResizeFreeAllowed(true),
    mbResizePropAllowed(true),
    mbRotateFreeAllowed(true),
    mbRotate90Allowed(true),
    mbMirrorFreeAllowed(true),
    mbMirror45Allowed(true),
    mbMirror90Allowed(true),
    mbTransparenceAllowed(true),
    mbGradientAllowed(true),
    mbShearAllowed(true),
    mbEdgeRadiusAllowed(true),
    mbNoOrthoDesired(true),
    mbNoContortion(true),
    mbCanConvToPath(true),
    mbCanConvToPoly(true),
    mbCanConvToContour(false),
    mbCanConvToPathLineToArea(true),
    mbCanConvToPolyLineToArea(true)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// helper to allow setting the SvxShape (UnoShape) at SdrObject, but only from SvxShape itself

void SVX_DLLPUBLIC SetUnoShapeAtSdrObjectFromSvxShape(SdrObject& rSdrObject, const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxUnoShape)
{
    rSdrObject.impl_setUnoShape(_rxUnoShape);
}

//////////////////////////////////////////////////////////////////////////////

sdr::properties::BaseProperties* SdrObject::CreateObjectSpecificProperties()
{
    return new sdr::properties::EmptyProperties(*this);
}

sdr::properties::BaseProperties& SdrObject::GetProperties() const
{
    if(!mpProperties)
    {
        const_cast< SdrObject* >(this)->mpProperties = const_cast< SdrObject* >(this)->CreateObjectSpecificProperties();
    }

    return *mpProperties;
}

sdr::contact::ViewContact* SdrObject::CreateObjectSpecificViewContact()
{
    return new sdr::contact::ViewContactOfSdrObj(*this);
}

sdr::contact::ViewContact& SdrObject::GetViewContact() const
{
    if(!mpViewContact)
    {
        const_cast< SdrObject* >(this)->mpViewContact = const_cast< SdrObject* >(this)->CreateObjectSpecificViewContact();
    }

    return *mpViewContact;
}

sdr::gluepoint::GluePointProvider* SdrObject::CreateObjectSpecificGluePointProvider()
{
    return new sdr::gluepoint::GluePointProvider();
}

sdr::gluepoint::GluePointProvider& SdrObject::GetGluePointProvider() const
{
    if(!mpGluePointProvider)
    {
        const_cast< SdrObject* >(this)->mpGluePointProvider = const_cast< SdrObject* >(this)->CreateObjectSpecificGluePointProvider();
    }

    return *mpGluePointProvider;
}

void SdrObject::ActionChanged() const
{
    // Do necessary ViewContact actions
    GetViewContact().ActionChanged();
}

SdrPage* SdrObject::getSdrPageFromSdrObject() const
{
    if(getParentOfSdrObject())
    {
        return getParentOfSdrObject()->getSdrPageFromSdrObjList();
    }

    return 0;
}

SdrModel& SdrObject::getSdrModelFromSdrObject() const
{
    return mrSdrModelFromSdrObject;
}

SdrObjList* SdrObject::getChildrenOfSdrObject() const
{
    // default has no children
    return 0;
}

void SdrObject::SetOrdNum(sal_uInt32 nOrdNum)
{
    if(mnOrdNum != nOrdNum)
    {
        mnOrdNum = nOrdNum;
    }
}

void SdrObject::setParentOfSdrObject(SdrObjList* pNew)
{
    if(getParentOfSdrObject() != pNew)
    {
        // remember current page
        SdrPage* pOldPage = getSdrPageFromSdrObject();

        // set new parent
        mpParentOfSdrObject = pNew;

        // get new page
        SdrPage* pNewPage = getSdrPageFromSdrObject();

        // broadcast page change over objects if needed
        if(pOldPage != pNewPage)
        {
            handlePageChange(pOldPage, pNewPage);
        }
    }
}

SdrObject::SdrObject(SdrModel& rSdrModel, const basegfx::B2DHomMatrix& rTransform)
:   SfxListener(),
    SfxBroadcaster(),
    tools::WeakBase< SdrObject >(),
    mrSdrModelFromSdrObject(rSdrModel),
    mpProperties(0),
    mpViewContact(0),
    mpGluePointProvider(0),
    mpSvxShape(0),
    maWeakUnoShape(),
    maSdrObjectTransformation(rTransform),
    mpParentOfSdrObject(),
    mpPlusData(0),
    mnOrdNum(0),
    mnLayerID(0),
    mbMoveProtect(false),
    mbSizeProtect(false),
    mbNoPrint(false),
    mbMarkProtect(false),
    mbVisible(true),
    mbEmptyPresObj(false),
    mbNotVisibleAsMaster(false),
    mbUniversalApplicationFlag01(false),
    mbPasteResize(false)
{
}

SdrObject::~SdrObject()
{
    try
    {
        SvxShape* pSvxShape = getSvxShape();
        if ( pSvxShape )
        {
            OSL_ENSURE(!pSvxShape->HasSdrObjectOwnership(),"Please check where this call come from and replace it with deleteSdrObjectSafeAndClearPointer");
            pSvxShape->InvalidateSdrObject();
            uno::Reference< lang::XComponent > xShapeComp( getWeakUnoShape(), uno::UNO_QUERY_THROW );
            xShapeComp->dispose();
        }
    }
    catch( const uno::Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    if(mpPlusData)
    {
        delete mpPlusData;
        mpPlusData = 0;
    }

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

    if(mpGluePointProvider)
    {
        delete mpGluePointProvider;
        mpGluePointProvider = 0;
    }
}

void SdrObject::deleteSafe(SdrObject* pObject)
{
    if(pObject)
    {
        SvxShape* pShape = pObject->getSvxShape();

        if(pShape && pShape->HasSdrObjectOwnership())
        {
            // only the shape is allowed to delete me, and will reset the ownership before doing so
        }
        else
        {
            // immediately broadcast to listeners directly listening at SdrObject
            {
                const SdrBaseHint aSdrBaseHint(*pObject, HINT_SDROBJECTDYING);
                pObject->Broadcast(aSdrBaseHint);
            }

            // delete SdrObject. This should be the onlylegal call to the
            // SdrObject destructor
            delete pObject;
        }
    }
}

SdrObjPlusData* SdrObject::NewPlusData() const
{
    return new SdrObjPlusData;
}

void SdrObject::handlePageChange(SdrPage* /*pOldPage*/, SdrPage* /*pNewPage*/)
{
    // nothing to do in the default
}

SfxItemPool& SdrObject::GetObjectItemPool() const
{
    return getSdrModelFromSdrObject().GetItemPool();
}

sal_uInt32 SdrObject::GetObjInventor()   const
{
    return SdrInventor;
}

sal_uInt16 SdrObject::GetObjIdentifier() const
{
    return static_cast< sal_uInt16 >(OBJ_NONE);
}

void SdrObject::TakeObjInfo(SdrObjTransformInfoRec& rInfo) const
{
    rInfo.mbRotateFreeAllowed = false;
    rInfo.mbMirrorFreeAllowed = false;
    rInfo.mbTransparenceAllowed = false;
    rInfo.mbGradientAllowed = false;
    rInfo.mbShearAllowed = false;
    rInfo.mbEdgeRadiusAllowed = false;
    rInfo.mbCanConvToPath = false;
    rInfo.mbCanConvToPoly = false;
    rInfo.mbCanConvToContour = false;
    rInfo.mbCanConvToPathLineToArea = false;
    rInfo.mbCanConvToPolyLineToArea = false;
}

SdrLayerID SdrObject::GetLayer() const
{
    return mnLayerID;
}

void SdrObject::getMergedHierarchyLayerSet(SetOfByte& rSet) const
{
    rSet.Set(GetLayer());
}

void SdrObject::SetLayer(SdrLayerID nLayer)
{
    if(GetLayer() != nLayer)
    {
        const SdrObjectChangeBroadcaster aSdrObjectChangeBroadcaster(*this);
        mnLayerID = nLayer;
        SetChanged();
    }
}

SdrObject* SdrObject::GetParentSdrObject() const
{
    return getParentOfSdrObject()
        ? getParentOfSdrObject()->getSdrObjectFromSdrObjList()
        : 0;
}

void SdrObject::SetName(const String& rStr)
{
    if(rStr.Len() && !mpPlusData)
    {
        ImpForcePlusData();
    }

    if(mpPlusData && mpPlusData->maObjName != rStr)
    {
        // Undo/Redo for setting object's name
        const SdrObjectChangeBroadcaster aSdrObjectChangeBroadcaster(*this);
        bool bUndo( false );

        if(getSdrModelFromSdrObject().IsUndoEnabled())
        {
            bUndo = true;
            SdrUndoAction* pUndoAction = getSdrModelFromSdrObject().GetSdrUndoFactory().CreateUndoObjectStrAttr(
                *this, SdrUndoObjStrAttr::OBJ_NAME, GetName(), rStr);
            getSdrModelFromSdrObject().BegUndo( pUndoAction->GetComment() );
            getSdrModelFromSdrObject().AddUndo( pUndoAction );
        }

        mpPlusData->maObjName = rStr;

        if(bUndo)
        {
            getSdrModelFromSdrObject().EndUndo();
        }

        SetChanged();
    }
}

String SdrObject::GetName() const
{
    if(mpPlusData)
    {
        return mpPlusData->maObjName;
    }

    return String();
}

void SdrObject::SetTitle(const String& rStr)
{
    if(rStr.Len() && !mpPlusData)
    {
        ImpForcePlusData();
    }

    if(mpPlusData && mpPlusData->maObjTitle != rStr)
    {
        // Undo/Redo for setting object's title
        bool bUndo( false );
        const SdrObjectChangeBroadcaster aSdrObjectChangeBroadcaster(*this);

        if(getSdrModelFromSdrObject().IsUndoEnabled())
        {
            bUndo = true;
            SdrUndoAction* pUndoAction = getSdrModelFromSdrObject().GetSdrUndoFactory().CreateUndoObjectStrAttr(
                *this, SdrUndoObjStrAttr::OBJ_TITLE, GetTitle(), rStr );
            getSdrModelFromSdrObject().BegUndo( pUndoAction->GetComment() );
            getSdrModelFromSdrObject().AddUndo( pUndoAction );
        }

        mpPlusData->maObjTitle = rStr;

        if(bUndo)
        {
            getSdrModelFromSdrObject().EndUndo();
        }

        SetChanged();
    }
}

String SdrObject::GetTitle() const
{
    if(mpPlusData)
    {
        return mpPlusData->maObjTitle;
    }

    return String();
}

void SdrObject::SetDescription(const String& rStr)
{
    if(rStr.Len() && !mpPlusData)
    {
        ImpForcePlusData();
    }

    if(mpPlusData && mpPlusData->maObjDescription != rStr)
    {
        // Undo/Redo for setting object's description
        bool bUndo( false );
        const SdrObjectChangeBroadcaster aSdrObjectChangeBroadcaster(*this);

        if(getSdrModelFromSdrObject().IsUndoEnabled())
        {
            bUndo = true;
            SdrUndoAction* pUndoAction = getSdrModelFromSdrObject().GetSdrUndoFactory().CreateUndoObjectStrAttr(
                *this, SdrUndoObjStrAttr::OBJ_DESCRIPTION, GetDescription(), rStr );
            getSdrModelFromSdrObject().BegUndo( pUndoAction->GetComment() );
            getSdrModelFromSdrObject().AddUndo( pUndoAction );
        }

        mpPlusData->maObjDescription = rStr;

        if(bUndo)
        {
            getSdrModelFromSdrObject().EndUndo();
        }

        SetChanged();
    }
}

String SdrObject::GetDescription() const
{
    if(mpPlusData)
    {
        return mpPlusData->maObjDescription;
    }

    return String();
}

void SdrObject::SetHTMLName(const String& rStr)
{
    if(rStr.Len() && !mpPlusData)
    {
        ImpForcePlusData();
    }

    if(mpPlusData && mpPlusData->maObjName != rStr)
    {
        mpPlusData->maHTMLName = rStr;
        SetChanged();
    }
}

String SdrObject::GetHTMLName() const
{
    if(mpPlusData)
    {
        return mpPlusData->maHTMLName;
    }

    return String();
}

sal_uInt32 SdrObject::GetNavigationPosition() const
{
    if(!getParentOfSdrObject())
    {
        return 0;
    }

    return mnOrdNum;
}

const basegfx::B2DRange& SdrObject::getObjectRange(const SdrView* pSdrView) const
{
    // try to get the view dependent range first
    if(pSdrView)
    {
        const SdrPageView* pSdrPageView = pSdrView->GetSdrPageView();

        if(pSdrPageView)
        {
            if(1 == pSdrPageView->PageWindowCount())
            {
                const sdr::contact::ViewObjectContact& rViewObjectContact = GetViewContact().GetViewObjectContact(
                    pSdrPageView->GetPageWindow(0)->GetObjectContact());

                return rViewObjectContact.getViewDependentRange();
            }
            else
            {
                OSL_ENSURE(false, "no unique ObjectContact found, determine one yourself and get the range from there (!)");
            }
        }
    }

    // no ObjectContact found, fallback to view-independent range
    return GetViewContact().getViewIndependentRange();
}

const basegfx::B2DRange& SdrObject::getSnapRange() const
{
    return GetViewContact().getSnapRange();
}

void SdrObject::SetChanged()
{
    ActionChanged();

    if(IsObjectInserted())
    {
        getSdrModelFromSdrObject().SetChanged();
    }
}

// Tooling for painting a single object to a OutputDevice.
bool SdrObject::SingleObjectPainter(OutputDevice& rOut) const
{
    SdrObjectVector aObjectVector;
    aObjectVector.push_back(const_cast< SdrObject* >(this));

    sdr::contact::ObjectContactOfObjListPainter aPainter(rOut, aObjectVector, getSdrPageFromSdrObject());
    sdr::contact::DisplayInfo aDisplayInfo;

    // do processing
    aPainter.ProcessDisplay(aDisplayInfo);

    return true;
}

bool SdrObject::LineGeometryUsageIsNecessary() const
{
    const XLineStyle eXLS((XLineStyle)((const XLineStyleItem&)GetMergedItem(XATTR_LINESTYLE)).GetValue());

    return (XLINE_NONE != eXLS);
}

void SdrObject::copyDataFromSdrObject(const SdrObject& rSource)
{
    if(this != &rSource)
    {
        if(mpProperties)
        {
            delete mpProperties;
            mpProperties = 0;
        }

        if(rSource.mpProperties)
        {
            mpProperties = &rSource.GetProperties().Clone(*this);
        }

        if(mpViewContact)
        {
            delete mpViewContact;
            mpViewContact = 0;
        }

        if(mpGluePointProvider)
        {
            delete mpGluePointProvider;
               mpGluePointProvider = 0;
        }

        if(rSource.mpGluePointProvider)
        {
            mpGluePointProvider = &rSource.GetGluePointProvider().Clone();
        }

        maSdrObjectTransformation = rSource.maSdrObjectTransformation.getB2DHomMatrix();
        mnLayerID = rSource.mnLayerID;
        mbMoveProtect = rSource.mbMoveProtect;
        mbSizeProtect = rSource.mbSizeProtect;
        mbNoPrint = rSource.mbNoPrint;
        mbMarkProtect = rSource.mbMarkProtect;
        mbVisible = rSource.mbVisible;
        mbEmptyPresObj = rSource.mbEmptyPresObj;
        mbNotVisibleAsMaster = rSource.mbNotVisibleAsMaster;

        if(mpPlusData)
        {
            delete mpPlusData;
            mpPlusData = 0;
        }

        if(rSource.mpPlusData)
        {
            mpPlusData = rSource.mpPlusData->Clone(this);
        }
    }
}

SdrObject* SdrObject::CloneSdrObject(SdrModel* pTargetModel) const
{
    SdrObject* pClone = new SdrObject(
        pTargetModel ? *pTargetModel : getSdrModelFromSdrObject());
    OSL_ENSURE(pClone, "CloneSdrObject error (!)");
    pClone->copyDataFromSdrObject(*this);

    return pClone;
}

void SdrObject::TakeObjNameSingul(XubString& rName) const
{
    rName = ImpGetResStr(STR_ObjNameSingulNONE);
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
    rName = ImpGetResStr(STR_ObjNamePluralNONE);
}

void SdrObject::TakeMarkedDescriptionString(sal_uInt16 nStrCacheID, XubString& rStr, sal_uInt16 nVal) const
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

XubString SdrObject::GetWinkStr(sal_Int32 nWink, bool bNoDegChar) const
{
    XubString aStr;

    getSdrModelFromSdrObject().TakeWinkStr(nWink, aStr, bNoDegChar);

    return aStr;
}

XubString SdrObject::GetMetrStr(sal_Int32 nVal, MapUnit /*eWantMap*/, bool bNoUnitChars) const
{
    XubString aStr;

    getSdrModelFromSdrObject().TakeMetricStr(nVal, aStr, bNoUnitChars);

    return aStr;
}

basegfx::B2DPolyPolygon SdrObject::TakeXorPoly() const
{
    return basegfx::B2DPolyPolygon(basegfx::tools::createPolygonFromRect(getObjectRange(0)));
}

void SdrObject::AddToHdlList(SdrHdlList& rHdlList) const
{
    const basegfx::B2DHomMatrix& rObjTrans = getSdrObjectTransformation();

    new SdrHdl(rHdlList, this, HDL_UPLFT, rObjTrans * basegfx::B2DPoint(0.0, 0.0)); // TopLeft
    new SdrHdl(rHdlList, this, HDL_UPPER, rObjTrans * basegfx::B2DPoint(0.5, 0.0)); // Top
    new SdrHdl(rHdlList, this, HDL_UPRGT, rObjTrans * basegfx::B2DPoint(1.0, 0.0)); // TopRight
    new SdrHdl(rHdlList, this, HDL_LEFT , rObjTrans * basegfx::B2DPoint(0.0, 0.5)); // Left
    new SdrHdl(rHdlList, this, HDL_RIGHT, rObjTrans * basegfx::B2DPoint(1.0, 0.5)); // Right
    new SdrHdl(rHdlList, this, HDL_LWLFT, rObjTrans * basegfx::B2DPoint(0.0, 1.0)); // BottomLeft
    new SdrHdl(rHdlList, this, HDL_LOWER, rObjTrans * basegfx::B2DPoint(0.5, 1.0)); // Bottom
    new SdrHdl(rHdlList, this, HDL_LWRGT, rObjTrans * basegfx::B2DPoint(1.0, 1.0)); // BottomRight
}

sal_uInt32 SdrObject::GetPlusHdlCount(const SdrHdl& /*rHdl*/) const
{
    return 0;
}

void SdrObject::GetPlusHdl(SdrHdlList& /*rHdlList*/, const SdrObject& /*rSdrObject*/, const SdrHdl& /*rHdl*/, sal_uInt32 /*nPlNum*/) const
{
}

bool SdrObject::hasSpecialDrag() const
{
    // default has no special drag
    return false;
}

bool SdrObject::supportsFullDrag() const
{
    // default supports full drag
    return true;
}

SdrObject* SdrObject::getFullDragClone() const
{
    // default uses simple clone
    return CloneSdrObject();
}

bool SdrObject::beginSpecialDrag(SdrDragStat& rDrag) const
{
    const SdrHdl* pHdl = rDrag.GetActiveHdl();

    if(pHdl)
    {
        const SdrHdlKind eHdl(pHdl->GetKind());

        if( eHdl==HDL_UPLFT || eHdl==HDL_UPPER || eHdl==HDL_UPRGT ||
            eHdl==HDL_LEFT  || eHdl==HDL_RIGHT ||
            eHdl==HDL_LWLFT || eHdl==HDL_LOWER || eHdl==HDL_LWRGT)
        {
            // confirm special drag
            return true;
        }
    }

    return false;
}

bool SdrObject::applySpecialDrag(SdrDragStat& rDrag)
{
    const SdrHdl* pHdl = rDrag.GetActiveHdl();

    if(pHdl)
    {
        const SdrHdlKind eHdl(pHdl->GetKind());
        const bool bLeft(eHdl==HDL_UPLFT || eHdl==HDL_LEFT  || eHdl==HDL_LWLFT);
        const bool bTop(eHdl==HDL_UPRGT || eHdl==HDL_UPPER || eHdl==HDL_UPLFT);
        const bool bRight(eHdl==HDL_UPRGT || eHdl==HDL_RIGHT || eHdl==HDL_LWRGT);
        const bool bBottom(eHdl==HDL_LWRGT || eHdl==HDL_LOWER || eHdl==HDL_LWLFT);
        double fLeft(0.0);
        double fTop(0.0);
        double fRight(1.0);
        double fBottom(1.0);
        basegfx::B2DHomMatrix aInvObject(getSdrObjectTransformation());
        aInvObject.invert();
        const basegfx::B2DPoint aMousePos(aInvObject * rDrag.GetNow());

        if(bLeft)
        {
            fLeft = aMousePos.getX();
        }

        if(bTop)
        {
            fTop = aMousePos.getY();
        }

        if(bRight)
        {
            fRight = aMousePos.getX();
        }

        if(bBottom)
        {
            fBottom = aMousePos.getY();
        }

        const bool bCorner(eHdl==HDL_UPLFT || eHdl==HDL_UPRGT || eHdl==HDL_LWLFT || eHdl==HDL_LWRGT);
        const bool bOrtho(rDrag.GetSdrViewFromSdrDragStat().IsOrthogonal());

        if(bOrtho)
        {
            const double fXScale(fRight - fLeft);
            const double fYScale(fBottom - fTop);

            if(!basegfx::fTools::equal(fXScale, fYScale))
            {
                if(bCorner)
                {
                    static bool bInverse(false);
                    const bool bBigOrtho(bInverse
                        ? !rDrag.GetSdrViewFromSdrDragStat().IsBigOrthogonal()
                        : rDrag.GetSdrViewFromSdrDragStat().IsBigOrthogonal());

                    if((fabs(fXScale) < fabs(fYScale)) != bBigOrtho)
                    {
                        // adapt height
                        const double fChange((fXScale < 0.0) != (fYScale < 0.0) ? -fXScale : fXScale);

                        if(bTop)
                        {
                            fTop = fBottom - fChange;
                        }
                        else // if(bBottom)
                        {
                            fBottom = fTop + fChange;
                        }
                    }
                    else
                    {
                        // adapt width
                        const double fChange((fXScale < 0.0) != (fYScale < 0.0) ? -fYScale : fYScale);

                        if(bLeft)
                        {
                            fLeft = fRight - fChange;
                        }
                        else // if(bRight)
                        {
                            fRight = fLeft + fChange;
                        }
                    }
                }
                else
                {
                    if(bLeft || bRight)
                    {
                        // adapt height
                        const double fChange((fXScale - fYScale) * 0.5);

                        fTop -= fChange;
                        fBottom += fChange;

                        if(fXScale < 0.0)
                        {
                            std::swap(fTop, fBottom);
                        }
                    }
                    else // bTop || bBottom
                    {
                        // adapt width
                        const double fChange((fYScale - fXScale) * 0.5);

                        fLeft -= fChange;
                        fRight += fChange;

                        if(fYScale < 0.0)
                        {
                            std::swap(fLeft, fRight);
                        }
                    }
                }
            }
        }

        const basegfx::B2DVector aNewScale(fRight - fLeft, fBottom - fTop);
        const basegfx::B2DPoint aNewTranslate(fLeft, fTop);

        if(!aNewScale.equal(getSdrObjectScale()) || !aNewTranslate.equalZero())
        {
            basegfx::B2DHomMatrix aNew(
                getSdrObjectTransformation() *
                basegfx::tools::createScaleTranslateB2DHomMatrix(
                    aNewScale,
                    aNewTranslate));

            setSdrObjectTransformation(aNew);
        }
    }

    return true;
}

String SdrObject::getSpecialDragComment(const SdrDragStat& /*rDrag*/) const
{
    // default has no drag comment
    return String();
}

basegfx::B2DPolyPolygon SdrObject::getSpecialDragPoly(const SdrDragStat& /*rDrag*/) const
{
    // default has nothing to add
    return basegfx::B2DPolyPolygon();
}

bool SdrObject::BegCreate(SdrDragStat& rStat)
{
    rStat.SetActionRange(rStat.TakeCreateRange());
    rStat.SetOrtho4Possible();

    const basegfx::B2DVector aScale(rStat.GetNow() - rStat.GetStart());
    const basegfx::B2DPoint aTranslate(rStat.GetStart());

    setSdrObjectTransformation(
        basegfx::tools::createScaleTranslateB2DHomMatrix(
            aScale,
            aTranslate));

    return true;
}

namespace
{
    void impSingleCreateStep(SdrObject& rObject, SdrDragStat& rStat)
    {
        basegfx::B2DVector aScale(rStat.GetNow() - rStat.GetStart());
        basegfx::B2DPoint aTranslate(rStat.GetStart());

        static bool bForceCrCe(false);
        if(bForceCrCe || rStat.GetSdrViewFromSdrDragStat().IsCreate1stPointAsCenter())
        {
            aTranslate -= aScale;
            aScale *= 2.0;
        }

        rObject.setSdrObjectTransformation(
            basegfx::tools::createScaleTranslateB2DHomMatrix(
                aScale,
                aTranslate));
    }
}

bool SdrObject::MovCreate(SdrDragStat& rStat)
{
    rStat.SetActionRange(rStat.TakeCreateRange());
    impSingleCreateStep(*this, rStat);

    return true;
}

bool SdrObject::EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd)
{
    impSingleCreateStep(*this, rStat);

    return (eCmd==SDRCREATE_FORCEEND || rStat.GetPointAnz() >= 2);
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
    return basegfx::B2DPolyPolygon(basegfx::tools::createPolygonFromRect(rDrag.TakeCreateRange()));
}

Pointer SdrObject::GetCreatePointer(const SdrView& /*rSdrView*/) const
{
    return Pointer(POINTER_CROSS);
}

void SdrObject::SetAnchorPos(const basegfx::B2DPoint& rPnt)
{
    if(!rPnt.equalZero() && !mpPlusData)
    {
        ImpForcePlusData();
    }

    if(mpPlusData && mpPlusData->maObjectAnchor != rPnt)
    {
        mpPlusData->maObjectAnchor = rPnt;
    }
}

basegfx::B2DPoint SdrObject::GetAnchorPos() const
{
    if(mpPlusData)
    {
        return mpPlusData->maObjectAnchor;
    }

    return basegfx::B2DPoint(0.0, 0.0);
}

void SdrObject::AdjustToMaxRange( const basegfx::B2DRange& rMaxRange, bool /* bShrinkOnly = false */ )
{
    sdr::legacy::SetLogicRange(*this, rMaxRange);
}

sal_uInt32 SdrObject::GetSnapPointCount() const
{
    return GetObjectPointCount();
}

basegfx::B2DPoint  SdrObject::GetSnapPoint(sal_uInt32 i) const
{
    return GetObjectPoint(i);
}

bool SdrObject::IsPolygonObject() const
{
    return false;
}

sal_uInt32 SdrObject::GetObjectPointCount() const
{
    return 0;
}

basegfx::B2DPoint SdrObject::GetObjectPoint(sal_uInt32 /*i*/) const
{
    return basegfx::B2DPoint(0.0, 0.0);
}

void SdrObject::SetObjectPoint(const basegfx::B2DPoint& /*rPnt*/, sal_uInt32 /*i*/)
{
    // default does nothing
}

bool SdrObject::HasTextEdit() const
{
    return false;
}

bool SdrObject::BegTextEdit(SdrOutliner& /*rOutl*/)
{
    return false;
}

void SdrObject::EndTextEdit(SdrOutliner& /*rOutl*/)
{
}

void SdrObject::SetOutlinerParaObject(OutlinerParaObject* /*pTextObject*/)
{
    // default does nothing
}

OutlinerParaObject* SdrObject::GetOutlinerParaObject() const
{
    return 0;
}

void SdrObject::ReformatText()
{
    // default does nothing
}

void SdrObject::BurnInStyleSheetAttributes()
{
    GetProperties().ForceStyleToHardAttributes();
}

SdrObjUserData* SdrObject::ImpGetMacroUserData() const
{
    SdrObjUserData* pData = 0;
    const sal_uInt32 nAnz(GetUserDataCount());

    for(sal_uInt32 nNum(nAnz); nNum > 0 && !pData;)
    {
        nNum--;
        pData = GetUserData(nNum);

        if(!pData->HasMacro(this))
        {
            pData = 0;
        }
    }

    return pData;
}

bool SdrObject::HasMacro() const
{
    SdrObjUserData* pData=ImpGetMacroUserData();

    return pData ? pData->HasMacro(this) : false;
}

SdrObject* SdrObject::CheckMacroHit(const SdrObjMacroHitRec& rRec) const
{
    SdrObjUserData* pData = ImpGetMacroUserData();

    if(pData)
    {
        return pData->CheckMacroHit(rRec, this);
    }

    if(rRec.mpSdrView)
    {
        return SdrObjectPrimitiveHit(*this, rRec.maPos, rRec.mfTol, *rRec.mpSdrView, false, 0);
    }

    return 0;
}

Pointer SdrObject::GetMacroPointer(const SdrObjMacroHitRec& rRec) const
{
    SdrObjUserData* pData = ImpGetMacroUserData();

    if(pData)
    {
        return pData->GetMacroPointer(rRec, this);
    }

    return Pointer(POINTER_REFHAND);
}

void SdrObject::PaintMacro(OutputDevice& rOut, const SdrObjMacroHitRec& rRec) const
{
    SdrObjUserData* pData = ImpGetMacroUserData();

    if(pData)
    {
        pData->PaintMacro(rOut, rRec, this);
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
    SdrObjUserData* pData = ImpGetMacroUserData();

    if(pData)
    {
        return pData->DoMacro(rRec, this);
    }

    return false;
}

XubString SdrObject::GetMacroPopupComment(const SdrObjMacroHitRec& rRec) const
{
    SdrObjUserData* pData = ImpGetMacroUserData();

    if(pData)
    {
        return pData->GetMacroPopupComment(rRec, this);
    }

    return String();
}

SdrObjGeoData* SdrObject::NewGeoData() const
{
    return new SdrObjGeoData;
}

void SdrObject::SaveGeoData(SdrObjGeoData& rGeo) const
{
    rGeo.maSdrObjectTransformation = maSdrObjectTransformation.getB2DHomMatrix();

    if(mpPlusData)
    {
        rGeo.maObjectAnchor = mpPlusData->maObjectAnchor;
    }
    else
    {
        rGeo.maObjectAnchor = basegfx::B2DPoint(0.0, 0.0);
    }

    rGeo.mbMoveProtect = mbMoveProtect;
    rGeo.mbSizeProtect = mbSizeProtect;
    rGeo.mbNoPrint = mbNoPrint;
    rGeo.mbVisible = mbVisible;
    rGeo.mnLayerID = mnLayerID;

    // Benutzerdefinierte Klebepunkte
    if(mpPlusData && mpPlusData->mpGluePoints)
    {
        if(rGeo.mpGPL)
        {
            *rGeo.mpGPL = *mpPlusData->mpGluePoints;
        }
        else
        {
            rGeo.mpGPL = new SdrGluePointList(*mpPlusData->mpGluePoints);
        }
    }
    else
    {
        if(rGeo.mpGPL)
        {
            delete rGeo.mpGPL;
            rGeo.mpGPL = 0;
        }
    }
}

void SdrObject::RestGeoData(const SdrObjGeoData& rGeo)
{
    ActionChanged();

    maSdrObjectTransformation.setB2DHomMatrix(rGeo.maSdrObjectTransformation);

    if(!rGeo.maObjectAnchor.equalZero())
    {
        ImpForcePlusData();
    }

    if(mpPlusData)
    {
        mpPlusData->maObjectAnchor = rGeo.maObjectAnchor;
    }

    mbMoveProtect = rGeo.mbMoveProtect;
    mbSizeProtect = rGeo.mbSizeProtect;
    mbNoPrint = rGeo.mbNoPrint;
    mbVisible = rGeo.mbVisible;
    mnLayerID = rGeo.mnLayerID;

    // Benutzerdefinierte Klebepunkte
    if(rGeo.mpGPL)
    {
        ImpForcePlusData();

        if(mpPlusData->mpGluePoints)
        {
            *mpPlusData->mpGluePoints = *rGeo.mpGPL;
        }
        else
        {
            mpPlusData->mpGluePoints = new SdrGluePointList(*rGeo.mpGPL);
        }
    }
    else
    {
        if(mpPlusData && mpPlusData->mpGluePoints)
        {
            delete mpPlusData->mpGluePoints;
            mpPlusData->mpGluePoints = 0;
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
    const SdrObjectChangeBroadcaster aSdrObjectChangeBroadcaster(*this);

    RestGeoData(rGeo);
    SetChanged();
}

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
    const SdrObjectChangeBroadcaster aSdrObjectChangeBroadcaster(*this);
    SetChanged();

    const Rectangle aSnap(sdr::legacy::GetSnapRect(*this));
    const Rectangle aLogic(sdr::legacy::GetLogicRect(*this));
    Point aRef1(aSnap.Center());
    Point aRef2(aRef1); aRef2.Y()++;
    const SfxPoolItem *pPoolItem = 0;

    if(SFX_ITEM_SET == rAttr.GetItemState(SDRATTR_TRANSFORMREF1X, true, &pPoolItem))
    {
        aRef1.X() = ((const SdrMetricItem*)pPoolItem)->GetValue();
    }

    if(SFX_ITEM_SET == rAttr.GetItemState(SDRATTR_TRANSFORMREF1Y, true, &pPoolItem))
    {
        aRef1.Y() = ((const SdrMetricItem*)pPoolItem)->GetValue();
    }

    if(SFX_ITEM_SET == rAttr.GetItemState(SDRATTR_TRANSFORMREF2X, true, &pPoolItem))
    {
        aRef2.X() = ((const SdrMetricItem*)pPoolItem)->GetValue();
    }

    if(SFX_ITEM_SET == rAttr.GetItemState(SDRATTR_TRANSFORMREF2Y, true, &pPoolItem))
    {
        aRef2.Y() = ((const SdrMetricItem*)pPoolItem)->GetValue();
    }

    Rectangle aNewSnap(aSnap);

    if(SFX_ITEM_SET == rAttr.GetItemState(SDRATTR_MOVEX, true, &pPoolItem))
    {
        sal_Int32 n = ((const SdrMetricItem*)pPoolItem)->GetValue();
        aNewSnap.Move(n,0);
    }

    if(SFX_ITEM_SET == rAttr.GetItemState(SDRATTR_MOVEY, true, &pPoolItem))
    {
        sal_Int32 n = ((const SdrMetricItem*)pPoolItem)->GetValue();
        aNewSnap.Move(0,n);
    }

    if(SFX_ITEM_SET == rAttr.GetItemState(SDRATTR_ONEPOSITIONX, true, &pPoolItem))
    {
        sal_Int32 n = ((const SdrMetricItem*)pPoolItem)->GetValue();
        aNewSnap.Move(n-aNewSnap.Left(),0);
    }

    if(SFX_ITEM_SET == rAttr.GetItemState(SDRATTR_ONEPOSITIONY, true, &pPoolItem))
    {
        sal_Int32 n = ((const SdrMetricItem*)pPoolItem)->GetValue();
        aNewSnap.Move(0,n-aNewSnap.Top());
    }

    if(SFX_ITEM_SET == rAttr.GetItemState(SDRATTR_ONESIZEWIDTH, true, &pPoolItem))
    {
        sal_Int32 n = ((const SdrMetricItem*)pPoolItem)->GetValue();
        aNewSnap.Right()=aNewSnap.Left()+n;
    }

    if(SFX_ITEM_SET == rAttr.GetItemState(SDRATTR_ONESIZEHEIGHT, true, &pPoolItem))
    {
        sal_Int32 n = ((const SdrMetricItem*)pPoolItem)->GetValue();
        aNewSnap.Bottom()=aNewSnap.Top()+n;
    }

    if(aNewSnap!=aSnap)
    {
        if(aNewSnap.GetSize() == aSnap.GetSize())
        {
            sdr::legacy::MoveSdrObject(*this, Size(aNewSnap.Left()-aSnap.Left(),aNewSnap.Top()-aSnap.Top()));
        }
        else
        {
            sdr::legacy::SetSnapRect(*this, aNewSnap);
        }
    }

    if(SFX_ITEM_SET == rAttr.GetItemState(SDRATTR_SHEARANGLE, true, &pPoolItem))
    {
        sal_Int32 n(((const SdrAngleItem*)pPoolItem)->GetValue());
        n -= sdr::legacy::GetShearAngleX(*this);

        if(n)
        {
            sdr::legacy::ShearSdrObject(*this, aRef1, n, false);
        }
    }

    if(SFX_ITEM_SET == rAttr.GetItemState(SDRATTR_ROTATEANGLE, true, &pPoolItem))
    {
        sal_Int32 n = ((const SdrAngleItem*)pPoolItem)->GetValue();
        n -= sdr::legacy::GetRotateAngle(*this);

        if(n)
        {
            sdr::legacy::RotateSdrObject(*this, aRef1, n);
        }
    }

    if(SFX_ITEM_SET == rAttr.GetItemState(SDRATTR_ROTATEONE, true, &pPoolItem))
    {
        sal_Int32 n = ((const SdrAngleItem*)pPoolItem)->GetValue();
        sdr::legacy::RotateSdrObject(*this, aRef1, n);
    }

    if(SFX_ITEM_SET == rAttr.GetItemState(SDRATTR_HORZSHEARONE, true, &pPoolItem))
    {
        const sal_Int32 n(((const SdrAngleItem*)pPoolItem)->GetValue());

        if(n)
        {
            sdr::legacy::ShearSdrObject(*this, aRef1, n, false);
        }
    }

    if(SFX_ITEM_SET == rAttr.GetItemState(SDRATTR_VERTSHEARONE, true, &pPoolItem))
    {
        const sal_Int32 n(((const SdrAngleItem*)pPoolItem)->GetValue());

        if(n)
        {
            sdr::legacy::ShearSdrObject(*this, aRef1, n, true);
        }
    }

    if(SFX_ITEM_SET == rAttr.GetItemState(SDRATTR_OBJMOVEPROTECT, true, &pPoolItem))
    {
        bool b = ((const SdrYesNoItem*)pPoolItem)->GetValue();
        SetMoveProtect(b);
    }

    if(SFX_ITEM_SET == rAttr.GetItemState(SDRATTR_OBJSIZEPROTECT, true, &pPoolItem))
    {
        bool b = ((const SdrYesNoItem*)pPoolItem)->GetValue();
        SetResizeProtect(b);
    }

    /* #67368# move protect always sets size protect */
    if(IsMoveProtect())
    {
        SetResizeProtect(true);
    }

    if(SFX_ITEM_SET == rAttr.GetItemState(SDRATTR_OBJPRINTABLE, true, &pPoolItem))
    {
        bool b = ((const SdrYesNoItem*)pPoolItem)->GetValue();
        SetPrintable(b);
    }

    if(SFX_ITEM_SET == rAttr.GetItemState(SDRATTR_OBJVISIBLE, true, &pPoolItem))
    {
        bool b = ((const SdrYesNoItem*)pPoolItem)->GetValue();
        SetVisible(b);
    }

    SdrLayerID nLayer(SDRLAYER_NOTFOUND);

    if(SFX_ITEM_SET == rAttr.GetItemState(SDRATTR_LAYERID, true, &pPoolItem))
    {
        nLayer = SdrLayerID(((const SfxUInt16Item*)pPoolItem)->GetValue());
    }

    SdrPage* pPage = getSdrPageFromSdrObject();

    if(SFX_ITEM_SET == rAttr.GetItemState(SDRATTR_LAYERNAME, true, &pPoolItem))
    {
        XubString aLayerName=((const SfxStringItem*)pPoolItem)->GetValue();
        const SdrLayerAdmin* pLayAd = pPage
            ? &pPage->GetPageLayerAdmin()
            : &pPage->getSdrModelFromSdrPage().GetModelLayerAdmin();

        if(pLayAd)
        {
            const SdrLayer* pLayer = pLayAd->GetLayer(aLayerName, true);

            if(pLayer)
            {
                nLayer = pLayer->GetID();
            }
        }

    }

    if(SDRLAYER_NOTFOUND != nLayer)
    {
        SetLayer(nLayer);
    }

    if(SFX_ITEM_SET == rAttr.GetItemState(SDRATTR_OBJECTNAME, true, &pPoolItem))
    {
        XubString aName = ((const SfxStringItem*)pPoolItem)->GetValue();
        SetName(aName);
    }

    Rectangle aNewLogic(aLogic);

    if(SFX_ITEM_SET == rAttr.GetItemState(SDRATTR_LOGICSIZEWIDTH, true, &pPoolItem))
    {
        sal_Int32 n=((const SdrMetricItem*)pPoolItem)->GetValue();
        aNewLogic.Right()=aNewLogic.Left()+n;
    }

    if(SFX_ITEM_SET == rAttr.GetItemState(SDRATTR_LOGICSIZEHEIGHT, true, &pPoolItem))
    {
        sal_Int32 n = ((const SdrMetricItem*)pPoolItem)->GetValue();
        aNewLogic.Bottom() = aNewLogic.Top() + n;
    }

    if(aNewLogic != aLogic)
    {
        sdr::legacy::SetLogicRect(*this, aNewLogic);
    }

    Fraction aResizeX(1,1);
    Fraction aResizeY(1,1);

    if(SFX_ITEM_SET == rAttr.GetItemState(SDRATTR_RESIZEXONE, true, &pPoolItem))
    {
        aResizeX *= ((const SdrFractionItem*)pPoolItem)->GetValue();
    }

    if(SFX_ITEM_SET == rAttr.GetItemState(SDRATTR_RESIZEYONE, true, &pPoolItem))
    {
        aResizeY *= ((const SdrFractionItem*)pPoolItem)->GetValue();
    }

    if(aResizeX != Fraction(1,1) || aResizeY != Fraction(1,1))
    {
        sdr::legacy::ResizeSdrObject(*this, aRef1, aResizeX, aResizeY);
    }
}

void lcl_SetItem(SfxItemSet& rAttr, bool bMerge, const SfxPoolItem& rItem)
{
    if(bMerge)
    {
        rAttr.MergeValue(rItem, true);
    }
    else
    {
        rAttr.Put(rItem);
    }
}

void SdrObject::TakeNotPersistAttr(SfxItemSet& rAttr, bool bMerge) const
{
    const Rectangle aSnap(sdr::legacy::GetSnapRect(*this));
    const Rectangle aLogic(sdr::legacy::GetLogicRect(*this));

    lcl_SetItem(rAttr, bMerge, SdrYesNoItem(SDRATTR_OBJMOVEPROTECT, IsMoveProtect()));
    lcl_SetItem(rAttr, bMerge, SdrYesNoItem(SDRATTR_OBJSIZEPROTECT, IsResizeProtect()));
    lcl_SetItem(rAttr, bMerge, SdrYesNoItem(SDRATTR_OBJPRINTABLE, IsPrintable()));
    lcl_SetItem(rAttr, bMerge, SdrYesNoItem(SDRATTR_OBJVISIBLE, IsVisible()));
    lcl_SetItem(rAttr, bMerge, SdrAngleItem(SDRATTR_ROTATEANGLE, sdr::legacy::GetRotateAngle(*this)));
    lcl_SetItem(rAttr, bMerge, SdrAngleItem(SDRATTR_SHEARANGLE, sdr::legacy::GetShearAngleX(*this)));
    lcl_SetItem(rAttr, bMerge, SdrMetricItem(SDRATTR_ONESIZEWIDTH, aSnap.GetWidth() - 1));
    lcl_SetItem(rAttr, bMerge, SdrMetricItem(SDRATTR_ONESIZEHEIGHT, aSnap.GetHeight() - 1));
    lcl_SetItem(rAttr, bMerge, SdrMetricItem(SDRATTR_ONEPOSITIONX, aSnap.Left()));
    lcl_SetItem(rAttr, bMerge, SdrMetricItem(SDRATTR_ONEPOSITIONY, aSnap.Top()));

    if(aLogic.GetWidth() != aSnap.GetWidth())
    {
        lcl_SetItem(rAttr, bMerge, SdrMetricItem(SDRATTR_LOGICSIZEWIDTH, aLogic.GetWidth() - 1));
    }

    if(aLogic.GetHeight() != aSnap.GetHeight())
    {
        lcl_SetItem(rAttr, bMerge, SdrMetricItem(SDRATTR_LOGICSIZEHEIGHT, aLogic.GetHeight() - 1));
    }

    XubString aName(GetName());

    if(aName.Len())
    {
        lcl_SetItem(rAttr, bMerge, SfxStringItem(SDRATTR_OBJECTNAME, aName));
    }

    lcl_SetItem(rAttr, bMerge, SfxUInt16Item(SDRATTR_LAYERID, GetLayer()));

    SdrPage* pPage = getSdrPageFromSdrObject();
    const SdrLayerAdmin* pLayAd = pPage
        ? &pPage->GetPageLayerAdmin()
        : &pPage->getSdrModelFromSdrPage().GetModelLayerAdmin();

    if(pLayAd)
    {
        const SdrLayer* pLayer = pLayAd->GetLayerPerID(GetLayer());

        if(pLayer)
        {
            lcl_SetItem(rAttr, bMerge, SfxStringItem(SDRATTR_LAYERNAME, pLayer->GetName()));
        }
    }

    Point aRef1(aSnap.Center());
    Point aRef2(aRef1); aRef2.Y()++;

    lcl_SetItem(rAttr, bMerge, SdrMetricItem(SDRATTR_TRANSFORMREF1X, aRef1.X()));
    lcl_SetItem(rAttr, bMerge, SdrMetricItem(SDRATTR_TRANSFORMREF1Y, aRef1.Y()));
    lcl_SetItem(rAttr, bMerge, SdrMetricItem(SDRATTR_TRANSFORMREF2X, aRef2.X()));
    lcl_SetItem(rAttr, bMerge, SdrMetricItem(SDRATTR_TRANSFORMREF2Y, aRef2.Y()));
}

SfxStyleSheet* SdrObject::GetStyleSheet() const
{
    return GetProperties().GetStyleSheet();
}

void SdrObject::SetStyleSheet(SfxStyleSheet* pNewStyleSheet, bool bDontRemoveHardAttr)
{
    // only allow graphic and presentation styles for shapes
    if(pNewStyleSheet
        && (SFX_STYLE_FAMILY_PARA == pNewStyleSheet->GetFamily())
        && (SFX_STYLE_FAMILY_PAGE == pNewStyleSheet->GetFamily()))
    {
        return;
    }

    if(GetProperties().GetStyleSheet() != pNewStyleSheet)
    {
        const SdrObjectChangeBroadcaster aSdrObjectChangeBroadcaster(*this, HINT_OBJCHG_ATTR);

        GetProperties().SetStyleSheet(pNewStyleSheet, bDontRemoveHardAttr);
        SetChanged();
    }
}

SdrGluePoint SdrObject::GetVertexGluePoint(sal_uInt32 nPosNum) const
{
    basegfx::B2DPoint aGluePoint(0.5, 0.5);

    switch(nPosNum)
    {
        default: //case 0: TopCenter
        {
            aGluePoint.setY(0.0);
            break;
        }
        case 1: // RightCenter
        {
            aGluePoint.setX(1.0);
            break;
        }
        case 2: // BottomCenter
        {
            aGluePoint.setY(1.0);
            break;
        }
        case 3: // LeftCenter
        {
            aGluePoint.setX(0.0);
            break;
        }
    }

    aGluePoint = getSdrObjectTransformation() * aGluePoint;
    SdrGluePoint aGP(aGluePoint - sdr::legacy::GetSnapRange(*this).getCenter());
    aGP.SetPercent(false);

    return aGP;
}

const SdrGluePointList* SdrObject::GetGluePointList() const
{
    if(mpPlusData)
    {
        return mpPlusData->mpGluePoints;
    }

    return 0;
}

SdrGluePointList* SdrObject::ForceGluePointList()
{
    ImpForcePlusData();

    if(!mpPlusData->mpGluePoints)
    {
        mpPlusData->mpGluePoints = new SdrGluePointList;
    }

    return mpPlusData->mpGluePoints;
}

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
            // use neutral ViewInformation
            const drawinglayer::geometry::ViewInformation2D aViewInformation2D;

            // create extractor, process and get result
            drawinglayer::processor2d::LineGeometryExtractor2D aExtractor(aViewInformation2D);
            aExtractor.process(xSequence);

            // #i102241# check for line results
            const basegfx::B2DPolygonVector& rHairlineVector = aExtractor.getExtractedHairlines();

            if(rHairlineVector.size())
            {
                // for SdrObject creation, just copy all to a single Hairline-PolyPolygon
                for(sal_uInt32 a(0); a < rHairlineVector.size(); a++)
                {
                    aMergedHairlinePolyPolygon.append(rHairlineVector[a]);
                }
            }

            // #i102241# check for fill rsults
            const basegfx::B2DPolyPolygonVector& rLineFillVector(aExtractor.getExtractedLineFills());

            if(rLineFillVector.size())
            {
                // merge to a single PolyPolygon (OR)
                aMergedLineFillPolyPolygon = basegfx::tools::mergeToSinglePolyPolygon(rLineFillVector);
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
                aLinePolygonPart = new SdrPathObj(
                    getSdrModelFromSdrObject(),
                    aMergedLineFillPolyPolygon);

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
                aLineHairlinePart = new SdrPathObj(
                    getSdrModelFromSdrObject(),
                    aMergedHairlinePolyPolygon);

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
            SdrPathObj* pPath = dynamic_cast< SdrPathObj* >( pRet);

            if(pPath && pPath->isClosed())
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
                SdrObjGroup* pGroup = new SdrObjGroup(getSdrModelFromSdrObject());

                if(bAddOriginalGeometry)
                {
                    // Add a clone of the original geometry.
                    aSet.ClearItem();
                    aSet.Put(pRet->GetMergedItemSet());
                    aSet.Put(XLineStyleItem(XLINE_NONE));
                    aSet.Put(XLineWidthItem(0L));

                    SdrObject* pClone = pRet->CloneSdrObject();

                    pClone->SetMergedItemSet(aSet);
                    pGroup->InsertObjectToSdrObjList(*pClone);
                }

                if(aLinePolygonPart)
                {
                    pGroup->InsertObjectToSdrObjList(*aLinePolygonPart);
                }

                if(aLineHairlinePart)
                {
                    pGroup->InsertObjectToSdrObjList(*aLineHairlinePart);
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
        SdrObject* pClone = pRet->CloneSdrObject();
        pRet = pClone;
    }

    return pRet;
}

bool SdrObject::IsClosedObj() const
{
    // default is false
    return false;
}

// convert this path object to contour object, even when it is a group
SdrObject* SdrObject::ConvertToContourObj(SdrObject* pRet, bool bForceLineDash) const
{
    if(dynamic_cast< SdrObjGroup* >(pRet))
    {
        SdrObjList* pObjList2 = pRet->getChildrenOfSdrObject();
        SdrObjGroup* pGroup = new SdrObjGroup(getSdrModelFromSdrObject());

        for(sal_uInt32 a(0); a < pObjList2->GetObjCount(); a++)
        {
            SdrObject* pIterObj = pObjList2->GetObj(a);
            SdrObject* pConverted = ConvertToContourObj(pIterObj, bForceLineDash);

            if(pConverted)
            {
                pGroup->InsertObjectToSdrObjList(*pConverted);
            }
            else
            {
                OSL_ENSURE(false, "OOps, ConvertToContourObj() returned NO shape (!)");
            }
        }

        pRet = pGroup;
    }
    else
    {
        SdrPathObj* pPathObj = dynamic_cast< SdrPathObj* >(pRet);

        if(pPathObj)
        {
            // bezier geometry got created, even for straight edges since the given
            // object is a result of DoConvertToPolygonObject. For conversion to contour
            // this is not really needed and can be reduced again AFAP
            pPathObj->setB2DPolyPolygonInObjectCoordinates(
                basegfx::tools::simplifyCurveSegments(
                    pPathObj->getB2DPolyPolygonInObjectCoordinates()));
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

bool SdrObject::IsSdrEdgeObj() const
{
    return false;
}

bool SdrObject::IsE3dObject() const
{
    return false;
}

bool SdrObject::IsSdrUnoObj() const
{
    return false;
}

bool SdrObject::IsSdrGrafObj() const
{
    return false;
}

bool SdrObject::DoesSupportTextIndentingOnLineWidthChange() const
{
    return false;
}

SdrObject* SdrObject::ConvertToPolyObj(bool bBezier, bool bLineToArea) const
{
    SdrObject* pRet = DoConvertToPolygonObject(bBezier, true);

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

SdrObject* SdrObject::DoConvertToPolygonObject(bool /*bBezier*/, bool /*bAddText*/) const
{
    return NULL;
}

void SdrObject::SetMoveProtect(bool bNew)
{
    if(IsMoveProtect() != bNew)
    {
        const SdrObjectChangeBroadcaster aSdrObjectChangeBroadcaster(*this);
        mbMoveProtect = bNew;
        SetChanged();
    }
}

void SdrObject::SetResizeProtect(bool bNew)
{
    if(IsResizeProtect() != bNew)
    {
        const SdrObjectChangeBroadcaster aSdrObjectChangeBroadcaster(*this);
        mbSizeProtect = bNew;
        SetChanged();
    }
}

void SdrObject::SetPrintable(bool bNew)
{
    if(bNew == IsPrintable())
    {
        const SdrObjectChangeBroadcaster aSdrObjectChangeBroadcaster(*this);
        mbNoPrint = !bNew;
        SetChanged();
    }
}

void SdrObject::SetVisible(bool bNew)
{
    if(bNew == IsVisible())
    {
        const SdrObjectChangeBroadcaster aSdrObjectChangeBroadcaster(*this);
        mbVisible = bNew;
        SetChanged();
    }
}

sal_uInt32 SdrObject::GetUserDataCount() const
{
    if(!mpPlusData || !mpPlusData->mpUserDataList)
    {
        return 0;
    }

    return mpPlusData->mpUserDataList->GetUserDataCount();
}

SdrObjUserData* SdrObject::GetUserData(sal_uInt32 nNum) const
{
    if(!mpPlusData || !mpPlusData->mpUserDataList)
    {
        return 0;
    }

    return mpPlusData->mpUserDataList->GetUserData(nNum);
}

void SdrObject::InsertUserData(SdrObjUserData* pData, sal_uInt32 nPos)
{
    if(pData)
    {
        ImpForcePlusData();

        if(!mpPlusData->mpUserDataList)
        {
            mpPlusData->mpUserDataList = new SdrObjUserDataList;
        }

        mpPlusData->mpUserDataList->InsertUserData(pData, nPos);
    }
}

void SdrObject::DeleteUserData(sal_uInt32 nNum)
{
    const sal_uInt32 nAnz(GetUserDataCount());

    if(nNum < nAnz)
    {
        mpPlusData->mpUserDataList->DeleteUserData(nNum);

        if(1 == nAnz)
        {
            delete mpPlusData->mpUserDataList;
            mpPlusData->mpUserDataList = 0;
        }
    }
    else
    {
        DBG_ERROR("SdrObject::DeleteUserData(): ungueltiger Index");
    }
}

void SdrObject::impl_setUnoShape( const uno::Reference< uno::XInterface >& _rxUnoShape )
{
    maWeakUnoShape = _rxUnoShape;
    mpSvxShape = SvxShape::getImplementation( _rxUnoShape );
}

/** only for internal use! */
SvxShape* SdrObject::getSvxShape() const
{
    DBG_TESTSOLARMUTEX();
    // retrieving the impl pointer and subsequently using it is not thread-safe, of course, so it needs to be
    // guarded by the SolarMutex

    uno::Reference< uno::XInterface > xShape( maWeakUnoShape );
#if OSL_DEBUG_LEVE > 0
    OSL_ENSURE( !( !xShape.is() && mpSvxShape ),
        "SdrObject::getSvxShape: still having IMPL-Pointer to dead object!" );
#endif
    //#113608#, make sure mpSvxShape is always synchronized with maWeakUnoShape
    if ( mpSvxShape && !xShape.is() )
    {
        const_cast< SdrObject* >(this)->mpSvxShape = NULL;
    }

    return mpSvxShape;
}

::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SdrObject::getUnoShape()
{
    // try weak reference first
    uno::Reference< uno::XInterface > xShape( getWeakUnoShape() );
    if( !xShape.is() )
    {
        OSL_ENSURE( mpSvxShape == NULL, "SdrObject::getUnoShape: XShape already dead, but still an IMPL pointer!" );
        SdrPage* pOwningPage = getSdrPageFromSdrObject();

        if ( pOwningPage )
        {
            uno::Reference< uno::XInterface > xPage( pOwningPage->getUnoPage() );
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
            mpSvxShape = SvxDrawPage::CreateShapeBySvxShapeKind(
                SdrObjectCreatorInventorToSvxShapeKind(GetObjIdentifier(), GetObjInventor()),
                this,
                0);
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

const basegfx::B2DHomMatrix& SdrObject::getSdrObjectTransformation() const
{
    return maSdrObjectTransformation.getB2DHomMatrix();
}

void SdrObject::setSdrObjectTransformation(const basegfx::B2DHomMatrix& rTransformation)
{
    // TTTT: Need to handle GluePoints, too. Check GluePoint transformation
    //SetGlueReallyAbsolute(true);
    //NbcShearGluePoints(rRef,nWink,tn,bVShear);
    //SetGlueReallyAbsolute(false);

    //if (GetGluePointList()!=NULL) {
    //  SdrGluePointList* pGPL=ForceGluePointList();
    //  pGPL->SetReallyAbsolute(true,*this);
    //  NbcShearGluePoints(rRef,nWink,tn,bVShear);
    //  pGPL->SetReallyAbsolute(false,*this);
    //}

    if(rTransformation != maSdrObjectTransformation.getB2DHomMatrix())
    {
        if(GetGluePointList())
        {
            ForceGluePointList()->TransformGluePoints(rTransformation, sdr::legacy::GetSnapRange(*this));
        }

        const SdrObjectChangeBroadcaster aSdrObjectChangeBroadcaster(*this);
        maSdrObjectTransformation.setB2DHomMatrix(rTransformation);
        SetChanged();
    }
}

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

const basegfx::B2DRange SdrObject::GetBLIPSizeRange() const
{
    if(mpPlusData)
    {
        return mpPlusData->maBLIPSizeRange;
    }
    else
    {
        return basegfx::B2DRange();
    }
}

void SdrObject::SetBLIPSizeRange(const basegfx::B2DRange& aRange)
{
    if(aRange != GetBLIPSizeRange())
    {
        ImpForcePlusData();
        mpPlusData->maBLIPSizeRange = aRange;
    }
}

void SdrObject::SetContextWritingMode( const sal_Int16 /*_nContextWritingMode*/ )
{
    // this base class does not support different writing modes, so ignore the call
}

::std::vector< SdrEdgeObj* > SdrObject::getAllConnectedSdrEdgeObj() const
{
    // travel over broadcaster/listener to access connected edges
    ::std::vector< SdrEdgeObj* > aRetval;
    const sal_uInt16 nListenerCount(GetListenerCount());

    for(sal_uInt16 nListener(0); nListener < nListenerCount; nListener++)
    {
        SdrEdgeObj* pEdge = dynamic_cast< SdrEdgeObj* >(GetListener(nListener));

        if(pEdge)
        {
            aRetval.push_back(pEdge);
        }
    }

    return aRetval;
}

//////////////////////////////////////////////////////////////////////////////
// eof
