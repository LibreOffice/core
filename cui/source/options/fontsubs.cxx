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

#include "sal/config.h"

#include <officecfg/Office/Common.hxx>
#include <svtools/ctrltool.hxx>
#include <svtools/svlbitm.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <svtools/fontsubstconfig.hxx>
#include "fontsubs.hxx"
#include <dialmgr.hxx>
#include "helpid.hrc"
#include <cuires.hrc>

/*********************************************************************/
/*                                                                   */
/*  TabPage font replacement                                         */
/*                                                                   */
/*********************************************************************/

SvxFontSubstTabPage::SvxFontSubstTabPage( vcl::Window* pParent,
                                const SfxItemSet& rSet )
    : SfxTabPage(pParent, "OptFontsPage", "cui/ui/optfontspage.ui", &rSet)
    , pConfig(new SvtFontSubstConfig)
    , pCheckButtonData(nullptr)
{
    get(m_pUseTableCB, "usetable");
    get(m_pReplacements, "replacements");
    get(m_pFont1CB, "font1");
    get(m_pFont2CB, "font2");
    m_pFont1CB->SetStyle(m_pFont1CB->GetStyle() | WB_SORT);
    m_pFont2CB->SetStyle(m_pFont2CB->GetStyle() | WB_SORT);
    get(m_pApply, "apply");
    get(m_pDelete, "delete");
    get(m_pFontNameLB, "fontname");
    m_sAutomatic = m_pFontNameLB->GetEntry(0);
    assert(!m_sAutomatic.isEmpty());
    get(m_pNonPropFontsOnlyCB, "nonpropfontonly");
    get(m_pFontHeightLB, "fontheight");

    SvSimpleTableContainer *pCheckLBContainer = get<SvSimpleTableContainer>("checklb");
    Size aControlSize(248, 75);
    aControlSize = LogicToPixel(aControlSize, MAP_APPFONT);
    pCheckLBContainer->set_width_request(aControlSize.Width());
    pCheckLBContainer->set_height_request(aControlSize.Height());

    m_pCheckLB = VclPtr<SvxFontSubstCheckListBox>::Create(*pCheckLBContainer, 0);
    m_pCheckLB->SetHelpId(HID_OFA_FONT_SUBST_CLB);

    m_pCheckLB->SetStyle(m_pCheckLB->GetStyle()|WB_HSCROLL|WB_VSCROLL);
    m_pCheckLB->SetSelectionMode(MULTIPLE_SELECTION);
    m_pCheckLB->SortByCol(2);
    long aStaticTabs[] = { 4, 0, 0, 0, 0 };
    m_pCheckLB->SvSimpleTable::SetTabs(&aStaticTabs[0]);

    OUString sHeader1(get<FixedText>("always")->GetText());
    OUString sHeader2(get<FixedText>("screenonly")->GetText());
    OUStringBuffer sHeader;
    sHeader.append(sHeader1).append("\t").append(sHeader2)
        .append("\t ").append(get<FixedText>("font")->GetText())
        .append("\t ").append(get<FixedText>("replacewith")->GetText());
    m_pCheckLB->InsertHeaderEntry(sHeader.makeStringAndClear());

    HeaderBar &rBar = m_pCheckLB->GetTheHeaderBar();
    HeaderBarItemBits nBits = rBar.GetItemBits(1) | HeaderBarItemBits::FIXEDPOS | HeaderBarItemBits::FIXED;
    nBits &= ~HeaderBarItemBits::CLICKABLE;
    rBar.SetItemBits(1, nBits);
    rBar.SetItemBits(2, nBits);

    m_pCheckLB->setColSizes();

    aTextColor = m_pCheckLB->GetTextColor();
    Link<Edit&,void> aLink(LINK(this, SvxFontSubstTabPage, SelectEditHdl));
    Link<ComboBox&,void> aLink2(LINK(this, SvxFontSubstTabPage, SelectComboBoxHdl));
    Link<Button*,void> aClickLink(LINK(this, SvxFontSubstTabPage, ClickHdl));

    m_pCheckLB->SetSelectHdl(LINK(this, SvxFontSubstTabPage, TreeListBoxSelectHdl));
    m_pUseTableCB->SetClickHdl(aClickLink);
    m_pFont1CB->SetSelectHdl(aLink2);
    m_pFont1CB->SetModifyHdl(aLink);
    m_pFont2CB->SetSelectHdl(aLink2);
    m_pFont2CB->SetModifyHdl(aLink);
    m_pApply->SetClickHdl(aClickLink);
    m_pDelete->SetClickHdl(aClickLink);

    m_pNonPropFontsOnlyCB->SetClickHdl(LINK(this, SvxFontSubstTabPage, NonPropFontsHdl));

    sal_uInt16 nHeight;
    for(nHeight = 6; nHeight <= 16; nHeight++)
        m_pFontHeightLB->InsertEntry(OUString::number(nHeight));
    for(nHeight = 18; nHeight <= 28; nHeight+= 2)
        m_pFontHeightLB->InsertEntry(OUString::number(nHeight));
    for(nHeight = 32; nHeight <= 48; nHeight+= 4)
        m_pFontHeightLB->InsertEntry(OUString::number(nHeight));
    for(nHeight = 54; nHeight <= 72; nHeight+= 6)
        m_pFontHeightLB->InsertEntry(OUString::number(nHeight));
    for(nHeight = 80; nHeight <= 96; nHeight+= 8)
        m_pFontHeightLB->InsertEntry(OUString::number(nHeight));
}

SvTreeListEntry* SvxFontSubstTabPage::CreateEntry(OUString& rFont1, OUString& rFont2)
{
    SvTreeListEntry* pEntry = new SvTreeListEntry;

    if( !pCheckButtonData )
        pCheckButtonData = new SvLBoxButtonData( m_pCheckLB );

    pEntry->AddItem(std::unique_ptr<SvLBoxContextBmp>(new SvLBoxContextBmp(
            pEntry, 0, Image(), Image(), false))); // otherwise boom!

    pEntry->AddItem(std::unique_ptr<SvLBoxButton>(new SvLBoxButton(pEntry,
           SvLBoxButtonKind_enabledCheckbox, 0, pCheckButtonData)));
    pEntry->AddItem(std::unique_ptr<SvLBoxButton>(new SvLBoxButton(pEntry,
           SvLBoxButtonKind_enabledCheckbox, 0, pCheckButtonData)));

    pEntry->AddItem(std::unique_ptr<SvLBoxString>(new SvLBoxString(
                    pEntry, 0, rFont1)));
    pEntry->AddItem(std::unique_ptr<SvLBoxString>(new SvLBoxString(
                    pEntry, 0, rFont2)));

    return pEntry;
}

SvxFontSubstTabPage::~SvxFontSubstTabPage()
{
    disposeOnce();
}

void SvxFontSubstTabPage::dispose()
{
    delete pCheckButtonData;
    pCheckButtonData = nullptr;
    delete pConfig;
    pConfig = nullptr;
    m_pCheckLB.disposeAndClear();
    m_pUseTableCB.clear();
    m_pReplacements.clear();
    m_pFont1CB.clear();
    m_pFont2CB.clear();
    m_pApply.clear();
    m_pDelete.clear();
    m_pFontNameLB.clear();
    m_pNonPropFontsOnlyCB.clear();
    m_pFontHeightLB.clear();
    SfxTabPage::dispose();
}

VclPtr<SfxTabPage> SvxFontSubstTabPage::Create( vcl::Window* pParent,
                                                const SfxItemSet* rAttrSet)
{
    return VclPtr<SvxFontSubstTabPage>::Create(pParent, *rAttrSet);
}

bool  SvxFontSubstTabPage::FillItemSet( SfxItemSet* )
{
    pConfig->ClearSubstitutions();// remove all entries

    pConfig->Enable(m_pUseTableCB->IsChecked());

    SvTreeListEntry* pEntry = m_pCheckLB->First();

    while (pEntry)
    {
        SubstitutionStruct aAdd;
        aAdd.sFont = SvTabListBox::GetEntryText(pEntry, 0);
        aAdd.sReplaceBy = SvTabListBox::GetEntryText(pEntry, 1);
        aAdd.bReplaceAlways = SvxFontSubstCheckListBox::IsChecked(pEntry);
        aAdd.bReplaceOnScreenOnly = SvxFontSubstCheckListBox::IsChecked(pEntry, 1);
        pConfig->AddSubstitution(aAdd);
        pEntry = m_pCheckLB->Next(pEntry);
    }
    if(pConfig->IsModified())
        pConfig->Commit();
    pConfig->Apply();
    std::shared_ptr< comphelper::ConfigurationChanges > batch(
        comphelper::ConfigurationChanges::create());
    if(m_pFontHeightLB->IsValueChangedFromSaved())
        officecfg::Office::Common::Font::SourceViewFont::FontHeight::set(
            static_cast< sal_Int16 >(m_pFontHeightLB->GetSelectEntry().toInt32()),
            batch);
    if(m_pNonPropFontsOnlyCB->IsValueChangedFromSaved())
        officecfg::Office::Common::Font::SourceViewFont::
            NonProportionalFontsOnly::set(
                m_pNonPropFontsOnlyCB->IsChecked(), batch);
    //font name changes cannot be detected by saved values
    OUString sFontName;
    if(m_pFontNameLB->GetSelectEntryPos())
        sFontName = m_pFontNameLB->GetSelectEntry();
    officecfg::Office::Common::Font::SourceViewFont::FontName::set(
        boost::optional< OUString >(sFontName), batch);
    batch->commit();

    return false;
}

void  SvxFontSubstTabPage::Reset( const SfxItemSet* )
{
    m_pCheckLB->SetUpdateMode(false);
    m_pCheckLB->Clear();

    FontList aFntLst( Application::GetDefaultDevice() );
    m_pFont1CB->Fill( &aFntLst );
    m_pFont2CB->Fill( &aFntLst );

    sal_Int32 nCount = pConfig->SubstitutionCount();
    if (nCount)
        m_pUseTableCB->Check(pConfig->IsEnabled());

    for (sal_Int32  i = 0; i < nCount; ++i)
    {
        const SubstitutionStruct* pSubs = pConfig->GetSubstitution(i);
        OUString aTmpStr1(pSubs->sFont);
        OUString aTmpStr2(pSubs->sReplaceBy);
        SvTreeListEntry* pEntry = CreateEntry(aTmpStr1, aTmpStr2);
        m_pCheckLB->Insert(pEntry);
        m_pCheckLB->CheckEntry(pEntry, 0, pSubs->bReplaceAlways);
        m_pCheckLB->CheckEntry(pEntry, 1, pSubs->bReplaceOnScreenOnly);
    }

    CheckEnable();
    m_pCheckLB->SetUpdateMode(true);

    //fill font name box first
    m_pNonPropFontsOnlyCB->Check(
        officecfg::Office::Common::Font::SourceViewFont::
        NonProportionalFontsOnly::get());
    NonPropFontsHdl(m_pNonPropFontsOnlyCB);
    OUString sFontName(
        officecfg::Office::Common::Font::SourceViewFont::FontName::get().
        get_value_or(OUString()));
    if(!sFontName.isEmpty())
        m_pFontNameLB->SelectEntry(sFontName);
    else
        m_pFontNameLB->SelectEntryPos(0);
    m_pFontHeightLB->SelectEntry(
        OUString::number(
            officecfg::Office::Common::Font::SourceViewFont::FontHeight::
            get()));
    m_pNonPropFontsOnlyCB->SaveValue();
    m_pFontHeightLB->SaveValue();
}

IMPL_LINK_TYPED(SvxFontSubstTabPage, ClickHdl, Button*, pButton, void)
{
    SelectHdl(pButton);
}
IMPL_LINK_TYPED(SvxFontSubstTabPage, TreeListBoxSelectHdl, SvTreeListBox*, pButton, void)
{
    SelectHdl(pButton);
}
IMPL_LINK_TYPED(SvxFontSubstTabPage, SelectComboBoxHdl, ComboBox&, rBox, void)
{
    SelectHdl(&rBox);
}
IMPL_LINK_TYPED(SvxFontSubstTabPage, SelectEditHdl, Edit&, rBox, void)
{
    SelectHdl(&rBox);
}
void SvxFontSubstTabPage::SelectHdl(vcl::Window* pWin)
{
    if (pWin == m_pApply || pWin == m_pDelete)
    {
        SvTreeListEntry* pEntry;
        // nCol is stupidly the nCol'th text column, not counted!
        // Therefore "0" as column.
        sal_uLong nPos = m_pCheckLB->GetEntryPos(m_pFont1CB->GetText(), 0);

        if (pWin == m_pApply)
        {
            if (nPos != 0xffffffff)
            {
                // change entry
                m_pCheckLB->SetEntryText(m_pFont2CB->GetText(), nPos, 1);
                pEntry = m_pCheckLB->GetEntry(nPos);
            }
            else
            {
                // new entry
                OUString sFont1 = m_pFont1CB->GetText();
                OUString sFont2 = m_pFont2CB->GetText();

                pEntry = CreateEntry(sFont1, sFont2);
                m_pCheckLB->Insert(pEntry);
            }
            m_pCheckLB->SelectAll(false);
            m_pCheckLB->Select(pEntry);
        }
        else if (pWin == m_pDelete)
        {
            if (nPos != 0xffffffff)
            {
                pEntry = m_pCheckLB->FirstSelected();
                while (pEntry)
                {
                    SvTreeListEntry* pDelEntry = pEntry;
                    pEntry = m_pCheckLB->NextSelected(pEntry);
                    m_pCheckLB->RemoveEntry(pDelEntry);
                }
            }
        }
    }

    if (pWin == m_pCheckLB)
    {
        SvTreeListEntry* pEntry = m_pCheckLB->FirstSelected();

        if (m_pCheckLB->NextSelected(pEntry) == nullptr)
        {
            m_pFont1CB->SetText(SvTabListBox::GetEntryText(pEntry, 0));
            m_pFont2CB->SetText(SvTabListBox::GetEntryText(pEntry, 1));
        }
    }

    if (pWin == m_pFont1CB)
    {
        sal_uLong nPos = m_pCheckLB->GetEntryPos(m_pFont1CB->GetText(), 0);

        if (nPos != 0xffffffff)
        {
            SvTreeListEntry* pEntry = m_pCheckLB->GetEntry(nPos);

            if (pEntry != m_pCheckLB->FirstSelected())
            {
                m_pCheckLB->SelectAll(false);
                m_pCheckLB->Select(pEntry);
            }
        }
    }

    CheckEnable();
}


IMPL_LINK_TYPED(SvxFontSubstTabPage, NonPropFontsHdl, Button*, pBox, void)
{
    OUString sFontName = m_pFontNameLB->GetSelectEntry();
    bool bNonPropOnly = static_cast<CheckBox*>(pBox)->IsChecked();
    m_pFontNameLB->Clear();
    FontList aFntLst( Application::GetDefaultDevice() );
    m_pFontNameLB->InsertEntry(m_sAutomatic);
    sal_uInt16 nFontCount = aFntLst.GetFontNameCount();
    for(sal_uInt16 nFont = 0; nFont < nFontCount; nFont++)
    {
        const vcl::FontInfo& rInfo = aFntLst.GetFontName( nFont );
        if(!bNonPropOnly || rInfo.GetPitch() == PITCH_FIXED)
            m_pFontNameLB->InsertEntry(rInfo.GetFamilyName());
    }
    m_pFontNameLB->SelectEntry(sFontName);
}

void SvxFontSubstTabPage::CheckEnable()
{
    bool bEnableAll = m_pUseTableCB->IsChecked();
    m_pReplacements->Enable(bEnableAll);
    if (bEnableAll)
    {
        bool bApply, bDelete;

        SvTreeListEntry* pEntry = m_pCheckLB->FirstSelected();

        OUString sEntry = m_pFont1CB->GetText();
        sEntry += "\t";
        sEntry += m_pFont2CB->GetText();

        // because of OS/2 optimization error (Bug #56267) a bit more intricate:
        if (m_pFont1CB->GetText().isEmpty() || m_pFont2CB->GetText().isEmpty())
            bApply = false;
        else if(m_pFont1CB->GetText() == m_pFont2CB->GetText())
            bApply = false;
        else if(m_pCheckLB->GetEntryPos(sEntry) != 0xffffffff)
            bApply = false;
        else if(pEntry != nullptr && m_pCheckLB->NextSelected(pEntry) != nullptr)
            bApply = false;
        else
            bApply = true;

        bDelete = pEntry != nullptr;

        m_pApply->Enable(bApply);
        m_pDelete->Enable(bDelete);
    }

    if (bEnableAll)
    {
        if (!m_pCheckLB->IsEnabled())
        {
            m_pCheckLB->EnableTable();
            m_pCheckLB->SetTextColor(aTextColor);
            m_pCheckLB->Invalidate();
            SelectHdl(m_pFont1CB);
        }
    }
    else
    {
        if (m_pCheckLB->IsEnabled())
        {
            m_pCheckLB->DisableTable();
            m_pCheckLB->SetTextColor(Color(COL_GRAY));
            m_pCheckLB->Invalidate();
            m_pCheckLB->SelectAll(false);
        }
    }
}

void SvxFontSubstCheckListBox::setColSizes()
{
    HeaderBar &rBar = GetTheHeaderBar();
    if (rBar.GetItemCount() < 4)
        return;
    long nW1 = rBar.GetTextWidth(rBar.GetItemText(3));
    long nW2 = rBar.GetTextWidth(rBar.GetItemText(4));
    long nMax = std::max( nW1, nW2 ) + 6; // width of the longest header + a little offset
    long nMin = rBar.LogicToPixel(Size(10, 0), MAP_APPFONT).Width();
    nMax = std::max( nMax, nMin );
    const long nDoubleMax = 2*nMax;
    const long nRest = GetSizePixel().Width() - nDoubleMax;
    long aStaticTabs[] = { 4, 0, 0, 0, 0 };
    aStaticTabs[2] = nMax;
    aStaticTabs[3] = nDoubleMax;
    aStaticTabs[4] = nDoubleMax + nRest/2;
    SvSimpleTable::SetTabs(aStaticTabs, MAP_PIXEL);
}

void SvxFontSubstCheckListBox::Resize()
{
    SvSimpleTable::Resize();
    setColSizes();
}

void SvxFontSubstCheckListBox::SetTabs()
{
    SvSimpleTable::SetTabs();
    SvLBoxTabFlags nAdjust = SvLBoxTabFlags::ADJUST_RIGHT|SvLBoxTabFlags::ADJUST_LEFT|SvLBoxTabFlags::ADJUST_CENTER|SvLBoxTabFlags::ADJUST_NUMERIC|SvLBoxTabFlags::FORCE;

    SvLBoxTab* pTab = aTabs[1];
    pTab->nFlags &= ~nAdjust;
    pTab->nFlags |= SvLBoxTabFlags::PUSHABLE|SvLBoxTabFlags::ADJUST_CENTER|SvLBoxTabFlags::FORCE;

    pTab = aTabs[2];
    pTab->nFlags &= ~nAdjust;
    pTab->nFlags |= SvLBoxTabFlags::PUSHABLE|SvLBoxTabFlags::ADJUST_CENTER|SvLBoxTabFlags::FORCE;
}

void    SvxFontSubstCheckListBox::KeyInput( const KeyEvent& rKEvt )
{
    if(!rKEvt.GetKeyCode().GetModifier() &&
        KEY_SPACE == rKEvt.GetKeyCode().GetCode())
    {
        sal_uLong nSelPos = GetModel()->GetAbsPos(GetCurEntry());
        sal_uInt16 nCol = GetCurrentTabPos() - 1;
        if ( nCol < 2 )
        {
            CheckEntryPos( nSelPos, nCol, !IsChecked( nSelPos, nCol ) );
            CallImplEventListeners( VCLEVENT_CHECKBOX_TOGGLE, static_cast<void*>(GetEntry( nSelPos )) );
        }
        else
        {
            sal_uInt16 nCheck = IsChecked(nSelPos, 1) ? 1 : 0;
            if(IsChecked(nSelPos))
                nCheck += 2;
            nCheck--;
            nCheck &= 3;
            CheckEntryPos(nSelPos, 1, 0 != (nCheck & 1));
            CheckEntryPos(nSelPos, 0, 0 != (nCheck & 2));
        }
    }
    else
        SvSimpleTable::KeyInput(rKEvt);
}

void SvxFontSubstCheckListBox::CheckEntryPos(sal_uLong nPos, sal_uInt16 nCol, bool bChecked)
{
    if ( nPos < GetEntryCount() )
        SetCheckButtonState(
            GetEntry(nPos),
            nCol,
            bChecked ? SvButtonState( SV_BUTTON_CHECKED ) :
                                       SvButtonState( SV_BUTTON_UNCHECKED ) );
}

void SvxFontSubstCheckListBox::CheckEntry(SvTreeListEntry* pEntry, sal_uInt16 nCol, bool bChecked)
{
    if ( pEntry )
        SetCheckButtonState(
            pEntry,
            nCol,
            bChecked ? SvButtonState( SV_BUTTON_CHECKED ) :
                                       SvButtonState( SV_BUTTON_UNCHECKED ) );
}

bool SvxFontSubstCheckListBox::IsChecked(sal_uLong nPos, sal_uInt16 nCol)
{
    return GetCheckButtonState( GetEntry(nPos), nCol ) == SV_BUTTON_CHECKED;
}

bool SvxFontSubstCheckListBox::IsChecked(SvTreeListEntry* pEntry, sal_uInt16 nCol)
{
    return GetCheckButtonState( pEntry, nCol ) == SV_BUTTON_CHECKED;
}

void SvxFontSubstCheckListBox::SetCheckButtonState( SvTreeListEntry* pEntry, sal_uInt16 nCol, SvButtonState eState)
{
    SvLBoxButton& rItem = static_cast<SvLBoxButton&>(pEntry->GetItem(nCol + 1));

    if (rItem.GetType() == SV_ITEM_ID_LBOXBUTTON)
    {
        switch( eState )
        {
            case SV_BUTTON_CHECKED:
                rItem.SetStateChecked();
                break;

            case SV_BUTTON_UNCHECKED:
                rItem.SetStateUnchecked();
                break;

            case SV_BUTTON_TRISTATE:
                rItem.SetStateTristate();
                break;
        }
        InvalidateEntry( pEntry );
    }
}

SvButtonState SvxFontSubstCheckListBox::GetCheckButtonState( SvTreeListEntry* pEntry, sal_uInt16 nCol )
{
    SvButtonState eState = SV_BUTTON_UNCHECKED;
    SvLBoxButton& rItem = static_cast<SvLBoxButton&>(pEntry->GetItem(nCol + 1));

    if (rItem.GetType() == SV_ITEM_ID_LBOXBUTTON)
    {
        SvItemStateFlags nButtonFlags = rItem.GetButtonFlags();
        eState = SvLBoxButtonData::ConvertToButtonState( nButtonFlags );
    }

    return eState;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
