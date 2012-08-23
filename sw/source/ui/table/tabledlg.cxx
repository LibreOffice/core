/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include <hintids.hxx>
#include <comphelper/string.hxx>
#include <vcl/msgbox.hxx>
#include <svl/stritem.hxx>
#include <svl/intitem.hxx>
#include <editeng/keepitem.hxx>
#include <editeng/brkitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <svl/ctloptions.hxx>
#include <swmodule.hxx>
#include <fmtornt.hxx>
#include <fmtpdsc.hxx>
#include <fmtlsplt.hxx>

#include <svtools/htmlcfg.hxx>
#include <fmtrowsplt.hxx>
#include <svx/htmlmode.hxx>

#include "access.hrc"

#include <docsh.hxx>
#include <wrtsh.hxx>
#include <view.hxx>
#include <viewopt.hxx>
#include <uitool.hxx>
#include <frmatr.hxx>
#include <tabledlg.hxx>
#include <tablepg.hxx>
#include <tablemgr.hxx>
#include <pagedesc.hxx>
#include <uiitems.hxx>
#include <poolfmt.hxx>
#include <SwStyleNameMapper.hxx>

#include <cmdid.h>
#include <tabledlg.hrc>
#include <table.hrc>
#include <svx/svxids.hrc>
#include <svx/dialogs.hrc>
#include <svx/flagsdef.hxx>
#include <svx/svxdlg.hxx>

using namespace ::com::sun::star;


SwFormatTablePage::SwFormatTablePage( Window* pParent, const SfxItemSet& rSet ) :
    SfxTabPage(pParent, SW_RES( TP_FORMAT_TABLE ), rSet ),
    aOptionsFL(this,    SW_RES( FL_OPTIONS )),
    aNameFT(this,       SW_RES( FT_NAME )),
    aNameED(this,       SW_RES( ED_NAME )),
    aWidthFT(this,      SW_RES( FT_WIDTH )),
    aWidthMF(this,      SW_RES( ED_WIDTH )),
    aRelWidthCB(this,   SW_RES( CB_REL_WIDTH )),

    aPosFL(this,       SW_RES( FL_POS )),
    aFullBtn(this,      SW_RES( RB_FULL )),
    aLeftBtn(this,      SW_RES( RB_LEFT )),
    aFromLeftBtn(this,  SW_RES( RB_FROM_LEFT )),
    aRightBtn(this,     SW_RES( RB_RIGHT )),
    aCenterBtn(this,    SW_RES( RB_CENTER )),
    aFreeBtn(this,      SW_RES( RB_FREE )),

    aDistFL(this,       SW_RES( FL_DIST )),
    aLeftFT(this,       SW_RES( FT_LEFT_DIST )),
    aLeftMF(this,       SW_RES( ED_LEFT_DIST )),
    aRightFT(this,      SW_RES( FT_RIGHT_DIST )),
    aRightMF(this,      SW_RES( ED_RIGHT_DIST )),
    aTopFT (this,       SW_RES( FT_TOP_DIST )),
    aTopMF(this,        SW_RES( ED_TOP_DIST )),
    aBottomFT(this,     SW_RES( FT_BOTTOM_DIST )),
    aBottomMF(this,     SW_RES( ED_BOTTOM_DIST )),

    aPropertiesFL(this,     SW_RES( FL_PROPERTIES    )),
    aTextDirectionFT(this,  SW_RES( FT_TEXTORIENTATION )),
    aTextDirectionLB(this,  SW_RES( LB_TEXTORIENTATION )),

    pTblData(0),
    nSaveWidth(0),
    nMinTableWidth(MINLAY),
    bModified(sal_False),
    bFull(0),
    bHtmlMode(sal_False)
{
    FreeResource();
    SetExchangeSupport();

    const SfxPoolItem* pItem;
    if(SFX_ITEM_SET == rSet.GetItemState(SID_HTML_MODE, sal_False, &pItem))
        bHtmlMode = 0 != (((const SfxUInt16Item*)pItem)->GetValue() & HTMLMODE_ON);

    sal_Bool bCTL = SW_MOD()->GetCTLOptions().IsCTLFontEnabled();
    if( !bHtmlMode && bCTL )
    {
        aPropertiesFL.Show();
        aTextDirectionFT.Show();
        aTextDirectionLB.Show();
    }

    Init();
}


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

    aLk = LINK( this, SwFormatTablePage, UpDownLoseFocusHdl );
    aTopMF.SetUpHdl( aLk );
    aBottomMF.SetUpHdl( aLk );
    aRightMF.SetUpHdl( aLk );
    aLeftMF.SetUpHdl( aLk );
    aWidthMF.SetUpHdl( aLk );

    aTopMF.SetDownHdl( aLk );
    aBottomMF.SetDownHdl( aLk );
    aRightMF.SetDownHdl( aLk );
    aLeftMF.SetDownHdl( aLk );
    aWidthMF.SetDownHdl( aLk );

    aTopMF.SetLoseFocusHdl( aLk );
    aBottomMF.SetLoseFocusHdl( aLk );
    aRightMF.SetLoseFocusHdl( aLk );
    aLeftMF.SetLoseFocusHdl( aLk );
    aWidthMF.SetLoseFocusHdl( aLk );

    aRelWidthCB.SetClickHdl(LINK( this, SwFormatTablePage, RelWidthClickHdl ));
}

/*------------------------------------------------------------------------*/

IMPL_LINK( SwFormatTablePage, RelWidthClickHdl, CheckBox *, pBtn )
{
    OSL_ENSURE(pTblData, "table data not available?");
    sal_Bool bIsChecked = pBtn->IsChecked();
    sal_Int64 nLeft  = aLeftMF.DenormalizePercent(aLeftMF.GetValue(FUNIT_TWIP ));
    sal_Int64 nRight = aRightMF.DenormalizePercent(aRightMF.GetValue(FUNIT_TWIP ));
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
        aLeftMF.MetricField::SetMax(99); //
        aRightMF.MetricField::SetMax(99);//
        aLeftMF.SetPrcntValue(aLeftMF.NormalizePercent(nLeft ), FUNIT_TWIP );
        aRightMF.SetPrcntValue(aRightMF.NormalizePercent(nRight ), FUNIT_TWIP );
    }
    else
        ModifyHdl(&aLeftMF);    // Werte wieder korrigieren

    if(aFreeBtn.IsChecked())
    {
        sal_Bool bEnable = !pBtn->IsChecked();
        aRightMF.Enable(bEnable);
        aRightFT.Enable(bEnable);
    }
    bModified = sal_True;

    return 0;
}


IMPL_LINK( SwFormatTablePage, AutoClickHdl, CheckBox *, pBox )
{
    sal_Bool bRestore = sal_True,
         bLeftEnable = sal_False,
         bRightEnable= sal_False,
         bWidthEnable= sal_False,
         bOthers = sal_True;
    if( (RadioButton *) pBox == &aFullBtn )
    {
        aLeftMF.SetPrcntValue(0);
        aRightMF.SetPrcntValue(0);
        nSaveWidth = static_cast< SwTwips >(aWidthMF.DenormalizePercent(aWidthMF.GetValue(FUNIT_TWIP )));
        aWidthMF.SetPrcntValue(aWidthMF.NormalizePercent(pTblData->GetSpace() ), FUNIT_TWIP );
        bFull = sal_True;
        bRestore = sal_False;
    }
    else if( (RadioButton *) pBox == &aLeftBtn )
    {
        bRightEnable = bWidthEnable = sal_True;
        aLeftMF.SetPrcntValue(0);
    }
    else if( (RadioButton *) pBox == &aFromLeftBtn )
    {
        bLeftEnable = bWidthEnable = sal_True;
        aRightMF.SetPrcntValue(0);
    }
    else if( (RadioButton *) pBox == &aRightBtn )
    {
        bLeftEnable = bWidthEnable = sal_True;
        aRightMF.SetPrcntValue(0);
    }
    else if( ( RadioButton * ) pBox == &aCenterBtn )
    {
        bLeftEnable = bWidthEnable = sal_True;
    }
    else if( ( RadioButton * ) pBox == &aFreeBtn )
    {
        RightModify();
        bLeftEnable = sal_True;
        bWidthEnable = sal_True;
        bOthers = sal_False;
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
        bFull = sal_False;
        aWidthMF.SetPrcntValue(aWidthMF.NormalizePercent(nSaveWidth ), FUNIT_TWIP );
    }
    ModifyHdl(&aWidthMF);
    bModified = sal_True;
    return 0;
}

/*----------------------------------------------------------------------*/
void SwFormatTablePage::RightModify()
{
    if(aFreeBtn.IsChecked())
    {
        sal_Bool bEnable = aRightMF.GetValue() == 0;
        aRelWidthCB.Enable(bEnable);
        if ( !bEnable )
        {
            aRelWidthCB.Check(sal_False);
            RelWidthClickHdl(&aRelWidthCB);
        }
        bEnable = aRelWidthCB.IsChecked();
        aRightMF.Enable(!bEnable);
        aRightFT.Enable(!bEnable);
    }
}


IMPL_LINK_INLINE_START( SwFormatTablePage, UpDownLoseFocusHdl, MetricField *, pEdit )
{
    if( &aRightMF == pEdit)
        RightModify();
    ModifyHdl( pEdit );
    return 0;
}
IMPL_LINK_INLINE_END( SwFormatTablePage, UpDownLoseFocusHdl, MetricField *, pEdit )

void  SwFormatTablePage::ModifyHdl( Edit* pEdit )
{

    SwTwips nCurWidth  = static_cast< SwTwips >(aWidthMF.DenormalizePercent(aWidthMF.GetValue( FUNIT_TWIP )));
    SwTwips nPrevWidth = nCurWidth;
    SwTwips nRight = static_cast< SwTwips >(aRightMF.DenormalizePercent(aRightMF.GetValue( FUNIT_TWIP )));
    SwTwips nLeft  = static_cast< SwTwips >(aLeftMF.DenormalizePercent(aLeftMF.GetValue( FUNIT_TWIP )));
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
            sal_Bool bCenter = aCenterBtn.IsChecked();
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
        aWidthMF.SetPrcntValue( aWidthMF.NormalizePercent( nCurWidth ), FUNIT_TWIP );
    aRightMF.SetPrcntValue( aRightMF.NormalizePercent( nRight ), FUNIT_TWIP );
    aLeftMF.SetPrcntValue( aLeftMF.NormalizePercent( nLeft ), FUNIT_TWIP );
    bModified = sal_True;
}


SfxTabPage*  SwFormatTablePage::Create( Window* pParent,
                                   const SfxItemSet& rAttrSet)
{
    return new SwFormatTablePage( pParent, rAttrSet );
}


sal_Bool  SwFormatTablePage::FillItemSet( SfxItemSet& rCoreSet )
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
            aULSpace.SetUpper( (sal_uInt16) aTopMF.Denormalize(
                                        aTopMF.GetValue( FUNIT_TWIP )));
            aULSpace.SetLower( (sal_uInt16) aBottomMF.Denormalize(
                                        aBottomMF.GetValue( FUNIT_TWIP )));
            rCoreSet.Put(aULSpace);
        }

    }
    if(aNameED.GetText() != aNameED.GetSavedValue())
    {
        rCoreSet.Put(SfxStringItem( FN_PARAM_TABLE_NAME, aNameED.GetText()));
        bModified = sal_True;
    }

    sal_uInt16 nPos;
    if( aTextDirectionLB.IsVisible() &&
        ( nPos = aTextDirectionLB.GetSelectEntryPos() ) !=
                                            aTextDirectionLB.GetSavedValue() )
    {
        sal_uInt32 nDirection = (sal_uInt32)(sal_uIntPtr)aTextDirectionLB.GetEntryData( nPos );
        rCoreSet.Put( SvxFrameDirectionItem( (SvxFrameDirection)nDirection, RES_FRAMEDIR));
        bModified = sal_True;
    }

    return bModified;
}


void  SwFormatTablePage::Reset( const SfxItemSet& )
{
    const SfxItemSet& rSet = GetItemSet();
    const SfxPoolItem*  pItem;

    if(bHtmlMode)
    {
        aNameED .Disable();
        aTopFT  .Hide();
        aTopMF  .Hide();
        aBottomFT.Hide();
        aBottomMF.Hide();
        aFreeBtn.Enable(sal_False);
    }
    FieldUnit aMetric = ::GetDfltMetric(bHtmlMode);
    SetMetric( aWidthMF, aMetric );
    SetMetric( aRightMF, aMetric );
    SetMetric( aLeftMF, aMetric );
    SetMetric( aTopMF, aMetric );
    SetMetric( aBottomMF, aMetric );

    //Name
    if(SFX_ITEM_SET == rSet.GetItemState( FN_PARAM_TABLE_NAME, sal_False, &pItem ))
    {
        aNameED.SetText(((const SfxStringItem*)pItem)->GetValue());
        aNameED.SaveValue();
    }

    if(SFX_ITEM_SET == rSet.GetItemState( FN_TABLE_REP, sal_False, &pItem ))
    {
        pTblData = (SwTableRep*)((const SwPtrItem*) pItem)->GetValue();
        nMinTableWidth = pTblData->GetColCount() * MINLAY;

        if(pTblData->GetWidthPercent())
        {
            aRelWidthCB.Check(sal_True);
            RelWidthClickHdl(&aRelWidthCB);
            aWidthMF.SetPrcntValue(pTblData->GetWidthPercent(), FUNIT_CUSTOM);

            aWidthMF.SaveValue();
            nSaveWidth = static_cast< SwTwips >(aWidthMF.GetValue(FUNIT_CUSTOM));
        }
        else
        {
            aWidthMF.SetPrcntValue(aWidthMF.NormalizePercent(
                    pTblData->GetWidth()), FUNIT_TWIP);
            aWidthMF.SaveValue();
            nSaveWidth = pTblData->GetWidth();
            nMinTableWidth = Min( nSaveWidth, nMinTableWidth );
        }

        aWidthMF.SetRefValue(pTblData->GetSpace());
        aWidthMF.SetLast(aWidthMF.NormalizePercent( pTblData->GetSpace() ));
        aLeftMF.SetLast(aLeftMF.NormalizePercent( pTblData->GetSpace() ));
        aRightMF.SetLast(aRightMF.NormalizePercent( pTblData->GetSpace() ));

        aLeftMF.SetPrcntValue(aLeftMF.NormalizePercent(
                    pTblData->GetLeftSpace()), FUNIT_TWIP);
        aRightMF.SetPrcntValue(aRightMF.NormalizePercent(
                    pTblData->GetRightSpace()), FUNIT_TWIP);
        aLeftMF.SaveValue();
        aRightMF.SaveValue();

        nOldAlign = pTblData->GetAlign();

        sal_Bool bSetRight = sal_False, bRightEnable = sal_False,
             bSetLeft  = sal_False, bLeftEnable  = sal_False;
        switch( nOldAlign )
        {
            case text::HoriOrientation::NONE:
                aFreeBtn.Check();
                if(aRelWidthCB.IsChecked())
                    bSetRight = sal_True;
            break;
            case text::HoriOrientation::FULL:
            {
                bSetRight = bSetLeft = sal_True;
                aFullBtn.Check();
                aWidthMF.Enable(sal_False);
                aRelWidthCB.Enable(sal_False);
                aWidthFT.Enable(sal_False);
            }
            break;
            case text::HoriOrientation::LEFT:
            {
                bSetLeft = sal_True;
                aLeftBtn.Check();
            }
            break;
            case text::HoriOrientation::LEFT_AND_WIDTH :
            {
                bSetRight = sal_True;
                aFromLeftBtn.Check();
            }
            break;
            case text::HoriOrientation::RIGHT:
            {
                bSetRight = sal_True;
                aRightBtn.Check();
            }
            break;
            case text::HoriOrientation::CENTER:
            {
                bSetRight = sal_True;
                aCenterBtn.Check();
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
    if(SFX_ITEM_SET == rSet.GetItemState( RES_UL_SPACE, sal_False,&pItem ))
    {
        aTopMF.SetValue(aTopMF.Normalize(
                        ((const SvxULSpaceItem*)pItem)->GetUpper()), FUNIT_TWIP);
        aBottomMF.SetValue(aBottomMF.Normalize(
                        ((const SvxULSpaceItem*)pItem)->GetLower()), FUNIT_TWIP);
        aTopMF.SaveValue();
        aBottomMF.SaveValue();
    }

    //text direction
    if( SFX_ITEM_SET == rSet.GetItemState( RES_FRAMEDIR, sal_True, &pItem ) )
    {
        sal_uIntPtr nVal  = ((SvxFrameDirectionItem*)pItem)->GetValue();
        sal_uInt16 nPos = aTextDirectionLB.GetEntryPos( (void*) nVal );
        aTextDirectionLB.SelectEntryPos( nPos );
        aTextDirectionLB.SaveValue();
    }

    aWidthMF.SetMax( 2*aWidthMF.NormalizePercent( pTblData->GetSpace() ), FUNIT_TWIP );
    aRightMF.SetMax( aRightMF.NormalizePercent( pTblData->GetSpace() ), FUNIT_TWIP );
    aLeftMF.SetMax( aLeftMF.NormalizePercent( pTblData->GetSpace() ), FUNIT_TWIP );
    aWidthMF.SetMin( aWidthMF.NormalizePercent( nMinTableWidth ), FUNIT_TWIP );

}


void    SwFormatTablePage::ActivatePage( const SfxItemSet& rSet )
{
    OSL_ENSURE(pTblData, "table data not available?");
    if(SFX_ITEM_SET == rSet.GetItemState( FN_TABLE_REP ))
    {
        SwTwips nCurWidth = text::HoriOrientation::FULL != pTblData->GetAlign() ?
                                        pTblData->GetWidth() :
                                            pTblData->GetSpace();
        if(pTblData->GetWidthPercent() == 0 &&
                nCurWidth != aWidthMF.DenormalizePercent(aWidthMF.GetValue(FUNIT_TWIP )))
        {
            aWidthMF.SetPrcntValue(aWidthMF.NormalizePercent(
                            nCurWidth), FUNIT_TWIP);
            aWidthMF.SaveValue();
            nSaveWidth = nCurWidth;
            aLeftMF.SetPrcntValue(aLeftMF.NormalizePercent(
                            pTblData->GetLeftSpace()), FUNIT_TWIP);
            aLeftMF.SaveValue();
            aRightMF.SetPrcntValue(aRightMF.NormalizePercent(
                            pTblData->GetRightSpace()), FUNIT_TWIP);
            aRightMF.SaveValue();
        }
    }

}

int  SwFormatTablePage::DeactivatePage( SfxItemSet* _pSet )
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
    if(_pSet)
    {
        FillItemSet(*_pSet);
        if(bModified)
        {
            SwTwips lLeft  = static_cast< SwTwips >(aLeftMF.DenormalizePercent(aLeftMF.GetValue( FUNIT_TWIP )));
            SwTwips lRight = static_cast< SwTwips >(aRightMF.DenormalizePercent(aRightMF.GetValue( FUNIT_TWIP )));


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
                sal_uInt16 nPercentWidth = (sal_uInt16)aWidthMF.GetValue(FUNIT_CUSTOM);
                if(pTblData->GetWidthPercent() != nPercentWidth)
                {
                    pTblData->SetWidthPercent(nPercentWidth);
                    pTblData->SetWidthChanged();
                }
            }
            else
            {
                pTblData->SetWidthPercent(0);
                lWidth = static_cast< SwTwips >(aWidthMF.DenormalizePercent(aWidthMF.GetValue( FUNIT_TWIP )));
            }
            pTblData->SetWidth(lWidth);

            SwTwips nColSum = 0;
            sal_uInt16 i;

            for( i = 0; i < pTblData->GetColCount(); i++)
            {
                nColSum += pTblData->GetColumns()[i].nWidth;
            }
            if(nColSum != pTblData->GetWidth())
            {
                SwTwips nMinWidth = Min( (long)MINLAY,
                                    (long) (pTblData->GetWidth() /
                                            pTblData->GetColCount() - 1));
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

            sal_Int16 nAlign = 0;
            if(aRightBtn.IsChecked())
                nAlign = text::HoriOrientation::RIGHT;
            else if(aLeftBtn.IsChecked())
                nAlign = text::HoriOrientation::LEFT;
            else if(aFromLeftBtn.IsChecked())
                nAlign = text::HoriOrientation::LEFT_AND_WIDTH;
            else if(aCenterBtn.IsChecked())
                nAlign = text::HoriOrientation::CENTER;
            else if(aFreeBtn.IsChecked())
                nAlign = text::HoriOrientation::NONE;
            else if(aFullBtn.IsChecked())
            {
                nAlign = text::HoriOrientation::FULL;
                lWidth = lAutoWidth;
            }
            if(nAlign != pTblData->GetAlign())
            {
                pTblData->SetWidthChanged();
                pTblData->SetAlign(nAlign);
            }

            if(pTblData->GetWidth() != lWidth )
            {
                pTblData->SetWidthChanged();
                pTblData->SetWidth(
                    nAlign == text::HoriOrientation::FULL ? pTblData->GetSpace() : lWidth );
            }
            if(pTblData->HasWidthChanged())
                _pSet->Put(SwPtrItem(FN_TABLE_REP, pTblData));
        }
    }
    return sal_True;
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

    aColFL(this,            SW_RES(COL_FL_LAYOUT)),
    aUpBtn(this,            SW_RES(COL_BTN_UP)),
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
    aDownBtn(this,          SW_RES(COL_BTN_DOWN)),

    nTableWidth(0),
    nMinWidth( MINLAY ),
    nNoOfCols( 0 ),
    nNoOfVisibleCols( 0 ),
    bModified(sal_False),
    bModifyTbl(sal_False),
    bPercentMode(sal_False)
{
    FreeResource();
    SetExchangeSupport();

    aDownBtn.SetAccessibleRelationMemberOf(&aColFL);
    aUpBtn.SetAccessibleRelationMemberOf(&aColFL);

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
    Init((SFX_ITEM_SET == rSet.GetItemState( SID_HTML_MODE, sal_False,&pItem )
        && ((const SfxUInt16Item*)pItem)->GetValue() & HTMLMODE_ON));

};
/*------------------------------------------------------------------------
    Beschreibung: Seite Spaltenkonfiguration
------------------------------------------------------------------------*/
 SwTableColumnPage::~SwTableColumnPage()
{
};


SfxTabPage*   SwTableColumnPage::Create( Window* pParent,
                                const SfxItemSet& rAttrSet)
{
    return new SwTableColumnPage( pParent, rAttrSet );
};


void  SwTableColumnPage::Reset( const SfxItemSet& )
{
    const SfxItemSet& rSet = GetItemSet();

    const SfxPoolItem* pItem;
    if(SFX_ITEM_SET == rSet.GetItemState( FN_TABLE_REP, sal_False, &pItem ))
    {
        pTblData = (SwTableRep*)((const SwPtrItem*) pItem)->GetValue();
        nNoOfVisibleCols = pTblData->GetColCount();
        nNoOfCols = pTblData->GetAllColCount();
        nTableWidth = pTblData->GetAlign() != text::HoriOrientation::FULL &&
                            pTblData->GetAlign() != text::HoriOrientation::LEFT_AND_WIDTH?
                        pTblData->GetWidth() : pTblData->GetSpace();

        sal_uInt16 i;
        for( i = 0; i < nNoOfCols; i++ )
        {
            if( pTblData->GetColumns()[i].nWidth  < nMinWidth )
                    nMinWidth = pTblData->GetColumns()[i].nWidth;
        }
        sal_Int64 nMinTwips = pFieldArr[0]->NormalizePercent( nMinWidth );
        sal_Int64 nMaxTwips = pFieldArr[0]->NormalizePercent( nTableWidth );
        for( i = 0; (i < MET_FIELDS) && (i < nNoOfVisibleCols); i++ )
        {
            pFieldArr[i]->SetPrcntValue( pFieldArr[i]->NormalizePercent(
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


void  SwTableColumnPage::Init(sal_Bool bWeb)
{
    FieldUnit aMetric = ::GetDfltMetric(bWeb);
    Link aLkUp = LINK( this, SwTableColumnPage, UpHdl );
    Link aLkDown = LINK( this, SwTableColumnPage, DownHdl );
    Link aLkLF = LINK( this, SwTableColumnPage, LoseFocusHdl );
    for( sal_uInt16 i = 0; i < MET_FIELDS; i++ )
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


IMPL_LINK( SwTableColumnPage, AutoClickHdl, CheckBox *, pBox )
{
    //Anzeigefenster verschieben
    if(pBox == (CheckBox *)&aDownBtn)
    {
        if(aValueTbl[0] > 0)
        {
            for( sal_uInt16 i=0; i < MET_FIELDS; i++ )
                aValueTbl[i] -= 1;
        }
    }
    if(pBox == (CheckBox *)&aUpBtn)
    {
        if( aValueTbl[ MET_FIELDS -1 ] < nNoOfVisibleCols -1  )
        {
            for(sal_uInt16 i=0;i < MET_FIELDS;i++)
                aValueTbl[i] += 1;
        }
    }
    for( sal_uInt16 i = 0; (i < nNoOfVisibleCols ) && ( i < MET_FIELDS); i++ )
    {
        String sEntry = rtl::OUString('~');
        String sIndex = String::CreateFromInt32( aValueTbl[i] + 1 );
        sEntry += sIndex;
        pTextArr[i]->SetText( sEntry );

        //added by menghu for SODC_5143,12/12/2006
        String sColumnWidth = SW_RESSTR( STR_ACCESS_COLUMN_WIDTH);
        sColumnWidth.SearchAndReplace( DEFINE_CONST_UNICODE("%1"), sIndex );
        pFieldArr[i]->SetAccessibleName( sColumnWidth );
    }

    aDownBtn.Enable(aValueTbl[0] > 0);
    aUpBtn.Enable(aValueTbl[ MET_FIELDS -1 ] < nNoOfVisibleCols -1 );
    UpdateCols(0);
    return 0;
};


IMPL_LINK_INLINE_START( SwTableColumnPage, UpHdl, PercentField *, pEdit )
{
    bModified = sal_True;
    ModifyHdl( pEdit );
    return 0;
};
IMPL_LINK_INLINE_END( SwTableColumnPage, UpHdl, PercentField *, pEdit )


IMPL_LINK_INLINE_START( SwTableColumnPage, DownHdl, PercentField *, pEdit )
{
    bModified = sal_True;
    ModifyHdl( pEdit );
    return 0;
};
IMPL_LINK_INLINE_END( SwTableColumnPage, DownHdl, PercentField *, pEdit )


IMPL_LINK_INLINE_START( SwTableColumnPage, LoseFocusHdl, PercentField *, pEdit )
{
    if(pEdit->IsModified())
    {
        bModified = sal_True;
        ModifyHdl( pEdit );
    }
    return 0;
};
IMPL_LINK_INLINE_END( SwTableColumnPage, LoseFocusHdl, PercentField *, pEdit )


IMPL_LINK( SwTableColumnPage, ModeHdl, CheckBox*, pBox )
{
    sal_Bool bCheck = pBox->IsChecked();
    if(pBox == &aProportionalCB)
    {
        if(bCheck)
            aModifyTableCB.Check();
        aModifyTableCB.Enable(!bCheck && bModifyTbl);
    }
    return 0;
};


sal_Bool  SwTableColumnPage::FillItemSet( SfxItemSet& )
{
    for( sal_uInt16 i = 0; i < MET_FIELDS; i++ )
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


void   SwTableColumnPage::ModifyHdl( PercentField* pEdit )
{
        sal_uInt16 nAktPos;
        sal_uInt16 i;

        for( i = 0; i < MET_FIELDS; i++)
            if(pEdit == pFieldArr[i])
                break;

        if (MET_FIELDS <= i)
        {
            OSL_ENSURE(false, "cannot happen.");
            return;
        }

        SetVisibleWidth(aValueTbl[i], static_cast< SwTwips >(pEdit->DenormalizePercent(pEdit->GetValue( FUNIT_TWIP ))) );
        nAktPos = aValueTbl[i];

        UpdateCols( nAktPos );
};


void   SwTableColumnPage::UpdateCols( sal_uInt16 nAktPos )
{
    SwTwips nSum = 0;
    sal_uInt16 i;

    for( i = 0; i < nNoOfCols; i++ )
    {
        nSum += (pTblData->GetColumns())[i].nWidth;
    }
    SwTwips nDiff = nSum - nTableWidth;

    sal_Bool bModifyTable = aModifyTableCB.IsChecked();
    sal_Bool bProp =    aProportionalCB.IsChecked();

    if(!bModifyTable && !bProp )
    {
//      the table width is constant, the difference is balanced with the other columns
        sal_uInt16 nLoopCount = 0;
        while( nDiff )
        {
            if( ++nAktPos == nNoOfVisibleCols)
            {
                nAktPos = 0;
                ++nLoopCount;
                //#i101353# in small tables it might not be possible to balance column width
                if( nLoopCount > 1 )
                    break;
            }
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
                OSL_ENSURE(nDiff >= 0, "nDiff < 0 cannot be here!");
            }
        }
    }
    else if(bModifyTable && !bProp)
    {
//      Differenz wird ueber die Tabellenbreite ausgeglichen,
//      andere Spalten bleiben unveraendert
        OSL_ENSURE(nDiff <= pTblData->GetSpace() - nTableWidth, "wrong maximum" );
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
    else if(bModifyTable & bProp)
    {
//      Alle Spalten werden proportional mitveraendert, die Tabellenbreite wird
//      entsprechend angepasst
        OSL_ENSURE(nDiff * nNoOfVisibleCols <= pTblData->GetSpace() - nTableWidth, "wrong maximum" );
        long nAdd = nDiff;
        if(nDiff * nNoOfVisibleCols > pTblData->GetSpace() - nTableWidth)
        {
            nAdd = (pTblData->GetSpace() - nTableWidth) / nNoOfVisibleCols;
            SetVisibleWidth(nAktPos, GetVisibleWidth(nAktPos) - nDiff + nAdd );
            nDiff = nAdd;
        }
        if(nAdd)
            for(i = 0; i < nNoOfVisibleCols; i++ )
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

    if(!bPercentMode)
        aSpaceED.SetValue(aSpaceED.Normalize( pTblData->GetSpace() - nTableWidth) , FUNIT_TWIP);

    for( i = 0; ( i < nNoOfVisibleCols ) && ( i < MET_FIELDS ); i++)
    {
        pFieldArr[i]->SetPrcntValue(pFieldArr[i]->NormalizePercent(
                        GetVisibleWidth(aValueTbl[i]) ), FUNIT_TWIP);
        pFieldArr[i]->ClearModifyFlag();
    }

}


void    SwTableColumnPage::ActivatePage( const SfxItemSet& )
{
    bPercentMode = pTblData->GetWidthPercent() != 0;
    for( sal_uInt16 i = 0; (i < MET_FIELDS) && (i < nNoOfVisibleCols); i++ )
    {
        pFieldArr[i]->SetRefValue(pTblData->GetWidth());
        pFieldArr[i]->ShowPercent( bPercentMode );
    }

    sal_uInt16 nTblAlign = pTblData->GetAlign();
    if((text::HoriOrientation::FULL != nTblAlign && nTableWidth != pTblData->GetWidth()) ||
    (text::HoriOrientation::FULL == nTblAlign && nTableWidth != pTblData->GetSpace()))
    {
        nTableWidth = text::HoriOrientation::FULL == nTblAlign ?
                                    pTblData->GetSpace() :
                                        pTblData->GetWidth();
        UpdateCols(0);
    }
    bModifyTbl = sal_True;
    if(pTblData->GetWidthPercent() ||
                text::HoriOrientation::FULL == nTblAlign ||
                        pTblData->IsLineSelected()  )
        bModifyTbl = sal_False;
    if(bPercentMode)
    {
        aModifyTableCB  .Check(sal_False);
        aProportionalCB .Check(sal_False);
    }
    else if( !bModifyTbl )
    {
        aProportionalCB.Check(sal_False);
        aModifyTableCB.Check(sal_False);
    }
    aSpaceFT.Enable(!bPercentMode);
    aSpaceED.Enable(!bPercentMode);
    aModifyTableCB.Enable( !bPercentMode && bModifyTbl );
    aProportionalCB.Enable(!bPercentMode && bModifyTbl );

    aSpaceED.SetValue(aSpaceED.Normalize(
                pTblData->GetSpace() - nTableWidth) , FUNIT_TWIP);

}


int  SwTableColumnPage::DeactivatePage( SfxItemSet* _pSet )
{
    if(_pSet)
    {
        FillItemSet(*_pSet);
        if(text::HoriOrientation::FULL != pTblData->GetAlign() && pTblData->GetWidth() != nTableWidth)
        {
            pTblData->SetWidth(nTableWidth);
            SwTwips nDiff = pTblData->GetSpace() - pTblData->GetWidth() -
                            pTblData->GetLeftSpace() - pTblData->GetRightSpace();
            switch( pTblData->GetAlign()  )
            {
                case text::HoriOrientation::RIGHT:
                    pTblData->SetLeftSpace(pTblData->GetLeftSpace() + nDiff);
                break;
                case text::HoriOrientation::LEFT:
                    pTblData->SetRightSpace(pTblData->GetRightSpace() + nDiff);
                break;
                case text::HoriOrientation::NONE:
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
                case text::HoriOrientation::CENTER:
                    pTblData->SetRightSpace(pTblData->GetRightSpace() + nDiff/2);
                    pTblData->SetLeftSpace(pTblData->GetLeftSpace() + nDiff/2);
                break;
                case text::HoriOrientation::LEFT_AND_WIDTH :
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
        _pSet->Put(SwPtrItem( FN_TABLE_REP, pTblData ));
    }
    return sal_True;
}


SwTwips  SwTableColumnPage::GetVisibleWidth(sal_uInt16 nPos)
{
    sal_uInt16 i=0;

    while( nPos )
    {
        if(pTblData->GetColumns()[i].bVisible && nPos)
            nPos--;
        i++;
    }
    SwTwips nReturn = pTblData->GetColumns()[i].nWidth;
    OSL_ENSURE(i < nNoOfCols, "Array index out of range");
    while(!pTblData->GetColumns()[i].bVisible && (i + 1) < nNoOfCols)
        nReturn += pTblData->GetColumns()[++i].nWidth;

    return nReturn;
}


void SwTableColumnPage::SetVisibleWidth(sal_uInt16 nPos, SwTwips nNewWidth)
{
    sal_uInt16 i=0;
    while( nPos )
    {
        if(pTblData->GetColumns()[i].bVisible && nPos)
            nPos--;
        i++;
    }
    OSL_ENSURE(i < nNoOfCols, "Array index out of range");
    pTblData->GetColumns()[i].nWidth = nNewWidth;
    while(!pTblData->GetColumns()[i].bVisible && (i + 1) < nNoOfCols)
        pTblData->GetColumns()[++i].nWidth = 0;

}


SwTableTabDlg::SwTableTabDlg(Window* pParent, SfxItemPool& ,
                    const SfxItemSet* pItemSet, SwWrtShell* pSh ) :
        SfxTabDialog(pParent, SW_RES(DLG_FORMAT_TABLE), pItemSet,0),
        pShell(pSh),
        nHtmlMode(::GetHtmlMode(pSh->GetView().GetDocShell()))
{
    FreeResource();
    SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
    OSL_ENSURE(pFact, "Dialogdiet fail!");
    AddTabPage(TP_FORMAT_TABLE, &SwFormatTablePage::Create, 0 );
    AddTabPage(TP_TABLE_TEXTFLOW, &SwTextFlowPage::Create, 0 );
    AddTabPage(TP_TABLE_COLUMN, &SwTableColumnPage::Create, 0 );
    AddTabPage(TP_BACKGROUND, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BACKGROUND ), 0 );
    AddTabPage(TP_BORDER, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BORDER ), 0 );
}


void  SwTableTabDlg::PageCreated(sal_uInt16 nId, SfxTabPage& rPage)
{
    SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
    if( TP_BACKGROUND == nId )
    {
        sal_Int32 nFlagType = SVX_SHOW_TBLCTL;
        if(!( nHtmlMode & HTMLMODE_ON ) ||
            nHtmlMode & HTMLMODE_SOME_STYLES)
            nFlagType |= SVX_SHOW_SELECTOR;
        aSet.Put (SfxUInt32Item(SID_FLAG_TYPE, nFlagType));
        rPage.PageCreated(aSet);
    }
    else if(TP_BORDER == nId)
    {
        aSet.Put (SfxUInt16Item(SID_SWMODE_TYPE,SW_BORDER_MODE_TABLE));
        rPage.PageCreated(aSet);
    }
    else if(TP_TABLE_TEXTFLOW == nId)
    {
        ((SwTextFlowPage&)rPage).SetShell(pShell);
        const sal_uInt16 eType = pShell->GetFrmType(0,sal_True);
        if( !(FRMTYPE_BODY & eType) )
            ((SwTextFlowPage&)rPage).DisablePageBreak();
    }
}

SwTextFlowPage::SwTextFlowPage( Window* pParent,
                                const SfxItemSet& rSet ) :
    SfxTabPage(pParent, SW_RES( TP_TABLE_TEXTFLOW ), rSet ),
    aFlowFL         (this, SW_RES(FL_FLOW            )),
    aPgBrkCB        (this, SW_RES(CB_PAGEBREAK      )),
    aPgBrkRB        (this, SW_RES(RB_BREAKPAGE      )),
    aColBrkRB       (this, SW_RES(RB_BREAKCOLUMN    )),
    aPgBrkBeforeRB  (this, SW_RES(RB_PAGEBREAKBEFORE)),
    aPgBrkAfterRB   (this, SW_RES(RB_PAGEBREAKAFTER )),
    aPageCollCB     (this, SW_RES(CB_PAGECOLL       )),
    aPageCollLB     (this, SW_RES(LB_PAGECOLL       )),
    aPageNoFT       (this, SW_RES(FT_PAGENUM        )),
    aPageNoNF       (this, SW_RES(NF_PAGENUM        )),
    aSplitCB        (this, SW_RES(CB_SPLIT          )),
    aSplitRowCB     (this, SW_RES(CB_SPLIT_ROW      )),
    aKeepCB         (this, SW_RES(CB_KEEP           )),
    aHeadLineCB     (this, SW_RES(CB_HEADLINE       )),
    aRepeatHeaderFT         (this, SW_RES(FT_REPEAT_HEADER  )),
    aRepeatHeaderBeforeFT   (this),
    aRepeatHeaderNF         (this, SW_RES(NF_REPEAT_HEADER  )),
    aRepeatHeaderAfterFT    (this),
    aRepeatHeaderCombo      (this, SW_RES(WIN_REPEAT_HEADER), aRepeatHeaderNF, aRepeatHeaderBeforeFT, aRepeatHeaderAfterFT),
    aTextDirectionFT(this, SW_RES(FT_TEXTORIENTATION  )),
    aTextDirectionLB(this, SW_RES(LB_TEXTORIENTATION  )),

    aVertOrientFL   (this, SW_RES(FL_VERT_ORIENT    )),
    aVertOrientFT(this,  SW_RES(FT_VERTORIENT       )),
    aVertOrientLB(this,  SW_RES(LB_VERTORIENT       )),

    pShell(0),

    bPageBreak(sal_True),
    bHtmlMode(sal_False)
{
    FreeResource();

    aPgBrkRB.SetAccessibleRelationMemberOf(&aPgBrkCB);
    aColBrkRB.SetAccessibleRelationMemberOf(&aPgBrkCB);
    aPgBrkBeforeRB.SetAccessibleRelationMemberOf(&aPgBrkCB);
    aPgBrkAfterRB.SetAccessibleRelationMemberOf(&aPgBrkCB);
    aPageCollLB.SetAccessibleRelationLabeledBy(&aPageCollCB);
    aPageCollLB.SetAccessibleName(aPageCollCB.GetText());

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
    aSplitCB.SetClickHdl(
        LINK( this, SwTextFlowPage, SplitHdl_Impl));
    aSplitRowCB.SetClickHdl(
        LINK( this, SwTextFlowPage, SplitRowHdl_Impl));
    aHeadLineCB.SetClickHdl( LINK( this, SwTextFlowPage, HeadLineCBClickHdl ) );

#ifndef SW_FILEFORMAT_40
    const SfxPoolItem *pItem;
    if(SFX_ITEM_SET == rSet.GetItemState( SID_HTML_MODE, sal_False,&pItem )
        && ((const SfxUInt16Item*)pItem)->GetValue() & HTMLMODE_ON)
#endif
    {
        aKeepCB.Hide();
        aSplitCB.Hide();
        aSplitRowCB.Hide();
    }

    aRepeatHeaderCombo.Arrange( aRepeatHeaderFT );

    HeadLineCBClickHdl();
}

 SwTextFlowPage::~SwTextFlowPage()
{
}

SfxTabPage*   SwTextFlowPage::Create( Window* pParent,
                                const SfxItemSet& rAttrSet)
{
    return new SwTextFlowPage(pParent, rAttrSet);
}

sal_Bool  SwTextFlowPage::FillItemSet( SfxItemSet& rSet )
{
    sal_Bool bModified = sal_False;

    // Repeat Heading
    if(aHeadLineCB.IsChecked() != aHeadLineCB.GetSavedValue() ||
        String::CreateFromInt32( static_cast< sal_Int32 >(aRepeatHeaderNF.GetValue()) ) != aRepeatHeaderNF.GetSavedValue() )
    {
        bModified |= 0 != rSet.Put(
            SfxUInt16Item(FN_PARAM_TABLE_HEADLINE, aHeadLineCB.IsChecked()? sal_uInt16(aRepeatHeaderNF.GetValue()) : 0 ));
    }
    if(aKeepCB.IsChecked() != aKeepCB.GetSavedValue())
        bModified |= 0 != rSet.Put( SvxFmtKeepItem( aKeepCB.IsChecked(), RES_KEEP));

    if(aSplitCB.IsChecked() != aSplitCB.GetSavedValue())
        bModified |= 0 != rSet.Put( SwFmtLayoutSplit( aSplitCB.IsChecked()));

    if(aSplitRowCB.IsChecked() != aSplitRowCB.GetSavedValue())
        bModified |= 0 != rSet.Put( SwFmtRowSplit( aSplitRowCB.IsChecked()));


    const SvxFmtBreakItem* pBreak = (const SvxFmtBreakItem*)GetOldItem( rSet, RES_BREAK );
    const SwFmtPageDesc* pDesc = (const SwFmtPageDesc*) GetOldItem( rSet, RES_PAGEDESC );


    sal_Bool bState = aPageCollCB.IsChecked();

    // If we have a page style, then there's no break
    sal_Bool bPageItemPut = sal_False;
    if ( bState != aPageCollCB.GetSavedValue() ||
         ( bState &&
           aPageCollLB.GetSelectEntryPos() != aPageCollLB.GetSavedValue() )
           || (aPageNoNF.IsEnabled() && aPageNoNF.IsValueModified()) )
    {
        String sPage;

        if ( bState )
        {
            sPage = aPageCollLB.GetSelectEntry();
        }
        sal_uInt16 nPgNum = static_cast< sal_uInt16 >(aPageNoNF.GetValue());
        if ( !pDesc || !pDesc->GetPageDesc() ||
            ( pDesc->GetPageDesc() && ((pDesc->GetPageDesc()->GetName() != sPage) ||
                    !comphelper::string::equals(aPageNoNF.GetSavedValue(), nPgNum))))
        {
            SwFmtPageDesc aFmt( pShell->FindPageDescByName( sPage, sal_True ) );
            aFmt.SetNumOffset(bState ? nPgNum : 0);
            bModified |= 0 != rSet.Put( aFmt );
            bPageItemPut = bState;
        }
    }
    sal_Bool bIsChecked = aPgBrkCB.IsChecked();
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
            sal_Bool bBefore = aPgBrkBeforeRB.IsChecked();

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

    if(aTextDirectionLB.GetSelectEntryPos() != aTextDirectionLB.GetSavedValue())
    {
          bModified |= 0 != rSet.Put(
                    SvxFrameDirectionItem(
                        (SvxFrameDirection)(sal_uLong)aTextDirectionLB.GetEntryData(aTextDirectionLB.GetSelectEntryPos())
                        , FN_TABLE_BOX_TEXTORIENTATION));
    }

    if(aVertOrientLB.GetSelectEntryPos() != aVertOrientLB.GetSavedValue())
    {
        sal_uInt16 nOrient = USHRT_MAX;
        switch(aVertOrientLB.GetSelectEntryPos())
        {
            case 0 : nOrient = text::VertOrientation::NONE; break;
            case 1 : nOrient = text::VertOrientation::CENTER; break;
            case 2 : nOrient = text::VertOrientation::BOTTOM; break;
        }
        if(nOrient != USHRT_MAX)
            bModified |= 0 != rSet.Put(SfxUInt16Item(FN_TABLE_SET_VERT_ALIGN, nOrient));
    }

    return bModified;

}

void   SwTextFlowPage::Reset( const SfxItemSet& rSet )
{
    const SfxPoolItem* pItem;
    SvxHtmlOptions& rHtmlOpt = SvxHtmlOptions::Get();
    sal_Bool bFlowAllowed = !bHtmlMode || rHtmlOpt.IsPrintLayoutExtension();
    if(bFlowAllowed)
    {
        // Einfuegen der vorhandenen Seitenvorlagen in die Listbox
        const sal_uInt16 nCount = pShell->GetPageDescCnt();
        sal_uInt16 i;

        for( i = 0; i < nCount; ++i)
        {
            const SwPageDesc &rPageDesc = pShell->GetPageDesc(i);
            aPageCollLB.InsertEntry(rPageDesc.GetName());
        }

        String aFmtName;
        for(i = RES_POOLPAGE_BEGIN; i < RES_POOLPAGE_END; ++i)
            if( LISTBOX_ENTRY_NOTFOUND == aPageCollLB.GetEntryPos(
                    aFmtName = SwStyleNameMapper::GetUIName( i, aFmtName ) ))
                aPageCollLB.InsertEntry( aFmtName );

        if(SFX_ITEM_SET == rSet.GetItemState( RES_KEEP, sal_False, &pItem ))
        {
            aKeepCB.Check( ((const SvxFmtKeepItem*)pItem)->GetValue() );
            aKeepCB.SaveValue();
        }
        if(SFX_ITEM_SET == rSet.GetItemState( RES_LAYOUT_SPLIT, sal_False, &pItem ))
        {
            aSplitCB.Check( ((const SwFmtLayoutSplit*)pItem)->GetValue() );
        }
        else
            aSplitCB.Check();

        aSplitCB.SaveValue();
        SplitHdl_Impl(&aSplitCB);

        if(SFX_ITEM_SET == rSet.GetItemState( RES_ROW_SPLIT, sal_False, &pItem ))
        {
            aSplitRowCB.Check( ((const SwFmtRowSplit*)pItem)->GetValue() );
        }
        else
            aSplitRowCB.SetState(STATE_DONTKNOW);
        aSplitRowCB.SaveValue();

        if(bPageBreak)
        {
            if(SFX_ITEM_SET == rSet.GetItemState( RES_PAGEDESC, sal_False, &pItem ))
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

                    aPgBrkCB.Check( sal_True );
                    aColBrkRB.Check( sal_False );
                    aPgBrkBeforeRB.Check( sal_True );
                    aPgBrkAfterRB.Check( sal_False );
                }
                else
                {
                    aPageCollLB.SetNoSelection();
                    aPageCollCB.Check(sal_False);
                }
            }

            if(SFX_ITEM_SET == rSet.GetItemState( RES_BREAK, sal_False, &pItem ))
            {
                const SvxFmtBreakItem* pPageBreak = (const SvxFmtBreakItem*)pItem;
                SvxBreak eBreak = (SvxBreak)pPageBreak->GetValue();

                if ( eBreak != SVX_BREAK_NONE )
                {
                    aPgBrkCB.Check();
                    aPageCollCB.Enable(sal_False);
                    aPageCollLB.Enable(sal_False);
                    aPageNoFT.Enable(sal_False);
                    aPageNoNF.Enable(sal_False);
                }
                switch ( eBreak )
                {
                    case SVX_BREAK_PAGE_BEFORE:
                        aPgBrkRB.Check( sal_True );
                        aColBrkRB.Check( sal_False );
                        aPgBrkBeforeRB.Check( sal_True );
                        aPgBrkAfterRB.Check( sal_False );
                        break;
                    case SVX_BREAK_PAGE_AFTER:
                        aPgBrkRB.Check( sal_True );
                        aColBrkRB.Check( sal_False );
                        aPgBrkBeforeRB.Check( sal_False );
                        aPgBrkAfterRB.Check( sal_True );
                        break;
                    case SVX_BREAK_COLUMN_BEFORE:
                        aPgBrkRB.Check( sal_False );
                        aColBrkRB.Check( sal_True );
                        aPgBrkBeforeRB.Check( sal_True );
                        aPgBrkAfterRB.Check( sal_False );
                        break;
                    case SVX_BREAK_COLUMN_AFTER:
                        aPgBrkRB.Check( sal_False );
                        aColBrkRB.Check( sal_True );
                        aPgBrkBeforeRB.Check( sal_False );
                        aPgBrkAfterRB.Check( sal_True );
                        break;
                    default:; //prevent warning
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
        aPgBrkRB.Enable(sal_False);
        aColBrkRB.Enable(sal_False);
        aPgBrkBeforeRB.Enable(sal_False);
        aPgBrkAfterRB.Enable(sal_False);
        aKeepCB .Enable(sal_False);
        aSplitCB.Enable(sal_False);
        aPgBrkCB.Enable(sal_False);
        aPageCollCB.Enable(sal_False);
        aPageCollLB.Enable(sal_False);
    }

    if(SFX_ITEM_SET == rSet.GetItemState( FN_PARAM_TABLE_HEADLINE, sal_False, &pItem ))
    {
        sal_uInt16 nRep = ((const SfxUInt16Item*)pItem)->GetValue();
        aHeadLineCB.Check( nRep > 0 );
        aHeadLineCB.SaveValue();
        aRepeatHeaderNF.SetValue( nRep );
        aRepeatHeaderNF.SaveValue();
    }
    if ( rSet.GetItemState(FN_TABLE_BOX_TEXTORIENTATION) > SFX_ITEM_AVAILABLE )
    {
        sal_uLong nDirection = ((const SvxFrameDirectionItem&)rSet.Get(FN_TABLE_BOX_TEXTORIENTATION)).GetValue();
        aTextDirectionLB.SelectEntryPos(aTextDirectionLB.GetEntryPos( (const void*)nDirection ));
    }

    if ( rSet.GetItemState(FN_TABLE_SET_VERT_ALIGN) > SFX_ITEM_AVAILABLE )
    {
        sal_uInt16 nVert = ((const SfxUInt16Item&)rSet.Get(FN_TABLE_SET_VERT_ALIGN)).GetValue();
        sal_uInt16 nPos = 0;
        switch(nVert)
        {
            case text::VertOrientation::NONE:     nPos = 0;   break;
            case text::VertOrientation::CENTER:   nPos = 1;   break;
            case text::VertOrientation::BOTTOM:   nPos = 2;   break;
        }
        aVertOrientLB.SelectEntryPos(nPos);
    }

    aPageCollCB.SaveValue();
    aPageCollLB.SaveValue();
    aPgBrkCB.SaveValue();
    aPgBrkRB.SaveValue();
    aColBrkRB.SaveValue();
    aPgBrkBeforeRB.SaveValue();
    aPgBrkAfterRB.SaveValue();
    aPageNoNF.SaveValue();
    aTextDirectionLB.SaveValue();
    aVertOrientLB.SaveValue();

    HeadLineCBClickHdl();
}

void SwTextFlowPage::SetShell(SwWrtShell* pSh)
{
    pShell = pSh;
    bHtmlMode = 0 != (::GetHtmlMode(pShell->GetView().GetDocShell()) & HTMLMODE_ON);
    if(bHtmlMode)
    {
        aPageNoNF.Enable(sal_False);
        aPageNoFT.Enable(sal_False);
    }
}

IMPL_LINK_NOARG(SwTextFlowPage, PageBreakHdl_Impl)
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

                sal_Bool bEnable = aPageCollCB.IsChecked() &&
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
            aPageCollCB.Check( sal_False );
            aPageCollCB.Enable(sal_False);
            aPageCollLB.Enable(sal_False);
            aPageNoFT.Enable(sal_False);
            aPageNoNF.Enable(sal_False);
            aPgBrkRB.       Enable(sal_False);
            aColBrkRB.      Enable(sal_False);
            aPgBrkBeforeRB. Enable(sal_False);
            aPgBrkAfterRB.  Enable(sal_False);
    }
    return 0;
}

IMPL_LINK_NOARG(SwTextFlowPage, ApplyCollClickHdl_Impl)
{
    sal_Bool bEnable = sal_False;
    if ( aPageCollCB.IsChecked() &&
         aPageCollLB.GetEntryCount() )
    {
        bEnable = sal_True;
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

IMPL_LINK( SwTextFlowPage, PageBreakPosHdl_Impl, RadioButton*, pBtn )
{
    if ( aPgBrkCB.IsChecked() )
    {
        if ( pBtn == &aPgBrkBeforeRB && aPgBrkRB.IsChecked() )
        {
            aPageCollCB.Enable();

            sal_Bool bEnable = aPageCollCB.IsChecked()  &&
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
            aPageCollCB .Check( sal_False );
            aPageCollCB .Enable(sal_False);
            aPageCollLB .Enable(sal_False);
            aPageNoFT   .Enable(sal_False);
            aPageNoNF   .Enable(sal_False);
        }
    }
    return 0;
}

IMPL_LINK( SwTextFlowPage, PageBreakTypeHdl_Impl, RadioButton*, pBtn )
{
    if ( pBtn == &aColBrkRB || aPgBrkAfterRB.IsChecked() )
    {
        aPageCollCB .Check(sal_False);
        aPageCollCB .Enable(sal_False);
        aPageCollLB .Enable(sal_False);
        aPageNoFT   .Enable(sal_False);
        aPageNoNF   .Enable(sal_False);
    }
    else if ( aPgBrkBeforeRB.IsChecked() )
        PageBreakPosHdl_Impl( &aPgBrkBeforeRB );
    return 0;
}

IMPL_LINK( SwTextFlowPage, SplitHdl_Impl, CheckBox*, pBox )
{
    aSplitRowCB.Enable(pBox->IsChecked());
    return 0;
}

IMPL_LINK( SwTextFlowPage, SplitRowHdl_Impl, TriStateBox*, pBox )
{
    pBox->EnableTriState(sal_False);
    return 0;
}

IMPL_LINK_NOARG(SwTextFlowPage, HeadLineCBClickHdl)
{
    aRepeatHeaderCombo.Enable(aHeadLineCB.IsChecked());

    return 0;
}

void SwTextFlowPage::DisablePageBreak()
{
    bPageBreak = sal_False;
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



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
