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

#include "toolpanelcollection.hxx"

#include <tools/diagnose_ex.h>

#include <vector>

//........................................................................
namespace svt
{
//........................................................................

    //====================================================================
    //= ToolPanelCollection_Data
    //====================================================================
    struct ToolPanelCollection_Data
    {
        ::std::vector< PToolPanel >                     aPanels;
        ::std::vector< IToolPanelContainerListener* >   aListeners;
    };

    //====================================================================
    //= ToolPanelCollection
    //====================================================================
    //--------------------------------------------------------------------
    ToolPanelCollection::ToolPanelCollection()
        :m_pData( new ToolPanelCollection_Data )
    {
    }

    //--------------------------------------------------------------------
    ToolPanelCollection::~ToolPanelCollection()
    {
    }

    //--------------------------------------------------------------------
    IMPLEMENT_IREFERENCE( ToolPanelCollection )

    //--------------------------------------------------------------------
    size_t ToolPanelCollection::GetPanelCount() const
    {
        return m_pData->aPanels.size();
    }

    //--------------------------------------------------------------------
    PToolPanel ToolPanelCollection::GetPanel( const size_t i_nPos ) const
    {
        OSL_ENSURE( i_nPos < m_pData->aPanels.size(), "ToolPanelCollection::GetPanel: illegal position!" );
        if ( i_nPos >= m_pData->aPanels.size() )
            return PToolPanel();
        return m_pData->aPanels[ i_nPos ];
    }

    //--------------------------------------------------------------------
    size_t ToolPanelCollection::InsertPanel( const PToolPanel& i_pPanel, const size_t i_nPosition )
    {
        OSL_ENSURE( i_pPanel.get(), "ToolPanelCollection::InsertPanel: illegal panel!" );
        if ( !i_pPanel.get() )
            return 0;

        const size_t position = i_nPosition < m_pData->aPanels.size() ? i_nPosition : m_pData->aPanels.size();
        m_pData->aPanels.insert( m_pData->aPanels.begin() + position, i_pPanel );

        // notifications
        for (   ::std::vector< IToolPanelContainerListener* >::const_iterator loop = m_pData->aListeners.begin();
                loop != m_pData->aListeners.end();
                ++loop
            )
        {
            (*loop)->PanelInserted( i_pPanel, i_nPosition );
        }

        return position;
    }

    //--------------------------------------------------------------------
    void ToolPanelCollection::AddListener( IToolPanelContainerListener& i_rListener )
    {
        m_pData->aListeners.push_back( &i_rListener );
    }

    //--------------------------------------------------------------------
    void ToolPanelCollection::RemoveListener( IToolPanelContainerListener& i_rListener )
    {
        for (   ::std::vector< IToolPanelContainerListener* >::iterator lookup = m_pData->aListeners.begin();
                lookup != m_pData->aListeners.end();
                ++lookup
            )
        {
            if ( *lookup == &i_rListener )
            {
                m_pData->aListeners.erase( lookup );
                return;
            }
        }
    }

//........................................................................
} // namespace svt
//........................................................................
