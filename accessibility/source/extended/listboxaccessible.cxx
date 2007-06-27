/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: listboxaccessible.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-06-27 15:34:31 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_accessibility.hxx"

#ifndef ACCESSIBILITY_EXT_LISTBOX_ACCESSIBLE
#include <accessibility/extended/listboxaccessible.hxx>
#endif
#ifndef _SVTREEBOX_HXX
#include <svtools/svtreebx.hxx>
#endif

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
        DBG_ASSERT( pEvent && pEvent->ISA( VclWindowEvent ), "ListBoxAccessibleBase::WindowEventListener: unexpected WindowEvent!" );
        if ( pEvent && pEvent->ISA( VclWindowEvent ) )
        {
            DBG_ASSERT( static_cast< VclWindowEvent* >( pEvent )->GetWindow() , "ListBoxAccessibleBase::WindowEventListener: no event window!" );
            DBG_ASSERT( static_cast< VclWindowEvent* >( pEvent )->GetWindow() == m_pWindow, "ListBoxAccessibleBase::WindowEventListener: where did this come from?" );

            ProcessWindowEvent( *static_cast< VclWindowEvent* >( pEvent ) );
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
