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


#include <svtools/toolpanel/paneltabbar.hxx>
#include <svtools/toolpanel/toolpaneldeck.hxx>
#include <svtools/svtresid.hxx>
#include <svtools/svtools.hrc>

#include "tabitemdescriptor.hxx"
#include "paneltabbarpeer.hxx"
#include "tabbargeometry.hxx"

#include <vcl/button.hxx>
#include <vcl/help.hxx>
#include <vcl/virdev.hxx>
#include <vcl/settings.hxx>
#include <tools/diagnose_ex.h>

#include <memory>
#include <vector>

// space around an item
#define ITEM_OUTER_SPACE        2 * 3
// spacing before and after an item's text
#define ITEM_TEXT_FLOW_SPACE    5
// space between item icon and icon text
#define ITEM_ICON_TEXT_DISTANCE 4


namespace svt
{


    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::awt::XWindowPeer;

    typedef sal_uInt16  ItemFlags;

    #define ITEM_STATE_NORMAL   0x00
    #define ITEM_STATE_ACTIVE   0x01
    #define ITEM_STATE_HOVERED  0x02
    #define ITEM_STATE_FOCUSED  0x04
    #define ITEM_POSITION_FIRST 0x08
    #define ITEM_POSITION_LAST  0x10


    //= helper

    namespace
    {
        ControlState lcl_ItemToControlState( const ItemFlags i_nItemFlags )
        {
            ControlState nState = ControlState::ENABLED;
            if ( i_nItemFlags & ITEM_STATE_FOCUSED )    nState |= ControlState::FOCUSED | ControlState::PRESSED;
            if ( i_nItemFlags & ITEM_STATE_HOVERED )    nState |= ControlState::ROLLOVER;
            if ( i_nItemFlags & ITEM_STATE_ACTIVE )     nState |= ControlState::SELECTED;
            return nState;
        }
    }


    //= ITabBarRenderer

    class SAL_NO_VTABLE ITabBarRenderer
    {
    public:
        /** fills the background of our target device
        */
        virtual void        renderBackground() const = 0;
        virtual Rectangle   calculateDecorations( const Rectangle& i_rContentArea, const ItemFlags i_nItemFlags ) const = 0;
        virtual void        preRenderItem( const Rectangle& i_rContentRect, const ItemFlags i_nItemFlags ) const = 0;
        virtual void        postRenderItem( vcl::Window& i_rActualWindow, const Rectangle& i_rItemRect, const ItemFlags i_nItemFlags ) const = 0;

        // TODO: postRenderItem takes the "real" window, i.e. effectively the tab bar. This is because
        // DrawSelectionBackground needs to be applied after everything else is painted, and is available at the Window
        // class, but not at the OutputDevice. This makes the API somewhat weird, as we're now mixing operations on the
        // target device, done in a normalized geometry, with operations on the window, done in a transformed geometry.
        // So, we should get rid of postRenderItem completely.

    protected:
        ~ITabBarRenderer() {}
    };
    typedef std::shared_ptr< ITabBarRenderer >  PTabBarRenderer;


    //= VCLItemRenderer - declaration

    class VCLItemRenderer : public ITabBarRenderer
    {
    public:
        explicit VCLItemRenderer( OutputDevice& i_rTargetDevice )
            :m_rTargetDevice( i_rTargetDevice )
        {
        }
        virtual ~VCLItemRenderer() {}

        // ITabBarRenderer
        virtual void        renderBackground() const override;
        virtual Rectangle   calculateDecorations( const Rectangle& i_rContentArea, const ItemFlags i_nItemFlags ) const override;
        virtual void        preRenderItem( const Rectangle& i_rContentRect, const ItemFlags i_nItemFlags ) const override;
        virtual void        postRenderItem( vcl::Window& i_rActualWindow, const Rectangle& i_rItemRect, const ItemFlags i_nItemFlags ) const override;

    protected:
        OutputDevice&   getTargetDevice() const { return m_rTargetDevice; }

    private:
        OutputDevice&   m_rTargetDevice;
    };


    //= VCLItemRenderer - implementation


    void VCLItemRenderer::renderBackground() const
    {
        getTargetDevice().DrawRect( Rectangle( Point(), getTargetDevice().GetOutputSizePixel() ) );
    }


    Rectangle VCLItemRenderer::calculateDecorations( const Rectangle& i_rContentArea, const ItemFlags i_nItemFlags ) const
    {
        (void)i_nItemFlags;
        // no decorations at all
        return i_rContentArea;
    }


    void VCLItemRenderer::preRenderItem( const Rectangle& i_rContentRect, const ItemFlags i_nItemFlags ) const
    {
        (void)i_rContentRect;
        (void)i_nItemFlags;
    }


    void VCLItemRenderer::postRenderItem( vcl::Window& i_rActualWindow, const Rectangle& i_rItemRect, const ItemFlags i_nItemFlags ) const
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
                ( bHovered || bFocused ) ? ( bActive ? 1 : 2 ) : 0 /* highlight */,
                bActive /* check */,
                true /* border */,
                false /* ext border only */,
                0 /* corner radius */,
                NULL,
                NULL
            );
        }
    }


    //= NWFToolboxItemRenderer - declaration

    class NWFToolboxItemRenderer : public ITabBarRenderer
    {
    public:
        explicit NWFToolboxItemRenderer( OutputDevice& i_rTargetDevice )
            :m_rTargetDevice( i_rTargetDevice )
        {
        }
        virtual ~NWFToolboxItemRenderer() {}

        // ITabBarRenderer
        virtual void        renderBackground() const override;
        virtual Rectangle   calculateDecorations( const Rectangle& i_rContentArea, const ItemFlags i_nItemFlags ) const override;
        virtual void        preRenderItem( const Rectangle& i_rContentRect, const ItemFlags i_nItemFlags ) const override;
        virtual void        postRenderItem( vcl::Window& i_rActualWindow, const Rectangle& i_rItemRect, const ItemFlags i_nItemFlags ) const override;

    protected:
        OutputDevice&   getTargetDevice() const { return m_rTargetDevice; }

    private:
        OutputDevice&   m_rTargetDevice;
    };


    //= NWFToolboxItemRenderer - implementation


    void NWFToolboxItemRenderer::renderBackground() const
    {
        getTargetDevice().DrawRect( Rectangle( Point(), getTargetDevice().GetOutputSizePixel() ) );
    }


    Rectangle NWFToolboxItemRenderer::calculateDecorations( const Rectangle& i_rContentArea, const ItemFlags i_nItemFlags ) const
    {
        // don't ask GetNativeControlRegion, this will not deliver proper results in all cases.
        // Instead, simply assume that both the content and the bounding region are the same.
//        const ControlState nState( lcl_ItemToControlState( i_nItemFlags );
//        const ImplControlValue aControlValue;
//        bool bNativeOK = m_rTargetWindow.GetNativeControlRegion(
//            CTRL_TOOLBAR, PART_BUTTON,
//            i_rContentArea, nState,
//            aControlValue, OUString(),
//            aBoundingRegion, aContentRegion
//        );
        (void)i_nItemFlags;
        return Rectangle(
            Point( i_rContentArea.Left() - 1, i_rContentArea.Top() - 1 ),
            Size( i_rContentArea.GetWidth() + 2, i_rContentArea.GetHeight() + 2 )
        );
    }


    void NWFToolboxItemRenderer::preRenderItem( const Rectangle& i_rContentRect, const ItemFlags i_nItemFlags ) const
    {
        const ControlState nState = lcl_ItemToControlState( i_nItemFlags );

        ImplControlValue aControlValue;
        aControlValue.setTristateVal( ( i_nItemFlags & ITEM_STATE_ACTIVE ) ? BUTTONVALUE_ON : BUTTONVALUE_OFF );

        bool bNativeOK = getTargetDevice().DrawNativeControl( CTRL_TOOLBAR, PART_BUTTON, i_rContentRect, nState, aControlValue, OUString() );
        (void)bNativeOK;
        OSL_ENSURE( bNativeOK, "NWFToolboxItemRenderer::preRenderItem: inconsistent NWF implementation!" );
            // IsNativeControlSupported returned true, previously, otherwise we would not be here ...
    }


    void NWFToolboxItemRenderer::postRenderItem( vcl::Window& i_rActualWindow, const Rectangle& i_rItemRect, const ItemFlags i_nItemFlags ) const
    {
        (void)i_rActualWindow;
        (void)i_rItemRect;
        (void)i_nItemFlags;
    }


#if defined WNT
    //= NWFTabItemRenderer - declaration

    class NWFTabItemRenderer : public ITabBarRenderer
    {
    public:
        explicit NWFTabItemRenderer( OutputDevice& i_rTargetDevice )
            :m_rTargetDevice( i_rTargetDevice )
        {
        }

        virtual ~NWFTabItemRenderer() {}

        // ITabBarRenderer
        virtual void        renderBackground() const override;
        virtual Rectangle   calculateDecorations( const Rectangle& i_rContentArea, const ItemFlags i_nItemFlags ) const override;
        virtual void        preRenderItem( const Rectangle& i_rContentRect, const ItemFlags i_nItemFlags ) const override;
        virtual void        postRenderItem( vcl::Window& i_rActualWindow, const Rectangle& i_rItemRect, const ItemFlags i_nItemFlags ) const override;

    protected:
        OutputDevice&   getTargetDevice() const { return m_rTargetDevice; }

    private:
        OutputDevice&   m_rTargetDevice;
    };


    //= NWFTabItemRenderer - implementation


    void NWFTabItemRenderer::renderBackground() const
    {
        Rectangle aBackground( Point(), getTargetDevice().GetOutputSizePixel() );
        getTargetDevice().DrawRect( aBackground );

        aBackground.Top() = aBackground.Bottom();
        getTargetDevice().DrawNativeControl( CTRL_TAB_PANE, PART_ENTIRE_CONTROL, aBackground,
            ControlState::ENABLED, ImplControlValue(), OUString() );
    }


    Rectangle NWFTabItemRenderer::calculateDecorations( const Rectangle& i_rContentArea, const ItemFlags i_nItemFlags ) const
    {
        const ControlState nState( lcl_ItemToControlState( i_nItemFlags ) );

        TabitemValue tiValue(Rectangle(i_rContentArea.Left() + TAB_TABOFFSET_X,
                                       i_rContentArea.Right() - TAB_TABOFFSET_X,
                                       i_rContentArea.Top() + TAB_TABOFFSET_Y,
                                       i_rContentArea.Bottom() - TAB_TABOFFSET_Y));

        Rectangle aBoundingRegion, aContentRegion;
        bool bNativeOK = getTargetDevice().GetNativeControlRegion(
            CTRL_TAB_ITEM, PART_ENTIRE_CONTROL,
            i_rContentArea, nState,
            tiValue, OUString(),
            aBoundingRegion, aContentRegion
        );
        (void)bNativeOK;
        OSL_ENSURE( bNativeOK, "NWFTabItemRenderer::calculateDecorations: GetNativeControlRegion not implemented for CTRL_TAB_ITEM?!" );

        return aBoundingRegion;
    }


    void NWFTabItemRenderer::preRenderItem( const Rectangle& i_rContentRect, const ItemFlags i_nItemFlags ) const
    {
        const ControlState nState = lcl_ItemToControlState( i_nItemFlags );

        TabitemValue tiValue(Rectangle(i_rContentRect.Left() + TAB_TABOFFSET_X,
                                       i_rContentRect.Right() - TAB_TABOFFSET_X,
                                       i_rContentRect.Top() + TAB_TABOFFSET_Y,
                                       i_rContentRect.Bottom() - TAB_TABOFFSET_Y));

        if ( i_nItemFlags & ITEM_POSITION_FIRST )
            tiValue.mnAlignment |= TabitemFlags::FirstInGroup;
        if ( i_nItemFlags & ITEM_POSITION_LAST )
            tiValue.mnAlignment |= TabitemFlags::LastInGroup;


        bool bNativeOK = getTargetDevice().DrawNativeControl( CTRL_TAB_ITEM, PART_ENTIRE_CONTROL, i_rContentRect, nState, tiValue, OUString() );
        (void)bNativeOK;
        OSL_ENSURE( bNativeOK, "NWFTabItemRenderer::preRenderItem: inconsistent NWF implementation!" );
            // IsNativeControlSupported returned true, previously, otherwise we would not be here ...
    }


    void NWFTabItemRenderer::postRenderItem( vcl::Window& i_rActualWindow, const Rectangle& i_rItemRect, const ItemFlags i_nItemFlags ) const
    {
        (void)i_rActualWindow;
        (void)i_rItemRect;
        (void)i_nItemFlags;
    }
#endif

    //= PanelTabBar_Impl

    class PanelTabBar_Impl : public IToolPanelDeckListener
    {
    public:
        PanelTabBar_Impl(PanelTabBar& i_rTabBar, IToolPanelDeck& i_rPanelDeck,
                         const TabAlignment i_eAlignment, const TabItemContent i_eItemContent);

        virtual ~PanelTabBar_Impl()
        {
            m_rPanelDeck.RemoveListener(*this);
        }

        // IToolPanelDeckListener
        virtual void PanelInserted(const PToolPanel& i_pPanel, const size_t i_nPosition) override
        {
            (void) i_pPanel;
            (void) i_nPosition;
            m_bItemsDirty = true;
            m_rTabBar.Invalidate();

            Relayout();
        }

        virtual void PanelRemoved( const size_t i_nPosition ) override
        {
            m_bItemsDirty = true;
            m_rTabBar.Invalidate();

            if ( i_nPosition < m_nScrollPosition )
                --m_nScrollPosition;

            Relayout();
        }

        virtual void ActivePanelChanged(const boost::optional<size_t>& i_rOldActive,
                                        const boost::optional<size_t>& i_rNewActive) override;
        virtual void LayouterChanged(const PDeckLayouter& i_rNewLayouter) override;
        virtual void Dying() override;

        void UpdateScrollButtons()
        {
            m_aScrollBack->Enable(m_nScrollPosition > 0);
            m_aScrollForward->Enable(m_nScrollPosition < m_aItems.size() - 1);
        }

        void                        Relayout();
        void                        EnsureItemsCache();
        boost::optional<size_t>     FindItemForPoint( const Point& i_rPoint ) const;
        void                        DrawItem(vcl::RenderContext& rRenderContext, const size_t i_nItemIndex, const Rectangle& i_rBoundaries) const;
        void                        InvalidateItem( const size_t i_nItemIndex, const ItemFlags i_nAdditionalItemFlags = 0 ) const;
        void                        CopyFromRenderDevice(vcl::RenderContext& rRenderContext, const Rectangle& i_rLogicalRect) const;
        Rectangle                   GetActualLogicalItemRect( const Rectangle& i_rLogicalItemRect ) const;
        Rectangle                   GetItemScreenRect( const size_t i_nItemPos ) const;

        void                        FocusItem( const ::boost::optional< size_t >& i_rItemPos );

        inline bool                 IsVertical() const
        {
            return  (   ( m_eTabAlignment == TABS_LEFT )
                    ||  ( m_eTabAlignment == TABS_RIGHT )
                    );
        }

    protected:
        DECL_LINK_TYPED( OnScroll, Button*, void );

        void        impl_calcItemRects();
        Size        impl_calculateItemContentSize( const PToolPanel& i_pPanel, const TabItemContent i_eItemContent ) const;
        void        impl_renderItemContent(vcl::RenderContext& rRenderContext, const PToolPanel& i_pPanel,
                                           const Rectangle& i_rContentArea, const TabItemContent i_eItemContent) const;
        ItemFlags   impl_getItemFlags( const size_t i_nItemIndex ) const;

    public:
        PanelTabBar&                m_rTabBar;
        TabBarGeometry              m_aGeometry;
        NormalizedArea              m_aNormalizer;
        TabAlignment                m_eTabAlignment;
        IToolPanelDeck&             m_rPanelDeck;

        ScopedVclPtr<VirtualDevice> m_aRenderDevice;
        PTabBarRenderer             m_pRenderer;

        boost::optional<size_t>     m_aHoveredItem;
        boost::optional<size_t>     m_aFocusedItem;
        bool                        m_bMouseButtonDown;

        ItemDescriptors             m_aItems;
        bool                        m_bItemsDirty;

        VclPtr<PushButton>          m_aScrollBack;
        VclPtr<PushButton>          m_aScrollForward;

        size_t                      m_nScrollPosition;
    };


    //= helper

    namespace
    {

    #if OSL_DEBUG_LEVEL > 0
        static void lcl_checkConsistency( const PanelTabBar_Impl& i_rImpl )
        {
            if ( !i_rImpl.m_bItemsDirty )
            {
                if ( i_rImpl.m_rPanelDeck.GetPanelCount() != i_rImpl.m_aItems.size() )
                {
                    OSL_FAIL( "lcl_checkConsistency: inconsistent array sizes!" );
                    return;
                }
                for ( size_t i = 0; i < i_rImpl.m_rPanelDeck.GetPanelCount(); ++i )
                {
                    if ( i_rImpl.m_rPanelDeck.GetPanel( i ).get() != i_rImpl.m_aItems[i].pPanel.get() )
                    {
                        OSL_FAIL( "lcl_checkConsistency: array elements are inconsistent!" );
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


        class ClipItemRegion
        {
        public:
            explicit ClipItemRegion( const PanelTabBar_Impl& i_rImpl )
                :m_rDevice( i_rImpl.m_rTabBar )
            {
                m_rDevice.Push( PushFlags::CLIPREGION );
                m_rDevice.SetClipRegion(vcl::Region(
                    i_rImpl.m_aNormalizer.getTransformed(
                        i_rImpl.m_aGeometry.getItemsRect(),
                        i_rImpl.m_eTabAlignment )));
            }

            ~ClipItemRegion()
            {
                m_rDevice.Pop();
            }

        private:
            OutputDevice&   m_rDevice;
        };
    }


    //= PanelTabBar_Impl - implementation


    PanelTabBar_Impl::PanelTabBar_Impl( PanelTabBar& i_rTabBar, IToolPanelDeck& i_rPanelDeck, const TabAlignment i_eAlignment, const TabItemContent i_eItemContent )
        :m_rTabBar( i_rTabBar )
        ,m_aGeometry( i_eItemContent )
        ,m_aNormalizer()
        ,m_eTabAlignment( i_eAlignment )
        ,m_rPanelDeck( i_rPanelDeck )
        ,m_aRenderDevice( VclPtr<VirtualDevice>::Create(i_rTabBar) )
        ,m_pRenderer()
        ,m_aHoveredItem()
        ,m_aFocusedItem()
        ,m_bMouseButtonDown( false )
        ,m_aItems()
        ,m_bItemsDirty( true )
        ,m_aScrollBack( VclPtr<PushButton>::Create(&i_rTabBar, WB_BEVELBUTTON) )
        ,m_aScrollForward( VclPtr<PushButton>::Create(&i_rTabBar, WB_BEVELBUTTON) )
        ,m_nScrollPosition( 0 )
    {
#ifdef WNT
        if (m_aRenderDevice->IsNativeControlSupported(CTRL_TAB_ITEM, PART_ENTIRE_CONTROL))
            // this mode requires the NWF framework to be able to render those items onto a virtual
            // device. For some frameworks (some GTK themes, in particular), this is known to fail.
            // So, be on the safe side for the moment.
            m_pRenderer.reset(new NWFTabItemRenderer(*m_aRenderDevice.get()));
        else
#endif
        if (m_aRenderDevice->IsNativeControlSupported(CTRL_TOOLBAR, PART_BUTTON))
            m_pRenderer.reset(new NWFToolboxItemRenderer(*m_aRenderDevice.get()));
        else
            m_pRenderer.reset(new VCLItemRenderer(*m_aRenderDevice.get()));

        m_aRenderDevice->SetLineColor();

        m_rPanelDeck.AddListener( *this );

        m_aScrollBack->SetSymbol( IsVertical() ? SymbolType::ARROW_UP : SymbolType::ARROW_LEFT );
        m_aScrollBack->Show();
        m_aScrollBack->SetClickHdl( LINK( this, PanelTabBar_Impl, OnScroll ) );
        m_aScrollBack->SetAccessibleDescription( SvtResId( STR_SVT_TOOL_PANEL_BUTTON_FWD ).toString() );
        m_aScrollBack->SetAccessibleName( m_aScrollBack->GetAccessibleDescription() );

        m_aScrollForward->SetSymbol( IsVertical() ? SymbolType::ARROW_DOWN : SymbolType::ARROW_RIGHT );
        m_aScrollForward->Show();
        m_aScrollForward->SetClickHdl( LINK( this, PanelTabBar_Impl, OnScroll ) );
        m_aScrollForward->SetAccessibleDescription( SvtResId( STR_SVT_TOOL_PANEL_BUTTON_BACK ).toString() );
        m_aScrollForward->SetAccessibleName( m_aScrollForward->GetAccessibleDescription() );
    }


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

            const Size aCompleteSize( impl_calculateItemContentSize( pPanel, TABITEM_IMAGE_AND_TEXT ) );
            const Size aIconOnlySize( impl_calculateItemContentSize( pPanel, TABITEM_IMAGE_ONLY ) );
            const Size aTextOnlySize( impl_calculateItemContentSize( pPanel, TABITEM_TEXT_ONLY ) );

            // TODO: have one method calculating all sizes?

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


    Size PanelTabBar_Impl::impl_calculateItemContentSize( const PToolPanel& i_pPanel, const TabItemContent i_eItemContent ) const
    {
        // calculate the size needed for the content
        OSL_ENSURE( i_eItemContent != TABITEM_AUTO, "PanelTabBar_Impl::impl_calculateItemContentSize: illegal TabItemContent value!" );

        const Image aImage( i_pPanel->GetImage() );
        const bool bUseImage = !!aImage && ( i_eItemContent != TABITEM_TEXT_ONLY );

        const OUString sItemText( i_pPanel->GetDisplayName() );
        const bool bUseText = ( !sItemText.isEmpty() ) && ( i_eItemContent != TABITEM_IMAGE_ONLY );

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

        if ( !bUseImage && !bUseText )
        {
            // have a minimal size - this is pure heuristics, but if it doesn't suit your needs, then give your panels
            // a name and or image! :)
            aItemContentSize = Size( 16, 16 );
        }

        aItemContentSize.Width() += 2 * ITEM_OUTER_SPACE;
        aItemContentSize.Height() += 2 * ITEM_OUTER_SPACE;

        return aItemContentSize;
    }


    void PanelTabBar_Impl::impl_renderItemContent(vcl::RenderContext& rRenderContext, const PToolPanel& i_pPanel, const Rectangle& i_rContentArea, const TabItemContent i_eItemContent) const
    {
        OSL_ENSURE(i_eItemContent != TABITEM_AUTO, "PanelTabBar_Impl::impl_renderItemContent: illegal TabItemContent value!");

        Rectangle aRenderArea(i_rContentArea);
        if (IsVertical())
        {
            aRenderArea.Top() += ITEM_OUTER_SPACE;
        }
        else
        {
            aRenderArea.Left() += ITEM_OUTER_SPACE;
        }

        // draw the image
        const Image aItemImage(i_pPanel->GetImage());
        const Size aImageSize(aItemImage.GetSizePixel());
        const bool bUseImage = !!aItemImage && (i_eItemContent != TABITEM_TEXT_ONLY);

        if (bUseImage)
        {
            Point aImagePos;
            if (IsVertical())
            {
                aImagePos.X() = aRenderArea.Left() + (aRenderArea.GetWidth() - aImageSize.Width()) / 2;
                aImagePos.Y() = aRenderArea.Top();
            }
            else
            {
                aImagePos.X() = aRenderArea.Left();
                aImagePos.Y() = aRenderArea.Top() + (aRenderArea.GetHeight() - aImageSize.Height()) / 2;
            }
            rRenderContext.DrawImage(aImagePos, aItemImage);
        }

        const OUString sItemText(i_pPanel->GetDisplayName());
        const bool bUseText = (!sItemText.isEmpty()) && (i_eItemContent != TABITEM_IMAGE_ONLY);

        if (bUseText)
        {
            if (IsVertical())
            {
                if (bUseImage)
                    aRenderArea.Top() += aImageSize.Height() + ITEM_ICON_TEXT_DISTANCE;
                aRenderArea.Top() += ITEM_TEXT_FLOW_SPACE;
            }
            else
            {
                if (bUseImage)
                    aRenderArea.Left() += aImageSize.Width() + ITEM_ICON_TEXT_DISTANCE;
                aRenderArea.Left() += ITEM_TEXT_FLOW_SPACE;
            }

            // draw the text
            const Size aTextSize(m_rTabBar.GetCtrlTextWidth(sItemText), rRenderContext.GetTextHeight());
            Point aTextPos(aRenderArea.TopLeft());
            if (IsVertical())
            {
                rRenderContext.Push(PushFlags::FONT);

                vcl::Font aFont(rRenderContext.GetFont());
                aFont.SetOrientation(2700);
                aFont.SetVertical(true);
                rRenderContext.SetFont(aFont);

                aTextPos.X() += aTextSize.Height();
                aTextPos.X() += (aRenderArea.GetWidth() - aTextSize.Height()) / 2;
            }
            else
            {
                aTextPos.Y() += (aRenderArea.GetHeight() - aTextSize.Height()) / 2;
            }

            rRenderContext.DrawText(aTextPos, sItemText);

            if (IsVertical())
            {
                rRenderContext.Pop();
            }
        }
    }

    void PanelTabBar_Impl::CopyFromRenderDevice(vcl::RenderContext& rRenderContext, const Rectangle& i_rLogicalRect) const
    {
        BitmapEx aBitmap(m_aRenderDevice->GetBitmapEx(i_rLogicalRect.TopLeft(),
                                                      Size(i_rLogicalRect.GetSize().Width(),
                                                           i_rLogicalRect.GetSize().Height())));
        if (IsVertical())
        {
            aBitmap.Rotate(2700, COL_BLACK);
            if (m_eTabAlignment == TABS_LEFT)
                aBitmap.Mirror(BmpMirrorFlags::Horizontal);
        }
        else if (m_eTabAlignment == TABS_BOTTOM)
        {
            aBitmap.Mirror(BmpMirrorFlags::Vertical);
        }

        const Rectangle aActualRect(m_aNormalizer.getTransformed(i_rLogicalRect, m_eTabAlignment));
        rRenderContext.DrawBitmapEx(aActualRect.TopLeft(), aBitmap);
    }


    void PanelTabBar_Impl::InvalidateItem( const size_t i_nItemIndex, const ItemFlags i_nAdditionalItemFlags ) const
    {
        const ItemDescriptor& rItem( m_aItems[ i_nItemIndex ] );
        const ItemFlags nItemFlags( impl_getItemFlags( i_nItemIndex ) | i_nAdditionalItemFlags );

        const Rectangle aNormalizedContent( GetActualLogicalItemRect( rItem.GetCurrentRect() ) );
        const Rectangle aNormalizedBounds( m_pRenderer->calculateDecorations( aNormalizedContent, nItemFlags ) );

        const Rectangle aActualBounds = m_aNormalizer.getTransformed( aNormalizedBounds, m_eTabAlignment );
        m_rTabBar.Invalidate( aActualBounds );
    }


    ItemFlags PanelTabBar_Impl::impl_getItemFlags( const size_t i_nItemIndex ) const
    {
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

        return nItemFlags;
    }


    void PanelTabBar_Impl::DrawItem(vcl::RenderContext& rRenderContext, const size_t i_nItemIndex, const Rectangle& i_rBoundaries) const
    {
        const ItemDescriptor& rItem(m_aItems[i_nItemIndex]);
        const ItemFlags nItemFlags(impl_getItemFlags(i_nItemIndex));

        // the normalized bounding and content rect
        const Rectangle aNormalizedContent(GetActualLogicalItemRect(rItem.GetCurrentRect()));
        const Rectangle aNormalizedBounds(m_pRenderer->calculateDecorations(aNormalizedContent, nItemFlags));

        // check whether the item actually overlaps with the painting area
        if (!i_rBoundaries.IsEmpty())
        {
            const Rectangle aItemRect(GetActualLogicalItemRect(rItem.GetCurrentRect()));
            if (!aItemRect.IsOver(i_rBoundaries))
                return;
        }

        m_rTabBar.SetUpdateMode(false);

        // the aligned bounding and content rect
        const Rectangle aActualBounds = m_aNormalizer.getTransformed( aNormalizedBounds, m_eTabAlignment );
        const Rectangle aActualContent = m_aNormalizer.getTransformed( aNormalizedContent, m_eTabAlignment );

        // render item "background" layer
        m_pRenderer->preRenderItem(aNormalizedContent, nItemFlags);

        // copy from the virtual device to ourself
        CopyFromRenderDevice(rRenderContext, aNormalizedBounds);

        // render the actual item content
        impl_renderItemContent(rRenderContext, rItem.pPanel, aActualContent, rItem.eContent);

        // render item "foreground" layer
        m_pRenderer->postRenderItem(m_rTabBar, aActualBounds, nItemFlags);

        m_rTabBar.SetUpdateMode(true);
    }


    void PanelTabBar_Impl::EnsureItemsCache()
    {
        if ( !m_bItemsDirty )
        {
            DBG_CHECK( *this );
            return;
        }
        impl_calcItemRects();
        SAL_WARN_IF( m_bItemsDirty , "svtools", "PanelTabBar_Impl::EnsureItemsCache: cache still dirty!" );
        DBG_CHECK( *this );
    }


    void PanelTabBar_Impl::Relayout()
    {
        EnsureItemsCache();

        const Size aOutputSize( m_rTabBar.GetOutputSizePixel() );
        m_aNormalizer = NormalizedArea( Rectangle( Point(), aOutputSize ), IsVertical() );
        const Size aLogicalOutputSize( m_aNormalizer.getReferenceSize() );

        // forward actual output size to our render device
        m_aRenderDevice->SetOutputSizePixel( aLogicalOutputSize );

        // re-calculate the size of the scroll buttons and of the items
        m_aGeometry.relayout( aLogicalOutputSize, m_aItems );

        if ( m_aGeometry.getButtonBackRect().IsEmpty() )
        {
            m_aScrollBack->Hide();
        }
        else
        {
            const Rectangle aButtonBack( m_aNormalizer.getTransformed( m_aGeometry.getButtonBackRect(), m_eTabAlignment ) );
            m_aScrollBack->SetPosSizePixel( aButtonBack.TopLeft(), aButtonBack.GetSize() );
            m_aScrollBack->Show();
        }

        if ( m_aGeometry.getButtonForwardRect().IsEmpty() )
        {
            m_aScrollForward->Hide();
        }
        else
        {
            const Rectangle aButtonForward( m_aNormalizer.getTransformed( m_aGeometry.getButtonForwardRect(), m_eTabAlignment ) );
            m_aScrollForward->SetPosSizePixel( aButtonForward.TopLeft(), aButtonForward.GetSize() );
            m_aScrollForward->Show();
        }

        UpdateScrollButtons();
    }


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


    Rectangle PanelTabBar_Impl::GetItemScreenRect( const size_t i_nItemPos ) const
    {
        ENSURE_OR_RETURN( i_nItemPos < m_aItems.size(), "PanelTabBar_Impl::GetItemScreenRect: invalid item pos!", Rectangle() );
        const ItemDescriptor& rItem( m_aItems[ i_nItemPos ] );
        const Rectangle aItemRect( m_aNormalizer.getTransformed(
            GetActualLogicalItemRect( rItem.GetCurrentRect() ),
            m_eTabAlignment ) );

        const Rectangle aTabBarRect( m_rTabBar.GetWindowExtentsRelative( NULL ) );
        return Rectangle(
            Point( aTabBarRect.Left() + aItemRect.Left(), aTabBarRect.Top() + aItemRect.Top() ),
            aItemRect.GetSize()
        );
    }


    void PanelTabBar_Impl::FocusItem( const ::boost::optional< size_t >& i_rItemPos )
    {
        // reset old focus item
        if ( !!m_aFocusedItem )
            InvalidateItem( *m_aFocusedItem );
        m_aFocusedItem.reset();

        // mark the active icon as focused
        if ( !!i_rItemPos )
        {
            m_aFocusedItem = i_rItemPos;
            InvalidateItem( *m_aFocusedItem );
        }
    }


    IMPL_LINK_TYPED( PanelTabBar_Impl, OnScroll, Button*, pButton, void )
    {
        PushButton* i_pButton = static_cast<PushButton*>(pButton);
        if ( i_pButton == m_aScrollBack.get() )
        {
            OSL_ENSURE( m_nScrollPosition > 0, "PanelTabBar_Impl::OnScroll: inconsistency!" );
            --m_nScrollPosition;
            m_rTabBar.Invalidate();
        }
        else if ( i_pButton == m_aScrollForward.get() )
        {
            OSL_ENSURE( m_nScrollPosition < m_aItems.size() - 1, "PanelTabBar_Impl::OnScroll: inconsistency!" );
            ++m_nScrollPosition;
            m_rTabBar.Invalidate();
        }

        UpdateScrollButtons();
    }


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


    //= PanelTabBar_Impl


    void PanelTabBar_Impl::ActivePanelChanged( const ::boost::optional< size_t >& i_rOldActive, const ::boost::optional< size_t >& i_rNewActive )
    {
        EnsureItemsCache();

        if ( !!i_rOldActive )
            InvalidateItem( *i_rOldActive, ITEM_STATE_ACTIVE );
        if ( !!i_rNewActive )
            InvalidateItem( *i_rNewActive );
    }


    void PanelTabBar_Impl::LayouterChanged( const PDeckLayouter& i_rNewLayouter )
    {
        // not interested in
        (void)i_rNewLayouter;
    }


    void PanelTabBar_Impl::Dying()
    {
        // not interested in - the notifier is a member of this instance here, so we're dying ourself at the moment
    }


    //= PanelTabBar


    PanelTabBar::PanelTabBar( vcl::Window& i_rParentWindow, IToolPanelDeck& i_rPanelDeck, const TabAlignment i_eAlignment, const TabItemContent i_eItemContent )
        :Control( &i_rParentWindow, 0 )
        ,m_pImpl( new PanelTabBar_Impl( *this, i_rPanelDeck, i_eAlignment, i_eItemContent ) )
    {
        DBG_CHECK( *m_pImpl );
    }

    PanelTabBar::~PanelTabBar()
    {
        disposeOnce();
    }

    void PanelTabBar::dispose()
    {
        Control::dispose();
    }

    TabItemContent PanelTabBar::GetTabItemContent() const
    {
        return m_pImpl->m_aGeometry.getItemContent();
    }


    void PanelTabBar::SetTabItemContent( const TabItemContent& i_eItemContent )
    {
        m_pImpl->m_aGeometry.setItemContent( i_eItemContent );
        m_pImpl->Relayout();
        Invalidate();
    }


    IToolPanelDeck& PanelTabBar::GetPanelDeck() const
    {
        DBG_CHECK( *m_pImpl );
        return m_pImpl->m_rPanelDeck;
    }


    Size PanelTabBar::GetOptimalSize() const
    {
        m_pImpl->EnsureItemsCache();
        Size aOptimalSize(m_pImpl->m_aGeometry.getOptimalSize(m_pImpl->m_aItems));
        if ( m_pImpl->IsVertical() )
            ::std::swap( aOptimalSize.Width(), aOptimalSize.Height() );
        return aOptimalSize;
    }


    void PanelTabBar::Resize()
    {
        Control::Resize();
        m_pImpl->Relayout();
    }


    void PanelTabBar::Paint(vcl::RenderContext& rRenderContext, const Rectangle& i_rRect)
    {
        m_pImpl->EnsureItemsCache();

        // background
        const Rectangle aNormalizedPaintArea(m_pImpl->m_aNormalizer.getNormalized(i_rRect, m_pImpl->m_eTabAlignment));
        m_pImpl->m_aRenderDevice->Push(PushFlags::CLIPREGION);
        m_pImpl->m_aRenderDevice->SetClipRegion(vcl::Region(aNormalizedPaintArea));
        m_pImpl->m_pRenderer->renderBackground();
        m_pImpl->m_aRenderDevice->Pop();
        m_pImpl->CopyFromRenderDevice(rRenderContext, aNormalizedPaintArea);

        // ensure the items really paint into their own playground only
        ClipItemRegion aClipItems(*m_pImpl);

        const Rectangle aLogicalPaintRect(m_pImpl->m_aNormalizer.getNormalized(i_rRect, m_pImpl->m_eTabAlignment));

        const boost::optional<size_t> aActivePanel(m_pImpl->m_rPanelDeck.GetActivePanel());
        const boost::optional<size_t> aHoveredPanel(m_pImpl->m_aHoveredItem);

        // items:
        // 1. paint all non-active, non-hovered items
        size_t i = 0;
        ItemDescriptors::const_iterator item;
        for (item = m_pImpl->m_aItems.begin(); item != m_pImpl->m_aItems.end(); ++item, ++i)
        {
            if (i == aActivePanel)
                continue;

            if (aHoveredPanel == i)
                continue;

            m_pImpl->DrawItem(rRenderContext, i, aLogicalPaintRect);
        }

        // 2. paint the item which is hovered, /without/ the mouse button pressed down
        if (!!aHoveredPanel && !m_pImpl->m_bMouseButtonDown)
            m_pImpl->DrawItem(rRenderContext, *aHoveredPanel, aLogicalPaintRect);

        // 3. paint the active item
        if (!!aActivePanel)
            m_pImpl->DrawItem(rRenderContext, *aActivePanel, aLogicalPaintRect);

        // 4. paint the item which is hovered, /with/ the mouse button pressed down
        if (!!aHoveredPanel && m_pImpl->m_bMouseButtonDown)
            m_pImpl->DrawItem(rRenderContext, *aHoveredPanel, aLogicalPaintRect);
    }


    void PanelTabBar::MouseMove( const MouseEvent& i_rMouseEvent )
    {
        m_pImpl->EnsureItemsCache();

        boost::optional< size_t > aOldItem(m_pImpl->m_aHoveredItem);
        boost::optional< size_t > aNewItem(m_pImpl->FindItemForPoint(i_rMouseEvent.GetPosPixel()));

        if (i_rMouseEvent.IsLeaveWindow())
            aNewItem = boost::optional<size_t>();

        bool const bChanged(
                ( !aOldItem && aNewItem )
                || ( aOldItem && !aNewItem )
                || ( aOldItem && aNewItem && aOldItem != aNewItem ) );

        if (bChanged)
        {
            if (aOldItem)
                m_pImpl->InvalidateItem( *aOldItem );

            m_pImpl->m_aHoveredItem = aNewItem;

            if (aNewItem)
                m_pImpl->InvalidateItem( *aNewItem );
        }
    }


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

        m_pImpl->InvalidateItem( *aHitItem );
    }


    void PanelTabBar::MouseButtonUp( const MouseEvent& i_rMouseEvent )
    {
        Control::MouseButtonUp( i_rMouseEvent );

        if ( m_pImpl->m_bMouseButtonDown )
        {
            ::boost::optional< size_t > aHitItem( m_pImpl->FindItemForPoint( i_rMouseEvent.GetPosPixel() ) );
            if ( !!aHitItem )
            {
                // re-draw that item now that we're not in mouse-down mode anymore
                m_pImpl->InvalidateItem( *aHitItem );
                // activate the respective panel
                m_pImpl->m_rPanelDeck.ActivatePanel( *aHitItem );
            }

            OSL_ENSURE( IsMouseCaptured(), "PanelTabBar::MouseButtonUp: inconsistency!" );
            if ( IsMouseCaptured() )
                ReleaseMouse();
            m_pImpl->m_bMouseButtonDown = false;
        }
    }


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

        const OUString sItemText( rItem.pPanel->GetDisplayName() );
        if ( i_rHelpEvent.GetMode() == HelpEventMode::BALLOON )
            Help::ShowBalloon( this, OutputToScreenPixel( rItem.GetCurrentRect().Center() ), rItem.GetCurrentRect(), sItemText );
        else
            Help::ShowQuickHelp( this, rItem.GetCurrentRect(), sItemText );
    }


    void PanelTabBar::GetFocus()
    {
        Control::GetFocus();
        if ( !m_pImpl->m_aFocusedItem )
            m_pImpl->FocusItem( m_pImpl->m_rPanelDeck.GetActivePanel() );
    }


    void PanelTabBar::LoseFocus()
    {
        Control::LoseFocus();

        if ( m_pImpl )
        {
            if ( !!m_pImpl->m_aFocusedItem )
                m_pImpl->InvalidateItem( *m_pImpl->m_aFocusedItem );

            m_pImpl->m_aFocusedItem.reset();
        }
    }


    class KeyInputHandler
    {
    public:
        KeyInputHandler( Control& i_rControl, const KeyEvent& i_rKeyEvent )
            :m_rControl( i_rControl )
            ,m_rKeyEvent( i_rKeyEvent )
            ,m_bHandled( false )
        {
        }

        ~KeyInputHandler()
        {
            if ( !m_bHandled )
                m_rControl.Control::KeyInput( m_rKeyEvent );
        }

        void   setHandled()
        {
            m_bHandled = true;
        }

    private:
        Control&        m_rControl;
        const KeyEvent& m_rKeyEvent;
        bool            m_bHandled;
    };


    void PanelTabBar::KeyInput( const KeyEvent& i_rKeyEvent )
    {
        KeyInputHandler aKeyInputHandler( *this, i_rKeyEvent );

        const vcl::KeyCode& rKeyCode( i_rKeyEvent.GetKeyCode() );
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

        m_pImpl->InvalidateItem( *m_pImpl->m_aFocusedItem );
        if ( bFocusNext )
        {
            m_pImpl->m_aFocusedItem.reset( ( *m_pImpl->m_aFocusedItem + 1 ) % nPanelCount );
        }
        else
        {
            m_pImpl->m_aFocusedItem.reset( ( *m_pImpl->m_aFocusedItem + nPanelCount - 1 ) % nPanelCount );
        }
        m_pImpl->InvalidateItem( *m_pImpl->m_aFocusedItem );

        // don't delegate to base class
        aKeyInputHandler.setHandled();
    }


    void PanelTabBar::DataChanged( const DataChangedEvent& i_rDataChanedEvent )
    {
        Control::DataChanged( i_rDataChanedEvent );

        if  (   ( i_rDataChanedEvent.GetType() == DataChangedEventType::SETTINGS )
            &&  ( i_rDataChanedEvent.GetFlags() & AllSettingsFlags::STYLE )
            )
        {
            Invalidate();
        }
    }


    bool PanelTabBar::IsVertical() const
    {
        return m_pImpl->IsVertical();
    }


    PushButton& PanelTabBar::GetScrollButton( const bool i_bForward )
    {
        return i_bForward ? *m_pImpl->m_aScrollForward.get() : *m_pImpl->m_aScrollBack.get();
    }


    ::boost::optional< size_t > PanelTabBar::GetFocusedPanelItem() const
    {
        return m_pImpl->m_aFocusedItem;
    }


    void PanelTabBar::FocusPanelItem( const size_t i_nItemPos )
    {
        ENSURE_OR_RETURN_VOID( i_nItemPos < m_pImpl->m_rPanelDeck.GetPanelCount(), "PanelTabBar::FocusPanelItem: illegal item pos!" );

        if ( !HasChildPathFocus() )
            GrabFocus();

        m_pImpl->FocusItem( i_nItemPos );
        SAL_WARN_IF( !m_pImpl->m_aFocusedItem, "svtools", "PanelTabBar::FocusPanelItem: have the focus, but no focused item?" );
        if ( !!m_pImpl->m_aFocusedItem )
            m_pImpl->InvalidateItem( *m_pImpl->m_aFocusedItem );
        m_pImpl->m_aFocusedItem.reset( i_nItemPos );
    }


    Rectangle PanelTabBar::GetItemScreenRect( const size_t i_nItemPos ) const
    {
        return m_pImpl->GetItemScreenRect( i_nItemPos );
    }


    Reference< XWindowPeer > PanelTabBar::GetComponentInterface( bool i_bCreate )
    {
        Reference< XWindowPeer > xWindowPeer( Control::GetComponentInterface( false ) );
        if ( !xWindowPeer.is() && i_bCreate )
        {
            xWindowPeer.set( new PanelTabBarPeer( *this ) );
            SetComponentInterface( xWindowPeer );
        }
        return xWindowPeer;
    }


} // namespace svt


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
