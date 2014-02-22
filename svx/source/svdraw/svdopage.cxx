/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include <svx/svdopage.hxx>
#include "svx/svdglob.hxx"
#include "svx/svdstr.hrc"
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




sdr::properties::BaseProperties* SdrPageObj::CreateObjectSpecificProperties()
{
    return new sdr::properties::PageProperties(*this);
}




sdr::contact::ViewContact* SdrPageObj::CreateObjectSpecificViewContact()
{
    return new sdr::contact::ViewContactOfPageObj(*this);
}





void SdrPageObj::PageInDestruction(const SdrPage& rPage)
{
    if(mpShownPage && mpShownPage == &rPage)
    {
        
        
        

        mpShownPage = 0L;
    }
}



TYPEINIT1(SdrPageObj,SdrObject);

SdrPageObj::SdrPageObj(SdrPage* pNewPage)
:   mpShownPage(pNewPage)
{
    if(mpShownPage)
    {
        mpShownPage->AddPageUser(*this);
    }
}

SdrPageObj::SdrPageObj(const Rectangle& rRect, SdrPage* pNewPage)
:   mpShownPage(pNewPage)
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

SdrPage* SdrPageObj::GetReferencedPage() const
{
    return mpShownPage;
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


void SdrPageObj::SetBoundRectDirty()
{
    
    
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
    rInfo.bGradientAllowed = false;
    rInfo.bShearAllowed     =false;
    rInfo.bEdgeRadiusAllowed=false;
    rInfo.bNoOrthoDesired   =false;
    rInfo.bCanConvToPath    =false;
    rInfo.bCanConvToPoly    =false;
    rInfo.bCanConvToPathLineToArea=false;
    rInfo.bCanConvToPolyLineToArea=false;
}

SdrPageObj* SdrPageObj::Clone() const
{
    return CloneHelper< SdrPageObj >();
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
    OUStringBuffer sName(ImpGetResStr(STR_ObjNameSingulPAGE));

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
    return ImpGetResStr(STR_ObjNamePluralPAGE);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
