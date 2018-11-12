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

#include <roadmap.hxx>

#include <vector>
#include <algorithm>
#include <vcl/bitmap.hxx>
#include <vcl/event.hxx>
#include <vcl/settings.hxx>
#include <tools/color.hxx>
#include <rtl/ustring.hxx>

#define ROADMAP_INDENT_X        4
#define ROADMAP_INDENT_Y        27
#define ROADMAP_ITEM_DISTANCE_Y 6

namespace svt
{

typedef std::vector< RoadmapItem* > HL_Vector;

//= ColorChanger

class IDLabel :  public FixedText
{
public:
    IDLabel( vcl::Window* _pParent, WinBits _nWinStyle );
    virtual void DataChanged( const DataChangedEvent& rDCEvt ) override;
    virtual void ApplySettings(vcl::RenderContext& rRenderContext) override;
};

class RoadmapItem : public RoadmapTypes
{
private:
    VclPtr<IDLabel>         mpID;
    VclPtr<HyperLabel>      mpDescription;
    const Size              m_aItemPlayground;

public:
    RoadmapItem( ORoadmap& _rParent, const Size& _rItemPlayground );
    ~RoadmapItem();

    void                    SetID( sal_Int16 ID );
    sal_Int16               GetID() const;

    void                    SetIndex( ItemIndex Index );
    ItemIndex               GetIndex() const;

    void                    Update( ItemIndex RMIndex, const OUString& _rText );

    void                    SetPosition( RoadmapItem const * OldHyperLabel );

    void                    ToggleBackgroundColor( const Color& _rGBColor );
    void                    SetInteractive( bool _bInteractive );

    void                    SetClickHdl( const Link<HyperLabel*,void>& rLink );
    void                    Enable( bool bEnable );
    bool                    IsEnabled() const;
    void                    GrabFocus();

    bool                    Contains( const vcl::Window* _pWindow ) const;

private:
    void                    ImplUpdateIndex( const ItemIndex _nIndex );
    void                    ImplUpdatePosSize();
};

//= RoadmapImpl

class RoadmapImpl : public RoadmapTypes
{
protected:
    const ORoadmap&     m_rAntiImpl;
    Link<LinkParamNone*,void> m_aSelectHdl;
    BitmapEx            m_aPicture;
    HL_Vector           m_aRoadmapSteps;
    ItemId              m_iCurItemID;
    bool                m_bInteractive : 1;
    bool                m_bComplete : 1;
    Size                m_aItemSizePixel;
public:
    bool                m_bPaintInitialized : 1;

public:
    explicit RoadmapImpl(const ORoadmap& rAntiImpl)
        : m_rAntiImpl(rAntiImpl)
        , m_iCurItemID(-1)
        , m_bInteractive(true)
        , m_bComplete(true)
        , m_bPaintInitialized(false)
        , InCompleteHyperLabel(nullptr)
    {}

    RoadmapItem* InCompleteHyperLabel;

    HL_Vector& getHyperLabels()
    {
        return m_aRoadmapSteps;
    }

    void insertHyperLabel(ItemIndex Index, RoadmapItem* _rRoadmapStep)
    {
        m_aRoadmapSteps.insert(m_aRoadmapSteps.begin() + Index, _rRoadmapStep);
    }

    ItemIndex getItemCount() const
    {
        return m_aRoadmapSteps.size();
    }

    void setCurItemID(ItemId i)
    {
        m_iCurItemID = i;
    }
    ItemId getCurItemID() const
    {
        return m_iCurItemID;
    }

    void setInteractive(const bool _bInteractive)
    {
        m_bInteractive = _bInteractive;
    }
    bool isInteractive() const
    {
        return m_bInteractive;
    }

    void setComplete(const bool _bComplete)
    {
        m_bComplete = _bComplete;
    }
    bool isComplete() const
    {
        return m_bComplete;
    }

    void setPicture(const BitmapEx& _rPic)
    {
        m_aPicture = _rPic;
    }
    const BitmapEx& getPicture() const
    {
        return m_aPicture;
    }

    void setSelectHdl(const Link<LinkParamNone*,void>& _rHdl)
    {
        m_aSelectHdl = _rHdl;
    }
    const Link<LinkParamNone*,void>& getSelectHdl() const
    {
        return m_aSelectHdl;
    }

    void initItemSize();
    const Size& getItemSize() const
    {
        return m_aItemSizePixel;
    }

    void removeHyperLabel(ItemIndex Index)
    {
        if ((Index > -1) && (Index < getItemCount()))
        {
            delete m_aRoadmapSteps[Index];
            m_aRoadmapSteps.erase(m_aRoadmapSteps.begin() + Index);
        }
    }
};

void RoadmapImpl::initItemSize()
{
    Size aLabelSize( m_rAntiImpl.GetOutputSizePixel() );
    aLabelSize.setHeight( m_rAntiImpl.LogicToPixel(Size(0, LABELBASEMAPHEIGHT), MapMode(MapUnit::MapAppFont)).Height() );
    aLabelSize.AdjustWidth( -(m_rAntiImpl.LogicToPixel(Size(2 * ROADMAP_INDENT_X, 0), MapMode(MapUnit::MapAppFont)).Width()) );
    m_aItemSizePixel = aLabelSize;
}

//= Roadmap

ORoadmap::ORoadmap(vcl::Window* _pParent, WinBits _nWinStyle)
    : Control(_pParent, _nWinStyle)
    , m_pImpl(new RoadmapImpl(*this))
{
}

void ORoadmap::implInit(vcl::RenderContext& rRenderContext)
{
    m_pImpl->InCompleteHyperLabel = nullptr;
    m_pImpl->setCurItemID(-1);
    m_pImpl->setComplete(true);
    m_pImpl->m_bPaintInitialized = true;

    // Roadmap control should be reachable as one unit with a Tab key
    // the next Tab key should spring out of the control.
    // To reach it the control itself should get focus and set it
    // on entries. The entries themself should not be reachable with
    // the Tab key directly. So each entry should have WB_NOTABSTOP.

    // In other words the creator should create the control with the following
    // flags:
    // SetStyle( ( GetStyle() | WB_TABSTOP ) & ~WB_DIALOGCONTROL );

// TODO: if somebody sets a new font from outside (OutputDevice::SetFont), we would have to react
// on this with calculating a new bold font.
// Unfortunately, the OutputDevice does not offer a notify mechanism for a changed font.
// So settings the font from outside is simply a forbidden scenario at the moment
    rRenderContext.EnableMapMode(false);
}

ORoadmap::~ORoadmap()
{
    disposeOnce();
}

void ORoadmap::dispose()
{
    HL_Vector aItemsCopy = m_pImpl->getHyperLabels();
    m_pImpl->getHyperLabels().clear();
    for (auto const& itemCopy : aItemsCopy)
    {
        delete itemCopy;
    }
    if ( ! m_pImpl->isComplete() )
        delete m_pImpl->InCompleteHyperLabel;
    m_pImpl.reset();
    Control::dispose();
}

RoadmapTypes::ItemId ORoadmap::GetCurrentRoadmapItemID() const
{
    return m_pImpl->getCurItemID();
}

RoadmapItem* ORoadmap::GetPreviousHyperLabel(ItemIndex Index)
{
    RoadmapItem* pOldItem = nullptr;
    if ( Index > 0 )
        pOldItem = m_pImpl->getHyperLabels().at( Index - 1 );
    return pOldItem;
}

RoadmapItem* ORoadmap::InsertHyperLabel(ItemIndex Index, const OUString& _sLabel, ItemId RMID, bool _bEnabled, bool _bIncomplete)
{
    if (m_pImpl->getItemCount() == 0)
        m_pImpl->initItemSize();

    RoadmapItem* pItem = nullptr;
    RoadmapItem* pOldItem = GetPreviousHyperLabel( Index );

    pItem = new RoadmapItem( *this, m_pImpl->getItemSize() );
    if ( _bIncomplete )
    {
        pItem->SetInteractive( false );
    }
    else
    {
        pItem->SetInteractive( m_pImpl->isInteractive() );
        m_pImpl->insertHyperLabel( Index, pItem );
    }
    pItem->SetPosition( pOldItem );
    pItem->Update( Index, _sLabel );
    pItem->SetClickHdl(LINK( this, ORoadmap, ImplClickHdl ) );
    pItem->SetID( RMID );
    pItem->SetIndex( Index );
    if (!_bEnabled)
        pItem->Enable( _bEnabled );
    return pItem;
}

void ORoadmap::SetRoadmapBitmap(const BitmapEx& _rBmp)
{
    m_pImpl->setPicture( _rBmp );
    Invalidate( );
}

void ORoadmap::SetRoadmapInteractive(bool _bInteractive)
{
    m_pImpl->setInteractive( _bInteractive );

    const HL_Vector& rItems = m_pImpl->getHyperLabels();
    for (auto const& item : rItems)
    {
        item->SetInteractive( _bInteractive );
    }
}

bool ORoadmap::IsRoadmapInteractive()
{
    return m_pImpl->isInteractive();
}

void ORoadmap::SetRoadmapComplete(bool _bComplete)
{
    bool bWasComplete = m_pImpl->isComplete();
    m_pImpl->setComplete( _bComplete );
    if (_bComplete)
    {
        if (m_pImpl->InCompleteHyperLabel != nullptr)
        {
            delete m_pImpl->InCompleteHyperLabel;
            m_pImpl->InCompleteHyperLabel = nullptr;
        }
    }
    else if (bWasComplete)
        m_pImpl->InCompleteHyperLabel = InsertHyperLabel(m_pImpl->getItemCount(), "...", -1, true/*bEnabled*/, true/*bIncomplete*/ );
}

void ORoadmap::UpdatefollowingHyperLabels(ItemIndex _nIndex)
{
    const HL_Vector& rItems = m_pImpl->getHyperLabels();
    if ( _nIndex < static_cast<ItemIndex>(rItems.size()) )
    {
        for ( HL_Vector::const_iterator i = rItems.begin() + _nIndex;
              i != rItems.end();
              ++i, ++_nIndex
            )
        {
            RoadmapItem* pItem = *i;

            pItem->SetIndex( _nIndex );
            pItem->SetPosition( GetPreviousHyperLabel( _nIndex ) );
        }

    }
    if ( ! m_pImpl->isComplete() )
    {
        RoadmapItem* pOldItem = GetPreviousHyperLabel( m_pImpl->getItemCount() );
        m_pImpl->InCompleteHyperLabel->SetPosition( pOldItem );
        m_pImpl->InCompleteHyperLabel->Update( m_pImpl->getItemCount(), "..." );
    }
}

void ORoadmap::ReplaceRoadmapItem(ItemIndex Index, const OUString& roadmapItem, ItemId RMID, bool _bEnabled)
{
    RoadmapItem* pItem = GetByIndex( Index);
    if ( pItem != nullptr )
    {
        pItem->Update( Index, roadmapItem );
        pItem->SetID( RMID );
        pItem->Enable( _bEnabled );
    }
}

RoadmapTypes::ItemIndex ORoadmap::GetItemCount() const
{
    return m_pImpl->getItemCount();
}

RoadmapTypes::ItemId ORoadmap::GetItemID(ItemIndex _nIndex) const
{
    const RoadmapItem* pHyperLabel = GetByIndex( _nIndex );
    if ( pHyperLabel )
        return pHyperLabel->GetID();
    return -1;
}

void ORoadmap::InsertRoadmapItem(ItemIndex Index, const OUString& RoadmapItem, ItemId _nUniqueId, bool _bEnabled)
{
    InsertHyperLabel( Index, RoadmapItem, _nUniqueId, _bEnabled, false/*bIncomplete*/ );
    // TODO YPos is superfluous, if items are always appended
    UpdatefollowingHyperLabels( Index + 1 );
}

void ORoadmap::DeleteRoadmapItem(ItemIndex Index)
{
    if ( m_pImpl->getItemCount() > 0 && ( Index > -1)  &&  ( Index < m_pImpl->getItemCount() ) )
    {
        m_pImpl->removeHyperLabel( Index );
        UpdatefollowingHyperLabels( Index );
    }
}

bool ORoadmap::IsRoadmapComplete() const
{
    return m_pImpl->isComplete();
}

void ORoadmap::EnableRoadmapItem( ItemId _nItemId, bool _bEnable )
{
    RoadmapItem* pItem = GetByID( _nItemId );
    if ( pItem != nullptr )
        pItem->Enable( _bEnable );
}

void ORoadmap::ChangeRoadmapItemLabel( ItemId _nID, const OUString& _sLabel )
{
    RoadmapItem* pItem = GetByID( _nID );
    if ( pItem == nullptr )
        return;

    pItem->Update( pItem->GetIndex(), _sLabel );

    const HL_Vector& rItems = m_pImpl->getHyperLabels();
    size_t nPos = 0;
    for (auto const& item : rItems)
    {
        item->SetPosition( GetPreviousHyperLabel(nPos) );
        ++nPos;
    }
}

void ORoadmap::ChangeRoadmapItemID(ItemId _nID, ItemId NewID)
{
    RoadmapItem* pItem = GetByID( _nID );
    if ( pItem != nullptr )
        pItem->SetID( NewID );
}

RoadmapItem* ORoadmap::GetByID(ItemId _nID)
{
    ItemId nLocID = 0;
    const HL_Vector& rItems = m_pImpl->getHyperLabels();
    for (auto const& item : rItems)
    {
        nLocID = item->GetID();
        if ( nLocID == _nID )
            return item;
    }
    return nullptr;
}

const RoadmapItem* ORoadmap::GetByID(ItemId _nID) const
{
    return const_cast< ORoadmap* >( this )->GetByID( _nID );
}

RoadmapItem* ORoadmap::GetByIndex(ItemIndex _nItemIndex)
{
    const HL_Vector& rItems = m_pImpl->getHyperLabels();
    if ( ( _nItemIndex > -1 ) && ( _nItemIndex < static_cast<ItemIndex>(rItems.size()) ) )
    {
        return rItems.at( _nItemIndex );
    }
    return nullptr;
}

const RoadmapItem* ORoadmap::GetByIndex(ItemIndex _nItemIndex) const
{
    return const_cast< ORoadmap* >( this )->GetByIndex( _nItemIndex );
}

RoadmapTypes::ItemId ORoadmap::GetNextAvailableItemId(ItemIndex _nNewIndex)
{
    ItemIndex searchIndex = ++_nNewIndex;
    while ( searchIndex < m_pImpl->getItemCount() )
    {
        RoadmapItem* pItem = GetByIndex( searchIndex );
        if ( pItem->IsEnabled() )
            return pItem->GetID( );

        ++searchIndex;
    }
    return -1;
}

RoadmapTypes::ItemId ORoadmap::GetPreviousAvailableItemId(ItemIndex _nNewIndex)
{
    ItemIndex searchIndex = --_nNewIndex;
    while ( searchIndex > -1 )
    {
        RoadmapItem* pItem = GetByIndex( searchIndex );
        if ( pItem->IsEnabled() )
            return pItem->GetID( );

        searchIndex--;
    }
    return -1;
}

void ORoadmap::DeselectOldRoadmapItems()
{
    const HL_Vector& rItems = m_pImpl->getHyperLabels();
    for (auto const& item : rItems)
    {
        item->ToggleBackgroundColor( COL_TRANSPARENT );
    }
}

void ORoadmap::SetItemSelectHdl(const Link<LinkParamNone*,void>& _rHdl)
{
    m_pImpl->setSelectHdl(_rHdl);
}

Link<LinkParamNone*,void> const & ORoadmap::GetItemSelectHdl() const
{
    return m_pImpl->getSelectHdl();
}

void ORoadmap::Select()
{
    GetItemSelectHdl().Call( nullptr );
    CallEventListeners( VclEventId::RoadmapItemSelected );
}

void ORoadmap::GetFocus()
{
    RoadmapItem* pCurHyperLabel = GetByID( GetCurrentRoadmapItemID() );
    if ( pCurHyperLabel != nullptr )
        pCurHyperLabel->GrabFocus();
}

bool ORoadmap::SelectRoadmapItemByID( ItemId _nNewID )
{
    DeselectOldRoadmapItems();
    RoadmapItem* pItem = GetByID( _nNewID );
    if ( pItem != nullptr )
    {
        if ( pItem->IsEnabled() )
        {
            const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
            pItem->ToggleBackgroundColor( rStyleSettings.GetHighlightColor() ); //HighlightColor

            pItem->GrabFocus();
            m_pImpl->setCurItemID(_nNewID);

            Select();
            return true;
        }
    }
    return false;
}

void ORoadmap::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& _rRect)
{
    if (!m_pImpl->m_bPaintInitialized)
        implInit(rRenderContext);
    Control::Paint(rRenderContext, _rRect);

    // draw the bitmap
    if (!!m_pImpl->getPicture())
    {
        Size aBitmapSize = m_pImpl->getPicture().GetSizePixel();
        Size aMySize(GetOutputSizePixel());

        Point aBitmapPos(aMySize.Width() - aBitmapSize.Width(),  aMySize.Height() - aBitmapSize.Height());

        // draw it
        rRenderContext.DrawBitmapEx( aBitmapPos, m_pImpl->getPicture() );
    }

    // draw the headline
    DrawHeadline(rRenderContext);
}

void ORoadmap::DrawHeadline(vcl::RenderContext& rRenderContext)
{
    Point aTextPos = LogicToPixel(Point(ROADMAP_INDENT_X, 8), MapMode(MapUnit::MapAppFont));

    Size aOutputSize(GetOutputSizePixel());

    // draw it
    rRenderContext.DrawText(tools::Rectangle(aTextPos, aOutputSize), GetText(),
                            DrawTextFlags::Left | DrawTextFlags::Top | DrawTextFlags::MultiLine | DrawTextFlags::WordBreak);
    rRenderContext.DrawTextLine(aTextPos, aOutputSize.Width(), STRIKEOUT_NONE, LINESTYLE_SINGLE, LINESTYLE_NONE);
    const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();
    rRenderContext.SetLineColor(rStyleSettings.GetFieldTextColor());
    rRenderContext.SetTextColor(rStyleSettings.GetFieldTextColor());
}

RoadmapItem* ORoadmap::GetByPointer(vcl::Window const * pWindow)
{
    const HL_Vector& rItems = m_pImpl->getHyperLabels();
    for (auto const& item : rItems)
    {
        if ( item->Contains( pWindow ) )
            return item;
    }
    return nullptr;
}

bool ORoadmap::PreNotify(NotifyEvent& _rNEvt)
{
    // capture KeyEvents for taskpane cycling
    if ( _rNEvt.GetType() == MouseNotifyEvent::KEYINPUT )
    {
        vcl::Window* pWindow = _rNEvt.GetWindow();
        RoadmapItem* pItem = GetByPointer( pWindow );
        if ( pItem != nullptr )
        {
            sal_Int16 nKeyCode = _rNEvt.GetKeyEvent()->GetKeyCode().GetCode();
            switch( nKeyCode )
            {
                case KEY_UP:
                    {   // Note: Performancewise this is not optimal, because we search for an ID in the labels
                        //       and afterwards we search again for a label with the appropriate ID ->
                        //       unnecessarily we search twice!!!
                        ItemId nPrevItemID = GetPreviousAvailableItemId( pItem->GetIndex() );
                        if ( nPrevItemID != -1 )
                            return SelectRoadmapItemByID( nPrevItemID );
                    }
                    break;
                case KEY_DOWN:
                    {
                        ItemId nNextItemID = GetNextAvailableItemId( pItem->GetIndex() );
                        if ( nNextItemID != -1 )
                            return SelectRoadmapItemByID( nNextItemID );
                    }
                    break;
                case KEY_SPACE:
                    return SelectRoadmapItemByID( pItem->GetID() );
            }
        }
    }
    return Window::PreNotify( _rNEvt );
}

IMPL_LINK(ORoadmap, ImplClickHdl, HyperLabel*, CurHyperLabel, void)
{
   SelectRoadmapItemByID( CurHyperLabel->GetID() );
}

void ORoadmap::DataChanged(const DataChangedEvent& rDCEvt)
{
    if (!((( rDCEvt.GetType() == DataChangedEventType::SETTINGS )   ||
        ( rDCEvt.GetType() == DataChangedEventType::DISPLAY   ))  &&
        ( rDCEvt.GetFlags() & AllSettingsFlags::STYLE        )))
        return;

    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    SetBackground( Wallpaper( rStyleSettings.GetFieldColor() ) );
    Color aTextColor = rStyleSettings.GetFieldTextColor();
    vcl::Font aFont = GetFont();
    aFont.SetColor( aTextColor );
    SetFont( aFont );
    RoadmapTypes::ItemId curItemID = GetCurrentRoadmapItemID();
    RoadmapItem* pLabelItem = GetByID( curItemID );
    if (pLabelItem != nullptr)
    {
        pLabelItem->ToggleBackgroundColor(rStyleSettings.GetHighlightColor());
    }
    Invalidate();
}

void ORoadmap::ApplySettings(vcl::RenderContext& rRenderContext)
{
    const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();
    Color aTextColor = rStyleSettings.GetFieldTextColor();
    vcl::Font aFont = rRenderContext.GetFont();
    aFont.SetColor(aTextColor);
    aFont.SetWeight(WEIGHT_BOLD);
    aFont.SetUnderline(LINESTYLE_SINGLE);
    rRenderContext.SetFont(aFont);
    rRenderContext.SetBackground(rStyleSettings.GetFieldColor());
}

RoadmapItem::RoadmapItem(ORoadmap& _rParent, const Size& _rItemPlayground)
    : m_aItemPlayground(_rItemPlayground)
{
    mpID = VclPtr<IDLabel>::Create( &_rParent, WB_WORDBREAK );
    mpID->Show();
    mpDescription = VclPtr<HyperLabel>::Create( &_rParent, WB_NOTABSTOP | WB_WORDBREAK );
    mpDescription->Show();
}

RoadmapItem::~RoadmapItem()
{
    mpID.disposeAndClear();
    mpDescription.disposeAndClear();
}

bool RoadmapItem::Contains(const vcl::Window* _pWindow) const
{
    return ( mpID == _pWindow ) || ( mpDescription == _pWindow );
}

void RoadmapItem::GrabFocus()
{
    if ( mpDescription )
        mpDescription->GrabFocus();
}

void RoadmapItem::SetInteractive(bool _bInteractive)
{
    if ( mpDescription )
    mpDescription->SetInteractive(_bInteractive);
}

void RoadmapItem::SetID(sal_Int16 ID)
{
    if ( mpDescription )
        mpDescription->SetID(ID);
}

sal_Int16 RoadmapItem::GetID() const
{
    return mpDescription ? mpDescription->GetID() : sal_Int16(-1);
}

void RoadmapItem::ImplUpdateIndex(const ItemIndex _nIndex)
{
    mpDescription->SetIndex( _nIndex );

    OUString aIDText = OUString::number( _nIndex + 1 ) + ".";
    mpID->SetText( aIDText );

    // update the geometry of both controls
    ImplUpdatePosSize();
}

void RoadmapItem::SetIndex(ItemIndex Index)
{
    ImplUpdateIndex(Index);
}

RoadmapTypes::ItemIndex RoadmapItem::GetIndex() const
{
    return mpDescription ? mpDescription->GetIndex() : ItemIndex(-1);
}

void RoadmapItem::SetPosition(RoadmapItem const * _pOldItem)
{
    Point aIDPos;
    if ( _pOldItem == nullptr )
    {
        aIDPos = mpID->LogicToPixel(Point(ROADMAP_INDENT_X, ROADMAP_INDENT_Y), MapMode(MapUnit::MapAppFont));
    }
    else
    {
        Size aOldSize = _pOldItem->mpDescription->GetSizePixel();

        aIDPos = _pOldItem->mpID->GetPosPixel();
        aIDPos.AdjustY(aOldSize.Height() );
        aIDPos.AdjustY(mpID->GetParent()->LogicToPixel( Size( 0, ROADMAP_ITEM_DISTANCE_Y ) ).Height() );
    }
    mpID->SetPosPixel( aIDPos );

    sal_Int32 nDescPos = aIDPos.X() + mpID->GetSizePixel().Width();
    mpDescription->SetPosPixel( Point( nDescPos, aIDPos.Y() ) );
}

void RoadmapItem::Enable(bool _bEnable)
{
    mpID->Enable(_bEnable);
    mpDescription->Enable(_bEnable);
}

bool RoadmapItem::IsEnabled() const
{
    return mpID->IsEnabled();
}

void RoadmapItem::ToggleBackgroundColor(const Color& _rGBColor)
{
    if (_rGBColor == COL_TRANSPARENT)
        mpID->SetControlBackground();
    else
        mpID->SetControlBackground( mpID->GetSettings().GetStyleSettings().GetHighlightColor() );
    mpDescription->ToggleBackgroundColor(_rGBColor);
}

void RoadmapItem::ImplUpdatePosSize()
{
    // calculate widths
    long nIDWidth = mpID->GetTextWidth( mpID->GetText() );
    long nMaxIDWidth = mpID->GetTextWidth( "100." );
    nIDWidth = ::std::min( nIDWidth, nMaxIDWidth );

    // check how many space the description would need
    Size aDescriptionSize = mpDescription->CalcMinimumSize( m_aItemPlayground.Width() - nIDWidth );

    // position and size both controls
    Size aIDSize( nIDWidth, aDescriptionSize.Height() );
    mpID->SetSizePixel( aIDSize );

    Point aIDPos = mpID->GetPosPixel();
    mpDescription->SetPosPixel( Point( aIDPos.X() + nIDWidth, aIDPos.Y() ) );
    mpDescription->SetSizePixel( aDescriptionSize );
}

void RoadmapItem::Update(ItemIndex RMIndex, const OUString& _rText)
{
    // update description label
    mpDescription->SetLabel( _rText );

    // update the index in both controls, which triggers updating the geometry of both
    ImplUpdateIndex( RMIndex );
}

void RoadmapItem::SetClickHdl(const Link<HyperLabel*,void>& rLink)
{
    if ( mpDescription )
        mpDescription->SetClickHdl( rLink);
}

IDLabel::IDLabel(vcl::Window* _pParent, WinBits _nWinStyle)
    : FixedText(_pParent, _nWinStyle)
{
}

void IDLabel::ApplySettings(vcl::RenderContext& rRenderContext)
{
    FixedText::ApplySettings(rRenderContext);

    const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();
    if (GetControlBackground() == COL_TRANSPARENT)
        rRenderContext.SetTextColor(rStyleSettings.GetFieldTextColor());
    else
        rRenderContext.SetTextColor(rStyleSettings.GetHighlightTextColor());
}

void IDLabel::DataChanged(const DataChangedEvent& rDCEvt)
{
    FixedText::DataChanged( rDCEvt );

    if ((( rDCEvt.GetType() == DataChangedEventType::SETTINGS )   ||
        ( rDCEvt.GetType() == DataChangedEventType::DISPLAY   ))  &&
        ( rDCEvt.GetFlags() & AllSettingsFlags::STYLE        ))
    {
        const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
        if (GetControlBackground() != COL_TRANSPARENT)
            SetControlBackground(rStyleSettings.GetHighlightColor());
        Invalidate();
    }
}

}   // namespace svt

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
