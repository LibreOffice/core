/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
#include <app.hrc>
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

static void lcl_ClearLstBoxAndDelUserData( ListBox& rLstBox )
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
static bool lcl_GetSelTbl( SwWrtShell &rSh, sal_uInt16& rX, sal_uInt16& rY )
{
    const SwTableNode* pTblNd = rSh.IsCrsrInTbl();
    if( !pTblNd )
        return false;

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
        return false;

    rY = aFndBox.GetLines().front().GetBoxes().size();
    return true;
}

/*--------------------------------------------------------------------
     Description: init list
 --------------------------------------------------------------------*/
SwSortDlg::SwSortDlg(Window* pParent, SwWrtShell &rShell)
    : SvxStandardDialog(pParent, "SortDialog", "modules/swriter/ui/sortdialog.ui")
    , aColTxt(SW_RES(STR_COL))
    , aRowTxt(SW_RES(STR_ROW))
    , aNumericTxt(SW_RES(STR_NUMERIC))
    , rSh(rShell)
    , pColRes(0)
    , nX(99)
    , nY(99)
{
    get(m_pColLbl, "column");
    get(m_pTypLbl, "keytype");

    get(m_pKeyCB1, "key1");
    get(m_pColEdt1, "colsb1");
    get(m_pTypDLB1, "typelb1");
    get(m_pSortUp1RB, "up1");
    get(m_pSortDn1RB, "down1");

    get(m_pKeyCB2, "key2");
    get(m_pColEdt2, "colsb2");
    get(m_pTypDLB2, "typelb2");
    get(m_pSortUp2RB, "up2");
    get(m_pSortDn2RB, "down2");

    get(m_pKeyCB3, "key3");
    get(m_pColEdt3, "colsb3");
    get(m_pTypDLB3, "typelb3");
    get(m_pSortUp3RB, "up3");
    get(m_pSortDn3RB, "down3");

    get(m_pColumnRB, "columns");
    get(m_pRowRB, "rows");

    get(m_pDelimTabRB, "tabs");
    get(m_pDelimFreeRB, "character");
    get(m_pDelimEdt, "separator");
    get(m_pDelimPB, "delimpb");

    get(m_pLangLB, "langlb");
    get(m_pCaseCB, "matchcase");

    m_pColEdt1->SetAccessibleName(m_pColLbl->GetText());
    m_pColEdt2->SetAccessibleName(m_pColLbl->GetText());
    m_pColEdt3->SetAccessibleName(m_pColLbl->GetText());
    m_pTypDLB1->SetAccessibleName(m_pTypLbl->GetText());
    m_pTypDLB2->SetAccessibleName(m_pTypLbl->GetText());
    m_pTypDLB3->SetAccessibleName(m_pTypLbl->GetText());
    m_pSortUp1RB->SetAccessibleRelationMemberOf(m_pKeyCB1);
    m_pSortDn1RB->SetAccessibleRelationMemberOf(m_pKeyCB1);
    m_pSortUp2RB->SetAccessibleRelationMemberOf(m_pKeyCB2);
    m_pSortDn2RB->SetAccessibleRelationMemberOf(m_pKeyCB2);
    m_pSortUp3RB->SetAccessibleRelationMemberOf(m_pKeyCB3);
    m_pSortDn3RB->SetAccessibleRelationMemberOf(m_pKeyCB3);

    m_pDelimEdt->SetMaxTextLen( 1 );
    if(rSh.GetSelectionType() &
            (nsSelectionType::SEL_TBL|nsSelectionType::SEL_TBL_CELLS) )
    {
        m_pColumnRB->Check(bCol);
        m_pColLbl->SetText(bCol ? aRowTxt : aColTxt);
        m_pRowRB->Check(!bCol);
        m_pDelimTabRB->Enable(sal_False);
        m_pDelimFreeRB->Enable(sal_False);
        m_pDelimEdt->Enable(sal_False);
    }
    else
    {
        m_pColumnRB->Enable(sal_False);
        m_pRowRB->Check(sal_True);
        m_pColLbl->SetText(aColTxt);
    }

    // initialise
    Link aLk = LINK(this,SwSortDlg, CheckHdl);
    m_pKeyCB1->SetClickHdl( aLk );
    m_pKeyCB2->SetClickHdl( aLk );
    m_pKeyCB3->SetClickHdl( aLk );
    m_pColumnRB->SetClickHdl( aLk );
    m_pRowRB->SetClickHdl( aLk );

    aLk = LINK(this,SwSortDlg, DelimHdl);
    m_pDelimFreeRB->SetClickHdl(aLk);
    m_pDelimTabRB->SetClickHdl(aLk);

    m_pDelimPB->SetClickHdl( LINK( this, SwSortDlg, DelimCharHdl ));

    m_pKeyCB1->Check(bCheck1);
    m_pKeyCB2->Check(bCheck2);
    m_pKeyCB3->Check(bCheck3);

    m_pColEdt1->SetValue(nCol1);
    m_pColEdt2->SetValue(nCol2);
    m_pColEdt3->SetValue(nCol3);

    // first initialise the language, then select the
    if( LANGUAGE_NONE == nLang || LANGUAGE_DONTKNOW == nLang )
        nLang = (sal_uInt16)GetAppLanguage();

    m_pLangLB->SetLanguageList( LANG_LIST_ALL | LANG_LIST_ONLY_KNOWN, sal_True, sal_False);
    m_pLangLB->SelectLanguage( nLang );

    LanguageHdl( 0 );
    m_pLangLB->SetSelectHdl( LINK( this, SwSortDlg, LanguageHdl ));

    m_pSortUp1RB->Check(bAsc1);
    m_pSortDn1RB->Check(!bAsc1);
    m_pSortUp2RB->Check(bAsc2);
    m_pSortDn2RB->Check(!bAsc2);
    m_pSortUp3RB->Check(bAsc3);
    m_pSortDn3RB->Check(!bAsc3);

    m_pCaseCB->Check( bCsSens );

    m_pDelimTabRB->Check(cDeli == '\t');
    if(!m_pDelimTabRB->IsChecked())
    {
        m_pDelimEdt->SetText(rtl::OUString(cDeli));
        m_pDelimFreeRB->Check(sal_True);
        DelimHdl(m_pDelimFreeRB);
    }
    else
        DelimHdl(m_pDelimTabRB);

    if( ::lcl_GetSelTbl( rSh, nX, nY) )
    {
        sal_uInt16 nMax = m_pRowRB->IsChecked()? nY : nX;
        m_pColEdt1->SetMax(nMax);
        m_pColEdt2->SetMax(nMax);
        m_pColEdt3->SetMax(nMax);
    }

    m_pDelimPB->SetAccessibleRelationMemberOf(m_pDelimFreeRB);

    m_pColEdt1->SetAccessibleRelationMemberOf(m_pKeyCB1);
    m_pTypDLB1->SetAccessibleRelationMemberOf(m_pKeyCB1);

    m_pColEdt2->SetAccessibleRelationMemberOf(m_pKeyCB2);
    m_pTypDLB2->SetAccessibleRelationMemberOf(m_pKeyCB2);

    m_pColEdt3->SetAccessibleRelationMemberOf(m_pKeyCB3);
    m_pTypDLB3->SetAccessibleRelationMemberOf(m_pKeyCB3);
}

SwSortDlg::~SwSortDlg()
{
    ::lcl_ClearLstBoxAndDelUserData(*m_pTypDLB1);
    ::lcl_ClearLstBoxAndDelUserData(*m_pTypDLB2);
    ::lcl_ClearLstBoxAndDelUserData(*m_pTypDLB3);
    delete pColRes;
}

sal_Unicode SwSortDlg::GetDelimChar() const
{
    sal_Unicode cRet = '\t';
    if( !m_pDelimTabRB->IsChecked() )
    {
        String aTmp( m_pDelimEdt->GetText() );
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
    bCheck1 = m_pKeyCB1->IsChecked();
    bCheck2 = m_pKeyCB2->IsChecked();
    bCheck3 = m_pKeyCB3->IsChecked();

    nCol1 = (sal_uInt16)m_pColEdt1->GetValue();
    nCol2 = (sal_uInt16)m_pColEdt2->GetValue();
    nCol3 = (sal_uInt16)m_pColEdt3->GetValue();

    nType1 = m_pTypDLB1->GetSelectEntryPos();
    nType2 = m_pTypDLB2->GetSelectEntryPos();
    nType3 = m_pTypDLB3->GetSelectEntryPos();

    bAsc1 = m_pSortUp1RB->IsChecked();
    bAsc2 = m_pSortUp2RB->IsChecked();
    bAsc3 = m_pSortUp3RB->IsChecked();
    bCol = m_pColumnRB->IsChecked();
    nLang = m_pLangLB->GetSelectLanguage();
    cDeli = GetDelimChar();
    bCsSens = m_pCaseCB->IsChecked();

    void* pUserData;
    SwSortOptions aOptions;
    if( bCheck1 )
    {
        String sEntry( m_pTypDLB1->GetSelectEntry() );
        if( sEntry == aNumericTxt )
            sEntry.Erase();
        else if( 0 != (pUserData = m_pTypDLB1->GetEntryData(
                                            m_pTypDLB1->GetSelectEntryPos())) )
            sEntry = *(String*)pUserData;

        SwSortKey *pKey = new SwSortKey( nCol1, sEntry,
                                    bAsc1 ? SRT_ASCENDING : SRT_DESCENDING );
        aOptions.aKeys.push_back( pKey );
    }

    if( bCheck2 )
    {
        String sEntry( m_pTypDLB2->GetSelectEntry() );
        if( sEntry == aNumericTxt )
            sEntry.Erase();
        else if( 0 != (pUserData = m_pTypDLB2->GetEntryData(
                                            m_pTypDLB2->GetSelectEntryPos())) )
            sEntry = *(String*)pUserData;

        SwSortKey *pKey = new SwSortKey( nCol2, sEntry,
                                    bAsc2 ? SRT_ASCENDING : SRT_DESCENDING );
        aOptions.aKeys.push_back( pKey );
    }

    if( bCheck3 )
    {
        String sEntry( m_pTypDLB3->GetSelectEntry() );
        if( sEntry == aNumericTxt )
            sEntry.Erase();
        else if( 0 != (pUserData = m_pTypDLB3->GetEntryData(
                                            m_pTypDLB3->GetSelectEntryPos())) )
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
    sal_Bool bEnable = pButton == m_pDelimFreeRB && m_pDelimFreeRB->IsEnabled();
    m_pDelimEdt->Enable( bEnable );
    m_pDelimPB->Enable( bEnable );
    return 0;
}

IMPL_LINK_NOARG(SwSortDlg, DelimCharHdl)
{
    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    if(pFact)
    {
        SfxAllItemSet aSet( rSh.GetAttrPool() );
        aSet.Put( SfxInt32Item( SID_ATTR_CHAR, GetDelimChar() ) );
        SfxAbstractDialog* pMap = pFact->CreateSfxDialog( m_pDelimPB, aSet,
            rSh.GetView().GetViewFrame()->GetFrame().GetFrameInterface(), RID_SVXDLG_CHARMAP );
        if( RET_OK == pMap->Execute() )
        {
            SFX_ITEMSET_ARG( pMap->GetOutputItemSet(), pItem, SfxInt32Item, SID_ATTR_CHAR, sal_False );
            if ( pItem )
                m_pDelimEdt->SetText( rtl::OUString(pItem->GetValue()) );
        }

        delete pMap;
    }
    return 0;
}

IMPL_LINK( SwSortDlg, CheckHdl, CheckBox *, pCheck )
{
    if( pCheck == ( CheckBox* ) m_pRowRB)
    {
        m_pColLbl->SetText(aColTxt);
        m_pColEdt1->SetMax(nY);
        m_pColEdt2->SetMax(nY);
        m_pColEdt3->SetMax(nY);

        m_pColEdt1->SetAccessibleName(aColTxt);
        m_pColEdt2->SetAccessibleName(aColTxt);
        m_pColEdt3->SetAccessibleName(aColTxt);
    }
    else if( pCheck == ( CheckBox* ) m_pColumnRB)
    {
        m_pColLbl->SetText(aRowTxt);
        m_pColEdt1->SetMax(nX);
        m_pColEdt2->SetMax(nX);
        m_pColEdt3->SetMax(nX);

        m_pColEdt1->SetAccessibleName(aRowTxt);
        m_pColEdt2->SetAccessibleName(aRowTxt);
        m_pColEdt3->SetAccessibleName(aRowTxt);
    }
    else if(!m_pKeyCB1->IsChecked() &&
                !m_pKeyCB2->IsChecked() &&
                    !m_pKeyCB3->IsChecked())
        pCheck->Check(sal_True);
    return 0;
}

IMPL_LINK( SwSortDlg, LanguageHdl, ListBox*, pLBox )
{
    Sequence < OUString > aSeq( GetAppCollator().listCollatorAlgorithms(
                LanguageTag( m_pLangLB->GetSelectLanguage()).getLocale() ));

    if( !pColRes )
        pColRes = new CollatorResource();

    const sal_uInt16 nLstBoxCnt = 3;
    ListBox* aLstArr[ nLstBoxCnt ] = { m_pTypDLB1, m_pTypDLB2, m_pTypDLB3 };
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
