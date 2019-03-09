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

#include <svx/dialogs.hrc>
#include <svx/flagsdef.hxx>

#include <paragr.hxx>
#include <sdattr.hrc>

class SdParagraphNumTabPage : public SfxTabPage
{
public:
    SdParagraphNumTabPage(vcl::Window* pParent, const SfxItemSet& rSet );
    virtual ~SdParagraphNumTabPage() override;
    virtual void dispose() override;

    static VclPtr<SfxTabPage>  Create( TabPageParent pParent, const SfxItemSet* rSet );
    static const sal_uInt16*  GetRanges();

    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;

private:
    VclPtr<TriStateBox>     m_pNewStartCB;
    VclPtr<TriStateBox>     m_pNewStartNumberCB;
    VclPtr<NumericField>    m_pNewStartNF;
    bool             mbModified;

    DECL_LINK( ImplNewStartHdl, Button*, void );
};

SdParagraphNumTabPage::SdParagraphNumTabPage(vcl::Window* pParent, const SfxItemSet& rAttr )
                      : SfxTabPage(pParent,
                                   "DrawParaNumbering",
                                   "modules/sdraw/ui/paranumberingtab.ui",
                                   &rAttr),
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
    disposeOnce();
}

void SdParagraphNumTabPage::dispose()
{
    m_pNewStartCB.clear();
    m_pNewStartNumberCB.clear();
    m_pNewStartNF.clear();
    SfxTabPage::dispose();
}

VclPtr<SfxTabPage> SdParagraphNumTabPage::Create(TabPageParent pParent, const SfxItemSet * rAttrSet)
{
    return VclPtr<SdParagraphNumTabPage>::Create( pParent.pParent, *rAttrSet );
}

const sal_uInt16* SdParagraphNumTabPage::GetRanges()
{
    static const sal_uInt16 aRange[] =
    {
        ATTR_PARANUMBERING_START, ATTR_PARANUMBERING_END,
        0
    };

    return aRange;
}

bool SdParagraphNumTabPage::FillItemSet( SfxItemSet* rSet )
{
    if(m_pNewStartCB->IsValueChangedFromSaved() ||
       m_pNewStartNumberCB->IsValueChangedFromSaved()||
       m_pNewStartNF->IsValueChangedFromSaved())
    {
        mbModified = true;
        bool bNewStartChecked = TRISTATE_TRUE == m_pNewStartCB->GetState();
        bool bNumberNewStartChecked = TRISTATE_TRUE == m_pNewStartNumberCB->GetState();
        rSet->Put(SfxBoolItem(ATTR_NUMBER_NEWSTART, bNewStartChecked));

        const sal_Int16 nStartAt = static_cast<sal_Int16>(m_pNewStartNF->GetValue());
        rSet->Put(SfxInt16Item(ATTR_NUMBER_NEWSTART_AT, bNumberNewStartChecked && bNewStartChecked ? nStartAt : -1));
    }

    return mbModified;
}

void SdParagraphNumTabPage::Reset( const SfxItemSet* rSet )
{
    SfxItemState eItemState = rSet->GetItemState( ATTR_NUMBER_NEWSTART );
    if(eItemState > SfxItemState::DEFAULT )
    {
        const SfxBoolItem& rStart = static_cast<const SfxBoolItem&>(rSet->Get(ATTR_NUMBER_NEWSTART));
        m_pNewStartCB->SetState( rStart.GetValue() ? TRISTATE_TRUE : TRISTATE_FALSE );
        m_pNewStartCB->EnableTriState(false);
    }
    else
    {
        m_pNewStartCB->SetState(TRISTATE_INDET);
        m_pNewStartCB->Disable();
    }
    m_pNewStartCB->SaveValue();

    eItemState = rSet->GetItemState( ATTR_NUMBER_NEWSTART_AT);
    if( eItemState > SfxItemState::DEFAULT )
    {
        sal_Int16 nNewStart = static_cast<const SfxInt16Item&>(rSet->Get(ATTR_NUMBER_NEWSTART_AT)).GetValue();
        m_pNewStartNumberCB->Check(-1 != nNewStart);
        if(-1 == nNewStart)
            nNewStart = 1;

        m_pNewStartNF->SetValue(nNewStart);
        m_pNewStartNumberCB->EnableTriState(false);
    }
    else
    {
        m_pNewStartCB->SetState(TRISTATE_INDET);
    }
    ImplNewStartHdl(m_pNewStartCB);
    m_pNewStartNF->SaveValue();
    m_pNewStartNumberCB->SaveValue();
    mbModified = false;
}

IMPL_LINK_NOARG(SdParagraphNumTabPage, ImplNewStartHdl, Button*, void)
{
    bool bEnable = m_pNewStartCB->IsChecked();
    m_pNewStartNumberCB->Enable(bEnable);
    m_pNewStartNF->Enable(bEnable && m_pNewStartNumberCB->IsChecked());
}

SdParagraphDlg::SdParagraphDlg(weld::Window* pParent, const SfxItemSet* pAttr)
    : SfxTabDialogController(pParent, "modules/sdraw/ui/drawparadialog.ui",
                             "DrawParagraphPropertiesDialog", pAttr)
{
    AddTabPage( "labelTP_PARA_STD", RID_SVXPAGE_STD_PARAGRAPH);

    SvtCJKOptions aCJKOptions;
    if( aCJKOptions.IsAsianTypographyEnabled() )
        AddTabPage( "labelTP_PARA_ASIAN", RID_SVXPAGE_PARA_ASIAN);
    else
        RemoveTabPage( "labelTP_PARA_ASIAN" );

    AddTabPage( "labelTP_PARA_ALIGN", RID_SVXPAGE_ALIGN_PARAGRAPH);

    static const bool bShowParaNumbering = ( getenv( "SD_SHOW_NUMBERING_PAGE" ) != nullptr );
    if( bShowParaNumbering )
        AddTabPage( "labelNUMBERING", SdParagraphNumTabPage::Create, SdParagraphNumTabPage::GetRanges );
    else
        RemoveTabPage( "labelNUMBERING" );

    AddTabPage("labelTP_TABULATOR", RID_SVXPAGE_TABULATOR);
}

void SdParagraphDlg::PageCreated(const OString& rId, SfxTabPage &rPage)
{
    SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
    if (rId == "labelTP_PARA_STD")
    {
        aSet.Put(SfxUInt32Item(SID_SVXSTDPARAGRAPHTABPAGE_ABSLINEDIST, MM50/2));
        rPage.PageCreated(aSet);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
