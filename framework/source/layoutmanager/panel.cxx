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

#include "panel.hxx"
#include "helpers.hxx"

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#include <vos/mutex.hxx>
#include <vcl/svapp.hxx>
#include <toolkit/unohlp.hxx>

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
