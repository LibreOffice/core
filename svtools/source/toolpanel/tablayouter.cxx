/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include "svtools/toolpanel/tablayouter.hxx"
#include "svtools/toolpanel/toolpaneldeck.hxx"
#include "svtools/toolpanel/paneltabbar.hxx"
#include "svtaccessiblefactory.hxx"

#include <tools/gen.hxx>
#include <tools/diagnose_ex.h>

//........................................................................
namespace svt
{
//........................................................................

    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::accessibility::XAccessible;

    //====================================================================
    //= TabDeckLayouter_Data
    //====================================================================
    struct TabDeckLayouter_Data
    {
        TabAlignment                    eAlignment;
        IToolPanelDeck&                 rPanels;
        ::std::auto_ptr< PanelTabBar >  pTabBar;
        AccessibleFactoryAccess         aAccessibleFactory;

        TabDeckLayouter_Data( Window& i_rParent, IToolPanelDeck& i_rPanels,
                const TabAlignment i_eAlignment, const TabItemContent i_eItemContent )
            :eAlignment( i_eAlignment )
            ,rPanels( i_rPanels )
            ,pTabBar( new PanelTabBar( i_rParent, i_rPanels, i_eAlignment, i_eItemContent ) )
        {
            pTabBar->Show();
        }
    };

    //====================================================================
    //= helper
    //====================================================================
    namespace
    {
        static bool lcl_isVerticalTabBar( const TabAlignment i_eAlignment )
        {
            return  ( i_eAlignment == TABS_RIGHT )
                ||  ( i_eAlignment == TABS_LEFT );
        }

        static bool lcl_checkDisposed( const TabDeckLayouter_Data& i_rData )
        {
            if ( !i_rData.pTabBar.get() )
            {
                OSL_FAIL( "lcl_checkDisposed: already disposed!" );
                return true;
            }
            return false;
        }
    }

    //====================================================================
    //= TabDeckLayouter
    //====================================================================
    //--------------------------------------------------------------------
    TabDeckLayouter::TabDeckLayouter( Window& i_rParent, IToolPanelDeck& i_rPanels,
            const TabAlignment i_eAlignment, const TabItemContent i_eItemContent )
        :m_pData( new TabDeckLayouter_Data( i_rParent, i_rPanels, i_eAlignment, i_eItemContent ) )
    {
    }

    //--------------------------------------------------------------------
    TabDeckLayouter::~TabDeckLayouter()
    {
    }

    //--------------------------------------------------------------------
    IMPLEMENT_IREFERENCE( TabDeckLayouter )

    //--------------------------------------------------------------------
    TabItemContent TabDeckLayouter::GetTabItemContent() const
    {
        if ( lcl_checkDisposed( *m_pData ) )
            return TABITEM_IMAGE_AND_TEXT;
        return m_pData->pTabBar->GetTabItemContent();
    }

    //--------------------------------------------------------------------
    void TabDeckLayouter::SetTabItemContent( const TabItemContent& i_eItemContent )
    {
        if ( lcl_checkDisposed( *m_pData ) )
            return;
        m_pData->pTabBar->SetTabItemContent( i_eItemContent );
    }

    //--------------------------------------------------------------------
    TabAlignment TabDeckLayouter::GetTabAlignment() const
    {
        if ( lcl_checkDisposed( *m_pData ) )
            return TABS_RIGHT;
        return m_pData->eAlignment;
    }

    //--------------------------------------------------------------------
    ::boost::optional< size_t > TabDeckLayouter::GetFocusedPanelItem() const
    {
        if ( lcl_checkDisposed( *m_pData ) )
            return ::boost::optional< size_t >();
        return m_pData->pTabBar->GetFocusedPanelItem();
    }

    //--------------------------------------------------------------------
    void TabDeckLayouter::FocusPanelItem( const size_t i_nItemPos )
    {
        if ( lcl_checkDisposed( *m_pData ) )
            return;
        m_pData->pTabBar->FocusPanelItem( i_nItemPos );
    }

    //--------------------------------------------------------------------
    bool TabDeckLayouter::IsPanelSelectorEnabled() const
    {
        if ( lcl_checkDisposed( *m_pData ) )
            return false;
        return m_pData->pTabBar->IsEnabled();
    }

    //--------------------------------------------------------------------
    bool TabDeckLayouter::IsPanelSelectorVisible() const
    {
        if ( lcl_checkDisposed( *m_pData ) )
            return false;
        return m_pData->pTabBar->IsVisible();
    }

    //--------------------------------------------------------------------
    Rectangle TabDeckLayouter::GetItemScreenRect( const size_t i_nItemPos ) const
    {
        if ( lcl_checkDisposed( *m_pData ) )
            return Rectangle();
        return m_pData->pTabBar->GetItemScreenRect( i_nItemPos );
    }

    //--------------------------------------------------------------------
    Rectangle TabDeckLayouter::Layout( const Rectangle& i_rDeckPlayground )
    {
        if ( lcl_checkDisposed( *m_pData ) )
            return i_rDeckPlayground;

        const Size aPreferredSize( m_pData->pTabBar->GetOptimalSize( WINDOWSIZE_PREFERRED ) );
        if ( lcl_isVerticalTabBar( m_pData->eAlignment ) )
        {
            Size aTabBarSize =  ( aPreferredSize.Width() < i_rDeckPlayground.GetWidth() )
                            ?   aPreferredSize
                            :   m_pData->pTabBar->GetOptimalSize( WINDOWSIZE_MINIMUM );
            aTabBarSize.Height() = i_rDeckPlayground.GetHeight();

            Rectangle aPanelRect( i_rDeckPlayground );
            if ( m_pData->eAlignment == TABS_RIGHT )
            {
                aPanelRect.Right() -= aTabBarSize.Width();
                Point aTabBarTopLeft( aPanelRect.TopRight() );
                aTabBarTopLeft.X() += 1;
                m_pData->pTabBar->SetPosSizePixel( aTabBarTopLeft, aTabBarSize );
            }
            else
            {
                m_pData->pTabBar->SetPosSizePixel( aPanelRect.TopLeft(), aTabBarSize );
                aPanelRect.Left() += aTabBarSize.Width();
            }
            if ( aPanelRect.Left() >= aPanelRect.Right() )
                aPanelRect = Rectangle();

            return aPanelRect;
        }

        Size aTabBarSize =  ( aPreferredSize.Height() < i_rDeckPlayground.GetHeight() )
                        ?   aPreferredSize
                        :   m_pData->pTabBar->GetOptimalSize( WINDOWSIZE_MINIMUM );
        aTabBarSize.Width() = i_rDeckPlayground.GetWidth();

        Rectangle aPanelRect( i_rDeckPlayground );
        if ( m_pData->eAlignment == TABS_TOP )
        {
            m_pData->pTabBar->SetPosSizePixel( aPanelRect.TopLeft(), aTabBarSize );
            aPanelRect.Top() += aTabBarSize.Height();
        }
        else
        {
            aPanelRect.Bottom() -= aTabBarSize.Height();
            Point aTabBarTopLeft( aPanelRect.BottomLeft() );
            aTabBarTopLeft.Y() -= 1;
            m_pData->pTabBar->SetPosSizePixel( aTabBarTopLeft, aTabBarSize );
        }
        if ( aPanelRect.Top() >= aPanelRect.Bottom() )
            aPanelRect = Rectangle();

        return aPanelRect;
    }

    //--------------------------------------------------------------------
    void TabDeckLayouter::Destroy()
    {
        m_pData->pTabBar.reset();
    }

    //--------------------------------------------------------------------
    void TabDeckLayouter::SetFocusToPanelSelector()
    {
        if ( lcl_checkDisposed( *m_pData ) )
            return;
        m_pData->pTabBar->GrabFocus();
    }

    //--------------------------------------------------------------------
    size_t TabDeckLayouter::GetAccessibleChildCount() const
    {
        if ( lcl_checkDisposed( *m_pData ) )
            return 0;

        return 1;
    }

    //--------------------------------------------------------------------
    Reference< XAccessible > TabDeckLayouter::GetAccessibleChild( const size_t i_nChildIndex, const Reference< XAccessible >& i_rParentAccessible )
    {
        (void)i_nChildIndex;
        (void)i_rParentAccessible;
        if ( lcl_checkDisposed( *m_pData ) )
            return NULL;

        return m_pData->pTabBar->GetAccessible( TRUE );
    }

//........................................................................
} // namespace svt
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
