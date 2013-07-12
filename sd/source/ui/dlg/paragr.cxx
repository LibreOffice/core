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

#include <vcl/field.hxx>

#include <svl/cjkoptions.hxx>
#include <svl/eitem.hxx>
#include <svl/intitem.hxx>

#include <editeng/editdata.hxx>
#include <svx/dialogs.hrc>
#include <editeng/eeitem.hxx>
#include <svx/flagsdef.hxx>

#include <editeng/outliner.hxx>
#include "paragr.hxx"
#include "sdresid.hxx"
#include "glob.hrc"
#include "sdattr.hrc"

class SdParagraphNumTabPage : public SfxTabPage
{
public:
    SdParagraphNumTabPage(Window* pParent, const SfxItemSet& rSet );
    ~SdParagraphNumTabPage();

    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rSet );
    static sal_uInt16*  GetRanges();

    virtual sal_Bool    FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );

private:
    TriStateBox*     m_pNewStartCB;
    TriStateBox*     m_pNewStartNumberCB;
    NumericField*    m_pNewStartNF;
    bool             mbModified;

    DECL_LINK( ImplNewStartHdl, void* );
};

SdParagraphNumTabPage::SdParagraphNumTabPage(Window* pParent, const SfxItemSet& rAttr )
                      : SfxTabPage(pParent,
                                   "DrawParaNumbering",
                                   "modules/sdraw/ui/paranumberingtab.ui",
                                   rAttr),
                        mbModified(false)
{
    get(m_pNewStartCB,"checkbuttonCB_NEW_START");
    get(m_pNewStartNumberCB,"checkbuttonCB_NUMBER_NEW_START");
    get(m_pNewStartNF,"spinbuttonNF_NEW_START");

    m_pNewStartCB->SetClickHdl(LINK(this, SdParagraphNumTabPage, ImplNewStartHdl));
    m_pNewStartNumberCB->SetClickHdl(LINK(this, SdParagraphNumTabPage, ImplNewStartHdl));
}

SdParagraphNumTabPage::~SdParagraphNumTabPage()
{
}

SfxTabPage* SdParagraphNumTabPage::Create(Window *pParent, const SfxItemSet & rAttrSet)
{
    return new SdParagraphNumTabPage( pParent, rAttrSet );
}

sal_uInt16* SdParagraphNumTabPage::GetRanges()
{
    static sal_uInt16 aRange[] =
    {
        ATTR_PARANUMBERING_START, ATTR_PARANUMBERING_END,
        0
    };

    return aRange;
}

sal_Bool SdParagraphNumTabPage::FillItemSet( SfxItemSet& rSet )
{
    if(m_pNewStartCB->GetState() != m_pNewStartCB->GetSavedValue() ||
        m_pNewStartNumberCB->GetState() != m_pNewStartNumberCB->GetSavedValue()||
       m_pNewStartNF->GetText() != m_pNewStartNF->GetSavedValue())
    {
        mbModified = true;
        sal_Bool bNewStartChecked = STATE_CHECK == m_pNewStartCB->GetState();
        sal_Bool bNumberNewStartChecked = STATE_CHECK == m_pNewStartNumberCB->GetState();
        rSet.Put(SfxBoolItem(ATTR_NUMBER_NEWSTART, bNewStartChecked));

        const sal_Int16 nStartAt = (sal_Int16)m_pNewStartNF->GetValue();
        rSet.Put(SfxInt16Item(ATTR_NUMBER_NEWSTART_AT, bNumberNewStartChecked && bNewStartChecked ? nStartAt : -1));
    }

    return mbModified;
}

void SdParagraphNumTabPage::Reset( const SfxItemSet& rSet )
{
    SfxItemState eItemState = rSet.GetItemState( ATTR_NUMBER_NEWSTART );
    if(eItemState > SFX_ITEM_AVAILABLE )
    {
        const SfxBoolItem& rStart = (const SfxBoolItem&)rSet.Get(ATTR_NUMBER_NEWSTART);
        m_pNewStartCB->SetState( rStart.GetValue() ? STATE_CHECK : STATE_NOCHECK );
        m_pNewStartCB->EnableTriState(sal_False);
    }
    else
    {
        m_pNewStartCB->SetState(STATE_DONTKNOW);
        m_pNewStartCB->Disable();
    }
    m_pNewStartCB->SaveValue();

    eItemState = rSet.GetItemState( ATTR_NUMBER_NEWSTART_AT);
    if( eItemState > SFX_ITEM_AVAILABLE )
    {
        sal_Int16 nNewStart = ((const SfxInt16Item&)rSet.Get(ATTR_NUMBER_NEWSTART_AT)).GetValue();
        m_pNewStartNumberCB->Check(-1 != nNewStart);
        if(-1 == nNewStart)
            nNewStart = 1;

        m_pNewStartNF->SetValue(nNewStart);
        m_pNewStartNumberCB->EnableTriState(sal_False);
    }
    else
    {
        m_pNewStartCB->SetState(STATE_DONTKNOW);
    }
    ImplNewStartHdl(m_pNewStartCB);
    m_pNewStartNF->SaveValue();
    m_pNewStartNumberCB->SaveValue();
    mbModified = sal_False;
}

IMPL_LINK_NOARG(SdParagraphNumTabPage, ImplNewStartHdl)
{
    sal_Bool bEnable = m_pNewStartCB->IsChecked();
    m_pNewStartNumberCB->Enable(bEnable);
    m_pNewStartNF->Enable(bEnable && m_pNewStartNumberCB->IsChecked());
    return 0;
}

SdParagraphDlg::SdParagraphDlg( Window* pParent, const SfxItemSet* pAttr )
               : SfxTabDialog( pParent,
                               "DrawParagraphPropertiesDialog",
                               "modules/sdraw/ui/drawparadialog.ui",
                               pAttr )
{
    m_nParaStd = AddTabPage( "labelTP_PARA_STD", RID_SVXPAGE_STD_PARAGRAPH);

    SvtCJKOptions aCJKOptions;
    if( aCJKOptions.IsAsianTypographyEnabled() )
        AddTabPage( "labelTP_PARA_ASIAN", RID_SVXPAGE_PARA_ASIAN);
    else
        RemoveTabPage( "labelTP_PARA_ASIAN" );

    AddTabPage( "labelTP_PARA_ALIGN", RID_SVXPAGE_ALIGN_PARAGRAPH);

    static const sal_Bool bShowParaNumbering = ( getenv( "SD_SHOW_NUMBERING_PAGE" ) != NULL );
    if( bShowParaNumbering )
        AddTabPage( "labelNUMBERING", SdParagraphNumTabPage::Create, SdParagraphNumTabPage::GetRanges );
    else
        RemoveTabPage( "labelNUMBERING" );

      AddTabPage("labelTP_TABULATOR", RID_SVXPAGE_TABULATOR);
}

void SdParagraphDlg::PageCreated( sal_uInt16 nId, SfxTabPage &rPage )
{
    SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
    if (m_nParaStd == nId)
    {
        aSet.Put(SfxUInt32Item(SID_SVXSTDPARAGRAPHTABPAGE_ABSLINEDIST, MM50/2));
        rPage.PageCreated(aSet);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
