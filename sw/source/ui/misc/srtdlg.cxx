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

#include "srtdlg.hxx"

#include <vcl/msgbox.hxx>
#include <svl/intitem.hxx>
#include <svl/eitem.hxx>
#include <sfx2/dispatch.hxx>
#include <svx/svxids.hrc>
#include <editeng/unolingu.hxx>
#include <svx/svxdlg.hxx>
#include <svx/dialogs.hrc>
#include <unotools/collatorwrapper.hxx>
#include <svtools/collatorres.hxx>
#include <swwait.hxx>
#include <view.hxx>
#include <cmdid.h>
#include <wrtsh.hxx>
#include <misc.hrc>
#include <srtdlg.hrc>
#include <swtable.hxx>
#include <node.hxx>
#include <tblsel.hxx>
#include <sfx2/request.hxx>

static sal_Bool bCheck1 = sal_True;
static sal_Bool bCheck2 = sal_False;
static sal_Bool bCheck3 = sal_False;

static sal_uInt16 nCol1 = 1;
static sal_uInt16 nCol2 = 1;
static sal_uInt16 nCol3 = 1;

static sal_uInt16 nType1 = 0;
static sal_uInt16 nType2 = 0;
static sal_uInt16 nType3 = 0;

static sal_uInt16 nLang = LANGUAGE_NONE;

static sal_Bool   bAsc1  = sal_True;
static sal_Bool   bAsc2  = sal_True;
static sal_Bool   bAsc3  = sal_True;
static sal_Bool   bCol   = sal_False;
static sal_Bool   bCsSens= sal_False;

static sal_Unicode    cDeli  = '\t';

using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;
using ::rtl::OUString;

void lcl_ClearLstBoxAndDelUserData( ListBox& rLstBox )
{
    void* pDel;
    for( sal_uInt16 n = 0, nEnd = rLstBox.GetEntryCount(); n < nEnd; ++n )
        if( 0 != ( pDel = rLstBox.GetEntryData( n )) )
            delete (String*)pDel;
    rLstBox.Clear();
}

/*--------------------------------------------------------------------
     Description: determine lines and columns for table selection
 --------------------------------------------------------------------*/
sal_Bool lcl_GetSelTbl( SwWrtShell &rSh, sal_uInt16& rX, sal_uInt16& rY )
{
    const SwTableNode* pTblNd = rSh.IsCrsrInTbl();
    if( !pTblNd )
        return sal_False;

    _FndBox aFndBox( 0, 0 );

    // look for all boxes / lines
    {
        SwSelBoxes aSelBoxes;
        ::GetTblSel( rSh, aSelBoxes );
        _FndPara aPara( aSelBoxes, &aFndBox );
        const SwTable& rTbl = pTblNd->GetTable();
        ForEach_FndLineCopyCol( (SwTableLines&)rTbl.GetTabLines(), &aPara );
    }
    rX = aFndBox.GetLines().size();
    if( !rX )
        return sal_False;

    rY = aFndBox.GetLines().front().GetBoxes().size();
    return sal_True;
}

/*--------------------------------------------------------------------
     Description: init list
 --------------------------------------------------------------------*/
SwSortDlg::SwSortDlg(Window* pParent, SwWrtShell &rShell) :

    SvxStandardDialog(pParent, SW_RES(DLG_SORTING)),

    aColLbl(this,       SW_RES(FT_COL   )),
    aTypLbl(this,       SW_RES(FT_KEYTYP)),
    aDirLbl(this,       SW_RES(FT_DIR   )),
    aSortFL(this,      SW_RES(FL_SORT_2  )),


    aKeyCB1(this,       SW_RES(CB_KEY1  )),
    aColEdt1(this,      SW_RES(ED_KEY1  )),
    aTypDLB1(this,      SW_RES(DLB_KEY1 )),
    aSortUpRB(this,     SW_RES(RB_UP    )),
    aSortDnRB(this,     SW_RES(RB_DN    )),

    aKeyCB2(this,       SW_RES(CB_KEY2  )),
    aColEdt2(this,      SW_RES(ED_KEY2  )),
    aTypDLB2(this,      SW_RES(DLB_KEY2 )),
    aSortUp2RB(this,    SW_RES(RB_UP2    )),
    aSortDn2RB(this,    SW_RES(RB_DN2    )),

    aKeyCB3(this,       SW_RES(CB_KEY3  )),
    aColEdt3(this,      SW_RES(ED_KEY3  )),
    aTypDLB3(this,      SW_RES(DLB_KEY3 )),
    aSortUp3RB(this,    SW_RES(RB_UP3    )),
    aSortDn3RB(this,    SW_RES(RB_DN3    )),
    aDirFL(this,       SW_RES(FL_DIR   )),

    aColumnRB(this,     SW_RES(RB_COL   )),
    aRowRB(this,        SW_RES(RB_ROW   )),

    aDelimFL(this,     SW_RES(FL_DELIM )),
    aDelimTabRB(this,   SW_RES(RB_TAB   )),
    aDelimFreeRB(this,  SW_RES(RB_TABCH )),
    aDelimEdt(this,     SW_RES(ED_TABCH )),
    aDelimPB(this,      SW_RES( PB_DELIM)),

    aLangFL(this,       SW_RES( FL_LANG )),
    aLangLB(this,       SW_RES( LB_LANG )),

    aSortOptFL(this,    SW_RES( FL_SORT )),
    aCaseCB(this,       SW_RES( CB_CASE )),

    aOkBtn(this,        SW_RES(BT_OK    )),
    aCancelBtn(this,    SW_RES(BT_CANCEL)),
    aHelpBtn(this,      SW_RES(BT_HELP  )),

    aColTxt(            SW_RES(STR_COL)),
    aRowTxt(            SW_RES(STR_ROW)),
    aNumericTxt(        SW_RES(STR_NUMERIC)),
    rSh(rShell),
    pColRes( 0 ),
    nX( 99 ),
    nY( 99 )
{
    aColEdt1.SetAccessibleName(aColLbl.GetText());
    aColEdt2.SetAccessibleName(aColLbl.GetText());
    aColEdt3.SetAccessibleName(aColLbl.GetText());
    aTypDLB1.SetAccessibleName(aTypLbl.GetText());
    aTypDLB2.SetAccessibleName(aTypLbl.GetText());
    aTypDLB3.SetAccessibleName(aTypLbl.GetText());
    aSortUpRB.SetAccessibleRelationMemberOf( &aKeyCB1 );
    aSortDnRB.SetAccessibleRelationMemberOf( &aKeyCB1 );
    aSortUp2RB.SetAccessibleRelationMemberOf( &aKeyCB2 );
    aSortDn2RB.SetAccessibleRelationMemberOf( &aKeyCB2 );
    aSortUp3RB.SetAccessibleRelationMemberOf( &aKeyCB3 );
    aSortDn3RB.SetAccessibleRelationMemberOf( &aKeyCB3 );

    aDelimEdt.SetMaxTextLen( 1 );
    if(rSh.GetSelectionType() &
            (nsSelectionType::SEL_TBL|nsSelectionType::SEL_TBL_CELLS) )
    {
        aColumnRB.Check(bCol);
        aColLbl.SetText(bCol ? aRowTxt : aColTxt);
        aRowRB.Check(!bCol);
        aDelimTabRB.Enable(sal_False);
        aDelimFreeRB.Enable(sal_False);
        aDelimEdt.Enable(sal_False);
    }
    else
    {
        aColumnRB.Enable(sal_False);
        aRowRB.Check(sal_True);
        aColLbl.SetText(aColTxt);
    }

    // initialise
    Link aLk = LINK(this,SwSortDlg, CheckHdl);
    aKeyCB1.SetClickHdl( aLk );
    aKeyCB2.SetClickHdl( aLk );
    aKeyCB3.SetClickHdl( aLk );
    aColumnRB.SetClickHdl( aLk );
    aRowRB.SetClickHdl( aLk );

    aLk = LINK(this,SwSortDlg, DelimHdl);
    aDelimFreeRB.SetClickHdl(aLk);
    aDelimTabRB.SetClickHdl(aLk);

    aDelimPB.SetClickHdl( LINK( this, SwSortDlg, DelimCharHdl ));

    aKeyCB1.Check(bCheck1);
    aKeyCB2.Check(bCheck2);
    aKeyCB3.Check(bCheck3);

    aColEdt1.SetValue(nCol1);
    aColEdt2.SetValue(nCol2);
    aColEdt3.SetValue(nCol3);

    // first initialise the language, then select the
    if( LANGUAGE_NONE == nLang || LANGUAGE_DONTKNOW == nLang )
        nLang = (sal_uInt16)GetAppLanguage();

    aLangLB.SetLanguageList( LANG_LIST_ALL | LANG_LIST_ONLY_KNOWN, sal_True, sal_False);
    aLangLB.SelectLanguage( nLang );

    LanguageHdl( 0 );
    aLangLB.SetSelectHdl( LINK( this, SwSortDlg, LanguageHdl ));

    aSortUpRB.Check(bAsc1);
    aSortDnRB.Check(!bAsc1);
    aSortUp2RB.Check(bAsc2);
    aSortDn2RB.Check(!bAsc2);
    aSortUp3RB.Check(bAsc3);
    aSortDn3RB.Check(!bAsc3);

    aCaseCB.Check( bCsSens );

    aDelimTabRB.Check(cDeli == '\t');
    if(!aDelimTabRB.IsChecked())
    {
        aDelimEdt.SetText(rtl::OUString(cDeli));
        aDelimFreeRB.Check(sal_True);
        DelimHdl(&aDelimFreeRB);
    }
    else
        DelimHdl(&aDelimTabRB);

    FreeResource();
    if( ::lcl_GetSelTbl( rSh, nX, nY) )
    {
        sal_uInt16 nMax = aRowRB.IsChecked()? nY : nX;
        aColEdt1.SetMax(nMax);
        aColEdt2.SetMax(nMax);
        aColEdt3.SetMax(nMax);
    }

    aDelimEdt.SetAccessibleRelationLabeledBy(&aDelimFreeRB);
    aDelimPB.SetAccessibleRelationLabeledBy(&aDelimFreeRB);
    aDelimPB.SetAccessibleRelationMemberOf(&aDelimFL);

    aColEdt1.SetAccessibleRelationMemberOf(&aKeyCB1);
    aColEdt1.SetAccessibleRelationLabeledBy(&aColLbl);
    aTypDLB1.SetAccessibleRelationMemberOf(&aKeyCB1);
    aTypDLB1.SetAccessibleRelationLabeledBy(&aTypLbl);

    aColEdt2.SetAccessibleRelationMemberOf(&aKeyCB2);
    aColEdt2.SetAccessibleRelationLabeledBy(&aColLbl);
    aTypDLB2.SetAccessibleRelationMemberOf(&aKeyCB2);
    aTypDLB2.SetAccessibleRelationLabeledBy(&aTypLbl);

    aColEdt3.SetAccessibleRelationMemberOf(&aKeyCB3);
    aColEdt3.SetAccessibleRelationLabeledBy(&aColLbl);
    aTypDLB3.SetAccessibleRelationMemberOf(&aKeyCB3);
    aTypDLB3.SetAccessibleRelationLabeledBy(&aTypLbl);
}

SwSortDlg::~SwSortDlg()
{
    ::lcl_ClearLstBoxAndDelUserData( aTypDLB1 );
    ::lcl_ClearLstBoxAndDelUserData( aTypDLB2 );
    ::lcl_ClearLstBoxAndDelUserData( aTypDLB3 );
    delete pColRes;
}

sal_Unicode SwSortDlg::GetDelimChar() const
{
    sal_Unicode cRet = '\t';
    if( !aDelimTabRB.IsChecked() )
    {
        String aTmp( aDelimEdt.GetText() );
        if( aTmp.Len() )
            cRet = aTmp.GetChar( 0 );
    }
    return cRet;
}

/*--------------------------------------------------------------------
    Description: pass on to the Core
 --------------------------------------------------------------------*/
void SwSortDlg::Apply()
{
    // save all settings
    bCheck1 = aKeyCB1.IsChecked();
    bCheck2 = aKeyCB2.IsChecked();
    bCheck3 = aKeyCB3.IsChecked();

    nCol1 = (sal_uInt16)aColEdt1.GetValue();
    nCol2 = (sal_uInt16)aColEdt2.GetValue();
    nCol3 = (sal_uInt16)aColEdt3.GetValue();

    nType1 = aTypDLB1.GetSelectEntryPos();
    nType2 = aTypDLB2.GetSelectEntryPos();
    nType3 = aTypDLB3.GetSelectEntryPos();

    bAsc1 = aSortUpRB.IsChecked();
    bAsc2 = aSortUp2RB.IsChecked();
    bAsc3 = aSortUp3RB.IsChecked();
    bCol = aColumnRB.IsChecked();
    nLang = aLangLB.GetSelectLanguage();
    cDeli = GetDelimChar();
    bCsSens = aCaseCB.IsChecked();

    void* pUserData;
    SwSortOptions aOptions;
    if( bCheck1 )
    {
        String sEntry( aTypDLB1.GetSelectEntry() );
        if( sEntry == aNumericTxt )
            sEntry.Erase();
        else if( 0 != (pUserData = aTypDLB1.GetEntryData(
                                            aTypDLB1.GetSelectEntryPos())) )
            sEntry = *(String*)pUserData;

        SwSortKey *pKey = new SwSortKey( nCol1, sEntry,
                                    bAsc1 ? SRT_ASCENDING : SRT_DESCENDING );
        aOptions.aKeys.push_back( pKey );
    }

    if( bCheck2 )
    {
        String sEntry( aTypDLB2.GetSelectEntry() );
        if( sEntry == aNumericTxt )
            sEntry.Erase();
        else if( 0 != (pUserData = aTypDLB2.GetEntryData(
                                            aTypDLB2.GetSelectEntryPos())) )
            sEntry = *(String*)pUserData;

        SwSortKey *pKey = new SwSortKey( nCol2, sEntry,
                                    bAsc2 ? SRT_ASCENDING : SRT_DESCENDING );
        aOptions.aKeys.push_back( pKey );
    }

    if( bCheck3 )
    {
        String sEntry( aTypDLB3.GetSelectEntry() );
        if( sEntry == aNumericTxt )
            sEntry.Erase();
        else if( 0 != (pUserData = aTypDLB3.GetEntryData(
                                            aTypDLB3.GetSelectEntryPos())) )
            sEntry = *(String*)pUserData;

        SwSortKey *pKey = new SwSortKey( nCol3, sEntry,
                                    bAsc3 ? SRT_ASCENDING : SRT_DESCENDING );
        aOptions.aKeys.push_back( pKey );
    }

    aOptions.eDirection =  bCol ? SRT_COLUMNS : SRT_ROWS;
    aOptions.cDeli = cDeli;
    aOptions.nLanguage = nLang;
    aOptions.bTable = rSh.IsTableMode();
    aOptions.bIgnoreCase = !bCsSens;

    sal_Bool bRet;
    {
        SwWait aWait( *rSh.GetView().GetDocShell(), sal_True );
        rSh.StartAllAction();
        if( 0 != (bRet = rSh.Sort( aOptions )))
            rSh.SetModified();
        rSh.EndAllAction();
    }

    if( !bRet )
        InfoBox( this->GetParent(), SW_RES(MSG_SRTERR)).Execute();
}

IMPL_LINK( SwSortDlg, DelimHdl, RadioButton*, pButton )
{
    sal_Bool bEnable = pButton == &aDelimFreeRB && aDelimFreeRB.IsEnabled();
    aDelimEdt.Enable( bEnable );
    aDelimPB.Enable( bEnable );
    return 0;
}

IMPL_LINK_NOARG(SwSortDlg, DelimCharHdl)
{
    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    if(pFact)
    {
        SfxAllItemSet aSet( rSh.GetAttrPool() );
        aSet.Put( SfxInt32Item( SID_ATTR_CHAR, GetDelimChar() ) );
        SfxAbstractDialog* pMap = pFact->CreateSfxDialog( &aDelimPB, aSet,
            rSh.GetView().GetViewFrame()->GetFrame().GetFrameInterface(), RID_SVXDLG_CHARMAP );
        if( RET_OK == pMap->Execute() )
        {
            SFX_ITEMSET_ARG( pMap->GetOutputItemSet(), pItem, SfxInt32Item, SID_ATTR_CHAR, sal_False );
            if ( pItem )
                aDelimEdt.SetText( rtl::OUString(pItem->GetValue()) );
        }

        delete pMap;
    }
    return 0;
}

IMPL_LINK( SwSortDlg, CheckHdl, CheckBox *, pCheck )
{
    if( pCheck == ( CheckBox* ) &aRowRB)
    {
        aColLbl.SetText(aColTxt);
        aColEdt1.SetMax(nY);
        aColEdt2.SetMax(nY);
        aColEdt3.SetMax(nY);

        aColEdt1.SetAccessibleName(aColTxt);
        aColEdt2.SetAccessibleName(aColTxt);
        aColEdt3.SetAccessibleName(aColTxt);
    }
    else if( pCheck == ( CheckBox* ) &aColumnRB)
    {
        aColLbl.SetText(aRowTxt);
        aColEdt1.SetMax(nX);
        aColEdt2.SetMax(nX);
        aColEdt3.SetMax(nX);

        aColEdt1.SetAccessibleName(aRowTxt);
        aColEdt2.SetAccessibleName(aRowTxt);
        aColEdt3.SetAccessibleName(aRowTxt);
    }
    else if(!aKeyCB1.IsChecked() &&
                !aKeyCB2.IsChecked() &&
                    !aKeyCB3.IsChecked())
        pCheck->Check(sal_True);
    return 0;
}

IMPL_LINK( SwSortDlg, LanguageHdl, ListBox*, pLBox )
{
    lang::Locale aLcl( SvxCreateLocale( aLangLB.GetSelectLanguage() ) );
    Sequence < OUString > aSeq(
                            GetAppCollator().listCollatorAlgorithms( aLcl ));

    if( !pColRes )
        pColRes = new CollatorResource();

    const sal_uInt16 nLstBoxCnt = 3;
    ListBox* aLstArr[ nLstBoxCnt ] = { &aTypDLB1, &aTypDLB2, &aTypDLB3 };
    sal_uInt16* aTypeArr[ nLstBoxCnt ] = { &nType1, &nType2, &nType3 };
    String aOldStrArr[ nLstBoxCnt ];
    sal_uInt16 n;

    void* pUserData;
    for( n = 0; n < nLstBoxCnt; ++n )
    {
        ListBox* pL = aLstArr[ n ];
        if( 0 != (pUserData = pL->GetEntryData( pL->GetSelectEntryPos())) )
            aOldStrArr[ n ] = *(String*)pUserData;
        ::lcl_ClearLstBoxAndDelUserData( *pL );
    }

    sal_uInt16 nInsPos;
    String sAlg, sUINm;
    for( long nCnt = 0, nEnd = aSeq.getLength(); nCnt <= nEnd; ++nCnt )
    {
        if( nCnt < nEnd )
            sUINm = pColRes->GetTranslation( sAlg = aSeq[ nCnt ] );
        else
            sUINm = sAlg = aNumericTxt;

        for( n = 0; n < nLstBoxCnt; ++n )
        {
            ListBox* pL = aLstArr[ n ];
            nInsPos = pL->InsertEntry( sUINm );
            pL->SetEntryData( nInsPos, new String( sAlg ));
            if( pLBox && sAlg == aOldStrArr[ n ] )
                pL->SelectEntryPos( nInsPos );
        }
    }

    for( n = 0; n < nLstBoxCnt; ++n )
    {
        ListBox* pL = aLstArr[ n ];
        if( !pLBox )
            pL->SelectEntryPos( *aTypeArr[n] );
        else if( LISTBOX_ENTRY_NOTFOUND == pL->GetSelectEntryPos() )
            pL->SelectEntryPos( 0 );
    }
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
