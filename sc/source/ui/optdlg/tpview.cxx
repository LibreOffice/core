/*************************************************************************
 *
 *  $RCSfile: tpview.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: os $ $Date: 2001-03-28 13:27:35 $
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

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

//------------------------------------------------------------------

#include "scitems.hxx"
#include "tpview.hxx"
#include "global.hxx"
#include "viewopti.hxx"
#include "tabvwsh.hxx"
#include "uiitems.hxx"
#include "scresid.hxx"
#include "docsh.hxx"
#include "sc.hrc"       // -> Slot-IDs
#include "optdlg.hrc"
#include "globstr.hrc"
#ifndef SC_APPOPTIO_HXX
#include <appoptio.hxx>
#endif
#ifndef SC_SCMOD_HXX
#include <scmod.hxx>
#endif
#include <svx/dlgutil.hxx>
#include <svx/drawitem.hxx>
#include <svx/xtable.hxx>
#include <offmgr/app.hxx>


// STATIC DATA -----------------------------------------------------------



/*-----------------11.01.97 10.52-------------------
    Optionen Inhalte
--------------------------------------------------*/

ScTpContentOptions::ScTpContentOptions( Window*         pParent,
                             const SfxItemSet&  rArgSet ) :
    SfxTabPage(pParent, ScResId( RID_SCPAGE_CONTENT ), rArgSet),

    aLinesGB( this,         ResId(GB_LINES          )),
    aGridCB( this,          ResId(CB_GRID           )),
    aColorFT( this,         ResId(FT_COLOR          )),
    aColorLB( this,         ResId(LB_COLOR          )),
    aBreakCB( this,         ResId(CB_PAGEBREAKS     )),
    aGuideLineCB( this,     ResId(CB_GUIDELINE      )),
    aHandleCB( this,        ResId(CB_HANDLES        )),
    aBigHandleCB( this,     ResId(CB_BIGHANDLES     )),

    aDisplayGB( this,   ResId(GB_DISPLAY)),
    aFormulaCB( this,   ResId(CB_FORMULA)),
    aNilCB( this,       ResId(CB_NIL    )),
    aAnnotCB( this,     ResId(CB_ANNOT  )),
    aValueCB( this,     ResId(CB_VALUE  )),
    aAnchorCB( this,    ResId(CB_ANCHOR )),
    aClipMarkCB( this,  ResId(CB_CLIP   )),
    aRangeFindCB( this,     ResId( CB_RFIND     )),

    aObjectGB( this,    ResId(GB_OBJECT )),
    aObjGrfFT( this,    ResId(FT_OBJGRF )),
    aObjGrfLB( this,    ResId(LB_OBJGRF )),

    aDiagramFT( this,   ResId(FT_DIAGRAM)),
    aDiagramLB( this,   ResId(LB_DIAGRAM)),
    aDrawFT( this,      ResId(FT_DRAW   )),
    aDrawLB( this,      ResId(LB_DRAW   )),
    aWindowGB( this,        ResId(GB_WINDOW         )),
    aRowColHeaderCB(this,   ResId(CB_ROWCOLHEADER   )),
    aHScrollCB( this,       ResId(CB_HSCROLL        )),
    aVScrollCB( this,       ResId(CB_VSCROLL        )),
    aTblRegCB( this,        ResId(CB_TBLREG         )),
    aOutlineCB( this,       ResId(CB_OUTLINE        )),
    pLocalOptions(0)
{
    FreeResource();
    SetExchangeSupport();
    Link aSelObjHdl(LINK( this, ScTpContentOptions, SelLbObjHdl ) );
    aObjGrfLB.  SetSelectHdl(aSelObjHdl);
    aDiagramLB. SetSelectHdl(aSelObjHdl);
    aDrawLB.    SetSelectHdl(aSelObjHdl);

    Link aCBHdl(LINK( this, ScTpContentOptions, CBHdl ) );
    aFormulaCB  .SetClickHdl(aCBHdl);
    aNilCB      .SetClickHdl(aCBHdl);
    aAnnotCB    .SetClickHdl(aCBHdl);
    aValueCB    .SetClickHdl(aCBHdl);
    aAnchorCB   .SetClickHdl(aCBHdl);
    aClipMarkCB .SetClickHdl(aCBHdl);

    aVScrollCB  .SetClickHdl(aCBHdl);
    aHScrollCB  .SetClickHdl(aCBHdl);
    aTblRegCB   .SetClickHdl(aCBHdl);
    aOutlineCB  .SetClickHdl(aCBHdl);
    aBreakCB    .SetClickHdl(aCBHdl);
    aGuideLineCB.SetClickHdl(aCBHdl);
    aHandleCB   .SetClickHdl(aCBHdl);
    aBigHandleCB.SetClickHdl(aCBHdl);
    aRowColHeaderCB.SetClickHdl(aCBHdl);

    aGridCB     .SetClickHdl( LINK( this, ScTpContentOptions, GridHdl ) );
}
/*-----------------11.01.97 10.52-------------------

--------------------------------------------------*/

ScTpContentOptions::~ScTpContentOptions()
{
    delete pLocalOptions;
}
/*-----------------11.01.97 10.52-------------------

--------------------------------------------------*/

SfxTabPage* ScTpContentOptions::Create( Window*     pParent,
                              const SfxItemSet&     rCoreSet )
{
    return new ScTpContentOptions(pParent, rCoreSet);
}
/*-----------------11.01.97 10.52-------------------

--------------------------------------------------*/
BOOL    ScTpContentOptions::FillItemSet( SfxItemSet& rCoreSet )
{
    BOOL bRet = FALSE;
    if( aFormulaCB  .GetSavedValue() != aFormulaCB  .IsChecked() ||
        aNilCB      .GetSavedValue() != aNilCB      .IsChecked() ||
        aAnnotCB    .GetSavedValue() != aAnnotCB    .IsChecked() ||
        aValueCB    .GetSavedValue() != aValueCB    .IsChecked() ||
        aAnchorCB   .GetSavedValue() != aAnchorCB   .IsChecked() ||
        aClipMarkCB .GetSavedValue() != aClipMarkCB .IsChecked() ||
        aObjGrfLB   .GetSavedValue() != aObjGrfLB   .GetSelectEntryPos() ||
        aDiagramLB  .GetSavedValue() != aDiagramLB  .GetSelectEntryPos() ||
        aDrawLB     .GetSavedValue() != aDrawLB     .GetSelectEntryPos() ||
        aGridCB         .GetSavedValue() != aGridCB.IsChecked() ||
        aRowColHeaderCB .GetSavedValue() != aRowColHeaderCB.IsChecked() ||
        aHScrollCB      .GetSavedValue() != aHScrollCB     .IsChecked() ||
        aVScrollCB      .GetSavedValue() != aVScrollCB     .IsChecked() ||
        aTblRegCB       .GetSavedValue() != aTblRegCB      .IsChecked() ||
        aOutlineCB      .GetSavedValue() != aOutlineCB     .IsChecked() ||
        aColorLB        .GetSavedValue() != aColorLB       .GetSelectEntryPos() ||
        aBreakCB        .GetSavedValue() != aBreakCB       .IsChecked() ||
        aGuideLineCB    .GetSavedValue() != aGuideLineCB   .IsChecked() ||
        aHandleCB       .GetSavedValue() != aHandleCB      .IsChecked() ||
        aBigHandleCB    .GetSavedValue() != aBigHandleCB   .IsChecked())
    {
        pLocalOptions->SetGridColor( aColorLB.GetSelectEntryColor(),
                                     aColorLB.GetSelectEntry() );
        rCoreSet.Put(ScTpViewItem(SID_SCVIEWOPTIONS, *pLocalOptions));
        bRet = TRUE;
    }
    if(aRangeFindCB.GetSavedValue() != aRangeFindCB.IsChecked())
    {
        rCoreSet.Put(SfxBoolItem(SID_SC_INPUT_RANGEFINDER, aRangeFindCB.IsChecked()));
        bRet = TRUE;
    }


    return bRet;
}
/*-----------------11.01.97 10.53-------------------

--------------------------------------------------*/

void    ScTpContentOptions::Reset( const SfxItemSet& rCoreSet )
{
    const SfxPoolItem* pItem;
    if(SFX_ITEM_SET == rCoreSet.GetItemState(SID_SCVIEWOPTIONS, FALSE , &pItem))
        pLocalOptions  = new ScViewOptions(
                            ((const ScTpViewItem*)pItem)->GetViewOptions() );
    else
        pLocalOptions = new ScViewOptions;
    aFormulaCB  .Check(pLocalOptions->GetOption(VOPT_FORMULAS));
    aNilCB      .Check(pLocalOptions->GetOption(VOPT_NULLVALS));
    aAnnotCB    .Check(pLocalOptions->GetOption(VOPT_NOTES));
    aValueCB    .Check(pLocalOptions->GetOption(VOPT_SYNTAX));
    aAnchorCB   .Check(pLocalOptions->GetOption(VOPT_ANCHOR));
    aClipMarkCB .Check(pLocalOptions->GetOption(VOPT_CLIPMARKS));

    aObjGrfLB   .SelectEntryPos( (USHORT)pLocalOptions->GetObjMode(VOBJ_TYPE_OLE) );
    aDiagramLB  .SelectEntryPos( (USHORT)pLocalOptions->GetObjMode(VOBJ_TYPE_CHART) );
    aDrawLB     .SelectEntryPos( (USHORT)pLocalOptions->GetObjMode(VOBJ_TYPE_DRAW) );

    aRowColHeaderCB.Check( pLocalOptions->GetOption(VOPT_HEADER) );
    aHScrollCB .Check( pLocalOptions->GetOption(VOPT_HSCROLL) );
    aVScrollCB .Check( pLocalOptions->GetOption(VOPT_VSCROLL) );
    aTblRegCB  .Check( pLocalOptions->GetOption(VOPT_TABCONTROLS) );
    aOutlineCB .Check( pLocalOptions->GetOption(VOPT_OUTLINER) );

    InitGridOpt();

    aBreakCB.Check( pLocalOptions->GetOption(VOPT_PAGEBREAKS) );
    aGuideLineCB.Check( pLocalOptions->GetOption(VOPT_HELPLINES) );
    aHandleCB.Check( !pLocalOptions->GetOption(VOPT_SOLIDHANDLES) );    // inverted
    aBigHandleCB.Check( pLocalOptions->GetOption(VOPT_BIGHANDLES) );

    if(SFX_ITEM_SET == rCoreSet.GetItemState(SID_SC_INPUT_RANGEFINDER, FALSE, &pItem))
        aRangeFindCB.Check(((const SfxBoolItem*)pItem)->GetValue());

    aRangeFindCB.SaveValue();

    aFormulaCB  .SaveValue();
    aNilCB      .SaveValue();
    aAnnotCB    .SaveValue();
    aValueCB    .SaveValue();
    aAnchorCB   .SaveValue();
    aClipMarkCB .SaveValue();
    aObjGrfLB   .SaveValue();
    aDiagramLB  .SaveValue();
    aDrawLB     .SaveValue();
    aRowColHeaderCB .SaveValue();
    aHScrollCB      .SaveValue();
    aVScrollCB      .SaveValue();
    aTblRegCB       .SaveValue();
    aOutlineCB      .SaveValue();
    aGridCB         .SaveValue();
    aColorLB        .SaveValue();
    aBreakCB        .SaveValue();
    aGuideLineCB    .SaveValue();
    aHandleCB       .SaveValue();
    aBigHandleCB    .SaveValue();
}
/*-----------------11.01.97 12.45-------------------

--------------------------------------------------*/

void ScTpContentOptions::ActivatePage( const SfxItemSet& rSet)
{
    const SfxPoolItem* pItem;
    if(SFX_ITEM_SET == rSet.GetItemState(SID_SCVIEWOPTIONS, FALSE , &pItem))
        *pLocalOptions = ((const ScTpViewItem*)pItem)->GetViewOptions();
}
/*-----------------11.01.97 12.45-------------------

--------------------------------------------------*/

int ScTpContentOptions::DeactivatePage( SfxItemSet* pSet )
{
    if(pSet)
        FillItemSet(*pSet);
    return SfxTabPage::LEAVE_PAGE;
}
/*-----------------11.01.97 13.43-------------------

--------------------------------------------------*/

IMPL_LINK( ScTpContentOptions, SelLbObjHdl, ListBox*, pLb )
{
    USHORT      nSelPos = pLb->GetSelectEntryPos();
    ScVObjMode  eMode   = ScVObjMode(nSelPos);
    ScVObjType  eType   = VOBJ_TYPE_OLE;

    if ( pLb == &aDiagramLB )
        eType = VOBJ_TYPE_CHART;
    else if ( pLb == &aDrawLB )
        eType = VOBJ_TYPE_DRAW;

    pLocalOptions->SetObjMode( eType, eMode );

    return NULL;
}

/*-----------------11.01.97 14.25-------------------

--------------------------------------------------*/

IMPL_LINK( ScTpContentOptions, CBHdl, CheckBox*, pBtn )
{
    ScViewOption eOption;
    BOOL         bChecked = pBtn->IsChecked();

    if (      &aFormulaCB   == pBtn )   eOption = VOPT_FORMULAS;
    else if ( &aNilCB       == pBtn )   eOption = VOPT_NULLVALS;
    else if ( &aAnnotCB     == pBtn )   eOption = VOPT_NOTES;
    else if ( &aValueCB     == pBtn )   eOption = VOPT_SYNTAX;
    else if ( &aAnchorCB    == pBtn )   eOption = VOPT_ANCHOR;
    else if ( &aClipMarkCB  == pBtn )   eOption = VOPT_CLIPMARKS;
    else if ( &aVScrollCB       == pBtn )   eOption = VOPT_VSCROLL;
    else if ( &aHScrollCB       == pBtn )   eOption = VOPT_HSCROLL;
    else if ( &aTblRegCB        == pBtn )   eOption = VOPT_TABCONTROLS;
    else if ( &aOutlineCB       == pBtn )   eOption = VOPT_OUTLINER;
    else if ( &aBreakCB         == pBtn )   eOption = VOPT_PAGEBREAKS;
    else if ( &aGuideLineCB     == pBtn )   eOption = VOPT_HELPLINES;
    else if ( &aHandleCB        == pBtn )   eOption = VOPT_SOLIDHANDLES;
    else if ( &aBigHandleCB     == pBtn )   eOption = VOPT_BIGHANDLES;
    else if ( &aRowColHeaderCB  == pBtn )   eOption = VOPT_HEADER;

    //  VOPT_SOLIDHANDLES is inverted (CheckBox is "simple handles")
    if ( eOption == VOPT_SOLIDHANDLES )
        pLocalOptions->SetOption( eOption, !bChecked );
    else
        pLocalOptions->SetOption( eOption, bChecked );


    return NULL;
}
/*-----------------11.01.97 13.13-------------------

--------------------------------------------------*/

void ScTpContentOptions::InitGridOpt()
{
    BOOL bGrid = pLocalOptions->GetOption( VOPT_GRID );

    aGridCB.Check( bGrid );

    if ( bGrid )
        aColorFT.Enable(), aColorLB.Enable();
    else
        aColorFT.Disable(), aColorLB.Disable();

    if ( aColorLB.GetEntryCount() == 0 )
    {
        SfxObjectShell* pDocSh = SfxObjectShell::Current();
        // hier koennte auch eine andere DocShell kommen!
        pDocSh = PTR_CAST(ScDocShell, pDocSh);

        XColorTable* pColorTable = NULL;

        if ( pDocSh  )
        {
            const SfxPoolItem* pItem = pDocSh->GetItem( ITEMID_COLOR_TABLE );

            if ( pItem )
                pColorTable = ((SvxColorTableItem*)pItem)->GetColorTable();
        }
        else
            pColorTable = OFF_APP()->GetStdColorTable();

        if ( !pColorTable )
            return;

        //------------------------------------------------------

        aColorLB.SetUpdateMode( FALSE );

        //  Eintraege aus der Colortable

        long nCount = pColorTable->Count();
        for ( long n=0; n<nCount; n++ )
        {
            XColorEntry* pEntry = pColorTable->Get(n);
            aColorLB.InsertEntry( pEntry->GetColor(), pEntry->GetName() );
        }

        //  Standard-Gitterfarbe (hellgrau) immer

        Color aStdCol( COL_LIGHTGRAY );         // wie Default in ScViewOptions
        if ( LISTBOX_ENTRY_NOTFOUND ==
                aColorLB.GetEntryPos( aStdCol ) )
            aColorLB.InsertEntry( aStdCol, ScGlobal::GetRscString( STR_GRIDCOLOR ) );

        aColorLB.SetUpdateMode( TRUE );

        Invalidate();
    }

    //  #79720# also select grid color entry on subsequent calls

    String  aName;
    Color   aCol    = pLocalOptions->GetGridColor( &aName );
    USHORT  nSelPos = aColorLB.GetEntryPos( aCol );

    if ( LISTBOX_ENTRY_NOTFOUND != nSelPos )
        aColorLB.SelectEntryPos( nSelPos );
    else
        aColorLB.SelectEntryPos( aColorLB.InsertEntry( aCol, aName ) );
}
/*-----------------11.01.97 13.40-------------------

--------------------------------------------------*/

IMPL_LINK( ScTpContentOptions, GridHdl, CheckBox*, pBox )
{
    BOOL bChecked = pBox->IsChecked();
    aColorFT.Enable(bChecked);
    aColorLB.Enable(bChecked);
    pLocalOptions->SetOption( VOPT_GRID, bChecked );
    return 0;
}
/*-----------------11.01.97 10.53-------------------

--------------------------------------------------*/

ScTpLayoutOptions::ScTpLayoutOptions(   Window* pParent,
                                        const SfxItemSet&   rArgSet ) :
    SfxTabPage(pParent, ScResId( RID_SCPAGE_LAYOUT ), rArgSet),
    aUnitGB( this,          ResId(GB_UNIT           )),
    aUnitFT( this,          ResId(FT_UNIT           )),
    aUnitLB( this,          ResId(LB_UNIT           )),
    aTabFT( this,           ResId( FT_TAB           )),
    aTabMF( this,           ResId( MF_TAB           )),

    aLinkFT(this, ResId(FT_UPDATE_LINKS )),
    aAlwaysRB   (this, ResId(RB_ALWAYS  )),
    aRequestRB  (this, ResId(RB_REQUEST )),
    aNeverRB    (this, ResId(RB_NEVER   )),
    aLinkGB     (this, ResId(GB_LINK    )),

    aOptionsGB( this,       ResId( GB_OPTIONS   )),
    aAlignCB  ( this,       ResId( CB_ALIGN     )),
    aAlignLB  ( this,       ResId( LB_ALIGN     )),
    aEditModeCB( this,      ResId( CB_EDITMODE  )),
    aFormatCB( this,        ResId( CB_FORMAT    )),
    aExpRefCB( this,        ResId( CB_EXPREF    )),
    aMarkHdrCB( this,       ResId( CB_MARKHDR   )),
    aUnitArr(               ResId(ST_UNIT           )),
    pDoc(NULL),
    pLocalOptions(0)
{
    FreeResource();
    SetExchangeSupport();

    aUnitLB.    SetSelectHdl( LINK( this, ScTpLayoutOptions, MetricHdl ) );

    aAlignCB.SetClickHdl(LINK(this, ScTpLayoutOptions, AlignHdl));


    for ( USHORT i = 0; i < aUnitArr.Count(); ++i )
    {
        String sMetric = aUnitArr.GetStringByPos( i );
        FieldUnit eFUnit = (FieldUnit)aUnitArr.GetValue( i );

        switch ( eFUnit )
        {
            case FUNIT_MM:
            case FUNIT_CM:
            case FUNIT_POINT:
            case FUNIT_PICA:
            case FUNIT_INCH:
            {
                // nur diese Metriken benutzen
                USHORT nPos = aUnitLB.InsertEntry( sMetric );
                aUnitLB.SetEntryData( nPos, (void*)(long)eFUnit );
            }
        }
    }

}
/*-----------------11.01.97 10.53-------------------

--------------------------------------------------*/

ScTpLayoutOptions::~ScTpLayoutOptions()
{
    delete pLocalOptions;
}
/*-----------------11.01.97 10.53-------------------

--------------------------------------------------*/

SfxTabPage* ScTpLayoutOptions::Create( Window*          pParent,
                                    const SfxItemSet&   rCoreSet )
{
    ScTpLayoutOptions* pNew = new ScTpLayoutOptions(pParent, rCoreSet);
    ScDocShell* pDocSh = PTR_CAST(ScDocShell,SfxObjectShell::Current());

    if(pDocSh!=NULL)
        pNew->SetDocument(pDocSh->GetDocument());
    return pNew;
}
/*-----------------11.01.97 10.53-------------------

--------------------------------------------------*/

BOOL    ScTpLayoutOptions::FillItemSet( SfxItemSet& rCoreSet )
{
    BOOL bRet = TRUE;
    const USHORT nMPos = aUnitLB.GetSelectEntryPos();
    if ( nMPos != aUnitLB.GetSavedValue() )
    {
        USHORT nFieldUnit = (USHORT)(long)aUnitLB.GetEntryData( nMPos );
        rCoreSet.Put( SfxUInt16Item( SID_ATTR_METRIC,
                                     (UINT16)nFieldUnit ) );
        bRet = TRUE;
    }

    if(aTabMF.GetText() != aTabMF.GetSavedValue())
    {
        rCoreSet.Put(SfxUInt16Item(SID_ATTR_DEFTABSTOP,
                    aTabMF.Denormalize(aTabMF.GetValue(FUNIT_TWIP))));
        bRet = TRUE;
    }

    ScLkUpdMode nSet=LM_ALWAYS;

    if(aRequestRB.IsChecked())
    {
        nSet=LM_ON_DEMAND;
    }
    else if(aNeverRB.IsChecked())
    {
        nSet=LM_NEVER;
    }

    if(aRequestRB.IsChecked() != aNeverRB.GetSavedValue() ||
            aNeverRB.IsChecked() != aRequestRB.GetSavedValue() )
    {
        if(pDoc)
            pDoc->SetLinkMode(nSet);
        ScAppOptions aAppOptions=SC_MOD()->GetAppOptions();
        aAppOptions.SetLinkMode(nSet );
        SC_MOD()->SetAppOptions(aAppOptions);
        bRet = TRUE;
    }
    if(aAlignCB.GetSavedValue() != aAlignCB.IsChecked())
    {
        rCoreSet.Put(SfxBoolItem(SID_SC_INPUT_SELECTION, aAlignCB.IsChecked()));
        bRet = TRUE;
    }

    if(aAlignLB.GetSavedValue() != aAlignLB.GetSelectEntryPos())
    {
        rCoreSet.Put(SfxUInt16Item(SID_SC_INPUT_SELECTIONPOS, aAlignLB.GetSelectEntryPos()));
        bRet = TRUE;
    }

    if(aEditModeCB.GetSavedValue() != aEditModeCB.IsChecked())
    {
        rCoreSet.Put(SfxBoolItem(SID_SC_INPUT_EDITMODE, aEditModeCB.IsChecked()));
        bRet = TRUE;
    }

    if(aFormatCB.GetSavedValue() != aFormatCB.IsChecked())
    {
        rCoreSet.Put(SfxBoolItem(SID_SC_INPUT_FMT_EXPAND, aFormatCB.IsChecked()));
        bRet = TRUE;
    }


    if(aExpRefCB.GetSavedValue() != aExpRefCB.IsChecked())
    {
        rCoreSet.Put(SfxBoolItem(SID_SC_INPUT_REF_EXPAND, aExpRefCB.IsChecked()));
        bRet = TRUE;
    }

    if(aMarkHdrCB.GetSavedValue() != aMarkHdrCB.IsChecked())
    {
        rCoreSet.Put(SfxBoolItem(SID_SC_INPUT_MARK_HEADER, aMarkHdrCB.IsChecked()));
        bRet = TRUE;
    }
    return bRet;
}
/*-----------------11.01.97 10.53-------------------

--------------------------------------------------*/

void    ScTpLayoutOptions::Reset( const SfxItemSet& rCoreSet )
{
    aUnitLB.SetNoSelection();
    if ( rCoreSet.GetItemState( SID_ATTR_METRIC ) >= SFX_ITEM_AVAILABLE )
    {
        const SfxUInt16Item& rItem = (SfxUInt16Item&)rCoreSet.Get( SID_ATTR_METRIC );
        FieldUnit eFieldUnit = (FieldUnit)rItem.GetValue();

        for ( USHORT i = 0; i < aUnitLB.GetEntryCount(); ++i )
        {
            if ( (FieldUnit)(long)aUnitLB.GetEntryData( i ) == eFieldUnit )
            {
                aUnitLB.SelectEntryPos( i );
                break;
            }
        }
        ::SetFieldUnit(aTabMF, eFieldUnit);
    }
    aUnitLB.SaveValue();

    const SfxPoolItem* pItem;
    if(SFX_ITEM_SET == rCoreSet.GetItemState(SID_ATTR_DEFTABSTOP, FALSE, &pItem))
        aTabMF.SetValue(aTabMF.Normalize(((SfxUInt16Item*)pItem)->GetValue()), FUNIT_TWIP);
    aTabMF.SaveValue();

    aUnitLB         .SaveValue();
    aTabMF          .SaveValue();

    ScLkUpdMode nSet=LM_UNKNOWN;

    if(pDoc!=NULL)
    {
        nSet=pDoc->GetLinkMode();
    }

    if(nSet==LM_UNKNOWN)
    {
        ScAppOptions aAppOptions=SC_MOD()->GetAppOptions();
        nSet=aAppOptions.GetLinkMode();
    }

    switch(nSet)
    {
        case LM_ALWAYS:     aAlwaysRB.  Check();    break;
        case LM_NEVER:      aNeverRB.   Check();    break;
        case LM_ON_DEMAND:  aRequestRB. Check();    break;
    }
    if(SFX_ITEM_SET == rCoreSet.GetItemState(SID_SC_INPUT_SELECTION, FALSE, &pItem))
        aAlignCB.Check(((const SfxBoolItem*)pItem)->GetValue());

    if(SFX_ITEM_SET == rCoreSet.GetItemState(SID_SC_INPUT_SELECTIONPOS, FALSE, &pItem))
        aAlignLB.SelectEntryPos(((const SfxUInt16Item*)pItem)->GetValue());

    if(SFX_ITEM_SET == rCoreSet.GetItemState(SID_SC_INPUT_EDITMODE, FALSE, &pItem))
        aEditModeCB.Check(((const SfxBoolItem*)pItem)->GetValue());

    if(SFX_ITEM_SET == rCoreSet.GetItemState(SID_SC_INPUT_FMT_EXPAND, FALSE, &pItem))
        aFormatCB.Check(((const SfxBoolItem*)pItem)->GetValue());


    if(SFX_ITEM_SET == rCoreSet.GetItemState(SID_SC_INPUT_REF_EXPAND, FALSE, &pItem))
        aExpRefCB.Check(((const SfxBoolItem*)pItem)->GetValue());

    if(SFX_ITEM_SET == rCoreSet.GetItemState(SID_SC_INPUT_MARK_HEADER, FALSE, &pItem))
        aMarkHdrCB.Check(((const SfxBoolItem*)pItem)->GetValue());

    aAlignCB    .SaveValue();
    aAlignLB    .SaveValue();
    aEditModeCB .SaveValue();
    aFormatCB   .SaveValue();

    aExpRefCB   .SaveValue();
    aMarkHdrCB  .SaveValue();
    AlignHdl(&aAlignCB);

    aAlwaysRB.SaveValue();
    aNeverRB.SaveValue();
    aRequestRB.SaveValue();
}

/*-----------------11.01.97 12.46-------------------

--------------------------------------------------*/

void    ScTpLayoutOptions::ActivatePage( const SfxItemSet& rCoreSet)
{
    const SfxPoolItem* pItem;
    if(SFX_ITEM_SET == rCoreSet.GetItemState(SID_SCVIEWOPTIONS, FALSE , &pItem))
        *pLocalOptions = ((const ScTpViewItem*)pItem)->GetViewOptions();
}
/*-----------------11.01.97 12.46-------------------

--------------------------------------------------*/

int ScTpLayoutOptions::DeactivatePage( SfxItemSet* pSet )
{
    if(pSet)
        FillItemSet(*pSet);
    return SfxTabPage::LEAVE_PAGE;
}


/*-----------------13.01.97 14.44-------------------
    Metric des Deftabstops umschalten
--------------------------------------------------*/

IMPL_LINK(ScTpLayoutOptions, MetricHdl, ListBox*, EMPTYARG)
{
    const USHORT nMPos = aUnitLB.GetSelectEntryPos();
    if(nMPos != USHRT_MAX)
    {
        FieldUnit eFieldUnit = (FieldUnit)(long)aUnitLB.GetEntryData( nMPos );
        long nVal =
            aTabMF.Denormalize( aTabMF.GetValue( FUNIT_TWIP ) );
        ::SetFieldUnit( aTabMF, eFieldUnit );
        aTabMF.SetValue( aTabMF.Normalize( nVal ), FUNIT_TWIP );
    }

    return 0;
}
/*-----------------11.01.97 15.30-------------------

--------------------------------------------------*/
IMPL_LINK( ScTpLayoutOptions, AlignHdl, CheckBox*, pBox )
{
    aAlignLB.Enable(pBox->IsChecked());
    return 0;
}


