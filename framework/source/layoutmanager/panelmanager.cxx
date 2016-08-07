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
#include <toolkit/helper/vclunohelper.hxx>

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
        vos::OGuard aGuard( Application::GetSolarMutex() );
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
