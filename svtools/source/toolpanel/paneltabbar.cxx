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

#include <basegfx/range/b2drange.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <vcl/help.hxx>
#include <vcl/virdev.hxx>

#include <boost/optional.hpp>
#include <vector>

// space around an item
#define ITEM_OUTER_SPACE        2 * 2
// spacing before and after (in writing direction, whether this is horizontal or vertical) an item's text
#define ITEM_TEXT_FLOW_SPACE    5
// distance between two items
#define ITEM_DISTANCE_PIXEL     0
// space between item icon and icon text
#define ITEM_ICON_TEXT_DISTANCE 4

//........................................................................
namespace svt
{
//........................................................................

    typedef sal_uInt16  ItemFlags;

    #define ITEM_STATE_NORMAL   0x00
    #define ITEM_STATE_ACTIVE   0x01
    #define ITEM_STATE_HOVERED  0x02
    #define ITEM_STATE_FOCUSED  0x04
    #define ITEM_POSITION_FIRST 0x08
    #define ITEM_POSITION_LAST  0x10

    //==================================================================================================================
    //= ItemDescriptor
    //==================================================================================================================
    struct ItemDescriptor
    {
        PToolPanel      pPanel;
        Rectangle       aCompleteArea;
        Rectangle       aIconOnlyArea;
        Rectangle       aTextOnlyArea;
        TabItemContent  eContent;
            // content to be used for this particular item. Might differ from item content which has been set
            // up for the complete control, in case not the complete content fits into the available space.

        ItemDescriptor()
            :pPanel()
            ,aCompleteArea()
            ,aIconOnlyArea()
            ,aTextOnlyArea()
            ,eContent( TABITEM_IMAGE_AND_TEXT )
        {
        }

        const Rectangle& GetRect( const TabItemContent i_eItemContent ) const
        {
            return  ( i_eItemContent == TABITEM_IMAGE_AND_TEXT )
                ?   aCompleteArea
                :   (   ( i_eItemContent == TABITEM_TEXT_ONLY )
                    ?   aTextOnlyArea
                    :   aIconOnlyArea
                    );
        }

        const Rectangle& GetCurrentRect() const
        {
            return GetRect( eContent );
        }
    };

    typedef ::std::vector< ItemDescriptor > ItemDescriptors;

    //==================================================================================================================
    //= IItemsLayout
    //==================================================================================================================
    // TODO: originally, IItemsLayout was intended to abstract from the "vertical vs. horizontal" problem. Meanwhile,
    // the only existing implementation (VerticalItemLayout) does much more than just caring for the vertical layout,
    // a lot of the code would need to be re-used for horizontal layout.
    // Thinking about, horizontal and vertical layout only differ in some coordinate and font rotations. So,
    // one could probably have a single layouter instance, which calculates, say, a default horizontal layout, and
    // derive every other layout by just rotating the coordinates.

    class IItemsLayout
    {
    public:
        /** calculates the size of the area occupied by the item representing the given tool panel
            @param i_pPanel
                denotes the panel whose item's size should be calculated
            @param i_eItemContent
                defines which content to draw on the tab item
            @param o_rBoundingSize
                contains, upon return, the overall size needed to render the item, including possible decorations which are
                <em>not</em> available for the item content
            @param o_rContentArea
                contains, upon return, the area which is available to render the item content. It lies completely
                within an assuming bounding rectangle, which starts at corrdinates (0, 0), and has the size returned
                in o_rBoundingSize.

        */
        virtual void    CalculateItemSize(
                            const PToolPanel& i_pPanel,
                            const TabItemContent i_eItemContent,
                            Size& o_rBoundingSize,
                            Rectangle& o_rContentArea
                        ) const = 0;

        /** returns the position where the next item should start, assuming the previous item occupies a given area
        */
        virtual Point   GetNextItemPosition( const Rectangle& i_rPreviousItemArea ) const = 0;

        /** fills the background of our target device
        */
        virtual void    DrawBackground( const Rectangle& i_rArea ) = 0;

        /** draws the item onto the given device, in the givem area
            @param i_pPanel
                the panel whose item representation is to be drawn
            @param i_rPosition
                the position to paint the item to. Starting at this point, an area of the size returned
                as "bounding size" in CalculateItemSize might be used.
            @param i_nItemFlags
                defines in which state to draw the item
            @param i_eItemContent
                defines which content to draw on the tab item
        */
        virtual void    DrawItem( const PToolPanel& i_pPanel, const Point& i_rPosition,
                            const ItemFlags i_nItemFlags, const TabItemContent i_eItemContent ) = 0;
    };

    typedef ::boost::shared_ptr< IItemsLayout >  PItemsLayout;

    //==================================================================================================================
    //= VerticalItemLayout
    //==================================================================================================================
    class VerticalItemLayout : public IItemsLayout
    {
    public:
        VerticalItemLayout( Window& i_rTargetWindow, const bool i_bLeft );

        // IItemsLayout overridables
        virtual void    CalculateItemSize(
                            const PToolPanel& i_pPanel,
                            const TabItemContent i_eItemContent,
                            Size& o_rBoundingSize,
                            Rectangle& o_rContentArea
                        ) const;
        virtual Point   GetNextItemPosition( const Rectangle& i_rPreviousItemArea ) const;
        virtual void    DrawBackground( const Rectangle& i_rArea );
        virtual void    DrawItem(
                            const PToolPanel& i_pPanel,
                            const Point& i_rPosition,
                            const ItemFlags i_nItemFlags,
                            const TabItemContent i_eItemContent
                        );

    private:
        void    impl_preRender(
                    const Rectangle& i_rBoundingArea,
                    const Rectangle& i_rContentArea,
                    const ItemFlags& i_nFlags
                );
        void    impl_renderContent(
                    const PToolPanel& i_pPanel,
                    const Rectangle& i_rContentArea,
                    const TabItemContent i_eItemContent
                );
        void    impl_postRender(
                    const Rectangle& i_rBoundingArea,
                    const Rectangle& i_rContentArea,
                    const ItemFlags& i_nFlags
                );

        /** rotates a pair of bounding/content rect in formation, so they properly keep their relative position,
            and so that the upper left corner of the bounding rect is constant.
        */
        static void
                impl_rotateFormation( Rectangle& io_rBoundingRect, Rectangle& io_rContentRect, const bool i_bLeft );

    private:
        Window&                 m_rTargetWindow;
        const bool              m_bLeft;

        enum ItemRenderMode
        {
            /// the items are to be rendered in the look of a native widget toolbar item
            NWF_TOOLBAR_ITEM,
            /// the items are to be rendered in the look of a native widget tab bar item
            NWF_TABBAR_ITEM,
            /// the items are to be rendered with VCL functionality only
            VCL_BASED
        };
        ItemRenderMode  m_eRenderMode;
    };

    //------------------------------------------------------------------------------------------------------------------
    VerticalItemLayout::VerticalItemLayout( Window& i_rTargetWindow, const bool i_bLeft )
        :m_rTargetWindow( i_rTargetWindow )
        ,m_bLeft( i_bLeft )
        ,m_eRenderMode( NWF_TOOLBAR_ITEM )
    {
#ifdef WNT
        if ( m_rTargetWindow.IsNativeControlSupported( CTRL_TAB_ITEM, PART_ENTIRE_CONTROL ) )
            // this mode requires the NWF framework to be able to render those items onto a virtual
            // device. For some frameworks (some GTK themes, in particular), this is known to fail.
            // So, be on the safe side for the moment.
            m_eRenderMode = NWF_TABBAR_ITEM;
        else
#endif
        if ( m_rTargetWindow.IsNativeControlSupported( CTRL_TOOLBAR, PART_BUTTON ) )
            m_eRenderMode = NWF_TOOLBAR_ITEM;
        else
            m_eRenderMode = VCL_BASED;
    }

    //------------------------------------------------------------------------------------------------------------------
    void VerticalItemLayout::impl_rotateFormation( Rectangle& io_rBoundingRect, Rectangle& io_rContentRect, const bool i_bLeft )
    {
        // move the to-be-upper-left corner to (0,0)
        ::basegfx::B2DHomMatrix aTransformation;
        aTransformation.translate(
                i_bLeft ? -io_rBoundingRect.Right() : -io_rBoundingRect.Left(),
                i_bLeft ? -io_rBoundingRect.Top()   : -io_rBoundingRect.Bottom() );
        // rotate by 90 resp. -90 degrees
        aTransformation.rotate( i_bLeft ? -F_PI2 : F_PI2 );
            // note on the screen, the ordinate goes top-down, while basegfx calculates in a system where the ordinate
            // does bottom-up; thus the "wrong" sign before F_PI2 here
        // move back to original coordinates
        aTransformation.translate( io_rBoundingRect.Left(), io_rBoundingRect.Top() );

        // apply
        ::basegfx::B2DRange aBoundingRange( io_rBoundingRect.Left(), io_rBoundingRect.Top(), io_rBoundingRect.Right(), io_rBoundingRect.Bottom() );
        aBoundingRange.transform( aTransformation );
        io_rBoundingRect.Left() = long( aBoundingRange.getMinX() );
        io_rBoundingRect.Top() = long( aBoundingRange.getMinY() );
        io_rBoundingRect.Right() = long( aBoundingRange.getMaxX() );
        io_rBoundingRect.Bottom() = long( aBoundingRange.getMaxY() );

        ::basegfx::B2DRange aContentRange( io_rContentRect.Left(), io_rContentRect.Top(), io_rContentRect.Right(), io_rContentRect.Bottom() );
        aContentRange.transform( aTransformation );
        io_rContentRect.Left() = long( aContentRange.getMinX() );
        io_rContentRect.Top() = long( aContentRange.getMinY() );
        io_rContentRect.Right() = long( aContentRange.getMaxX() );
        io_rContentRect.Bottom() = long( aContentRange.getMaxY() );
    }

    //------------------------------------------------------------------------------------------------------------------
    void VerticalItemLayout::CalculateItemSize( const PToolPanel& i_pPanel, const TabItemContent i_eItemContent,
            Size& o_rBoundingSize, Rectangle& o_rContentArea ) const
    {
        const Image aImage( i_pPanel->GetImage() );
        const bool bUseImage = !!aImage && ( i_eItemContent != TABITEM_TEXT_ONLY );

        const ::rtl::OUString sItemText( i_pPanel->GetDisplayName() );
        const bool bUseText = ( sItemText.getLength() != 0 ) && ( i_eItemContent != TABITEM_IMAGE_ONLY );

        Size aItemContentSize;
        if ( bUseImage )
        {
            aItemContentSize = aImage.GetSizePixel();
        }

        if ( bUseText )
        {
            if ( bUseImage )
                aItemContentSize.Height() += ITEM_ICON_TEXT_DISTANCE;

            // add space for vertical text
            const Size aTextSize( m_rTargetWindow.GetCtrlTextWidth( sItemText ), m_rTargetWindow.GetTextHeight() );
            aItemContentSize.Height() += aTextSize.Width();
            aItemContentSize.Width() = ::std::max( aItemContentSize.Width(), aTextSize.Height() );

            aItemContentSize.Height() += 2 * ITEM_TEXT_FLOW_SPACE;
        }

        aItemContentSize.Width() += 2 * ITEM_OUTER_SPACE;
        aItemContentSize.Height() += 2 * ITEM_OUTER_SPACE;

        Region aBoundingRegion, aContentRegion;
        bool bNativeOK = false;
        if ( m_eRenderMode == NWF_TOOLBAR_ITEM )
        {
            // don't ask GetNativeControlRegion, this will not deliver proper results in all cases.
            // Instead, simply assume that both the content and the bounding region are the same.
//            const ImplControlValue aControlValue;
//            bNativeOK = m_rTargetWindow.GetNativeControlRegion(
//                CTRL_TOOLBAR, PART_BUTTON,
//                Rectangle( Point(), aItemContentSize ), CTRL_STATE_ENABLED | CTRL_STATE_ROLLOVER,
//                aControlValue, ::rtl::OUString(),
//                aBoundingRegion, aContentRegion
//            );
            aContentRegion = Rectangle( Point( 1, 1 ), aItemContentSize );
            aBoundingRegion = Rectangle( Point( 0, 0 ), Size( aItemContentSize.Width() + 2, aItemContentSize.Height() + 2 ) );
            bNativeOK = true;
        }
        else if ( m_eRenderMode == NWF_TABBAR_ITEM )
        {
            Rectangle aRotatedContentArea( Point(), aItemContentSize );
            aRotatedContentArea.Transpose();

            TabitemValue tiValue;
            ImplControlValue aControlValue( (void*)(&tiValue) );

            bNativeOK = m_rTargetWindow.GetNativeControlRegion(
                CTRL_TAB_ITEM, PART_ENTIRE_CONTROL,
                Rectangle( aRotatedContentArea ),
                CTRL_STATE_ENABLED | CTRL_STATE_FOCUSED | CTRL_STATE_ROLLOVER | CTRL_STATE_SELECTED,
                aControlValue, ::rtl::OUString(),
                aBoundingRegion, aContentRegion
            );
            OSL_ENSURE( bNativeOK, "VerticalItemLayout::CalculateItemSize: GetNativeControlRegion not implemented for CTRL_TAB_ITEM?!" );

            Rectangle aBoundingRect( aBoundingRegion.GetBoundRect() );
            Rectangle aContentRect( aContentRegion.GetBoundRect() );

            impl_rotateFormation( aBoundingRect, aContentRect, m_bLeft );

            aBoundingRegion = aBoundingRect;
            aContentRegion = aContentRect;
        }

        if ( bNativeOK )
        {
            const Rectangle aBoundingRect( aBoundingRegion.GetBoundRect() );

            o_rContentArea = aContentRegion.GetBoundRect();
            o_rBoundingSize = aBoundingRect.GetSize();

            // normalize the content area, it is assumed to be relative to a rectangle which starts
            // at (0,0), and has a size of o_rBoundingSize.
            o_rContentArea.Move( -aBoundingRect.Left(), -aBoundingRect.Top() );
        }
        else
        {
            o_rContentArea = Rectangle( Point( 0, 0 ), aItemContentSize );
            o_rBoundingSize = aItemContentSize;

            // don't attempt native rendering, again. If it didn't work this time, it won't work in the future.
            const_cast< VerticalItemLayout* >( this )->m_eRenderMode = VCL_BASED;
        }
    }

    //------------------------------------------------------------------------------------------------------------------
    Point VerticalItemLayout::GetNextItemPosition( const Rectangle& i_rPreviousItemArea ) const
    {
        Point aNewPos( i_rPreviousItemArea.BottomLeft() );
        aNewPos.Y() += ITEM_DISTANCE_PIXEL;
        return aNewPos;
    }

    //------------------------------------------------------------------------------------------------------------------
    void VerticalItemLayout::impl_preRender( const Rectangle& i_rBoundingArea, const Rectangle& i_rContentArea, const ItemFlags& i_nItemFlags )
    {
        ControlState nState = CTRL_STATE_ENABLED;
        if ( i_nItemFlags & ITEM_STATE_FOCUSED )    nState |= CTRL_STATE_FOCUSED;
        if ( i_nItemFlags & ITEM_STATE_HOVERED )    nState |= CTRL_STATE_ROLLOVER;
        if ( i_nItemFlags & ITEM_STATE_ACTIVE )     nState |= CTRL_STATE_SELECTED;

        switch ( m_eRenderMode )
        {
        case NWF_TOOLBAR_ITEM:
        {
            // completely erase the target area, toolbar item NWF is not expected to do this
            m_rTargetWindow.DrawRect( i_rBoundingArea );

            const Region aCtrlRegion( i_rContentArea );

            ImplControlValue aControlValue;
            aControlValue.setTristateVal( ( i_nItemFlags & ITEM_STATE_ACTIVE ) ? BUTTONVALUE_ON : BUTTONVALUE_OFF );

            bool bNativeOK = m_rTargetWindow.DrawNativeControl( CTRL_TOOLBAR, PART_BUTTON, aCtrlRegion, nState, aControlValue, rtl::OUString() );
            (void)bNativeOK;
            OSL_ENSURE( bNativeOK, "VerticalItemLayout::impl_preRender: inconsistent NWF implementation!" );
                // IsNativeControlSupported returned true, previously, otherwise we would not be here ...
        }
        break;

        case NWF_TABBAR_ITEM:
        {
            VirtualDevice aRenderDevice( m_rTargetWindow );

            Rectangle aRotatedBoundingArea( i_rBoundingArea );
            Rectangle aRotatedContentArea( i_rContentArea );
            impl_rotateFormation( aRotatedBoundingArea, aRotatedContentArea, !m_bLeft );

            const Size aRotatedBoundingSize( aRotatedBoundingArea.GetSize() );
            aRenderDevice.SetOutputSizePixel( aRotatedBoundingSize );

            const Point aRotatedContentOffset( aRotatedContentArea.Left() - aRotatedBoundingArea.Left(), aRotatedContentArea.Top() - aRotatedBoundingArea.Top() );
            const Size aRotatedContentSize( aRotatedContentArea.GetSize() );
            const Region aCtrlRegion( Rectangle( aRotatedContentOffset, aRotatedContentSize ) );

            TabitemValue tiValue;
            if ( i_nItemFlags & ITEM_POSITION_FIRST )
                tiValue.mnAlignment |= TABITEM_FIRST_IN_GROUP;
            if ( i_nItemFlags & ITEM_POSITION_LAST )
                tiValue.mnAlignment |= TABITEM_LAST_IN_GROUP;

            ImplControlValue aControlValue( (void *)(&tiValue) );

            bool bNativeOK = aRenderDevice.DrawNativeControl( CTRL_TAB_ITEM, PART_ENTIRE_CONTROL, aCtrlRegion, nState, aControlValue, rtl::OUString() );
            OSL_ENSURE( bNativeOK, "VerticalItemLayout::impl_preRender: inconsistent NWF implementation!" );
                // IsNativeControlSupported returned true, previously, otherwise we would not be here ...

            BitmapEx aBitmap( aRenderDevice.GetBitmapEx( Point( 0, 0 ), Size( aRotatedBoundingSize.Width() - 1, aRotatedBoundingSize.Height() - 1 ) ) );
            aBitmap.Rotate( m_bLeft ? 900 : 2700, Color( COL_BLACK ) );

            m_rTargetWindow.DrawBitmapEx( i_rBoundingArea.TopLeft(), aBitmap );
        }
        break;

        case VCL_BASED:
            // completely erase the target area. Otherwise, the DrawSelectionBackground from postRender will constantly add up
            m_rTargetWindow.DrawRect( i_rBoundingArea );
            break;
        }
    }

    //------------------------------------------------------------------------------------------------------------------
    void VerticalItemLayout::impl_renderContent( const PToolPanel& i_pPanel, const Rectangle& i_rContentArea, const TabItemContent i_eItemContent )
    {
        Point aDrawPos( i_rContentArea.TopLeft() );
        aDrawPos.Y() += ITEM_OUTER_SPACE;

        // draw the image
        const Image aItemImage( i_pPanel->GetImage() );
        const Size aImageSize( aItemImage.GetSizePixel() );
        const bool bUseImage = !!aItemImage && ( i_eItemContent != TABITEM_TEXT_ONLY );

        if ( bUseImage )
        {
            m_rTargetWindow.DrawImage(
                Point( aDrawPos.X() + ( i_rContentArea.GetWidth() - aImageSize.Width() ) / 2, aDrawPos.Y() ),
                aItemImage
            );
        }

        const ::rtl::OUString sItemText( i_pPanel->GetDisplayName() );
        const bool bUseText = ( sItemText.getLength() != 0 ) && ( i_eItemContent != TABITEM_IMAGE_ONLY );

        if ( bUseText )
        {
            if ( bUseImage )
                aDrawPos.Y() += aImageSize.Height() + ITEM_ICON_TEXT_DISTANCE;

            aDrawPos.Y() += ITEM_TEXT_FLOW_SPACE;

            // draw the text
            m_rTargetWindow.Push( PUSH_FONT );

            Font aFont( m_rTargetWindow.GetFont() );
            aFont.SetOrientation( 2700 );
            aFont.SetVertical( TRUE );
            m_rTargetWindow.SetFont( aFont );

            const Size aTextSize( m_rTargetWindow.GetCtrlTextWidth( sItemText ), m_rTargetWindow.GetTextHeight() );

            Point aTextPos( aDrawPos );
            aTextPos.X() += aTextSize.Height();
            aTextPos.X() += ( i_rContentArea.GetWidth() - aTextSize.Height() ) / 2;

            m_rTargetWindow.DrawText( aTextPos, sItemText );

            m_rTargetWindow.Pop();
        }
    }

    //------------------------------------------------------------------------------------------------------------------
    void VerticalItemLayout::impl_postRender( const Rectangle& i_rBoundingArea, const Rectangle& i_rContentArea, const ItemFlags& i_nItemFlags )
    {
        (void)i_rBoundingArea;

        if ( m_eRenderMode == VCL_BASED )
        {
            const bool bActive = ( ( i_nItemFlags & ITEM_STATE_ACTIVE ) != 0 );
            const bool bHovered = ( ( i_nItemFlags & ITEM_STATE_HOVERED ) != 0 );
            const bool bFocused = ( ( i_nItemFlags & ITEM_STATE_FOCUSED ) != 0 );
            if ( bActive || bHovered || bFocused )
            {
                Rectangle aSelectionRect( i_rContentArea );
                aSelectionRect.Left() += ITEM_OUTER_SPACE / 2;
                aSelectionRect.Top() += ITEM_OUTER_SPACE / 2;
                aSelectionRect.Right() -= ITEM_OUTER_SPACE / 2;
                aSelectionRect.Bottom() -= ITEM_OUTER_SPACE / 2;
                m_rTargetWindow.DrawSelectionBackground(
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
    }

    //------------------------------------------------------------------------------------------------------------------
    void VerticalItemLayout::DrawBackground( const Rectangle& i_rArea )
    {
        bool bNativeOK = false;
        switch ( m_eRenderMode )
        {
        case VCL_BASED:
        case NWF_TOOLBAR_ITEM:
            break;

        case NWF_TABBAR_ITEM:
        {
            ControlState nState( 0 );
            if ( m_rTargetWindow.IsEnabled() )
                nState |= CTRL_STATE_ENABLED;
            if ( m_rTargetWindow.HasChildPathFocus() )
                nState |= CTRL_STATE_FOCUSED;

            bNativeOK = m_rTargetWindow.IsNativeControlSupported( CTRL_TAB_PANE, PART_ENTIRE_CONTROL )
                    &&  m_rTargetWindow.DrawNativeControl( CTRL_TAB_PANE, PART_ENTIRE_CONTROL, i_rArea, nState,
                            ImplControlValue(), rtl::OUString() );
        }
        break;
        }

        if ( !bNativeOK )
            m_rTargetWindow.DrawRect( i_rArea );
    }

    //------------------------------------------------------------------------------------------------------------------
    void VerticalItemLayout::DrawItem( const PToolPanel& i_pPanel, const Point& i_rPosition,
            const ItemFlags i_nItemFlags, const TabItemContent i_eItemContent )
    {
        Rectangle aContentArea;
        Size aBoundingSize;
        CalculateItemSize( i_pPanel, i_eItemContent, aBoundingSize, aContentArea );

        aContentArea.Move( i_rPosition.X(), i_rPosition.Y() );
        const Rectangle aBoundingArea( i_rPosition, aBoundingSize );

        impl_preRender( aBoundingArea, aContentArea, i_nItemFlags );
        impl_renderContent( i_pPanel, aContentArea, i_eItemContent );
        impl_postRender( aBoundingArea, aContentArea, i_nItemFlags );
    }

    //==================================================================================================================
    //= PanelTabBar_Data
    //==================================================================================================================
    class PanelTabBar_Data  :public IToolPanelContainerListener
                            ,public IToolPanelDeckListener
    {
    public:
        PanelTabBar_Data( PanelTabBar& i_rTabBar, const TabAlignment i_eAlignment, const TabItemContent i_eItemContent )
            :rTabBar( i_rTabBar )
            ,rPanelDeck( dynamic_cast< ToolPanelDeck& >( *i_rTabBar.GetParent() ) )
            ,eAlignment( i_eAlignment )
            ,eTabItemContent( i_eItemContent )
            ,pLayout( new VerticalItemLayout( i_rTabBar, i_eAlignment == TABS_LEFT ) )
            ,aHoveredItem()
            ,aFocusedItem()
            ,bMouseButtonDown( false )
            ,aItems()
            ,bItemsDirty( true )
        {
            OSL_ENSURE( ( i_eAlignment == TABS_LEFT ) || ( i_eAlignment == TABS_RIGHT ),
                "PanelTabBar_Data: unsupported alignment!" );

            rPanelDeck.AddListener( *this );
            rPanelDeck.GetPanels()->AddListener( *this );

            if ( i_eAlignment == TABS_LEFT )
            {
                aTopLeftSpace = Size( 2, 2 );
                aBottomRightSpace = Size( 1, 1 );
            }
            else if ( i_eAlignment == TABS_RIGHT )
            {
                aTopLeftSpace = Size( 1, 1 );
                aBottomRightSpace = Size( 2, 2 );
            }
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

        const TabAlignment          eAlignment;
        TabItemContent              eTabItemContent;
        PItemsLayout                pLayout;

        ::boost::optional< size_t > aHoveredItem;
        ::boost::optional< size_t > aFocusedItem;
        bool                        bMouseButtonDown;

        ItemDescriptors             aItems;
        bool                        bItemsDirty;

        Size                        aTopLeftSpace;
        Size                        aBottomRightSpace;
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
        static void lcl_calcItemRects( PanelTabBar_Data& io_rData )
        {
            io_rData.aItems.resize(0);

            Point aCompletePos( io_rData.aTopLeftSpace.Width(), io_rData.aTopLeftSpace.Height() );
            Point aIconOnlyPos( aCompletePos );
            Point aTextOnlyPos( aCompletePos );

            for (   size_t i = 0;
                    i < io_rData.rPanelDeck.GetPanels()->GetPanelCount();
                    ++i
                )
            {
                PToolPanel pPanel( io_rData.rPanelDeck.GetPanels()->GetPanel( i ) );

                ItemDescriptor aItem;
                aItem.pPanel = pPanel;

                Rectangle aContentArea;

                Size aCompleteSize;
                io_rData.pLayout->CalculateItemSize( pPanel, TABITEM_IMAGE_AND_TEXT, aCompleteSize, aContentArea );

                Size aIconOnlySize;
                io_rData.pLayout->CalculateItemSize( pPanel, TABITEM_IMAGE_ONLY, aIconOnlySize, aContentArea );

                Size aTextOnlySize;
                io_rData.pLayout->CalculateItemSize( pPanel, TABITEM_TEXT_ONLY, aTextOnlySize, aContentArea );

                // TODO: have one method calculating all sizes?

                aItem.aCompleteArea = Rectangle( aCompletePos, aCompleteSize );
                aItem.aIconOnlyArea = Rectangle( aIconOnlyPos, aIconOnlySize );
                aItem.aTextOnlyArea = Rectangle( aTextOnlyPos, aTextOnlySize );

                io_rData.aItems.push_back( aItem );

                aCompletePos = io_rData.pLayout->GetNextItemPosition( aItem.aCompleteArea );
                aIconOnlyPos = io_rData.pLayout->GetNextItemPosition( aItem.aIconOnlyArea );
                aTextOnlyPos = io_rData.pLayout->GetNextItemPosition( aItem.aTextOnlyArea );
            }

            io_rData.bItemsDirty = false;
        }

        //--------------------------------------------------------------------------------------------------------------
        static void lcl_ensureItemsCache( PanelTabBar_Data& io_rData )
        {
            if ( io_rData.bItemsDirty == false )
            {
                DBG_CHECK( io_rData );
                return;
            }
            lcl_calcItemRects( io_rData );
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

            if ( 0 == i_nItemIndex )
                nItemFlags |= ITEM_POSITION_FIRST;

            if ( i_rData.rPanelDeck.GetPanels()->GetPanelCount() - 1 == i_nItemIndex )
                nItemFlags |= ITEM_POSITION_LAST;

            i_rData.rTabBar.SetUpdateMode( FALSE );
            i_rData.pLayout->DrawItem( rItem.pPanel, rItem.GetCurrentRect().TopLeft(), nItemFlags, rItem.eContent );
            i_rData.rTabBar.SetUpdateMode( TRUE );
        }

        //--------------------------------------------------------------------------------------------------------------
        static void lcl_fitItems( PanelTabBar_Data& io_rData )
        {
            if ( io_rData.aItems.empty() )
                // nothing to do
                return;

            // the available size
            Size aOutputSize( io_rData.rTabBar.GetOutputSizePixel() );
            // shrunk by the outer space
            aOutputSize.Width() -= io_rData.aBottomRightSpace.Width();
            aOutputSize.Height() -= io_rData.aBottomRightSpace.Height();
            const Rectangle aFitInto( Point( 0, 0 ), aOutputSize );

            // the "content modes" to try
            TabItemContent eTryThis[] =
            {
                TABITEM_IMAGE_ONLY,     // assumed to have the smalles rects
                TABITEM_TEXT_ONLY,
                TABITEM_IMAGE_AND_TEXT  // assumed to have the largest rects
            };

            // do not start with the largest, but with the one currently set up for the TabBar
            size_t nTryIndex = 0;
            while   (   ( nTryIndex < ( sizeof( eTryThis ) / sizeof( eTryThis[0] ) ) )
                    &&  ( eTryThis[nTryIndex] != io_rData.eTabItemContent )
                    )
            {
                ++nTryIndex;
            }

            // determine which of the different version fits
            TabItemContent eContent = eTryThis[0];
            while ( nTryIndex > 0 )
            {
                const Point aBottomRight( io_rData.aItems.rbegin()->GetRect( eTryThis[ nTryIndex ] ).BottomRight() );
                if ( aFitInto.IsInside( aBottomRight ) )
                {
                    eContent = eTryThis[ nTryIndex ];
                    break;
                }
                --nTryIndex;
            }

            // propagate to the items
            for (   ItemDescriptors::iterator item = io_rData.aItems.begin();
                    item != io_rData.aItems.end();
                    ++item
                )
            {
                item->eContent = eContent;
            }
        }
    }

    //==================================================================================================================
    //= PanelTabBar_Data
    //==================================================================================================================
    void PanelTabBar_Data::ActivePanelChanged( const ::boost::optional< size_t >& i_rOldActive, const size_t i_nNewActive )
    {
        lcl_ensureItemsCache( *this );

        if ( !!i_rOldActive )
            lcl_drawItem( *this, *i_rOldActive );
        lcl_drawItem( *this, i_nNewActive );
    }

    //==================================================================================================================
    //= PanelTabBar
    //==================================================================================================================
    //------------------------------------------------------------------------------------------------------------------
    PanelTabBar::PanelTabBar( ToolPanelDeck& i_rPanelDeck, const TabAlignment i_eAlignment, const TabItemContent i_eItemContent )
        :Control( &i_rPanelDeck, 0 )
        ,m_pData( new PanelTabBar_Data( *this, i_eAlignment, i_eItemContent ) )
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
    TabItemContent PanelTabBar::GetTabItemContent() const
    {
        return m_pData->eTabItemContent;
    }

    //------------------------------------------------------------------------------------------------------------------
    void PanelTabBar::SetTabItemContent( const TabItemContent& i_eItemContent )
    {
        m_pData->eTabItemContent = i_eItemContent;
        lcl_fitItems( *m_pData );
        Invalidate();
    }

    //------------------------------------------------------------------------------------------------------------------
    Size PanelTabBar::GetOptimalSize( WindowSizeType i_eType ) const
    {
        lcl_ensureItemsCache( *m_pData );

        if ( m_pData->aItems.empty() )
            return Size(
                m_pData->aTopLeftSpace.Width() + m_pData->aBottomRightSpace.Width(),
                m_pData->aTopLeftSpace.Height() + m_pData->aBottomRightSpace.Height()
            );

        const bool bMinimalSize = ( i_eType == WINDOWSIZE_MINIMUM );
        // the rect of the last item
        const Rectangle& rLastItemRect( bMinimalSize ? m_pData->aItems.rbegin()->aIconOnlyArea : m_pData->aItems.rbegin()->aCompleteArea );
        const Point aBottomRight( rLastItemRect.BottomRight() );
        return Size(
                    aBottomRight.X() + 1 + m_pData->aBottomRightSpace.Width(),
                    aBottomRight.Y() + 1 + m_pData->aBottomRightSpace.Height()
                );
    }

    //------------------------------------------------------------------------------------------------------------------
    void PanelTabBar::Resize()
    {
        Control::Resize();
        lcl_fitItems( *m_pData );
    }

    //------------------------------------------------------------------------------------------------------------------
    void PanelTabBar::Paint( const Rectangle& i_rRect )
    {
        lcl_ensureItemsCache( *m_pData );

        // background
        m_pData->pLayout->DrawBackground( Rectangle( Point(), GetOutputSizePixel() ) );

        // items
        size_t i=0;
        for (   ItemDescriptors::const_iterator item = m_pData->aItems.begin();
                item != m_pData->aItems.end();
                ++item, ++i
            )
        {
            const Rectangle& rItemRect( item->GetCurrentRect() );
            if ( rItemRect.IsOver( i_rRect ) )
            {
                lcl_drawItem( *m_pData, i );
            }
        }
    }

    //------------------------------------------------------------------------------------------------------------------
    void PanelTabBar::MouseMove( const MouseEvent& i_rMouseEvent )
    {
        lcl_ensureItemsCache( *m_pData );

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

        lcl_ensureItemsCache( *m_pData );

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
        lcl_ensureItemsCache( *m_pData );

        ::boost::optional< size_t > aHelpItem( lcl_findItemForPoint( *m_pData, ScreenToOutputPixel( i_rHelpEvent.GetMousePosPixel() ) ) );
        if ( !aHelpItem )
            return;

        const ItemDescriptor& rItem( m_pData->aItems[ *aHelpItem ] );
        if ( rItem.eContent != TABITEM_IMAGE_ONLY )
            // if the text is displayed for the item, we do not need to show it as tooltip
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
