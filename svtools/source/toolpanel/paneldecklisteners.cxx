/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
