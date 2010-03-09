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
        /** fills the background of our target device
        */
        virtual void        renderBackground() const = 0;
        virtual Rectangle   calculateDecorations( const Size& i_rContentSize ) const = 0;
        virtual void        preRenderItem( const Rectangle& i_rContentRect, const ItemFlags i_nItemFlags ) const = 0;
        virtual void        postRenderItem( Window& i_rActualWindow, const Rectangle& i_rItemRect, const ItemFlags i_nItemFlags ) const = 0;

        // TODO: postRenderItem takes the "real" window, i.e. effectively the tab bar. This is because
        // DrawSelectionBackground needs to be applied after everything else is painted, and is available at the Window
        // class, but not at the OutputDevice. This makes the API somewhat weird, as we're now mixing operations on the
        // target device, done in a normalized geometry, with operations on the window, done in a transformed geometry.
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
        virtual void        renderBackground() const;
        virtual Rectangle   calculateDecorations( const Size& i_rContentSize ) const;
        virtual void        preRenderItem( const Rectangle& i_rContentRect, const ItemFlags i_nItemFlags ) const;
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
    void VCLItemRenderer::renderBackground() const
    {
        getTargetDevice().DrawRect( Rectangle( Point(), getTargetDevice().GetOutputSizePixel() ) );
    }

    //------------------------------------------------------------------------------------------------------------------
    Rectangle VCLItemRenderer::calculateDecorations( const Size& i_rContentSize ) const
    {
        // no decorations at all
        return Rectangle( Point(), i_rContentSize );
    }

    //------------------------------------------------------------------------------------------------------------------
    void VCLItemRenderer::preRenderItem( const Rectangle& i_rContentRect, const ItemFlags i_nItemFlags ) const
    {
        (void)i_rContentRect;
        (void)i_nItemFlags;
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

    //==================================================================================================================
    //= NWFToolboxItemRenderer - declaration
    //==================================================================================================================
    class NWFToolboxItemRenderer : public ITabBarRenderer
    {
    public:
        NWFToolboxItemRenderer( OutputDevice& i_rTargetDevice )
            :m_rTargetDevice( i_rTargetDevice )
        {
        }

        // ITabBarRenderer
        virtual void        renderBackground() const;
        virtual Rectangle   calculateDecorations( const Size& i_rContentSize ) const;
        virtual void        preRenderItem( const Rectangle& i_rContentRect, const ItemFlags i_nItemFlags ) const;
        virtual void        postRenderItem( Window& i_rActualWindow, const Rectangle& i_rItemRect, const ItemFlags i_nItemFlags ) const;

    protected:
        OutputDevice&   getTargetDevice() const { return m_rTargetDevice; }

    private:
        OutputDevice&   m_rTargetDevice;
    };

    //==================================================================================================================
    //= NWFToolboxItemRenderer - implementation
    //==================================================================================================================
    //------------------------------------------------------------------------------------------------------------------
    void NWFToolboxItemRenderer::renderBackground() const
    {
        getTargetDevice().DrawRect( Rectangle( Point(), getTargetDevice().GetOutputSizePixel() ) );
    }

    //------------------------------------------------------------------------------------------------------------------
    Rectangle NWFToolboxItemRenderer::calculateDecorations( const Size& i_rContentSize ) const
    {
        // don't ask GetNativeControlRegion, this will not deliver proper results in all cases.
        // Instead, simply assume that both the content and the bounding region are the same.
//        const ImplControlValue aControlValue;
//        bool bNativeOK = m_rTargetWindow.GetNativeControlRegion(
//            CTRL_TOOLBAR, PART_BUTTON,
//            Rectangle( Point(), i_rContentSize ), CTRL_STATE_ENABLED | CTRL_STATE_ROLLOVER,
//            aControlValue, ::rtl::OUString(),
//            aBoundingRegion, aContentRegion
//        );
        return Rectangle(
            Point( -1, -1 ),
            Size( i_rContentSize.Width() + 2, i_rContentSize.Height() + 2 )
        );
    }

    //------------------------------------------------------------------------------------------------------------------
    void NWFToolboxItemRenderer::preRenderItem( const Rectangle& i_rContentRect, const ItemFlags i_nItemFlags ) const
    {
        ControlState nState = CTRL_STATE_ENABLED;
        if ( i_nItemFlags & ITEM_STATE_FOCUSED )    nState |= CTRL_STATE_FOCUSED;
        if ( i_nItemFlags & ITEM_STATE_HOVERED )    nState |= CTRL_STATE_ROLLOVER;
        if ( i_nItemFlags & ITEM_STATE_ACTIVE )     nState |= CTRL_STATE_SELECTED;

        ImplControlValue aControlValue;
        aControlValue.setTristateVal( ( i_nItemFlags & ITEM_STATE_ACTIVE ) ? BUTTONVALUE_ON : BUTTONVALUE_OFF );

        bool bNativeOK = getTargetDevice().DrawNativeControl( CTRL_TOOLBAR, PART_BUTTON, i_rContentRect, nState, aControlValue, rtl::OUString() );
        (void)bNativeOK;
        OSL_ENSURE( bNativeOK, "NWFToolboxItemRenderer::preRenderItem: inconsistent NWF implementation!" );
            // IsNativeControlSupported returned true, previously, otherwise we would not be here ...
    }

    //------------------------------------------------------------------------------------------------------------------
    void NWFToolboxItemRenderer::postRenderItem( Window& i_rActualWindow, const Rectangle& i_rItemRect, const ItemFlags i_nItemFlags ) const
    {
        (void)i_rActualWindow;
        (void)i_rItemRect;
        (void)i_nItemFlags;
    }

    //==================================================================================================================
    //= NWFTabItemRenderer - declaration
    //==================================================================================================================
    class NWFTabItemRenderer : public ITabBarRenderer
    {
    public:
        NWFTabItemRenderer( OutputDevice& i_rTargetDevice )
            :m_rTargetDevice( i_rTargetDevice )
        {
        }

        // ITabBarRenderer
        virtual void        renderBackground() const;
        virtual Rectangle   calculateDecorations( const Size& i_rContentSize ) const;
        virtual void        preRenderItem( const Rectangle& i_rContentRect, const ItemFlags i_nItemFlags ) const;
        virtual void        postRenderItem( Window& i_rActualWindow, const Rectangle& i_rItemRect, const ItemFlags i_nItemFlags ) const;

    protected:
        OutputDevice&   getTargetDevice() const { return m_rTargetDevice; }

    private:
        OutputDevice&   m_rTargetDevice;
    };

    //==================================================================================================================
    //= NWFTabItemRenderer - implementation
    //==================================================================================================================
    //------------------------------------------------------------------------------------------------------------------
    void NWFTabItemRenderer::renderBackground() const
    {
        getTargetDevice().DrawRect( Rectangle( Point(), getTargetDevice().GetOutputSizePixel() ) );
    }

    //------------------------------------------------------------------------------------------------------------------
    Rectangle NWFTabItemRenderer::calculateDecorations( const Size& i_rContentSize ) const
    {
        TabitemValue tiValue;
        ImplControlValue aControlValue( (void*)(&tiValue) );

        Region aBoundingRegion, aContentRegion;
        bool bNativeOK = getTargetDevice().GetNativeControlRegion(
            CTRL_TAB_ITEM, PART_ENTIRE_CONTROL,
            Rectangle( Point(), i_rContentSize ),
            CTRL_STATE_ENABLED | CTRL_STATE_FOCUSED | CTRL_STATE_ROLLOVER | CTRL_STATE_SELECTED,
            aControlValue, ::rtl::OUString(),
            aBoundingRegion, aContentRegion
        );
        (void)bNativeOK;
        OSL_ENSURE( bNativeOK, "NWFTabItemRenderer::calculateDecorations: GetNativeControlRegion not implemented for CTRL_TAB_ITEM?!" );

        return aBoundingRegion.GetBoundRect();
    }

    //------------------------------------------------------------------------------------------------------------------
    void NWFTabItemRenderer::preRenderItem( const Rectangle& i_rContentRect, const ItemFlags i_nItemFlags ) const
    {
        ControlState nState = CTRL_STATE_ENABLED;
        if ( i_nItemFlags & ITEM_STATE_FOCUSED )    nState |= CTRL_STATE_FOCUSED;
        if ( i_nItemFlags & ITEM_STATE_HOVERED )    nState |= CTRL_STATE_ROLLOVER;
        if ( i_nItemFlags & ITEM_STATE_ACTIVE )     nState |= CTRL_STATE_SELECTED;

        TabitemValue tiValue;
        if ( i_nItemFlags & ITEM_POSITION_FIRST )
            tiValue.mnAlignment |= TABITEM_FIRST_IN_GROUP;
        if ( i_nItemFlags & ITEM_POSITION_LAST )
            tiValue.mnAlignment |= TABITEM_LAST_IN_GROUP;

        ImplControlValue aControlValue( (void *)(&tiValue) );

        bool bNativeOK = getTargetDevice().DrawNativeControl( CTRL_TAB_ITEM, PART_ENTIRE_CONTROL, i_rContentRect, nState, aControlValue, rtl::OUString() );
        (void)bNativeOK;
        OSL_ENSURE( bNativeOK, "NWFTabItemRenderer::preRenderItem: inconsistent NWF implementation!" );
            // IsNativeControlSupported returned true, previously, otherwise we would not be here ...
    }

    //------------------------------------------------------------------------------------------------------------------
    void NWFTabItemRenderer::postRenderItem( Window& i_rActualWindow, const Rectangle& i_rItemRect, const ItemFlags i_nItemFlags ) const
    {
        (void)i_rActualWindow;
        (void)i_rItemRect;
        (void)i_nItemFlags;
    }

    //==================================================================================================================
    //= PanelTabBar_Impl
    //==================================================================================================================
    class PanelTabBar_Impl : public IToolPanelDeckListener
    {
    public:
        PanelTabBar_Impl( PanelTabBar& i_rTabBar, IToolPanelDeck& i_rPanelDeck, const TabAlignment i_eAlignment, const TabItemContent i_eItemContent );

        ~PanelTabBar_Impl()
        {
            m_rPanelDeck.RemoveListener( *this );
        }

        // IToolPanelDeckListener
        virtual void PanelInserted( const PToolPanel& i_pPanel, const size_t i_nPosition )
        {
            (void)i_pPanel;
            (void)i_nPosition;
            m_bItemsDirty = true;
            m_rTabBar.Invalidate();

            Relayout();
        }

        virtual void PanelRemoved( const size_t i_nPosition )
        {
            m_bItemsDirty = true;
            m_rTabBar.Invalidate();

            if ( i_nPosition < m_nScrollPosition )
                --m_nScrollPosition;

            Relayout();
        }

        virtual void ActivePanelChanged( const ::boost::optional< size_t >& i_rOldActive, const ::boost::optional< size_t >& i_rNewActive );
        virtual void Dying();

        void    UpdateScrollButtons()
        {
            m_aScrollBack.Enable( m_nScrollPosition > 0 );
            m_aScrollForward.Enable( m_nScrollPosition < m_aItems.size() - 1 );
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
        PanelTabBar&                m_rTabBar;
        TabBarGeometry              m_aGeometry;
        NormalizedArea              m_aNormalizer;
        TabAlignment                m_eTabAlignment;
        IToolPanelDeck&             m_rPanelDeck;

        VirtualDevice               m_aRenderDevice;
        PTabBarRenderer             m_pRenderer;

        ::boost::optional< size_t > m_aHoveredItem;
        ::boost::optional< size_t > m_aFocusedItem;
        bool                        m_bMouseButtonDown;

        ItemDescriptors             m_aItems;
        bool                        m_bItemsDirty;

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
            if ( !i_rImpl.m_bItemsDirty )
            {
                if ( i_rImpl.m_rPanelDeck.GetPanelCount() != i_rImpl.m_aItems.size() )
                {
                    OSL_ENSURE( false, "lcl_checkConsistency: inconsistent array sizes!" );
                    return;
                }
                for ( size_t i = 0; i < i_rImpl.m_rPanelDeck.GetPanelCount(); ++i )
                {
                    if ( i_rImpl.m_rPanelDeck.GetPanel( i ).get() != i_rImpl.m_aItems[i].pPanel.get() )
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
                :m_rDevice( i_rImpl.m_rTabBar )
            {
                m_rDevice.Push( PUSH_CLIPREGION );
                m_rDevice.SetClipRegion( i_rImpl.m_aNormalizer.getTransformed( i_rImpl.m_aGeometry.getItemsRect(), i_rImpl.m_eTabAlignment ) );
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
        :m_rTabBar( i_rTabBar )
        ,m_aGeometry( i_eItemContent )
        ,m_aNormalizer()
        ,m_eTabAlignment( i_eAlignment )
        ,m_rPanelDeck( i_rPanelDeck )
        ,m_aRenderDevice( i_rTabBar )
        ,m_pRenderer()
        ,m_aHoveredItem()
        ,m_aFocusedItem()
        ,m_bMouseButtonDown( false )
        ,m_aItems()
        ,m_bItemsDirty( true )
        ,m_aScrollBack( &i_rTabBar, WB_BEVELBUTTON )
        ,m_aScrollForward( &i_rTabBar, WB_BEVELBUTTON )
        ,m_nScrollPosition( 0 )
    {
#ifdef WNT
        if ( m_aRenderDevice.IsNativeControlSupported( CTRL_TAB_ITEM, PART_ENTIRE_CONTROL ) )
            // this mode requires the NWF framework to be able to render those items onto a virtual
            // device. For some frameworks (some GTK themes, in particular), this is known to fail.
            // So, be on the safe side for the moment.
            m_pRenderer.reset( new NWFTabItemRenderer( m_aRenderDevice ) );
        else
#endif
        if ( m_aRenderDevice.IsNativeControlSupported( CTRL_TOOLBAR, PART_BUTTON ) )
            m_pRenderer.reset( new NWFToolboxItemRenderer( m_aRenderDevice ) );
        else
            m_pRenderer.reset( new VCLItemRenderer( m_aRenderDevice ) );

        OSL_ENSURE( ( i_eAlignment == TABS_LEFT ) || ( i_eAlignment == TABS_RIGHT ),
            "PanelTabBar_Impl::PanelTabBar_Impl: unsupported alignment!" );

        m_aRenderDevice.SetLineColor();
        //m_aRenderDevice.SetFillColor( m_rTabBar.GetSettings().GetStyleSettings().GetDialogColor() );

        m_rPanelDeck.AddListener( *this );

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
        m_aItems.resize(0);

        Point aCompletePos( m_aGeometry.getFirstItemPosition() );
        Point aIconOnlyPos( aCompletePos );
        Point aTextOnlyPos( aCompletePos );

        for (   size_t i = 0;
                i < m_rPanelDeck.GetPanelCount();
                ++i
            )
        {
            PToolPanel pPanel( m_rPanelDeck.GetPanel( i ) );

            ItemDescriptor aItem;
            aItem.pPanel = pPanel;

            Rectangle aContentArea;

            Size aCompleteSize( impl_calculateItemContentSize( pPanel, TABITEM_IMAGE_AND_TEXT ) );
            Size aIconOnlySize( impl_calculateItemContentSize( pPanel, TABITEM_IMAGE_AND_TEXT ) );
            Size aTextOnlySize( impl_calculateItemContentSize( pPanel, TABITEM_IMAGE_AND_TEXT ) );

            // TODO: have one method calculating all sizes?

            // care for the decorations
            const Rectangle aBoundingRect( m_pRenderer->calculateDecorations( aCompleteSize ) );
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

            m_aItems.push_back( aItem );

            aCompletePos = aItem.aCompleteArea.TopRight();
            aIconOnlyPos = aItem.aIconOnlyArea.TopRight();
            aTextOnlyPos = aItem.aTextOnlyArea.TopRight();
        }

        m_bItemsDirty = false;
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
            const Size aTextSize( m_rTabBar.GetCtrlTextWidth( sItemText ), m_rTabBar.GetTextHeight() );
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
            m_rTabBar.DrawImage( aImagePos, aItemImage );
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
            const Size aTextSize( m_rTabBar.GetCtrlTextWidth( sItemText ), m_rTabBar.GetTextHeight() );
            Point aTextPos( aRenderArea.TopLeft() );
            if ( IsVertical() )
            {
                m_rTabBar.Push( PUSH_FONT );

                Font aFont( m_rTabBar.GetFont() );
                aFont.SetOrientation( 2700 );
                aFont.SetVertical( TRUE );
                m_rTabBar.SetFont( aFont );

                aTextPos.X() += aTextSize.Height();
                aTextPos.X() += ( aRenderArea.GetWidth() - aTextSize.Height() ) / 2;
            }
            else
            {
                aTextPos.Y() += ( aRenderArea.GetHeight() - aTextSize.Height() ) / 2;
            }

            m_rTabBar.DrawText( aTextPos, sItemText );

            if ( IsVertical() )
            {
                m_rTabBar.Pop();
            }
        }
    }

    //------------------------------------------------------------------------------------------------------------------
    void PanelTabBar_Impl::DrawItem( const size_t i_nItemIndex ) const
    {
        const ItemDescriptor& rItem( m_aItems[ i_nItemIndex ] );

        ItemFlags nItemFlags( ITEM_STATE_NORMAL );
        if ( m_aHoveredItem == i_nItemIndex )
        {
            nItemFlags |= ITEM_STATE_HOVERED;
            if ( m_bMouseButtonDown )
                nItemFlags |= ITEM_STATE_ACTIVE;
        }

        if ( m_rPanelDeck.GetActivePanel() == i_nItemIndex )
            nItemFlags |= ITEM_STATE_ACTIVE;

        if ( m_aFocusedItem == i_nItemIndex )
            nItemFlags |= ITEM_STATE_FOCUSED;

        if ( 0 == i_nItemIndex )
            nItemFlags |= ITEM_POSITION_FIRST;

        if ( m_rPanelDeck.GetPanelCount() - 1 == i_nItemIndex )
            nItemFlags |= ITEM_POSITION_LAST;

        m_rTabBar.SetUpdateMode( FALSE );

        // some item geometry
        // - the normalized bounding and content rect
        const Rectangle aNormalizedBounds( rItem.GetCurrentRect() );
        Rectangle aNormalizedContent( aNormalizedBounds );
        lcl_deflateRect( aNormalizedContent, rItem.aContentInset );

        // - the aligned bounding and content rect
        const Rectangle aActualBounds = m_aNormalizer.getTransformed( aNormalizedBounds, m_eTabAlignment );
        const Rectangle aActualContent = m_aNormalizer.getTransformed( aNormalizedContent, m_eTabAlignment );

        // completely erase the bounding region, renderers are not expected to do this
        const_cast< VirtualDevice& >( m_aRenderDevice ).DrawRect( aNormalizedBounds );

        // render item "background" layer
        m_pRenderer->preRenderItem( aNormalizedContent, nItemFlags );

        // copy from the virtual device to ourself
        BitmapEx aBitmap( m_aRenderDevice.GetBitmapEx(
            aNormalizedBounds.TopLeft(),
            Size(
                aNormalizedBounds.GetSize().Width() - 1,
                aNormalizedBounds.GetSize().Height() - 1
            )
        ) );
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
        m_rTabBar.DrawBitmapEx( aActualBounds.TopLeft(), aBitmap );

        // render the actual item content
        impl_renderItemContent( rItem.pPanel, aActualContent, m_aGeometry.getItemContent() );

        // render item "foreground" layer
        m_pRenderer->postRenderItem( m_rTabBar, aActualBounds, nItemFlags );

        m_rTabBar.SetUpdateMode( TRUE );
    }

    //------------------------------------------------------------------------------------------------------------------
    void PanelTabBar_Impl::EnsureItemsCache()
    {
        if ( m_bItemsDirty == false )
        {
            DBG_CHECK( *this );
            return;
        }
        impl_calcItemRects();
        OSL_POSTCOND( m_bItemsDirty == false, "EnsureItemsCache: cache still dirty!" );
        DBG_CHECK( *this );
    }

    //------------------------------------------------------------------------------------------------------------------
    void PanelTabBar_Impl::Relayout()
    {
        EnsureItemsCache();

        const Size aOutputSize( m_rTabBar.GetOutputSizePixel() );
        m_aNormalizer = NormalizedArea( Rectangle( Point(), aOutputSize ), IsVertical() );
        const Size aLogicalOutputSize( m_aNormalizer.getReferenceSize() );

        // forward actual output size to our render device
        m_aRenderDevice.SetOutputSizePixel( aLogicalOutputSize );

        // re-calculate the size of the scroll buttons and of the items
        m_aGeometry.relayout( aLogicalOutputSize, m_aItems );

        if ( m_aGeometry.getButtonBackRect().IsEmpty() )
        {
            m_aScrollBack.Hide();
        }
        else
        {
            const Rectangle aButtonBack( m_aNormalizer.getTransformed( m_aGeometry.getButtonBackRect(), m_eTabAlignment ) );
            m_aScrollBack.SetPosSizePixel( aButtonBack.TopLeft(), aButtonBack.GetSize() );
            m_aScrollBack.Show();
        }

        if ( m_aGeometry.getButtonForwardRect().IsEmpty() )
        {
            m_aScrollForward.Hide();
        }
        else
        {
            const Rectangle aButtonForward( m_aNormalizer.getTransformed( m_aGeometry.getButtonForwardRect(), m_eTabAlignment ) );
            m_aScrollForward.SetPosSizePixel( aButtonForward.TopLeft(), aButtonForward.GetSize() );
            m_aScrollForward.Show();
        }

        UpdateScrollButtons();
    }

    //------------------------------------------------------------------------------------------------------------------
    ::boost::optional< size_t > PanelTabBar_Impl::FindItemForPoint( const Point& i_rPoint ) const
    {
        Point aPoint( IsVertical() ? i_rPoint.Y() : i_rPoint.X(), IsVertical() ? i_rPoint.X() : i_rPoint.Y() );

        if ( !m_aGeometry.getItemsRect().IsInside( aPoint ) )
            return ::boost::optional< size_t >();

        size_t i=0;
        for (   ItemDescriptors::const_iterator item = m_aItems.begin();
                item != m_aItems.end();
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
            m_rTabBar.Invalidate();
        }
        else if ( i_pButton == &m_aScrollForward )
        {
            OSL_ENSURE( m_nScrollPosition < m_aItems.size() - 1, "PanelTabBar_Impl::OnScroll: inconsistency!" );
            ++m_nScrollPosition;
            m_rTabBar.Invalidate();
        }

        UpdateScrollButtons();

        return 0L;
    }

    //------------------------------------------------------------------------------------------------------------------
    Rectangle PanelTabBar_Impl::GetActualLogicalItemRect( const Rectangle& i_rLogicalItemRect ) const
    {
        // care for the offset imposed by our geometry, i.e. whether or not we have scroll buttons
        Rectangle aItemRect( i_rLogicalItemRect );
        aItemRect.Move( m_aGeometry.getItemsRect().Left() - m_aGeometry.getButtonBackRect().Left(), 0 );

        // care for the current scroll position
        OSL_ENSURE( m_nScrollPosition < m_aItems.size(), "GetActualLogicalItemRect: invalid scroll position!" );
        if ( ( m_nScrollPosition > 0 ) && ( m_nScrollPosition < m_aItems.size() ) )
        {
            long nOffsetX = m_aItems[ m_nScrollPosition ].GetCurrentRect().Left() - m_aItems[ 0 ].GetCurrentRect().Left();
            long nOffsetY = m_aItems[ m_nScrollPosition ].GetCurrentRect().Top() - m_aItems[ 0 ].GetCurrentRect().Top();
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
        return m_pImpl->m_aGeometry.getItemContent();
    }

    //------------------------------------------------------------------------------------------------------------------
    void PanelTabBar::SetTabItemContent( const TabItemContent& i_eItemContent )
    {
        m_pImpl->m_aGeometry.setItemContent( i_eItemContent );
        m_pImpl->Relayout();
        Invalidate();
    }

    //------------------------------------------------------------------------------------------------------------------
    Size PanelTabBar::GetOptimalSize( WindowSizeType i_eType ) const
    {
        m_pImpl->EnsureItemsCache();
        Size aOptimalSize( m_pImpl->m_aGeometry.getOptimalSize( m_pImpl->m_aItems, i_eType == WINDOWSIZE_MINIMUM ) );
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
        m_pImpl->m_aRenderDevice.Push( PUSH_CLIPREGION );
        m_pImpl->m_aRenderDevice.SetClipRegion( i_rRect );
        m_pImpl->m_pRenderer->renderBackground();
        m_pImpl->m_aRenderDevice.Pop();

        // ensure the items really paint into their own playground only
        ClipItemRegion aClipItems( *m_pImpl );

        const Rectangle aLogicalPaintRect( m_pImpl->m_aNormalizer.getNormalized( i_rRect, m_pImpl->m_eTabAlignment ) );

        // items
        size_t i=0;
        for (   ItemDescriptors::const_iterator item = m_pImpl->m_aItems.begin();
                item != m_pImpl->m_aItems.end();
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

        ::boost::optional< size_t > aOldItem( m_pImpl->m_aHoveredItem );
        ::boost::optional< size_t > aNewItem( m_pImpl->FindItemForPoint( i_rMouseEvent.GetPosPixel() ) );

        if  ( i_rMouseEvent.IsLeaveWindow() )
            aNewItem.reset();

        if ( aOldItem != aNewItem )
        {
            m_pImpl->m_aHoveredItem = aNewItem;

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
        m_pImpl->m_bMouseButtonDown = true;

        ClipItemRegion aClipItems( *m_pImpl );
        m_pImpl->DrawItem( *aHitItem );
    }

    //------------------------------------------------------------------------------------------------------------------
    void PanelTabBar::MouseButtonUp( const MouseEvent& i_rMouseEvent )
    {
        Control::MouseButtonUp( i_rMouseEvent );

        if ( m_pImpl->m_bMouseButtonDown )
        {
            OSL_ENSURE( IsMouseCaptured(), "PanelTabBar::MouseButtonUp: inconsistency!" );
            if ( IsMouseCaptured() )
                ReleaseMouse();
            m_pImpl->m_bMouseButtonDown = false;

            ::boost::optional< size_t > aHitItem( m_pImpl->FindItemForPoint( i_rMouseEvent.GetPosPixel() ) );
            if ( !!aHitItem )
            {
                // re-draw that item now that we're not in mouse-down mode anymore
                ClipItemRegion aClipItems( *m_pImpl );
                m_pImpl->DrawItem( *aHitItem );
                // activate the respective panel
                m_pImpl->m_rPanelDeck.ActivatePanel( *aHitItem );
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

        const ItemDescriptor& rItem( m_pImpl->m_aItems[ *aHelpItem ] );
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
        if ( m_pImpl->m_rPanelDeck.GetPanelCount() )
        {
            ::boost::optional< size_t > aActivePanel( m_pImpl->m_rPanelDeck.GetActivePanel() );
            if ( !!aActivePanel )
            {
                m_pImpl->m_aFocusedItem = aActivePanel;
                ClipItemRegion aClipItems( *m_pImpl );
                m_pImpl->DrawItem( *m_pImpl->m_aFocusedItem );
            }
        }
    }

    //------------------------------------------------------------------------------------------------------------------
    void PanelTabBar::LoseFocus()
    {
        Control::LoseFocus();

        ::boost::optional< size_t > aPreviouslyFocused( m_pImpl->m_aFocusedItem );
        m_pImpl->m_aFocusedItem.reset();

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
        const size_t nPanelCount( m_pImpl->m_rPanelDeck.GetPanelCount() );
        if ( nPanelCount < 2 )
            return;

        OSL_PRECOND( !!m_pImpl->m_aFocusedItem, "PanelTabBar::KeyInput: we should have a focused item here!" );
            // if we get KeyInput events, we should have the focus. In this case, m_aFocusedItem should not be empty,
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
            m_pImpl->m_rPanelDeck.ActivatePanel( *m_pImpl->m_aFocusedItem );
            break;
        }

        if ( !bFocusNext && !bFocusPrev )
            return;

        const size_t nOldFocus = *m_pImpl->m_aFocusedItem;
        if ( bFocusNext )
        {
            m_pImpl->m_aFocusedItem.reset( ( *m_pImpl->m_aFocusedItem + 1 ) % nPanelCount );
        }
        else
        {
            m_pImpl->m_aFocusedItem.reset( ( *m_pImpl->m_aFocusedItem + nPanelCount - 1 ) % nPanelCount );
        }

        ClipItemRegion aClipItems( *m_pImpl );
        m_pImpl->DrawItem( nOldFocus );
        m_pImpl->DrawItem( *m_pImpl->m_aFocusedItem );
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
