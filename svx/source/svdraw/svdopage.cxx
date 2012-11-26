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

#include <svx/svdopage.hxx>
#include "svx/svdglob.hxx"  // Stringcache
#include "svx/svdstr.hrc"   // Objektname
#include <svx/svdtrans.hxx>
#include <svx/svdetc.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdoutl.hxx>
#include <svtools/colorcfg.hxx>
#include <svl/itemset.hxx>
#include <svx/sdr/properties/pageproperties.hxx>
#include <svx/sdr/contact/viewcontactofpageobj.hxx>
#include <svl/smplhint.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>

////////////////////////////////////////////////////////////////////////////////////////////////////

sdr::properties::BaseProperties* SdrPageObj::CreateObjectSpecificProperties()
{
    return new sdr::properties::PageProperties(*this);
}

sdr::contact::ViewContact* SdrPageObj::CreateObjectSpecificViewContact()
{
    return new sdr::contact::ViewContactOfPageObj(*this);
}

SdrPageObj::SdrPageObj(SdrModel& rSdrModel, const basegfx::B2DHomMatrix& rTransform, SdrPage* pNewPage)
:   SdrObject(rSdrModel, rTransform),
    mpShownPage(pNewPage)
{
    if(mpShownPage)
    {
        mpShownPage->AddListener(*this);
    }
}

SdrPageObj::~SdrPageObj()
{
    if(mpShownPage)
    {
        mpShownPage->RemoveListener(*this);
    }
}

void SdrPageObj::copyDataFromSdrObject(const SdrObject& rSource)
{
    if(this != &rSource)
    {
        const SdrPageObj* pSource = dynamic_cast< const SdrPageObj* >(&rSource);

        if(pSource)
        {
            // call parent
            SdrObject::copyDataFromSdrObject(rSource);

            // copy SdrPage reference
            SetReferencedPage(pSource->GetReferencedPage());
        }
        else
        {
            OSL_ENSURE(false, "copyDataFromSdrObject with ObjectType of Source different from Target (!)");
        }
    }
}

SdrObject* SdrPageObj::CloneSdrObject(SdrModel* pTargetModel) const
{
    SdrPageObj* pClone = new SdrPageObj(
        pTargetModel ? *pTargetModel : getSdrModelFromSdrObject());
    OSL_ENSURE(pClone, "CloneSdrObject error (!)");
    pClone->copyDataFromSdrObject(*this);

    return pClone;
}

// derived from SfxListener
void SdrPageObj::Notify(SfxBroadcaster& rBC, const SfxHint& rHint)
{
    // call parent
    SdrObject::Notify(rBC, rHint);

    if(mpShownPage)
    {
        const SdrBaseHint* pSdrHint = dynamic_cast< const SdrBaseHint* >(&rHint);

        if(pSdrHint
            && HINT_SDRPAGEDYING == pSdrHint->GetSdrHintKind()
            && pSdrHint->GetSdrHintPage()
            && pSdrHint->GetSdrHintPage() == mpShownPage)
        {
            // #i58769# Do not call ActionChanged() here, because that would
            // lead to the construction of a view contact object for a page that
            // is being destroyed.
            mpShownPage = 0;
        }
    }
}

// #111111#
SdrPage* SdrPageObj::GetReferencedPage() const
{
    return mpShownPage;
}

// #111111#
void SdrPageObj::SetReferencedPage(SdrPage* pNewPage)
{
    if(mpShownPage != pNewPage)
    {
        const SdrObjectChangeBroadcaster aSdrObjectChangeBroadcaster(*this);

        if(mpShownPage)
        {
            mpShownPage->RemoveListener(*this);
        }

        mpShownPage = pNewPage;

        if(mpShownPage)
        {
            mpShownPage->AddListener(*this);
        }

        SetChanged();
    }
}

sal_uInt16 SdrPageObj::GetObjIdentifier() const
{
    return sal_uInt16(OBJ_PAGE);
}

void SdrPageObj::TakeObjInfo(SdrObjTransformInfoRec& rInfo) const
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
    rInfo.mbNoOrthoDesired = false;
    rInfo.mbCanConvToPath = false;
    rInfo.mbCanConvToPoly = false;
    rInfo.mbCanConvToPathLineToArea = false;
    rInfo.mbCanConvToPolyLineToArea = false;
}

void SdrPageObj::TakeObjNameSingul(XubString& rName) const
{
    rName=ImpGetResStr(STR_ObjNameSingulPAGE);

    String aName( GetName() );
    if(aName.Len())
    {
        rName += sal_Unicode(' ');
        rName += sal_Unicode('\'');
        rName += aName;
        rName += sal_Unicode('\'');
    }
}

void SdrPageObj::TakeObjNamePlural(XubString& rName) const
{
    rName=ImpGetResStr(STR_ObjNamePluralPAGE);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// eof
