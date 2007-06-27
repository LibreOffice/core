/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dstribut.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2007-06-27 17:04:09 $
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
#ifndef _SVX_DSTRIBUT_HXX
#define _SVX_DSTRIBUT_HXX

#ifndef _SVX_DLG_CTRL_HXX
#include <svx/dlgctrl.hxx>
#endif

#ifndef _SV_GROUP_HXX
#include <vcl/group.hxx>
#endif

#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif

#ifndef _SVX_DSTRIBUT_ENUM_HXX
#include <svx/dstribut_enum.hxx> //CHINA001
#endif
/*************************************************************************
|*
\************************************************************************/

/*
enum SvxDistributeHorizontal
{
    SvxDistributeHorizontalNone = 0,
    SvxDistributeHorizontalLeft,
    SvxDistributeHorizontalCenter,
    SvxDistributeHorizontalDistance,
    SvxDistributeHorizontalRight
};

enum SvxDistributeVertical
{
    SvxDistributeVerticalNone = 0,
    SvxDistributeVerticalTop,
    SvxDistributeVerticalCenter,
    SvxDistributeVerticalDistance,
    SvxDistributeVerticalBottom
};
*/

/*************************************************************************
|*
\************************************************************************/

class SvxDistributePage : public SvxTabPage
{
    SvxDistributeHorizontal     meDistributeHor;
    SvxDistributeVertical       meDistributeVer;

    FixedLine                   maFlHorizontal      ;
    RadioButton                 maBtnHorNone        ;
    RadioButton                 maBtnHorLeft        ;
    RadioButton                 maBtnHorCenter      ;
    RadioButton                 maBtnHorDistance    ;
    RadioButton                 maBtnHorRight       ;
    FixedImage                  maHorLow            ;
    FixedImage                  maHorCenter         ;
    FixedImage                  maHorDistance       ;
    FixedImage                  maHorHigh           ;
    FixedLine                   maFlVertical        ;
    RadioButton                 maBtnVerNone        ;
    RadioButton                 maBtnVerTop         ;
    RadioButton                 maBtnVerCenter      ;
    RadioButton                 maBtnVerDistance    ;
    RadioButton                 maBtnVerBottom      ;
    FixedImage                  maVerLow            ;
    FixedImage                  maVerCenter         ;
    FixedImage                  maVerDistance       ;
    FixedImage                  maVerHigh           ;

public:
    SvxDistributePage(Window* pWindow, const SfxItemSet& rInAttrs,
        SvxDistributeHorizontal eHor = SvxDistributeHorizontalNone,
        SvxDistributeVertical eVer = SvxDistributeVerticalNone);
    ~SvxDistributePage();

    static SfxTabPage* Create(Window*, const SfxItemSet&,
        SvxDistributeHorizontal eHor, SvxDistributeVertical eVer);
    static UINT16* GetRanges();
    virtual BOOL FillItemSet(SfxItemSet&);
    virtual void Reset(const SfxItemSet&);
    virtual void PointChanged(Window* pWindow, RECT_POINT eRP);

    SvxDistributeHorizontal GetDistributeHor() const { return meDistributeHor; }
    SvxDistributeVertical GetDistributeVer() const { return meDistributeVer; }
};

/*************************************************************************
|*
\************************************************************************/

class SvxDistributeDialog : public SfxSingleTabDialog
{
    SvxDistributePage*          mpPage;

public:
    SvxDistributeDialog(Window* pParent, const SfxItemSet& rAttr,
        SvxDistributeHorizontal eHor = SvxDistributeHorizontalNone,
        SvxDistributeVertical eVer = SvxDistributeVerticalNone);
    ~SvxDistributeDialog();

    SvxDistributeHorizontal GetDistributeHor() const { return mpPage->GetDistributeHor(); }
    SvxDistributeVertical GetDistributeVer() const { return mpPage->GetDistributeVer(); }
};


#endif // _SVX_DSTRIBUT_HXX

