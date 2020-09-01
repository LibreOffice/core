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


#include <vcl/toolkit/ivctrl.hxx>
#include "imivctl.hxx"
#include <vcl/accessiblefactory.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/commandevent.hxx>
#include <vcl/mnemonic.hxx>
#include <vcl/settings.hxx>
#include <vcl/tabctrl.hxx>
#include <vcl/vclevent.hxx>
#include <vcl/uitest/uiobject.hxx>
#include <verticaltabctrl.hxx>

using namespace ::com::sun::star::accessibility;

/*****************************************************************************
|
| class : SvxIconChoiceCtrlEntry
|
\*****************************************************************************/

SvxIconChoiceCtrlEntry::SvxIconChoiceCtrlEntry( const OUString& rText,
                                                const Image& rImage )
    : aImage(rImage)
    , aText(rText)
    , nPos(0)
    , pblink(nullptr)
    , pflink(nullptr)
    , eTextMode(SvxIconChoiceCtrlTextMode::Short)
    , nX(0)
    , nY(0)
    , nFlags(SvxIconViewFlags::NONE)
{
}

OUString SvxIconChoiceCtrlEntry::GetDisplayText() const
{
    return MnemonicGenerator::EraseAllMnemonicChars( aText );
}


SvxIconChoiceCtrlColumnInfo::SvxIconChoiceCtrlColumnInfo( const SvxIconChoiceCtrlColumnInfo& rInfo )
{
    nWidth = rInfo.nWidth;
}

/*****************************************************************************
|
| class : SvtIconChoiceCtrl
|
\*****************************************************************************/

SvtIconChoiceCtrl::SvtIconChoiceCtrl( vcl::Window* pParent, WinBits nWinStyle ) :

     // WB_CLIPCHILDREN on, as ScrollBars lie on the window!
    Control( pParent, nWinStyle | WB_CLIPCHILDREN ),

    _pImpl           ( new SvxIconChoiceCtrl_Impl( this, nWinStyle ) )
{
    SetLineColor();
    _pImpl->InitSettings();
    _pImpl->SetPositionMode( SvxIconChoiceCtrlPositionMode::AutoArrange );
}

SvtIconChoiceCtrl::~SvtIconChoiceCtrl()
{
    disposeOnce();
}

void SvtIconChoiceCtrl::dispose()
{
    if (_pImpl)
    {
        _pImpl->CallEventListeners( VclEventId::ObjectDying, nullptr );
        _pImpl.reset();
    }
    Control::dispose();
}

SvxIconChoiceCtrlEntry* SvtIconChoiceCtrl::InsertEntry( const OUString& rText, const Image& rImage  )
{
    SvxIconChoiceCtrlEntry* pEntry = new SvxIconChoiceCtrlEntry( rText, rImage);

    _pImpl->InsertEntry(std::unique_ptr<SvxIconChoiceCtrlEntry>(pEntry), _pImpl->GetEntryCount());

    return pEntry;
}

void SvtIconChoiceCtrl::RemoveEntry(sal_Int32 nIndex)
{
    _pImpl->RemoveEntry(nIndex);
}

void SvtIconChoiceCtrl::DrawEntryImage( SvxIconChoiceCtrlEntry const * pEntry, const Point& rPos, OutputDevice& rDev )
{
    rDev.DrawImage( rPos, pEntry->GetImage() );
}

OUString SvtIconChoiceCtrl::GetEntryText( SvxIconChoiceCtrlEntry const * pEntry )
{
    return pEntry->GetText();
}

void SvtIconChoiceCtrl::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect)
{
    _pImpl->Paint(rRenderContext, rRect);
}

void SvtIconChoiceCtrl::MouseButtonDown( const MouseEvent& rMEvt )
{
    if( !_pImpl->MouseButtonDown( rMEvt ) )
        Control::MouseButtonDown( rMEvt );
}

void SvtIconChoiceCtrl::MouseButtonUp( const MouseEvent& rMEvt )
{
    if( !_pImpl->MouseButtonUp( rMEvt ) )
        Control::MouseButtonUp( rMEvt );
}

void SvtIconChoiceCtrl::MouseMove( const MouseEvent& rMEvt )
{
    if( !_pImpl->MouseMove( rMEvt ) )
        Control::MouseMove( rMEvt );
}
void SvtIconChoiceCtrl::ArrangeIcons()
{
    if ( GetStyle() & WB_ALIGN_TOP )
    {
        Size aFullSize;
        tools::Rectangle aEntryRect;

        for ( sal_Int32 i = 0; i < GetEntryCount(); i++ )
        {
            SvxIconChoiceCtrlEntry* pEntry = GetEntry ( i );
            aEntryRect = _pImpl->GetEntryBoundRect ( pEntry );

            aFullSize.setWidth ( aFullSize.getWidth()+aEntryRect.GetWidth() );
        }

        _pImpl->Arrange ( false, aFullSize.getWidth(), 0 );
    }
    else if ( GetStyle() & WB_ALIGN_LEFT )
    {
        Size aFullSize;
        tools::Rectangle aEntryRect;

        for ( sal_Int32 i = 0; i < GetEntryCount(); i++ )
        {
            SvxIconChoiceCtrlEntry* pEntry = GetEntry ( i );
            aEntryRect = _pImpl->GetEntryBoundRect ( pEntry );

            aFullSize.setHeight ( aFullSize.getHeight()+aEntryRect.GetHeight() );
        }

        _pImpl->Arrange ( false, 0, aFullSize.getHeight() );
    }
    else
    {
        _pImpl->Arrange(false, 0, 0);
    }
    _pImpl->Arrange( false, 0, 1000 );
}
void SvtIconChoiceCtrl::Resize()
{
    _pImpl->Resize();
    Control::Resize();
}

void SvtIconChoiceCtrl::GetFocus()
{
    _pImpl->GetFocus();
    Control::GetFocus();
    SvxIconChoiceCtrlEntry* pSelectedEntry = GetSelectedEntry();
    if ( pSelectedEntry )
        _pImpl->CallEventListeners( VclEventId::ListboxSelect, pSelectedEntry );
}

void SvtIconChoiceCtrl::LoseFocus()
{
    if (_pImpl)
        _pImpl->LoseFocus();
    Control::LoseFocus();
}

void SvtIconChoiceCtrl::SetFont(const vcl::Font& rFont)
{
    if (rFont != GetFont())
    {
        Control::SetFont(rFont);
        _pImpl->FontModified();
    }
}

void SvtIconChoiceCtrl::SetPointFont(const vcl::Font& rFont)
{
    if (rFont != GetPointFont(*this)) //FIXME
    {
        Control::SetPointFont(*this, rFont); //FIXME
        _pImpl->FontModified();
    }
}

WinBits SvtIconChoiceCtrl::GetStyle() const
{
    return _pImpl->GetStyle();
}

void SvtIconChoiceCtrl::Command(const CommandEvent& rCEvt)
{
    _pImpl->Command( rCEvt );
    //pass at least alt press/release to parent impl
    if (rCEvt.GetCommand() == CommandEventId::ModKeyChange)
        Control::Command(rCEvt);
}

#ifdef DBG_UTIL
void SvtIconChoiceCtrl::SetEntryTextMode( SvxIconChoiceCtrlTextMode eMode, SvxIconChoiceCtrlEntry* pEntry )
{
    _pImpl->SetEntryTextMode( eMode, pEntry );
}
#endif

sal_Int32 SvtIconChoiceCtrl::GetEntryCount() const
{
    return _pImpl ? _pImpl->GetEntryCount() : 0;
}

SvxIconChoiceCtrlEntry* SvtIconChoiceCtrl::GetEntry( sal_Int32 nPos ) const
{
    return _pImpl ? _pImpl->GetEntry( nPos ) : nullptr;
}

SvxIconChoiceCtrlEntry* SvtIconChoiceCtrl::GetSelectedEntry() const
{
    return _pImpl ? _pImpl->GetFirstSelectedEntry() : nullptr;
}

void SvtIconChoiceCtrl::ClickIcon()
{
    GetSelectedEntry();
    _aClickIconHdl.Call( this );
}

void SvtIconChoiceCtrl::KeyInput( const KeyEvent& rKEvt )
{
    bool bKeyUsed = DoKeyInput( rKEvt );
    if ( !bKeyUsed )
    {
        Control::KeyInput( rKEvt );
    }
}
bool SvtIconChoiceCtrl::DoKeyInput( const KeyEvent& rKEvt )
{
    return _pImpl->KeyInput( rKEvt );
}
sal_Int32 SvtIconChoiceCtrl::GetEntryListPos( SvxIconChoiceCtrlEntry const * pEntry ) const
{
    return _pImpl->GetEntryListPos( pEntry );
}
SvxIconChoiceCtrlEntry* SvtIconChoiceCtrl::GetCursor( ) const
{
    return _pImpl->GetCurEntry( );
}
void SvtIconChoiceCtrl::SetCursor( SvxIconChoiceCtrlEntry* pEntry )
{
    _pImpl->SetCursor( pEntry );
}

void SvtIconChoiceCtrl::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( ((rDCEvt.GetType() == DataChangedEventType::SETTINGS) ||
         (rDCEvt.GetType() == DataChangedEventType::FONTSUBSTITUTION) ||
         (rDCEvt.GetType() == DataChangedEventType::FONTS) ) &&
         (rDCEvt.GetFlags() & AllSettingsFlags::STYLE) )
    {
        _pImpl->InitSettings();
        Invalidate(InvalidateFlags::NoChildren);
    }
    else
        Control::DataChanged( rDCEvt );
}

void SvtIconChoiceCtrl::SetBackground( const Wallpaper& rPaper )
{
    if( rPaper == GetBackground() )
        return;

    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    // if it is the default (empty) wallpaper
    if (rPaper.IsEmpty())
    {
        Control::SetBackground( rStyleSettings.GetFieldColor() );
    }
    else
    {
        Wallpaper aBackground( rPaper );
        // HACK, as background might be transparent!
        if( !aBackground.IsBitmap() )
            aBackground.SetStyle( WallpaperStyle::Tile );

        WallpaperStyle eStyle = aBackground.GetStyle();
        Color aBack( aBackground.GetColor());
        if( aBack == COL_TRANSPARENT &&
            (!aBackground.IsBitmap() ||
             aBackground.GetBitmap().IsTransparent() ||
             (eStyle != WallpaperStyle::Tile && eStyle != WallpaperStyle::Scale)) )
        {
            aBackground.SetColor( rStyleSettings.GetFieldColor() );
        }
        if( aBackground.IsScrollable() )
        {
            tools::Rectangle aRect;
            aRect.SetSize( Size(32765, 32765) );
            aBackground.SetRect( aRect );
        }
        else
        {
            tools::Rectangle aRect( _pImpl->GetOutputRect() );
            aBackground.SetRect( aRect );
        }
        Control::SetBackground( aBackground );
    }

    // If text colors are attributed "hard," don't use automatism to select
    // a readable text color.
    vcl::Font aFont( GetFont() );
    aFont.SetColor( rStyleSettings.GetFieldTextColor() );
    SetFont( aFont );

    Invalidate(InvalidateFlags::NoChildren);
}

void SvtIconChoiceCtrl::RequestHelp( const HelpEvent& rHEvt )
{
    if ( !_pImpl->RequestHelp( rHEvt ) )
        Control::RequestHelp( rHEvt );
}

tools::Rectangle SvtIconChoiceCtrl::GetBoundingBox( SvxIconChoiceCtrlEntry* pEntry ) const
{
    return _pImpl->GetEntryBoundRect( pEntry );
}

void SvtIconChoiceCtrl::FillLayoutData() const
{
    CreateLayoutData();
    const_cast<SvtIconChoiceCtrl*>(this)->Invalidate();
}

tools::Rectangle SvtIconChoiceCtrl::GetEntryCharacterBounds( const sal_Int32 _nEntryPos, const sal_Int32 _nCharacterIndex ) const
{
    tools::Rectangle aRect;

    Pair aEntryCharacterRange = GetLineStartEnd( _nEntryPos );
    if ( aEntryCharacterRange.A() + _nCharacterIndex < aEntryCharacterRange.B() )
    {
        aRect = GetCharacterBounds( aEntryCharacterRange.A() + _nCharacterIndex );
    }

    return aRect;
}

void SvtIconChoiceCtrl::SetNoSelection()
{
    _pImpl->SetNoSelection();
}

void SvtIconChoiceCtrl::CallImplEventListeners(VclEventId nEvent, void* pData)
{
    CallEventListeners(nEvent, pData);
}
css::uno::Reference< XAccessible > SvtIconChoiceCtrl::CreateAccessible()
{
    vcl::Window* pParent = GetAccessibleParentWindow();
    DBG_ASSERT( pParent, "SvTreeListBox::CreateAccessible - accessible parent not found" );

    css::uno::Reference< XAccessible > xAccessible;
    if ( pParent )
    {
        css::uno::Reference< XAccessible > xAccParent = pParent->GetAccessible();
        if ( xAccParent.is() )
        {
            css::uno::Reference< css::awt::XWindowPeer > xHoldAlive(GetComponentInterface());
            xAccessible = _pImpl->GetAccessibleFactory().createAccessibleIconChoiceCtrl( *this, xAccParent );
        }
    }
    return xAccessible;
}

struct VerticalTabPageData
{
    OString sId;
    SvxIconChoiceCtrlEntry* pEntry;
    VclPtr<vcl::Window> xPage;      ///< the TabPage itself
};

VerticalTabControl::VerticalTabControl(vcl::Window* pParent)
    : VclHBox(pParent)
    , m_xChooser(VclPtr<SvtIconChoiceCtrl>::Create(this, WB_3DLOOK | WB_ICON | WB_BORDER |
                                                         WB_NOCOLUMNHEADER | WB_HIGHLIGHTFRAME |
                                                         WB_NODRAGSELECTION | WB_TABSTOP | WB_CLIPCHILDREN |
                                                         WB_ALIGN_LEFT | WB_NOHSCROLL))
    , m_xBox(VclPtr<VclVBox>::Create(this))
{
    SetType(WindowType::VERTICALTABCONTROL);
    m_xChooser->SetClickHdl(LINK(this, VerticalTabControl, ChosePageHdl_Impl));
    m_xChooser->set_width_request(110);
    m_xChooser->set_height_request(400);
    m_xChooser->SetSizePixel(Size(110, 400));
    m_xBox->set_vexpand(true);
    m_xBox->set_hexpand(true);
    m_xBox->set_expand(true);
    m_xBox->Show();
    m_xChooser->Show();
}

VerticalTabControl::~VerticalTabControl()
{
    disposeOnce();
}

void VerticalTabControl::dispose()
{
    m_xChooser.disposeAndClear();
    m_xBox.disposeAndClear();
    VclHBox::dispose();
}

IMPL_LINK_NOARG(VerticalTabControl, ChosePageHdl_Impl, SvtIconChoiceCtrl*, void)
{
    SvxIconChoiceCtrlEntry *pEntry = m_xChooser->GetSelectedEntry();
    if (!pEntry)
        pEntry = m_xChooser->GetCursor();

    VerticalTabPageData* pData = GetPageData(pEntry);

    if (pData->sId != m_sCurrentPageId)
        SetCurPageId(pData->sId);
}

void VerticalTabControl::ActivatePage()
{
    m_aActivateHdl.Call( this );
}

bool VerticalTabControl::DeactivatePage()
{
    return !m_aDeactivateHdl.IsSet() || m_aDeactivateHdl.Call(this);
}

VerticalTabPageData* VerticalTabControl::GetPageData(const SvxIconChoiceCtrlEntry* pEntry) const
{
    VerticalTabPageData* pRet = nullptr;
    for (auto & pData : maPageList)
    {
        if (pData->pEntry == pEntry)
        {
            pRet = pData.get();
            break;
        }
    }
    return pRet;
}

VerticalTabPageData* VerticalTabControl::GetPageData(const OString& rId) const
{
    VerticalTabPageData* pRet = nullptr;
    for (auto & pData : maPageList)
    {
        if (pData->sId == rId)
        {
            pRet = pData.get();
            break;
        }
    }
    return pRet;
}

void VerticalTabControl::SetCurPageId(const OString& rId)
{
    OString sOldPageId = GetCurPageId();
    if (sOldPageId == rId)
        return;

    VerticalTabPageData* pOldData = GetPageData(sOldPageId);
    if (pOldData && pOldData->xPage)
    {
        if (!DeactivatePage())
            return;
        pOldData->xPage->Hide();
    }

    m_sCurrentPageId = "";

    VerticalTabPageData* pNewData = GetPageData(rId);
    if (pNewData && pNewData->xPage)
    {
        m_sCurrentPageId = rId;
        m_xChooser->SetCursor(pNewData->pEntry);

        ActivatePage();
        pNewData->xPage->Show();
    }
}

OString VerticalTabControl::GetPageId(sal_uInt16 nIndex) const
{
    return maPageList[nIndex]->sId;
}

void VerticalTabControl::InsertPage(const rtl::OString &rIdent, const rtl::OUString& rLabel, const Image& rImage,
                                    const rtl::OUString& rTooltip, VclPtr<vcl::Window> xPage, int nPos)
{
    SvxIconChoiceCtrlEntry* pEntry = m_xChooser->InsertEntry(rLabel, rImage);
    pEntry->SetQuickHelpText(rTooltip);
    m_xChooser->ArrangeIcons();
    VerticalTabPageData* pNew;
    if (nPos == -1)
    {
        maPageList.emplace_back(new VerticalTabPageData);
        pNew = maPageList.back().get();
    }
    else
    {
        maPageList.emplace(maPageList.begin() + nPos, new VerticalTabPageData);
        pNew = maPageList[nPos].get();
    }
    pNew->sId = rIdent;
    pNew->pEntry = pEntry;
    pNew->xPage = xPage;
    Size aOrigPrefSize(m_xBox->get_preferred_size());
    Size aPagePrefSize(xPage->get_preferred_size());
    m_xBox->set_width_request(std::max(aOrigPrefSize.Width(), aPagePrefSize.Width()));
    m_xBox->set_height_request(std::max(aOrigPrefSize.Height(), aPagePrefSize.Height()));
    pNew->xPage->Hide();
}

void VerticalTabControl::RemovePage(const rtl::OString &rPageId)
{
    for (auto it = maPageList.begin(), end = maPageList.end(); it != end; ++it)
    {
        VerticalTabPageData* pData = it->get();
        if (pData->sId == rPageId)
        {
            sal_Int32 nEntryListPos = m_xChooser->GetEntryListPos(pData->pEntry);
            m_xChooser->RemoveEntry(nEntryListPos);
            m_xChooser->ArrangeIcons();
            maPageList.erase(it);
            break;
        }
    }
}

sal_uInt16 VerticalTabControl::GetPagePos(const OString& rPageId) const
{
    VerticalTabPageData* pData = GetPageData(rPageId);
    if (!pData)
        return TAB_PAGE_NOTFOUND;
    return m_xChooser->GetEntryListPos(pData->pEntry);
}

VclPtr<vcl::Window> VerticalTabControl::GetPage(const OString& rPageId)
{
    VerticalTabPageData* pData = GetPageData(rPageId);
    if (!pData)
        return nullptr;
    return pData->xPage;
}

OUString VerticalTabControl::GetPageText(const OString& rPageId) const
{
    VerticalTabPageData* pData = GetPageData(rPageId);
    if (!pData)
        return OUString();
    return pData->pEntry->GetText();
}

void VerticalTabControl::SetPageText(const OString& rPageId, const OUString& rText)
{
    VerticalTabPageData* pData = GetPageData(rPageId);
    if (!pData)
        return;
    pData->pEntry->SetText(rText);
}

FactoryFunction VerticalTabControl::GetUITestFactory() const
{
    return VerticalTabControlUIObject::create;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
