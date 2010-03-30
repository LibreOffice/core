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

#ifndef SD_TOOLPANEL_TOOLPANELDRAWER_HXX
#define SD_TOOLPANEL_TOOLPANELDRAWER_HXX

#include <svtools/toolpanel/refbase.hxx>
#include <svtools/toolpanel/toolpaneldeck.hxx>
#include <svtools/toolpanel/decklayouter.hxx>

#include <boost/shared_ptr.hpp>

//......................................................................................................................
namespace sd { namespace toolpanel
{
//......................................................................................................................

    class ToolPanelViewShell;
    class TitleBar;
    typedef ::boost::shared_ptr< TitleBar > PTitleBar;

    //==================================================================================================================
    //= ToolPanelDrawer
    //==================================================================================================================
    /** a class which implements a tool panel selector in the form of the classical drawers
    */
    class ToolPanelDrawer   :public ::svt::RefBase
                            ,public ::svt::IDeckLayouter
                            ,public ::svt::IToolPanelDeckListener
    {
    public:
        ToolPanelDrawer(
            ::Window& i_rParentWindow,
            ::svt::IToolPanelDeck& i_rPanels
        );
        ~ToolPanelDrawer();

        // IReference
        DECLARE_IREFERENCE()

        // IDeckLayouter
        virtual Rectangle   Layout( const Rectangle& i_rDeckPlayground );
        virtual void        Destroy();
        virtual void        SetFocusToPanelSelector();
        ::boost::optional< size_t >
                            GetPanelItemFromScreenPos( const ::Point& i_rScreenPos );
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
                            GetPanelItemAccessible(
                                const size_t i_nItemPos,
                                const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& i_rParentAccessible
                            );

        // IToolPanelDeckListener
        virtual void PanelInserted( const ::svt::PToolPanel& i_pPanel, const size_t i_nPosition );
        virtual void PanelRemoved( const size_t i_nPosition );
        virtual void ActivePanelChanged( const ::boost::optional< size_t >& i_rOldActive, const ::boost::optional< size_t >& i_rNewActive );
        virtual void LayouterChanged( const ::svt::PDeckLayouter& i_rNewLayouter );
        virtual void Dying();

    private:
        // triggers a re-arrance of the panel deck elements
        void    impl_triggerRearrange() const;
        size_t  impl_getPanelPositionFromWindow( const Window* i_pTitleBarWindow ) const;
        void    impl_removeTitleBar( const size_t i_nPosition );

        DECL_LINK( OnWindowEvent, VclSimpleEvent* );

private:
        Window&                     m_rParentWindow;
        ::svt::IToolPanelDeck&      m_rPanelDeck;
        ::std::vector< PTitleBar >  m_aDrawers;
        ::boost::optional< size_t > m_aLastKnownActivePanel;
    };

//......................................................................................................................
} } // namespace sd::toolpanel
//......................................................................................................................

#endif // SD_TOOLPANEL_TOOLPANELDRAWER_HXX
