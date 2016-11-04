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
#include <cuires.hrc>
#include "optchart.hxx"
#include <dialmgr.hxx>
#include <vcl/builderfactory.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/svapp.hxx>
#include <svx/svxids.hrc>

class ColorLB : public ListBox
{
    ImpColorList  aColorList; // separate liste, in case of user data are required from outside

public:
    ColorLB(vcl::Window* pParent, WinBits nWinStyle = WB_BORDER);
    virtual         ~ColorLB() override;
    virtual void    dispose() override;

    using ListBox::InsertEntry;
    sal_Int32       InsertEntry( const Color& rColor, const OUString& rStr,
                                 sal_Int32  nPos = LISTBOX_APPEND );
    using ListBox::RemoveEntry;
    void            RemoveEntry( sal_Int32  nPos );
    void            Clear();

    Color           GetEntryColor( sal_Int32  nPos ) const;

    Color GetSelectEntryColor() const
    {
        sal_Int32  nPos = GetSelectEntryPos();
        Color aColor;
        if ( nPos != LISTBOX_ENTRY_NOTFOUND )
            aColor = GetEntryColor( nPos );
        return aColor;
    }

    void Append( const XColorEntry& rEntry );
    void Modify( const XColorEntry& rEntry, sal_Int32 nPos );
};

VCL_BUILDER_DECL_FACTORY(ColorLB)
{
    bool bDropdown = VclBuilder::extractDropdown(rMap);
    WinBits nWinBits = WB_LEFT|WB_VCENTER|WB_3DLOOK|WB_SIMPLEMODE|WB_TABSTOP;
    if (bDropdown)
        nWinBits |= WB_DROPDOWN;
    OString sBorder = VclBuilder::extractCustomProperty(rMap);
    if (!sBorder.isEmpty())
        nWinBits |= WB_BORDER;
    VclPtrInstance<ColorLB> pListBox(pParent, nWinBits);
    pListBox->EnableAutoSize(true);
    rRet = pListBox;
}

ColorLB::ColorLB( vcl::Window* pParent, WinBits nWinStyle ) :
    ListBox( pParent, nWinStyle )
{
    SetEdgeBlending(true);
}

ColorLB::~ColorLB()
{
    disposeOnce();
}

void ColorLB::dispose()
{
    ListBox::dispose();
}

sal_Int32 ColorLB::InsertEntry( const Color& rColor, const OUString& rStr,
                                sal_Int32 nPos )
{
    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    Size aImageSize = rStyleSettings.GetListBoxPreviewDefaultPixelSize();

    VclPtr<VirtualDevice> xDevice = VclPtr<VirtualDevice>::Create();
    xDevice->SetOutputSize(aImageSize);
    const Rectangle aRect(Point(0, 0), aImageSize);
    xDevice->SetFillColor(rColor);
    xDevice->SetLineColor(rStyleSettings.GetDisableColor());
    xDevice->DrawRect(aRect);
    Bitmap aBitmap(xDevice->GetBitmap(Point(0, 0), xDevice->GetOutputSize()));

    nPos = ListBox::InsertEntry(rStr, Image(aBitmap), nPos);
    if ( nPos != LISTBOX_ERROR )
    {
        if ( static_cast<size_t>(nPos) < aColorList.size() )
        {
            ImpColorList::iterator it = aColorList.begin();
            ::std::advance( it, nPos );
            aColorList.insert( it, rColor );
        }
        else
        {
            aColorList.push_back( rColor );
            nPos = aColorList.size() - 1;
        }
    }
    return nPos;
}

void ColorLB::RemoveEntry( sal_Int32 nPos )
{
    ListBox::RemoveEntry( nPos );
    if ( 0 <= nPos && static_cast<size_t>(nPos) < aColorList.size() )
    {
        ImpColorList::iterator it = aColorList.begin();
        std::advance(it, nPos);
        aColorList.erase(it);
    }
}

void ColorLB::Clear()
{
    aColorList.clear();
    ListBox::Clear();
}

Color ColorLB::GetEntryColor( sal_Int32 nPos ) const
{
    Color aColor;
    if (0 <= nPos && static_cast<size_t>(nPos) < aColorList.size())
        aColor = aColorList[nPos];
    return aColor;
}

void ColorLB::Append( const XColorEntry& rEntry )
{
    InsertEntry( rEntry.GetColor(), rEntry.GetName() );
}

void ColorLB::Modify( const XColorEntry& rEntry, sal_Int32 nPos )
{
    RemoveEntry( nPos );
    InsertEntry( rEntry.GetColor(), rEntry.GetName(), nPos );
}

namespace
{
    void FillBoxChartColorLB(ColorLB *pLB, const SvxChartColorTable & rTab)
    {
        pLB->SetUpdateMode(false);
        pLB->Clear();
        long nCount = rTab.size();
        for(long i = 0; i < nCount; ++i)
        {
            pLB->Append(rTab[i]);
        }
        pLB->SetUpdateMode(true);
    }
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
        pColorConfig = (static_cast< SvxChartColorTableItem* >(pItem->Clone()) );
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
    if( pColorConfig )
        FillBoxChartColorLB(m_pLbChartColors, pColorConfig->GetColorList());

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
        rOutAttrs->Put( *(static_cast< SfxPoolItem* >(pColorConfig)));

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
    return -1L;
}


// event handlers


// ResetToDefaults


IMPL_LINK_NOARG(SvxDefaultColorOptPage, ResetToDefaults, Button*, void)
{
    if( pColorConfig )
    {
        pColorConfig->GetColorList().useDefault();

        FillBoxChartColorLB(m_pLbChartColors, pColorConfig->GetColorList());

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
        ColorData black = RGB_COLORDATA( 0x00, 0x00, 0x00 );

        pColorConfig->GetColorList().append (XColorEntry ( black, pColorConfig->GetColorList().getDefaultName(pColorConfig->GetColorList().size())));

        FillBoxChartColorLB(m_pLbChartColors, pColorConfig->GetColorList());

        m_pLbChartColors->GetFocus();
        m_pLbChartColors->SelectEntryPos( pColorConfig->GetColorList().size() - 1 );
        m_pPBRemove->Enable();
    }
}

// RemoveChartColor


IMPL_LINK( SvxDefaultColorOptPage, RemoveChartColor, Button*, pButton, void )
{
    sal_Int32 nIndex = m_pLbChartColors->GetSelectEntryPos();

    if (m_pLbChartColors->GetSelectEntryCount() == 0)
        return;

    if( pColorConfig )
    {
        OSL_ENSURE(pColorConfig->GetColorList().size() > 1, "don't delete the last chart color");

        ScopedVclPtrInstance<MessageDialog> aQuery(pButton, "QueryDeleteChartColorDialog",
                                                   "cui/ui/querydeletechartcolordialog.ui");
        if (RET_YES == aQuery->Execute())
        {
            pColorConfig->GetColorList().remove( nIndex  );

            FillBoxChartColorLB(m_pLbChartColors, pColorConfig->GetColorList());

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

IMPL_LINK( SvxDefaultColorOptPage, ListClickedHdl, ListBox&, _rBox, void )
{
    Color aCol = static_cast<ColorLB&>(_rBox).GetSelectEntryColor();

    long nIndex = GetColorIndex( aCol );

    if( nIndex == -1 )      // not found
        m_pValSetColorBox->SetNoSelection();
    else
        m_pValSetColorBox->SelectItem( nIndex + 1 );       // ValueSet is 1-based
}

IMPL_LINK_NOARG(SvxDefaultColorOptPage, BoxClickedHdl, ValueSet*, void)
{
    sal_Int32 nIdx = m_pLbChartColors->GetSelectEntryPos();
    if( nIdx != LISTBOX_ENTRY_NOTFOUND )
    {
        const XColorEntry aEntry( m_pValSetColorBox->GetItemColor( m_pValSetColorBox->GetSelectItemId() ), m_pLbChartColors->GetSelectEntry() );

        m_pLbChartColors->Modify( aEntry, nIdx );
        pColorConfig->ReplaceColorByIndex( nIdx, aEntry );

        m_pLbChartColors->SelectEntryPos( nIdx );  // reselect entry
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
