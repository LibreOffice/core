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

#include <uielement/panelwindow.hxx>

namespace framework
{

PanelWindow::PanelWindow( Window* pParent, WinBits nWinBits ) :
    DockingWindow( pParent, nWinBits )
{
}

PanelWindow::~PanelWindow()
{
}

const ::rtl::OUString& PanelWindow::getResourceURL() const
{
    return m_aResourceURL;
}

void PanelWindow::setResourceURL(const ::rtl::OUString& rResourceURL)
{
    m_aResourceURL = rResourceURL;
}

Window* PanelWindow::getContentWindow() const
{
    return m_pContentWindow;
}

void PanelWindow::setContentWindow( Window* pContentWindow )
{
    m_pContentWindow = pContentWindow;
    if ( m_pContentWindow != NULL )
    {
        m_pContentWindow->SetParent(this);
        m_pContentWindow->SetSizePixel( GetOutputSizePixel() );
        m_pContentWindow->Show();
    }
}

void PanelWindow::Command( const CommandEvent& rCEvt )
{
    if ( m_aCommandHandler.IsSet() )
        m_aCommandHandler.Call( (void *)( &rCEvt ));
    DockingWindow::Command( rCEvt );
}

void PanelWindow::StateChanged( StateChangedType nType )
{
    DockingWindow::StateChanged( nType );
    if ( m_aStateChangedHandler.IsSet() )
        m_aStateChangedHandler.Call( &nType );
}

void PanelWindow::DataChanged( const DataChangedEvent& rDCEvt )
{
    DockingWindow::DataChanged( rDCEvt );
    if ( m_aDataChangedHandler.IsSet() )
        m_aDataChangedHandler.Call( (void*)&rDCEvt );
}

void PanelWindow::Resize()
{
    DockingWindow::Resize();
    if ( m_pContentWindow )
        m_pContentWindow->SetSizePixel( GetOutputSizePixel() );
}

}
