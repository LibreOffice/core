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

#ifndef PANELDECKLISTENERS_HXX
#define PANELDECKLISTENERS_HXX

#include "svtools/toolpanel/toolpaneldeck.hxx"

#include <boost/optional.hpp>
#include <vector>

//........................................................................
namespace svt
{
//........................................................................

    class IToolPanelDeckListener;

    //====================================================================
    //= PanelDeckListeners
    //====================================================================
    /** implements a container for IToolPanelDeckListeners
    */
    class PanelDeckListeners
    {
    public:
        PanelDeckListeners();
        ~PanelDeckListeners();

        // IToolPanelDeckListener equivalents, forward the events to all registered listeners
        void    PanelInserted( const PToolPanel& i_pPanel, const size_t i_nPosition );
        void    PanelRemoved( const size_t i_nPosition );
        void    ActivePanelChanged( const ::boost::optional< size_t >& i_rOldActive, const ::boost::optional< size_t >& i_rNewActive );
        void    LayouterChanged( const PDeckLayouter& i_rNewLayouter );
        void    Dying();

        // listener maintainance
        void    AddListener( IToolPanelDeckListener& i_rListener );
        void    RemoveListener( IToolPanelDeckListener& i_rListener );

    private:
        ::std::vector< IToolPanelDeckListener* >    m_aListeners;
    };

//........................................................................
} // namespace svt
//........................................................................

#endif // PANELDECKLISTENERS_HXX
