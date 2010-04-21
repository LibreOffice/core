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

// #111111#
#include <svx/sdr/contact/viewcontactofpageobj.hxx>

////////////////////////////////////////////////////////////////////////////////////////////////////
// BaseProperties section

sdr::properties::BaseProperties* SdrPageObj::CreateObjectSpecificProperties()
{
    return new sdr::properties::PageProperties(*this);
}

//////////////////////////////////////////////////////////////////////////////
// DrawContact section

sdr::contact::ViewContact* SdrPageObj::CreateObjectSpecificViewContact()
{
    return new sdr::contact::ViewContactOfPageObj(*this);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
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

        mpShownPage = 0L;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

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
    // #111111#
    if(mpShownPage)
    {
        mpShownPage->RemovePageUser(*this);
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

UINT16 SdrPageObj::GetObjIdentifier() const
{
    return UINT16(OBJ_PAGE);
}

void SdrPageObj::TakeObjInfo(SdrObjTransformInfoRec& rInfo) const
{
    rInfo.bRotateFreeAllowed=FALSE;
    rInfo.bRotate90Allowed  =FALSE;
    rInfo.bMirrorFreeAllowed=FALSE;
    rInfo.bMirror45Allowed  =FALSE;
    rInfo.bMirror90Allowed  =FALSE;
    rInfo.bTransparenceAllowed = FALSE;
    rInfo.bGradientAllowed = FALSE;
    rInfo.bShearAllowed     =FALSE;
    rInfo.bEdgeRadiusAllowed=FALSE;
    rInfo.bNoOrthoDesired   =FALSE;
    rInfo.bCanConvToPath    =FALSE;
    rInfo.bCanConvToPoly    =FALSE;
    rInfo.bCanConvToPathLineToArea=FALSE;
    rInfo.bCanConvToPolyLineToArea=FALSE;
}

void SdrPageObj::operator=(const SdrObject& rObj)
{
    SdrObject::operator=(rObj);
    SetReferencedPage(((const SdrPageObj&)rObj).GetReferencedPage());
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

// eof
