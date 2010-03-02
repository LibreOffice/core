/*************************************************************************
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

#include "precompiled_svtools.hxx"

#include "svtools/toolpanel/toolpaneldeck.hxx"
#include "paneltabbar.hxx"

#include <vcl/help.hxx>

#include <boost/optional.hpp>
#include <vector>

// space around an item
#define ITEM_OUTER_SPACE        2 * 2
// spacing before and after (in writing direction, whether this is horizontal or vertical) an item's text
#define ITEM_TEXT_FLOW_SPACE    2
// distance between two items
#define ITEM_DISTANCE_PIXEL     2
// space between item icon and icon text
#define ITEM_ICON_TEXT_DISTANCE 4
// outer space (aka margin) of the complete tab bar control
#define TAB_BAR_OUTER_SPACE     2

//........................................................................
namespace svt
{
//........................................................................

    typedef sal_uInt16  ItemFlags;

    #define ITEM_STATE_NORMAL   0x00
    #define ITEM_STATE_ACTIVE   0x01
    #define ITEM_STATE_HOVERED  0x02
    #define ITEM_STATE_FOCUSED  0x04

    //==================================================================================================================
    //= ItemDescriptor
    //==================================================================================================================
    struct ItemDescriptor
    {
        PToolPanel  pPanel;
        Rectangle   aMinArea;
        Rectangle   aPrefArea;
        bool        bUseMinimal;

        ItemDescriptor()
            :pPanel()
            ,aMinArea()
            ,aPrefArea()
            ,bUseMinimal( false )
        {
        }

        const Rectangle& GetCurrentRect() const
        {
            return bUseMinimal ? aMinArea : aPrefArea;
        }
    };

    typedef ::std::vector< ItemDescriptor > ItemDescriptors;

    //==================================================================================================================
    //= IItemsLayout
    //==================================================================================================================
    class IItemsLayout
    {
    public:
        /** calculates the size of the area occupied by the item representing the given tool panel
        */
        virtual Size    CalculateItemSize( const PToolPanel& i_pPanel, const OutputDevice& i_rReferenceDevice, const bool i_bMinimum ) const = 0;

        /** returns the position where the next item should start, assuming the previous item occupies a given area
        */
        virtual Point   GetNextItemPosition( const Rectangle& i_rPreviousItemArea ) const = 0;

        /** draws the item onto the given device, in the givem area
            @param i_pPanel
                the panel whose item representation is to be drawn
            @param i_rTargetWindow
                the window onto which to drawa
            @param i_rItemRect
                the area to paint the item into. The caller is responsible for ensuring that is has a size sufficient to
                hold the item (which can be determined by calling CalculateItemSize beforehand).
            @param i_nItemFlags
                defines in which state to draw the item
            @param i_bDrawMinimal
                defines whether the minimal version of the item should be drawn
        */
        virtual void    DrawItem( const PToolPanel& i_pPanel, Window& i_rTargetWindow, const Rectangle& i_rItemRect,
                            const ItemFlags i_nItemFlags, const bool i_bDrawMinimal ) = 0;

        /** updates the given items to use their minimal or optimal size, so they fit (if possible) into the given
            area.
        */
        virtual void    FitItemRects( ItemDescriptors& i_rItems, const Rectangle& i_rFitInto ) = 0;
    };

    typedef ::boost::shared_ptr< IItemsLayout >  PItemsLayout;

    //==================================================================================================================
    //= VerticalItemLayout
    //==================================================================================================================
    class VerticalItemLayout : public IItemsLayout
    {
    public:
        VerticalItemLayout()
        {
        }

        // IItemsLayout overridables
        virtual Size    CalculateItemSize( const PToolPanel& i_pPanel, const OutputDevice& i_rReferenceDevice, const bool i_bMinimum ) const;
        virtual Point   GetNextItemPosition( const Rectangle& i_rPreviousItemArea ) const;
        virtual void    DrawItem( const PToolPanel& i_pPanel, Window& i_rTargetWindow, const Rectangle& i_rItemRect,
                            const ItemFlags i_nItemFlags, const bool i_bDrawMinimal );
        virtual void    FitItemRects( ItemDescriptors& i_rItems, const Rectangle& i_rFitInto );
    };

    //------------------------------------------------------------------------------------------------------------------
    Size VerticalItemLayout::CalculateItemSize( const PToolPanel& i_pPanel, const OutputDevice& i_rReferenceDevice, const bool i_bMinimum ) const
    {
        const Image aImage( i_pPanel->GetImage() );
        const ::rtl::OUString sItemText( i_pPanel->GetDisplayName() );

        // for the moment, we display the icons only
        Size aItemSize;
        if ( !!aImage )
        {
            aItemSize = aImage.GetSizePixel();
        }

        if ( !i_bMinimum && sItemText.getLength() )
        {
            if ( !!aImage )
                aItemSize.Height() += ITEM_ICON_TEXT_DISTANCE;

            // add space for vertical text
            const Size aTextSize( i_rReferenceDevice.GetCtrlTextWidth( sItemText ), i_rReferenceDevice.GetTextHeight() );
            aItemSize.Height() += aTextSize.Width();
            aItemSize.Width() = ::std::max( aItemSize.Width(), aTextSize.Height() );

            aItemSize.Height() += 2 * ITEM_TEXT_FLOW_SPACE;
        }

        aItemSize.Width() += 2 * ITEM_OUTER_SPACE;
        aItemSize.Height() += 2 * ITEM_OUTER_SPACE;

        return aItemSize;
    }

    //------------------------------------------------------------------------------------------------------------------
    Point VerticalItemLayout::GetNextItemPosition( const Rectangle& i_rPreviousItemArea ) const
    {
        Point aNewPos( i_rPreviousItemArea.BottomLeft() );
        aNewPos.Y() += ITEM_DISTANCE_PIXEL;
        return aNewPos;
    }

    //------------------------------------------------------------------------------------------------------------------
    void VerticalItemLayout::DrawItem( const PToolPanel& i_pPanel, Window& i_rTargetWindow, const Rectangle& i_rItemRect,
            const ItemFlags i_nItemFlags, const bool i_bDrawMinimal )
    {
        const Size aItemSize( CalculateItemSize( i_pPanel, i_rTargetWindow, i_bDrawMinimal ) );

        Point aDrawPos( i_rItemRect.TopLeft() );
        aDrawPos.Y() += ITEM_OUTER_SPACE;

        // draw the image
        const Image aItemImage( i_pPanel->GetImage() );
        if ( !!aItemImage )
        {
            const Size aImageSize( aItemImage.GetSizePixel() );
            i_rTargetWindow.DrawImage(
                Point( aDrawPos.X() + ( aItemSize.Width() - aImageSize.Width() ) / 2, aDrawPos.Y() ),
                aItemImage
            );
            aDrawPos.Y() += aImageSize.Height() + ITEM_ICON_TEXT_DISTANCE;
        }

        if ( !i_bDrawMinimal )
        {
            aDrawPos.Y() += ITEM_TEXT_FLOW_SPACE;

            // draw the text
            i_rTargetWindow.Push( PUSH_FONT );

            Font aFont( i_rTargetWindow.GetFont() );
            aFont.SetOrientation( 2700 );
            aFont.SetVertical( TRUE );
            i_rTargetWindow.SetFont( aFont );

            const ::rtl::OUString sItemText( i_pPanel->GetDisplayName() );
            const Size aTextSize( i_rTargetWindow.GetCtrlTextWidth( sItemText ), i_rTargetWindow.GetTextHeight() );

            Point aTextPos( aDrawPos );
            aTextPos.X() += aTextSize.Height();
            aTextPos.X() += ( i_rItemRect.GetWidth() - aTextSize.Height() ) / 2;

            i_rTargetWindow.DrawText( aTextPos, sItemText );

            i_rTargetWindow.Pop();
        }

        const bool bActive = ( ( i_nItemFlags & ITEM_STATE_ACTIVE ) != 0 );
        const bool bHovered = ( ( i_nItemFlags & ITEM_STATE_HOVERED ) != 0 );
        const bool bFocused = ( ( i_nItemFlags & ITEM_STATE_FOCUSED ) != 0 );
        if ( bActive || bHovered || bFocused )
        {
            Rectangle aSelectionRect( i_rItemRect );
            aSelectionRect.Left() += ITEM_OUTER_SPACE / 2;
            aSelectionRect.Top() += ITEM_OUTER_SPACE / 2;
            aSelectionRect.Right() -= ITEM_OUTER_SPACE / 2;
            aSelectionRect.Bottom() -= ITEM_OUTER_SPACE / 2;
            i_rTargetWindow.DrawSelectionBackground(
                aSelectionRect,
                ( bHovered || bFocused ) ? ( bActive ? 1 : 2 ) : 0 /* hilight */,
                bActive /* check */,
                TRUE /* border */,
                FALSE /* ext border only */,
                0 /* corner radius */,
                NULL,
                NULL
            );
        }
    }

    //------------------------------------------------------------------------------------------------------------------
    void VerticalItemLayout::FitItemRects( ItemDescriptors& i_rItems, const Rectangle& i_rFitInto )
    {
        if ( i_rItems.empty() )
            // nothing to do
            return;

        // use the minimal sizes if and only if the preferred sizes do not fit
        const Point aBottomRight( i_rItems.rbegin()->aPrefArea.BottomRight() );
        bool bUseMinimal = ( aBottomRight.Y() >= i_rFitInto.Bottom() );
        for (   ItemDescriptors::iterator item = i_rItems.begin();
                item != i_rItems.end();
                ++item
            )
        {
            item->bUseMinimal = bUseMinimal;
        }
    }

    //==================================================================================================================
    //= PanelTabBar_Data
    //==================================================================================================================
    class PanelTabBar_Data  :public IToolPanelContainerListener
                            ,public IToolPanelDeckListener
    {
    public:
        PanelTabBar_Data( PanelTabBar& i_rTabBar )
            :rTabBar( i_rTabBar )
            ,rPanelDeck( dynamic_cast< ToolPanelDeck& >( *i_rTabBar.GetParent() ) )
            ,pLayout( new VerticalItemLayout )
            ,aHoveredItem()
            ,aFocusedItem()
            ,bMouseButtonDown( false )
            ,aItems()
            ,bItemsDirty( true )
        {
            rPanelDeck.AddListener( *this );
            rPanelDeck.GetPanels()->AddListener( *this );
        }

        ~PanelTabBar_Data()
        {
            rPanelDeck.GetPanels()->RemoveListener( *this );
            rPanelDeck.RemoveListener( *this );
        }

        // IToolPanelContainerListener
        virtual void PanelInserted( const PToolPanel& i_pPanel, const size_t i_nPosition )
        {
            (void)i_pPanel;
            (void)i_nPosition;
            bItemsDirty = true;
            rTabBar.Invalidate();
        }

        // IToolPanelDeckListener
        virtual void ActivePanelChanged( const ::boost::optional< size_t >& i_rOldActive, const size_t i_nNewActive );

    public:
        PanelTabBar&                rTabBar;
        ToolPanelDeck&              rPanelDeck;
        const PItemsLayout          pLayout;
        ::boost::optional< size_t > aHoveredItem;
        ::boost::optional< size_t > aFocusedItem;
        bool                        bMouseButtonDown;

        ItemDescriptors             aItems;
        bool                        bItemsDirty;
    };

    //==================================================================================================================
    //= helper
    //==================================================================================================================
    namespace
    {
        //--------------------------------------------------------------------------------------------------------------
    #if OSL_DEBUG_LEVEL > 0
        static void lcl_checkConsistency( const PanelTabBar_Data& i_rData )
        {
            if ( !i_rData.rPanelDeck.GetPanels().get() )
            {
                OSL_ENSURE( false, "lcl_checkConsistency: NULL panels?!" );
                return;
            }
            if ( !i_rData.bItemsDirty )
            {
                if ( i_rData.rPanelDeck.GetPanels()->GetPanelCount() != i_rData.aItems.size() )
                {
                    OSL_ENSURE( false, "lcl_checkConsistency: inconsistent array sizes!" );
                    return;
                }
                for ( size_t i = 0; i < i_rData.rPanelDeck.GetPanels()->GetPanelCount(); ++i )
                {
                    if ( i_rData.rPanelDeck.GetPanels()->GetPanel( i ).get() != i_rData.aItems[i].pPanel.get() )
                    {
                        OSL_ENSURE( false, "lcl_checkConsistency: array elements are inconsistent!" );
                        return;
                    }
                }
            }
        }

        #define DBG_CHECK( data ) \
            lcl_checkConsistency( data );
    #else
        #define DBG_CHECK( data ) \
            (void)data;
    #endif

        //--------------------------------------------------------------------------------------------------------------
        static void lcl_calcItemRects( PanelTabBar_Data& io_rData, const OutputDevice& i_rReferenceDevice )
        {
            io_rData.aItems.resize(0);

            Point aMinItemPos( TAB_BAR_OUTER_SPACE, TAB_BAR_OUTER_SPACE );
            Point aPrefItemPos( TAB_BAR_OUTER_SPACE, TAB_BAR_OUTER_SPACE );

            for (   size_t i = 0;
                    i < io_rData.rPanelDeck.GetPanels()->GetPanelCount();
                    ++i
                )
            {
                PToolPanel pPanel( io_rData.rPanelDeck.GetPanels()->GetPanel( i ) );

                ItemDescriptor aItem;
                aItem.pPanel = pPanel;

                const Size aMinItemSize = io_rData.pLayout->CalculateItemSize( pPanel, i_rReferenceDevice, true );
                const Size aPrefItemSize = io_rData.pLayout->CalculateItemSize( pPanel, i_rReferenceDevice, false );
                    // TODO: have one method calculating both sizes

                aItem.aMinArea = Rectangle( aMinItemPos, aMinItemSize );
                aItem.aPrefArea = Rectangle( aPrefItemPos, aPrefItemSize );

                io_rData.aItems.push_back( aItem );

                aMinItemPos = io_rData.pLayout->GetNextItemPosition( aItem.aMinArea );
                aPrefItemPos = io_rData.pLayout->GetNextItemPosition( aItem.aPrefArea );
            }

            io_rData.bItemsDirty = false;
        }

        //--------------------------------------------------------------------------------------------------------------
        static void lcl_ensureItemsCache( PanelTabBar_Data& io_rData, const OutputDevice& i_rReferenceDevice )
        {
            if ( io_rData.bItemsDirty == false )
            {
                DBG_CHECK( io_rData );
                return;
            }
            lcl_calcItemRects( io_rData, i_rReferenceDevice );
            OSL_POSTCOND( io_rData.bItemsDirty == false, "lcl_ensureItemsCache: cache still dirty!" );
            DBG_CHECK( io_rData );
        }

        //--------------------------------------------------------------------------------------------------------------
        static ::boost::optional< size_t > lcl_findItemForPoint( const PanelTabBar_Data& i_rData, const Point& i_rPoint )
        {
            size_t i=0;
            for (   ItemDescriptors::const_iterator item = i_rData.aItems.begin();
                    item != i_rData.aItems.end();
                    ++item, ++i
                )
            {
                const Rectangle& rItemRect( item->GetCurrentRect() );
                if ( rItemRect.IsInside( i_rPoint ) )
                {
                    return ::boost::optional< size_t >( i );
                }
            }
            return ::boost::optional< size_t >();
        }

        //--------------------------------------------------------------------------------------------------------------
        static void lcl_drawItem( const PanelTabBar_Data& i_rData, const size_t i_nItemIndex )
        {
            const ItemDescriptor& rItem( i_rData.aItems[ i_nItemIndex ] );

            ItemFlags nItemFlags( ITEM_STATE_NORMAL );
            if ( i_rData.aHoveredItem == i_nItemIndex )
            {
                nItemFlags |= ITEM_STATE_HOVERED;
                if ( i_rData.bMouseButtonDown )
                    nItemFlags |= ITEM_STATE_ACTIVE;
            }

            if ( i_rData.rPanelDeck.GetActivePanel() == i_nItemIndex )
                nItemFlags |= ITEM_STATE_ACTIVE;

            if ( i_rData.aFocusedItem == i_nItemIndex )
                nItemFlags |= ITEM_STATE_FOCUSED;

            i_rData.rTabBar.DrawRect( rItem.GetCurrentRect() );
            i_rData.pLayout->DrawItem( rItem.pPanel, i_rData.rTabBar, rItem.GetCurrentRect(), nItemFlags, rItem.bUseMinimal );
        }

        //--------------------------------------------------------------------------------------------------------------
        static void lcl_drawItems( const PanelTabBar_Data& i_rData, const Rectangle& i_rTargetArea )
        {
            size_t i=0;
            for (   ItemDescriptors::const_iterator item = i_rData.aItems.begin();
                    item != i_rData.aItems.end();
                    ++item, ++i
                )
            {
                const Rectangle& rItemRect( item->GetCurrentRect() );
                if ( rItemRect.IsOver( i_rTargetArea ) )
                {
                    lcl_drawItem( i_rData, i );
                }
            }
        }
    }

    //==================================================================================================================
    //= PanelTabBar_Data
    //==================================================================================================================
    void PanelTabBar_Data::ActivePanelChanged( const ::boost::optional< size_t >& i_rOldActive, const size_t i_nNewActive )
    {
        lcl_ensureItemsCache( *this, rTabBar );

        if ( !!i_rOldActive )
            lcl_drawItem( *this, *i_rOldActive );
        lcl_drawItem( *this, i_nNewActive );
    }

    //==================================================================================================================
    //= PanelTabBar
    //==================================================================================================================
    //------------------------------------------------------------------------------------------------------------------
    PanelTabBar::PanelTabBar( ToolPanelDeck& i_rPanelDeck )
        :Control( &i_rPanelDeck, 0 )
        ,m_pData( new PanelTabBar_Data( *this ) )
    {
        DBG_CHECK( *m_pData );

        SetLineColor();
        SetFillColor( GetSettings().GetStyleSettings().GetDialogColor() );
    }

    //------------------------------------------------------------------------------------------------------------------
    PanelTabBar::~PanelTabBar()
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    Size PanelTabBar::GetOptimalSize( WindowSizeType i_eType ) const
    {
        lcl_ensureItemsCache( *m_pData, *this );

        if ( m_pData->aItems.empty() )
            return Size( 2 * TAB_BAR_OUTER_SPACE, 2 * TAB_BAR_OUTER_SPACE );

        const bool nMinimalSize = ( i_eType == WINDOWSIZE_MINIMUM );
        // the rect of the last item
        const Rectangle& rLastItemRect( nMinimalSize ? m_pData->aItems.rbegin()->aMinArea : m_pData->aItems.rbegin()->aPrefArea );
        const Point aBottomRight( rLastItemRect.BottomRight() );
        return Size( aBottomRight.X() + TAB_BAR_OUTER_SPACE, aBottomRight.Y() + TAB_BAR_OUTER_SPACE );
    }

    //------------------------------------------------------------------------------------------------------------------
    void PanelTabBar::Resize()
    {
        Control::Resize();

        // decide whether we should use the minimal or the prefered version of the items

        // the available size
        Size aOutputSize( GetOutputSizePixel() );
        // shrunk by the outer space
        aOutputSize.Width() -= TAB_BAR_OUTER_SPACE;
        aOutputSize.Height() -= TAB_BAR_OUTER_SPACE;
        // let the layouter decide
        m_pData->pLayout->FitItemRects( m_pData->aItems, Rectangle( Point(), aOutputSize ) );
    }

    //------------------------------------------------------------------------------------------------------------------
    void PanelTabBar::Paint( const Rectangle& i_rRect )
    {
        lcl_ensureItemsCache( *m_pData, *this );

        // background
        DrawRect( Rectangle( Point(), GetOutputSizePixel() ) );

        // items
        lcl_drawItems( *m_pData, i_rRect );
    }

    //------------------------------------------------------------------------------------------------------------------
    void PanelTabBar::MouseMove( const MouseEvent& i_rMouseEvent )
    {
        lcl_ensureItemsCache( *m_pData, *this );

        ::boost::optional< size_t > aOldItem( m_pData->aHoveredItem );
        ::boost::optional< size_t > aNewItem( lcl_findItemForPoint( *m_pData, i_rMouseEvent.GetPosPixel() ) );

        if  ( i_rMouseEvent.IsLeaveWindow() )
            aNewItem.reset();

        if ( aOldItem != aNewItem )
        {
            m_pData->aHoveredItem = aNewItem;

            if ( !!aOldItem )
            {
                lcl_drawItem( *m_pData, *aOldItem );
            }

            if ( !!aNewItem )
            {
                lcl_drawItem( *m_pData, *aNewItem );
            }
        }
    }

    //------------------------------------------------------------------------------------------------------------------
    void PanelTabBar::MouseButtonDown( const MouseEvent& i_rMouseEvent )
    {
        Control::MouseButtonDown( i_rMouseEvent );

        if ( !i_rMouseEvent.IsLeft() )
            return;

        lcl_ensureItemsCache( *m_pData, *this );

        ::boost::optional< size_t > aHitItem( lcl_findItemForPoint( *m_pData, i_rMouseEvent.GetPosPixel() ) );
        if ( !aHitItem )
            return;

        CaptureMouse();
        m_pData->bMouseButtonDown = true;

        lcl_drawItem( *m_pData, *aHitItem );
    }

    //------------------------------------------------------------------------------------------------------------------
    void PanelTabBar::MouseButtonUp( const MouseEvent& i_rMouseEvent )
    {
        Control::MouseButtonUp( i_rMouseEvent );

        if ( m_pData->bMouseButtonDown )
        {
            OSL_ENSURE( IsMouseCaptured(), "PanelTabBar::MouseButtonUp: inconsistency!" );
            if ( IsMouseCaptured() )
                ReleaseMouse();
            m_pData->bMouseButtonDown = false;

            ::boost::optional< size_t > aHitItem( lcl_findItemForPoint( *m_pData, i_rMouseEvent.GetPosPixel() ) );
            if ( !!aHitItem )
            {
                // re-draw that item now that we're not in mouse-down mode anymore
                lcl_drawItem( *m_pData, *aHitItem );
                // activate the respective panel
                m_pData->rPanelDeck.ActivatePanel( *aHitItem );
            }
        }
    }

    //------------------------------------------------------------------------------------------------------------------
    void PanelTabBar::RequestHelp( const HelpEvent& i_rHelpEvent )
    {
        lcl_ensureItemsCache( *m_pData, *this );

        ::boost::optional< size_t > aHelpItem( lcl_findItemForPoint( *m_pData, ScreenToOutputPixel( i_rHelpEvent.GetMousePosPixel() ) ) );
        if ( !aHelpItem )
            return;

        const ItemDescriptor& rItem( m_pData->aItems[ *aHelpItem ] );
        if ( !rItem.bUseMinimal )
            // if we do not use the minimal representation of the item, then the text is completely drawn - no
            // need to show it as tooltip, too
            return;

        const ::rtl::OUString sItemText( rItem.pPanel->GetDisplayName() );
        if ( i_rHelpEvent.GetMode() == HELPMODE_BALLOON )
            Help::ShowBalloon( this, OutputToScreenPixel( rItem.GetCurrentRect().Center() ), rItem.GetCurrentRect(), sItemText );
        else
            Help::ShowQuickHelp( this, rItem.GetCurrentRect(), sItemText );
    }

    //------------------------------------------------------------------------------------------------------------------
    void PanelTabBar::GetFocus()
    {
        Control::GetFocus();
        if ( m_pData->rPanelDeck.GetPanels()->GetPanelCount() )
        {
            m_pData->aFocusedItem.reset( m_pData->rPanelDeck.GetActivePanel() );
            lcl_drawItem( *m_pData, *m_pData->aFocusedItem );
        }
    }

    //------------------------------------------------------------------------------------------------------------------
    void PanelTabBar::LoseFocus()
    {
        Control::LoseFocus();

        ::boost::optional< size_t > aPreviouslyFocused( m_pData->aFocusedItem );
        m_pData->aFocusedItem.reset();
        if ( !!aPreviouslyFocused )
            lcl_drawItem( *m_pData, *aPreviouslyFocused );
    }

    //------------------------------------------------------------------------------------------------------------------
    void PanelTabBar::KeyInput( const KeyEvent& i_rKeyEvent )
    {
        Control::KeyInput( i_rKeyEvent );

        const KeyCode& rKeyCode( i_rKeyEvent.GetKeyCode() );
        if ( rKeyCode.GetModifier() != 0 )
            // only interested in mere key presses
            return;

        // if there are less than 2 panels, we cannot travel them ...
        const size_t nPanelCount( m_pData->rPanelDeck.GetPanels()->GetPanelCount() );
        if ( nPanelCount < 2 )
            return;

        OSL_PRECOND( !!m_pData->aFocusedItem, "PanelTabBar::KeyInput: we should have a focused item here!" );
            // if we get KeyInput events, we should have the focus. In this case, aFocusedItem should not be empty,
            // except if there are no panels, but then we bail out of this method here earlier ...

        bool bFocusNext = false;
        bool bFocusPrev = false;

        switch ( rKeyCode.GetCode() )
        {
        case KEY_UP:    bFocusPrev = true; break;
        case KEY_DOWN:  bFocusNext = true; break;
        case KEY_LEFT:
            if ( IsRTLEnabled() )
                bFocusNext = true;
            else
                bFocusPrev = true;
            break;
        case KEY_RIGHT:
            if ( IsRTLEnabled() )
                bFocusPrev = true;
            else
                bFocusNext = true;
            break;
        case KEY_RETURN:
            m_pData->rPanelDeck.ActivatePanel( *m_pData->aFocusedItem );
            break;
        }

        if ( !bFocusNext && !bFocusPrev )
            return;

        const size_t nOldFocus = *m_pData->aFocusedItem;
        if ( bFocusNext )
        {
            m_pData->aFocusedItem.reset( ( *m_pData->aFocusedItem + 1 ) % nPanelCount );
        }
        else
        {
            m_pData->aFocusedItem.reset( ( *m_pData->aFocusedItem + nPanelCount - 1 ) % nPanelCount );
        }

        lcl_drawItem( *m_pData, nOldFocus );
        lcl_drawItem( *m_pData, *m_pData->aFocusedItem );
    }

    //------------------------------------------------------------------------------------------------------------------
    void PanelTabBar::DataChanged( const DataChangedEvent& i_rDataChanedEvent )
    {
        Control::DataChanged( i_rDataChanedEvent );

        if  (   ( i_rDataChanedEvent.GetType() == DATACHANGED_SETTINGS )
            &&  ( ( i_rDataChanedEvent.GetFlags() & SETTINGS_STYLE ) != 0 )
            )
        {
            SetFillColor( GetSettings().GetStyleSettings().GetDialogColor() );
            Invalidate();
        }
    }

//........................................................................
} // namespace svt
//........................................................................
