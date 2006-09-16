/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: toolbar.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 14:25:29 $
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
#include "precompiled_framework.hxx"

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_UIELEMENT_TOOLBAR_HXX_
#include <uielement/toolbar.hxx>
#endif

#ifndef __FRAMEWORK_UIELEMENT_TOOLBARMANAGER_HXX_
#include <uielement/toolbarmanager.hxx>
#endif

namespace framework
{

ToolBar::ToolBar( Window* pParent, WinBits nWinBits ) :
    ToolBox( pParent, nWinBits )
    ,   m_pToolBarManager( 0 )
{
}

ToolBar::~ToolBar()
{
}

void ToolBar::SetToolBarManager( ToolBarManager* pTbMgr )
{
    m_pToolBarManager = pTbMgr;
}

void ToolBar::Command( const CommandEvent& rCEvt )
{
    if ( m_aCommandHandler.IsSet() )
        m_aCommandHandler.Call( (void *)( &rCEvt ));
    ToolBox::Command( rCEvt );
}

void ToolBar::StateChanged( StateChangedType nType )
{
    ToolBox::StateChanged( nType );
    if ( m_aStateChangedHandler.IsSet() )
        m_aStateChangedHandler.Call( &nType );
}

void ToolBar::DataChanged( const DataChangedEvent& rDCEvt )
{
    ToolBox::DataChanged( rDCEvt );
    if ( m_aDataChangedHandler.IsSet() )
        m_aDataChangedHandler.Call( (void*)&rDCEvt );
}

}
