/*************************************************************************
 *
 *  $RCSfile: svdopage.cxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 14:48:05 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "svdopage.hxx"
#include "svdglob.hxx"  // Stringcache
#include "svdstr.hrc"   // Objektname
#include "svdtrans.hxx"
#include "svdetc.hxx"
#include "svdio.hxx"
#include "svdxout.hxx"
#include "svdmodel.hxx"
#include "svdpage.hxx"
#include "svdpagv.hxx"
#include "svdoutl.hxx"

#ifndef INCLUDED_SVTOOLS_COLORCFG_HXX
#include <svtools/colorcfg.hxx>
#endif

#ifndef _SFXITEMSET_HXX
#include <svtools/itemset.hxx>
#endif

#ifndef _SDR_PROPERTIES_PAGEPROPERTIES_HXX
#include <svx/sdr/properties/pageproperties.hxx>
#endif

// #111111#
#ifndef _SDR_CONTACT_VIEWCONTACTOFPAGEOBJ_HXX
#include <svx/sdr/contact/viewcontactofpageobj.hxx>
#endif

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
        ActionChanged();
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

void SdrPageObj::WriteData(SvStream& rOut) const
{
    SdrObject::WriteData(rOut);
    SdrDownCompat aCompat(rOut,STREAM_WRITE); // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
#ifdef DBG_UTIL
    aCompat.SetID("SdrPageObj");
#endif

    // #111111#
    if(mpShownPage)
    {
        rOut << (sal_uInt16)mpShownPage->GetPageNum();
    }
    else
    {
        rOut << (sal_uInt16)0;
    }
}

void SdrPageObj::ReadData(const SdrObjIOHeader& rHead, SvStream& rIn)
{
    if (rIn.GetError()!=0) return;
    SdrObject::ReadData(rHead,rIn);
    SdrDownCompat aCompat(rIn,STREAM_READ); // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
#ifdef DBG_UTIL
    aCompat.SetID("SdrPageObj");
#endif

    // #111111#
    sal_uInt16 nPageNum;
    rIn >> nPageNum;

    if(GetModel())
    {
        SdrPage* pNewPage = GetModel()->GetPage(nPageNum);
        SetReferencedPage(pNewPage);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// eof
