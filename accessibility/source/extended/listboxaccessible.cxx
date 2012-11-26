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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_accessibility.hxx"
#include <accessibility/extended/listboxaccessible.hxx>
#include <svtools/svtreebx.hxx>

//........................................................................
namespace accessibility
{
//........................................................................

    //====================================================================
    //= ListBoxAccessibleBase
    //====================================================================
    //--------------------------------------------------------------------
    ListBoxAccessibleBase::ListBoxAccessibleBase( SvTreeListBox& _rWindow )
        :m_pWindow( &_rWindow )
    {
        m_pWindow->AddEventListener( LINK( this, ListBoxAccessibleBase, WindowEventListener ) );
    }

    //--------------------------------------------------------------------
    ListBoxAccessibleBase::~ListBoxAccessibleBase( )
    {
        if ( m_pWindow )
        {
            // cannot call "dispose" here, as it is abstract, so the VTABLE of the derived class
            // is not intact anymore
            // so we call our "disposing" only
            disposing();
        }
    }

    //--------------------------------------------------------------------
    IMPL_LINK( ListBoxAccessibleBase, WindowEventListener, VclSimpleEvent*, pEvent )
    {
        VclWindowEvent* pVclWindowEvent = dynamic_cast< VclWindowEvent* >(pEvent);
        DBG_ASSERT( pVclWindowEvent, "ListBoxAccessibleBase::WindowEventListener: unexpected WindowEvent!" );

        if ( pVclWindowEvent )
        {
            DBG_ASSERT( pVclWindowEvent->GetWindow() , "ListBoxAccessibleBase::WindowEventListener: no event window!" );
            DBG_ASSERT( pVclWindowEvent->GetWindow() == m_pWindow, "ListBoxAccessibleBase::WindowEventListener: where did this come from?" );

            ProcessWindowEvent( *pVclWindowEvent );
        }
        return 0;
    }

    // -----------------------------------------------------------------------------
    void ListBoxAccessibleBase::disposing()
    {
        if ( m_pWindow )
            m_pWindow->RemoveEventListener( LINK( this, ListBoxAccessibleBase, WindowEventListener ) );
        m_pWindow = NULL;
    }

    // -----------------------------------------------------------------------------
    void ListBoxAccessibleBase::ProcessWindowEvent( const VclWindowEvent& _rVclWindowEvent )
    {
        if ( isAlive() )
        {
            switch ( _rVclWindowEvent.GetId() )
            {
                case  VCLEVENT_OBJECT_DYING :
                {
                    if ( m_pWindow )
                        m_pWindow->RemoveEventListener( LINK( this, ListBoxAccessibleBase, WindowEventListener ) );
                    m_pWindow = NULL;
                    dispose();
                    break;
                }
            }
        }
    }

//........................................................................
}   // namespace accessibility
//........................................................................
