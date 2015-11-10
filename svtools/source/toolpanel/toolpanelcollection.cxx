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


#include "toolpanelcollection.hxx"
#include "paneldecklisteners.hxx"

#include <tools/diagnose_ex.h>

#include <vector>


namespace svt
{



    //= ToolPanelCollection_Data

    struct ToolPanelCollection_Data
    {
        ::std::vector< PToolPanel >                 aPanels;
        ::boost::optional< size_t >                 aActivePanel;
        PanelDeckListeners                          aListeners;
    };


    //= ToolPanelCollection


    ToolPanelCollection::ToolPanelCollection()
        :m_pData( new ToolPanelCollection_Data )
    {
    }


    ToolPanelCollection::~ToolPanelCollection()
    {
        m_pData->aListeners.Dying();
    }


    size_t ToolPanelCollection::GetPanelCount() const
    {
        return m_pData->aPanels.size();
    }


    ::boost::optional< size_t > ToolPanelCollection::GetActivePanel() const
    {
        return m_pData->aActivePanel;
    }


    void ToolPanelCollection::ActivatePanel( const ::boost::optional< size_t >& i_rPanel )
    {
        if ( !!i_rPanel )
        {
            OSL_ENSURE( *i_rPanel < GetPanelCount(), "ToolPanelCollection::ActivatePanel: illegal panel no.!" );
            if ( *i_rPanel >= GetPanelCount() )
                return;
        }

        if ( m_pData->aActivePanel == i_rPanel )
            return;

        const ::boost::optional< size_t > aOldPanel( m_pData->aActivePanel );
        m_pData->aActivePanel = i_rPanel;

        // notify listeners
        m_pData->aListeners.ActivePanelChanged( aOldPanel, m_pData->aActivePanel );
    }


    PToolPanel ToolPanelCollection::GetPanel( const size_t i_nPos ) const
    {
        OSL_ENSURE( i_nPos < m_pData->aPanels.size(), "ToolPanelCollection::GetPanel: illegal position!" );
        if ( i_nPos >= m_pData->aPanels.size() )
            return PToolPanel();
        return m_pData->aPanels[ i_nPos ];
    }


    size_t ToolPanelCollection::InsertPanel( const PToolPanel& i_pPanel, const size_t i_nPosition )
    {
        OSL_ENSURE( i_pPanel.get(), "ToolPanelCollection::InsertPanel: illegal panel!" );
        if ( !i_pPanel.get() )
            return 0;

        // insert
        const size_t position = i_nPosition < m_pData->aPanels.size() ? i_nPosition : m_pData->aPanels.size();
        m_pData->aPanels.insert( m_pData->aPanels.begin() + position, i_pPanel );

        // update active panel
        if ( !!m_pData->aActivePanel )
        {
            if ( i_nPosition <= *m_pData->aActivePanel )
                ++*m_pData->aActivePanel;
        }

        // notifications
        m_pData->aListeners.PanelInserted( i_pPanel, i_nPosition );

        return position;
    }


    PToolPanel ToolPanelCollection::RemovePanel( const size_t i_nPosition )
    {
        OSL_ENSURE( i_nPosition < m_pData->aPanels.size(), "ToolPanelCollection::RemovePanel: illegal position!" );
        if ( i_nPosition >= m_pData->aPanels.size() )
            return nullptr;

        // if the active panel is going to be removed, activate another one (before the actual removal)
        if ( m_pData->aActivePanel == i_nPosition )
        {
            const ::boost::optional< size_t > aOldActive( m_pData->aActivePanel );

            if ( i_nPosition + 1 < GetPanelCount() )
            {
                ++*m_pData->aActivePanel;
            }
            else if ( i_nPosition > 0 )
            {
                --*m_pData->aActivePanel;
            }
            else
            {
                m_pData->aActivePanel.reset();
            }

            m_pData->aListeners.ActivePanelChanged( aOldActive, m_pData->aActivePanel );
        }

        // remember the removed panel for the aller
        PToolPanel pRemovedPanel( m_pData->aPanels[ i_nPosition ] );

        // actually remove
        m_pData->aPanels.erase( m_pData->aPanels.begin() + i_nPosition );

        if ( !!m_pData->aActivePanel )
        {
            if ( i_nPosition < *m_pData->aActivePanel )
            {
                --*m_pData->aActivePanel;
            }
        }

        // notify removed panel
        m_pData->aListeners.PanelRemoved( i_nPosition );

        return pRemovedPanel;
    }


    void ToolPanelCollection::AddListener( IToolPanelDeckListener& i_rListener )
    {
        m_pData->aListeners.AddListener( i_rListener );
    }


    void ToolPanelCollection::RemoveListener( IToolPanelDeckListener& i_rListener )
    {
        m_pData->aListeners.RemoveListener( i_rListener );
    }


} // namespace svt


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
