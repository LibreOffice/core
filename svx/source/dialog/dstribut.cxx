/*************************************************************************
 *
 *  $RCSfile: dstribut.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:08 $
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

#ifndef _BASEDLGS_HXX
#include <sfx2/basedlgs.hxx>
#endif

#include "dialogs.hrc"
#include "dstribut.hxx"
#include "dstribut.hrc"

#include "svddef.hxx"
#include "dialmgr.hxx"

#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif

static USHORT pRanges[] =
{
    SDRATTR_MEASURE_FIRST,
    SDRATTR_MEASURE_LAST,
    0
};

/*************************************************************************
|*
|* Dialog
|*
\************************************************************************/

SvxDistributeDialog::SvxDistributeDialog(
    Window* pParent,
    const SfxItemSet& rInAttrs,
    SvxDistributeHorizontal eHor,
    SvxDistributeVertical eVer)
:   SfxSingleTabDialog(pParent, rInAttrs, RID_SVXPAGE_DISTRIBUTE, FALSE),
    mpPage(0L)
{
    mpPage = new SvxDistributePage(this, rInAttrs, eHor, eVer);
    SetTabPage(mpPage);
    SetText(mpPage->GetText());
}

/*************************************************************************
|*
|* Dtor
|*
\************************************************************************/

SvxDistributeDialog::~SvxDistributeDialog()
{
}

/*************************************************************************
|*
|* Tabpage
|*
\************************************************************************/

SvxDistributePage::SvxDistributePage(
    Window* pWindow,
    const SfxItemSet& rInAttrs,
    SvxDistributeHorizontal eHor,
    SvxDistributeVertical eVer)
:   SvxTabPage(pWindow, ResId(RID_SVXPAGE_DISTRIBUTE, DIALOG_MGR()), rInAttrs),
    meDistributeHor(eHor),
    meDistributeVer(eVer),
    maGrpHorizontal     (this, ResId(GRP_HORIZONTAL     )),
    maBtnHorNone        (this, ResId(BTN_HOR_NONE       )),
    maBtnHorLeft        (this, ResId(BTN_HOR_LEFT       )),
    maBtnHorCenter      (this, ResId(BTN_HOR_CENTER     )),
    maBtnHorDistance    (this, ResId(BTN_HOR_DISTANCE   )),
    maBtnHorRight       (this, ResId(BTN_HOR_RIGHT      )),
    maHorLow            (this, ResId(IMG_HOR_LOW        )),
    maHorCenter         (this, ResId(IMG_HOR_CENTER     )),
    maHorDistance       (this, ResId(IMG_HOR_DISTANCE   )),
    maHorHigh           (this, ResId(IMG_HOR_HIGH       )),
    maGrpVertical       (this, ResId(GRP_VERTICAL       )),
    maBtnVerNone        (this, ResId(BTN_VER_NONE       )),
    maBtnVerTop         (this, ResId(BTN_VER_TOP        )),
    maBtnVerCenter      (this, ResId(BTN_VER_CENTER     )),
    maBtnVerDistance    (this, ResId(BTN_VER_DISTANCE   )),
    maBtnVerBottom      (this, ResId(BTN_VER_BOTTOM     )),
    maVerDistance       (this, ResId(IMG_VER_DISTANCE   )),
    maVerLow            (this, ResId(IMG_VER_LOW        )),
    maVerCenter         (this, ResId(IMG_VER_CENTER     )),
    maVerHigh           (this, ResId(IMG_VER_HIGH       )),
    maGrpPreview        (this, ResId(GRP_PREVIEW        ))
{
    FreeResource();

    maGrpPreview.Hide();
}

/*************************************************************************
|*
|* Dtor
|*
\************************************************************************/

SvxDistributePage::~SvxDistributePage()
{
}

/*************************************************************************
|*
|* create the tabpage
|*
\************************************************************************/

SfxTabPage* SvxDistributePage::Create(Window* pWindow, const SfxItemSet& rAttrs,
    SvxDistributeHorizontal eHor, SvxDistributeVertical eVer)
{
    return(new SvxDistributePage(pWindow, rAttrs, eHor, eVer));
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

UINT16* SvxDistributePage::GetRanges()
{
    return(pRanges);
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

void SvxDistributePage::PointChanged(Window* pWindow, RECT_POINT eRP)
{
}

/*************************************************************************
|*
|* read the delivered Item-Set
|*
\************************************************************************/

void __EXPORT SvxDistributePage::Reset(const SfxItemSet& rAttrs)
{
    maBtnHorNone.SetState(FALSE);
    maBtnHorLeft.SetState(FALSE);
    maBtnHorCenter.SetState(FALSE);
    maBtnHorDistance.SetState(FALSE);
    maBtnHorRight.SetState(FALSE);

    switch(meDistributeHor)
    {
        case SvxDistributeHorizontalNone : maBtnHorNone.SetState(TRUE); break;
        case SvxDistributeHorizontalLeft : maBtnHorLeft.SetState(TRUE); break;
        case SvxDistributeHorizontalCenter : maBtnHorCenter.SetState(TRUE); break;
        case SvxDistributeHorizontalDistance : maBtnHorDistance.SetState(TRUE); break;
        case SvxDistributeHorizontalRight : maBtnHorRight.SetState(TRUE); break;
    }

    maBtnVerNone.SetState(FALSE);
    maBtnVerTop.SetState(FALSE);
    maBtnVerCenter.SetState(FALSE);
    maBtnVerDistance.SetState(FALSE);
    maBtnVerBottom.SetState(FALSE);

    switch(meDistributeVer)
    {
        case SvxDistributeVerticalNone : maBtnVerNone.SetState(TRUE); break;
        case SvxDistributeVerticalTop : maBtnVerTop.SetState(TRUE); break;
        case SvxDistributeVerticalCenter : maBtnVerCenter.SetState(TRUE); break;
        case SvxDistributeVerticalDistance : maBtnVerDistance.SetState(TRUE); break;
        case SvxDistributeVerticalBottom : maBtnVerBottom.SetState(TRUE); break;
    }
}

/*************************************************************************
|*
|* Fill the delivered Item-Set with dialogbox-attributes
|*
\************************************************************************/

BOOL SvxDistributePage::FillItemSet( SfxItemSet& rAttrs)
{
    SvxDistributeHorizontal eDistributeHor(SvxDistributeHorizontalNone);
    SvxDistributeVertical eDistributeVer(SvxDistributeVerticalNone);

    if(maBtnHorLeft.IsChecked())
        eDistributeHor = SvxDistributeHorizontalLeft;
    else if(maBtnHorCenter.IsChecked())
        eDistributeHor = SvxDistributeHorizontalCenter;
    else if(maBtnHorDistance.IsChecked())
        eDistributeHor = SvxDistributeHorizontalDistance;
    else if(maBtnHorRight.IsChecked())
        eDistributeHor = SvxDistributeHorizontalRight;

    if(maBtnVerTop.IsChecked())
        eDistributeVer = SvxDistributeVerticalTop;
    else if(maBtnVerCenter.IsChecked())
        eDistributeVer = SvxDistributeVerticalCenter;
    else if(maBtnVerDistance.IsChecked())
        eDistributeVer = SvxDistributeVerticalDistance;
    else if(maBtnVerBottom.IsChecked())
        eDistributeVer = SvxDistributeVerticalBottom;

    if(eDistributeHor != meDistributeHor || eDistributeVer != meDistributeVer)
    {
        meDistributeHor = eDistributeHor;
        meDistributeVer = eDistributeVer;
        return TRUE;
    }

    return FALSE;
}


