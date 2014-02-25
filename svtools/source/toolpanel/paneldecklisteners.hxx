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

#ifndef INCLUDED_SVTOOLS_SOURCE_TOOLPANEL_PANELDECKLISTENERS_HXX
#define INCLUDED_SVTOOLS_SOURCE_TOOLPANEL_PANELDECKLISTENERS_HXX

#include "svtools/toolpanel/toolpaneldeck.hxx"

#include <boost/optional.hpp>
#include <vector>


namespace svt
{


    class IToolPanelDeckListener;


    //= PanelDeckListeners

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


} // namespace svt


#endif // INCLUDED_SVTOOLS_SOURCE_TOOLPANEL_PANELDECKLISTENERS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
