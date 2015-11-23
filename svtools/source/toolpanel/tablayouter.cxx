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


#include <svtools/toolpanel/tablayouter.hxx>
#include <svtools/toolpanel/toolpaneldeck.hxx>
#include <svtools/toolpanel/paneltabbar.hxx>
#include "svtaccessiblefactory.hxx"

#include <tools/gen.hxx>
#include <tools/diagnose_ex.h>


namespace svt
{


    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::accessibility::XAccessible;


    //= TabDeckLayouter_Data

    struct TabDeckLayouter_Data
    {
        TabAlignment                    eAlignment;
        VclPtr< PanelTabBar >           pTabBar;
        AccessibleFactoryAccess         aAccessibleFactory;

        TabDeckLayouter_Data( vcl::Window& i_rParent, IToolPanelDeck& i_rPanels,
                const TabAlignment i_eAlignment, const TabItemContent i_eItemContent )
            :eAlignment( i_eAlignment )
            ,pTabBar( VclPtr<PanelTabBar>::Create( i_rParent, i_rPanels, i_eAlignment, i_eItemContent ) )
        {
            pTabBar->Show();
        }
    };


    //= helper

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


    //= TabDeckLayouter


    TabDeckLayouter::TabDeckLayouter( vcl::Window& i_rParent, IToolPanelDeck& i_rPanels,
            const TabAlignment i_eAlignment, const TabItemContent i_eItemContent )
        :m_pData( new TabDeckLayouter_Data( i_rParent, i_rPanels, i_eAlignment, i_eItemContent ) )
    {
    }


    TabDeckLayouter::~TabDeckLayouter()
    {
    }


    TabItemContent TabDeckLayouter::GetTabItemContent() const
    {
        if ( lcl_checkDisposed( *m_pData ) )
            return TABITEM_IMAGE_AND_TEXT;
        return m_pData->pTabBar->GetTabItemContent();
    }


    void TabDeckLayouter::SetTabItemContent( const TabItemContent& i_eItemContent )
    {
        if ( lcl_checkDisposed( *m_pData ) )
            return;
        m_pData->pTabBar->SetTabItemContent( i_eItemContent );
    }


    TabAlignment TabDeckLayouter::GetTabAlignment() const
    {
        if ( lcl_checkDisposed( *m_pData ) )
            return TABS_RIGHT;
        return m_pData->eAlignment;
    }


    Rectangle TabDeckLayouter::Layout( const Rectangle& i_rDeckPlayground )
    {
        if ( lcl_checkDisposed( *m_pData ) )
            return i_rDeckPlayground;

        const Size aPreferredSize(m_pData->pTabBar->GetOptimalSize());
        if ( lcl_isVerticalTabBar( m_pData->eAlignment ) )
        {
            Size aTabBarSize(aPreferredSize.Width(), i_rDeckPlayground.GetHeight());

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

        Size aTabBarSize(i_rDeckPlayground.GetWidth(), aPreferredSize.Height());

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


    void TabDeckLayouter::Destroy()
    {
        m_pData->pTabBar.reset();
    }


    void TabDeckLayouter::SetFocusToPanelSelector()
    {
        if ( lcl_checkDisposed( *m_pData ) )
            return;
        m_pData->pTabBar->GrabFocus();
    }


    size_t TabDeckLayouter::GetAccessibleChildCount() const
    {
        if ( lcl_checkDisposed( *m_pData ) )
            return 0;

        return 1;
    }


    Reference< XAccessible > TabDeckLayouter::GetAccessibleChild( const size_t i_nChildIndex, const Reference< XAccessible >& i_rParentAccessible )
    {
        (void)i_nChildIndex;
        (void)i_rParentAccessible;
        if ( lcl_checkDisposed( *m_pData ) )
            return nullptr;

        return m_pData->pTabBar->GetAccessible();
    }


} // namespace svt


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
