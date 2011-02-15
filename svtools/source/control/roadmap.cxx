/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svtools.hxx"
#include <svtools/roadmap.hxx>

#ifndef _STRING_HXX
#define _STRING_HXX
#endif

#include <vector>
#include <algorithm>
#include <vcl/bitmap.hxx>
#include <tools/color.hxx>
#include <memory>

#define ROADMAP_INDENT_X        4
#define ROADMAP_INDENT_Y        27
#define ROADMAP_ITEM_DISTANCE_Y 6
#define RMINCOMPLETE        -1
#define NADDITEM            1
#define INCOMPLETELABEL     ::String::CreateFromAscii("...")        // TODO: Cast to String

//.........................................................................
namespace svt
{
//.........................................................................

    typedef std::vector< ::rtl::OUString > S_Vector;
    typedef std::vector< RoadmapItem* > HL_Vector;

    //=====================================================================
    //= ColorChanger
    //=====================================================================
    class IDLabel :  public FixedText
    {
    public:
        IDLabel( Window* _pParent, WinBits _nWinStyle = 0 );
        ~IDLabel( );
        virtual void    DataChanged( const DataChangedEvent& rDCEvt );
    };

    //=====================================================================
    //= ColorChanger
    //=====================================================================
    class ColorChanger
    {
    protected:
        OutputDevice*   m_pDev;

    public:
        ColorChanger( OutputDevice* _pDev, const Color& _rNewLineColor, const Color& _rNewFillColor )
            :m_pDev( _pDev )
        {
            m_pDev->Push( PUSH_LINECOLOR | PUSH_FILLCOLOR );
            m_pDev->SetLineColor( _rNewLineColor );
            m_pDev->SetFillColor( _rNewFillColor );
        }

        ~ColorChanger()
        {
            m_pDev->Pop();
        }
    };

    //=====================================================================
    //= RoadmapItem
    //=====================================================================
    class RoadmapItem : public RoadmapTypes
    {
    private:
        IDLabel*                mpID;
        HyperLabel*             mpDescription;
        const Size              m_aItemPlayground;

    public:
        RoadmapItem( ORoadmap& _rParent, const Size& _rItemPlayground );
        ~RoadmapItem( );

        void                    SetID( sal_Int16 _ID );
        sal_Int16               GetID() const;

        void                    SetIndex( ItemIndex _Index );
        ItemIndex               GetIndex() const;

        void                    SetLabel( const ::rtl::OUString& _rText );
        ::rtl::OUString         GetLabel( );

        void                    Update( ItemIndex _RMIndex, const ::rtl::OUString& _rText );

        void                    SetPosition( RoadmapItem* OldHyperLabel );

        void                    ToggleBackgroundColor( const Color& _rGBColor );
        void                    SetInteractive( sal_Bool _bInteractive );

        void                    SetClickHdl( const Link& rLink );
        const Link&             GetClickHdl() const;
        void                    SetZOrder( RoadmapItem* pRefRoadmapHyperLabel, sal_uInt16 nFlags );
        void                    Enable( sal_Bool bEnable = sal_True);
        sal_Bool                    IsEnabled() const;
        void                    GrabFocus();

        bool                    Contains( const Window* _pWindow ) const;

        HyperLabel*             GetDescriptionHyperLabel() const { return mpDescription; }

    private:
        void                    ImplUpdateIndex( const ItemIndex _nIndex );
        void                    ImplUpdatePosSize();
    };

    //=====================================================================
    //= RoadmapImpl
    //=====================================================================
    class RoadmapImpl : public RoadmapTypes
    {
    protected:
        const ORoadmap&     m_rAntiImpl;
        Link                m_aSelectHdl;
        BitmapEx            m_aPicture;
        HL_Vector           m_aRoadmapSteps;
        ItemId              m_iCurItemID;
        sal_Bool            m_bInteractive;
        sal_Bool            m_bComplete;
        Size                m_aItemSizePixel;

    public:
        RoadmapImpl( const ORoadmap& _rAntiImpl )
            :m_rAntiImpl( _rAntiImpl )
            ,m_iCurItemID( -1 )
            ,m_bInteractive( sal_True )
            ,m_bComplete( sal_True )
        {
        }

        RoadmapItem* InCompleteHyperLabel;

        void                addHyperLabel( RoadmapItem*  _rRoadmapStep ) { m_aRoadmapSteps.push_back(_rRoadmapStep); }

        HL_Vector&          getHyperLabels() { return m_aRoadmapSteps; }
        const HL_Vector&    getHyperLabels() const { return m_aRoadmapSteps; }

        void                insertHyperLabel( ItemIndex _Index, RoadmapItem* _rRoadmapStep ) { m_aRoadmapSteps.insert( m_aRoadmapSteps.begin() + _Index, _rRoadmapStep ); }

        ItemIndex           getItemCount() const { return m_aRoadmapSteps.size();}

        void                setCurItemID( ItemId i ) {m_iCurItemID = i; }
        ItemId              getCurItemID() const { return m_iCurItemID; }

        void                setInteractive(const sal_Bool _bInteractive) {m_bInteractive = _bInteractive; }
        sal_Bool            isInteractive() const { return m_bInteractive; };

        void                setComplete(const sal_Bool _bComplete) {m_bComplete = _bComplete; }
        sal_Bool            isComplete() const { return m_bComplete; };

        void                setPicture( const BitmapEx& _rPic ) { m_aPicture = _rPic; }
        const BitmapEx&     getPicture( ) const { return m_aPicture; }

        void                setSelectHdl( const Link& _rHdl ) { m_aSelectHdl = _rHdl; }
        const Link&         getSelectHdl( ) const { return m_aSelectHdl; }

        void                initItemSize();
        const Size&         getItemSize() const { return m_aItemSizePixel; }

        void removeHyperLabel( ItemIndex _Index )
        {
            if ( ( _Index > -1 ) && ( _Index < getItemCount() ) )
            {
                delete m_aRoadmapSteps[_Index];
                m_aRoadmapSteps.erase( m_aRoadmapSteps.begin() + _Index);
            }
        }
    };


    //=====================================================================
    //= Roadmap
    //=====================================================================
    //---------------------------------------------------------------------
    void RoadmapImpl::initItemSize()
    {
        Size aLabelSize( m_rAntiImpl.GetOutputSizePixel() );
        aLabelSize.Height() = m_rAntiImpl.LogicToPixel( Size( 0, LABELBASEMAPHEIGHT ), MAP_APPFONT ).Height();
        aLabelSize.Width() -= m_rAntiImpl.LogicToPixel( Size( 2 * ROADMAP_INDENT_X, 0 ), MAP_APPFONT ).Width();
        m_aItemSizePixel = aLabelSize;
    }

    //=====================================================================
    //= Roadmap
    //=====================================================================
    //---------------------------------------------------------------------
    ORoadmap::ORoadmap( Window* _pParent, const ResId& _rId )
        :Control( _pParent, _rId )
        ,m_pImpl( new RoadmapImpl( *this ) )
    {
        implInit();
    }

    //---------------------------------------------------------------------
    ORoadmap::ORoadmap( Window* _pParent, WinBits _nWinStyle )
        :Control( _pParent, _nWinStyle )
        ,m_pImpl( new RoadmapImpl( *this ) )

    {
        implInit();
    }

    //---------------------------------------------------------------------
    void ORoadmap::implInit()
    {
        const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
        Color aTextColor = rStyleSettings.GetFieldTextColor();
        Font aFont = GetFont( );
        aFont.SetColor( aTextColor );
        aFont.SetWeight( WEIGHT_BOLD );
        aFont.SetUnderline( UNDERLINE_SINGLE );
        SetFont( aFont );
        SetBackground( Wallpaper( rStyleSettings.GetFieldColor() ) );
        m_pImpl->InCompleteHyperLabel = NULL;
        m_pImpl->setCurItemID(-1 );
        m_pImpl->setComplete( sal_True );

        // Roadmap control should be reachable as one unit with a Tab key
        // the next Tab key should spring out of the control.
        // To reach it the control itself should get focus and set it
        // on entries. The entries themself should not be reachable with
        // the Tab key directly. So each entry should have WB_NOTABSTOP.
        //
        // In other words the creator should create the control with the following
        // flags:
        // SetStyle( ( GetStyle() | WB_TABSTOP ) & ~WB_DIALOGCONTROL );

// TODO: if somebody sets a new font from outside (OutputDevice::SetFont), we would have to react
// on this with calculating a new bold font.
// Unfortunately, the OutputDevice does not offer a notify mechanism for a changed font.
// So settings the font from outside is simply a forbidded scenario at the moment
        EnableMapMode( sal_False );
    }

    //---------------------------------------------------------------------
    ORoadmap::~ORoadmap( )
    {
        HL_Vector aItemsCopy = m_pImpl->getHyperLabels();
        m_pImpl->getHyperLabels().clear();
        for ( HL_Vector::iterator i = aItemsCopy.begin(); i< aItemsCopy.end(); ++i )
        {
            delete *i;
        }
        if ( ! m_pImpl->isComplete() )
            delete m_pImpl->InCompleteHyperLabel;
        delete m_pImpl;
        m_pImpl = NULL;
    }


    RoadmapTypes::ItemId ORoadmap::GetCurrentRoadmapItemID() const
    {
        return m_pImpl->getCurItemID();
    }


    RoadmapItem* ORoadmap::GetPreviousHyperLabel( ItemIndex _Index)
    {
        RoadmapItem* pOldItem = NULL;
        if ( _Index > 0 )
            pOldItem = m_pImpl->getHyperLabels().at( _Index - 1 );
        return pOldItem;
    }


       //---------------------------------------------------------------------

    RoadmapItem* ORoadmap::InsertHyperLabel( ItemIndex _Index, const ::rtl::OUString& _sLabel, ItemId _RMID, sal_Bool _bEnabled)
    {
        if ( m_pImpl->getItemCount() == 0 )
            m_pImpl->initItemSize();

        RoadmapItem* pItem = NULL;
        RoadmapItem* pOldItem = GetPreviousHyperLabel( _Index );

        pItem = new RoadmapItem( *this, m_pImpl->getItemSize() );
        if ( _RMID != RMINCOMPLETE )
        {
            pItem->SetInteractive( m_pImpl->isInteractive() );
            m_pImpl->insertHyperLabel( _Index, pItem );
        }
        else
        {
            pItem->SetInteractive( sal_False );
        }
        pItem->SetPosition( pOldItem );
        pItem->Update( _Index, _sLabel );
        pItem->SetClickHdl(LINK( this, ORoadmap, ImplClickHdl ) );
        pItem->SetID( _RMID );
        pItem->SetIndex( _Index );
        if (!_bEnabled)
            pItem->Enable( _bEnabled );
        return pItem;
    }

    //---------------------------------------------------------------------
    void ORoadmap::SetRoadmapBitmap( const BitmapEx& _rBmp, sal_Bool _bInvalidate )
    {
        m_pImpl->setPicture( _rBmp );
        if ( _bInvalidate )
            Invalidate( );
    }

    //---------------------------------------------------------------------
    const BitmapEx& ORoadmap::GetRoadmapBitmap( ) const
    {
        return m_pImpl->getPicture( );
    }

    //---------------------------------------------------------------------
    void ORoadmap::SetRoadmapInteractive( sal_Bool _bInteractive )
    {
        m_pImpl->setInteractive( _bInteractive );

        const HL_Vector& rItems = m_pImpl->getHyperLabels();
        for (   HL_Vector::const_iterator i = rItems.begin();
                i < rItems.end();
                ++i
            )
        {
            (*i)->SetInteractive( _bInteractive );
        }
    }

    //---------------------------------------------------------------------
    sal_Bool ORoadmap::IsRoadmapInteractive()
    {
        return m_pImpl->isInteractive();
    }

    //---------------------------------------------------------------------
    void ORoadmap::SetRoadmapComplete( sal_Bool _bComplete )
    {
        sal_Bool bWasComplete = m_pImpl->isComplete();
        m_pImpl->setComplete( _bComplete );
        if ( _bComplete )
        {
            if ( m_pImpl->InCompleteHyperLabel != NULL)
            {
                delete m_pImpl->InCompleteHyperLabel;
                m_pImpl->InCompleteHyperLabel = NULL;
            }
        }
        else if ( bWasComplete )
            m_pImpl->InCompleteHyperLabel = InsertHyperLabel( m_pImpl->getItemCount(), ::String::CreateFromAscii( "..." ), RMINCOMPLETE );
    }

    //---------------------------------------------------------------------
    void ORoadmap::UpdatefollowingHyperLabels( ItemIndex _nIndex )
    {
        const HL_Vector& rItems = m_pImpl->getHyperLabels();
        if ( _nIndex < (ItemIndex)rItems.size() )
        {
            RoadmapItem* pItem = NULL;
            for (   HL_Vector::const_iterator i = rItems.begin() + _nIndex;
                    i< rItems.end();
                    ++i, ++_nIndex
                )
            {
                pItem = *i;

                pItem->SetIndex( _nIndex );
                pItem->SetPosition( GetPreviousHyperLabel( _nIndex ) );
            }
        }
        if ( ! m_pImpl->isComplete() )
        {
            RoadmapItem* pOldItem = GetPreviousHyperLabel( m_pImpl->getItemCount() );
            m_pImpl->InCompleteHyperLabel->SetPosition( pOldItem );
            m_pImpl->InCompleteHyperLabel->Update( m_pImpl->getItemCount(), ::String::CreateFromAscii("...") );
        }
    }

    //---------------------------------------------------------------------
    void ORoadmap::ReplaceRoadmapItem( ItemIndex _Index, const ::rtl::OUString& _RoadmapItem, ItemId _RMID, sal_Bool _bEnabled )
    {
        RoadmapItem* pItem = GetByIndex( _Index);
        if ( pItem != NULL )
        {
            pItem->Update( _Index,  _RoadmapItem );
            pItem->SetID( _RMID );
            pItem->Enable( _bEnabled );
        }
    }

    //---------------------------------------------------------------------
    RoadmapTypes::ItemIndex ORoadmap::GetItemCount() const
    {
        return m_pImpl->getItemCount();
    }

    //---------------------------------------------------------------------
    RoadmapTypes::ItemId ORoadmap::GetItemID( ItemIndex _nIndex ) const
    {
        const RoadmapItem* pHyperLabel = GetByIndex( _nIndex );
        if ( pHyperLabel )
            return pHyperLabel->GetID();
        return -1;
    }

    //---------------------------------------------------------------------
    RoadmapTypes::ItemIndex ORoadmap::GetItemIndex( ItemId _nID ) const
    {
        ItemId nLocID = 0;
        const HL_Vector& rItems = m_pImpl->getHyperLabels();
        for (   HL_Vector::const_iterator i = rItems.begin();
                i < rItems.end();
                ++i
            )
        {
            nLocID = (*i)->GetID();
            if ( nLocID == _nID )
                return ItemIndex( i - rItems.begin() );
        }
        return -1;
    }

    //---------------------------------------------------------------------
    void ORoadmap::InsertRoadmapItem( ItemIndex _Index, const ::rtl::OUString& _RoadmapItem, ItemId _nUniqueId, sal_Bool _bEnabled )
    {
        InsertHyperLabel( _Index, _RoadmapItem, _nUniqueId, _bEnabled );
            // Todo: YPos is superfluous, if items are always appended
        UpdatefollowingHyperLabels( _Index + 1 );
    }

    //---------------------------------------------------------------------
    void ORoadmap::DeleteRoadmapItem( ItemIndex _Index )
    {
        if ( m_pImpl->getItemCount() > 0 && ( _Index > -1)  &&  ( _Index < m_pImpl->getItemCount() ) )
        {
            m_pImpl->removeHyperLabel( _Index );
            UpdatefollowingHyperLabels( _Index );
        }
    }

    //---------------------------------------------------------------------
    sal_Bool ORoadmap::IsRoadmapComplete( ) const
    {
        return m_pImpl->isComplete();
    }

    //---------------------------------------------------------------------
    sal_Bool ORoadmap::IsRoadmapItemEnabled( ItemId _nItemId, ItemIndex _nStartIndex  ) const
    {
        const RoadmapItem* _pLabelItem = GetByID( _nItemId, _nStartIndex  );
        return _pLabelItem ? _pLabelItem->IsEnabled() : sal_False;
    }

    //---------------------------------------------------------------------
    void ORoadmap::EnableRoadmapItem( ItemId _nItemId, sal_Bool _bEnable, ItemIndex _nStartIndex )
    {
        RoadmapItem* pItem = GetByID( _nItemId, _nStartIndex );
        if ( pItem != NULL )
            pItem->Enable( _bEnable );
    }

    //---------------------------------------------------------------------
    void ORoadmap::ChangeRoadmapItemLabel( ItemId _nID, const ::rtl::OUString& _sLabel, ItemIndex _nStartIndex )
    {
        RoadmapItem* pItem = GetByID( _nID, _nStartIndex );
        if ( pItem != NULL )
        {
            pItem->Update( pItem->GetIndex(), _sLabel );

            const HL_Vector& rItems = m_pImpl->getHyperLabels();
            for (   HL_Vector::const_iterator i = rItems.begin() + _nStartIndex;
                    i < rItems.end();
                    ++i
                )
            {
                (*i)->SetPosition( GetPreviousHyperLabel( i - rItems.begin() ) );
            }
        }
    }

    //---------------------------------------------------------------------

    ::rtl::OUString ORoadmap::GetRoadmapItemLabel( ItemId _nID, ItemIndex _nStartIndex )
    {
        RoadmapItem* pItem = GetByID( _nID, _nStartIndex );
        if ( pItem != NULL )
            return pItem->GetLabel();
        else
            return ::rtl::OUString();
    }

    //---------------------------------------------------------------------
    void ORoadmap::ChangeRoadmapItemID( ItemId _nID, ItemId _NewID, ItemIndex _nStartIndex )
    {
        RoadmapItem* pItem = GetByID( _nID, _nStartIndex );
        if ( pItem != NULL )
            pItem->SetID( _NewID );
    }

    //---------------------------------------------------------------------
    RoadmapItem* ORoadmap::GetByID( ItemId _nID, ItemIndex _nStartIndex)
    {
        ItemId nLocID = 0;
        const HL_Vector& rItems = m_pImpl->getHyperLabels();
        for (   HL_Vector::const_iterator i = rItems.begin() + _nStartIndex;
                i < rItems.end();
                ++i
            )
        {
            nLocID = (*i)->GetID();
            if ( nLocID == _nID )
                return *i;
        }
        return NULL;
    }

    //---------------------------------------------------------------------
    const RoadmapItem* ORoadmap::GetByID( ItemId _nID, ItemIndex _nStartIndex  ) const
    {
        return const_cast< ORoadmap* >( this )->GetByID( _nID, _nStartIndex );
    }

    //---------------------------------------------------------------------
    RoadmapItem* ORoadmap::GetByIndex( ItemIndex _nItemIndex)
    {
        const HL_Vector& rItems = m_pImpl->getHyperLabels();
        if ( ( _nItemIndex > -1 ) && ( _nItemIndex < (ItemIndex)rItems.size() ) )
        {
            return rItems.at( _nItemIndex );
        }
        return NULL;
    }

    //---------------------------------------------------------------------
    const RoadmapItem* ORoadmap::GetByIndex( ItemIndex _nItemIndex ) const
    {
        return const_cast< ORoadmap* >( this )->GetByIndex( _nItemIndex );
    }

    //---------------------------------------------------------------------
    RoadmapTypes::ItemId ORoadmap::GetNextAvailableItemId( ItemIndex _nNewIndex )
    {
        RoadmapItem* pItem = NULL;

        ItemIndex searchIndex = ++_nNewIndex;
        while ( searchIndex < m_pImpl->getItemCount() )
        {
            pItem = GetByIndex( searchIndex );
            if ( pItem->IsEnabled() )
                return pItem->GetID( );

            ++searchIndex;
        }
        return -1;
    }

    //---------------------------------------------------------------------
    RoadmapTypes::ItemId ORoadmap::GetPreviousAvailableItemId( ItemIndex _nNewIndex )
    {
        RoadmapItem* pItem = NULL;
        ItemIndex searchIndex = --_nNewIndex;
        while ( searchIndex > -1 )
        {
            pItem = GetByIndex( searchIndex );
            if ( pItem->IsEnabled() )
                return pItem->GetID( );

            searchIndex--;
        }
        return -1;
    }

    //---------------------------------------------------------------------
    void ORoadmap::DeselectOldRoadmapItems()
    {
        const HL_Vector& rItems = m_pImpl->getHyperLabels();
        for (   HL_Vector::const_iterator i = rItems.begin();
                i < rItems.end();
                ++i
            )
        {
            (*i)->ToggleBackgroundColor( COL_TRANSPARENT );
        }
    }

    //---------------------------------------------------------------------
    void ORoadmap::SetItemSelectHdl( const Link& _rHdl )
    {
        m_pImpl->setSelectHdl( _rHdl );
    }

    //---------------------------------------------------------------------
    Link ORoadmap::GetItemSelectHdl( ) const
    {
        return m_pImpl->getSelectHdl();
    }

    //---------------------------------------------------------------------
    void ORoadmap::Select()
    {
        GetItemSelectHdl().Call( this );
        CallEventListeners( VCLEVENT_ROADMAP_ITEMSELECTED );
    }

    //---------------------------------------------------------------------
    void ORoadmap::GetFocus()
    {
        RoadmapItem* pCurHyperLabel = GetByID( GetCurrentRoadmapItemID() );
        if ( pCurHyperLabel != NULL )
            pCurHyperLabel->GrabFocus();
    }

    //---------------------------------------------------------------------
    sal_Bool ORoadmap::SelectRoadmapItemByID( ItemId _nNewID )
    {
        DeselectOldRoadmapItems();
        RoadmapItem* pItem = GetByID( _nNewID );
        if ( pItem != NULL )
        {
            if ( pItem->IsEnabled() )
            {
                const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
                pItem->ToggleBackgroundColor( rStyleSettings.GetHighlightColor() ); //HighlightColor

                pItem->GrabFocus();
                m_pImpl->setCurItemID(_nNewID);

                Select();
                return sal_True;
            }
        }
        return sal_False;
    }

    //---------------------------------------------------------------------
    void ORoadmap::Paint( const Rectangle& _rRect )
    {
        Control::Paint( _rRect );


        // draw the bitmap
        if ( !!m_pImpl->getPicture() )
        {
            Size aBitmapSize = m_pImpl->getPicture().GetSizePixel();
            Size aMySize = GetOutputSizePixel();

            Point aBitmapPos( aMySize.Width() - aBitmapSize.Width(),  aMySize.Height() - aBitmapSize.Height() );

            // draw it
            DrawBitmapEx( aBitmapPos, m_pImpl->getPicture() );
        }

        //.................................................................
        // draw the headline
        DrawHeadline();
    }

    //---------------------------------------------------------------------
    void ORoadmap::DrawHeadline()
    {
        Point aTextPos = LogicToPixel( Point( ROADMAP_INDENT_X, 8 ), MAP_APPFONT );

        Size aOutputSize( GetOutputSizePixel() );

        // draw it
        DrawText( Rectangle( aTextPos, aOutputSize ), GetText(), TEXT_DRAW_LEFT | TEXT_DRAW_TOP | TEXT_DRAW_MULTILINE | TEXT_DRAW_WORDBREAK );
        DrawTextLine( aTextPos, aOutputSize.Width(), STRIKEOUT_NONE, UNDERLINE_SINGLE, UNDERLINE_NONE, sal_False );
        const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
        SetLineColor( rStyleSettings.GetFieldTextColor());
        SetTextColor(rStyleSettings.GetFieldTextColor());
    }

    //---------------------------------------------------------------------
    RoadmapItem* ORoadmap::GetByPointer(Window* pWindow)
    {
        const HL_Vector& rItems = m_pImpl->getHyperLabels();
        for (   HL_Vector::const_iterator i = rItems.begin();
                i < rItems.end();
                ++i
            )
        {
            if ( (*i)->Contains( pWindow ) )
                return *i;
        }
        return NULL;
    }

    //---------------------------------------------------------------------
    long ORoadmap::PreNotify( NotifyEvent& _rNEvt )
    {
        // capture KeyEvents for taskpane cycling
        if ( _rNEvt.GetType() == EVENT_KEYINPUT )
        {
            Window* pWindow = _rNEvt.GetWindow();
            RoadmapItem* pItem = GetByPointer( pWindow );
            if ( pItem != NULL )
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

    //---------------------------------------------------------------------
      IMPL_LINK(ORoadmap, ImplClickHdl, HyperLabel*, _CurHyperLabel)
    {
       return SelectRoadmapItemByID( _CurHyperLabel->GetID() );
    }



    //---------------------------------------------------------------------
    void ORoadmap::DataChanged( const DataChangedEvent& rDCEvt )
    {
        if ((( rDCEvt.GetType() == DATACHANGED_SETTINGS )   ||
            ( rDCEvt.GetType() == DATACHANGED_DISPLAY   ))  &&
            ( rDCEvt.GetFlags() & SETTINGS_STYLE        ))
        {
            const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
            SetBackground( Wallpaper( rStyleSettings.GetFieldColor() ) );
            Color aTextColor = rStyleSettings.GetFieldTextColor();
            Font aFont = GetFont();
            aFont.SetColor( aTextColor );
            SetFont( aFont );
            RoadmapTypes::ItemId curItemID = GetCurrentRoadmapItemID();
            RoadmapItem* pLabelItem = GetByID( curItemID );
            pLabelItem->ToggleBackgroundColor(rStyleSettings.GetHighlightColor());
            Invalidate();
        }
    }


    //---------------------------------------------------------------------
    RoadmapItem::RoadmapItem( ORoadmap& _rParent, const Size& _rItemPlayground )
        :m_aItemPlayground( _rItemPlayground )
    {
        mpID = new IDLabel( &_rParent, WB_WORDBREAK );
        mpID->SetTextColor( mpID->GetSettings().GetStyleSettings().GetFieldTextColor( ) );
        mpID->Show();
        mpDescription = new HyperLabel( &_rParent, WB_NOTABSTOP | WB_WORDBREAK );
        mpDescription->Show();
    }

    //---------------------------------------------------------------------
    bool RoadmapItem::Contains( const Window* _pWindow ) const
    {
        return ( mpID == _pWindow ) || ( mpDescription == _pWindow );
    }

    //---------------------------------------------------------------------
    void RoadmapItem::GrabFocus()
    {
        if ( mpDescription )
            mpDescription->GrabFocus();
    }

    //---------------------------------------------------------------------
    void RoadmapItem::SetInteractive( sal_Bool _bInteractive )
    {
        if ( mpDescription )
        mpDescription->SetInteractive(_bInteractive);
    }

    //---------------------------------------------------------------------
    void RoadmapItem::SetID( sal_Int16 _ID )
    {
        if ( mpDescription )
            mpDescription->SetID(_ID);
    }

    //---------------------------------------------------------------------
    sal_Int16 RoadmapItem::GetID() const
    {
        return mpDescription ? mpDescription->GetID() : sal_Int16(-1);
    }

    //---------------------------------------------------------------------
    void RoadmapItem::ImplUpdateIndex( const ItemIndex _nIndex )
    {
        if ( mpDescription )
            mpDescription->SetIndex( _nIndex );

        if ( mpID )
        {
            ::rtl::OUString aIDText = ::rtl::OUString::valueOf( (sal_Int32)( _nIndex + 1 ) ) +  ::rtl::OUString::createFromAscii( "." );
             mpID->SetText( aIDText );
        }

        // update the geometry of both controls
        ImplUpdatePosSize();
    }

    //---------------------------------------------------------------------
    void RoadmapItem::SetIndex( ItemIndex _Index )
    {
        ImplUpdateIndex( _Index );
    }

    //---------------------------------------------------------------------
    RoadmapTypes::ItemIndex RoadmapItem::GetIndex() const
    {
        return mpDescription ? mpDescription->GetIndex() : ItemIndex(-1);
    }

    //---------------------------------------------------------------------
    void RoadmapItem::SetLabel( const ::rtl::OUString& _rText )
    {
        if ( mpDescription )
            mpDescription->SetText(_rText);
    }

    //---------------------------------------------------------------------
    ::rtl::OUString RoadmapItem::GetLabel( )
    {
        return mpDescription ? mpDescription->GetText() : String();
    }

    //---------------------------------------------------------------------
    void RoadmapItem::SetPosition( RoadmapItem* _pOldItem )
    {
        Point aIDPos;
        if ( _pOldItem == NULL )
        {
            aIDPos = mpID->LogicToPixel( Point( ROADMAP_INDENT_X, ROADMAP_INDENT_Y ), MAP_APPFONT );
        }
        else
        {
            Size aOldSize = _pOldItem->GetDescriptionHyperLabel()->GetSizePixel();

            aIDPos = _pOldItem->mpID->GetPosPixel();
            aIDPos.Y() += aOldSize.Height();
            aIDPos.Y() += mpID->GetParent()->LogicToPixel( Size( 0, ROADMAP_ITEM_DISTANCE_Y ) ).Height();
        }
        mpID->SetPosPixel( aIDPos );

        sal_Int32 nDescPos = aIDPos.X() + mpID->GetSizePixel().Width();
        mpDescription->SetPosPixel( Point( nDescPos, aIDPos.Y() ) );
    }

    //---------------------------------------------------------------------
    void RoadmapItem::SetZOrder( RoadmapItem* pRefRoadmapHyperLabel, sal_uInt16 nFlags )
    {
        if (pRefRoadmapHyperLabel == NULL)
            mpDescription->SetZOrder( NULL, nFlags); //WINDOW_ZORDER_FIRST );
        else
            mpDescription->SetZOrder( pRefRoadmapHyperLabel->mpDescription, nFlags); //, WINDOW_ZORDER_BEHIND );
    }

    //---------------------------------------------------------------------
    void RoadmapItem::Enable( sal_Bool _bEnable)
    {
        mpID->Enable(_bEnable);
        mpDescription->Enable(_bEnable);
    }

    //---------------------------------------------------------------------
    sal_Bool RoadmapItem::IsEnabled() const
    {
        return mpID->IsEnabled();
    }

    //---------------------------------------------------------------------
    void RoadmapItem::ToggleBackgroundColor( const Color& _rGBColor )
    {
        if (_rGBColor == COL_TRANSPARENT)
        {
            mpID->SetTextColor( mpID->GetSettings().GetStyleSettings().GetFieldTextColor( ) );
            mpID->SetControlBackground( COL_TRANSPARENT );
        }
        else
        {
            mpID->SetControlBackground( mpID->GetSettings().GetStyleSettings().GetHighlightColor() );
            mpID->SetTextColor( mpID->GetSettings().GetStyleSettings().GetHighlightTextColor( ) );
        }
        mpDescription->ToggleBackgroundColor(_rGBColor);
    }

    //---------------------------------------------------------------------
    void RoadmapItem::ImplUpdatePosSize()
    {
        // calculate widths
        long nIDWidth = mpID->GetTextWidth( mpID->GetText() );
        long nMaxIDWidth = mpID->GetTextWidth( ::rtl::OUString::createFromAscii( "100." ) );
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

    //---------------------------------------------------------------------
    void RoadmapItem::Update( ItemIndex _RMIndex, const ::rtl::OUString& _rText )
    {
        // update description label
        mpDescription->SetLabel( _rText );

        // update the index in both controls, which triggers updating the geometry of both
        ImplUpdateIndex( _RMIndex );
    }

    //---------------------------------------------------------------------
    RoadmapItem::~RoadmapItem( )
    {
        {
            ::std::auto_ptr<Control> aTemp(mpID);
            mpID = NULL;
        }
        {
            ::std::auto_ptr<Control> aTemp(mpDescription);
            mpDescription = NULL;
        }
    }

    //---------------------------------------------------------------------
    void RoadmapItem::SetClickHdl( const Link& rLink )
    {
        if ( mpDescription )
            mpDescription->SetClickHdl( rLink);
    }

    //---------------------------------------------------------------------
    const Link& RoadmapItem::GetClickHdl( ) const
    {
        return mpDescription->GetClickHdl();
    }

    //---------------------------------------------------------------------
    IDLabel::IDLabel( Window* _pParent, WinBits _nWinStyle )
        :FixedText( _pParent, _nWinStyle )
    {

    }

    //---------------------------------------------------------------------
    IDLabel::~IDLabel( )
    {
    }

    //---------------------------------------------------------------------
    void IDLabel::DataChanged( const DataChangedEvent& rDCEvt )
    {
        const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
        FixedText::DataChanged( rDCEvt );
        if ((( rDCEvt.GetType() == DATACHANGED_SETTINGS )   ||
            ( rDCEvt.GetType() == DATACHANGED_DISPLAY   ))  &&
            ( rDCEvt.GetFlags() & SETTINGS_STYLE        ))
        {
            const Color& rGBColor = GetControlBackground();
            if (rGBColor == COL_TRANSPARENT)
                SetTextColor( rStyleSettings.GetFieldTextColor( ) );
            else
            {
                SetControlBackground(rStyleSettings.GetHighlightColor());
                SetTextColor( rStyleSettings.GetHighlightTextColor( ) );
            }
            Invalidate();
        }
    }




//.........................................................................
}   // namespace svt
//.........................................................................
