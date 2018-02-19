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

#include <unotools/pathoptions.hxx>
#include "optchart.hxx"
#include <dialmgr.hxx>
#include <vcl/builderfactory.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/weld.hxx>
#include <vcl/svapp.hxx>
#include <svx/svxids.hrc>

Color SvxDefaultColorOptPage::GetSelectEntryColor() const
{
    sal_Int32 nPos = m_pLbChartColors->GetSelectedEntryPos();
    Color aColor;
    if (nPos != LISTBOX_ENTRY_NOTFOUND)
        aColor = GetEntryColor(nPos);
    return aColor;
}

void SvxDefaultColorOptPage::InsertColorEntry(const XColorEntry& rEntry, sal_Int32 nPos)
{
    const Color& rColor = rEntry.GetColor();
    const OUString& rStr = rEntry.GetName();

    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    Size aImageSize = rStyleSettings.GetListBoxPreviewDefaultPixelSize();

    ScopedVclPtrInstance<VirtualDevice> xDevice;
    xDevice->SetOutputSize(aImageSize);
    const ::tools::Rectangle aRect(Point(0, 0), aImageSize);
    xDevice->SetFillColor(rColor);
    xDevice->SetLineColor(rStyleSettings.GetDisableColor());
    xDevice->DrawRect(aRect);
    Bitmap aBitmap(xDevice->GetBitmap(Point(0, 0), xDevice->GetOutputSize()));

    nPos = m_pLbChartColors->InsertEntry(rStr, Image(aBitmap), nPos);

    if (nPos != LISTBOX_ERROR)
    {
        if ( static_cast<size_t>(nPos) < aColorList.size() )
        {
            ImpColorList::iterator it = aColorList.begin();
            std::advance( it, nPos );
            aColorList.insert( it, rColor );
        }
        else
        {
            aColorList.push_back( rColor );
            nPos = aColorList.size() - 1;
        }
    }
}

void SvxDefaultColorOptPage::RemoveColorEntry(sal_Int32 nPos)
{
    m_pLbChartColors->RemoveEntry(nPos);
    if ( 0 <= nPos && static_cast<size_t>(nPos) < aColorList.size() )
    {
        ImpColorList::iterator it = aColorList.begin();
        std::advance(it, nPos);
        aColorList.erase(it);
    }
}

void SvxDefaultColorOptPage::ClearColorEntries()
{
    aColorList.clear();
    m_pLbChartColors->Clear();
}

Color SvxDefaultColorOptPage::GetEntryColor(sal_Int32 nPos) const
{
    Color aColor;
    if (0 <= nPos && static_cast<size_t>(nPos) < aColorList.size())
        aColor = aColorList[nPos];
    return aColor;
}

void SvxDefaultColorOptPage::ModifyColorEntry(const XColorEntry& rEntry, sal_Int32 nPos)
{
    RemoveColorEntry(nPos);
    InsertColorEntry(rEntry, nPos);
}

void SvxDefaultColorOptPage::FillBoxChartColorLB()
{
    if (!pColorConfig)
        return;

    const SvxChartColorTable & rTab = pColorConfig->GetColorList();
    m_pLbChartColors->SetUpdateMode(false);
    ClearColorEntries();
    long nCount = rTab.size();
    for (long i = 0; i < nCount; ++i)
        InsertColorEntry(rTab[i]);
    m_pLbChartColors->SetUpdateMode(true);
}

SvxDefaultColorOptPage::SvxDefaultColorOptPage(vcl::Window* pParent, const SfxItemSet& rInAttrs)
    : SfxTabPage(pParent, "OptChartColorsPage","cui/ui/optchartcolorspage.ui", &rInAttrs)
{
    get(m_pPBRemove, "delete");
    get(m_pPBAdd, "add");
    get(m_pPBDefault, "default");
    get(m_pValSetColorBox, "table");
    get(m_pLbChartColors, "colors");
    m_pLbChartColors->set_height_request(m_pLbChartColors->GetTextHeight()*16);

    m_pPBDefault->SetClickHdl( LINK( this, SvxDefaultColorOptPage, ResetToDefaults ) );
    m_pPBAdd->SetClickHdl( LINK( this, SvxDefaultColorOptPage, AddChartColor ) );
    m_pPBRemove->SetClickHdl( LINK( this, SvxDefaultColorOptPage, RemoveChartColor ) );
    m_pLbChartColors->SetSelectHdl( LINK( this, SvxDefaultColorOptPage, ListClickedHdl ) );
    m_pValSetColorBox->SetSelectHdl( LINK( this, SvxDefaultColorOptPage, BoxClickedHdl ) );

    m_pValSetColorBox->SetStyle( m_pValSetColorBox->GetStyle()
                                    | WB_ITEMBORDER | WB_NAMEFIELD );
    m_pValSetColorBox->SetColCount( 8 );
    m_pValSetColorBox->SetLineCount( 14 );
    m_pValSetColorBox->SetExtraSpacing( 0 );
    m_pValSetColorBox->Show();

    pChartOptions = new SvxChartOptions;
    pColorList = XColorList::CreateStdColorList();

    const SfxPoolItem* pItem = nullptr;
    if ( rInAttrs.GetItemState( SID_SCH_EDITOPTIONS, false, &pItem ) == SfxItemState::SET )
    {
        pColorConfig = static_cast< SvxChartColorTableItem* >(pItem->Clone());
    }
    else
    {
        SvxChartColorTable aTable;
        aTable.useDefault();
        pColorConfig = new SvxChartColorTableItem( SID_SCH_EDITOPTIONS, aTable );
        pColorConfig->SetOptions( pChartOptions );
    }

    Construct();
}

SvxDefaultColorOptPage::~SvxDefaultColorOptPage()
{
    disposeOnce();
}

void SvxDefaultColorOptPage::dispose()
{
    // save changes
    if (pChartOptions)
    {
        pChartOptions->SetDefaultColors( pColorConfig->GetColorList() );
        pChartOptions->Commit();

        delete pColorConfig;
        pColorConfig = nullptr;
        delete pChartOptions;
        pChartOptions = nullptr;
    }
    m_pLbChartColors.clear();
    m_pValSetColorBox.clear();
    m_pPBDefault.clear();
    m_pPBAdd.clear();
    m_pPBRemove.clear();
    SfxTabPage::dispose();
}

void SvxDefaultColorOptPage::Construct()
{
    FillBoxChartColorLB();
    FillColorBox();

    m_pLbChartColors->SelectEntryPos( 0 );
    ListClickedHdl(*m_pLbChartColors);
}


VclPtr<SfxTabPage> SvxDefaultColorOptPage::Create( vcl::Window* pParent, const SfxItemSet* rAttrs )
{
    return VclPtr<SvxDefaultColorOptPage>::Create( pParent, *rAttrs );
}

bool SvxDefaultColorOptPage::FillItemSet( SfxItemSet* rOutAttrs )
{
    if( pColorConfig )
        rOutAttrs->Put( *static_cast< SfxPoolItem* >(pColorConfig) );

    return true;
}

void SvxDefaultColorOptPage::Reset( const SfxItemSet* )
{
    m_pLbChartColors->SelectEntryPos( 0 );
    ListClickedHdl(*m_pLbChartColors);
}

void SvxDefaultColorOptPage::FillColorBox()
{
    if( !pColorList.is() ) return;

    long nCount = pColorList->Count();

    if( nCount > 104 )
        m_pValSetColorBox->SetStyle( m_pValSetColorBox->GetStyle() | WB_VSCROLL );

    for( long i = 0; i < nCount; i++ )
    {
        const XColorEntry* pColorEntry = pColorList->GetColor(i);
        m_pValSetColorBox->InsertItem( i + 1, pColorEntry->GetColor(), pColorEntry->GetName() );
    }
}


long SvxDefaultColorOptPage::GetColorIndex( const Color& rCol )
{
    if( pColorList.is() )
    {
        long nCount = pColorList->Count();

        for( long i = nCount - 1; i >= 0; i-- )         // default chart colors are at the end of the table
        {
            const XColorEntry* pColorEntry = pColorList->GetColor(i);
            if( pColorEntry && pColorEntry->GetColor() == rCol )
                return pColorList->GetIndex( pColorEntry->GetName() );
        }
    }
    return -1;
}


// event handlers


// ResetToDefaults


IMPL_LINK_NOARG(SvxDefaultColorOptPage, ResetToDefaults, Button*, void)
{
    if( pColorConfig )
    {
        pColorConfig->GetColorList().useDefault();

        FillBoxChartColorLB();

        m_pLbChartColors->GetFocus();
        m_pLbChartColors->SelectEntryPos( 0 );
        m_pPBRemove->Enable();
    }
}

// AddChartColor


IMPL_LINK_NOARG(SvxDefaultColorOptPage, AddChartColor, Button*, void)
{
    if( pColorConfig )
    {
        ColorData const black = RGB_COLORDATA( 0x00, 0x00, 0x00 );

        pColorConfig->GetColorList().append (XColorEntry ( black, pColorConfig->GetColorList().getDefaultName(pColorConfig->GetColorList().size())));

        FillBoxChartColorLB();

        m_pLbChartColors->GetFocus();
        m_pLbChartColors->SelectEntryPos( pColorConfig->GetColorList().size() - 1 );
        m_pPBRemove->Enable();
    }
}

// RemoveChartColor


IMPL_LINK_NOARG( SvxDefaultColorOptPage, RemoveChartColor, Button*, void )
{
    sal_Int32 nIndex = m_pLbChartColors->GetSelectedEntryPos();

    if (m_pLbChartColors->GetSelectedEntryCount() == 0)
        return;

    if( pColorConfig )
    {
        OSL_ENSURE(pColorConfig->GetColorList().size() > 1, "don't delete the last chart color");

        std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(GetFrameWeld(), "cui/ui/querydeletechartcolordialog.ui"));
        std::unique_ptr<weld::MessageDialog> xQuery(xBuilder->weld_message_dialog("QueryDeleteChartColorDialog"));

        if (RET_YES == xQuery->run())
        {
            pColorConfig->GetColorList().remove( nIndex  );

            FillBoxChartColorLB();

            m_pLbChartColors->GetFocus();

            if (nIndex == m_pLbChartColors->GetEntryCount() && m_pLbChartColors->GetEntryCount() > 0)
                m_pLbChartColors->SelectEntryPos( pColorConfig->GetColorList().size() - 1 );
            else if (m_pLbChartColors->GetEntryCount() > 0)
                m_pLbChartColors->SelectEntryPos( nIndex );
            else
                m_pPBRemove->Enable();
        }
    }
}

IMPL_LINK_NOARG( SvxDefaultColorOptPage, ListClickedHdl, ListBox&, void )
{
    Color aCol = GetSelectEntryColor();

    long nIndex = GetColorIndex( aCol );

    if( nIndex == -1 )      // not found
        m_pValSetColorBox->SetNoSelection();
    else
        m_pValSetColorBox->SelectItem( nIndex + 1 );       // ValueSet is 1-based
}

IMPL_LINK_NOARG(SvxDefaultColorOptPage, BoxClickedHdl, ValueSet*, void)
{
    sal_Int32 nIdx = m_pLbChartColors->GetSelectedEntryPos();
    if( nIdx != LISTBOX_ENTRY_NOTFOUND )
    {
        const XColorEntry aEntry( m_pValSetColorBox->GetItemColor( m_pValSetColorBox->GetSelectItemId() ), m_pLbChartColors->GetSelectedEntry() );

        ModifyColorEntry(aEntry, nIdx);
        pColorConfig->ReplaceColorByIndex( nIdx, aEntry );

        m_pLbChartColors->SelectEntryPos( nIdx );  // reselect entry
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
