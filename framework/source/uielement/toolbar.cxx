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
#include "precompiled_framework.hxx"

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________
#include <uielement/toolbar.hxx>
#include <uielement/toolbarmanager.hxx>
#include <rtl/logfile.hxx>

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
