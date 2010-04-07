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

#include <vcl/window.hxx>
#include <vcl/virdev.hxx>

//......................................................................................................................
namespace svt
{
//......................................................................................................................

    //==================================================================================================================
    //= ToolPanelDrawer
    //==================================================================================================================
    //------------------------------------------------------------------------------------------------------------------
    class ToolPanelDrawer : public Window
    {
    public:
        ToolPanelDrawer( Window& i_rParent );
        ~ToolPanelDrawer();

        long GetPreferredHeightPixel();

        void    SetExpanded( const bool i_bExpanded );

        virtual void Paint( const Rectangle& i_rBoundingBox );
        virtual void GetFocus();
        virtual void LoseFocus();
        virtual void DataChanged( const DataChangedEvent& i_rEvent );
        virtual void SetText( const String& i_rText );

    private:
        Rectangle   impl_calcTextBoundingBox();
        Rectangle   impl_calcTitleBarBox( const Rectangle& i_rTextBox );
        void        impl_paintBackground( const Rectangle& i_rTitleBarBox );
        USHORT      impl_getTextStyle();
        void        impl_paintFocusIndicator( const Rectangle& i_rTextBox );
        Rectangle   impl_paintExpansionIndicator( const Rectangle& i_rTextBox );
        Image       impl_getExpansionIndicator() const;

    private:
        ::std::auto_ptr< VirtualDevice >    m_pPaintDevice;
        bool                                m_bFocused;
        bool                                m_bExpanded;
    };

//......................................................................................................................
} // namespace svt
//......................................................................................................................
