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

#include "panel.hxx"
#include "helpers.hxx"

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#include <vos/mutex.hxx>
#include <vcl/svapp.hxx>
#include <toolkit/helper/vclunohelper.hxx>

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

using namespace ::com::sun::star;

namespace framework
{

Panel::Panel(
    const uno::Reference< lang::XMultiServiceFactory >& rSMGR,
    const uno::Reference< awt::XWindow >& rParent,
    PanelPosition nPanel ) :
    m_xSMGR(rSMGR), m_nPanelPosition(nPanel)
{
    uno::Reference< awt::XWindowPeer > xWindowPeer( rParent, uno::UNO_QUERY );
    m_xPanelWindow = uno::Reference< awt::XWindow >( createToolkitWindow( rSMGR, xWindowPeer, "splitwindow" ), uno::UNO_QUERY );

    vos::OGuard aGuard( Application::GetSolarMutex() );
    SplitWindow* pSplitWindow = dynamic_cast< SplitWindow* >( VCLUnoHelper::GetWindow( m_xPanelWindow ));

    if ( pSplitWindow )
    {
        // Set alignment
        if (nPanel == PANEL_TOP)
            pSplitWindow->SetAlign( WINDOWALIGN_TOP );
        else if (nPanel == PANEL_BOTTOM)
            pSplitWindow->SetAlign( WINDOWALIGN_BOTTOM );
        else if (nPanel == PANEL_LEFT)
            pSplitWindow->SetAlign( WINDOWALIGN_LEFT );
        else
            pSplitWindow->SetAlign( WINDOWALIGN_RIGHT );
    }
}

Panel::~Panel()
{
}

} // namespace framework
