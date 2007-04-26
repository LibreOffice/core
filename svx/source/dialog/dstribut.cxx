/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dstribut.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: rt $ $Date: 2007-04-26 07:30:23 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#ifdef SVX_DLLIMPLEMENTATION
#undef SVX_DLLIMPLEMENTATION
#endif

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
:   SfxSingleTabDialog(pParent, rInAttrs, RID_SVXPAGE_DISTRIBUTE ),
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
:   SvxTabPage(pWindow, SVX_RES(RID_SVXPAGE_DISTRIBUTE), rInAttrs),
    meDistributeHor(eHor),
    meDistributeVer(eVer),
    maFlHorizontal      (this, SVX_RES(FL_HORIZONTAL        )),
    maBtnHorNone        (this, SVX_RES(BTN_HOR_NONE     )),
    maBtnHorLeft        (this, SVX_RES(BTN_HOR_LEFT     )),
    maBtnHorCenter      (this, SVX_RES(BTN_HOR_CENTER       )),
    maBtnHorDistance    (this, SVX_RES(BTN_HOR_DISTANCE )),
    maBtnHorRight       (this, SVX_RES(BTN_HOR_RIGHT        )),
    maHorLow            (this, SVX_RES(IMG_HOR_LOW      )),
    maHorCenter         (this, SVX_RES(IMG_HOR_CENTER       )),
    maHorDistance       (this, SVX_RES(IMG_HOR_DISTANCE )),
    maHorHigh           (this, SVX_RES(IMG_HOR_HIGH     )),
    maFlVertical        (this, SVX_RES(FL_VERTICAL      )),
    maBtnVerNone        (this, SVX_RES(BTN_VER_NONE     )),
    maBtnVerTop         (this, SVX_RES(BTN_VER_TOP      )),
    maBtnVerCenter      (this, SVX_RES(BTN_VER_CENTER       )),
    maBtnVerDistance    (this, SVX_RES(BTN_VER_DISTANCE )),
    maBtnVerBottom      (this, SVX_RES(BTN_VER_BOTTOM       )),
    maVerLow            (this, SVX_RES(IMG_VER_LOW        )),
    maVerCenter         (this, SVX_RES(IMG_VER_CENTER       )),
    maVerDistance       (this, SVX_RES(IMG_VER_DISTANCE   )),
    maVerHigh           (this, SVX_RES(IMG_VER_HIGH       ))
{
    maHorLow.SetModeImage( Image( SVX_RES( IMG_HOR_LOW_H ) ), BMP_COLOR_HIGHCONTRAST );
    maHorCenter.SetModeImage( Image( SVX_RES( IMG_HOR_CENTER_H ) ), BMP_COLOR_HIGHCONTRAST );
    maHorDistance.SetModeImage( Image( SVX_RES( IMG_HOR_DISTANCE_H ) ), BMP_COLOR_HIGHCONTRAST );
    maHorHigh.SetModeImage( Image( SVX_RES( IMG_HOR_HIGH_H ) ), BMP_COLOR_HIGHCONTRAST );
    maVerDistance.SetModeImage( Image( SVX_RES( IMG_VER_DISTANCE_H ) ), BMP_COLOR_HIGHCONTRAST );
    maVerLow.SetModeImage( Image( SVX_RES( IMG_VER_LOW_H ) ), BMP_COLOR_HIGHCONTRAST );
    maVerCenter.SetModeImage( Image( SVX_RES( IMG_VER_CENTER_H ) ), BMP_COLOR_HIGHCONTRAST );
    maVerHigh.SetModeImage( Image( SVX_RES( IMG_VER_HIGH_H ) ), BMP_COLOR_HIGHCONTRAST );

    FreeResource();
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

void SvxDistributePage::PointChanged(Window* /*pWindow*/, RECT_POINT /*eRP*/)
{
}

/*************************************************************************
|*
|* read the delivered Item-Set
|*
\************************************************************************/

void __EXPORT SvxDistributePage::Reset(const SfxItemSet& )
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

BOOL SvxDistributePage::FillItemSet( SfxItemSet& )
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


