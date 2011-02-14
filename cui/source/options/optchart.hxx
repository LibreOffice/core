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

#ifndef _SVX_OPTCHART_HXX
#define _SVX_OPTCHART_HXX

// header for SfxTabPage
#include <sfx2/tabdlg.hxx>
#include <vcl/fixed.hxx>
// header for ValueSet
#include <svtools/valueset.hxx>
// header for ColorLB
#include <svx/dlgctrl.hxx>
// header for PushButton
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
// header for XColorTable
#include <svx/xtable.hxx>

#include "cfgchart.hxx"

class ChartColorLB : public ColorLB
{
public:
    ChartColorLB( Window* pParent, ResId Id ) : ColorLB( pParent, Id ) {}
    ChartColorLB( Window* pParent, WinBits aWB ) : ColorLB( pParent, aWB ) {}

    void FillBox( const SvxChartColorTable & rTab );
};


class SvxDefaultColorOptPage : public SfxTabPage
{
private:
    FixedLine               aGbChartColors;
    ChartColorLB            aLbChartColors;
    FixedLine               aGbColorBox;
    ValueSet                aValSetColorBox;
    PushButton              aPBDefault;

    SvxChartOptions*        pChartOptions;
    SvxChartColorTableItem* pColorConfig;
    XColorTable*            pColorTab;

    DECL_LINK( ResetToDefaults, void * );
    DECL_LINK( ListClickedHdl, ChartColorLB * );
    DECL_LINK( BoxClickedHdl, ValueSet * );

    void FillColorBox();
    long GetColorIndex( const Color& rCol );

public:
    SvxDefaultColorOptPage( Window* pParent, const SfxItemSet& rInAttrs );
    virtual ~SvxDefaultColorOptPage();

    void    Construct();

    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rInAttrs );
    virtual sal_Bool        FillItemSet( SfxItemSet& rOutAttrs );
    virtual void        Reset( const SfxItemSet& rInAttrs );
};

#endif  // _SVX_OPTCHART_HXX

