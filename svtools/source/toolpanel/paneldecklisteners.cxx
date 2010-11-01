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

#include "precompiled_svtools.hxx"

#include "paneldecklisteners.hxx"
#include "svtools/toolpanel/toolpaneldeck.hxx"

//........................................................................
namespace svt
{
//........................................................................

    //====================================================================
    //= PanelDeckListeners
    //====================================================================
    //--------------------------------------------------------------------
    PanelDeckListeners::PanelDeckListeners()
    {
    }

    //--------------------------------------------------------------------
    PanelDeckListeners::~PanelDeckListeners()
    {
    }

    //--------------------------------------------------------------------
    void PanelDeckListeners::PanelInserted( const PToolPanel& i_pPanel, const size_t i_nPosition )
    {
        ::std::vector< IToolPanelDeckListener* > aListeners( m_aListeners );
        for (   ::std::vector< IToolPanelDeckListener* >::const_iterator loop = aListeners.begin();
                loop != aListeners.end();
                ++loop
            )
        {
            (*loop)->PanelInserted( i_pPanel, i_nPosition );
        }
    }

    //--------------------------------------------------------------------
    void PanelDeckListeners::PanelRemoved( const size_t i_nPosition )
    {
        ::std::vector< IToolPanelDeckListener* > aListeners( m_aListeners );
        for (   ::std::vector< IToolPanelDeckListener* >::const_iterator loop = aListeners.begin();
                loop != aListeners.end();
                ++loop
            )
        {
            (*loop)->PanelRemoved( i_nPosition );
        }
    }

    //--------------------------------------------------------------------
    void PanelDeckListeners::ActivePanelChanged( const ::boost::optional< size_t >& i_rOldActive, const ::boost::optional< size_t >& i_rNewActive )
    {
        ::std::vector< IToolPanelDeckListener* > aListeners( m_aListeners );
        for (   ::std::vector< IToolPanelDeckListener* >::const_iterator loop = aListeners.begin();
                loop != aListeners.end();
                ++loop
            )
        {
            (*loop)->ActivePanelChanged( i_rOldActive, i_rNewActive );
        }
    }

    //--------------------------------------------------------------------
    void PanelDeckListeners::LayouterChanged( const PDeckLayouter& i_rNewLayouter )
    {
        ::std::vector< IToolPanelDeckListener* > aListeners( m_aListeners );
        for (   ::std::vector< IToolPanelDeckListener* >::const_iterator loop = aListeners.begin();
                loop != aListeners.end();
                ++loop
            )
        {
            (*loop)->LayouterChanged( i_rNewLayouter );
        }
    }

    //--------------------------------------------------------------------
    void PanelDeckListeners::Dying()
    {
        while ( !m_aListeners.empty() )
        {
            IToolPanelDeckListener* pListener( *m_aListeners.begin() );
            m_aListeners.erase( m_aListeners.begin() );
            pListener->Dying();
        }
    }

    //--------------------------------------------------------------------
    void PanelDeckListeners::AddListener( IToolPanelDeckListener& i_rListener )
    {
        m_aListeners.push_back( &i_rListener );
    }

    //--------------------------------------------------------------------
    void PanelDeckListeners::RemoveListener( IToolPanelDeckListener& i_rListener )
    {
        for (   ::std::vector< IToolPanelDeckListener* >::iterator lookup = m_aListeners.begin();
                lookup != m_aListeners.end();
                ++lookup
            )
        {
            if ( *lookup == &i_rListener )
            {
                m_aListeners.erase( lookup );
                return;
            }
        }
    }

//........................................................................
} // namespace svt
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
