/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: optchart.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 15:17:32 $
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

#ifndef _SVX_OPTCHART_HXX
#define _SVX_OPTCHART_HXX

// header for SfxTabPage
#ifndef _SFXTABDLG_HXX
#include <sfx2/tabdlg.hxx>
#endif
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
// header for ValueSet
#ifndef _VALUESET_HXX
#include <svtools/valueset.hxx>
#endif
// header for ColorLB
#ifndef _SVX_DLG_CTRL_HXX
#include "dlgctrl.hxx"
#endif
// header for PushButton
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
// header for XColorTable
#ifndef _XTABLE_HXX
#include <xtable.hxx>
#endif

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
    virtual BOOL        FillItemSet( SfxItemSet& rOutAttrs );
    virtual void        Reset( const SfxItemSet& rInAttrs );
};

#endif  // _SVX_OPTCHART_HXX

