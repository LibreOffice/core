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

#ifndef SVT_DRAWERLAYOUTER_HXX
#define SVT_DRAWERLAYOUTER_HXX

#include "svtools/svtdllapi.h"
#include "svtools/toolpanel/refbase.hxx"
#include "svtools/toolpanel/toolpaneldeck.hxx"
#include "svtools/toolpanel/decklayouter.hxx"

#include <boost/shared_ptr.hpp>

//......................................................................................................................
namespace svt
{
//......................................................................................................................

    class ToolPanelViewShell;
    class ToolPanelDrawer;
    typedef ::boost::shared_ptr< ToolPanelDrawer >  PToolPanelDrawer;

    //==================================================================================================================
    //= ToolPanelDrawer
    //==================================================================================================================
    /** a class which implements a tool panel selector in the form of the classical drawers
    */
    class SVT_DLLPUBLIC DrawerDeckLayouter  :public RefBase
                                            ,public IDeckLayouter
                                            ,public IToolPanelDeckListener
    {
    public:
        DrawerDeckLayouter(
            ::Window& i_rParentWindow,
            IToolPanelDeck& i_rPanels
        );
        ~DrawerDeckLayouter();

        // IReference
        DECLARE_IREFERENCE()

        // IDeckLayouter
        virtual Rectangle   Layout( const Rectangle& i_rDeckPlayground );
        virtual void        Destroy();
        virtual void        SetFocusToPanelSelector();
        virtual size_t      GetAccessibleChildCount() const;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
                            GetAccessibleChild(
                                const size_t i_nChildIndex,
                                const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& i_rParentAccessible
                            );

        // IToolPanelDeckListener
        virtual void PanelInserted( const PToolPanel& i_pPanel, const size_t i_nPosition );
        virtual void PanelRemoved( const size_t i_nPosition );
        virtual void ActivePanelChanged( const ::boost::optional< size_t >& i_rOldActive, const ::boost::optional< size_t >& i_rNewActive );
        virtual void LayouterChanged( const PDeckLayouter& i_rNewLayouter );
        virtual void Dying();

    private:
        // triggers a re-arrance of the panel deck elements
        void    impl_triggerRearrange() const;
        size_t  impl_getPanelPositionFromWindow( const Window* i_pDrawerWindow ) const;
        void    impl_removeDrawer( const size_t i_nPosition );

        DECL_LINK( OnWindowEvent, VclSimpleEvent* );

private:
        Window&                             m_rParentWindow;
        IToolPanelDeck&                     m_rPanelDeck;
        ::std::vector< PToolPanelDrawer >   m_aDrawers;
        ::boost::optional< size_t >         m_aLastKnownActivePanel;
    };

//......................................................................................................................
} // namespace svt
//......................................................................................................................

#endif // SVT_DRAWERLAYOUTER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
