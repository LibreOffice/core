/*************************************************************************
 *
 *  $RCSfile: uitool.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: rt $ $Date: 2003-09-19 08:49:52 $
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


#pragma hdrstop

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif
#ifndef _UIPARAM_HXX
#include <uiparam.hxx>
#endif


#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _UNOTOOLS_COLLATORWRAPPER_HXX
#include <unotools/collatorwrapper.hxx>
#endif

#ifndef SVTOOLS_URIHELPER_HXX
#include <svtools/urihelper.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif
#ifndef _SFXDOCFILE_HXX //autogen
#include <sfx2/docfile.hxx>
#endif
#ifndef _SFX_DOCFILT_HACK_HXX //autogen
#include <sfx2/docfilt.hxx>
#endif
#ifndef _SVX_PMDLITEM_HXX //autogen
#include <svx/pmdlitem.hxx>
#endif
#ifndef _SVX_TSTPITEM_HXX //autogen
#include <svx/tstpitem.hxx>
#endif
#ifndef _SVX_BOXITEM_HXX //autogen
#include <svx/boxitem.hxx>
#endif
#ifndef _SVX_SIZEITEM_HXX //autogen
#include <svx/sizeitem.hxx>
#endif
#ifndef _SVX_PAGEITEM_HXX //autogen
#include <svx/pageitem.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif
#ifndef _SFXSTYLE_HXX //autogen
#include <svtools/style.hxx>
#endif
#ifndef _VCL_LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif
#ifndef _UNOTOOLS_LOCALEDATAWRAPPER_HXX
#include <unotools/localedatawrapper.hxx>
#endif

#ifndef _FMTORNT_HXX //autogen
#include <fmtornt.hxx>
#endif
#ifndef _TABCOL_HXX //autogen
#include <tabcol.hxx>
#endif

#ifndef _FMTFSIZE_HXX //autogen
#include <fmtfsize.hxx>
#endif
#ifndef _FMTHDFT_HXX //autogen
#include <fmthdft.hxx>
#endif
#ifndef _FMTPDSC_HXX //autogen
#include <fmtpdsc.hxx>
#endif
#ifndef _SWWVIEW_HXX //autogen
#include <wview.hxx>
#endif
#ifndef _UIITEMS_HXX
#include <uiitems.hxx>
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#ifndef _UITOOL_HXX
#include <uitool.hxx>
#endif
#ifndef _PAGEDESC_HXX
#include <pagedesc.hxx>
#endif
#ifndef _FRMATR_HXX
#include <frmatr.hxx>
#endif
#ifndef _PARATR_HXX
#include <paratr.hxx>
#endif
#ifndef _FMTCOL_HXX
#include <fmtcol.hxx>
#endif
#ifndef _POOLFMT_HXX
#include <poolfmt.hxx>
#endif
#ifndef _USRPREF_HXX
#include "usrpref.hxx"
#endif

#ifndef _ERROR_H
#include <error.h>
#endif
#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _GLOBALS_HRC
#include <globals.hrc>
#endif
#ifndef _UTLUI_HRC
#include <utlui.hrc>
#endif
#ifndef _DOC_HXX //autogen wg. SwDoc
#include <doc.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _CHARFMT_HXX //autogen wg. SwCharFmt
#include <charfmt.hxx>
#endif
#ifndef _SWSTYLENAMEMAPPER_HXX
#include <SwStyleNameMapper.hxx>
#endif
// 50 cm 28350
//
#define MAXHEIGHT 28350
#define MAXWIDTH  28350

/*--------------------------------------------------------------------
    Beschreibung: Allgemeine List von StringPointern
 --------------------------------------------------------------------*/


/*--------------------------------------------------------------------
    Beschreibung: Metric umschalten
 --------------------------------------------------------------------*/


void SetMetric(MetricFormatter& rCtrl, FieldUnit eUnit)
{
    SwTwips nMin = rCtrl.GetMin(FUNIT_TWIP);
    SwTwips nMax = rCtrl.GetMax(FUNIT_TWIP);

    rCtrl.SetUnit(eUnit);

    rCtrl.SetMin(nMin, FUNIT_TWIP);
    rCtrl.SetMax(nMax, FUNIT_TWIP);
}

/*--------------------------------------------------------------------
    Beschreibung:   Boxinfo-Attribut setzen
 --------------------------------------------------------------------*/


void PrepareBoxInfo(SfxItemSet& rSet, const SwWrtShell& rSh)
{
    SvxBoxInfoItem aBoxInfo;
    const SfxPoolItem *pBoxInfo;
    if ( SFX_ITEM_SET == rSet.GetItemState( SID_ATTR_BORDER_INNER,
                                        TRUE, &pBoxInfo))
        aBoxInfo = *(SvxBoxInfoItem*)pBoxInfo;

        // Tabellenvariante, wenn mehrere Tabellenzellen selektiert
    rSh.GetCrsr();                  //Damit GetCrsrCnt() auch das Richtige liefert
    aBoxInfo.SetTable          (rSh.IsTableMode() && rSh.GetCrsrCnt() > 1);
        // Abstandsfeld immer anzeigen
    aBoxInfo.SetDist           ((BOOL) TRUE);
        // Minimalgroesse in Tabellen und Absaetzen setzen
    aBoxInfo.SetMinDist        (rSh.IsTableMode() || rSh.GetSelectionType() & (SwWrtShell::SEL_TXT | SwWrtShell::SEL_TBL));
        // Default-Abstand immer setzen
    aBoxInfo.SetDefDist        (MIN_BORDER_DIST);
        // Einzelne Linien koennen nur in Tabellen DontCare-Status haben
    aBoxInfo.SetValid(VALID_DISABLE, !rSh.IsTableMode());

    rSet.Put(aBoxInfo);
}

/*--------------------------------------------------------------------
    Beschreibung:   Header Footer fuellen
 --------------------------------------------------------------------*/


void FillHdFt(SwFrmFmt* pFmt, const  SfxItemSet& rSet)
{
    SwAttrSet aSet(pFmt->GetAttrSet());
    aSet.Put(rSet);

    const SvxSizeItem& rSize = (const SvxSizeItem&)rSet.Get(SID_ATTR_PAGE_SIZE);
    const SfxBoolItem& rDynamic = (const SfxBoolItem&)rSet.Get(SID_ATTR_PAGE_DYNAMIC);

    // Groesse umsetzen
    //
    SwFmtFrmSize aFrmSize(rDynamic.GetValue() ? ATT_MIN_SIZE : ATT_FIX_SIZE,
                            rSize.GetSize().Width(),
                            rSize.GetSize().Height());
    aSet.Put(aFrmSize);
    pFmt->SetAttr(aSet);
}

/*--------------------------------------------------------------------
    Beschreibung:   PageDesc <-> in Sets wandeln und zurueck
 --------------------------------------------------------------------*/


void ItemSetToPageDesc( const SfxItemSet& rSet, SwPageDesc& rPageDesc )
{
    SwFrmFmt& rMaster = rPageDesc.GetMaster();

    // alle allgemeinen Rahmen-Attribute uebertragen
    //
    rMaster.SetAttr(rSet);

    // PageData
    //
    if(rSet.GetItemState(SID_ATTR_PAGE) == SFX_ITEM_SET)
    {
        const SvxPageItem& rPageItem = (const SvxPageItem&)rSet.Get(SID_ATTR_PAGE);

        USHORT nUse = (USHORT)rPageItem.GetPageUsage();
        if(nUse & 0x04)
            nUse |= 0x03;
        if(nUse)
            rPageDesc.SetUseOn( (UseOnPage) nUse );
        rPageDesc.SetLandscape(rPageItem.IsLandscape());
        SvxNumberType aNumType;
        aNumType.SetNumberingType(rPageItem.GetNumType());
        rPageDesc.SetNumType(aNumType);
    }
    // Groesse
    //
    if(rSet.GetItemState(SID_ATTR_PAGE_SIZE) == SFX_ITEM_SET)
    {
        const SvxSizeItem& rSizeItem = (const SvxSizeItem&)rSet.Get(SID_ATTR_PAGE_SIZE);
        SwFmtFrmSize aSize(ATT_FIX_SIZE);
        aSize.SetSize(rSizeItem.GetSize());
        rMaster.SetAttr(aSize);
    }
    // Kopzeilen-Attribute auswerten
    //
    const SfxPoolItem* pItem;
    if( SFX_ITEM_SET == rSet.GetItemState( SID_ATTR_PAGE_HEADERSET,
            FALSE, &pItem ) )
    {
        const SfxItemSet& rHeaderSet = ((SvxSetItem*)pItem)->GetItemSet();
        const SfxBoolItem& rHeaderOn = (const SfxBoolItem&)rHeaderSet.Get(SID_ATTR_PAGE_ON);

        if(rHeaderOn.GetValue())
        {
            // Werte uebernehmen
            if(!rMaster.GetHeader().IsActive())
                rMaster.SetAttr(SwFmtHeader(TRUE));

            // Das Headerformat rausholen und anpassen
            //
            SwFmtHeader aHeaderFmt(rMaster.GetHeader());
            SwFrmFmt *pHeaderFmt = aHeaderFmt.GetHeaderFmt();
            ASSERT(pHeaderFmt != 0, "kein HeaderFormat");

            ::FillHdFt(pHeaderFmt, rHeaderSet);

            rPageDesc.ChgHeaderShare(((const SfxBoolItem&)
                        rHeaderSet.Get(SID_ATTR_PAGE_SHARED)).GetValue());
        }
        else
        {   // Header ausschalten
            //
            if(rMaster.GetHeader().IsActive())
            {
                rMaster.SetAttr(SwFmtHeader(BOOL(FALSE)));
                rPageDesc.ChgHeaderShare(FALSE);
            }
        }
    }

    // Fusszeilen-Attribute auswerten
    //
    if( SFX_ITEM_SET == rSet.GetItemState( SID_ATTR_PAGE_FOOTERSET,
            FALSE, &pItem ) )
    {
        const SfxItemSet& rFooterSet = ((SvxSetItem*)pItem)->GetItemSet();
        const SfxBoolItem& rFooterOn = (const SfxBoolItem&)rFooterSet.Get(SID_ATTR_PAGE_ON);

        if(rFooterOn.GetValue())
        {
            // Werte uebernehmen
            if(!rMaster.GetFooter().IsActive())
                rMaster.SetAttr(SwFmtFooter(TRUE));

            // Das Footerformat rausholen und anpassen
            //
            SwFmtFooter aFooterFmt(rMaster.GetFooter());
            SwFrmFmt *pFooterFmt = aFooterFmt.GetFooterFmt();
            ASSERT(pFooterFmt != 0, "kein FooterFormat");

            ::FillHdFt(pFooterFmt, rFooterSet);

            rPageDesc.ChgFooterShare(((const SfxBoolItem&)
                        rFooterSet.Get(SID_ATTR_PAGE_SHARED)).GetValue());
        }
        else
        {   // Footer ausschalten
            //
            if(rMaster.GetFooter().IsActive())
            {
                rMaster.SetAttr(SwFmtFooter(BOOL(FALSE)));
                rPageDesc.ChgFooterShare(FALSE);
            }
        }
    }

    // Fussnoten
    //
    if( SFX_ITEM_SET == rSet.GetItemState( FN_PARAM_FTN_INFO,
            FALSE, &pItem ) )
        rPageDesc.SetFtnInfo( ((SwPageFtnInfoItem*)pItem)->GetPageFtnInfo() );


    //
    // Columns
    //

    // Registerhaltigkeit

    if(SFX_ITEM_SET == rSet.GetItemState(
                            SID_SWREGISTER_MODE, FALSE, &pItem))
    {
        BOOL bSet = ((const SfxBoolItem*)pItem)->GetValue();
        if(!bSet)
            rPageDesc.SetRegisterFmtColl(0);
        else if(SFX_ITEM_SET == rSet.GetItemState(
                                SID_SWREGISTER_COLLECTION, FALSE, &pItem))
        {
            const String& rColl = ((const SfxStringItem*)pItem)->GetValue();
            SwDoc& rDoc = *rMaster.GetDoc();
            SwTxtFmtColl* pColl = rDoc.FindTxtFmtCollByName( rColl );
            if( !pColl )
            {
                USHORT nId = SwStyleNameMapper::GetPoolIdFromUIName( rColl, GET_POOLID_TXTCOLL );
                if( USHRT_MAX != nId )
                    pColl = rDoc.GetTxtCollFromPool( nId );
                else
                    pColl = rDoc.MakeTxtFmtColl( rColl,
                                (SwTxtFmtColl*)rDoc.GetDfltTxtFmtColl() );
            }
            if( pColl )
                pColl->SetAttr( SwRegisterItem ( TRUE ));
            rPageDesc.SetRegisterFmtColl( pColl );
        }
    }
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


void PageDescToItemSet( const SwPageDesc& rPageDesc, SfxItemSet& rSet)
{
    const SwFrmFmt& rMaster = rPageDesc.GetMaster();

    // Seitendaten
    //
    SvxPageItem aPageItem(SID_ATTR_PAGE);
    aPageItem.SetDescName(rPageDesc.GetName());
    aPageItem.SetPageUsage((SvxPageUsage)rPageDesc.GetUseOn());
    aPageItem.SetLandscape(rPageDesc.GetLandscape());
    aPageItem.SetNumType((SvxNumType)rPageDesc.GetNumType().GetNumberingType());
    rSet.Put(aPageItem);

    // Groesse
    SvxSizeItem aSizeItem(SID_ATTR_PAGE_SIZE, rMaster.GetFrmSize().GetSize());
    rSet.Put(aSizeItem);

    // Maximale Groesse
    SvxSizeItem aMaxSizeItem(SID_ATTR_PAGE_MAXSIZE, Size(MAXWIDTH, MAXHEIGHT));
    rSet.Put(aMaxSizeItem);

    // Raender, Umrandung und das andere Zeug
    //
    rSet.Put(rMaster.GetAttrSet());

    SvxBoxInfoItem aBoxInfo;
    const SfxPoolItem *pBoxInfo;
    if ( SFX_ITEM_SET == rSet.GetItemState( SID_ATTR_BORDER_INNER,
                                            TRUE, &pBoxInfo) )
        aBoxInfo = *(SvxBoxInfoItem*)pBoxInfo;

    aBoxInfo.SetTable( FALSE );
        // Abstandsfeld immer anzeigen
    aBoxInfo.SetDist( TRUE);
        // Minimalgroesse in Tabellen und Absaetzen setzen
    aBoxInfo.SetMinDist( FALSE );
    // Default-Abstand immer setzen
    aBoxInfo.SetDefDist( MIN_BORDER_DIST );
        // Einzelne Linien koennen nur in Tabellen DontCare-Status haben
    aBoxInfo.SetValid( VALID_DISABLE );
    rSet.Put( aBoxInfo );


    SfxStringItem aFollow(SID_ATTR_PAGE_EXT1, aEmptyStr);
    if(rPageDesc.GetFollow())
        aFollow.SetValue(rPageDesc.GetFollow()->GetName());
    rSet.Put(aFollow);

    // Header
    //
    if(rMaster.GetHeader().IsActive())
    {
        const SwFmtHeader &rHeaderFmt = rMaster.GetHeader();
        const SwFrmFmt *pHeaderFmt = rHeaderFmt.GetHeaderFmt();
        ASSERT(pHeaderFmt != 0, kein HeaderFormat.);

        // HeaderInfo, Raender, Hintergrund, Umrandung
        //
        SfxItemSet aHeaderSet( *rSet.GetPool(),
                    SID_ATTR_PAGE_ON,       SID_ATTR_PAGE_SHARED,
                    SID_ATTR_PAGE_SIZE,     SID_ATTR_PAGE_SIZE,
                    SID_ATTR_BORDER_INNER,  SID_ATTR_BORDER_INNER,
                    RES_FRMATR_BEGIN,       RES_FRMATR_END-1,
                    0);

        // dynamische oder feste Hoehe
        //
        SfxBoolItem aOn(SID_ATTR_PAGE_ON, TRUE);
        aHeaderSet.Put(aOn);

        const SwFmtFrmSize &rFrmSize = pHeaderFmt->GetFrmSize();
        const SwFrmSize eSizeType = rFrmSize.GetSizeType();
        SfxBoolItem aDynamic(SID_ATTR_PAGE_DYNAMIC, eSizeType != ATT_FIX_SIZE);
        aHeaderSet.Put(aDynamic);

        // Links gleich rechts
        //
        SfxBoolItem aShared(SID_ATTR_PAGE_SHARED, rPageDesc.IsHeaderShared());
        aHeaderSet.Put(aShared);

        // Groesse
        SvxSizeItem aSize(SID_ATTR_PAGE_SIZE, Size(rFrmSize.GetSize()));
        aHeaderSet.Put(aSize);

        // Rahmen-Attribute umschaufeln
        //
        aHeaderSet.Put(pHeaderFmt->GetAttrSet());
        aHeaderSet.Put( aBoxInfo );

        // SetItem erzeugen
        //
        SvxSetItem aSetItem(SID_ATTR_PAGE_HEADERSET, aHeaderSet);
        rSet.Put(aSetItem);
    }

    // Footer
    if(rMaster.GetFooter().IsActive())
    {
        const SwFmtFooter &rFooterFmt = rMaster.GetFooter();
        const SwFrmFmt *pFooterFmt = rFooterFmt.GetFooterFmt();
        ASSERT(pFooterFmt != 0, kein FooterFormat.);

        // FooterInfo, Raender, Hintergrund, Umrandung
        //
        SfxItemSet aFooterSet( *rSet.GetPool(),
                    SID_ATTR_PAGE_ON,       SID_ATTR_PAGE_SHARED,
                    SID_ATTR_PAGE_SIZE,     SID_ATTR_PAGE_SIZE,
                    SID_ATTR_BORDER_INNER,  SID_ATTR_BORDER_INNER,
                    RES_FRMATR_BEGIN,       RES_FRMATR_END-1,
                    0);

        // dynamische oder feste Hoehe
        //
        SfxBoolItem aOn(SID_ATTR_PAGE_ON, TRUE);
        aFooterSet.Put(aOn);

        const SwFmtFrmSize &rFrmSize = pFooterFmt->GetFrmSize();
        const SwFrmSize eSizeType = rFrmSize.GetSizeType();
        SfxBoolItem aDynamic(SID_ATTR_PAGE_DYNAMIC, eSizeType != ATT_FIX_SIZE);
        aFooterSet.Put(aDynamic);

        // Links gleich rechts
        //
        SfxBoolItem aShared(SID_ATTR_PAGE_SHARED, rPageDesc.IsFooterShared());
        aFooterSet.Put(aShared);

        // Groesse
        SvxSizeItem aSize(SID_ATTR_PAGE_SIZE, Size(rFrmSize.GetSize()));
        aFooterSet.Put(aSize);

        // Rahmen-Attribute umschaufeln
        //
        aFooterSet.Put(pFooterFmt->GetAttrSet());
        aFooterSet.Put( aBoxInfo );

        // SetItem erzeugen
        //
        SvxSetItem aSetItem(SID_ATTR_PAGE_FOOTERSET, aFooterSet);
        rSet.Put(aSetItem);
    }

    // Fussnoten einbauen
    //
    SwPageFtnInfo& rInfo = (SwPageFtnInfo&)rPageDesc.GetFtnInfo();
    SwPageFtnInfoItem aFtnItem(FN_PARAM_FTN_INFO, rInfo);
    rSet.Put(aFtnItem);

    // Registerhaltigkeit

    const SwTxtFmtColl* pCol = rPageDesc.GetRegisterFmtColl();
    SwRegisterItem aReg(pCol != 0);
    aReg.SetWhich(SID_SWREGISTER_MODE);
    rSet.Put(aReg);
    if(pCol)
        rSet.Put(SfxStringItem(SID_SWREGISTER_COLLECTION, pCol->GetName()));

}


/*--------------------------------------------------------------------
    Beschreibung:   DefaultTabs setzen
 --------------------------------------------------------------------*/


void MakeDefTabs(SwTwips nDefDist, SvxTabStopItem& rTabs)
{
    if( rTabs.Count() )
        return;
    {
        SvxTabStop aSwTabStop( nDefDist, SVX_TAB_ADJUST_DEFAULT );
        rTabs.Insert( aSwTabStop );
    }
}

/*--------------------------------------------------------------------
    Beschreibung:   Abstand zwischen zwei Tabs
 --------------------------------------------------------------------*/


USHORT GetTabDist(const SvxTabStopItem& rTabs)
{
    USHORT nDefDist;
    if( rTabs.Count() )
        nDefDist = (USHORT)( rTabs[0].GetTabPos() );
    else
        nDefDist = 1134;     // 2cm
    return nDefDist;
}


// erfrage ob im Set eine Sfx-PageDesc-Kombination vorliegt und returne diese
void SfxToSwPageDescAttr( const SwWrtShell& rShell, SfxItemSet& rSet )
{
    const SfxPoolItem* pItem;
    SwFmtPageDesc aPgDesc;

    BOOL bChanged = FALSE;
    // Seitennummer
    if(SFX_ITEM_SET == rSet.GetItemState(SID_ATTR_PARA_PAGENUM, FALSE, &pItem))
    {
        aPgDesc.SetNumOffset(((SfxUInt16Item*)pItem)->GetValue());
        bChanged = TRUE;
    }
    if( SFX_ITEM_SET == rSet.GetItemState( SID_ATTR_PARA_MODEL, FALSE, &pItem ))
    {
        const String& rDescName = ((SvxPageModelItem*)pItem)->GetValue();
        if( rDescName.Len() )   // kein Name -> PageDesc ausschalten!
        {
            // nur loeschen, wenn PageDesc eingschaltet wird!
            rSet.ClearItem( RES_BREAK );
            SwPageDesc* pDesc = ((SwWrtShell&)rShell).FindPageDescByName(
                                                    rDescName, TRUE );
            if( pDesc )
                pDesc->Add( &aPgDesc );
        }
        rSet.ClearItem( SID_ATTR_PARA_MODEL );
        bChanged = TRUE;
    }
    else
    {
        SfxItemSet aCoreSet(rShell.GetView().GetPool(), RES_PAGEDESC, RES_PAGEDESC );
        rShell.GetAttr( aCoreSet );
        if(SFX_ITEM_SET == aCoreSet.GetItemState( RES_PAGEDESC, TRUE, &pItem ) )
        {
            if( ((SwFmtPageDesc*)pItem)->GetPageDesc() )
            {
                ((SwFmtPageDesc*)pItem)->GetPageDesc()->Add(&aPgDesc);
            }
        }
    }


    if(bChanged)
        rSet.Put( aPgDesc );
}


// erfrage ob im Set eine Sfx-PageDesc-Kombination vorliegt und returne diese
void SwToSfxPageDescAttr( SfxItemSet& rCoreSet )
{
    const SfxPoolItem* pItem = 0;
    String aName;
    USHORT nPageNum = 0;
    BOOL bPut = TRUE;
    switch( rCoreSet.GetItemState( RES_PAGEDESC, TRUE, &pItem ) )
    {
    case SFX_ITEM_SET:
        {
            if( ((SwFmtPageDesc*)pItem)->GetPageDesc() )
            {
                aName = ((SwFmtPageDesc*)pItem)->GetPageDesc()->GetName();
                nPageNum = ((SwFmtPageDesc*)pItem)->GetNumOffset();
            }
            rCoreSet.ClearItem( RES_PAGEDESC );
            // Seitennummer
        }
        break;

    case SFX_ITEM_AVAILABLE:
        break;

    default:
        bPut = FALSE;
    }
    SfxUInt16Item aPageNum( SID_ATTR_PARA_PAGENUM, nPageNum );
    rCoreSet.Put( aPageNum );

    if(bPut)
        rCoreSet.Put( SvxPageModelItem( aName, TRUE, SID_ATTR_PARA_MODEL ) );
}

/*--------------------------------------------------------------------
    Beschreibung:   Metric ermitteln
 --------------------------------------------------------------------*/


FieldUnit   GetDfltMetric(BOOL bWeb)
{
    return SW_MOD()->GetUsrPref(bWeb)->GetMetric();
}

/*--------------------------------------------------------------------
    Beschreibung:   Metric ermitteln
 --------------------------------------------------------------------*/


void    SetDfltMetric( FieldUnit eMetric, BOOL bWeb )
{
    SW_MOD()->ApplyUserMetric(eMetric, bWeb);
}

/*-----------------15.07.97 10:49-------------------

--------------------------------------------------*/
// Umwandlung:  Sub(LibName.ModulName) -> LibName.ModulName.Sub
String ConvertMacroFormat(const String& rInput )
{
    String sTemp(rInput);
    USHORT nFound = sTemp.Search('(');
    USHORT nTokenCount = sTemp.GetTokenCount('.');
    String sMac(rInput);
    if( STRING_NOTFOUND != nFound || nTokenCount < 2)
    {
        if(nTokenCount < 2)
            nFound = sTemp.Len();
        // anderes Format: "Macro(Bibliothek.Modul)"
        sMac = sTemp.Copy(0, nFound);
        sTemp.Erase(0, nFound);
        if(sTemp.Len() > 2)
        {
            sMac.Insert('.', 0);
            sMac.Insert(sTemp.Copy(1, sTemp.Len() - 2), 0);
        }
    }
    return sMac;
}


BOOL GetFileFilterNameDlg( Window& rParent, String& rFileName,
                            String* pPassword, String* pFilterName,
                            SfxMedium** ppMedium )
{
    Window* pDefDlgParent = Application::GetDefDialogParent();
    Application::SetDefDialogParent( &rParent );
    SfxMedium* pMed = SFX_APP()->InsertDocumentDialog( 0, String::CreateFromAscii("swriter") );
    if( pMed )
    {
        if( pFilterName )
            *pFilterName = pMed->GetFilter()->GetFilterName();

        rFileName = URIHelper::SmartRelToAbs( pMed->GetName() );
        if( pPassword )
        {
            const SfxPoolItem* pItem;
            if( SFX_ITEM_SET == pMed->GetItemSet()->GetItemState(
                                    SID_PASSWORD, FALSE, &pItem ) )
                *pPassword = ((SfxStringItem*)pItem)->GetValue();
        }

        if( ppMedium )
            *ppMedium = pMed;
        else
            delete pMed;        // das brauchen wir nicht mehr !
    }
    Application::SetDefDialogParent( pDefDlgParent );
    return 0 != pMed;
}

/*-----------------09.04.98 16:58-------------------

--------------------------------------------------*/
USHORT InsertStringSorted(const String& rEntry, ListBox& rToFill, USHORT nOffset )
{
    USHORT i = nOffset;
    CollatorWrapper& rCaseColl = ::GetAppCaseCollator();

    for( ; i < rToFill.GetEntryCount(); i++ )
    {
        if( 0 < rCaseColl.compareString( rToFill.GetEntry(i), rEntry ))
            break;
    }
    return rToFill.InsertEntry(rEntry, i);
}
void FillCharStyleListBox(ListBox& rToFill, SwDocShell* pDocSh, BOOL bSorted, BOOL bWithDefault)
{
    BOOL bHasOffset = rToFill.GetEntryCount() > 0;
    SfxStyleSheetBasePool* pPool = pDocSh->GetStyleSheetPool();
    pPool->SetSearchMask(SFX_STYLE_FAMILY_CHAR, SFXSTYLEBIT_ALL);
    SwDoc* pDoc = pDocSh->GetDoc();
    const SfxStyleSheetBase* pBase = pPool->First();
    String sStandard;
    SwStyleNameMapper::FillUIName( RES_POOLCOLL_STANDARD, sStandard );
    while(pBase)
    {
        if(bWithDefault || pBase->GetName() !=  sStandard)
        {
            USHORT nPos;
            if(bSorted)
                nPos = InsertStringSorted(pBase->GetName(), rToFill, bHasOffset );
            else
                nPos = rToFill.InsertEntry(pBase->GetName());
            long nPoolId = SwStyleNameMapper::GetPoolIdFromUIName( pBase->GetName(), GET_POOLID_CHRFMT );
            rToFill.SetEntryData( nPos, (void*) (nPoolId));
        }
        pBase = pPool->Next();
    }
    // non-pool styles
    const SwCharFmts* pFmts = pDoc->GetCharFmts();
    for(USHORT i = 0; i < pFmts->Count(); i++)
    {
        const SwCharFmt* pFmt = (*pFmts)[i];
        if(pFmt->IsDefault())
            continue;
        const String& rName = pFmt->GetName();
        if(rToFill.GetEntryPos(rName) == LISTBOX_ENTRY_NOTFOUND)
        {
            USHORT nPos;
            if(bSorted)
                nPos = InsertStringSorted(rName, rToFill, bHasOffset );
            else
                nPos = rToFill.InsertEntry(rName);
            long nPoolId = USHRT_MAX;
            rToFill.SetEntryData( nPos, (void*) (nPoolId));
        }
    }
};

/* -----------------27.04.98 08:26-------------------
 *
 * --------------------------------------------------*/
SwTwips GetTableWidth( SwFrmFmt* pFmt, SwTabCols& rCols, USHORT *pPercent,
            SwWrtShell* pSh )
{
    //Die Breite zu besorgen ist etwas komplizierter.
    SwTwips nWidth;
    const SwHoriOrient eOri = pFmt->GetHoriOrient().GetHoriOrient();
    switch(eOri)
    {
        case HORI_FULL: nWidth = rCols.GetRight(); break;
        case HORI_LEFT_AND_WIDTH:
        case HORI_LEFT:
        case HORI_RIGHT:
        case HORI_CENTER:
            nWidth = pFmt->GetFrmSize().GetWidth();
        break;
        default:
        {
            if(pSh)
            {
                const SwFrmFmt *pFlyFmt;
                if ( 0 == (pFlyFmt = pSh->GetFlyFrmFmt()) )
                {
                    nWidth = pSh->GetAnyCurRect(RECT_PAGE_PRT).Width();
                }
                else
                {
                    nWidth = pSh->GetAnyCurRect(RECT_FLY_PRT_EMBEDDED).Width();
                }
            }
            else
            {
                DBG_ERROR("wo soll die Breite denn herkommen?")
            }
            const SvxLRSpaceItem& rLRSpace = pFmt->GetLRSpace();
            nWidth -= (rLRSpace.GetRight() + rLRSpace.GetLeft());
        }
    }
    if (pPercent)
        *pPercent = pFmt->GetFrmSize().GetWidthPercent();
    return nWidth;
}

/*------------------------------------------------------------------------*/

String GetAppLangDateTimeString( const DateTime& rDT )
{
    LocaleDataWrapper& rAppLclData = GetAppLocaleData();
    String sRet( rAppLclData.getDate( rDT ));
    ( sRet += ' ' ) += rAppLclData.getTime( rDT, FALSE, FALSE );
    return sRet;
}
