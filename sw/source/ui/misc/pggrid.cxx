/*************************************************************************
 *
 *  $RCSfile: pggrid.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: os $ $Date: 2002-02-07 15:09:27 $
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

#ifdef PRECOMPILED
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif
#ifndef _UIPARAM_HXX
#include <uiparam.hxx>
#endif
#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
#ifndef _GLOBALS_HRC
#include <globals.hrc>
#endif

#ifndef _OFF_APP_HXX //autogen
#include <offmgr/app.hxx>
#endif
#ifndef _XTABLE_HXX //autogen
#include <svx/xtable.hxx>
#endif

#ifndef _PGGRID_HXX
#include <pggrid.hxx>
#endif
#ifndef SW_TGRDITEM_HXX
#include <tgrditem.hxx>
#endif
#include <pggrid.hrc>

/*-- 06.02.2002 15:25:39---------------------------------------------------

  -----------------------------------------------------------------------*/
SwTextGridPage::SwTextGridPage(Window *pParent, const SfxItemSet &rSet) :
    SfxTabPage(pParent, SW_RES(TP_TEXTGRID_PAGE), rSet),
    aGridTypeFL             (this, ResId(FL_GRID_TYPE       )),
    aNoGridRB               (this, ResId(RB_NOGRID          )),
    aLinesGridRB            (this, ResId(RB_LINESGRID       )),
    aCharsGridRB            (this, ResId(RB_CHARSGRID       )),
    aExampleWN              (this, ResId(WN_EXAMPLE         )),
    aLayoutFL               (this, ResId(FL_LAYOUT          )),
    aLinesPerPageFT         (this, ResId(FT_LINESPERPAGE    )),
    aLinesPerPageNF         (this, ResId(NF_LINESPERPAGE    )),
    aLinesPerPageHintFT     (this, ResId(FT_LINESPERPAGEHINT)),
    aTextSizeFT             (this, ResId(FT_TEXTSIZE        )),
    aTextSizeMF             (this, ResId(MF_TEXTSIZE        )),
    aCharsPerLineFT         (this, ResId(FT_CHARSPERLINE    )),
    aCharsPerLineNF         (this, ResId(NF_CHARSPERLINE    )),
    aCharsPerLineHintFT     (this, ResId(FT_CHARSPERLINEHINT)),
    aRubySizeFT             (this, ResId(FT_RUBYSIZE        )),
    aRubySizeMF             (this, ResId(MF_RUBYSIZE        )),
    aRubyBelowCB            (this, ResId(CB_RUBYBELOW       )),
    aDisplayFL              (this, ResId(FL_DISPLAY         )),
    aDisplayCB              (this, ResId(CB_DISPLAY         )),
    aPrintCB                (this, ResId(CB_PRINT           )),
    aColorFT                (this, ResId(FT_COLOR           )),
    aColorLB                (this, ResId(LB_COLOR           ))
{
    FreeResource();
    XColorTable* pColorTbl = OFF_APP()->GetStdColorTable();
    aColorLB.InsertAutomaticEntry();
    for( USHORT i = 0; i < pColorTbl->Count(); ++i )
    {
        XColorEntry* pEntry = pColorTbl->Get( i );
        Color aColor = pEntry->GetColor();
        String sName = pEntry->GetName();
        aColorLB.InsertEntry( aColor, sName );
    }
    aColorLB.SetUpdateMode( TRUE );
}
/*-- 06.02.2002 15:25:40---------------------------------------------------

  -----------------------------------------------------------------------*/
SwTextGridPage::~SwTextGridPage()
{
}
/*-- 06.02.2002 15:25:40---------------------------------------------------

  -----------------------------------------------------------------------*/
SfxTabPage *SwTextGridPage::Create(Window *pParent, const SfxItemSet &rSet)
{
    return new SwTextGridPage(pParent, rSet);
}
/*-- 06.02.2002 15:25:40---------------------------------------------------

  -----------------------------------------------------------------------*/
BOOL    SwTextGridPage::FillItemSet(SfxItemSet &rSet)
{
    BOOL bRet = FALSE;
    if(aNoGridRB.GetSavedValue() != aNoGridRB.IsChecked()||
        aLinesGridRB.GetSavedValue() != aLinesGridRB.IsChecked()||
        aLinesPerPageNF.GetSavedValue() != aLinesPerPageNF.GetValue()||
        aTextSizeMF.GetSavedValue() != aTextSizeMF.GetValue()||
        aCharsPerLineNF.GetSavedValue() != aCharsPerLineNF.GetValue()||
        aRubySizeMF.GetSavedValue() != aRubySizeMF.GetValue()||
        aRubyBelowCB.GetSavedValue() != aRubyBelowCB.IsChecked()||
        aDisplayCB.GetSavedValue() != aDisplayCB.IsChecked()||
        aPrintCB.GetSavedValue() != aPrintCB.IsChecked()||
        aColorLB.GetSavedValue() != aColorLB.GetSelectEntryPos())
    {
        SwTextGridItem aGridItem;
        aGridItem.SetGridType(aNoGridRB.IsChecked() ? GRID_NONE :
            aLinesGridRB.IsChecked() ? GRID_LINES_ONLY : GRID_LINES_CHARS );
        aGridItem.SetLines(aLinesPerPageNF.GetValue());
        aGridItem.SetBaseHeight(aTextSizeMF.Denormalize(aTextSizeMF.GetValue(FUNIT_TWIP)));
        aGridItem.SetRubyHeight(aRubySizeMF.Denormalize(aRubySizeMF.GetValue(FUNIT_TWIP)));
        aGridItem.SetRubyTextBelow(aRubyBelowCB.IsChecked());
        aGridItem.SetDisplayGrid(aDisplayCB.IsChecked());
        aGridItem.SetPrintGrid(aPrintCB.IsChecked());
        aGridItem.SetColor(aColorLB.GetSelectEntryColor());
        rSet.Put(aGridItem);
        bRet = TRUE;
    }

    return bRet;
}
/*-- 06.02.2002 15:25:40---------------------------------------------------

  -----------------------------------------------------------------------*/
void    SwTextGridPage::Reset(const SfxItemSet &rSet)
{
    if(SFX_ITEM_AVAILABLE <= rSet.GetItemState(RES_TEXTGRID, TRUE))
    {
        const SwTextGridItem& rGridItem = (const SwTextGridItem&)rSet.Get(RES_TEXTGRID);
        switch(rGridItem.GetGridType())
        {
            case GRID_NONE :        aNoGridRB.Check();    break;
            case GRID_LINES_ONLY  : aLinesGridRB.Check(); break;
            default:                aCharsGridRB.Check();
        }
        aLinesPerPageNF.SetValue(rGridItem.GetLines());
        aTextSizeMF.SetValue(aTextSizeMF.Normalize(rGridItem.GetBaseHeight()), FUNIT_TWIP);
        // has to be calculated       aCharsPerLineNF.SetValue();
        aRubySizeMF.SetValue(aRubySizeMF.Normalize(rGridItem.GetRubyHeight()), FUNIT_TWIP);
        aRubyBelowCB.Check(rGridItem.IsRubyTextBelow());
        aDisplayCB.Check(rGridItem.IsDisplayGrid());
        aPrintCB.Check(rGridItem.IsPrintGrid());
        aColorLB.SelectEntry(rGridItem.GetColor());
    }
    aNoGridRB.SaveValue();
    aLinesGridRB.SaveValue();
    aLinesPerPageNF.SaveValue();
    aTextSizeMF.SaveValue();
    aCharsPerLineNF.SaveValue();
    aRubySizeMF.SaveValue();
    aRubyBelowCB.SaveValue();
    aDisplayCB.SaveValue();
    aPrintCB.SaveValue();
    aColorLB.SaveValue();

}
/*-- 06.02.2002 15:25:41---------------------------------------------------

  -----------------------------------------------------------------------*/
void    SwTextGridPage::ActivatePage( const SfxItemSet& rSet )
{
    aExampleWN.Hide();
    if( SFX_ITEM_SET == rSet.GetItemState( SID_ATTR_PAGE_SIZE ))
    {
//            const SvxSizeItem& rSize = (const SvxSizeItem&)rSet.Get(
//                                                SID_ATTR_PAGE_SIZE);
//            const SvxLRSpaceItem& rLRSpace = (const SvxLRSpaceItem&)rSet.Get(
//                                                                RES_LR_SPACE );
//            const SvxBoxItem& rBox = (const SvxBoxItem&) rSet.Get(RES_BOX);
//            USHORT nActWidth = (USHORT)rSize.GetSize().Width()
//                            - rLRSpace.GetLeft() - rLRSpace.GetRight() - rBox.GetDistance();

    //update characters per line and lines per page control
    }
    aExampleWN.Show();
}
/*-- 06.02.2002 15:25:41---------------------------------------------------

  -----------------------------------------------------------------------*/
int SwTextGridPage::DeactivatePage( SfxItemSet* pSet )
{
    return 0;
}
/* -----------------------------06.02.2002 15:24------------------------------

 ---------------------------------------------------------------------------*/
USHORT* SwTextGridPage::GetRanges()
{
    static USHORT __FAR_DATA aPageRg[] = {
        RES_TEXTGRID, RES_TEXTGRID,
        0};
    return aPageRg;
}

