/*************************************************************************
 *
 *  $RCSfile: tabledlg.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: os $ $Date: 2001-03-21 10:08:51 $
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

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _LIST_HXX //autogen
#include <tools/list.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _SFXINTITEM_HXX //autogen
#include <svtools/intitem.hxx>
#endif
#ifndef _SVX_BORDER_HXX //autogen
#include <svx/border.hxx>
#endif
#ifndef _SVX_BACKGRND_HXX //autogen
#include <svx/backgrnd.hxx>
#endif
#ifndef _SVX_HTMLMODE_HXX
#include <svx/htmlmode.hxx>
#endif
#ifndef _SVX_KEEPITEM_HXX //autogen
#include <svx/keepitem.hxx>
#endif
#ifndef _SVX_BRKITEM_HXX //autogen
#include <svx/brkitem.hxx>
#endif
#ifndef _SVX_ULSPITEM_HXX //autogen
#include <svx/ulspitem.hxx>
#endif


#ifndef _FMTORNT_HXX //autogen
#include <fmtornt.hxx>
#endif
#ifndef _FMTPDSC_HXX //autogen
#include <fmtpdsc.hxx>
#endif
#ifndef _FMTTSPLT_HXX //autogen
#include <fmtlsplt.hxx>
#endif
#ifndef _OFA_HTMLCFG_HXX //autogen
#include <offmgr/htmlcfg.hxx>
#endif
#ifndef _OFF_APP_HXX //autogen
#include <offmgr/app.hxx>
#endif
#ifndef _SVX_HTMLMODE_HXX
#include <svx/htmlmode.hxx>
#endif

#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#ifndef _VIEWOPT_HXX
#include <viewopt.hxx>
#endif
#ifndef _UITOOL_HXX
#include <uitool.hxx>
#endif
#ifndef _FRMATR_HXX
#include <frmatr.hxx>
#endif
#ifndef _TABLEDLG_HXX
#include <tabledlg.hxx>
#endif
#ifndef _TABLEPG_HXX
#include <tablepg.hxx>
#endif
#ifndef _TABLEMGR_HXX
#include <tablemgr.hxx>
#endif
#ifndef _PAGEDESC_HXX
#include <pagedesc.hxx>
#endif
#ifndef _UIITEMS_HXX
#include <uiitems.hxx>
#endif
#ifndef _POOLFMT_HXX
#include <poolfmt.hxx>
#endif

#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _TABLEDLG_HRC
#include <tabledlg.hrc>
#endif
#ifndef _TABLE_HRC
#include <table.hrc>
#endif
#include <algorithm>


#ifdef DEBUG_TBLDLG
void DbgTblRep(SwTableRep* pRep)
{
    DBG_ERROR(String(pRep->GetColCount()))
    DBG_ERROR(String(pRep->GetAllColCount()))
    SwTwips nSum = 0;
    for(USHORT i = 0; i < pRep->GetAllColCount(); i++)
    {
        String sMsg(i);
        sMsg += pRep->GetColumns()[i].bVisible ? " v " : " h ";
        sMsg += pRep->GetColumns()[i].nWidth;
        nSum +=pRep->GetColumns()[i].nWidth;
        DBG_ERROR(sMsg)
    }
    String sMsg("Spaltensumme: ");
    sMsg += nSum;
    sMsg += " Tblbreite: ";
    sMsg += pRep->GetWidth();
    DBG_ERROR(sMsg)
    sMsg = "Gesamt/Links/Rechts: ";
    sMsg += pRep->GetSpace();
    sMsg += '/';
    sMsg += pRep->GetLeftSpace();
    sMsg += '/';
    sMsg += pRep->GetRightSpace();
    DBG_ERROR(sMsg)
    sMsg = "Align: ";
    sMsg += pRep->GetAlign();
    DBG_ERROR(sMsg)

};

void DbgTColumn(TColumn* pTColumn, USHORT nCount)
{
    for(USHORT i = 0; i < nCount; i++)
    {
        String sMsg(i);
        sMsg += pTColumn[i].bVisible ? " v " : " h ";
        sMsg += pTColumn[i].nWidth;
        DBG_ERROR(sMsg)
    }
}
#endif


#ifdef DEBUG_TBLDLG
#define DEBUG_TBLDLG_TABLEREP(pRep) DbgTblRep(pRep)
#define DEBUG_TBLDLG_TCOLUMN(pTColumn, nCount) DbgTColumn(pTColumn, nCount)
#else
#define DEBUG_TBLDLG_TABLEREP
#define DEBUG_TBLDLG_TCOLUMN
#endif

SwFormatTablePage::SwFormatTablePage( Window* pParent, const SfxItemSet& rSet ) :
    SfxTabPage(pParent, SW_RES( TP_FORMAT_TABLE ), rSet ),
    aNameFT(this,       SW_RES( FT_NAME )),
    aNameED(this,       SW_RES( ED_NAME )),
    aWidthFT(this,      SW_RES( FT_WIDTH )),
    aWidthMF(this,      SW_RES( ED_WIDTH )),
    aRelWidthCB(this,   SW_RES( CB_REL_WIDTH )),
    aLeftFT(this,       SW_RES( FT_LEFT_DIST )),
    aLeftMF(this,       SW_RES( ED_LEFT_DIST )),
    aRightFT(this,      SW_RES( FT_RIGHT_DIST )),
    aRightMF(this,      SW_RES( ED_RIGHT_DIST )),
    aTopFT (this,       SW_RES( FT_TOP_DIST )),
    aTopMF(this,        SW_RES( ED_TOP_DIST )),
    aBottomFT(this,     SW_RES( FT_BOTTOM_DIST )),
    aBottomMF(this,     SW_RES( ED_BOTTOM_DIST )),
    aDistGB(this,       SW_RES( GB_DIST )),
    aOptionsGB(this,    SW_RES( GB_OPTIONS )),
    aFullBtn(this,      SW_RES( RB_FULL )),
    aFreeBtn(this,      SW_RES( RB_FREE )),
    aLeftBtn(this,      SW_RES( RB_LEFT )),
    aFromLeftBtn(this,  SW_RES( RB_FROM_LEFT )),
    aRightBtn(this,     SW_RES( RB_RIGHT )),
    aCenterBtn(this,    SW_RES( RB_CENTER )),
    aPosFrm(this,       SW_RES( GB_POS )),
    pTblData(0),
    nSaveWidth(0),
    nMinTableWidth(MINLAY),
    bModified(FALSE),
    bFull(0)
{
    FreeResource();
    SetExchangeSupport();

    Init();
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void  SwFormatTablePage::Init()
{
    aLeftMF.MetricField::SetMin(-999999);
    aRightMF.MetricField::SetMin(-999999);

    // handler
    Link aLk = LINK( this, SwFormatTablePage, AutoClickHdl );
    aFullBtn.SetClickHdl( aLk );
    aFreeBtn.SetClickHdl( aLk );
    aLeftBtn.SetClickHdl( aLk );
    aFromLeftBtn.SetClickHdl( aLk );
    aRightBtn.SetClickHdl( aLk );
    aCenterBtn.SetClickHdl( aLk );

    aLk = LINK( this, SwFormatTablePage, UpHdl );
    aTopMF.SetUpHdl( aLk );
    aBottomMF.SetUpHdl( aLk );
    aRightMF.SetUpHdl( aLk );
    aLeftMF.SetUpHdl( aLk );
    aWidthMF.SetUpHdl( aLk );

    aLk = LINK( this, SwFormatTablePage, DownHdl );
    aTopMF.SetDownHdl( aLk );
    aBottomMF.SetDownHdl( aLk );
    aRightMF.SetDownHdl( aLk );
    aLeftMF.SetDownHdl( aLk );
    aWidthMF.SetDownHdl( aLk );

    aLk = LINK( this, SwFormatTablePage, LoseFocusHdl );
    aTopMF.SetLoseFocusHdl( aLk );
    aBottomMF.SetLoseFocusHdl( aLk );
    aRightMF.SetLoseFocusHdl( aLk );
    aLeftMF.SetLoseFocusHdl( aLk );
    aWidthMF.SetLoseFocusHdl( aLk );

    aRightMF.SetModifyHdl(LINK(this, SwFormatTablePage, RightModifyHdl));
    aRelWidthCB.SetClickHdl(LINK( this, SwFormatTablePage, RelWidthClickHdl ));
}

/*------------------------------------------------------------------------*/

IMPL_LINK( SwFormatTablePage, RelWidthClickHdl, CheckBox *, pBtn )
{
    DBG_ASSERT(pTblData, "Tabellendaten nicht da?")
    BOOL bIsChecked = pBtn->IsChecked();
    long nLeft = aLeftMF.Denormalize(aLeftMF.GetValue(FUNIT_TWIP ));
    long nRight = aRightMF.Denormalize(aRightMF.GetValue(FUNIT_TWIP ));
    aWidthMF.ShowPercent(bIsChecked);
    aLeftMF.ShowPercent(bIsChecked);
    aRightMF.ShowPercent(bIsChecked);

    if (bIsChecked)
    {
        aWidthMF.SetRefValue(pTblData->GetSpace());
        aLeftMF.SetRefValue(pTblData->GetSpace());
        aRightMF.SetRefValue(pTblData->GetSpace());
        aLeftMF.MetricField::SetMin(0); // wird vom Percentfield ueberschrieben
        aRightMF.MetricField::SetMin(0);//                 -""-
        aLeftMF.SetPrcntValue(aLeftMF.Normalize( nLeft ), FUNIT_TWIP );
        aRightMF.SetPrcntValue(aRightMF.Normalize( nRight ), FUNIT_TWIP );
    }
    else
        ModifyHdl(&aLeftMF);    // Werte wieder korrigieren

    if(aFreeBtn.IsChecked())
    {
        BOOL bEnable = !pBtn->IsChecked();
        aRightMF.Enable(bEnable);
        aRightFT.Enable(bEnable);
    }
    bModified = TRUE;

    return 0;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
IMPL_LINK( SwFormatTablePage, AutoClickHdl, CheckBox *, pBox )
{
    BOOL bRestore = TRUE,
         bLeftEnable = FALSE,
         bRightEnable= FALSE,
         bWidthEnable= FALSE,
         bOthers = TRUE;
    if( (RadioButton *) pBox == &aFullBtn )
    {
        aLeftMF.SetPrcntValue(0);
        aRightMF.SetPrcntValue(0);
        nSaveWidth = aWidthMF.Denormalize(aWidthMF.GetValue(FUNIT_TWIP ));
        aWidthMF.SetPrcntValue(aWidthMF.Normalize( pTblData->GetSpace() ), FUNIT_TWIP );
        aLeftMF.SetText( aEmptyStr );
        aRightMF.SetText( aEmptyStr );
        bFull = TRUE;
        bRestore = FALSE;
    }
    else if( (RadioButton *) pBox == &aLeftBtn )
    {
        bRightEnable = bWidthEnable = TRUE;
        aLeftMF.SetText( aEmptyStr );
        aLeftMF.SetPrcntValue(0);
    }
    else if( (RadioButton *) pBox == &aFromLeftBtn )
    {
        bLeftEnable = bWidthEnable = TRUE;
        aRightMF.SetText( aEmptyStr );
        aRightMF.SetPrcntValue(0);
    }
    else if( (RadioButton *) pBox == &aRightBtn )
    {
        bLeftEnable = bWidthEnable = TRUE;
        aRightMF.SetPrcntValue(0);
        aRightMF.SetText( aEmptyStr );
    }
    else if( ( RadioButton * ) pBox == &aCenterBtn )
    {
        bLeftEnable = bWidthEnable = TRUE;
        aRightMF.SetText( aEmptyStr );
    }
    else if( ( RadioButton * ) pBox == &aFreeBtn )
    {
        RightModifyHdl(&aRightMF);
        bLeftEnable = TRUE;
        bWidthEnable = TRUE;
        bOthers = FALSE;
    }
    aLeftMF.Enable(bLeftEnable);
    aLeftFT.Enable(bLeftEnable);
    aWidthMF.Enable(bWidthEnable);
    aWidthFT.Enable(bWidthEnable);
    if ( bOthers )
    {
        aRightMF.Enable(bRightEnable);
        aRightFT.Enable(bRightEnable);
        aRelWidthCB.Enable(bWidthEnable);
    }

    if(bFull && bRestore)
    {
        // nachdem auf autom. geschaltet wurde, wurde die Breite gemerkt,
        // um sie beim Zurueckschalten restaurieren zu koennen
        bFull = FALSE;
        aWidthMF.SetPrcntValue(aWidthMF.Normalize( nSaveWidth ), FUNIT_TWIP );
    }
    ModifyHdl(&aWidthMF);
    bModified = TRUE;
    return 0;
}

/*----------------------------------------------------------------------*/
IMPL_LINK( SwFormatTablePage, RightModifyHdl, MetricField *, pFld )
{
    if(aFreeBtn.IsChecked())
    {
        BOOL bEnable = aRightMF.GetValue() == 0;
//      aWidthMF.Enable(bEnable);
        aRelWidthCB.Enable(bEnable);
//      aWidthFT.Enable(bEnable);
        if ( !bEnable )
        {
            aRelWidthCB.Check(FALSE);
            RelWidthClickHdl(&aRelWidthCB);
        }
        bEnable = aRelWidthCB.IsChecked();
        aRightMF.Enable(!bEnable);
        aRightFT.Enable(!bEnable);
    }
    return 0;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
IMPL_LINK_INLINE_START( SwFormatTablePage, UpHdl, MetricField *, pEdit )
{
    ModifyHdl( pEdit );
    return 0;
}
IMPL_LINK_INLINE_END( SwFormatTablePage, UpHdl, MetricField *, pEdit )

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
IMPL_LINK_INLINE_START( SwFormatTablePage, DownHdl, MetricField *, pEdit )
{
    ModifyHdl( pEdit );
    return 0;
}
IMPL_LINK_INLINE_END( SwFormatTablePage, DownHdl, MetricField *, pEdit )

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
IMPL_LINK_INLINE_START( SwFormatTablePage, LoseFocusHdl, MetricField *, pEdit )
{
    ModifyHdl( pEdit );
    return 0;
}
IMPL_LINK_INLINE_END( SwFormatTablePage, LoseFocusHdl, MetricField *, pEdit )

void  SwFormatTablePage::ModifyHdl( Edit* pEdit )
{

    SwTwips nCurWidth = aWidthMF.Denormalize( aWidthMF.GetValue( FUNIT_TWIP ));
    SwTwips nPrevWidth = nCurWidth;
    SwTwips nRight = aRightMF.Denormalize( aRightMF.GetValue( FUNIT_TWIP ));
    SwTwips nLeft = aLeftMF.Denormalize( aLeftMF.GetValue( FUNIT_TWIP ));
    SwTwips nDiff;

    if( pEdit == &aWidthMF )
    {
        if( nCurWidth < MINLAY )
            nCurWidth = MINLAY;
        nDiff = nRight + nLeft + nCurWidth - pTblData->GetSpace() ;
        //rechtsbuendig nur linken Rand veraendern
        if(aRightBtn.IsChecked())
            nLeft -= nDiff;
        //linksbuendig nur rechten Rand veraendern
        else if(aLeftBtn.IsChecked())
            nRight -= nDiff;
        //linker Rand und Breite erlaubt - erst rechts - dann links
        else if(aFromLeftBtn.IsChecked())
        {
            if( nRight >= nDiff )
                nRight -= nDiff;
            else
            {
                nDiff -= nRight;
                nRight = 0;
                if(nLeft >= nDiff)
                    nLeft -= nDiff;
                else
                {
                    nRight += nLeft - nDiff;
                    nLeft = 0;
                    nCurWidth = pTblData->GetSpace();
                }

            }
        }
        //zentriert beide Seiten gleichmaessig veraendern
        else if(aCenterBtn.IsChecked())
        {
            if((nLeft != nRight))
            {
                nDiff += nLeft + nRight;
                nLeft = nDiff/2;
                nRight = nDiff/2;
            }
            else
            {
                    nLeft -= nDiff/2;
                    nRight -= nDiff/2;
            }
        }
        //freie Ausrichtung: beide Raender verkleinern
        else if(aFreeBtn.IsChecked())
        {
            nLeft -= nDiff/2;
            nRight -= nDiff/2;
        }
    }
    if( pEdit == &aRightMF  )
    {

        if( nRight + nLeft > pTblData->GetSpace() - MINLAY )
            nRight = pTblData->GetSpace() -nLeft - MINLAY;

        nCurWidth = pTblData->GetSpace() - nLeft - nRight;
    }
    if( pEdit == &aLeftMF )
    {
        if(!aFromLeftBtn.IsChecked())
        {
            BOOL bCenter = aCenterBtn.IsChecked();
            if( bCenter )
                nRight = nLeft;
            if(nRight + nLeft > pTblData->GetSpace() - MINLAY )
            {
                nLeft  = bCenter ?  (pTblData->GetSpace() - MINLAY) /2 :
                                    (pTblData->GetSpace() - MINLAY) - nRight;
                nRight = bCenter ?  (pTblData->GetSpace() - MINLAY) /2 : nRight;
            }
            nCurWidth = pTblData->GetSpace() - nLeft - nRight;
        }
        else
        {
            //hier wird bei Aenderung an der linken Seite zuerst der
            //rechte Rand veraendert, dann die Breite
            nDiff = nRight + nLeft + nCurWidth - pTblData->GetSpace() ;

            nRight -= nDiff;
            nCurWidth = pTblData->GetSpace() - nLeft - nRight;
        }
    }
    if (nCurWidth != nPrevWidth )
        aWidthMF.SetPrcntValue( aWidthMF.Normalize( nCurWidth ), FUNIT_TWIP );
    aRightMF.SetPrcntValue( aRightMF.Normalize( nRight ), FUNIT_TWIP );
    aLeftMF.SetPrcntValue( aLeftMF.Normalize( nLeft ), FUNIT_TWIP );
    bModified = TRUE;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
SfxTabPage*  SwFormatTablePage::Create( Window* pParent,
                                   const SfxItemSet& rAttrSet)
{
    return new SwFormatTablePage( pParent, rAttrSet );
}

/*------------------------------------------------------------------------
-------------------------------------------------------------------------*/
BOOL  SwFormatTablePage::FillItemSet( SfxItemSet& rCoreSet )
{
    // Testen, ob eins der Control noch den Focus hat
    if(aWidthMF.HasFocus())
        ModifyHdl(&aWidthMF);
    else if(aLeftMF.HasFocus())
        ModifyHdl(&aLeftMF);
    else if(aRightMF.HasFocus())
        ModifyHdl(&aRightMF);
    else if(aTopMF.HasFocus())
        ModifyHdl(&aTopMF);
    else if(aBottomMF.HasFocus())
        ModifyHdl(&aBottomMF);

    if(bModified)
    {
        if( aBottomMF.GetText() != aBottomMF.GetSavedValue() ||
                                    aTopMF.GetText() != aTopMF.GetSavedValue() )
        {
            SvxULSpaceItem aULSpace(RES_UL_SPACE);
            aULSpace.SetUpper( (USHORT) aTopMF.Denormalize(
                                        aTopMF.GetValue( FUNIT_TWIP )));
            aULSpace.SetLower( (USHORT) aBottomMF.Denormalize(
                                        aBottomMF.GetValue( FUNIT_TWIP )));
            rCoreSet.Put(aULSpace);
        }

    }
    if(aNameED.GetText() != aNameED.GetSavedValue())
    {
        rCoreSet.Put(SfxStringItem( FN_PARAM_TABLE_NAME, aNameED.GetText()));
        bModified = TRUE;
    }
    return bModified;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void  SwFormatTablePage::Reset( const SfxItemSet& )
{
    const SfxItemSet& rSet = GetItemSet();
    const SfxPoolItem*  pItem;

    BOOL bHtmlMode = FALSE;
    if(SFX_ITEM_SET == rSet.GetItemState( SID_HTML_MODE, FALSE,&pItem )
        && ((const SfxUInt16Item*)pItem)->GetValue() & HTMLMODE_ON)
    {
        aNameED .Disable();
        aTopFT  .Hide();
        aTopMF  .Hide();
        aBottomFT.Hide();
        aBottomMF.Hide();
        aFreeBtn.Enable(FALSE);
        bHtmlMode = TRUE;
    }
    FieldUnit aMetric = ::GetDfltMetric(bHtmlMode);
    SetMetric( aWidthMF, aMetric );
    SetMetric( aRightMF, aMetric );
    SetMetric( aLeftMF, aMetric );
    SetMetric( aTopMF, aMetric );
    SetMetric( aBottomMF, aMetric );

    //Name
    if(SFX_ITEM_SET == rSet.GetItemState( FN_PARAM_TABLE_NAME, FALSE, &pItem ))
    {
        aNameED.SetText(((const SfxStringItem*)pItem)->GetValue());
        aNameED.SaveValue();
    }

    if(SFX_ITEM_SET == rSet.GetItemState( FN_TABLE_REP, FALSE, &pItem ))
    {
        pTblData = (SwTableRep*)((const SwPtrItem*) pItem)->GetValue();
        nMinTableWidth = pTblData->GetColCount() * MINLAY;

        if(pTblData->GetWidthPercent())
        {
            aRelWidthCB.Check(TRUE);
            RelWidthClickHdl(&aRelWidthCB);
            aWidthMF.SetPrcntValue(pTblData->GetWidthPercent(), FUNIT_CUSTOM);

            aWidthMF.SaveValue();
            nSaveWidth = aWidthMF.GetValue(FUNIT_CUSTOM);
        }
        else
        {
            aWidthMF.SetPrcntValue(aWidthMF.Normalize(
                            pTblData->GetWidth()), FUNIT_TWIP);
            aWidthMF.SaveValue();
            nSaveWidth = pTblData->GetWidth();
            nMinTableWidth = std::min(nSaveWidth, nMinTableWidth);
        }

        aWidthMF.SetRefValue(pTblData->GetSpace());
        aWidthMF.SetLast(aWidthMF.Normalize( pTblData->GetSpace() ));
        aLeftMF.SetLast(aLeftMF.Normalize( pTblData->GetSpace() ));
        aRightMF.SetLast(aRightMF.Normalize( pTblData->GetSpace() ));

        aLeftMF.SetPrcntValue(aLeftMF.Normalize(
                                pTblData->GetLeftSpace()), FUNIT_TWIP);
        aRightMF.SetPrcntValue(aRightMF.Normalize(
                            pTblData->GetRightSpace()), FUNIT_TWIP);
        aLeftMF.SaveValue();
        aRightMF.SaveValue();

        nOldAlign = pTblData->GetAlign();

        BOOL bSetRight = FALSE, bRightEnable = FALSE,
             bSetLeft  = FALSE, bLeftEnable  = FALSE;
        switch( nOldAlign )
        {
            case HORI_NONE:
                aFreeBtn.Check();
                if(aRelWidthCB.IsChecked())
                    bSetRight = TRUE;
            break;
            case HORI_FULL:
            {
                bSetRight = bSetLeft = TRUE;
                aFullBtn.Check();
                aRightMF.SetText( aEmptyStr );
                aLeftMF.SetText( aEmptyStr );
                aWidthMF.Enable(FALSE);
                aRelWidthCB.Enable(FALSE);
                aWidthFT.Enable(FALSE);
            }
            break;
            case HORI_LEFT:
            {
                bSetLeft = TRUE;
                aLeftBtn.Check();
                aLeftMF.SetText( aEmptyStr );
            }
            break;
            case HORI_LEFT_AND_WIDTH :
            {
                bSetRight = TRUE;
                aFromLeftBtn.Check();
                aRightMF.SetText( aEmptyStr );
            }
            break;
            case HORI_RIGHT:
            {
                bSetRight = TRUE;
                aRightBtn.Check();
                aRightMF.SetText( aEmptyStr );
            }
            break;
            case HORI_CENTER:
            {
                bSetRight = TRUE;
                aCenterBtn.Check();
                aRightMF.SetText( aEmptyStr );
            }
            break;
        }
        if ( bSetRight )
        {
            aRightMF.Enable(bRightEnable);
            aRightFT.Enable(bRightEnable);
        }
        if ( bSetLeft )
        {
            aLeftMF.Enable(bLeftEnable);
            aLeftFT.Enable(bLeftEnable);
        }

    }

    //Raender
    if(SFX_ITEM_SET == rSet.GetItemState( RES_UL_SPACE, FALSE,&pItem ))
    {
        aTopMF.SetValue(aTopMF.Normalize(
                        ((const SvxULSpaceItem*)pItem)->GetUpper()), FUNIT_TWIP);
        aBottomMF.SetValue(aBottomMF.Normalize(
                        ((const SvxULSpaceItem*)pItem)->GetLower()), FUNIT_TWIP);
        aTopMF.SaveValue();
        aBottomMF.SaveValue();
    }

    aWidthMF.SetMax( 2*aWidthMF.Normalize( pTblData->GetSpace() ), FUNIT_TWIP );
    aRightMF.SetMax( aRightMF.Normalize( pTblData->GetSpace() ), FUNIT_TWIP );
    aLeftMF.SetMax( aLeftMF.Normalize( pTblData->GetSpace() ), FUNIT_TWIP );
    aWidthMF.SetMin( aWidthMF.Normalize( nMinTableWidth ), FUNIT_TWIP );

}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void    SwFormatTablePage::ActivatePage( const SfxItemSet& rSet )
{
    DBG_ASSERT(pTblData, "Tabellendaten nicht da?")
    if(SFX_ITEM_SET == rSet.GetItemState( FN_TABLE_REP ))
    {
        SwTwips nCurWidth = HORI_FULL != pTblData->GetAlign() ?
                                        pTblData->GetWidth() :
                                            pTblData->GetSpace();
        if(pTblData->GetWidthPercent() == 0 &&
                nCurWidth != aWidthMF.Denormalize(aWidthMF.GetValue(FUNIT_TWIP )))
        {
            aWidthMF.SetPrcntValue(aWidthMF.Normalize(
                            nCurWidth), FUNIT_TWIP);
            aWidthMF.SaveValue();
            nSaveWidth = nCurWidth;
            aLeftMF.SetPrcntValue(aLeftMF.Normalize(
                            pTblData->GetLeftSpace()), FUNIT_TWIP);
            aLeftMF.SaveValue();
            aRightMF.SetPrcntValue(aRightMF.Normalize(
                            pTblData->GetRightSpace()), FUNIT_TWIP);
            aRightMF.SaveValue();
        }
    }

}
/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
int  SwFormatTablePage::DeactivatePage( SfxItemSet* pSet )
{
    // os: VCL sorgt nicht dafuer, dass das aktive Control im
    // dialog bei OK den focus verliert
    aNameED.GrabFocus();
    // Test des Tabellennamens auf Leerzeichen
    String sTblName = aNameED.GetText();
    if(sTblName.Search(' ') != STRING_NOTFOUND)
    {
        InfoBox(this, SW_RES(MSG_WRONG_TABLENAME)).Execute();
        aNameED.GrabFocus();
        return KEEP_PAGE;
    }
    if(pSet)
    {
        FillItemSet(*pSet);
        if(bModified)
        {
            SwTwips lLeft = aLeftMF.Denormalize( aLeftMF.GetValue( FUNIT_TWIP ));
            SwTwips lRight = aRightMF.Denormalize( aRightMF.GetValue( FUNIT_TWIP ));


            if( aLeftMF.GetText() != aLeftMF.GetSavedValue() ||
                                    aRightMF.GetText() != aRightMF.GetSavedValue() )
            {
                pTblData->SetWidthChanged();
                pTblData->SetLeftSpace( lLeft);
                pTblData->SetRightSpace( lRight);
            }

            SwTwips lWidth;
            if (aRelWidthCB.IsChecked() && aRelWidthCB.IsEnabled())
            {
                lWidth = pTblData->GetSpace() - lRight - lLeft;
                USHORT nPercentWidth = (USHORT)aWidthMF.GetValue(FUNIT_CUSTOM);
                if(pTblData->GetWidthPercent() != nPercentWidth)
                {
                    pTblData->SetWidthPercent(nPercentWidth);
                    pTblData->SetWidthChanged();
                }
            }
            else
            {
                pTblData->SetWidthPercent(0);
                lWidth = aWidthMF.Denormalize(aWidthMF.GetValue( FUNIT_TWIP ));
            }
            pTblData->SetWidth(lWidth);

            SwTwips nColSum = 0;
            for(USHORT i = 0; i < pTblData->GetColCount(); i++)
            {
                nColSum += pTblData->GetColumns()[i].nWidth;
            }
            if(nColSum != pTblData->GetWidth())
            {
                SwTwips nMinWidth = std::min((long)MINLAY, (long) (pTblData->GetWidth() / pTblData->GetColCount() - 1));
                SwTwips nDiff = nColSum - pTblData->GetWidth();
                while ( Abs(nDiff) > pTblData->GetColCount() + 1 )
                {
                    SwTwips nSub = nDiff / pTblData->GetColCount();
                    for( i = 0; i < pTblData->GetColCount(); i++)
                    {
                        if(pTblData->GetColumns()[i].nWidth - nMinWidth > nSub)
                        {
                            pTblData->GetColumns()[i].nWidth -= nSub;
                            nDiff -= nSub;
                        }
                        else
                        {
                            nDiff -= pTblData->GetColumns()[i].nWidth - nMinWidth;
                            pTblData->GetColumns()[i].nWidth = nMinWidth;
                        }

                    }
                }
            }

            int nAlign = 0;
            if(aRightBtn.IsChecked())
                nAlign = HORI_RIGHT;
            else if(aLeftBtn.IsChecked())
                nAlign = HORI_LEFT;
            else if(aFromLeftBtn.IsChecked())
                nAlign = HORI_LEFT_AND_WIDTH;
            else if(aCenterBtn.IsChecked())
                nAlign = HORI_CENTER;
            else if(aFreeBtn.IsChecked())
                nAlign = HORI_NONE;
            else if(aFullBtn.IsChecked())
            {
                nAlign = HORI_FULL;
                lWidth = lAutoWidth;
            }
            if(nAlign != pTblData->GetAlign())
            {
                pTblData->SetWidthChanged();
                pTblData->SetAlign(nAlign);
            }


    //      if(  HORI_CENTER && lWidth != (SwTwips)aWidthMF.GetSavedValue())
            if(pTblData->GetWidth() != lWidth )
            {
                pTblData->SetWidthChanged();
                pTblData->SetWidth(
                    nAlign == HORI_FULL ? pTblData->GetSpace() : lWidth );
            }
            if(pTblData->HasWidthChanged())
                pSet->Put(SwPtrItem(FN_TABLE_REP, pTblData));
        }
DEBUG_TBLDLG_TABLEREP(pTblData);
    }
    return TRUE;
}
/*------------------------------------------------------------------------
    Beschreibung: Seite Spaltenkonfiguration
------------------------------------------------------------------------*/
SwTableColumnPage::SwTableColumnPage( Window* pParent,
            const SfxItemSet& rSet ) :
    SfxTabPage(pParent, SW_RES( TP_TABLE_COLUMN ), rSet ),
    aModifyTableCB(this,    SW_RES(CB_MOD_TBL)),
    aProportionalCB(this,   SW_RES(CB_PROP)),
    aSpaceFT(this,          SW_RES(FT_SPACE)),
    aSpaceED(this,          SW_RES(ED_SPACE)),
    aFT1(this,              SW_RES(COL_FT_1)),
    aMF1(this,              SW_RES(COL_MF_1)),
    aFT2(this,              SW_RES(COL_FT_2)),
    aMF2(this,              SW_RES(COL_MF_2)),
    aFT3(this,              SW_RES(COL_FT_3)),
    aMF3(this,              SW_RES(COL_MF_3)),
    aFT4(this,              SW_RES(COL_FT_4)),
    aMF4(this,              SW_RES(COL_MF_4)),
    aFT5(this,              SW_RES(COL_FT_5)),
    aMF5(this,              SW_RES(COL_MF_5)),
    aFT6(this,              SW_RES(COL_FT_6)),
    aMF6(this,              SW_RES(COL_MF_6)),
    aColGB(this,            SW_RES(COL_GB_LAYOUT)),
    aUpBtn(this,            SW_RES(COL_BTN_UP)),
    aDownBtn(this,          SW_RES(COL_BTN_DOWN)),
    nNoOfCols( 0 ),
    nNoOfVisibleCols( 0 ),
    nMinWidth( MINLAY ),
    bModified(FALSE),
    bModifyTbl(FALSE),
    bPercentMode(FALSE),
    nTableWidth(0)
{
    FreeResource();
    SetExchangeSupport();

    pFieldArr[0] = &aMF1;
    pFieldArr[1] = &aMF2;
    pFieldArr[2] = &aMF3;
    pFieldArr[3] = &aMF4;
    pFieldArr[4] = &aMF5;
    pFieldArr[5] = &aMF6;

    pTextArr[0] = &aFT1;
    pTextArr[1] = &aFT2;
    pTextArr[2] = &aFT3;
    pTextArr[3] = &aFT4;
    pTextArr[4] = &aFT5;
    pTextArr[5] = &aFT6;

    const SfxPoolItem* pItem;
    Init((SFX_ITEM_SET == rSet.GetItemState( SID_HTML_MODE, FALSE,&pItem )
        && ((const SfxUInt16Item*)pItem)->GetValue() & HTMLMODE_ON));

};
/*------------------------------------------------------------------------
    Beschreibung: Seite Spaltenkonfiguration
------------------------------------------------------------------------*/
 SwTableColumnPage::~SwTableColumnPage()
{
};

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
SfxTabPage*   SwTableColumnPage::Create( Window* pParent,
                                const SfxItemSet& rAttrSet)
{
    return new SwTableColumnPage( pParent, rAttrSet );
};

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void  SwTableColumnPage::Reset( const SfxItemSet& )
{
    const SfxItemSet& rSet = GetItemSet();

    const SfxPoolItem* pItem;
    if(SFX_ITEM_SET == rSet.GetItemState( FN_TABLE_REP, FALSE, &pItem ))
    {
        pTblData = (SwTableRep*)((const SwPtrItem*) pItem)->GetValue();
        nNoOfVisibleCols = pTblData->GetColCount();
        nNoOfCols = pTblData->GetAllColCount();
        nTableWidth = pTblData->GetAlign() != HORI_FULL &&
                            pTblData->GetAlign() != HORI_LEFT_AND_WIDTH?
                        pTblData->GetWidth() : pTblData->GetSpace();

        for( USHORT i = 0; i < nNoOfCols; i++ )
        {
            if( pTblData->GetColumns()[i].nWidth  < nMinWidth )
                    nMinWidth = pTblData->GetColumns()[i].nWidth;
        }
        long nMinTwips = pFieldArr[0]->Normalize( nMinWidth );
        long nMaxTwips = pFieldArr[0]->Normalize( nTableWidth );
        for( i = 0; (i < MET_FIELDS) && (i < nNoOfVisibleCols); i++ )
        {
            pFieldArr[i]->SetPrcntValue( pFieldArr[i]->Normalize(
                                                GetVisibleWidth(i) ), FUNIT_TWIP );
            pFieldArr[i]->SetMin( nMinTwips , FUNIT_TWIP );
            pFieldArr[i]->SetMax( nMaxTwips , FUNIT_TWIP );
            pFieldArr[i]->Enable();
            pTextArr[i]->Enable();
        }

        if( nNoOfVisibleCols > MET_FIELDS )
            aUpBtn.Enable();
        i = nNoOfVisibleCols;
        while( i < MET_FIELDS )
        {
            pFieldArr[i]->SetText( aEmptyStr );
            pTextArr[i]->Hide();
            i++;
        }
    }
    ActivatePage(rSet);

};

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void  SwTableColumnPage::Init(BOOL bWeb)
{
    FieldUnit aMetric = ::GetDfltMetric(bWeb);
    Link aLkUp = LINK( this, SwTableColumnPage, UpHdl );
    Link aLkDown = LINK( this, SwTableColumnPage, DownHdl );
    Link aLkLF = LINK( this, SwTableColumnPage, LoseFocusHdl );
    for( USHORT i = 0; i < MET_FIELDS; i++ )
    {
        aValueTbl[i] = i;
        SetMetric(*pFieldArr[i], aMetric);
        pFieldArr[i]->SetUpHdl( aLkUp );
        pFieldArr[i]->SetDownHdl( aLkDown );
        pFieldArr[i]->SetLoseFocusHdl( aLkLF );

    }
    SetMetric(aSpaceED, aMetric);

    Link aLk = LINK( this, SwTableColumnPage, AutoClickHdl );
    aUpBtn.SetClickHdl( aLk );
    aDownBtn.SetClickHdl( aLk );

    aLk = LINK( this, SwTableColumnPage, ModeHdl );
    aModifyTableCB .SetClickHdl( aLk );
    aProportionalCB.SetClickHdl( aLk );
};

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
IMPL_LINK( SwTableColumnPage, AutoClickHdl, CheckBox *, pBox )
{
    //Anzeigefenster verschieben
    if(pBox == (CheckBox *)&aDownBtn)
    {
        if(aValueTbl[0] > 0)
        {
            for( USHORT i=0; i < MET_FIELDS; i++ )
                aValueTbl[i] -= 1;
        }
    }
    if(pBox == (CheckBox *)&aUpBtn)
    {
        if( aValueTbl[ MET_FIELDS -1 ] < nNoOfVisibleCols -1  )
        {
            for(USHORT i=0;i < MET_FIELDS;i++)
                aValueTbl[i] += 1;
        }
    }
    for( USHORT i = 0; (i < nNoOfVisibleCols ) && ( i < MET_FIELDS); i++ )
    {
        String sEntry('~');
        sEntry += String::CreateFromInt32( aValueTbl[i] + 1 );
        pTextArr[i]->SetText( sEntry );
    }

    aDownBtn.Enable(aValueTbl[0] > 0);
    aUpBtn.Enable(aValueTbl[ MET_FIELDS -1 ] < nNoOfVisibleCols -1 );
    UpdateCols(0);
    return 0;
};

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
IMPL_LINK_INLINE_START( SwTableColumnPage, UpHdl, PercentField *, pEdit )
{
    bModified = TRUE;
    ModifyHdl( pEdit );
    return 0;
};
IMPL_LINK_INLINE_END( SwTableColumnPage, UpHdl, PercentField *, pEdit )

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
IMPL_LINK_INLINE_START( SwTableColumnPage, DownHdl, PercentField *, pEdit )
{
    bModified = TRUE;
    ModifyHdl( pEdit );
    return 0;
};
IMPL_LINK_INLINE_END( SwTableColumnPage, DownHdl, PercentField *, pEdit )

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
IMPL_LINK_INLINE_START( SwTableColumnPage, LoseFocusHdl, PercentField *, pEdit )
{
    if(pEdit->IsModified())
    {
        bModified = TRUE;
        ModifyHdl( pEdit );
    }
    return 0;
};
IMPL_LINK_INLINE_END( SwTableColumnPage, LoseFocusHdl, PercentField *, pEdit )

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
IMPL_LINK( SwTableColumnPage, ModeHdl, CheckBox*, pBox )
{
    BOOL bCheck = pBox->IsChecked();
    if(pBox == &aProportionalCB)
    {
        if(bCheck)
            aModifyTableCB.Check();
        aModifyTableCB.Enable(!bCheck && bModifyTbl);
    }
    return 0;
};

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
BOOL  SwTableColumnPage::FillItemSet( SfxItemSet& rSet )
{
    for( USHORT i = 0; i < MET_FIELDS; i++ )
    {
        if(pFieldArr[i]->HasFocus())
        {
            LoseFocusHdl(pFieldArr[i]);
            break;
        }
    }

    if(bModified)
    {
        pTblData->SetColsChanged();
    }
    return bModified;
};

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void   SwTableColumnPage::ModifyHdl( PercentField* pEdit )
{
        USHORT nAktPos;
        for( USHORT i = 0; i < MET_FIELDS; i++)
            if(pEdit == pFieldArr[i])
                break;

        SetVisibleWidth(aValueTbl[i], pEdit->Denormalize( pEdit->GetValue( FUNIT_TWIP ) ));
        nAktPos = aValueTbl[i];

        UpdateCols( nAktPos );
};

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void   SwTableColumnPage::UpdateCols( USHORT nAktPos )
{
    SwTwips nSum = 0;

    for(USHORT i = 0; i < nNoOfCols; i++ )
    {
        nSum += (pTblData->GetColumns())[i].nWidth;
    }
    SwTwips nDiff = nSum - nTableWidth;

    BOOL bModifyTbl = aModifyTableCB.IsChecked();
    BOOL bProp =    aProportionalCB.IsChecked();

    if(!bModifyTbl && !bProp )
    {
//      Tabellenbreite bleibt, Differenz wird mit der/den
//      naechsten Zellen ausgeglichen
        while( nDiff )
        {
            if( ++nAktPos == nNoOfVisibleCols)
                nAktPos = 0;
            if( nDiff < 0 )
            {
                SetVisibleWidth(nAktPos, GetVisibleWidth(nAktPos) -nDiff);
                nDiff = 0;
            }
            else if( GetVisibleWidth(nAktPos) >= nDiff + nMinWidth )
            {
                SetVisibleWidth(nAktPos, GetVisibleWidth(nAktPos) -nDiff);
                nDiff = 0;
            }
            if( nDiff > 0 && GetVisibleWidth(nAktPos) > nMinWidth )
            {
                if( nDiff >= (GetVisibleWidth(nAktPos) - nMinWidth) )
                {
                    nDiff -= (GetVisibleWidth(nAktPos) - nMinWidth);
                    SetVisibleWidth(nAktPos, nMinWidth);
                }
                else
                {
                    nDiff = 0;
                    SetVisibleWidth(nAktPos, GetVisibleWidth(nAktPos) -nDiff);
                }
                DBG_ASSERT(nDiff >= 0, "nDiff < 0 kann hier nicht sein!")
            }
        }
    }
    else if(bModifyTbl && !bProp)
    {
//      Differenz wird ueber die Tabellenbreite ausgeglichen,
//      andere Spalten bleiben unveraendert
        DBG_ASSERT(nDiff <= pTblData->GetSpace() - nTableWidth, "Maximum falsch eingestellt" )
        SwTwips nActSpace = pTblData->GetSpace() - nTableWidth;
        if(nDiff > nActSpace)
        {
            nTableWidth = pTblData->GetSpace();
            SetVisibleWidth(nAktPos, GetVisibleWidth(nAktPos) - nDiff + nActSpace );
        }
        else
        {
            nTableWidth += nDiff;
        }
    }
    else if(bModifyTbl & bProp)
    {
//      Alle Spalten werden proportional mitveraendert, die Tabellenbreite wird
//      entsprechend angepasst
        DBG_ASSERT(nDiff * nNoOfVisibleCols <= pTblData->GetSpace() - nTableWidth, "Maximum falsch eingestellt" )
        long nAdd = nDiff;
        if(nDiff * nNoOfVisibleCols > pTblData->GetSpace() - nTableWidth)
        {
            nAdd = (pTblData->GetSpace() - nTableWidth) / nNoOfVisibleCols;
            SetVisibleWidth(nAktPos, GetVisibleWidth(nAktPos) - nDiff + nAdd );
            nDiff = nAdd;
        }
        if(nAdd)
            for(USHORT i = 0; i < nNoOfVisibleCols; i++ )
            {
                if(i == nAktPos)
                    continue;
                SwTwips nVisWidth;
                if((nVisWidth = GetVisibleWidth(i)) + nDiff < MINLAY)
                {
                    nAdd += nVisWidth - MINLAY;
                    SetVisibleWidth(i, MINLAY);
                }
                else
                {
                    SetVisibleWidth(i, nVisWidth + nDiff);
                    nAdd += nDiff;
                }

            }
        nTableWidth += nAdd;

    }
    else
    {
//      Die Differenz wird gleichmaessig auf alle anderen Spalten aufgeteilt
//      die Tabellenbreite bleibt konstant
/*
        SwTwips nDiffn = nDiff/(nNoOfVisibleCols - 1);
        if(nDiff < 0 && (nNoOfVisibleCols - 1) * nDiffn != nDiff)
            nDiffn-- ;
        USHORT nStart = nAktPos++;
        if(nAktPos == nNoOfVisibleCols)
            nStart = 0;
        for(USHORT i = 0; i < nNoOfVisibleCols; i++ )
        {
            if((nVisWidth = GetVisibleWidth(i)) + nDiff < MINLAY)
            {
                nAdd += nVisWidth - MINLAY;
                SetVisibleWidth(i, MINLAY);
            }
        }
*/

    }

DEBUG_TBLDLG_TABLEREP(pTblData);

    if(!bPercentMode)
        aSpaceED.SetValue(aSpaceED.Normalize( pTblData->GetSpace() - nTableWidth) , FUNIT_TWIP);

    for( i = 0; ( i < nNoOfVisibleCols ) && ( i < MET_FIELDS ); i++)
    {
        pFieldArr[i]->SetPrcntValue(pFieldArr[i]->Normalize(
                        GetVisibleWidth(aValueTbl[i]) ), FUNIT_TWIP);
        pFieldArr[i]->ClearModifyFlag();
    }

}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void    SwTableColumnPage::ActivatePage( const SfxItemSet& rSet )
{
    bPercentMode = pTblData->GetWidthPercent() != 0;
    for( USHORT i = 0; (i < MET_FIELDS) && (i < nNoOfVisibleCols); i++ )
    {
        pFieldArr[i]->SetRefValue(pTblData->GetWidth());
        pFieldArr[i]->ShowPercent( bPercentMode );
    }

    USHORT nTblAlign = pTblData->GetAlign();
    if((HORI_FULL != nTblAlign && nTableWidth != pTblData->GetWidth()) ||
    (HORI_FULL == nTblAlign && nTableWidth != pTblData->GetSpace()))
    {
        nTableWidth = HORI_FULL == nTblAlign ?
                                    pTblData->GetSpace() :
                                        pTblData->GetWidth();
        UpdateCols(0);
    }
    bModifyTbl = TRUE;
    if(pTblData->GetWidthPercent() ||
                HORI_FULL == nTblAlign ||
                        pTblData->IsLineSelected()  )
        bModifyTbl = FALSE;
    if(bPercentMode)
    {
        aModifyTableCB  .Check(FALSE);
        aProportionalCB .Check(FALSE);
    }
    else if( !bModifyTbl )
    {
        aProportionalCB.Check(FALSE);
        aModifyTableCB.Check(FALSE);
    }
    aSpaceFT.Enable(!bPercentMode);
    aSpaceED.Enable(!bPercentMode);
    aModifyTableCB.Enable( !bPercentMode && bModifyTbl );
    aProportionalCB.Enable(!bPercentMode && bModifyTbl );

/*  if(pTblData->IsLineSelected() && pTblData->IsComplex())
    {

    }*/
    aSpaceED.SetValue(aSpaceED.Normalize(
                pTblData->GetSpace() - nTableWidth) , FUNIT_TWIP);

}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
int  SwTableColumnPage::DeactivatePage( SfxItemSet* pSet )
{
    if(pSet)
    {
        FillItemSet(*pSet);
        if(HORI_FULL != pTblData->GetAlign() && pTblData->GetWidth() != nTableWidth)
        {
            pTblData->SetWidth(nTableWidth);
            SwTwips nDiff = pTblData->GetSpace() - pTblData->GetWidth() -
                            pTblData->GetLeftSpace() - pTblData->GetRightSpace();
            switch( pTblData->GetAlign()  )
            {
                case HORI_RIGHT:
                    pTblData->SetLeftSpace(pTblData->GetLeftSpace() + nDiff);
                break;
                case HORI_LEFT:
                    pTblData->SetRightSpace(pTblData->GetRightSpace() + nDiff);
                break;
                case HORI_NONE:
                {
                    SwTwips nDiff2 = nDiff/2;
                    if( nDiff > 0 ||
                        (-nDiff2 < pTblData->GetRightSpace() && - nDiff2 < pTblData->GetLeftSpace()))
                    {
                        pTblData->SetRightSpace(pTblData->GetRightSpace() + nDiff2);
                        pTblData->SetLeftSpace(pTblData->GetLeftSpace() + nDiff2);
                    }
                    else
                    {
                        if(pTblData->GetRightSpace() > pTblData->GetLeftSpace())
                        {
                            pTblData->SetLeftSpace(0);
                            pTblData->SetRightSpace(pTblData->GetSpace() - pTblData->GetWidth());
                        }
                        else
                        {
                            pTblData->SetRightSpace(0);
                            pTblData->SetLeftSpace(pTblData->GetSpace() - pTblData->GetWidth());
                        }
                    }
                }
                break;
                case HORI_CENTER:
                    pTblData->SetRightSpace(pTblData->GetRightSpace() + nDiff/2);
                    pTblData->SetLeftSpace(pTblData->GetLeftSpace() + nDiff/2);
                break;
                case HORI_LEFT_AND_WIDTH :
                    if(nDiff > pTblData->GetRightSpace())
                    {
                        pTblData->SetLeftSpace(pTblData->GetSpace() - pTblData->GetWidth());
                    }
                    pTblData->SetRightSpace(
                        pTblData->GetSpace() - pTblData->GetWidth() - pTblData->GetLeftSpace());
                break;
            }
            pTblData->SetWidthChanged();
        }
DEBUG_TBLDLG_TABLEREP(pTblData);
        pSet->Put(SwPtrItem( FN_TABLE_REP, pTblData ));
    }
    return TRUE;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
SwTwips  SwTableColumnPage::GetVisibleWidth(USHORT nPos)
{
    USHORT i=0;

    while( nPos )
    {
        if(pTblData->GetColumns()[i].bVisible && nPos)
            nPos--;
        i++;
    }
    SwTwips nReturn = pTblData->GetColumns()[i].nWidth;
    DBG_ASSERT(i < nNoOfCols, "Array index out of range")
    while(!pTblData->GetColumns()[i].bVisible && (i + 1) < nNoOfCols)
        nReturn += pTblData->GetColumns()[++i].nWidth;

//  return (*ppTableColumns)[i].nWidth;
    return nReturn;
}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void SwTableColumnPage::SetVisibleWidth(USHORT nPos, SwTwips nNewWidth)
{
    USHORT i=0;
    while( nPos )
    {
        if(pTblData->GetColumns()[i].bVisible && nPos)
            nPos--;
        i++;
    }
    DBG_ASSERT(i < nNoOfCols, "Array index out of range")
    pTblData->GetColumns()[i].nWidth = nNewWidth;
    while(!pTblData->GetColumns()[i].bVisible && (i + 1) < nNoOfCols)
        pTblData->GetColumns()[++i].nWidth = 0;

}

/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
SwTableTabDlg::SwTableTabDlg(Window* pParent, SfxItemPool& ,
                    const SfxItemSet* pItemSet, SwWrtShell* pSh ) :
        SfxTabDialog(pParent, SW_RES(DLG_FORMAT_TABLE), pItemSet,0),
        pShell(pSh)
{
    FreeResource();
    nHtmlMode = ::GetHtmlMode(pSh->GetView().GetDocShell());
    AddTabPage(TP_FORMAT_TABLE, &SwFormatTablePage::Create, 0 );
    AddTabPage(TP_TABLE_TEXTFLOW, &SwTextFlowPage::Create, 0 );
    AddTabPage(TP_TABLE_COLUMN, &SwTableColumnPage::Create, 0 );
    AddTabPage(TP_BACKGROUND,   SvxBackgroundTabPage::Create,   0);
    AddTabPage(TP_BORDER,       SvxBorderTabPage::Create,       0);
}


/*------------------------------------------------------------------------
------------------------------------------------------------------------*/
void  SwTableTabDlg::PageCreated(USHORT nId, SfxTabPage& rPage)
{
    if( TP_BACKGROUND == nId )
    {
        //ShowTblControl() zuerst rufen, wegen HTMLMode
        ((SvxBackgroundTabPage&)rPage).ShowTblControl();
        if(!( nHtmlMode & HTMLMODE_ON ) ||
            nHtmlMode & HTMLMODE_SOME_STYLES)
            ((SvxBackgroundTabPage&)rPage).ShowSelector();

    }
    else if(TP_BORDER == nId)
    {
        ((SvxBorderTabPage&)rPage).SetSWMode(SW_BORDER_MODE_TABLE);
    }
    else if(TP_TABLE_TEXTFLOW == nId)
    {
        ((SwTextFlowPage&)rPage).SetShell(pShell);
        const USHORT eType = pShell->GetFrmType(0,TRUE);
        if( !(FRMTYPE_BODY & eType) )
            ((SwTextFlowPage&)rPage).DisablePageBreak();
    }
}

/*-----------------20.08.96 09.43-------------------
--------------------------------------------------*/
SwTableRep::SwTableRep( const SwTabCols& rTabCol, BOOL bCplx )
    : nTblWidth(0),
    nSpace(0),
    nLeftSpace(0),
    nRightSpace(0),
    nAlign(0),
    nWidthPercent(0),
    bLineSelected(FALSE),
    bComplex(bCplx),
    bWidthChanged(FALSE),
    bColsChanged(FALSE)
{
    nAllCols = nColCount = rTabCol.Count();
    pTColumns = new TColumn[ nColCount + 1 ];
    SwTwips nStart = 0,
            nEnd;
    for( USHORT i = 0; i < nAllCols; ++i )
    {
        nEnd  = rTabCol[ i ] - rTabCol.GetLeft();
        pTColumns[ i ].nWidth = nEnd - nStart;
        pTColumns[ i ].bVisible = !rTabCol.IsHidden(i);
        if(!pTColumns[ i ].bVisible)
            nColCount --;
        nStart = nEnd;
    }
    pTColumns[ nAllCols ].nWidth = rTabCol.GetRight() - rTabCol.GetLeft() - nStart;
    pTColumns[ nAllCols ].bVisible = TRUE;
    nColCount++;
    nAllCols++;
}

/*-----------------20.08.96 09.43-------------------
--------------------------------------------------*/
SwTableRep::~SwTableRep()
{
    delete pTColumns;
}

/*-----------------20.08.96 13.33-------------------
--------------------------------------------------*/
BOOL SwTableRep::FillTabCols( SwTabCols& rTabCols ) const
{
    USHORT nOldLeft = rTabCols.GetLeft();
    USHORT nOldRight = rTabCols.GetRight();

    BOOL bSingleLine = FALSE;
    for ( USHORT i = 0; i < rTabCols.Count(); ++i )
        if(!pTColumns[i].bVisible)
        {
            bSingleLine = TRUE;
            break;
        }

DEBUG_TBLDLG_TCOLUMN(pTColumns, nAllCols);

    SwTwips nPos = 0;
    SwTwips nLeft = GetLeftSpace();
    rTabCols.SetLeft((USHORT)nLeft);
    if(bSingleLine)
    {
        // die unsichtbaren Trenner werden aus den alten TabCols genommen
        // die sichtbaren kommen aus pTColumns
        TColumn*    pOldTColumns = new TColumn[nAllCols + 1];
        SwTwips nStart = 0,
                nEnd;
        for(USHORT i = 0; i < nAllCols - 1; i++)
        {
            nEnd  = rTabCols[i] - rTabCols.GetLeft();
            pOldTColumns[i].nWidth = nEnd - nStart;
            pOldTColumns[i].bVisible = !rTabCols.IsHidden(i);
            nStart = nEnd;
        }
        pOldTColumns[nAllCols - 1].nWidth = rTabCols.GetRight() - rTabCols.GetLeft() - nStart;
        pOldTColumns[nAllCols - 1].bVisible = TRUE;

DEBUG_TBLDLG_TCOLUMN(pOldTColumns, nAllCols);

        USHORT nOldPos = 0;
        USHORT nNewPos = 0;
        SwTwips nOld = 0;
        SwTwips nNew = 0;
        BOOL bOld = FALSE;
        BOOL bFirst = TRUE;
        i = 0;

        while ( i < nAllCols -1 )
        {
            while((bFirst || bOld ) && nOldPos < nAllCols )
            {
                nOld += pOldTColumns[nOldPos].nWidth;
                nOldPos++;
                if(!pOldTColumns[nOldPos - 1].bVisible)
                    break;
            }
            while((bFirst || !bOld ) && nNewPos < nAllCols )
            {
                nNew += pTColumns[nNewPos].nWidth;
                nNewPos++;
                if(pOldTColumns[nNewPos - 1].bVisible)
                    break;
            }
            bFirst = FALSE;
            // sie muessen sortiert eingefuegt werden
            bOld = nOld < nNew;
            nPos = USHORT(bOld ? nOld : nNew);
            rTabCols[i] = nPos + nLeft;
            rTabCols.SetHidden( i, bOld );
            i++;
        }
        rTabCols.SetRight(nLeft + nTblWidth);

        delete pOldTColumns;
    }
    else
    {
        for ( i = 0; i < nAllCols - 1; ++i )
        {
            nPos += pTColumns[i].nWidth;
            rTabCols[i] = nPos + rTabCols.GetLeft();
            rTabCols.SetHidden( i, !pTColumns[i].bVisible );
            rTabCols.SetRight(nLeft + pTColumns[nAllCols - 1].nWidth + nPos);
        }
    }

// Rundungsfehler abfangen
    if(Abs((long)nOldLeft - (long)rTabCols.GetLeft()) < 3)
        rTabCols.SetLeft(nOldLeft);

    if(Abs((long)nOldRight - (long)rTabCols.GetRight()) < 3)
        rTabCols.SetRight(nOldRight);

    if(rTabCols.GetRight() > rTabCols.GetRightMax())
        rTabCols.SetRight(rTabCols.GetRightMax());
    return bSingleLine;
}




/*-----------------12.12.96 12.22-------------------
--------------------------------------------------*/
SwTextFlowPage::SwTextFlowPage( Window* pParent,
                                const SfxItemSet& rSet ) :
    SfxTabPage(pParent, SW_RES( TP_TABLE_TEXTFLOW ), rSet ),
    aFlowGB         (this, SW_RES(GB_FLOW            )),
    aPgBrkCB        (this, SW_RES(CB_PAGEBREAK      )),
    aPgBrkRB        (this, SW_RES(RB_BREAKPAGE      )),
    aColBrkRB       (this, SW_RES(RB_BREAKCOLUMN    )),
    aPgBrkBeforeRB  (this, SW_RES(RB_PAGEBREAKBEFORE)),
    aPgBrkAfterRB   (this, SW_RES(RB_PAGEBREAKAFTER )),
    aPageCollCB     (this, SW_RES(CB_PAGECOLL       )),
    aPageCollLB     (this, SW_RES(LB_PAGECOLL       )),
    aPageNoFT       (this, SW_RES(FT_PAGENUM        )),
    aPageNoNF       (this, SW_RES(NF_PAGENUM        )),
    aKeepCB         (this, SW_RES(CB_KEEP           )),
    aSplitCB        (this, SW_RES(CB_SPLIT          )),
    aHeadLineCB     (this, SW_RES(CB_HEADLINE       )),
    aVertOrientGB   (this, SW_RES(GB_VERT_ORIENT    )),
    aTopRB          (this, SW_RES(RB_VERT_TOP       )),
    aCenterRB       (this, SW_RES(RB_VERT_CENTER    )),
    aBottomRB       (this, SW_RES(RB_VERT_BOTTOM    )),
    pShell(0),
    bPageBreak(TRUE),
    bHtmlMode(FALSE)
{
    FreeResource();

    aPgBrkCB.SetClickHdl(LINK(this, SwTextFlowPage, PageBreakHdl_Impl));
    aPgBrkBeforeRB.SetClickHdl(
        LINK( this, SwTextFlowPage, PageBreakPosHdl_Impl ) );
    aPgBrkAfterRB.SetClickHdl(
        LINK( this, SwTextFlowPage, PageBreakPosHdl_Impl ) );
    aPageCollCB.SetClickHdl(
        LINK( this, SwTextFlowPage, ApplyCollClickHdl_Impl ) );
    aColBrkRB.SetClickHdl(
        LINK( this, SwTextFlowPage, PageBreakTypeHdl_Impl ) );
    aPgBrkRB.SetClickHdl(
        LINK( this, SwTextFlowPage, PageBreakTypeHdl_Impl ) );

#ifndef SW_FILEFORMAT_40
    const SfxPoolItem *pItem;
    if(SFX_ITEM_SET == rSet.GetItemState( SID_HTML_MODE, FALSE,&pItem )
        && ((const SfxUInt16Item*)pItem)->GetValue() & HTMLMODE_ON)
#endif
    {
        aKeepCB.Hide();
        aSplitCB.Hide();
    }
}

/*-----------------12.12.96 12.22-------------------
--------------------------------------------------*/
 SwTextFlowPage::~SwTextFlowPage()
{
}

/*-----------------12.12.96 12.22-------------------
--------------------------------------------------*/
SfxTabPage*   SwTextFlowPage::Create( Window* pParent,
                                const SfxItemSet& rAttrSet)
{
    return new SwTextFlowPage(pParent, rAttrSet);
}

/*-----------------12.12.96 12.22-------------------
--------------------------------------------------*/
BOOL  SwTextFlowPage::FillItemSet( SfxItemSet& rSet )
{
    BOOL bModified = FALSE;

    //Ueberschrift wiederholen
    if(aHeadLineCB.IsChecked() != aHeadLineCB.GetSavedValue())
    {
        bModified |= 0 != rSet.Put( SfxBoolItem(FN_PARAM_TABLE_HEADLINE,
                                    aHeadLineCB.IsChecked()));
    }
    if(aKeepCB.IsChecked() != aKeepCB.GetSavedValue())
        bModified |= 0 != rSet.Put( SvxFmtKeepItem( aKeepCB.IsChecked()));

    if(aSplitCB.IsChecked() != aSplitCB.GetSavedValue())
        bModified |= 0 != rSet.Put( SwFmtLayoutSplit( !aSplitCB.IsChecked()));

    const SvxFmtBreakItem* pBreak = (const SvxFmtBreakItem*)GetOldItem( rSet, RES_BREAK );
    const SwFmtPageDesc* pDesc = (const SwFmtPageDesc*) GetOldItem( rSet, RES_PAGEDESC );


    BOOL bState = aPageCollCB.IsChecked();

    //Wenn Seitenvorlage, dann kein Break
    BOOL bPageItemPut = FALSE;
    if ( bState != aPageCollCB.GetSavedValue() ||
         ( bState &&
           aPageCollLB.GetSelectEntryPos() != aPageCollLB.GetSavedValue() )
           || aPageNoNF.IsEnabled() && aPageNoNF.IsValueModified())
    {
        String sPage;

        if ( bState )
        {
            sPage = aPageCollLB.GetSelectEntry();
        }
        USHORT nPgNum = aPageNoNF.GetValue();
        if ( !pDesc || !pDesc->GetPageDesc() ||
            ( pDesc->GetPageDesc() && ((pDesc->GetPageDesc()->GetName() != sPage) ||
                    aPageNoNF.GetSavedValue() != (String)nPgNum)))
        {
            SwFmtPageDesc aFmt( pShell->FindPageDescByName( sPage, TRUE ) );
            aFmt.SetNumOffset(bState ? nPgNum : 0);
            bModified |= 0 != rSet.Put( aFmt );
            bPageItemPut = bState;
        }
    }
    BOOL bIsChecked = aPgBrkCB.IsChecked();
    if ( !bPageItemPut &&
        (   bState != aPageCollCB.GetSavedValue() ||
            bIsChecked != aPgBrkCB.GetSavedValue()              ||
            aPgBrkBeforeRB.IsChecked() != aPgBrkBeforeRB.GetSavedValue()    ||
            aPgBrkRB.IsChecked() != aPgBrkRB.GetSavedValue() ))
    {
        SvxFmtBreakItem aBreak(
            (const SvxFmtBreakItem&)GetItemSet().Get( RES_BREAK ) );

        if(bIsChecked)
        {
            BOOL bBefore = aPgBrkBeforeRB.IsChecked();

            if ( aPgBrkRB.IsChecked() )
            {
                if ( bBefore )
                    aBreak.SetValue( SVX_BREAK_PAGE_BEFORE );
                else
                    aBreak.SetValue( SVX_BREAK_PAGE_AFTER );
            }
            else
            {
                if ( bBefore )
                    aBreak.SetValue( SVX_BREAK_COLUMN_BEFORE );
                else
                    aBreak.SetValue( SVX_BREAK_COLUMN_AFTER );
            }
        }
        else
        {
                aBreak.SetValue( SVX_BREAK_NONE );
        }

        if ( !pBreak || !( *(const SvxFmtBreakItem*)pBreak == aBreak ) )
        {
            bModified |= 0 != rSet.Put( aBreak );
        }
    }

    USHORT nOrient = USHRT_MAX;
    if(aTopRB.IsChecked() &&  !aTopRB.GetSavedValue())
        nOrient = VERT_NONE;
    else if( aCenterRB.IsChecked() && !aCenterRB.GetSavedValue() )
        nOrient = VERT_CENTER;
    else if( aBottomRB.IsChecked() && !aBottomRB.GetSavedValue() )
        nOrient = VERT_BOTTOM;
    if(nOrient != USHRT_MAX)
        bModified |= 0 != rSet.Put(SfxUInt16Item(FN_TABLE_SET_VERT_ALIGN, nOrient));

    return bModified;

}

/*-----------------12.12.96 12.22-------------------
--------------------------------------------------*/
void   SwTextFlowPage::Reset( const SfxItemSet& rSet )
{
    const SfxPoolItem* pItem;
    OfaHtmlOptions* pHtmlOpt = OFF_APP()->GetHtmlOptions();
    BOOL bFlowAllowed = !bHtmlMode || pHtmlOpt->IsPrintLayoutExtension();
    if(bFlowAllowed)
    {
        // Einfuegen der vorhandenen Seitenvorlagen in die Listbox
        const USHORT nCount = pShell->GetPageDescCnt();
        for(USHORT i = 0; i < nCount; ++i)
        {
            const SwPageDesc &rPageDesc = pShell->GetPageDesc(i);
            aPageCollLB.InsertEntry(rPageDesc.GetName());
        }

        for(i = RES_POOLPAGE_BEGIN; i <= RES_POOLPAGE_REGISTER; ++i)
        {
            String aFmtName; GetDocPoolNm( i, aFmtName );
            if( LISTBOX_ENTRY_NOTFOUND == aPageCollLB.GetEntryPos( aFmtName ))
                aPageCollLB.InsertEntry( aFmtName );
        }

        if(SFX_ITEM_SET == rSet.GetItemState( RES_KEEP, FALSE, &pItem ))
        {
            aKeepCB.Check( ((const SvxFmtKeepItem*)pItem)->GetValue() );
            aKeepCB.SaveValue();
        }
        if(SFX_ITEM_SET == rSet.GetItemState( RES_LAYOUT_SPLIT, FALSE, &pItem ))
        {
            aSplitCB.Check( !((const SwFmtLayoutSplit*)pItem)->GetValue() );
            aSplitCB.SaveValue();
        }

        if(bPageBreak)
        {
            if(SFX_ITEM_SET == rSet.GetItemState( RES_PAGEDESC, FALSE, &pItem ))
            {
                String sPageDesc;
                const SwPageDesc* pDesc = ((const SwFmtPageDesc*)pItem)->GetPageDesc();
                aPageNoNF.SetValue(((const SwFmtPageDesc*)pItem)->GetNumOffset());
                if(pDesc)
                    sPageDesc = pDesc->GetName();
                if ( sPageDesc.Len() &&
                        aPageCollLB.GetEntryPos( sPageDesc ) != LISTBOX_ENTRY_NOTFOUND )
                {
                    aPageCollLB.SelectEntry( sPageDesc );
                    aPageCollCB.Check();

                    aPgBrkCB.Enable();
                    aPgBrkRB.Enable();
                    aColBrkRB.Enable();
                    aPgBrkBeforeRB.Enable();
                    aPgBrkAfterRB.Enable();
                    aPageCollCB.Enable();
                    aPgBrkCB.Check();

                    aPgBrkCB.Check( TRUE );
                    aColBrkRB.Check( FALSE );
                    aPgBrkBeforeRB.Check( TRUE );
                    aPgBrkAfterRB.Check( FALSE );
                }
                else
                {
                    aPageCollLB.SetNoSelection();
                    aPageCollCB.Check(FALSE);
                }
            }

            if(SFX_ITEM_SET == rSet.GetItemState( RES_BREAK, FALSE, &pItem ))
            {
                const SvxFmtBreakItem* pPageBreak = (const SvxFmtBreakItem*)pItem;
                SvxBreak eBreak = (SvxBreak)pPageBreak->GetValue();

                if ( eBreak != SVX_BREAK_NONE )
                {
                    aPgBrkCB.Check();
                    aPageCollCB.Enable(FALSE);
                    aPageCollLB.Enable(FALSE);
                    aPageNoFT.Enable(FALSE);
                    aPageNoNF.Enable(FALSE);
                }
                switch ( eBreak )
                {
                    case SVX_BREAK_PAGE_BEFORE:
                        aPgBrkRB.Check( TRUE );
                        aColBrkRB.Check( FALSE );
                        aPgBrkBeforeRB.Check( TRUE );
                        aPgBrkAfterRB.Check( FALSE );
                        break;
                    case SVX_BREAK_PAGE_AFTER:
                        aPgBrkRB.Check( TRUE );
                        aColBrkRB.Check( FALSE );
                        aPgBrkBeforeRB.Check( FALSE );
                        aPgBrkAfterRB.Check( TRUE );
                        break;
                    case SVX_BREAK_COLUMN_BEFORE:
                        aPgBrkRB.Check( FALSE );
                        aColBrkRB.Check( TRUE );
                        aPgBrkBeforeRB.Check( TRUE );
                        aPgBrkAfterRB.Check( FALSE );
                        break;
                    case SVX_BREAK_COLUMN_AFTER:
                        aPgBrkRB.Check( FALSE );
                        aColBrkRB.Check( TRUE );
                        aPgBrkBeforeRB.Check( FALSE );
                        aPgBrkAfterRB.Check( TRUE );
                        break;
                }

            }
            if ( aPgBrkBeforeRB.IsChecked() )
                PageBreakPosHdl_Impl( &aPgBrkBeforeRB );
            else if ( aPgBrkAfterRB.IsChecked() )
                PageBreakPosHdl_Impl( &aPgBrkAfterRB );
            PageBreakHdl_Impl( &aPgBrkCB );
        }
    }
    else
    {
        aPgBrkRB.Enable(FALSE);
        aColBrkRB.Enable(FALSE);
        aPgBrkBeforeRB.Enable(FALSE);
        aPgBrkAfterRB.Enable(FALSE);
        aKeepCB .Enable(FALSE);
        aSplitCB.Enable(FALSE);
        aPgBrkCB.Enable(FALSE);
        aPageCollCB.Enable(FALSE);
        aPageCollLB.Enable(FALSE);
    }

    if(SFX_ITEM_SET == rSet.GetItemState( FN_PARAM_TABLE_HEADLINE, FALSE, &pItem ))
    {
        aHeadLineCB.Check( ((const SfxBoolItem*)pItem)->GetValue() );
        aHeadLineCB.SaveValue();
    }
    if ( rSet.GetItemState(FN_TABLE_SET_VERT_ALIGN) > SFX_ITEM_AVAILABLE )
    {
        USHORT nVert = ((const SfxUInt16Item&)rSet.Get(FN_TABLE_SET_VERT_ALIGN)).GetValue();
        switch(nVert)
        {
            case VERT_NONE:     aTopRB.Check();     break;
            case VERT_CENTER:   aCenterRB.Check();  break;
            case VERT_BOTTOM:   aBottomRB.Check();  break;
        }
    }

    aPageCollCB.SaveValue();
    aPageCollLB.SaveValue();
    aPgBrkCB.SaveValue();
    aPgBrkRB.SaveValue();
    aColBrkRB.SaveValue();
    aPgBrkBeforeRB.SaveValue();
    aPgBrkAfterRB.SaveValue();
    aPageNoNF.SaveValue();
    aTopRB.SaveValue();
    aCenterRB.SaveValue();
    aBottomRB.SaveValue();
}
/*-----------------16.04.98 14:48-------------------

--------------------------------------------------*/

void SwTextFlowPage::SetShell(SwWrtShell* pSh)
{
    pShell = pSh;
    bHtmlMode = 0 != (::GetHtmlMode(pShell->GetView().GetDocShell()) & HTMLMODE_ON);
    if(bHtmlMode)
    {
        aPageNoNF.Enable(FALSE);
        aPageNoFT.Enable(FALSE);
    }
}

/*-----------------12.12.96 16.18-------------------
--------------------------------------------------*/
IMPL_LINK( SwTextFlowPage, PageBreakHdl_Impl, CheckBox*, EMPTYARG )
{
    if( aPgBrkCB.IsChecked() )
    {
            aPgBrkRB.       Enable();
            aColBrkRB.      Enable();
            aPgBrkBeforeRB. Enable();
            aPgBrkAfterRB.  Enable();

            if ( aPgBrkRB.IsChecked() && aPgBrkBeforeRB.IsChecked() )
            {
                aPageCollCB.Enable();

                BOOL bEnable = aPageCollCB.IsChecked() &&
                                            aPageCollLB.GetEntryCount();
                aPageCollLB.Enable(bEnable);
                if(!bHtmlMode)
                {
                    aPageNoFT.Enable(bEnable);
                    aPageNoNF.Enable(bEnable);
                }
            }
    }
    else
    {
            aPageCollCB.Check( FALSE );
            aPageCollCB.Enable(FALSE);
            aPageCollLB.Enable(FALSE);
            aPageNoFT.Enable(FALSE);
            aPageNoNF.Enable(FALSE);
            aPgBrkRB.       Enable(FALSE);
            aColBrkRB.      Enable(FALSE);
            aPgBrkBeforeRB. Enable(FALSE);
            aPgBrkAfterRB.  Enable(FALSE);
    }
    return 0;
}

/*-----------------12.12.96 16.18-------------------
--------------------------------------------------*/
IMPL_LINK( SwTextFlowPage, ApplyCollClickHdl_Impl, CheckBox*, EMPTYARG )
{
    BOOL bEnable = FALSE;
    if ( aPageCollCB.IsChecked() &&
         aPageCollLB.GetEntryCount() )
    {
        bEnable = TRUE;
        aPageCollLB.SelectEntryPos( 0 );
    }
    else
    {
        aPageCollLB.SetNoSelection();
    }
    aPageCollLB.Enable(bEnable);
    if(!bHtmlMode)
    {
        aPageNoFT.Enable(bEnable);
        aPageNoNF.Enable(bEnable);
    }
    return 0;
}

/*-----------------12.12.96 16.18-------------------
--------------------------------------------------*/
IMPL_LINK( SwTextFlowPage, PageBreakPosHdl_Impl, RadioButton*, pBtn )
{
    if ( aPgBrkCB.IsChecked() )
    {
        if ( pBtn == &aPgBrkBeforeRB )
        {
            aPageCollCB.Enable();

            BOOL bEnable = aPageCollCB.IsChecked()  &&
                                        aPageCollLB.GetEntryCount();

            aPageCollLB.Enable(bEnable);
            if(!bHtmlMode)
            {
                aPageNoFT.Enable(bEnable);
                aPageNoNF.Enable(bEnable);
            }
        }
        else if ( pBtn == &aPgBrkAfterRB )
        {
            aPageCollCB .Check( FALSE );
            aPageCollCB .Enable(FALSE);
            aPageCollLB .Enable(FALSE);
            aPageNoFT   .Enable(FALSE);
            aPageNoNF   .Enable(FALSE);
        }
    }
    return 0;
}

/*-----------------12.12.96 16.18-------------------
--------------------------------------------------*/
IMPL_LINK( SwTextFlowPage, PageBreakTypeHdl_Impl, RadioButton*, pBtn )
{
    if ( pBtn == &aColBrkRB || aPgBrkAfterRB.IsChecked() )
    {
        aPageCollCB .Check(FALSE);
        aPageCollCB .Enable(FALSE);
        aPageCollLB .Enable(FALSE);
        aPageNoFT   .Enable(FALSE);
        aPageNoNF   .Enable(FALSE);
    }
    else if ( aPgBrkBeforeRB.IsChecked() )
        PageBreakPosHdl_Impl( &aPgBrkBeforeRB );
    return 0;
}

/*-----------------30.05.97 07:37-------------------

--------------------------------------------------*/
void SwTextFlowPage::DisablePageBreak()
{
    bPageBreak = FALSE;
    aPgBrkCB       .Disable();
    aPgBrkRB       .Disable();
    aColBrkRB      .Disable();
    aPgBrkBeforeRB .Disable();
    aPgBrkAfterRB  .Disable();
    aPageCollCB    .Disable();
    aPageCollLB    .Disable();
    aPageNoFT      .Disable();
    aPageNoNF      .Disable();
}



