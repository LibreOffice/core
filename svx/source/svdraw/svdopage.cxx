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


#include <svx/svdopage.hxx>
#include <svx/dialmgr.hxx>
#include <svx/strings.hrc>
#include <svx/svdtrans.hxx>
#include <svx/svdetc.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdoutl.hxx>
#include <svtools/colorcfg.hxx>
#include <svl/itemset.hxx>
#include <sdr/properties/pageproperties.hxx>
#include <svx/sdr/contact/viewcontactofpageobj.hxx>


// BaseProperties section

std::unique_ptr<sdr::properties::BaseProperties> SdrPageObj::CreateObjectSpecificProperties()
{
    return std::make_unique<sdr::properties::PageProperties>(*this);
}


// DrawContact section

std::unique_ptr<sdr::contact::ViewContact> SdrPageObj::CreateObjectSpecificViewContact()
{
    return std::make_unique<sdr::contact::ViewContactOfPageObj>(*this);
}


// this method is called form the destructor of the referenced page.
// do all necessary action to forget the page. It is not necessary to call
// RemovePageUser(), that is done form the destructor.
void SdrPageObj::PageInDestruction(const SdrPage& rPage)
{
    if(mpShownPage && mpShownPage == &rPage)
    {
        // #i58769# Do not call ActionChanged() here, because that would
        // lead to the construction of a view contact object for a page that
        // is being destroyed.

        mpShownPage = nullptr;
    }
}

SdrPageObj::SdrPageObj(
    SdrModel& rSdrModel,
    SdrPage* pNewPage)
:   SdrObject(rSdrModel),
    mpShownPage(pNewPage)
{
    if(mpShownPage)
    {
        mpShownPage->AddPageUser(*this);
    }
}

SdrPageObj::SdrPageObj(
    SdrModel& rSdrModel,
    const tools::Rectangle& rRect,
    SdrPage* pNewPage)
:   SdrObject(rSdrModel),
    mpShownPage(pNewPage)
{
    if(mpShownPage)
    {
        mpShownPage->AddPageUser(*this);
    }

    aOutRect = rRect;
}

SdrPageObj::~SdrPageObj()
{
    if(mpShownPage)
    {
        mpShownPage->RemovePageUser(*this);
    }
}


void SdrPageObj::SetReferencedPage(SdrPage* pNewPage)
{
    if(mpShownPage != pNewPage)
    {
        if(mpShownPage)
        {
            mpShownPage->RemovePageUser(*this);
        }

        mpShownPage = pNewPage;

        if(mpShownPage)
        {
            mpShownPage->AddPageUser(*this);
        }

        SetChanged();
        BroadcastObjectChange();
    }
}

// #i96598#
void SdrPageObj::SetBoundRectDirty()
{
    // avoid resetting aOutRect which in case of this object is model data,
    // not re-creatable view data
}

sal_uInt16 SdrPageObj::GetObjIdentifier() const
{
    return sal_uInt16(OBJ_PAGE);
}

void SdrPageObj::TakeObjInfo(SdrObjTransformInfoRec& rInfo) const
{
    rInfo.bRotateFreeAllowed=false;
    rInfo.bRotate90Allowed  =false;
    rInfo.bMirrorFreeAllowed=false;
    rInfo.bMirror45Allowed  =false;
    rInfo.bMirror90Allowed  =false;
    rInfo.bTransparenceAllowed = false;
    rInfo.bShearAllowed     =false;
    rInfo.bEdgeRadiusAllowed=false;
    rInfo.bNoOrthoDesired   =false;
    rInfo.bCanConvToPath    =false;
    rInfo.bCanConvToPoly    =false;
    rInfo.bCanConvToPathLineToArea=false;
    rInfo.bCanConvToPolyLineToArea=false;
}

SdrPageObj* SdrPageObj::CloneSdrObject(SdrModel& rTargetModel) const
{
    return CloneHelper< SdrPageObj >(rTargetModel);
}

SdrPageObj& SdrPageObj::operator=(const SdrPageObj& rObj)
{
    if( this == &rObj )
        return *this;
    SdrObject::operator=(rObj);
    SetReferencedPage( rObj.GetReferencedPage());
    return *this;
}

OUString SdrPageObj::TakeObjNameSingul() const
{
    OUStringBuffer sName(SvxResId(STR_ObjNameSingulPAGE));

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

OUString SdrPageObj::TakeObjNamePlural() const
{
    return SvxResId(STR_ObjNamePluralPAGE);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
