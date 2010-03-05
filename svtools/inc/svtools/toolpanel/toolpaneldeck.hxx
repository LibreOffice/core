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

#ifndef TOOLPANELDECK_HXX
#define TOOLPANELDECK_HXX

#include "svtools/toolpanel/toolpanel.hxx"
#include "svtools/toolpanel/decklayouter.hxx"

#include <vcl/ctrl.hxx>

#include <boost/optional.hpp>
#include <memory>

//........................................................................
namespace svt
{
//........................................................................

    class ToolPanelCollection;
    class ToolPanelDeck_Impl;

    //====================================================================
    //= IToolPanelDeckListener
    //====================================================================
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

        /** called when the tool panel deck which the listener registered at is dying. The listener is required to
            release all references to the deck then.
        */
        virtual void Dying() = 0;
    };

    //====================================================================
    //= IToolPanelDeck
    //====================================================================
    class SAL_NO_VTABLE IToolPanelDeck
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
                            GetActivePanel() const;

        /** activates the panel with the given number. If the given number is larger or equal to the number of panels
            in the deck, this will be reported via an assertion in non-product builds, and otherwise ignored.
        */
        virtual void        ActivatePanel( const size_t i_nPanel ) = 0;

        /** inserts a new panel into the container. NULL panels are not allowed, as are positions greater than the
            current panel count. Violations of this will be reported via an assertion in the non-product version, and
            silently ignored in the product version.
        */
        virtual size_t      InsertPanel( const PToolPanel& i_pPanel, const size_t i_nPosition ) = 0;

        /** removes a panel specified by its position.
        */
        virtual void        RemovePanel( const size_t i_nPosition ) = 0;

        /** adds a new listener to be notified when the container content changes. The caller is responsible
            for life time control, i.e. removing the listener before it actually dies.
        */
        virtual void        AddListener( IToolPanelDeckListener& i_rListener ) = 0;

        /** removes a container listener previously added via addListener.
        */
        virtual void        RemoveListener( IToolPanelDeckListener& i_rListener ) = 0;
    };

    //====================================================================
    //= ToolPanelDeck
    //====================================================================
    class ToolPanelDeck :public Control
                        ,public IToolPanelDeck
    {
    public:
        ToolPanelDeck( Window& i_rParent, const WinBits i_nStyle );
        ~ToolPanelDeck();

        // attributes
        PDeckLayouter       GetLayouter() const;
        void                SetLayouter( const PDeckLayouter& i_pNewLayouter );

        // IToolPanelDeck
        virtual size_t      GetPanelCount() const;
        virtual PToolPanel  GetPanel( const size_t i_nPos ) const;
        virtual ::boost::optional< size_t >
                            GetActivePanel() const;
        virtual void        ActivatePanel( const size_t i_nPanel );
        virtual size_t      InsertPanel( const PToolPanel& i_pPanel, const size_t i_nPosition );
        virtual void        RemovePanel( const size_t i_nPosition );
        virtual void        AddListener( IToolPanelDeckListener& i_rListener );
        virtual void        RemoveListener( IToolPanelDeckListener& i_rListener );

    protected:
        // Window overridables
        virtual void Resize();
        virtual long Notify( NotifyEvent& i_rNotifyEvent );
        virtual void GetFocus();

    private:
        ::std::auto_ptr< ToolPanelDeck_Impl >   m_pImpl;
    };

//........................................................................
} // namespace svt
//........................................................................

#endif // TOOLPANELDECK_HXX
