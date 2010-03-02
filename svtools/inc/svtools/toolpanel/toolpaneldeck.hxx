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

#include "svtools/toolpanel/decklayouter.hxx"
#include "svtools/toolpanel/toolpanelcontainer.hxx"

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
        virtual void ActivePanelChanged( const ::boost::optional< size_t >& i_rOldActive, const size_t i_nNewActive ) = 0;
    };

    //====================================================================
    //= ToolPanelDeck
    //====================================================================
    class ToolPanelDeck : public Control
    {
    public:
        ToolPanelDeck( Window& i_rParent, const WinBits i_nStyle );
        ~ToolPanelDeck();

        // attributes
        PDeckLayouter       GetLayouter() const;
        void                SetLayouter( const PDeckLayouter& i_pNewLayouter );

        PToolPanelContainer GetPanels() const;

        size_t              GetActivePanel() const;
        void                ActivatePanel( const size_t i_nPanel );

        // listeners
        void                AddListener( IToolPanelDeckListener& i_rListener );
        void                RemoveListener( IToolPanelDeckListener& i_rListener );

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
