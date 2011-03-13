/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: layoutmanager.hxx,v $
 * $Revision: 1.34 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_framework.hxx"

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#include "panelmanager.hxx"
#include "services.h"
#include "services/modelwinservice.hxx"

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________


//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#include <vcl/svapp.hxx>
#include <toolkit/unohlp.hxx>

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

using namespace ::com::sun::star;

namespace framework
{

//_________________________________________________________________________________________________________________
//  non exported definitions
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  declarations
//_________________________________________________________________________________________________________________


PanelManager::PanelManager(
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rSMGR,
    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame ) :
    m_xSMGR( rSMGR ),
    m_xFrame( rFrame )
{
    m_aPanels[PANEL_TOP]    = 0;
    m_aPanels[PANEL_BOTTOM] = 0;
    m_aPanels[PANEL_LEFT]   = 0;
    m_aPanels[PANEL_RIGHT]  = 0;
}

PanelManager::~PanelManager()
{
}

//---------------------------------------------------------------------------------------------------------
// Creation and layouting
//---------------------------------------------------------------------------------------------------------
bool PanelManager::createPanels()
{
    if ( m_xFrame.is() )
    {
        SolarMutexGuard aGuard;
        uno::Reference< awt::XWindow > xWindow( m_xFrame->getContainerWindow(), uno::UNO_QUERY );
        if ( xWindow.is() )
        {
            // destroy old panel windows
            delete m_aPanels[PANEL_TOP   ];
            delete m_aPanels[PANEL_BOTTOM];
            delete m_aPanels[PANEL_LEFT  ];
            delete m_aPanels[PANEL_RIGHT ];

            m_aPanels[PANEL_TOP   ] = new Panel( m_xSMGR, xWindow, PANEL_TOP    );
            m_aPanels[PANEL_BOTTOM] = new Panel( m_xSMGR, xWindow, PANEL_BOTTOM );
            m_aPanels[PANEL_LEFT  ] = new Panel( m_xSMGR, xWindow, PANEL_LEFT   );
            m_aPanels[PANEL_RIGHT ] = new Panel( m_xSMGR, xWindow, PANEL_RIGHT  );
            return true;
        }
    }

    return false;
}

awt::Rectangle PanelManager::getPreferredSize() const
{
    return awt::Rectangle();
}

void PanelManager::layoutPanels( const awt::Rectangle /*newSize*/ )
{
}

//---------------------------------------------------------------------------------------------------------
//  Panel functions
//---------------------------------------------------------------------------------------------------------
UIElement* PanelManager::findDockingWindow( const ::rtl::OUString& /*rResourceName*/ )
{
    return NULL;
}

bool PanelManager::addDockingWindow( const ::rtl::OUString& /*rResourceName*/, const ::com::sun::star::uno::Reference< ::com::sun::star::ui::XUIElement >& /*xUIElement*/ )
{
    return false;
}

bool PanelManager::destroyDockingWindow( const ::rtl::OUString& /*rResourceName*/ )
{
    return false;
}

//---------------------------------------------------------------------------------------------------------
//  XDockableWindowListener
//---------------------------------------------------------------------------------------------------------
void SAL_CALL PanelManager::startDocking( const awt::DockingEvent& )
throw (uno::RuntimeException)
{
}

awt::DockingData SAL_CALL PanelManager::docking( const awt::DockingEvent& )
throw (uno::RuntimeException)
{
    return awt::DockingData();
}

void SAL_CALL PanelManager::endDocking( const awt::EndDockingEvent& )
throw (uno::RuntimeException)
{
}

sal_Bool SAL_CALL PanelManager::prepareToggleFloatingMode( const lang::EventObject& )
throw (uno::RuntimeException)
{
    return false;
}

void SAL_CALL PanelManager::toggleFloatingMode( const lang::EventObject& )
throw (uno::RuntimeException)
{
}

void SAL_CALL PanelManager::closed( const lang::EventObject& )
throw (uno::RuntimeException)
{
}

void SAL_CALL PanelManager::endPopupMode( const awt::EndPopupModeEvent& )
throw (uno::RuntimeException)
{
}

} // namespace framework
