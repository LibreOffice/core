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

class SAL_WARN_UNUSED SVX_DLLPUBLIC ColorLB : public ListBox
{
    ImpColorList*   pColorList; // separate liste, in case of user data are required from outside
    Size            aImageSize;

    using Window::ImplInit;
    SVT_DLLPRIVATE void         ImplInit();
    SVT_DLLPRIVATE void         ImplDestroyColorEntries();

public:
                    ColorLB( vcl::Window* pParent,
                                  WinBits nWinStyle = WB_BORDER );
    virtual         ~ColorLB() override;
    virtual void    dispose() override;

    virtual void    UserDraw( const UserDrawEvent& rUDEvt ) override;

    using ListBox::InsertEntry;
    sal_Int32       InsertEntry( const OUString& rStr,
                                 sal_Int32  nPos = LISTBOX_APPEND );
    sal_Int32       InsertEntry( const Color& rColor, const OUString& rStr,
                                 sal_Int32  nPos = LISTBOX_APPEND );
    bool            IsAutomaticSelected() { return !GetSelectEntryPos(); }
    using ListBox::RemoveEntry;
    void            RemoveEntry( sal_Int32  nPos );
    void            Clear();
    void            CopyEntries( const ColorLB& rBox );

    using ListBox::GetEntryPos;
    sal_Int32       GetEntryPos( const Color& rColor ) const;
    Color           GetEntryColor( sal_Int32  nPos ) const;

    void            SelectEntry( const OUString& rStr )
                        { ListBox::SelectEntry( rStr ); }

    void SelectEntry( const Color& rColor )
    {
        sal_Int32  nPos = GetEntryPos( rColor );
        if ( nPos != LISTBOX_ENTRY_NOTFOUND )
            ListBox::SelectEntryPos( nPos );
    }

    Color GetSelectEntryColor() const
    {
        sal_Int32  nPos = GetSelectEntryPos();
        Color aColor;
        if ( nPos != LISTBOX_ENTRY_NOTFOUND )
            aColor = GetEntryColor( nPos );
        return aColor;
    }

    using ListBox::IsEntrySelected;

    bool            IsEntrySelected(const Color& rColor) const
    {
        sal_Int32  nPos = GetEntryPos( rColor );
        if ( nPos != LISTBOX_ENTRY_NOTFOUND )
            return IsEntryPosSelected( nPos );
        else
            return false;
    }

    void Fill( const XColorListRef &pTab );

    void Append( const XColorEntry& rEntry );
    void Modify( const XColorEntry& rEntry, sal_Int32 nPos );

private:
                    ColorLB( const ColorLB& ) = delete;
    ColorLB&   operator =( const ColorLB& ) = delete;
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

class ImplColorListData
{
public:
    Color       aColor;
    bool        bColor;

                ImplColorListData() : aColor( COL_BLACK ) { bColor = false; }
                explicit ImplColorListData( const Color& rColor ) : aColor( rColor ) { bColor = true; }
};

void ColorLB::ImplInit()
{
    pColorList = new ImpColorList();

    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    aImageSize = rStyleSettings.GetListBoxPreviewDefaultPixelSize();
    EnableUserDraw( true );
    SetUserItemSize( aImageSize );
}

void ColorLB::ImplDestroyColorEntries()
{
    for ( size_t n = pColorList->size(); n; )
        delete (*pColorList)[ --n ];
    pColorList->clear();
}

ColorLB::ColorLB( vcl::Window* pParent, WinBits nWinStyle ) :
    ListBox( pParent, nWinStyle )
{
    ImplInit();
    SetEdgeBlending(true);
}

ColorLB::~ColorLB()
{
    disposeOnce();
}

void ColorLB::dispose()
{
    if ( pColorList )
    {
        ImplDestroyColorEntries();
        delete pColorList;
        pColorList = nullptr;
    }
    ListBox::dispose();
}

sal_Int32 ColorLB::InsertEntry( const OUString& rStr, sal_Int32 nPos )
{
    nPos = ListBox::InsertEntry( rStr, nPos );
    if ( nPos != LISTBOX_ERROR )
    {
        ImplColorListData* pData = new ImplColorListData;
        if ( static_cast<size_t>(nPos) < pColorList->size() )
        {
            ImpColorList::iterator it = pColorList->begin();
            ::std::advance( it, nPos );
            pColorList->insert( it, pData );
        }
        else
        {
            pColorList->push_back( pData );
            nPos = pColorList->size() - 1;
        }
    }
    return nPos;
}

sal_Int32 ColorLB::InsertEntry( const Color& rColor, const OUString& rStr,
                                sal_Int32 nPos )
{
    nPos = ListBox::InsertEntry( rStr, nPos );
    if ( nPos != LISTBOX_ERROR )
    {
        ImplColorListData* pData = new ImplColorListData( rColor );
        if ( static_cast<size_t>(nPos) < pColorList->size() )
        {
            ImpColorList::iterator it = pColorList->begin();
            ::std::advance( it, nPos );
            pColorList->insert( it, pData );
        }
        else
        {
            pColorList->push_back( pData );
            nPos = pColorList->size() - 1;
        }
    }
    return nPos;
}

void ColorLB::RemoveEntry( sal_Int32 nPos )
{
    ListBox::RemoveEntry( nPos );
    if ( 0 <= nPos && static_cast<size_t>(nPos) < pColorList->size() )
    {
            ImpColorList::iterator it = pColorList->begin();
            ::std::advance( it, nPos );
            delete *it;
            pColorList->erase( it );
    }
}

void ColorLB::Clear()
{
    ImplDestroyColorEntries();
    ListBox::Clear();
}

void ColorLB::CopyEntries( const ColorLB& rBox )
{
    // Liste leeren
    ImplDestroyColorEntries();

    // Daten kopieren
    size_t nCount = rBox.pColorList->size();
    for ( size_t n = 0; n < nCount; n++ )
    {
        ImplColorListData* pData = (*rBox.pColorList)[ n ];
        sal_Int32 nPos = InsertEntry( rBox.GetEntry( n ) );
        if ( nPos != LISTBOX_ERROR )
        {
            if ( static_cast<size_t>(nPos) < pColorList->size() )
            {
                ImpColorList::iterator it = pColorList->begin();
                ::std::advance( it, nPos );
                pColorList->insert( it, new ImplColorListData( *pData ) );
            }
            else
            {
                pColorList->push_back( new ImplColorListData( *pData ) );
            }
        }
    }
}

sal_Int32 ColorLB::GetEntryPos( const Color& rColor ) const
{
    for( sal_Int32 n = (sal_Int32) pColorList->size(); n; )
    {
        ImplColorListData* pData = (*pColorList)[ --n ];
        if ( pData->bColor && ( pData->aColor == rColor ) )
            return n;
    }
    return LISTBOX_ENTRY_NOTFOUND;
}

Color ColorLB::GetEntryColor( sal_Int32 nPos ) const
{
    Color aColor;
    ImplColorListData* pData = ( 0 <= nPos && static_cast<size_t>(nPos) < pColorList->size() ) ?
        (*pColorList)[ nPos ] : nullptr;
    if ( pData && pData->bColor )
        aColor = pData->aColor;
    return aColor;
}

void ColorLB::UserDraw( const UserDrawEvent& rUDEvt )
{
    size_t nPos = rUDEvt.GetItemId();
    ImplColorListData* pData = ( nPos < pColorList->size() ) ? (*pColorList)[ nPos ] : nullptr;
    if ( pData )
    {
        if ( pData->bColor )
        {
            Point aPos( rUDEvt.GetRect().TopLeft() );

            aPos.X() += 2;
            aPos.Y() += ( rUDEvt.GetRect().GetHeight() - aImageSize.Height() ) / 2;

            const Rectangle aRect(aPos, aImageSize);

            vcl::RenderContext* pRenderContext = rUDEvt.GetRenderContext();
            pRenderContext->Push();
            pRenderContext->SetFillColor(pData->aColor);
            pRenderContext->SetLineColor(pRenderContext->GetTextColor());
            pRenderContext->DrawRect(aRect);
            pRenderContext->Pop();

            const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
            const sal_uInt16 nEdgeBlendingPercent(GetEdgeBlending() ? rStyleSettings.GetEdgeBlending() : 0);

            if(nEdgeBlendingPercent)
            {
                const Color& rTopLeft(rStyleSettings.GetEdgeBlendingTopLeftColor());
                const Color& rBottomRight(rStyleSettings.GetEdgeBlendingBottomRightColor());
                const sal_uInt8 nAlpha((nEdgeBlendingPercent * 255) / 100);
                const BitmapEx aBlendFrame(createBlendFrame(aRect.GetSize(), nAlpha, rTopLeft, rBottomRight));

                if(!aBlendFrame.IsEmpty())
                {
                    pRenderContext->DrawBitmapEx(aRect.TopLeft(), aBlendFrame);
                }
            }

            ListBox::DrawEntry( rUDEvt, false, false );
        }
        else
            ListBox::DrawEntry( rUDEvt, false, true );
    }
    else
        ListBox::DrawEntry( rUDEvt, true, false );
}


// Fills the Listbox with color and strings

void ColorLB::Fill( const XColorListRef &pColorTab )
{
    if( !pColorTab.is() )
        return;

    long nCount = pColorTab->Count();
    SetUpdateMode( false );

    for( long i = 0; i < nCount; i++ )
    {
        const XColorEntry* pEntry = pColorTab->GetColor(i);
        InsertEntry( pEntry->GetColor(), pEntry->GetName() );
    }

    AdaptDropDownLineCountToMaximum();
    SetUpdateMode( true );
}

void ColorLB::Append( const XColorEntry& rEntry )
{
    InsertEntry( rEntry.GetColor(), rEntry.GetName() );
    AdaptDropDownLineCountToMaximum();
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
