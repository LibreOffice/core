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
#include "tabitemdescriptor.hxx"
#include "paneltabbar.hxx"
#include "tabbargeometry.hxx"

#include <basegfx/range/b2drange.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <vcl/button.hxx>
#include <vcl/help.hxx>
#include <vcl/virdev.hxx>

#include <boost/optional.hpp>
#include <vector>

// space around an item
#define ITEM_OUTER_SPACE        2 * 2
// the distance between the button and the control's border, in "upright" direction (what this means depends on the alignment)
#define BUTTON_UPRIGHT_DISTANCE 2
// spacing before and after (in writing direction, whether this is horizontal or vertical) an item's text
#define ITEM_TEXT_FLOW_SPACE    5
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
        /** fills the background of our target device
        */
        virtual void    DrawBackground( const Rectangle& i_rArea ) = 0;

        /** calculates the size of the area occupied by the item representing the given tool panel
            @param i_pPanel
                denotes the panel whose item's size should be calculated
            @param i_eItemContent
                defines which content to draw on the tab item
            @param o_rBoundingSize
                contains, upon return, the overall size needed to render the item, including possible decorations which are
                <em>not</em> available for the item content. The rectangle is in normalized form, i.e. assuming that
                the tab bar is aligned at top.
            @param o_rContentArea
                contains, upon return, the area which is available to render the item content. It lies completely
                within an assuming bounding rectangle, which starts at corrdinates (0, 0), and has the size returned
                in o_rBoundingSize. The rectangle is in normalized form, i.e. assuming that the tab bar is aligned at top.
        */
        virtual void CalculateItemSize( const PToolPanel& i_pPanel, const TabItemContent i_eItemContent, Size& o_rBoundingSize, Rectangle& o_rContentArea ) const = 0;

        /** draws the item onto the given device, in the givem area
            @param i_pPanel
                the panel whose item representation is to be drawn
            @param i_rPosition
                the position to paint the item to.
            @param i_nItemFlags
                defines in which state to draw the item
            @param i_eItemContent
                defines which content to draw on the tab item
        */
        virtual void    DrawItem(
                            const TabBarGeometry& i_rGeometry,
                            const PToolPanel& i_pPanel,
                            const Point& i_rPosition,
                            const ItemFlags i_nItemFlags,
                            const TabItemContent i_eItemContent
                        ) = 0;
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
        virtual void    DrawBackground( const Rectangle& i_rArea );
        virtual void    CalculateItemSize(
                            const PToolPanel& i_pPanel,
                            const TabItemContent i_eItemContent,
                            Size& o_rBoundingSize,
                            Rectangle& o_rContentArea
                        ) const;
        virtual void    DrawItem(
                            const TabBarGeometry& i_rGeometry,
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

    private:
        Window&     m_rTargetWindow;
        const bool  m_bLeft;

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

            NormalizedArea aNormalized( i_rBoundingArea, true );
            const Rectangle aNormalizedBounds( aNormalized.getReference() );
            const Rectangle aNormalizedContent( aNormalized.getNormalized( i_rContentArea, m_bLeft ? TABS_LEFT : TABS_RIGHT ) );

            const Size aNormalizedBoundingSize( aNormalizedBounds.GetSize() );
            aRenderDevice.SetOutputSizePixel( aNormalizedBoundingSize );

            // draw the actual item
            const Point aNormalizedContentOffset( aNormalizedContent.Left() - aNormalizedBounds.Left(), aNormalizedContent.Top() - aNormalizedBounds.Top() );
            const Region aCtrlRegion( Rectangle( aNormalizedContentOffset, aNormalizedContent.GetSize() ) );

            TabitemValue tiValue;
            if ( i_nItemFlags & ITEM_POSITION_FIRST )
                tiValue.mnAlignment |= TABITEM_FIRST_IN_GROUP;
            if ( i_nItemFlags & ITEM_POSITION_LAST )
                tiValue.mnAlignment |= TABITEM_LAST_IN_GROUP;

            ImplControlValue aControlValue( (void *)(&tiValue) );

            bool bNativeOK = aRenderDevice.DrawNativeControl( CTRL_TAB_ITEM, PART_ENTIRE_CONTROL, aCtrlRegion, nState, aControlValue, rtl::OUString() );
            (void)bNativeOK;
            OSL_ENSURE( bNativeOK, "VerticalItemLayout::impl_preRender: inconsistent NWF implementation!" );
                // IsNativeControlSupported returned true, previously, otherwise we would not be here ...

            // copy content over to the target window
            BitmapEx aBitmap( aRenderDevice.GetBitmapEx( Point( 0, 0 ), Size( aNormalizedBoundingSize.Width() - 1, aNormalizedBoundingSize.Height() - 1 ) ) );
            aBitmap.Rotate( 2700, COL_BLACK );
            if ( m_bLeft )
                aBitmap.Mirror( BMP_MIRROR_HORZ );

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
        OSL_ENSURE( i_eItemContent != TABITEM_AUTO, "VerticalItemLayout::impl_renderContent: illegal TabItemContent value!" );

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
            bNativeOK = true;
            // don't draw any background. The default behavior of VCL windows - draw a dialog face color -
            // is completely sufficient here.
            break;
        }

        if ( !bNativeOK )
            m_rTargetWindow.DrawRect( i_rArea );
    }

    //------------------------------------------------------------------------------------------------------------------
    void VerticalItemLayout::CalculateItemSize( const PToolPanel& i_pPanel,
        const TabItemContent i_eItemContent, Size& o_rBoundingSize, Rectangle& o_rContentArea ) const
    {
        OSL_ENSURE( i_eItemContent != TABITEM_AUTO, "VerticalItemLayout::CalculateItemSize: illegal TabItemContent value!" );

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
                aItemContentSize.Width() += ITEM_ICON_TEXT_DISTANCE;

            // add space for text
            const Size aTextSize( m_rTargetWindow.GetCtrlTextWidth( sItemText ), m_rTargetWindow.GetTextHeight() );
            aItemContentSize.Width() += aTextSize.Width();
            aItemContentSize.Height() = ::std::max( aItemContentSize.Height(), aTextSize.Height() );

            aItemContentSize.Width() += 2 * ITEM_TEXT_FLOW_SPACE;
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
            TabitemValue tiValue;
            ImplControlValue aControlValue( (void*)(&tiValue) );

            bNativeOK = m_rTargetWindow.GetNativeControlRegion(
                CTRL_TAB_ITEM, PART_ENTIRE_CONTROL,
                Rectangle( Point(), aItemContentSize ),
                CTRL_STATE_ENABLED | CTRL_STATE_FOCUSED | CTRL_STATE_ROLLOVER | CTRL_STATE_SELECTED,
                aControlValue, ::rtl::OUString(),
                aBoundingRegion, aContentRegion
            );
            OSL_ENSURE( bNativeOK, "VerticalItemLayout::CalculateItemSize: GetNativeControlRegion not implemented for CTRL_TAB_ITEM?!" );
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
    void VerticalItemLayout::DrawItem( const TabBarGeometry& i_rGeometry, const PToolPanel& i_pPanel, const Point& i_rPosition,
            const ItemFlags i_nItemFlags, const TabItemContent i_eItemContent )
    {
        // calculate content and bounding size
        Rectangle aContentArea;
        Size aBoundingSize;
        CalculateItemSize( i_pPanel, i_eItemContent, aBoundingSize, aContentArea );

        // CalculateItemSize returns a normalized size, i.e. one assuming TABS_TOP
        const NormalizedArea aNormalized( Rectangle( Point(), aBoundingSize ), false );
        aContentArea = aNormalized.getTransformed( aContentArea, i_rGeometry.getAlignment() );
        aBoundingSize = Size( aBoundingSize.Height(), aBoundingSize.Width() );

        // move both rects to the given position
        aContentArea.Move( i_rPosition.X(), i_rPosition.Y() );
        const Rectangle aBoundingArea( i_rPosition, aBoundingSize );

        // actually draw
        impl_preRender( aBoundingArea, aContentArea, i_nItemFlags );
        impl_renderContent( i_pPanel, aContentArea, i_eItemContent );
        impl_postRender( aBoundingArea, aContentArea, i_nItemFlags );
    }

    //==================================================================================================================
    //= PanelTabBar_Data
    //==================================================================================================================
    class PanelTabBar_Data : public IToolPanelDeckListener
    {
    public:
        PanelTabBar_Data( PanelTabBar& i_rTabBar, IToolPanelDeck& i_rPanelDeck, const TabAlignment i_eAlignment, const TabItemContent i_eItemContent );

        ~PanelTabBar_Data()
        {
            rPanelDeck.RemoveListener( *this );
        }

        // IToolPanelDeckListener
        virtual void PanelInserted( const PToolPanel& i_pPanel, const size_t i_nPosition )
        {
            (void)i_pPanel;
            (void)i_nPosition;
            bItemsDirty = true;
            rTabBar.Invalidate();
        }

        virtual void PanelRemoved( const size_t i_nPosition )
        {
            (void)i_nPosition;
            bItemsDirty = true;
            rTabBar.Invalidate();
        }

        virtual void ActivePanelChanged( const ::boost::optional< size_t >& i_rOldActive, const ::boost::optional< size_t >& i_rNewActive );
        virtual void Dying();

        void    UpdateScrollButtons()
        {
            m_aScrollBack.Enable( m_nScrollPosition > 0 );
            m_aScrollForward.Enable( m_nScrollPosition < aItems.size() - 1 );
        }

    protected:
        DECL_LINK( OnScroll, const PushButton* );

    public:
        PanelTabBar&                rTabBar;
        TabBarGeometry              aGeometry;
        IToolPanelDeck&             rPanelDeck;

        PItemsLayout                pLayout;

        ::boost::optional< size_t > aHoveredItem;
        ::boost::optional< size_t > aFocusedItem;
        bool                        bMouseButtonDown;

        ItemDescriptors             aItems;
        bool                        bItemsDirty;

        PushButton                  m_aScrollBack;
        PushButton                  m_aScrollForward;

        size_t                      m_nScrollPosition;
    };

    //==================================================================================================================
    //= PanelTabBar_Data - implementation
    //==================================================================================================================
    //------------------------------------------------------------------------------------------------------------------
    PanelTabBar_Data::PanelTabBar_Data( PanelTabBar& i_rTabBar, IToolPanelDeck& i_rPanelDeck, const TabAlignment i_eAlignment, const TabItemContent i_eItemContent )
        :rTabBar( i_rTabBar )
        ,aGeometry( i_eAlignment, i_eItemContent )
        ,rPanelDeck( i_rPanelDeck )
        ,pLayout( new VerticalItemLayout( i_rTabBar, i_eAlignment == TABS_LEFT ) )
        ,aHoveredItem()
        ,aFocusedItem()
        ,bMouseButtonDown( false )
        ,aItems()
        ,bItemsDirty( true )
        ,m_aScrollBack( &i_rTabBar, WB_BEVELBUTTON )
        ,m_aScrollForward( &i_rTabBar, WB_BEVELBUTTON )
        ,m_nScrollPosition( 0 )
    {
        OSL_ENSURE( ( i_eAlignment == TABS_LEFT ) || ( i_eAlignment == TABS_RIGHT ),
            "PanelTabBar_Data: unsupported alignment!" );

        rPanelDeck.AddListener( *this );

        m_aScrollBack.SetSymbol( SYMBOL_ARROW_UP );
        m_aScrollBack.Show();
        m_aScrollBack.SetClickHdl( LINK( this, PanelTabBar_Data, OnScroll ) );

        m_aScrollForward.SetSymbol( SYMBOL_ARROW_DOWN );
        m_aScrollForward.Show();
        m_aScrollForward.SetClickHdl( LINK( this, PanelTabBar_Data, OnScroll ) );
    }

    //------------------------------------------------------------------------------------------------------------------
    IMPL_LINK( PanelTabBar_Data, OnScroll, const PushButton*, i_pButton )
    {
        if ( i_pButton == &m_aScrollBack )
        {
            OSL_ENSURE( m_nScrollPosition > 0, "PanelTabBar_Data::OnScroll: inconsistency!" );
            --m_nScrollPosition;
            rTabBar.Invalidate();
        }
        else if ( i_pButton == &m_aScrollForward )
        {
            OSL_ENSURE( m_nScrollPosition < aItems.size() - 1, "PanelTabBar_Data::OnScroll: inconsistency!" );
            ++m_nScrollPosition;
            rTabBar.Invalidate();
        }

        UpdateScrollButtons();

        return 0L;
    }

    //==================================================================================================================
    //= helper
    //==================================================================================================================
    namespace
    {
        //--------------------------------------------------------------------------------------------------------------
    #if OSL_DEBUG_LEVEL > 0
        static void lcl_checkConsistency( const PanelTabBar_Data& i_rData )
        {
            if ( !i_rData.bItemsDirty )
            {
                if ( i_rData.rPanelDeck.GetPanelCount() != i_rData.aItems.size() )
                {
                    OSL_ENSURE( false, "lcl_checkConsistency: inconsistent array sizes!" );
                    return;
                }
                for ( size_t i = 0; i < i_rData.rPanelDeck.GetPanelCount(); ++i )
                {
                    if ( i_rData.rPanelDeck.GetPanel( i ).get() != i_rData.aItems[i].pPanel.get() )
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

            Point aCompletePos( io_rData.aGeometry.getFirstItemPosition() );
            Point aIconOnlyPos( aCompletePos );
            Point aTextOnlyPos( aCompletePos );

            for (   size_t i = 0;
                    i < io_rData.rPanelDeck.GetPanelCount();
                    ++i
                )
            {
                PToolPanel pPanel( io_rData.rPanelDeck.GetPanel( i ) );

                ItemDescriptor aItem;
                aItem.pPanel = pPanel;

                Rectangle aContentArea;

                Size aCompleteSize;
                io_rData.pLayout->CalculateItemSize( pPanel, TABITEM_IMAGE_AND_TEXT, aCompleteSize, aContentArea );
                ::std::swap( aCompleteSize.Width(), aCompleteSize.Height() );

                Size aIconOnlySize;
                io_rData.pLayout->CalculateItemSize( pPanel, TABITEM_IMAGE_ONLY, aIconOnlySize, aContentArea );
                ::std::swap( aIconOnlySize.Width(), aIconOnlySize.Height() );

                Size aTextOnlySize;
                io_rData.pLayout->CalculateItemSize( pPanel, TABITEM_TEXT_ONLY, aTextOnlySize, aContentArea );
                ::std::swap( aTextOnlySize.Width(), aTextOnlySize.Height() );

                // TODO: have one method calculating all sizes?

                aItem.aCompleteArea = Rectangle( aCompletePos, aCompleteSize );
                aItem.aIconOnlyArea = Rectangle( aIconOnlyPos, aIconOnlySize );
                aItem.aTextOnlyArea = Rectangle( aTextOnlyPos, aTextOnlySize );

                io_rData.aItems.push_back( aItem );

                aCompletePos = aItem.aCompleteArea.BottomLeft();
                aIconOnlyPos = aItem.aIconOnlyArea.BottomLeft();
                aTextOnlyPos = aItem.aTextOnlyArea.BottomLeft();
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
        Rectangle lcl_getActualItemRect( const PanelTabBar_Data& i_rData, const Rectangle& i_rLogicalItemRect )
        {
            // care for the offset imposed by our geometry, i.e. whether or not we have scroll buttons
            Rectangle aItemRect( i_rLogicalItemRect );

            const TabBarGeometry& rGeometry( i_rData.aGeometry );
            aItemRect.Move(
                rGeometry.isVertical() ? 0 : rGeometry.getItemsRect().Left() - rGeometry.getButtonBackRect().Left(),
                rGeometry.isVertical() ? rGeometry.getItemsRect().Top() - rGeometry.getButtonBackRect().Top() : 0
            );

            // care for the current scroll position
            OSL_ENSURE( i_rData.m_nScrollPosition < i_rData.aItems.size(), "lcl_getActualItemRect: invalid scroll position!" );
            if ( ( i_rData.m_nScrollPosition > 0 ) && ( i_rData.m_nScrollPosition < i_rData.aItems.size() ) )
            {
                long nOffsetX = i_rData.aItems[ i_rData.m_nScrollPosition ].GetCurrentRect().Left() - i_rData.aItems[ 0 ].GetCurrentRect().Left();
                long nOffsetY = i_rData.aItems[ i_rData.m_nScrollPosition ].GetCurrentRect().Top() - i_rData.aItems[ 0 ].GetCurrentRect().Top();
                aItemRect.Move( -nOffsetX, -nOffsetY );
            }

            return aItemRect;
        }

        //--------------------------------------------------------------------------------------------------------------
        static ::boost::optional< size_t > lcl_findItemForPoint( const PanelTabBar_Data& i_rData, const Point& i_rPoint )
        {
            if ( !i_rData.aGeometry.getItemsRect().IsInside( i_rPoint ) )
                return ::boost::optional< size_t >();

            size_t i=0;
            for (   ItemDescriptors::const_iterator item = i_rData.aItems.begin();
                    item != i_rData.aItems.end();
                    ++item, ++i
                )
            {
                Rectangle aItemRect( lcl_getActualItemRect( i_rData, item->GetCurrentRect() ) );
                if ( aItemRect.IsInside( i_rPoint ) )
                {
                    return ::boost::optional< size_t >( i );
                }
            }
            return ::boost::optional< size_t >();
        }

        //--------------------------------------------------------------------------------------------------------------
        class ClipItemRegion
        {
        public:
            ClipItemRegion( const PanelTabBar_Data& i_rData )
                :m_rDevice( i_rData.rTabBar )
            {
                m_rDevice.Push( PUSH_CLIPREGION );
                m_rDevice.SetClipRegion( i_rData.aGeometry.getItemsRect() );
            }

            ~ClipItemRegion()
            {
                m_rDevice.Pop();
            }

        private:
            OutputDevice&   m_rDevice;
        };

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

            if ( i_rData.rPanelDeck.GetPanelCount() - 1 == i_nItemIndex )
                nItemFlags |= ITEM_POSITION_LAST;

            // the actual item pos might differ from the saved one, if we have scroll buttons
            const Point aActualItemPos( lcl_getActualItemRect( i_rData, rItem.GetCurrentRect() ).TopLeft() );

            i_rData.rTabBar.SetUpdateMode( FALSE );
            i_rData.pLayout->DrawItem( i_rData.aGeometry, rItem.pPanel, aActualItemPos, nItemFlags, rItem.eContent );
            i_rData.rTabBar.SetUpdateMode( TRUE );
        }

        //--------------------------------------------------------------------------------------------------------------
        static void lcl_relayout( PanelTabBar_Data& io_rData )
        {
            io_rData.aGeometry.relayout( io_rData.rTabBar.GetOutputSizePixel(), io_rData.aItems );

            const Rectangle& rButtonBack( io_rData.aGeometry.getButtonBackRect() );
            io_rData.m_aScrollBack.SetPosSizePixel( rButtonBack.TopLeft(), rButtonBack.GetSize() );
            io_rData.m_aScrollBack.Show( !rButtonBack.IsEmpty() );

            const Rectangle& rButtonForward( io_rData.aGeometry.getButtonForwardRect() );
            io_rData.m_aScrollForward.SetPosSizePixel( rButtonForward.TopLeft(), rButtonForward.GetSize() );
            io_rData.m_aScrollForward.Show( !rButtonForward.IsEmpty() );

            io_rData.UpdateScrollButtons();
        }
    }

    //==================================================================================================================
    //= PanelTabBar_Data
    //==================================================================================================================
    //------------------------------------------------------------------------------------------------------------------
    void PanelTabBar_Data::ActivePanelChanged( const ::boost::optional< size_t >& i_rOldActive, const ::boost::optional< size_t >& i_rNewActive )
    {
        lcl_ensureItemsCache( *this );

        ClipItemRegion aClipItems( *this );
        if ( !!i_rOldActive )
            lcl_drawItem( *this, *i_rOldActive );
        if ( !!i_rNewActive )
            lcl_drawItem( *this, *i_rNewActive );
    }

    //------------------------------------------------------------------------------------------------------------------
    void PanelTabBar_Data::Dying()
    {
        // not interested in - the notifier is a member of this instance here, so we're dying ourself at the moment
    }

    //==================================================================================================================
    //= PanelTabBar
    //==================================================================================================================
    //------------------------------------------------------------------------------------------------------------------
    PanelTabBar::PanelTabBar( Window& i_rParentWindow, IToolPanelDeck& i_rPanelDeck, const TabAlignment i_eAlignment, const TabItemContent i_eItemContent )
        :Control( &i_rParentWindow, 0 )
        ,m_pData( new PanelTabBar_Data( *this, i_rPanelDeck, i_eAlignment, i_eItemContent ) )
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
        return m_pData->aGeometry.getItemContent();
    }

    //------------------------------------------------------------------------------------------------------------------
    void PanelTabBar::SetTabItemContent( const TabItemContent& i_eItemContent )
    {
        m_pData->aGeometry.setItemContent( i_eItemContent );
        lcl_relayout( *m_pData );
        Invalidate();
    }

    //------------------------------------------------------------------------------------------------------------------
    Size PanelTabBar::GetOptimalSize( WindowSizeType i_eType ) const
    {
        lcl_ensureItemsCache( *m_pData );
        return m_pData->aGeometry.getOptimalSize( m_pData->aItems, i_eType == WINDOWSIZE_MINIMUM );
    }

    //------------------------------------------------------------------------------------------------------------------
    void PanelTabBar::Resize()
    {
        Control::Resize();
        lcl_relayout( *m_pData );
    }

    //------------------------------------------------------------------------------------------------------------------
    void PanelTabBar::Paint( const Rectangle& i_rRect )
    {
        lcl_ensureItemsCache( *m_pData );

        // background
        m_pData->pLayout->DrawBackground( Rectangle( Point(), GetOutputSizePixel() ) );

        // ensure the items really paint into their own playground only
        ClipItemRegion aClipItems( *m_pData );

        // items
        size_t i=0;
        for (   ItemDescriptors::const_iterator item = m_pData->aItems.begin();
                item != m_pData->aItems.end();
                ++item, ++i
            )
        {
            Rectangle aItemRect( lcl_getActualItemRect( *m_pData, item->GetCurrentRect() ) );
            if ( aItemRect.IsOver( i_rRect ) )
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

            ClipItemRegion aClipItems( *m_pData );

            if ( !!aOldItem )
                lcl_drawItem( *m_pData, *aOldItem );

            if ( !!aNewItem )
                lcl_drawItem( *m_pData, *aNewItem );
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

        ClipItemRegion aClipItems( *m_pData );
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
                ClipItemRegion aClipItems( *m_pData );
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
        if ( m_pData->rPanelDeck.GetPanelCount() )
        {
            ::boost::optional< size_t > aActivePanel( m_pData->rPanelDeck.GetActivePanel() );
            if ( !!aActivePanel )
            {
                m_pData->aFocusedItem = aActivePanel;
                ClipItemRegion aClipItems( *m_pData );
                lcl_drawItem( *m_pData, *m_pData->aFocusedItem );
            }
        }
    }

    //------------------------------------------------------------------------------------------------------------------
    void PanelTabBar::LoseFocus()
    {
        Control::LoseFocus();

        ::boost::optional< size_t > aPreviouslyFocused( m_pData->aFocusedItem );
        m_pData->aFocusedItem.reset();

        if ( !!aPreviouslyFocused )
        {
            ClipItemRegion aClipItems( *m_pData );
            lcl_drawItem( *m_pData, *aPreviouslyFocused );
        }
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
        const size_t nPanelCount( m_pData->rPanelDeck.GetPanelCount() );
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

        ClipItemRegion aClipItems( *m_pData );
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
