/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
