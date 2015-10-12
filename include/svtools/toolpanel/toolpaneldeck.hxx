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

#ifndef INCLUDED_SVTOOLS_TOOLPANEL_TOOLPANELDECK_HXX
#define INCLUDED_SVTOOLS_TOOLPANEL_TOOLPANELDECK_HXX

#include <svtools/svtdllapi.h>
#include <svtools/toolpanel/toolpanel.hxx>
#include <svtools/toolpanel/decklayouter.hxx>

#include <vcl/ctrl.hxx>

#include <boost/optional.hpp>
#include <memory>


namespace svt
{


    class ToolPanelCollection;
    class ToolPanelDeck_Impl;


    //= IToolPanelDeckListener

    class SAL_NO_VTABLE IToolPanelDeckListener
    {
    public:
        /** called when a panel has been inserted into the deck
        */
        virtual void PanelInserted( const PToolPanel& i_pPanel, const size_t i_nPosition ) = 0;

        /** called when a panel has been removed from the deck
        */
        virtual void PanelRemoved( const size_t i_nPosition ) = 0;

        /** called when the active panel of the deck changed
        */
        virtual void ActivePanelChanged( const ::boost::optional< size_t >& i_rOldActive, const ::boost::optional< size_t >& i_rNewActive ) = 0;

        /** called when a new layouter has been set at a tool panel deck.

            The method is called after the old layouter has been disposed (i.e. its Destroy method has been
            invoked), and after the complete deck has been re-layouter.
        */
        virtual void LayouterChanged( const PDeckLayouter& i_rNewLayouter ) = 0;

        /** called when the tool panel deck which the listener registered at is dying. The listener is required to
            release all references to the deck then.
        */
        virtual void Dying() = 0;

    protected:
        ~IToolPanelDeckListener() {}
    };


    //= IToolPanelDeck

    class SVT_DLLPUBLIC IToolPanelDeck
    {
    public:
        /** returns the number of panels in the container
        */
        virtual size_t      GetPanelCount() const = 0;

        /** retrieves the panel with the given index. Invalid indexes will be reported via an assertion in the
            non-product version, and silently ignored in the product version, with a NULL panel being returned.
        */
        virtual PToolPanel  GetPanel( const size_t i_nPos ) const = 0;

        /** returns the number of the currently active panel.
        */
        virtual ::boost::optional< size_t >
                            GetActivePanel() const = 0;

        /** activates the panel with the given number. If the given number is larger or equal to the number of panels
            in the deck, this will be reported via an assertion in non-product builds, and otherwise ignored.
            @param i_rPanel
                the number of the panel to activate. If this is not set, the currently active panel is de-activated,
                and no new panel is activated at all. Whether or not this makes sense for your application is at
                your own discretion.
        */
        virtual void        ActivatePanel( const ::boost::optional< size_t >& i_rPanel ) = 0;

        /** inserts a new panel into the container. NULL panels are not allowed, as are positions greater than the
            current panel count. Violations of this will be reported via an assertion in the non-product version, and
            silently ignored in the product version.
        */
        virtual size_t      InsertPanel( const PToolPanel& i_pPanel, const size_t i_nPosition ) = 0;

        /** removes a panel specified by its position.

            Note: It is the responsibility of the caller to ensure that the panel is destroyed appropriately. That is,
            the tool panel deck will <em>not</em> invoke <member>IToolPanel::Dispose</member> on the removed panel.
            The advantage is that the panel might be re-used later, with the disadvantage that the owner of the panel
            deck must know whether Dispose must be invoked after removal, or whether the panel will properly
            dispose itself when its ref count drops to 0.
        */
        virtual PToolPanel  RemovePanel( const size_t i_nPosition ) = 0;

        /** adds a new listener to be notified when the container content changes. The caller is responsible
            for life time control, i.e. removing the listener before it actually dies.
        */
        virtual void        AddListener( IToolPanelDeckListener& i_rListener ) = 0;

        /** removes a container listener previously added via addListener.
        */
        virtual void        RemoveListener( IToolPanelDeckListener& i_rListener ) = 0;

    protected:
        ~IToolPanelDeck() {}
    };


    //= ToolPanelDeck

    class SVT_DLLPUBLIC ToolPanelDeck   :public Control
                                        ,public IToolPanelDeck
    {
    public:
        ToolPanelDeck( vcl::Window& i_rParent, const WinBits i_nStyle = WB_DIALOGCONTROL );
        virtual ~ToolPanelDeck();
        virtual void dispose() override;

        // attributes
        PDeckLayouter       GetLayouter() const;
        void                SetLayouter( const PDeckLayouter& i_pNewLayouter );

        /** returns the window which acts as anchor for the panel windows.

            This is a single dedicated window, which is passed to the IToolPanel::ActivatePanel method
            whenever a panel is activated, to act as parent window for the panel's VCL-Window.
        */
        vcl::Window&           GetPanelWindowAnchor();
        const vcl::Window&     GetPanelWindowAnchor() const;

        // IToolPanelDeck
        virtual size_t      GetPanelCount() const override;
        virtual PToolPanel  GetPanel( const size_t i_nPos ) const override;
        virtual ::boost::optional< size_t >
                            GetActivePanel() const override;
        virtual void        ActivatePanel( const ::boost::optional< size_t >& i_rPanel ) override;
        virtual size_t      InsertPanel( const PToolPanel& i_pPanel, const size_t i_nPosition ) override;
        virtual PToolPanel  RemovePanel( const size_t i_nPosition ) override;
        virtual void        AddListener( IToolPanelDeckListener& i_rListener ) override;
        virtual void        RemoveListener( IToolPanelDeckListener& i_rListener ) override;

    protected:
        // Window overridables
        virtual void Resize() override;
        virtual bool Notify( NotifyEvent& i_rNotifyEvent ) override;
        virtual void GetFocus() override;

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >
                     GetComponentInterface( bool i_bCreate ) override;

    private:
        ::std::unique_ptr< ToolPanelDeck_Impl >   m_pImpl;
    };


} // namespace svt


#endif // INCLUDED_SVTOOLS_TOOLPANEL_TOOLPANELDECK_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
