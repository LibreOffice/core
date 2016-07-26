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

#include <cstdint>

#include "srtdlg.hxx"

#include <vcl/lstbox.hxx>
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
#include <memory>

static bool bCheck1 = true;
static bool bCheck2 = false;
static bool bCheck3 = false;

static sal_uInt16 nCol1 = 1;
static sal_uInt16 nCol2 = 1;
static sal_uInt16 nCol3 = 1;

static sal_uInt16 nType1 = 0;
static sal_uInt16 nType2 = 0;
static sal_uInt16 nType3 = 0;

static sal_uInt16 nLang = LANGUAGE_NONE;

static bool   bAsc1  = true;
static bool   bAsc2  = true;
static bool   bAsc3  = true;
static bool   bCol   = false;
static bool   bCsSens= false;

static sal_Unicode    cDeli  = '\t';

using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;

static void lcl_ClearLstBoxAndDelUserData( ListBox& rLstBox )
{
    const sal_Int32 nEnd = rLstBox.GetEntryCount();
    for( sal_Int32 n = 0; n < nEnd; ++n )
    {
        delete static_cast<OUString *>(rLstBox.GetEntryData( n ));
    }
    rLstBox.Clear();
}

// determine lines and columns for table selection
static bool lcl_GetSelTable( SwWrtShell &rSh, sal_uInt16& rX, sal_uInt16& rY )
{
    const SwTableNode* pTableNd = rSh.IsCursorInTable();
    if( !pTableNd )
        return false;

    FndBox_ aFndBox( nullptr, nullptr );

    // look for all boxes / lines
    {
        SwSelBoxes aSelBoxes;
        ::GetTableSel( rSh, aSelBoxes );
        FndPara aPara( aSelBoxes, &aFndBox );
        const SwTable& rTable = pTableNd->GetTable();
        ForEach_FndLineCopyCol( (SwTableLines&)rTable.GetTabLines(), &aPara );
    }
    rX = aFndBox.GetLines().size();
    if( !rX )
        return false;

    rY = aFndBox.GetLines().front()->GetBoxes().size();
    return true;
}

// init list
SwSortDlg::SwSortDlg(vcl::Window* pParent, SwWrtShell &rShell)
    : SvxStandardDialog(pParent, "SortDialog", "modules/swriter/ui/sortdialog.ui")
    , aColText(SW_RES(STR_COL))
    , aRowText(SW_RES(STR_ROW))
    , aNumericText(SW_RES(STR_NUMERIC))
    , rSh(rShell)
    , pColRes(nullptr)
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
        m_pColLbl->SetText(bCol ? aRowText : aColText);
        m_pRowRB->Check(!bCol);
        m_pDelimTabRB->Enable(false);
        m_pDelimFreeRB->Enable(false);
        m_pDelimEdt->Enable(false);
    }
    else
    {
        m_pColumnRB->Enable(false);
        m_pRowRB->Check();
        m_pColLbl->SetText(aColText);
    }

    // initialise
    Link<Button*,void> aLk = LINK(this,SwSortDlg, CheckHdl);
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

    m_pLangLB->SetLanguageList( SvxLanguageListFlags::ALL | SvxLanguageListFlags::ONLY_KNOWN, true );
    m_pLangLB->SelectLanguage( nLang );

    LanguageHdl( nullptr );
    m_pLangLB->SetSelectHdl( LINK( this, SwSortDlg, LanguageListBoxHdl ));

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
        m_pDelimEdt->SetText(OUString(cDeli));
        m_pDelimFreeRB->Check();
        DelimHdl(m_pDelimFreeRB);
    }
    else
        DelimHdl(m_pDelimTabRB);

    if( ::lcl_GetSelTable( rSh, nX, nY) )
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
    disposeOnce();
}

void SwSortDlg::dispose()
{
    ::lcl_ClearLstBoxAndDelUserData(*m_pTypDLB1);
    ::lcl_ClearLstBoxAndDelUserData(*m_pTypDLB2);
    ::lcl_ClearLstBoxAndDelUserData(*m_pTypDLB3);
    delete pColRes;
    m_pColLbl.clear();
    m_pTypLbl.clear();
    m_pKeyCB1.clear();
    m_pColEdt1.clear();
    m_pTypDLB1.clear();
    m_pSortUp1RB.clear();
    m_pSortDn1RB.clear();
    m_pKeyCB2.clear();
    m_pColEdt2.clear();
    m_pTypDLB2.clear();
    m_pSortUp2RB.clear();
    m_pSortDn2RB.clear();
    m_pKeyCB3.clear();
    m_pColEdt3.clear();
    m_pTypDLB3.clear();
    m_pSortUp3RB.clear();
    m_pSortDn3RB.clear();
    m_pColumnRB.clear();
    m_pRowRB.clear();
    m_pDelimTabRB.clear();
    m_pDelimFreeRB.clear();
    m_pDelimEdt.clear();
    m_pDelimPB.clear();
    m_pLangLB.clear();
    m_pCaseCB.clear();
    SvxStandardDialog::dispose();
}

sal_Unicode SwSortDlg::GetDelimChar() const
{
    sal_Unicode cRet = '\t';
    if( !m_pDelimTabRB->IsChecked() )
    {
        OUString aTmp( m_pDelimEdt->GetText() );
        if( !aTmp.isEmpty() )
            cRet = aTmp[0];
    }
    return cRet;
}

// pass on to the Core
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
        OUString sEntry( m_pTypDLB1->GetSelectEntry() );
        if( sEntry == aNumericText )
            sEntry.clear();
        else if( nullptr != (pUserData = m_pTypDLB1->GetSelectEntryData()) )
            sEntry = *static_cast<OUString*>(pUserData);

        SwSortKey *pKey = new SwSortKey( nCol1, sEntry,
                                    bAsc1 ? SRT_ASCENDING : SRT_DESCENDING );
        aOptions.aKeys.push_back( pKey );
    }

    if( bCheck2 )
    {
        OUString sEntry( m_pTypDLB2->GetSelectEntry() );
        if( sEntry == aNumericText )
            sEntry.clear();
        else if( nullptr != (pUserData = m_pTypDLB2->GetSelectEntryData()) )
            sEntry = *static_cast<OUString*>(pUserData);

        SwSortKey *pKey = new SwSortKey( nCol2, sEntry,
                                    bAsc2 ? SRT_ASCENDING : SRT_DESCENDING );
        aOptions.aKeys.push_back( pKey );
    }

    if( bCheck3 )
    {
        OUString sEntry( m_pTypDLB3->GetSelectEntry() );
        if( sEntry == aNumericText )
            sEntry.clear();
        else if( nullptr != (pUserData = m_pTypDLB3->GetSelectEntryData()) )
            sEntry = *static_cast<OUString*>(pUserData);

        SwSortKey *pKey = new SwSortKey( nCol3, sEntry,
                                    bAsc3 ? SRT_ASCENDING : SRT_DESCENDING );
        aOptions.aKeys.push_back( pKey );
    }

    aOptions.eDirection =  bCol ? SRT_COLUMNS : SRT_ROWS;
    aOptions.cDeli = cDeli;
    aOptions.nLanguage = nLang;
    aOptions.bTable = rSh.IsTableMode();
    aOptions.bIgnoreCase = !bCsSens;

    bool bRet;
    {
        SwWait aWait( *rSh.GetView().GetDocShell(), true );
        rSh.StartAllAction();
        if( (bRet = rSh.Sort( aOptions )))
            rSh.SetModified();
        rSh.EndAllAction();
    }

    if( !bRet )
        ScopedVclPtrInstance<MessageDialog>( this->GetParent(), SW_RES(STR_SRTERR), VCL_MESSAGE_INFO)->Execute();
}

IMPL_LINK_TYPED( SwSortDlg, DelimHdl, Button*, pButton, void )
{
    bool bEnable = pButton == m_pDelimFreeRB && m_pDelimFreeRB->IsEnabled();
    m_pDelimEdt->Enable( bEnable );
    m_pDelimPB->Enable( bEnable );
}

IMPL_LINK_NOARG_TYPED(SwSortDlg, DelimCharHdl, Button*, void)
{
    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    if(pFact)
    {
        SfxAllItemSet aSet( rSh.GetAttrPool() );
        aSet.Put( SfxInt32Item( SID_ATTR_CHAR, GetDelimChar() ) );
        std::unique_ptr<SfxAbstractDialog> pMap(pFact->CreateSfxDialog( m_pDelimPB, aSet,
            rSh.GetView().GetViewFrame()->GetFrame().GetFrameInterface(), RID_SVXDLG_CHARMAP ));
        if( RET_OK == pMap->Execute() )
        {
            const SfxInt32Item* pItem = SfxItemSet::GetItem<SfxInt32Item>(pMap->GetOutputItemSet(), SID_ATTR_CHAR, false);
            if ( pItem )
                m_pDelimEdt->SetText( OUString(sal_Unicode(pItem->GetValue())) );
        }
    }
}

IMPL_LINK_TYPED( SwSortDlg, CheckHdl, Button*, pControl, void )
{
    if( pControl == m_pRowRB.get())
    {
        m_pColLbl->SetText(aColText);
        m_pColEdt1->SetMax(nY);
        m_pColEdt2->SetMax(nY);
        m_pColEdt3->SetMax(nY);

        m_pColEdt1->SetAccessibleName(aColText);
        m_pColEdt2->SetAccessibleName(aColText);
        m_pColEdt3->SetAccessibleName(aColText);
    }
    else if( pControl == m_pColumnRB.get())
    {
        m_pColLbl->SetText(aRowText);
        m_pColEdt1->SetMax(nX);
        m_pColEdt2->SetMax(nX);
        m_pColEdt3->SetMax(nX);

        m_pColEdt1->SetAccessibleName(aRowText);
        m_pColEdt2->SetAccessibleName(aRowText);
        m_pColEdt3->SetAccessibleName(aRowText);
    }
    else if(!m_pKeyCB1->IsChecked() &&
                !m_pKeyCB2->IsChecked() &&
                    !m_pKeyCB3->IsChecked())
        static_cast<CheckBox *>(pControl)->Check();
}

IMPL_LINK_TYPED( SwSortDlg, LanguageListBoxHdl, ListBox&, rLBox, void )
{
    LanguageHdl(&rLBox);
}

void SwSortDlg::LanguageHdl(ListBox* pLBox)
{
    Sequence < OUString > aSeq( GetAppCollator().listCollatorAlgorithms(
                LanguageTag( m_pLangLB->GetSelectLanguage()).getLocale() ));

    if( !pColRes )
        pColRes = new CollatorResource();

    const int nLstBoxCnt = 3;
    ListBox* aLstArr[ nLstBoxCnt ] = { m_pTypDLB1, m_pTypDLB2, m_pTypDLB3 };
    sal_uInt16* aTypeArr[ nLstBoxCnt ] = { &nType1, &nType2, &nType3 };
    OUString aOldStrArr[ nLstBoxCnt ];

    for( int n = 0; n < nLstBoxCnt; ++n )
    {
        ListBox* pL = aLstArr[ n ];
        void* pUserData = pL->GetSelectEntryData();
        if (pUserData)
            aOldStrArr[ n ] = *static_cast<OUString*>(pUserData);
        ::lcl_ClearLstBoxAndDelUserData( *pL );
    }

    OUString sAlg, sUINm;
    const sal_Int32 nEnd = aSeq.getLength();
    for( sal_Int32 nCnt = 0; nCnt <= nEnd; ++nCnt )
    {
        if( nCnt < nEnd )
            sUINm = pColRes->GetTranslation( sAlg = aSeq[ nCnt ] );
        else
            sUINm = sAlg = aNumericText;

        for( int n = 0; n < nLstBoxCnt; ++n )
        {
            ListBox* pL = aLstArr[ n ];
            const sal_Int32 nInsPos = pL->InsertEntry( sUINm );
            pL->SetEntryData( nInsPos, new OUString( sAlg ));
            if( pLBox && sAlg == aOldStrArr[ n ] )
                pL->SelectEntryPos( nInsPos );
        }
    }

    for( int n = 0; n < nLstBoxCnt; ++n )
    {
        ListBox* pL = aLstArr[ n ];
        if( !pLBox )
            pL->SelectEntryPos( *aTypeArr[n] );
        else if( LISTBOX_ENTRY_NOTFOUND == pL->GetSelectEntryPos() )
            pL->SelectEntryPos( 0 );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
