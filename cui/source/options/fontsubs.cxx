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
#include <tools/shl.hxx>
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

SvxFontSubstTabPage::SvxFontSubstTabPage( Window* pParent,
                                const SfxItemSet& rSet )
    : SfxTabPage(pParent, "OptFontsPage", "cui/ui/optfontspage.ui", rSet)
    , pConfig(new SvtFontSubstConfig)
    , pCheckButtonData(0)
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

    SvxSimpleTableContainer *pCheckLBContainer = get<SvxSimpleTableContainer>("checklb");
    Size aControlSize(248, 75);
    aControlSize = LogicToPixel(aControlSize, MAP_APPFONT);
    pCheckLBContainer->set_width_request(aControlSize.Width());
    pCheckLBContainer->set_height_request(aControlSize.Height());

    m_pCheckLB = new SvxFontSubstCheckListBox(*pCheckLBContainer, 0);
    m_pCheckLB->SetHelpId(HID_OFA_FONT_SUBST_CLB);

    m_pCheckLB->SetStyle(m_pCheckLB->GetStyle()|WB_HSCROLL|WB_VSCROLL);
    m_pCheckLB->SetSelectionMode(MULTIPLE_SELECTION);
    m_pCheckLB->SortByCol(2);
    long aStaticTabs[] = { 4, 0, 0, 0, 0 };
    m_pCheckLB->SvxSimpleTable::SetTabs(&aStaticTabs[0]);

    OUString sHeader1(get<FixedText>("always")->GetText());
    OUString sHeader2(get<FixedText>("screenonly")->GetText());
    OUStringBuffer sHeader;
    sHeader.append(sHeader1).append("\t").append(sHeader2)
        .append("\t ").append(get<FixedText>("font")->GetText())
        .append("\t ").append(get<FixedText>("replacewith")->GetText());
    m_pCheckLB->InsertHeaderEntry(sHeader.makeStringAndClear());

    HeaderBar &rBar = m_pCheckLB->GetTheHeaderBar();
    HeaderBarItemBits nBits = rBar.GetItemBits(1) | HIB_FIXEDPOS | HIB_FIXED;
    nBits &= ~HIB_CLICKABLE;
    rBar.SetItemBits(1, nBits);
    rBar.SetItemBits(2, nBits);

    m_pCheckLB->setColSizes();

    aTextColor = m_pCheckLB->GetTextColor();
    Link aLink(LINK(this, SvxFontSubstTabPage, SelectHdl));

    m_pCheckLB->SetSelectHdl(aLink);
    m_pUseTableCB->SetClickHdl(aLink);
    m_pFont1CB->SetSelectHdl(aLink);
    m_pFont1CB->SetModifyHdl(aLink);
    m_pFont2CB->SetSelectHdl(aLink);
    m_pFont2CB->SetModifyHdl(aLink);
    m_pApply->SetClickHdl(aLink);
    m_pDelete->SetClickHdl(aLink);

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

SvTreeListEntry* SvxFontSubstTabPage::CreateEntry(String& rFont1, String& rFont2)
{
    SvTreeListEntry* pEntry = new SvTreeListEntry;

    if( !pCheckButtonData )
        pCheckButtonData = new SvLBoxButtonData( m_pCheckLB );

    pEntry->AddItem( new SvLBoxContextBmp( pEntry, 0, Image(), Image(), 0));    // Sonst Puff!

    pEntry->AddItem( new SvLBoxButton( pEntry,
                                           SvLBoxButtonKind_enabledCheckbox, 0,
                                           pCheckButtonData ) );
    pEntry->AddItem( new SvLBoxButton( pEntry,
                                           SvLBoxButtonKind_enabledCheckbox, 0,
                                           pCheckButtonData ) );

    pEntry->AddItem( new SvLBoxString( pEntry, 0, rFont1 ) );
    pEntry->AddItem( new SvLBoxString( pEntry, 0, rFont2 ) );

    return pEntry;
}

SvxFontSubstTabPage::~SvxFontSubstTabPage()
{
    delete pCheckButtonData;
    delete pConfig;
    delete m_pCheckLB;
}

SfxTabPage*  SvxFontSubstTabPage::Create( Window* pParent,
                                const SfxItemSet& rAttrSet)
{
    return new SvxFontSubstTabPage(pParent, rAttrSet);
}

sal_Bool  SvxFontSubstTabPage::FillItemSet( SfxItemSet& )
{
    pConfig->ClearSubstitutions();// remove all entries

    pConfig->Enable(m_pUseTableCB->IsChecked());

    SvTreeListEntry* pEntry = m_pCheckLB->First();

    while (pEntry)
    {
        SubstitutionStruct aAdd;
        aAdd.sFont = m_pCheckLB->GetEntryText(pEntry, 0);
        aAdd.sReplaceBy = m_pCheckLB->GetEntryText(pEntry, 1);
        aAdd.bReplaceAlways = m_pCheckLB->IsChecked(pEntry, 0);
        aAdd.bReplaceOnScreenOnly = m_pCheckLB->IsChecked(pEntry, 1);
        pConfig->AddSubstitution(aAdd);
        pEntry = m_pCheckLB->Next(pEntry);
    }
    if(pConfig->IsModified())
        pConfig->Commit();
    pConfig->Apply();
    boost::shared_ptr< comphelper::ConfigurationChanges > batch(
        comphelper::ConfigurationChanges::create());
    if(m_pFontHeightLB->GetSavedValue() != m_pFontHeightLB->GetSelectEntryPos())
        officecfg::Office::Common::Font::SourceViewFont::FontHeight::set(
            static_cast< sal_Int16 >(m_pFontHeightLB->GetSelectEntry().ToInt32()),
            batch);
    if(m_pNonPropFontsOnlyCB->GetSavedValue() != m_pNonPropFontsOnlyCB->IsChecked())
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

    return sal_False;
}

void  SvxFontSubstTabPage::Reset( const SfxItemSet& )
{
    m_pCheckLB->SetUpdateMode(sal_False);
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
        String aTmpStr1(pSubs->sFont);
        String aTmpStr2(pSubs->sReplaceBy);
        SvTreeListEntry* pEntry = CreateEntry(aTmpStr1, aTmpStr2);
        m_pCheckLB->Insert(pEntry);
        m_pCheckLB->CheckEntry(pEntry, 0, pSubs->bReplaceAlways);
        m_pCheckLB->CheckEntry(pEntry, 1, pSubs->bReplaceOnScreenOnly);
    }

    CheckEnable();
    m_pCheckLB->SetUpdateMode(sal_True);

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

IMPL_LINK(SvxFontSubstTabPage, SelectHdl, Window*, pWin)
{
    if (pWin == m_pApply || pWin == m_pDelete)
    {
        SvTreeListEntry* pEntry;
        // nCol is stupidly the nCol'th text column, not counted!
        // Therefor "0" as column.
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
                String sFont1 = m_pFont1CB->GetText();
                String sFont2 = m_pFont2CB->GetText();

                pEntry = CreateEntry(sFont1, sFont2);
                m_pCheckLB->Insert(pEntry);
            }
            m_pCheckLB->SelectAll(sal_False);
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

        if (m_pCheckLB->NextSelected(pEntry) == 0)
        {
            m_pFont1CB->SetText(m_pCheckLB->GetEntryText(pEntry, 0));
            m_pFont2CB->SetText(m_pCheckLB->GetEntryText(pEntry, 1));
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
                m_pCheckLB->SelectAll(sal_False);
                m_pCheckLB->Select(pEntry);
            }
        }
    }

    CheckEnable();

    return 0;
}

//--------------------------------------------------------------------------
IMPL_LINK(SvxFontSubstTabPage, NonPropFontsHdl, CheckBox*, pBox)
{
    String sFontName = m_pFontNameLB->GetSelectEntry();
    sal_Bool bNonPropOnly = pBox->IsChecked();
    m_pFontNameLB->Clear();
    FontList aFntLst( Application::GetDefaultDevice() );
    m_pFontNameLB->InsertEntry(m_sAutomatic);
    sal_uInt16 nFontCount = aFntLst.GetFontNameCount();
    for(sal_uInt16 nFont = 0; nFont < nFontCount; nFont++)
    {
        const FontInfo& rInfo = aFntLst.GetFontName( nFont );
        if(!bNonPropOnly || rInfo.GetPitch() == PITCH_FIXED)
            m_pFontNameLB->InsertEntry(rInfo.GetName());
    }
    m_pFontNameLB->SelectEntry(sFontName);
    return 0;
}

void SvxFontSubstTabPage::CheckEnable()
{
    sal_Bool bEnableAll = m_pUseTableCB->IsChecked();
    m_pReplacements->Enable(bEnableAll);
    if (bEnableAll)
    {
        sal_Bool bApply, bDelete;

        SvTreeListEntry* pEntry = m_pCheckLB->FirstSelected();

        String sEntry = m_pFont1CB->GetText();
        sEntry += '\t';
        sEntry += m_pFont2CB->GetText();

        // because of OS/2 optimization error (Bug #56267) a bit more intricate:
        if (m_pFont1CB->GetText().isEmpty() || m_pFont2CB->GetText().isEmpty())
            bApply = sal_False;
        else if(m_pFont1CB->GetText() == m_pFont2CB->GetText())
            bApply = sal_False;
        else if(m_pCheckLB->GetEntryPos(sEntry) != 0xffffffff)
            bApply = sal_False;
        else if(pEntry != 0 && m_pCheckLB->NextSelected(pEntry) != 0)
            bApply = sal_False;
        else
            bApply = sal_True;

        bDelete = pEntry != 0;

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
            m_pCheckLB->SelectAll(sal_False);
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
    SvxSimpleTable::SetTabs(aStaticTabs, MAP_PIXEL);
}

void SvxFontSubstCheckListBox::Resize()
{
    SvxSimpleTable::Resize();
    setColSizes();
}

void SvxFontSubstCheckListBox::SetTabs()
{
    SvxSimpleTable::SetTabs();
    sal_uInt16 nAdjust = SV_LBOXTAB_ADJUST_RIGHT|SV_LBOXTAB_ADJUST_LEFT|SV_LBOXTAB_ADJUST_CENTER|SV_LBOXTAB_ADJUST_NUMERIC|SV_LBOXTAB_FORCE;

    SvLBoxTab* pTab = aTabs[1];
    pTab->nFlags &= ~nAdjust;
    pTab->nFlags |= SV_LBOXTAB_PUSHABLE|SV_LBOXTAB_ADJUST_CENTER|SV_LBOXTAB_FORCE;

    pTab = aTabs[2];
    pTab->nFlags &= ~nAdjust;
    pTab->nFlags |= SV_LBOXTAB_PUSHABLE|SV_LBOXTAB_ADJUST_CENTER|SV_LBOXTAB_FORCE;
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
            CallImplEventListeners( VCLEVENT_CHECKBOX_TOGGLE, (void*)GetEntry( nSelPos ) );
        }
        else
        {
            sal_uInt16 nCheck = IsChecked(nSelPos, 1) ? 1 : 0;
            if(IsChecked(nSelPos, 0))
                nCheck += 2;
            nCheck--;
            nCheck &= 3;
            CheckEntryPos(nSelPos, 1, 0 != (nCheck & 1));
            CheckEntryPos(nSelPos, 0, 0 != (nCheck & 2));
        }
    }
    else
        SvxSimpleTable::KeyInput(rKEvt);
}

void SvxFontSubstCheckListBox::CheckEntryPos(sal_uLong nPos, sal_uInt16 nCol, sal_Bool bChecked)
{
    if ( nPos < GetEntryCount() )
        SetCheckButtonState(
            GetEntry(nPos),
            nCol,
            bChecked ? SvButtonState( SV_BUTTON_CHECKED ) :
                                       SvButtonState( SV_BUTTON_UNCHECKED ) );
}

void SvxFontSubstCheckListBox::CheckEntry(SvTreeListEntry* pEntry, sal_uInt16 nCol, sal_Bool bChecked)
{
    if ( pEntry )
        SetCheckButtonState(
            pEntry,
            nCol,
            bChecked ? SvButtonState( SV_BUTTON_CHECKED ) :
                                       SvButtonState( SV_BUTTON_UNCHECKED ) );
}

sal_Bool SvxFontSubstCheckListBox::IsChecked(sal_uLong nPos, sal_uInt16 nCol)
{
    return GetCheckButtonState( GetEntry(nPos), nCol ) == SV_BUTTON_CHECKED;
}

sal_Bool SvxFontSubstCheckListBox::IsChecked(SvTreeListEntry* pEntry, sal_uInt16 nCol)
{
    return GetCheckButtonState( pEntry, nCol ) == SV_BUTTON_CHECKED;
}

void SvxFontSubstCheckListBox::SetCheckButtonState( SvTreeListEntry* pEntry, sal_uInt16 nCol, SvButtonState eState)
{
    SvLBoxButton* pItem = (SvLBoxButton*)(pEntry->GetItem(nCol + 1));

    DBG_ASSERT(pItem,"SetCheckButton:Item not found");
    if (pItem->GetType() == SV_ITEM_ID_LBOXBUTTON)
    {
        switch( eState )
        {
            case SV_BUTTON_CHECKED:
                pItem->SetStateChecked();
                break;

            case SV_BUTTON_UNCHECKED:
                pItem->SetStateUnchecked();
                break;

            case SV_BUTTON_TRISTATE:
                pItem->SetStateTristate();
                break;
        }
        InvalidateEntry( pEntry );
    }
}

SvButtonState SvxFontSubstCheckListBox::GetCheckButtonState( SvTreeListEntry* pEntry, sal_uInt16 nCol ) const
{
    SvButtonState eState = SV_BUTTON_UNCHECKED;
    SvLBoxButton* pItem = (SvLBoxButton*)(pEntry->GetItem(nCol + 1));
    DBG_ASSERT(pItem,"GetChButnState:Item not found");

    if (pItem->GetType() == SV_ITEM_ID_LBOXBUTTON)
    {
        sal_uInt16 nButtonFlags = pItem->GetButtonFlags();
        eState = pCheckButtonData->ConvertToButtonState( nButtonFlags );
    }

    return eState;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
