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

#ifndef INCLUDED_SVTOOLS_TOOLPANEL_DRAWERLAYOUTER_HXX
#define INCLUDED_SVTOOLS_TOOLPANEL_DRAWERLAYOUTER_HXX

#include <svtools/svtdllapi.h>
#include <svtools/toolpanel/toolpaneldeck.hxx>
#include <svtools/toolpanel/decklayouter.hxx>

#include <memory>


namespace svt
{
    class ToolPanelViewShell;
    class ToolPanelDrawer;

    //= ToolPanelDrawer

    /** a class which implements a tool panel selector in the form of the classical drawers
    */
    class SVT_DLLPUBLIC DrawerDeckLayouter  :public IDeckLayouter
                                            ,public IToolPanelDeckListener
    {
    public:
        DrawerDeckLayouter(
            vcl::Window& i_rParentWindow,
            IToolPanelDeck& i_rPanels
        );
        virtual ~DrawerDeckLayouter();

        // IDeckLayouter
        virtual Rectangle   Layout( const Rectangle& i_rDeckPlayground ) SAL_OVERRIDE;
        virtual void        Destroy() SAL_OVERRIDE;
        virtual void        SetFocusToPanelSelector() SAL_OVERRIDE;
        virtual size_t      GetAccessibleChildCount() const SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
                            GetAccessibleChild(
                                const size_t i_nChildIndex,
                                const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& i_rParentAccessible
                            ) SAL_OVERRIDE;

        // IToolPanelDeckListener
        virtual void PanelInserted( const PToolPanel& i_pPanel, const size_t i_nPosition ) SAL_OVERRIDE;
        virtual void PanelRemoved( const size_t i_nPosition ) SAL_OVERRIDE;
        virtual void ActivePanelChanged( const ::boost::optional< size_t >& i_rOldActive, const ::boost::optional< size_t >& i_rNewActive ) SAL_OVERRIDE;
        virtual void LayouterChanged( const PDeckLayouter& i_rNewLayouter ) SAL_OVERRIDE;
        virtual void Dying() SAL_OVERRIDE;

    private:
        // triggers a re-arrange of the panel deck elements
        void    impl_triggerRearrange() const;
        size_t  impl_getPanelPositionFromWindow( const vcl::Window* i_pDrawerWindow ) const;
        void    impl_removeDrawer( const size_t i_nPosition );

        DECL_LINK_TYPED( OnWindowEvent, VclWindowEvent&, void );

private:
        vcl::Window&                             m_rParentWindow;
        IToolPanelDeck&                     m_rPanelDeck;
        ::std::vector< VclPtr<ToolPanelDrawer> >   m_aDrawers;
        ::boost::optional< size_t >         m_aLastKnownActivePanel;
    };


} // namespace svt


#endif // INCLUDED_SVTOOLS_TOOLPANEL_DRAWERLAYOUTER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
