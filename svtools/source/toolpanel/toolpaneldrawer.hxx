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
        ToolPanelDrawer( Window& i_rParent, const ::rtl::OUString& i_rTitle );
        ~ToolPanelDrawer();

        long    GetPreferredHeightPixel() const;
        void    SetExpanded( const bool i_bExpanded );
        bool    IsExpanded() const { return m_bExpanded; }

    protected:
        // Window overridables
        virtual void Paint( const Rectangle& i_rBoundingBox );
        virtual void GetFocus();
        virtual void LoseFocus();
        virtual void DataChanged( const DataChangedEvent& i_rEvent );
        virtual void MouseButtonDown( const MouseEvent& i_rMouseEvent );

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >
                     GetComponentInterface( BOOL i_bCreate );

    private:
        Rectangle   impl_calcTextBoundingBox() const;
        Rectangle   impl_calcTitleBarBox( const Rectangle& i_rTextBox ) const;
        void        impl_paintBackground( const Rectangle& i_rTitleBarBox );
        USHORT      impl_getTextStyle() const;
        void        impl_paintFocusIndicator( const Rectangle& i_rTextBox );
        Rectangle   impl_paintExpansionIndicator( const Rectangle& i_rTextBox );
        Image       impl_getExpansionIndicator() const;

        // don't expose SetText. Our text is used as AccessibleName/Desc, and those are not expected to change.
        using Window::SetText;

    private:
        ::std::auto_ptr< VirtualDevice >    m_pPaintDevice;
        bool                                m_bFocused;
        bool                                m_bExpanded;
    };

//......................................................................................................................
} // namespace svt
//......................................................................................................................
