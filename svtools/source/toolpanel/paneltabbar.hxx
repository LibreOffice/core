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
#ifndef PANELTABBAR_HXX
#define PANELTABBAR_HXX

#include "svtools/toolpanel/toolpanelcontainer.hxx"

#include <vcl/ctrl.hxx>

#include <memory>

//........................................................................
namespace svt
{
//........................................................................

    class ToolPanelDeck;
    class PanelTabBar_Data;

    //====================================================================
    //= PanelTabBar
    //====================================================================
    /** a tab bar for selecting panels

        At the moment, this control aligns the tabs vertically, this might be extended to also support a horizontal
        layout in the future.
    */
    class PanelTabBar : public Control
    {
    public:
        PanelTabBar( ToolPanelDeck& i_rParent );
        ~PanelTabBar();

        // Window overridables
        virtual Size    GetOptimalSize( WindowSizeType i_eType ) const;
        virtual void    Paint( const Rectangle& i_rRect );
        virtual void    Resize();
        virtual void    MouseMove( const MouseEvent& i_rMouseEvent );
        virtual void    MouseButtonDown( const MouseEvent& i_rMouseEvent );
        virtual void    MouseButtonUp( const MouseEvent& i_rMouseEvent );
        virtual void    RequestHelp( const HelpEvent& i_rHelpEvent );
        virtual void    GetFocus();
        virtual void    LoseFocus();
        virtual void    KeyInput( const KeyEvent& i_rKeyEvent );
        virtual void    DataChanged( const DataChangedEvent& i_rDataChanedEvent );

    private:
        ::std::auto_ptr< PanelTabBar_Data > m_pData;
    };

//........................................................................
} // namespace svt
//........................................................................

#endif // PANELTABBAR_HXX

