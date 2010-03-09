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
    //= ITabBarRenderer
    //==================================================================================================================
    class SAL_NO_VTABLE ITabBarRenderer
    {
    public:
        virtual void        renderBackground( const Rectangle& i_rArea ) const = 0;
        virtual Rectangle   calculateDecorations( const Size& i_rContentSize ) const = 0;
        virtual void        preRenderItem( const Rectangle& i_rItemRect, const ItemFlags i_nItemFlags ) const = 0;
        virtual void        postRenderItem( Window& i_rActualWindow, const Rectangle& i_rItemRect, const ItemFlags i_nItemFlags ) const = 0;

        // TODO: postRenderItem takes the "real" output device, i.e. effectively the tab bar. This is because
        // DrawSelectionBackground needs to be applied after everything else is painted. This is weird, and implies
        // that now the renderer needs to know about the TabAlignment, which is No Good (TM).
        // So, we should get rid of postRenderItem completely.
    };
    typedef ::boost::shared_ptr< ITabBarRenderer >  PTabBarRenderer;

    //==================================================================================================================
    //= VCLItemRenderer - declaration
    //==================================================================================================================
    class VCLItemRenderer : public ITabBarRenderer
    {
    public:
        VCLItemRenderer( OutputDevice& i_rTargetDevice )
            :m_rTargetDevice( i_rTargetDevice )
        {
        }

        // ITabBarRenderer
        virtual void        renderBackground( const Rectangle& i_rArea ) const;
        virtual Rectangle   calculateDecorations( const Size& i_rContentSize ) const;
        virtual void        preRenderItem( const Rectangle& i_rItemRect, const ItemFlags i_nItemFlags ) const;
        virtual void        postRenderItem( Window& i_rActualWindow, const Rectangle& i_rItemRect, const ItemFlags i_nItemFlags ) const;

    protected:
        OutputDevice&   getTargetDevice() const { return m_rTargetDevice; }

    private:
        OutputDevice&   m_rTargetDevice;
    };

    //==================================================================================================================
    //= VCLItemRenderer - implementation
    //==================================================================================================================
    //------------------------------------------------------------------------------------------------------------------
    void VCLItemRenderer::renderBackground( const Rectangle& i_rArea ) const
    {
        getTargetDevice().DrawRect( i_rArea );
    }

    //------------------------------------------------------------------------------------------------------------------
    Rectangle VCLItemRenderer::calculateDecorations( const Size& i_rContentSize ) const
    {
        // no decorations at all
        return Rectangle( Point(), i_rContentSize );
    }

    //------------------------------------------------------------------------------------------------------------------
    void VCLItemRenderer::preRenderItem( const Rectangle& i_rItemRect, const ItemFlags i_nItemFlags ) const
    {
        // completely erase the target area. Otherwise, the DrawSelectionBackground from postRender will constantly add up
        getTargetDevice().DrawRect( i_rItemRect );
    }

    //------------------------------------------------------------------------------------------------------------------
    void VCLItemRenderer::postRenderItem( Window& i_rActualWindow, const Rectangle& i_rItemRect, const ItemFlags i_nItemFlags ) const
    {
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
            i_rActualWindow.DrawSelectionBackground(
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


#if 0
    //==================================================================================================================
    //= IItemsLayout
    //==================================================================================================================
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
    }
#endif

    //==================================================================================================================
    //= PanelTabBar_Impl
    //==================================================================================================================
    class PanelTabBar_Impl : public IToolPanelDeckListener
    {
    public:
        PanelTabBar_Impl( PanelTabBar& i_rTabBar, IToolPanelDeck& i_rPanelDeck, const TabAlignment i_eAlignment, const TabItemContent i_eItemContent );

        ~PanelTabBar_Impl()
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

            Relayout();
        }

        virtual void PanelRemoved( const size_t i_nPosition )
        {
            bItemsDirty = true;
            rTabBar.Invalidate();

            if ( i_nPosition < m_nScrollPosition )
                --m_nScrollPosition;

            Relayout();
        }

        virtual void ActivePanelChanged( const ::boost::optional< size_t >& i_rOldActive, const ::boost::optional< size_t >& i_rNewActive );
        virtual void Dying();

        void    UpdateScrollButtons()
        {
            m_aScrollBack.Enable( m_nScrollPosition > 0 );
            m_aScrollForward.Enable( m_nScrollPosition < aItems.size() - 1 );
        }

        void                        Relayout();
        void                        EnsureItemsCache();
        ::boost::optional< size_t > FindItemForPoint( const Point& i_rPoint ) const;
        void                        DrawItem( const size_t i_nItemIndex ) const;
        Rectangle                   GetActualLogicalItemRect( const Rectangle& i_rLogicalItemRect ) const;

        inline bool                 IsVertical() const
        {
            return  (   ( m_eTabAlignment == TABS_LEFT )
                    ||  ( m_eTabAlignment == TABS_RIGHT )
                    );
        }

    protected:
        DECL_LINK( OnScroll, const PushButton* );

        void    impl_calcItemRects();
        Size    impl_calculateItemContentSize( const PToolPanel& i_pPanel, const TabItemContent i_eItemContent ) const;
        void    impl_renderItemContent( const PToolPanel& i_pPanel, const Rectangle& i_rContentArea, const TabItemContent i_eItemContent ) const;

    public:
        PanelTabBar&                rTabBar;
        TabBarGeometry              aGeometry;
        TabAlignment                m_eTabAlignment;
        IToolPanelDeck&             rPanelDeck;

        VirtualDevice               aRenderDevice;
        PTabBarRenderer             pRenderer;

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
    //= helper
    //==================================================================================================================
    namespace
    {
        //--------------------------------------------------------------------------------------------------------------
    #if OSL_DEBUG_LEVEL > 0
        static void lcl_checkConsistency( const PanelTabBar_Impl& i_rImpl )
        {
            if ( !i_rImpl.bItemsDirty )
            {
                if ( i_rImpl.rPanelDeck.GetPanelCount() != i_rImpl.aItems.size() )
                {
                    OSL_ENSURE( false, "lcl_checkConsistency: inconsistent array sizes!" );
                    return;
                }
                for ( size_t i = 0; i < i_rImpl.rPanelDeck.GetPanelCount(); ++i )
                {
                    if ( i_rImpl.rPanelDeck.GetPanel( i ).get() != i_rImpl.aItems[i].pPanel.get() )
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
        class ClipItemRegion
        {
        public:
            ClipItemRegion( const PanelTabBar_Impl& i_rImpl )
                :m_rDevice( i_rImpl.rTabBar )
            {
                m_rDevice.Push( PUSH_CLIPREGION );
                NormalizedArea aNormalizer( Rectangle( Point(), i_rImpl.aRenderDevice.GetOutputSizePixel() ), false );
                m_rDevice.SetClipRegion( aNormalizer.getTransformed( i_rImpl.aGeometry.getItemsRect(), i_rImpl.m_eTabAlignment ) );
            }

            ~ClipItemRegion()
            {
                m_rDevice.Pop();
            }

        private:
            OutputDevice&   m_rDevice;
        };

        //--------------------------------------------------------------------------------------------------------------
        static void lcl_inflateSize( Size& io_rSize, const Inset& i_rInset )
        {
            io_rSize.Width() += i_rInset.nLeft + i_rInset.nRight;
            io_rSize.Height() += i_rInset.nTop + i_rInset.nBottom;
        }

        //--------------------------------------------------------------------------------------------------------------
        static void lcl_deflateRect( Rectangle& io_rArea, const Inset& i_rInset )
        {
            io_rArea.Left() += i_rInset.nLeft;
            io_rArea.Top() += i_rInset.nTop;
            io_rArea.Right() -= i_rInset.nRight;
            io_rArea.Bottom() -= i_rInset.nBottom;
        }
    }

    //==================================================================================================================
    //= PanelTabBar_Impl - implementation
    //==================================================================================================================
    //------------------------------------------------------------------------------------------------------------------
    PanelTabBar_Impl::PanelTabBar_Impl( PanelTabBar& i_rTabBar, IToolPanelDeck& i_rPanelDeck, const TabAlignment i_eAlignment, const TabItemContent i_eItemContent )
        :rTabBar( i_rTabBar )
        ,aGeometry( i_eItemContent )
        ,m_eTabAlignment( i_eAlignment )
        ,rPanelDeck( i_rPanelDeck )
        ,aRenderDevice( i_rTabBar )
        ,pRenderer( new VCLItemRenderer( aRenderDevice ) )
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
            "PanelTabBar_Impl: unsupported alignment!" );

        aRenderDevice.SetLineColor();
        //aRenderDevice.SetFillColor( rTabBar.GetSettings().GetStyleSettings().GetDialogColor() );

        rPanelDeck.AddListener( *this );

        m_aScrollBack.SetSymbol( SYMBOL_ARROW_UP );
        m_aScrollBack.Show();
        m_aScrollBack.SetClickHdl( LINK( this, PanelTabBar_Impl, OnScroll ) );

        m_aScrollForward.SetSymbol( SYMBOL_ARROW_DOWN );
        m_aScrollForward.Show();
        m_aScrollForward.SetClickHdl( LINK( this, PanelTabBar_Impl, OnScroll ) );
    }

    //------------------------------------------------------------------------------------------------------------------
    void PanelTabBar_Impl::impl_calcItemRects()
    {
        aItems.resize(0);

        Point aCompletePos( aGeometry.getFirstItemPosition() );
        Point aIconOnlyPos( aCompletePos );
        Point aTextOnlyPos( aCompletePos );

        for (   size_t i = 0;
                i < rPanelDeck.GetPanelCount();
                ++i
            )
        {
            PToolPanel pPanel( rPanelDeck.GetPanel( i ) );

            ItemDescriptor aItem;
            aItem.pPanel = pPanel;

            Rectangle aContentArea;

            Size aCompleteSize( impl_calculateItemContentSize( pPanel, TABITEM_IMAGE_AND_TEXT ) );
            Size aIconOnlySize( impl_calculateItemContentSize( pPanel, TABITEM_IMAGE_AND_TEXT ) );
            Size aTextOnlySize( impl_calculateItemContentSize( pPanel, TABITEM_IMAGE_AND_TEXT ) );

            // TODO: have one method calculating all sizes?

            // care for the decorations
            const Rectangle aBoundingRect( pRenderer->calculateDecorations( aCompleteSize ) );
            OSL_POSTCOND( ( aBoundingRect.Left() <= 0 ) && ( aBoundingRect.Top() <= 0 ),
                "PanelTabBar_Impl::impl_calcItemRects: hmmm ...." );

            aItem.aContentInset.nLeft = -aBoundingRect.Left();
            aItem.aContentInset.nTop = -aBoundingRect.Top();
            aItem.aContentInset.nRight = aBoundingRect.Right() - aCompleteSize.Width() + 1;
            aItem.aContentInset.nBottom = aBoundingRect.Bottom() - aCompleteSize.Height() + 1;

            lcl_inflateSize( aCompleteSize, aItem.aContentInset );
            lcl_inflateSize( aIconOnlySize, aItem.aContentInset );
            lcl_inflateSize( aTextOnlySize, aItem.aContentInset );

            // remember the three areas
            aItem.aCompleteArea = Rectangle( aCompletePos, aCompleteSize );
            aItem.aIconOnlyArea = Rectangle( aIconOnlyPos, aIconOnlySize );
            aItem.aTextOnlyArea = Rectangle( aTextOnlyPos, aTextOnlySize );

            aItems.push_back( aItem );

            aCompletePos = aItem.aCompleteArea.TopRight();
            aIconOnlyPos = aItem.aIconOnlyArea.TopRight();
            aTextOnlyPos = aItem.aTextOnlyArea.TopRight();
        }

        bItemsDirty = false;
    }

    //------------------------------------------------------------------------------------------------------------------
    Size PanelTabBar_Impl::impl_calculateItemContentSize( const PToolPanel& i_pPanel, const TabItemContent i_eItemContent ) const
    {
        // calculate the size needed for the content
        OSL_ENSURE( i_eItemContent != TABITEM_AUTO, "PanelTabBar_Impl::impl_calculateItemContentSize: illegal TabItemContent value!" );

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
            const Size aTextSize( rTabBar.GetCtrlTextWidth( sItemText ), rTabBar.GetTextHeight() );
            aItemContentSize.Width() += aTextSize.Width();
            aItemContentSize.Height() = ::std::max( aItemContentSize.Height(), aTextSize.Height() );

            aItemContentSize.Width() += 2 * ITEM_TEXT_FLOW_SPACE;
        }

        aItemContentSize.Width() += 2 * ITEM_OUTER_SPACE;
        aItemContentSize.Height() += 2 * ITEM_OUTER_SPACE;

        return aItemContentSize;
    }

    //------------------------------------------------------------------------------------------------------------------
    void PanelTabBar_Impl::impl_renderItemContent( const PToolPanel& i_pPanel, const Rectangle& i_rContentArea, const TabItemContent i_eItemContent ) const
    {
        OSL_ENSURE( i_eItemContent != TABITEM_AUTO, "PanelTabBar_Impl::impl_renderItemContent: illegal TabItemContent value!" );

        Rectangle aRenderArea( i_rContentArea );
        if ( IsVertical() )
        {
            aRenderArea.Top() += ITEM_OUTER_SPACE;
        }
        else
        {
            aRenderArea.Left() += ITEM_OUTER_SPACE;
        }

        // draw the image
        const Image aItemImage( i_pPanel->GetImage() );
        const Size aImageSize( aItemImage.GetSizePixel() );
        const bool bUseImage = !!aItemImage && ( i_eItemContent != TABITEM_TEXT_ONLY );

        if ( bUseImage )
        {
            Point aImagePos;
            if ( IsVertical() )
            {
                aImagePos.X() = aRenderArea.Left() + ( aRenderArea.GetWidth() - aImageSize.Width() ) / 2;
                aImagePos.Y() = aRenderArea.Top();
            }
            else
            {
                aImagePos.X() = aRenderArea.Left();
                aImagePos.Y() = aRenderArea.Top() + ( aRenderArea.GetHeight() - aImageSize.Height() ) / 2;
            }
            rTabBar.DrawImage( aImagePos, aItemImage );
        }

        const ::rtl::OUString sItemText( i_pPanel->GetDisplayName() );
        const bool bUseText = ( sItemText.getLength() != 0 ) && ( i_eItemContent != TABITEM_IMAGE_ONLY );

        if ( bUseText )
        {
            if ( IsVertical() )
            {
                if ( bUseImage )
                    aRenderArea.Top() += aImageSize.Height() + ITEM_ICON_TEXT_DISTANCE;
                aRenderArea.Top() += ITEM_TEXT_FLOW_SPACE;
            }
            else
            {
                if ( bUseImage )
                    aRenderArea.Left() += aImageSize.Width() + ITEM_ICON_TEXT_DISTANCE;
                aRenderArea.Left() += ITEM_TEXT_FLOW_SPACE;
            }

            // draw the text
            const Size aTextSize( rTabBar.GetCtrlTextWidth( sItemText ), rTabBar.GetTextHeight() );
            Point aTextPos( aRenderArea.TopLeft() );
            if ( IsVertical() )
            {
                rTabBar.Push( PUSH_FONT );

                Font aFont( rTabBar.GetFont() );
                aFont.SetOrientation( 2700 );
                aFont.SetVertical( TRUE );
                rTabBar.SetFont( aFont );

                aTextPos.X() += aTextSize.Height();
                aTextPos.X() += ( aRenderArea.GetWidth() - aTextSize.Height() ) / 2;
            }
            else
            {
                aTextPos.Y() += ( aRenderArea.GetHeight() - aTextSize.Height() ) / 2;
            }

            rTabBar.DrawText( aTextPos, sItemText );

            if ( IsVertical() )
            {
                rTabBar.Pop();
            }
        }
    }

    //------------------------------------------------------------------------------------------------------------------
    void PanelTabBar_Impl::DrawItem( const size_t i_nItemIndex ) const
    {
        const ItemDescriptor& rItem( aItems[ i_nItemIndex ] );

        ItemFlags nItemFlags( ITEM_STATE_NORMAL );
        if ( aHoveredItem == i_nItemIndex )
        {
            nItemFlags |= ITEM_STATE_HOVERED;
            if ( bMouseButtonDown )
                nItemFlags |= ITEM_STATE_ACTIVE;
        }

        if ( rPanelDeck.GetActivePanel() == i_nItemIndex )
            nItemFlags |= ITEM_STATE_ACTIVE;

        if ( aFocusedItem == i_nItemIndex )
            nItemFlags |= ITEM_STATE_FOCUSED;

        if ( 0 == i_nItemIndex )
            nItemFlags |= ITEM_POSITION_FIRST;

        if ( rPanelDeck.GetPanelCount() - 1 == i_nItemIndex )
            nItemFlags |= ITEM_POSITION_LAST;

        rTabBar.SetUpdateMode( FALSE );

//        // the actual item pos might differ from the saved one, if we have scroll buttons
//        const Point aActualItemPos( GetActualLogicalItemRect( rItem.GetCurrentRect() ).TopLeft() );
//        pLayout->DrawItem( aGeometry, rItem.pPanel, aActualItemPos, nItemFlags, rItem.eContent );

        // render item "background" layer
        pRenderer->preRenderItem( rItem.GetCurrentRect(), nItemFlags );

        // copy from the virtual device to ourself
        const NormalizedArea aNormalizer( Rectangle( Point(), aRenderDevice.GetOutputSizePixel() ), false );
        const Rectangle aActualItemRect = aNormalizer.getTransformed( rItem.GetCurrentRect(), m_eTabAlignment );

        BitmapEx aBitmap( aRenderDevice.GetBitmapEx( rItem.GetCurrentRect().TopLeft(), rItem.GetCurrentRect().GetSize() ) );
        if ( IsVertical() )
        {
            aBitmap.Rotate( 2700, COL_BLACK );
            if ( m_eTabAlignment == TABS_LEFT )
                aBitmap.Mirror( BMP_MIRROR_HORZ );
        }
//        else if ( m_eTabAlignment == TABS_BOTTOM )
//        {
//                aBitmap.Mirror( BMP_MIRROR_VERT );
//        }
        rTabBar.DrawBitmapEx( aActualItemRect.TopLeft(), aBitmap );

        // render the actual item content
        Rectangle aActualContentRect( aActualItemRect );
        lcl_deflateRect( aActualContentRect, rItem.aContentInset );
        impl_renderItemContent( rItem.pPanel, aActualContentRect, aGeometry.getItemContent() );

        // render item "foreground" layer
        pRenderer->postRenderItem( rTabBar, aActualItemRect, nItemFlags );

        rTabBar.SetUpdateMode( TRUE );
    }

    //------------------------------------------------------------------------------------------------------------------
    void PanelTabBar_Impl::EnsureItemsCache()
    {
        if ( bItemsDirty == false )
        {
            DBG_CHECK( *this );
            return;
        }
        impl_calcItemRects();
        OSL_POSTCOND( bItemsDirty == false, "EnsureItemsCache: cache still dirty!" );
        DBG_CHECK( *this );
    }

    //------------------------------------------------------------------------------------------------------------------
    void PanelTabBar_Impl::Relayout()
    {
        EnsureItemsCache();

        const Size aOutputSize( rTabBar.GetOutputSizePixel() );
        NormalizedArea aNormalizer( Rectangle( Point(), aOutputSize ), IsVertical() );
        const Size aLogicalOutputSize( aNormalizer.getReferenceSize() );

        // forward actual output size to our render device
        aRenderDevice.SetOutputSizePixel( aLogicalOutputSize );

        // re-calculate the size of the scroll buttons and of the items
        aGeometry.relayout( aLogicalOutputSize, aItems );

        const Rectangle aButtonBack( aNormalizer.getTransformed( aGeometry.getButtonBackRect(), m_eTabAlignment ) );
        m_aScrollBack.SetPosSizePixel( aButtonBack.TopLeft(), aButtonBack.GetSize() );
        m_aScrollBack.Show( !aButtonBack.IsEmpty() );

        const Rectangle aButtonForward( aNormalizer.getTransformed( aGeometry.getButtonForwardRect(), m_eTabAlignment ) );
        m_aScrollForward.SetPosSizePixel( aButtonForward.TopLeft(), aButtonForward.GetSize() );
        m_aScrollForward.Show( !aButtonForward.IsEmpty() );

        UpdateScrollButtons();
    }

    //------------------------------------------------------------------------------------------------------------------
    ::boost::optional< size_t > PanelTabBar_Impl::FindItemForPoint( const Point& i_rPoint ) const
    {
        Point aPoint( IsVertical() ? i_rPoint.Y() : i_rPoint.X(), IsVertical() ? i_rPoint.X() : i_rPoint.Y() );

        if ( !aGeometry.getItemsRect().IsInside( aPoint ) )
            return ::boost::optional< size_t >();

        size_t i=0;
        for (   ItemDescriptors::const_iterator item = aItems.begin();
                item != aItems.end();
                ++item, ++i
            )
        {
            Rectangle aItemRect( GetActualLogicalItemRect( item->GetCurrentRect() ) );
            if ( aItemRect.IsInside( aPoint ) )
            {
                return ::boost::optional< size_t >( i );
            }
        }
        return ::boost::optional< size_t >();
    }
    //------------------------------------------------------------------------------------------------------------------
    IMPL_LINK( PanelTabBar_Impl, OnScroll, const PushButton*, i_pButton )
    {
        if ( i_pButton == &m_aScrollBack )
        {
            OSL_ENSURE( m_nScrollPosition > 0, "PanelTabBar_Impl::OnScroll: inconsistency!" );
            --m_nScrollPosition;
            rTabBar.Invalidate();
        }
        else if ( i_pButton == &m_aScrollForward )
        {
            OSL_ENSURE( m_nScrollPosition < aItems.size() - 1, "PanelTabBar_Impl::OnScroll: inconsistency!" );
            ++m_nScrollPosition;
            rTabBar.Invalidate();
        }

        UpdateScrollButtons();

        return 0L;
    }

    //------------------------------------------------------------------------------------------------------------------
    Rectangle PanelTabBar_Impl::GetActualLogicalItemRect( const Rectangle& i_rLogicalItemRect ) const
    {
        // care for the offset imposed by our geometry, i.e. whether or not we have scroll buttons
        Rectangle aItemRect( i_rLogicalItemRect );
        aItemRect.Move( aGeometry.getItemsRect().Left() - aGeometry.getButtonBackRect().Left(), 0 );

        // care for the current scroll position
        OSL_ENSURE( m_nScrollPosition < aItems.size(), "GetActualLogicalItemRect: invalid scroll position!" );
        if ( ( m_nScrollPosition > 0 ) && ( m_nScrollPosition < aItems.size() ) )
        {
            long nOffsetX = aItems[ m_nScrollPosition ].GetCurrentRect().Left() - aItems[ 0 ].GetCurrentRect().Left();
            long nOffsetY = aItems[ m_nScrollPosition ].GetCurrentRect().Top() - aItems[ 0 ].GetCurrentRect().Top();
            aItemRect.Move( -nOffsetX, -nOffsetY );
        }

        return aItemRect;
    }

    //==================================================================================================================
    //= PanelTabBar_Impl
    //==================================================================================================================
    //------------------------------------------------------------------------------------------------------------------
    void PanelTabBar_Impl::ActivePanelChanged( const ::boost::optional< size_t >& i_rOldActive, const ::boost::optional< size_t >& i_rNewActive )
    {
        EnsureItemsCache();

        ClipItemRegion aClipItems( *this );
        if ( !!i_rOldActive )
            DrawItem( *i_rOldActive );
        if ( !!i_rNewActive )
            DrawItem( *i_rNewActive );
    }

    //------------------------------------------------------------------------------------------------------------------
    void PanelTabBar_Impl::Dying()
    {
        // not interested in - the notifier is a member of this instance here, so we're dying ourself at the moment
    }

    //==================================================================================================================
    //= PanelTabBar
    //==================================================================================================================
    //------------------------------------------------------------------------------------------------------------------
    PanelTabBar::PanelTabBar( Window& i_rParentWindow, IToolPanelDeck& i_rPanelDeck, const TabAlignment i_eAlignment, const TabItemContent i_eItemContent )
        :Control( &i_rParentWindow, 0 )
        ,m_pImpl( new PanelTabBar_Impl( *this, i_rPanelDeck, i_eAlignment, i_eItemContent ) )
    {
        DBG_CHECK( *m_pImpl );
    }

    //------------------------------------------------------------------------------------------------------------------
    PanelTabBar::~PanelTabBar()
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    TabItemContent PanelTabBar::GetTabItemContent() const
    {
        return m_pImpl->aGeometry.getItemContent();
    }

    //------------------------------------------------------------------------------------------------------------------
    void PanelTabBar::SetTabItemContent( const TabItemContent& i_eItemContent )
    {
        m_pImpl->aGeometry.setItemContent( i_eItemContent );
        m_pImpl->Relayout();
        Invalidate();
    }

    //------------------------------------------------------------------------------------------------------------------
    Size PanelTabBar::GetOptimalSize( WindowSizeType i_eType ) const
    {
        m_pImpl->EnsureItemsCache();
        Size aOptimalSize( m_pImpl->aGeometry.getOptimalSize( m_pImpl->aItems, i_eType == WINDOWSIZE_MINIMUM ) );
        if ( m_pImpl->IsVertical() )
            ::std::swap( aOptimalSize.Width(), aOptimalSize.Height() );
        return aOptimalSize;
    }

    //------------------------------------------------------------------------------------------------------------------
    void PanelTabBar::Resize()
    {
        Control::Resize();
        m_pImpl->Relayout();
    }

    //------------------------------------------------------------------------------------------------------------------
    void PanelTabBar::Paint( const Rectangle& i_rRect )
    {
        m_pImpl->EnsureItemsCache();

        // background
        m_pImpl->pRenderer->renderBackground( Rectangle( Point(), GetOutputSizePixel() ) );

        // ensure the items really paint into their own playground only
        ClipItemRegion aClipItems( *m_pImpl );

        NormalizedArea aTransformer( Rectangle( Point(), m_pImpl->aRenderDevice.GetOutputSizePixel() ), false );
            // TODO: make the transformer a member
        const Rectangle aLogicalPaintRect( aTransformer.getNormalized( i_rRect, m_pImpl->m_eTabAlignment ) );

        // items
        size_t i=0;
        for (   ItemDescriptors::const_iterator item = m_pImpl->aItems.begin();
                item != m_pImpl->aItems.end();
                ++item, ++i
            )
        {
            Rectangle aItemRect( m_pImpl->GetActualLogicalItemRect( item->GetCurrentRect() ) );
            if ( aItemRect.IsOver( aLogicalPaintRect ) )
            {
                m_pImpl->DrawItem( i );
            }
        }
    }

    //------------------------------------------------------------------------------------------------------------------
    void PanelTabBar::MouseMove( const MouseEvent& i_rMouseEvent )
    {
        m_pImpl->EnsureItemsCache();

        ::boost::optional< size_t > aOldItem( m_pImpl->aHoveredItem );
        ::boost::optional< size_t > aNewItem( m_pImpl->FindItemForPoint( i_rMouseEvent.GetPosPixel() ) );

        if  ( i_rMouseEvent.IsLeaveWindow() )
            aNewItem.reset();

        if ( aOldItem != aNewItem )
        {
            m_pImpl->aHoveredItem = aNewItem;

            ClipItemRegion aClipItems( *m_pImpl );

            if ( !!aOldItem )
                m_pImpl->DrawItem( *aOldItem );

            if ( !!aNewItem )
                m_pImpl->DrawItem( *aNewItem );
        }
    }

    //------------------------------------------------------------------------------------------------------------------
    void PanelTabBar::MouseButtonDown( const MouseEvent& i_rMouseEvent )
    {
        Control::MouseButtonDown( i_rMouseEvent );

        if ( !i_rMouseEvent.IsLeft() )
            return;

        m_pImpl->EnsureItemsCache();

        ::boost::optional< size_t > aHitItem( m_pImpl->FindItemForPoint( i_rMouseEvent.GetPosPixel() ) );
        if ( !aHitItem )
            return;

        CaptureMouse();
        m_pImpl->bMouseButtonDown = true;

        ClipItemRegion aClipItems( *m_pImpl );
        m_pImpl->DrawItem( *aHitItem );
    }

    //------------------------------------------------------------------------------------------------------------------
    void PanelTabBar::MouseButtonUp( const MouseEvent& i_rMouseEvent )
    {
        Control::MouseButtonUp( i_rMouseEvent );

        if ( m_pImpl->bMouseButtonDown )
        {
            OSL_ENSURE( IsMouseCaptured(), "PanelTabBar::MouseButtonUp: inconsistency!" );
            if ( IsMouseCaptured() )
                ReleaseMouse();
            m_pImpl->bMouseButtonDown = false;

            ::boost::optional< size_t > aHitItem( m_pImpl->FindItemForPoint( i_rMouseEvent.GetPosPixel() ) );
            if ( !!aHitItem )
            {
                // re-draw that item now that we're not in mouse-down mode anymore
                ClipItemRegion aClipItems( *m_pImpl );
                m_pImpl->DrawItem( *aHitItem );
                // activate the respective panel
                m_pImpl->rPanelDeck.ActivatePanel( *aHitItem );
            }
        }
    }

    //------------------------------------------------------------------------------------------------------------------
    void PanelTabBar::RequestHelp( const HelpEvent& i_rHelpEvent )
    {
        m_pImpl->EnsureItemsCache();

        ::boost::optional< size_t > aHelpItem( m_pImpl->FindItemForPoint( ScreenToOutputPixel( i_rHelpEvent.GetMousePosPixel() ) ) );
        if ( !aHelpItem )
            return;

        const ItemDescriptor& rItem( m_pImpl->aItems[ *aHelpItem ] );
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
        if ( m_pImpl->rPanelDeck.GetPanelCount() )
        {
            ::boost::optional< size_t > aActivePanel( m_pImpl->rPanelDeck.GetActivePanel() );
            if ( !!aActivePanel )
            {
                m_pImpl->aFocusedItem = aActivePanel;
                ClipItemRegion aClipItems( *m_pImpl );
                m_pImpl->DrawItem( *m_pImpl->aFocusedItem );
            }
        }
    }

    //------------------------------------------------------------------------------------------------------------------
    void PanelTabBar::LoseFocus()
    {
        Control::LoseFocus();

        ::boost::optional< size_t > aPreviouslyFocused( m_pImpl->aFocusedItem );
        m_pImpl->aFocusedItem.reset();

        if ( !!aPreviouslyFocused )
        {
            ClipItemRegion aClipItems( *m_pImpl );
            m_pImpl->DrawItem( *aPreviouslyFocused );
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
        const size_t nPanelCount( m_pImpl->rPanelDeck.GetPanelCount() );
        if ( nPanelCount < 2 )
            return;

        OSL_PRECOND( !!m_pImpl->aFocusedItem, "PanelTabBar::KeyInput: we should have a focused item here!" );
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
            m_pImpl->rPanelDeck.ActivatePanel( *m_pImpl->aFocusedItem );
            break;
        }

        if ( !bFocusNext && !bFocusPrev )
            return;

        const size_t nOldFocus = *m_pImpl->aFocusedItem;
        if ( bFocusNext )
        {
            m_pImpl->aFocusedItem.reset( ( *m_pImpl->aFocusedItem + 1 ) % nPanelCount );
        }
        else
        {
            m_pImpl->aFocusedItem.reset( ( *m_pImpl->aFocusedItem + nPanelCount - 1 ) % nPanelCount );
        }

        ClipItemRegion aClipItems( *m_pImpl );
        m_pImpl->DrawItem( nOldFocus );
        m_pImpl->DrawItem( *m_pImpl->aFocusedItem );
    }

    //------------------------------------------------------------------------------------------------------------------
    void PanelTabBar::DataChanged( const DataChangedEvent& i_rDataChanedEvent )
    {
        Control::DataChanged( i_rDataChanedEvent );

        if  (   ( i_rDataChanedEvent.GetType() == DATACHANGED_SETTINGS )
            &&  ( ( i_rDataChanedEvent.GetFlags() & SETTINGS_STYLE ) != 0 )
            )
        {
            //SetFillColor( GetSettings().GetStyleSettings().GetDialogColor() );
            Invalidate();
        }
    }

//........................................................................
} // namespace svt
//........................................................................
