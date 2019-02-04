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

#include <srtdlg.hxx>

#include <vcl/weld.hxx>
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
#include <app.hrc>
#include <strings.hrc>
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

static LanguageType nLang = LANGUAGE_NONE;

static bool   bAsc1  = true;
static bool   bAsc2  = true;
static bool   bAsc3  = true;
static bool   bCol   = false;
static bool   bCsSens= false;

static sal_Unicode    cDeli  = '\t';

using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;

// determine lines and columns for table selection
static bool lcl_GetSelTable( SwWrtShell const &rSh, sal_uInt16& rX, sal_uInt16& rY )
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
        ForEach_FndLineCopyCol( const_cast<SwTableLines&>(rTable.GetTabLines()), &aPara );
    }
    rX = aFndBox.GetLines().size();
    if( !rX )
        return false;

    rY = aFndBox.GetLines().front()->GetBoxes().size();
    return true;
}

// init list
SwSortDlg::SwSortDlg(weld::Window* pParent, SwWrtShell &rShell)
    : GenericDialogController(pParent, "modules/swriter/ui/sortdialog.ui", "SortDialog")
    , m_pParent(pParent)
    , m_xColLbl(m_xBuilder->weld_label("column"))
    , m_xKeyCB1(m_xBuilder->weld_check_button("key1"))
    , m_xColEdt1(m_xBuilder->weld_spin_button("colsb1"))
    , m_xTypDLB1(m_xBuilder->weld_combo_box("typelb1"))
    , m_xSortUp1RB(m_xBuilder->weld_radio_button("up1"))
    , m_xSortDn1RB(m_xBuilder->weld_radio_button("down1"))
    , m_xKeyCB2(m_xBuilder->weld_check_button("key2"))
    , m_xColEdt2(m_xBuilder->weld_spin_button("colsb2"))
    , m_xTypDLB2(m_xBuilder->weld_combo_box("typelb2"))
    , m_xSortUp2RB(m_xBuilder->weld_radio_button("up2"))
    , m_xSortDn2RB(m_xBuilder->weld_radio_button("down2"))
    , m_xKeyCB3(m_xBuilder->weld_check_button("key3"))
    , m_xColEdt3(m_xBuilder->weld_spin_button("colsb3"))
    , m_xTypDLB3(m_xBuilder->weld_combo_box("typelb3"))
    , m_xSortUp3RB(m_xBuilder->weld_radio_button("up3"))
    , m_xSortDn3RB(m_xBuilder->weld_radio_button("down3"))
    , m_xColumnRB(m_xBuilder->weld_radio_button("columns"))
    , m_xRowRB(m_xBuilder->weld_radio_button("rows"))
    , m_xDelimTabRB(m_xBuilder->weld_radio_button("tabs"))
    , m_xDelimFreeRB(m_xBuilder->weld_radio_button("character"))
    , m_xDelimEdt(m_xBuilder->weld_entry("separator"))
    , m_xDelimPB(m_xBuilder->weld_button("delimpb"))
    , m_xLangLB(new LanguageBox(m_xBuilder->weld_combo_box("langlb")))
    , m_xCaseCB(m_xBuilder->weld_check_button("matchcase"))
    , aColText(SwResId(STR_COL))
    , aRowText(SwResId(STR_ROW))
    , aNumericText(SwResId(STR_NUMERIC))
    , rSh(rShell)
    , nX(99)
    , nY(99)
{
    if(rSh.GetSelectionType() &
            (SelectionType::Table|SelectionType::TableCell) )
    {
        m_xColumnRB->set_active(bCol);
        m_xColLbl->set_label(bCol ? aRowText : aColText);
        m_xRowRB->set_active(!bCol);
        m_xDelimTabRB->set_sensitive(false);
        m_xDelimFreeRB->set_sensitive(false);
        m_xDelimEdt->set_sensitive(false);
    }
    else
    {
        m_xColumnRB->set_sensitive(false);
        m_xRowRB->set_active(true);
        m_xColLbl->set_label(aColText);
    }

    // Set accessible names here because text of m_xColLbl may be changed
    // by the if-else block above
    m_xColEdt1->set_accessible_name(m_xColLbl->get_label());
    m_xColEdt2->set_accessible_name(m_xColLbl->get_label());
    m_xColEdt3->set_accessible_name(m_xColLbl->get_label());

    // initialise
    Link<weld::ToggleButton&,void> aLk = LINK(this, SwSortDlg, CheckHdl);
    m_xKeyCB1->connect_toggled( aLk );
    m_xKeyCB2->connect_toggled( aLk );
    m_xKeyCB3->connect_toggled( aLk );
    m_xColumnRB->connect_toggled( aLk );
    m_xRowRB->connect_toggled( aLk );

    aLk = LINK(this, SwSortDlg, DelimHdl);
    m_xDelimFreeRB->connect_toggled(aLk);
    m_xDelimTabRB->connect_toggled(aLk);

    m_xDelimPB->connect_clicked( LINK( this, SwSortDlg, DelimCharHdl ));

    m_xKeyCB1->set_active(bCheck1);
    m_xKeyCB2->set_active(bCheck2);
    m_xKeyCB3->set_active(bCheck3);

    m_xColEdt1->set_value(nCol1);
    m_xColEdt2->set_value(nCol2);
    m_xColEdt3->set_value(nCol3);

    // first initialise the language, then select the
    if( LANGUAGE_NONE == nLang || LANGUAGE_DONTKNOW == nLang )
        nLang = GetAppLanguage();

    m_xLangLB->SetLanguageList( SvxLanguageListFlags::ALL | SvxLanguageListFlags::ONLY_KNOWN, true );
    m_xLangLB->set_active_id(nLang);

    LanguageHdl( nullptr );
    m_xLangLB->connect_changed( LINK( this, SwSortDlg, LanguageListBoxHdl ));

    m_xSortUp1RB->set_active(bAsc1);
    m_xSortDn1RB->set_active(!bAsc1);
    m_xSortUp2RB->set_active(bAsc2);
    m_xSortDn2RB->set_active(!bAsc2);
    m_xSortUp3RB->set_active(bAsc3);
    m_xSortDn3RB->set_active(!bAsc3);

    m_xCaseCB->set_active( bCsSens );

    m_xDelimTabRB->set_active(cDeli == '\t');
    if(!m_xDelimTabRB->get_active())
    {
        m_xDelimEdt->set_text(OUString(cDeli));
        m_xDelimFreeRB->set_active(true);
        DelimHdl(*m_xDelimFreeRB);
    }
    else
        DelimHdl(*m_xDelimTabRB);

    if( ::lcl_GetSelTable( rSh, nX, nY) )
    {
        sal_uInt16 nMax = m_xRowRB->get_active()? nY : nX;
        m_xColEdt1->set_max(nMax);
        m_xColEdt2->set_max(nMax);
        m_xColEdt3->set_max(nMax);
    }
}

sal_Unicode SwSortDlg::GetDelimChar() const
{
    sal_Unicode cRet = '\t';
    if( !m_xDelimTabRB->get_active() )
    {
        OUString aTmp(m_xDelimEdt->get_text());
        if( !aTmp.isEmpty() )
            cRet = aTmp[0];
    }
    return cRet;
}

short SwSortDlg::run()
{
    short nRet = GenericDialogController::run();
    if (nRet == RET_OK)
        Apply();
    return nRet;
}

// pass on to the Core
void SwSortDlg::Apply()
{
    // save all settings
    bCheck1 = m_xKeyCB1->get_active();
    bCheck2 = m_xKeyCB2->get_active();
    bCheck3 = m_xKeyCB3->get_active();

    nCol1 = m_xColEdt1->get_value();
    nCol2 = m_xColEdt2->get_value();
    nCol3 = m_xColEdt3->get_value();

    nType1 = m_xTypDLB1->get_active();
    nType2 = m_xTypDLB2->get_active();
    nType3 = m_xTypDLB3->get_active();

    bAsc1 = m_xSortUp1RB->get_active();
    bAsc2 = m_xSortUp2RB->get_active();
    bAsc3 = m_xSortUp3RB->get_active();
    bCol = m_xColumnRB->get_active();
    nLang = m_xLangLB->get_active_id();
    cDeli = GetDelimChar();
    bCsSens = m_xCaseCB->get_active();

    SwSortOptions aOptions;
    if( bCheck1 )
    {
        OUString sEntry( m_xTypDLB1->get_active_text() );
        if( sEntry == aNumericText )
            sEntry.clear();
        else if (!m_xTypDLB1->get_active_id().isEmpty())
            sEntry = m_xTypDLB1->get_active_id();

        aOptions.aKeys.push_back(
            std::make_unique<SwSortKey>( nCol1, sEntry,
                                    bAsc1 ? SRT_ASCENDING : SRT_DESCENDING ));
    }

    if( bCheck2 )
    {
        OUString sEntry( m_xTypDLB2->get_active_text() );
        if( sEntry == aNumericText )
            sEntry.clear();
        else if (!m_xTypDLB2->get_active_id().isEmpty())
            sEntry = m_xTypDLB2->get_active_id();

        aOptions.aKeys.push_back(
            std::make_unique<SwSortKey>( nCol2, sEntry,
                                    bAsc2 ? SRT_ASCENDING : SRT_DESCENDING ));
    }

    if( bCheck3 )
    {
        OUString sEntry( m_xTypDLB3->get_active_text() );
        if( sEntry == aNumericText )
            sEntry.clear();
        else if (!m_xTypDLB3->get_active_id().isEmpty())
            sEntry = m_xTypDLB3->get_active_id();

        aOptions.aKeys.push_back(
            std::make_unique<SwSortKey>( nCol3, sEntry,
                                    bAsc3 ? SRT_ASCENDING : SRT_DESCENDING ));
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
        bRet = rSh.Sort( aOptions );
        if( bRet )
            rSh.SetModified();
        rSh.EndAllAction();
    }

    if (!bRet)
    {
        std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(m_pParent,
                                                      VclMessageType::Info, VclButtonsType::Ok,
                                                      SwResId(STR_SRTERR)));
        xInfoBox->run();
    }
}

IMPL_LINK( SwSortDlg, DelimHdl, weld::ToggleButton&, rButton, void )
{
    bool bEnable = &rButton == m_xDelimFreeRB.get() && m_xDelimFreeRB->get_sensitive();
    m_xDelimEdt->set_sensitive( bEnable );
    m_xDelimPB->set_sensitive( bEnable );
}

IMPL_LINK_NOARG(SwSortDlg, DelimCharHdl, weld::Button&, void)
{
    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    SfxAllItemSet aSet( rSh.GetAttrPool() );
    aSet.Put( SfxInt32Item( SID_ATTR_CHAR, GetDelimChar() ) );
    ScopedVclPtr<SfxAbstractDialog> pMap(pFact->CreateCharMapDialog(m_xDialog.get(), aSet, false));
    if( RET_OK == pMap->Execute() )
    {
        const SfxInt32Item* pItem = SfxItemSet::GetItem<SfxInt32Item>(pMap->GetOutputItemSet(), SID_ATTR_CHAR, false);
        if ( pItem )
            m_xDelimEdt->set_text(OUString(sal_Unicode(pItem->GetValue())));
    }
}

IMPL_LINK( SwSortDlg, CheckHdl, weld::ToggleButton&, rControl, void )
{
    if (&rControl == m_xRowRB.get())
    {
        m_xColLbl->set_label(aColText);
        m_xColEdt1->set_max(nY);
        m_xColEdt2->set_max(nY);
        m_xColEdt3->set_max(nY);

        m_xColEdt1->set_accessible_name(aColText);
        m_xColEdt2->set_accessible_name(aColText);
        m_xColEdt3->set_accessible_name(aColText);
    }
    else if (&rControl == m_xColumnRB.get())
    {
        m_xColLbl->set_label(aRowText);
        m_xColEdt1->set_max(nX);
        m_xColEdt2->set_max(nX);
        m_xColEdt3->set_max(nX);

        m_xColEdt1->set_accessible_name(aRowText);
        m_xColEdt2->set_accessible_name(aRowText);
        m_xColEdt3->set_accessible_name(aRowText);
    }
    else if(!m_xKeyCB1->get_active() &&
            !m_xKeyCB2->get_active() &&
            !m_xKeyCB3->get_active())
    {
        rControl.set_active(true);
    }
}

IMPL_LINK( SwSortDlg, LanguageListBoxHdl, weld::ComboBox&, rLBox, void )
{
    LanguageHdl(&rLBox);
}

void SwSortDlg::LanguageHdl(weld::ComboBox const* pLBox)
{
    Sequence < OUString > aSeq( GetAppCollator().listCollatorAlgorithms(
                LanguageTag( m_xLangLB->get_active_id()).getLocale() ));

    if (!m_xColRes)
        m_xColRes.reset(new CollatorResource);

    const int nLstBoxCnt = 3;
    weld::ComboBox* aLstArr[ nLstBoxCnt ] = { m_xTypDLB1.get(), m_xTypDLB2.get(), m_xTypDLB3.get() };
    sal_uInt16* const aTypeArr[ nLstBoxCnt ] = { &nType1, &nType2, &nType3 };
    OUString aOldStrArr[ nLstBoxCnt ];

    for( int n = 0; n < nLstBoxCnt; ++n )
    {
        weld::ComboBox* pL = aLstArr[ n ];
        OUString sUserData = pL->get_active_id();
        if (!sUserData.isEmpty())
            aOldStrArr[ n ] = sUserData;
        pL->clear();
    }

    OUString sAlg, sUINm;
    const sal_Int32 nEnd = aSeq.getLength();
    for( sal_Int32 nCnt = 0; nCnt <= nEnd; ++nCnt )
    {
        if( nCnt < nEnd )
        {
            sAlg = aSeq[ nCnt ];
            sUINm = m_xColRes->GetTranslation( sAlg );
        }
        else
            sUINm = sAlg = aNumericText;

        for( int n = 0; n < nLstBoxCnt; ++n )
        {
            weld::ComboBox* pL = aLstArr[ n ];
            pL->append(sAlg, sUINm);
            if (pLBox && sAlg == aOldStrArr[n])
                pL->set_active_id(sAlg);
        }
    }

    for( int n = 0; n < nLstBoxCnt; ++n )
    {
        weld::ComboBox* pL = aLstArr[ n ];
        if( !pLBox )
            pL->set_active(*aTypeArr[n]);
        else if (pL->get_active() == -1)
            pL->set_active(0);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
