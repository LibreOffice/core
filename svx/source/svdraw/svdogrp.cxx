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
#include <sfx2/linkmgr.hxx>

#include <ucbhelper/content.hxx>
#include <ucbhelper/contentbroker.hxx>
#include <unotools/datetime.hxx>
#include <svx/svdogrp.hxx>
#include <sfx2/lnkbase.hxx>
#include <tools/urlobj.hxx>
#include <svl/urihelper.hxx>
#include <svx/xpool.hxx>
#include <svx/xpoly.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdpage.hxx>
#include <svx/svditer.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdtrans.hxx>
#include <svx/svdetc.hxx>
#include <svx/svdattrx.hxx>  // NotPersistItems
#include <svx/svdoedge.hxx>  // #32383# Die Verbinder nach Move nochmal anbroadcasten
#include <svx/svdglob.hxx>   // StringCache
#include <svx/svdstr.hrc>    // Objektname

#include <svx/svxids.hrc>
#include <svl/whiter.hxx>
#include <svx/svdpool.hxx>
#include <svx/sdr/properties/groupproperties.hxx>
#include <svx/sdr/contact/viewcontactofgroup.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <svx/svdlegacy.hxx>

//////////////////////////////////////////////////////////////////////////////

sdr::properties::BaseProperties* SdrObjGroup::CreateObjectSpecificProperties()
{
    return new sdr::properties::GroupProperties(*this);
}

sdr::contact::ViewContact* SdrObjGroup::CreateObjectSpecificViewContact()
{
    return new sdr::contact::ViewContactOfGroup(*this);
}

SdrObjGroup::SdrObjGroup(SdrModel& rSdrModel)
:   SdrObject(rSdrModel),
    SdrObjList()
{
}

SdrObjGroup::~SdrObjGroup()
{
    if(GetObjCount())
    {
        // cannot be called in SdrObjList::ClearSdrObjList() where it originally was
        // since there it would be a pure virtual function call. Needs to be called
        // in all destructors of classes derived from SdrObjList
        getSdrModelFromSdrObjList().SetChanged();
    }
}

void SdrObjGroup::copyDataFromSdrObject(const SdrObject& rSource)
{
    if(this != &rSource)
    {
        const SdrObjGroup* pSource = dynamic_cast< const SdrObjGroup* >(&rSource);

        if(pSource)
        {
            // call parent
            SdrObject::copyDataFromSdrObject(rSource);

            // copy SubList
            copyDataFromSdrObjList(*pSource);
        }
        else
        {
            OSL_ENSURE(false, "copyDataFromSdrObject with ObjectType of Source different from Target (!)");
        }
    }
}

SdrObject* SdrObjGroup::CloneSdrObject(SdrModel* pTargetModel) const
{
    SdrObjGroup* pClone = new SdrObjGroup(
        pTargetModel ? *pTargetModel : getSdrModelFromSdrObject());
    OSL_ENSURE(pClone, "CloneSdrObject error (!)");
    pClone->copyDataFromSdrObject(*this);

    return pClone;
}

SdrPage* SdrObjGroup::getSdrPageFromSdrObjList() const
{
    return getSdrPageFromSdrObject();
}

SdrObject* SdrObjGroup::getSdrObjectFromSdrObjList() const
{
    return const_cast< SdrObjGroup* >(this);
}

SdrModel& SdrObjGroup::getSdrModelFromSdrObjList() const
{
    return getSdrModelFromSdrObject();
}

void SdrObjGroup::handleContentChange(const SfxHint& rHint)
{
    // call parent
    SdrObjList::handleContentChange(rHint);

    // needed object updates
    SetChanged();

    // reset local transformation to allow on-demand recalculation
    maSdrObjectTransformation.setB2DHomMatrix(basegfx::B2DHomMatrix());
}

SdrObjList* SdrObjGroup::getChildrenOfSdrObject() const
{
    return const_cast< SdrObjGroup* >(this);
}

void SdrObjGroup::TakeObjInfo(SdrObjTransformInfoRec& rInfo) const
{
    rInfo.mbNoContortion = false;
    const sal_uInt32 nObjAnz(GetObjCount());

    for(sal_uInt32 i(0); i < nObjAnz; i++)
    {
        SdrObject* pObj = GetObj(i);
        SdrObjTransformInfoRec aInfo;
        pObj->TakeObjInfo(aInfo);

        if(!aInfo.mbMoveAllowed)
        {
            rInfo.mbMoveAllowed = false;
        }

        if(!aInfo.mbResizeFreeAllowed)
        {
            rInfo.mbResizeFreeAllowed = false;
        }

        if(!aInfo.mbResizePropAllowed)
        {
            rInfo.mbResizePropAllowed = false;
        }

        if(!aInfo.mbRotateFreeAllowed)
        {
            rInfo.mbRotateFreeAllowed = false;
        }

        if(!aInfo.mbRotate90Allowed)
        {
            rInfo.mbRotate90Allowed = false;
        }

        if(!aInfo.mbMirrorFreeAllowed)
        {
            rInfo.mbMirrorFreeAllowed = false;
        }

        if(!aInfo.mbMirror45Allowed)
        {
            rInfo.mbMirror45Allowed = false;
        }

        if(!aInfo.mbMirror90Allowed)
        {
            rInfo.mbMirror90Allowed = false;
        }

        if(!aInfo.mbShearAllowed)
        {
            rInfo.mbShearAllowed = false;
        }

        if(!aInfo.mbEdgeRadiusAllowed)
        {
            rInfo.mbEdgeRadiusAllowed = false;
        }

        if(!aInfo.mbNoOrthoDesired)
        {
            rInfo.mbNoOrthoDesired = false;
        }

        if(aInfo.mbNoContortion)
        {
            rInfo.mbNoContortion = true;
        }

        if(!aInfo.mbCanConvToPath)
        {
            rInfo.mbCanConvToPath = false;
        }

        if(!aInfo.mbCanConvToContour)
        {
            rInfo.mbCanConvToContour = false;
        }

        if(!aInfo.mbCanConvToPoly)
        {
            rInfo.mbCanConvToPoly = false;
        }

        if(!aInfo.mbCanConvToPathLineToArea)
        {
            rInfo.mbCanConvToPathLineToArea = false;
        }

        if(!aInfo.mbCanConvToPolyLineToArea)
        {
            rInfo.mbCanConvToPolyLineToArea = false;
        }
    }

    if(!nObjAnz)
    {
        rInfo.mbRotateFreeAllowed = false;
        rInfo.mbRotate90Allowed = false;
        rInfo.mbMirrorFreeAllowed = false;
        rInfo.mbMirror45Allowed = false;
        rInfo.mbMirror90Allowed = false;
        rInfo.mbTransparenceAllowed = false;
        rInfo.mbGradientAllowed = false;
        rInfo.mbShearAllowed = false;
        rInfo.mbEdgeRadiusAllowed = false;
        rInfo.mbNoContortion = true;
    }

    if(1 != nObjAnz)
    {
        // only allowed if single object selected
        rInfo.mbTransparenceAllowed = false;
        rInfo.mbGradientAllowed = false;
    }
}

sal_uInt16 SdrObjGroup::GetObjIdentifier() const
{
    return sal_uInt16(OBJ_GRUP);
}

SdrLayerID SdrObjGroup::GetLayer() const
{
    bool b1st(true);
    SdrLayerID nLay(SdrObject::GetLayer());
    const sal_uInt32 nObjAnz(GetObjCount());

    for(sal_uInt32 i(0); i < nObjAnz; i++)
    {
        SdrLayerID nLay1 = GetObj(i)->GetLayer();

        if(b1st)
        {
            nLay = nLay1;
            b1st = false;
        }
        else if(nLay1 != nLay)
        {
            return 0;
        }
    }

    return nLay;
}

void SdrObjGroup::SetLayer(SdrLayerID nLayer)
{
    SdrObject::SetLayer(nLayer);
    const sal_uInt32 nObjAnz(GetObjCount());

    for(sal_uInt32 i(0); i < nObjAnz; i++)
    {
        GetObj(i)->SetLayer(nLayer);
    }
}

void SdrObjGroup::handlePageChange(SdrPage* pOldPage, SdrPage* pNewPage)
{
    if(pOldPage != pNewPage)
    {
        // call parent
        SdrObject::handlePageChange(pOldPage, pNewPage);

        for(sal_uInt32 i(0); i < GetObjCount(); i++)
        {
            GetObj(i)->handlePageChange(pOldPage, pNewPage);
        }
    }
}

void SdrObjGroup::TakeObjNameSingul(XubString& rName) const
{
    if(!GetObjCount())
    {
        rName = ImpGetResStr(STR_ObjNameSingulGRUPEMPTY);
    }
    else
    {
        rName = ImpGetResStr(STR_ObjNameSingulGRUP);
    }

    const String aName(GetName());

    if(aName.Len())
    {
        rName += sal_Unicode(' ');
        rName += sal_Unicode('\'');
        rName += aName;
        rName += sal_Unicode('\'');
    }
}

void SdrObjGroup::TakeObjNamePlural(XubString& rName) const
{
    if(!GetObjCount())
    {
        rName = ImpGetResStr(STR_ObjNamePluralGRUPEMPTY);
    }
    else
    {
        rName = ImpGetResStr(STR_ObjNamePluralGRUP);
    }
}

basegfx::B2DPolyPolygon SdrObjGroup::TakeXorPoly() const
{
    basegfx::B2DPolyPolygon aRetval;
    const sal_uInt32 nObjCount(GetObjCount());

    for(sal_uInt32 a(0); a < nObjCount; a++)
    {
        SdrObject* pObj = GetObj(a);
        aRetval.append(pObj->TakeXorPoly());
    }

    if(!aRetval.count())
    {
        const basegfx::B2DRange aSnapRange(sdr::legacy::GetAllObjSnapRange(getSdrObjectVector()));

        aRetval.append(basegfx::tools::createPolygonFromRect(aSnapRange));
    }

    return aRetval;
}

bool SdrObjGroup::beginSpecialDrag(SdrDragStat& /*rDrag*/) const
{
    return false;
}

bool SdrObjGroup::BegCreate(SdrDragStat& /*rStat*/)
{
    // do not construct empty groups interactively
    return false;
}

const basegfx::B2DHomMatrix& SdrObjGroup::getSdrObjectTransformation() const
{
    const sal_uInt32 nCount(GetObjCount());

    if(nCount)
    {
        // transformation of the group is it's size (scale) and position (translation)
        // of all sub-objects combined. To not always create this, use isIdentity() as
        // hint for recalculation
        if(maSdrObjectTransformation.getB2DHomMatrix().isIdentity())
        {
            const basegfx::B2DRange aSnapRange(sdr::legacy::GetAllObjSnapRange(getSdrObjectVector()));

            const_cast< SdrObjGroup* >(this)->maSdrObjectTransformation.setB2DHomMatrix(
                basegfx::tools::createScaleTranslateB2DHomMatrix(
                    aSnapRange.getRange(),
                    aSnapRange.getMinimum()));
        }
    }
    else
    {
        // keep and return the last known transformation when there is no content,
        // so just do nothing
    }

    // call parent
    return SdrObject::getSdrObjectTransformation();
}

void SdrObjGroup::setSdrObjectTransformation(const basegfx::B2DHomMatrix& rTransformation)
{
    const basegfx::B2DHomMatrix& rCurrent = getSdrObjectTransformation();

    if(rTransformation != rCurrent)
    {
        const sal_uInt32 nCount(GetObjCount());

        if(nCount)
        {
            // to apply the transformation to sub-objects, remove current transformation
            // by using it's inverse, then transform by the new transformation. Prepare
            // inverse combined with transformation
            basegfx::B2DHomMatrix aTransform(rCurrent);

            aTransform.invert();
            aTransform = rTransformation * aTransform;

            // apply to all sub-objects
            for(sal_uInt32 a(0); a < nCount; a++)
            {
                SdrObject* pCandidate = GetObj(a);
                pCandidate->setSdrObjectTransformation(aTransform * pCandidate->getSdrObjectTransformation());
            }

            // call parent; needed to trigger invalidateObjectRange and others
            SdrObject::setSdrObjectTransformation(rTransformation);

            // reset local transformation since it's a sum-up of the content
            maSdrObjectTransformation.setB2DHomMatrix(basegfx::B2DHomMatrix());
        }
        else
        {
            // no sub-objects; set directly
            SdrObject::setSdrObjectTransformation(rTransformation);
        }
    }
}

void SdrObjGroup::SetAnchorPos(const basegfx::B2DPoint& rPnt)
{
    SdrObject::SetAnchorPos(rPnt);

    for(sal_uInt32 a(0); a < GetObjCount(); a++)
    {
        GetObj(a)->SetAnchorPos(rPnt);
    }
}

void SdrObjGroup::ReformatText()
{
    SdrObject::ReformatText();

    for(sal_uInt32 a(0); a < GetObjCount(); a++)
    {
        GetObj(a)->ReformatText();
    }
}

SdrObject* SdrObjGroup::DoConvertToPolygonObject(bool bBezier, bool bAddText) const
{
    SdrObjGroup* pGroup = new SdrObjGroup(getSdrModelFromSdrObject());

    for(sal_uInt32 a(0); a < GetObjCount(); a++)
    {
        SdrObject* pIterObj = GetObj(a);
        SdrObject* pResult = pIterObj->DoConvertToPolygonObject(bBezier, bAddText);

        // pResult can be NULL e.g. for empty objects
        if(pResult)
        {
            pGroup->InsertObjectToSdrObjList(*pResult);
        }
    }

    return pGroup;
}

void SdrObjGroup::getMergedHierarchyLayerSet(SetOfByte& rSet) const
{
    SdrObject::getMergedHierarchyLayerSet(rSet);
    const sal_uInt32 nObjAnz(GetObjCount());

    for(sal_uInt32 nObjNum(0); nObjNum < nObjAnz; nObjNum++)
    {
        GetObj(nObjNum)->getMergedHierarchyLayerSet(rSet);
    }
}

//////////////////////////////////////////////////////////////////////////////
// eof
